#include "EditViewModel.h"
#include "States/EditState.h"
#include "States/SelectProjectState.h"
#include "App.h"
#include "Views/EditView.h"

#include <Compiler.h>


EditViewModel::EditViewModel(EditView* view, EditModel* model, EditorModel* editor)
	: m_Model(model), m_View(view), m_Editor(editor)
{
	m_Model->SubEmuTerminated([this]() { OnEmulationTerminated(); });

	LOG_GRAPHICS("EditViewModel Created\n");
}


void EditViewModel::OpenFile(const fs::path& path)
{
	if (fs::is_directory(path))
		return;

	m_Editor->OpenOrFocus(path);
}
void EditViewModel::DeletePath(const fs::path& path)
{
	m_Model->DeletePath(path);
	m_Editor->OnPathDeleted(path);
}


void EditViewModel::GoHome()
{
	App::RequestNewState<SelectProjectState>();
}
void EditViewModel::CloseApp()
{
	App::RequestClose();
}


void EditViewModel::StartEmulation(const CompilationTarget& tgt)
{
	bool res = m_Model->StartEmulation(tgt);
	// TODO: ::emo_view:: spamming emulate fast enough causes double outputs being shown in the UI, perhaps because of notifications being sent as callbacks on the main thread
	/* Log
	Starting Emulation of "BF"
	START -> thread #1 created
	Starting Emulation of "BF"
	Starting Emulation of "BF"
	END -> thread #1 ended
	START -> thread #2 created
	STOP -> #1 end notified
	Starting Emulation of "BF"
	END -> thread #2 ended
	STOP -> #2 end notified
	*/
	if (res)
	{
		m_View->EmulationStarted();
		m_View->OpenEmuIOTool();
	}
}
void EditViewModel::StopEmulation()
{
	m_Model->StopEmulation();
	m_View->EmulationStopped();
}

void EditViewModel::SetEmulationStepping(bool stepping)
{
	m_Model->SetEmulationStepping(stepping);
}
void EditViewModel::EmulationStep()
{
	m_Model->EmulationStep();
}

void EditViewModel::EmulationInput(bf_mem_t input)
{
	bool res = m_Model->EmulationInput(input);
}

bool EditViewModel::IsEmulating()
{
	return m_Model->IsEmulating();
}
const std::vector<bf_mem_t>& EditViewModel::GetEmulationMemory()
{
	return m_Model->GetEmulationMemory();
}
const u32* EditViewModel::GetEmulationAddress()
{
	return m_Model->GetEmulationAddress();
}

void EditViewModel::OnEmulationTerminated()
{
	m_View->EmulationStopped();
}



void EditViewModel::Compile(const CompilationTarget& tgt)
{
	m_Model->Compile(tgt);
}
void EditViewModel::Compile(const std::initializer_list<fs::path>& files)
{
	m_Model->Compile(files);
}
