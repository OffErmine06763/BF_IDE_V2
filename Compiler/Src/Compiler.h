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



public:
	static std::string GetUnreconTokenError(const char c, const u32 row, const u32 col) {
		return std::format("Unrecognized Token '{}' at position [{}:{}]", c, row, col);
	}
	static std::string GetInvalidCommentError(const u32 row, const u32 col) {
		return std::format("Invalid Comment at position [{}:{}]", row, col);
	}
	static std::string GetUnmatchedOpenError(const coord<u32>& pos) {
		return "Unmatched [ at position "s + pos + '\n';
	}
	static std::string GetUnmatchedCloseError(const coord<u32>& pos) {
		return "Unmatched ] at position "s + pos + '\n';
	}
};