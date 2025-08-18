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
	m_Model->SubEmuOutput([this](bf_mem_t o) { OnEmulationOutputChanged(o); });
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
		m_View->OpenEmulationTab();
	}
}
void EditViewModel::StopEmulation()
{
	m_Model->StopEmulation();
	m_View->EmulationStopped();
}

void EditViewModel::CloseEmulationTab()
{
	m_View->OpenEmulationTab(false);
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

void EditViewModel::OnEmulationTerminated()
{
	m_View->EmulationStopped();
}
void EditViewModel::OnEmulationOutputChanged(bf_mem_t o)
{
	m_View->EmulationOutputChanged(o);
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
