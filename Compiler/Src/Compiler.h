#pragma once
#include <Utility.h>
#include "Token.h"
#include "AST.h"
#include "IR.h"


struct Program
{
	enum InterOp
	{
		NONE, OBJ, ALL
	};
	enum Target
	{
		TOKEN, PARSE, ANAL, OPT, INTER, FULL
	};

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
};


class Compiler
{
public:
	static bool ParseArgs(Program& p, const std::vector<std::string>& args);
	static int Compile(const std::vector<std::string>& args);
	static int Compile(Program& p);


	// Lexical Analyzer / Scanner / Lexer
	static expected<TokenizeResult, std::string> Tokenize(const fs::path& file) {
		std::ifstream in(file);
		std::string content = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
		in.close();
		//auto res = Tokenize(content);
		//prog.files.insert({ file, tr });
		return Tokenize(content);
	}
	static expected<TokenizeResult, std::string> Tokenize(const std::string& content);

	static expected<TU, std::string> Parse(TokenizeResult&& tr);

	static std::optional<std::string> Analyze(const TU& tu);

	static void Optimize(TU& tu);
	
	/// basically go from LOOP { body } to LABEL loop1 body JNZ loop1
	static IR Intermediate(TU&& tu);


	static void ToASM_AMDWin64(const IR& ir, std::ostream& out, bool main);

	// TODO: run (compile and run the code in that block before compiling, used to initialize memory) 

public:
	static std::string GetUnreconTokenError(const char c, const u32 row, const u32 col) {
		return std::format("Unrecognized Token '{}' at position [{}:{}]", c, row, col);
	}
	static std::string GetInvalidCommentError(const u32 row, const u32 col) {
		return std::format("Invalid Comment at position [{}:{}]", row, col);
	}
	static std::string GetUnmatchedOpenError(const coord<u32>& pos) {
		return "Unmatched [ at position "s + pos;
	}
	static std::string GetUnmatchedCloseError(const coord<u32>& pos) {
		return "Unmatched ] at position "s + pos;
	}
	static std::string GetLabelRedefinitionError(const std::string& name) {
		return "Label redefinition: "s + name;
	}

public:
	//static void ToLLVM(const IR& ir);
	// https://chatgpt.com/c/6868f511-6f14-8002-a7ba-51572cb83a2f#:~:text=Hai%20detto%3A-,target%20LLVM,-ChatGPT%20ha%20detto
	// https://chatgpt.com/c/6868f511-6f14-8002-a7ba-51572cb83a2f#:~:text=Hai%20detto%3A-,Add%20JIT%20execution,-ChatGPT%20ha%20detto
	//static void ToTCC(const IR& ir);
};



/* FOR LLVM / TCC

Additional Include Directories
$(ProjectDir)vendor
D:\DEV\Sources\AAA_GitHub\llvm-project\build\include
D:\DEV\Sources\AAA_GitHub\llvm-project\llvm\include

Additional Library Directories
DEBUG:   D:\DEV\Sources\AAA_GitHub\llvm-project\build\Debug\lib
RELEASE: D:\DEV\Sources\AAA_GitHub\llvm-project\build\Release\lib

Additional Dependencies
legacy_stdio_definitions.lib
clangHandleLLVM.lib
LLVMAggressiveInstCombine.lib
LLVMAnalysis.lib
LLVMAsmParser.lib
LLVMAsmPrinter.lib
LLVMBinaryFormat.lib
LLVMBitReader.lib
LLVMBitstreamReader.lib
LLVMBitWriter.lib
LLVMCFGuard.lib
LLVMCFIVerify.lib
LLVMCodeGen.lib
LLVMCodeGenTypes.lib
LLVMCore.lib
LLVMCoroutines.lib
LLVMCoverage.lib
LLVMDebugInfoBTF.lib
LLVMDebugInfoCodeView.lib
LLVMDebuginfod.lib
LLVMDebugInfoDWARF.lib
LLVMDebugInfoGSYM.lib
LLVMDebugInfoLogicalView.lib
LLVMDebugInfoMSF.lib
LLVMDebugInfoPDB.lib
LLVMDemangle.lib
LLVMDiff.lib
LLVMDlltoolDriver.lib
LLVMDWARFLinker.lib
LLVMDWARFLinkerParallel.lib
LLVMDWP.lib
LLVMExecutionEngine.lib
LLVMExegesis.lib
LLVMExegesisX86.lib
LLVMExtensions.lib
LLVMFileCheck.lib
LLVMFrontendHLSL.lib
LLVMFrontendOpenACC.lib
LLVMFrontendOpenMP.lib
LLVMFuzzerCLI.lib
LLVMFuzzMutate.lib
LLVMGlobalISel.lib
LLVMInstCombine.lib
LLVMInstrumentation.lib
LLVMInterfaceStub.lib
LLVMInterpreter.lib
LLVMipo.lib
LLVMIRPrinter.lib
LLVMIRReader.lib
LLVMJITLink.lib
LLVMLibDriver.lib
LLVMLineEditor.lib
LLVMLinker.lib
LLVMLTO.lib
LLVMMC.lib
LLVMMCA.lib
LLVMMCDisassembler.lib
LLVMMCJIT.lib
LLVMMCParser.lib
LLVMMIRParser.lib
LLVMObjCARCOpts.lib
LLVMObjCopy.lib
LLVMObject.lib
LLVMObjectYAML.lib
LLVMOption.lib
LLVMOrcJIT.lib
LLVMOrcShared.lib
LLVMOrcTargetProcess.lib
LLVMPasses.lib
LLVMProfileData.lib
LLVMRemarks.lib
LLVMRuntimeDyld.lib
LLVMScalarOpts.lib
LLVMSelectionDAG.lib
LLVMSupport.lib
LLVMSymbolize.lib
LLVMTableGen.lib
LLVMTableGenCommon.lib
LLVMTarget.lib
LLVMTargetParser.lib
LLVMTextAPI.lib
LLVMTransformUtils.lib
LLVMVectorize.lib
LLVMWindowsDriver.lib
LLVMWindowsManifest.lib
LLVMX86AsmParser.lib
LLVMX86CodeGen.lib
LLVMX86Desc.lib
LLVMX86Disassembler.lib
LLVMX86Info.lib
LLVMX86TargetMCA.lib
LLVMXRay.lib
ntdll.lib


*/