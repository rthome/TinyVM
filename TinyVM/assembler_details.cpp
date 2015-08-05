#include "assembler_details.hpp"

#include <cctype>
#include <functional>
#include <vector>
#include <array>
#include <ostream>

#include "util.hpp"

////////
// Token stringification
////////

namespace
{
	const std::array<const char *, TOKEN_TYPE_COUNT> TOKEN_NAMES
	{
		"T_INVALID",
		"T_EOF",
		"T_DOT",
		"T_COLON",
		"T_POUND",
		"T_COMMENT",
		"T_IDENTIFIER",
		"T_NUMBER",
		"T_LEFTBRACKET",
		"T_RIGHTBRACKET",
		"T_NEWLINE",
	};

	const std::array<const char *, (size_t)HLTokenType::HLTokenTypeCount> HLTOKEN_NAMES
	{
		"Invalid",
		"Eof",    
		"Newline",
		"Specifier", 
		"Label",     
		"Identifier",
		"Number",    
		"Literal",   
		"IndirectionStart", 
		"IndirectionEnd",
	};
}

std::ostream& operator<<(std::ostream& stream, TokenType token)
{
	return (stream << TOKEN_NAMES[(size_t)token]);
}

std::ostream& operator<<(std::ostream& stream, HLTokenType token)
{
	return (stream << HLTOKEN_NAMES[(size_t)token]);
}

////////
// Scanner implementation
////////

namespace
{
	Token token_reader_helper(Token token, char initial,
		std::function<int(void)> peek_func,
		std::function<int(void)> get_func,
		std::function<bool(int)> predicate) noexcept
	{
		std::string content;
		content.push_back(initial);

		int c;
		while (true)
		{
			c = peek_func();
			if (c >= 0 && predicate(c))
				content.push_back(static_cast<char>(get_func()));
			else
				break;
		}

		token.value = std::move(content);
		return token;
	}
}

Scanner::Scanner(const char * const begin, const char * const end) noexcept
	: m_end(end),
	m_next(begin)
{

}

int Scanner::peek() const noexcept
{
	if (m_next + 1 >= m_end)
		return -1;
	return *(m_next + 1);
}

int Scanner::get() noexcept
{
	if (m_next >= m_end)
		return -1;
	m_textpos.line_offset++;
	return *(m_next++);
}

Token Scanner::makeToken(TokenType type) const noexcept
{
	Token token;
	token.type = type;
	token.pos = m_textpos;
	return token;
}

void Scanner::skipWhitespace() noexcept
{
	while (m_next != m_end)
	{
		// pass over space, except newlines - need those
		if (isspace(*m_next) && '\r' != *m_next && '\n' != *m_next)
		{
			get();
		}
		else
			break;
	}
}

Token Scanner::read() noexcept
{
	skipWhitespace();

	auto c = get();
	if (c < 0)
		return makeToken(T_EOF);

	if ('\r' == c || '\n' == c)
	{
		int n = peek();
		if ('\r' == n || '\n' == n)
			get();
		Token newline_tok = makeToken(T_NEWLINE);

		m_textpos.line_offset = 0;
		m_textpos.line++;
		return newline_tok;
	}

	if ('[' == c)
		return makeToken(T_LEFTBRACKET);
	if (']' == c)
		return makeToken(T_RIGHTBRACKET);
	if (':' == c)
		return makeToken(T_COLON);
	if ('.' == c)
		return makeToken(T_DOT);
	if ('#' == c)
		return makeToken(T_POUND);
	if (';' == c)
		return token_reader_helper(makeToken(T_COMMENT), c,
			[&]() { return peek(); },
			[&]() { return get(); },
			[](int c) { return (c != '\r' && c != '\n'); });
	if (isdigit(c))
		return token_reader_helper(makeToken(T_NUMBER), c,
			[&]() { return peek(); },
			[&]() { return get(); },
			[](int c) { return isdigit(c); });
	if (isalpha(c))
		return token_reader_helper(makeToken(T_IDENTIFIER), c,
			[&]() { return peek(); },
			[&]() { return get(); },
			[](int c) { return !isspace(c); });

	return makeToken(T_INVALID);

	return makeToken(T_INVALID);
}

Token Scanner::getNext() noexcept
{
	auto token = this->read();
	return token;
}

//
// TokenAggregator
//

Token TokenAggregator::nextToken() noexcept
{
	if (m_tokenQueue.empty())
		return m_scanner.getNext();
	auto token = m_tokenQueue.front();
	m_tokenQueue.pop();
	return token;
}

TokenAggregator::TokenAggregator(Scanner& scanner) noexcept
	: m_scanner(scanner)
{ }

