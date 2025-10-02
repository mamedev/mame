--
-- xcode_scheme.lua
--

local premake = premake
local xcode   = premake.xcode


--
-- Print a BuildableReference element.
--

	local function buildableref(indent, prj, cfg)
		cfg = cfg or premake.eachconfig(prj)()

		_p(indent + 0, '<BuildableReference')
		_p(indent + 1, 'BuildableIdentifier = "primary"')
		_p(indent + 1, 'BlueprintIdentifier = "%s"', prj.xcode.projectnode.targetid)
		_p(indent + 1, 'BuildableName = "%s"', cfg.buildtarget.name)
		_p(indent + 1, 'BlueprintName = "%s"', prj.name)
		_p(indent + 1, 'ReferencedContainer = "container:%s.xcodeproj">', prj.name)
		_p(indent + 0, '</BuildableReference>')
	end


--
-- Print a CommandLineArgs element, if needed by the provided config.
--

	local function cmdlineargs(indent, cfg)
		if #cfg.debugargs > 0 then
			_p(indent, '<CommandLineArguments>')
			for _, arg in ipairs(cfg.debugargs) do
				_p(indent + 1, '<CommandLineArgument')
				_p(indent + 2, 'argument = "%s"', arg)
				_p(indent + 2, 'isEnabled = "YES">')
				_p(indent + 1, '</CommandLineArgument>')
			end
			_p(indent, '</CommandLineArguments>')
		end
	end


--
-- Print an EnvironmentVariables element, if needed by the provided config.

	local function envvars(indent, cfg)
		if #cfg.debugenvs > 0 then
			_p(indent, '<EnvironmentVariables>')
			for _, arg in ipairs(cfg.debugenvs) do
				local eq = arg:find("=")
				local k = arg:sub(1, eq)
				local v = arg:sub(eq + 1)
				_p(indent + 1, '<EnvironmentVariable')
				_p(indent + 2, 'key = "%s"', arg:sub(1, eq))
				_p(indent + 2, 'value = "%s"', arg:sub(eq))
				_p(indent + 2, 'isEnabled = "YES">')
				_p(indent + 1, '</EnvironmentVariable>')
			end
			_p(indent, '</EnvironmentVariables>')
		end
	end


--
-- Generate the customWorkingDir path from the given dir.
--

	local function workingdir(dir)
		if not path.isabsolute(dir) then
			dir = "$PROJECT_DIR/" .. dir
		end
		return dir
	end


--
-- Determine the best matching configuration.
--
-- @param fordebug
-- 	  True to find a debug configuration.
--

	local function bestconfig(prj, fordebug)
		local bestcfg = nil
		local bestscore = -1

		for cfg in premake.eachconfig(prj) do
			local score = 0

			if cfg.platform == "Native" then
				score = score + 10
			end

			if fordebug and cfg.name == "Debug" then
				score = score + 1
			end

			if not fordebug and cfg.name == "Release" then
				score = score + 1
			end

			if score > bestscore then
				bestcfg = cfg
				bestscore = score
			end
		end

		return bestcfg
	end


