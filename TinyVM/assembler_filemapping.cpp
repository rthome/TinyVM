#include "assembler_details.hpp"

#include <cstdlib>
#include <cstddef>

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
