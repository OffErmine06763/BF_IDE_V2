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
				res.AddToken(Token::ToType.at(c), row, col);
			else
				res.AddToken(Token::ToType.at(c));
		}
		else if (std::isalpha(c))
		{
			size_t start = i;
			do {
				i++;
				col++;
			} while (i < content.length() && std::isalnum(content[i]));

			if (content[i] == ':')
				res.AddToken(T_LABEL, content.substr(start, i - start));
			else
			{
				res.AddToken(T_GOTO, content.substr(start, i - start));
				i--;
				col--;
			}
			col++;
			continue;
		}
		else if (c == ';')
		{
			// Position is used to warn about unreachable code and give an error for a return outside a label (beginning of file)
			res.AddToken(T_RETURN, row, col);
		}
		else if (c == '#')
		{
			if (i < content.length() - 1 && std::isalpha(content[i + 1]))
			{
				i++;
				col++;
				size_t start = i;
				do {
					i++;
					col++;
				} while (i < content.length() && std::isalnum(content[i]));

				res.AddToken(T_INCLUDE, content.substr(start, i - start));
				i--;
				continue;
			}
			return { "Invalid include at position ["s + std::to_string(row) + ':' + std::to_string(col) + ']' };
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
		: tr(tr), it(tr.cbegin()), next(tr.cbegin())
	{
		if (!Done())
			next++;
	}

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
	void MoveToNextToken()
	{
		if (!Done() && (*it).second != 0)
		{
			it.ForceNext();
			next = it;
			if (!Done())
				next++;
		}
	}

	std::string Symbol(const u32 id) { return tr.symbolsI.at(id); }
	coord<u32> Position(const u32 id) { return tr.loop.at(id); }

private:
	TokenizeResult::CIterator it, next;
	const TokenizeResult& tr;
};

