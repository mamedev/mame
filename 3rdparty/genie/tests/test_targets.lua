--
-- tests/test_targets.lua
-- Automated test suite for premake.gettarget()
-- Copyright (c) 2008, 2009 Jason Perkins and the Premake project
--

	T.targets = { }

	local cfg
	function T.targets.setup()
		cfg = { }
		cfg.basedir    = "."
		cfg.location   = "."
		cfg.targetdir  = "../bin"
		cfg.language   = "C++"
		cfg.project    = { name = "MyProject" }
		cfg.flags      = { }
		cfg.objectsdir = "obj"
		cfg.platform   = "Native"
	end


--
--  Path Style   Name Style    Example Environment
--  ----------   ----------    -------------------
--   windows      windows       VStudio with MSC
--   posix        posix         GMake with GCC
--   windows      posix         VStudio for PS3
--   posix        windows       GMake for .NET
--




--
-- ConsoleApp tests
--

	function T.targets.ConsoleApp_Build_WindowsNames()
		cfg.kind = "ConsoleApp"
		result = premake.gettarget(cfg, "build", "posix", "windows", "macosx")
		test.isequal([[../bin/MyProject.exe]], result.fullpath)
	end

	function T.targets.ConsoleApp_Build_PosixNames_OnWindows()
		cfg.kind = "ConsoleApp"
		result = premake.gettarget(cfg, "build", "posix", "posix", "windows")
		test.isequal([[../bin/MyProject.exe]], result.fullpath)
	end

	function T.targets.ConsoleApp_Build_PosixNames_OnLinux()
		cfg.kind = "ConsoleApp"
		result = premake.gettarget(cfg, "build", "posix", "posix", "linux")
		test.isequal([[../bin/MyProject]], result.fullpath)
	end

	function T.targets.ConsoleApp_Build_PosixNames_OnMacOSX()
		cfg.kind = "ConsoleApp"
		result = premake.gettarget(cfg, "build", "posix", "posix", "macosx")
		test.isequal([[../bin/MyProject]], result.fullpath)
	end

	function T.targets.ConsoleApp_Build_PS3Names()
		cfg.kind = "ConsoleApp"
		result = premake.gettarget(cfg, "build", "posix", "PS3", "macosx")
		test.isequal([[../bin/MyProject.elf]], result.fullpath)
	end



--
-- WindowedApp tests
--

	function T.targets.WindowedApp_Build_WindowsNames()
		cfg.kind = "WindowedApp"
		result = premake.gettarget(cfg, "build", "posix", "windows", "macosx")
		test.isequal([[../bin/MyProject.exe]], result.fullpath)
	end

	function T.targets.WindowedApp_Build_PosixNames_OnWindows()
		cfg.kind = "WindowedApp"
		result = premake.gettarget(cfg, "build", "posix", "posix", "windows")
		test.isequal([[../bin/MyProject.exe]], result.fullpath)
	end

	function T.targets.WindowedApp_Build_PosixNames_OnLinux()
		cfg.kind = "WindowedApp"
		result = premake.gettarget(cfg, "build", "posix", "posix", "linux")
		test.isequal([[../bin/MyProject]], result.fullpath)
	end

	function T.targets.WindowedApp_Build_PosixNames_OnMacOSX()
		cfg.kind = "WindowedApp"
		result = premake.gettarget(cfg, "build", "posix", "posix", "macosx")
		test.isequal([[../bin/MyProject.app/Contents/MacOS/MyProject]], result.fullpath)
	end

	function T.targets.WindowedApp_Build_PS3Names()
		cfg.kind = "WindowedApp"
		result = premake.gettarget(cfg, "build", "posix", "PS3", "macosx")
		test.isequal([[../bin/MyProject.elf]], result.fullpath)
	end


