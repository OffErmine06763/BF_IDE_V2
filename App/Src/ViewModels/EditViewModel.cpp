#include "EditViewModel.h"
#include "States/EditState.h"
#include "App.h"
#include "Views/EditView.h"


EditViewModel::EditViewModel(EditView* view, EditModel* model, EditorModel* editor)
	: m_Model(model), m_View(view), m_Editor(editor)
{
	m_Model->SubEmuTerminated([this]() { OnEmulationTerminated(); });
	m_Model->SubEmuOutput([this]() { OnEmulationOutputChanged(); });
	m_Model->SubEmuInput([this]() { OnEmulationInputRequested(); });
}

void EditViewModel::StartEmulation()
{
	bool res = m_Model->StartEmulation();
	if (res)
	{
		m_View->EmulationStarted();
		m_View->OpenEmulationTab();
	}
}

void EditViewModel::StopEmulation()
{
	bool res = m_Model->StopEmulation();
	if (res) m_View->EmulationStopped();
}

void EditViewModel::CloseApp()
{
	App::RequestClose();
}

void EditViewModel::CloseEmulationTab()
{
	m_View->CloseEmulationTab();
}

void EditViewModel::EmulationInput(bf_mem_t input)
{
	bool res = m_Model->EmulationInput(input);
	if (res)
		m_View->EmulationWantsInput(false);
}

void EditViewModel::OnEmulationTerminated()
{
	m_View->EmulationStopped();
}

void EditViewModel::OnEmulationOutputChanged()
{
	m_View->EmulationOutputChanged();
}

void EditViewModel::OnEmulationInputRequested()
{
	m_View->EmulationWantsInput(true);
}
