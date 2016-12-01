--
-- Generates Ninja project file for Swift
-- Copyright (c) 2016 Stuart Carnie and the GENie project
--

local ninja = premake.ninja
local swift = {}
local p     = premake

-- generate project + config build file
	function ninja.generate_swift(prj)
		local pxy = ninja.get_proxy("prj", prj)
		local tool = premake.gettool(prj)
		
		-- build a list of supported target platforms that also includes a generic build
		local platforms = premake.filterplatforms(prj.solution, tool.platforms, "Native")

		for _, platform in ipairs(platforms) do
			for cfg in p.eachconfig(pxy, platform) do
				p.generate(cfg, cfg:getprojectfilename(), function() swift.generate_config(prj, cfg) end)
			end
		end
	end
	
	function swift.generate_config(prj, cfg)
		local tool = premake.gettool(prj)
		
		local flags = {
			swiftcflags    = ninja.list(tool.getswiftcflags(cfg)),
			swiftlinkflags = ninja.list(tool.getswiftlinkflags(cfg)),
		}
		
		_p("# Swift project build file")
		_p("# generated with GENie ninja")
		_p("")

		-- needed for implicit outputs, introduced in 1.7
		_p("ninja_required_version = 1.7")
		_p("")
		
		_p("out_dir = %s", cfg.buildtarget.directory)
		_p("obj_dir = %s", path.join(cfg.objectsdir, prj.name .. ".build"))
		_p("target = $out_dir/%s", cfg.buildtarget.name)
		_p("module_name = %s", prj.name)
		_p("module_maps = %s", ninja.list(tool.getmodulemaps(cfg)))
		_p("swiftc_flags = %s", flags.swiftcflags)
		_p("swiftlink_flags = %s", flags.swiftlinkflags)
		_p("ar_flags = %s", ninja.list(tool.getarchiveflags(cfg, cfg, false)))
		--_p("lib_dir_flags = %s", ninja.list(tool.getlibdirflags(cfg)))
		_p("ld_flags = %s", ninja.list(tool.getldflags(cfg)))
		
		if cfg.flags.Symbols then
			_p("symbols = $target.dSYM")
			symbols_command = string.format("&& %s $target -o $symbols", tool.dsymutil)
		else
			_p("symbols = ")
			symbols_command = ""
		end

		local sdk = tool.get_sdk_path(cfg)
		if sdk then
			_p("toolchain_path = %s", tool.get_toolchain_path(cfg))
			_p("sdk_path = %s", sdk)
			_p("platform_path = %s", tool.get_sdk_platform_path(cfg))
			_p("sdk = -sdk $sdk_path")
		else
			_p("sdk_path =")
			_p("sdk =")
		end
		_p("")
		
		_p("# core rules for %s", cfg.name)
		_p("rule swiftc")
		_p(1, "command = %s -frontend -c $in -enable-objc-interop $sdk -I $out_dir $swiftc_flags -module-cache-path $out_dir/ModuleCache -D SWIFT_PACKAGE $module_maps -emit-module-doc-path $out_dir/$module_name.swiftdoc -module-name $module_name -emit-module-path $out_dir/$module_name.swiftmodule -num-threads 8 $obj_files", tool.swift)
		_p(1, "description = compile $out")
		_p("")
		
		_p("rule swiftlink")
		_p(1, "command = %s $sdk -L $out_dir -o $out $swiftlink_flags $ld_flags $in %s", tool.swiftc, symbols_command)
		_p(1, "description = create executable")
		_p("")
		
		_p("rule ar")
		_p(1, "command = %s cr $ar_flags $out $in %s", tool.ar, (os.is("MacOSX") and " 2>&1 > /dev/null | sed -e '/.o) has no symbols$$/d'" or ""))
		_p(1, "description = ar $out")
		_p("")
		
		local objfiles = {}
		for _, file in ipairs(cfg.files) do
			if path.isswiftfile(file) then
				table.insert(objfiles, swift.objectname(cfg, file))
			end
		end
		
		swift.file_rules(cfg, objfiles)
		
		_p("")

		swift.linker(prj, cfg, objfiles, tool)
	end
	
	function swift.objectname(cfg, file)
		return path.join("$obj_dir", path.getname(file)..".o")
	end
	
	function swift.file_rules(cfg, objfiles)
		_p("build %s: swiftc %s", ninja.list(objfiles), ninja.list(cfg.files))
		_p(1, "obj_files = %s", ninja.arglist("-o", objfiles))
	end
	
	function swift.linker(prj, cfg, objfiles, tool)
		local lddeps = ninja.list(premake.getlinks(cfg, "siblings", "fullpath")) 
		
		if cfg.kind == "StaticLib" then
			_p("build $target: ar %s | %s ", ninja.list(objfiles), lddeps)
		else
			local lddeps = ninja.list(premake.getlinks(cfg, "siblings", "fullpath"))
			_p("build $target: swiftlink %s | %s", ninja.list(objfiles), lddeps)
		end
	end
