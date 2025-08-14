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
	listener_id SubEmuOutput(consumer<bf_mem_t> cb) { return m_EmulationOutputEvent.Subscribe(cb); }
	listener_id SubEmuInput(consumer<bf_mem_t> cb) { return m_EmulationInputEvent.Subscribe(cb); }
	listener_id SubEmuWantInput(callable cb) { return m_EmulationWantInputEvent.Subscribe(cb); }


private:
	//void OnEditorFileChanged(const fs::path& dir);
	void OnEmulationTerminated();

private:
	EditorModel* m_Editor;
	fs::path m_WorkDir;

	Event<const fs::path&> m_DeletePathEvent;

	std::string m_EmuOutput;
	bf_mem_t m_EmuInput;
	Emulator m_Emulator;
	uptr<std::thread> m_EmulatorThread;
	std::mutex m_EmuMutex;
	std::condition_variable m_EmuCV;
	

	Event<void> m_EmulationStartedEvent, m_EmulationTerminatedEvent, m_EmulationWantInputEvent;
	Event<bf_mem_t> m_EmulationOutputEvent, m_EmulationInputEvent;
};