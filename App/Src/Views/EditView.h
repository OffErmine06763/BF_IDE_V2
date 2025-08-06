#pragma once
#include "Utility.h"
#include "ViewModels/EditViewModel.h"
#include "Views/EditorView.h"

typedef unsigned int ImGuiID;

enum class ToolView
{
	NONE, TREE
};
enum class ToolPosition
{
	LEFT, RIGHT, BOTTOM
};


class EditView
{
public:
	struct TreeEntry
	{
		fs::path Path;
		bool Directory = false;
		bool Collapsed = true;
		std::vector<TreeEntry> Children;

		/// maps a path in the cached m_TreeEntries to it's index in the vector
		hmap<fs::path, size_t> Map; // TODO: replace with a tree structure, each node extends the parent path and contains the index
	};

public:
	EditView(EditModel* model, EditorModel* editor);
	~EditView();

	void Render();

	void OpenEmulationTab(bool open = true);

	void EmulationStarted();
	void EmulationStopped();

	void EmulationOutputChanged();
	void EmulationWantsInput(bool wants);

	void OpenToolView(ToolView view, ToolPosition pos);


private:
	void ProcessShortcuts();
	void RenderMainMenu();
	void RenderEmulation();
	void RenderEditor();
	void RenderSidebars();

	void RenderTreeTool();


	void CacheDirectoryTree(TreeEntry& parent);
	void RenderTreeEntry(TreeEntry& entry);

	// TAG: Toolbar
	// void RenderEmulation() override; 

private:
	EditorView m_EditorView;
	EditViewModel m_VM;

	ImGuiID m_DockspaceID = 0, m_DockIDLeft = 0, m_DockIDCenter = 0;

	ToolView m_LeftSidebarView = ToolView::NONE;

	TreeEntry m_TreeRoot;
	u32 m_TreeCacheCounter = 0;


	bf_mem_t m_EmuInput = 0;
	bool m_EmuTabOpen = false;
	bool m_CanEmulate = true;
	std::string m_EmuOutput;
	std::mutex m_EmuMutex;
	bool m_EmuWantsInput = false;
	bool m_EmuFocusInput = false;
};