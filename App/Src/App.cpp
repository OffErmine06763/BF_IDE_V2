#include "App.h"
#include "Shortcuts.h"

// #include "States/WorkingState.h"
#include "States/SelectProjectState.h"
#include "States/OpenProjectState.h"
// #include "States/FileState.h"
#include "States/EditState.h"

#include <imgui.h>
#include "d3d12_stuff.h"

#include <format>



std::unique_ptr<App> App::Instance = nullptr;
const fs::path App::HistoryPath = fs::current_path() / "history.bfidedata";
App::DXData App::_DXData = { 0 };


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
void App::Init(DXData data)
{
	if (Instance == nullptr)
	{
#ifdef _DEBUG
		Terminal::SetUpConsole(); // TODO: handle exceptions
#endif
		_DXData = data;

		LOG_APP("Initializing Application\n");
		Instance = std::unique_ptr<App>(new App());
		// create state only after initialization
		Instance->m_NewStateRequested = true;
		//Instance->m_NextState = std::make_unique<SelectProjectState>();
		Instance->m_NextState = std::make_unique<EditState>(Instance->m_History.begin()->Path);
	}
}
void App::Stop()
{
	// Destroy the object to free any UI memory before the final checks at the end of main
	Instance.reset();
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

	std::lock_guard lk(m_TaskMutex);
	for (callable cb : m_Tasks)
		cb();
	m_Tasks.clear();
}


void App::ProcessGlobalShortcuts()
{
	if (ImGui::IsKeyChordPressed(GS_CloseApp.Chord))
		RequestClose();
}


void App::RequestOpenPath(const fs::path& path)
{
	if (!fs::exists(path))
		return;

	LOG_APP("Opening " << path << '\n');
	Instance->m_History.SetAsMostRecent(path);
	RequestNewState<EditState>(path);
}



void App::ScheduleTask(callable cb)
{
	if (!Instance) return;
	std::lock_guard lk(Instance->m_TaskMutex);
	Instance->m_Tasks.push_back(cb);
}

//void App::ScheduleDXTask(callable cb)
//{
//	if (!Instance) return;
//	std::lock_guard lk(Instance->m_DXTaskMutex);
//	Instance->m_DXTasks.push_back(cb);
//}
//void App::ScheduleDXResourceRelease(callable cb)
//{
//	if (!Instance) return;
//	std::lock_guard lk(Instance->m_DXResourceTaskMutex);
//	Instance->m_DXResourceTasks.push_back(cb);
//}
//void App::ExecuteDXCommands()
//{
//	if (!Instance) return;
//	std::lock_guard lk(Instance->m_DXTaskMutex);
//	for (callable cb : Instance->m_DXTasks)
//		cb();
//	Instance->m_DXTasks.clear();
//}
//void App::ExecuteDXResourceTasks()
//{
//	if (!Instance) return;
//	std::lock_guard lk(Instance->m_DXResourceTaskMutex);
//	for (callable cb : Instance->m_DXResourceTasks)
//		cb();
//	Instance->m_DXResourceTasks.clear();
//}
