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
	if (m_Emulator != nullptr)
	{
		m_Emulator->Stop();
		m_Emulator->join();
	}
}

void FileState::Render()
{
	ProcessShortcuts();
	RenderMainMenu();
	RenderEditor();
	RenderEmulation();

	if (m_Emulating && m_Emulator != nullptr && m_Emulator->Done())
	{
		m_Emulator->join();
		m_Emulating = false;
		m_Emulator = nullptr;
		m_Editor.Lock(false);
	}
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
			if (ImGui::MenuItem("Run", nullptr, nullptr, !m_Emulating))
			{
				dbg << "Running: " << m_FocusedFile << '\n';
				m_Emulating = true;
				m_EmuTabOpen = true;
				m_EmuOutput.clear();
				m_Editor.Lock(true);
				m_Emulator = std::make_unique<Emulator>(m_FocusedFile, m_EmuOutput);
			}
			if (ImGui::MenuItem("Stop", nullptr, nullptr, m_Emulating))
			{
				dbg << "Stopping: " << m_FocusedFile << '\n';
				m_Emulator->Stop();
				m_Emulator->join();
				m_Emulator = nullptr;
				m_Emulating = false;
				m_Editor.Lock(false);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
void FileState::RenderEmulation() {
	if (!m_EmuTabOpen)
		return;

	static ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos({ viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y / 2 });
	ImGui::SetNextWindowSize({ viewport->WorkSize.x, viewport->WorkSize.y / 2 });

	ImGui::Begin("Emulation", &m_EmuTabOpen, flags);
	if (!m_EmuTabOpen && m_Emulator != nullptr) // TODO: might wanna close the tab but keep emulation going, add button to reopen the tab
	{
		m_Emulator->Stop();
		m_Emulator->join();
		m_Emulator = nullptr;
		m_Emulating = false;
		m_Editor.Lock(false);
	}

	if (m_Emulator != nullptr)
		m_Emulator->Lock(true);
	
	ImGui::Text(m_EmuOutput.c_str());

	if (m_Emulator != nullptr && m_Emulator->WantsInput())
	{
		ImGui::Text(">>"); ImGui::SameLine();
		ImGui::InputScalar("##input", ImGuiDataType_U8, &m_EmuInput, nullptr, nullptr, nullptr);
		if (ImGui::Button("Confirm"))
		{
			m_Emulator->GiveInput(m_EmuInput);
			m_EmuInput = 0;
		}
	}
	if (m_Emulator != nullptr)
		m_Emulator->Lock(false);

	ImGui::End();
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