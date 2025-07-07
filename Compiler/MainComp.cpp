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


	// TOKENIZATION

	stdc::time_point start = stdc::high_resolution_clock::now();
	
	//auto expTokens = Compiler::Tokenize(fs::path("Res/Code.bf"));
	auto expTokens = Compiler::Tokenize(fs::path("Res/badapple.bf"));
	
	stdc::time_point end = stdc::high_resolution_clock::now();
	std::cout << "Tokenization done in: " << stdc::duration_cast<stdc::milliseconds>(end - start) << '\n';

	if (!expTokens.success())
	{
		out << expTokens.getU().value() << '\n';
		return 0;
	}

	auto tokens = expTokens.getE().value();
	out << tokens << '\n';


	// PARSING

	start = stdc::high_resolution_clock::now();
	
	auto expParse = Compiler::Parse(tokens);
	
	end = stdc::high_resolution_clock::now();
	std::cout << "Parsing done in: " << stdc::duration_cast<stdc::milliseconds>(end - start) << '\n';


	if (!expParse.success())
	{
		out << expParse.getU().value() << '\n';
		return 0;
	}

	auto ast = expParse.getE().value();
	out << ast << '\n';
}