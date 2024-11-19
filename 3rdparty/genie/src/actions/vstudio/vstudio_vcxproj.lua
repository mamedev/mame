--
-- vs2010_vcxproj.lua
-- Generate a Visual Studio 2010 C/C++ project.
-- Copyright (c) 2009-2011 Jason Perkins and the Premake project
--

	premake.vstudio.vc2010 = { }
	local vc2010 = premake.vstudio.vc2010
	local vstudio = premake.vstudio


	local function vs2010_config(prj)
		-- only include this bit if there's a Tegra platform in there
		for _, cfginfo in ipairs(prj.solution.vstudio_configs) do
			if cfginfo.src_platform == "TegraAndroid" then
				_p(1,'<PropertyGroup Label="NsightTegraProject">')
					_p(2,'<NsightTegraProjectRevisionNumber>11</NsightTegraProjectRevisionNumber>')
				_p(1,'</PropertyGroup>')
				break
			end
		end
		_p(1,'<ItemGroup Label="ProjectConfigurations">')
		for _, cfginfo in ipairs(prj.solution.vstudio_configs) do
				_p(2,'<ProjectConfiguration Include="%s">', premake.esc(cfginfo.name))
					_p(3,'<Configuration>%s</Configuration>',cfginfo.buildcfg)
					_p(3,'<Platform>%s</Platform>',cfginfo.platform)
				_p(2,'</ProjectConfiguration>')
		end
		_p(1,'</ItemGroup>')
	end

	local function vs2010_globals(prj)
		local action = premake.action.current()
		_p(1,'<PropertyGroup Label="Globals">')
			_p(2, '<ProjectGuid>{%s}</ProjectGuid>',prj.uuid)
			_p(2, '<RootNamespace>%s</RootNamespace>',prj.name)
			if vstudio.storeapp ~= "durango" then
				local windowsTargetPlatformVersion = prj.windowstargetplatformversion or action.vstudio.windowsTargetPlatformVersion
				if windowsTargetPlatformVersion ~= nil then
					_p(2,'<WindowsTargetPlatformVersion>%s</WindowsTargetPlatformVersion>',windowsTargetPlatformVersion)

					if windowsTargetPlatformVersion and string.startswith(windowsTargetPlatformVersion, "10.") then
						_p(2,'<WindowsTargetPlatformMinVersion>%s</WindowsTargetPlatformMinVersion>', prj.windowstargetplatformminversion or "10.0.10240.0")
					end
				end
			end
		--if prj.flags is required as it is not set at project level for tests???
		--vs200x generator seems to swap a config for the prj in test setup
		if prj.flags and prj.flags.Managed then
			local frameworkVersion = prj.framework or "4.0"
			_p(2, '<TargetFrameworkVersion>v%s</TargetFrameworkVersion>', frameworkVersion)
			_p(2, '<Keyword>ManagedCProj</Keyword>')
		elseif vstudio.iswinrt() then
			_p(2, '<DefaultLanguage>en-US</DefaultLanguage>')
			if vstudio.storeapp == "durango" then
				_p(2, '<Keyword>Win32Proj</Keyword>')
				_p(2, '<ApplicationEnvironment>title</ApplicationEnvironment>')
				_p(2, '<MinimumVisualStudioVersion>14.0</MinimumVisualStudioVersion>')
				_p(2, '<TargetRuntime>Native</TargetRuntime>')
			else
				_p(2, '<AppContainerApplication>true</AppContainerApplication>')
				_p(2, '<MinimumVisualStudioVersion>12.0</MinimumVisualStudioVersion>')
				_p(2, '<ApplicationType>Windows Store</ApplicationType>')
				_p(2, '<ApplicationTypeRevision>%s</ApplicationTypeRevision>', vstudio.storeapp)
			end
		else
			_p(2, '<Keyword>Win32Proj</Keyword>')
		end

		if not vstudio.xpwarning then
			_p(2, '<XPDeprecationWarning>false</XPDeprecationWarning>')
		end

		_p(1,'</PropertyGroup>')
	end

	function vc2010.config_type(config)
		local t =
		{
			SharedLib = "DynamicLibrary",
			StaticLib = "StaticLibrary",
			ConsoleApp = "Application",
			WindowedApp = "Application"
		}
		return t[config.kind]
	end



	local function if_config_and_platform()
		return 'Condition="\'$(Configuration)|$(Platform)\'==\'%s\'"'
	end

	local function optimisation(cfg)
		local result = "Disabled"
		for _, value in ipairs(cfg.flags) do
			if (value == "Optimize") then
				result = "Full"
			elseif (value == "OptimizeSize") then
				result = "MinSpace"
			elseif (value == "OptimizeSpeed") then
				result = "MaxSpeed"
			end
		end
		return result
	end


