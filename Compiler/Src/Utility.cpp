#include "Utility.h"


RGBA::RGBA(u8 r, u8 g, u8 b, u8 a)
	: r(r), g(g), b(b), a(a)
{ }
RGBA::RGBA(u32 rgba)
{
	a = rgba >> 24;
	b = (rgba >> 16) & 0xFF;
	g = (rgba >> 8) & 0xFF;
	r = (rgba) & 0xFF;
}
void RGBA::operator=(u32 rgba)
{
	a =  rgba >> 24;
	b = (rgba >> 16) & 0xFF;
	g = (rgba >> 8)  & 0xFF;
	r = (rgba)       & 0xFF;
}
RGBA::operator u32() const
{
	return a << 24 | b << 16 | g << 8 | r;
}


std::ostream& print_time(std::ostream& out, const stdc::nanoseconds& time)
{
	if (time < 1us)
		out << time;
	else if (time < 1ms)
		out << to<stdc::microseconds>(time);
	else if (time < 1s)
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

void ShowInExplorer(const fs::path& path)
{
#if defined(_WIN32)
	if (std::filesystem::is_directory(path))
		std::system(("explorer \""s + path.string() + "\"").c_str());
	else {
		std::system(("explorer /select,\""s + path.string() + "\"").c_str());
	}
#elif defined(__APPLE__)
	std::system((std::string("open \"") + path.string() + "\"").c_str());
#elif defined(__linux__)
	std::system((std::string("xdg-open \"") + path.string() + "\"").c_str());
#else
	std::cerr << "Unsupported platform.\n";
#endif
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