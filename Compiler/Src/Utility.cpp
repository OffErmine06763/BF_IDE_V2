#include "Utility.h"



std::ostream& print_time(std::ostream& out, const stdc::nanoseconds& time)
{
	if (time < 1000ns)
		out << time;
	else if (time < 1000us)
		out << to<stdc::microseconds>(time);
	else if (time < 1000ms)
		out << to<stdc::milliseconds>(time);
	else
		out << to<stdc::seconds>(time);
	return out;
}

std::string ReadFile(const fs::path& file)
{
	std::ifstream in(file);
	std::string content = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
	in.close();
	return content;
}

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




// ################################################################## BF ##################################################################
bool IsValidBF(const char c)
{
	return c == BF_INC || c == BF_DEC ||
		   c == BF_OPN || c == BF_CLS ||
		   c == BF_MVR || c == BF_MVL ||
		   c == BF_OUT || c == BF_INP;
}
// ################################################################## BF ##################################################################