--
-- This property group describes a particular configuration: what
-- kind of binary it produces, and some global settings.
--

	function vc2010.configurationPropertyGroup(cfg, cfginfo)
		_p(1, '<PropertyGroup '..if_config_and_platform() ..' Label="Configuration">'
			, premake.esc(cfginfo.name))

		local is2019 = premake.action.current() == premake.action.get("vs2019")
		local is2022 = premake.action.current() == premake.action.get("vs2022")
		if is2019 or is2022 then
		    _p(2, '<VCProjectVersion>%s</VCProjectVersion>', action.vstudio.toolsVersion)
			if cfg.flags.UnitySupport then
			    _p(2, '<EnableUnitySupport>true</EnableUnitySupport>')
			end
		end
		_p(2, '<ConfigurationType>%s</ConfigurationType>', vc2010.config_type(cfg))
		_p(2, '<UseDebugLibraries>%s</UseDebugLibraries>', iif(optimisation(cfg) == "Disabled","true","false"))
		_p(2, '<PlatformToolset>%s</PlatformToolset>',     premake.vstudio.toolset)

		if os.is64bit() then
			_p(2, '<PreferredToolArchitecture>x64</PreferredToolArchitecture>')
		end

		if cfg.flags.Unicode then
			_p(2,'<CharacterSet>Unicode</CharacterSet>')
		end

		if cfg.flags.Managed then
			_p(2,'<CLRSupport>true</CLRSupport>')
		end

		if cfg.platform == "TegraAndroid" then
			if cfg.androidtargetapi then
				_p(2,'<AndroidTargetAPI>android-%s</AndroidTargetAPI>', cfg.androidtargetapi)
			end
			if cfg.androidminapi then
				_p(2,'<AndroidMinAPI>android-%s</AndroidMinAPI>', cfg.androidminapi)
			end
			if cfg.androidarch then
				_p(2,'<AndroidArch>%s</AndroidArch>', cfg.androidarch)
			end
			if cfg.androidndktoolchainversion then
				_p(2,'<NdkToolchainVersion>%s</NdkToolchainVersion>', cfg.androidndktoolchainversion)
			end
			if cfg.androidstltype then
				_p(2,'<AndroidStlType>%s</AndroidStlType>', cfg.androidstltype)
			end
		end

		if cfg.platform == "NX32" or cfg.platform == "NX64" then
			_p(2,'<NintendoSdkRoot>$(NINTENDO_SDK_ROOT)\\</NintendoSdkRoot>')
			_p(2,'<NintendoSdkSpec>NX</NintendoSdkSpec>')
			--TODO: Allow specification of the 'Develop' build type
			if premake.config.isdebugbuild(cfg) then
				_p(2,'<NintendoSdkBuildType>Debug</NintendoSdkBuildType>')
			else
				_p(2,'<NintendoSdkBuildType>Release</NintendoSdkBuildType>')
			end
		end

		-- Workaround for https://github.com/Microsoft/msbuild/issues/2353
		if cfg.flags.Symbols and (premake.action.current() == premake.action.get("vs2017") or is2019) then
			_p(2, '<DebugSymbols>true</DebugSymbols>')
		end

		_p(1,'</PropertyGroup>')
	end


	local function import_props(prj)
		for _, cfginfo in ipairs(prj.solution.vstudio_configs) do
			local cfg = premake.getconfig(prj, cfginfo.src_buildcfg, cfginfo.src_platform)
			_p(1,'<ImportGroup '..if_config_and_platform() ..' Label="PropertySheets">'
					,premake.esc(cfginfo.name))
				_p(2,'<Import Project="$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props" Condition="exists(\'$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\')" Label="LocalAppDataPlatform" />')

			if #cfg.propertysheets > 0 then
				local dirs = cfg.propertysheets
				for _, dir in ipairs(dirs) do
					local translated = path.translate(dir)
					_p(2,'<Import Project="%s" Condition="exists(\'%s\')" />', translated, translated)
				end
			end

			_p(1,'</ImportGroup>')
		end
	end

	local function add_trailing_backslash(dir)
		if dir:len() > 0 and dir:sub(-1) ~= "\\" then
			return dir.."\\"
		end
		return dir
	end

	function vc2010.outputProperties(prj)
		for _, cfginfo in ipairs(prj.solution.vstudio_configs) do
			local cfg = premake.getconfig(prj, cfginfo.src_buildcfg, cfginfo.src_platform)
			local target = cfg.buildtarget
			local outdir = add_trailing_backslash(target.directory)
			local intdir = add_trailing_backslash(iif(action.vstudio.intDirAbsolute
							, path.translate(
								  path.join(prj.solution.location, cfg.objectsdir)
								, '\\')
							, cfg.objectsdir
							))

			_p(1,'<PropertyGroup '..if_config_and_platform() ..'>', premake.esc(cfginfo.name))

			_p(2,'<OutDir>%s</OutDir>', iif(outdir:len() > 0, premake.esc(outdir), ".\\"))

			if cfg.platform == "Xbox360" then
				_p(2,'<OutputFile>$(OutDir)%s</OutputFile>', premake.esc(target.name))
			end

			_p(2,'<IntDir>%s</IntDir>', premake.esc(intdir))
			_p(2,'<TargetName>%s</TargetName>', premake.esc(path.getbasename(target.name)))
			_p(2,'<TargetExt>%s</TargetExt>', premake.esc(path.getextension(target.name)))

			if cfg.kind == "SharedLib" then
				local ignore = (cfg.flags.NoImportLib ~= nil)
				_p(2,'<IgnoreImportLibrary>%s</IgnoreImportLibrary>', tostring(ignore))
			end

			if cfg.platform == "NX32" or cfg.platform == "NX64" then
				if cfg.flags.Cpp17 then
					_p(2,'<CppLanguageStandard>Gnu++17</CppLanguageStandard>')
				elseif cfg.flags.Cpp20 then
					_p(2,'<CppLanguageStandard>Gnu++20</CppLanguageStandard>')
				end
			end

			if cfg.platform == "Durango" then
				_p(2, '<ReferencePath>$(Console_SdkLibPath);$(Console_SdkWindowsMetadataPath)</ReferencePath>')
				_p(2, '<LibraryPath>$(Console_SdkLibPath)</LibraryPath>')
				_p(2, '<LibraryWPath>$(Console_SdkLibPath);$(Console_SdkWindowsMetadataPath)</LibraryWPath>')
				_p(2, '<IncludePath>$(Console_SdkIncludeRoot)</IncludePath>')
				_p(2, '<ExecutablePath>$(Console_SdkRoot)bin;$(VCInstallDir)bin\\x86_amd64;$(VCInstallDir)bin;$(WindowsSDK_ExecutablePath_x86);$(VSInstallDir)Common7\\Tools\\bin;$(VSInstallDir)Common7\\tools;$(VSInstallDir)Common7\\ide;$(ProgramFiles)\\HTML Help Workshop;$(MSBuildToolsPath32);$(FxCopDir);$(PATH);</ExecutablePath>')

				if cfg.imagepath then
					_p(2, '<LayoutDir>%s</LayoutDir>', cfg.imagepath)
				else
					_p(2, '<LayoutDir>%s</LayoutDir>', prj.name)
				end

				if cfg.pullmappingfile ~= nil then
					_p(2,'<PullMappingFile>%s</PullMappingFile>', premake.esc(cfg.pullmappingfile))
				end

				_p(2, '<LayoutExtensionFilter>*.pdb;*.ilk;*.exp;*.lib;*.winmd;*.appxrecipe;*.pri;*.idb</LayoutExtensionFilter>')
				_p(2, '<IsolateConfigurationsOnDeploy>true</IsolateConfigurationsOnDeploy>')
			end

			if vstudio.isgdkconsole(cfg) then
				_p(2, '<ExecutablePath>$(Console_SdkRoot)bin;$(Console_SdkToolPath);$(ExecutablePath)</ExecutablePath>')
				_p(2, '<IncludePath>$(Console_SdkIncludeRoot)</IncludePath>')
				_p(2, '<ReferencePath>$(Console_SdkLibPath);$(Console_SdkWindowsMetadataPath)</ReferencePath>')
				_p(2, '<LibraryPath>$(Console_SdkLibPath)</LibraryPath>')
				_p(2, '<LibraryWPath>$(Console_SdkLibPath);$(Console_SdkWindowsMetadataPath)</LibraryWPath>')
			end

			if vstudio.isgdkdesktop(cfg) then
				_p(2, '<IncludePath>$(Console_SdkIncludeRoot);$(IncludePath)</IncludePath>')
				_p(2, '<LibraryPath>$(Console_SdkLibPath);$(LibraryPath)</LibraryPath>')
			end

			if cfg.kind ~= "StaticLib" then
				_p(2,'<LinkIncremental>%s</LinkIncremental>', tostring(premake.config.isincrementallink(cfg)))
			end

			if cfg.applicationdatadir ~= nil then
				_p(2,'<ApplicationDataDir>%s</ApplicationDataDir>', premake.esc(cfg.applicationdatadir))
			end

			if cfg.flags.NoManifest then
				_p(2,'<GenerateManifest>false</GenerateManifest>')
			end

			_p(1,'</PropertyGroup>')
		end
	end

	local function runtime(cfg)
		local runtime
		local flags = cfg.flags
		if premake.config.isdebugbuild(cfg) then
			runtime = iif(flags.StaticRuntime and not flags.Managed, "MultiThreadedDebug", "MultiThreadedDebugDLL")
		else
			runtime = iif(flags.StaticRuntime and not flags.Managed, "MultiThreaded", "MultiThreadedDLL")
		end
		return runtime
	end

	local function precompiled_header(cfg)
      	if not cfg.flags.NoPCH and cfg.pchheader then
			_p(3,'<PrecompiledHeader>Use</PrecompiledHeader>')
			_p(3,'<PrecompiledHeaderFile>%s</PrecompiledHeaderFile>', cfg.pchheader)
		else
			_p(3,'<PrecompiledHeader></PrecompiledHeader>')
		end
	end

	local function preprocessor(indent,cfg,escape)
		if #cfg.defines > 0 then
			-- Visual Studio requires escaping of command line arguments to RC.
			local defines = table.concat(cfg.defines, ";")
			if escape then
				defines = defines:gsub('"', '\\"')
			end

			-- do not add `%(PreprocessorDefinitions)` if it's already part of defines
			local isPreprocessorDefinitionPresent = string.find(defines, "%%%(PreprocessorDefinitions%)")
			if isPreprocessorDefinitionPresent then
				_p(indent,'<PreprocessorDefinitions>%s</PreprocessorDefinitions>'
					,premake.esc(defines))
			else
				_p(indent,'<PreprocessorDefinitions>%s;%%(PreprocessorDefinitions)</PreprocessorDefinitions>'
					,premake.esc(defines))
			end
		else
			_p(indent,'<PreprocessorDefinitions></PreprocessorDefinitions>')
		end
	end

	local function include_dirs(indent,cfg)
		local includedirs = table.join(cfg.userincludedirs, cfg.includedirs, cfg.systemincludedirs)

		if #includedirs> 0 then
			_p(indent,'<AdditionalIncludeDirectories>%s;%%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>'
					,premake.esc(path.translate(table.concat(includedirs, ";"), '\\')))
		end
	end

	local function using_dirs(indent,cfg)
		if #cfg.usingdirs > 0 then
			_p(indent,'<AdditionalUsingDirectories>%s;%%(AdditionalUsingDirectories)</AdditionalUsingDirectories>'
					,premake.esc(path.translate(table.concat(cfg.usingdirs, ";"), '\\')))
		end
	end

	local function resource_compile(cfg)
		_p(2,'<ResourceCompile>')
			preprocessor(3,cfg,true)
			include_dirs(3,cfg)
		_p(2,'</ResourceCompile>')

	end

	local function cppstandard(cfg)
		if cfg.flags.CppLatest then
			_p(3, '<LanguageStandard>stdcpplatest</LanguageStandard>')
			_p(3, '<EnableModules>true</EnableModules>')
		elseif cfg.flags.Cpp20 then
			_p(3, '<LanguageStandard>stdcpp20</LanguageStandard>')
		elseif cfg.flags.Cpp17 then
			_p(3, '<LanguageStandard>stdcpp17</LanguageStandard>')
		elseif cfg.flags.Cpp14 then
			_p(3, '<LanguageStandard>stdcpp14</LanguageStandard>')
		end
	end

	local function exceptions(cfg)
		if cfg.platform == "Orbis" then
			if cfg.flags.NoExceptions then
				_p(3, '<CppExceptions>false</CppExceptions>')
			end
		elseif cfg.platform == "TegraAndroid" then
			if cfg.flags.NoExceptions then
				_p(3, '<GccExceptionHandling>false</GccExceptionHandling>')
			end
		elseif cfg.platform == "NX32" or cfg.platform == "NX64" then
			if cfg.flags.NoExceptions then
				_p(3, '<CppExceptions>false</CppExceptions>')
			else
				_p(3, '<CppExceptions>true</CppExceptions>')
			end
		else
			if cfg.flags.NoExceptions then
				_p(3, '<ExceptionHandling>false</ExceptionHandling>')
			elseif cfg.flags.SEH then
				_p(3, '<ExceptionHandling>Async</ExceptionHandling>')
			--SEH is not required for Managed and is implied
			end
		end
	end

	local function rtti(cfg)
		if cfg.flags.NoRTTI and not cfg.flags.Managed then
			_p(3,'<RuntimeTypeInfo>false</RuntimeTypeInfo>')
		end
	end

	local function calling_convention(cfg)
		if cfg.flags.FastCall then
			_p(3,'<CallingConvention>FastCall</CallingConvention>')
		elseif cfg.flags.StdCall then
			_p(3,'<CallingConvention>StdCall</CallingConvention>')
		end
	end

	local function wchar_t_builtin(cfg)
		if cfg.flags.NativeWChar then
			_p(3,'<TreatWChar_tAsBuiltInType>true</TreatWChar_tAsBuiltInType>')
		elseif cfg.flags.NoNativeWChar then
			_p(3,'<TreatWChar_tAsBuiltInType>false</TreatWChar_tAsBuiltInType>')
		end
	end

	local function sse(cfg)
		if cfg.flags.EnableSSE then
			_p(3, '<EnableEnhancedInstructionSet>StreamingSIMDExtensions</EnableEnhancedInstructionSet>')
		elseif cfg.flags.EnableSSE2 then
			_p(3, '<EnableEnhancedInstructionSet>StreamingSIMDExtensions2</EnableEnhancedInstructionSet>')
		elseif cfg.flags.EnableAVX then
			_p(3, '<EnableEnhancedInstructionSet>AdvancedVectorExtensions</EnableEnhancedInstructionSet>')
		elseif cfg.flags.EnableAVX2 then
			_p(3, '<EnableEnhancedInstructionSet>AdvancedVectorExtensions2</EnableEnhancedInstructionSet>')
		end
	end

	local function floating_point(cfg)
		if cfg.platform == "Orbis" then
			if cfg.flags.FloatFast then
				_p(3,'<FastMath>true</FastMath>')
			end
		elseif cfg.platform == "TegraAndroid" then
			-- TODO: tegra setting
		elseif cfg.platform == "NX32" or cfg.platform == "NX64" then
			if cfg.flags.FloatFast then
				_p(3, '<FastMath>true</FastMath>')
			end
		else
			if cfg.flags.FloatFast then
				_p(3,'<FloatingPointModel>Fast</FloatingPointModel>')
			elseif cfg.flags.FloatStrict and not cfg.flags.Managed then
				_p(3,'<FloatingPointModel>Strict</FloatingPointModel>')
			end
		end
	end


	local function debug_info(cfg)
	--
	--	EditAndContinue /ZI
	--	ProgramDatabase /Zi
	--	OldStyle C7 Compatable /Z7
	--
		local debug_info = ''
		if cfg.flags.Symbols then
			if cfg.flags.C7DebugInfo then
				debug_info = "OldStyle"
			elseif (action.vstudio.supports64bitEditContinue == false and cfg.platform == "x64")
				or not premake.config.iseditandcontinue(cfg)
			then
				debug_info = "ProgramDatabase"
			else
				debug_info = "EditAndContinue"
			end
		end

		_p(3,'<DebugInformationFormat>%s</DebugInformationFormat>',debug_info)
	end

	local function minimal_build(cfg)
		if premake.config.isdebugbuild(cfg) and cfg.flags.EnableMinimalRebuild then
			_p(3,'<MinimalRebuild>true</MinimalRebuild>')
		else
			_p(3,'<MinimalRebuild>false</MinimalRebuild>')
		end
	end

	local function compile_language(cfg)
		if cfg.options.ForceCPP then
			_p(3,'<CompileAs>CompileAsCpp</CompileAs>')
		else
			if cfg.language == "C" then
				_p(3,'<CompileAs>CompileAsC</CompileAs>')
			end
		end
	end

	local function forcedinclude_files(indent,cfg)
		if #cfg.forcedincludes > 0 then
			_p(indent,'<ForcedIncludeFiles>%s</ForcedIncludeFiles>'
					,premake.esc(path.translate(table.concat(cfg.forcedincludes, ";"), '\\')))
		end
	end

	local function vs10_clcompile(cfg)
		_p(2,'<ClCompile>')

		local unsignedChar = "/J "
		local buildoptions = cfg.buildoptions

		if cfg.platform == "Orbis" then
			unsignedChar = "-funsigned-char ";
			_p(3,'<GenerateDebugInformation>%s</GenerateDebugInformation>', tostring(cfg.flags.Symbols ~= nil))
		end

		if cfg.platform == "NX32" or cfg.platform == "NX64" then
			unsignedChar = "-funsigned-char ";
			_p(3,'<GenerateDebugInformation>%s</GenerateDebugInformation>', tostring(cfg.flags.Symbols ~= nil))
		end

		if cfg.language == "C" and not cfg.options.ForceCPP then
			buildoptions = table.join(buildoptions, cfg.buildoptions_c)
		else
			buildoptions = table.join(buildoptions, cfg.buildoptions_cpp)
		end

		_p(3,'<AdditionalOptions>%s %s%%(AdditionalOptions)</AdditionalOptions>'
			, table.concat(premake.esc(buildoptions), " ")
			, iif(cfg.flags.UnsignedChar and cfg.platform ~= "TegraAndroid", unsignedChar, " ")
			)

		if cfg.platform == "TegraAndroid" then
			_p(3,'<SignedChar>%s</SignedChar>', tostring(cfg.flags.UnsignedChar == nil))
			_p(3,'<GenerateDebugInformation>%s</GenerateDebugInformation>', tostring(cfg.flags.Symbols ~= nil))
			if cfg.androidcppstandard then
				_p(3,'<CppLanguageStandard>%s</CppLanguageStandard>', cfg.androidcppstandard)
			end
		end

		if cfg.platform == "Orbis" then
			local opt = optimisation(cfg)
			if opt == "Disabled" then
				_p(3,'<OptimizationLevel>Level0</OptimizationLevel>')
			elseif opt == "MinSpace" then
				_p(3,'<OptimizationLevel>Levelz</OptimizationLevel>') -- Oz is more aggressive than Os
			elseif opt == "MaxSpeed" then
				_p(3,'<OptimizationLevel>Level3</OptimizationLevel>')
			else
				_p(3,'<OptimizationLevel>Level2</OptimizationLevel>')
			end
		elseif cfg.platform == "TegraAndroid" then
			local opt = optimisation(cfg)
			if opt == "Disabled" then
				_p(3,'<OptimizationLevel>O0</OptimizationLevel>')
			elseif opt == "MinSpace" then
				_p(3,'<OptimizationLevel>Os</OptimizationLevel>')
			elseif opt == "MaxSpeed" then
				_p(3,'<OptimizationLevel>O3</OptimizationLevel>')
			else
				_p(3,'<OptimizationLevel>O2</OptimizationLevel>')
			end
		elseif cfg.platform == "NX32" or cfg.platform == "NX64" then
			local opt = optimisation(cfg)
			if opt == "Disabled" then
				_p(3,'<OptimizationLevel>O0</OptimizationLevel>')
			elseif opt == "MinSpace" then
				_p(3,'<OptimizationLevel>Os</OptimizationLevel>')
			elseif opt == "MaxSpeed" then
				_p(3,'<OptimizationLevel>O3</OptimizationLevel>')
			else
				_p(3,'<OptimizationLevel>O2</OptimizationLevel>')
			end
		else
			_p(3,'<Optimization>%s</Optimization>', optimisation(cfg))
		end

		include_dirs(3, cfg)
		using_dirs(3, cfg)
		preprocessor(3, cfg)
		minimal_build(cfg)

		if premake.config.isoptimizedbuild(cfg.flags) then
			-- Edit and continue is unstable with release/optimized projects. If the current project
			-- is optimized, but linker optimizations are disabled and has opted out of edit and continue
			-- support, then ensure that function level linking is disabled. This ensures that libs that
			-- do have edit and continue enabled don't run into undefined behavior at runtime when linking
			-- in optimized libs.
			if cfg.flags.NoOptimizeLink and cfg.flags.NoEditAndContinue then
				_p(3, '<StringPooling>false</StringPooling>')
				_p(3, '<FunctionLevelLinking>false</FunctionLevelLinking>')
			else
				_p(3, '<StringPooling>true</StringPooling>')
				_p(3, '<FunctionLevelLinking>true</FunctionLevelLinking>')
			end
		else
			_p(3, '<FunctionLevelLinking>true</FunctionLevelLinking>')
			if cfg.flags.NoRuntimeChecks then
				_p(3, '<BasicRuntimeChecks>Default</BasicRuntimeChecks>')
			elseif not cfg.flags.Managed then
				_p(3, '<BasicRuntimeChecks>EnableFastChecks</BasicRuntimeChecks>')
			end

			if cfg.flags.ExtraWarnings then
