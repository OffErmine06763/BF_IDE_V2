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
	void EmulationInput(bf_mem_t input);
	bool IsEmulating();
	const std::vector<bf_mem_t>& GetEmulationMemory();
	const u32* GetEmulationAddress();

	void GoHome();
	void CloseApp();

	void OnEmulationTerminated();
	const std::string& GetEmulationOutput() { return m_Model->GetEmulationOutput(); }

	void Compile(const CompilationTarget& file);
	void Compile(const std::initializer_list<fs::path>& files);

	fs::path GetWorkDir() const { return m_Model->GetWorkDir(); }

	listener_id SubEmuOutput(consumer<bf_mem_t> cb) { return m_Model->SubEmuOutput(cb); }
	listener_id SubEmuWantInput(callable cb) { return m_Model->SubEmuWantInput(cb); }
	listener_id SubEmuInput(consumer<bf_mem_t> cb) { return m_Model->SubEmuInput(cb); }
	listener_id SubEmuTerminated(callable cb) { return m_Model->SubEmuTerminated(cb); }
	listener_id SubEmuStarted(callable cb) { return m_Model->SubEmuStarted(cb); }

	bool UnsubEmuOutput(listener_id id) { return m_Model->UnsubEmuOutput(id); }
	bool UnsubEmuWantInput(listener_id id) { return m_Model->UnsubEmuWantInput(id); }
	bool UnsubEmuInput(listener_id id) { return m_Model->UnsubEmuInput(id); }
	bool UnsubEmuTerminated(listener_id id) { return m_Model->UnsubEmuTerminated(id); }
	bool UnsubEmuStarted(listener_id id) { return m_Model->UnsubEmuStarted(id); }

private:
	EditModel* m_Model;
	EditView* m_View;
	EditorModel* m_Editor;
};