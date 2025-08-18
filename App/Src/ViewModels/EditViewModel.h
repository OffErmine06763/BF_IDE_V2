#pragma once
#include "Models/EditModel.h"
#include "Views/EditorView.h"
#include "Utility.h"
#include <Compiler.h>


class EditView;

class EditViewModel
{
public:
	EditViewModel(EditView* view, EditModel* model, EditorModel* editor);
	~EditViewModel() { LOG_GRAPHICS("EditViewModel Destroyed\n"); }

	void OpenFile(const fs::path& path);
	void DeletePath(const fs::path& path);

	void StartEmulation(const CompilationTarget& tgt);
	void StopEmulation();
	void CloseEmulationTab();
	void EmulationInput(bf_mem_t input);
	bool IsEmulating();
	const std::vector<bf_mem_t>& GetEmulationMemory();
	
	void GoHome();
	void CloseApp();

	void OnEmulationTerminated();
	void OnEmulationOutputChanged(bf_mem_t o);
	void OnEmulationInputRequested();
	const std::string& GetEmulationOutput() { return m_Model->GetEmulationOutput(); }

	void Compile(const CompilationTarget& file);
	void Compile(const std::initializer_list<fs::path>& files);

	fs::path GetWorkDir() const { return m_Model->GetWorkDir(); }

private:
	EditModel* m_Model;
	EditView* m_View;
	EditorModel* m_Editor;
};