--
-- vs200x_vcproj.lua
-- Generate a Visual Studio 2002-2008 C/C++ project.
-- Copyright (c) 2009-2013 Jason Perkins and the Premake project
--


--
-- Set up a namespace for this file
--

	premake.vstudio.vc200x = { }
	local vc200x = premake.vstudio.vc200x
	local tree = premake.tree


--
-- Return the version-specific text for a boolean value.
--

	local function bool(value)
		return iif(value, "true", "false")
	end


--
-- Return the optimization code.
--

	function vc200x.optimization(cfg)
		local result = 0
		for _, value in ipairs(cfg.flags) do
			if (value == "Optimize") then
				result = 3
			elseif (value == "OptimizeSize") then
				result = 1
			elseif (value == "OptimizeSpeed") then
				result = 2
			end
		end
		return result
	end



--
-- Write the project file header
--

	function vc200x.header(element)
		io.eol = "\r\n"
		_p('<?xml version="1.0" encoding="Windows-1252"?>')
		_p('<%s', element)
		_p(1,'ProjectType="Visual C++"')
		_p(1,'Version="9.00"')
	end


--
-- Write out the <Configuration> element.
--

	function vc200x.Configuration(name, cfg)
		_p(2,'<Configuration')
		_p(3,'Name="%s"', premake.esc(name))
		_p(3,'OutputDirectory="%s"', premake.esc(cfg.buildtarget.directory))
		_p(3,'IntermediateDirectory="%s"', premake.esc(cfg.objectsdir))

		local cfgtype
		if (cfg.kind == "SharedLib") then
			cfgtype = 2
		elseif (cfg.kind == "StaticLib") then
			cfgtype = 4
		else
			cfgtype = 1
		end
		_p(3,'ConfigurationType="%s"', cfgtype)

		if (cfg.flags.MFC) then
			_p(3, 'UseOfMFC="%d"', iif(cfg.flags.StaticRuntime, 1, 2))
		end
		if (cfg.flags.ATL or cfg.flags.StaticATL) then
			_p(3, 'UseOfATL="%d"', iif(cfg.flags.StaticATL, 1, 2))
		end
		_p(3,'CharacterSet="%s"', iif(cfg.flags.Unicode, 1, 2))
		if cfg.flags.Managed then
			_p(3,'ManagedExtensions="1"')
		end
		_p(3,'>')
	end


--
-- Write out the <Files> element.
--

	function vc200x.Files(prj)
		local tr = premake.project.buildsourcetree(prj)

		tree.traverse(tr, {
			-- folders are handled at the internal nodes
			onbranchenter = function(node, depth)
				_p(depth, '<Filter')
				_p(depth, '\tName="%s"', node.name)
				_p(depth, '\tFilter=""')
				_p(depth, '\t>')
			end,

			onbranchexit = function(node, depth)
				_p(depth, '</Filter>')
			end,

			-- source files are handled at the leaves
			onleaf = function(node, depth)
				local fname = node.cfg.name

				_p(depth, '<File')
				_p(depth, '\tRelativePath="%s"', path.translate(fname, "\\"))
				_p(depth, '\t>')
				depth = depth + 1

				local excluded = table.icontains(prj.excludes, fname)

				-- handle file configuration stuff. This needs to be cleaned up and simplified.
				-- configurations are cached, so this isn't as bad as it looks
				for _, cfginfo in ipairs(prj.solution.vstudio_configs) do
					if cfginfo.isreal then
						local cfg = premake.getconfig(prj, cfginfo.src_buildcfg, cfginfo.src_platform)

						local usePCH = (not prj.flags.NoPCH and prj.pchsource == node.cfg.name)
						local isSourceCode = path.iscppfile(fname)
						local needsCompileAs = (path.iscfile(fname) ~= premake.project.iscproject(prj))

						if usePCH or isSourceCode then
							_p(depth, '<FileConfiguration')
							_p(depth, '\tName="%s"', cfginfo.name)
							if excluded or table.icontains(cfg.excludes, fname) then
								_p(depth, '\tExcludedFromBuild="true"')
							end
							_p(depth, '\t>')
							_p(depth, '\t<Tool')
							_p(depth, '\t\tName="%s"'
								, iif(cfg.system == "Xbox360", "VCCLX360CompilerTool", "VCCLCompilerTool")
								)
							_p(depth, '\t\tObjectFile="$(IntDir)\\%s.obj"'
								, path.translate(path.trimdots(path.removeext(fname)), "\\")
								)

							if needsCompileAs then
								_p(depth, '\t\tCompileAs="%s"', iif(path.iscfile(fname), 1, 2))
							end

							if usePCH then
								if cfg.system == "PS3" then
									local options = table.join(premake.snc.getcflags(cfg),
									                           premake.snc.getcxxflags(cfg),
									                           cfg.buildoptions)
									options = table.concat(options, " ");
									options = options .. ' --create_pch="$(IntDir)/$(TargetName).pch"'
									_p(depth, '\t\tAdditionalOptions="%s"', premake.esc(options))
								else
									_p(depth, '\t\tUsePrecompiledHeader="1"')
								end
							end

							_p(depth, '\t/>')
							_p(depth, '</FileConfiguration>')
						end

					end
				end

				depth = depth - 1
				_p(depth, '</File>')
			end,
		}, false, 2)

	end


