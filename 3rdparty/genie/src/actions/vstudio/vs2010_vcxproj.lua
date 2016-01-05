--
-- vs2010_vcxproj.lua
-- Generate a Visual Studio 2010 C/C++ project.
-- Copyright (c) 2009-2011 Jason Perkins and the Premake project
--

	premake.vstudio.vc2010 = { }
	local vc2010 = premake.vstudio.vc2010
	local vstudio = premake.vstudio


	local function vs2010_config(prj)
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
			_p(2,'<ProjectGuid>{%s}</ProjectGuid>',prj.uuid)
			_p(2,'<RootNamespace>%s</RootNamespace>',prj.name)
			local windowsTargetPlatformVersion = prj.windowstargetplatformversion or action.vstudio.windowsTargetPlatformVersion
			if windowsTargetPlatformVersion ~= nil then
				_p(2,'<WindowsTargetPlatformVersion>%s</WindowsTargetPlatformVersion>',windowsTargetPlatformVersion)
			end
		--if prj.flags is required as it is not set at project level for tests???
		--vs200x generator seems to swap a config for the prj in test setup
		if prj.flags and prj.flags.Managed then
			_p(2,'<TargetFrameworkVersion>v4.0</TargetFrameworkVersion>')
			_p(2,'<Keyword>ManagedCProj</Keyword>')
		elseif vstudio.iswinrt() then
			_p(2,'<DefaultLanguage>en-US</DefaultLanguage>')
			_p(2,'<MinimumVisualStudioVersion>12.0</MinimumVisualStudioVersion>')
			_p(2,'<AppContainerApplication>true</AppContainerApplication>')
			if vstudio.toolset == "v120_wp81" then
				_p(2,'<ApplicationType>Windows Phone</ApplicationType>')
			else
				_p(2,'<ApplicationType>Windows Store</ApplicationType>')
			end
			_p(2,'<ApplicationTypeRevision>%s</ApplicationTypeRevision>', vstudio.storeapp)
		else
			_p(2,'<Keyword>Win32Proj</Keyword>')
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
		_p(1,'<PropertyGroup '..if_config_and_platform() ..' Label="Configuration">'
				, premake.esc(cfginfo.name))
		_p(2,'<ConfigurationType>%s</ConfigurationType>',vc2010.config_type(cfg))
		_p(2,'<UseDebugLibraries>%s</UseDebugLibraries>', iif(optimisation(cfg) == "Disabled","true","false"))

		_p(2,'<PlatformToolset>%s</PlatformToolset>', premake.vstudio.toolset)

		if cfg.flags.MFC then
			_p(2,'<UseOfMfc>%s</UseOfMfc>', iif(cfg.flags.StaticRuntime, "Static", "Dynamic"))
		end

		if cfg.flags.ATL or cfg.flags.StaticATL then
			_p(2,'<UseOfAtl>%s</UseOfAtl>', iif(cfg.flags.StaticATL, "Static", "Dynamic"))
		end

		if cfg.flags.Managed then
			_p(2,'<CLRSupport>true</CLRSupport>')
		end
		_p(1,'</PropertyGroup>')
	end


	local function import_props(prj)
		for _, cfginfo in ipairs(prj.solution.vstudio_configs) do
			local cfg = premake.getconfig(prj, cfginfo.src_buildcfg, cfginfo.src_platform)
			_p(1,'<ImportGroup '..if_config_and_platform() ..' Label="PropertySheets">'
					,premake.esc(cfginfo.name))
				_p(2,'<Import Project="$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props" Condition="exists(\'$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\')" Label="LocalAppDataPlatform" />')
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
				local intdir = add_trailing_backslash(cfg.objectsdir)

				_p(1,'<PropertyGroup '..if_config_and_platform() ..'>', premake.esc(cfginfo.name))

				_p(2,'<OutDir>%s</OutDir>', premake.esc(outdir))

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

				if cfg.kind ~= "StaticLib" then
					_p(2,'<LinkIncremental>%s</LinkIncremental>', tostring(premake.config.isincrementallink(cfg)))
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

	local function preprocessor(indent,cfg)
		if #cfg.defines > 0 then
			_p(indent,'<PreprocessorDefinitions>%s;%%(PreprocessorDefinitions)</PreprocessorDefinitions>'
				,premake.esc(table.concat(cfg.defines, ";")))
		else
			_p(indent,'<PreprocessorDefinitions></PreprocessorDefinitions>')
		end
	end

	local function include_dirs(indent,cfg)
		if #cfg.includedirs > 0 then
			_p(indent,'<AdditionalIncludeDirectories>%s;%%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>'
					,premake.esc(path.translate(table.concat(cfg.includedirs, ";"), '\\')))
		end
	end

	local function resource_compile(cfg)
		_p(2,'<ResourceCompile>')
			preprocessor(3,cfg)
			include_dirs(3,cfg)
		_p(2,'</ResourceCompile>')

	end

	local function exceptions(cfg)
		if cfg.flags.NoExceptions then
			_p(2,'<ExceptionHandling>false</ExceptionHandling>')
		elseif cfg.flags.SEH then
			_p(2,'<ExceptionHandling>Async</ExceptionHandling>')
		--SEH is not required for Managed and is implied
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

	local function wchar_t_buildin(cfg)
		if cfg.flags.NativeWChar then
			_p(3,'<TreatWChar_tAsBuiltInType>true</TreatWChar_tAsBuiltInType>')
		elseif cfg.flags.NoNativeWChar then
			_p(3,'<TreatWChar_tAsBuiltInType>false</TreatWChar_tAsBuiltInType>')
		end
	end

	local function sse(cfg)
		if cfg.flags.EnableSSE then
			_p(3,'<EnableEnhancedInstructionSet>StreamingSIMDExtensions</EnableEnhancedInstructionSet>')
		elseif cfg.flags.EnableSSE2 then
			_p(3,'<EnableEnhancedInstructionSet>StreamingSIMDExtensions2</EnableEnhancedInstructionSet>')
		end
	end

	local function floating_point(cfg)
	     if cfg.flags.FloatFast then
			_p(3,'<FloatingPointModel>Fast</FloatingPointModel>')
		elseif cfg.flags.FloatStrict and not cfg.flags.Managed then
			_p(3,'<FloatingPointModel>Strict</FloatingPointModel>')
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
			if (action.vstudio.supports64bitEditContinue == false and cfg.platform == "x64")
				or cfg.flags.Managed
				or premake.config.isoptimizedbuild(cfg.flags)
				or cfg.flags.NoEditAndContinue
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

		_p(3,'<AdditionalOptions>%s %s%%(AdditionalOptions)</AdditionalOptions>'
			, table.concat(premake.esc(cfg.buildoptions), " ")
			, iif(cfg.flags.UnsignedChar, "/J ", " ")
			)

		_p(3,'<Optimization>%s</Optimization>',optimisation(cfg))

		include_dirs(3,cfg)
		preprocessor(3,cfg)
		minimal_build(cfg)

		if  not premake.config.isoptimizedbuild(cfg.flags) then
			if not cfg.flags.Managed then
				_p(3,'<BasicRuntimeChecks>EnableFastChecks</BasicRuntimeChecks>')
			end

			if cfg.flags.ExtraWarnings then
