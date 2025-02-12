#include "WorkingState.h"
#include "App.h"
#include "Shortcuts.h"

#include <imgui.h>


namespace fs = std::filesystem;


// ################################################################## WORKING ##################################################################
WorkingState::WorkingState(const fs::path& dir)
	: m_WorkDir(dir), m_Editor(this)
{
	//dbg << "Working Created: " << dir << '\n';
}
WorkingState::~WorkingState()
{
	//dbg << "Working Destroyed: " << m_WorkDir << '\n';
}

void WorkingState::ChangedFocus(const fs::path& dir)
{
	dbg << "WorkingState::ChangedFocus m_FocusedFile = " << dir << '\n';
	m_FocusedFile = dir;
}

void WorkingState::ProcessShortcuts() 
{ }
void WorkingState::RenderMainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("App"))
		{
			if (ImGui::MenuItem("Close", GS_CloseApp.Label))
				App::RequestClose();
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
void WorkingState::RenderEditor()
{
	m_Editor.Render();
}
// ################################################################## WORKING ##################################################################




// ################################################################## FILE ##################################################################
FileState::FileState(const fs::path& workdir)
	: WorkingState(workdir)
{
	dbg << "FileState::FileState m_WorkDir = " << m_WorkDir << '\n';
	m_Editor.OpenFile(workdir);
}
FileState::~FileState()
{
	dbg << "FileState::~FileState m_WorkDir = " << m_WorkDir << '\n';
}

void FileState::Render()
{
	ProcessShortcuts();
	RenderMainMenu();
	RenderEditor();
}
void FileState::ProcessShortcuts()
{
	WorkingState::ProcessShortcuts();
}
void FileState::RenderMainMenu()
{
	WorkingState::RenderMainMenu();
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Run"))
		{
			if (ImGui::MenuItem("Compile"))
			{ 
				dbg << "Compiling: " << m_FocusedFile << '\n';
			}
			if (ImGui::MenuItem("Run"))
			{
				dbg << "Running: " << m_FocusedFile << '\n';
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
// ################################################################## FILE ##################################################################



// ################################################################## FOLDER ##################################################################
FolderState::FolderState(const fs::path& workdir)
	: WorkingState(workdir)
{
	dbg << "FolderState::FolderState m_WorkDir = " << m_WorkDir << '\n';
}
FolderState::~FolderState()
{
	dbg << "FolderState::~FolderState m_WorkDir = " << m_WorkDir << '\n';
}

void FolderState::Render()
{
	RenderMainMenu();
	RenderEditor();
	RenderFSTree();
}
void FolderState::RenderMainMenu()
{
	WorkingState::RenderMainMenu();
}
void FolderState::RenderFSTree()
{
}
// ################################################################## FOLDER ##################################################################



// ################################################################## PROJECT ##################################################################
ProjectState::ProjectState(const fs::path& workdir)
	: WorkingState(workdir)
{
	dbg << "ProjectState::ProjectState m_WorkDir = " << m_WorkDir << '\n';
}
ProjectState::~ProjectState()
{
	dbg << "ProjectState::~ProjectState m_WorkDir = " << m_WorkDir << '\n';
}

void ProjectState::Render()
{
	RenderMainMenu();
	RenderEditor();
	RenderFSTree();
}
void ProjectState::RenderMainMenu()
{
	WorkingState::RenderMainMenu();
}
void ProjectState::RenderFSTree()
{
}
// ################################################################## PROJECT ##################################################################