#ifndef UNITTEST_MEMORYOUTSTREAM_H
#define UNITTEST_MEMORYOUTSTREAM_H

#include "Config.h"
#include "HelperMacros.h"

#ifdef UNITTEST_MEMORYOUTSTREAM_IS_STD_OSTRINGSTREAM

#include <sstream>

namespace UnitTest
{

class UNITTEST_LINKAGE MemoryOutStream : public std::ostringstream
{
public:
    MemoryOutStream() {}
    ~MemoryOutStream() {}
	void Clear();
	char const* GetText() const;

private:
    MemoryOutStream(MemoryOutStream const&);
    void operator =(MemoryOutStream const&);

    mutable std::string m_text;
};

#ifdef UNITTEST_COMPILER_IS_MSVC6
std::ostream& operator<<(std::ostream& stream, __int64 const n);
std::ostream& operator<<(std::ostream& stream, unsigned __int64 const n);
#endif

}

#else

#include <cstddef>

#ifdef UNITTEST_COMPILER_IS_MSVC6
namespace std {}
#endif

namespace UnitTest
{

class UNITTEST_LINKAGE MemoryOutStream
{
public:
    explicit MemoryOutStream(int const size = 256);
    ~MemoryOutStream();

	void Clear();
    char const* GetText() const;

    MemoryOutStream& operator <<(char const* txt);
    MemoryOutStream& operator <<(int n);
    MemoryOutStream& operator <<(long n);
    MemoryOutStream& operator <<(unsigned long n);
#ifdef UNITTEST_COMPILER_IS_MSVC6
    MemoryOutStream& operator <<(__int64 n);
    MemoryOutStream& operator <<(unsigned __int64 n);
#else
    MemoryOutStream& operator <<(long long n);
    MemoryOutStream& operator <<(unsigned long long n);
#endif
   MemoryOutStream& operator <<(float f);
    MemoryOutStream& operator <<(double d);
    MemoryOutStream& operator <<(void const* p);
    MemoryOutStream& operator <<(unsigned int s);

    enum { GROW_CHUNK_SIZE = 32 };
    int GetCapacity() const;

private:
    void operator= (MemoryOutStream const&);
    void GrowBuffer(int capacity);

    int m_capacity;
    char* m_buffer;
};

}

#endif

#endif
