#ifndef UNITTEST_ASSERT_H
#define UNITTEST_ASSERT_H

#include "HelperMacros.h"

namespace UnitTest {

UNITTEST_LINKAGE void ReportAssert(char const* description, char const* filename, int lineNumber);

}

#endif