--				_p(3,'<SmallerTypeCheck>true</SmallerTypeCheck>')
			end
		else
			_p(3,'<StringPooling>true</StringPooling>')
		end

		_p(3,'<RuntimeLibrary>%s</RuntimeLibrary>', runtime(cfg))
		_p(3,'<FunctionLevelLinking>true</FunctionLevelLinking>')

		-- If we aren't running NoMultiprocessorCompilation and not wanting a minimal rebuild,
		-- then enable MultiProcessorCompilation.
		if not cfg.flags.NoMultiProcessorCompilation and not cfg.flags.EnableMinimalRebuild then
			_p(3,'<MultiProcessorCompilation>true</MultiProcessorCompilation>')
		else
			_p(3,'<MultiProcessorCompilation>false</MultiProcessorCompilation>')
		end

		precompiled_header(cfg)

		if cfg.flags.ExtraWarnings then
			_p(3,'<WarningLevel>Level4</WarningLevel>')
		else
			_p(3,'<WarningLevel>Level3</WarningLevel>')
		end

		if cfg.flags.FatalWarnings then
			_p(3,'<TreatWarningAsError>true</TreatWarningAsError>')
		end

		exceptions(cfg)
		rtti(cfg)
		calling_convention(cfg)
		wchar_t_buildin(cfg)
		sse(cfg)
		floating_point(cfg)
		debug_info(cfg)

		if cfg.flags.Symbols then
			_p(3,'<ProgramDataBaseFileName>$(OutDir)%s.pdb</ProgramDataBaseFileName>'
				, path.getbasename(cfg.buildtarget.name))
		end

		if cfg.flags.NoFramePointer then
			_p(3,'<OmitFramePointers>true</OmitFramePointers>')
		end

		compile_language(cfg)

		forcedinclude_files(3,cfg);
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


