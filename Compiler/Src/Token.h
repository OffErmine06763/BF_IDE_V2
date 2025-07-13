#pragma once
#include <Utility.h>


// NOTE: consecutive labels and gotos are not compressed, as it's currently not possible to know where each is located
enum TType : u8
{
	T_INC   = 0b00000000,
	T_DEC   = 0b00000001,
	T_LEFT  = 0b00000010,
	T_RIGHT = 0b00000011,
	T_LOOPS = 0b00000100,
	T_LOOPE = 0b00000101,
	T_I     = 0b00000110,
	T_O     = 0b00000111,

	T_LABEL   = 0b00001000,
	T_GOTO    = 0b00001001,
	T_INCLUDE = 0b00001010,
	T_RETURN  = 0b00001011,

	T_EXT  = 0b00001000,
	T_NONE = 0b00001111,
	T_MAX  = T_NONE,
	T_MASK = T_NONE,
};
std::ostream& operator<<(std::ostream& out, const TType& token);

struct Token
{
	static const hmap<TType, std::string> ToString;
	static const hmap<TType, char> ToSymbol;
	static const hmap<char, TType> ToType;
	static const u8 MAX_COUNT = (1 << 8) - 1;

	// TODO: different Types can use different divisions of the pack,
	// eg. + doesn't need an ID, the extra bits can be used for count
	// however the max count = 255, so idk
	u32 type : 4;
	u32 count : 8;
	u32 ID : 20;

	Token(const TType type = T_NONE, const u8 count = 0, const u32 id = 0);
	Token(const Token& other);

	inline static bool IsMapped(TType type) { return type & T_EXT; }
};
std::ostream& operator<<(std::ostream& out, const Token& token);

struct TokenizeResult
{
	using tokenit_t = std::vector<Token>::iterator;
	using tokencit_t = std::vector<Token>::const_iterator;
	std::vector<Token> tokens;

	// TODO: since ids are progressive 1-indexed, use array? (use another ID for loop)
	hmap<u32, std::string> symbolsI;
	hmap<std::string, u32> symbolsS;
	hmap<u32, coord<u32>> loop;


	void AddToken(const TType type);
	void AddToken(const TType type, const u32 row, const u32 col);
	void AddToken(const TType type/*, const u32 row, const u32 col*/, const std::string& symbol);

	struct Iterator;
	struct CIterator;

	Iterator begin() { return Iterator(tokens.begin()); }
	Iterator end() { return Iterator(tokens.end()); }
	CIterator begin() const  { return CIterator(tokens.cbegin()); }
	CIterator end()   const  { return CIterator(tokens.cend());   }
	CIterator cbegin() const { return CIterator(tokens.cbegin()); }
	CIterator cend()   const { return CIterator(tokens.cend());   }

	struct CIterator
	{
		explicit CIterator(tokencit_t it) : it(it) {}

		CIterator& ForceNext() { count = 0; it++; return *this; }

		std::pair<const Token&, u8> operator*() const { return { *it, count }; }
		CIterator& operator++();
		CIterator& operator++(int);
		CIterator& operator--();
		CIterator& operator--(int);
		bool operator!=(const CIterator& other) const;
		bool operator==(const CIterator& other) const;

		tokencit_t it;
		u8 count = 0;
	};
	struct Iterator : public CIterator
	{
		explicit Iterator(tokenit_t it) : CIterator(it) {}

		std::pair<Token&, u8> operator*() const { return { const_cast<Token&>(*it), count }; }
		Iterator& operator++() { CIterator::operator++(); return *this; }
		Iterator& operator--() { CIterator::operator--(); return *this; };
	};

	u32 NextID = 1;
};
std::ostream& operator<<(std::ostream& out, const TokenizeResult& tokens);