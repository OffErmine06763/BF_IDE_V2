#include <iostream>
#include <Utility.h>

#include "Src/Compiler.h"


// TODO: allow args to use the compiler/emulator
// TODO: functional optional
// expT.if_present(std::cout << val).map(Compiler::Parse)...
int main(int argc, char** argv)
{
	//auto& out = std::cout;
	std::ofstream out{ "Generated/generated.txt" };

	stdc::nanoseconds total = 0ns;

	// TOKENIZATION
	stdc::time_point start = stdc::high_resolution_clock::now();
	
	auto expTokens = Compiler::Tokenize(fs::path("Res/Code.bf"));
	//auto expTokens = Compiler::Tokenize(fs::path("Res/badapple.bf"));
	
	stdc::time_point end = stdc::high_resolution_clock::now();
	total += end - start;
	std::cout << "Tokenization done in: " << to<stdc::milliseconds>(end - start) << '\n';

	if (!expTokens.success())
	{
		std::cout << expTokens.getUUnchecked() << '\n';
		return 0;
	}

	auto tokens = expTokens.getEUnchecked();
	out << "TOKENS\n" << tokens << '\n';


	// PARSING

	start = stdc::high_resolution_clock::now();
	
	auto expParse = Compiler::Parse(std::move(tokens));
	
	end = stdc::high_resolution_clock::now();
	total += end - start;
	std::cout << "Parsing done in: " << to<stdc::milliseconds>(end - start) << '\n';

	if (!expParse.success())
	{
		std::cout << expParse.getUUnchecked() << '\n';
		return 0;
	}

	auto ast = expParse.getEUnchecked();
	out << "AST\n" << ast << '\n';


	// ANALYZING

	start = stdc::high_resolution_clock::now();

	auto expAnalyze = Compiler::Analyze(ast);

	end = stdc::high_resolution_clock::now();
	total += end - start;
	std::cout << "Analyzing done in: " << to<stdc::nanoseconds>(end - start) << '\n';

	if (expAnalyze.has_value())
	{
		std::cout << expAnalyze.value() << '\n';
		return 0;
	}

	out << "Analyze SUCCESS\n" << '\n';


	std::cout << "Compilation done in: " << to<stdc::milliseconds>(total) << '\n';
}