--
-- Write out the <Platforms> element; ensures that each target platform
-- is listed only once. Skips over .NET's pseudo-platforms (like "Any CPU").
--

	function vc200x.Platforms(prj)
		local used = { }
		_p(1,'<Platforms>')
		for _, cfg in ipairs(prj.solution.vstudio_configs) do
			if cfg.isreal and not table.contains(used, cfg.platform) then
				table.insert(used, cfg.platform)
				_p(2,'<Platform')
				_p(3,'Name="%s"', cfg.platform)
				_p(2,'/>')
			end
		end
		_p(1,'</Platforms>')
	end


--
-- Return the debugging symbols level for a configuration.
--

	function vc200x.Symbols(cfg)
		if (not cfg.flags.Symbols) then
			return 0
		else
			-- Edit-and-continue does't work for some configurations
			if cfg.flags.NoEditAndContinue or
			   vc200x.optimization(cfg) ~= 0 or
			   cfg.flags.Managed or
			   cfg.platform == "x64" then
				return 3
			else
				return 4
			end
		end
	end


--
-- Compiler block for Windows and XBox360 platforms.
--

	function vc200x.VCCLCompilerTool(cfg)
		_p(3,'<Tool')
		_p(4,'Name="%s"', iif(cfg.platform ~= "Xbox360", "VCCLCompilerTool", "VCCLX360CompilerTool"))

		if cfg.flags.UnsignedChar then
			table.insert(cfg.buildoptions, '/J')
		end

		if #cfg.buildoptions > 0 then
			_p(4,'AdditionalOptions="%s /MP"', table.concat(premake.esc(cfg.buildoptions), " "))
		end

		_p(4,'Optimization="%s"', vc200x.optimization(cfg))

		if cfg.flags.NoFramePointer then
			_p(4,'OmitFramePointers="%s"', bool(true))
		end

		if #cfg.includedirs > 0 then
			_p(4,'AdditionalIncludeDirectories="%s"', premake.esc(path.translate(table.concat(cfg.includedirs, ";"), '\\')))
		end

		if #cfg.defines > 0 then
			_p(4,'PreprocessorDefinitions="%s"', premake.esc(table.concat(cfg.defines, ";")))
		end

		if premake.config.isdebugbuild(cfg) and cfg.flags.EnableMinimalRebuild and not cfg.flags.Managed then
			_p(4,'MinimalRebuild="%s"', bool(true))
		end

		if cfg.flags.NoExceptions then
			_p(4,'ExceptionHandling="0"')
		elseif cfg.flags.SEH then
			_p(4,'ExceptionHandling="2"')
		end

		if vc200x.optimization(cfg) == 0 and not cfg.flags.Managed then
			_p(4,'BasicRuntimeChecks="3"')
		end
		if vc200x.optimization(cfg) ~= 0 then
			_p(4,'StringPooling="%s"', bool(true))
		end

		local runtime
		if premake.config.isdebugbuild(cfg) then
			runtime = iif(cfg.flags.StaticRuntime, 1, 3)
		else
			runtime = iif(cfg.flags.StaticRuntime, 0, 2)
		end
		_p(4,'RuntimeLibrary="%s"', runtime)

		_p(4,'EnableFunctionLevelLinking="%s"', bool(true))

		if cfg.platform ~= "Xbox360" and cfg.platform ~= "x64" and cfg.platform ~= "Durango" then
			if cfg.flags.EnableSSE then
				_p(4,'EnableEnhancedInstructionSet="1"')
			elseif cfg.flags.EnableSSE2 then
				_p(4,'EnableEnhancedInstructionSet="2"')
			end
		end

		if cfg.flags.FloatFast then
			_p(4,'FloatingPointModel="2"')
		elseif cfg.flags.FloatStrict then
			_p(4,'FloatingPointModel="1"')
		end

		if cfg.flags.NoRTTI and not cfg.flags.Managed then
			_p(4,'RuntimeTypeInfo="%s"', bool(false))
		end

		if cfg.flags.FastCall then
			_p(4,'CallingConvention="1"')
		elseif cfg.flags.StdCall then
			_p(4,'CallingConvention="2"')
		end

		if cfg.flags.NativeWChar then
			_p(4,'TreatWChar_tAsBuiltInType="%s"', bool(true))
		elseif cfg.flags.NoNativeWChar then
			_p(4,'TreatWChar_tAsBuiltInType="%s"', bool(false))
		end

		if not cfg.flags.NoPCH and cfg.pchheader then
			_p(4,'UsePrecompiledHeader="2"')
			_p(4,'PrecompiledHeaderThrough="%s"', cfg.pchheader)
		else
			_p(4,'UsePrecompiledHeader="%s"', iif(cfg.flags.NoPCH, 0, 2))
		end

		_p(4,'WarningLevel="%s"', iif(cfg.flags.ExtraWarnings, 4, 3))

		if cfg.flags.FatalWarnings then
			_p(4,'WarnAsError="%s"', bool(true))
		end

		if _ACTION < "vs2008" and not cfg.flags.Managed then
			_p(4,'Detect64BitPortabilityProblems="%s"', bool(not cfg.flags.No64BitChecks))
		end

		_p(4,'ProgramDataBaseFileName="$(OutDir)\\%s.pdb"', path.getbasename(cfg.buildtarget.name))
		_p(4,'DebugInformationFormat="%s"', vc200x.Symbols(cfg))
		if cfg.language == "C" then
			_p(4, 'CompileAs="1"')
		end
		_p(3,'/>')
	end