--
-- Generate the <Link> element and its children.
--

	function vc2010.link(cfg)
		_p(2,'<Link>')
		_p(3,'<SubSystem>%s</SubSystem>', iif(cfg.kind == "ConsoleApp", "Console", "Windows"))
		_p(3,'<GenerateDebugInformation>%s</GenerateDebugInformation>', tostring(cfg.flags.Symbols ~= nil))

		if premake.config.isoptimizedbuild(cfg.flags) then
			_p(3,'<EnableCOMDATFolding>true</EnableCOMDATFolding>')
			_p(3,'<OptimizeReferences>true</OptimizeReferences>')
		end

		if cfg.kind ~= 'StaticLib' then
			vc2010.additionalDependencies(cfg)
			_p(3,'<OutputFile>$(OutDir)%s</OutputFile>', cfg.buildtarget.name)

			if #cfg.libdirs > 0 then
				_p(3,'<AdditionalLibraryDirectories>%s;%%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>',
						premake.esc(path.translate(table.concat(cfg.libdirs, ';'), '\\')))
			end

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
		end

		_p(2,'</Link>')
	end


--
-- Generate the <Link/AdditionalDependencies> element, which links in system
-- libraries required by the project (but not sibling projects; that's handled
-- by an <ItemGroup/ProjectReference>).
--

	function vc2010.additionalDependencies(cfg)
		local links = premake.getlinks(cfg, "system", "fullpath")
		if #links > 0 then
			_p(3,'<AdditionalDependencies>%s;%%(AdditionalDependencies)</AdditionalDependencies>',
						table.concat(links, ";"))
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
				event_hooks(cfg)
			_p(1,'</ItemDefinitionGroup>')
		end
	end


