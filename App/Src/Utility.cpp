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
	return PathType::UNKNOWN;
}
std::ostream& operator<<(std::ostream& out, const PathType& t)
{
	return out << static_cast<std::underlying_type_t<PathType>>(t);
}

WorkingDirectory::WorkingDirectory(const fs::path& path)
	: Path(path), PathType(GetPathType(path))
{
}

std::ostream& operator<<(std::ostream& out, const WorkingDirectory& d)
{
	return out << d.Path << ' ' << d.PathType;
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
	while (!(file >> buf).eof())
	{
		file >> fav;
		fs::path newpath(buf);
		Entries.push_back({ newpath, fav });
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

	for (auto& [path, fav] : Entries)
		file << path.string() << ' ' << fav << '\n';

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
	Entries.push_back({ path, fav });
}

void History::UpdatePath(const fs::path& oldpath, const fs::path& newpath)
{
	auto res = stdr::find_if(Entries, [&oldpath](const Entry& f) { return f.Path.compare(oldpath) == 0; });
	if (res != Entries.end())
	{
		Dirty = true;
		res->Path = newpath;
	}
}

void History::SetAsMostRecent(const fs::path& path)
{
	auto res = stdr::find_if(Entries, [&path](const Entry& f) { return f.Path.compare(path) == 0; });
	if (res == Entries.end())
		Add(path);
	else 
	{
		Dirty = true;
		bool fav = res->Fav;
		Entries.erase(res);
		Entries.push_back({ path, fav });
	}
}
void History::SetAsMostRecent(const size_t index)
{
	if (index >= Entries.size())
		return;

	Dirty = true;
	Entry el = Entries[index];
	Entries.erase(Entries.begin() + index);
	Entries.push_back(el);
}

void History::SetFavourite(const size_t index, const bool fav)
{
	if (index < Entries.size() && Entries[index].Fav != fav)
	{
		Dirty = true;
		Entries[index].Fav = fav;
	}
}
void History::SetFavourite(const fs::path& path, const bool fav)
{
	auto res = stdr::find_if(Entries, [&path](const Entry& f) { return f.Path.compare(path) == 0; });
	if (res != Entries.end() && (*res).Fav != fav)
	{
		Dirty = true;
		(*res).Fav = fav;
	}
}

void History::Remove(const fs::path& path)
{
	auto where = stdr::find_if(Entries, [&path](const Entry& f) { return f.Path.compare(path) == 0; });
	if (where != Entries.end())
	{
		Dirty = true;
		Entries.erase(where);
	}
}
void History::Remove(const size_t index)
{
	if (index < Entries.size())
	{
		Dirty = true;
		Entries.erase(Entries.begin() + index);
	}
}
// ################################################################## HISTORY ##################################################################

// ################################################################## BF ##################################################################
bool IsValidBF(const char c)
{
	return c == BF_INC || c == BF_DEC ||
		   c == BF_OPN || c == BF_CLS ||
		   c == BF_MVR || c == BF_MVL ||
		   c == BF_OUT || c == BF_INP;
}
// ################################################################## BF ##################################################################