--
-- Linker block for Windows and Xbox 360 platforms.
--

	function vc200x.VCLinkerTool(cfg)
		_p(3,'<Tool')
		if cfg.kind ~= "StaticLib" then
			_p(4,'Name="%s"', iif(cfg.platform ~= "Xbox360", "VCLinkerTool", "VCX360LinkerTool"))

			if cfg.flags.NoImportLib then
				_p(4,'IgnoreImportLibrary="%s"', bool(true))
			end

			if #cfg.linkoptions > 0 then
				_p(4,'AdditionalOptions="%s"', table.concat(premake.esc(cfg.linkoptions), " "))
			end

			if #cfg.links > 0 then
				_p(4,'AdditionalDependencies="%s"', table.concat(premake.getlinks(cfg, "all", "fullpath"), " "))
			end

			_p(4,'OutputFile="$(OutDir)\\%s"', cfg.buildtarget.name)

			_p(4,'LinkIncremental="%s"',
				iif(premake.config.isincrementallink(cfg) , 2, 1))

			_p(4,'AdditionalLibraryDirectories="%s"', table.concat(premake.esc(path.translate(cfg.libdirs, '\\')) , ";"))

			local deffile = premake.findfile(cfg, ".def")
			if deffile then
				_p(4,'ModuleDefinitionFile="%s"', deffile)
			end

			if cfg.flags.NoManifest then
				_p(4,'GenerateManifest="%s"', bool(false))
			end

			_p(4,'GenerateDebugInformation="%s"', bool(vc200x.Symbols(cfg) ~= 0))

			if vc200x.Symbols(cfg) ~= 0 then
				_p(4,'ProgramDataBaseFileName="$(OutDir)\\%s.pdb"', path.getbasename(cfg.buildtarget.name))
			end

			_p(4,'SubSystem="%s"', iif(cfg.kind == "ConsoleApp", 1, 2))

			if vc200x.optimization(cfg) ~= 0 then
				_p(4,'OptimizeReferences="2"')
				_p(4,'EnableCOMDATFolding="2"')
			end

			if (cfg.kind == "ConsoleApp" or cfg.kind == "WindowedApp") and not cfg.flags.WinMain then
				_p(4,'EntryPointSymbol="mainCRTStartup"')
			end

			if cfg.kind == "SharedLib" then
				local implibname = cfg.linktarget.fullpath
				_p(4,'ImportLibrary="%s"', iif(cfg.flags.NoImportLib, cfg.objectsdir .. "\\" .. path.getname(implibname), implibname))
			end

			_p(4,'TargetMachine="%d"', iif(cfg.platform == "x64", 17, 1))

		else
			_p(4,'Name="VCLibrarianTool"')

			if #cfg.links > 0 then
				_p(4,'AdditionalDependencies="%s"', table.concat(premake.getlinks(cfg, "all", "fullpath"), " "))
			end

			_p(4,'OutputFile="$(OutDir)\\%s"', cfg.buildtarget.name)

			if #cfg.libdirs > 0 then
				_p(4,'AdditionalLibraryDirectories="%s"', premake.esc(path.translate(table.concat(cfg.libdirs , ";"))))
			end

			local addlOptions = {}
			if cfg.platform == "x32" then
				table.insert(addlOptions, "/MACHINE:X86")
			elseif cfg.platform == "x64" then
				table.insert(addlOptions, "/MACHINE:X64")
			end
			addlOptions = table.join(addlOptions, cfg.linkoptions)
			if #addlOptions > 0 then
				_p(4,'AdditionalOptions="%s"', table.concat(premake.esc(addlOptions), " "))
			end
		end

		_p(3,'/>')
	end


