#pragma once
#include "Utility.h"


namespace BFC
{
	struct CompilerError
	{
		enum Code : u32
		{
			NONE = 0,
			HELP = 1,

			GENERIC = 0b00001 << 24,
			TOKEN = 0b00010 << 24,
			PARSE = 0b00100 << 24,
			ANAL = 0b01000 << 24,
			ARGS = 0b10000 << 24,

			TOKEN_COMMENT = TOKEN | 0b0001,
			TOKEN_LABEL_NAME = TOKEN | 0b0010,
			TOKEN_PREPROCESSOR = TOKEN | 0b0011,
			TOKEN_SYMBOL = TOKEN | 0b0100,

			PARSE_UNMATCH_OPEN = PARSE | 0b0001,
			PARSE_UNMATCH_CLOSE = PARSE | 0b0010,
			PARSE_LABEL_IN_LOOP = PARSE | 0b0011,
			PARSE_LABEL_REDEF = PARSE | 0b0100,

			ANAL_LABEL_EXT_DEC = ANAL | 0b0001,

			ARGS_OUT_NAME = ARGS | 0b0001,
			ARGS_INTER_NAME = ARGS | 0b0010,
			ARGS_PHASE = ARGS | 0b0011,
			ARGS_PHASE_UNKN = ARGS | 0b0100,
			ARGS_MAIN_NAME = ARGS | 0b0101,

			ARGS_NO_TGT = ARGS | 0b0110,
			ARGS_MAIN_TGT = ARGS | 0b0111,
			ARGS_MAIN_FOLDER = ARGS | 0b1000,
			ARGS_OUT_FOLDER = ARGS | 0b1001,
			ARGS_INTER_FILE = ARGS | 0b1010,
			ARGS_TGT_NOT_EXISTS = ARGS | 0b1011,
		};


		Code ec;
		std::string message;

		CompilerError(Code ec, const std::string& message) : ec(ec), message(message) {}

		operator bool() const { return ec != NONE; }
	};

	std::ostream& operator<<(std::ostream& out, const CompilerError& ce);
	bool operator==(const CompilerError& a, const CompilerError& b);
	template <typename T> requires std::is_integral_v<T>
	CompilerError::Code operator|(const CompilerError::Code a, const T b) { return to<CompilerError::Code>(a | to<CompilerError::Code>(b)); }
}