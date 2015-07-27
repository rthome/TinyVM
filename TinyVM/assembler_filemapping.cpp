#include "assembler_details.hpp"

#include "config.hpp"

#ifdef ENABLE_WIN32_FILEMAPPING_MMAP
// Win32 mmap implementation

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

FileMapping::FileMapping(const char *filename) noexcept
{

}

FileMapping::~FileMapping() noexcept
{

}

#else
#ifdef ENABLE_POSIX_FILEMAPPING_MMAP
// POSIX mmap implementation

#include <sys/mman.h>

FileMapping::FileMapping(const char *filename) noexcept
{

}

FileMapping::~FileMapping() noexcept
{

}

#else
// Fallback implementation

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


#endif // ENABLE_POSIX_FILEMAPPING_MMAP
#endif // ENABLE_WIN32_FILEMAPPING_MMAP