#include "../UnitTest++.h"

// We're really testing if it's possible to use the same suite in two files
// to compile and link successfuly (TestTestSuite.cpp has suite with the same name)
// Note: we are outside of the anonymous namespace
SUITE(SameTestSuite)
{
    TEST(DummyTest2)
    {
    }
}

