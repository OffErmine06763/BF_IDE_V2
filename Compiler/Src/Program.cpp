#include "Program.h"

CompilerError Program::Validate()
{
	if (tgts.size() == 0)
		return { CompilerError::ARGS_NO_TGT, "Please provide the list of files to compile\n" };
	if (!main.empty() && stdr::find(tgts, main) == tgts.end())
		return { CompilerError::ARGS_MAIN_TGT, "The main file specified is not in the list of files to compile\n" };

	if (outputPath.empty())
	{
		if (tgts.size() == 1)
		{
			const auto& tgt = tgts[0];
			if (fs::is_directory(tgt))
				outputPath = tgt.string() + ".exe";
			else
				outputPath = tgt.parent_path() / (tgt.stem().string() + ".exe");
		}
		else
			outputPath = "out.exe";
	}

	for (size_t i = 0; i < tgts.size(); i++)
	{
		const fs::path& tgt = tgts[i];
		if (!fs::is_directory(tgt))
			continue;
		for (const fs::path& sub : fs::directory_iterator(tgt))
		{
			if (sub.extension() == ".bf" || fs::is_directory(sub))
				tgts.push_back(sub);
		}
		tgts.erase(tgts.begin() + i);
		i--;
	}

	if (fs::exists(outputPath)) {
		if (fs::is_directory(outputPath))
			return { CompilerError::ARGS_OUT_FOLDER, "The output file must be a file" };
	}
	else if (!outputPath.parent_path().empty())
		fs::create_directories(outputPath.parent_path());

	if (!interPath.empty())
	{
		if (fs::exists(interPath)) {
			if (!fs::is_directory(interPath))
				return { CompilerError::ARGS_INTER_FILE, "The intermediate output folder must be a folder" };
		}
		else if(!interPath.parent_path().empty())
			fs::create_directories(interPath);
	}

	for (const fs::path& tgt : tgts)
	{
		if (!fs::exists(tgt))
			return { CompilerError::ARGS_TGT_NOT_EXISTS, "The target \""s + tgt.string() + "\" doen't exist"};
	}

	return { CompilerError::NONE, "" };
}
