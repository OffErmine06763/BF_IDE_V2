#include "Compiler.h"


expected<TokenizeResult, std::string> Compiler::Tokenize(const std::string& content)
{
	TokenizeResult res;
	res.tokens.reserve(content.length()); // in standard bf 1 char = 1 token

	u32 row = 0, col = 0;
	for (size_t i = 0; i < content.length(); i++)
	{
		const char c = content[i];

		if (c == '\n') {
			row++;
			col = 0;
			continue;
		}
		else if (c == '/')
		{
			if (i < content.length() - 1 && content[i + 1] == '/')
			{
				i = content.find('\n', i);
				if (i == std::string::npos)
					i = content.length() - 1;
				continue;
			}
			else
				return { std::format("Invalid Comment at position {}:{}", row, col) };
		}
		else if (BF_ALL_S.contains(c))
		{
			if (c == '[' || c == ']')
				res.AddToken({ Token::ToType.at(c) }, row, col);
			else
				res.AddToken({ Token::ToType.at(c) });
		}
		else if (std::isalpha(c))
		{
			size_t start = i;
			do {
				i++;
			} while (i < content.length() && std::isalnum(content[i]));

			if (content[i] == ':')
				res.AddToken({ TType::LABEL }, content.substr(start, i - start));
			else
			{
				res.AddToken({ TType::GOTO }, content.substr(start, i - start));
				i--;
			}
			continue;
		}
		else if (c != ' ' && c != '\t' && c != '\n')
			return { std::format("Unrecognized Token '{}' at position {}:{}", c, row, col)};

		col++;
	}

	return res;
}


expected<std::shared_ptr<Program>, std::string> Compiler::Parse(const TokenizeResult& tr)
{
	auto program = std::make_shared<Program>();
	std::stack<std::vector<std::shared_ptr<ASTNode>>*> loopStack;
	std::vector<std::shared_ptr<ASTNode>>* currentBlock = &program->statements;

	const auto& tokens = tr.tokens;
	const auto& symbols = tr.symbolsI;
	const auto& ss = tr.symbolsS;

	for (size_t i = 0; i < tokens.size(); ++i) {
		const Token ch = tokens[i];
		std::shared_ptr<ASTNode> node = nullptr;

		switch (ch.type()) {
		case TType::RIGHT: node = std::make_shared<ASTNode>(ch.type()); break;
		case TType::LEFT: node = std::make_shared<ASTNode>(ch.type()); break;
		case TType::INC: node = std::make_shared<ASTNode>(ch.type()); break;
		case TType::DEC: node = std::make_shared<ASTNode>(ch.type()); break;
		case TType::O: node = std::make_shared<ASTNode>(ch.type()); break;
		case TType::I: node = std::make_shared<ASTNode>(ch.type()); break;

		case TType::LOOPS: {
			auto loop = std::make_shared<Loop>();
			currentBlock->push_back(loop);
			loopStack.push(currentBlock);
			currentBlock = &loop->body;
			continue;
		}
		case TType::LOOPE: {
			if (loopStack.empty())
				return std::format("Unmatched ']' at token {}", i);
			currentBlock = loopStack.top();
			loopStack.pop();
			continue;
		}

		default:
			continue;
		}

		currentBlock->push_back(node);
	}

	if (!loopStack.empty())
		return std::format("Unmatched '[' in source code.");

	return program;
}