#pragma once
#include "Models/EditModel.h"
#include "Views/EditorView.h"
#include "Utility.h"


class EditView;

enum class CompilationTarget
{
	CURRENT, OPEN
};

class EditViewModel
{
public:
	EditViewModel(EditView* view, EditModel* model, EditorModel* editor);

	void OpenFile(const fs::path& path);

	void StartEmulation();
	void StopEmulation();
	void CloseApp();
	void CloseEmulationTab();
	void EmulationInput(bf_mem_t input);

	void OnEmulationTerminated();
	void OnEmulationOutputChanged();
	void OnEmulationInputRequested();
	const std::string& GetEmulationOutput() { return m_Model->GetEmulationOutput(); }

	void Compile(const CompilationTarget& file);

	fs::path GetWorkDir() const { return m_Model->GetWorkDir(); }

private:
	EditModel* m_Model;
	EditView* m_View;
	EditorModel* m_Editor;
};