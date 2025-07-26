#include "Compiler.h"

#define NOMINMAX
#include <Windows.h>

#include <Terminal.h>


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
			// Position is used to warn about unreachable code and for a return outside a label (beginning of file)
			res.AddToken(T_RETURN, row, col);
		}
		else if (c == '#')
		{
			std::string externstr = "extern", exportstr = "export";
			if (content.substr(i + 1, externstr.length()) == externstr)
			{
				i   += externstr.length() + 1;
				col += to<u32>(externstr.length() + 1);

				do
				{
					while (i < content.length() && (content[i] == ' ' || content[i] == '\t' || content[i] == ','))
					{
						i++;
						col++;
					}
					if (i == content.length())
						break;
					else if (!std::isalpha(content[i]))
						return { "Labels must start with a letter: [" + std::to_string(row) + ':' + std::to_string(col) + ']' };

					size_t start = i;
					do {
						i++;
						col++;
					} while (i < content.length() && std::isalnum(content[i]));

					res.AddExtern(content.substr(start, i - start));
				} while (content[i] == ',' || content[i] == ' ' || content[i] == '\t');
			}
			else if (content.substr(i + 1, exportstr.length()) == exportstr)
			{
				i += exportstr.length() + 1;
				col += to<u32>(exportstr.length() + 1);

				do
				{
					while (i < content.length() && (content[i] == ' ' || content[i] == '\t' || content[i] == ','))
					{
						i++;
						col++;
					}
					if (i == content.length())
						break;
					else if (!std::isalpha(content[i]))
						return { "Labels must start with a letter: [" + std::to_string(row) + ':' + std::to_string(col) + ']' };

					size_t start = i;
					do {
						i++;
						col++;
					} while (i < content.length() && std::isalnum(content[i]));

					res.AddExport(content.substr(start, i - start));
				} while (content[i] == ',' || content[i] == ' ' || content[i] == '\t');
			}
			else
				return { "Invalid preprocessor directive at position [" + std::to_string(row) + ':' + std::to_string(col) + ']' };
		}
		else if (c != ' ' && c != '\t' && c != '\n')
			return GetUnreconTokenError(c, row, col);

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

static expected<Loop, std::string> ParseLoop(Parser& parser, TU& tu)
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
			body.push_back({ subLoop.getE().value() });
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
				for (u32 i = iteration; i < loop.count; i++)
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
			body.push_back({ Goto{ token.ID } });
			tu.gotos.insert(token.ID);
			parser.MoveToNextToken();
			break;
		case T_LABEL:
			tu.bodies.insert({ loop.ID, body });
			return { "Cannot declare a label inside a loop\n" };
		case T_RETURN:
			body.push_back({ Return{} });
			break;
		}
	}
	tu.bodies.insert({ loop.ID, body });
	return { Compiler::GetUnmatchedOpenError(parser.Position(start.ID)) };
}
static expected<Label, std::string> ParseLabel(Parser& parser, TU& tu)
{
	Label label = { parser.Consume().first.ID };
	if (tu.labels.contains(label.ID))
		return { Compiler::GetLabelRedefinitionError(tu.symbolsI.at(label.ID)) };
	if (tu.externs.contains(tu.symbolsI.at(label.ID)))
		return { "Label " + tu.symbolsI.at(label.ID) + " declared as extern but defined in the current file" };
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
			body.push_back({ Goto{ token.ID } });
			tu.gotos.insert(token.ID);
			parser.MoveToNextToken();
			break;
		case T_LABEL:
			parser.Back();
			tu.bodies.insert({ label.ID, body });
			return label; // the next label will be parsed by the caller
		case T_RETURN:
			body.push_back({ Return{} });
		}
	}

	tu.bodies.insert({ label.ID, body });
	return label;
}
static expected<Block, std::string> ParseRoot(Parser& parser, TU& tu)
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
			block.items.push_back({ Stmt{ Goto{ token.ID } } });
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
		case T_RETURN:
			// intentionally allow returns outside a label, will be a warning
			block.items.push_back({ Stmt{ Return{} } });
			break;
		}
	}

	return block;
}

