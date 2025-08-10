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

		if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			m_SelectEvent.Notify(entry.Path);

		if (ImGui::BeginPopupContextItem()) // right click
		{
			if (ImGui::MenuItem("Show in File Explorer"))
				ShowInExplorer(entry.Path);
			ImGui::EndPopup();
		}
	}
}
void TreeTool::Render()
{
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
				entry = old;
			else
				entry = { .Path = p, .Directory = !old.Directory };
		}
		else
			entry = { .Path = p, .Directory = fs::is_directory(p) };

		if (entry.Directory && !entry.Collapsed)
			CacheDirectoryTree(entry);
		newchildren.push_back(entry);
	}
	parent.Children = std::move(newchildren);
	parent.Map.clear();
	for (size_t i = 0; i < parent.Children.size(); i++)
		parent.Map.insert({ parent.Children[i].Path, i });
}