--				_p(3, '<SmallerTypeCheck>true</SmallerTypeCheck>')
			end
		end

		if cfg.platform == "Durango" or cfg.flags.NoWinRT then
			_p(3, '<CompileAsWinRT>false</CompileAsWinRT>')
		end

		_p(3, '<RuntimeLibrary>%s</RuntimeLibrary>', runtime(cfg))

		if cfg.flags.NoBufferSecurityCheck then
			_p(3, '<BufferSecurityCheck>false</BufferSecurityCheck>')
		end



		-- If we aren't running NoMultiprocessorCompilation and not wanting a minimal rebuild,
		-- then enable MultiProcessorCompilation.
		if not cfg.flags.NoMultiProcessorCompilation and not cfg.flags.EnableMinimalRebuild then
			_p(3, '<MultiProcessorCompilation>true</MultiProcessorCompilation>')
		else
			_p(3, '<MultiProcessorCompilation>false</MultiProcessorCompilation>')
		end

		precompiled_header(cfg)

		if cfg.platform == "Orbis" then
			if cfg.flags.PedanticWarnings then
				_p(3, '<Warnings>MoreWarnings</Warnings>')
				_p(3, '<ExtraWarnings>true</ExtraWarnings>')
			elseif cfg.flags.ExtraWarnings then
				_p(3, '<Warnings>NormalWarnings</Warnings>')
				_p(3, '<ExtraWarnings>true</ExtraWarnings>')
			elseif cfg.flags.MinimumWarnings then
				_p(3, '<Warnings>WarningsOff</Warnings>')
				_p(3, '<ExtraWarnings>false</ExtraWarnings>')
			else
				_p(3, '<Warnings>NormalWarnings</Warnings>')
				_p(3, '<ExtraWarnings>false</ExtraWarnings>')
			end
			if cfg.flags.FatalWarnings then
				_p(3, '<WarningsAsErrors>true</WarningsAsErrors>')
			end
		elseif cfg.platform == "TegraAndroid" then
			if cfg.flags.PedanticWarnings or cfg.flags.ExtraWarnings then
				_p(3, '<Warnings>AllWarnings</Warnings>')
			elseif cfg.flags.MinimumWarnings then
				_p(3, '<Warnings>DisableAllWarnings</Warnings>')
			else
				_p(3, '<Warnings>NormalWarnings</Warnings>')
			end
			if cfg.flags.FatalWarnings then
				_p(3, '<WarningsAsErrors>true</WarningsAsErrors>')
			end
		elseif cfg.platform == "NX32" or cfg.platform == "NX64" then
			if cfg.flags.PedanticWarnings then
				_p(3, '<Warnings>MoreWarnings</Warnings>')
				_p(3, '<ExtraWarnings>true</ExtraWarnings>')
			elseif cfg.flags.ExtraWarnings then
				_p(3, '<Warnings>NormalWarnings</Warnings>')
				_p(3, '<ExtraWarnings>true</ExtraWarnings>')
			elseif cfg.flags.MinimumWarnings then
				_p(3, '<Warnings>WarningsOff</Warnings>')
				_p(3, '<ExtraWarnings>false</ExtraWarnings>')
			else
				_p(3, '<Warnings>NormalWarnings</Warnings>')
				_p(3, '<ExtraWarnings>false</ExtraWarnings>')
			end
			if cfg.flags.FatalWarnings then
				_p(3, '<WarningsAsErrors>true</WarningsAsErrors>')
			end
		else
			if cfg.flags.PedanticWarnings then
				_p(3, '<WarningLevel>EnableAllWarnings</WarningLevel>')
			elseif cfg.flags.ExtraWarnings then
				_p(3, '<WarningLevel>Level4</WarningLevel>')
			elseif cfg.flags.MinimumWarnings then
				_p(3, '<WarningLevel>Level1</WarningLevel>')
			else
				_p(3 ,'<WarningLevel>Level3</WarningLevel>')
			end
		end

		if cfg.flags.FatalWarnings then
			_p(3, '<TreatWarningAsError>true</TreatWarningAsError>')
		end

		if premake.action.current() == premake.action.get("vs2017") or
		   premake.action.current() == premake.action.get("vs2019") or
		   premake.action.current() == premake.action.get("vs2022") then
			cppstandard(cfg)
		end

		exceptions(cfg)
		rtti(cfg)
		calling_convention(cfg)
		wchar_t_builtin(cfg)
		sse(cfg)
		floating_point(cfg)
		debug_info(cfg)

		if cfg.flags.Symbols then
			-- The compiler pdb should be different than the linker pdb, and
			-- the linker pdb is what should be distributed and used for
			-- debugging. But in the case of static libraries, they have no
			-- linker pdb, so then the compiler pdb should be in the output
			-- dir instead...
			if cfg.kind == "StaticLib" then
				_p(3, '<ProgramDataBaseFileName>$(OutDir)%s.pdb</ProgramDataBaseFileName>'
					, path.getbasename(cfg.buildtarget.name)
					)
			else
				_p(3, '<ProgramDataBaseFileName>$(IntDir)%s.compile.pdb</ProgramDataBaseFileName>'
					, path.getbasename(cfg.buildtarget.name)
					)
			end
		end

		if cfg.flags.Hotpatchable then
			_p(3, '<CreateHotpatchableImage>true</CreateHotpatchableImage>')
		end

		if cfg.flags.NoFramePointer then
			_p(3, '<OmitFramePointers>true</OmitFramePointers>')
		end

		if cfg.flags.UseFullPaths then
			_p(3, '<UseFullPaths>true</UseFullPaths>')
		end

		if cfg.flags.NoJMC then
			_p(3,'<SupportJustMyCode>false</SupportJustMyCode>' )
		end

		compile_language(cfg)

		forcedinclude_files(3,cfg);

		if vstudio.diagformat then
			_p(3, '<DiagnosticsFormat>%s</DiagnosticsFormat>', vstudio.diagformat)
		else
			_p(3, '<DiagnosticsFormat>Caret</DiagnosticsFormat>')
		end

		_p(2,'</ClCompile>')
	end


	local function event_hooks(cfg)
		if #cfg.postbuildcommands> 0 then
		    _p(2,'<PostBuildEvent>')
				_p(3,'<Command>%s</Command>',premake.esc(table.implode(cfg.postbuildcommands, "", "", "\r\n")))
			_p(2,'</PostBuildEvent>')
		end

		if #cfg.prebuildcommands> 0 then
		    _p(2,'<PreBuildEvent>')
				_p(3,'<Command>%s</Command>',premake.esc(table.implode(cfg.prebuildcommands, "", "", "\r\n")))
			_p(2,'</PreBuildEvent>')
		end

		if #cfg.prelinkcommands> 0 then
		    _p(2,'<PreLinkEvent>')
				_p(3,'<Command>%s</Command>',premake.esc(table.implode(cfg.prelinkcommands, "", "", "\r\n")))
			_p(2,'</PreLinkEvent>')
		end
	end

	local function additional_options(indent,cfg)
		if #cfg.linkoptions > 0 then
				_p(indent,'<AdditionalOptions>%s %%(AdditionalOptions)</AdditionalOptions>',
					table.concat(premake.esc(cfg.linkoptions), " "))
		end
	end

	local function link_target_machine(index,cfg)
		local platforms = {x32 = 'MachineX86', x64 = 'MachineX64'}
		if platforms[cfg.platform] then
			_p(index,'<TargetMachine>%s</TargetMachine>', platforms[cfg.platform])
		end
	end

	local function item_def_lib(cfg)
		-- The Xbox360 project files are stored in another place in the project file.
		if cfg.kind == 'StaticLib' and cfg.platform ~= "Xbox360" then
			_p(1,'<Lib>')
				_p(2,'<OutputFile>$(OutDir)%s</OutputFile>',cfg.buildtarget.name)
				additional_options(2,cfg)
				link_target_machine(2,cfg)
			_p(1,'</Lib>')
		end
	end

	local function import_lib(cfg)
		--Prevent the generation of an import library for a Windows DLL.
		if cfg.kind == "SharedLib" then
			local implibname = cfg.linktarget.fullpath
			_p(3,'<ImportLibrary>%s</ImportLibrary>',iif(cfg.flags.NoImportLib, cfg.objectsdir .. "\\" .. path.getname(implibname), implibname))
		end
	end

	local function hasmasmfiles(prj)
		local files = vc2010.getfilegroup(prj, "MASM")
		return #files > 0
	end

	local function ismanagedprj(prj, cfgname, pltname)
		local cfg = premake.getconfig(prj, cfgname, pltname)
		return cfg.flags.Managed == true
	end

	local function getcfglinks(cfg)
		local haswholearchive = #cfg.wholearchive > 0
		local msvcnaming 	  = premake.getnamestyle(cfg) == "windows"
		local iscppprj   	  = premake.iscppproject(cfg)
		local isnetprj   	  = premake.isdotnetproject(cfg)
		local linkobjs   	  = {}
		local links      	  = iif(haswholearchive
			, premake.getlinks(cfg, "all", "object")
			, premake.getlinks(cfg, "system", "fullpath")
			)

		for _, link in ipairs(links) do
			local name      = nil
			local directory = nil
			local whole     = nil

			if type(link) == "table" then
				-- if the link is to a managed project, we should ignore it
				-- as managed projects don't have lib files.
				if not ismanagedprj(link.project, cfg.name, cfg.platform) then
					-- project config
					name      = link.linktarget.basename
					directory = path.rebase(link.linktarget.directory, link.location, cfg.location)
					whole     = table.icontains(cfg.wholearchive, link.project.name)
				end
			else
				-- link name
				name      = link
				whole     = table.icontains(cfg.wholearchive, link)
			end

			if name then
				-- If we called premake.getlinks with "object", we need to
				-- re-add the file extensions since it didn't do it for us.
				if haswholearchive and msvcnaming then
					if iscppprj then
						name = name .. ".lib"
					elseif isnetprj then
						name = name .. ".dll"
					end
				end

				table.insert(linkobjs, {name=name, directory=directory, wholearchive=whole})
			end
		end

		return linkobjs
	end

	local function vs10_masm(prj, cfg)
		if hasmasmfiles(prj) then
			_p(2, '<MASM>')

			_p(3,'<AdditionalOptions>%s %%(AdditionalOptions)</AdditionalOptions>'
				, table.concat(premake.esc(table.join(cfg.buildoptions, cfg.buildoptions_asm)), " ")
				)

			local includedirs = table.join(cfg.userincludedirs, cfg.includedirs, cfg.systemincludedirs)

			if #includedirs > 0 then
				_p(3, '<IncludePaths>%s;%%(IncludePaths)</IncludePaths>'
					, premake.esc(path.translate(table.concat(includedirs, ";"), '\\'))
					)
			end

			-- table.join is used to create a copy rather than a reference
			local defines = table.join(cfg.defines)

			-- pre-defined preprocessor defines:
			-- _DEBUG:  For debug configurations
			-- _WIN32:  For 32-bit platforms
			-- _WIN64:  For 64-bit platforms
			-- _EXPORT: `EXPORT` for shared libraries, empty for other project kinds
			table.insertflat(defines, iif(premake.config.isdebugbuild(cfg), "_DEBUG", {}))
			table.insert(defines, iif(cfg.platform == "x64" or cfg.platform == "ARM64", "_WIN64", "_WIN32"))
			table.insert(defines, iif(prj.kind == "SharedLib", "_EXPORT=EXPORT", "_EXPORT="))

			_p(3, '<PreprocessorDefinitions>%s;%%(PreprocessorDefinitions)</PreprocessorDefinitions>'
				, premake.esc(table.concat(defines, ";"))
				)

			if cfg.flags.FatalWarnings then
				_p(3,'<TreatWarningsAsErrors>true</TreatWarningsAsErrors>')
			end

			-- MASM only has 3 warning levels where 3 is default, so we can ignore `ExtraWarnings`
			if cfg.flags.MinimumWarnings then
				_p(3,'<WarningLevel>0</WarningLevel>')
			else
				_p(3,'<WarningLevel>3</WarningLevel>')
			end

			_p(2, '</MASM>')
		end
	end

	local function additional_manifest(cfg)
		if(cfg.dpiawareness ~= nil) then
			_p(2,'<Manifest>')
				if(cfg.dpiawareness == "None") then
					_p(3, '<EnableDpiAwareness>false</EnableDpiAwareness>')
				end
				if(cfg.dpiawareness == "High") then
					_p(3, '<EnableDpiAwareness>true</EnableDpiAwareness>')
				end
				if(cfg.dpiawareness == "HighPerMonitor") then
					_p(3, '<EnableDpiAwareness>PerMonitorHighDPIAware</EnableDpiAwareness>')
				end
			_p(2,'</Manifest>')
		end
	end

