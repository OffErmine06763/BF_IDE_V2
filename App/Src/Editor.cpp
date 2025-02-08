#include "Editor.h"
#include "App.h"

#include <numeric>

#define to static_cast


i32 Document::NextId = 0;


Editor::Editor(const fs::path& workdir)
	: m_WorkDir(workdir)
{}



i32 Editor::OpenFile(const fs::path& dir)
{
	Document newdoc = { dir };
	m_Documents.push_back(newdoc);

	auto it = stdr::find(m_Recent, dir);
	if (it == m_Recent.end())
	{
		m_Recent.insert(m_Recent.begin(), dir);
		m_Recent.resize(std::min(m_Recent.size(), 10ull));
	}
	else if (it != m_Recent.begin())
	{
		m_Recent.erase(it);
		m_Recent.insert(m_Recent.begin(), dir);
	}
	return newdoc.Id;
}
i32 Editor::OpenOrFocus(const fs::path& path)
{
	auto it = stdr::find(m_Documents, path, &Document::Path);
	if (it == m_Documents.end())
		return OpenFile(path);

	Focus(it->Id);
	return it->Id;
}

bool Editor::Focus(const i32 id)
{
	auto it = stdr::find(m_Documents, id, &Document::Id);
	if (it == m_Documents.end())
		return false;
	_Focus(to<i32>(std::distance(m_Documents.begin(), it)));
	return true;
}
void Editor::_Focus(const i32 ind)
{
	// TODO:
}

bool Editor::CloseFile(const fs::path& dir)
{
	auto it = stdr::find_if(m_Documents, [&dir](const Document& doc) { return doc.Path.compare(dir) == 0; });
	if (it == m_Documents.end())
		return false;
	if (it->Dirty)
		m_CloseQueue.push_back(to<i32>(std::distance(m_Documents.begin(), it)));
	else
		m_Documents.erase(it);
	return true;
}
bool Editor::CloseFile(const i32 id)
{
	auto it = stdr::find_if(m_Documents, [id](const Document& doc) { return doc.Id == id; });
	//auto it = stdr::lower_bound(m_Documents, id, stdr::less{}, &Document::Id);
	if (it == m_Documents.end())
		return false;
	if (it->Dirty)
		m_CloseQueue.push_back(to<i32>(std::distance(m_Documents.begin(), it)));
	else
		m_Documents.erase(it);
	return true;
}
void Editor::CloseAll()
{
	m_CloseQueue.resize(m_Documents.size());
	std::iota(m_CloseQueue.begin(), m_CloseQueue.end(), 0);
	//for (size_t n = 0; n < m_Documents.size(); n++)
	//	if (m_Documents[n].Dirty)
	//		m_CloseQueue.push_back(n);
	if (m_CloseQueue.empty())
		m_Documents.clear();
	/*else
		m_CloseAllOnConfirmClose = true;*/
}


