#pragma once

#include <cstddef>
#include <string>

// File mapper - Map a file into memory
// TODO: Optimize by using memory-mapped files on windows and unix?
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

////////
// Scanner declarations
////////

// Token types
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

// A single token, emitted by the Scanner
// Contains the type of token, an optional token text, and location information
struct Token
{
    TokenType type = T_INVALID;
    std::string value;
    unsigned line, column;
};

// Assembly scanner - Splits a memory range into a stream of tokens
class Scanner
{
    const char * const m_end  = nullptr;
    const char *m_next = nullptr;
    unsigned m_line = 1;
    unsigned m_column = 1;

    int peek() const noexcept;
    int get() noexcept;

    Token makeToken(TokenType type) const noexcept;

    void skipWhitespace() noexcept;

public:
    Scanner(const Scanner &) = delete;
    Scanner & operator=(const Scanner &) = delete;

    Scanner(const char * const begin, const char * const end) noexcept;

    // Read the next token
    Token read() noexcept;

    // Get the current line number
    inline unsigned line() const noexcept { return m_line; }
};

////////
// Parser declarations
////////

// Different kinds of syntax errors
enum SyntaxErrorType
{
    InvalidToken,
    RightBracketExpected,
};

// A syntax error thrown by the Parser
struct SyntaxError
{
    SyntaxErrorType error;
    std::string message;
    unsigned line, column;
};

// Assembly Parser - Process tokens from a scanner
// Consumes tokens from a scanner and produces a high-level representation of the file
class Parser
{
	Scanner& m_scanner;

public:
	Parser(Scanner& scanner);
};
