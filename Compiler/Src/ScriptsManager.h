#pragma once

#include <string>
#include <vector>
#include <filesystem>


struct ScriptExecutionResult
{
	bool FailedExecution() const { return ExecErrorCode != 0; }

	/// Equal to 0 if the script was executed successfully
	unsigned int ExecErrorCode;
	/// Equal to the exit value of the script
	unsigned int ExitCode;
	/// Script output
	std::string Output;
};


class ScriptsManager
{
public:
	static std::string CheckRequirementsScript, AssembleScript, LinkScript;

	static ScriptExecutionResult CheckRequirements();
	static ScriptExecutionResult LinkAndAssemble(const std::vector<std::filesystem::path>& asmPaths, const std::filesystem::path& outputPath);

private:
	static ScriptExecutionResult RunCommand(const std::string& command);
};