#include "assembler_details.hpp"

#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <functional>
#include <vector>

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
    ++m_column;
    return *m_next++;
}

Token Scanner::makeToken(TokenType type) const noexcept
{
    Token token;
    token.type = type;
    token.line = m_line;
    token.column = m_column;
    return token;
}

void Scanner::skipWhitespace() noexcept
{
    while (m_next != m_end)
    {
        if ('\n' == *m_next)
        {
            m_column = 0;
            ++m_line;
            ++m_next;
        }
        else if (isspace(*m_next))
        {
            ++m_column;
            ++m_next;
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

    if ('[' == c)
        return makeToken(T_LEFTBRACKET);
    if (']' == c)
        return makeToken(T_RIGHTBRACKET);
    if (':' == c)
        return makeToken(T_COLON);
    if ('.' == c)
        return makeToken(T_DOT);
    if (';' == c)
        return token_reader_helper(makeToken(T_COMMENT), c,
                                   [&]() { return get(); },
                                   [](int c) { return (c != '\r' && c != '\n'); });
    if (isdigit(c))
        return token_reader_helper(makeToken(T_NUMBER), c,
                                   [&]() { return get(); },
                                   [](int c) { return isdigit(c); });
    if (isalpha(c))
        return token_reader_helper(makeToken(T_STRING), c,
                                   [&]() { return get(); },
                                   [](int c) { return isalnum(c); });

    return makeToken(T_INVALID);
}

Token Scanner::readNextToken() noexcept
{
    auto token = this->read();
    return token;
}

////////
// BufferedTokenStream implementation
////////

BufferedTokenStream::BufferedTokenStream(Scanner &scanner) noexcept
    : m_scanner(scanner)
{

}

void BufferedTokenStream::checkpoint() noexcept
{
	m_checkpointStack.push(m_tokenBuffer.size());
}

bool BufferedTokenStream::rewind() noexcept
{
	if (m_checkpointStack.empty())
		return false;

	m_replayIndex = m_checkpointStack.top();
	m_replaying = true;
	m_checkpointStack.pop();
	return true;
}

Token BufferedTokenStream::currentToken() const noexcept
{
	if (m_tokenBuffer.empty())
		return Token();
	return m_tokenBuffer.back();
}

Token BufferedTokenStream::nextToken() noexcept
{
	if (m_replaying)
	{
		if (m_replayIndex < m_tokenBuffer.size())
			return m_tokenBuffer[m_replayIndex++];
		else
			m_replaying = false;
	}

	Token newToken = m_scanner.readNextToken();
	m_tokenBuffer.push_back(newToken);
	return newToken;
}

////////
// Parser/Assembler implementation
////////

// High-level assembly language representation structures
// For Parser output
namespace
{
    enum OperandType
    {
        OT_REGISTER, // Standard types
        OT_ADDRESS,
        OT_LITERAL,
        OT_LABEL, // Referencing a label for its address value
    };

    enum SpecifierType
    {
        ST_BASE,
    };

    enum EntityType
    {
        ET_INSTRUCTION,
        ET_LABEL,
        ET_SPECIFIER,
    };

    // Operand for an instruction
    struct Operand
    {
        OperandType type;
        bool indirect;
        union value
        {
            Registers reg;
            vmword addr;
            vmword literal;
            std::string label;
        };
    };

    struct ParseEntity
    {

    };

    // An instruction
    struct InstructionEntity : public ParseEntity
    {
        std::string name;
        size_t operand_count;
        Operand operands[3];
    };

    // A label
    struct LabelEntity : public ParseEntity
    {
        std::string name;
    };

    // A special specifier
    struct SpecifierEntity : public ParseEntity
    {
        SpecifierType type;
        vmword operand;
    };

    struct ParseBufferEntry
    {
        EntityType type;
        ParseEntity *entity;
    };

    inline ParseBufferEntry makeEntry(EntityType type, ParseEntity *entity)
    {
        ParseBufferEntry entry;
        entry.type = type;
        entry.entity = entity;
        return entry;
    }
}

struct ParseBuffer
{
    std::vector<ParseBufferEntry> entries;
};

bool parse_label(BufferedTokenStream &stream, LabelEntity &dst)
{
	return false;
}

bool parse_instruction(BufferedTokenStream &stream, InstructionEntity &dst)
{
	return false;
}

bool parse_specifier(BufferedTokenStream &stream, SpecifierEntity &dst)
{
	return false;
}

ParseBuffer* Parser::parse(BufferedTokenStream &stream)
{
    // Allocate a parse buffer
    auto buffer = new ParseBuffer;

    // Read the first token
	auto token = m_tokenStream.nextToken();

    // TODO: This will not really work out well.
    // Need to parse more tokens at once or allow
    // backing out after discovering that an
    // alternative parse does not work out.
    // -> Implement some sort of buffered token stream

    while(stream.currentToken().type != T_EOF)
    {
        SpecifierEntity specifier;
        if (parse_specifier(stream, specifier))
        {
            buffer->entries.push_back(makeEntry(ET_SPECIFIER, new SpecifierEntity(specifier)));
            continue;
        }

        LabelEntity label;
        if (parse_label(stream, label))
        {
            buffer->entries.push_back(makeEntry(ET_LABEL, new LabelEntity(label)));
            continue;
        }

        InstructionEntity instruction;
        if (parse_instruction(stream, instruction))
        {
            buffer->entries.push_back(makeEntry(ET_INSTRUCTION, new InstructionEntity(instruction)));
            continue;
        }

        SyntaxError error;
        error.error = SE_INVALIDTOKEN;
        error.message = "Expected a specifier, instruction, or label.";
        error.line = token.line;
        error.column = token.column;
        throw error;

		token = stream.nextToken();
    }

    return buffer;
}

////////
////////

bool Assembler::assemble(ParseBuffer *buffer, VMContext *context) noexcept
{
	return false;
}
