#pragma once
#include <Utility.h>
#include "Token.h"
#include "AST.h"
#include "IR.h"
#include "CompilerError.h"
#include "CompilationParams.h"



namespace BFC
{
	/// For full compilation use Compile.
	/// It's also possibile to manually call every compilation step
	class Compiler
	{
	public:
		static CompilerError Compile(const std::vector<std::string>& args);
		// TODO: how to access the script files when using the compiler as an API without providing the path offset
		static CompilerError Compile(CompilationParams p, const fs::path& offset)
		{
			auto original = fs::current_path();
			fs::current_path(offset);
#ifdef _WIN32
			auto res = _Compile(p, "CheckRequirements.bat", "Assemble.bat", "LinkObj.bat");
#else
			auto res = _Compile(p, "CheckRequirements.sh", "Assemble.sh", "LinkObj.sh");
#endif
			fs::current_path(original);
			return res;
		}
		static CompilerError Compile(CompilationParams p) {
#ifdef _WIN32
			return _Compile(p, "CheckRequirements.bat", "Assemble.bat", "LinkObj.bat");
#else
			return _Compile(p, "CheckRequirements.sh", "Assemble.sh", "LinkObj.sh");
#endif
		}


		/// Lexical Analyzer / Scanner / Lexer
		/// Reads the whole file and converts it into a vector of tokens
		/// Errors potentially returned include unrecognized token, invalid label name, invalid preprocessor directive, invalid start of comment (only one '/')
		static expected<TokenizeResult, CompilerError> Tokenize(const fs::path& file) {
			std::ifstream in(file);
			std::string content = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
			in.close();
			return Tokenize(content);
		}
		/// Converts the given bf code string into a vector of tokens
		static expected<TokenizeResult, CompilerError> Tokenize(const std::string& content);

		/// CONSUMES the tokens to create an AST.
		/// Errors potentially returned include unmatched parenthesis, label redefinition, label definition inside loop, declaration of label marked as extern
		static expected<TU, CompilerError> Parse(TokenizeResult&& tr);

		/// Verify that the code inside the single file is valid.
		/// Checks that every goto is matched by either a label or an extern definition and that every export is defined
		static std::optional<CompilerError> Analyze(const TU& tu);

		/// Recursively removes opposite adjacent operations and nested loops
		static void Optimize(TU& tu);

		/// Converts the tree code representation into a vector of instructions, mostly turning loops into conditional jumps.
		/// basically go from LOOP { body } to LABEL loop1 body JNZ loop1
		static IR Intermediate(TU&& tu);

		/// Output to 'out' the assembly code generated for Windows 64 bit
		static void EmitASM_Win64(const IR& ir, std::ostream& out, bool main);
		/// Output to 'out' the assembly code generated for Linux 64 bit
		static void EmitASM_Linux64(const IR& ir, std::ostream& out, bool main);

	private:
		/// used for testing, as the bat file paths are different
		static CompilerError _Compile(CompilationParams p, const std::string& cmd1, const std::string& cmd2, const std::string& cmd3);

	public:
		static inline CompilerError GetUnreconTokenError(const char c, const u32 row, const u32 col) {
			return { CompilerError::TOKEN_SYMBOL, std::format("Unrecognized Token '{}' at position [{}:{}]", c, row, col) };
		}
		static inline CompilerError GetInvalidCommentError(const u32 row, const u32 col) {
			return { CompilerError::TOKEN_COMMENT, std::format("Invalid Comment at position [{}:{}]", row, col) };
		}
		static inline CompilerError GetInvalidLabelNameError(const u32 row, const u32 col) {
			return { CompilerError::TOKEN_LABEL_NAME, std::format("Labels must start with a letter: [{}:{}]", std::to_string(row), std::to_string(col)) };
		}
		static inline CompilerError GetInvalidPreprocessorError(const u32 row, const u32 col) {
			return { CompilerError::TOKEN_PREPROCESSOR, std::format("Invalid preprocessor directive at position: [{}:{}]", std::to_string(row), std::to_string(col)) };
		}
		static inline CompilerError GetUnmatchedOpenError(const coord<u32>& pos) {
			return { CompilerError::PARSE_UNMATCH_OPEN, "Unmatched [ at position "s + pos };
		}
		static inline CompilerError GetUnmatchedCloseError(const coord<u32>& pos) {
			return { CompilerError::PARSE_UNMATCH_CLOSE, "Unmatched ] at position "s + pos };
		}
		static inline CompilerError GetLabelInLoopError() {
			return { CompilerError::PARSE_LABEL_IN_LOOP, "Cannot declare a label inside a loop\n" };
		}
		static inline CompilerError GetLabelRedefinitionError(const std::string& name) {
			return { CompilerError::PARSE_LABEL_REDEF, "Label redefinition: "s + name };
		}
		static inline CompilerError GetLabelExternDeclaredError(const std::string& name) {
			return { CompilerError::ANAL_LABEL_EXT_DEC , std::format("Label {} declared as extern but defined in the current file", name) };
		}
		static inline CompilerError GetUndefinedLabelError(const std::string& name) {
			return { CompilerError::ANAL_LABEL_EXT_DEC , std::format("Undefined label \"{}\"", name) };
		}
		static inline CompilerError GetOutputFilenameError(const std::string& flag) {
			return { CompilerError::ARGS_OUT_NAME, std::format("Specify output filename after flag {}", flag) };
		}
		static inline CompilerError GetIntermediatePathError(const std::string& flag) {
			return { CompilerError::ARGS_INTER_NAME, std::format("Specify intermediate output folder after flag {}", flag) };
		}
		static inline CompilerError GetPhaseNameError(const std::string& flag) {
			return { CompilerError::ARGS_PHASE, std::format("Specify target phase after flag {} (tokenize, parse, analyze, optimize, intermediate)", flag) };
		}
		static inline CompilerError GetPhaseUnknownError(const std::string& phase) {
			return { CompilerError::ARGS_PHASE_UNKN, std::format("Unrecognized compilation phase specified \"{}\"", phase) };
		}
		static inline CompilerError GetMainNameError(const std::string& flag) {
			return { CompilerError::ARGS_MAIN_NAME, std::format("Specify main file after flag {}", flag) };
		}



	public:
		//static void ToLLVM(const IR& ir);
		// https://chatgpt.com/c/6868f511-6f14-8002-a7ba-51572cb83a2f#:~:text=Hai%20detto%3A-,target%20LLVM,-ChatGPT%20ha%20detto
		// https://chatgpt.com/c/6868f511-6f14-8002-a7ba-51572cb83a2f#:~:text=Hai%20detto%3A-,Add%20JIT%20execution,-ChatGPT%20ha%20detto
		//static void ToTCC(const IR& ir);
	};
}


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

Preprocessor
CONFIG_TCC_STATIC
TCC_IS_NATIVE
TCC_TARGET_X86_64
TCC_TARGET_PE

*/