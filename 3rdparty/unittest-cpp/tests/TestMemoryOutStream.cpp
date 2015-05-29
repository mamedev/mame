#include "UnitTest++/UnitTestPP.h"

#include "UnitTest++/MemoryOutStream.h"
#include <cstring>
#include <cstdlib>
#include <climits>
#include <cfloat>

using namespace UnitTest;
using namespace std;

namespace {

const char* const maxSignedIntegralStr(size_t nBytes)
{
    switch(nBytes)
    {
        case 8:
            return "9223372036854775807";
        case 4:
            return "2147483647";
        case 2:
            return "32767";
        case 1:
            return "127";
        default:
            return "Unsupported signed integral size";
    }
}

const char* const minSignedIntegralStr(size_t nBytes)
{
    switch(nBytes)
    {
        case 8:
            return "-9223372036854775808";
        case 4:
            return "-2147483648";
        case 2:
            return "-32768";
        case 1:
            return "-128";
        default:
            return "Unsupported signed integral size";
    }
}

const char* const maxUnsignedIntegralStr(size_t nBytes)
{
    switch(nBytes)
    {
        case 8:
            return "18446744073709551615";
        case 4:
            return "4294967295";
        case 2:
            return "65535";
        case 1:
            return "255";
        default:
            return "Unsupported signed integral size";
    }
}

TEST(DefaultIsEmptyString)
{
    MemoryOutStream const stream;
    CHECK(stream.GetText() != 0);
    CHECK_EQUAL("", stream.GetText());
}

TEST(StreamingTextCopiesCharacters)
{
    MemoryOutStream stream;
    stream << "Lalala";
    CHECK_EQUAL("Lalala", stream.GetText());
}

TEST(StreamingMultipleTimesConcatenatesResult)
{
    MemoryOutStream stream;
    stream << "Bork" << "Foo" << "Bar";
    CHECK_EQUAL("BorkFooBar", stream.GetText());
}

TEST(StreamingIntWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << (int)123;
    CHECK_EQUAL("123", stream.GetText());
}

TEST(StreaminMaxIntWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << INT_MAX;
    CHECK_EQUAL(maxSignedIntegralStr(sizeof(int)), stream.GetText());    
}

TEST(StreamingMinIntWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << INT_MIN;
    CHECK_EQUAL(minSignedIntegralStr(sizeof(int)), stream.GetText());
}

TEST(StreamingUnsignedIntWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << (unsigned int)123;
    CHECK_EQUAL("123", stream.GetText());
}

TEST(StreamingMaxUnsignedIntWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << (unsigned int)UINT_MAX;
    CHECK_EQUAL(maxUnsignedIntegralStr(sizeof(unsigned int)), stream.GetText());
}

TEST(StreamingMinUnsignedIntWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << (unsigned int)0;
    CHECK_EQUAL("0", stream.GetText());
}

TEST(StreamingLongWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << (long)(-123);
    CHECK_EQUAL("-123", stream.GetText());
}

TEST(StreamingMaxLongWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << (long)(LONG_MAX);
    CHECK_EQUAL(maxSignedIntegralStr(sizeof(long)), stream.GetText());
}

TEST(StreamingMinLongWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << (long)(LONG_MIN);
    CHECK_EQUAL(minSignedIntegralStr(sizeof(long)), stream.GetText());
}

TEST(StreamingUnsignedLongWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << (unsigned long)123;
    CHECK_EQUAL("123", stream.GetText());
}

TEST(StreamingMaxUnsignedLongWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << (unsigned long)ULONG_MAX;
    CHECK_EQUAL(maxUnsignedIntegralStr(sizeof(unsigned long)), stream.GetText());
}

TEST(StreamingMinUnsignedLongWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << (unsigned long)0ul;
    CHECK_EQUAL("0", stream.GetText());
}

TEST(StreamingLongLongWritesCorrectCharacters)
{
	MemoryOutStream stream;
#ifdef UNITTEST_COMPILER_IS_MSVC6
   stream << (__int64)-12345i64;
#else
	stream << (long long)-12345ll;
#endif
   CHECK_EQUAL("-12345", stream.GetText());
}

#ifdef LLONG_MAX
TEST(StreamingMaxLongLongWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << (long long)LLONG_MAX;
    CHECK_EQUAL(maxSignedIntegralStr(sizeof(long long)), stream.GetText());
}
#endif

#ifdef LLONG_MIN
TEST(StreamingMinLongLongWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << (long long)LLONG_MIN;
    CHECK_EQUAL(minSignedIntegralStr(sizeof(long long)), stream.GetText());
}
#endif

TEST(StreamingUnsignedLongLongWritesCorrectCharacters)
{
	MemoryOutStream stream;
#ifdef UNITTEST_COMPILER_IS_MSVC6
   stream << (unsigned __int64)85899ui64;
#else
   stream << (unsigned long long)85899ull;
#endif
   CHECK_EQUAL("85899", stream.GetText());
}

#ifdef ULLONG_MAX
TEST(StreamingMaxUnsignedLongLongWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << (unsigned long long)ULLONG_MAX;
    CHECK_EQUAL(maxUnsignedIntegralStr(sizeof(unsigned long long)), stream.GetText());
}
#endif

TEST(StreamingMinUnsignedLongLongWritesCorrectCharacters)
{
    MemoryOutStream stream;
#ifdef UNITTEST_COMPILER_IS_MSVC6
    stream << (unsigned __int64)0ui64;
#else
    stream << (unsigned long long)0ull;
#endif
    CHECK_EQUAL("0", stream.GetText());
}

TEST(StreamingFloatWritesCorrectCharacters)
{
    MemoryOutStream stream;
    stream << 3.1415f;
	CHECK(strstr(stream.GetText(), "3.1415"));
}

TEST(StreamingDoubleWritesCorrectCharacters)
{
	MemoryOutStream stream;
	stream << 3.1415;
	CHECK(strstr(stream.GetText(), "3.1415"));
}

TEST(StreamingPointerWritesCorrectCharacters)
{
    MemoryOutStream stream;
    int* p = (int*)0x1234;
    stream << p;
    CHECK(strstr(stream.GetText(), "1234"));
}

TEST(StreamingSizeTWritesCorrectCharacters)
{
    MemoryOutStream stream;
    size_t const s = 53124;
    stream << s;
    CHECK_EQUAL("53124", stream.GetText());
}

TEST(ClearEmptiesMemoryOutStreamContents)
{
	MemoryOutStream stream;
	stream << "Hello world";
	stream.Clear();
	CHECK_EQUAL("", stream.GetText());
}

#ifndef UNITTEST_MEMORYOUTSTREAM_IS_STD_OSTRINGSTREAM

TEST(StreamInitialCapacityIsCorrect)
{
    MemoryOutStream stream(MemoryOutStream::GROW_CHUNK_SIZE);
    CHECK_EQUAL((int)MemoryOutStream::GROW_CHUNK_SIZE, stream.GetCapacity());
}

TEST(StreamInitialCapacityIsMultipleOfGrowChunkSize)
{
    MemoryOutStream stream(MemoryOutStream::GROW_CHUNK_SIZE + 1);
    CHECK_EQUAL((int)MemoryOutStream::GROW_CHUNK_SIZE * 2, stream.GetCapacity());
}


TEST(ExceedingCapacityGrowsBuffer)
{
    MemoryOutStream stream(MemoryOutStream::GROW_CHUNK_SIZE);
    stream << "012345678901234567890123456789";
    char const* const oldBuffer = stream.GetText();
    stream << "0123456789";
    CHECK(oldBuffer != stream.GetText());
}

TEST(ExceedingCapacityGrowsBufferByGrowChunk)
{
    MemoryOutStream stream(MemoryOutStream::GROW_CHUNK_SIZE);
    stream << "0123456789012345678901234567890123456789";
    CHECK_EQUAL(MemoryOutStream::GROW_CHUNK_SIZE * 2, stream.GetCapacity());
}

TEST(WritingStringLongerThanCapacityFitsInNewBuffer)
{
    MemoryOutStream stream(8);
    stream << "0123456789ABCDEF";
    CHECK_EQUAL("0123456789ABCDEF", stream.GetText());
}

TEST(WritingIntLongerThanCapacityFitsInNewBuffer)
{
    MemoryOutStream stream(8);
    stream << "aaaa" << 123456;
    CHECK_EQUAL("aaaa123456", stream.GetText());
}

TEST(WritingFloatLongerThanCapacityFitsInNewBuffer)
{
    MemoryOutStream stream(8);
    stream << "aaaa" << 123456.0f;
    CHECK_EQUAL("aaaa123456.000000", stream.GetText());
}

TEST(WritingSizeTLongerThanCapacityFitsInNewBuffer)
{
    MemoryOutStream stream(8);
    stream << "aaaa" << size_t(32145);
    CHECK_EQUAL("aaaa32145", stream.GetText());
}

TEST(VerifyLargeDoubleCanBeStreamedWithoutCrashing)
{
    MemoryOutStream stream(8);
    stream << DBL_MAX;
    CHECK(true);
}

#endif

}
