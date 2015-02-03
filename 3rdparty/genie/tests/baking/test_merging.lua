--
-- tests/baking/test_merging.lua
-- Verifies different field types are merged properly during baking.
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

	T.baking_merging = { }
	local suite = T.baking_merging

--
-- Setup code
--

	local sln, prj, cfg
	function suite.setup()
		sln = solution "MySolution"
		configurations { "Debug", "Release" }		
	end

	local function prepare()
		premake.bake.buildconfigs()
		prj = premake.solution.getproject(sln, 1)
	end
	

--
-- String value tests
--

	function suite.Strings_AreReplaced()
		kind "SharedLib"
		project "MyProject"
		kind "StaticLib"
		prepare()
		test.isequal("StaticLib", prj.kind)
	end

	function suite.Strings_KeepPreviousValue()
		kind "SharedLib"
		project "MyProject"
		prepare()
		test.isequal("SharedLib", prj.kind)
	end
		

--
-- List tests
--

	function suite.Lists_KeepPreviousValue()
		project "MyProject"
		prepare()
		test.isequal("Debug:Release", table.concat(prj.configurations, ":"))
	end
	
	function suite.Lists_AreJoined()
		defines { "SOLUTION" }
		project "MyProject"
		defines { "PROJECT" }
		prepare()
		test.isequal("SOLUTION:PROJECT", table.concat(prj.defines, ":"))
	end

	function suite.Lists_RemoveDuplicates()
		defines { "SOLUTION", "DUPLICATE" }
		project "MyProject"
		defines { "PROJECT", "DUPLICATE" }
		prepare()
		test.isequal("SOLUTION:DUPLICATE:PROJECT", table.concat(prj.defines, ":"))
	end
	
	function suite.Lists_FlattensNestedTables()
		defines { "ROOT", { "NESTED" } }
		project "MyProject"
		prepare()
		test.isequal("ROOT:NESTED", table.concat(prj.defines, ":"))
	end
		
	
--
-- Key/value tests
--

	function suite.KeyValue_AreMerged()
		vpaths { ["Solution"] = "*.sln" }
		project "MyProject"
		vpaths { ["Project"] = "*.prj" }
		prepare()
		test.isequal({"*.sln"}, prj.vpaths["Solution"])
		test.isequal({"*.prj"}, prj.vpaths["Project"])
	end
	
	function suite.KeyValue_MergesValues()
		vpaths { ["Solution"] = "*.sln", ["Project"] = "*.prj" }
		project "MyProject"
		vpaths { ["Project"] = "*.prjx" }
		prepare()
		test.isequal({"*.prj","*.prjx"}, prj.vpaths["Project"])
	end
