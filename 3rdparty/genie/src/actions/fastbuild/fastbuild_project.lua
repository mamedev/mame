-- Generates a FASTBuild config file for a project.

-- Note that table order iteration should be deterministic, so the .bff file content is not
-- arbitrarily changed each time it's generated. There are several places in this file
-- where sorts are done for that reason.

-- Remaining flags to handle:
--		NoImportLib = 1,
--		NoIncrementalLink = 1,
--		NoManifest = 1,
--		NoPCH = 1,
--		SingleOutputDir = 1,
--		StaticATL = 1,
--		Symbols = 1,
--		Unicode = 1,
--		Unsafe = 1,
--		WinMain = 1,
-- API todo:
--		custombuildtask
--		dependency

local function add_trailing_backslash(dir)
	if dir:len() > 0 and dir:sub(-1) ~= "\\" then
		return dir.."\\"
	end
	return dir
end

local function compile(indentlevel, prj, cfg, commonbasepath)

	local firstflag = true
	for _, cfgexclude in ipairs(cfg.excludes) do
		if path.issourcefile(cfgexclude) then
			if firstflag then
				_p(indentlevel, '// Excluded files:')
				firstflag = false
			end
			_p(indentlevel, ".CompilerInputFiles - '%s'", cfgexclude)
		end
	end
	if not firstflag then
		_p('')
	end

	_p(indentlevel, ".CompilerOutputPath = '%s'", add_trailing_backslash(cfg.objectsdir))

	_p(indentlevel, ".Defines = ''")
	for _, define in ipairs(cfg.defines) do
		_p(indentlevel+1, "+ ' /D%s'", define)
	end

	if cfg.kind == 'SharedLib' then
		_p(indentlevel+1, "+ ' /D_WINDLL'")
	end

	_p(indentlevel, ".IncludeDirs = ''")
	local sortedincdirs = table.join(cfg.userincludedirs, cfg.includedirs, cfg.systemincludedirs)

	-- Setup for special include dir sort to ensure that 'nearby' dirs get precedence over others.
	-- Gets the relative path from commonbasepath and counts the steps in that path.
	local function getpathnodecount(p)
		local nodefinder = string.gmatch(p, "[^\\/]+")
		local result = 0
		local node = nodefinder()
		while node do
			result = result + ((node ~= '.' and 1) or 0)
			node = nodefinder()
		end
		return result
	end

	local stepsfrombase = {}

	for _, includedir in ipairs(sortedincdirs) do
		stepsfrombase[includedir] = getpathnodecount(path.getrelative(commonbasepath, includedir))
	end

	local function includesort(a, b)
		if stepsfrombase[a] == stepsfrombase[b] then
			return a < b
		else
			return stepsfrombase[a] < stepsfrombase[b]
		end
	end

	table.sort(sortedincdirs, includesort)
	for _, includedir in ipairs(sortedincdirs) do
		_p(indentlevel+1, "+ ' /I\"%s\"'", includedir)
	end

	_p(indentlevel+1, "+ .MSVCIncludes")

	local compileroptions = {
		'"%1"',
		'/nologo',
		'/c',
		'/Gm-',
		'/Zc:inline',
		'/errorReport:prompt',
		'/FS',
		}

	if cfg.options.ForceCPP then
		table.insert(compileroptions, '/TP')
	end

	if cfg.flags.PedanticWarnings
	or cfg.flags.ExtraWarnings
	then
		table.insert(compileroptions, '/W4')
	end

	if (cfg.flags.NativeWChar == false or
		cfg.flags.NoNativeWChar) then
		table.insert(compileroptions, '/Zc:wchar_t-')
	end

	if (cfg.flags.EnableMinimalRebuild or
		cfg.flags.NoMultiProcessorCompilation) then
		-- Not compatible with FastBuild
	end

	if cfg.flags.FloatFast then
		table.insert(compileroptions, '/fp:fast')
	elseif cfg.flags.FloatStrict then
		table.insert(compileroptions, '/fp:strict')
	else
		table.insert(compileroptions, '/fp:precise')
	end

	if cfg.flags.FastCall then
		table.insert(compileroptions, '/Gr')
	elseif cfg.flags.StdCall then
		table.insert(compileroptions, '/Gd')
	end

	if cfg.flags.UnsignedChar then
		table.insert(compileroptions, '/J')
	end

	if premake.config.isdebugbuild(cfg) then
		if cfg.flags.StaticRuntime then
			table.insert(compileroptions, '/MTd')
		else
			table.insert(compileroptions, '/MDd')
		end
	else
		if cfg.flags.StaticRuntime then
			table.insert(compileroptions, '/MT')
		else
			table.insert(compileroptions, '/MD')
		end
	end

	if cfg.flags.Symbols then
		if (cfg.flags.C7DebugInfo) then
			table.insert(compileroptions, '/Z7')
		else
			if premake.config.iseditandcontinue(cfg) then
				table.insert(compileroptions, '/ZI')
			else
				table.insert(compileroptions, '/Zi')
			end
			local targetdir = add_trailing_backslash(cfg.buildtarget.directory)
			table.insert(compileroptions, string.format("/Fd\"%s%s.pdb\"", targetdir, cfg.buildtarget.basename))
		end
	end

	local isoptimised = true
	if (cfg.flags.Optimize) then
		table.insert(compileroptions, '/Ox')
	elseif (cfg.flags.OptimizeSize) then
		table.insert(compileroptions, '/O1')
	elseif (cfg.flags.OptimizeSpeed) then
		table.insert(compileroptions, '/O2')
	else
		isoptimised = false
	end

	if isoptimised then
		-- Refer to vstudio.vcxproj.lua about FunctionLevelLinking
		if cfg.flags.NoOptimizeLink and cfg.flags.NoEditAndContinue then
			table.insert(compileroptions, '/GF-')
			table.insert(compileroptions, '/Gy-')
		else
			table.insert(compileroptions, '/GF')
			table.insert(compileroptions, '/Gy')
		end
	else
		table.insert(compileroptions, '/Gy')
		table.insert(compileroptions, '/Od')
		table.insert(compileroptions, '/RTC1')
	end

	if cfg.flags.FatalWarnings then
		table.insert(compileroptions, '/WX')
	else
		table.insert(compileroptions, '/WX-')
	end

	if cfg.platform == 'x32' then
		if cfg.flags.EnableSSE2 then
			table.insert(compileroptions, '/arch:SSE2')
		elseif cfg.flags.EnableSSE then
			table.insert(compileroptions, '/arch:SSE')
		end
	end

	if cfg.flags.NoExceptions then
	else
		if cfg.flags.SEH then
			table.insert(compileroptions, '/EHa')
		else
			table.insert(compileroptions, '/EHsc')
		end
	end

	if cfg.flags.NoRTTI then
		table.insert(compileroptions, '/GR-')
	else
		table.insert(compileroptions, '/GR')
	end

	if cfg.flags.NoFramePointer then
		table.insert(compileroptions, '/Oy-')
	else
		table.insert(compileroptions, '/Oy')
	end

	for _, addloption in ipairs(cfg.buildoptions) do
		table.insert(compileroptions, addloption)
	end

	_p(indentlevel, ".CompilerOptions = ''")
	for _, option in ipairs(compileroptions) do
		_p(indentlevel+1, "+ ' %s'", option)
	end

	_p(indentlevel+1, "+ .IncludeDirs")
	_p(indentlevel+1, "+ .Defines")

	if not cfg.flags.NoPCH and cfg.pchheader then
		_p(indentlevel, ".CompilerInputFiles - '%s'", cfg.pchsource)
		_p(indentlevel, ".PCHInputFile = '%s'", cfg.pchsource)
		_p(indentlevel, ".PCHOutputFile = .CompilerOutputPath + '%s.pch'", prj.name)
		_p(indentlevel, ".CompilerOptions + ' /Fp\"' + .CompilerOutputPath + '%s.pch\"'", prj.name)
		_p(indentlevel, ".PCHOptions = .CompilerOptions")
		_p(indentlevel+1, "+ ' /Yc\"%s\"'", cfg.pchheader)
		_p(indentlevel+1, "+ ' /Fo\"%%3\"'")
		_p(indentlevel, ".CompilerOptions + ' /Yu\"%s\"'", cfg.pchheader)
	end
	_p(indentlevel+1, "+ ' /Fo\"%%2\"'") -- make sure the previous property is .CompilerOptions
