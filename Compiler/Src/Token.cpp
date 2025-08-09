#include "Token.h"


namespace BFC
{
	// TTYPE
	const hmap<TType, std::string> Token::ToString = {
			{ T_INC, "INCREMENT" },
			{ T_DEC, "DECREMENT" },
			{ T_LEFT, "LEFT" },
			{ T_RIGHT, "RIGHT" },
			{ T_LOOPS, "LOOP_START" },
			{ T_LOOPE, "LOOP_END" },
			{ T_I, "INPUT" },
			{ T_O, "OUTPUT" },
			{ T_LABEL, "LABEL" },
			{ T_GOTO, "GOTO" },
			{ T_RETURN, "RETURN" },
	};
	const hmap<TType, char> Token::ToSymbol = {
			{ T_INC, '+' },
			{ T_DEC, '-' },
			{ T_LEFT, '<' },
			{ T_RIGHT, '>' },
			{ T_LOOPS, '[' },
			{ T_LOOPE, ']' },
			{ T_O, '.' },
			{ T_I, ',' },
	};
	const hmap<char, TType> Token::ToType = {
			{ '+', T_INC },
			{ '-', T_DEC },
			{ '<', T_LEFT },
			{ '>', T_RIGHT },
			{ '[', T_LOOPS },
			{ ']', T_LOOPE },
			{ '.', T_O },
			{ ',', T_I },
	};
	std::ostream& operator<<(std::ostream& out, const TType& token)
	{
		return out << Token::ToString.at(token);
	}


	// TOKEN
	Token::Token(const TType type, const u8 count, const u32 id)
	{
		this->type = type;
		this->ID = id;
		this->count = count;
	}
	Token::Token(const Token& other)
	{
		this->type = other.type;
		this->ID = other.ID;
		this->count = other.count;
	}
	std::ostream& operator<<(std::ostream& out, const Token& token)
	{
		return out << (TType)token.type << " id " << token.ID << " cnt " << to<u32>(token.count);
	}


	// TOKENIZE RESULT
	void TokenizeResult::AddToken(const TType type)
	{
		auto last = tokens.rbegin();
		if (last != tokens.rend() && last->type == type && last->count < MAX_COUNT)
			last->count++;
		else
		{
			Token token(type);
			tokens.push_back(token);
		}
	}
	void TokenizeResult::AddToken(const TType type, const u32 row, const u32 col)
	{
		// since this function is being used for loops and returns, it's unlikely the compression will be relevant
		auto last = tokens.rbegin();
		if (last != tokens.rend() && last->type == type && last->count < MAX_COUNT)
		{
			// must create a new node for tokens that start or end on a different line, to output correct position information in warnings
			// same applies for ' '
			if (row == positions.at(last->ID).first && col == positions.at(last->ID).second + last->count + 1)
			{
				last->count++;
				return;
			}
		}

		Token token = { type, 0, NextID++ };
		tokens.push_back(token);
		positions.insert({ token.ID, { row, col } });
	}
	void TokenizeResult::AddToken(const TType type/*, const u32 row, const u32 col*/, const std::string& symbol)
	{
		// to know where a symbol is we must allow multiple locations for the same ID
		/*auto last = tokens.rbegin();
		if (last != tokens.rend() && last->type == type)
			last->increment();*/

		u32 id;
		if (symbolsS.contains(symbol))
		{
			id = symbolsS.at(symbol);
			//if (position.at(id).first != posi)
		}
		else
		{
			id = NextID++;
			symbolsI.insert({ id, symbol });
			symbolsS.insert({ symbolsI.at(id), id });
			//position.insert({ id, { row, col } });
		}

		Token token = { type, 0, id };
		tokens.push_back(token);
	}

	void TokenizeResult::AddExtern(const std::string& symbol)
	{
		externs.insert(symbol);
	}
	void TokenizeResult::AddExport(const std::string& symbol)
	{
		exports.insert(symbol);
	}

	

	// TOKENIZE RESULT ITERATOR
	TokenizeResult::CIterator& TokenizeResult::CIterator::operator++()
	{
		if (count == it->count)
		{
			++it;
			count = 0;
		}
		else
			count++;
		return *this;
	}
	TokenizeResult::CIterator& TokenizeResult::CIterator::operator--()
	{
		if (count == 0)
		{
			--it;
			count = it->count;
		}
		else
			count--;
		return *this;
	}
	TokenizeResult::CIterator& TokenizeResult::CIterator::operator++(int)
	{
		auto& res = *this;
		operator++();
		return res;
	}
	TokenizeResult::CIterator& TokenizeResult::CIterator::operator--(int)
	{
		auto& res = *this;
		operator--();
		return res;
	}
	bool TokenizeResult::CIterator::operator!=(const CIterator& other) const
	{
		return it != other.it || count != other.count;
	}
	bool TokenizeResult::CIterator::operator==(const CIterator& other) const
	{
		return !(*this != other);
	}

	std::ostream& operator<<(std::ostream& out, const TokenizeResult& tokens)
	{
		int indent = 0;
		for (auto it = tokens.cbegin(); it != tokens.cend(); it.ForceNext()) {
			const auto& [token, count] = *it;

			if (token.type == T_LOOPE)
				indent = std::max(indent - 1, 0);
			if (token.type != T_LABEL)
				out << std::string(indent, ' ');

			out << token;

			if (Token::IsMapped((TType)token.type))
				out << ' ' << tokens.symbolsI.at(token.ID);
			else if (token.type == T_LOOPS) {
				indent++;
				out << ' ' << (tokens.positions.at(token.ID) + coord<u32>{ 0, count });
			}
			else if (token.type == T_LOOPE)
				out << ' ' << tokens.positions.at(token.ID);
			out << '\n';
		}
		return out;
	}
}

