#pragma once

#include <cstddef>
#include <string>
#include <iosfwd>
#include <memory>
#include <queue>

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
    T_IDENTIFIER,
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
    unsigned line = 1;
    unsigned line_offset = 0;
};

// A single token, emitted by the Scanner
// Contains the type of token, an optional token text, and location information
struct Token
{
    TokenType type = T_INVALID;
	ScanPosition pos;
    std::string value;
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

// A "high level" token, created from one or more primitive Tokens
enum class HLTokenType
{
	Invalid, // Invalid token
	Eof,     // End of file
	Newline, // Newline
	
	Specifier,  // A specifier name
	Label,      // A label name
	Identifier, // A generic identifier
	Number,     // A number
	Literal,    // A literal (#number)
	IndirectionStart, // Start of an indirection, [
	IndirectionEnd,   // End of an indirection, ]

	HLTokenTypeCount
};

std::ostream& operator<<(std::ostream& stream, HLTokenType token);

struct HLToken
{
	HLTokenType type = (HLTokenType)0;
	ScanPosition pos;

	vmword number_value = 0;      // For numeric values such as numbers or literals
	std::string string_value = ""; // For string values such as identifiers, labels, etc

	HLToken() { }
	HLToken(HLTokenType t, ScanPosition p) : type(t), pos(p) { }
	HLToken(HLTokenType t, ScanPosition p, vmword v) : type(t), pos(p), number_value(v) { }
	HLToken(HLTokenType t, ScanPosition p, const std::string& s) : type(t), pos(p), string_value(s) { }
};

class TokenAggregator
{
	Scanner& m_scanner;
	std::queue<Token> m_tokenQueue;

	Token nextToken() noexcept;

public:
	TokenAggregator(Scanner& scanner) noexcept;

	HLToken getNext() noexcept;

	inline ScanPosition pos() const noexcept { return m_scanner.pos(); }
};

////////
// Parsing
////////

// A refcounted array of HLTokens
class TokenBuffer
{
	size_t *m_refcnt = nullptr;

public:
	size_t count = 0;
	HLToken * tokens = nullptr;

	TokenBuffer(size_t c, const HLToken *token_data);
	TokenBuffer(const TokenBuffer& b);
	TokenBuffer(TokenBuffer&& b) = default;
	~TokenBuffer();

	TokenBuffer& operator=(const TokenBuffer& b);
};

// read tokens until a T_NEWLINE is encountered and return them in a buffer
TokenBuffer read_hltoken_line(TokenAggregator& aggregator);

//
// Parsed representations
//

enum SpecifierType
{
	ST_BASE,
};

enum OperandType
{
	OT_LITERAL,
	OT_REGISTER,
	OT_MEMORY,
	OT_LABELREF,
};

struct ParsedLabel
{
	std::string name;
};

struct ParsedSpecifier
{
	SpecifierType specifier;
	vmword operand;
};

struct ParsedInstruction
{
	Opcode opcode;
	size_t operand_count;
	typedef struct
	{
		bool indirect; // whether this uses indirect addressing
		OperandType type; // what kind of operand this is
		union value
		{
			std::string label_name; // If it's a label reference, this is the name of the label
			vmword numeric_value; // Otherwise, it's a word
		};
	} operands[3];
};

//
// The parsing api
//

enum SyntaxErrorType
{

};

struct SyntaxError
{
	SyntaxErrorType type; // the type of error
	Token tok; // the offending token
	std::string message; // the error message
};

enum ParsedLineType
{
	LT_LABEL,
	LT_SPECIFIER,
	LT_INSTRUCITON,
	LT_ERROR,
};

struct ParsedElement
{
	ParsedLineType type;
	union
	{
		SyntaxError error;
		ParsedLabel label;
		ParsedSpecifier specifier;
		ParsedInstruction instruction;
	};
};

// Parse the next element from a tokenbuffer
//   Returns the number of tokens consumed from the buffer
size_t parse_element(
	TokenBuffer tokens,   // The buffer from which to retrieve tokens
	size_t token_offset,  // Offset into the token buffer, where to start reading tokens
	ParsedElement *elem); // Destination for the resulting ParsedElement. Will only be assigned when there was no error

////////
// Assemblation
////////
