#pragma once
#include "Utility.h"
#include "ViewModels/EditViewModel.h"
#include "Views/EditorView.h"


class EditView
{
public:
	static constexpr PathType Type = PathType::FILE;

	EditView(EditModel* model, EditorModel* editor);
	~EditView();

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
	
	EditViewModel m_VM;

	bf_mem_t m_EmuInput = 0;
	bool m_EmuTabOpen = false;
	bool m_CanEmulate = true;
	std::string m_EmuOutput;
	std::mutex m_EmuMutex;
	bool m_EmuWantsInput = false;
};