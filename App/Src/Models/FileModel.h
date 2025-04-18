#pragma once
#include "Utility.h"
#include "Emulator.h"
#include "EditorModel.h"


class FileModel
{
public:
	FileModel(const fs::path& workdir, EditorModel* editor, const callable& onEmuEnd, const callable& onOutput, const callable& onInput);
	~FileModel();

	bool StartEmulation();
	bool StopEmulation();
	bool EmulationInput(bf_mem_t input);

	fs::path GetWorkDir() const { return m_WorkDir; }
	const std::string& GetEmulationOutput() const { return m_EmuOutput; }

private:
	void OnEditorFileChanged(const fs::path& dir);
	void OnEmulationTerminated();

private:
	EditorModel* m_Editor;
	fs::path m_FocusedFile;
	const fs::path m_WorkDir;


	bool m_Emulating = false;
	std::string m_EmuOutput;
	uptr<Emulator> m_Emulator;

	callable m_OnEmulationTerminated;
	callable m_OnEmulationOutput, m_OnEmulationInput;
	bool m_EmuWantsInput = false;
};