#include "catch.hpp"

#include "emucore.h"
#include "eminline.h"
#include "attotime.h"

TEST_CASE("convert 1 sec to attotime", "[emu]")
{
   attotime value = attotime::from_seconds(1);
   REQUIRE(1000000000000000000 == value.as_attoseconds());
}
