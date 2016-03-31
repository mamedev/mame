--
-- project.lua
-- Functions for working with the project data.
-- Copyright (c) 2002 Jason Perkins and the Premake project
--

	premake.project = { }


--
-- Create a tree from a project's list of files, representing the filesystem hierarchy.
--
-- @param prj
--    The project containing the files to map.
-- @returns
--    A new tree object containing a corresponding filesystem hierarchy. The root node
--    contains a reference back to the original project: prj = tr.project.
--

	function premake.project.buildsourcetree(prj)
		local tr = premake.tree.new(prj.name)
		tr.project = prj

		local isvpath

		local function onadd(node)
			node.isvpath = isvpath
		end

		for fcfg in premake.project.eachfile(prj) do
			isvpath = (fcfg.name ~= fcfg.vpath)
			local node = premake.tree.add(tr, fcfg.vpath, onadd)
			node.cfg = fcfg
		end

		premake.tree.sort(tr)
		return tr
	end


--
-- Returns an iterator for a set of build configuration settings. If a platform is
-- specified, settings specific to that platform and build configuration pair are
-- returned.
--

	function premake.eachconfig(prj, platform)
		-- I probably have the project root config, rather than the actual project
		if prj.project then prj = prj.project end

		local cfgs = prj.solution.configurations
		local i = 0

		return function ()
			i = i + 1
			if i <= #cfgs then
				return premake.getconfig(prj, cfgs[i], platform)
			end
		end
	end



