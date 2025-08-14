#pragma once
#include "Utility.h"
#include "ViewModels/EditViewModel.h"
#include "Views/EditorView.h"

typedef unsigned int ImGuiID;


class Tool;

template <typename S, std::enable_if_t<std::is_base_of_v<Tool, S>, bool> = true>
concept ToolType = true;



class EditView
{
public:
	enum class ToolPosition
	{
		LEFT, RIGHT, BOTTOM
	};

public:
	EditView(EditModel* model, EditorModel* editor);
	~EditView();

	void Init();
	void Render();

	void OpenEmulationTab(bool open = true);

	void EmulationStarted();
	void EmulationStopped();

	void EmulationOutputChanged();
	void EmulationWantsInput(bool wants);

	template <ToolType T, typename... Args>
	void OpenToolView(ToolPosition pos, Args&&... args);
	void OpenToolView(Tool* tool, ToolPosition pos);

	void RenderDeleteConfirmationUI();
	void RenderNewFileUI();

private:
	void ProcessShortcuts();
	void RenderMainMenu();
	void RenderEmulation();
	void RenderEditor();
	void RenderSidebars();

	// TAG: Toolbar
	// void RenderEmulation() override; 

private:
	EditorView m_EditorView;
	EditViewModel m_VM;

	ImGuiID m_DockspaceID = 0, m_DockIDLeft = 0, m_DockIDRight = 0, m_DockIDBottom = 0, m_DockIDCenter = 0;

	uptr<Tool> m_LeftSidebarTool, m_RightSidebarTool, m_BottomSidebarTool;

	fs::path m_PathToDelete, m_PathToNew;

	bf_mem_t m_EmuInput = 0;
	bool m_EmuTabOpen = false;
	bool m_CanEmulate = true;
	std::string m_EmuOutput;
	std::mutex m_EmuMutex;
	bool m_EmuWantsInput = false;
	bool m_EmuFocusInput = false;
};



template <ToolType T, typename... Args>
inline void EditView::OpenToolView(ToolPosition pos, Args&&... args)
{
	Tool* tool = new T(std::forward<Args>(args)...);
	OpenToolView(tool, pos);
}