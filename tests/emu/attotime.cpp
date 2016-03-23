#include "gtest/gtest.h"
#include "emucore.h"
#include "eminline.h"
#include "attotime.h"

TEST(attotime,as_attoseconds) 
{
   attotime value = attotime::from_seconds(1);
   EXPECT_EQ(1000000000000000000, value.as_attoseconds());
}
