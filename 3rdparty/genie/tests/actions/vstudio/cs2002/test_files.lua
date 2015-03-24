--
-- tests/actions/vstudio/cs2002/test_files.lua
-- Validate generation of <Files/> block in Visual Studio 2002 .csproj
-- Copyright (c) 2009-2012 Jason Perkins and the Premake project
--

	T.vstudio_cs2002_files = { }
	local suite = T.vstudio_cs2002_files
	local cs2002 = premake.vstudio.cs2002


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
		sln.vstudio_configs = premake.vstudio.buildconfigs(sln)
		cs2002.Files(prj)
	end


--
-- Test grouping and nesting
--

	function suite.SimpleSourceFile()
		files { "Hello.cs" }
		prepare()
		test.capture [[
				<File
					RelPath = "Hello.cs"
					BuildAction = "Compile"
					SubType = "Code"
				/>
		]]
	end

	function suite.NestedSourceFile()
		files { "Src/Hello.cs" }
		prepare()
		test.capture [[
				<File
					RelPath = "Src\Hello.cs"
					BuildAction = "Compile"
					SubType = "Code"
				/>
		]]
	end


--
-- The relative path to the file is correct for files that live outside
-- the project's folder.
--

	function suite.filesUseRelativePath_onOutOfTreePath()
		files { "../Src/Hello.cs" }
		prepare()
		test.capture [[
				<File
					RelPath = "..\Src\Hello.cs"
					BuildAction = "Compile"
					SubType = "Code"
				/>
		]]
	end



--
-- Test file dependencies
--

	function suite.SimpleResourceDependency()
		files { "Resources.resx", "Resources.cs" }
		prepare()
		test.capture [[
				<File
					RelPath = "Resources.cs"
					BuildAction = "Compile"
					SubType = "Code"
				/>
				<File
					RelPath = "Resources.resx"
					BuildAction = "EmbeddedResource"
					DependentUpon = "Resources.cs"
				/>
		]]
	end


--
-- Test build actions
--

	function suite.BuildAction_Compile()
		files { "Hello.png" }
		configuration "*.png"
			buildaction "Compile"
		prepare()
		test.capture [[
				<File
					RelPath = "Hello.png"
					BuildAction = "Compile"
				/>
		]]
	end

	function suite.BuildAction_Copy()
		files { "Hello.png" }
		configuration "*.png"
			buildaction "Copy"
		prepare()
		test.capture [[
				<File
					RelPath = "Hello.png"
					BuildAction = "Content"
				/>
		]]
	end

	function suite.BuildAction_Embed()
		files { "Hello.png" }
		configuration "*.png"
			buildaction "Embed"
		prepare()
		test.capture [[
				<File
					RelPath = "Hello.png"
					BuildAction = "EmbeddedResource"
				/>
		]]
	end
