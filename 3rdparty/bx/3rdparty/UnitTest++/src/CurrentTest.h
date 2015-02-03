#ifndef UNITTEST_CURRENTTESTRESULTS_H
#define UNITTEST_CURRENTTESTRESULTS_H

namespace UnitTest {

class TestResults;
class TestDetails;

namespace CurrentTest
{
	TestResults*& Results();
	const TestDetails*& Details();
}

}

#endif
