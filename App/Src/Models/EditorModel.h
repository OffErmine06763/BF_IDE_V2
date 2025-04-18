#pragma once
#include "Utility.h"

struct Document
{
	using idt = u32;
	static constexpr idt InvalidID = 0;

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


class EditorModel
{
public:
	using idt = Document::idt;
	static constexpr i32 InvalidIndex = -1;
	static constexpr u32 RecentOpenSize = 10, RecentCloseSize = 10;

public:
	EditorModel(const fs::path& workdir, const consumer<const Document&>& onFocus);

	bool Close(std::vector<u32> inds, bool save);
	void OpenOrFocus(const fs::path& path);

	const std::vector<fs::path>& GetRecentOpen()  const { return m_RecOpen; }
	const std::vector<fs::path>& GetRecentClose() const { return m_RecClose; }
	const stdr::ref_view<std::vector<Document>> GetDocuments() { return m_Documents; }

	void PerformSave(Document& doc) const;
	void PerformRename(Document& doc, const std::string& name);

	bool ChangeFile(const idt id);
	void FileChanged(const Document& doc);

	void Lock(bool lock) { m_Locked = lock; }

	void MoveCursor(Document* doc, const i32 pos);

	void Edited(Document* doc, const char change);

	void SetOnFileChangedCallback(const consumer<fs::path>& cb) { m_OnFileChanged = cb; }

private:
	std::vector<Document> m_Documents;
	std::vector<fs::path> m_RecOpen, m_RecClose;

	bool m_Locked = false;
	i32 m_FocusInd = InvalidIndex;

	consumer<fs::path> m_OnFileChanged;
	consumer<const Document&> m_OnFileFocus;
};