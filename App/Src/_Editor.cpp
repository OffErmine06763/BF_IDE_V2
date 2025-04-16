#include "Editor.h"
#include "App.h"
#include "States/WorkingState.h"
#include "MyImGuiWidgets.h"
#include "Shortcuts.h"

#include <numeric>
#include <fstream>

Document::idt Document::NextId = 0;

Document::Document(const fs::path& path)
	: Id(NextId++), Path(path), Name(path.filename().string())
{
	std::ifstream in(path);
	Content = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
	in.close();
}


std::ostream& operator<<(std::ostream& out, const Document& doc)
{
	return out << "[Id = " << doc.Id << ", Path = " << doc.Path << ", Dirty = " << doc.Dirty << ']';
}


static int DocumentEditorCallback(ImGuiInputTextCallbackData* data);


Editor::idt Editor::OpenFile(const fs::path& dir)
{
	auto itd = stdr::find(m_Documents, dir, &Document::Path);
	if (itd != m_Documents.end())
	{
		auto itr = stdr::find(m_RecOpen, dir);
		m_RecOpen.erase(itr);
		m_RecOpen.insert(m_RecOpen.cbegin(), dir); 
		return itd->Id;
	}

	Document newdoc = { dir };
	m_Documents.push_back(newdoc);
	if (m_FocusInd < 0 && m_WantFocus < 0)
		m_WantFocus = to<i32>(m_Documents.size() - 1);

	auto itr = stdr::find(m_RecOpen, dir);
	if (itr != m_RecOpen.end()) 
	{
		m_RecOpen.erase(itr);
		m_RecOpen.insert(m_RecOpen.cbegin(), dir);
	}
	else 
	{
		m_RecOpen.insert(m_RecOpen.cbegin(), dir); // todo: push_back and render in reverse
		if (m_RecOpen.size() > RecentOpenSize)
			m_RecOpen.resize(RecentOpenSize);
	}
	return newdoc.Id;
}
Editor::idt Editor::OpenOrFocus(const fs::path& path)
{
	auto id = OpenFile(path);
	Focus(id);
	return id;
}

bool Editor::Focus(const idt id)
{
	auto it = stdr::find(m_Documents, id, &Document::Id);
	if (it == m_Documents.end())
		return false;
	_Focus(to<i32>(std::distance(m_Documents.begin(), it)));
	return true;
}
void Editor::_Focus(const u32 ind)
{
	m_WantFocus = ind;
}

bool Editor::CloseFile(const fs::path& dir)
{
	auto it = stdr::find_if(m_Documents, [&dir](const Document& doc) { return doc.Path.compare(dir) == 0; });
	if (it == m_Documents.end())
		return false;
	_CloseFile(to<u32>(std::distance(m_Documents.begin(), it)));
	return true;
}
bool Editor::CloseFile(const idt id)
{
	auto it = stdr::find_if(m_Documents, [id](const Document& doc) { return doc.Id == id; });
	//auto it = stdr::lower_bound(m_Documents, id, stdr::less{}, &Document::Id);
	if (it == m_Documents.end())
		return false;
	_CloseFile(to<u32>(std::distance(m_Documents.begin(), it)));
	return true;
}
void Editor::_CloseFile(const u32 ind)
{
	/*auto it = stdr::lower_bound(m_CloseQueue, ind);
	if (it == m_CloseQueue.end())
		m_CloseQueue.push_back(ind);
	else
		m_CloseQueue.insert(it, ind);*/
	m_CloseQueue.push_back(ind);
}
void Editor::CloseAll()
{
	m_CloseQueue.resize(m_Documents.size());
	std::iota(m_CloseQueue.begin(), m_CloseQueue.end(), 0);
}

void Editor::WantRename(const u32 ind)
{
	m_RenamingDocInd = ind;
	m_RenamingStarted = true;
}



