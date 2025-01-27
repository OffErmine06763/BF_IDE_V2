#pragma once
#include <inttypes.h>
#include <filesystem>
#include <vector>


struct WorkingDirectory
{
	enum Type : uint16_t
	{
		NONE = 0, FILE, FOLDER, PROJECT
	};
	static constexpr Type GetType(const std::filesystem::path& path);

	WorkingDirectory(const std::filesystem::path& path);

	const std::filesystem::path& Path;
	const Type DirType;
};

// TODO: add callbacks for changes?
struct Hystory
{
	Hystory(const std::filesystem::path& path);
	~Hystory();


	void InitFromFile(const std::filesystem::path& path);
	void SaveToFile();
	void SaveToFile(const std::filesystem::path& path);
	void Add(const std::filesystem::path& path, const bool fav = false);
	void SetFavourite(const size_t index, const bool fav = true);
	void SetFavourite(const std::filesystem::path& path, const bool fav = true);
	void Remove(const std::filesystem::path& path);
	void Remove(const size_t index);


	std::vector<std::pair<std::filesystem::path, bool>> Files;

	std::filesystem::path SaveFile;
	bool Dirty = false;
};