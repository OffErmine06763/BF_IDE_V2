#include "Utility.h"

#include <fstream>


// ################################################################## PATH ##################################################################
constexpr PathType GetPathType(const fs::path& path)
{
	if (path.extension().string() == ".bf")
		return PathType::FILE;
	if (path.extension().string() == ".bfproj")
		return PathType::PROJECT;
	if (fs::is_directory(path))
		return PathType::FOLDER;
	return PathType::NONE;
}


WorkingDirectory::WorkingDirectory(const fs::path& path)
	: Path(path), DirType(GetPathType(path))
{
}
// ################################################################## PATH ##################################################################




// ################################################################## HISTORY ##################################################################
History::History(const fs::path& path)
	: SaveFile(path)
{
	InitFromFile();
}
History::~History()
{
	if (Dirty)
		SaveToFile();
}


// TODO: YAML?
void History::InitFromFile()
{
	std::ifstream file(SaveFile);
	if (!file.is_open())
		return;

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
void History::InitFromFile(const fs::path& path)
{
	SaveFile = path;
	InitFromFile();
}
void History::SaveToFile()
{
	std::ofstream file(SaveFile);

	for (auto& [path, fav] : Files)
		file << path.string() << ' ' << fav;

	file.close();
	Dirty = false;
}
void History::SaveToFile(const fs::path& path)
{
	SaveFile = path;
	SaveToFile();
}

void History::Add(const fs::path& path, const bool fav)
{
	Dirty = true;
	Files.push_back({ path, fav });
}

void History::SetAsMostRecent(const fs::path& path)
{
	auto res = stdr::find_if(Files, [&path](const Entry& f) { return f.Path.compare(path) == 0; });
	if (res == Files.end())
		Add(path);
	else 
	{
		Dirty = true;
		bool fav = res->Fav;
		Files.erase(res);
		Files.push_back({ path, fav });
	}
}
void History::SetAsMostRecent(const size_t index)
{
	if (index >= Files.size())
		return;

	Dirty = true;
	Entry el = Files[index];
	Files.erase(Files.begin() + index);
	Files.push_back(el);
}

void History::SetFavourite(const size_t index, const bool fav)
{
	if (index < Files.size() && Files[index].Fav != fav)
	{
		Dirty = true;
		Files[index].Fav = fav;
	}
}
void History::SetFavourite(const fs::path& path, const bool fav)
{
	auto res = stdr::find_if(Files, [&path](const Entry& f) { return f.Path.compare(path) == 0; });
	if (res != Files.end() && (*res).Fav != fav)
	{
		Dirty = true;
		(*res).Fav = fav;
	}
}

void History::Remove(const fs::path& path)
{
	auto where = stdr::find_if(Files, [&path](const Entry& f) { return f.Path.compare(path) == 0; });
	if (where != Files.end())
	{
		Dirty = true;
		Files.erase(where);
	}
}
void History::Remove(const size_t index)
{
	if (index < Files.size())
	{
		Dirty = true;
		Files.erase(Files.begin() + index);
	}
}
// ################################################################## HISTORY ##################################################################