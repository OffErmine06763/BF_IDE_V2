#include "pch.h"
#include "CppUnitTest.h"
#include <Compiler.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CompilerTests
{
	TEST_CLASS(CompilerTests)
	{
	public:
		TEST_METHOD(TestTokenize)
		{
			auto res = Compiler::Tokenize("+++++++++++++++++-[[+]]\n//]\n<< < >> > <>\n//<><>\nmain:	main//+"s);
			Assert::IsTrue(res.success(), wstring("The provided code is valid, however:\n"s + (res.success() ? "" : res.getU().value())).c_str());

			std::string recon = Reconstruct(res.getE().value());
			std::string strip = Strip("+++++++++++++++++-[[+]]\n//]\n<< < >> > <>\n//<><>\nmain:	main//+"s);
			Assert::AreEqual(strip, recon, L"The generated tokens do not match the given code");

			res = Compiler::Tokenize("+++d;+++"s);
			Assert::IsTrue(!res.success(), wstring("The provided code has an invalid char ';', however tokenization succeeded"s).c_str());
			Assert::AreEqual(Compiler::GetUnreconTokenError(';', 0, 4), res.getU().value());

			res = Compiler::Tokenize("+++d/;+++"s);
			Assert::IsTrue(!res.success(), wstring("The provided code has an invalid comment, however tokenization succeeded"s).c_str());
			Assert::AreEqual(Compiler::GetInvalidCommentError(0, 4), res.getU().value());
		}


		TEST_METHOD(TestParse)
		{
			auto tokens = Compiler::Tokenize("+++++++++++++++++-[[+]]\n//]\n<< < >> > <>\n//<><>\nmain:	main//+"s);
			Assert::IsTrue(tokens.success(), wstring("The provided code is valid, however:\n"s + (tokens.success() ? "" : tokens.getU().value())).c_str());

			auto parse = Compiler::Parse(tokens.getE().value());
			Assert::IsTrue(parse.success(), wstring("The provided code is valid, however:\n"s + (parse.success() ? "" : parse.getU().value())).c_str());

			std::string recon = Reconstruct(parse.getE().value());
			std::string strip = Strip("+++++++++++++++++-[[+]]\n//]\n<< < >> > <>\n//<><>\nmain:	main//+"s);
			Assert::AreEqual(strip, recon, L"The generated AST doesn't match the given code");
		}



	private:
		std::wstring wstring(const std::string& s)
		{
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
		std::string Reconstruct(const TranslationUnit& tu)
		{
			std::string code;

			for (const auto& bi : tu.body.items)
				code.append(_Reconstruct(bi, tu));

			return code;
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
		std::string _Reconstruct(const Loop& l, const TranslationUnit& tu)
		{
			std::string res = "[";
			for (const auto& s : l.body)
				res += _Reconstruct(s, tu);
			res += "]";
			return res;
		}
		std::string _Reconstruct(const Goto& g, const TranslationUnit& tu)
		{
			return tu.symbolsI.at(g.ID);
		}
		std::string _Reconstruct(const Return& g, const TranslationUnit& tu)
		{
			return "ret";
		}
		std::string _Reconstruct(const Operation& g, const TranslationUnit& tu)
		{
			return std::string(g.count + 1, Operation::ToSymbol.at((OpType)g.type));
		}
		std::string _Reconstruct(const Stmt& s, const TranslationUnit& tu)
		{
			if (std::holds_alternative<Goto>(s.value))
				return _Reconstruct(std::get<Goto>(s.value), tu);
			if (std::holds_alternative<Return>(s.value))
				return _Reconstruct(std::get<Return>(s.value), tu);
			if (std::holds_alternative<Operation>(s.value))
				return _Reconstruct(std::get<Operation>(s.value), tu);
			return _Reconstruct(std::get<Loop>(s.value), tu);
		}
		std::string _Reconstruct(const Label& l, const TranslationUnit& tu)
		{
			std::string res = tu.symbolsI.at(l.ID) + ':';
			for (const auto& s : l.body)
				res += _Reconstruct(s, tu);
			return res;
		}
		std::string _Reconstruct(const Decl& d, const TranslationUnit& tu)
		{
			return _Reconstruct(d.label, tu);
		}
		std::string _Reconstruct(const BlockItem& bi, const TranslationUnit& tu)
		{
			if (std::holds_alternative<Stmt>(bi.value))
				return _Reconstruct(std::get<Stmt>(bi.value), tu);
			return _Reconstruct(std::get<Decl>(bi.value), tu);
		}
	};
}
