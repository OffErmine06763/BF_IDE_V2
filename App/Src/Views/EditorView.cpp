#include "EditorView.h"

#include "App.h"
#include "MyImGuiWidgets.h"
#include "Shortcuts.h"

#include <numeric>
#include <fstream>


static int DocumentEditorCallback(ImGuiInputTextCallbackData* data);




EditorView::EditorView(EditorModel* model)
	: m_VM(this, model), m_Documents(m_VM.GetDocuments()), m_RecClose(m_VM.GetRecentClose()), m_RecOpen(m_VM.GetRecentOpen())
{
	m_VM.SubscribeFocus([this](const Document& doc) { Focused(doc); });
	auto focus = m_VM.GetFocusedFile();
	if (focus)
		m_FocusInd = std::distance(m_Documents.begin(), stdr::find(m_Documents, focus->Id, &Document::Id));
}

void EditorView::CloseAll()
{
	m_CloseQueue.resize(m_Documents.size());
	std::iota(m_CloseQueue.begin(), m_CloseQueue.end(), 0);
}
void EditorView::CloseFile(const u32 ind)
{
	m_CloseQueue.push_back(ind);
}
void EditorView::PerformedClose()
{
	m_CloseQueue.clear();
}

void EditorView::StartRename(const u32 ind)
{
	m_RenamingDocInd = ind;
	m_RenamingStarted = true;
}

void EditorView::Focused(const Document& doc)
{
	m_WantFocus = doc.Id;
}


