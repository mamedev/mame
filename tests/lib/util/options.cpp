#include "catch.hpp"

#include "options.h"

TEST_CASE("Empty options should return size of zero", "[util]")
{
	core_options options;
	REQUIRE(options.entries().size() == 0);
}
