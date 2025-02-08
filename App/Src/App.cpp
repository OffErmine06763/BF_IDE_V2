#include "App.h"

#include "States/WorkingState.h"
#include "States/SelectProjectState.h"
#include "States/OpenProjectState.h"

#include <format>



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
		// Instance->m_State = std::make_unique<SelectProjectState>(); // create state only after initialization
		Instance->m_State = std::make_unique<FileState>(Instance->m_History.begin()->Path);
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

void App::RequestOpenPath(const fs::path& path)
{
	Instance->m_History.SetAsMostRecent(path);
	const PathType type = GetPathType(path);
	switch (type)
	{
	case PathType::FILE: 
		RequestNewState<FileState>(path);
		return;
	case PathType::FOLDER:
		RequestNewState<FolderState>(path);
		return;
	case PathType::PROJECT:
		RequestNewState<ProjectState>(path);
		return;
	default:
		dbg << "RequestOpenPath: Invalid path... do nothing\n";
		break;
	}
}