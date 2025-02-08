#include "WorkingState.h"
#include "App.h"

#include <imgui.h>

#include <iostream>

namespace fs = std::filesystem;


// ################################################################## WORKING ##################################################################
WorkingState::WorkingState(const fs::path& dir)
	: m_WorkDir(dir), m_Editor(dir)
{
	//dbg << "Working Created: " << dir << '\n';
}
WorkingState::~WorkingState()
{
	//dbg << "Working Destroyed: " << m_WorkDir << '\n';
}


void WorkingState::RenderMainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("App"))
		{
			if (ImGui::MenuItem("Close"))
				App::RequestClose();
			ImGui::EndMenu();
		}
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("desc");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		//if (ImGui::BeginMenu("Edit"))
		//{
		//	if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
		//	if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
		//	ImGui::Separator();
		//	if (ImGui::MenuItem("Cut", "CTRL+X")) {}
		//	if (ImGui::MenuItem("Copy", "CTRL+C")) {}
		//	if (ImGui::MenuItem("Paste", "CTRL+V")) {}
		//	ImGui::EndMenu();
		//}
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
	dbg << "Working File Created: " << m_WorkDir << '\n';
	m_Editor.OpenFile(workdir);
}
FileState::~FileState()
{
	dbg << "Working File Created: " << m_WorkDir << '\n';
}

void FileState::Render()
{
	RenderMainMenu();
	RenderEditor();
}
void FileState::RenderMainMenu()
{
	WorkingState::RenderMainMenu();
}
// ################################################################## FILE ##################################################################



// ################################################################## FOLDER ##################################################################
FolderState::FolderState(const fs::path& workdir)
	: WorkingState(workdir)
{
	dbg << "Working Folder Created: " << m_WorkDir << '\n';
}
FolderState::~FolderState()
{
	dbg << "Working Folder Created: " << m_WorkDir << '\n';
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
	dbg << "Working Project Created: " << m_WorkDir << '\n';
}
ProjectState::~ProjectState()
{
	dbg << "Working Project Created: " << m_WorkDir << '\n';
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