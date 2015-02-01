--
-- codeblocks_cbp.lua
-- Generate a Code::Blocks C/C++ project.
-- Copyright (c) 2009, 2011 Jason Perkins and the Premake project
--

	local codeblocks = premake.codeblocks


--
-- Write out a list of the source code files in the project.
--

	function codeblocks.files(prj)
		local pchheader
		if (prj.pchheader) then
			pchheader = path.getrelative(prj.location, prj.pchheader)
		end
		
		for fcfg in premake.project.eachfile(prj) do
			_p(2,'<Unit filename="%s">', premake.esc(fcfg.name))
			if fcfg.name ~= fcfg.vpath then
				_p(3,'<Option virtualFolder="%s" />', path.getdirectory(fcfg.vpath))
			end
			if path.isresourcefile(fcfg.name) then
				_p(3,'<Option compilerVar="WINDRES" />')
			elseif path.iscfile(fcfg.name) and prj.language == "C++" then
				_p(3,'<Option compilerVar="CC" />')
			end
			if not prj.flags.NoPCH and fcfg.name == pchheader then
				_p(3,'<Option compilerVar="%s" />', iif(prj.language == "C", "CC", "CPP"))
				_p(3,'<Option compile="1" />')
				_p(3,'<Option weight="0" />')
				_p(3,'<Add option="-x c++-header" />')
			end
			_p(2,'</Unit>')
		end
	end

	function premake.codeblocks.debugenvs(cfg)
		--Assumption: if gcc is being used then so is gdb although this section will be ignored by
		--other debuggers. If using gcc and not gdb it will silently not pass the
		--environment arguments to the debugger
		if premake.gettool(cfg) == premake.gcc then
			_p(3,'<debugger>')
				_p(4,'<remote_debugging target="%s">', premake.esc(cfg.longname))
					local args = ''
					local sz = #cfg.debugenvs
					for idx, v in ipairs(cfg.debugenvs) do
						args = args .. 'set env ' .. v 
						if sz ~= idx then args = args .. '&#x0A;' end
					end
					_p(5,'<options additional_cmds_before="%s" />',args)
				_p(4,'</remote_debugging>')
			_p(3,'</debugger>')
		else
			 error('Sorry at this moment there is no support for debug environment variables with this debugger and codeblocks')
		end
	end
	
--
-- The main function: write out the project file.
--
	
	function premake.codeblocks.cbp(prj)
		-- alias the C/C++ compiler interface
		local cc = premake.gettool(prj)
		
		_p('<?xml version="1.0" encoding="UTF-8" standalone="yes" ?>')
		_p('<CodeBlocks_project_file>')
		_p(1,'<FileVersion major="1" minor="6" />')
		
		-- write project block header
		_p(1,'<Project>')
		_p(2,'<Option title="%s" />', premake.esc(prj.name))
		_p(2,'<Option pch_mode="2" />')
		_p(2,'<Option compiler="%s" />', _OPTIONS.cc)

		-- build a list of supported target platforms; I don't support cross-compiling yet
		local platforms = premake.filterplatforms(prj.solution, cc.platforms, "Native")
		for i = #platforms, 1, -1 do
			if premake.platforms[platforms[i]].iscrosscompiler then
				table.remove(platforms, i)
			end
		end 
		
		-- write configuration blocks
		_p(2,'<Build>')
		for _, platform in ipairs(platforms) do		
			for cfg in premake.eachconfig(prj, platform) do
				_p(3,'<Target title="%s">', premake.esc(cfg.longname))
				
				_p(4,'<Option output="%s" prefix_auto="0" extension_auto="0" />', premake.esc(cfg.buildtarget.fullpath))
				
				if cfg.debugdir then
					_p(4,'<Option working_dir="%s" />', premake.esc(cfg.debugdir))
				end
				
				_p(4,'<Option object_output="%s" />', premake.esc(cfg.objectsdir))

				-- identify the type of binary
				local types = { WindowedApp = 0, ConsoleApp = 1, StaticLib = 2, SharedLib = 3 }
				_p(4,'<Option type="%d" />', types[cfg.kind])

				_p(4,'<Option compiler="%s" />', _OPTIONS.cc)
				
				if (cfg.kind == "SharedLib") then
					_p(4,'<Option createDefFile="0" />')
					_p(4,'<Option createStaticLib="%s" />', iif(cfg.flags.NoImportLib, 0, 1))
				end

				-- begin compiler block --
				_p(4,'<Compiler>')
				for _,flag in ipairs(table.join(cc.getcflags(cfg), cc.getcxxflags(cfg), cc.getdefines(cfg.defines), cfg.buildoptions)) do
					_p(5,'<Add option="%s" />', premake.esc(flag))
				end
				if not cfg.flags.NoPCH and cfg.pchheader then
					_p(5,'<Add option="-Winvalid-pch" />')
					_p(5,'<Add option="-include &quot;%s&quot;" />', premake.esc(cfg.pchheader))
				end
				for _,v in ipairs(cfg.includedirs) do
					_p(5,'<Add directory="%s" />', premake.esc(v))
				end
				_p(4,'</Compiler>')
				-- end compiler block --
				
				-- begin linker block --
				_p(4,'<Linker>')
				for _,flag in ipairs(table.join(cc.getldflags(cfg), cfg.linkoptions)) do
					_p(5,'<Add option="%s" />', premake.esc(flag))
				end
				for _,v in ipairs(premake.getlinks(cfg, "all", "directory")) do
					_p(5,'<Add directory="%s" />', premake.esc(v))
				end
				for _,v in ipairs(premake.getlinks(cfg, "all", "basename")) do
					_p(5,'<Add library="%s" />', premake.esc(v))
				end
				_p(4,'</Linker>')
				-- end linker block --
				
				-- begin resource compiler block --
				if premake.findfile(cfg, ".rc") then
					_p(4,'<ResourceCompiler>')
					for _,v in ipairs(cfg.includedirs) do
						_p(5,'<Add directory="%s" />', premake.esc(v))
					end
					for _,v in ipairs(cfg.resincludedirs) do
						_p(5,'<Add directory="%s" />', premake.esc(v))
					end
					_p(4,'</ResourceCompiler>')
				end
				-- end resource compiler block --
				
				-- begin build steps --
				if #cfg.prebuildcommands > 0 or #cfg.postbuildcommands > 0 then
					_p(4,'<ExtraCommands>')
					for _,v in ipairs(cfg.prebuildcommands) do
						_p(5,'<Add before="%s" />', premake.esc(v))
					end
					for _,v in ipairs(cfg.postbuildcommands) do
						_p(5,'<Add after="%s" />', premake.esc(v))
					end

					_p(4,'</ExtraCommands>')
				end
				-- end build steps --
				
				_p(3,'</Target>')
			end
		end
		_p(2,'</Build>')
		
		codeblocks.files(prj)
		
		_p(2,'<Extensions>')
        for _, platform in ipairs(platforms) do
			for cfg in premake.eachconfig(prj, platform) do
				if cfg.debugenvs and #cfg.debugenvs > 0 then
					premake.codeblocks.debugenvs(cfg)
				end
			end
		end
		_p(2,'</Extensions>')

		_p(1,'</Project>')
		_p('</CodeBlocks_project_file>')
		_p('')
		
	end
