#include "FileModel.h"

FileModel::FileModel(const fs::path& workdir, EditorModel* editor, const callable& onEmulationTerminated, const callable& onOutput, const callable& onInput)
	: m_OnEmulationTerminated(onEmulationTerminated), m_OnEmulationOutput(onOutput), m_OnEmulationInput(onInput), m_Editor(editor)
{
	// m_Editor->SetOnFileChangedCallback([this](const fs::path& dir) { OnEditorFileChanged(dir); });
	m_Editor->OpenOrFocus(workdir);
}

FileModel::~FileModel()
{
	if (m_Emulator != nullptr)
	{
		m_Emulator->Stop();
		m_Emulator->join();
	}
}

bool FileModel::StartEmulation()
{
	dbg << "Running: " << m_WorkDir << '\n';
	if (m_Emulating) return false;

	m_Emulating = true;
	m_EmuOutput.clear();
	m_Editor->Lock(true);
	m_Emulator = std::make_unique<Emulator>(m_FocusedFile, 
		[this](const std::string& output) { m_EmuOutput.append(output); m_OnEmulationOutput(); },
		[this]() { m_EmuWantsInput = true;  m_OnEmulationInput(); },
		[this]() { m_OnEmulationTerminated(); });
	
	return true;

	/* TAG: Toolbar
	m_FirstShowEmulation = true;
	m_ToolsVisible |= true;
	m_CanEmulate = false;
	*/
}

bool FileModel::StopEmulation()
{
	dbg << "Stopping: " << m_WorkDir << '\n';
	if (!m_Emulating) return false;

	m_Emulator->Stop();
	m_Emulator->join();
	m_Emulator = nullptr; // TODO: might nullptr before calling unlock.
	m_Editor->Lock(false);
	m_Emulating = false;

	return true;
}

bool FileModel::EmulationInput(bf_mem_t input)
{
	if (!m_EmuWantsInput) return false;

	m_Emulator->GiveInput(input);
	return true;
}

void FileModel::OnEditorFileChanged(const fs::path& dir)
{
	dbg << "WorkingState::ChangedFocus m_FocusedFile = " << dir << '\n';
	m_FocusedFile = dir;
}
