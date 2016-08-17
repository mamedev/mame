--
-- codelite_project.lua
-- Generate a CodeLite C/C++ project file.
-- Copyright (c) 2009, 2011 Jason Perkins and the Premake project
--

	local codelite = premake.codelite
	local tree = premake.tree


--
-- Write out a list of the source code files in the project.
--

	function codelite.files(prj)
		local tr = premake.project.buildsourcetree(prj)
		tree.traverse(tr, {
			
			-- folders are handled at the internal nodes
			onbranchenter = function(node, depth)
				_p(depth, '<VirtualDirectory Name="%s">', node.name)
			end,

			onbranchexit = function(node, depth)
				_p(depth, '</VirtualDirectory>')
			end,

			-- source files are handled at the leaves
			onleaf = function(node, depth)
				_p(depth, '<File Name="%s"/>', node.cfg.name)
			end,
			
		}, true, 1)
	end
	

--
-- The main function: write out the project file.
--

	function premake.codelite.project(prj)
		io.indent = "  "
		
		_p('<?xml version="1.0" encoding="utf-8"?>')
		_p('<CodeLite_Project Name="%s">', premake.esc(prj.name))

		-- Write out the list of source code files in the project
		codelite.files(prj)

		local types = { 
			ConsoleApp  = "Executable", 
			WindowedApp = "Executable", 
			StaticLib   = "Static Library",
			SharedLib   = "Dynamic Library",
		}
		_p('  <Settings Type="%s">', types[prj.kind])
		
		-- build a list of supported target platforms; I don't support cross-compiling yet
		local platforms = premake.filterplatforms(prj.solution, premake[_OPTIONS.cc].platforms, "Native")
		for i = #platforms, 1, -1 do
			if premake.platforms[platforms[i]].iscrosscompiler then
				table.remove(platforms, i)
			end
		end 

		for _, platform in ipairs(platforms) do
			for cfg in premake.eachconfig(prj, platform) do
				local name = premake.esc(cfg.longname):gsub("|","_")
				local compiler = iif(cfg.language == "C", "gcc", "g++")
				_p('    <Configuration Name="%s" CompilerType="gnu %s" DebuggerType="GNU gdb debugger" Type="%s">', name, compiler, types[cfg.kind])
			
				local fname  = premake.esc(cfg.buildtarget.fullpath)
				local objdir = premake.esc(cfg.objectsdir)
				local runcmd = cfg.buildtarget.name
				local rundir = cfg.debugdir or cfg.buildtarget.directory
				local runargs = table.concat(cfg.debugargs, " ")
				local pause  = iif(cfg.kind == "WindowedApp", "no", "yes")
				local includedirs = table.join(cfg.userincludedirs, cfg.includedirs)
				_p('      <General OutputFile="%s" IntermediateDirectory="%s" Command="./%s" CommandArguments="%s" WorkingDirectory="%s" PauseExecWhenProcTerminates="%s"/>', fname, objdir, runcmd, runargs, rundir, pause)
				
				-- begin compiler block --
				local flags = premake.esc(table.join(premake.gcc.getcflags(cfg), premake.gcc.getcxxflags(cfg), cfg.buildoptions))
				_p('      <Compiler Required="yes" Options="%s">', table.concat(flags, ";"))
				
				for _,v in ipairs(includedirs) do
					_p('        <IncludePath Value="%s"/>', premake.esc(v))
				end
				for _,v in ipairs(cfg.defines) do
					_p('        <Preprocessor Value="%s"/>', premake.esc(v))
				end
				_p('      </Compiler>')
				-- end compiler block --
				
				-- begin linker block --
				flags = premake.esc(table.join(premake.gcc.getldflags(cfg), cfg.linkoptions))
				_p('      <Linker Required="yes" Options="%s">', table.concat(flags, ";"))
				for _,v in ipairs(premake.getlinks(cfg, "all", "directory")) do
					_p('        <LibraryPath Value="%s" />', premake.esc(v))
				end
				for _,v in ipairs(premake.getlinks(cfg, "all", "basename")) do
					_p('        <Library Value="%s" />', premake.esc(v))
				end
				_p('      </Linker>')
				-- end linker block --
				
				-- begin resource compiler block --
				if premake.findfile(cfg, ".rc") then
					local defines = table.implode(table.join(cfg.defines, cfg.resdefines), "-D", ";", "")
					local options = table.concat(cfg.resoptions, ";")
					_p('      <ResourceCompiler Required="yes" Options="%s%s">', defines, options)
					for _,v in ipairs(table.join(cfg.includedirs, cfg.resincludedirs)) do
						_p('        <IncludePath Value="%s"/>', premake.esc(v))
					end
					_p('      </ResourceCompiler>')
				else
					_p('      <ResourceCompiler Required="no" Options=""/>')
				end
				-- end resource compiler block --
				
				-- begin build steps --
				if #cfg.prebuildcommands > 0 then
					_p('      <PreBuild>')
					for _,v in ipairs(cfg.prebuildcommands) do
						_p('        <Command Enabled="yes">%s</Command>', premake.esc(v))
					end
					_p('      </PreBuild>')
				end
				if #cfg.postbuildcommands > 0 then
					_p('      <PostBuild>')
					for _,v in ipairs(cfg.postbuildcommands) do
						_p('        <Command Enabled="yes">%s</Command>', premake.esc(v))
					end
					_p('      </PostBuild>')
				end
				-- end build steps --
				
				_p('      <CustomBuild Enabled="no">')
				_p('        <CleanCommand></CleanCommand>')
				_p('        <BuildCommand></BuildCommand>')
				_p('        <SingleFileCommand></SingleFileCommand>')
				_p('        <MakefileGenerationCommand></MakefileGenerationCommand>')
				_p('        <ThirdPartyToolName>None</ThirdPartyToolName>')
				_p('        <WorkingDirectory></WorkingDirectory>')
				_p('      </CustomBuild>')
				_p('      <AdditionalRules>')
				_p('        <CustomPostBuild></CustomPostBuild>')
				_p('        <CustomPreBuild></CustomPreBuild>')
				_p('      </AdditionalRules>')
				_p('    </Configuration>')
			end
		end
		_p('  </Settings>')

		for _, platform in ipairs(platforms) do
			for cfg in premake.eachconfig(prj, platform) do
				_p('  <Dependencies name="%s">', cfg.longname:gsub("|","_"))
				for _,dep in ipairs(premake.getdependencies(prj)) do
					_p('    <Project Name="%s"/>', dep.name)
				end
				_p('  </Dependencies>')
			end
		end
		
		_p('</CodeLite_Project>')
	end
