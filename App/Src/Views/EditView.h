#pragma once
#include "Utility.h"
#include "ViewModels/EditViewModel.h"
#include "Views/EditorView.h"
#include "Tools/Tool.h"

typedef unsigned int ImGuiID;

template <typename S, std::enable_if_t<std::is_base_of_v<Tool, S>, bool> = true>
concept ToolType = true;



class EditView
{
public:
	enum class ToolPosition
	{
		LEFT, RIGHT, BOTTOM
	};

private:
	struct ToolInfo
	{
		uptr<Tool> ToolPtr;
		ToolPosition Position;
		callable OnDestroy;
	};
	using ToolIterator = std::list<ToolInfo>::iterator;
	using ToolCIterator = std::list<ToolInfo>::const_iterator;

public:
	EditView(EditModel* model, EditorModel* editor);
	~EditView();

	void Init();
	void Render();



	void EmulationStarted();
	void EmulationStopped();

	void OpenEmuIOTool(bool open = true);

	void RenderDeleteConfirmationUI();
	void RenderNewFileUI();

private:
	void ProcessShortcuts();
	void RenderMainMenu();
	void RenderEditor();
	void RenderSidebars();

	bool CloseTool(Tool::Type type);
	ToolIterator GetTool(Tool::Type type);
	bool IsToolOpen(Tool::Type type);
	
	void ToggleTreeView();
	void ToggleMemoryTool();
	void ToggleEmuIOTool();
	void ToggleEmuImgTool();

	void _OpenEmuIOTool();

	template <ToolType T, typename... Args>
	void OpenToolView(ToolPosition pos, Args&&... args);
	void OpenToolView(Tool* tool, ToolPosition pos);


private:
	EditorView m_EditorView;
	EditViewModel m_VM;

	ImGuiID m_DockspaceID = 0, m_DockIDLeft = 0, m_DockIDRight = 0, m_DockIDBottom = 0, m_DockIDCenter = 0;
	std::list<ToolInfo> m_Tools;

	fs::path m_PathToDelete, m_PathToNew;
	bool m_CanEmulate = true, m_AllowStdEmuInput = true;
};



template <ToolType T, typename... Args>
inline void EditView::OpenToolView(ToolPosition pos, Args&&... args)
{
	Tool* tool = new T(std::forward<Args>(args)...);
	OpenToolView(tool, pos);
}