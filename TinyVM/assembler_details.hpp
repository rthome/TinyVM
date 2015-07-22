#pragma once

#include <cstddef>
#include <string>

enum TokenType
{
    T_INVALID,
    T_EOF,

    T_DOT,
    T_COLON,
    T_COMMENT,
    T_STRING,
    T_NUMBER,
    T_LEFTBRACKET,
    T_RIGHTBRACKET,
};

struct Token
{
    TokenType type = T_INVALID;
    std::string value;

    inline Token(TokenType t) : type(t) { }
    inline Token(TokenType t, std::string&& s) : type(t), value(s) { }
};

class FileMapping
{
    char *m_begin = nullptr;
    char *m_end = nullptr;

public:
    // Disable copying
    FileMapping(const FileMapping&) = delete;
    FileMapping& operator=(const FileMapping&) = delete;

    // Move semantics
    FileMapping(FileMapping&&) = default;
    FileMapping& operator=(FileMapping&&) = default;

    explicit FileMapping(const char *filename) noexcept;
    ~FileMapping() noexcept;

    inline explicit operator bool() const noexcept
    {
        return m_begin != nullptr;
    }

    inline size_t size() const noexcept
    {
        return m_end - m_begin;
    }

    inline const char *begin() const noexcept
    {
        return m_begin;
    }

    inline const char *end() const noexcept
    {
        return  m_end;
    }
};

class Scanner
{
    const char * const m_end  = nullptr;
    const char *m_next = nullptr;

    int peek() const noexcept;
    int get() noexcept;

    void skipWhitespace() noexcept;
    Token readNumber(char initial) noexcept;
    Token readString(char initial) noexcept;
    Token readComment(char initial) noexcept;

public:
    Scanner(Scanner const &) = delete;
    Scanner & operator=(Scanner const &) = delete;

    Scanner(const char * const begin, const char * const end) noexcept;

    Token read() noexcept;
};