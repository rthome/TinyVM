#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <stack>
#include <deque>
#include <memory>

#include "vm.hpp"

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

struct ScanPosition
{
    unsigned line;
    unsigned line_offset;
};

// A single token, emitted by the Scanner
// Contains the type of token, an optional token text, and location information
struct Token
{
    TokenType type = T_INVALID;
    std::string value;
    ScanPosition pos;
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
    Token read() noexcept;

public:
    Scanner(const Scanner &) = delete;
    Scanner & operator=(const Scanner &) = delete;

    Scanner(const char * const begin, const char * const end) noexcept;

    // Read the next token
    Token readNextToken() noexcept;

    // Get the current line number
    inline unsigned line() const noexcept { return m_line; }
};

////////
// A buffered token stream with checkpointing
////////

class BufferedTokenStream
{
    Scanner& m_scanner;
    std::stack<size_t> m_checkpointStack;
    std::deque<Token> m_tokenBuffer;

	bool m_replaying = false;
	size_t m_replayIndex = 0;

public:
    BufferedTokenStream(Scanner& scanner) noexcept;

    void checkpoint() noexcept;
    bool rewind() noexcept;

	Token currentToken() const noexcept;
    Token nextToken() noexcept;
};

////////
// Assembler & Parser declarations
////////

// Different kinds of syntax errors
enum SyntaxErrorType
{
    SE_NOERROR,

    SE_INVALIDTOKEN,
    SE_RIGHTBRACKETEXPECTED,
};

// A syntax error thrown by the Assembler
struct SyntaxError
{
    SyntaxErrorType error;
    std::string message;
    Token token;
};

// Opaque struct for parsing results
struct ParseBuffer;

// Parser - Parse tokens into a high-level representation
class Parser
{
	BufferedTokenStream &m_tokenStream;

public:
    // Parse a stream of tokens from a scanner - can throw SyntaxErrors
    std::unique_ptr<ParseBuffer> parse(BufferedTokenStream &stream);
};

// Assembler - Turn the parsed representation of the file into
class Assembler
{
    // Maps labels to memory locations
    std::unordered_map<std::string, vmword> m_labelMap;

    // The current memory address, set by .base specifier
    vmword m_address;

    VMContext *m_context;

public:
    // Take a ParseBuffer and assemble it into a vm context
    bool assemble(ParseBuffer *buffer, VMContext *context) noexcept;
};
