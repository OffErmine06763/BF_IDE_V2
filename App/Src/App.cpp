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
ID3D12Device* App::D3D12Device = nullptr;
ExampleDescriptorHeapAllocator* App::D3D12Allocator = nullptr;


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
void App::Init(ID3D12Device* d3d12_device, ExampleDescriptorHeapAllocator* d3d12_allocator)
{
	if (Instance == nullptr)
	{
#ifdef _DEBUG
		Terminal::SetUpConsole(); // TODO: handle exceptions
#endif
		D3D12Device = d3d12_device;
		D3D12Allocator = d3d12_allocator;

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
	std::lock_guard lk(Instance->m_TaskMutex);
	Instance->m_Tasks.push_back(cb);
}
