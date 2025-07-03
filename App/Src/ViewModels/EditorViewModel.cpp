#include "EditorViewModel.h"
#include "Views/EditorView.h"

EditorViewModel::EditorViewModel(EditorView* view, EditorModel* model)
	: m_View(view), m_Model(model)
{
}

void EditorViewModel::OnCloseAll()
{
	m_View->CloseAll();
}

void EditorViewModel::OnPerformSave(Document& doc)
{
	m_Model->PerformSave(doc);
}

void EditorViewModel::OnCloseFile(const idt id)
{
}

void EditorViewModel::OnOpenOrFocus(const fs::path& path)
{
	m_Model->OpenOrFocus(path);
}


void EditorViewModel::OnWantCloseFile(const u32 ind)
{
	m_View->CloseFile(ind);
}

void EditorViewModel::OnWantFileChange(const Document& doc)
{
	m_Model->ChangeFile(doc.Id);
}

void EditorViewModel::OnFileClosed(std::vector<u32> inds, bool save)
{
	bool res = m_Model->Close(inds, save);
	if (res)
		m_View->PerformedClose();
}

void EditorViewModel::OnCancelClose()
{
}

void EditorViewModel::OnPerformRename(const idt id, const std::string& name)
{
}

void EditorViewModel::OnEdit(Document* doc, const char change)
{
	m_Model->Edited(doc, change);
}

void EditorViewModel::OnCursorMoved(Document* doc, const i32 pos)
{
	m_Model->MoveCursor(doc, pos);
}
