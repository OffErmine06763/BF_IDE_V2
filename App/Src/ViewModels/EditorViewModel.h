#pragma once
#include "Models/EditorModel.h"

class EditorView;

class EditorViewModel
{
public:
	using idt = Document::idt;

public:
	EditorViewModel(EditorView* view, const fs::path& workdir, const consumer<Document>& onOpen, const consumer<idt>& onFocus);

	EditorModel* GetModel() { return &m_Model; }

	void OnCloseAll();
	void OnPerformSave(const idt id);
	void OnCloseFile(const idt id);
	void OnOpenOrFocus(const fs::path& path);
	void OnFileChanged(const idt id);
	void OnWantCloseFile(const idt id);
	void OnWantFileChange(const idt id);
	void OnFileClosed(const std::vector<idt>& ids, bool save);
	void OnCancelClose();
	void OnPerformRename(const idt id, const std::string& name);
	void OnEdit(const idt id, const char change);

	const std::vector<fs::path>& GetRecentOpen() const { return m_Model.GetRecentOpen(); }
	const std::vector<fs::path>& GetRecentClose() const { return m_Model.GetRecentClose(); }
	const std::vector<Document>& GetDocuments() const { return m_Model.GetDocuments(); }
private:
	EditorModel m_Model;
	EditorView* m_View;
};