#include "App.h"
#include "Shortcuts.h"

// #include "States/WorkingState.h"
#include "States/SelectProjectState.h"
#include "States/OpenProjectState.h"
// #include "States/FileState.h"
#include "States/EditState.h"

#include <imgui.h>

#include <format>



std::unique_ptr<App> App::Instance = nullptr;
const fs::path App::HistoryPath = fs::current_path() / "history.bfidedata";


App::App()
	: m_History(HistoryPath)
{ }
App::~App()
{
	LOG_APP("Destroying Application\n");
#ifdef _DEBUG
	Terminal::ResetConsole();
#endif
}
void App::Init()
{
	if (Instance == nullptr)
	{
#ifdef _DEBUG
		Terminal::SetUpConsole(); // TODO: handle exceptions
#endif

		LOG_APP("Initializing Application\n");
		Instance = std::unique_ptr<App>(new App());
		// create state only after initialization
		Instance->m_NewStateRequested = true;
		//Instance->m_NextState = std::make_unique<SelectProjectState>();
		Instance->m_NextState = std::make_unique<EditState>(Instance->m_History.begin()->Path);
	}
}


void App::Render(bool* done)
{
	Instance->_Render();
	if (!Instance->m_IsOpen)
		*done = true;
}
void App::_Render()
{
	if (m_NewStateRequested)
	{
		m_NewStateRequested = false;
		delete m_State.release();
		m_State.swap(m_NextState);
		m_State->Init();
	}

	ProcessGlobalShortcuts();
	m_State->Render();
}


void App::ProcessGlobalShortcuts()
{
	if (ImGui::IsKeyChordPressed(GS_CloseApp.Chord))
		RequestClose();
}


void App::RequestOpenPath(const fs::path& path)
{
	LOG_APP("Opening " << path << '\n');
	Instance->m_History.SetAsMostRecent(path);
	RequestNewState<EditState>(path);
}