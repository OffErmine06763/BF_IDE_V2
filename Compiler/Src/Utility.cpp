#include "Utility.h"



//bool Regex::Parse(const std::string& str)
//{
//	regex.clear();
//	for (size_t i = 0; i < str.length(); i++)
//	{
//		if (str[i] == '[')
//		{
//			bool white = true, range = false;
//			if (str[i + 1] == '^')
//				white = false;
//			size_t end = str.find(']', i + 1);
//			size_t dash = str.find("-", i + 1, end - i - 1);
//			if (dash != std::string::npos)
//				range = true;
//
//			regex.push_back(std::make_unique<RegexCharFilter>(str.substr(i + 1, end - i - 1), white, range));
//			i = end;
//		}
//		else if (str[i] == '+')
//		{
//			if (regex.empty()) return false;
//			uptr<RegexFilter> rep = std::make_unique<RegexRepeatFilter>(1, -1, regex.rbegin()->release());
//			regex.rbegin()->swap(rep);
//		}
//		else if (str[i] == '*')
//		{
//			if (regex.empty()) return false;
//			uptr<RegexFilter> rep = std::make_unique<RegexRepeatFilter>(0, -1, regex.rbegin()->release());
//			regex.rbegin()->swap(rep);
//		}
//		else if (str[i] == '?')
//		{
//			if (regex.empty()) return false;
//			uptr<RegexFilter> rep = std::make_unique<RegexRepeatFilter>(0, 1, regex.rbegin()->release());
//			regex.rbegin()->swap(rep);
//		}
//		else if (std::isalnum(str[i]))
//		{
//			regex.push_back(std::make_unique<RegexCharFilter>(str.substr(i, 1), true, false));
//		}
//	}
//
//	return true;
//}
//RegexCharFilter::RegexCharFilter(const std::string& list, bool white, bool range)
//	: list(list), white(white), range(range)
//{ }
//RegexRepeatFilter::RegexRepeatFilter(u32 min, u32 max, RegexFilter* query)
//	: min(min), max(max), query(query)
//{ }
//
//
//bool Regex::Match(const std::string& test) const
//{
//	RegexTest rt{ test, 0, -1 };
//
//	for (const auto& step : regex)
//		if (!step->_Match(rt))
//			return false;
//	return true;
//}
//bool RegexCharFilter::_Match(RegexTest& test) const
//{
//	auto& [str, start, count] = test;
//	if (count == 0)
//		return false;
//
//	if (white)
//	{
//		if (range)
//		{
//			if (str[start] >= list[0] && str[start] <= list[1])
//			{
//				start++;
//				count--;
//				return true;
//			}
//		}
//		else
//		{
//			for (const char c : list)
//			{
//				if (str[start] == c)
//				{
//					start++;
//					count--;
//					return true;
//				}
//			}
//		}
//
//		return false;
//	}
//	else
//	{
//		if (range)
//		{
//			if (str[start] >= list[0] && str[start] <= list[1])
//				return false;
//		}
//		else
//		{
//			for (const char c : list)
//				if (str[start] == c)
//					return false;
//		}
//
//		start++;
//		count--;
//		return true;
//	}
//
//	return false;
//}
//bool RegexRepeatFilter::_Match(RegexTest& test) const
//{
//	// test the presence of the minimum number of matches
//	for (size_t i = 0; i < min; i++)
//		if (!query->_Match(test))
//			return false;
//
//	// any fail between min and max is still a success, but we have to consume the input
//	for (size_t i = min; i < max && test.start < test.str.length(); i++)
//		if (!query->_Match(test))
//			return true;
//
//	// any match past the max is a fail
//	return !query->_Match(test);
//}



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