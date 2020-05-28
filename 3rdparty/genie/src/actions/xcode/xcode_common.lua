--
-- xcode_common.lua
-- Functions to generate the different sections of an Xcode project.
-- Copyright (c) 2009-2011 Jason Perkins and the Premake project
--
	premake.xcode.parameters = { }
	local xcode = premake.xcode
	local tree  = premake.tree
--
-- Return the Xcode build category for a given file, based on the file extension.
--
-- @param node
--    The node to identify.
-- @returns
--    An Xcode build category, one of "Sources", "Resources", "Frameworks", or nil.
--

	function xcode.getbuildcategory(node)
		local categories = {
			[".a"] = "Frameworks",
			[".h"] = "Headers",
			[".hh"] = "Headers",
			[".hpp"] = "Headers",
			[".hxx"] = "Headers",
			[".inl"] = "Headers",
			[".c"] = "Sources",
			[".cc"] = "Sources",
			[".cpp"] = "Sources",
			[".cxx"] = "Sources",
			[".c++"] = "Sources",
			[".dylib"] = "Frameworks",
			[".bundle"] = "Frameworks",
			[".framework"] = "Frameworks",
			[".tbd"] = "Frameworks",
			[".m"] = "Sources",
			[".mm"] = "Sources",
			[".S"] = "Sources",
			[".strings"] = "Resources",
			[".nib"] = "Resources",
			[".xib"] = "Resources",
			[".icns"] = "Resources",
			[".bmp"] = "Resources",
			[".wav"] = "Resources",
			[".xcassets"]  = "Resources",
			[".xcdatamodeld"] = "Sources",
			[".swift"] = "Sources",
		}
		return categories[path.getextension(node.name)] or
			categories[string.lower(path.getextension(node.name))]
	end


--
-- Return the displayed name for a build configuration, taking into account the
-- configuration and platform, i.e. "Debug 32-bit Universal".
--
-- @param cfg
--    The configuration being identified.
-- @returns
--    A build configuration name.
--

	function xcode.getconfigname(cfg)
		local name = cfg.name
		if #cfg.project.solution.xcode.platforms > 1 then
			name = name .. " " .. premake.action.current().valid_platforms[cfg.platform]
		end
		return name
	end


--
-- Return the Xcode type for a given file, based on the file extension.
--
-- @param fname
--    The file name to identify.
-- @returns
--    An Xcode file type, string.
--

	function xcode.getfiletype(node)
		local types = {
			[".c"]         = "sourcecode.c.c",
			[".cc"]        = "sourcecode.cpp.cpp",
			[".cpp"]       = "sourcecode.cpp.cpp",
			[".css"]       = "text.css",
			[".cxx"]       = "sourcecode.cpp.cpp",
			[".c++"]       = "sourcecode.cpp.cpp",
			[".entitlements"] = "text.xml",
			[".bundle"]    = "wrapper.cfbundle",
			[".framework"] = "wrapper.framework",
			[".tbd"]       = "sourcecode.text-based-dylib-definition",
			[".gif"]       = "image.gif",
			[".h"]         = "sourcecode.c.h",
			[".hh"]        = "sourcecode.cpp.h",
			[".hpp"]       = "sourcecode.cpp.h",
			[".hxx"]       = "sourcecode.cpp.h",
			[".inl"]       = "sourcecode.cpp.h",
			[".html"]      = "text.html",
			[".lua"]       = "sourcecode.lua",
			[".m"]         = "sourcecode.c.objc",
			[".mm"]        = "sourcecode.cpp.objcpp",
			[".S"]         = "sourcecode.asm",
			[".nib"]       = "wrapper.nib",
			[".pch"]       = "sourcecode.c.h",
			[".plist"]     = "text.plist.xml",
			[".strings"]   = "text.plist.strings",
			[".xib"]       = "file.xib",
			[".icns"]      = "image.icns",
			[".bmp"]       = "image.bmp",
			[".wav"]       = "audio.wav",
			[".xcassets"]  = "folder.assetcatalog",
			[".xcdatamodeld"] = "wrapper.xcdatamodeld",
			[".swift"]     = "sourcecode.swift",
		}
		return types[path.getextension(node.path)] or
			(types[string.lower(path.getextension(node.path))] or "text")
	end

--
-- Return the Xcode type for a given file, based on the file extension.
--
-- @param fname
--    The file name to identify.
-- @returns
--    An Xcode file type, string.
--

	function xcode.getfiletypeForced(node)
		local types = {
			[".c"]         = "sourcecode.cpp.cpp",
			[".cc"]        = "sourcecode.cpp.cpp",
			[".cpp"]       = "sourcecode.cpp.cpp",
			[".css"]       = "text.css",
			[".cxx"]       = "sourcecode.cpp.cpp",
			[".c++"]       = "sourcecode.cpp.cpp",
			[".entitlements"] = "text.xml",
			[".bundle"]    = "wrapper.cfbundle",
			[".framework"] = "wrapper.framework",
			[".tbd"]       = "wrapper.framework",
			[".gif"]       = "image.gif",
			[".h"]         = "sourcecode.cpp.h",
			[".hh"]        = "sourcecode.cpp.h",
			[".hpp"]       = "sourcecode.cpp.h",
			[".hxx"]       = "sourcecode.cpp.h",
			[".inl"]       = "sourcecode.cpp.h",
			[".html"]      = "text.html",
			[".lua"]       = "sourcecode.lua",
			[".m"]         = "sourcecode.cpp.objcpp",
			[".mm"]        = "sourcecode.cpp.objcpp",
			[".nib"]       = "wrapper.nib",
			[".pch"]       = "sourcecode.cpp.h",
			[".plist"]     = "text.plist.xml",
			[".strings"]   = "text.plist.strings",
			[".xib"]       = "file.xib",
			[".icns"]      = "image.icns",
			[".bmp"]       = "image.bmp",
			[".wav"]       = "audio.wav",
			[".xcassets"]  = "folder.assetcatalog",
			[".xcdatamodeld"] = "wrapper.xcdatamodeld",
			[".swift"]     = "sourcecode.swift",
		}
		return types[path.getextension(node.path)] or
			(types[string.lower(path.getextension(node.path))] or "text")
	end
