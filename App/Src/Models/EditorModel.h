#pragma once
#include "Utility.h"

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


class EditorModel
{
public:
	using idt = Document::idt;
	using document_collection_t = std::vector<Document>;
	static constexpr i32 InvalidIndex = -1;
	static constexpr u32 RecentOpenSize = 10, RecentCloseSize = 10;

public:
	EditorModel(const fs::path& workdir, const consumer<Document>& onOpen, const consumer<idt>& onFocus);

	bool Close(const std::vector<idt>& ids, bool save);
	void OpenOrFocus(const fs::path& path);

	const std::vector<fs::path>& GetRecentOpen() const { return m_RecOpen; }
	const std::vector<fs::path>& GetRecentClose() const { return m_RecClose; }
	const std::vector<Document>& GetDocuments() const { return m_Documents; }

	void PerformSave(Document& doc) const;
	bool PerformSave(idt id);
	void PerformRename(Document& doc, const std::string& name);

	bool ChangeFile(const idt id);

	void Lock(bool lock) { m_Locked = lock; }

private:
	std::vector<Document> m_Documents;
	std::vector<fs::path> m_RecOpen, m_RecClose;

	bool m_Locked = false;
	i32 m_FocusInd = InvalidIndex;

	consumer<Document> m_OnFileOpen;
	consumer<idt> m_OnFileFocus;
};