--
-- Compiler and linker blocks for the PS3 platform, which uses Sony's SNC.
--

	function vc200x.VCCLCompilerTool_PS3(cfg)
		_p(3,'<Tool')
		_p(4,'Name="VCCLCompilerTool"')

		local buildoptions = table.join(premake.snc.getcflags(cfg), premake.snc.getcxxflags(cfg), cfg.buildoptions)
		if not cfg.flags.NoPCH and cfg.pchheader then
			_p(4,'UsePrecompiledHeader="2"')
			_p(4,'PrecompiledHeaderThrough="%s"', path.getname(cfg.pchheader))
			table.insert(buildoptions, '--use_pch="$(IntDir)/$(TargetName).pch"')
		else
			_p(4,'UsePrecompiledHeader="%s"', iif(cfg.flags.NoPCH, 0, 2))
		end

		_p(4,'AdditionalOptions="%s"', premake.esc(table.concat(buildoptions, " ")))

		if #cfg.includedirs > 0 then
			_p(4,'AdditionalIncludeDirectories="%s"', premake.esc(path.translate(table.concat(cfg.includedirs, ";"), '\\')))
		end

		if #cfg.defines > 0 then
			_p(4,'PreprocessorDefinitions="%s"', table.concat(premake.esc(cfg.defines), ";"))
		end

		_p(4,'ProgramDataBaseFileName="$(OutDir)\\%s.pdb"', path.getbasename(cfg.buildtarget.name))
		_p(4,'DebugInformationFormat="0"')
		_p(4,'CompileAs="0"')
		_p(3,'/>')
	end


	function vc200x.VCLinkerTool_PS3(cfg)
		_p(3,'<Tool')
		if cfg.kind ~= "StaticLib" then
			_p(4,'Name="VCLinkerTool"')

			local buildoptions = table.join(premake.snc.getldflags(cfg), cfg.linkoptions)
			if #buildoptions > 0 then
				_p(4,'AdditionalOptions="%s"', premake.esc(table.concat(buildoptions, " ")))
			end

			if #cfg.links > 0 then
				_p(4,'AdditionalDependencies="%s"', table.concat(premake.getlinks(cfg, "all", "fullpath"), " "))
			end

			_p(4,'OutputFile="$(OutDir)\\%s"', cfg.buildtarget.name)
			_p(4,'LinkIncremental="0"')
			_p(4,'AdditionalLibraryDirectories="%s"', table.concat(premake.esc(path.translate(cfg.libdirs, '\\')) , ";"))
			_p(4,'GenerateManifest="%s"', bool(false))
			_p(4,'ProgramDatabaseFile=""')
			_p(4,'RandomizedBaseAddress="1"')
			_p(4,'DataExecutionPrevention="0"')
		else
			_p(4,'Name="VCLibrarianTool"')

			local buildoptions = table.join(premake.snc.getldflags(cfg), cfg.linkoptions)
			if #buildoptions > 0 then
				_p(4,'AdditionalOptions="%s"', premake.esc(table.concat(buildoptions, " ")))
			end

			if #cfg.links > 0 then
				_p(4,'AdditionalDependencies="%s"', table.concat(premake.getlinks(cfg, "all", "fullpath"), " "))
			end

			_p(4,'OutputFile="$(OutDir)\\%s"', cfg.buildtarget.name)

			if #cfg.libdirs > 0 then
				_p(4,'AdditionalLibraryDirectories="%s"', premake.esc(path.translate(table.concat(cfg.libdirs , ";"))))
			end
		end

		_p(3,'/>')
	end



