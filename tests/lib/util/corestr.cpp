#include "gtest/gtest.h"
#include "corestr.h"

TEST(corestr,strmakeupper) 
{
   std::string value = "test";
   EXPECT_STREQ("TEST", strmakeupper(value).c_str());
}

TEST(corestr,strmakelower) 
{
   std::string value = "ValUE";
   EXPECT_STREQ("value", strmakelower(value).c_str());
}

TEST(corestr,strreplace) 
{
   std::string value = "Main string";
   EXPECT_EQ(1, strreplace(value,"str","aaa"));
   EXPECT_STREQ("Main aaaing", value.c_str());
   EXPECT_EQ(4, strreplace(value,"a","b"));
}

TEST(corestr,strtrimspace) 
{
   std::string value = "    a value  for test  ";
   EXPECT_STREQ("a value  for test", strtrimspace(value).c_str());
   value = "\r\n\ta value  for test\r\n\n\r";
   EXPECT_STREQ("a value  for test", strtrimspace(value).c_str());
}

TEST(corestr,strreplacechr) 
{
   std::string value = "String for doing replaces";
   strreplacechr(value,'a','A');
   strreplacechr(value,'e','E');
   strreplacechr(value,'i','I');
   strreplacechr(value,'o','O');
   EXPECT_STREQ("StrIng fOr dOIng rEplAcEs", value.c_str());
}

TEST(corestr,strdelchr) 
{
   std::string value = "String for doing deletes";
   strdelchr(value,'a');
   strdelchr(value,'e');
   strdelchr(value,'i');
   strdelchr(value,'o');
   EXPECT_STREQ("Strng fr dng dlts", value.c_str());
}

