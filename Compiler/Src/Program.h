#pragma once
#include "Utility.h"
#include "AST.h"
#include "CompilerError.h"


struct Program
{
	enum InterOp { NONE, OBJ, ALL };
	enum Target  { TOKEN, PARSE, ANAL, OPT, INTER, FULL };

	bool optimize = true;
	InterOp inter = NONE;
	fs::path interPath;
	Target tgtPhase = FULL;
	fs::path outputPath;
	std::vector<fs::path> tgts;
	fs::path main;

	inline fs::path GetIntermediatePath(const fs::path& file) const {
		return (interPath.empty() ? file.parent_path() : interPath);
	}
	inline bool IsMainFile(const fs::path& file, const TU& tu) const {
		if (main.empty())
			return tgts.size() == 1 || file == tgts[0];
		else
			return main == file;
	}

	CompilerError Validate();
};