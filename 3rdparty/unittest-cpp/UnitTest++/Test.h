#ifndef UNITTEST_TEST_H
#define UNITTEST_TEST_H

#include "TestDetails.h"

namespace UnitTest {

class TestResults;
class TestList;

class UNITTEST_LINKAGE Test
{
public:
    explicit Test(char const* testName, char const* suiteName = "DefaultSuite", char const* filename = "", int lineNumber = 0);
    virtual ~Test();
    void Run();

    TestDetails const m_details;
    Test* m_nextTest;

	mutable bool m_isMockTest;

    static TestList& GetTestList();

    virtual void RunImpl() const;

private:
	Test(Test const&);
    Test& operator =(Test const&);
};


}

#endif