void Editor::PerformSave(Document& doc) const
{
	if (m_Locked) return;

	dbg << "Editor::PerformSave doc = " << doc << '\n';
	std::ofstream out(doc.Path);
	out << doc.Content;
	out.close();
	doc.Dirty = false;
}
void Editor::PerformClose(bool save)
{
	if (save && m_Locked) return;

	// Update recently closed files
	size_t offset = 0;
	for (size_t ind : stdv::reverse(m_CloseQueue) | stdv::take(RecentCloseSize))
	{
		fs::path& dir = m_Documents[ind].Path;
		auto itr = stdr::find(m_RecClose, dir);
		if (itr != m_RecClose.end())
		{
			m_RecClose.erase(itr);
			m_RecClose.insert(m_RecClose.cbegin() + offset, dir);
		}
		else
		{
			m_RecClose.insert(m_RecClose.cbegin() + offset, dir); // todo: push_back and render in reverse
		}
	}
	if (m_RecClose.size() > RecentCloseSize)
		m_RecClose.resize(RecentCloseSize);
	
	// Erase files from the document list
	std::sort(m_CloseQueue.begin(), m_CloseQueue.end());
	bool closefocus = stdr::binary_search(m_CloseQueue, to<u32>(m_FocusInd));

	// [1, 2, 3, 4, 5, 6, 7, 8, 9]
	// [1, 2, x, x, x, 6, x, 8, 9]
	// [1, 2, 6, 9, 8, 6, 9, 8, 9]
	for (offset = 0; offset < m_CloseQueue.size(); offset++)
	{
		Document& dst = m_Documents[*(m_CloseQueue.crbegin() + offset)];
		const Document& src = *(m_Documents.crbegin() + offset);

		if (save) PerformSave(dst);
		dst = src;
	}
	m_Documents.erase(m_Documents.cend() - m_CloseQueue.size(), m_Documents.cend());
	m_CloseQueue.clear();

	if (closefocus)
	{
		// TODO: move focus to the previous focused element (keep navigation history)
		m_WantFocus = m_Documents.empty() ? -1 : 0;
		dbg << "Editor::PerformClose m_WantFocus = " << m_WantFocus << '\n';
	}
}
void Editor::PerformRename(Document& doc, const std::string& name)
{
	if (m_Locked) return;

	doc.Name = name;
	auto newpath = doc.Path.parent_path() / doc.Name;

	std::filesystem::rename(doc.Path, newpath);
	App::GetHistory().UpdatePath(doc.Path, newpath);

	auto it = stdr::find(m_RecOpen, doc.Path);
	if (it != m_RecOpen.end())
		*it = newpath;
	it = stdr::find(m_RecClose, doc.Path);
	if (it != m_RecClose.end())
		*it = newpath;
	doc.Path = newpath;
}