--
-- Resource compiler block.
--

	function vc200x.VCResourceCompilerTool(cfg)
		_p(3,'<Tool')
		_p(4,'Name="VCResourceCompilerTool"')

		if #cfg.resoptions > 0 then
			_p(4,'AdditionalOptions="%s"', table.concat(premake.esc(cfg.resoptions), " "))
		end

		if #cfg.defines > 0 or #cfg.resdefines > 0 then
			_p(4,'PreprocessorDefinitions="%s"', table.concat(premake.esc(table.join(cfg.defines, cfg.resdefines)), ";"))
		end

		if #cfg.includedirs > 0 or #cfg.resincludedirs > 0 then
			local dirs = table.join(cfg.includedirs, cfg.resincludedirs)
			_p(4,'AdditionalIncludeDirectories="%s"', premake.esc(path.translate(table.concat(dirs, ";"), '\\')))
		end

		_p(3,'/>')
	end



--
-- Manifest block.
--

	function vc200x.VCManifestTool(cfg)
		-- locate all manifest files
		local manifests = { }
		for _, fname in ipairs(cfg.files) do
			if path.getextension(fname) == ".manifest" then
				table.insert(manifests, fname)
			end
		end

		_p(3,'<Tool')
		_p(4,'Name="VCManifestTool"')
		if #manifests > 0 then
			_p(4,'AdditionalManifestFiles="%s"', premake.esc(table.concat(manifests, ";")))
		end
		_p(3,'/>')
	end



--
-- VCMIDLTool block
--

	function vc200x.VCMIDLTool(cfg)
		_p(3,'<Tool')
		_p(4,'Name="VCMIDLTool"')
		if cfg.platform == "x64" then
			_p(4,'TargetEnvironment="3"')
		end
		_p(3,'/>')
	end



--
-- Write out a custom build steps block.
--

	function vc200x.buildstepsblock(name, steps)
		_p(3,'<Tool')
		_p(4,'Name="%s"', name)
		if #steps > 0 then
			_p(4,'CommandLine="%s"', premake.esc(table.implode(steps, "", "", "\r\n")))
		end
		_p(3,'/>')
	end



--
-- Map project tool blocks to handler functions. Unmapped blocks will output
-- an empty <Tool> element.
--

	local blockmap =
	{
		VCCLCompilerTool       = vc200x.VCCLCompilerTool,
		VCCLCompilerTool_PS3   = vc200x.VCCLCompilerTool_PS3,
		VCLinkerTool           = vc200x.VCLinkerTool,
		VCLinkerTool_PS3       = vc200x.VCLinkerTool_PS3,
		VCManifestTool         = vc200x.VCManifestTool,
		VCMIDLTool             = vc200x.VCMIDLTool,
		VCResourceCompilerTool = vc200x.VCResourceCompilerTool,
	}


