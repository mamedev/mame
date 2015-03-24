--
-- tests/actions/test_clean.lua
-- Automated test suite for the "clean" action.
-- Copyright (c) 2009 Jason Perkins and the Premake project
--

	T.clean = { }


--
-- Replacement functions for remove() and rmdir() for testing
--

	local os_remove, os_rmdir, cwd
	local removed
	
	local function test_remove(fn)
		if not cwd then cwd = os.getcwd() end
		table.insert(removed, path.getrelative(cwd, fn))
	end


--
-- Setup/teardown
--

	local sln
	function T.clean.setup()
		_ACTION = "clean"

		os_remove = os.remove
		os_rmdir  = os.rmdir
		os.remove = test_remove
		os.rmdir  = test_remove
		removed = {}
				
		sln = solution "MySolution"
		configurations { "Debug", "Release" }
	end

	function T.clean.teardown()
		os.remove = os_remove
		os.rmdir  = os_rmdir
	end
	
	local function prepare()
		premake.bake.buildconfigs()
		premake.action.call("clean")		
	end



--
-- Tests
--

	function T.clean.SolutionFiles()
		prepare()		
		test.contains(removed, "MySolution.sln")
		test.contains(removed, "MySolution.suo")
		test.contains(removed, "MySolution.ncb")
		test.contains(removed, "MySolution.userprefs")
		test.contains(removed, "MySolution.usertasks")
		test.contains(removed, "MySolution.workspace")
		test.contains(removed, "MySolution_wsp.mk")
		test.contains(removed, "MySolution.tags")
		test.contains(removed, "Makefile")
	end


	function T.clean.CppProjectFiles()
		prj = project "MyProject"
		language "C++"
		kind "ConsoleApp"
		prepare()
		test.contains(removed, "MyProject.vcproj")
		test.contains(removed, "MyProject.pdb")
		test.contains(removed, "MyProject.idb")
		test.contains(removed, "MyProject.ilk")
		test.contains(removed, "MyProject.cbp")
		test.contains(removed, "MyProject.depend")
		test.contains(removed, "MyProject.layout")
		test.contains(removed, "MyProject.mk")
		test.contains(removed, "MyProject.list")
		test.contains(removed, "MyProject.out")
		test.contains(removed, "MyProject.make")
	end


	function T.clean.CsProjectFiles()
		prj = project "MyProject"
		language "C#"
		kind "ConsoleApp"
		prepare()
		test.contains(removed, "MyProject.csproj")
		test.contains(removed, "MyProject.csproj.user")
		test.contains(removed, "MyProject.pdb")
		test.contains(removed, "MyProject.idb")
		test.contains(removed, "MyProject.ilk")
		test.contains(removed, "MyProject.make")
	end


	function T.clean.ObjectDirsAndFiles()
		prj = project "MyProject"
		language "C++"
		kind "ConsoleApp"
		prepare()
		test.contains(removed, "obj/Debug")
		test.contains(removed, "obj/Release")
	end


	function T.clean.CppConsoleAppFiles()
		prj = project "MyProject"
		language "C++"
		kind "ConsoleApp"
		prepare()
		test.contains(removed, "MyProject")
		test.contains(removed, "MyProject.exe")
		test.contains(removed, "MyProject.elf")
		test.contains(removed, "MyProject.vshost.exe")
		test.contains(removed, "MyProject.exe.manifest")
	end


	function T.clean.CppWindowedAppFiles()
		prj = project "MyProject"
		language "C++"
		kind "WindowedApp"
		prepare()
		test.contains(removed, "MyProject")
		test.contains(removed, "MyProject.exe")
		test.contains(removed, "MyProject.app")
	end
	

	function T.clean.CppSharedLibFiles()
		prj = project "MyProject"
		language "C++"
		kind "SharedLib"
		prepare()
		test.contains(removed, "MyProject.dll")
		test.contains(removed, "libMyProject.so")
		test.contains(removed, "MyProject.lib")
		test.contains(removed, "libMyProject.dylib")
	end


	function T.clean.CppStaticLibFiles()
		prj = project "MyProject"
		language "C++"
		kind "StaticLib"
		prepare()
		test.contains(removed, "MyProject.lib")
		test.contains(removed, "libMyProject.a")
	end


	function T.clean.PlatformObjects()
		platforms { "Native", "x32" }
		prj = project "MyProject"
		language "C++"
		kind "ConsoleApp"
		prepare()
		test.contains(removed, "obj/Debug")
		test.contains(removed, "obj/Release")
		test.contains(removed, "obj/x32/Debug")
		test.contains(removed, "obj/x32/Release")
	end


	function T.clean.CppConsoleAppFiles_OnSuffix()
		prj = project "MyProject"
		language "C++"
		kind "ConsoleApp"
		targetsuffix "_x"
		prepare()
		test.contains(removed, "MyProject_x")
		test.contains(removed, "MyProject_x.exe")
		test.contains(removed, "MyProject_x.elf")
		test.contains(removed, "MyProject_x.vshost.exe")
		test.contains(removed, "MyProject_x.exe.manifest")
	end