end

local function library(prj, cfg, useconfig, commonbasepath)
	_p(1, "Library('%s-%s-%s')", prj.name, cfg.name, cfg.platform)
	_p(1, '{')

	useconfig(2)
	compile(2, prj, cfg, commonbasepath)

	local librarianoptions = {
		'"%1"',
		'/OUT:"%2"',
		'/NOLOGO',
		}

	if cfg.platform == 'x64' then
		table.insert(librarianoptions, '/MACHINE:X64')
	else
		table.insert(librarianoptions, '/MACHINE:X86')
	end

	_p(2, ".LibrarianOptions = ''")
	for _, option in ipairs(librarianoptions) do
		_p(3, "+ ' %s'", option)
	end

	_p(2, ".LibrarianOutput = '%s'", cfg.buildtarget.fullpath)

	_p(1, '}')
	_p('')
end

local function binary(prj, cfg, useconfig, bintype, commonbasepath)
	_p(1, "ObjectList('%s_obj-%s-%s')", prj.name, cfg.name, cfg.platform)
	_p(1, '{')

	useconfig(2)
	compile(2, prj, cfg, commonbasepath)

	_p(1, '}')
	_p('')

	_p(1, "%s('%s-%s-%s')", bintype, prj.name, cfg.name, cfg.platform)
	_p(1, '{')

	useconfig(2)
	_p(2, '.Libraries = {')
	_p(3, "'%s_obj-%s-%s',", prj.name, cfg.name, cfg.platform) -- Refer to the ObjectList

	local sorteddeplibs = {}
	for _, deplib in ipairs(premake.getlinks(cfg, "dependencies", "basename")) do
		table.insert(sorteddeplibs, string.format("'%s-%s-%s',", deplib, cfg.name, cfg.platform))
	end
	table.sort(sorteddeplibs)
	for _, deplib in ipairs(sorteddeplibs) do
		_p(3, deplib)
	end
	_p(3, '}')

	_p(2, '.LinkerLinkObjects = false')

	local linkeroptions = {
		'"%1"',
		'/OUT:"%2"',
		'/NOLOGO',
		'/errorReport:prompt',
		}

	local subsystemversion = '",5.02"'
	if cfg.platform == 'x32' then
		subsystemversion = '",5.01"'
	end

	if cfg.kind == 'ConsoleApp' then
		table.insert(linkeroptions, '/SUBSYSTEM:CONSOLE'..subsystemversion)
	else
		table.insert(linkeroptions, '/SUBSYSTEM:WINDOWS'..subsystemversion)
	end

	if cfg.kind == 'SharedLib' then
		table.insert(linkeroptions, '/DLL')
	end

	if cfg.platform == 'x64' then
		table.insert(linkeroptions, '/MACHINE:X64')
	else
		table.insert(linkeroptions, '/MACHINE:X86')
	end

	for _, libdir in ipairs(cfg.libdirs) do
		table.insert(linkeroptions, string.format('/LIBPATH:%s', path.translate(libdir, '\\')))
	end

	if not cfg.flags.WinMain and (cfg.kind == 'ConsoleApp' or cfg.kind == 'WindowedApp') then
		if cfg.flags.Unicode then
			table.insert(linkeroptions, '/ENTRY:wmainCRTStartup')
		else
			table.insert(linkeroptions, '/ENTRY:mainCRTStartup')
		end
	end

	local deffile = premake.findfile(cfg, ".def")
	if deffile then
		table.insert(linkeroptions, '/DEF:%s', deffile)
	end

	if (cfg.flags.Symbols ~= nil) then
		table.insert(linkeroptions, '/DEBUG')
	end

	if premake.config.islinkeroptimizedbuild(cfg.flags) then
		table.insert(linkeroptions, '/OPT:REF')
		table.insert(linkeroptions, '/OPT:ICF')
	end

	table.insert(linkeroptions, '/INCREMENTAL'..((premake.config.isincrementallink(cfg) and '') or ':NO'))

	for _, addloption in ipairs(cfg.linkoptions) do
		table.insert(linkeroptions, addloption)
	end

	local linklibs = premake.getlinks(cfg, "all", "fullpath")

	for _, linklib in ipairs(linklibs) do
		table.insert(linkeroptions, path.getname(linklib))
	end

	table.sort(linkeroptions)

	_p(2, ".LinkerOptions = ''")
	for _, option in ipairs(linkeroptions) do
		_p(3, "+ ' %s'", option)
	end
	_p(3, "+ .MSVCLibPaths")

	_p(2, ".LinkerOutput = '%s'", cfg.buildtarget.fullpath)

	_p(1, '}')
	_p('')