--
-- Return a list of sections for a particular Visual Studio version and target platform.
--

	local function getsections(version, platform)
		if platform == "Xbox360" then
			return {
				"VCPreBuildEventTool",
				"VCCustomBuildTool",
				"VCXMLDataGeneratorTool",
				"VCWebServiceProxyGeneratorTool",
				"VCMIDLTool",
				"VCCLCompilerTool",
				"VCManagedResourceCompilerTool",
				"VCResourceCompilerTool",
				"VCPreLinkEventTool",
				"VCLinkerTool",
				"VCALinkTool",
				"VCX360ImageTool",
				"VCBscMakeTool",
				"VCX360DeploymentTool",
				"VCPostBuildEventTool",
				"DebuggerTool",
			}
		end

		if platform == "PS3" then
			return {
				"VCPreBuildEventTool",
				"VCCustomBuildTool",
				"VCXMLDataGeneratorTool",
				"VCWebServiceProxyGeneratorTool",
				"VCMIDLTool",
				"VCCLCompilerTool_PS3",
				"VCManagedResourceCompilerTool",
				"VCResourceCompilerTool",
				"VCPreLinkEventTool",
				"VCLinkerTool_PS3",
				"VCALinkTool",
				"VCManifestTool",
				"VCXDCMakeTool",
				"VCBscMakeTool",
				"VCFxCopTool",
				"VCAppVerifierTool",
				"VCWebDeploymentTool",
				"VCPostBuildEventTool"
			}
		end

		return {
			"VCPreBuildEventTool",
			"VCCustomBuildTool",
			"VCXMLDataGeneratorTool",
			"VCWebServiceProxyGeneratorTool",
			"VCMIDLTool",
			"VCCLCompilerTool",
			"VCManagedResourceCompilerTool",
			"VCResourceCompilerTool",
			"VCPreLinkEventTool",
			"VCLinkerTool",
			"VCALinkTool",
			"VCManifestTool",
			"VCXDCMakeTool",
			"VCBscMakeTool",
			"VCFxCopTool",
			"VCAppVerifierTool",
			"VCWebDeploymentTool",
			"VCPostBuildEventTool"
		}
	end



--
-- The main function: write the project file.
--

	function vc200x.generate(prj)
		vc200x.header('VisualStudioProject')

		_p(1,'Name="%s"', premake.esc(prj.name))
		_p(1,'ProjectGUID="{%s}"', prj.uuid)
		_p(1,'RootNamespace="%s"', prj.name)
		_p(1,'Keyword="%s"', iif(prj.flags.Managed, "ManagedCProj", "Win32Proj"))
		_p(1,'>')

		-- list the target platforms
		vc200x.Platforms(prj)

		_p(1,'<ToolFiles>')
		_p(1,'</ToolFiles>')

		_p(1,'<Configurations>')
		for _, cfginfo in ipairs(prj.solution.vstudio_configs) do
			if cfginfo.isreal then
				local cfg = premake.getconfig(prj, cfginfo.src_buildcfg, cfginfo.src_platform)

				-- Start a configuration
				vc200x.Configuration(cfginfo.name, cfg)
				for _, block in ipairs(getsections(_ACTION, cfginfo.src_platform)) do

					if blockmap[block] then
						blockmap[block](cfg)

					-- Build event blocks --
					elseif block == "VCPreBuildEventTool" then
						vc200x.buildstepsblock("VCPreBuildEventTool", cfg.prebuildcommands)
					elseif block == "VCPreLinkEventTool" then
						vc200x.buildstepsblock("VCPreLinkEventTool", cfg.prelinkcommands)
					elseif block == "VCPostBuildEventTool" then
						vc200x.buildstepsblock("VCPostBuildEventTool", cfg.postbuildcommands)
					-- End build event blocks --

					-- Xbox 360 custom sections --
					elseif block == "VCX360DeploymentTool" then
						_p(3,'<Tool')
						_p(4,'Name="VCX360DeploymentTool"')
						_p(4,'DeploymentType="0"')
						if #cfg.deploymentoptions > 0 then
							_p(4,'AdditionalOptions="%s"', table.concat(premake.esc(cfg.deploymentoptions), " "))
						end
						_p(3,'/>')

					elseif block == "VCX360ImageTool" then
						_p(3,'<Tool')
						_p(4,'Name="VCX360ImageTool"')
						if #cfg.imageoptions > 0 then
							_p(4,'AdditionalOptions="%s"', table.concat(premake.esc(cfg.imageoptions), " "))
						end
						if cfg.imagepath ~= nil then
							_p(4,'OutputFileName="%s"', premake.esc(path.translate(cfg.imagepath)))
						end
						_p(3,'/>')

					elseif block == "DebuggerTool" then
						_p(3,'<DebuggerTool')
						_p(3,'/>')

					-- End Xbox 360 custom sections --

					else
						_p(3,'<Tool')
						_p(4,'Name="%s"', block)
						_p(3,'/>')
					end

				end

				_p(2,'</Configuration>')
			end
		end
		_p(1,'</Configurations>')

		_p(1,'<References>')
		_p(1,'</References>')

		_p(1,'<Files>')
		vc200x.Files(prj)
		_p(1,'</Files>')

		_p(1,'<Globals>')
		_p(1,'</Globals>')
		_p('</VisualStudioProject>')
	end