expected<TU, std::string> Compiler::Parse(TokenizeResult&& tr)
{
	TU tu;
	tu.symbolsI = std::move(tr.symbolsI);
	tu.symbolsS = std::move(tr.symbolsS);
	tu.exports  = std::move(tr.exports);
	tu.externs  = std::move(tr.externs);
	tu.NextID   = tr.NextID;

	Parser parser = { tr };
	auto res = ParseRoot(parser, tu);
	if (!res.success())
		return res._getU();

	tu.body = res._getE();
	return tu;
}



std::optional<std::string> Compiler::Analyze(const TU& tu)
{
	for (const u32 id : tu.gotos)
	{
		if (!tu.labels.contains(id) && !tu.externs.contains(tu.symbolsI.at(id)))
			return { "Undefined label \""s + tu.symbolsI.at(id) + '\"' };
	}
	for (const std::string& exp : tu.exports)
	{
		if (!tu.symbolsS.contains(exp))
			return { "Exported symbol \"" + exp + "\" not defined" };
	}

	return std::nullopt;
}



enum class MergeResult : u8 { NOOP, HALF, FULL };
static MergeResult Merge(Stmt& prev, Stmt& stmt)
{
	if (holds<Operation>(stmt.value, prev.value))
	{
		Operation& prevo = std::get<Operation>(prev.value);
		Operation& op = std::get<Operation>(stmt.value);
		if (op.type + prevo.type == 0 && op.type != prevo.type)
		{
			if (op.count == prevo.count)
				return MergeResult::FULL;
			else if (op.count > prevo.count)
			{
				// +++ ---- == -
				//  2    3  == 0
				prevo.count = op.count - prevo.count - 1;
				prevo.type = op.type;
				return MergeResult::HALF;
			}
			else
			{
				// ++++ --- = +
				//    3   2 = 0
				prevo.count -= op.count + 1;
				return MergeResult::HALF;
			}
		}
	}
	return MergeResult::NOOP;
}
static void OptimizeStatement(Stmt& stmt, TU& tu);
static void OptimizeLoop(Loop& loop, TU& tu)
{
	loop.count = 0; // in the AST a loop is (at least) a pair of square brackets, [[body]] makes no sense
	std::vector<Stmt>& body = tu.bodies.at(loop.ID);
	if (body.empty())
		return;

	size_t offset = 0;
	OptimizeStatement(body[0], tu);
	for (size_t i = 1; i < body.size(); i++)
	{
		Stmt& prevs = body[i - 1 - offset];
		Stmt& stmt = body[i];
		OptimizeStatement(stmt, tu);
		MergeResult res = Merge(prevs, stmt);
		if (res == MergeResult::HALF)
			offset++;
		else if (res == MergeResult::FULL)
		{
			offset += 2;
			if (i - 1 - offset < 0)
				i++;
		}
		else if (offset != 0)
			body[i - offset] = body[i];
	}
	body.resize(body.size() - offset);

	if (body.size() == 1 && holds<Loop>(body[0].value))
	{
		Loop& inner = std::get<Loop>(body[0].value);
		auto innerID = inner.ID;
		tu.bodies.erase(loop.ID);
		loop.ID = innerID;
		// we are in the situation where optimizing this loop body left only another loop
		// we can "merge" the two, but since nested loop are equivalent to a single loop
		// we just replace the current with the nested
	}
}
static void OptimizeStatement(Stmt& stmt, TU& tu)
{
	if (holds<Loop>(stmt.value))
		OptimizeLoop(std::get<Loop>(stmt.value), tu);
}
static void OptimizeLabel(Label& label, TU& tu)
{
	std::vector<Stmt>& body = tu.bodies.at(label.ID);
	if (body.empty())
		return;

	OptimizeStatement(body[0], tu);
	if (holds<Return>(body[0].value))
	{
		body.resize(1);
		return;
	}

	size_t offset = 0;
	for (size_t i = 1; i < body.size(); i++)
	{
		Stmt& prevs = body[i - 1 - offset];
		Stmt& stmt = body[i];
		OptimizeStatement(stmt, tu);
		MergeResult res = Merge(prevs, stmt);
		if (res == MergeResult::HALF)
			offset++;
		else if (res == MergeResult::FULL)
		{
			offset += 2;
			if (i - 1 - offset < 0)
				i++;
		}
		else if (offset != 0)
			body[i - offset] = body[i];
		if (holds<Return>(stmt.value))
		{
			body.resize(i + 1);
			return;
		}
	}
	body.resize(body.size() - offset);
}
static void OptimizeBlock(Block& block, TU& tu)
{
	if (block.items.empty())
		return;

	size_t offset = 0;
	std::vector<BlockItem>& body = block.items;
	if (holds<Stmt>(body[0].value))
		OptimizeStatement(std::get<Stmt>(body[0].value), tu);
	else if (holds<Decl>(body[0].value))
		OptimizeLabel(std::get<Decl>(body[0].value).label, tu);

	for (size_t i = 1; i < body.size(); i++)
	{
		BlockItem& prev = body[i - 1 - offset];
		BlockItem& item = body[i];
		if (holds<Stmt>(item.value, prev.value))
		{
			Stmt& prevs = std::get<Stmt>(prev.value);
			Stmt& stmt = std::get<Stmt>(item.value);
			OptimizeStatement(stmt, tu);
			MergeResult res = Merge(prevs, stmt);
			if (res == MergeResult::HALF)
				offset++;
			else if (res == MergeResult::FULL)
			{
				offset += 2;
				if (i - 1 - offset < 0)
					i++;
			}
			else if (offset != 0)
				body[i - offset] = body[i];
		}
		else if (holds<Decl>(item.value))
			OptimizeLabel(std::get<Decl>(item.value).label, tu);
	}
	body.resize(body.size() - offset);
}

