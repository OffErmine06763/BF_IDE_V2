#include <iostream>
#include <Utility.h>

#include "Src/Compiler.h"


int main(int argc, char** argv)
{
	// TODO: allow args to use the compiler/emulator

	//std::string code = "+-[[]]]//<><>\nmain:\tmain//+";

	auto expTokens = Compiler::Tokenize(fs::path("Res/Code.bf"));
	if (expTokens.getE().has_value())
	{
		auto val = expTokens.getE().value();
		std::cout << val << '\n';
	}
	else
		std::cout << expTokens.getU().value() << '\n';
}