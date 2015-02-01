#include "../UnitTest++.h"
#include "../CurrentTest.h"
#include "ScopedCurrentTest.h"

namespace 
{

TEST(CanSetandGetDetails)
{
	bool ok = false;
	{
		ScopedCurrentTest scopedTest;

		const UnitTest::TestDetails* details = reinterpret_cast< const UnitTest::TestDetails* >(12345);
		UnitTest::CurrentTest::Details() = details;

		ok = (UnitTest::CurrentTest::Details() == details);
	}

	CHECK(ok);
}

TEST(CanSetAndGetResults)
{
	bool ok = false;
	{
		ScopedCurrentTest scopedTest;

		UnitTest::TestResults results;
		UnitTest::CurrentTest::Results() = &results;

		ok = (UnitTest::CurrentTest::Results() == &results);
	}

	CHECK(ok);
}

}
