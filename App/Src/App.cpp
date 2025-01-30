#include "App.h"

#include "Utility.h"
#include "States/WorkingState.h"
#include "States/SelectProjectState.h"
#include "States/OpenProjectState.h"

#include <format>

namespace fs = std::filesystem;


std::unique_ptr<App> App::Instance = nullptr;
static fs::path DEFAULT_HISTORY_PATH = fs::current_path() / "history.bfidedata";


App::App() :
	m_History(DEFAULT_HISTORY_PATH)
{ }
App::~App()
{ }
void App::Init()
{
	if (Instance == nullptr)
	{
		Instance = std::unique_ptr<App>(new App());
		Instance->m_State = std::make_unique<SelectProjectState>(); // create state only after initialization
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
	m_State->Render();

	if (m_NewStateRequested)
	{
		m_NewStateRequested = false;
		delete m_State.release();
		m_State.swap(m_NextState);
	}
}

void App::OpenPath(const fs::path& path)
{
	Instance->m_History.SetAsMostRecent(path);
	RequestNewState<WorkingState>(WorkingDirectory(path));
}