--
-- Print out an xcscheme file.
--
-- @param tobuild
--    The list of targets to build. Assumed to be sorted.
-- @param primary
--    The target to set as test/profile/launch target.
--

	function xcode.scheme(tobuild, primary, schemecfg)
		_p('<?xml version="1.0" encoding="UTF-8"?>')
		_p('<Scheme')
		_p(1, 'LastUpgradeVersion = "0940"')
		_p(1, 'version = "1.3">')
		_p(1, '<BuildAction')
		_p(2, 'parallelizeBuildables = "YES"')
		_p(2, 'buildImplicitDependencies = "YES">')
		_p(2, '<BuildActionEntries>')

		for _, prj in ipairs(tobuild) do
			_p(3, '<BuildActionEntry')
			_p(4, 'buildForTesting = "YES"')
			_p(4, 'buildForRunning = "YES"')
			_p(4, 'buildForProfiling = "YES"')
			_p(4, 'buildForArchiving = "YES"')
			_p(4, 'buildForAnalyzing = "YES">')
			buildableref(4, prj)
			_p(3, '</BuildActionEntry>')
		end

		local debugcfg    = schemecfg or bestconfig(primary, true)
		local releasecfg  = schemecfg or bestconfig(primary, false)
		local debugname   = xcode.getconfigname(debugcfg)
		local releasename = xcode.getconfigname(releasecfg)

		_p(2, '</BuildActionEntries>')
		_p(1, '</BuildAction>')
		_p(1, '<TestAction')
		_p(2, 'buildConfiguration = "%s"', debugname)
		_p(2, 'selectedDebuggerIdentifier = "Xcode.DebuggerFoundation.Debugger.LLDB"')
		_p(2, 'selectedLauncherIdentifier = "Xcode.DebuggerFoundation.Launcher.LLDB"')
		_p(2, 'shouldUseLaunchSchemeArgsEnv = "YES">')
		_p(2, '<Testables>')
		_p(2, '</Testables>')
		_p(2, '<MacroExpansion>')
		buildableref(3, primary, debugcfg)
		_p(2, '</MacroExpansion>')
		_p(2, '<AdditionalOptions>')
		_p(2, '</AdditionalOptions>')
		_p(1, '</TestAction>')
		_p(1, '<LaunchAction')
		_p(2, 'buildConfiguration = "%s"', debugname)
		_p(2, 'selectedDebuggerIdentifier = "Xcode.DebuggerFoundation.Debugger.LLDB"')
		_p(2, 'selectedLauncherIdentifier = "Xcode.DebuggerFoundation.Launcher.LLDB"')
		_p(2, 'launchStyle = "0"')
		if debugcfg.debugdir then
			_p(2, 'useCustomWorkingDirectory = "YES"')
			_p(2, 'customWorkingDirectory = "%s"', workingdir(debugcfg.debugdir))
		else
			_p(2, 'useCustomWorkingDirectory = "NO"')
		end
		_p(2, 'ignoresPersistentStateOnLaunch = "NO"')
		_p(2, 'debugDocumentVersioning = "YES"')
		_p(2, 'debugServiceExtension = "internal"')
		_p(2, 'allowLocationSimulation = "YES">')
		if debugcfg.debugcmd then
			_p(2, '<PathRunnable')
			_p(3, 'runnableDebuggingMode = "0"')
			_p(3, 'FilePath = "%s">', debugcfg.debugcmd)
			_p(2, '</PathRunnable>')
		else
			_p(2, '<BuildableProductRunnable')
			_p(3, 'runnableDebuggingMode = "0">')
			buildableref(3, primary, debugcfg)
			_p(2, '</BuildableProductRunnable>')
		end
		cmdlineargs(2, debugcfg)
		envvars(2, debugcfg)
		_p(2, '<AdditionalOptions>')
		_p(2, '</AdditionalOptions>')
		_p(1, '</LaunchAction>')
		_p(1, '<ProfileAction')
		_p(2, 'buildConfiguration = "%s"', releasename)
		_p(2, 'shouldUseLaunchSchemeArgsEnv = "YES"')
		_p(2, 'savedToolIdentifier = ""')
		if releasecfg.debugdir then
			_p(2, 'useCustomWorkingDirectory = "YES"')
			_p(2, 'customWorkingDirectory = "%s"', workingdir(releasecfg.debugdir))
		else
			_p(2, 'useCustomWorkingDirectory = "NO"')
		end
		_p(2, 'debugDocumentVersioning = "YES">')
		_p(2, '<BuildableProductRunnable')
		_p(3, 'runnableDebuggingMode = "0">')
		buildableref(3, primary, releasecfg)
		_p(2, '</BuildableProductRunnable>')
		cmdlineargs(2, releasecfg)
		envvars(2, releasecfg)
		_p(1, '</ProfileAction>')
		_p(1, '<AnalyzeAction')
		_p(2, 'buildConfiguration = "%s">', debugname)
		_p(1, '</AnalyzeAction>')
		_p(1, '<ArchiveAction')
		_p(2, 'buildConfiguration = "%s"', releasename)
		_p(2, 'revealArchiveInOrganizer = "YES">')
		_p(1, '</ArchiveAction>')
		_p('</Scheme>')
	end

--
-- Generate XCode schemes for the given project.
--

	function xcode.generate_schemes(prj, base_path)
		if (prj.kind == "ConsoleApp" or prj.kind == "WindowedApp") or (prj.options and prj.options.XcodeLibrarySchemes) then

			if prj.options and prj.options.XcodeSchemeNoConfigs then
				premake.generate(prj, path.join(base_path, "%%.xcscheme"),
					function(prj) xcode.scheme({prj}, prj) end)
			else
				for cfg in premake.eachconfig(prj) do
					premake.generate(prj, path.join(base_path, "%% " .. cfg.name .. ".xcscheme"),
						function(prj) xcode.scheme({prj}, prj, cfg) end)
				end
			end
		end
	end

