#pragma once
#include "Utility.h"
#include "ViewModels/FileViewModel.h"
#include "Views/EditorView.h"


class FileView
{
public:
	static constexpr PathType Type = PathType::FILE;

	FileView(const fs::path& workdir);
	~FileView();

	void Render();

	void OpenEmulationTab();
	void CloseEmulationTab();

	void EmulationStarted();
	void EmulationStopped();

	void EmulationOutputChanged();
	void EmulationWantsInput(bool wants);

protected:
	void ProcessShortcuts();
	void RenderMainMenu();
	void RenderEmulation();
	void RenderEditor();

	// TAG: Toolbar 
	// void RenderEmulation() override; 

private:
	EditorView m_EditorView;
	
	FileViewModel m_VM;

	bf_mem_t m_EmuInput = 0;
	bool m_EmuTabOpen = false;
	bool m_CanEmulate = true;
	std::string m_EmuOutput;
	std::mutex m_EmuMutex;
	bool m_EmuWantsInput = false;
};