#pragma once
#include "Token.h"


namespace BFC
{
	enum OpType : i8
	{
		O_INC = 1,
		O_DEC = -1,
		O_LEFT = 2,
		O_RIGHT = -2,
		O_I = 3,
		O_O = 4, // O and I do not cancel each other
		O_NONE = 5
	};
	OpType OpFromTType(const TType& type);

	struct Operation
	{
		i16 type : FIELD_TYPE;
		u16 count : FIELD_COUNT = 0;

		static const hmap<OpType, std::string> ToString;
		static const hmap<OpType, char> ToSymbol;
	};

	struct Goto
	{
		u32 ID : 20 = INVALID_ID;
	};

	struct Stmt;

	struct Label
	{
		u32 ID : FIELD_ID = INVALID_ID;
		// TODO: keep a single vector and a reference to a subrange, since all bodies are consecutive
	};

	struct Loop
	{
		u32 count : FIELD_COUNT = 0;
		u32 ID : FIELD_ID = INVALID_ID;
	};

	struct Return
	{

	};

	enum class StmtType : u8
	{
		GOTO,
		RET,
		OP,
		LOOP
	};
	struct Stmt
	{
		// TODO: all variant types use less than 4Bytes, could fit the type in the remaining bits
		std::variant<Goto, Return, Operation, Loop> value;
		//StmtType type;
		/*union
		{
			Goto label;
			Return ret;
			Operation op;
			Loop loop;
		} value;*/
	};

	struct Decl
	{
		Label label;
	};

	struct BlockItem
	{
		/*static constexpr bool STMT = false, DECL = !STMT;
		bool which = false;
		union
		{
			Stmt stmt;
			Decl decl;
		} value;*/
		std::variant<Stmt, Decl> value;
	};

	struct Block
	{
		std::vector<BlockItem> items;
	};

	struct TU
	{
		Block body;

		/// maps a symbol's ID to its name
		hmap<u32, std::string> symbolsI;
		/// maps a symbol's name to its ID
		hmap<std::string, u32> symbolsS;
		/// maps an AST node ID to the subtree rooted in such node, used for loops and labels
		hmap<u32, std::vector<Stmt>> bodies;

		u32 NextID = INVALID_ID + 1;

		/// used for label redefinition
		hset<u32> labels;
		hset<std::string> externs;
		hset<std::string> exports;
		/// used for undefined label
		hset<u32> gotos;
	};


	std::ostream& operator<<(std::ostream& out, const Loop& lo);
	std::ostream& operator<<(std::ostream& out, const Operation& o);
	std::ostream& operator<<(std::ostream& out, const Return& r);
	std::ostream& operator<<(std::ostream& out, const Goto& g);
	std::ostream& operator<<(std::ostream& out, const Stmt& s);
	std::ostream& operator<<(std::ostream& out, const Label& la);
	std::ostream& operator<<(std::ostream& out, const Decl& d);
	std::ostream& operator<<(std::ostream& out, const BlockItem& bi);
	std::ostream& operator<<(std::ostream& out, const Block& b);
	std::ostream& operator<<(std::ostream& out, const TU& tu);
}