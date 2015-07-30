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
}

std::ostream& operator<<(std::ostream& stream, TokenType token)
{
    if (token >= 0 && token < TOKEN_TYPE_COUNT)
        stream << TOKEN_NAMES[(size_t)token];
	return stream;
}

////////
// Scanner implementation
////////

namespace
{
    Token token_reader_helper(Token token, char initial,
                              std::function<int(void)> get_func,
                              std::function<bool(int)> predicate) noexcept
    {
        std::string content;
        content.push_back(initial);

        int c;
        while (true)
        {
            c = get_func();
            if (c >= 0 && predicate(c))
                content.push_back(static_cast<char>(c));
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
    if (m_next + 1 == m_end)
        return -1;
    return *(m_next + 1);
}

int Scanner::get() noexcept
{
    if (m_next == m_end)
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
        m_textpos.line_offset = 0;
        m_textpos.line++;

        int n = peek();
        if ('\r' == n || '\n' == n)
            get();
        return makeToken(T_NEWLINE);
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
                                   [&]() { return get(); },
                                   [](int c) { return (c != '\r' && c != '\n'); });
    if (isdigit(c))
        return token_reader_helper(makeToken(T_NUMBER), c,
                                   [&]() { return get(); },
                                   [](int c) { return isdigit(c); });
    if (isalpha(c))
        return token_reader_helper(makeToken(T_IDENTIFIER), c,
                                   [&]() { return get(); },
                                   [](int c) { return isalnum(c); });

    return makeToken(T_INVALID);
}

Token Scanner::getNext() noexcept
{
    auto token = this->read();
    return token;
}

////////
// Parsing implementation
////////

//
// TokenBuffer implementation
//

TokenBuffer::TokenBuffer(size_t c, const Token *token_data)
	: m_refcnt(new size_t(1)),
	count(c),
	tokens(new Token[c])
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
	Token *old_tokens = tokens;

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

		operator bool() const {	return ok; }
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

TokenBuffer read_scanner_line(Scanner &scanner)
{
	std::vector<Token> buf;
	while (true)
	{
		Token tok = scanner.getNext();
		// If a eof is encountered, break
		if (tok.type == T_EOF)
			break;
		// Also get rid of comments here
		if (tok.type == T_COMMENT)
			continue;
		// If there was an empty line, continue. Otherwise, return the collected tokens.
		if (tok.type == T_NEWLINE)
			if (buf.size() == 0)
				continue;
			else
				break;
		buf.push_back(std::move(tok));
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