#pragma once
#include <memory>
#include <vector>
#include <format>
#include <imgui.h>

#include "Utility.h"


struct Document
{
	std::string Name;
	fs::path Path;
	i32 Id;
	bool Dirty = false;

	Document(const fs::path& path)
		: Id(NextId++), Path(path), Name(path.filename().string())
	{ }
	void operator=(const Document& other)
	{
		Name = other.Name;
		Path = other.Path;
		Dirty = other.Dirty;
		Id = other.Id;
	}

	static i32 NextId;
};


/*
* Utility class to handle tabs of opened files:
* - Render
* - Handle Inputs
*/
class Editor
{
public:
	Editor(const fs::path& workdir);
	~Editor() = default;

	void Render();

	i32 OpenFile(const fs::path& dir);
	i32 OpenOrFocus(const fs::path& path);

	bool Focus(const i32 id);

	bool CloseFile(const fs::path& dir);
	bool CloseFile(const i32 id);
	void CloseAll();
	
	inline void RequestRedock() { m_WantRedock = true; }

	void DisplayDocContents(const i32 n);
	void DisplayDocContextMenu(const i32 n);

private:
	void SaveDoc(Document& doc);

	void _Focus(const i32 ind);

	const fs::path m_WorkDir;

	bool m_WantRedock = false;
	// bool m_CloseAllOnConfirmClose = false;
	bool m_RenamingStarted = false;

	std::vector<Document> m_Documents;
	std::vector<u32> m_CloseQueue;
	Document* m_RenamingDoc = nullptr;

	std::vector<fs::path> m_Recent;
};