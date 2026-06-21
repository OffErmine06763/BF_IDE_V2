#include <iostream>
#include <Utility.h>

#include "Src/Compiler.h"

#ifdef _WIN32
#include <Terminal.h>
#endif

// TODO: preprocessor directive to set the memory tape size
// TODO: functional optional
// expT.if_present(std::cout << val).map(Compiler::Parse)...

int main(int argc, char** argv)
{
	auto args = stdr::subrange(argv, argv + argc)
			  | stdv::transform([](char* arg) -> std::string {
					std::string str{ arg };
					//std::transform(str.begin(), str.end(), str.begin(), std::tolower);
					return str;
				});

	// abbondante sopprimere il sacrificio tronco buono
	BFC::CompilerError error = BFC::Compiler::Compile({ args.begin(), args.end() });
	// BFC::CompilerError error = BFC::Compiler::Compile({ "Compiler", "Res/Code/Code.bf", "Res/Code/Code2.bf", "-m", "Res/Code/Code.bf", "-o", "Generated/CODE.exe" });
	// BFC::CompilerError error = BFC::Compiler::Compile({ "Compiler", "Res/Code/badapple.bf", "-o", "Generated/CODE.exe" });

	if (error)
	{
#ifdef _WIN32
		std::cout << Terminal::TEXT_F_BRED << "Compilation Error:\n" 
				  << Terminal::TEXT_RESET  << error << '\n';
#else
		std::cout << "Compilation Error:\n" << error << '\n';
#endif
	}
}