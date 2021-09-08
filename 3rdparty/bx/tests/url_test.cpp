/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/string.h>
#include <bx/url.h>

struct UrlTest
{
	bool result;
	const char* url;
	const char* tokens[bx::UrlView::Count];
};

static const UrlTest s_urlTest[] =
{
	{ true
	, "scheme://username:password@host.rs:80/this/is/path/index.php?query=\"value\"#fragment",
	{ "scheme", "username", "password", "host.rs", "80", "/this/is/path/index.php", "query=\"value\"", "fragment" }
	},
	{ true
	, "scheme://host.rs/",
	{ "scheme", "", "", "host.rs", "", "/", "", "" },
	},
	{ true
	, "scheme://host.rs:1389/",
	{ "scheme", "", "", "host.rs", "1389", "/", "", "" },
	},
	{ true
	, "host.rs/abvgd.html",
	{ "", "", "", "host.rs", "", "/abvgd.html", "", "" },
	},
	{ true
	, "https://192.168.0.1:8080/",
	{ "https", "", "", "192.168.0.1", "8080", "/", "", "" },
	},

	{ true
	, "file:///d:/tmp/archive.tar.gz",
	{ "file", "", "", "", "", "/d:/tmp/archive.tar.gz", "", "" },
	},
};

TEST_CASE("tokenizeUrl", "")
{
	bx::UrlView url;

	for (uint32_t ii = 0; ii < BX_COUNTOF(s_urlTest); ++ii)
	{
		const UrlTest& urlTest = s_urlTest[ii];

		bool result = url.parse(urlTest.url);
		REQUIRE(urlTest.result == result);

		if (result)
		{
			for (uint32_t token = 0; token < bx::UrlView::Count; ++token)
			{
//				char tmp[1024];
//				strCopy(tmp, BX_COUNTOF(tmp), url.get(bx::UrlView::Enum(token)) );
//				printf("`%s`, expected: `%s`\n", tmp, urlTest.tokens[token]);

				REQUIRE(0 == bx::strCmp(urlTest.tokens[token], url.get(bx::UrlView::Enum(token)) ) );
			}
		}
	}
}
