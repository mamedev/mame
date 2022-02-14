/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/file.h>

struct FilePathTest
{
	const char* filePath;
	const char* expected;
};

FilePathTest s_filePathTest[] =
{
	// Already clean
	{"", "."},
	{"abc", "abc"},
	{"abc/def", "abc/def"},
	{"a/b/c", "a/b/c"},
	{".", "."},
	{"..", ".."},
	{"../..", "../.."},
	{"../../abc", "../../abc"},
	{"/abc", "/abc"},
	{"/", "/"},

	// Do not remove trailing slash
	{"abc/", "abc/"},
	{"abc/def/", "abc/def/"},
	{"a/b/c/", "a/b/c/"},
	{"./", "./"},
	{"../", "../"},
	{"../../", "../../"},
	{"/abc/", "/abc/"},

	// Remove doubled slash
	{"abc//def//ghi", "abc/def/ghi"},
	{"//abc", "/abc"},
	{"///abc", "/abc"},
	{"//abc//", "/abc/"},
	{"abc//", "abc/"},

	// Remove . elements
	{"abc/./def", "abc/def"},
	{"/./abc/def", "/abc/def"},
	{"abc/.", "abc"},

	// Remove .. elements
	{"abc/def/ghi/../jkl", "abc/def/jkl"},
	{"abc/def/../ghi/../jkl", "abc/jkl"},
	{"abc/def/..", "abc"},
	{"abc/def/../..", "."},
	{"/abc/def/../..", "/"},
	{"abc/def/../../..", ".."},
	{"/abc/def/../../..", "/"},
	{"abc/def/../../../ghi/jkl/../../../mno", "../../mno"},

	// Combinations
	{"abc/./../def", "def"},
	{"abc//./../def", "def"},
	{"abc/../../././../def", "../../def"},

	{"abc\\/../..\\/././../def", "../../def"},
	{"\\abc/def\\../..\\..", "/"},
};

struct FilePathSplit
{
	const char* filePath;
	bool absolute;
	const char* path;
	const char* fileName;
	const char* baseName;
	const char* extension;
};

static const FilePathSplit s_filePathSplit[] =
{
	{ "\\abc/def\\../..\\../test.txt", true, "/", "test.txt", "test", ".txt" },
	{ "/abv/gd/555/333/pod.mac", true, "/abv/gd/555/333/", "pod.mac", "pod", ".mac" },
	{ "archive.tar.gz", false, "", "archive.tar.gz", "archive", ".tar.gz" },
	{ "tmp/archive.tar.gz", false, "tmp/", "archive.tar.gz", "archive", ".tar.gz" },
	{ "/tmp/archive.tar.gz", true, "/tmp/", "archive.tar.gz", "archive", ".tar.gz" },
	{ "d:/tmp/archive.tar.gz", true, "D:/tmp/", "archive.tar.gz", "archive", ".tar.gz" },
	{ "/tmp/abv/gd", true, "/tmp/abv/", "gd", "gd", "" },
	{ "/tmp/abv/gd/", true, "/tmp/abv/gd/", "", "", "" },
};

TEST_CASE("FilePath", "")
{
	bx::FilePath fp;
	for (uint32_t ii = 0; ii < BX_COUNTOF(s_filePathTest); ++ii)
	{
		const FilePathTest& test = s_filePathTest[ii];

		fp.set(test.filePath);

		REQUIRE(0 == bx::strCmp(test.expected, fp) );
	}

	for (uint32_t ii = 0; ii < BX_COUNTOF(s_filePathSplit); ++ii)
	{
		const FilePathSplit& test = s_filePathSplit[ii];

		fp.set(test.filePath);
		const bx::StringView path     = fp.getPath();
		const bx::StringView fileName = fp.getFileName();
		const bx::StringView baseName = fp.getBaseName();
		const bx::StringView ext      = fp.getExt();

		REQUIRE(0 == bx::strCmp(test.path,      path) );
		REQUIRE(0 == bx::strCmp(test.fileName,  fileName) );
		REQUIRE(0 == bx::strCmp(test.baseName,  baseName) );
		REQUIRE(0 == bx::strCmp(test.extension, ext) );
		REQUIRE(test.absolute == fp.isAbsolute() );
	};
}

TEST_CASE("FilePath temp", "")
{
	bx::FilePath tmp(bx::Dir::Temp);
	REQUIRE(0 != bx::strCmp(".", tmp.getPath().getPtr() ) );

	bx::Error err;
	tmp.join("test/abvgd/555333/test");
	REQUIRE(bx::makeAll(tmp, &err) );
	REQUIRE(err.isOk() );

	tmp.set(bx::Dir::Temp);
	tmp.join("test");
	REQUIRE(bx::removeAll(tmp, &err) );
	REQUIRE(err.isOk() );
}
