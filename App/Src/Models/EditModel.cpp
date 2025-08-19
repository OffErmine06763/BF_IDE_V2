#include "EditModel.h"

#include "App.h"
#include "States/SelectProjectState.h"

//#define BIND(T, fn) [this](T p) { this->fn(p); }
//#define BIND_MODEL(T, fn, model, event) model.Subscribe<T>(event, BIND(T, fn))


EditModel::EditModel(const fs::path& workdir, EditorModel* editor)
	: m_Editor(editor)
{
	//m_Editor->SubscribeFocus([this](const Document& doc) { OnEditorFileChanged(doc.Path); });
	if (fs::is_directory(workdir))
		m_WorkDir = workdir;
	else
	{
		m_WorkDir = workdir.parent_path();
		m_Editor->OpenOrFocus(workdir);
	}

	LOG_APP("EditModel Created\n");

	//m_Editor.Subscribe<Document::idt>(EditorModel::FOCUS, [this](Document::idt param) { this->Test(param); });
	//m_Editor.Subscribe<u32>(EditorModel::FOCUS, BIND(u32, Test));
	//BIND_MODEL(u32, Test, m_Editor, EditorModel::FOCUS);
	//m_Editor.Subscribe(EditorModel::FOCUS, [this]() { this->Test(); });
}
EditModel::~EditModel()
{
	std::lock_guard lk(m_EmuExtMtx);
	if (m_Emulator.Running())
		m_Emulator.Stop();
	if (m_EmulatorThread && m_EmulatorThread->joinable())
		m_EmulatorThread->join();

	LOG_APP("EditModel Destroyed\n");
}


void EditModel::DeletePath(const fs::path& path)
{
	fs::remove_all(path);
	m_DeletePathEvent.Notify(path);
	if (path == m_WorkDir)
	{
		App::GetHistory().RemoveRecursive(path);
		App::RequestNewState<SelectProjectState>();
	}
}


bool EditModel::StartEmulation(const CompilationTarget& tgt)
{
	LOG("Starting Emulation\n");

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
		if (!focus) return false;
		p.tgts.push_back(focus->Path);
		p.outputPath = fs::path{ p.tgts[0] }.replace_extension(".exe");
	}
	else if (tgt == CompilationTarget::FOLDER)
	{
		const auto dir = GetWorkDir();
		p.tgts.push_back(dir);
		p.outputPath = dir / (dir.filename().string() + ".exe");
	}


	std::lock_guard lk1(m_EmuExtMtx);
	{
		std::lock_guard lk2(m_EmuLoopMtx);
		if (m_Emulator.Running())
			return false;
	}
	if (m_EmulatorThread && m_EmulatorThread->joinable()) // m_Emulator.Running() is set to false before the thread actually terminating
		m_EmulatorThread->join();

	m_Emulator.Reset();

	m_EmuOutput.clear();
	m_Editor->Lock(true);

	const auto dir = GetWorkDir();
	p.tgts.push_back(dir);
	// rn can't move Start (which does the compilation) inside the new thread, as m_Emulator.Running() is set inside this function
	// (it's quite fast in release, badapple in 0.3s)
	BFC::CompilerError err = m_Emulator.Start(p);
	if (err) LOG_COMP(err.message);
	
	std::cout << "START\n";
	m_EmulatorThread = std::make_unique<std::thread>([&]() { EmulationLoop(); });

	return true;
}
void EditModel::StopEmulation()
{
	LOG("Stopping Emulation\n");
	std::lock_guard lk1(m_EmuExtMtx);
	{
		std::lock_guard lk2(m_EmuLoopMtx);
		if (m_Emulator.Running())
			m_Emulator.Stop();
	}
	if (m_EmulatorThread && m_EmulatorThread->joinable())
		m_EmulatorThread->join();
	m_EmulatorThread.reset();
	m_Editor->Lock(false);
}

bool EditModel::EmulationInput(bf_mem_t input)
{
	std::lock_guard lk1(m_EmuExtMtx);
	std::lock_guard lk2(m_EmuLoopMtx);
	if (!m_Emulator.Running() || !m_Emulator.WantsInput())
		return false;
	m_EmuInput = input;
	m_EmuCV.notify_one();
	return true;
}
void EditModel::EmulationLoop()
{
	App::ScheduleTask([&]() { m_EmulationStartedEvent.Notify(); });

	while (1)
	{
		std::unique_lock lk(m_EmuLoopMtx);
		if (!m_Emulator.Running())
			break;

		Emulator::StepParams p;
		p.I = m_EmuInput;
		m_Emulator.Step(p);

		if (p.OutputGenerated) // first print any generated output
		{
			m_EmuOutput.push_back(p.O);
			App::ScheduleTask([this, p]() { m_EmulationOutputEvent.Notify(p.O); });
		}
		if (p.Error != Emulator::NONE)
			std::cout << p.ErrorDescription; // TODO
		else if (p.InputRequested)
		{
			App::ScheduleTask([&]() { m_EmulationWantInputEvent.Notify(); });
			m_EmuCV.wait(lk);
		}
	}

	App::ScheduleTask([&]()
		{
			m_EmulationTerminatedEvent.Notify();
			m_Editor->Lock(false);
			std::cout << "STOP\n";
		});
	// ::emo_view::
	std::cout << "END\n";
}

bool EditModel::IsEmulating()
{
	std::lock_guard lk1(m_EmuExtMtx);
	std::lock_guard lk2(m_EmuLoopMtx);
	return m_Emulator.Running();
}
const std::vector<bf_mem_t>& EditModel::GetEmulationMemory()
{
	std::lock_guard lk1(m_EmuExtMtx);
	std::lock_guard lk2(m_EmuLoopMtx);
	return m_Emulator.GetCMemory();
}
const u32* EditModel::GetEmulationAddress()
{
	std::lock_guard lk1(m_EmuExtMtx);
	std::lock_guard lk2(m_EmuLoopMtx);
	return m_Emulator.GetAddress();
}


//void EditModel::OnEditorFileChanged(const fs::path& dir)
//{
//	dbg << "EditModel::OnEditorFileChanged focused file = " << dir << '\n';
//}