void Editor::Render(/* const ImVec2& pos, const ImVec2& size // TAG: Toolbar */)
{
	ProcessShortcuts();
	RenderMainMenu();
	RenderBody(/* pos, size // TAG: Toolbar */);
}
void Editor::ProcessShortcuts() 
{
	if (ImGui::IsKeyChordPressed(ES_CloseAll.Chord))
		CloseAll();
	if (m_FocusInd >= 0 && m_FocusInd < m_Documents.size())
	{
		Document& doc = m_Documents[m_FocusInd];
		if (ImGui::IsKeyChordPressed(ES_Rename.Chord) && !m_Locked)
			WantRename(m_FocusInd);
		if (ImGui::IsKeyChordPressed(ES_Save.Chord) && !m_Locked)
			PerformSave(doc);
		if (ImGui::IsKeyChordPressed(ES_CloseOne.Chord))
			_CloseFile(m_FocusInd);
	}
}
void Editor::RenderMainMenu()
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
				for (fs::path& rec : m_RecOpen)
					if (ImGui::MenuItem((rec.filename().string() + "##open").c_str()))
						toopen.push_back(rec);
				for (auto& p : toopen)
					OpenOrFocus(p);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Recent Close", !m_RecClose.empty()))
			{
				for (fs::path& rec : m_RecClose)
					if (ImGui::MenuItem((rec.filename().string() + "##close").c_str()))
						OpenOrFocus(rec);
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Close All", ES_CloseAll.Label, false, !m_Documents.empty()))
				CloseAll();
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
void Editor::RenderBody(/* const ImVec2& pos, const ImVec2& size // TAG: Toolbar  */)
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
		Document& doc = m_Documents[n];

		ImGui::SetNextWindowDockID(dockspace_id, m_WantRedock ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
		ImGuiWindowFlags window_flags = (doc.Dirty ? ImGuiWindowFlags_UnsavedDocument : 0) | ImGuiWindowFlags_NoBringToFrontOnFocus;

		bool notwantclose = true;
		if (m_WantFocus == n)
		{
			ImGui::SetNextWindowFocus();
			m_FocusInd = n;
			if (m_FileChangedCB) m_FileChangedCB(doc.Path);
			m_WantFocus = InvalidIndex;
		}
		bool visible = ImGui::Begin(doc.Name.c_str(), &notwantclose, window_flags);

		if (!notwantclose)
			_CloseFile(n);
		if (ImGui::IsItemClicked() && n != m_FocusInd)
		{
			m_FocusInd = n;
			if (m_FileChangedCB) m_FileChangedCB(doc.Path);
		}

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
void Editor::RenderClosingConfirmationUI()
{
	size_t close_queue_unsaved_documents = stdr::count_if(m_CloseQueue, &Document::Dirty, [this](const u32 n) -> const Document& { return m_Documents[n]; });
	if (close_queue_unsaved_documents == 0)
	{
		PerformClose(false);
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
			PerformClose(true);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("No", button_size))
		{
			PerformClose(false);
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
void Editor::RenderRenamingDocUI()
{
	static char name[256] = "";
	Document& doc = m_Documents[m_RenamingDocInd];
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
			PerformRename(doc, name);
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



void Editor::DisplayDocContents(const u32 n)
{
	Document& doc = m_Documents[n];

	ImGui::PushID(doc.Id);
	
	static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackResize 
		// | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory // can't combine with multiline & AllowTabInput
		| ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackAlways;
	if (false) // TODO: readonly during compile/debug
		flags |= ImGuiInputTextFlags_ReadOnly;
	
	EditorCallbackUserData ud{ .Doc = &doc };
	ImGui::InputTextMultiline("##source", 
		(char*)doc.Content.c_str(), doc.Content.capacity() + 1, 
		ImVec2(-FLT_MIN, -FLT_MIN), flags, 
		DocumentEditorCallback, &ud);

	ImGui::PopID();
}
void Editor::DisplayDocContextMenu(const u32 n)
{
	if (!ImGui::BeginPopupContextItem())
		return;

	Document& doc = m_Documents[n];
	if (ImGui::MenuItem(std::format("Save {}", doc.Name).c_str(), ES_Save.Label, false, doc.Dirty && !m_Locked))
		PerformSave(doc);
	if (ImGui::MenuItem("Rename...", ES_Rename.Label, false, !m_Locked))
		WantRename(n);
	if (ImGui::MenuItem("Close", ES_CloseOne.Label))
		_CloseFile(n);
	ImGui::EndPopup();
}




void Editor::Lock(bool lock)
{
	m_Locked = lock;
}



int DocumentEditorCallback(ImGuiInputTextCallbackData* data)
{
	Editor::EditorCallbackUserData* user_data = (Editor::EditorCallbackUserData*)data->UserData;
	Document* doc = user_data->Doc;

	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		// Resize string callback
		// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
		std::string* str = &doc->Content;
		str->resize(data->BufTextLen);
		data->Buf = (char*)str->c_str();
		dbg << "DocumentEditorCallback EventFlag = Resize\n";
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
		dbg << "DocumentEditorCallback EventFlag = Edit\n";
		doc->Dirty = true;
	}
	else if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
	{
		// Callback on each iteration. User code may query cursor position, modify text buffer.
		doc->CursorPos = data->CursorPos;

		static u64 count = 0;
		count++;
		if (count == 144)
		{
			count = 0;
			// data->InsertChars(0, "]");
			dbg << "DocumentEditorCallback EventFlag = Always\n";
		}
	}

	return 0;
}