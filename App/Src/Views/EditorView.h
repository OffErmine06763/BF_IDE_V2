#pragma once
#include "Utility.h"
#include "Models/EditorModel.h"
#include "ViewModels/EditorViewModel.h"

typedef unsigned int ImGuiID;

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
	static constexpr idt InvalidID = Document::InvalidID;

public:
	EditorView(EditorModel* model);
	~EditorView() = default;

	void Render(ImGuiID dockspace_id /*, const ImVec2& pos, const ImVec2& size // TAG: Toolbar */);

	// idt OpenFile(const fs::path& dir);
	// idt OpenOrFocus(const fs::path& path);
	// 
	// bool Focus(const idt id);
	// 
	// bool CloseFile(const fs::path& dir);
	// bool CloseFile(const idt id);
	void CloseAll();
	void CloseFile(const u32 ind);
	void PerformedClose();
	void AbortClose();
	// 
	// inline void RequestRedock() { m_WantRedock = true; }
	// 
	// 
	// void Lock(bool lock);

	void Focused(const Document& doc);

	EditorViewModel* GetViewModel() { return &m_VM; }

private:
	void RenderMainMenu();
	void RenderBody(ImGuiID dockspace_id /* const ImVec2& pos, const ImVec2& size // TAG: Toolbar */);
	void RenderClosingConfirmationUI();
	void RenderRenamingDocUI();
	void ProcessShortcuts();

	void DisplayDocContents(const u32 n);
	void DisplayDocContextMenu(const u32 n);

	void StartRename(const u32 ind);


private:
	EditorViewModel m_VM;


	bool m_WantRedock = false;
	bool m_RenamingStarted = false;
	bool m_Locked = false;

	i32 m_FocusInd = InvalidIndex, m_RenamingDocInd = InvalidIndex;
	idt m_WantFocus = InvalidID;

	std::vector<u32> m_CloseQueueIds;

	const std::vector<fs::path>& m_RecOpen, &m_RecClose;
	std::vector<Document>& m_Documents;
};