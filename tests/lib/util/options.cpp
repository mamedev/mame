#include "catch.hpp"

#include "options.h"

TEST_CASE("Empty options should return first as nullptr", "[util]")
{
	core_options options;
	REQUIRE(options.first() == nullptr);
}

TEST_CASE("Empty options should have iterators same", "[util]")
{
	core_options options;
	REQUIRE(options.begin() == options.end());
}
