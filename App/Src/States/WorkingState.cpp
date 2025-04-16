#include "WorkingState.h"
#include "App.h"
#include "Shortcuts.h"

#include <imgui.h>


namespace fs = std::filesystem;


// ################################################################## WORKING ##################################################################
WorkingState::WorkingState(const fs::path& dir)
	: m_WorkDir(dir)//, m_Editor(this)
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
	// m_Editor.Render();
}
// ################################################################## WORKING ##################################################################




// ################################################################## FILE ##################################################################

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



/* TAG: Toolbar 
void WorkingState::RenderTools()
{
	static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	m_ToolsSize.x = viewport->WorkSize.x;
	ImGui::SetNextWindowSize(m_ToolsSize);

	ImGui::Begin("Tools", nullptr, flags | (m_ToolsVisible ? 0 : ImGuiWindowFlags_NoResize));

	m_ToolsSize.y = ImGui::GetWindowHeight();
	if (m_ToolsSize.y < ImGui::GetFrameHeight())
	{
		m_ToolsVisible = false;
		m_ToolsSize.y = ImGui::GetFrameHeight();
	}

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::MenuItem("Close"))
		{
			m_ToolsVisible = false;
			m_ToolsSize.y = 35;
		}
		ImGui::EndMenuBar();
	}

	if (m_ToolsVisible)
	{
		m_ToolsDockspaceID = ImGui::GetID("ToolsDockSpace");
		ImGui::DockSpace(m_ToolsDockspaceID, { 0, 0 }, ImGuiDockNodeFlags_None);
		// dbg << m_ToolsDockspaceID << '\n';

		RenderEmulation();

		if (!m_ShowEmulation)
			m_ToolsVisible = false;
	}

	ImGui::SetWindowPos({ viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - m_ToolsSize.y });
	m_EditorSize.y -= m_ToolsSize.y;

	ImGui::End();
}
void WorkingState::RenderEmulation()
{
	static const ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse;

	if (m_FirstShowEmulation)
	{
		m_ShowEmulation = true;
		m_FirstShowEmulation = false;

		m_ToolsSize.y = 200;
		ImGui::SetNextWindowFocus();
		ImGui::SetNextWindowDockID(m_ToolsDockspaceID);
	}
	if (m_ShowEmulation)
	{
		if (ImGui::Begin("Emulator", &m_ShowEmulation, flags))
		{
			//auto vp = ImGui::GetWindowViewport();
			//dbg << vp->ID << ' ' << vp->ParentViewportId << ' ' << ImGui::GetWindowDockID() << '\n';
			ImGui::Text("AAA");
		}
		ImGui::End();
	}
}
*/