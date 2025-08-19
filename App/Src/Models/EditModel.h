#pragma once
#include "Utility.h"
#include "Emulator.h"
#include "EditorModel.h"
#include "Events/Event.h"

enum class CompilationTarget
{
	CURRENT, OPEN, FOLDER
};

class EditModel
{
public:
	EditModel(const fs::path& workdir, EditorModel* editor);
	~EditModel();

	void DeletePath(const fs::path& path);

	bool StartEmulation(const CompilationTarget& tgt);
	void StopEmulation();
	bool EmulationInput(bf_mem_t input);
	bool IsEmulating();
	const std::vector<bf_mem_t>& GetEmulationMemory();
	const u32* GetEmulationAddress();

	fs::path GetWorkDir() const { return m_WorkDir; }
	const std::string& GetEmulationOutput() const { return m_EmuOutput; }

	// Those are called by the main thread, no need to synch with the emulation locks.
	listener_id SubEmuStarted(callable cb) { return m_EmulationStartedEvent.Subscribe(cb); }
	listener_id SubEmuTerminated(callable cb) { return m_EmulationTerminatedEvent.Subscribe(cb); }
	listener_id SubEmuOutput(consumer<bf_mem_t> cb) { return m_EmulationOutputEvent.Subscribe(cb); }
	listener_id SubEmuInput(consumer<bf_mem_t> cb) { return m_EmulationInputEvent.Subscribe(cb); }
	listener_id SubEmuWantInput(callable cb) { return m_EmulationWantInputEvent.Subscribe(cb); }

	bool UnsubEmuOutput(listener_id id) { return m_EmulationOutputEvent.Unsubscribe(id); }
	bool UnsubEmuWantInput(listener_id id) { return m_EmulationWantInputEvent.Unsubscribe(id); }
	bool UnsubEmuTerminated(listener_id id) { return m_EmulationTerminatedEvent.Unsubscribe(id); }
	bool UnsubEmuStarted(listener_id id) { return m_EmulationStartedEvent.Unsubscribe(id); }

private:
	//void OnEditorFileChanged(const fs::path& dir);
	void EmulationLoop();

private:
	EditorModel* m_Editor;
	fs::path m_WorkDir;

	Event<const fs::path&> m_DeletePathEvent;

	std::string m_EmuOutput;
	bf_mem_t m_EmuInput = 0;
	Emulator m_Emulator;
	uptr<std::thread> m_EmulatorThread;
	/// Mutexes for locking the emulation loop inside m_EmulatorThread and for external operations over m_Emulator and the thread.
	std::mutex m_EmuLoopMtx, m_EmuExtMtx;
	std::condition_variable m_EmuCV;
	

	Event<void> m_EmulationStartedEvent, m_EmulationTerminatedEvent, m_EmulationWantInputEvent;
	Event<bf_mem_t> m_EmulationOutputEvent, m_EmulationInputEvent;
};