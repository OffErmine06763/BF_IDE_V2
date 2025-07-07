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

enum class OpType : u8
{
	INC,
	DEC,
	LEFT,
	RIGHT,
	I,
	O,
	NONE
};
OpType OpFromTType(const TType& type);

struct Operation
{
	OpType type;

	static const hmap<OpType, std::string> ToString;
};

struct Goto
{
	std::string name;
};

struct Stmt;

struct Label
{
	std::string name;
	std::vector<Stmt> body;
};

struct Loop
{
	std::vector<Stmt> body;
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