--
-- Iterator for a project's files; returns a file configuration object.
--

	function premake.project.eachfile(prj)
		-- project root config contains the file config list
		if not prj.project then prj = premake.getconfig(prj) end
		local i = 0
		local t = prj.files
		return function ()
			i = i + 1
			if (i <= #t) then
				local fcfg = prj.__fileconfigs[t[i]]
				fcfg.vpath = premake.project.getvpath(prj, fcfg.name)
				return fcfg
			end
		end
	end



--
-- Apply XML escaping to a value.
--

	function premake.esc(value)
		if (type(value) == "table") then
			local result = { }
			for _,v in ipairs(value) do
				table.insert(result, premake.esc(v))
			end
			return result
		else
			value = value:gsub('&',  "&amp;")
			value = value:gsub('"',  "&quot;")
			value = value:gsub("'",  "&apos;")
			value = value:gsub('<',  "&lt;")
			value = value:gsub('>',  "&gt;")
			value = value:gsub('\r', "&#x0D;")
			value = value:gsub('\n', "&#x0A;")
			return value
		end
	end



--
-- Given a map of supported platform identifiers, filters the solution's list
-- of platforms to match. A map takes the form of a table like:
--
--  { x32 = "Win32", x64 = "x64" }
--
-- Only platforms that are listed in both the solution and the map will be
-- included in the results. An optional default platform may also be specified;
-- if the result set would otherwise be empty this platform will be used.
--

	function premake.filterplatforms(sln, map, default)
		local result = { }
		local keys = { }
		if sln.platforms then
			for _, p in ipairs(sln.platforms) do
				if map[p] and not table.contains(keys, map[p]) then
					table.insert(result, p)
					table.insert(keys, map[p])
				end
			end
		end

		if #result == 0 and default then
			table.insert(result, default)
		end

		return result
	end



--
-- Locate a project by name; case insensitive.
--

	function premake.findproject(name)
		for sln in premake.solution.each() do
			for prj in premake.solution.eachproject(sln) do
				if (prj.name == name) then
					return  prj
				end
			end
		end
	end



--
-- Locate a file in a project with a given extension; used to locate "special"
-- items such as Windows .def files.
--

	function premake.findfile(prj, extension)
		for _, fname in ipairs(prj.files) do
			if fname:endswith(extension) then return fname end
		end
	end



--
-- Retrieve a configuration for a given project/configuration pairing.
-- @param prj
--   The project to query.
-- @param cfgname
--   The target build configuration; only settings applicable to this configuration
--   will be returned. May be nil to retrieve project-wide settings.
-- @param pltname
--   The target platform; only settings applicable to this platform will be returned.
--   May be nil to retrieve platform-independent settings.
-- @returns
--   A configuration object containing all the settings for the given platform/build
--   configuration pair.
--

	function premake.getconfig(prj, cfgname, pltname)
		-- might have the root configuration, rather than the actual project
		prj = prj.project or prj

		-- if platform is not included in the solution, use general settings instead
		if pltname == "Native" or not table.contains(prj.solution.platforms or {}, pltname) then
			pltname = nil
		end

		local key = (cfgname or "")
		if pltname then key = key .. pltname end
		return prj.__configs[key]
	end



--
-- Build a name from a build configuration/platform pair. The short name
-- is good for makefiles or anywhere a user will have to type it in. The
-- long name is more readable.
--

	function premake.getconfigname(cfgname, platform, useshortname)
		if cfgname then
			local name = cfgname
			if platform and platform ~= "Native" then
				if useshortname then
					name = name .. premake.platforms[platform].cfgsuffix
				else
					name = name .. "|" .. platform
				end
			end
			return iif(useshortname, name:lower(), name)
		end
	end



--
-- Returns a list of sibling projects on which the specified project depends.
-- This is used to list dependencies within a solution or workspace. Must
-- consider all configurations because Visual Studio does not support per-config
-- project dependencies.
--
-- @param prj
--    The project to query.
-- @returns
--    A list of dependent projects, as an array of objects.
--

	function premake.getdependencies(prj)
		-- make sure I've got the project and not root config
		prj = prj.project or prj

		local results = { }
		for _, cfg in pairs(prj.__configs) do
			for _, link in ipairs(cfg.links) do
				local dep = premake.findproject(link)
				if dep and not table.contains(results, dep) then
					table.insert(results, dep)
				end
			end
		end

		return results
	end



--
-- Uses a pattern to format the basename of a file (i.e. without path).
--
-- @param prjname
--    A project name (string) to use.
-- @param pattern
--    A naming pattern. The sequence "%%" will be replaced by the
--    project name.
-- @returns
--    A filename (basename only) matching the specified pattern, without
--    path components.
--

	function premake.project.getbasename(prjname, pattern)
		return pattern:gsub("%%%%", prjname)
	end



--
-- Uses information from a project (or solution) to format a filename.
--
-- @param prj
--    A project or solution object with the file naming information.
-- @param pattern
--    A naming pattern. The sequence "%%" will be replaced by the
--    project name.
-- @returns
--    A filename matching the specified pattern, with a relative path
--    from the current directory to the project location.
--

	function premake.project.getfilename(prj, pattern)
		local fname = premake.project.getbasename(prj.name, pattern)
		fname = path.join(prj.location, fname)
		return path.getrelative(os.getcwd(), fname)
	end



--
-- Returns a list of link targets. Kind may be one of:
--   siblings     - linkable sibling projects
--   system       - system (non-sibling) libraries
--   dependencies - all sibling dependencies, including non-linkable
--   all          - return everything
--
-- Part may be one of:
--   name      - the decorated library name with no directory
--   basename  - the undecorated library name
--   directory - just the directory, no name
--   fullpath  - full path with decorated name
--   object    - return the project object of the dependency
--

 	function premake.getlinks(cfg, kind, part)
		-- if I'm building a list of link directories, include libdirs
		local result = iif (part == "directory" and kind == "all", cfg.libdirs, {})

		-- am I getting links for a configuration or a project?
		local cfgname = iif(cfg.name == cfg.project.name, "", cfg.name)

		-- how should files be named?
		local pathstyle = premake.getpathstyle(cfg)
		local namestyle = premake.getnamestyle(cfg)

		local function canlink(source, target)
			if (target.kind ~= "SharedLib" and target.kind ~= "StaticLib") then
				return false
			end
			if premake.iscppproject(source) then
				return premake.iscppproject(target)
			elseif premake.isdotnetproject(source) then
				return premake.isdotnetproject(target)
			end
		end

		for _, link in ipairs(cfg.links) do
			local item

			-- is this a sibling project?
			local prj = premake.findproject(link)
			if prj and kind ~= "system" then

				local prjcfg = premake.getconfig(prj, cfgname, cfg.platform)
				if kind == "dependencies" or canlink(cfg, prjcfg) then
					if (part == "directory") then
						item = path.rebase(prjcfg.linktarget.directory, prjcfg.location, cfg.location)
					elseif (part == "basename") then
						item = prjcfg.linktarget.basename
					elseif (part == "fullpath") then
						item = path.rebase(prjcfg.linktarget.fullpath, prjcfg.location, cfg.location)
					elseif (part == "object") then
						item = prjcfg
					end
				end

			elseif not prj and (kind == "system" or kind == "all") then

				if (part == "directory") then
					item = path.getdirectory(link)
				elseif (part == "fullpath") then
					item = link
					if namestyle == "windows" then
						if premake.iscppproject(cfg) then
							item = item .. ".lib"
						elseif premake.isdotnetproject(cfg) then
							item = item .. ".dll"
						end
					end
				elseif part == "name" then
					item = path.getname(link)
				elseif part == "basename" then
					item = path.getbasename(link)
				else
					item = link
				end

				if item:find("/", nil, true) then
					item = path.getrelative(cfg.project.location, item)
				end

			end

			if item then
				if pathstyle == "windows" and part ~= "object" then
					item = path.translate(item, "\\")
				end
				if not table.contains(result, item) then
					table.insert(result, item)
				end
			end
		end

		return result
	end



--
-- Gets the name style for a configuration, indicating what kind of prefix,
-- extensions, etc. should be used in target file names.
--
-- @param cfg
--    The configuration to check.
-- @returns
--    The target naming style, one of "windows", "posix", or "PS3".
--

	function premake.getnamestyle(cfg)
		return premake.platforms[cfg.platform].namestyle or premake.gettool(cfg).namestyle or "posix"
	end



--
-- Gets the path style for a configuration, indicating what kind of path separator
-- should be used in target file names.
--
-- @param cfg
--    The configuration to check.
-- @returns
--    The target path style, one of "windows" or "posix".
--

	function premake.getpathstyle(cfg)
		if premake.action.current().os == "windows" then
			return "windows"
		else
			return "posix"
		end
	end


--
-- Assembles a target for a particular tool/system/configuration.
--
-- @param cfg
--    The configuration to be targeted.
-- @param direction
--    One of 'build' for the build target, or 'link' for the linking target.
-- @param pathstyle
--    The path format, one of "windows" or "posix". This comes from the current
--    action: Visual Studio uses "windows", GMake uses "posix", etc.
-- @param namestyle
--    The file naming style, one of "windows" or "posix". This comes from the
--    current tool: GCC uses "posix", MSC uses "windows", etc.
-- @param system
--    The target operating system, which can modify the naming style. For example,
--    shared libraries on Mac OS X use a ".dylib" extension.
-- @returns
--    An object with these fields:
--      basename   - the target with no directory or file extension
--      name       - the target name and extension, with no directory
--      directory  - relative path to the target, with no file name
--      prefix     - the file name prefix
--      suffix     - the file name suffix
--      fullpath   - directory, name, and extension
--      bundlepath - the relative path and file name of the bundle
--

	function premake.gettarget(cfg, direction, pathstyle, namestyle, system)
		if system == "bsd" then
			system = "linux"
		end

		-- Fix things up based on the current system
		local kind = cfg.kind
		if premake.iscppproject(cfg) then
			-- On Windows, shared libraries link against a static import library
			if (namestyle == "windows" or system == "windows")
				and kind == "SharedLib" and direction == "link"
				and not cfg.flags.NoImportLib
			then
				kind = "StaticLib"
			end

			-- Posix name conventions only apply to static libs on windows (by user request)
			if namestyle == "posix" and system == "windows" and kind ~= "StaticLib" then
				namestyle = "windows"
			end
		end

		-- Initialize the target components
		local field   = "build"
		if direction == "link" and cfg.kind == "SharedLib" then
			field = "implib"
		end

		local name    = cfg[field.."name"] or cfg.targetname or cfg.project.name
		local dir     = cfg[field.."dir"] or cfg.targetdir or path.getrelative(cfg.location, cfg.basedir)
		local subdir  = cfg[field.."subdir"] or cfg.targetsubdir or "."
		local prefix  = ""
		local suffix  = ""
		local ext     = ""
		local bundlepath, bundlename

		-- targetpath/targetsubdir/bundlepath/prefix..name..suffix..ext

		dir = path.join(dir, subdir)


		if namestyle == "windows" then
			if kind == "ConsoleApp" or kind == "WindowedApp" then
				ext = ".exe"
			elseif kind == "SharedLib" then
				ext = ".dll"
			elseif kind == "StaticLib" then
				ext = ".lib"
			end
		elseif namestyle == "posix" then
			if kind == "WindowedApp" and system == "macosx" then
				bundlename = name .. ".app"
				bundlepath = path.join(dir, bundlename)
				dir = path.join(bundlepath, "Contents/MacOS")
			elseif (kind == "ConsoleApp" or kind == "WindowedApp") and system == "os2" then
				ext = ".exe"
			elseif kind == "SharedLib" then
				prefix = "lib"
				ext = iif(system == "macosx", ".dylib", ".so")
			elseif kind == "StaticLib" then
				prefix = "lib"
				ext = ".a"
			end
		elseif namestyle == "PS3" then
			if kind == "ConsoleApp" or kind == "WindowedApp" then
				ext = ".elf"
			elseif kind == "StaticLib" then
				prefix = "lib"
				ext = ".a"
			end
		elseif namestyle == "Orbis" then
			if kind == "ConsoleApp" or kind == "WindowedApp" then
				ext = ".elf"
			elseif kind == "StaticLib" then
				prefix = "lib"
				ext = ".a"
			elseif kind == "SharedLib" then
				ext = ".prx"
			end
		end

		prefix = cfg[field.."prefix"] or cfg.targetprefix or prefix
		suffix = cfg[field.."suffix"] or cfg.targetsuffix or suffix
		ext    = cfg[field.."extension"] or cfg.targetextension or ext

		-- build the results object
		local result = { }
		result.basename     = name .. suffix
		result.name         = prefix .. name .. suffix .. ext
		result.directory    = dir
		result.subdirectory = subdir
		result.prefix       = prefix
		result.suffix       = suffix
		result.fullpath     = path.join(result.directory, result.name)
		result.bundlepath   = bundlepath or result.fullpath

		if pathstyle == "windows" then
			result.directory    = path.translate(result.directory, "\\")
			result.subdirectory = path.translate(result.subdirectory, "\\")
			result.fullpath     = path.translate(result.fullpath,  "\\")
		end

		return result
	end


--
-- Return the appropriate tool interface, based on the target language and
-- any relevant command-line options.
--

	function premake.gettool(cfg)
		if premake.iscppproject(cfg) then
			if _OPTIONS.cc then
				return premake[_OPTIONS.cc]
			end
			local action = premake.action.current()
			if action.valid_tools then
				return premake[action.valid_tools.cc[1]]
			end
			return premake.gcc
		else
			return premake.dotnet
		end
	end



--
-- Given a source file path, return a corresponding virtual path based on
-- the vpath entries in the project. If no matching vpath entry is found,
-- the original path is returned.
--

	function premake.project.getvpath(prj, abspath)
		-- If there is no match, the result is the original filename
		local vpath = abspath

		-- The file's name must be maintained in the resulting path; use these
		-- to make sure I don't cut off too much

		local fname = path.getname(abspath)
		local max = abspath:len() - fname:len()

		-- Look for matching patterns
		for replacement, patterns in pairs(prj.vpaths or {}) do
			for _, pattern in ipairs(patterns) do
				local i = abspath:find(path.wildcards(pattern))
				if i == 1 then

					-- Trim out the part of the name that matched the pattern; what's
					-- left is the part that gets appended to the replacement to make
					-- the virtual path. So a pattern like "src/**.h" matching the
					-- file src/include/hello.h, I want to trim out the src/ part,
					-- leaving include/hello.h.

					-- Find out where the wildcard appears in the match. If there is
					-- no wildcard, the match includes the entire pattern

					i = pattern:find("*", 1, true) or (pattern:len() + 1)

					-- Trim, taking care to keep the actual file name intact.

					local leaf
					if i < max then
						leaf = abspath:sub(i)
					else
						leaf = fname
					end

					if leaf:startswith("/") then
						leaf = leaf:sub(2)
					end

					-- check for (and remove) stars in the replacement pattern.
					-- If there are none, then trim all path info from the leaf
					-- and use just the filename in the replacement (stars should
					-- really only appear at the end; I'm cheating here)

					local stem = ""
					if replacement:len() > 0 then
						stem, stars = replacement:gsub("%*", "")
						if stars == 0 then
							leaf = path.getname(leaf)
						end
					else
						leaf = path.getname(leaf)
					end

					vpath = path.join(stem, leaf)

				end
			end
		end

		return path.trimdots(vpath)
	end


--
-- Returns true if the solution contains at least one C/C++ project.
--

	function premake.hascppproject(sln)
		for prj in premake.solution.eachproject(sln) do
			if premake.iscppproject(prj) then
				return true
			end
		end
	end



--
-- Returns true if the solution contains at least one .NET project.
--

	function premake.hasdotnetproject(sln)
		for prj in premake.solution.eachproject(sln) do
			if premake.isdotnetproject(prj) then
				return true
			end
		end
	end



--
-- Returns true if the project use the C language.
--

	function premake.project.iscproject(prj)
		return prj.language == "C"
	end


--
-- Returns true if the project uses a C/C++ language.
--

	function premake.iscppproject(prj)
		return (prj.language == "C" or prj.language == "C++")
	end



--
-- Returns true if the project uses a .NET language.
--

	function premake.isdotnetproject(prj)
		return (prj.language == "C#")
	end
