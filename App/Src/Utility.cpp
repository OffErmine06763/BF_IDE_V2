#include "Utility.h"

#include <fstream>

namespace fs = std::filesystem;


constexpr WorkingDirectory::Type WorkingDirectory::GetType(const fs::path& path)
{
	if (path.extension().string() == ".bf")
		return FILE;
	if (path.extension().string() == ".bfproj")
		return PROJECT;
	if (fs::is_directory(path))
		return FOLDER;
	return NONE;
}

WorkingDirectory::WorkingDirectory(const fs::path& path)
	: Path(path), DirType(GetType(path))
{
}


Hystory::Hystory(const fs::path& path)
	: SaveFile(path)
{ }
Hystory::~Hystory()
{
	if (Dirty)
		SaveToFile();
}


// TODO: YAML?
void Hystory::InitFromFile(const fs::path& path)
{
	std::ifstream file(path);

	std::string buf;
	bool fav;
	while (!file.eof())
	{
		file >> buf >> fav;
		fs::path newpath(buf);
		Files.push_back({ newpath, fav });
	}

	file.close();
}
void Hystory::SaveToFile()
{
	std::ofstream file(SaveFile);

	for (auto& [path, fav] : Files)
		file << path << ' ' << fav;

	file.close();
	Dirty = false;
}
void Hystory::SaveToFile(const fs::path& path)
{
	SaveFile = path;
	SaveToFile();
}

void Hystory::Add(const fs::path& path, const bool fav)
{
	Dirty = true;
	Files.push_back({ path, fav });
}

void Hystory::SetFavourite(const size_t index, const bool fav)
{
	if (index < Files.size() && Files[index].second != fav)
	{
		Dirty = true;
		Files[index].second = fav;
	}
}
void Hystory::SetFavourite(const fs::path& path, const bool fav)
{
	auto res = std::ranges::find_if(Files, [&path](const std::pair<fs::path, bool>& f) { return f.first.compare(path) == 0; });
	if (res != Files.end() && (*res).second != fav)
	{
		Dirty = true;
		(*res).second = fav;
	}
}

void Hystory::Remove(const fs::path& path)
{
	auto where = std::ranges::find_if(Files, [&path](const std::pair<fs::path, bool>& f) { return f.first.compare(path) == 0; });
	if (where != Files.end())
	{
		Dirty = true;
		Files.erase(where);
	}
}
void Hystory::Remove(const size_t index)
{
	if (index < Files.size())
	{
		Dirty = true;
		Files.erase(Files.begin() + index);
	}
}