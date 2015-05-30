#ifndef UNITTEST_SCOPEDCURRENTTEST_H
#define UNITTEST_SCOPEDCURRENTTEST_H

#include "../CurrentTest.h"
#include <cstddef>

class ScopedCurrentTest
{
public:
	ScopedCurrentTest()
		: m_oldTestResults(UnitTest::CurrentTest::Results())
		, m_oldTestDetails(UnitTest::CurrentTest::Details())
	{
	}

	explicit ScopedCurrentTest(UnitTest::TestResults& newResults, const UnitTest::TestDetails* newDetails = NULL)
		: m_oldTestResults(UnitTest::CurrentTest::Results())
		, m_oldTestDetails(UnitTest::CurrentTest::Details())
	{
		UnitTest::CurrentTest::Results() = &newResults;

		if (newDetails != NULL)
			UnitTest::CurrentTest::Details() = newDetails;
	}

	~ScopedCurrentTest()
	{
		UnitTest::CurrentTest::Results() = m_oldTestResults;
		UnitTest::CurrentTest::Details() = m_oldTestDetails;
	}

private:
	UnitTest::TestResults* m_oldTestResults;
	const UnitTest::TestDetails* m_oldTestDetails;
};

#endif
