#pragma once
#include <memory>
#include <vector>
#include <format>
#include <imgui.h>

struct MyDocument
{
	char        Name[32];   // Document title
	int         UID;        // Unique ID (necessary as we can change title)
	bool        Open;       // Set when open (we keep an array of all available documents to simplify demo code!)
	bool        OpenPrev;   // Copy of Open from last update.
	bool        Dirty;      // Set when the document has been modified
	ImVec4      Color;      // An arbitrary variable associated to the document

	MyDocument(int uid, const char* name, bool open = true, const ImVec4& color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
	{
		UID = uid;
		snprintf(Name, sizeof(Name), "%s", name);
		Open = OpenPrev = open;
		Dirty = false;
		Color = color;
	}
	void DoOpen() { Open = true; }
	void DoForceClose() { Open = false; Dirty = false; }
	void DoSave() { Dirty = false; }
};

class Editor
{
public:
	Editor();
	~Editor() = default;

	void Render();

	inline void RequestRedock() { m_WantRedock = true; }

	void GetTabName(MyDocument* doc, char* out_buf, size_t out_buf_size);
	void DisplayDocContents(MyDocument* doc);
	void DisplayDocContextMenu(MyDocument* doc);
	void NotifyOfDocumentsClosedElsewhere();

private:
	bool m_WantRedock = false;

	std::vector<MyDocument> Documents;
	std::vector<MyDocument*> CloseQueue;
	MyDocument* RenamingDoc = NULL;
	bool RenamingStarted = false;
};