void Compiler::Optimize(TU& tu)
{
	// NOTE: harder to find unreferenced labels as there is fallthrough
	// NOTE: intentionally leave multiple consecutive input operations (might want to consume a buffer)
	// [[++<>-]] == [+]
	// [[++]+-] == [[++]] == [++]

	OptimizeBlock(tu.body, tu);
}




static void IntermediateStatement(const Stmt& s, IR& ir, const TU& tu);
static void IntermediateLoop(const Loop& lo, IR& ir, const TU& tu)
{
	u32 id = lo.ID;
	u32 endid = ir.NextID++;
	
	ir.code.push_back({ .type = IR_LOOP, .ID = id });
	ir.names.insert({ id, std::format("_LOOP_START_{}", id) });
	ir.code.push_back({ .type = IR_JZ, .ID = endid });
	
	for (const Stmt& s : tu.bodies.at(id))
		IntermediateStatement(s, ir, tu);
	
	ir.code.push_back({ .type = IR_JMP, .ID = id });
	ir.code.push_back({ .type = IR_LOOP, .ID = endid });
	ir.names.insert({ endid, std::format("_LOOP_END_{}", endid) });
}
static void IntermediateOperation(const Operation& o, IR& ir, const TU& tu)
{
	ir.code.push_back({ .type = IRFromOpType((OpType)o.type), .count = o.count });
}
static void IntermediateReturn(const Return& r, IR& ir, const TU& tu)
{
	ir.code.push_back({ .type = IR_RET });
}
static void IntermediateGoto(const Goto& g, IR& ir, const TU& tu)
{
	ir.code.push_back({ .type = IR_GOTO, .ID = g.ID });
}
static void IntermediateStatement(const Stmt& s, IR& ir, const TU& tu)
{
	std::visit(visitor{
		[&](const Goto& el)      { IntermediateGoto(el, ir, tu); },
		[&](const Return& el)    { IntermediateReturn(el, ir, tu); },
		[&](const Operation& el) { IntermediateOperation(el, ir, tu); },
		[&](const Loop& el)      { IntermediateLoop(el, ir, tu); },
		[](const auto& el)		 { static_assert(false, "non-exhaustive Stmt ICG visitor"); }
	}, s.value);
}
static void IntermediateDeclaration(const Decl& d, IR& ir, const TU& tu)
{
	auto id = d.label.ID;
	ir.code.push_back({ .type = IR_LABEL, .ID = id });
	for (const Stmt& s : tu.bodies.at(id))
		IntermediateStatement(s, ir, tu);
}

