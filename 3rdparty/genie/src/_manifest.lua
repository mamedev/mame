--
-- _manifest.lua
-- Manage the list of built-in Premake scripts.
-- Copyright (c) 2002-2011 Jason Perkins and the Premake project
--

-- The master list of built-in scripts. Order is important! If you want to
-- build a new script into Premake, add it to this list.

	return
	{
		-- core files
		"base/os.lua",
		"base/path.lua",
		"base/string.lua",
		"base/table.lua",
		"base/io.lua",
		"base/globals.lua",
		"base/action.lua",
		"base/option.lua",
		"base/tree.lua",
		"base/solution.lua",
		"base/project.lua",
		"base/config.lua",
		"base/bake.lua",
		"base/api.lua",
		"base/cmdline.lua",
		"base/inspect.lua",
		"base/profiler.lua",
		"tools/dotnet.lua",
		"tools/gcc.lua",
		"tools/ghs.lua",
		"tools/msc.lua",
		"tools/ow.lua",
		"tools/snc.lua",
		"tools/valac.lua",
		"tools/swift.lua",
		"base/validate.lua",
		"base/help.lua",
		"base/premake.lua",
		"base/iter.lua",
		"base/set.lua",

		-- CMake action
		"actions/cmake/_cmake.lua",
		"actions/cmake/cmake_workspace.lua",
		"actions/cmake/cmake_project.lua",

		-- GNU make action
		"actions/make/_make.lua",
		"actions/make/make_solution.lua",
		"actions/make/make_cpp.lua",
		"actions/make/make_csharp.lua",
		"actions/make/make_vala.lua",
		"actions/make/make_swift.lua",

		-- Visual Studio actions
		"actions/vstudio/_vstudio.lua",
		"actions/vstudio/vstudio_solution.lua",
		"actions/vstudio/vstudio_vcxproj.lua",
		"actions/vstudio/vstudio_vcxproj_filters.lua",
		"actions/vstudio/vs2010.lua",
		"actions/vstudio/vs2012.lua",
		"actions/vstudio/vs2013.lua",
		"actions/vstudio/vs2015.lua",
		"actions/vstudio/vs2017.lua",
		"actions/vstudio/vs2019.lua",
		"actions/vstudio/vs2022.lua",

		-- Xcode action
		"actions/xcode/_xcode.lua",
		"actions/xcode/xcode_common.lua",
		"actions/xcode/xcode_project.lua",
		"actions/xcode/xcode_scheme.lua",
		"actions/xcode/xcode_workspace.lua",
		"actions/xcode/xcode8.lua",
		"actions/xcode/xcode9.lua",
		"actions/xcode/xcode10.lua",
		"actions/xcode/xcode11.lua",
		"actions/xcode/xcode14.lua",
		"actions/xcode/xcode15.lua",

		-- ninja action
		"actions/ninja/_ninja.lua",
		"actions/ninja/ninja_base.lua",
		"actions/ninja/ninja_solution.lua",
		"actions/ninja/ninja_cpp.lua",
		"actions/ninja/ninja_swift.lua",
		"actions/ninja/ninja_swift_incremental.lua",

		-- jcdb action
		"actions/jcdb/_jcdb.lua",
		"actions/jcdb/jcdb_solution.lua",
	}
