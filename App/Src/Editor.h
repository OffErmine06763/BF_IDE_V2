#pragma once
#include "Utility.h"


struct ImGuiInputTextCallbackData;
class WorkingState;


struct Document
{
	using idt = u32;

	std::string Name;
	fs::path Path;
	idt Id;
	bool Dirty = false;
	std::string Content;
	u32 CursorPos = 0;

	Document(const fs::path& path);
	void operator=(const Document& other)
	{
		Name = other.Name;
		Path = other.Path;
		Dirty = other.Dirty;
		Id = other.Id;
		Content = other.Content;
	}

	static idt NextId;
};

std::ostream& operator<<(std::ostream& out, const Document& doc);



/*
* Utility class to handle tabs of opened files:
* - Render
* - Handle Inputs
*/
class Editor
{
public:
	using idt = Document::idt;
	struct EditorCallbackUserData
	{
		Document* Doc;
	};
	static constexpr i32 InvalidIndex = -1;
	static constexpr u32 RecentOpenSize = 10, RecentCloseSize = 10;

public:
	Editor(WorkingState* state);
	~Editor() = default;

	void Render();

	idt OpenFile(const fs::path& dir);
	idt OpenOrFocus(const fs::path& path);

	bool Focus(const idt id);

	bool CloseFile(const fs::path& dir);
	bool CloseFile(const idt id);
	void CloseAll();
	
	inline void RequestRedock() { m_WantRedock = true; }

	void DisplayDocContents(const u32 n);
	void DisplayDocContextMenu(const u32 n);


private:
	void RenderMainMenu();
	void RenderBody();
	void RenderClosingConfirmationUI();
	void RenderRenamingDocUI();
	void ProcessShortcuts();

	void WantRename(const u32 ind);

	void PerformSave(Document& doc);
	void PerformClose(bool save);
	void PerformRename(Document& doc, const std::string& name);

	void _Focus(const u32 ind);
	void _CloseFile(const u32 ind);


private:
	WorkingState* m_State;

	bool m_WantRedock = false;
	bool m_RenamingStarted = false;

	i32 m_FocusInd = InvalidIndex, m_WantFocus = InvalidIndex, m_RenamingDocInd = InvalidIndex;

	std::vector<Document> m_Documents;
	std::vector<u32> m_CloseQueue;
	std::vector<fs::path> m_RecOpen, m_RecClose;
};