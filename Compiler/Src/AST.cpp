#include "AST.h"

OpType OpFromTType(const TType& type)
{
	switch (type)
	{
	case T_INC: return O_INC;
	case T_DEC: return O_DEC;
	case T_LEFT: return O_LEFT;
	case T_RIGHT: return O_RIGHT;
	case T_I: return O_I;
	case T_O: return O_O;
	default: return O_NONE;
	}
}

const hmap<OpType, std::string> Operation::ToString = {
		{ O_NONE, "NONE" },
		{ O_INC, "INCREMENT" },
		{ O_DEC, "DECREMENT" },
		{ O_LEFT, "LEFT" },
		{ O_RIGHT, "RIGHT" },
		{ O_I, "INPUT" },
		{ O_O, "OUTPUT" },
};
const hmap<OpType, char> Operation::ToSymbol = {
		{ O_INC, '+'},
		{ O_DEC, '-'},
		{ O_LEFT, '<'},
		{ O_RIGHT, '>'},
		{ O_I, ','},
		{ O_O, '.'},
};




void PrintStatement(const Stmt& s, std::ostream& out, const TranslationUnit& tu, int indent)
{
	out << std::string(indent, ' ');
	if (std::holds_alternative<Goto>(s.value))
	{
		const Goto& g = std::get<Goto>(s.value);
		out << "GOTO id " << g.ID << " cnt " << g.count << ' ' << tu.symbolsI.at(g.ID);
	}
	else if (std::holds_alternative<Return>(s.value))
	{
		const Return& r = std::get<Return>(s.value);
		out << "RETURN";
	}
	else if (std::holds_alternative<Operation>(s.value))
	{
		const Operation& o = std::get<Operation>(s.value);
		out << Operation::ToString.at((OpType)o.type) << " cnt " << o.count;
	}
	else
	{
		const Loop& lo = std::get<Loop>(s.value);
		out << "LOOP_START\n";
		for (const Stmt& i : lo.body)
		{
			PrintStatement(i, out, tu, indent + 1);
			out << '\n';
		}
		out << std::string(indent, ' ') << "LOOP_END";
	}
}

std::ostream& operator<<(std::ostream& out, const Loop& lo)
{
	out << "[\n";
	for (const auto& i : lo.body)
		out << i << '\n';
	out << "]";
	return out;
}
std::ostream& operator<<(std::ostream& out, const Operation& o)
{
	return out << Operation::ToString.at((OpType)o.type) << " cnt " << o.count;
}
std::ostream& operator<<(std::ostream& out, const Return& r)
{
	return out << "RETURN";
}
std::ostream& operator<<(std::ostream& out, const Goto& g)
{
	return out << "GOTO ID " << g.ID << " cnt " << g.count;
}
std::ostream& operator<<(std::ostream& out, const Stmt& s)
{
	if (std::holds_alternative<Goto>(s.value))
		return out << std::get<Goto>(s.value);
	else if (std::holds_alternative<Return>(s.value))
		return out << std::get<Return>(s.value);
	else if (std::holds_alternative<Operation>(s.value))
		return out << std::get<Operation>(s.value);
	return out << std::get<Loop>(s.value);
}
std::ostream& operator<<(std::ostream& out, const Label& la)
{
	out << la.ID << '\n';
	for (const auto& i : la.body)
		out << i << '\n';
	return out;
}
std::ostream& operator<<(std::ostream& out, const Decl& d)
{
	return out << d.label;
}
std::ostream& operator<<(std::ostream& out, const BlockItem& bi)
{
	if (std::holds_alternative<Stmt>(bi.value))
		return out << std::get<Stmt>(bi.value);
	return out << std::get<Decl>(bi.value);
}
std::ostream& operator<<(std::ostream& out, const Block& b)
{
	for (const auto& i : b.items)
		out << i << '\n';
	return out;
}
std::ostream& operator<<(std::ostream& out, const TranslationUnit& tu)
{
	for (const BlockItem& bi : tu.body.items)
	{
		if (std::holds_alternative<Stmt>(bi.value))
		{
			const Stmt& s = std::get<Stmt>(bi.value);
			PrintStatement(s, out, tu, 0);
			out << '\n';
		}
		else
		{
			const Decl& d = std::get<Decl>(bi.value);
			const Label& la = d.label;
			out << "LABEL id " << la.ID << ' ' << tu.symbolsI.at(la.ID) << '\n';
			for (const Stmt& s : la.body)
			{
				PrintStatement(s, out, tu, 1);
				out << '\n';
			}
		}
	}
	return out;
}
