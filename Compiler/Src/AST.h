#pragma once
#include "Token.h"

//struct ASTNode
//{
//	u32 type : 4;
//	u32 ind  : 28;
//};
//
//struct NodeStorage
//{
//	union {
//
//	};
//};
//
//
//struct ParseResult
//{
//	std::shared_ptr<ASTNode> root;
//	std::vector<NodeStorage> storages;
//
//	u32 AddNode(TType type, std::shared_ptr<ASTNode>& parent)
//	{
//		NodeStorage storage = storages[parent->ind];
//
//	}
//};


// TODO:
// could use a vector of the tokens storages (label string, vector of loop body...)
// and each node in the AST is again a 32bit pack with a type and index in the vector

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
	//std::string name;
};

struct Stmt;

struct Label
{
	u32 ID : 20;
	//std::vector<Stmt> body; // TODO: keep a single vector and a reference to a subrange, since all bodies are consecutive
};

struct Loop
{
	u32 count : 8 = 0;
	u32 ID : 20;
	//std::vector<Stmt> body;
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
	//StmtType type;
	std::variant<Goto, Return, Operation, Loop> value;
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

// TODO: add a preprocessor to identify includes, each of them will recursively call the tokenized and parser
struct TranslationUnit
{
	Block body;

	hmap<u32, std::string> symbolsI;
	hmap<std::string_view, u32> symbolsS;
	hmap<u32, std::vector<Stmt>> bodies;

	u32 NextID = 0;
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