HLToken TokenAggregator::getNext() noexcept
{
	auto tok = nextToken();
	switch (tok.type)
	{
	// "Special"/Whitespace tokens
	case T_INVALID:
		return HLToken(HLTokenType::Invalid, tok.pos);
	case T_EOF:
		return HLToken(HLTokenType::Eof, tok.pos);
	case T_NEWLINE:
		return HLToken(HLTokenType::Newline, tok.pos);

	// Brackets
	case T_LEFTBRACKET:
		return HLToken(HLTokenType::IndirectionStart, tok.pos);
	case T_RIGHTBRACKET:
		return HLToken(HLTokenType::IndirectionEnd, tok.pos);

	// Simple tokens
	case T_NUMBER:
		return HLToken(HLTokenType::Number, tok.pos, std::stoull(tok.value));

	// Potential "compound" tokens
	case T_COMMENT: {
		// If a comment shows up, just skip it
		return getNext();
	}
	case T_DOT: {
		// A for signifies a specifier
		auto spec_name_tok = nextToken();
		if (spec_name_tok.type == T_IDENTIFIER)
			return HLToken(HLTokenType::Specifier, tok.pos, spec_name_tok.value);
		else
		{
			// ... or not?
			m_tokenQueue.push(spec_name_tok);
			return HLToken(HLTokenType::Invalid, tok.pos, ".");
		}
	}
	case T_IDENTIFIER: {
		// An identifier could be a label, if it is followed by a colon
		auto next_token = nextToken();
		if (next_token.type == T_COLON)
			return HLToken(HLTokenType::Label, tok.pos, tok.value);
		else
		{
			m_tokenQueue.push(next_token);
			return HLToken(HLTokenType::Identifier, tok.pos, tok.value);
		}
	}
	case T_POUND: {
		// A pound should be followed by a number to form a literal
		auto value_token = nextToken();
		if (value_token.type == T_NUMBER)
			return HLToken(HLTokenType::Literal, tok.pos, std::stoull(value_token.value));
		else
		{
			m_tokenQueue.push(value_token);
			return HLToken(HLTokenType::Invalid, tok.pos, "#");
		}
	}

	default:
		return HLToken(HLTokenType::Invalid, tok.pos, tok.value);
	}
}

////////
// Parsing implementation
////////

//
// TokenBuffer implementation
//

TokenBuffer::TokenBuffer(size_t c, const HLToken *token_data)
	: m_refcnt(new size_t(1)),
	count(c),
	tokens(new HLToken[c])
{
	if (token_data != nullptr)
		for (size_t i = 0; i < count; i++)
			tokens[i] = token_data[i];
}

TokenBuffer::TokenBuffer(const TokenBuffer& b)
	: m_refcnt(b.m_refcnt),
	count(b.count),
	tokens(b.tokens)
{
	(*m_refcnt)++;
}

TokenBuffer& TokenBuffer::operator=(const TokenBuffer& b)
{
	// To handle self-assignment, don't change to order of these statements

	size_t *old_refcnt = m_refcnt;
	HLToken *old_tokens = tokens;

	this->m_refcnt = b.m_refcnt;
	this->count = b.count;
	this->tokens = b.tokens;
	(*m_refcnt)++;

	if (--(*old_refcnt) == 0)
	{
		delete old_refcnt;
		delete[] old_tokens;
	}

	return *this;
}

TokenBuffer::~TokenBuffer()
{
	if (--(*m_refcnt) == 0)
	{
		delete m_refcnt;
		delete[] tokens;
	}
}

namespace
{
	//
	// Private parsing helper functions
	//

	struct ExpectResult
	{
		bool ok;

		operator bool() const { return ok; }
		ExpectResult(bool r) : ok(r) { }
	};

	ExpectResult expect(TokenType type, const Token *token, std::string *dst_data = nullptr)
	{
		if (type == token->type)
		{
			if (dst_data != nullptr)
				*dst_data = token->value;
			return false;
		}
		return{ false };
	}

	ExpectResult expect_either(size_t count, const TokenType *types, const Token *token, std::string *dst_data = nullptr)
	{
		bool match = false;
		for (size_t i = 0; i < count; i++)
			if (types[i] == token->type)
			{
				match = true;
				break;
			}
		if (match && dst_data != nullptr)
			*dst_data = token->value;
		return{ match };
	}
}

TokenBuffer read_hltoken_line(TokenAggregator& aggregator)
{
	std::vector<HLToken> buf;
	while (true)
	{
		auto tok = aggregator.getNext();
		// If a eof is encountered, break
		if (tok.type == HLTokenType::Eof)
			break;
		// If there was an empty line, continue. Otherwise, return the collected tokens.
		if (tok.type == HLTokenType::Newline)
			if (buf.size() == 0)
				continue;
			else
				break;
		buf.push_back(tok);
	}
	return{ buf.size(), buf.data() };
}

//
// Parsing api implementation
//

size_t parse_element(TokenBuffer tokens, size_t token_offset, ParsedElement *elem)
{
	return 0;
}