--
-- Return the Xcode product type, based target kind.
--
-- @param node
--    The product node to identify.
-- @returns
--    An Xcode product type, string.
--

	function xcode.getproducttype(node)
		local types = {
			ConsoleApp  = "com.apple.product-type.tool",
			WindowedApp = node.cfg.options.SkipBundling and "com.apple.product-type.tool" or "com.apple.product-type.application",
			StaticLib   = "com.apple.product-type.library.static",
			SharedLib   = "com.apple.product-type.library.dynamic",
			Bundle      = node.cfg.options.SkipBundling and "com.apple.product-type.tool" or "com.apple.product-type.bundle",
		}
		return types[node.cfg.kind]
	end


--
-- Return the Xcode target type, based on the target file extension.
--
-- @param node
--    The product node to identify.
-- @returns
--    An Xcode target type, string.
--

	function xcode.gettargettype(node)
		local types = {
			ConsoleApp  = "\"compiled.mach-o.executable\"",
			WindowedApp = node.cfg.options.SkipBundling and "\"compiled.mach-o.executable\"" or "wrapper.application",
			StaticLib   = "archive.ar",
			SharedLib   = "\"compiled.mach-o.dylib\"",
			Bundle      = node.cfg.options.SkipBundling and "\"compiled.mach-o.bundle\"" or "wrapper.cfbundle",
		}
		return types[node.cfg.kind]
	end


--
-- Return a unique file name for a project. Since Xcode uses .xcodeproj's to
-- represent both solutions and projects there is a likely change of a name
-- collision. Tack on a number to differentiate them.
--
-- @param prj
--    The project being queried.
-- @returns
--    A uniqued file name
--

	function xcode.getxcodeprojname(prj)
		-- if there is a solution with matching name, then use "projectname1.xcodeproj"
		-- just get something working for now
		local fname = premake.project.getfilename(prj, "%%.xcodeproj")
		return fname
	end


--
-- Returns true if the file name represents a framework.
--
-- @param fname
--    The name of the file to test.
--

	function xcode.isframework(fname)
		return (path.getextension(fname) == ".framework" or path.getextension(fname) == ".tbd")
	end

--
-- Generates a unique 12 byte ID.
-- Parameter is optional
--
-- @returns
--    A 24-character string representing the 12 byte ID.
--
	function xcode.uuid(param)
		return os.uuid(param):upper():gsub('-',''):sub(0,24)
	end

--
-- Retrieves a unique 12 byte ID for an object. This function accepts and ignores two
-- parameters 'node' and 'usage', which are used by an alternative implementation of
-- this function for testing.
--
-- @returns
--    A 24-character string representing the 12 byte ID.
--

	function xcode.newid(node, usage)
		local base = ''

		-- Seed the uuid with the project name and a project-specific counter.
		-- This is to prevent matching strings from generating the same uuid,
		-- while still generating the same uuid for the same properties across
		-- runs.
		local prj = node.project

		if prj == nil then
			local parent = node.parent
			while parent ~= nil do
				if parent.project ~= nil then
					prj = parent.project
					break
				end
				parent = parent.parent
			end
		end

		if prj ~= nil then
			prj.uuidcounter = (prj.uuidcounter or 0) + 1
			base = base .. prj.name .. "$" .. prj.uuidcounter .. "$"
		end

		base = base .. "$" .. (node.path or node.name or "")
		base = base .. "$" .. (usage or "")
		return xcode.uuid(base)
	end

--
-- Creates a label for a given scriptphase
--  based on command and config
-- such as the result looks like this:
-- 'Script Phase <number> [cmd] (Config)', e.g. 'Script Phase 1 [rsync] (Debug)'
--
-- This function is used for generating `PBXShellScriptBuildPhase` from `xcodescriptphases`.
-- (Thus required in more than 1 place).
--
-- @param cmd
--    The command itself
-- @param count
--    counter to avoid having duplicate label names
-- @param cfg
--    The configuration the command is generated for
--

	function xcode.getscriptphaselabel(cmd, count, cfg)
		return string.format("\"Script Phase %s [%s] (%s)\"", count, cmd:match("(%w+)(.+)"), iif(cfg, xcode.getconfigname(cfg), "all"))
	end


