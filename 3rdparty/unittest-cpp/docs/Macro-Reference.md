Suites
--------
`SUITE(Name)`: Organizes your tests into suites (groups). Tests can be added to suites across multiple test source files. e.g.:

```cpp
SUITE(MySuite)
{
   // tests go here
}
```

Tests
-------
`TEST(Name)`: Creates a single test case. All checks in a test will be run using the standard runners, unless an exception is thrown or an early return is introduced.

```cpp
TEST(MyTest)
{
   // checks go here
}
``` 

`TEST_FIXTURE(FixtureClass, TestName)`: Creates a single test case using a fixture. The FixtureClass is default instantiated before the test is run, then the test runs with access to anything `public` or `protected` in the fixture. Useful for sharing setup / teardown code.

```cpp
class MyFixture
{
public:
   MyFixture() { // setup goes here }
   ~MyFixture() { // teardown goes here }
};

TEST_FIXTURE(MyFixture, MyFixtureTest)
{
   // checks go here
}
```

Checks
--------
`CHECK(statement)`: Verifies the statement evaluates to true (not necessary boolean true / false).

```cpp
CHECK(true); // passes
CHECK(1 == 2); // fails
CHECK(0); // fails
```

`CHECK_EQUAL(expected, actual)`: Verifies that the actual value matches the expected. Note that conversions can occur. Requires `operator==` for the types of `expected` and `actual`, and requires the ability for both types to be streamed to `UnitTest::MemoryOutStream` using `operator<<`.

```cpp
CHECK_EQUAL(1, 1); // passes
CHECK_EQUAL("123", std::string("123")); //passes
CHECK_EQUAL((1.0 / 40.0), 0.025000000000000001); // passes... wait what? be careful with floating point types!
```

`CHECK_CLOSE(expected, actual, tolerance)`: Verifies that the actual value is within +/- tolerance of the expected value. This has the same requirements of the types involved as `CHECK_EQUAL`.

```cpp
CHECK_CLOSE(0.025000000000000002, (1.0 / 40.0), 0.000000000000000001); // passes
CHECK_CLOSE(0.025, (1.0 / 40.0), 0.000000000000000001); // also passes
CHECK_CLOSE(0.025000000000000020, (1.0 / 40.0), 0.000000000000000001); // fails
```

`CHECK_THROW(expression, ExpectedExceptionType)`: Verifies that the expression throws an exception that is polymorphically of the ExpectedExceptionType.

`CHECK_ARRAY_EQUAL(expected, actual, count)`: Like `CHECK_EQUAL`, but for arrays and containers that support random access (`operator[]`). `count` is the number of items in the array.

`CHECK_ARRAY_CLOSE(expected, actual, count, tolerance)`: Like `CHECK_CLOSE`, but for arrays and containers that support random access (`operator[]`). `count` is the number of items in the array.

`CHECK_ARRAY2D_CLOSE(expected, actual, rows, columns, tolerance)`: Like `CHECK_ARRAY_CLOSE` but for two-dimensional arrays.


