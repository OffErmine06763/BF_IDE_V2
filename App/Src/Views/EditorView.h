#pragma once
#include "Utility.h"
#include "ViewModels/EditorViewModel.h"


struct ImGuiInputTextCallbackData;
struct ImVec2;

class EditorView
{
public:
	using idt = Document::idt;
	struct EditorCallbackUserData
	{
		Document* Doc;
		EditorViewModel* VM;
	};
	static constexpr i32 InvalidIndex = -1;

public:
	EditorView(const fs::path& workdir);
	~EditorView() = default;

	// void SetOnFileChangedCallback(const consumer<const fs::path&>& cb) { m_FileChangedCB = cb; }

	void Render(/* const ImVec2& pos, const ImVec2& size // TAG: Toolbar */);

	// idt OpenFile(const fs::path& dir);
	// idt OpenOrFocus(const fs::path& path);
	// 
	// bool Focus(const idt id);
	// 
	// bool CloseFile(const fs::path& dir);
	// bool CloseFile(const idt id);
	void CloseAll();
	void PerformClose();
	// 
	// inline void RequestRedock() { m_WantRedock = true; }
	// 
	// 
	// void Lock(bool lock);

	void SetDirty(const idt id);
	void Saved(const idt id);
	void Opened(Document doc);
	void Focused(idt id);

	EditorViewModel* GetViewModel() { return &m_VM; }

private:
	void RenderMainMenu();
	void RenderBody(/* const ImVec2& pos, const ImVec2& size // TAG: Toolbar */);
	void RenderClosingConfirmationUI();
	void RenderRenamingDocUI();
	void ProcessShortcuts();

	void DisplayDocContents(const u32 n);
	void DisplayDocContextMenu(const u32 n);

	void StartRename(const u32 ind);
	// 
	// void PerformSave(Document& doc) const;
	// void PerformRename(Document& doc, const std::string& name);
	// 
	void _Focus(const u32 ind);
	// void _CloseFile(const u32 ind);


private:
	// std::function<void(const fs::path&)> m_FileChangedCB;

	bool m_WantRedock = false;
	bool m_RenamingStarted = false;
	bool m_Locked = false;

	i32 m_FocusInd = InvalidIndex, m_WantFocus = InvalidIndex, m_RenamingDocInd = InvalidIndex;

	std::vector<u32> m_CloseQueue;
	std::vector<fs::path> m_RecOpen, m_RecClose;
	 
	std::vector<Document> m_Documents;
	// stdr::ref_view<const EditorModel::document_collection_t> m_Documents;

	EditorViewModel m_VM;
};