--
-- Generate the <Link> element and its children.
--

	function vc2010.link(cfg)
		local vs2017OrLater = premake.action.current() == premake.action.get("vs2017") or
		    premake.action.current() == premake.action.get("vs2019")
		local links  = getcfglinks(cfg)

		_p(2,'<Link>')
		_p(3,'<SubSystem>%s</SubSystem>', iif(cfg.kind == "ConsoleApp", "Console", "Windows"))

		if vs2017OrLater and cfg.flags.FullSymbols then
			_p(3,'<GenerateDebugInformation>DebugFull</GenerateDebugInformation>')
		else
			_p(3,'<GenerateDebugInformation>%s</GenerateDebugInformation>', tostring(cfg.flags.Symbols ~= nil))
		end

		if cfg.flags.Symbols then
			_p(3, '<ProgramDatabaseFile>$(OutDir)%s.pdb</ProgramDatabaseFile>'
				, path.getbasename(cfg.buildtarget.name)
				)
		end

		if premake.config.islinkeroptimizedbuild(cfg.flags) then
			if cfg.platform == "Orbis" then
				_p(3,'<DataStripping>StripFuncsAndData</DataStripping>')
				_p(3,'<DuplicateStripping>true</DuplicateStripping>')
			else
				_p(3,'<EnableCOMDATFolding>true</EnableCOMDATFolding>')
				_p(3,'<OptimizeReferences>true</OptimizeReferences>')
			end
		elseif cfg.platform == "Orbis" and premake.config.iseditandcontinue(cfg) then
			_p(3,'<EditAndContinue>true</EditAndContinue>')
		end

		if cfg.finalizemetasource ~= nil then
			_p(3,'<FinalizeMetaSource>%s</FinalizeMetaSource>', premake.esc(cfg.finalizemetasource))
		end

		if cfg.kind ~= 'StaticLib' then
			vc2010.additionalDependencies(3, cfg, links)
			vc2010.additionalLibraryDirectories(3, cfg, links)
			_p(3,'<OutputFile>$(OutDir)%s</OutputFile>', cfg.buildtarget.name)

			if vc2010.config_type(cfg) == 'Application' and not cfg.flags.WinMain and not cfg.flags.Managed then
				if cfg.flags.Unicode then
					_p(3,'<EntryPointSymbol>wmainCRTStartup</EntryPointSymbol>')
				else
					_p(3,'<EntryPointSymbol>mainCRTStartup</EntryPointSymbol>')
				end
			end

			import_lib(cfg)

			local deffile = premake.findfile(cfg, ".def")
			if deffile then
				_p(3,'<ModuleDefinitionFile>%s</ModuleDefinitionFile>', deffile)
			end

			link_target_machine(3,cfg)
			additional_options(3,cfg)

			if cfg.flags.NoWinMD and vstudio.iswinrt() and prj.kind == "WindowedApp" then
				_p(3,'<GenerateWindowsMetadata>false</GenerateWindowsMetadata>' )
			end
		end

		if cfg.platform == "TegraAndroid" then
			if cfg.androidlinker then
				_p(3,'<UseLinker>%s</UseLinker>',cfg.androidlinker)
			end
		end

		if cfg.flags.Hotpatchable then
			_p(3, '<CreateHotPatchableImage>Enabled</CreateHotPatchableImage>')
		end

		if cfg.flags.GenerateMapFiles then
			_p(3, '<GenerateMapFile>true</GenerateMapFile>')
		end

		_p(2,'</Link>')

		-- If any libraries are to be linked as whole archive, we need to
		-- handle the linking manually, since there is no project configuration
		-- option for that.
		if #cfg.wholearchive > 0 then
			_p(2, '<ProjectReference>')
			_p(3, '<LinkLibraryDependencies>false</LinkLibraryDependencies>')
			_p(2, '</ProjectReference>')
		end
	end


	function vc2010.additionalLibraryDirectories(tab, cfg, links)
		local dirs = cfg.libdirs

		for _, link in ipairs(links) do
			if link.directory and not table.icontains(dirs, link.directory) then
				table.insert(dirs, link.directory)
			end
		end

		_p(tab, '<AdditionalLibraryDirectories>%s;%%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>'
			, premake.esc(path.translate(table.concat(dirs, ';'), '\\'))
			)
	end

