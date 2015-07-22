#include "assembler_details.hpp"

#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <functional>

namespace
{
    Token token_reader_helper(TokenType type, char initial, std::function<int(void)> get_func, std::function<bool(int)> predicate) noexcept
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

        return Token(type, std::move(content));
    }
}

////////
// FileMapping implementation
////////

FileMapping::FileMapping(const char *filename) noexcept
{
    // Open file
    FILE *file = fopen(filename, "rb");
    if (!file)
        return;

    // Get file size
    fseek(file, 0L, SEEK_END);
    size_t size = size_t(ftell(file));
    rewind(file);

    // Allocate memory
    this->m_begin = static_cast<char *>(calloc(size, sizeof(char)));
    if (!m_begin)
    {
        fclose(file);
        return;
    }

    // Read file contents
    this->m_end = m_begin + size;
    fread(m_begin, sizeof(char), size, file);
    fclose(file);
}

FileMapping::~FileMapping() noexcept
{
    free(m_begin);
}

////////
// Scanner Implementation
///////

Scanner::Scanner(const char * const begin, const char * const end) noexcept
    : m_end(end),
      m_next(begin)
{

}

void Scanner::skipWhitespace() noexcept
{
    while (m_next != m_end)
    {
        if (isspace(*m_next))
            ++m_next;
        else
            break;
    }
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
    return *m_next++;
}

Token Scanner::readNumber(char initial) noexcept
{
    return token_reader_helper(T_NUMBER, initial,
                               [&]() { return get(); },
                               [](int c) { return isdigit(c); });
}

Token Scanner::readString(char initial) noexcept
{
    return token_reader_helper(T_STRING, initial,
                               [&]() { return get(); },
                               [](int c) { return isalnum(c); });
}

Token Scanner::readComment(char initial) noexcept
{
    return token_reader_helper(T_COMMENT, initial,
                               [&]() { return get(); },
                               [](int c) { return (c != '\r' && c != '\n'); });
}

Token Scanner::read() noexcept
{
    skipWhitespace();

    auto c = get();
    if (c < 0)
        return Token(T_EOF);

    if ('[' == c)
        return Token(T_LEFTBRACKET);
    if (']' == c)
        return Token(T_RIGHTBRACKET);
    if (':' == c)
        return Token(T_COLON);
    if ('.' == c)
        return Token(T_DOT);
    if (';' == c)
        return readComment(c);
    if (isdigit(c))
        return readNumber(static_cast<char>(c));
    if (isalpha(c))
        return readString(static_cast<char>(c));

    return Token(T_INVALID);
}
