#pragma once
#include "State.h"
#include "Editor.h"
#include "Utility.h"



class WorkingState : public State 
{
public:
	~WorkingState() override;

	void Render() override = 0;

protected:
	WorkingState(const fs::path& workdir);

	virtual void RenderMainMenu();
	virtual void RenderEditor();

	Editor m_Editor;

	const fs::path m_WorkDir;
};



class FileState : public WorkingState
{
public:
	static constexpr PathType Type = PathType::FILE;

	FileState(const fs::path& workdir);
	~FileState() override;

	void Render() override;

protected:
	void RenderMainMenu() override;

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