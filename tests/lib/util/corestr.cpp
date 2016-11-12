#include "catch.hpp"

#include "corestr.h"

TEST_CASE("String make upper", "[util]")
{
   std::string value = "test";
   REQUIRE(strmakeupper(value) == "TEST");
}

TEST_CASE("String make lower", "[util]")
{
   std::string value = "ValUE";
   REQUIRE(strmakelower(value) == "value");
}

TEST_CASE("String replace", "[util]")
{
   std::string value = "Main string";
   REQUIRE(strreplace(value,"str","aaa") == 1);
   REQUIRE(value == "Main aaaing");
   REQUIRE(strreplace(value,"a","b") == 4);
}

TEST_CASE("String replace counting", "[util]")
{
   std::string value = "Main aaaing";
   REQUIRE(strreplace(value,"a","b") == 4);
}

TEST_CASE("String trimming spaces", "[util]")
{
   std::string value = "    a value  for test  ";
   REQUIRE(strtrimspace(value) == "a value  for test");
}

TEST_CASE("String trimming all special", "[util]")
{
   std::string value = "\r\n\ta value  for test\r\n\n\r";
   REQUIRE(strtrimspace(value) == "a value  for test");
}

TEST_CASE("String replace chars", "[util]")
{
   std::string value = "String for doing replaces";
   strreplacechr(value,'a','A');
   strreplacechr(value,'e','E');
   strreplacechr(value,'i','I');
   strreplacechr(value,'o','O');
   REQUIRE(value == "StrIng fOr dOIng rEplAcEs");
}

TEST_CASE("String delete char", "[util]")
{
   std::string value = "String for doing deletes";
   strdelchr(value,'a');
   strdelchr(value,'e');
   strdelchr(value,'i');
   strdelchr(value,'o');
   REQUIRE(value == "Strng fr dng dlts");
}

