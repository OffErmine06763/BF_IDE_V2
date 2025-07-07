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
				return { GetInvalidCommentError(row, col) };
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
				col++;
			} while (i < content.length() && std::isalnum(content[i]));

			if (content[i] == ':')
				res.AddToken({ TType::LABEL }, content.substr(start, i - start));
			else
			{
				res.AddToken({ TType::GOTO }, content.substr(start, i - start));
				i--;
				col--;
			}
			col++;
			continue;
		}
		else if (c != ' ' && c != '\t' && c != '\n')
			return { GetUnreconTokenError(c, row, col) };

		col++;
	}

	return res;
}


class Parser
{
public:
	Parser(const TokenizeResult& tr)
		: tr(tr), it(tr.cbegin()), next(++tr.cbegin())
	{}

	bool HasNext() { return next != tr.cend(); }
	bool Done() { return it == tr.cend(); }
	auto Get() { return *it; }
	auto Peek() { return *next; }
	auto Consume()
	{
		auto res = *it;
		it = next;
		if (!Done())
			++next;
		return res;
	};
	void Back()
	{
		next = it;
		if (it != tr.cbegin())
			--it;
	}

	std::string Symbol(const u32 id) { return tr.symbolsI.at(id); }
	coord<u32> Position(const u32 id) { return tr.loop.at(id); }

private:
	TokenizeResult::CIterator it, next;
	const TokenizeResult& tr;
};

expected<Loop, std::string> ParseLoop(Parser& parser)
{
	Loop loop;
	const auto& [start, _] = parser.Consume();

	while (!parser.Done())
	{
		const auto& [token, count] = parser.Consume();
		expected<Loop, std::string> subLoop = { "" };

		switch (token.type)
		{
		case INC: case DEC: case LEFT: case RIGHT: case I: case O:
			loop.body.push_back(Stmt{ Operation{ OpFromTType((TType)token.type) } });
			break;
		case LOOPS:
			parser.Back();
			subLoop = ParseLoop(parser);
			if (!subLoop.success())
				return subLoop.getU().value();
			loop.body.push_back(Stmt{ subLoop.getE().value() });
			break;
		case LOOPE:
			return loop;
		case GOTO:
			loop.body.push_back(Stmt{ Goto{ std::to_string(token.ID) } });
			break;
		case LABEL:
			return { "Cannot decleare a label inside a loop\n" };
		case RETURN:
			loop.body.push_back(Stmt{ Return{} });
			break;
		}
	}
	return { "Unmatched [ at position ("s + std::to_string(start.ID) + ")\n"s };
}
expected<Label, std::string> ParseLabel(Parser& parser)
{
	Label label = { parser.Symbol((parser.Consume()).first.ID) };
	while (!parser.Done())
	{
		const auto& [token, count] = parser.Consume();
		expected<Loop, std::string> subLoop = { "" };

		switch (token.type)
		{
		case INC: case DEC: case LEFT: case RIGHT: case I: case O:
			label.body.push_back(Stmt{ Operation{ OpFromTType((TType)token.type) } });
			break;
		case LOOPS:
			parser.Back();
			subLoop = ParseLoop(parser);
			if (!subLoop.success())
				return subLoop.getU().value();
			label.body.push_back(Stmt{ subLoop.getE().value() });
			break;
		case LOOPE:
			return { "Unmatched ] at position "s + parser.Position(token.ID) + '\n' };
		case GOTO:
			label.body.push_back(Stmt{ Goto{ parser.Symbol(token.ID) } });
			break;
		case LABEL:
			parser.Back();
			return label;
		}
	}
	
	return label;
}
expected<Block, std::string> ParseBlock(Parser& parser)
{
	Block block;
	while (!parser.Done())
	{
		const auto& [token, count] = parser.Consume();
		expected<Loop, std::string> subLoop = { "" };
		expected<Label, std::string> subLabel = { "" };

		switch (token.type)
		{
		case INC: case DEC: case LEFT: case RIGHT: case I: case O:
			block.items.push_back(BlockItem{ Stmt{ Operation{ OpFromTType((TType)token.type) } } });
			break;
		case LOOPS:
			parser.Back();
			subLoop = ParseLoop(parser);
			if (!subLoop.success())
				return subLoop.getU().value();
			block.items.push_back(BlockItem{ Stmt{ subLoop.getE().value() } });
			break;
		case LOOPE:
			return { "Unmatched ] at position "s + parser.Position(token.ID) + '\n'};
		case GOTO:
			block.items.push_back(BlockItem{ Stmt{ Goto{ parser.Symbol(token.ID) } } });
			break;
		case LABEL:
			parser.Back();
			subLabel = ParseLabel(parser);
			if (!subLabel.success())
				return subLabel.getU().value();
			block.items.push_back(BlockItem{ Decl{ subLabel.getE().value() } });
			break;
		}
	}

	return block;
}

expected<TranslationUnit, std::string> Compiler::Parse(const TokenizeResult& tr)
{
	TranslationUnit tu;
	Parser parser = { tr };
	auto res = ParseBlock(parser);
	if (!res.success())
		return res.getU().value();

	tu.body = res.getE().value();
	return tu;

	/*auto program = std::make_shared<ASTNode>();
	std::stack<std::vector<std::shared_ptr<ASTNode>>*> loopStack;
	std::vector<std::shared_ptr<ASTNode>>* currentBlock = &program->statements;

	const auto& tokens = tr.tokens;
	const auto& symbols = tr.symbolsI;
	const auto& ss = tr.symbolsS;

	for (size_t i = 0; i < tokens.size(); ++i) {
		const Token token = tokens[i];
		std::shared_ptr<ASTNode> node = nullptr;

		switch (token.type) {
		case TType::RIGHT: node = std::make_shared<ASTNode>(token.type); break;
		case TType::LEFT: node = std::make_shared<ASTNode>(token.type); break;
		case TType::INC: node = std::make_shared<ASTNode>(token.type); break;
		case TType::DEC: node = std::make_shared<ASTNode>(token.type); break;
		case TType::O: node = std::make_shared<ASTNode>(token.type); break;
		case TType::I: node = std::make_shared<ASTNode>(token.type); break;

		case TType::LOOPS: {
			auto loop = std::make_shared<Loop>();
			currentBlock->push_back(loop);
			loopStack.push(currentBlock);
			currentBlock = &loop->body;
			continue;
		}
		case TType::LOOPE: {
			if (loopStack.empty())
			{
				std::stringstream ss;
				ss << "Unmatched ']' at position " << tr.loop.at(token.ID());
				return ss.str();
			}
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

	return program;*/
}