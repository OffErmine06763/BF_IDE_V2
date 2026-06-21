#pragma once
#include <inttypes.h>
#include <numbers>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <format>
#include <chrono>

#include <string>
#include <sstream>
#include <vector>
#include <array>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>
#include <stack>
#include <queue>

#include <optional>
#include <variant>

#include <ranges>
#include <functional>

#include <memory>
#include <condition_variable>
#include <semaphore>

#include <regex>

#ifdef _DEBUG
#include <Terminal.h>
#endif

// ################################################################## ALIASES ##################################################################
namespace stdv = std::views;
namespace stdr = std::ranges;
namespace stdc = std::chrono;
namespace fs = std::filesystem;
using namespace std::string_literals;
using namespace std::chrono_literals;

using u8  = uint8_t ;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8  =  int8_t ;
using i16 =  int16_t;
using i32 =  int32_t; 
using i64 =  int64_t;
using f32 =  float  ;
using f64 =  double ;

template <typename K, typename V>
using hmap = std::unordered_map<K, V>;
template <typename V>
using hset = std::unordered_set<V>;
	
namespace std::chrono {
	using clock = high_resolution_clock;
}

template <typename T>
using uptr = std::unique_ptr<T>;
template <typename T>
using sptr = std::shared_ptr<T>;

using callable = std::function<void(void)>;
template <typename T>
using consumer = std::function<void(T)>;
template <typename T>
using provider = std::function<T(void)>;
// ################################################################## ALIASES ##################################################################


// PAIR
template <typename T>
using coord = std::pair<T, T>;
template <typename T>
std::ostream& operator<<(std::ostream& out, const coord<T>& c) {
	return out << std::format("[{}:{}]", c.first, c.second);
}
template <typename T>
coord<T> operator+(const coord<T> a, const coord<T>& b) {
	return { a.first + b.first, a.second + b.second };
}
template <typename T>
std::string operator+(const std::string& left, const coord<T>& right) {
	return left + std::to_string(right.first) + ':' + std::to_string(right.second);
}



// CASTING
template <typename T, typename X>
inline constexpr T to(const X& x) { return static_cast<T>(x); }
template <typename T, typename X, typename _X>
inline constexpr T to(const stdc::duration<_X, X>& x) { return stdc::duration_cast<T>(x); }


// CONCEPTS
template <typename T, typename Variant>
struct variant_contains : std::false_type {};
template <typename T, typename... Types>
struct variant_contains<T, std::variant<Types...>> : std::disjunction<std::is_same<T, Types>...> {};

template <typename T, typename Variant>
concept in_variant = variant_contains<T, Variant>::value;
template <typename T, typename... Variant>
concept in_variants = (variant_contains<T, Variant>::value && ...);

template <typename T, typename U>
concept not_same_as = !std::same_as<T, U>;
template <typename T>
concept not_void = !std::is_void_v<T>;

template<typename T, typename... Args>
constexpr bool is_one_of = (std::is_same_v<T, Args> || ...);
template <auto Value, auto... Accepted>
constexpr bool v_is_one_of = ((Value == Accepted) || ...);


// BINDING
#define BIND(fn) [this]() { this->fn(); }
template <class C, typename R = void, typename I> requires not_void<I>
constexpr std::function<R(I)> bind(C* _this, R(C::* fn)(I)) {
	return [_this, fn](I data) -> R { return (_this->*fn)(data); };
}
template <class C, typename R = void>
constexpr std::function<R(void)> bind(C* _this, R(C::* fn)()) {
	return [_this, fn]() -> R { return (_this->*fn)(); };
}


// VISITOR
template <class... Ts>
struct visitor : Ts... { using Ts::operator()...; };


// VARIANT
template <typename V, typename... T>
bool inline constexpr holds(const std::variant<T...>& var) { return std::holds_alternative<V>(var); }
template <typename V, typename... T> requires in_variants<V, T...>
bool inline constexpr holds(const T&... vars) { return (... && std::holds_alternative<V>(vars)); }


// CHRONO
std::ostream& print_time(std::ostream& out, const stdc::nanoseconds& time);


// FILE
std::string ReadFile(const fs::path& file);


// COLORS
struct RGBA
{
	u8 r, g, b, a;

