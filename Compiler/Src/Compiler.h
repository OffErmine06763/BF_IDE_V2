#pragma once
#include <Utility.h>
#include "Token.h"
#include "AST.h"
#include "IR.h"




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

	static expected<TranslationUnit, std::string> Parse(TokenizeResult&& tr);

	static std::optional<std::string> Analyze(const TranslationUnit& tu);

	static void Optimize(TranslationUnit& tu);
	
	/// basically go from LOOP { body } to LABEL loop1 body JNZ loop1
	static IR Intermediate(TranslationUnit&& tu);
	// https://chatgpt.com/c/6868f511-6f14-8002-a7ba-51572cb83a2f#:~:text=explain%20Intermediate%20Code%20Generator

	static void ToLLVM() {}
	// https://chatgpt.com/c/6868f511-6f14-8002-a7ba-51572cb83a2f#:~:text=Hai%20detto%3A-,target%20LLVM,-ChatGPT%20ha%20detto
	// https://chatgpt.com/c/6868f511-6f14-8002-a7ba-51572cb83a2f#:~:text=Hai%20detto%3A-,Add%20JIT%20execution,-ChatGPT%20ha%20detto

	static void ToASM() {}


	/*
	,[[->+<]>-]+

	IN
	LABEL L0
	LABEL L1
	DEC
	RIGHT
	INC
	LEFT
	JNZ L1
	RIGHT
	DEC
	JNZ L0
	INC

	
	MAIN:+;

	LABEL MAIN
	INC
	RET

	*/


public:
	static std::string GetUnreconTokenError(const char c, const u32 row, const u32 col) {
		return std::format("Unrecognized Token '{}' at position [{}:{}]", c, row, col);
	}
	static std::string GetInvalidCommentError(const u32 row, const u32 col) {
		return std::format("Invalid Comment at position [{}:{}]", row, col);
	}
	static std::string GetUnmatchedOpenError(const coord<u32>& pos) {
		return "Unmatched [ at position "s + pos;
	}
	static std::string GetUnmatchedCloseError(const coord<u32>& pos) {
		return "Unmatched ] at position "s + pos;
	}
	static std::string GetLabelRedefinitionError(const std::string& name) {
		return "Label redefinition: "s + name;
	}
};