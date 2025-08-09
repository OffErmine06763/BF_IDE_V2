#include "IR.h"
#include <cassert>


namespace BFC
{
	IRType IRFromOpType(const OpType& type)
	{
		switch (type)
		{
		case O_INC:   return IR_INC;
		case O_DEC:   return IR_DEC;
		case O_LEFT:  return IR_LEFT;
		case O_RIGHT: return IR_RIGHT;
		case O_I:     return IR_I;
		case O_O:     return IR_O;
		default:
			assert(false);
			return IR_NONE;
		}
	}


	std::ostream& operator<<(std::ostream& out, const IR& ir)
	{
		for (const auto& line : ir.code)
		{
			switch (line.type)
			{
			case IR_INC: out << "INC " << line.count + 1; break;
			case IR_DEC: out << "DEC " << line.count + 1; break;
			case IR_LEFT: out << "LEFT " << line.count + 1; break;
			case IR_RIGHT: out << "RIGHT " << line.count + 1; break;
			case IR_I: out << "IN"; break;
			case IR_O: out << "OUT"; break;
			case IR_GOTO: out << "CALL " << ir.names.at(line.ID) << ' ' << line.count + 1; break;
			case IR_LABEL: out << "LABEL " << ir.names.at(line.ID); break;
			case IR_LOOP: out << "LOOP " << ir.names.at(line.ID); break;
			case IR_JZ: out << "JZ " << ir.names.at(line.ID); break;
			case IR_JMP: out << "JMP " << ir.names.at(line.ID); break;
			case IR_RET: out << "RET"; break;
			default:
				assert(false);
				out << "";
			}
			out << '\n';
		}
		return out;
	}
}