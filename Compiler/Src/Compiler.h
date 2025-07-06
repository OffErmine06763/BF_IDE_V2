#pragma once
#include <Utility.h>
#include "Token.h"


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
	Program() : ASTNode(TType::NONE) {}
};




class Compiler
{
public:

	// Lexical Analyzer / Scanner / Lexer
	static expected<TokenizeResult, std::string> Tokenize(const fs::path& file) {
		std::ifstream in(file);
		std::string content = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
		in.close();
		return Tokenize(content);
	}
	static expected<TokenizeResult, std::string> Tokenize(const std::string& content);

	static expected<std::shared_ptr<Program>, std::string> Parse(const TokenizeResult& tr);
};