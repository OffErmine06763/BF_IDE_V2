#pragma once
#include <Utility.h>
#include "Token.h"
#include "AST.h"




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

	static expected<TranslationUnit, std::string> Parse(const TokenizeResult& tr);
};