--
-- Generate the <Link/AdditionalDependencies> element, which links in system
-- libraries required by the project (but not sibling projects; that's handled
-- by an <ItemGroup/ProjectReference>).
--

	function vc2010.additionalDependencies(tab, cfg, links)
		if #links > 0 then
			local deps = ""

			if cfg.platform == "Orbis" then
				local iswhole = false
				for _, link in ipairs(links) do
					if link.wholearchive and not iswhole then
						deps = deps .. "--whole-archive;"
						iswhole = true
					elseif not link.wholearchive and iswhole then
						deps = deps .. "--no-whole-archive;"
						iswhole = false
					end

					deps = deps .. "-l" .. link.name .. ";"
				end
			else
				for _, link in ipairs(links) do
					if link.wholearchive then
						deps = deps .. "/WHOLEARCHIVE:" .. link.name .. ";"
					else
						deps = deps .. link.name .. ";"
					end
				end
			end

			-- On Android, we need to shove a linking group in to resolve libs
			-- with circular deps.
			if cfg.platform == "TegraAndroid" then
				deps = "-Wl,--start-group;" .. deps .. ";-Wl,--end-group"
			end

			local adddeps =
				  iif(cfg.platform == "Durango",       '%(XboxExtensionsDependencies)'
				, iif(vstudio.isgdkconsole(cfg),       '$(Console_Libs);%(XboxExtensionsDependencies);%(AdditionalDependencies)'
				, iif(vstudio.isgdkdesktop(cfg),       '$(Console_Libs);%(AdditionalDependencies)'
				,                                      '%(AdditionalDependencies)')))
			_p(tab, '<AdditionalDependencies>%s;%s</AdditionalDependencies>', deps, adddeps)
		elseif cfg.platform == "Durango" then
			_p(tab, '<AdditionalDependencies>%%(XboxExtensionsDependencies)</AdditionalDependencies>')
		elseif vstudio.isgdkconsole(cfg) then
			_p(tab, '<AdditionalDependencies>$(Console_Libs);%%(XboxExtensionsDependencies);%%(AdditionalDependencies)</AdditionalDependencies>')
		elseif vstudio.isgdkdesktop(cfg) then
			_p(tab, '<AdditionalDependencies>$(Console_Libs);%%(AdditionalDependencies)</AdditionalDependencies>')
		end
	end

--
-- Generate the <AntBuild> element and its children.
--

	function ant_build(prj, cfg)
		-- only include this bit for Tegra
		if cfg.platform == "TegraAndroid" then
			local files = vc2010.getfilegroup(prj, "AndroidBuild")
			_p(2,'<AntBuild>')
			if #files > 0 then
				_p(3,'<AndroidManifestLocation>%s</AndroidManifestLocation>',path.translate(files[1].name))
			end
			local isdebugbuild = premake.config.isdebugbuild(cfg)
			_p(3,'<AntBuildType>%s</AntBuildType>',iif(isdebugbuild, 'Debug','Release'))
			_p(3,'<Debuggable>%s</Debuggable>',tostring(cfg.flags.AntBuildDebuggable ~= nil))
			if #cfg.antbuildjavasourcedirs > 0 then
				local dirs = table.concat(cfg.antbuildjavasourcedirs,";")
				_p(3,'<JavaSourceDir>%s</JavaSourceDir>',dirs)
			end
			if #cfg.antbuildjardirs > 0 then
				local dirs = table.concat(cfg.antbuildjardirs,";")
				_p(3,'<JarDirectories>%s</JarDirectories>',dirs)
			end
			if #cfg.antbuildjardependencies > 0 then
				local dirs = table.concat(cfg.antbuildjardependencies,";")
				_p(3,'<JarDependencies>%s</JarDependencies>',dirs)
			end
			if #cfg.antbuildnativelibdirs > 0 then
				local dirs = table.concat(cfg.antbuildnativelibdirs,";")
				_p(3,'<NativeLibDirectories>%s</NativeLibDirectories>',dirs)
			end
			if #cfg.antbuildnativelibdependencies > 0 then
				local dirs = table.concat(cfg.antbuildnativelibdependencies,";")
				_p(3,'<NativeLibDependencies>%s</NativeLibDependencies>',dirs)
			end
			if #cfg.antbuildassetsdirs > 0 then
				local dirs = table.concat(cfg.antbuildassetsdirs,";")
				_p(3,'<AssetsDirectories>%s</AssetsDirectories>',dirs)
			end
			_p(2,'</AntBuild>')
		end
	end


	local function item_definitions(prj)
		for _, cfginfo in ipairs(prj.solution.vstudio_configs) do
			local cfg = premake.getconfig(prj, cfginfo.src_buildcfg, cfginfo.src_platform)
			_p(1,'<ItemDefinitionGroup ' ..if_config_and_platform() ..'>'
					,premake.esc(cfginfo.name))
				vs10_clcompile(cfg)
				resource_compile(cfg)
				item_def_lib(cfg)
				vc2010.link(cfg)
				ant_build(prj, cfg)
				event_hooks(cfg)
				vs10_masm(prj, cfg)
				additional_manifest(cfg)
			_p(1,'</ItemDefinitionGroup>')
		end
	end

