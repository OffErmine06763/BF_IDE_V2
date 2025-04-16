#pragma once
#include "Models/FileModel.h"
#include "Views/EditorView.h"
#include "Utility.h"


class FileView;

class FileViewModel 
{
public:
	FileViewModel(FileView* state, EditorModel* editor, const fs::path& workdir);

	void StartEmulation();
	void StopEmulation();
	void CloseApp();
	void CloseEmulationTab();
	void EmulationInput(bf_mem_t input);

	void OnEmulationTerminated();
	void OnEmulationOutputChanged();
	void OnEmulationInputRequested();
	const std::string& GetEmulationOutput() { return m_Model.GetEmulationOutput(); }

	fs::path GetWorkDir() const { return m_Model.GetWorkDir(); }

private:
	FileModel m_Model;
	FileView* m_View;
};