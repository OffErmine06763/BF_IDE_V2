#pragma once
#include "Utility.h"

// TODO: add callbacks for changes?
struct History
{
public:
	struct Entry
	{
		fs::path Path;
		bool Fav;
	};
	using storage = std::vector<Entry>;
	using it = storage::iterator;
	using rit = storage::reverse_iterator;
	using cit = storage::const_iterator;
	using crit = storage::const_reverse_iterator;

	History(const fs::path& path);
	~History();


	void InitFromFile();
	void InitFromFile(const fs::path& path);

	void SaveToFile();
	void SaveToFile(const fs::path& path);

	void Add(const fs::path& path, const bool fav = false);

	void UpdatePath(const fs::path& oldpath, const fs::path& newpath);

	void SetAsMostRecent(const fs::path& path);
	void SetAsMostRecent(const size_t index);

	void SetFavourite(const size_t index, const bool fav = true);
	void SetFavourite(const fs::path& path, const bool fav = true);

	void Remove(const fs::path& path);
	void Remove(const size_t index);

	inline bool IsDirty() const { return Dirty; }
	inline size_t Size() const { return Entries.size(); }

	inline const History::Entry& operator[](const size_t i) const { return Entries[i]; }
	inline crit begin() const { return Entries.crbegin(); }
	inline crit end() const { return Entries.crend(); }

private:
	storage Entries;

	fs::path SaveFile;
	bool Dirty = false;
};