#include "EditorModel.h"
#include "App.h"

#include <numeric>
#include <fstream>


Document::idt Document::NextId = 1;

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


EditorModel::EditorModel(const fs::path& workdir, const consumer<const Document&>& onFocus)
	: m_OnFileFocus(onFocus)
{
}

bool EditorModel::Close(std::vector<u32> inds, bool save)
{
	if (save && m_Locked) return false;

	// Update recently closed files
	for (size_t ind : inds | stdv::take(RecentCloseSize))
	{
		fs::path& dir = m_Documents[ind].Path;
		auto itr = stdr::find(m_RecClose, dir);
		if (itr != m_RecClose.end())
		{
			m_RecClose.erase(itr);
			m_RecClose.insert(m_RecClose.cbegin(), dir);
		}
		else
		{
			m_RecClose.insert(m_RecClose.cbegin(), dir); // todo: push_back and render in reverse
		}
	}
	if (m_RecClose.size() > RecentCloseSize)
		m_RecClose.resize(RecentCloseSize);

	// Erase files from the document list
	std::sort(inds.begin(), inds.end());
	bool closefocus = stdr::binary_search(inds, to<u32>(m_FocusInd));

	// [1, 2, 3, 4, 5, 6, 7, 8, 9]
	// [1, 2, x, x, x, 6, x, 8, 9]
	// [1, 2, 6, 9, 8, 6, 9, 8, 9]
	for (size_t offset = 0; offset < inds.size(); offset++)
	{
		Document& dst = m_Documents[*(inds.crbegin() + offset)];
		const Document& src = *(m_Documents.crbegin() + offset);

		if (save) PerformSave(dst);
		dst = src;
	}
	m_Documents.erase(m_Documents.cend() - inds.size(), m_Documents.cend());

	if (closefocus)
	{
		// TODO: move focus to the previous focused element (keep navigation history)
		m_FocusInd = m_Documents.empty() ? InvalidIndex : 0;
		dbg << "EditorView::PerformClose m_WantFocus = " << m_FocusInd << '\n';
		m_OnFileFocus(m_Documents[m_FocusInd]);
	}

	return true;
}

void EditorModel::OpenOrFocus(const fs::path& dir)
{
	auto itd = stdr::find(m_Documents, dir, &Document::Path);
	if (itd != m_Documents.end())
	{
		auto itr = stdr::find(m_RecOpen, dir);
		m_RecOpen.erase(itr);
		m_RecOpen.insert(m_RecOpen.cbegin(), dir);
		m_OnFileFocus(*itd);
		return;
	}

	Document newdoc = { dir };
	m_Documents.push_back(newdoc);

	auto itr = stdr::find(m_RecOpen, dir);
	if (itr != m_RecOpen.end())
	{
		m_RecOpen.erase(itr);
		m_RecOpen.insert(m_RecOpen.cbegin(), dir);
	}
	else
	{
		m_RecOpen.insert(m_RecOpen.cbegin(), dir);
		if (m_RecOpen.size() > RecentOpenSize)
			m_RecOpen.resize(RecentOpenSize);
	}

	m_OnFileFocus(*m_Documents.crbegin());
}


void EditorModel::PerformRename(Document& doc, const std::string& name)
{
	if (m_Locked) return;

	doc.Name = name;
	auto newpath = doc.Path.parent_path() / doc.Name;

	std::filesystem::rename(doc.Path, newpath);
	App::GetHistory().UpdatePath(doc.Path, newpath); // TODO: this should be in FileModel

	auto it = stdr::find(m_RecOpen, doc.Path);
	if (it != m_RecOpen.end())
		*it = newpath;
	it = stdr::find(m_RecClose, doc.Path);
	if (it != m_RecClose.end())
		*it = newpath;
	doc.Path = newpath;
}
bool EditorModel::ChangeFile(const idt id)
{
	auto it = stdr::find(m_Documents, id, &Document::Id);
	if (it == m_Documents.end())
		return false;
	m_FocusInd = to<i32>(std::distance(m_Documents.begin(), it));
	m_OnFileFocus(*it);
	return true;
}
void EditorModel::FileChanged(const Document& doc)
{
	if (m_OnFileChanged)
		m_OnFileChanged(doc.Path);
}
void EditorModel::MoveCursor(Document* doc, const i32 pos)
{
	doc->CursorPos = pos;
}
void EditorModel::Edited(Document* doc, const char change)
{
	doc->Dirty = true;
}
void EditorModel::PerformSave(Document& doc) const
{
	if (m_Locked) return;

	dbg << "EditorView::PerformSave doc = " << doc << '\n';
	std::ofstream out(doc.Path);
	out << doc.Content;
	out.close();
	doc.Dirty = false;

	auto a = m_Documents | stdv::all;
}