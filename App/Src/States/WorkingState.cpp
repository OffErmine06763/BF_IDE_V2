#include "WorkingState.h"
#include "App.h"

#include <imgui.h>

#include <iostream>

namespace fs = std::filesystem;


WorkingState::WorkingState(const WorkingDirectory& dir)
	: m_WorkDir(dir)
{
	std::cout << "Working Created: " << dir.Path << ' ' << dir.DirType << '\n';
}
WorkingState::~WorkingState()
{
	std::cout << "Working Destroyed: " << m_WorkDir.Path << ' ' << m_WorkDir.DirType << '\n';
}

void WorkingState::Render()
{
	RenderMainMenu();
	RenderEditor();
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
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void WorkingState::RenderEditor()
{
	m_Editor.Render();
}