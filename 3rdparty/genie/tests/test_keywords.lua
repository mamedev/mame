--
-- tests/test_keywords.lua
-- Automated test suite for configuration block keyword filtering.
-- Copyright (c) 2008, 2009 Jason Perkins and the Premake project
--

	T.keywords = { }
	local suite = T.keywords

--
-- Keyword escaping tests
--

	function suite.escapes_special_chars()
		test.isequal("%.%-", path.wildcards(".-"))
	end
	

	function suite.escapes_star()
		test.isequal("vs[^/]*", path.wildcards("vs*"))
	end
	

	function suite.escapes_star_star()
		test.isequal("Images/.*%.bmp", path.wildcards("Images/**.bmp"))
	end




--
-- Keyword matching tests
--

	function T.keywords.matches_simple_strings()
		test.istrue(premake.iskeywordmatch("debug", { "debug", "windows", "vs2005" }))
	end
	
	
	function T.keywords.match_files_with_simple_strings()
		test.isfalse(premake.iskeywordmatch("release", { "debug", "windows", "vs2005" }))
	end
	
	
	function T.keywords.matches_with_patterns()
		test.istrue(premake.iskeywordmatch("vs20.*", { "debug", "windows", "vs2005" }))
	end
	
	
	function T.keywords.match_fails_with_not_term()
		test.isfalse(premake.iskeywordmatch("not windows", { "debug", "windows", "vs2005" }))
	end
	
	
	function T.keywords.match_ok_with_not_term()
		test.istrue(premake.iskeywordmatch("not linux", { "debug", "windows", "vs2005" }))
	end


	function T.keywords.match_ok_with_first_or()
		test.istrue(premake.iskeywordmatch("windows or linux", { "debug", "windows", "vs2005" }))
	end


	function T.keywords.match_ok_with_first_or()
		test.istrue(premake.iskeywordmatch("windows or linux", { "debug", "linux", "vs2005" }))
	end


	function T.keywords.match_ok_with_not_and_or()
		test.istrue(premake.iskeywordmatch("not macosx or linux", { "debug", "windows", "vs2005" }))
	end


	function T.keywords.match_fail_with_not_and_or()
		test.isfalse(premake.iskeywordmatch("not macosx or windows", { "debug", "windows", "vs2005" }))
	end


	function T.keywords.match_ok_required_term()
		test.istrue(premake.iskeywordsmatch({ "debug", "hello.c" }, { "debug", "windows", "vs2005", required="hello.c" }))
	end


	function T.keywords.match_fail_required_term()
		test.isfalse(premake.iskeywordsmatch({ "debug" }, { "debug", "windows", "vs2005", required="hello.c" }))
	end
