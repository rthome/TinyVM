#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <stack>
#include <deque>
#include <memory>
#include <array>
#include <iosfwd>

#include "vm.hpp"

// File mapper - Map a file into memory (read-only)
class FileMapping
{
    void *m_handle = nullptr; // Custom handle for implementations to use
    char *m_begin  = nullptr; // Starting address of the mapping
    char *m_end    = nullptr; // End address of the mapping (last readable address)

public:
    // Disable copying
    FileMapping(const FileMapping&) = delete;
    FileMapping& operator=(const FileMapping&) = delete;

    // Move semantics
    FileMapping(FileMapping&&) = default;
    FileMapping& operator=(FileMapping&&) = default;

    explicit FileMapping(const char *filename) noexcept; // Maps a file
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
    T_POUND,
    T_COMMENT,
    T_STRING,
    T_NUMBER,
    T_LEFTBRACKET,
    T_RIGHTBRACKET,
    T_NEWLINE,

    TOKEN_TYPE_COUNT
};

// Stringification of token names
std::ostream& operator<<(std::ostream& stream, TokenType token);

// A position in a text file, with line number and offset into the line
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
    ScanPosition m_textpos;

    int peek() const noexcept;
    int get() noexcept;

    Token makeToken(TokenType type) const noexcept;

    void skipWhitespace() noexcept;
    Token read() noexcept;

public:
    Scanner(const Scanner &) = delete;
    Scanner & operator=(const Scanner &) = delete;

    Scanner(const char * const begin, const char * const end) noexcept;

    // Get the next token
    Token getNext() noexcept;

    // Get the current text position
    inline ScanPosition pos() const noexcept { return m_textpos; }
};

////////
// A buffered stream with checkpointing
////////

// A checkpointable queue
//   Element - type of elements in the queue
//   Source  - type of producer, must have a .readNext() that returns the next Element
template<typename Element, typename Source>
class BufferedStream
{
    Source& m_source;

    std::stack<size_t> m_checkpointStack;
    std::deque<Element> m_buffer;

	bool m_replaying = false;
	size_t m_replayIndex = 0;

public:
    BufferedStream(Source& source) noexcept
        : m_source(source)
    { }

    void checkpoint() noexcept
    {
        m_checkpointStack.push(m_buffer.size());
    }

    bool rewind() noexcept
    {
        if (m_checkpointStack.empty())
            return false;

        m_replayIndex = m_checkpointStack.top();
        m_replaying = true;
        m_checkpointStack.pop();
        return true;
    }

    Element current() const noexcept
    {
        if (m_buffer.empty())
            return Element();
        else
            if (m_replaying && m_replayIndex < m_buffer.size())
                return m_buffer[m_replayIndex];
            else
                return m_buffer.back();
    }

    Element next() noexcept
    {
        if (m_replaying)
        {
            if (m_replayIndex < m_buffer.size())
                return m_buffer[m_replayIndex++];
            else
                m_replaying = false;
        }

        Element newElement = m_source.getNext();
        m_buffer.push_back(newElement);
        return newElement;
    }
};

// Specialise for the trivial case of token buffering
typedef BufferedStream<Token, Scanner> BufferedTokenStream;

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
