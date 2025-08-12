#pragma once
#include "Utility.h"
#include "Emulator.h"
#include "EditorModel.h"
#include "Events/Event.h"


class EditModel
{
public:
	EditModel(const fs::path& workdir, EditorModel* editor);
	~EditModel();

	void DeletePath(const fs::path& path);

	bool StartEmulation();
	bool StopEmulation();
	bool EmulationInput(bf_mem_t input);

	fs::path GetWorkDir() const { return m_WorkDir; }
	const std::string& GetEmulationOutput() const { return m_EmuOutput; }

	listener_id SubEmuTerminated(callable cb) { return m_EmulationTerminatedEvent.Subscribe(cb); }
	listener_id SubEmuOutput(callable cb) { return m_EmulationOutputEvent.Subscribe(cb); }
	listener_id SubEmuInput(callable cb) { return m_EmulationInputEvent.Subscribe(cb); }


private:
	//void OnEditorFileChanged(const fs::path& dir);
	void OnEmulationTerminated();
	void _StopEmulation();

private:
	EditorModel* m_Editor;
	fs::path m_WorkDir;

	Event<const fs::path&> m_DeletePathEvent;

	bool m_Emulating = false;
	std::string m_EmuOutput;
	uptr<Emulator> m_Emulator;

	Event<void> m_EmulationTerminatedEvent, m_EmulationOutputEvent, m_EmulationInputEvent;
	bool m_EmuWantsInput = false;
};