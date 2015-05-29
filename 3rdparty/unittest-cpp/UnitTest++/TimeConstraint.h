#ifndef UNITTEST_TIMECONSTRAINT_H
#define UNITTEST_TIMECONSTRAINT_H

#include "TimeHelpers.h"
#include "HelperMacros.h"

namespace UnitTest {

class TestResults;
class TestDetails;

class UNITTEST_LINKAGE TimeConstraint
{
public:
    TimeConstraint(int ms, TestDetails const& details);
    ~TimeConstraint();

private:
    void operator=(TimeConstraint const&); 
	TimeConstraint(TimeConstraint const&);

	Timer m_timer;
    TestDetails const& m_details;
	int const m_maxMs;
};

#define UNITTEST_TIME_CONSTRAINT(ms) \
	UnitTest::TimeConstraint unitTest__timeConstraint__(ms, UnitTest::TestDetails(m_details, __LINE__))

#define UNITTEST_TIME_CONSTRAINT_EXEMPT() \
	UNITTEST_MULTILINE_MACRO_BEGIN \
	m_details.timeConstraintExempt = true; \
	UNITTEST_MULTILINE_MACRO_END

}

#endif