--
-- Creates a label for a given copy phase
--  based on target
-- such as the result looks like this:
-- 'Copy <type> <number> [target]', e.g. 'Copy Files 1 [assets]'
--
-- This function is used for generating `PBXCopyFilesPhase` from `xcodecopyresources`.
-- (Thus required in more than 1 place).
--
-- @param type
--    The copy type ('Resources' for now)
-- @param count
--    counter to avoid having duplicate label names
-- @param target
--    The target subfolder
--

	function xcode.getcopyphaselabel(type, count, target)
		return string.format("\"Copy %s %s [%s]\"", type, count, target)
	end


--
-- Create a product tree node and all projects in a solution; assigning IDs
-- that are needed for inter-project dependencies.
--
-- @param sln
--    The solution to prepare.
--

	function xcode.preparesolution(sln)
		-- create and cache a list of supported platforms
		sln.xcode = { }
		sln.xcode.platforms = premake.filterplatforms(sln, premake.action.current().valid_platforms, "Universal")

		for prj in premake.solution.eachproject(sln) do
			-- need a configuration to get the target information
			local cfg = premake.getconfig(prj, prj.configurations[1], sln.xcode.platforms[1])

			-- build the product tree node
			local node = premake.tree.new(path.getname(cfg.buildtarget.bundlepath))
			node.cfg = cfg
			node.id = premake.xcode.newid(node, "product")
			node.targetid = premake.xcode.newid(node, "target")

			-- attach it to the project
			prj.xcode = {}
			prj.xcode.projectnode = node
		end
	end


--
-- Print out a list value in the Xcode format.
--
-- @param list
--    The list of values to be printed.
-- @param tag
--    The Xcode specific list tag.
--

	function xcode.printlist(list, tag, sort)
		if #list > 0 then
			if sort ~= nil and sort == true then
				table.sort(list)
			end
			_p(4,'%s = (', tag)
			for _, item in ipairs(list) do
				local escaped_item = item:gsub("\"", "\\\\\\\""):gsub("'", "\\\\'")
				_p(5, '"%s",', escaped_item)
			end
			_p(4,');')
		end
	end


--
-- Escape a string for use in an Xcode project file.
--

	function xcode.quotestr(str)
		-- simple strings don't need quotes
		if str:match("[^a-zA-Z0-9$._/]") == nil then
			return str
		end

		return "\"" .. str:gsub("[\"\\\"]", "\\%0") .. "\""
	end



---------------------------------------------------------------------------
-- Section generator functions, in the same order in which they appear
-- in the .pbxproj file
---------------------------------------------------------------------------

function xcode.Header(tr, objversion)
	_p('// !$*UTF8*$!')
	_p('{')
	_p(1,'archiveVersion = 1;')
	_p(1,'classes = {')
	_p(1,'};')
	_p(1,'objectVersion = %d;', objversion)
	_p(1,'objects = {')
	_p('')
