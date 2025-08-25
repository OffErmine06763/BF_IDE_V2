#include "TreeTool.h"

#include <imgui.h>
#include <imgui_internal.h>



TreeTool::TreeTool(const fs::path& root)
	: m_TreeRoot({ .Path = root, .Directory = true })
{
}


void TreeTool::RenderTreeEntry(TreeEntry& entry)
{
	static const ImGuiTreeNodeFlags dir_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	static const ImGuiTreeNodeFlags file_flags = dir_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet;

	// ::find::
	//auto to_focus = stdr::find(m_PathsToFind, entry.Path.filename().string(), &FindNode::Name);
	//if (to_focus != m_PathsToFind.end())
	//{
	//	ImGui::SetNextItemOpen(true);
	//	m_PathToFind.clear();
	//}
	if (entry.Directory)
	{
		const bool node_open = ImGui::TreeNodeEx(entry.Path.string().c_str(), dir_flags, entry.Path.filename().string().c_str());
		if (entry.Collapsed && node_open)
			m_TreeCacheCounter = 144;
		entry.Collapsed = !node_open;

		if (ImGui::BeginPopupContextItem()) // right click
		{
			if (ImGui::MenuItem("Show in File Explorer"))
				ShowInExplorer(entry.Path);
			if (ImGui::MenuItem("Compile"))
				m_CompileEvent.Notify(entry.Path);
			if (ImGui::MenuItem("Delete"))
				m_DeleteEvent.Notify(entry.Path);
			if (ImGui::MenuItem("New"))
				m_NewEvent.Notify(entry.Path);
			ImGui::EndPopup();
		}

		if (node_open)
		{
			for (TreeEntry& child : entry.Children)
				RenderTreeEntry(child);
			ImGui::TreePop();
		}
	}
	else
	{
		ImGui::TreeNodeEx(entry.Path.string().c_str(), file_flags, entry.Path.filename().string().c_str());

		if ((ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) || ImGui::IsItemActivated())
			m_SelectEvent.Notify(entry.Path);

		if (ImGui::BeginPopupContextItem()) // right click
		{
			if (ImGui::MenuItem("Show in File Explorer"))
				ShowInExplorer(entry.Path);
			if (ImGui::MenuItem("Compile"))
				m_CompileEvent.Notify(entry.Path);
			if (ImGui::MenuItem("Delete"))
				m_DeleteEvent.Notify(entry.Path);
			if (ImGui::MenuItem("New"))
				m_NewEvent.Notify(entry.Path.parent_path());
			ImGui::EndPopup();
		}
	}
}
void TreeTool::Render()
{
	// ::find::
	//static char buf[2];
	//bool enter = ImGui::InputText("Find", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue);
	//if (enter)
	//{
	//	fs::path partial = buf;
	//	FindPath(partial, m_TreeRoot.Path, true);
	//}

	RenderTreeEntry(m_TreeRoot);

	m_TreeCacheCounter++;
	if (m_TreeCacheCounter < 144)
		return;
	m_TreeCacheCounter = 0;

	CacheDirectoryTree(m_TreeRoot);
}
void TreeTool::CacheDirectoryTree(TreeEntry& parent)
{
	if (parent.Collapsed)
		return;

	std::vector<TreeEntry> newchildren;
	for (const fs::path& p : fs::directory_iterator(parent.Path, fs::directory_options::skip_permission_denied))
	{
		TreeEntry entry;
		if (parent.Map.contains(p))
		{
			TreeEntry& old = parent.Children[parent.Map[p]];
			if (old.Directory == fs::is_directory(p))
				entry = std::move(old);
			else
				entry = { .Path = p, .Directory = !old.Directory };
		}
		else
			entry = { .Path = p, .Directory = fs::is_directory(p) };

		if (entry.Directory && !entry.Collapsed)
			CacheDirectoryTree(entry);
		newchildren.push_back(std::move(entry));
	}
	parent.Children = std::move(newchildren);
	parent.Map.clear();
	for (size_t i = 0; i < parent.Children.size(); i++)
		parent.Map.insert({ parent.Children[i].Path, i });
}

// ::find::
//bool TreeTool::FindPath(fs::path& partial, const fs::path& parent, bool allow_partial_match)
//{
//	for (const fs::path& p : fs::directory_iterator(parent, fs::directory_options::skip_permission_denied))
//	{
//		if (( allow_partial_match && p.filename().string().find(partial.filename().string()) != std::string::npos) ||
//			(!allow_partial_match && p.filename().string() == partial.filename().string()))
//		{
//			m_PathToFind = p / m_PathToFind;
//			return true;
//		}
//	}
//}

/*
::find::

When searching for a path we have to keep a list of all the matches.
A path matches the searched one if the filename contains the filename of the search path and the parent paths are identical.

tree:
AAA/AAAA
AAAA/BBB/AAAA
AAAA/BBBB/AAAA
AAAA/BBBB/BBBB
find: BBB
results:
AAAA/BBB
AAAA/BBBB
AAAA/BBBB/BBBB

However when rendering we have to specify that the node is open before creating it.
So the results list should look like
AAAA
AAAA/BBB
AAAA/BBBB
AAAA/BBBB/BBBB

A tree node should be open if there is a found path that starts with the node's path.
Since we need to set the node open before rendering it, and thus before reaching a subnode with a searched path,
we cannot work our way back opening the child first and then it's parents,
we need a way to know in advance if the list contains a path starting with the current one.

We also have to load into cache the new open paths, otherwise there won't be a node to open.

*/