--
-- Retrieve a list of files for a particular build group, like
-- "ClInclude", "ClCompile", "ResourceCompile", "MASM", "None", etc.
--

	function vc2010.getfilegroup(prj, group)
		local sortedfiles = prj.vc2010sortedfiles
		if not sortedfiles then
			sortedfiles = {
				ClCompile = {},
				ClInclude = {},
				MASM = {},
				Object = {},
				None = {},
				ResourceCompile = {},
				AppxManifest = {},
				AndroidBuild = {},
				Natvis = {},
				Image = {},
				DeploymentContent = {}
			}

			local foundAppxManifest = false
			for file in premake.project.eachfile(prj, true) do
				if path.issourcefilevs(file.name) then
					table.insert(sortedfiles.ClCompile, file)
				elseif path.iscppheader(file.name) then
					if not table.icontains(prj.removefiles, file) then
						table.insert(sortedfiles.ClInclude, file)
					end
				elseif path.isobjectfile(file.name) then
					table.insert(sortedfiles.Object, file)
				elseif path.isresourcefile(file.name) then
					table.insert(sortedfiles.ResourceCompile, file)
				elseif path.isimagefile(file.name) then
					table.insert(sortedfiles.Image, file)
				elseif path.isappxmanifest(file.name) then
					foundAppxManifest = true
					table.insert(sortedfiles.AppxManifest, file)
				elseif path.isandroidbuildfile(file.name) then
					table.insert(sortedfiles.AndroidBuild, file)
				elseif path.isnatvis(file.name) then
					table.insert(sortedfiles.Natvis, file)
				elseif path.isasmfile(file.name) then
					table.insert(sortedfiles.MASM, file)
				elseif file.flags and table.icontains(file.flags, "DeploymentContent") then
					table.insert(sortedfiles.DeploymentContent, file)
				else
					table.insert(sortedfiles.None, file)
				end
			end

			-- WinRT projects get an auto-generated appxmanifest file if none is specified
			if vstudio.iswinrt() and prj.kind == "WindowedApp" and not foundAppxManifest then
				vstudio.needAppxManifest = true

				local fcfg = {}
				fcfg.name = prj.name .. "/Package.appxmanifest"
				fcfg.vpath = premake.project.getvpath(prj, fcfg.name)
				table.insert(sortedfiles.AppxManifest, fcfg)

				-- We also need a link to the splash screen because WinRT is retarded
				local logo = {}
				logo.name  = prj.name .. "/Logo.png"
				logo.vpath = logo.name
				table.insert(sortedfiles.Image, logo)

				local smallLogo = {}
				smallLogo.name  = prj.name .. "/SmallLogo.png"
				smallLogo.vpath = smallLogo.name
				table.insert(sortedfiles.Image, smallLogo)

				local storeLogo = {}
				storeLogo.name  = prj.name .. "/StoreLogo.png"
				storeLogo.vpath = storeLogo.name
				table.insert(sortedfiles.Image, storeLogo)

				local splashScreen = {}
				splashScreen.name  = prj.name .. "/SplashScreen.png"
				splashScreen.vpath = splashScreen.name
				table.insert(sortedfiles.Image, splashScreen)
			end

			-- Cache the sorted files; they are used several places
			prj.vc2010sortedfiles = sortedfiles
		end

		return sortedfiles[group]
	end


