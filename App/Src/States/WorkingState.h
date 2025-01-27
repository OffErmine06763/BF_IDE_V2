#pragma once
#include "State.h"
#include "Editor.h"
#include "Utility.h"

#include <filesystem>


class WorkingState : public State 
{
public:
	WorkingState(const WorkingDirectory& workingdir);
	~WorkingState() override;

	void Render() override;

private:
	void RenderMainMenu();
	void RenderEditor();


	bool m_WantRedock = false;
	Editor m_Editor;

	const WorkingDirectory m_WorkDir;
};

class FileState : public WorkingState
{
public:

private:

};
class FolderState : public WorkingState
{
public:

private:

};
class ProjectState : public WorkingState
{
public:

private:

};