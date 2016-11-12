#include "catch.hpp"

#include "corestr.h"

TEST_CASE("String make upper", "[util]")
{
   std::string value = "test";
   REQUIRE("TEST" == strmakeupper(value));
}

TEST_CASE("String make lower", "[util]")
{
   std::string value = "ValUE";
   REQUIRE("value" == strmakelower(value));
}

TEST_CASE("String replace", "[util]")
{
   std::string value = "Main string";
   REQUIRE(1 == strreplace(value,"str","aaa"));
   REQUIRE("Main aaaing" == value);
   REQUIRE(4 == strreplace(value,"a","b"));
}

TEST_CASE("String trimming", "[util]")
{
   std::string value = "    a value  for test  ";
   REQUIRE("a value  for test" == strtrimspace(value));
   value = "\r\n\ta value  for test\r\n\n\r";
   REQUIRE("a value  for test" == strtrimspace(value));
}

TEST_CASE("String replace chars", "[util]")
{
   std::string value = "String for doing replaces";
   strreplacechr(value,'a','A');
   strreplacechr(value,'e','E');
   strreplacechr(value,'i','I');
   strreplacechr(value,'o','O');
   REQUIRE("StrIng fOr dOIng rEplAcEs" == value);
}

TEST_CASE("String delete char", "[util]")
{
   std::string value = "String for doing deletes";
   strdelchr(value,'a');
   strdelchr(value,'e');
   strdelchr(value,'i');
   strdelchr(value,'o');
   REQUIRE("Strng fr dng dlts" == value);
}