expected<Loop, std::string> ParseLoop(Parser& parser, TranslationUnit& tu)
{
	const auto& [start, iteration] = parser.Consume();
	Loop loop = { start.count, start.ID };
	std::vector<Stmt> body;
	parser.MoveToNextToken();

	while (!parser.Done())
	{
		const auto& [token, iteration] = parser.Consume();
		expected<Loop, std::string> subLoop = { "" };
		u32 id, subcount;
		Loop sub;

		switch (token.type)
		{
		case T_INC: case T_DEC: case T_LEFT: case T_RIGHT: case T_I: case T_O:
			body.push_back({ Operation{ OpFromTType((TType)token.type), token.count } });
			parser.MoveToNextToken();
			break;
		case T_LOOPS:
			parser.Back();
			subLoop = ParseLoop(parser, tu);
			if (!subLoop.success())
			{
				tu.bodies.insert({ loop.ID, body });
				return subLoop.getU().value();
			}
			body.push_back(Stmt{ subLoop.getE().value() });
			break;
		case T_LOOPE:
			// decrementing by iteration to account for subloops like [+[-]]
			// the subloop will advance within the same ] token.
			subcount = token.count - iteration;
			if (subcount == loop.count)
			{
				// if #] == #[ return
				parser.MoveToNextToken();
				tu.bodies.insert({ loop.ID, body });
				return loop;
			}
			else if (subcount > loop.count)
			{
				// i have to consume loop.count times (i have already consumed the first ], and count is 0-indexed)
				// if #] > #[ we close the current loop, and return to the superloop
				for (int i = iteration; i < loop.count; i++)
					parser.Consume();
				tu.bodies.insert({ loop.ID, body });
				return loop;
			}

			// otherwise create a subloop that replaces the body
			// [[[[[+]]-]]-] == [[[token-]]-] == [token-]
			// MAYBE: multiple nested loops with no instructions is between are pointless, but not forbidden,
			//  maybe it would be better not to do this optimization at all and use less memory (but im already using 20bit ID).
			parser.MoveToNextToken();
			id = tu.NextID++;
			sub = Loop{ subcount, id };
			tu.bodies.insert({ id, std::move(body) });
			body.clear();
			body.push_back({ sub });
			loop.count -= subcount + 1; // 0 indexed, must subtract 1 more
			break;
		case T_GOTO:
			body.push_back({ Goto{ token.count, token.ID } });
			tu.gotos.insert(token.ID);
			parser.MoveToNextToken();
			break;
		case T_LABEL:
			tu.bodies.insert({ loop.ID, body });
			return { "Cannot declare a label inside a loop\n" };
		case T_RETURN:
			body.push_back({ Return{} });
			break;
		case T_INCLUDE:
			return { "Internal compiler error, include token at the parse phase." };
		}
	}
	tu.bodies.insert({ loop.ID, body });
	return { Compiler::GetUnmatchedOpenError(parser.Position(start.ID)) };
}
expected<Label, std::string> ParseLabel(Parser& parser, TranslationUnit& tu)
{
	Label label = { parser.Consume().first.ID };
	if (tu.labels.contains(label.ID))
		return { Compiler::GetLabelRedefinitionError(tu.symbolsI.at(label.ID)) };
	tu.labels.insert(label.ID);
	std::vector<Stmt> body;

	while (!parser.Done())
	{
		const auto& [token, iteration] = parser.Consume();
		expected<Loop, std::string> subLoop = { "" };

		switch (token.type)
		{
		case T_INC: case T_DEC: case T_LEFT: case T_RIGHT: case T_I: case T_O:
			body.push_back({ Operation{ OpFromTType((TType)token.type), token.count } });
			parser.MoveToNextToken();
			break;
		case T_LOOPS:
			parser.Back();
			subLoop = ParseLoop(parser, tu);
			if (!subLoop.success())
			{
				tu.bodies.insert({ label.ID, body });
				return subLoop.getU().value();
			}
			body.push_back({ subLoop.getE().value() });
			break;
		case T_LOOPE:
			tu.bodies.insert({ label.ID, body });
			return { Compiler::GetUnmatchedCloseError(parser.Position(token.ID)) };
		case T_GOTO:
			body.push_back({ Goto{ token.count, token.ID } });
			tu.gotos.insert(token.ID);
			parser.MoveToNextToken();
			break;
		case T_LABEL:
			parser.Back();
			tu.bodies.insert({ label.ID, body });
			return label; // the next label will be parsed by the caller
		case T_INCLUDE:
			return { "Internal compiler error, include token at the parse phase." };
		case T_RETURN:
			body.push_back({ Return{} });
		}
	}
	
	tu.bodies.insert({ label.ID, body });
	return label;
}
expected<Block, std::string> ParseRoot(Parser& parser, TranslationUnit& tu)
{
	Block block;
	while (!parser.Done())
	{
		const auto& [token, iteration] = parser.Consume();
		expected<Loop, std::string> subLoop = { "" };
		expected<Label, std::string> subLabel = { "" };

		switch (token.type)
		{
		case T_INC: case T_DEC: case T_LEFT: case T_RIGHT: case T_I: case T_O:
			block.items.push_back({ Stmt{ Operation{ OpFromTType((TType)token.type), token.count } } });
			parser.MoveToNextToken();
			break;
		case T_LOOPS:
			parser.Back();
			subLoop = ParseLoop(parser, tu);
			if (!subLoop.success())
				return subLoop.getU().value();
			block.items.push_back({ Stmt{ subLoop.getE().value() } });
			break;
		case T_LOOPE:
			return { Compiler::GetUnmatchedCloseError(parser.Position(token.ID)) };
		case T_GOTO:
			block.items.push_back({ Stmt{ Goto{ token.count, token.ID } } });
			tu.gotos.insert(token.ID);
			parser.MoveToNextToken();
			break;
		case T_LABEL:
			parser.Back();
			subLabel = ParseLabel(parser, tu);
			if (!subLabel.success())
				return subLabel.getU().value();
			block.items.push_back({ Decl{ subLabel.getE().value() } });
			break;
		case T_INCLUDE:
			return { "Internal compiler error, include token at the parse phase."};
		case T_RETURN:
			// intentionally allow returns outside a label, will be a warning
			block.items.push_back({ Stmt{ Return{} } });
			break;
		}
	}

	return block;
}

expected<TranslationUnit, std::string> Compiler::Parse(const TokenizeResult& tr)
{
	TranslationUnit tu;
	tu.symbolsI = tr.symbolsI;
	tu.symbolsS = tr.symbolsS;
	tu.NextID = tr.NextID;

	Parser parser = { tr };
	auto res = ParseRoot(parser, tu);
	if (!res.success())
		return res.getU().value();

	tu.body = res.getE().value();

	for (const u32 id : tu.gotos)
	{
		if (!tu.labels.contains(id))
			return { "Undefined label '"s + tu.symbolsI.at(id) + '\''};
	}

	tu.gotos.clear();
	tu.labels.clear();
	return tu;
}