--
-- Write the files section of the project file.
--

	function vc2010.files(prj)
		vc2010.simplefilesgroup(prj, "ClInclude")
		vc2010.compilerfilesgroup(prj)
		vc2010.simplefilesgroup(prj, "Object")
		vc2010.simplefilesgroup(prj, "None")
		vc2010.customtaskgroup(prj)
		vc2010.simplefilesgroup(prj, "ResourceCompile")
		vc2010.simplefilesgroup(prj, "AppxManifest")
		vc2010.simplefilesgroup(prj, "AndroidBuild")
		vc2010.simplefilesgroup(prj, "Natvis")
		vc2010.deploymentcontentgroup(prj, "Image")
		vc2010.deploymentcontentgroup(prj, "DeploymentContent", "None")
	end

	function vc2010.customtaskgroup(prj)
		local files = { }
		for _, custombuildtask in ipairs(prj.custombuildtask or {}) do
			for _, buildtask in ipairs(custombuildtask or {}) do
				local fcfg = { }
				fcfg.name = path.getrelative(prj.location,buildtask[1])
				fcfg.vpath = path.trimdots(fcfg.name)
				table.insert(files, fcfg)
			end
		end
		if #files > 0  then
			_p(1,'<ItemGroup>')
			local groupedBuildTasks = {}
			local buildTaskNames = {}

			for _, custombuildtask in ipairs(prj.custombuildtask or {}) do
				for _, buildtask in ipairs(custombuildtask or {}) do
					if (groupedBuildTasks[buildtask[1]] == nil) then
						groupedBuildTasks[buildtask[1]] = {}
						table.insert(buildTaskNames, buildtask[1])
					end
					table.insert(groupedBuildTasks[buildtask[1]], buildtask)
				end
			end

			for _, name in ipairs(buildTaskNames) do
				custombuildtask = groupedBuildTasks[name]
				_p(2,'<CustomBuild Include=\"%s\">', path.translate(path.getrelative(prj.location,name), "\\"))
				_p(3,'<FileType>Text</FileType>')
				local cmd = ""
				local outputs = ""
				for _, buildtask in ipairs(custombuildtask or {}) do
					for _, cmdline in ipairs(buildtask[4] or {}) do
						cmd = cmd .. cmdline
						local num = 1
						for _, depdata in ipairs(buildtask[3] or {}) do
							cmd = string.gsub(cmd,"%$%(" .. num .."%)", string.format("%s ",path.getrelative(prj.location,depdata)))
							num = num + 1
						end
						cmd = string.gsub(cmd, "%$%(<%)", string.format("%s ",path.getrelative(prj.location,buildtask[1])))
						cmd = string.gsub(cmd, "%$%(@%)", string.format("%s ",path.getrelative(prj.location,buildtask[2])))
						cmd = cmd .. "\r\n"
					end
					outputs = outputs .. path.getrelative(prj.location,buildtask[2]) .. ";"
				end
				_p(3,'<Command>%s</Command>',cmd)
				_p(3,'<Outputs>%s%%(Outputs)</Outputs>',outputs)
				_p(3,'<SubType>Designer</SubType>')
				_p(3,'<Message></Message>')
				_p(2,'</CustomBuild>')
			end
			_p(1,'</ItemGroup>')
		end
	end

	function vc2010.simplefilesgroup(prj, section, subtype)
		local configs = prj.solution.vstudio_configs
		local files = vc2010.getfilegroup(prj, section)

		if #files > 0  then
			local config_mappings = {}

			for _, cfginfo in ipairs(configs) do
				local cfg = premake.getconfig(prj, cfginfo.src_buildcfg, cfginfo.src_platform)
				if cfg.pchheader and cfg.pchsource and not cfg.flags.NoPCH then
					config_mappings[cfginfo] = path.translate(cfg.pchsource, "\\")
				end
			end

			_p(1,'<ItemGroup>')

			for _, file in ipairs(files) do
				local prjexcluded = table.icontains(prj.excludes, file.name)
				local excludedcfgs = {}

				if not prjexcluded then
					for _, vsconfig in ipairs(configs) do
						local cfg = premake.getconfig(prj, vsconfig.src_buildcfg, vsconfig.src_platform)
						local fileincfg = table.icontains(cfg.files, file.name)
						local cfgexcluded = table.icontains(cfg.excludes, file.name)

						if not fileincfg or cfgexcluded then
							table.insert(excludedcfgs, vsconfig.name)
						end
					end
				end

				if subtype or prjexcluded or #excludedcfgs > 0 then
					_p(2, '<%s Include=\"%s\">', section, path.translate(file.name, "\\"))

					if prjexcluded then
						_p(3, '<ExcludedFromBuild>true</ExcludedFromBuild>')
					else
						for _, cfgname in ipairs(excludedcfgs) do
							_p(3, '<ExcludedFromBuild '
								.. if_config_and_platform()
								.. '>true</ExcludedFromBuild>'
								, premake.esc(cfgname)
								)
						end
					end

					if subtype then
						_p(3, '<SubType>%s</SubType>', subtype)
					end

					_p(2,'</%s>', section)
				else
					_p(2, '<%s Include=\"%s\" />', section, path.translate(file.name, "\\"))
				end
			end

			_p(1,'</ItemGroup>')
		end
	end

	function vc2010.deploymentcontentgroup(prj, section, filetype)
		if filetype == nil then
			filetype = section
		end

		local files = vc2010.getfilegroup(prj, section)
		if #files > 0  then
			_p(1,'<ItemGroup>')
			for _, file in ipairs(files) do
				_p(2,'<%s Include=\"%s\">', filetype, path.translate(file.name, "\\"))

				_p(3,'<DeploymentContent>true</DeploymentContent>')
				_p(3,'<Link>%s</Link>', path.translate(file.vpath, "\\"))
				_p(2,'</%s>', filetype)
			end
			_p(1,'</ItemGroup>')
		end
	end

	function vc2010.compilerfilesgroup(prj)
		local configs = prj.solution.vstudio_configs
		local files = vc2010.getfilegroup(prj, "ClCompile")
		if #files > 0  then
			local config_mappings = {}
			for _, cfginfo in ipairs(configs) do
				local cfg = premake.getconfig(prj, cfginfo.src_buildcfg, cfginfo.src_platform)
				if cfg.pchheader and cfg.pchsource and not cfg.flags.NoPCH then
					config_mappings[cfginfo] = path.translate(cfg.pchsource, "\\")
				end
			end

			_p(1,'<ItemGroup>')
			local existingBasenames = {};
			for _, file in ipairs(files) do
				-- Having unique ObjectFileName for each file subverts MSBuilds ability to parallelize compilation with the /MP flag.
				-- Instead we detect duplicates and partition them in subfolders only if needed.
				local filename = string.lower(path.getbasename(file.name))
				local disambiguation = existingBasenames[filename] or 0;
				existingBasenames[filename] = disambiguation + 1

				local translatedpath = path.translate(file.name, "\\")
				_p(2, '<ClCompile Include=\"%s\">', translatedpath)

				for _, vsconfig in ipairs(configs) do
					-- Android and NX need a full path to an object file, not a dir.
					local cfg = premake.getconfig(prj, vsconfig.src_buildcfg, vsconfig.src_platform)
					local namestyle = premake.getnamestyle(cfg)
					if namestyle == "TegraAndroid" or namestyle == "NX" then
						_p(3, '<ObjectFileName '.. if_config_and_platform() .. '>$(IntDir)%s.o</ObjectFileName>', premake.esc(vsconfig.name), premake.esc(path.translate(path.trimdots(path.removeext(file.name)))) )
					else
						if disambiguation > 0 then
							_p(3, '<ObjectFileName '.. if_config_and_platform() .. '>$(IntDir)%s\\</ObjectFileName>', premake.esc(vsconfig.name), tostring(disambiguation))
						end
					end
				end

				if path.iscxfile(file.name) then
					_p(3, '<CompileAsWinRT>true</CompileAsWinRT>')
					_p(3, '<RuntimeTypeInfo>true</RuntimeTypeInfo>')
					_p(3, '<PrecompiledHeader>NotUsing</PrecompiledHeader>')
				end

				--For Windows Store Builds, if the file is .c we have to exclude it from /ZW compilation
				if vstudio.iswinrt() and string.len(file.name) > 2 and string.sub(file.name, -2) == ".c" then
					_p(3,'<CompileAsWinRT>FALSE</CompileAsWinRT>')
				end

				for _, cfginfo in ipairs(configs) do
					if config_mappings[cfginfo] and translatedpath == config_mappings[cfginfo] then
						_p(3,'<PrecompiledHeader '.. if_config_and_platform() .. '>Create</PrecompiledHeader>', premake.esc(cfginfo.name))
						config_mappings[cfginfo] = nil  --only one source file per pch
					end
				end

				local nopch = table.icontains(prj.nopch, file.name)
				for _, vsconfig in ipairs(configs) do
					local cfg = premake.getconfig(prj, vsconfig.src_buildcfg, vsconfig.src_platform)
					if nopch or table.icontains(cfg.nopch, file.name) then
						_p(3,'<PrecompiledHeader '.. if_config_and_platform() .. '>NotUsing</PrecompiledHeader>', premake.esc(vsconfig.name))
					end
				end

				local excluded = table.icontains(prj.excludes, file.name)
				for _, vsconfig in ipairs(configs) do
					local cfg = premake.getconfig(prj, vsconfig.src_buildcfg, vsconfig.src_platform)
					local fileincfg = table.icontains(cfg.files, file.name)
					local cfgexcluded = table.icontains(cfg.excludes, file.name)

					if excluded or not fileincfg or cfgexcluded then
						_p(3, '<ExcludedFromBuild '
							.. if_config_and_platform()
							.. '>true</ExcludedFromBuild>'
							, premake.esc(vsconfig.name)
							)
					end
				end

				if prj.flags and prj.flags.Managed then
					local prjforcenative = table.icontains(prj.forcenative, file.name)
					for _,vsconfig in ipairs(configs) do
						local cfg = premake.getconfig(prj, vsconfig.src_buildcfg, vsconfig.src_platform)
						if prjforcenative or table.icontains(cfg.forcenative, file.name) then
							_p(3, '<CompileAsManaged ' .. if_config_and_platform() .. '>false</CompileAsManaged>', premake.esc(vsconfig.name))
						end
					end
				end

				_p(2,'</ClCompile>')
			end
			_p(1,'</ItemGroup>')
		end
	end

	function vc2010.masmfiles(prj)
		local configs = prj.solution.vstudio_configs
		local files = vc2010.getfilegroup(prj, "MASM")
		if #files > 0 then
			_p(1, '<ItemGroup>')
			for _, file in ipairs(files) do
				local translatedpath = path.translate(file.name, "\\")
				_p(2, '<MASM Include="%s">', translatedpath)

				local excluded = table.icontains(prj.excludes, file.name)
				for _, vsconfig in ipairs(configs) do
					local cfg = premake.getconfig(prj, vsconfig.src_buildcfg, vsconfig.src_platform)
					local fileincfg = table.icontains(cfg.files, file.name)
					local cfgexcluded = table.icontains(cfg.excludes, file.name)

					if excluded or not fileincfg or cfgexcluded then
						_p(3, '<ExcludedFromBuild '
							.. if_config_and_platform()
							.. '>true</ExcludedFromBuild>'
							, premake.esc(vsconfig.name)
							)
					end
				end

				_p(2, '</MASM>')
			end
			_p(1, '</ItemGroup>')
		end
	end


--
-- Output the VC2010 project file header
--

	function vc2010.header(targets)
		io.eol = "\r\n"
		_p('<?xml version="1.0" encoding="utf-8"?>')

		local t = ""
		if targets then
			t = ' DefaultTargets="' .. targets .. '"'
		end

		_p('<Project%s ToolsVersion="%s" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">', t, action.vstudio.toolsVersion)
	end


--
-- Output the VC2010 C/C++ project file
--

	function premake.vs2010_vcxproj(prj)
		local usemasm = hasmasmfiles(prj)

		io.indent = "  "
		vc2010.header("Build")

			vs2010_config(prj)
			vs2010_globals(prj)

			_p(1,'<Import Project="$(VCTargetsPath)\\Microsoft.Cpp.Default.props" />')

			for _, cfginfo in ipairs(prj.solution.vstudio_configs) do
				local cfg = premake.getconfig(prj, cfginfo.src_buildcfg, cfginfo.src_platform)
				vc2010.configurationPropertyGroup(cfg, cfginfo)
			end

			_p(1,'<Import Project="$(VCTargetsPath)\\Microsoft.Cpp.props" />')

			_p(1,'<ImportGroup Label="ExtensionSettings">')
			if usemasm then
				_p(2, '<Import Project="$(VCTargetsPath)\\BuildCustomizations\\masm.props" />')
			end
			_p(1,'</ImportGroup>')

			import_props(prj)

			--what type of macros are these?
			_p(1,'<PropertyGroup Label="UserMacros" />')

			vc2010.outputProperties(prj)

			item_definitions(prj)

			vc2010.files(prj)
			vc2010.clrReferences(prj)
			vc2010.projectReferences(prj)
			vc2010.sdkReferences(prj)
			vc2010.masmfiles(prj)

			_p(1,'<Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />')
			_p(1,'<ImportGroup Label="ExtensionTargets">')
			if usemasm then
				_p(2, '<Import Project="$(VCTargetsPath)\\BuildCustomizations\\masm.targets" />')
			end
			_p(1,'</ImportGroup>')

		_p('</Project>')
	end

--
-- Generate the list of CLR references
--
	function vc2010.clrReferences(prj)
		if #prj.clrreferences == 0 then
			return
		end

		_p(1,'<ItemGroup>')

		for _, ref in ipairs(prj.clrreferences) do
			if os.isfile(ref) then
				local assembly = path.getbasename(ref)
				_p(2,'<Reference Include="%s">', assembly)
				_p(3,'<HintPath>%s</HintPath>', path.getrelative(prj.location, ref))
				_p(2,'</Reference>')
			else
				_p(2,'<Reference Include="%s" />', ref)
			end
		end

		_p(1,'</ItemGroup>')
	end

--
-- Generate the list of project dependencies.
--

	function vc2010.projectReferences(prj)
		local deps = premake.getdependencies(prj)

		if #deps == 0 and #prj.vsimportreferences == 0 then
			return
		end

		-- Sort dependencies by uuid to keep the project files from changing
		-- unnecessarily.
		local function compareuuid(a, b) return a.uuid < b.uuid end
		table.sort(deps, compareuuid)
		table.sort(table.join(prj.vsimportreferences), compareuuid)

		_p(1,'<ItemGroup>')

		for _, dep in ipairs(deps) do
			local deppath = path.getrelative(prj.location, vstudio.projectfile(dep))
			_p(2,'<ProjectReference Include=\"%s\">', path.translate(deppath, "\\"))
			_p(3,'<Project>{%s}</Project>', dep.uuid)
			if vstudio.iswinrt() then
				_p(3,'<ReferenceOutputAssembly>false</ReferenceOutputAssembly>')
			end
			_p(2,'</ProjectReference>')
		end

		for _, ref in ipairs(prj.vsimportreferences) do
			-- Convert the path from being relative to the project to being
			-- relative to the solution, for lookup.
			local slnrelpath = path.rebase(ref, prj.location, sln.location)
			local iprj = premake.vstudio.getimportprj(slnrelpath, prj.solution)
			_p(2,'<ProjectReference Include=\"%s\">', ref)
			_p(3,'<Project>{%s}</Project>', iprj.uuid)
			_p(2,'</ProjectReference>')
		end

		_p(1,'</ItemGroup>')
	end