--
-- SharedLib tests
--

	function T.targets.SharedLib_Build_WindowsNames()
		cfg.kind = "SharedLib"
		result = premake.gettarget(cfg, "build", "posix", "windows", "macosx")
		test.isequal([[../bin/MyProject.dll]], result.fullpath)
	end

	function T.targets.SharedLib_Link_WindowsNames()
		cfg.kind = "SharedLib"
		result = premake.gettarget(cfg, "link", "posix", "windows", "macosx")
		test.isequal([[../bin/MyProject.lib]], result.fullpath)
	end

	function T.targets.SharedLib_Build_PosixNames_OnWindows()
		cfg.kind = "SharedLib"
		result = premake.gettarget(cfg, "build", "posix", "posix", "windows")
		test.isequal([[../bin/MyProject.dll]], result.fullpath)
	end

	function T.targets.SharedLib_Link_PosixNames_OnWindows()
		cfg.kind = "SharedLib"
		result = premake.gettarget(cfg, "link", "posix", "posix", "windows")
		test.isequal([[../bin/libMyProject.a]], result.fullpath)
	end

	function T.targets.SharedLib_Build_PosixNames_OnLinux()
		cfg.kind = "SharedLib"
		result = premake.gettarget(cfg, "build", "posix", "posix", "linux")
		test.isequal([[../bin/libMyProject.so]], result.fullpath)
	end

	function T.targets.SharedLib_Link_PosixNames_OnLinux()
		cfg.kind = "SharedLib"
		result = premake.gettarget(cfg, "link", "posix", "posix", "linux")
		test.isequal([[../bin/libMyProject.so]], result.fullpath)
	end

	function T.targets.SharedLib_Build_PosixNames_OnMacOSX()
		cfg.kind = "SharedLib"
		result = premake.gettarget(cfg, "build", "posix", "posix", "macosx")
		test.isequal([[../bin/libMyProject.dylib]], result.fullpath)
	end

	function T.targets.SharedLib_Link_PosixNames_OnMacOSX()
		cfg.kind = "SharedLib"
		result = premake.gettarget(cfg, "link", "posix", "posix", "macosx")
		test.isequal([[../bin/libMyProject.dylib]], result.fullpath)
	end



--
-- StaticLib tests
--

	function T.targets.StaticLib_Build_WindowsNames()
		cfg.kind = "StaticLib"
		result = premake.gettarget(cfg, "build", "posix", "windows", "macosx")
		test.isequal([[../bin/MyProject.lib]], result.fullpath)
	end

	function T.targets.StaticLib_Link_WindowsNames()
		cfg.kind = "StaticLib"
		result = premake.gettarget(cfg, "link", "posix", "windows", "macosx")
		test.isequal([[../bin/MyProject.lib]], result.fullpath)
	end

	function T.targets.StaticLib_Build_PosixNames_OnWindows()
		cfg.kind = "StaticLib"
		result = premake.gettarget(cfg, "build", "posix", "posix", "windows")
		test.isequal([[../bin/libMyProject.a]], result.fullpath)
	end

	function T.targets.StaticLib_Link_PosixNames_OnWindows()
		cfg.kind = "StaticLib"
		result = premake.gettarget(cfg, "link", "posix", "posix", "windows")
		test.isequal([[../bin/libMyProject.a]], result.fullpath)
	end

	function T.targets.StaticLib_Build_PosixNames_OnLinux()
		cfg.kind = "StaticLib"
		result = premake.gettarget(cfg, "build", "posix", "posix", "linux")
		test.isequal([[../bin/libMyProject.a]], result.fullpath)
	end

	function T.targets.StaticLib_Link_PosixNames_OnLinux()
		cfg.kind = "StaticLib"
		result = premake.gettarget(cfg, "link", "posix", "posix", "linux")
		test.isequal([[../bin/libMyProject.a]], result.fullpath)
	end

	function T.targets.StaticLib_Build_PosixNames_OnMacOSX()
		cfg.kind = "StaticLib"
		result = premake.gettarget(cfg, "build", "posix", "posix", "macosx")
		test.isequal([[../bin/libMyProject.a]], result.fullpath)
	end

	function T.targets.StaticLib_Link_PosixNames_OnMacOSX()
		cfg.kind = "StaticLib"
		result = premake.gettarget(cfg, "link", "posix", "posix", "macosx")
		test.isequal([[../bin/libMyProject.a]], result.fullpath)
	end

	function T.targets.StaticLib_Build_PosixNames_OnPS3()
		cfg.kind = "StaticLib"
		result = premake.gettarget(cfg, "build", "posix", "PS3", "macosx")
		test.isequal([[../bin/libMyProject.a]], result.fullpath)
	end

	function T.targets.StaticLib_Link_PosixNames_OnPS3()
		cfg.kind = "StaticLib"
		result = premake.gettarget(cfg, "link", "posix", "PS3", "macosx")
		test.isequal([[../bin/libMyProject.a]], result.fullpath)
	end

	function T.targets.StaticLib_Link_IgnoresImpLib()
		cfg.kind = "StaticLib"
		cfg.implibdir = "../lib"
		result = premake.gettarget(cfg, "link", "posix", "posix", "macosx")
		test.isequal([[../bin/libMyProject.a]], result.fullpath)
	end



--
-- Windows path tests
--

	function T.targets.WindowsPaths()
		cfg.kind = "ConsoleApp"
		result = premake.gettarget(cfg, "build", "windows", "windows", "linux")
		test.isequal([[..\bin]], result.directory)
		test.isequal([[..\bin\MyProject.exe]], result.fullpath)
	end
