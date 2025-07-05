#pragma once
#include <Utility.h>

// TODO: optimize, no more than a few bytes per token
enum TType : u8
{
	BF = 0b10000000,
	INC = BF | 0b000001,
	DEC = BF | 0b000010,
	LEFT = BF | 0b000011,
	RIGHT = BF | 0b000100,
	LOOPS = BF | 0b000101,
	LOOPE = BF | 0b000110,
	I = BF | 0b000111,
	O = BF | 0b001000,

	EXT = 0b01000000,
	LABEL = EXT | 0b000001,
	GOTO = EXT | 0b000001,
	INCLUDE = EXT | 0b000010,
};

struct Token
{	
	static const std::unordered_map<TType, std::string> ToString;
	static const std::unordered_map<char, TType> ToType;

	TType Type = BF;
	std::string Value;
	size_t StartIndex;

	Token(const TType type, const size_t ind, const std::string& value);
	Token(const TType type, const size_t ind, const char value);
	Token(const Token& other);

	static Token OfUnchecked(const char c, const size_t i) { return { ToType.at(c), i, c }; }
};
std::ostream& operator<<(std::ostream& out, const Token& token);



struct ASTNode {
	TType type;

	ASTNode(const TType type) : type(type) {}
	virtual ~ASTNode() = default;
};

struct Loop : public ASTNode {
	std::vector<std::shared_ptr<ASTNode>> body;
	Loop() : ASTNode(TType::LOOPS) {}
};

class Label : public ASTNode {
public:
	std::string name;
	Label(std::string name) : ASTNode(TType::LABEL), name(name) {}
};

class Goto : public ASTNode {
public:
	std::string label;
	Goto(const std::string& label) : ASTNode(TType::GOTO), label(label) {}
};

class Program : public ASTNode {
public:
	std::vector<std::shared_ptr<ASTNode>> statements;
	Program() : ASTNode(TType::BF) {}
};


class Compiler
{
public:

	// Lexical Analyzer / Scanner / Lexer
	static expected<std::vector<Token>, std::string> Tokenize(const fs::path& file) {
		std::ifstream in(file);
		std::string content = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
		in.close();
		return Tokenize(content);
	}
	static expected<std::vector<Token>, std::string> Tokenize(const std::string& content);

	static expected<std::shared_ptr<Program>, std::string> Parser(const std::vector<Token>& tokens)
	{
		auto program = std::make_shared<Program>();
		std::stack<std::vector<std::shared_ptr<ASTNode>>*> loopStack;
		std::vector<std::shared_ptr<ASTNode>>* currentBlock = &program->statements;

		for (size_t i = 0; i < tokens.size(); ++i) {
			const Token ch = tokens[i];
			std::shared_ptr<ASTNode> node = nullptr;

			switch (ch.Type) {
			case TType::RIGHT: node = std::make_shared<ASTNode>(ch.Type); break;
			case TType::LEFT: node = std::make_shared<ASTNode>(ch.Type); break;
			case TType::INC: node = std::make_shared<ASTNode>(ch.Type); break;
			case TType::DEC: node = std::make_shared<ASTNode>(ch.Type); break;
			case TType::O: node = std::make_shared<ASTNode>(ch.Type); break;
			case TType::I: node = std::make_shared<ASTNode>(ch.Type); break;

			case TType::LOOPS: {
				auto loop = std::make_shared<Loop>();
				currentBlock->push_back(loop);
				loopStack.push(currentBlock);
				currentBlock = &loop->body;
				continue;
			}
			case TType::LOOPE: {
				if (loopStack.empty())
					return std::format("Unmatched ']' at position {}", i);
				currentBlock = loopStack.top();
				loopStack.pop();
				continue;
			}

			default:
				continue;
			}

			currentBlock->push_back(node);
		}

		if (!loopStack.empty())
			return std::format("Unmatched '[' in source code.");

		return program;
	}
};