IR Compiler::Intermediate(TU&& tu)
{
	IR ir;
	ir.exports = std::move(tu.exports);
	ir.externs = std::move(tu.externs);
	ir.NextID  = tu.NextID;
	ir.names   = std::move(tu.symbolsI);

	for (const BlockItem& bi : tu.body.items)
	{
		std::visit(visitor{
			[&](const Stmt& el) { IntermediateStatement(el, ir, tu); },
			[&](const Decl& el) { IntermediateDeclaration(el, ir, tu); },
			[](const auto& el)  { static_assert(false, "non-exhaustive BlockItem ICG visitor"); }
		}, bi.value);
	}

	return ir;
}



void Compiler::ToASM_AMDWin64(const IR& ir, std::ostream& out, bool main)
{
	if (main)
	{
		out << "section .data\n"
			<< "    hStdOut dq 0\n"
			<< "    hStdIn  dq 0\n";

		out << '\n';

		out << "section .bss\n"
			<< "    tape resb " << BF_MEMSIZE << '\n'
			<< "    bytes_read resd 1\n"; // Reserve 4 bytes buffer for StdIn

		out << '\n';
	}

	out << "section .text\n";
	if (main)
		out << "    global _main, _in, _out\n";
	if (!ir.exports.empty())
	{
		out << "    global ";
		for (const auto& exp : ir.exports)
			out << exp << ", ";
		out << '\n';
	}

	out << "    extern ExitProcess, GetStdHandle, WriteFile, ReadFile\n";
	if (!ir.externs.empty())
	{
		out << "    extern ";
		for (const auto& ext : ir.externs)
			out << ext << ", ";
		out << '\n';
	}
	if (!main)
		out << "    extern _in, _out";

	out << '\n';

	if (main)
	{
		out << "_out:\n";
		out << "    mov rcx, [rel hStdOut]\n"
			<< "    mov rdx, rsi\n"				// lpBuffer
			<< "    mov r8, 1\n"				// nNumberOfBytesToWrite
			<< "    xor r9, r9\n"				// lpNumberOfBytesWritten (NULL)
			<< "    sub rsp, 40\n"				// Shadow space
			<< "    mov qword [rsp + 32], 0\n"	// lpOverlapped (NULL)
			<< "    call WriteFile\n"
			<< "    add rsp, 40\n"				// Clean up stack
			<< "    ret\n";

		out << '\n';

		out << "_in:\n"
			<< "    mov rcx, [rel hStdIn]\n"
			<< "    mov rdx, rsi\n"					// lpBuffer
			<< "    mov r8, 1\n"					// nNumberOfBytesToRead
			<< "    lea r9, [rel bytes_read]\n"		// lpNumberOfBytesRead
			<< "    sub rsp, 40\n"					// Shadow space
			<< "    mov qword [rsp + 32], 0\n"		// lpOverlapped(NULL)
			<< "    call ReadFile\n"
			<< "    add rsp, 40\n"					// Clean up stack
			<< "    ret\n";

		out << '\n';

		out << "_main:\n";
		out << "    lea rsi, [rel tape]\n";
		out << "    mov rcx, -11\n" // STD_OUTPUT_HANDLE = -11
			<< "    sub rsp, 32\n"
			<< "    call GetStdHandle\n"
			<< "    add rsp, 32\n"
			<< "    mov [rel hStdOut], rax\n";
		out << "    mov rcx, -10\n" // STD_INPUT_HANDLE = -10
			<< "    sub rsp, 32\n"
			<< "    call GetStdHandle\n"
			<< "    add rsp, 32\n"
			<< "    mov [rel hStdIn], rax\n";

		out << '\n';
	}

	for (const auto& line : ir.code)
	{
		switch (line.type)
		{
		case IR_LABEL:
			out << '\n' << ir.names.at(line.ID) << ":\n";
			break;
		case IR_INC:
			if (line.count == 0) out << "    inc byte [rsi]\n";
			else				 out << "    add byte [rsi], " << line.count + 1 << '\n';
			break;
		case IR_DEC:
			if (line.count == 0) out << "    dec byte [rsi]\n";
			else				 out << "    sub byte [rsi], " << line.count + 1 << '\n';
			break;
		case IR_LEFT:
			if (line.count == 0) out << "    dec rsi\n";
			else				 out << "    sub rsi, " << line.count + 1 << '\n';
			break;
		case IR_RIGHT:
			if (line.count == 0) out << "    inc rsi\n";
			else				 out << "    add rsi, " << line.count + 1 << '\n';
			break;
		case IR_O:
			out << "    sub rsp, 32\n"
				<< "    call _out\n"
				<< "    add rsp, 32\n";

			//out << "    mov rcx, [rel hStdOut]\n"
			//	<< "    mov rdx, rsi\n"				// lpBuffer
			//	<< "    mov r8, 1\n"				// nNumberOfBytesToWrite
			//	<< "    xor r9, r9\n"				// lpNumberOfBytesWritten (NULL)
			//	<< "    sub rsp, 40\n"				// Shadow space
			//	<< "    mov qword [rsp + 32], 0\n"	// lpOverlapped (NULL)
			//	<< "    call WriteFile\n"
			//	<< "    add rsp, 40\n";				// Clean up stack

			//out << "    movzx rcx, byte [rsi]\n" << "    call putchar\n";
			break;
		case IR_I:
			out << "    sub rsp, 32\n"
				<< "    call _in\n"
				<< "    add rsp, 32\n";

			//out << "    mov rcx, [rel hStdIn]\n"
			//	<< "    mov rdx, rsi\n"					// lpBuffer
			//	<< "    mov r8, 1\n"					// nNumberOfBytesToRead
			//	<< "    lea r9, [rel bytes_read]\n"		// lpNumberOfBytesRead
			//	<< "    sub rsp, 40\n"					// Shadow space
			//	<< "    mov qword [rsp + 32], 0\n"		// lpOverlapped(NULL)
			//	<< "    call ReadFile\n"
			//	<< "    add rsp, 40\n";					// Clean up stack
			//out << "    call getchar\n" << "    mov [rsi], al\n";
			break;
		case IR_GOTO:
			out << "    sub rsp, 32\n" << "    call " << ir.names.at(line.ID) << '\n' << "    add rsp, 32\n";
			break;
		case IR_RET:
			out << "    ret\n";
			break;
		case IR_JZ:
			out << "    cmp byte [rsi], 0\n" << "    je " << ir.names.at(line.ID) << '\n';
			break;
		case IR_JMP:
			out << "    jmp " << ir.names.at(line.ID) << '\n';
			break;
		case IR_LOOP:
			out << ir.names.at(line.ID) << ":\n";
			break;
		}
	}

	//out << "\n    mov eax, 0\n" << "    ret\n";
	out << "    xor rcx, rcx\n" << "    call ExitProcess\n";

	// nasm -f win64 code.asm -o code.obj
	// link /ENTRY:_start /SUBSYSTEM:CONSOLE /NODEFAULTLIB code.obj kernel32.lib
	// C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\link
}






