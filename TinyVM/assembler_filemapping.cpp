#include "assembler_details.hpp"

#include "config.hpp"

#ifdef ENABLE_WIN32_FILEMAPPING_MMAP
// Win32 mmap implementation

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

FileMapping::FileMapping(const char *filename) noexcept
{
    HANDLE file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE)
        return;

    HANDLE mapped = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);
    if (mapped == INVALID_HANDLE_VALUE)
        return;
    m_handle = (void*)mapped;

    LPVOID mapped_view = MapViewOfFile(mapped, FILE_MAP_READ, 0, 0, 0);
    if (mapped_view != NULL)
    {
        LARGE_INTEGER file_size = { 0 };
        GetFileSizeEx(file, &file_size);
        m_begin = static_cast<char*>(mapped_view);
        m_end = m_begin + (size_t)file_size.QuadPart;
    }

    CloseHandle(file);
}

FileMapping::~FileMapping() noexcept
{
    HANDLE mapped = (HANDLE)m_handle;
    UnmapViewOfFile(m_begin);
    CloseHandle(mapped);
}

#else
#ifdef ENABLE_POSIX_FILEMAPPING_MMAP
// POSIX mmap implementation

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

FileMapping::FileMapping(const char *filename) noexcept
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
        return;

    struct stat sbuf;
    if (fstat(fd, &sbuf) == -1)
        return;

    auto mapped = mmap(nullptr, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped != MAP_FAILED)
    {
        m_begin = static_cast<char*>(mapped);
        m_end = m_begin + sbuf.st_size;
    }

    close(fd);
}

FileMapping::~FileMapping() noexcept
{
    if (m_begin != nullptr)
        munmap(m_begin, m_end - m_begin);
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