#include "AST.h"

OpType OpFromTType(const TType& type)
{
	switch (type)
	{
	case INC: return OpType::INC;
	case DEC: return OpType::DEC;
	case LEFT: return OpType::LEFT;
	case RIGHT: return OpType::RIGHT;
	case I: return OpType::I;
	case O: return OpType::O;
	default: return OpType::NONE;
	}
}

const hmap<OpType, std::string> Operation::ToString = {
		{ OpType::NONE, "NONE" },
		{ OpType::INC, "INCREMENT" },
		{ OpType::DEC, "DECREMENT" },
		{ OpType::LEFT, "LEFT" },
		{ OpType::RIGHT, "RIGHT" },
		{ OpType::I, "INPUT" },
		{ OpType::O, "OUTPUT" },
};
const hmap<OpType, char> Operation::ToSymbol = {
		{ OpType::INC, '+'},
		{ OpType::DEC, '-'},
		{ OpType::LEFT, '<'},
		{ OpType::RIGHT, '>'},
		{ OpType::I, ','},
		{ OpType::O, '.'},
};

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
	return out << Operation::ToString.at(o.type);
}
std::ostream& operator<<(std::ostream& out, const Return& r)
{
	return out << "RETURN";
}
std::ostream& operator<<(std::ostream& out, const Goto& g)
{
	return out << g.name;
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
	out << la.name << '\n';
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
	return out << tu.body;
}

//MERGE IDENTICAL CONSECUTIVE NODE AND REDUCE SPACE CPLXTY