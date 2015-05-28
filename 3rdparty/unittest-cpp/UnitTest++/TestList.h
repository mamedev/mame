#ifndef UNITTEST_TESTLIST_H
#define UNITTEST_TESTLIST_H

#include "HelperMacros.h"

namespace UnitTest {

class Test;

class UNITTEST_LINKAGE TestList
{
public:
    TestList();
    void Add (Test* test);

    Test* GetHead() const;

private:
    Test* m_head;
    Test* m_tail;
};


class UNITTEST_LINKAGE ListAdder
{
public:
    ListAdder(TestList& list, Test* test);
};

}


#endif
