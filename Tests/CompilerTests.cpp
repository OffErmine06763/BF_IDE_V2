#include "pch.h"
#define TEST
#include <Compiler.h>

#define NOMINMAX
#include <Windows.h>

#include <cassert>

#ifdef TEST_EXE
#define TEST_CLASS(x) class x
#define TEST_METHOD(x) void x()

class Assert
{
public:
	static void IsTrue(bool v, const std::wstring& msg) { std::wcout << v << ' ' << msg << '\n'; }
	static void IsTrue(bool v) { std::wcout << v << '\n'; }
	static void IsFalse(bool v, const std::wstring& msg) { std::wcout << v << ' ' << msg << '\n'; }
	static void IsFalse(bool v) { std::wcout << v << '\n'; }
	template <typename T>
	static void AreEqual(T a, T b, const std::wstring& msg) { std::cout << a << ' ' << b << ' '; std::wcout << msg << '\n'; }
	template <typename T>
	static void AreEqual(T a, T b) { std::cout << a << ' ' << b << '\n'; }
};

#else
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
#endif

template<> inline std::wstring 
Microsoft::VisualStudio::CppUnitTestFramework::ToString<CompilerError>(const CompilerError& t)
{ RETURN_WIDE_STRING(t); }

namespace Tests
{
	static expected<std::string, u32> GetExecutableOutput(const std::string& command) {
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = NULL;

		// Create pipes for stdout
		HANDLE hStdoutRead, hStdoutWrite;
		if (!CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0))
			return { 1 };

