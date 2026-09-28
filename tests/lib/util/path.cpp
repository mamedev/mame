#include "catch.hpp"

#include "path.h"

TEST_CASE("Filename base extraction", "[util]")
{
    REQUIRE(core_filename_extract_base("pacman.zip", true) == "pacman");
    REQUIRE(core_filename_extract_base("pacman", true) == "pacman");
    REQUIRE(core_filename_extract_base("/roms/pacman.zip", true) == "pacman");
}

TEST_CASE("Filename extension extraction", "[util]")
{
    REQUIRE(core_filename_extract_extension("pacman.zip", true) == "zip");
    REQUIRE(core_filename_extract_extension("pacman.zip", false) == ".zip");
    REQUIRE(core_filename_extract_extension("pacman", true) == "");
}

TEST_CASE("Filename extension matching", "[util]")
{
    REQUIRE(core_filename_ends_with("game.zip", ".zip"));
    REQUIRE(core_filename_ends_with("game.ZIP", ".zip"));
    REQUIRE_FALSE(core_filename_ends_with("game.zip", ".chd"));
    REQUIRE_FALSE(core_filename_ends_with("game", ".zip"));
}