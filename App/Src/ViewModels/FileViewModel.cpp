#include "FileViewModel.h"
#include "States/FileState.h"
#include "App.h"
#include "Views/FileView.h"


FileViewModel::FileViewModel(FileView* state, EditorModel* editor, const fs::path& workdir)
	: m_Model(workdir, editor, [this]() { OnEmulationTerminated(); }, [this]() { OnEmulationOutputChanged(); }, [this]() { OnEmulationInputRequested(); }), m_View(state)
{ }

void FileViewModel::StartEmulation()
{
	bool res = m_Model.StartEmulation();
	if (res)
	{
		m_View->EmulationStarted();
		m_View->OpenEmulationTab();
	}
}

void FileViewModel::StopEmulation()
{
	bool res = m_Model.StopEmulation();
	if (res) m_View->EmulationStopped();
}

void FileViewModel::CloseApp()
{
	App::RequestClose();
}

void FileViewModel::CloseEmulationTab()
{
	m_View->CloseEmulationTab();
}

void FileViewModel::EmulationInput(bf_mem_t input)
{
	bool res = m_Model.EmulationInput(input);
	if (res)
		m_View->EmulationWantsInput(false);
}

void FileViewModel::OnEmulationTerminated() 
{
	m_View->EmulationStopped();
}

void FileViewModel::OnEmulationOutputChanged()
{
	m_View->EmulationOutputChanged();
}

void FileViewModel::OnEmulationInputRequested()
{
	m_View->EmulationWantsInput(true);
}
