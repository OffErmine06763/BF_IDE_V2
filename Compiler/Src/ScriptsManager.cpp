#include "ScriptsManager.h"

#include "Utility.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif


#ifdef _WIN32
std::string
	ScriptsManager::CheckRequirementsScript = "Scripts\\CheckRequirements.bat",
	ScriptsManager::AssembleScript          = "Scripts\\Assemble.bat",
	ScriptsManager::LinkScript              = "Scripts\\LinkObj.bat";
#else
std::string
	ScriptsManager::CheckRequirementsScript = "Scripts\\CheckRequirements.sh",
	ScriptsManager::AssembleScript          = "Scripts\\Assemble.sh",
	ScriptsManager::LinkScript              = "Scripts\\LinkObj.sh";
#endif


ScriptExecutionResult ScriptsManager::CheckRequirements()
{
#ifdef _WIN32
	std::string cmd = "cmd /c "s + CheckRequirementsScript;
#else
	std::string cmd = "./"s + CheckRequirementsScript;
#endif
	return RunCommand(cmd);
}

ScriptExecutionResult ScriptsManager::LinkAndAssemble(const std::vector<std::filesystem::path>& asmPaths, const std::filesystem::path& outputPath)
{
	std::stringstream ss;

#ifdef _WIN32
	ss << "cmd /c " << AssembleScript << ' ';
#else
	ss << "./" << AssembleScript << ' ';
#endif
	for (const fs::path& tgt : asmPaths) {
		fs::path pobj = (tgt.parent_path() / tgt.stem()).string() + ".obj";
		ss << tgt << " " << pobj << " ";
	}
	ss << "&& ";
#ifdef _WIN32
	ss << LinkScript << ' ';
#else
	ss << "./" << LinkScript << ' ';
#endif
	ss << outputPath << " \"";
	for (const fs::path& tgt : asmPaths) {
		fs::path path = (tgt.parent_path() / tgt.stem()).string() + ".obj";
		ss << path << " ";
	}
	ss << '\"';
	std::string command = ss.str();

	ScriptExecutionResult out = RunCommand(command);

#ifdef _WIN32
	size_t mic = out.Output.find("Microsoft (R)");
	size_t nl1 = out.Output.find('\n', mic);
	size_t nl2 = out.Output.find('\n', nl1 + 1);
	size_t nl3 = out.Output.find('\n', nl2 + 1);
	if (mic != std::string::npos && nl3 != std::string::npos)
		out.Output.erase(mic, nl3 - mic + 1);
#endif
	return out;
}


ScriptExecutionResult ScriptsManager::RunCommand(const std::string& command)
{
#ifdef _WIN32
	HANDLE hStdOutRead, hStdOutWrite;
	SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };

	// Create pipe
	if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0))
	{
		return { .ExecErrorCode = GetLastError(), .ExitCode = 0, .Output = "" };
	}

	// Ensure the read handle is not inherited
	SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);

	PROCESS_INFORMATION pi{};
	STARTUPINFOA si{};
	si.cb = sizeof(si);
	si.hStdOutput = hStdOutWrite;
	si.hStdError = hStdOutWrite;
	si.dwFlags |= STARTF_USESTDHANDLES;

	// Create child process
	if (!CreateProcessA(
		nullptr,
		const_cast<char*>(command.c_str()),
		nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
		nullptr, nullptr, &si, &pi))
	{
		CloseHandle(hStdOutRead);
		CloseHandle(hStdOutWrite);
		return { .ExecErrorCode = GetLastError(), .ExitCode = 0, .Output = "" };
	}

	CloseHandle(hStdOutWrite); // parent doesn't need write end

	// Read output
	char buffer[128];
	DWORD bytesRead;
	std::string result;
	while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
		buffer[bytesRead] = '\0';
		result += buffer;
	}

	// Cleanup
	CloseHandle(hStdOutRead);
	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD exitCode = 0;
	GetExitCodeProcess(pi.hProcess, &exitCode);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return { .ExecErrorCode = 0, .ExitCode = exitCode, .Output = result };
#else
	std::array<char, 128> buffer;
	std::string output;

	FILE* pipe = popen(command.c_str(), "r");

	if (!pipe)
	{
		return { .ExecErrorCode = errno, .ExitCode = 0, .Output = "" };
	}

	while (fgets(buffer.data(), buffer.size(), pipe))
	{
		output += buffer.data();
	}

	int status = pclose(pipe);

	return {
		.ExecErrorCode = 0,
		.ExitCode = WEXITSTATUS(status),
		.Output = output
	};
#endif
}