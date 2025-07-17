#include <iostream>
#include <Utility.h>

#include "Src/Compiler.h"


// TODO: allow args to use the compiler/emulator
// TODO: preprocessor directive to set the memory tape size
// TODO: functional optional
// expT.if_present(std::cout << val).map(Compiler::Parse)...
int main(int argc, char** argv)
{
	//auto& out = std::cout;
	std::ofstream out{ "Generated/generated.txt" };

	stdc::nanoseconds total = 0ns;

	
	// TOKENIZATION
	
	stdc::time_point start = stdc::clock::now();
	
	//auto expTokens = Compiler::Tokenize(fs::path("Res/Code.bf"));
	auto expTokens = Compiler::Tokenize(fs::path("Res/badapple.bf"));
	
	stdc::time_point end = stdc::clock::now();
	total += end - start;
	std::cout << "Tokenization done in: "; print_time(std::cout, end - start) << '\n';

	if (!expTokens.success())
	{
		std::cout << expTokens.getUUnchecked() << '\n';
		return 0;
	}

	auto tokens = expTokens.getEUnchecked();
	//out << "TOKENS\n" << tokens << '\n';


	// PARSING

	start = stdc::clock::now();
	
	auto expParse = Compiler::Parse(std::move(tokens));
	
	end = stdc::clock::now();
	total += end - start;
	std::cout << "Parsing done in: "; print_time(std::cout, end - start) << '\n';

	if (!expParse.success())
	{
		std::cout << expParse.getUUnchecked() << '\n';
		return 0;
	}

	auto ast = expParse.getEUnchecked();
	//out << "AST\n" << ast << '\n';


	// ANALYZING

	start = stdc::clock::now();

	auto expAnalyze = Compiler::Analyze(ast);

	end = stdc::clock::now();
	total += end - start;
	std::cout << "Analyzing done in: "; print_time(std::cout, end - start) << '\n';

	if (expAnalyze.has_value())
	{
		std::cout << expAnalyze.value() << '\n';
		return 0;
	}

	//out << "Analyze SUCCESS\n\n";


	// OPTIMIZING

	size_t initialSize = ast.body.items.size();
	for (const auto& sub : ast.bodies)
		initialSize += sub.second.size();
	start = stdc::clock::now();

	Compiler::Optimize(ast);

	end = stdc::clock::now();
	total += end - start;
	std::cout << "Optimizing done in: "; print_time(std::cout, end - start) << '\n';
	size_t optimizedSize = ast.body.items.size();
	for (const auto& sub : ast.bodies)
		optimizedSize += sub.second.size();
	std::cout << "  size reduction: " << ((f64)optimizedSize / initialSize * 100) << "%\n";

	//out << "Optimized\n" << ast << '\n';


	// IR

	start = stdc::clock::now();

	auto ir = Compiler::Intermediate(std::move(ast));

	end = stdc::clock::now();
	total += end - start;
	std::cout << "IRC done in: "; print_time(std::cout, end - start) << '\n';

	out << "Intermediate Representation\n" << ir << '\n';


	std::cout << "Compilation done in: "; print_time(std::cout, total) << '\n';
}