end


	function xcode.PBXBuildFile(tr)
		local function gatherCopyFiles(which)
			local copyfiles = {}
			local targets = tr.project[which]
			if #targets > 0 then
				for _, t in ipairs(targets) do
					for __, tt in ipairs(t) do
						table.insertflat(copyfiles, tt[2])
					end
				end
			end
			return table.translate(copyfiles, path.getname)
		end

		local function gatherCopyFrameworks(which)
			local copyfiles = {}
			local targets = tr.project[which]
			if #targets > 0 then
				table.insertflat(copyfiles, targets)
			end
			return table.translate(copyfiles, path.getname)
		end

		local copyfiles = table.flatten({
			gatherCopyFiles('xcodecopyresources'),
			gatherCopyFrameworks('xcodecopyframeworks')
		})

		_p('/* Begin PBXBuildFile section */')
		tree.traverse(tr, {
			onnode = function(node)
				if node.buildid then
					_p(2,'%s /* %s in %s */ = {isa = PBXBuildFile; fileRef = %s /* %s */; };',
						node.buildid, node.name, xcode.getbuildcategory(node), node.id, node.name)
				end

				-- adds duplicate PBXBuildFile file entry as 'CopyFiles' for files marked to be copied
				-- for frameworks: add signOnCopy flag
				if table.icontains(copyfiles, node.name) then
					_p(2,'%s /* %s in %s */ = {isa = PBXBuildFile; fileRef = %s /* %s */; %s };',
						xcode.uuid(node.name .. 'in CopyFiles'), node.name, 'CopyFiles', node.id, node.name,
						iif(xcode.isframework(node.name), "settings = {ATTRIBUTES = (CodeSignOnCopy, ); };", "")
					)
				end
			end
		})
		_p('/* End PBXBuildFile section */')
		_p('')
	end


	function xcode.PBXContainerItemProxy(tr)
		if #tr.projects.children > 0 then
			_p('/* Begin PBXContainerItemProxy section */')
			for _, node in ipairs(tr.projects.children) do
				_p(2,'%s /* PBXContainerItemProxy */ = {', node.productproxyid)
				_p(3,'isa = PBXContainerItemProxy;')
				_p(3,'containerPortal = %s /* %s */;', node.id, path.getname(node.path))
				_p(3,'proxyType = 2;')
				_p(3,'remoteGlobalIDString = %s;', node.project.xcode.projectnode.id)
				_p(3,'remoteInfo = "%s";', node.project.xcode.projectnode.name)
				_p(2,'};')
				_p(2,'%s /* PBXContainerItemProxy */ = {', node.targetproxyid)
				_p(3,'isa = PBXContainerItemProxy;')
				_p(3,'containerPortal = %s /* %s */;', node.id, path.getname(node.path))
				_p(3,'proxyType = 1;')
				_p(3,'remoteGlobalIDString = %s;', node.project.xcode.projectnode.targetid)
				_p(3,'remoteInfo = "%s";', node.project.xcode.projectnode.name)
				_p(2,'};')
			end
			_p('/* End PBXContainerItemProxy section */')
			_p('')
		end
	end


	function xcode.PBXFileReference(tr,prj)
		_p('/* Begin PBXFileReference section */')

		tree.traverse(tr, {
			onleaf = function(node)
				-- I'm only listing files here, so ignore anything without a path
				if not node.path then
					return
				end

				-- is this the product node, describing the output target?
				if node.kind == "product" then
					_p(2,'%s /* %s */ = {isa = PBXFileReference; explicitFileType = %s; includeInIndex = 0; name = "%s"; path = "%s"; sourceTree = BUILT_PRODUCTS_DIR; };',
						node.id, node.name, xcode.gettargettype(node), node.name, path.getname(node.cfg.buildtarget.bundlepath))

				-- is this a project dependency?
				elseif node.parent.parent == tr.projects then
					local relpath = path.getrelative(tr.project.location, node.parent.project.location)
					_p(2,'%s /* %s */ = {isa = PBXFileReference; lastKnownFileType = "wrapper.pb-project"; name = "%s"; path = "%s"; sourceTree = SOURCE_ROOT; };',
						node.parent.id, node.parent.name, node.parent.name, path.join(relpath, node.parent.name))

				-- something else
				else
					local pth, src
					if xcode.isframework(node.path) then
						--respect user supplied paths
						-- look for special variable-starting paths for different sources
						local nodePath = node.path
						local _, matchEnd, variable = string.find(nodePath, "^%$%((.+)%)/")
						if variable then
							-- by skipping the last '/' we support the same absolute/relative
							-- paths as before
							nodePath = string.sub(nodePath, matchEnd + 1)
						end
						if string.find(nodePath,'/')  then
							if string.find(nodePath,'^%.')then
								--error('relative paths are not currently supported for frameworks')
								nodePath = path.getabsolute(path.join(tr.project.location, nodePath))
							end
							pth = nodePath
						elseif path.getextension(nodePath)=='.tbd' then
							pth = "/usr/lib/" .. nodePath
						else
							pth = "/System/Library/Frameworks/" .. nodePath
						end
						-- if it starts with a variable, use that as the src instead
						if variable then
							src = variable
							-- if we are using a different source tree, it has to be relative
							-- to that source tree, so get rid of any leading '/'
							if string.find(pth, '^/') then
								pth = string.sub(pth, 2)
							end
						else
							src = "<absolute>"
						end
					else
						-- something else; probably a source code file
						src = "<group>"

						if node.location then
							pth = node.location
						elseif node.parent.isvpath then
							-- if the parent node is virtual, it won't have a local path
							-- of its own; need to use full relative path from project
							-- (in fact, often almost all paths are virtual because vpath
							-- trims the leading dots from the full path)
							pth = node.cfg.name
						else
							pth = tree.getlocalpath(node)
						end
					end

					if (not prj.options.ForceCPP) then
						_p(2,'%s /* %s */ = {isa = PBXFileReference; lastKnownFileType = %s; name = "%s"; path = "%s"; sourceTree = "%s"; };',
							node.id, node.name, xcode.getfiletype(node), node.name, pth, src)
					else
						_p(2,'%s /* %s */ = {isa = PBXFileReference; explicitFileType = %s; name = "%s"; path = "%s"; sourceTree = "%s"; };',
							node.id, node.name, xcode.getfiletypeForced(node), node.name, pth, src)
					end
				end
			end
		})

		_p('/* End PBXFileReference section */')
		_p('')
	end


	function xcode.PBXFrameworksBuildPhase(tr)
		_p('/* Begin PBXFrameworksBuildPhase section */')
		_p(2,'%s /* Frameworks */ = {', tr.products.children[1].fxstageid)
		_p(3,'isa = PBXFrameworksBuildPhase;')
		_p(3,'buildActionMask = 2147483647;')
		_p(3,'files = (')

		-- write out library dependencies
		tree.traverse(tr.frameworks, {
			onleaf = function(node)
				_p(4,'%s /* %s in Frameworks */,', node.buildid, node.name)
			end
		})

		-- write out project dependencies
		tree.traverse(tr.projects, {
			onleaf = function(node)
				_p(4,'%s /* %s in Frameworks */,', node.buildid, node.name)
			end
		})

		_p(3,');')
		_p(3,'runOnlyForDeploymentPostprocessing = 0;')
		_p(2,'};')
		_p('/* End PBXFrameworksBuildPhase section */')
		_p('')
	end


	function xcode.PBXGroup(tr)
		_p('/* Begin PBXGroup section */')

		tree.traverse(tr, {
			onnode = function(node)
				-- Skip over anything that isn't a proper group
				if (node.path and #node.children == 0) or node.kind == "vgroup" then
					return
				end

				-- project references get special treatment
				if node.parent == tr.projects then
					_p(2,'%s /* Products */ = {', node.productgroupid)
				else
					_p(2,'%s /* %s */ = {', node.id, node.name)
				end

				_p(3,'isa = PBXGroup;')
				_p(3,'children = (')
				for _, childnode in ipairs(node.children) do
					_p(4,'%s /* %s */,', childnode.id, childnode.name)
				end
				_p(3,');')

				if node.parent == tr.projects then
					_p(3,'name = Products;')
				else
					_p(3,'name = "%s";', node.name)

					if node.location then
						_p(3,'path = "%s";', node.location)
					elseif node.path and not node.isvpath then
						local p = node.path
						if node.parent.path then
							p = path.getrelative(node.parent.path, node.path)
						end
						_p(3,'path = "%s";', p)
					end
				end

				_p(3,'sourceTree = "<group>";')
				_p(2,'};')
			end
		}, true)

		_p('/* End PBXGroup section */')
		_p('')
	end


	function xcode.PBXNativeTarget(tr)
		_p('/* Begin PBXNativeTarget section */')
		for _, node in ipairs(tr.products.children) do
			local name = tr.project.name

			-- This function checks whether there are build commands of a specific
			-- type to be executed; they will be generated correctly, but the project
			-- commands will not contain any per-configuration commands, so the logic
			-- has to be extended a bit to account for that.
			local function hasBuildCommands(which)
				-- standard check...this is what existed before
				if #tr.project[which] > 0 then
					return true
				end
				-- what if there are no project-level commands? check configs...
				for _, cfg in ipairs(tr.configs) do
					if #cfg[which] > 0 then
						return true
					end
				end
			end

			local function dobuildblock(id, label, which, action)
				if hasBuildCommands(which) then
					local commandcount = 0
					for _, cfg in ipairs(tr.configs) do
						commandcount = commandcount + #cfg[which]
					end
					if commandcount > 0 then
						action(id, label)
					end
				end
			end

			local function doscriptphases(which, action)
				local i = 0
				for _, cfg in ipairs(tr.configs) do
					local cfgcmds = cfg[which]
					if cfgcmds ~= nil then
						for __, scripts in ipairs(cfgcmds) do
							for ___, script in ipairs(scripts) do
								local cmd = script[1]
								local label = xcode.getscriptphaselabel(cmd, i, cfg)
								local id = xcode.uuid(label)
								action(id, label)
								i = i + 1
							end
						end
					end
				end
			end

			local function docopyresources(which, action)
				if hasBuildCommands(which) then
					local targets = tr.project[which]
					if #targets > 0 then
						local i = 0
						for _, t in ipairs(targets) do
							for __, tt in ipairs(t) do
								local label = xcode.getcopyphaselabel('Resources', i, tt[1])
								local id = xcode.uuid(label)
								action(id, label)
								i = i + 1
							end
						end
					end
				end
			end

			local function docopyframeworks(which, action)
				if hasBuildCommands(which) then
					local targets = tr.project[which]
					if #targets > 0 then
						local label = "Copy Frameworks"
						local id = xcode.uuid(label)
						action(id, label)
					end
				end
			end

			local function _p_label(id, label)
				_p(4, '%s /* %s */,', id, label)
			end


			_p(2,'%s /* %s */ = {', node.targetid, name)
			_p(3,'isa = PBXNativeTarget;')
			_p(3,'buildConfigurationList = %s /* Build configuration list for PBXNativeTarget "%s" */;', node.cfgsection, name)
			_p(3,'buildPhases = (')
			dobuildblock('9607AE1010C857E500CD1376', 'Prebuild', 'prebuildcommands', _p_label)
			_p(4,'%s /* Resources */,', node.resstageid)
			_p(4,'%s /* Sources */,', node.sourcesid)
			dobuildblock('9607AE3510C85E7E00CD1376', 'Prelink', 'prelinkcommands', _p_label)
			_p(4,'%s /* Frameworks */,', node.fxstageid)
			dobuildblock('9607AE3710C85E8F00CD1376', 'Postbuild', 'postbuildcommands', _p_label)
			doscriptphases("xcodescriptphases", _p_label)
			docopyresources("xcodecopyresources", _p_label)
			if tr.project.kind == "WindowedApp" then
				docopyframeworks("xcodecopyframeworks", _p_label)
			end

			_p(3,');')
			_p(3,'buildRules = (')
			_p(3,');')

			_p(3,'dependencies = (')
			for _, node in ipairs(tr.projects.children) do
				_p(4,'%s /* PBXTargetDependency */,', node.targetdependid)
			end
			_p(3,');')

			_p(3,'name = "%s";', name)

			local p
			if node.cfg.kind == "ConsoleApp" then
				p = "$(HOME)/bin"
			elseif node.cfg.kind == "WindowedApp" then
				p = "$(HOME)/Applications"
			end
			if p then
				_p(3,'productInstallPath = "%s";', p)
			end

			_p(3,'productName = "%s";', name)
			_p(3,'productReference = %s /* %s */;', node.id, node.name)
			_p(3,'productType = "%s";', xcode.getproducttype(node))
			_p(2,'};')
		end
		_p('/* End PBXNativeTarget section */')
		_p('')
	end


	function xcode.PBXProject(tr, compatVersion)
		_p('/* Begin PBXProject section */')
		_p(2,'__RootObject_ /* Project object */ = {')
		_p(3,'isa = PBXProject;')
		_p(3,'buildConfigurationList = 1DEB928908733DD80010E9CD /* Build configuration list for PBXProject "%s" */;', tr.name)
		_p(3,'compatibilityVersion = "Xcode %s";', compatVersion)
		_p(3,'hasScannedForEncodings = 1;')
		_p(3,'mainGroup = %s /* %s */;', tr.id, tr.name)
		_p(3,'projectDirPath = "";')

		if #tr.projects.children > 0 then
			_p(3,'projectReferences = (')
			for _, node in ipairs(tr.projects.children) do
				_p(4,'{')
				_p(5,'ProductGroup = %s /* Products */;', node.productgroupid)
				_p(5,'ProjectRef = %s /* %s */;', node.id, path.getname(node.path))
				_p(4,'},')
			end
			_p(3,');')
		end

		_p(3,'projectRoot = "";')
		_p(3,'targets = (')
		for _, node in ipairs(tr.products.children) do
			_p(4,'%s /* %s */,', node.targetid, node.name)
		end
		_p(3,');')
		_p(2,'};')
		_p('/* End PBXProject section */')
		_p('')
	end


	function xcode.PBXReferenceProxy(tr)
		if #tr.projects.children > 0 then
			_p('/* Begin PBXReferenceProxy section */')
			tree.traverse(tr.projects, {
				onleaf = function(node)
					_p(2,'%s /* %s */ = {', node.id, node.name)
					_p(3,'isa = PBXReferenceProxy;')
					_p(3,'fileType = %s;', xcode.gettargettype(node))
					_p(3,'path = "%s";', node.path)
					_p(3,'remoteRef = %s /* PBXContainerItemProxy */;', node.parent.productproxyid)
					_p(3,'sourceTree = BUILT_PRODUCTS_DIR;')
					_p(2,'};')
				end
			})
			_p('/* End PBXReferenceProxy section */')
			_p('')
		end
	end


	function xcode.PBXResourcesBuildPhase(tr)
		_p('/* Begin PBXResourcesBuildPhase section */')
		for _, target in ipairs(tr.products.children) do
			_p(2,'%s /* Resources */ = {', target.resstageid)
			_p(3,'isa = PBXResourcesBuildPhase;')
			_p(3,'buildActionMask = 2147483647;')
			_p(3,'files = (')
			tree.traverse(tr, {
				onnode = function(node)
					if xcode.getbuildcategory(node) == "Resources" then
						_p(4,'%s /* %s in Resources */,', node.buildid, node.name)
					end
				end
			})
			_p(3,');')
			_p(3,'runOnlyForDeploymentPostprocessing = 0;')
			_p(2,'};')
		end
		_p('/* End PBXResourcesBuildPhase section */')
		_p('')
	end

	function xcode.PBXShellScriptBuildPhase(tr)
		local wrapperWritten = false

		local function doblock(id, name, commands, files)
			if commands ~= nil then
				commands = table.flatten(commands)
			end
				if #commands > 0 then
					if not wrapperWritten then
						_p('/* Begin PBXShellScriptBuildPhase section */')
						wrapperWritten = true
					end
					_p(2,'%s /* %s */ = {', id, name)
					_p(3,'isa = PBXShellScriptBuildPhase;')
					_p(3,'buildActionMask = 2147483647;')
					_p(3,'files = (')
					_p(3,');')
					_p(3,'inputPaths = (');
					if files ~= nil then
						files = table.flatten(files)
						if #files > 0 then
							for _, file in ipairs(files) do
								_p(4, '"%s",', file)
							end
						end
					end
					_p(3,');');
					_p(3,'name = %s;', name);
					_p(3,'outputPaths = (');
					_p(3,');');
					_p(3,'runOnlyForDeploymentPostprocessing = 0;');
					_p(3,'shellPath = /bin/sh;');
					_p(3,'shellScript = "%s";', table.concat(commands, "\\n"):gsub('"', '\\"'))
					_p(2,'};')
				end
			end

		local function wrapcommands(cmds, cfg)
			local commands = {}
			if #cmds > 0 then
				table.insert(commands, 'if [ "${CONFIGURATION}" = "' .. xcode.getconfigname(cfg) .. '" ]; then')
				for i = 1, #cmds do
					local cmd = cmds[i]
					cmd = cmd:gsub('\\','\\\\')
					table.insert(commands, cmd)
				end
				table.insert(commands, 'fi')
			end
			return commands
		end

		local function dobuildblock(id, name, which)
			-- see if there are any commands to add for each config
			local commands = {}
			for _, cfg in ipairs(tr.configs) do
				local cfgcmds = wrapcommands(cfg[which], cfg)
				if #cfgcmds > 0 then
					for i, cmd in ipairs(cfgcmds) do
						table.insert(commands, cmd)
					end
				end
			end
			doblock(id, name, commands)
		end

		local function doscriptphases(which)
			local i = 0
			for _, cfg in ipairs(tr.configs) do
				local cfgcmds = cfg[which]
				if cfgcmds ~= nil then
					for __, scripts in ipairs(cfgcmds) do
						for ___, script in ipairs(scripts) do
							local cmd = script[1]
							local files = script[2]
							local label = xcode.getscriptphaselabel(cmd, i, cfg)
							local id = xcode.uuid(label)
							doblock(id, label, wrapcommands({cmd}, cfg), files)
							i = i + 1
						end
					end
				end
			end
		end

		dobuildblock("9607AE1010C857E500CD1376", "Prebuild", "prebuildcommands")
		dobuildblock("9607AE3510C85E7E00CD1376", "Prelink", "prelinkcommands")
		dobuildblock("9607AE3710C85E8F00CD1376", "Postbuild", "postbuildcommands")
		doscriptphases("xcodescriptphases")

		if wrapperWritten then
			_p('/* End PBXShellScriptBuildPhase section */')
		end
	end


	function xcode.PBXSourcesBuildPhase(tr,prj)
		_p('/* Begin PBXSourcesBuildPhase section */')
		for _, target in ipairs(tr.products.children) do
			_p(2,'%s /* Sources */ = {', target.sourcesid)
			_p(3,'isa = PBXSourcesBuildPhase;')
			_p(3,'buildActionMask = 2147483647;')
			_p(3,'files = (')
			tree.traverse(tr, {
				onleaf = function(node)
					if xcode.getbuildcategory(node) == "Sources" then
						if not table.icontains(prj.excludes, node.cfg.name) then -- if not excluded
							_p(4,'%s /* %s in Sources */,', node.buildid, node.name)
						end
					end
				end
			})
			_p(3,');')
			_p(3,'runOnlyForDeploymentPostprocessing = 0;')
			_p(2,'};')
		end
		_p('/* End PBXSourcesBuildPhase section */')
		_p('')
	end

	-- copyresources leads to this
	-- xcodeembedframeworks
	function xcode.PBXCopyFilesBuildPhase(tr)
		local wrapperWritten = false

		local function doblock(id, name, folderSpec, path, files)
			-- note: folder spec:
			-- 0: Absolute Path
			-- 1: Wrapper
			-- 6: Executables
			-- 7: Resources
			-- 10: Frameworks
			-- 16: Products Directory
			-- category: 'Frameworks' or 'CopyFiles'

			if #files > 0 then
				if not wrapperWritten then
					_p('/* Begin PBXCopyFilesBuildPhase section */')
					wrapperWritten = true
				end
				_p(2,'%s /* %s */ = {', id, name)
				_p(3,'isa = PBXCopyFilesBuildPhase;')
				_p(3,'buildActionMask = 2147483647;')
				_p(3,'dstPath = \"%s\";', path)
				_p(3,'dstSubfolderSpec = \"%s\";', folderSpec)
				_p(3,'files = (')
				tree.traverse(tr, {
					onleaf = function(node)
						-- print(node.name)
						if table.icontains(files, node.name) then
							_p(4,'%s /* %s in %s */,',
								xcode.uuid(node.name .. 'in CopyFiles'), node.name, 'CopyFiles')
						end
					end
				})
				_p(3,');')
				_p(3,'runOnlyForDeploymentPostprocessing = 0;');
				_p(2,'};')
			end
		end

		local function docopyresources(which)
			local targets = tr.project[which]
			if #targets > 0 then
				local i = 0
				for _, t in ipairs(targets) do
					for __, tt in ipairs(t) do
						local label = xcode.getcopyphaselabel('Resources', i, tt[1])
						local id = xcode.uuid(label)
						local files = table.translate(table.flatten(tt[2]), path.getname)
						doblock(id, label, 7, tt[1], files)
						i = i + 1
					end
				end
			end
		end

		local function docopyframeworks(which)
			local targets = tr.project[which]
			if #targets > 0 then
				local label = "Copy Frameworks"
				local id = xcode.uuid(label)
				local files = table.translate(table.flatten(targets), path.getname)
				doblock(id, label, 10, "", files)
			end
		end

		docopyresources("xcodecopyresources")
		if tr.project.kind == "WindowedApp" then
			docopyframeworks("xcodecopyframeworks")
		end

		if wrapperWritten then
			_p('/* End PBXCopyFilesBuildPhase section */')
		end
	end

	function xcode.PBXVariantGroup(tr)
		_p('/* Begin PBXVariantGroup section */')
		tree.traverse(tr, {
			onbranch = function(node)
				if node.kind == "vgroup" then
					_p(2,'%s /* %s */ = {', node.id, node.name)
					_p(3,'isa = PBXVariantGroup;')
					_p(3,'children = (')
					for _, lang in ipairs(node.children) do
						_p(4,'%s /* %s */,', lang.id, lang.name)
					end
					_p(3,');')
					_p(3,'name = %s;', node.name)
					_p(3,'sourceTree = "<group>";')
					_p(2,'};')
				end
			end
		})
		_p('/* End PBXVariantGroup section */')
		_p('')
	end


	function xcode.PBXTargetDependency(tr)
		if #tr.projects.children > 0 then
			_p('/* Begin PBXTargetDependency section */')
			tree.traverse(tr.projects, {
				onleaf = function(node)
					_p(2,'%s /* PBXTargetDependency */ = {', node.parent.targetdependid)
					_p(3,'isa = PBXTargetDependency;')
					_p(3,'name = "%s";', node.name)
					_p(3,'targetProxy = %s /* PBXContainerItemProxy */;', node.parent.targetproxyid)
					_p(2,'};')
				end
			})
			_p('/* End PBXTargetDependency section */')
			_p('')
		end
	end


	function xcode.cfg_excluded_files(prj, cfg)
		local excluded = {}

		-- Converts a file path to a pattern with no relative parts, prefixed with `*`.
		local function exclude_pattern(file)
			if path.isabsolute(file) then
				return file
			end

			-- handle `foo/../bar`
			local start, term = file:findlast("/%.%./")
			if term then
				return path.join("*", file:sub(term + 1))
			end

			-- handle `../foo/bar`
			start, term = file:find("%.%./")
			if start == 1 then
				return path.join("*", file:sub(term + 1))
			end

			-- handle `foo/bar`
			return path.join("*", file)
		end

		local function add_file(file)
			local name = exclude_pattern(file)
			if not table.icontains(excluded, name) then
				table.insert(excluded, name)
			end
		end

		local function verify_file(file)
			local name = exclude_pattern(file)
			if table.icontains(excluded, name) then
				-- xcode only allows us to exclude files based on filename, not path...
				error("'" .. file .. "' would be excluded by the rule to exclude '" .. name .. "'")
			end
		end

		for _, file in ipairs(cfg.excludes) do
			add_file(file)
		end

		for _, file in ipairs(prj.allfiles) do
			if not table.icontains(prj.excludes, file) and not table.icontains(cfg.excludes, file) then
				if not table.icontains(cfg.files, file) then
					add_file(file)
				else
					verify_file(file)
				end
			end
		end

		table.sort(excluded)
		return excluded
	end


	function xcode.XCBuildConfiguration_Impl(tr, id, opts, cfg)
		local cfgname = xcode.getconfigname(cfg)

		_p(2,'%s /* %s */ = {', id, cfgname)
		_p(3,'isa = XCBuildConfiguration;')
		_p(3,'buildSettings = {')

		for k, v in table.sortedpairs(opts) do
			if type(v) == "table" then
				if #v > 0 then
					_p(4,'%s = (', k)

					for i, v2 in ipairs(v) do
						_p(5,'%s,', xcode.quotestr(tostring(v2)))
					end

					_p(4,');')
				end
			else
				_p(4,'%s = %s;', k, xcode.quotestr(tostring(v)))
			end
		end

		_p(3,'};')
		_p(3,'name = %s;', xcode.quotestr(cfgname))
		_p(2,'};')
	end

	local function add_options(options, extras)
		for _, tbl in ipairs(extras) do
			for tkey, tval in pairs(tbl) do
				options[tkey] = tval
			end
		end
	end

	local function add_wholearchive_links(opts, cfg)
		if #cfg.wholearchive > 0 then
			local linkopts = {}

			for _, depcfg in ipairs(premake.getlinks(cfg, "siblings", "object")) do
				if table.icontains(cfg.wholearchive, depcfg.project.name) then
					local linkpath = path.rebase(depcfg.linktarget.fullpath, depcfg.location, cfg.location)
					table.insert(linkopts, "-force_load")
					table.insert(linkopts, linkpath)
				end
			end

			if opts.OTHER_LDFLAGS then
				linkopts = table.join(linkopts, opts.OTHER_LDFLAGS)
			end

			opts.OTHER_LDFLAGS = linkopts
		end
	end

	function xcode.XCBuildConfiguration(tr, prj, opts)
		_p('/* Begin XCBuildConfiguration section */')
		for _, target in ipairs(tr.products.children) do
			for _, cfg in ipairs(tr.configs) do
				local values = opts.ontarget(tr, target, cfg)
				add_options(values, cfg.xcodetargetopts)
				xcode.XCBuildConfiguration_Impl(tr, cfg.xcode.targetid, values, cfg)
			end
		end
		for _, cfg in ipairs(tr.configs) do
			local values = opts.onproject(tr, prj, cfg)
			add_options(values, cfg.xcodeprojectopts)
			add_wholearchive_links(values, cfg)
			xcode.XCBuildConfiguration_Impl(tr, cfg.xcode.projectid, values, cfg)
		end
		_p('/* End XCBuildConfiguration section */')
		_p('')
	end


	function xcode.XCBuildConfigurationList(tr)
		local sln = tr.project.solution

		_p('/* Begin XCConfigurationList section */')
		for _, target in ipairs(tr.products.children) do
			_p(2,'%s /* Build configuration list for PBXNativeTarget "%s" */ = {', target.cfgsection, target.name)
			_p(3,'isa = XCConfigurationList;')
			_p(3,'buildConfigurations = (')
			for _, cfg in ipairs(tr.configs) do
				_p(4,'%s /* %s */,', cfg.xcode.targetid, xcode.getconfigname(cfg))
			end
			_p(3,');')
			_p(3,'defaultConfigurationIsVisible = 0;')
			_p(3,'defaultConfigurationName = "%s";', xcode.getconfigname(tr.configs[1]))
			_p(2,'};')
		end
		_p(2,'1DEB928908733DD80010E9CD /* Build configuration list for PBXProject "%s" */ = {', tr.name)
		_p(3,'isa = XCConfigurationList;')
		_p(3,'buildConfigurations = (')
		for _, cfg in ipairs(tr.configs) do
			_p(4,'%s /* %s */,', cfg.xcode.projectid, xcode.getconfigname(cfg))
		end
		_p(3,');')
		_p(3,'defaultConfigurationIsVisible = 0;')
		_p(3,'defaultConfigurationName = "%s";', xcode.getconfigname(tr.configs[1]))
		_p(2,'};')
		_p('/* End XCConfigurationList section */')
		_p('')
	end


	function xcode.Footer()
		_p(1,'};')
		_p('\trootObject = __RootObject_ /* Project object */;')
		_p('}')
	end
