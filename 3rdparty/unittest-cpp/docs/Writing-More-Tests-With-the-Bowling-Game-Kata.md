The Bowling Game Kata refers to a test-driven development practice exercise recommended by Robert C. Martin or, as he is often referred to, Uncle Bob. You can find his version of it here: <http://butunclebob.com/ArticleS.UncleBob.TheBowlingGameKata>

We will progress through the exercise similarly to the Java version, using UnitTest++. To summarize, our goal is to create a class `Game` that has two methods:

* `roll(pins : int)` which is called each time the ball is rolled; `pins` is the number of pins knocked down.
* `score() : int` is called at the end of the game, returning the total score.

For simplicity's sake, I will be putting the `Game` class and the test cases in a single file.

The First Test
----------------
In our first test, we'll verify that a gutter game (20 gutter balls) scores a total of zero. First we create our Game class and our test suite:

```cpp
#include "UnitTest++/UnitTest++.h"

class Game
{ 
};

SUITE(BowlingGameTest)
{
    TEST(GutterGame)
    {
        Game g;
    }
}
```

The `SUITE` macro introduces a bundled set of tests. While not strictly required by UnitTest++, suites can be used to annotate and organize your tests, and to selectively run them. The `TEST` macro, as previous introduced, is the actual test case; right now it isn't asserting anything. Let's add the actual test.

```cpp
SUITE(BowlingGameTest)
{
    TEST(GutterGame)
    {
        Game g;
        
        for (int i = 0; i < 20; ++i)
        {
            g.roll(0);
        }
        
        CHECK_EQUAL(0, g.score());
    }
}
```

This will fail to compile until we add the requisite methods to game, so let's do that:

```cpp
class Game
{
public:
    void roll(int pins)
    {
    }
    
    int score() const
    {
        return -1;
    }
};
```

Now we build and run and we have a failure:

    TheFirstTest.cpp:27:1: error: Failure in GutterGame: Expected 0 but was -1
    FAILURE: 1 out of 1 tests failed (1 failures).

Now, obviously we know more about our final implementation than this, but for now we can "fake it" to make the build succeed by changing score to return 0. We end up with the full code below:

```cpp
#include "UnitTest++/UnitTest++.h"

class Game
{
public:
    void roll(int pins)
    {
    }
    
    int score() const
    {
        return 0;
    }
};

SUITE(BowlingGameTest)
{
    TEST(GutterGame)
    {
        Game g;
        
        for (int i = 0; i < 20; ++i)
        {
            g.roll(0);
        }
        
        CHECK_EQUAL(0, g.score());
    }
}
```

The test passes and we're ready to move on to [[Writing More Tests With the Bowling Game Kata: Test Two]].
