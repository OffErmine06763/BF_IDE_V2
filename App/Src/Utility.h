#pragma once
#include <inttypes.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include <ranges>
#include <memory>
#include <functional>
#include <condition_variable>
#include <variant>


// ################################################################## TYPES ##################################################################
namespace stdv = std::views;
namespace stdr = std::ranges;
namespace fs = std::filesystem;
using namespace std::string_literals;
using namespace std::chrono_literals;

using u8  = uint8_t ;
using u16 = uint32_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8  =  int8_t ;
using i16 =  int32_t;
using i32 =  int32_t; 
using i64 =  int64_t;
using f32 =  float  ;
using f64 =  double ;

// PROPERTIES
using prop_id = u8;
using listener_id = u32;

// FUNCTIONS
template <typename T>
using uptr = std::unique_ptr<T>;
template <typename T>
using sptr = std::shared_ptr<T>;
using callable = std::function<void(void)>;
template <typename T>
using consumer = std::function<void(T)>;
template <typename T>
using provider = std::function<T(void)>;

template <typename T, typename X>
inline constexpr T to(const X& x) { return static_cast<T>(x); }


// CONCEPTS
template <typename T, typename Variant>
struct variant_contains : std::false_type {};
template <typename T, typename... Types>
struct variant_contains<T, std::variant<Types...>> : std::disjunction<std::is_same<T, Types>...> {};

template <typename T, typename Variant>
concept InVariant = variant_contains<T, Variant>::value;

template <typename T, typename U>
concept not_same_as = !std::same_as<T, U>;
template <typename T>
concept not_void = !std::is_void_v<T>;
// ################################################################## TYPES ##################################################################


// ################################################################## PATH ##################################################################
enum class PathType : uint16_t
{
	UNKNOWN = 0, FILE, FOLDER, PROJECT
};
constexpr PathType GetPathType(const fs::path& path);
std::ostream& operator<<(std::ostream& out, const PathType& t);

struct WorkingDirectory
{
	WorkingDirectory(const fs::path& path);

	const fs::path Path;
	const PathType PathType;
};
std::ostream& operator<<(std::ostream& out, const WorkingDirectory& d);
// ################################################################## PATH ##################################################################


// ################################################################## HISTORY ##################################################################
// TODO: add callbacks for changes?
// TODO: move out of Utility.h
struct History
{
public:
	struct Entry
	{
		fs::path Path;
		bool Fav;
	};

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
	inline std::vector<Entry>::const_reverse_iterator begin() const { return Entries.crbegin(); }
	inline std::vector<Entry>::const_reverse_iterator end() const { return Entries.crend(); }

private:
	std::vector<Entry> Entries;

	fs::path SaveFile;
	bool Dirty = false;
};
// ################################################################## HISTORY ##################################################################

// ################################################################## DEBUG ##################################################################
#ifdef _DEBUG
struct Dbg { };
static constexpr Dbg dbg;
template <typename T>
const Dbg& operator<<(const Dbg& dbg, const T& data)
{
	std::cout << data;
	return dbg;
}
#else
struct Dbg { };
static constexpr Dbg dbg;
template <typename T>
const Dbg& operator<<(const Dbg& dbg, const T& data)
{
	return dbg;
}
#endif
// ################################################################## DEBUG ##################################################################

// ################################################################## BF ##################################################################
constexpr char 
	BF_INC = '+', BF_DEC = '-', 
	BF_OPN = '[', BF_CLS = ']', 
	BF_MVR = '>', BF_MVL = '<', 
	BF_OUT = '.', BF_INP = ',';
constexpr char BF_ALL[] = { BF_INC, BF_DEC, BF_OPN, BF_CLS, BF_MVR, BF_MVL, BF_OUT, BF_INP };
constexpr uint32_t BF_MEMSIZE = 256;
using bf_mem_t = uint8_t;
bool IsValidBF(const char c);

constexpr auto EmulationSleep = 1ns;
// ################################################################## BF ##################################################################