bool Compiler::ParseArgs(Program& p, const std::vector<std::string>& args)
{
	for (size_t i = 1; i < args.size(); i++)
	{
		const std::string& arg = args[i];
		if (!arg.starts_with('-'))
			p.tgts.push_back(arg);
		else if (arg == "-h" || arg == "-help") {
			std::cout << ReadFile("Res/help.txt");
			return false;
		}
		else if (arg == "-o" || arg == "-out" || arg == "--out")
		{
			if (i == args.size() - 1) {
				std::cout << "Specify output filename after flag " << arg << '\n';
				return false;
			}
			p.outputPath = args[++i];
		}
		else if (arg == "-keep" || arg == "--keep")
			p.inter = Program::OBJ;
		else if (arg == "-all" || arg == "--all")
			p.inter = Program::ALL;
		else if (arg == "-i" || arg == "-inter" || arg == "--inter")
		{
			if (i == args.size() - 1) {
				std::cout << "Specify intermediate output folder after flag " << arg << '\n';
				return false;
			}
			p.interPath = args[++i];
		}
		else if (arg == "-nopt" || arg == "--nopt")
			p.optimize = false;
		else if (arg == "-phase" || arg == "--phase")
		{
			if (i == args.size() - 1) {
				std::cout << "Specify target phase after flag " << arg << " (tokenize, parse, analyze, optimize, intermediate)\n";
				return false;
			}
			else if (args[++i] == "tokenize")
				p.tgtPhase = Program::TOKEN;
			else if (args[i] == "parse")
				p.tgtPhase = Program::PARSE;
			else if (args[i] == "analyze")
				p.tgtPhase = Program::ANAL;
			else if (args[i] == "optimize")
				p.tgtPhase = Program::OPT;
			else if (args[i] == "intermediate")
				p.tgtPhase = Program::INTER;
			else {
				std::cout << "Unrecognized compilation phase specified " << args[++i] << '\n';
				return false;
			}
		}
		else if (arg == "-m" || arg == "-main" || arg == "--main")
		{
			if (i == args.size() - 1) {
				std::cout << "Specify main file after flag " << arg << '\n';
				return false;
			}
			i++;
			p.main = args[i];
		}
	}

	return true;
}

