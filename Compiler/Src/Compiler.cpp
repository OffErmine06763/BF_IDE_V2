#include "Compiler.h"

const std::unordered_map<TType, std::string> Token::ToString = {
		{ TType::BF, "REGULAR" },
		{ TType::INC, "INCREMENT" },
		{ TType::DEC, "DECREMENT" },
		{ TType::LEFT, "LEFT" },
		{ TType::RIGHT, "RIGHT" },
		{ TType::LOOPS, "LOOP_START" },
		{ TType::LOOPE, "LOOP_END" },
		{ TType::I, "INPUT" },
		{ TType::O, "OUTPUT" },
		{ TType::EXT, "EXTENDED" },
		{ TType::LABEL, "LABEL" },
		{ TType::GOTO, "GOTO" },
		{ TType::INCLUDE, "INCLUDE" },
};
const std::unordered_map<char, TType> Token::ToType = {
		{ '+', TType::INC},
		{ '-', TType::DEC},
		{ '<', TType::LEFT},
		{ '>', TType::RIGHT},
		{ '[', TType::LOOPS},
		{ ']', TType::LOOPE},
		{ '.', TType::O},
		{ ',', TType::I},
};


Token::Token(const TType type, const size_t ind, const std::string& value)
{
	Type = type;
	StartIndex = ind;
	Value = value;
}
Token::Token(const TType type, const size_t ind, const char value)
{
	Type = type;
	StartIndex = ind;
	Value = value;
}
Token::Token(const Token& other)
{
	Type = other.Type;
	StartIndex = other.StartIndex;
	Value = other.Value;
}


std::ostream& operator<<(std::ostream& out, const Token& token)
{
	return out << Token::ToString.at(token.Type) << ' ' << token.Value << " at " << token.StartIndex;
}



expected<std::vector<Token>, std::string> Compiler::Tokenize(const std::string& content)
{
	std::vector<Token> tokens;
	tokens.reserve(content.length()); // in standard bf 1 char = 1 token

	for (size_t i = 0; i < content.length(); i++)
	{
		const char c = content[i];

		if (c == '/')
		{
			if (i < content.length() - 1 && content[i + 1] == '/')
			{
				i = content.find('\n', i);
				if (i == std::string::npos)
					i = content.length() - 1;
				continue;
			}
			else
				return { std::format("Invalid Comment at position {}", i) };
		}
		else if (BF_ALL_S.contains(c))
			tokens.push_back(Token::OfUnchecked(c, i));
		else if (std::isalpha(c))
		{
			size_t start = i;
			do {
				i++;
				if (i >= content.length())
					return { std::format("Invalid Label at position {}", i) };
			} while (std::isalnum(content[i]));

			if (content[i] == ':')
				tokens.push_back({ TType::LABEL, start, content.substr(start, i - start) });
			else
			{
				tokens.push_back({ TType::GOTO, start, content.substr(start, i - start) });
				i--;
			}
			continue;
		}
		else if (c != ' ' && c != '\t' && c != '\n')
			return { std::format("Unrecognized Token '{}' at position {}", c, i)};
	}

	return tokens;
}