		// Ensure the read handle is not inherited
		SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0);

		// Set up process startup info
		PROCESS_INFORMATION pi;
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		si.hStdError = hStdoutWrite;
		si.hStdOutput = hStdoutWrite;
		si.dwFlags |= STARTF_USESTDHANDLES;
		ZeroMemory(&pi, sizeof(pi));

		// Create the process
		if (!CreateProcess(
			NULL,                   // No module name (use command line)
			const_cast<LPWSTR>(std::wstring(command.begin(), command.end()).c_str()), // Command line
			NULL,                   // Process handle not inheritable
			NULL,                   // Thread handle not inheritable
			TRUE,                   // Set handle inheritance to TRUE
			CREATE_NO_WINDOW,       // Creation flags (no window)
			NULL,                   // Use parent's environment block
			NULL,                   // Use parent's starting directory
			&si,                    // Pointer to STARTUPINFO structure
			&pi                     // Pointer to PROCESS_INFORMATION structure
		)) {
			CloseHandle(hStdoutRead);
			CloseHandle(hStdoutWrite);
			return { 2 };
		}

		// Close the write end of the pipe so we can read from it
		CloseHandle(hStdoutWrite);

		// Read output from the child process
		std::string result;
		constexpr DWORD BUFSIZE = 4096;
		CHAR chBuf[BUFSIZE];
		DWORD dwRead;
		BOOL bSuccess = FALSE;
		HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

		for (;;) {
			bSuccess = ReadFile(hStdoutRead, chBuf, BUFSIZE, &dwRead, NULL);
			if (!bSuccess || dwRead == 0) break;

			result.append(chBuf, dwRead);
		}

		// Wait for the process to complete
		WaitForSingleObject(pi.hProcess, INFINITE);

		// Close handles
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(hStdoutRead);

		return result;
	}

	TEST_CLASS(CompilerTests)
	{
	public:
		TEST_METHOD(TestTokenize)
		{
			auto res = Compiler::Tokenize("+++++++++++++++++-[[+[-]]]\n//]\n<< < >> > <>\n//<><>\nmain:	main//+"s);
			Assert::IsTrue(res.success(), wstring("The provided code is valid, however:\n"s + (res.success() ? "" : res._getU().message)).c_str());

			std::string recon = Reconstruct(res._getE());
			std::string strip = Strip("+++++++++++++++++-[[+[-]]]\n//]\n<< < >> > <>\n//<><>\nmain:	main//+"s);
			Assert::AreEqual(strip, recon, L"The generated tokens do not match the given code");

			res = Compiler::Tokenize("+++d=+++"s);
			Assert::IsFalse(res.success(), wstring("The provided code has an invalid char '=', however tokenization succeeded"s).c_str());
			Assert::AreEqual(Compiler::GetUnreconTokenError('=', 0, 4), res._getU());

			res = Compiler::Tokenize("+++d/;+++"s);
			Assert::IsFalse(res.success(), wstring("The provided code has an invalid comment, however tokenization succeeded"s).c_str());
			Assert::AreEqual(Compiler::GetInvalidCommentError(0, 4), res._getU());
		}

		TEST_METHOD(TestParse)
		{
			auto tokens = Compiler::Tokenize("+++++++++++++++++-[[+[-]]+]\n//]\n<< < >> > <>\n//<><>\nmain:	main//+"s);
			Assert::IsTrue(tokens.success(), wstring("The provided code is valid, however:\n"s + (tokens.success() ? "" : tokens._getU().message)).c_str());

			auto parse = Compiler::Parse(tokens._consumeE());
			Assert::IsTrue(parse.success(), wstring("The provided code is valid, however:\n"s + (parse.success() ? "" : parse._getU().message)).c_str());

			std::string recon = Reconstruct(parse._getE());
			std::string strip = Strip("+++++++++++++++++-[[+[-]]+]\n//]\n<< < >> > <>\n//<><>\nmain:	main//+"s);
			Assert::AreEqual(strip, recon, L"The generated AST doesn't match the given code");



			tokens = Compiler::Tokenize("+[[[+]]-]"s);
			Assert::IsTrue(tokens.success(), wstring("The provided code is valid, however:\n"s + (tokens.success() ? "" : tokens._getU().message)).c_str());

			parse = Compiler::Parse(tokens._consumeE());
			Assert::IsTrue(parse.success(), wstring("The provided code is valid, however:\n"s + (parse.success() ? "" : parse._getU().message)).c_str());

			recon = Reconstruct(parse._getE());
			strip = Strip("+[[[+]]-]"s);
			Assert::AreEqual(strip, recon, L"The generated AST doesn't match the given code");



			tokens = Compiler::Tokenize("+[[[+]]-"s);
			Assert::IsTrue(tokens.success(), wstring("The provided code is valid, however:\n"s + (tokens.success() ? "" : tokens._getU().message)).c_str());

			parse = Compiler::Parse(tokens._consumeE());
			Assert::IsFalse(parse.success(), wstring("The provided code is invalid, however parsing succeeded:\n"s).c_str());
			Assert::AreEqual(Compiler::GetUnmatchedOpenError({ 0, 1 }), parse._getU());

			tokens = Compiler::Tokenize("+[[+]]-]"s);
			Assert::IsTrue(tokens.success(), wstring("The provided code is valid, however:\n"s + (tokens.success() ? "" : tokens._getU().message)).c_str());

			parse = Compiler::Parse(tokens._consumeE());
			Assert::IsFalse(parse.success(), wstring("The provided code is invalid, however parsing succeeded:\n"s).c_str());
			Assert::AreEqual(Compiler::GetUnmatchedCloseError({ 0, 7 }), parse._getU());
		}

		TEST_METHOD(TestOptimize)
		{
			auto tokens = Compiler::Tokenize("[[++<>-]]"s);
			Assert::IsTrue(tokens.success(), wstring("The provided code is valid, however:\n"s + (tokens.success() ? "" : tokens._getU().message)).c_str());

			auto parse = Compiler::Parse(tokens._consumeE());
			Assert::IsTrue(parse.success(), wstring("The provided code is valid, however:\n"s + (parse.success() ? "" : parse._getU().message)).c_str());

			auto ast = parse._getE();
			std::string recon = Reconstruct(ast);
			std::string strip = Strip("[[++<>-]]"s);
			Assert::AreEqual(strip, recon, L"The generated AST doesn't match the given code");

			auto anal = Compiler::Analyze(ast);
			Assert::IsFalse(anal.has_value(), wstring("The provided code is valid, however:\n"s + (anal.has_value() ? anal.value().message : "")).c_str());

			Compiler::Optimize(ast);
			recon = Reconstruct(ast);
			strip = Strip("[+]"s);
			Assert::AreEqual(strip, recon, L"The optimized AST doesn't match the given code");



			tokens = Compiler::Tokenize("[[++]+-]"s);
			Assert::IsTrue(tokens.success(), wstring("The provided code is valid, however:\n"s + (tokens.success() ? "" : tokens._getU().message)).c_str());

			parse = Compiler::Parse(tokens._consumeE());
			Assert::IsTrue(parse.success(), wstring("The provided code is valid, however:\n"s + (parse.success() ? "" : parse._getU().message)).c_str());

			ast = parse._getE();
			recon = Reconstruct(ast);
			strip = Strip("[[++]+-]"s);
			Assert::AreEqual(strip, recon, L"The generated AST doesn't match the given code");

			anal = Compiler::Analyze(ast);
			Assert::IsFalse(anal.has_value(), wstring("The provided code is valid, however:\n"s + (anal.has_value() ? anal.value().message : "")).c_str());

			Compiler::Optimize(ast);
			recon = Reconstruct(ast);
			strip = Strip("[++]"s);
			Assert::AreEqual(strip, recon, L"The optimized AST doesn't match the given code");



			tokens = Compiler::Tokenize("[[++<>-]+-]label:[[++<>-]+-];[[++<>-]+-]"s);
			Assert::IsTrue(tokens.success(), wstring("The provided code is valid, however:\n"s + (tokens.success() ? "" : tokens._getU().message)).c_str());

			parse = Compiler::Parse(tokens._consumeE());
			Assert::IsTrue(parse.success(), wstring("The provided code is valid, however:\n"s + (parse.success() ? "" : parse._getU().message)).c_str());

			ast = parse._getE();
			recon = Reconstruct(ast);
			strip = Strip("[[++<>-]+-]label:[[++<>-]+-];[[++<>-]+-]"s);
			Assert::AreEqual(strip, recon, L"The generated AST doesn't match the given code");

			anal = Compiler::Analyze(ast);
			Assert::IsFalse(anal.has_value(), wstring("The provided code is valid, however:\n"s + (anal.has_value() ? anal.value().message : "")).c_str());

			Compiler::Optimize(ast);
			recon = Reconstruct(ast);
			strip = Strip("[+]label:[+];"s);
			Assert::AreEqual(strip, recon, L"The optimized AST doesn't match the given code");
		}

		TEST_METHOD(TestIntermediate)
		{
			auto tokens = Compiler::Tokenize("[[++<>-]+-]label:[[++<>-]+-];label label"s);
			Assert::IsTrue(tokens.success(), wstring("The provided code is valid, however:\n"s + (tokens.success() ? "" : tokens._getU().message)).c_str());

			auto parse = Compiler::Parse(tokens._consumeE());
			Assert::IsTrue(parse.success(), wstring("The provided code is valid, however:\n"s + (parse.success() ? "" : parse._getU().message)).c_str());

			auto ast = parse._getE();
			std::string recon = Reconstruct(ast);
			std::string strip = Strip("[[++<>-]+-]label:[[++<>-]+-];label label"s);
			Assert::AreEqual(strip, recon, L"The generated AST doesn't match the given code");

			auto anal = Compiler::Analyze(ast);
			Assert::IsFalse(anal.has_value(), wstring("The provided code is valid, however:\n"s + (anal.has_value() ? anal.value().message : "")).c_str());

			// NO OPTIMIZE

			auto ir = Compiler::Intermediate(std::move(ast));
			recon = Reconstruct(ir);
			std::string exp =
				"LOOP _LOOP_START_1\n"
				"JZ _LOOP_END_11\n"
				"LOOP _LOOP_START_9\n"
				"JZ _LOOP_END_12\n"
				"INC 2\n"
				"LEFT 1\n"
				"RIGHT 1\n"
				"DEC 1\n"
				"JMP _LOOP_START_9\n"
				"LOOP _LOOP_END_12\n"
				"INC 1\n"
				"DEC 1\n"
				"JMP _LOOP_START_1\n"
				"LOOP _LOOP_END_11\n"
				"LABEL label\n"
				"LOOP _LOOP_START_5\n"
				"JZ _LOOP_END_13\n"
				"LOOP _LOOP_START_10\n"
				"JZ _LOOP_END_14\n"
				"INC 2\n"
				"LEFT 1\n"
				"RIGHT 1\n"
				"DEC 1\n"
				"JMP _LOOP_START_10\n"
				"LOOP _LOOP_END_14\n"
				"INC 1\n"
				"DEC 1\n"
				"JMP _LOOP_START_5\n"
				"LOOP _LOOP_END_13\n"
				"RET\n"
				"CALL label 1\n"
				"CALL label 1\n"
				;
			Assert::AreEqual(exp, recon);
		}

		TEST_METHOD(TestLink)
		{
			Program p;
			// Relative to Compiler folder
			p.main = "../Tests/Res/Code_Valid_Link_1.bf";
			p.tgts.push_back("../Tests/Res/Code_Valid_Link_1.bf");
			p.tgts.push_back("../Tests/Res/Code_Valid_Link_2.bf");
			p.outputPath = "../Tests/Generated/Valid_Link.exe";

			std::error_code ec;
#ifdef TEST_EXE
			fs::path original = fs::current_path();
			fs::current_path("../Compiler/", ec);
#else
			fs::path original = fs::current_path();
			fs::current_path("../../Compiler/", ec);
#endif
			if (ec) Assert::IsTrue(false, wstring(ec.message()).c_str());


			Assert::IsFalse(Compiler::Compile(p));

			auto out = GetExecutableOutput(p.outputPath.string());
			Assert::IsTrue(out.success());
			Assert::AreEqual("hi momhi momhi momhi momhi mom"s, out._getE());

			fs::current_path(original, ec);
			if (ec) Assert::IsTrue(false, wstring(ec.message()).c_str());
		}


		TEST_METHOD(TestValid)
		{
			std::vector<std::string> files = {
				"Code_Valid_1.bf"
			};

			for (const std::string& name : files)
			{
				auto file = path(name);

				std::ifstream in(file);
				auto strip = Strip(std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()));
				in.close();
				Assert::IsFalse(strip.empty());

				auto tokens = Compiler::Tokenize(file);
				Assert::IsTrue(tokens.success(), wstring("The provided code is valid, however:\n"s + (tokens.success() ? "" : tokens._getU().message)).c_str());

				auto parse = Compiler::Parse(tokens._consumeE());
				Assert::IsTrue(parse.success(), wstring("The provided code is valid, however:\n"s + (parse.success() ? "" : parse._getU().message)).c_str());
				auto ast = parse._getE();
				auto recon = Reconstruct(ast);

				Assert::AreEqual(strip, recon, L"The generated AST doesn't match the given code");

				auto analize = Compiler::Analyze(ast);
				Assert::IsFalse(analize.has_value(), wstring("The provided code is valid, however:\n"s + (analize.has_value() ? analize.value().message : "")).c_str());
			}
		}

		TEST_METHOD(TestInvalidToken)
		{
			std::vector<std::string> files = {
				"Code_Invalid_Token_2.bf", "Code_Invalid_Token_3.bf"
			};

			for (const std::string& name : files)
			{
				auto file = path(name);

				auto tokens = Compiler::Tokenize(file);
				Assert::IsFalse(tokens.success(), wstring("The provided code is invalid, however tokenization succeeded\n"s).c_str());
			}
		}

		TEST_METHOD(TestInvalidParse)
		{
			std::vector<std::string> files = {
				"Code_Invalid_Parse_1.bf", "Code_Invalid_Parse_2.bf", "Code_Invalid_Parse_3.bf", "Code_Invalid_Parse_4.bf", "Code_Invalid_Parse_5.bf",
			};

			for (const std::string& name : files)
			{
				auto file = path(name);

				auto tokens = Compiler::Tokenize(file);
				Assert::IsTrue(tokens.success(), wstring(name + " is valid, however:\n"s + (tokens.success() ? "" : tokens._getU().message)).c_str());

				auto parse = Compiler::Parse(tokens._consumeE());
				Assert::IsFalse(parse.success(), wstring(name + " is invalid, however parsing succeeded\n"s).c_str());
			}
		}

		TEST_METHOD(TestInvalidAnalyze)
		{
			std::vector<std::string> files = {
				"Code_Invalid_Analyze_1.bf"
			};

			for (const std::string& name : files)
			{
				auto file = path(name);

				auto tokens = Compiler::Tokenize(file);
				Assert::IsTrue(tokens.success(), wstring(name + " is valid, however:\n"s + (tokens.success() ? "" : tokens._getU().message)).c_str());

				auto parse = Compiler::Parse(tokens._consumeE());
				Assert::IsTrue(parse.success(), wstring(name + " is valid, however\n"s + (parse.success() ? "" : parse._getU().message)).c_str());

				auto analyze = Compiler::Analyze(parse._getE());
				Assert::IsTrue(analyze.has_value(), wstring(name + " is invalid, however analyzation succeeded\n"s).c_str());
			}
		}



	private:
		fs::path path(const std::string& str) {
#ifdef TEST_EXE
			return "Res/" + str;
#else
			return "../../Tests/Res/" + str;
#endif
		}

		std::wstring wstring(const std::string& s) { 
			return std::wstring(s.begin(), s.end());
		}

		std::string Reconstruct(const TokenizeResult& tr)
		{
			std::string code;

			for (const auto& [token, count] : tr)
			{
				switch (token.type)
				{
				case T_INC: case T_DEC: case T_I: case T_O: case T_LEFT: case T_RIGHT: case T_LOOPS: case T_LOOPE:
					code.push_back(Token::ToSymbol.at((TType)token.type));
					break;
				case T_LABEL:
					code.append(tr.symbolsI.at(token.ID));
					code.push_back(':');
					break;
				case T_GOTO:
					code.append(tr.symbolsI.at(token.ID));
					break;
				default:
					break;
				}
			}

			return code;
		}
		std::string Reconstruct(const TU& tu)
		{
			std::string code; // TODO: use sstream and operator<<?

			for (const auto& bi : tu.body.items)
				code.append(_Reconstruct(bi, tu));

			return code;
		}
		std::string Reconstruct(const IR& ir)
		{
			std::stringstream ss;
			ss << ir;
			return ss.str();
		}

		std::string Strip(const fs::path& path)
		{
			std::ifstream in(path);
			std::string content = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
			in.close();
			return Strip(content);
		}
		/// The code passed must be valid
		std::string Strip(const std::string& content)
		{
			std::string copy = content;

			size_t offset = 0;
			for (size_t i = 0; i < content.length(); i++)
			{
				const char c = content[i];
				if (c == ' ' || c == '\t' || c == '\n')
					offset++;
				else if (c == '/')
				{
					int pos = std::min(content.find('\n', i), content.length() - 1);
					offset += pos - i + 1;
					i = pos;
				}
				else
					copy[i - offset] = content[i];
			}

			copy = copy.substr(0, content.length() - offset);
			return copy;
		}



	private:
		std::string _Reconstruct(const Loop& l, const TU& tu)
		{
			std::string res = std::string(l.count + 1, '[');
			for (const auto& s : tu.bodies.at(l.ID))
				res += _Reconstruct(s, tu);
			res += std::string(l.count + 1, ']');
			return res;
		}
		std::string _Reconstruct(const Goto& g, const TU& tu)
		{
			return tu.symbolsI.at(g.ID);
		}
		std::string _Reconstruct(const Return& g, const TU& tu)
		{
			return ";";
		}
		std::string _Reconstruct(const Operation& g, const TU& tu)
		{
			return std::string(g.count + 1, Operation::ToSymbol.at((OpType)g.type));
		}
		std::string _Reconstruct(const Stmt& s, const TU& tu)
		{
			return std::visit([&](const auto& el) { return _Reconstruct(el, tu); }, s.value);
		}
		std::string _Reconstruct(const Label& l, const TU& tu)
		{
			std::string res = tu.symbolsI.at(l.ID) + ':';
			for (const auto& s : tu.bodies.at(l.ID))
				res += _Reconstruct(s, tu);
			return res;
		}
		std::string _Reconstruct(const Decl& d, const TU& tu)
		{
			return _Reconstruct(d.label, tu);
		}
		std::string _Reconstruct(const BlockItem& bi, const TU& tu)
		{
			return std::visit([&](const auto& el) { return _Reconstruct(el, tu); }, bi.value);
		}
	};
}

#ifdef TEST_EXE
int main()
{
	Tests::CompilerTests test;
	test.TestLink();
}
#endif