u32 RunCommand(const std::wstring& command)
{
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION pi;

	auto flag = NORMAL_PRIORITY_CLASS;

	if (!CreateProcessW(NULL, const_cast<wchar_t*>(command.c_str()),
					    NULL, NULL, FALSE, flag, NULL, NULL, &si, &pi))
	{
		std::cerr << "CreateProcess failed: " << GetLastError() << std::endl;
		return -1;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD exitCode = 0;
	GetExitCodeProcess(pi.hProcess, &exitCode);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return exitCode;
}

int Compiler::Compile(const std::vector<std::string>& args)
{
	Program p;
	if (!ParseArgs(p, args))
		return 1;

	return Compile(p);
}

int Compiler::_Compile(Program& p, const std::string& cmd1, const std::string& cmd2, const std::string& cmd3)
{
	auto error = p.Validate();
	if (error)
	{
		std::cout << error.value();
		return -1;
	}

	{
		std::string cmd = "cmd /c "s + cmd1;
		int reqEC = RunCommand(std::wstring(cmd.begin(), cmd.end()));
		if (reqEC != 0)
			return reqEC;
	}

	stdc::nanoseconds total = 0ns;
	std::vector<fs::path> asmPaths;
	for (const fs::path& path : p.tgts)
	{
		std::cout << "Compiling file " << path << '\n';
		const fs::path ipath = p.GetIntermediatePath(path);
		const std::string iname = path.stem().string();

		// TOKENIZATION

		stdc::time_point start = stdc::clock::now();

		//auto expTokens = Compiler::Tokenize(fs::path("Res/Code.bf"));
		auto expTokens = Compiler::Tokenize(path);

		stdc::time_point end = stdc::clock::now();
		total += end - start;
		std::cout << "Tokenization done in: "; print_time(std::cout, end - start) << '\n';

		if (!expTokens.success())
		{
			std::cout << Terminal::TEXT_F_BRED << expTokens._getU() << Terminal::TEXT_RESET << '\n';
			return 1;
		}

		auto& tokens = expTokens._getE();
		if (p.inter == Program::ALL)
		{
			std::ofstream out{ ipath / (iname + "_tokens.txt") };
			out << tokens;
		}


		// PARSING

		start = stdc::clock::now();

		auto expParse = Compiler::Parse(std::move(tokens));

		end = stdc::clock::now();
		total += end - start;
		std::cout << "Parsing done in: "; print_time(std::cout, end - start) << '\n';

		if (!expParse.success())
		{
			std::cout << Terminal::TEXT_F_BRED << expParse._getU() << Terminal::TEXT_RESET << '\n';
			return 1;
		}

		auto& ast = expParse._getE();
		if (p.inter == Program::ALL)
		{
			std::ofstream out{ ipath / (iname + "_AST.txt") };
			out << ast;
		}


		// ANALYZING

		start = stdc::clock::now();

		auto expAnalyze = Compiler::Analyze(ast);

		end = stdc::clock::now();
		total += end - start;
		std::cout << "Analyzing done in: "; print_time(std::cout, end - start) << '\n';

		if (expAnalyze.has_value())
		{
			std::cout << Terminal::TEXT_F_BRED << expAnalyze.value() << Terminal::TEXT_RESET << '\n';
			return 1;
		}

		bool main = p.IsMainFile(path, ast);

		// OPTIMIZING

		if (p.optimize)
		{
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
			std::cout << "  reduced size: " << ((f64)optimizedSize / initialSize * 100) << "%\n";

			if (p.inter == Program::ALL)
			{
				std::ofstream out{ ipath / (iname + "_ASToptimized.txt") };
				out << ast;
			}
		}


		// IR

		start = stdc::clock::now();

		auto ir = Compiler::Intermediate(std::move(ast));

		end = stdc::clock::now();
		total += end - start;
		std::cout << "IRC done in: "; print_time(std::cout, end - start) << '\n';

		if (p.inter == Program::ALL)
		{
			std::ofstream out{ ipath / (iname + "_IR.txt") };
			out << ir;
		}


		// ASM

		start = stdc::clock::now();

		fs::path pasm = ipath / (iname + ".asm");
		asmPaths.push_back(pasm);
		std::ofstream out{ pasm };
		Compiler::ToASM_AMDWin64(ir, out, main);
		out.close();

		end = stdc::clock::now();
		total += end - start;
		std::cout << "ASM emitted in: "; print_time(std::cout, end - start) << '\n';

		std::cout << '\n';
	}


	// EXE

	stdc::time_point start = stdc::clock::now();

	
	{
		std::stringstream ss;
		ss << "cmd /c " << cmd2 << ' ';
		for (const fs::path& tgt : asmPaths) {
			fs::path pobj = (tgt.parent_path() / tgt.stem()).string() + ".obj";
			ss << tgt << " " << pobj << " ";
		}
		ss << "&& " << cmd3 << ' ' << p.outputPath << " \"";
		for (const fs::path& tgt : asmPaths) {
			fs::path path = (tgt.parent_path() / tgt.stem()).string() + ".obj";
			ss << path << " ";
		}
		ss << '\"';
		std::string command = ss.str();

		u32 asslinkEC = RunCommand(std::wstring(command.begin(), command.end()));
		if (asslinkEC != 0)
			return asslinkEC;
	}


	stdc::time_point end = stdc::clock::now();
	auto external = end - start;
	std::cout << "Executable created in: "; print_time(std::cout, end - start) << '\n';

	/*int result = system(command.c_str());
	if (result != 0) {
		std::cerr << "Build failed with error code: " << result << '\n';
		return 1;
	}*/

	std::cout << "Compilation done in: "; print_time(std::cout, total + external) << " ("; print_time(std::cout, total) << ")\n";




	if (p.inter != Program::ALL)
	{
		std::error_code ec;
		for (const auto& pasm : asmPaths) {
			bool res = fs::remove(pasm, ec);
			if (ec) {
				std::cout << Terminal::TEXT_F_BRED << "Failed to remove " << pasm
					      << ", cause:\n" << Terminal::TEXT_RESET << ec.message() << '\n';
				return ec.value();
			}
		}
	}
	if (p.inter == Program::NONE)
	{
		std::error_code ec;

		for (const auto& tgt : asmPaths) {
			fs::path pobj = (tgt.parent_path() / tgt.stem()).string() + ".obj";
			bool res = fs::remove(pobj, ec);
			if (ec) {
				std::cout << Terminal::TEXT_F_BRED << "Failed to remove " << pobj
						  << ", cause:\n" << Terminal::TEXT_RESET << ec.message() << '\n';
				return ec.value();
			}
		}
	}

	return 0;
}

int Compiler::Compile(Program& p)
{
	return _Compile(p, "CheckRequirements.bat", "Assemble.bat", "LinkObj.bat");
}






//#pragma warning(push)
//#pragma warning(disable : 4146 4996 4244 4267 4624)
//#include <llvm/IR/IRBuilder.h>
//#include <llvm/IR/LLVMContext.h>
//#include <llvm/IR/Module.h>
//#include <llvm/IR/Verifier.h>
//#include <llvm/Support/TargetSelect.h>
//#include <llvm/ExecutionEngine/Orc/LLJIT.h>
//#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
//#include <llvm/Support/Error.h>
//#include <llvm/Support/raw_ostream.h>
//#include <llvm/IRReader/IRReader.h>
//#include <llvm/Support/SourceMgr.h>
//#pragma warning(pop)
//
//void Compiler::ToLLVM(const IR& ir)
//{
//	llvm::LLVMContext ctx;
//	llvm::SMDiagnostic err;
//	llvm::Module mod{ "Module", ctx };
//	//llvm::IRBuilder builder{ };
//
//	auto i8Ty = llvm::Type::getInt8Ty(ctx);
//	auto i32Ty = llvm::Type::getInt32Ty(ctx);
//	auto tapeTy = llvm::ArrayType::get(i8Ty, BF_MEMSIZE);
//
//	auto globalTape = new llvm::GlobalVariable(
//		mod,
//		tapeTy,
//		false,
//		llvm::GlobalValue::ExternalLinkage,
//		llvm::Constant::getNullValue(tapeTy),
//		"tape"
//	);
//
//	// int ptr = 0;
//	/*auto ptr = builder.CreateAlloca(i32Ty, nullptr, "ptr");
//	builder.CreateStore(llvm::ConstantInt::get(i32Ty, 0), ptr);*/
//
//
//	mod.print(llvm::outs(), nullptr);  // Prints LLVM IR to stdout
//}

//extern "C" {
//#pragma warning(push)
//#pragma warning(disable: 4996)
//#include "tinycc/libtcc.h"
//#pragma warning(pop)
//}
//
//void Compiler::ToTCC(const IR& ir)
//{
//	TCCState* s = tcc_new();
//	if (!s) {
//		std::cerr << "Could not create TCC state\n";
//		return;
//	}
//
//	tcc_set_output_type(s, TCC_OUTPUT_MEMORY);
//
//	const char* code =
//		"int foo() { return 42; }";
//
//	if (tcc_compile_string(s, code) == -1) {
//		std::cerr << "Compilation failed\n";
//		tcc_delete(s);
//		return;
//	}
//
//	tcc_relocate(s, TCC_RELOCATE_AUTO);
//	int (*foo)() = (int (*)())tcc_get_symbol(s, "foo");
//	std::cout << "Result: " << foo() << "\n";
//
//	tcc_delete(s);
//	return;
//}
