#pragma once
#include <Utility.h>
#include <CompilerUtility.h>


namespace BFC
{
	enum TType : u8
	{
		T_INC = 0b00000000,
		T_DEC = 0b00000001,
		T_LEFT = 0b00000010,
		T_RIGHT = 0b00000011,
		T_LOOPS = 0b00000100,
		T_LOOPE = 0b00000101,
		T_I = 0b00000110,
		T_O = 0b00000111,

		T_LABEL = 0b00001000,
		T_GOTO = 0b00001001,
		T_RETURN = 0b00001010,

		T_MAX,
		/// Any token greater than this (except MAX) belongs to the extended version of the language
		T_EXT = 0b00001000,
	};
	std::ostream& operator<<(std::ostream& out, const TType& token);

	/// NOTE: consecutive labels and gotos are not compressed, as it's currently not possible to know where each is located
	///       compression for position mapped symbols happens only for those that are not on different lines or separated by ' '
	struct Token
	{
		static const hmap<TType, std::string> ToString;
		/// STANDARD bf Token Types mapped to symbol
		static const hmap<TType, char> ToSymbol;
		/// STANDARD bf symbols mapped to Token Type
		static const hmap<char, TType> ToType;

		// TODO: different Types can use different divisions of the pack,
		//  eg. + doesn't need an ID, the extra bits can be used for count
		//  however the max count = 255, so idk
		u32 type : FIELD_TYPE;
		/// Number of ADDITIONAL equivalent consecutive tokens
		u32 count : FIELD_COUNT = 0;
		// NOTE: ] has its own ID (used for position map)
		u32 ID : FIELD_ID = INVALID_ID;

		Token(const TType type, const u8 count = 0, const u32 id = 0);
		Token(const Token& other);

		inline static bool IsMapped(TType type) { return type == T_LABEL || type == T_GOTO; }
	};
	std::ostream& operator<<(std::ostream& out, const Token& token);

	struct TokenizeResult
	{
		using tokenit_t = std::vector<Token>::iterator;
		using tokencit_t = std::vector<Token>::const_iterator;
		std::vector<Token> tokens;

		// TODO: since ids are progressive 1-indexed, use array? (use another ID for loop)
		/// Mapping from token id to its name in the source code
		hmap<u32, std::string> symbolsI;
		/// Mapping from token name in the source code to its id
		hmap<std::string, u32> symbolsS;
		/// Mapping from token id to its position in the source code. Used for loops and returns.
		/// For compressed tokens, it represents the position of the first symbol in the sequence.
		hmap<u32, coord<u32>> positions;

		hset<std::string> externs, exports;


		/// Add a token, merging with the previous one if its the same
		void AddToken(const TType type);
		/// Add a token, with its position in the source code, merging with the previous one if its the same
		/// Used for T_RETURN, T_LOOPS and T_LOOPE
		void AddToken(const TType type, const u32 row, const u32 col);
		/// Add a token, with its name in the source code.
		/// Used for T_LABEL and T_GOTO
		void AddToken(const TType type/*, const u32 row, const u32 col*/, const std::string& symbol);

		void AddExtern(const std::string& symbol);
		void AddExport(const std::string& symbol);


		struct Iterator;
		struct CIterator;

		Iterator begin() { return Iterator(tokens.begin()); }
		Iterator end() { return Iterator(tokens.end()); }
		CIterator begin() const { return CIterator(tokens.cbegin()); }
		CIterator end()   const { return CIterator(tokens.cend()); }
		CIterator cbegin() const { return CIterator(tokens.cbegin()); }
		CIterator cend()   const { return CIterator(tokens.cend()); }

		struct CIterator
		{
			explicit CIterator(tokencit_t it) : it(it) {}

			/// Move to the next token, skipping the compressed ones
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

		u32 NextID = INVALID_ID + 1;
	};
	
	std::ostream& operator<<(std::ostream& out, const TokenizeResult& tokens);

	// I genuinely don't know why I have to redefine the operator overload inside this namespace, calling the global one
	template <typename T>
	std::ostream& operator<<(std::ostream& out, const coord<T>& c)
	{
		::operator<<(out, c);
		return out;
	}
}
