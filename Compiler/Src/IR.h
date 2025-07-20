#pragma once
#include <AST.h>

enum IRType : u8
{
	IR_INC, IR_DEC, IR_LEFT, IR_RIGHT, IR_I, IR_O,
	IR_LABEL, IR_GOTO, IR_RET, IR_JZ, IR_NONE, IR_JMP,
	IR_LOOP
};
IRType IRFromOpType(const OpType& type);

struct IR;

struct IRInstruction
{
	u32 type : FIELD_TYPE;
	/// used for operations
	u32 count : FIELD_COUNT = 0;
	u32 ID : FIELD_ID = INVALID_ID;
};

struct IR
{
	std::vector<IRInstruction> code;

	/// ID to name, for labels (loops are labels)
	hmap<u32, std::string> names;

	u32 NextID = INVALID_ID;
};
std::ostream& operator<<(std::ostream& out, const IR& ir);