	RGBA() = default;
	RGBA(u8 r, u8 g, u8 b, u8 a);
	RGBA(u32 rgba);
	void operator=(u32 rgba);
	operator u32() const;
};


// ################################################################## EXPECTED ##################################################################
template <typename E, typename U>
struct expected
{
	std::variant<E, U> content;

	expected(const E& e) { content = e; }
	expected(const U& u) { content = u; }
	expected(const expected<E, U>& other) { content = other.content; }
	void operator=(const expected<E, U>& other) { content = other.content; }
	~expected() {};

	std::optional<E> getE() {
		return std::holds_alternative<E>(content) ? std::optional<E>{ std::get<E>(content) } : std::nullopt;
	}
	std::optional<U> getU() {
		return std::holds_alternative<U>(content) ? std::optional<U>{ std::get<U>(content) } : std::nullopt;
	}
	E& _getE() { return std::get<E>(content); }
	U& _getU() { return std::get<U>(content); }
	E&& _consumeE() { return std::move(std::get<E>(content)); }
	U&& _consumeU() { return std::move(std::get<U>(content)); }
	template <typename T> requires in_variant<T, std::variant<E, U>>
	std::optional<T> get() {
		return std::holds_alternative<T>(content) ? std::get<T>(content) : std::nullopt;
	}

	bool success() const { return std::holds_alternative<E>(content); }
	operator bool() const { return success(); }
};
// ################################################################## EXPECTED ##################################################################


// ################################################################## PATH ##################################################################
enum class PathType : u8
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

void ShowInExplorer(const fs::path& path);
// ################################################################## PATH ##################################################################


// ################################################################## LOGGING ##################################################################
#ifdef _DEBUG
#define LOG(x) dbg << x
#define LOG_APP(x) dbg << APP_LEVEL << x << RESET
#define LOG_COMP(x) dbg << COMP_LEVEL << x << RESET
#define LOG_GRAPHICS(x) dbg << GRAPHICS_LEVEL << x << RESET

struct Dbg { };
inline static constexpr Dbg dbg;
template <typename T>
inline const Dbg& operator<<(const Dbg& dbg, const T& data)
{
	std::cout << data;
	return dbg;
}
template <typename T> /*requires std::is_arithmetic_v<T>*/
inline const Dbg& operator<<(const Dbg& dbg, const std::vector<T>& data)
{
	for (const auto& i : data)
		std::cout << i << ' ';
	return dbg;
}
inline const Dbg& operator<<(const Dbg& dbg, std::ostream& (*manip)(std::ostream&))
{
	std::cout << manip;
	return dbg;
}

static constexpr auto
	APP_LEVEL = Terminal::TEXT_F_BGREEN,
	COMP_LEVEL = Terminal::TEXT_F_BYELLOW,
	GRAPHICS_LEVEL = Terminal::TEXT_F_BCYAN,
	RESET = Terminal::TEXT_RESET;

#else
#define LOG(x)
#define LOG_APP(x)
#define LOG_COMP(x)
#define LOG_GRAPHICS(x)

struct Dbg { };
inline static constexpr Dbg dbg;
template <typename T>
inline const Dbg& operator<<(const Dbg& dbg, const T& data)
{
	return dbg;
}
inline const Dbg& operator<<(const Dbg& dbg, std::ostream& (*manip)(std::ostream&))
{
	return dbg;
}
#endif
// ################################################################## LOGGING ##################################################################

// ################################################################## BF ##################################################################
constexpr char 
	BF_INC = '+', BF_DEC = '-', 
	BF_OPN = '[', BF_CLS = ']', 
	BF_MVR = '>', BF_MVL = '<', 
	BF_OUT = '.', BF_INP = ',';
constexpr char BF_ALL_L[] = { BF_INC, BF_DEC, BF_OPN, BF_CLS, BF_MVR, BF_MVL, BF_OUT, BF_INP };
const hset<char> BF_ALL_S = { BF_INC, BF_DEC, BF_OPN, BF_CLS, BF_MVR, BF_MVL, BF_OUT, BF_INP };
constexpr u32 BF_MEMSIZE = 1 << 15;
using bf_mem_t = u8;
bool IsValidBF(const char c);
// TODO: move this to CompilerUtility.h?

constexpr auto EmulationSleep = 1ns;
// ################################################################## BF ##################################################################