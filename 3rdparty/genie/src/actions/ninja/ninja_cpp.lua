premake.ninja.cpp = { }
local ninja = premake.ninja
local cpp   = premake.ninja.cpp
local p     = premake
local tree  = p.tree

-- generate project + config build file
	function ninja.generate_cpp(prj)
		local pxy = ninja.get_proxy("prj", prj)
		local tool = premake.gettool(prj)
		
		-- build a list of supported target platforms that also includes a generic build
		local platforms = premake.filterplatforms(prj.solution, tool.platforms, "Native")

		for _, platform in ipairs(platforms) do
			for cfg in p.eachconfig(pxy, platform) do
				p.generate(cfg, cfg:getprojectfilename(), function() cpp.generate_config(prj, cfg) end)
			end
		end
	end
	
	function cpp.generate_config(prj, cfg)
		local tool = premake.gettool(prj)
		
		_p("# project build file")
		_p("# generated with GENie ninja")
		_p("")

		-- needed for implicit outputs, introduced in 1.7
		_p("ninja_required_version = 1.7")
		_p("")
		
		local flags = {
			defines   = ninja.list(tool.getdefines(cfg.defines)),
			includes  = ninja.list(table.join(tool.getincludedirs(cfg.includedirs), tool.getquoteincludedirs(cfg.userincludedirs))),
			cppflags  = ninja.list(tool.getcppflags(cfg)),
			cflags    = ninja.list(table.join(tool.getcflags(cfg), cfg.buildoptions, cfg.buildoptions_c)),
			cxxflags  = ninja.list(table.join(tool.getcflags(cfg), tool.getcxxflags(cfg), cfg.buildoptions, cfg.buildoptions_cpp)),
			objcflags = ninja.list(table.join(tool.getcflags(cfg), tool.getcxxflags(cfg), cfg.buildoptions, cfg.buildoptions_objc)),
		}

		_p("")
			
		_p("# core rules for " .. cfg.name)
		_p("rule cc")
		_p("  command = " .. tool.cc .. " $defines $includes $flags -MMD -MF $out.d -c -o $out $in")
		_p("  description = cc $out")
		_p("  depfile = $out.d")
		_p("  deps = gcc")
		_p("")
		_p("rule cxx")
		_p("  command = " .. tool.cxx .. " $defines $includes $flags -MMD -MF $out.d -c -o $out $in")
		_p("  description = cxx $out")
		_p("  depfile = $out.d")
		_p("  deps = gcc")
		_p("")
		_p("rule ar")
		_p("  command = " .. tool.ar .. " $flags $out $in $libs " .. (os.is("MacOSX") and " 2>&1 > /dev/null | sed -e '/.o) has no symbols$$/d'" or ""))
		_p("  description = ar $out")
		_p("")
		
		
		local link = iif(cfg.language == "C", tool.cc, tool.cxx)
		_p("rule link")
		_p("  command = " .. link .. " -o $out $in $all_ldflags $libs")
		_p("  description = link $out")
		_p("")

		cpp.file_rules(cfg, flags)
		
		local objfiles = {}
		
		for _, file in ipairs(cfg.files) do
			if path.isSourceFile(file) then
				table.insert(objfiles, cpp.objectname(cfg, file))
			end
		end
		_p('')
		
		cpp.linker(prj, cfg, objfiles, tool, flags)

		_p("")
	end
	
	function cpp.objectname(cfg, file)
		return path.join(cfg.objectsdir, path.trimdots(path.removeext(file)) .. ".o")
	end

	function cpp.file_rules(cfg, flags)
		_p("# build files")
		
		for _, file in ipairs(cfg.files) do
			if path.isSourceFile(file) then
				local objfilename = cpp.objectname(cfg, file)
				
				local cflags = "cflags"
				if path.isobjcfile(file) then
					_p("build " .. objfilename .. ": cxx " .. file)
					cflags = "objcflags"
				elseif path.iscfile(file) and not cfg.options.ForceCPP then
					_p("build " .. objfilename .. ": cc " .. file)
				else
					_p("build " .. objfilename .. ": cxx " .. file)
					cflags = "cxxflags"
				end
				
				_p(1, "flags    = " .. flags[cflags])
				_p(1, "includes = " .. flags.includes)
				_p(1, "defines  = " .. flags.defines)
			elseif path.isresourcefile(file) then
				-- TODO
			end
		end
		
		_p("")
	end
	
	function cpp.linker(prj, cfg, objfiles, tool)
		local all_ldflags = ninja.list(table.join(tool.getlibdirflags(cfg), tool.getldflags(cfg), cfg.linkoptions))
		local lddeps      = ninja.list(premake.getlinks(cfg, "siblings", "fullpath")) 
		local libs        = lddeps .. " " .. ninja.list(tool.getlinkflags(cfg))
		
		local function writevars()
			_p(1, "all_ldflags = " .. all_ldflags)
			_p(1, "libs        = " .. libs)
		end
		
		if cfg.kind == "StaticLib" then
			local ar_flags = ninja.list(tool.getarchiveflags(cfg, cfg, false))
			_p("# link static lib")
			_p("build " .. cfg:getoutputfilename() .. ": ar " .. table.concat(objfiles, " ") .. " | " .. lddeps)
			_p(1, "flags = " .. ninja.list(tool.getarchiveflags(cfg, cfg, false)))
		elseif cfg.kind == "SharedLib" then
			local output = cfg:getoutputfilename()
			_p("# link shared lib")
			_p("build " .. output .. ": link " .. table.concat(objfiles, " ") .. " | " .. libs)
			writevars()
		elseif (cfg.kind == "ConsoleApp") or (cfg.kind == "WindowedApp") then
			_p("# link executable")
			_p("build " .. cfg:getoutputfilename() .. ": link " .. table.concat(objfiles, " ") .. " | " .. lddeps)
			writevars()
		else
			p.error("ninja action doesn't support this kind of target " .. cfg.kind)
		end

	end

	

