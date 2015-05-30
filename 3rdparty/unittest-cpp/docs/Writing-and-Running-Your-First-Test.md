Let's assume that, whatever compiler / IDE / build system you're using, you know how to include the headers and libraries built in [Building UnitTest++ using CMake](wiki/Building-UnitTest++-With-CMake). There are too many build systems to cover here, and is not the purpose of this guide.

Examples 

Creating Your Test Executable Main
------------------------------------

The most basic main function you can write that will run your unit tests will look something like this:

```cpp
// main.cpp -- take 1
#include "UnitTest++/UnitTest++.h"

int main(int, const char *[])
{
   return UnitTest::RunAllTests();
}
```

If this compiles and links, you can now run and you should get output resembling this:

```
Success: 0 tests passed.
Test time: 0.00 seconds.
```

Adding a Sanity Test
----------------------
If you ran the self-tests bundled with UnitTest++, we should be pretty comfortable that everything is working okay at this point. However, the next thing I usually like to do is add a "sanity test".

```cpp
// main.cpp -- take 2
#include "UnitTest++/UnitTest++.h"

TEST(Sanity) 
{
   CHECK_EQUAL(1, 1);
}

int main(int, const char *[])
{
   return UnitTest::RunAllTests();
}
```

The `TEST` token is a C-style macro that introduces a test case named 'Sanity'. The `CHECK_EQUAL` token is, similarly, a macro. In this case it is asserting that, for the test to pass, 1 must be equal to 1. Running this test program should output:

```
Success: 1 tests passed.
Test time: 0.00 seconds.
```

If you want to doubly check the sanity of things, we can make our test fail by changing `CHECK_EQUAL(1, 1)` to `CHECK_EQUAL(1, 2)`. Run this and you should now see something like this:

```
main.cpp:6:1: error: Failure in Sanity: Expected 1 but was 2
FAILURE: 1 out of 1 tests failed (1 failures).
Test time: 0.00 seconds.
```

The exact output will vary somewhat based on your environment. What is perhaps most important, however, is that the OS exit code will be non-zero, indicating a failure to your toolchain. `UnitTest::RunAllTests()` returns the number of failures that occurred during the run.

Next Steps
------------
If you're comfortable with the concepts of unit testing in general, you can probably go on now to the [[Macro Reference]] to learn about the CHECK and TEST macros available to you. However, if you'd like a more detailed guide to using UnitTest++, especially in the context of test-driven development, you might want to go on to [[Writing More Tests With the Bowling Game Kata]].