end

local function executable(prj, cfg, useconfig, commonbasepath)
	binary(prj, cfg, useconfig, 'Executable', commonbasepath)
end

local function dll(prj, cfg, useconfig, commonbasepath)
	binary(prj, cfg, useconfig, 'DLL', commonbasepath)
end

local function alias(prj, cfg, target)
	_p(1, "Alias('%s-%s-%s')", prj.name, cfg.name, cfg.platform)
	_p(1, '{')
	_p(2, ".Targets = '%s'", target)
	_p(1, '}')
	_p('')
end

function premake.fastbuild.project(prj)
	io.indent = '    '
	_p('// FASTBuild project configuration file autogenerated by GENie.')
	_p('')

	_p('{')

	local cppfiles = {}
	for file in premake.project.eachfile(prj) do
		if path.issourcefile(file.name) then
			table.insert(cppfiles, file.name)
		end
	end

	local commonbasepath = cppfiles[1]
	for _, file in ipairs(cppfiles) do
		commonbasepath = path.getcommonbasedir(commonbasepath..'/', file)
	end

	_p(1, ".CompilerInputFilesRoot = '%s/'", commonbasepath)
	_p(1, '.CompilerInputFiles = {')
	for _, file in ipairs(cppfiles) do
		_p(2, "'%s',", file)
	end
	_p(1, '}')
	_p('')

	local targetkindmap = {
		ConsoleApp = executable,
		WindowedApp = executable,
		SharedLib = dll,
		StaticLib = library,
		}

	local useconfigmap = {
		x32 = function(indentlevel)
				_p(indentlevel, 'Using(.MSVCx86Config)')
				_p('')
			end,
		x64 = function(indentlevel)
				_p(indentlevel, 'Using(.MSVCx64Config)')
				_p('')
			end,
		}

	local nativeplatform = (os.is64bit() and 'x64') or 'x32'

	for _, platform in ipairs(prj.solution.platforms) do
		for cfg in premake.eachconfig(prj, platform) do
			if cfg.platform == 'Native' then
				alias(prj, cfg, string.format('%s-%s-%s', prj.name, cfg.name, nativeplatform))
			else
				targetkindmap[prj.kind](prj, cfg, useconfigmap[cfg.platform], commonbasepath)
			end
		end
	end
	_p('}')

end
