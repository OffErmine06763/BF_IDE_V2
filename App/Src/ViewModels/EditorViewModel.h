#pragma once
#include "Models/EditorModel.h"

class EditorView;

class EditorViewModel
{
public:
	using idt = Document::idt;

public:
	EditorViewModel(EditorView* view, EditorModel* model);

	EditorModel* GetModel() { return m_Model; }

	void OnCloseAll();
	void OnPerformSave(Document& id);
	void OnCloseFile(const idt id);
	void OnOpenOrFocus(const fs::path& path);
	void OnWantCloseFile(const u32 ind);
	void OnWantFileChange(const Document& doc);
	void OnFileClosed(std::vector<u32> inds, bool save);
	void OnCancelClose();
	void OnPerformRename(const idt id, const std::string& name);
	void OnEdit(Document* doc, const char change);
	void OnCursorMoved(Document* doc, const i32 pos);

	const std::vector<fs::path>& GetRecentOpen()  const { return m_Model->GetRecentOpen(); }
	const std::vector<fs::path>& GetRecentClose() const { return m_Model->GetRecentClose(); }
	std::vector<Document>& GetDocuments() { return m_Model->GetDocuments(); }

	listener_id SubscribeFocus(consumer<const Document&> cb) { return m_Model->SubscribeFocus(cb); }

private:
	EditorModel* m_Model;
	EditorView* m_View;
};