void EditorView::Render(/* const ImVec2& pos, const ImVec2& size // TAG: Toolbar */)
{
	ProcessShortcuts();
	RenderMainMenu();
	RenderBody(/* pos, size // TAG: Toolbar */);
}
void EditorView::ProcessShortcuts()
{
	if (ImGui::IsKeyChordPressed(ES_CloseAll.Chord))
		m_VM.OnCloseAll();
	if (m_FocusInd >= 0 && m_FocusInd < m_Documents.size())
	{
		Document& doc = m_Documents[m_FocusInd];
		if (ImGui::IsKeyChordPressed(ES_Rename.Chord) && !m_Locked)
			StartRename(m_FocusInd);
		if (ImGui::IsKeyChordPressed(ES_Save.Chord) && !m_Locked)
			m_VM.OnPerformSave(doc);
		if (ImGui::IsKeyChordPressed(ES_CloseOne.Chord))
			m_VM.OnCloseFile(doc.Id);
	}
}
void EditorView::RenderMainMenu()
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
			if (ImGui::BeginMenu("Recent Open", !m_RecOpen.empty()))
			{
				std::vector<fs::path> toopen; // can't open files inside loop because it would update the recent files list
				for (const fs::path& rec : m_RecOpen)
					if (ImGui::MenuItem((rec.filename().string() + "##open").c_str()))
						toopen.push_back(rec);
				for (auto& p : toopen)
					m_VM.OnOpenOrFocus(p);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Recent Close", !m_RecClose.empty()))
			{
				for (const fs::path& rec : m_RecClose)
					if (ImGui::MenuItem((rec.filename().string() + "##close").c_str()))
						m_VM.OnOpenOrFocus(rec);
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Close All", ES_CloseAll.Label, false, !m_Documents.empty()))
				m_VM.OnCloseAll();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z", false, false)) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
void EditorView::RenderBody(/* const ImVec2& pos, const ImVec2& size // TAG: Toolbar  */)
{
	static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

	// TAG: Toolbar
	// ImGui::SetNextWindowPos(pos);
	// ImGui::SetNextWindowSize(size);

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);

	bool window_contents_visible = ImGui::Begin("Documents", nullptr, flags);

	ImGuiID dockspace_id = ImGui::GetID("EditorDockspace");
	ImGui::DockSpace(dockspace_id, { 0, 0 }, ImGuiDockNodeFlags_None);

	for (u32 n = 0; n < m_Documents.size(); n++)
	{
		const Document& doc = m_Documents[n];

		ImGui::SetNextWindowDockID(dockspace_id, m_WantRedock ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
		ImGuiWindowFlags window_flags = (doc.Dirty ? ImGuiWindowFlags_UnsavedDocument : 0) | ImGuiWindowFlags_NoBringToFrontOnFocus;

		if (m_WantFocus == doc.Id)
		{
			ImGui::SetNextWindowFocus();
			m_FocusInd = n;
			m_WantFocus = InvalidID;
		}

		bool notwantclose = true;
		bool visible = ImGui::Begin(doc.Name.c_str(), &notwantclose, window_flags);

		if (!notwantclose)
			m_VM.OnWantCloseFile(n);
		if (ImGui::IsItemClicked() && n != m_FocusInd)
			m_VM.OnWantFileChange(doc);

		DisplayDocContextMenu(n);
		if (visible)
			DisplayDocContents(n);

		ImGui::End();
	}
	m_WantRedock = false;

	if (!window_contents_visible)
	{
		ImGui::End();
		return;
	}

	if (m_RenamingDocInd >= 0)
		RenderRenamingDocUI();
	if (!m_CloseQueue.empty())
		RenderClosingConfirmationUI();

	ImGui::End();
}
void EditorView::RenderClosingConfirmationUI()
{
	size_t unsaved = stdr::count_if(m_CloseQueue, &Document::Dirty, [this](const u32 n) -> const Document& { return m_Documents[n]; });
	if (unsaved == 0)
	{
		m_VM.OnFileClosed(m_CloseQueue, false);
		return;
	}

	if (!ImGui::IsPopupOpen("Save?"))
		ImGui::OpenPopup("Save?");
	if (ImGui::BeginPopupModal("Save?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Save change to the following items?");
		float item_height = ImGui::GetTextLineHeightWithSpacing();
		if (ImGui::BeginChild(ImGui::GetID("frame"), ImVec2(-FLT_MIN, 6.25f * item_height), ImGuiChildFlags_FrameStyle))
			for (u32 doc : m_CloseQueue)
				if (m_Documents[doc].Dirty)
					ImGui::Text("%s", m_Documents[doc].Name.c_str());
		ImGui::EndChild();

		ImVec2 button_size(ImGui::GetFontSize() * 7.0f, 0.0f);
		if (ImGui::Button("Yes", button_size))
		{
			m_VM.OnFileClosed(m_CloseQueue, true);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("No", button_size))
		{
			m_VM.OnFileClosed(m_CloseQueue, false);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", button_size))
		{
			m_VM.OnCancelClose();
			// m_CloseQueue.clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
void EditorView::RenderRenamingDocUI()
{
	static char name[256] = "";
	const Document& doc = m_Documents[m_RenamingDocInd];
	if (m_RenamingStarted)
	{
		memcpy(name, doc.Name.c_str(), 256);
		ImGui::OpenPopup("Rename");
	}
	if (ImGui::BeginPopup("Rename"))
	{
		ImGui::SetNextItemWidth(ImGui::GetFontSize() * 30);
		if (ImGui::InputText("###Name", name, 256, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			ImGui::CloseCurrentPopup();
			m_VM.OnPerformRename(doc.Id, name);
			m_RenamingDocInd = InvalidIndex;
		}
		if (m_RenamingStarted)
			ImGui::SetKeyboardFocusHere(-1);
		ImGui::EndPopup();
	}
	else
		m_RenamingDocInd = InvalidIndex;
	m_RenamingStarted = false;
}



void EditorView::DisplayDocContents(const u32 n)
{
	Document& doc = m_Documents[n];

	ImGui::PushID(doc.Id);

	static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackResize
		// | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory // can't combine with multiline & AllowTabInput
		| ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackAlways;
	if (false) // TODO: readonly during compile/debug
		flags |= ImGuiInputTextFlags_ReadOnly;

	EditorCallbackUserData ud{ .Doc = &doc, .VM = &m_VM };
	ImGui::InputTextMultiline("##source",
		(char*)doc.Content.c_str(), doc.Content.capacity() + 1,
		ImVec2(-FLT_MIN, -FLT_MIN), flags,
		DocumentEditorCallback, &ud);

	ImGui::PopID();
}
void EditorView::DisplayDocContextMenu(const u32 n)
{
	if (!ImGui::BeginPopupContextItem())
		return;

	Document& doc = m_Documents[n];
	if (ImGui::MenuItem(std::format("Save {}", doc.Name).c_str(), ES_Save.Label, false, doc.Dirty && !m_Locked))
		m_VM.OnPerformSave(doc);
	if (ImGui::MenuItem("Rename...", ES_Rename.Label, false, !m_Locked))
		StartRename(n);
	if (ImGui::MenuItem("Close", ES_CloseOne.Label))
		m_VM.OnCloseFile(doc.Id);
	ImGui::EndPopup();
}




//void EditorView::Lock(bool lock)
//{
//	m_Locked = lock;
//}



int DocumentEditorCallback(ImGuiInputTextCallbackData* data)
{
	EditorView::EditorCallbackUserData* user_data = (EditorView::EditorCallbackUserData*)data->UserData;
	Document* doc = user_data->Doc;
	EditorViewModel* VM = user_data->VM;

	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		// Resize string callback
		// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
		std::string* str = &doc->Content;
		str->resize(data->BufTextLen);
		data->Buf = (char*)str->c_str();
		LOG_GRAPHICS("DocumentEditorCallback Resize\n");
	}
	else if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter)
	{
		if (!IsValidBF(to<char>(data->EventChar)) && data->EventChar != '\n')
			return 1;
		if (data->EventChar == BF_OPN)
		{
			// TODO: chiudi parentesi, già provati:
			// doc->Content.insert()
			// data->InsertChars() <- works in CallbackAlways
		}
	}
	else if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
	{
		// Callback on any edit. Note that InputText() already returns true on edit + you can always use IsItemEdited(). The callback is useful to manipulate the underlying buffer while focus is active.
		LOG_GRAPHICS("DocumentEditorCallback Edit\n");
		VM->OnEdit(doc, data->EventChar);
	}
	else if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
	{
		// Callback on each iteration. User code may query cursor position, modify text buffer.
		if (doc->CursorPos != data->CursorPos)
			VM->OnCursorMoved(doc, data->CursorPos);

		static u64 count = 0;
		count++;
		if (count == 144 * 5)
		{
			count = 0;
			// data->InsertChars(0, "]");
			LOG_GRAPHICS("DocumentEditorCallback Always\n");
		}
	}

	return 0;
}
