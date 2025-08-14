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
	m_Model->SubEmuOutput([this](bf_mem_t) { OnEmulationOutputChanged(); });
	m_Model->SubEmuWantInput([this]() { OnEmulationInputRequested(); });

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

void EditViewModel::GoHome()
{
	App::RequestNewState<SelectProjectState>();
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
	BFC::CompilationParams p;
	if (tgt == CompilationTarget::OPEN)
	{
		for (const Document& doc : m_Editor->GetDocuments())
			p.tgts.push_back(doc.Path);
		p.outputPath = fs::path{ p.tgts[0] }.replace_extension(".exe");
	}
	else if (tgt == CompilationTarget::CURRENT)
	{
		auto focus = m_Editor->GetFocusedFile();
		if (!focus) return;
		p.tgts.push_back(focus->Path);
		p.outputPath = fs::path{ p.tgts[0] }.replace_extension(".exe");
	}
	else if (tgt == CompilationTarget::FOLDER)
	{
		const auto dir = GetWorkDir();
		p.tgts.push_back(dir);
		p.outputPath = dir / (dir.filename().string() + ".exe");
	}
	BFC::CompilerError err = BFC::Compiler::Compile(p, "../Compiler/");
	if (err) LOG_COMP(err.message << '\n');
}
void EditViewModel::Compile(const std::initializer_list<fs::path>& files)
{
	if (files.size() == 0)
		return;

	BFC::CompilationParams p;
	p.tgts = files;
	auto first = p.tgts[0];
	if (fs::is_directory(first))
		p.outputPath = first / (first.filename().string() + ".exe");
	else
		p.outputPath = fs::path{ first }.replace_extension(".exe");
	BFC::CompilerError err = BFC::Compiler::Compile(p, "../Compiler/");
	if (err) LOG_COMP(err.message << '\n');
}