void Editor::SaveDoc(Document& doc)
{
	// TODO:
	doc.Dirty = false;
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
			if (ImGui::BeginMenu("Open", !m_Recent.empty()))
			{
				for (fs::path& rec : m_Recent)
					if (ImGui::MenuItem(rec.filename().string().c_str()))
						OpenOrFocus(rec);
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Close All", nullptr, false, !m_Documents.empty()))
			{
				m_CloseQueue.resize(m_Documents.size());
				std::iota(m_CloseQueue.begin(), m_CloseQueue.end(), 0);
			}
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

	// Create a DockSpace node where any window can be docked
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dockspace_id, { 0, 0 }, ImGuiDockNodeFlags_None);

	// Create Windows
	for (int doc_n = 0; doc_n < m_Documents.size(); doc_n++)
	{
		Document& doc = m_Documents[doc_n];

		ImGui::SetNextWindowDockID(dockspace_id, m_WantRedock ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
		ImGuiWindowFlags window_flags = (doc.Dirty ? ImGuiWindowFlags_UnsavedDocument : 0);
		bool notwantclose = true;
		bool visible = ImGui::Begin(doc.Name.c_str(), &notwantclose, window_flags);

		// Cancel attempt to close when unsaved add to save queue so we can display a popup.
		if (!notwantclose && doc.Dirty)
			m_CloseQueue.push_back(doc_n);

		DisplayDocContextMenu(doc_n);
		if (visible)
			DisplayDocContents(doc_n);

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
	if (m_RenamingDoc != nullptr)
	{
		static char name[256] = "";
		if (m_RenamingStarted)
		{
			memcpy(name, m_RenamingDoc->Name.c_str(), 256);
			ImGui::OpenPopup("Rename");
		}
		if (ImGui::BeginPopup("Rename"))
		{
			ImGui::SetNextItemWidth(ImGui::GetFontSize() * 30);
			if (ImGui::InputText("###Name", name, 256, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				ImGui::CloseCurrentPopup();
				
				m_RenamingDoc->Name = name;
				auto newpath = m_RenamingDoc->Path.parent_path() / m_RenamingDoc->Name;

				std::filesystem::rename(m_RenamingDoc->Path, newpath);
				App::GetHistory().UpdatePath(m_RenamingDoc->Path, newpath);
				m_RenamingDoc->Path = newpath;

				m_RenamingDoc = nullptr;
				//memset(name, 0, 256);
			}
			if (m_RenamingStarted)
				ImGui::SetKeyboardFocusHere(-1);
			ImGui::EndPopup();
		}
		else
			m_RenamingDoc = nullptr;
		m_RenamingStarted = false;
	}

	// Display closing confirmation UI
	if (!m_CloseQueue.empty())
	{
		size_t close_queue_unsaved_documents = stdr::count_if(m_CloseQueue, &Document::Dirty, [this](const i32 n) -> const Document& { return m_Documents[n]; });

		if (close_queue_unsaved_documents == 0)
		{
			// [1, 2, 3, 4, 5, 6, 7, 8, 9]
			// [1, 2, x, x, x, 6, x, 8, 9]
			// [1, 2, 6, 9, 8, 6, 9, 8, 9]
			for (size_t offset = 0; offset < m_CloseQueue.size(); offset++)
				m_Documents[*(m_CloseQueue.crbegin() + offset)] = *(m_Documents.crbegin() + offset);
			m_Documents.erase(m_Documents.cbegin() + m_Documents.size() - m_CloseQueue.size(), m_Documents.cend());
			m_CloseQueue.clear();
		}
		else
		{
			if (!ImGui::IsPopupOpen("Save?"))
				ImGui::OpenPopup("Save?");
			if (ImGui::BeginPopupModal("Save?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Save change to the following items?");
				float item_height = ImGui::GetTextLineHeightWithSpacing();
				if (ImGui::BeginChild(ImGui::GetID("frame"), ImVec2(-FLT_MIN, 6.25f * item_height), ImGuiChildFlags_FrameStyle))
					for (i32 doc : m_CloseQueue)
						if (m_Documents[doc].Dirty)
							ImGui::Text("%s", m_Documents[doc].Name.c_str());
				ImGui::EndChild();

				ImVec2 button_size(ImGui::GetFontSize() * 7.0f, 0.0f);
				if (ImGui::Button("Yes", button_size))
				{
					for (int offset = 0; offset < m_CloseQueue.size(); offset++)
					{
						SaveDoc(m_Documents[*(m_CloseQueue.rbegin() + offset)]);
						m_Documents[*(m_CloseQueue.crbegin() + offset)] = *(m_Documents.crbegin() + offset);
					}
					m_Documents.erase(m_Documents.cbegin() + m_Documents.size() - m_CloseQueue.size(), m_Documents.cend());
					m_CloseQueue.clear();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("No", button_size))
				{
					for (int offset = 0; offset < m_CloseQueue.size(); offset++)
						m_Documents[*(m_CloseQueue.crbegin() + offset)] = *(m_Documents.crbegin() + offset);
					m_Documents.erase(m_Documents.cbegin() + m_Documents.size() - m_CloseQueue.size(), m_Documents.cend());
					m_CloseQueue.clear();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", button_size))
				{
					m_CloseQueue.clear();
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
	}

	ImGui::End();
}


void Editor::DisplayDocContents(const i32 n)
{
	Document& doc = m_Documents[n];

	ImGui::PushID(doc.Id);
	
	ImGui::Text("Document \"%s\"", doc.Name.c_str());
	ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.");

	ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_R, ImGuiInputFlags_Tooltip);
	if (ImGui::Button("Rename.."))
	{
		m_RenamingDoc = &doc;
		m_RenamingStarted = true;
	}
	ImGui::SameLine();

	ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_M, ImGuiInputFlags_Tooltip);
	if (ImGui::Button("Modify"))
		doc.Dirty = true;

	ImGui::SameLine();
	ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_Tooltip);
	if (ImGui::Button("Save"))
		SaveDoc(doc);

	ImGui::SameLine();
	ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_W, ImGuiInputFlags_Tooltip);
	if (ImGui::Button("Close"))
		m_CloseQueue.push_back(n);

	ImGui::PopID();
}

void Editor::DisplayDocContextMenu(const i32 n)
{
	if (!ImGui::BeginPopupContextItem())
		return;

	Document& doc = m_Documents[n];
	if (ImGui::MenuItem(std::format("Save {}", doc.Name).c_str(), "Ctrl+S", false, doc.Dirty))
		SaveDoc(doc);
	if (ImGui::MenuItem("Rename...", "Ctrl+R", false))
		m_RenamingDoc = &doc;
	if (ImGui::MenuItem("Close", "Ctrl+W", false))
		m_CloseQueue.push_back(n);
	ImGui::EndPopup();
}
