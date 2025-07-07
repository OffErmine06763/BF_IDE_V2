#include <iostream>
#include <Utility.h>

#include "Src/Compiler.h"


int main(int argc, char** argv)
{
	// TODO: allow args to use the compiler/emulator

	auto expTokens = Compiler::Tokenize(fs::path("Res/Code.bf"));
	//auto expTokens = Compiler::Tokenize(fs::path("Res/badapple.bf"));
	auto& out = std::cout;
	//std::ofstream out{ "Generated/generated.txt" };

	// TODO: functional optional
	// expT.if_present(std::cout << val).map(Compiler::Parse)...
	
	if (expTokens.success())
	{
		auto tokens = expTokens.getE().value();
		out << tokens << '\n';
		auto expParse = Compiler::Parse(tokens);

		if (expParse.success())
		{
			auto ast = expParse.getE().value();
			out << ast << '\n';
		}
		else
		{
			out << expParse.getU().value() << '\n';
			return 0;
		}
	}
	else
	{
		out << expTokens.getU().value() << '\n';
		return 0;
	}
}