--
-- Retrieve a list of files for a particular build group, one of
-- "ClInclude", "ClCompile", "ResourceCompile", and "None".
--

	function vc2010.getfilegroup(prj, group)
		local sortedfiles = prj.vc2010sortedfiles
		if not sortedfiles then
			sortedfiles = {
				ClCompile = {},
				ClInclude = {},
				None = {},
				ResourceCompile = {},
				AppxManifest = {},
				Image = {}
			}

			local foundAppxManifest = false
			for file in premake.project.eachfile(prj) do
				if path.iscppfile(file.name) then
					table.insert(sortedfiles.ClCompile, file)
				elseif path.iscppheader(file.name) then
					if not table.icontains(prj.removefiles, file) then
						table.insert(sortedfiles.ClInclude, file)
					end
				elseif path.isresourcefile(file.name) then
					table.insert(sortedfiles.ResourceCompile, file)
				else
					local ext = path.getextension(file.name):lower()
					if ext == ".appxmanifest" then
						foundAppxManifest = true
						table.insert(sortedfiles.AppxManifest, file)
					else
						table.insert(sortedfiles.None, file)
					end
				end
			end

			-- WinRT projects get an auto-generated appxmanifest file if none is specified
			if vstudio.iswinrt() and prj.kind == "WindowedApp" and not foundAppxManifest then
				vstudio.needAppxManifest = true

				local fcfg = {}
				fcfg.name = prj.name .. ".appxmanifest"
				fcfg.vpath = premake.project.getvpath(prj, fcfg.name)
				table.insert(sortedfiles.AppxManifest, fcfg)

				-- We also need a link to the splash screen because WinRT is retarded
				local splashcfg = {}
				splashcfg.name = premake.vstudio.splashpath
				splashcfg.vpath = premake.vstudio.splashpath
				table.insert(sortedfiles.Image, splashcfg)
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
		vc2010.simplefilesgroup(prj, "None")
		vc2010.customtaskgroup(prj)
		vc2010.simplefilesgroup(prj, "ResourceCompile")
		vc2010.simplefilesgroup(prj, "AppxManifest")
		vc2010.deploymentcontentgroup(prj, "Image")
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
			for _, custombuildtask in ipairs(prj.custombuildtask or {}) do
				for _, buildtask in ipairs(custombuildtask or {}) do
					if (groupedBuildTasks[buildtask[1]] == nil) then
						groupedBuildTasks[buildtask[1]] = {}
					end
					table.insert(groupedBuildTasks[buildtask[1]], buildtask)
				end
			end

			for name, custombuildtask in pairs(groupedBuildTasks or {}) do
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
		local files = vc2010.getfilegroup(prj, section)
		if #files > 0  then
			_p(1,'<ItemGroup>')
			for _, file in ipairs(files) do
				if subtype then
					_p(2,'<%s Include=\"%s\">', section, path.translate(file.name, "\\"))
					_p(3,'<SubType>%s</SubType>', subtype)
					_p(2,'</%s>', section)
				else
					_p(2,'<%s Include=\"%s\" />', section, path.translate(file.name, "\\"))
				end
			end
			_p(1,'</ItemGroup>')
		end
	end

	function vc2010.deploymentcontentgroup(prj, section)
		local files = vc2010.getfilegroup(prj, section)
		if #files > 0  then
			_p(1,'<ItemGroup>')
			for _, file in ipairs(files) do
				_p(2,'<%s Include=\"%s\">', section, path.translate(file.name, "\\"))
				_p(3,'<DeploymentContent>true</DeploymentContent>')
				_p(2,'</%s>', section)
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
			for _, file in ipairs(files) do
				local translatedpath = path.translate(file.name, "\\")
				_p(2, '<ClCompile Include=\"%s\">', translatedpath)
				_p(3, '<ObjectFileName>$(IntDir)%s.obj</ObjectFileName>'
					, premake.esc(path.translate(path.trimdots(path.removeext(file.name))))
					)
				for _, cfginfo in ipairs(configs) do
					if config_mappings[cfginfo] and translatedpath == config_mappings[cfginfo] then
						_p(3,'<PrecompiledHeader '.. if_config_and_platform() .. '>Create</PrecompiledHeader>', premake.esc(cfginfo.name))
						config_mappings[cfginfo] = nil  --only one source file per pch
					end
				end

				local excluded = table.icontains(prj.excludes, file.name)
				for _, vsconfig in ipairs(configs) do
					local cfg = premake.getconfig(prj, vsconfig.src_buildcfg, vsconfig.src_platform)
					if excluded or table.icontains(cfg.excludes, file.name) then
						_p(3, '<ExcludedFromBuild '
							.. if_config_and_platform()
							.. '>true</ExcludedFromBuild>'
							, premake.esc(vsconfig.name)
							)
					end
				end

				_p(2,'</ClCompile>')
			end
			_p(1,'</ItemGroup>')
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

		_p('<Project%s ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">', t)
	end


--
-- Output the VC2010 C/C++ project file
--

	function premake.vs2010_vcxproj(prj)
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

			--check what this section is doing
			_p(1,'<ImportGroup Label="ExtensionSettings">')
			_p(1,'</ImportGroup>')


			import_props(prj)

			--what type of macros are these?
			_p(1,'<PropertyGroup Label="UserMacros" />')

			vc2010.outputProperties(prj)

			item_definitions(prj)

			vc2010.files(prj)
			vc2010.projectReferences(prj)

			_p(1,'<Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />')
			_p(1,'<ImportGroup Label="ExtensionTargets">')
			_p(1,'</ImportGroup>')

		_p('</Project>')
	end


--
-- Generate the list of project dependencies.
--

	function vc2010.projectReferences(prj)
		local deps = premake.getdependencies(prj)
		if #deps > 0 then
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
			_p(1,'</ItemGroup>')
		end
	end


