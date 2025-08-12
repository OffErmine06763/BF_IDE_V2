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
	return out << "[Id = " << doc.Id << ", Name = " << doc.Name << ", Dirty = " << doc.Dirty << ']';
}

// TODO: warning for nested loops like [[...]] and <>><, it just makes no sense

bool EditorModel::Close(std::vector<u32> ids, bool save)
{
	if (save && m_Locked) return false;

	LOG("Closing files: " << ids << '\n');

	auto indsv = stdv::iota(0u, to<u32>(m_Documents.size())) | stdv::filter([&](u32 ind) { return stdr::find(ids, m_Documents[ind].Id) != ids.end(); });
	std::vector<u32> inds = { indsv.begin(), indsv.end() };

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
			m_RecClose.insert(m_RecClose.cbegin(), dir); // TODO: push_back and render in reverse
		}
	}
	if (m_RecClose.size() > RecentCloseSize)
		m_RecClose.resize(RecentCloseSize);

	// Erase files from the document list
	std::sort(inds.begin(), inds.end());
	bool closefocus = m_FocusInd == -1 ? false : stdr::binary_search(inds, to<u32>(m_FocusInd));

	// [1, 2, 3, 4, 5, 6, 7, 8, 9]
	// [1, 2, x, x, x, 6, x, 8, 9]
	// [1, 2, 6, 9, 8, 6, 9, 8, 9]
	// MAYBE: scan the docs list from the first removed up to the first non removed one (inds is sorted)
	// then replace the first removed index with that, move both forward by one and repeat.
	// Each iteration ignores the inds list as the elements that shouldn't be removed have been moved and have to be replaced
	// The current approach takes |inds| steps, but changes the order,
	// the new one takes |docs| - inds[0] steps, but maintains the order
	// [1, 2, 3, 4, 5, 6, 7, 8, 9]
	// [1, 2, 3, x, x, 6, x, 8, 9]
	// [1, 2, 3, 6, x, x, x, 8, 9]
	// [1, 2, 3, 6, 8, 9, x, x, x]
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
		// TODO: move focus to the previous focused element (keep navigation history, 
		// note: the relative order in the docs list might change, must store the ID)
		m_FocusInd = m_Documents.empty() ? InvalidIndex : 0;
		dbg << "EditorModel::Close new focus index = " << m_FocusInd << '\n';
		if (m_FocusInd != InvalidIndex) // TODO: must notify of lost focus
			m_FocusEvent.Notify(m_Documents[m_FocusInd]);
	}

	m_CloseEvent.Notify();
	return true;
}
void EditorModel::OpenOrFocus(const fs::path& dir)
{
	auto itd = stdr::find(m_Documents, dir, &Document::Path);
	// if the file is already open => focus
	if (itd != m_Documents.end())
	{
		LOG("Focusing file " << dir << '\n');
		auto itr = stdr::find(m_RecOpen, dir);
		m_RecOpen.erase(itr);
		m_RecOpen.insert(m_RecOpen.cbegin(), dir);
		m_FocusEvent.Notify(m_Documents[m_FocusInd]);
		return;
	}

	LOG("Opening file " << dir << '\n');
	Document newdoc = { dir };
	m_Documents.push_back(newdoc);
	m_FocusInd = to<u32>(m_Documents.size()) - 1;

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

	m_FocusEvent.Notify(m_Documents[m_FocusInd]);
}


void EditorModel::PerformSave(Document& doc) const
{
	if (m_Locked) return;

	LOG("Saving file " << doc << '\n');
	std::ofstream out(doc.Path);
	out << doc.Content;
	out.close();
	doc.Dirty = false;
}
void EditorModel::PerformRename(const idt id, const std::string& name)
{
	if (m_Locked) return;

	auto find = stdr::find(m_Documents, id, &Document::Id);
	if (find == m_Documents.end())
		return;

	Document& doc = *find;
	LOG("Renaming file " << doc << " to " << name << '\n');
	doc.Name = name;
	auto newpath = doc.Path.parent_path() / doc.Name;

	std::filesystem::rename(doc.Path, newpath);
	App::GetHistory().UpdatePath(doc.Path, newpath); // TODO: this should be in EditModel

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
	{
		LOG("Requested focus id " << id << " missing\n");
		return false;
	}

	LOG("Changing focus to id " << id << '\n');
	m_FocusInd = to<i32>(std::distance(m_Documents.begin(), it));
	
	m_FocusEvent.Notify(m_Documents[m_FocusInd]);
	return true;
}

void EditorModel::MoveCursor(Document* doc, const i32 pos)
{
	doc->CursorPos = pos;
}

void EditorModel::Edited(Document* doc, const char change)
{
	doc->Dirty = true;
}

void EditorModel::OnPathDeleted(const fs::path& path)
{
	if (fs::is_directory(path))
		return;

	auto res = stdr::find(m_Documents, path, &Document::Path);
	if (res != m_Documents.end())
		m_Documents.erase(res);
	// No need to update recent closed and open, since the file doesn't exist anymore
	// might want to call the callback i guess
}



//listener_id EditorModel::Subscribe(Prop prop, callable& cb)
//{
//	m_Observers[prop][m_NextId] = cb;
//	return m_NextId++;
//}
//bool EditorModel::Unsubscribe(Prop prop, listener_id id)
//{
//	return m_Observers[prop].erase(id) == 1;
//}
//u64 EditorModel::Unsubscribe(listener_id id)
//{
//	u64 res = 0;
//	for (const auto& [prop, _] : m_Observers)
//		if (Unsubscribe(prop, id))
//			res++;
//	return res;
//}