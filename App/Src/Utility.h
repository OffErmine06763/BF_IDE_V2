#pragma once
#include <inttypes.h>
#include <filesystem>
#include <vector>

enum class PathType : uint16_t
{
	NONE = 0, FILE, FOLDER, PROJECT
};
constexpr PathType GetType(const std::filesystem::path& path);

struct WorkingDirectory
{
	WorkingDirectory(const std::filesystem::path& path);

	const std::filesystem::path Path;
	const PathType DirType;
};

// TODO: add callbacks for changes?
struct History
{
public:
	struct Entry
	{
		std::filesystem::path Path;
		bool Fav;
	};

	History(const std::filesystem::path& path);
	~History();


	void InitFromFile();
	void InitFromFile(const std::filesystem::path& path);
	void SaveToFile();
	void SaveToFile(const std::filesystem::path& path);
	void Add(const std::filesystem::path& path, const bool fav = false);
	void SetAsMostRecent(const std::filesystem::path& path);
	void SetAsMostRecent(const size_t index);
	void SetFavourite(const size_t index, const bool fav = true);
	void SetFavourite(const std::filesystem::path& path, const bool fav = true);
	void Remove(const std::filesystem::path& path);
	void Remove(const size_t index);

	inline bool IsDirty() const { return Dirty; }

	inline const History::Entry& operator[](const size_t i) const { return Files[i]; }
	inline std::vector<Entry>::const_iterator begin() const { return Files.cbegin(); }
	inline std::vector<Entry>::const_iterator end() const { return Files.cend(); }

private:
	std::vector<Entry> Files;

	std::filesystem::path SaveFile;
	bool Dirty = false;
};