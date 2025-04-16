#include "EditorViewModel.h"
#include "Views/EditorView.h"

EditorViewModel::EditorViewModel(EditorView* view, const fs::path& workdir, const consumer<Document>& onOpen, const consumer<idt>& onFocus)
	: m_View(view), m_Model(workdir, onOpen, onFocus)
{
}

void EditorViewModel::OnCloseAll()
{
	m_View->CloseAll();
}

void EditorViewModel::OnPerformSave(const idt id)
{
	bool res = m_Model.PerformSave(id);
	if (res)
		m_View->Saved(id);
}

void EditorViewModel::OnCloseFile(const idt id)
{
}

void EditorViewModel::OnOpenOrFocus(const fs::path& path)
{
	m_Model.OpenOrFocus(path);
}

void EditorViewModel::OnFileChanged(const idt id)
{
}

void EditorViewModel::OnWantCloseFile(const idt id)
{
}

void EditorViewModel::OnWantFileChange(const idt id)
{
	bool res = m_Model.ChangeFile(id);
	if (res)
		m_View->Focused(id);
}

void EditorViewModel::OnFileClosed(const std::vector<idt>& ids, bool save)
{
	bool res = m_Model.Close(ids, save);
	if (res) 
		m_View->PerformClose();
}

void EditorViewModel::OnCancelClose()
{
}

void EditorViewModel::OnPerformRename(const idt id, const std::string& name)
{
}

void EditorViewModel::OnEdit(const idt id, const char change)
{
}
