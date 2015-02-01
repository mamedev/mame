--
-- tests/actions/codeblocks/codeblocks_files.lua
-- Validate generation of files block in CodeLite C/C++ projects.
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

	T.codeblocks_files = { }
	local suite = T.codeblocks_files
	local codeblocks = premake.codeblocks


--
-- Setup 
--

	local sln, prj
	
	function suite.setup()
		sln = test.createsolution()
	end
	
	local function prepare()
		premake.bake.buildconfigs()
		prj = premake.solution.getproject(sln, 1)
		codeblocks.files(prj)
	end


--
-- Test grouping and nesting
--

	function suite.SimpleSourceFile()
		files { "hello.cpp" }
		prepare()
		test.capture [[
		<Unit filename="hello.cpp">
		</Unit>
		]]
	end


	function suite.SingleFolderLevel()
		files { "src/hello.cpp" }
		prepare()
		test.capture [[
		<Unit filename="src/hello.cpp">
		</Unit>
		]]
	end


	function suite.MultipleFolderLevels()
		files { "src/greetings/hello.cpp" }
		prepare()
		test.capture [[
		<Unit filename="src/greetings/hello.cpp">
		</Unit>
		]]
	end
	

--
-- Test source file type handling
--

	function suite.CFileInCppProject()
		files { "hello.c" }
		prepare()
		test.capture [[
		<Unit filename="hello.c">
			<Option compilerVar="CC" />
		</Unit>
		]]
	end


	function suite.WindowsResourceFile()
		files { "hello.rc" }
		prepare()
		test.capture [[
		<Unit filename="hello.rc">
			<Option compilerVar="WINDRES" />
		</Unit>
		]]
	end


	function suite.PchFile()
		files { "hello.h" }
		pchheader "hello.h"
		prepare()
		test.capture [[
		<Unit filename="hello.h">
			<Option compilerVar="CPP" />
			<Option compile="1" />
			<Option weight="0" />
			<Add option="-x c++-header" />
		</Unit>
		]]
	end
