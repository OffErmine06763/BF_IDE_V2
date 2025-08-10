#include "EditModel.h"


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
	if (m_Emulating)
		m_Emulator->Stop();
	if (m_Emulator && m_Emulator->joinable())
		m_Emulator->join();

	LOG_APP("EditModel Destroyed\n");
}


bool EditModel::StartEmulation()
{
	LOG("Starting Emulation of " << m_Editor->GetFocusedFile()->Name << '\n');
	if (m_Emulating) return false;

	m_Emulating = true;
	m_EmuOutput.clear();
	m_Editor->Lock(true);
	if (m_Emulator && m_Emulator->joinable())
		m_Emulator->join();
	m_Emulator = std::make_unique<Emulator>(m_Editor->GetFocusedFile()->Path,
		[this](const std::string& output) { m_EmuOutput.append(output); m_EmulationOutputEvent.Notify(); },
		[this]() { m_EmuWantsInput = true; m_EmulationInputEvent.Notify(); },
		[this]() { OnEmulationTerminated(); });

	return true;

	/* TAG: Toolbar
	m_FirstShowEmulation = true;
	m_ToolsVisible |= true;
	m_CanEmulate = false;
	*/
}
bool EditModel::StopEmulation()
{
	if (!m_Emulating) return false;

	_StopEmulation();
	m_Editor->Lock(false);
	m_Emulating = false;

	return true;
}
bool EditModel::EmulationInput(bf_mem_t input)
{
	if (!m_Emulating || !m_EmuWantsInput) return false;

	m_Emulator->GiveInput(input);
	return true;
}
void EditModel::OnEmulationTerminated()
{
	LOG("Emulation Terminated\n");
	m_Editor->Lock(false);
	m_Emulating = false;
	m_EmulationTerminatedEvent.Notify();
}


void EditModel::_StopEmulation()
{
	LOG("Stopping Emulation\n");
	m_Emulator->Stop();
	m_Emulator->join();
}

//void EditModel::OnEditorFileChanged(const fs::path& dir)
//{
//	dbg << "EditModel::OnEditorFileChanged focused file = " << dir << '\n';
//}