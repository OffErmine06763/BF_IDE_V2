#include "EditorViewModel.h"
#include "Views/EditorView.h"

EditorViewModel::EditorViewModel(EditorView* view, EditorModel* model)
	: m_View(view), m_Model(model)
{
	LOG_GRAPHICS("EditorViewModel Created\n");
}
EditorViewModel::~EditorViewModel()
{
	LOG_GRAPHICS("EditorViewModel Destroyed\n");
}

void EditorViewModel::OnCloseAll()
{
	m_View->CloseAll();
}

void EditorViewModel::OnPerformSave(Document& doc)
{
	m_Model->PerformSave(doc);
}

void EditorViewModel::OnOpenOrFocus(const fs::path& path)
{
	m_Model->OpenOrFocus(path);
}


void EditorViewModel::OnWantCloseFile(const u32 id)
{
	m_View->CloseFile(id);
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
	m_View->AbortClose();
}

void EditorViewModel::OnPerformRename(const idt id, const std::string& name)
{
	m_Model->PerformRename(id, name);
}

void EditorViewModel::OnEdit(Document* doc, const char change)
{
	m_Model->Edited(doc, change);
}

void EditorViewModel::OnCursorMoved(Document* doc, const i32 pos)
{
	m_Model->MoveCursor(doc, pos);
}

