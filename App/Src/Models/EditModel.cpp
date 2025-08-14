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
	{
		std::lock_guard lk(m_EmuMutex);
		if (m_Emulator.Running())
			m_Emulator.Stop();
	}
	if (m_EmulatorThread)
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


bool EditModel::StartEmulation()
{
	LOG("Starting Emulation of " << m_WorkDir.filename() << '\n');
	if (m_Emulator.Running()) return false;

	m_Emulator.Reset();

	m_EmuOutput.clear();
	m_Editor->Lock(true);
	
	BFC::CompilationParams p;
	const auto dir = GetWorkDir();
	p.tgts.push_back(dir);
	BFC::CompilerError err = m_Emulator.Start(p);
	if (err) LOG_COMP(err.message);

	if (m_EmulatorThread) // m_Emulator.Running() is set to false before the thread actually terminating
		m_EmulatorThread->join();
	m_EmulatorThread = std::make_unique<std::thread>([&]()
		{
			m_EmulationStartedEvent.Notify();

			while (m_Emulator.Running())
			{
				std::unique_lock lk(m_EmuMutex);

				Emulator::StepParams p;
				p.I = m_EmuInput;
				m_Emulator.Step(p);

				if (p.OutputGenerated) // first print any generated output
				{
					m_EmuOutput.push_back(p.O);
					m_EmulationOutputEvent.Notify(p.O);
				}
				if (p.Error != Emulator::NONE)
					std::cout << p.ErrorDescription; // TODO
				else if (p.InputRequested)
				{
					m_EmulationWantInputEvent.Notify();
					m_EmuCV.wait(lk);
				}
			}

			// TODO: notifies should not be sent from this thread: TerminatedEvent -> StartEmulation -> Deadlock on join.
			m_EmulationTerminatedEvent.Notify();
			m_Editor->Lock(false);
		});

	return true;

	/* TAG: Toolbar
	m_FirstShowEmulation = true;
	m_ToolsVisible |= true;
	m_CanEmulate = false;
	*/
}
bool EditModel::StopEmulation()
{
	{
		std::lock_guard lk(m_EmuMutex);
		if (!m_Emulator.Running()) return false;
		m_Emulator.Stop();
	}

	if (m_EmulatorThread->joinable())
		m_EmulatorThread->join();
	m_EmulatorThread.reset();
	m_Editor->Lock(false);
	return true;
}
bool EditModel::EmulationInput(bf_mem_t input)
{
	std::lock_guard lk(m_EmuMutex);
	if (!m_Emulator.Running() || !m_Emulator.WantsInput()) return false;
	m_EmuInput = input;
	m_EmuCV.notify_one();
	return true;
}
void EditModel::OnEmulationTerminated()
{
	LOG("Emulation Terminated\n");
	m_Editor->Lock(false);
	m_EmulationTerminatedEvent.Notify();
}


//void EditModel::OnEditorFileChanged(const fs::path& dir)
//{
//	dbg << "EditModel::OnEditorFileChanged focused file = " << dir << '\n';
//}