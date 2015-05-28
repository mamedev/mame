// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic

#include "UnitTest++/UnitTest++.h"
#include "corestr.h"

TEST(strmakeupper) 
{
   std::string value = "test";
   CHECK_EQUAL("TEST", strmakeupper(value).c_str());
}

TEST(strmakelower) 
{
   std::string value = "ValUE";
   CHECK_EQUAL("value", strmakelower(value).c_str());
}

TEST(strreplace) 
{
   std::string value = "Main string";
   CHECK_EQUAL(1, strreplace(value,"str","aaa"));
   CHECK_EQUAL("Main aaaing", value.c_str());
   CHECK_EQUAL(4, strreplace(value,"a","b"));
}

TEST(strtrimspace) 
{
   std::string value = "    a value  for test  ";
   CHECK_EQUAL("a value  for test", strtrimspace(value).c_str());
   value = "\r\n\ta value  for test\r\n\n\r";
   CHECK_EQUAL("a value  for test", strtrimspace(value).c_str());
}

TEST(strreplacechr) 
{
   std::string value = "String for doing replaces";
   strreplacechr(value,'a','A');
   strreplacechr(value,'e','E');
   strreplacechr(value,'i','I');
   strreplacechr(value,'o','O');
   CHECK_EQUAL("StrIng fOr dOIng rEplAcEs", value.c_str());
}

TEST(strdelchr) 
{
   std::string value = "String for doing deletes";
   strdelchr(value,'a');
   strdelchr(value,'e');
   strdelchr(value,'i');
   strdelchr(value,'o');
   CHECK_EQUAL("Strng fr dng dlts", value.c_str());
}

