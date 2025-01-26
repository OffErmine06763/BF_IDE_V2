#include "Editor.h"

Editor::Editor()
{
	Documents.push_back(MyDocument(0, "Lettuce", true, ImVec4(0.4f, 0.8f, 0.4f, 1.0f)));
	Documents.push_back(MyDocument(1, "Eggplant", true, ImVec4(0.8f, 0.5f, 1.0f, 1.0f)));
	Documents.push_back(MyDocument(2, "Carrot", true, ImVec4(1.0f, 0.8f, 0.5f, 1.0f)));
	Documents.push_back(MyDocument(3, "Tomato", false, ImVec4(1.0f, 0.3f, 0.4f, 1.0f)));
	Documents.push_back(MyDocument(4, "A Rather Long Title", false, ImVec4(0.4f, 0.8f, 0.8f, 1.0f)));
	Documents.push_back(MyDocument(5, "Some Document", false, ImVec4(0.8f, 0.8f, 1.0f, 1.0f)));
}

void Editor::Render()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("App"))
		{
			if (ImGui::MenuItem("Redock"))
				m_WantRedock = true;
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("File"))
		{
			int open_count = 0;
			for (MyDocument& doc : Documents)
				open_count += doc.Open ? 1 : 0;

			if (ImGui::BeginMenu("Open", open_count < Documents.size()))
			{
				for (MyDocument& doc : Documents)
					if (!doc.Open && ImGui::MenuItem(doc.Name))
						doc.DoOpen();
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Close All Documents", NULL, false, open_count > 0))
				for (MyDocument& doc : Documents)
					CloseQueue.push_back(&doc);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}


	static bool opt_reorderable = true;
	static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);

	// When (opt_target == Target_DockSpaceAndWindow) there is the possibily that one of our child Document window (e.g. "Eggplant")
	// that we emit gets docked into the same spot as the parent window ("Example: Documents").
	// This would create a problematic feedback loop because selecting the "Eggplant" tab would make the "Example: Documents" tab
	// not visible, which in turn would stop submitting the "Eggplant" window.
	// We avoid this problem by submitting our documents window even if our parent window is not currently visible.
	// Another solution may be to make the "Example: Documents" window use the ImGuiWindowFlags_NoDocking.

	bool window_contents_visible = ImGui::Begin("Documents", nullptr, flags);

	// Tabs
	NotifyOfDocumentsClosedElsewhere();

	// Create a DockSpace node where any window can be docked
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dockspace_id, { 0, 0 }, ImGuiDockNodeFlags_None);

	// Create Windows
	for (int doc_n = 0; doc_n < Documents.size(); doc_n++)
	{
		MyDocument* doc = &Documents[doc_n];
		if (!doc->Open)
			continue;

		ImGui::SetNextWindowDockID(dockspace_id, m_WantRedock ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
		ImGuiWindowFlags window_flags = (doc->Dirty ? ImGuiWindowFlags_UnsavedDocument : 0);
		bool visible = ImGui::Begin(doc->Name, &doc->Open, window_flags);

		// Cancel attempt to close when unsaved add to save queue so we can display a popup.
		if (!doc->Open && doc->Dirty)
		{
			doc->Open = true;
			CloseQueue.push_back(doc);
		}

		DisplayDocContextMenu(doc);
		if (visible)
			DisplayDocContents(doc);

		ImGui::End();
	}
	m_WantRedock = false;


	// Early out other contents
	if (!window_contents_visible)
	{
		ImGui::End();
		return;
	}

	// Display renaming UI
	if (RenamingDoc != NULL)
	{
		if (RenamingStarted)
			ImGui::OpenPopup("Rename");
		if (ImGui::BeginPopup("Rename"))
		{
			ImGui::SetNextItemWidth(ImGui::GetFontSize() * 30);
			if (ImGui::InputText("###Name", RenamingDoc->Name, IM_ARRAYSIZE(RenamingDoc->Name), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				ImGui::CloseCurrentPopup();
				RenamingDoc = NULL;
			}
			if (RenamingStarted)
				ImGui::SetKeyboardFocusHere(-1);
			ImGui::EndPopup();
		}
		else
		{
			RenamingDoc = NULL;
		}
		RenamingStarted = false;
	}

	// Display closing confirmation UI
	if (!CloseQueue.empty())
	{
		int close_queue_unsaved_documents = 0;
		for (int n = 0; n < CloseQueue.size(); n++)
			if (CloseQueue[n]->Dirty)
				close_queue_unsaved_documents++;

		if (close_queue_unsaved_documents == 0)
		{
			// Close documents when all are unsaved
			for (int n = 0; n < CloseQueue.size(); n++)
				CloseQueue[n]->DoForceClose();
			CloseQueue.clear();
		}
		else
		{
			if (!ImGui::IsPopupOpen("Save?"))
				ImGui::OpenPopup("Save?");
			if (ImGui::BeginPopupModal("Save?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Save change to the following items?");
				float item_height = ImGui::GetTextLineHeightWithSpacing();
				if (ImGui::BeginChild(ImGui::GetID("frame"), ImVec2(-FLT_MIN, 6.25f * item_height), ImGuiChildFlags_FrameStyle))
					for (MyDocument* doc : CloseQueue)
						if (doc->Dirty)
							ImGui::Text("%s", doc->Name);
				ImGui::EndChild();

				ImVec2 button_size(ImGui::GetFontSize() * 7.0f, 0.0f);
				if (ImGui::Button("Yes", button_size))
				{
					for (MyDocument* doc : CloseQueue)
					{
						if (doc->Dirty)
							doc->DoSave();
						doc->DoForceClose();
					}
					CloseQueue.clear();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("No", button_size))
				{
					for (MyDocument* doc : CloseQueue)
						doc->DoForceClose();
					CloseQueue.clear();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", button_size))
				{
					CloseQueue.clear();
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
	}

	ImGui::End();
}


void Editor::GetTabName(MyDocument* doc, char* out_buf, size_t out_buf_size)
{
	snprintf(out_buf, out_buf_size, "%s###doc%d", doc->Name, doc->UID);
}

void Editor::DisplayDocContents(MyDocument* doc)
{
	ImGui::PushID(doc);
	ImGui::Text("Document \"%s\"", doc->Name);
	ImGui::PushStyleColor(ImGuiCol_Text, doc->Color);
	ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.");
	ImGui::PopStyleColor();

	ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_R, ImGuiInputFlags_Tooltip);
	if (ImGui::Button("Rename.."))
	{
		RenamingDoc = doc;
		RenamingStarted = true;
	}
	ImGui::SameLine();

	ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_M, ImGuiInputFlags_Tooltip);
	if (ImGui::Button("Modify"))
		doc->Dirty = true;

	ImGui::SameLine();
	ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_Tooltip);
	if (ImGui::Button("Save"))
		doc->DoSave();

	ImGui::SameLine();
	ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_W, ImGuiInputFlags_Tooltip);
	if (ImGui::Button("Close"))
		CloseQueue.push_back(doc);
	ImGui::ColorEdit3("color", &doc->Color.x);  // Useful to test drag and drop and hold-dragged-to-open-tab behavior.
	ImGui::PopID();
}

void Editor::DisplayDocContextMenu(MyDocument* doc)
{
	if (!ImGui::BeginPopupContextItem())
		return;

	if (ImGui::MenuItem(std::format("Save {}", doc->Name).c_str(), "Ctrl+S", false, doc->Open))
		doc->DoSave();
	if (ImGui::MenuItem("Rename...", "Ctrl+R", false, doc->Open))
		RenamingDoc = doc;
	if (ImGui::MenuItem("Close", "Ctrl+W", false, doc->Open))
		CloseQueue.push_back(doc);
	ImGui::EndPopup();
}

// [Optional] Notify the system of Tabs/Windows closure that happened outside the regular tab interface.
// If a tab has been closed programmatically (aka closed from another source such as the Checkbox() in the demo,
// as opposed to clicking on the regular tab closing button) and stops being submitted, it will take a frame for
// the tab bar to notice its absence. During this frame there will be a gap in the tab bar, and if the tab that has
// disappeared was the selected one, the tab bar will report no selected tab during the frame. This will effectively
// give the impression of a flicker for one frame.
// We call SetTabItemClosed() to manually notify the Tab Bar or Docking system of removed tabs to avoid this glitch.
// Note that this completely optional, and only affect tab bars with the ImGuiTabBarFlags_Reorderable flag.
void Editor::NotifyOfDocumentsClosedElsewhere()
{
	for (MyDocument& doc : Documents)
	{
		if (!doc.Open && doc.OpenPrev)
			ImGui::SetTabItemClosed(doc.Name);
		doc.OpenPrev = doc.Open;
	}
}
