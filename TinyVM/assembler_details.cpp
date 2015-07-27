#include "assembler_details.hpp"

#include <cctype>
#include <functional>
#include <vector>

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
    token.pos.line = m_line;
    token.pos.line_offset = m_column;
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

    struct Entity { };

    // An instruction
    struct InstructionEntity : Entity
    {
        std::string name;
        size_t operand_count;
        Operand operands[3];
    };

    // A label
    struct LabelEntity : Entity
    {
        std::string name;
    };

    // A special specifier
    struct SpecifierEntity : Entity
    {
        SpecifierType type;
        vmword operand;
    };

    struct ParseBufferEntry
    {
        EntityType type;
        std::shared_ptr<Entity> entity_ptr;
    };

    inline ParseBufferEntry makeEntry(EntityType type, std::shared_ptr<Entity> entity)
    {
        ParseBufferEntry entry;
        entry.type = type;
        entry.entity_ptr = entity;
        return entry;
    }
}

struct ParseBuffer
{
    size_t entry_count;
    std::unique_ptr<ParseBufferEntry[]> entries;
};

// Parser functions
namespace
{
    std::shared_ptr<LabelEntity> parse_label(BufferedTokenStream &stream)
    {
        return std::shared_ptr<LabelEntity>(nullptr);
    }

    std::shared_ptr<InstructionEntity> parse_instruction(BufferedTokenStream &stream)
    {
        return std::shared_ptr<InstructionEntity>(nullptr);
    }

    std::shared_ptr<SpecifierEntity> parse_specifier(BufferedTokenStream &stream)
    {
        return std::shared_ptr<SpecifierEntity>(nullptr);
    }

    // helper function to call parser functions
    template<typename Entity>
    std::shared_ptr<Entity> call_parser_for(BufferedTokenStream &stream, std::function<std::shared_ptr<Entity>(BufferedTokenStream&)> parser_f)
    {
        stream.checkpoint();
        auto entity_ptr = parser_f(stream);
        if (entity_ptr)
            return entity_ptr;
        else
        {
            stream.rewind();
            return std::shared_ptr<Entity>(nullptr);
        }
    }
}

std::unique_ptr<ParseBuffer> Parser::parse(BufferedTokenStream &stream)
{
    // Temporary storage for buffer entries
    std::vector<ParseBufferEntry> entry_vector;

    // Read the first token
	m_tokenStream.nextToken();

    while(stream.currentToken().type != T_EOF)
    {
        auto specifier_ptr = call_parser_for<SpecifierEntity>(stream, parse_specifier);
        if (specifier_ptr)
        {
            entry_vector.push_back(makeEntry(ET_SPECIFIER, specifier_ptr));
            continue;
        }

        SyntaxError error;
        error.error = SE_INVALIDTOKEN;
        error.message = "Expected a specifier, instruction, or label.";
        error.token = stream.currentToken();
        throw error;

		stream.nextToken();
    }

    auto pbuffer = std::make_unique<ParseBuffer>();
    pbuffer->entry_count = entry_vector.size();
    pbuffer->entries = std::make_unique<ParseBufferEntry[]>(entry_vector.size());
    for (size_t i = 0; i < entry_vector.size(); i++)
        pbuffer->entries[i] = entry_vector[i];
    return pbuffer;
}

////////
////////

bool Assembler::assemble(ParseBuffer *buffer, VMContext *context) noexcept
{
	return false;
}