--
-- Generate the .vcxproj.user file
--

	function vc2010.debugdir(cfg)
		if cfg.debugdir and not vstudio.iswinrt() then
			_p('    <LocalDebuggerWorkingDirectory>%s</LocalDebuggerWorkingDirectory>', path.translate(cfg.debugdir, '\\'))
			_p('    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>')
		end
		if cfg.debugargs then
			_p('    <LocalDebuggerCommandArguments>%s</LocalDebuggerCommandArguments>', table.concat(cfg.debugargs, " "))
		end
	end

	function vc2010.debugenvs(cfg)
		if cfg.debugenvs and #cfg.debugenvs > 0 then
			_p(2,'<LocalDebuggerEnvironment>%s%s</LocalDebuggerEnvironment>',table.concat(cfg.debugenvs, "\n")
					,iif(cfg.flags.DebugEnvsInherit,'\n$(LocalDebuggerEnvironment)','')
				)
			if cfg.flags.DebugEnvsDontMerge then
				_p(2,'<LocalDebuggerMergeEnvironment>false</LocalDebuggerMergeEnvironment>')
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
			vc2010.debugenvs(cfg)
			_p('  </PropertyGroup>')
		end
		_p('</Project>')
	end

--- This whole thing is stupid
	function premake.vs2010_appxmanifest(prj)
		io.indent = "  "
		io.eol = "\r\n"
		_p('<?xml version="1.0" encoding="utf-8"?>')
		if vstudio.toolset == "v120_wp81" then
			_p('<Package xmlns="http://schemas.microsoft.com/appx/2010/manifest" xmlns:m2="http://schemas.microsoft.com/appx/2013/manifest" xmlns:m3="http://schemas.microsoft.com/appx/2014/manifest" xmlns:mp="http://schemas.microsoft.com/appx/2014/phone/manifest">')
		elseif vstudio.storeapp == "8.1" then
			_p('<Package xmlns="http://schemas.microsoft.com/appx/2010/manifest" xmlns:m3="http://schemas.microsoft.com/appx/2013/manifest">')
		else
			_p('<Package xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10" xmlns:mp="http://schemas.microsoft.com/appx/2014/phone/manifest" xmlns:m3="http://schemas.microsoft.com/appx/manifest/uap/windows10">')
		end

		_p(1,'<Identity Name="' .. prj.uuid .. '"')
		_p(2,'Publisher="CN=Unknown"')
		_p(2,'Version="1.0.0.0" />')

		if vstudio.toolset == "v120_wp81" or vstudio.storeapp == "8.2" then
			_p(1,'<mp:PhoneIdentity PhoneProductId="' .. prj.uuid .. '" PhonePublisherId="00000000-0000-0000-0000-000000000000"/>')
		end

		_p(1,'<Properties>')
		_p(2,'<DisplayName>' .. prj.name .. '</DisplayName>')
		_p(2,'<PublisherDisplayName>Unknown</PublisherDisplayName>')
		_p(2,'<Logo>EmptyLogo.png</Logo>')
		_p(1,'</Properties>')

		if vstudio.storeapp == "8.2" then
			_p(1,'<Dependencies>')
			_p(2,'<TargetDeviceFamily Name="Windows.Universal" MinVersion="10.0.10069.0" MaxVersionTested="10.0.10069.0" />')
			_p(1,'</Dependencies>')
		else
			_p(1,'<Prerequisites>')
			_p(2,'<OSMinVersion>6.3.0</OSMinVersion>')
			_p(2,'<OSMaxVersionTested>6.3.0</OSMaxVersionTested>')
			_p(1,'</Prerequisites>')
		end

		_p(1,'<Resources>')
		_p(2,'<Resource Language="x-generate"/>')
		_p(1,'</Resources>')

		_p(1,'<Applications>')
		_p(2,'<Application Id="App"')
		_p(3,'Executable="$targetnametoken$.exe"')
		_p(3,'EntryPoint="App">')
		_p(3,'<m3:VisualElements')
		_p(4,'DisplayName="Blah"')
		_p(4,'Square150x150Logo="Assets\\Logo.png"')
		if vstudio.toolset == "v120_wp81" or vstudio.storeapp == "8.2" then
			_p(4,'Square44x44Logo="Assets\\SmallLogo.png"')
		else
			_p(4,'Square30x30Logo="Assets\\SmallLogo.png"')
		end
		_p(4,'Description="Blah"')
		_p(4,'ForegroundText="light"')
		_p(4,'BackgroundColor="transparent">')
		_p(4,'<m3:SplashScreen Image="%s" />', path.getname(vstudio.splashpath))
		_p(3,'</m3:VisualElements>')
		_p(2,'</Application>')
		_p(1,'</Applications>')

		_p('</Package>')
	end
