#ifndef UNITTEST_CURRENTTESTRESULTS_H
#define UNITTEST_CURRENTTESTRESULTS_H

#include "HelperMacros.h"

namespace UnitTest {

class TestResults;
class TestDetails;

namespace CurrentTest
{
	UNITTEST_LINKAGE TestResults*& Results();
	UNITTEST_LINKAGE const TestDetails*& Details();
}

}

#endif
