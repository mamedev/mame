/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/settings.h>
#include <bx/file.h>

TEST_CASE("Settings", "")
{
	bx::FilePath filePath;
	filePath.set(bx::Dir::Temp);
	filePath.join("settings.ini");

	bx::DefaultAllocator allocator;

	bx::Settings settings(&allocator);

	settings.set("meh/podmac", "true");
	settings.set("test/foo/bar/abvgd", "1389");

	bx::FileWriter writer;
	if (bx::open(&writer, filePath) )
	{
		bx::write(&writer, settings);
		bx::close(&writer);
	}

	REQUIRE(NULL == settings.get("meh") );
	REQUIRE(0 == bx::strCmp(settings.get("meh/podmac"), "true") );
	REQUIRE(0 == bx::strCmp(settings.get("test/foo/bar/abvgd"), "1389") );

	settings.remove("meh/podmac");
	REQUIRE(NULL == settings.get("meh/podmac") );

	settings.clear();

	bx::FileReader reader;
	if (bx::open(&reader, filePath) )
	{
		bx::read(&reader, settings);
		bx::close(&reader);
	}

	REQUIRE(NULL == settings.get("meh") );
	REQUIRE(0 == bx::strCmp(settings.get("meh/podmac"), "true") );
	REQUIRE(0 == bx::strCmp(settings.get("test/foo/bar/abvgd"), "1389") );
}
