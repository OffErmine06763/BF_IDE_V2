#include "App.h"

#include "States/WorkingState.h"
#include "States/SelectProjectState.h"

#include <format>


std::unique_ptr<App> App::Instance = nullptr;

App::App() :
	m_State(std::make_unique<SelectProjectState>())
{ }
App::~App()
{

}
void App::Init()
{
	if (Instance == nullptr)
		Instance = std::unique_ptr<App>(new App());
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
}