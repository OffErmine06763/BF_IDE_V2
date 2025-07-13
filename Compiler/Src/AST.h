#pragma once
#include "Token.h"


enum OpType : u8
{
	O_INC,
	O_DEC,
	O_LEFT,
	O_RIGHT,
	O_I,
	O_O,
	O_NONE
};
OpType OpFromTType(const TType& type);

struct Operation
{
	u16 type : 4;
	u16 count : 8 = 0;

	static const hmap<OpType, std::string> ToString;
	static const hmap<OpType, char> ToSymbol;
};

struct Goto
{
	u32 count : 8 = 0;
	u32 ID : 20;
};

struct Stmt;

struct Label
{
	u32 ID : 20;
	// TODO: keep a single vector and a reference to a subrange, since all bodies are consecutive
};

struct Loop
{
	u32 count : 8 = 0;
	u32 ID : 20;
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

// TODO: add a preprocessor to identify includes, each of them will recursively call the tokenizer and parser
struct TranslationUnit
{
	Block body;

	/// maps a symbol's ID to its name
	hmap<u32, std::string> symbolsI;
	/// maps a symbol's name to its ID
	hmap<std::string, u32> symbolsS;
	/// maps an AST node ID to the subtree rooted in such node
	hmap<u32, std::vector<Stmt>> bodies;

	u32 NextID = 0;

	/// used for label redefinition
	hset<u32> labels; // TODO: better ways to do this?
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
std::ostream& operator<<(std::ostream& out, const TranslationUnit& tu);