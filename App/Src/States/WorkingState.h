#pragma once
#include "State.h"
// #include "Editor.h"
#include "Utility.h"
#include "Emulator.h"

// TAG: Toolbar 
// #define IMGUI_DEFINE_MATH_OPERATORS
// #include <imgui.h>


class WorkingState : public State 
{
public:
	~WorkingState() override;

	void Render() override = 0;


protected:
	WorkingState(const fs::path& workdir);

	void ChangedFocus(const fs::path& dir);

	virtual void ProcessShortcuts();
	virtual void RenderMainMenu();
	virtual void RenderEditor();

	// TAG: Toolbar 
	// virtual void RenderEmulation();
	// virtual void RenderTools();

protected:

	const fs::path m_WorkDir;
	fs::path m_FocusedFile;

	/* TAG: Toolbar 
	bool m_Emulating = false;
	bool m_ShowEmulation = false, m_FirstShowEmulation = false;
	bool m_ToolsVisible = false;
	ImVec2 m_EditorSize = { 0, 0 }, m_EditorPos = { 0, 0 }, m_ToolsSize = { 0, 0 };
	ImGuiID m_ToolsDockspaceID = 0;
	*/


public:
	friend class Editor;
};







class FolderState : public WorkingState
{
public:
	static constexpr PathType Type = PathType::FOLDER;

	FolderState(const fs::path& workdir);
	~FolderState() override;

	void Render() override;


protected:
	void RenderMainMenu() override;
	void RenderFSTree();
};



class ProjectState : public WorkingState
{
public:
	static constexpr PathType Type = PathType::PROJECT;

	ProjectState(const fs::path& workdir);
	~ProjectState() override;

	void Render() override;


protected:
	void RenderMainMenu() override;
	void RenderFSTree();
};