--
-- Generate the list of SDK references
--

	function vc2010.sdkReferences(prj)
		local refs = prj.sdkreferences
		if #refs > 0 then
			_p(1,'<ItemGroup>')
			for _, ref in ipairs(refs) do
				_p(2,'<SDKReference Include=\"%s\" />', ref)
			end
			_p(1,'</ItemGroup>')
		end
	end

--
-- Generate the .vcxproj.user file
--

	function vc2010.debugdir(cfg)
		local isnx = (cfg.platform == "NX32" or cfg.platform == "NX64")
		local debuggerFlavor =
			  iif(isnx,                           'OasisNXDebugger'
			, iif(cfg.platform == "Orbis",        'ORBISDebugger'
			, iif(cfg.platform == "Durango",      'XboxOneVCppDebugger'
			, iif(cfg.platform == "TegraAndroid", 'AndroidDebugger'
			, iif(vstudio.iswinrt(),              'AppHostLocalDebugger'
			,                                     'WindowsLocalDebugger'
			)))))
		_p(2, '<DebuggerFlavor>%s</DebuggerFlavor>', debuggerFlavor)

		if cfg.debugdir and not vstudio.iswinrt() then
			_p(2, '<LocalDebuggerWorkingDirectory>%s</LocalDebuggerWorkingDirectory>'
				, path.translate(cfg.debugdir, '\\')
				)
		end

		if cfg.debugcmd then
			_p(2, '<LocalDebuggerCommand>%s</LocalDebuggerCommand>', cfg.debugcmd)
		end

		if cfg.debugargs then
			_p(2, '<LocalDebuggerCommandArguments>%s</LocalDebuggerCommandArguments>'
				, table.concat(cfg.debugargs, " ")
				)
		end

		if cfg.debugenvs and #cfg.debugenvs > 0 then
			_p(2, '<LocalDebuggerEnvironment>%s%s</LocalDebuggerEnvironment>'
				, table.concat(cfg.debugenvs, "\n")
				, iif(cfg.flags.DebugEnvsInherit,'\n$(LocalDebuggerEnvironment)', '')
				)
			if cfg.flags.DebugEnvsDontMerge then
				_p(2, '<LocalDebuggerMergeEnvironment>false</LocalDebuggerMergeEnvironment>')
			end
		end

		if cfg.deploymode then
			_p(2, '<DeployMode>%s</DeployMode>', cfg.deploymode)
		end

		if cfg.platform == "TegraAndroid" then
			if cfg.androiddebugintentparams then
				_p(2, '<IntentParams>%s</IntentParams>'
					, table.concat(cfg.androiddebugintentparams, " ")
					)
			end
		end
	end

	function premake.vs2010_vcxproj_user(prj)
		io.indent = "  "
		vc2010.header()
		for _, cfginfo in ipairs(prj.solution.vstudio_configs) do
			local cfg = premake.getconfig(prj, cfginfo.src_buildcfg, cfginfo.src_platform)
			_p('  <PropertyGroup '.. if_config_and_platform() ..'>', premake.esc(cfginfo.name))
			vc2010.debugdir(cfg)
			_p('  </PropertyGroup>')
		end
		_p('</Project>')
	end

	local png1x1data = {
		0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, -- .PNG........IHDR
		0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x25, 0xdb, 0x56, -- .............%.V
		0xca, 0x00, 0x00, 0x00, 0x03, 0x50, 0x4c, 0x54, 0x45, 0x00, 0x00, 0x00, 0xa7, 0x7a, 0x3d, 0xda, -- .....PLTE....z=.
		0x00, 0x00, 0x00, 0x01, 0x74, 0x52, 0x4e, 0x53, 0x00, 0x40, 0xe6, 0xd8, 0x66, 0x00, 0x00, 0x00, -- ....tRNS.@..f...
		0x0a, 0x49, 0x44, 0x41, 0x54, 0x08, 0xd7, 0x63, 0x60, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0xe2, -- .IDAT..c`.......
		0x21, 0xbc, 0x33, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,       -- !.3....IEND.B`.
	}

	function png1x1(obj, filename)
		filename = premake.project.getfilename(obj, filename)

		local f, err = io.open(filename, "wb")
		if f then
			for _, byte in ipairs(png1x1data) do
				f:write(string.char(byte))
			end
			f:close()
		end
	end

	function premake.vs2010_appxmanifest(prj)
		io.indent = "  "
		io.eol = "\r\n"
		_p('<?xml version="1.0" encoding="utf-8"?>')
		if vstudio.storeapp == "10.0" then
			_p('<Package')
			_p(1, 'xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10"')
			_p(1, 'xmlns:mp="http://schemas.microsoft.com/appx/2014/phone/manifest"')
			_p(1, 'xmlns:uap="http://schemas.microsoft.com/appx/manifest/uap/windows10"')
			_p(1, 'IgnorableNamespaces="uap mp">')
		elseif vstudio.storeapp == "durango" then
			_p('<Package xmlns="http://schemas.microsoft.com/appx/2010/manifest" xmlns:mx="http://schemas.microsoft.com/appx/2013/xbox/manifest" IgnorableNamespaces="mx">')
		end

		_p(1, '<Identity')
		_p(2, 'Name="' .. prj.uuid .. '"')
		_p(2, 'Publisher="CN=Publisher"')
		_p(2, 'Version="1.0.0.0" />')

		if vstudio.storeapp == "10.0" then
			_p(1, '<mp:PhoneIdentity')
			_p(2, 'PhoneProductId="' .. prj.uuid .. '"')
			_p(2, 'PhonePublisherId="00000000-0000-0000-0000-000000000000"/>')
		end

		_p(1, '<Properties>')
		_p(2, '<DisplayName>' .. prj.name .. '</DisplayName>')
		_p(2, '<PublisherDisplayName>PublisherDisplayName</PublisherDisplayName>')
		_p(2, '<Logo>' .. prj.name .. '\\StoreLogo.png</Logo>')
		png1x1(prj, "%%/StoreLogo.png")
		_p(2, '<Description>' .. prj.name .. '</Description>')

		_p(1,'</Properties>')

		if vstudio.storeapp == "10.0" then
			_p(1, '<Dependencies>')
			_p(2, '<TargetDeviceFamily Name="Windows.Universal" MinVersion="10.0.10069.0" MaxVersionTested="10.0.10069.0" />')
			_p(1, '</Dependencies>')
		elseif vstudio.storeapp == "durango" then
			_p(1, '<Prerequisites>')
			_p(2, '<OSMinVersion>6.2</OSMinVersion>')
			_p(2, '<OSMaxVersionTested>6.2</OSMaxVersionTested>')
			_p(1, '</Prerequisites>')
		end

		_p(1, '<Resources>')
		_p(2, '<Resource Language="en-us"/>')
		_p(1, '</Resources>')

		_p(1, '<Applications>')
		_p(2, '<Application Id="App"')
		_p(3, 'Executable="$targetnametoken$.exe"')
		_p(3, 'EntryPoint="' .. prj.name .. '.App">')
		if vstudio.storeapp == "10.0" then
			_p(3, '<uap:VisualElements')
			_p(4, 'DisplayName="' .. prj.name .. '"')
			_p(4, 'Square150x150Logo="' .. prj.name .. '\\Logo.png"')
			png1x1(prj, "%%/Logo.png")
			if vstudio.storeapp == "10.0" then
				_p(4, 'Square44x44Logo="' .. prj.name .. '\\SmallLogo.png"')
				png1x1(prj, "%%/SmallLogo.png")
			else
				_p(4, 'Square30x30Logo="' .. prj.name .. '\\SmallLogo.png"')
				png1x1(prj, "%%/SmallLogo.png")
			end
			_p(4, 'Description="' .. prj.name .. '"')
			_p(4, 'BackgroundColor="transparent">')
			_p(4, '<uap:SplashScreen Image="' .. prj.name .. '\\SplashScreen.png" />')
			png1x1(prj, "%%/SplashScreen.png")
			_p(3, '</uap:VisualElements>')
		elseif vstudio.storeapp == "durango" then
			_p(3, '<VisualElements')
			_p(4, 'DisplayName="' .. prj.name .. '"')
			_p(4, 'Logo="' .. prj.name .. '\\Logo.png"')
			png1x1(prj, "%%/Logo.png")
			_p(4, 'SmallLogo="' .. prj.name .. '\\SmallLogo.png"')
			png1x1(prj, "%%/SmallLogo.png")
			_p(4, 'Description="' .. prj.name .. '"')
			_p(4, 'ForegroundText="light"')
			_p(4, 'BackgroundColor="transparent">')
			_p(5, '<SplashScreen Image="' .. prj.name .. '\\SplashScreen.png" />')
			png1x1(prj, "%%/SplashScreen.png")
			_p(3, '</VisualElements>')
			_p(3, '<Extensions>')
			_p(4, '<mx:Extension Category="xbox.system.resources">')
			_p(4, '<mx:XboxSystemResources />')
			_p(4, '</mx:Extension>')
			_p(3, '</Extensions>')
		end
		_p(2, '</Application>')
		_p(1, '</Applications>')

		_p('</Package>')
	end
