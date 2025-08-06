#include "EditViewModel.h"
#include "States/EditState.h"
#include "App.h"
#include "Views/EditView.h"

#include <Compiler.h>


EditViewModel::EditViewModel(EditView* view, EditModel* model, EditorModel* editor)
	: m_Model(model), m_View(view), m_Editor(editor)
{
	m_Model->SubEmuTerminated([this]() { OnEmulationTerminated(); });
	m_Model->SubEmuOutput([this]() { OnEmulationOutputChanged(); });
	m_Model->SubEmuInput([this]() { OnEmulationInputRequested(); });
}


void EditViewModel::OpenFile(const fs::path& path)
{
	if (fs::is_directory(path))
		return;

	m_Editor->OpenOrFocus(path);
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
	m_View->OpenEmulationTab(false);
}

void EditViewModel::EmulationInput(bf_mem_t input)
{
	bool res = m_Model->EmulationInput(input);
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

void EditViewModel::Compile(const CompilationTarget& tgt)
{
	Program p;
	if (tgt == CompilationTarget::OPEN)
	{
		for (const Document& doc : m_Editor->GetDocuments())
			p.tgts.push_back(doc.Path);
	}
	else if (tgt == CompilationTarget::CURRENT)
		p.tgts.push_back(m_Editor->GetFocusedFile()->Path);
	p.outputPath = fs::path{ p.tgts[0] }.replace_extension(".exe");
	CompilerError err = Compiler::Compile(p, "../Compiler/");
	if (err) LOG_COMP(err.message << '\n');
}
