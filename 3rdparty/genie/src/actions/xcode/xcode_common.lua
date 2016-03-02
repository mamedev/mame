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
			[".c"] = "Sources",
			[".cc"] = "Sources",
			[".cpp"] = "Sources",
			[".cxx"] = "Sources",
			[".dylib"] = "Frameworks",
			[".framework"] = "Frameworks",
			[".m"] = "Sources",
			[".mm"] = "Sources",
			[".strings"] = "Resources",
			[".nib"] = "Resources",
			[".xib"] = "Resources",
			[".icns"] = "Resources",
			[".bmp"] = "Resources",
			[".wav"] = "Resources",
		}
		return categories[path.getextension(node.name)]
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
			[".framework"] = "wrapper.framework",
			[".gif"]       = "image.gif",
			[".h"]         = "sourcecode.c.h",
			[".html"]      = "text.html",
			[".lua"]       = "sourcecode.lua",
			[".m"]         = "sourcecode.c.objc",
			[".mm"]        = "sourcecode.cpp.objcpp",
			[".nib"]       = "wrapper.nib",
			[".pch"]       = "sourcecode.c.h",
			[".plist"]     = "text.plist.xml",
			[".strings"]   = "text.plist.strings",
			[".xib"]       = "file.xib",
			[".icns"]      = "image.icns",
			[".bmp"]       = "image.bmp",
			[".wav"]       = "audio.wav",
		}
		return types[path.getextension(node.path)] or "text"
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
			[".framework"] = "wrapper.framework",
			[".gif"]       = "image.gif",
			[".h"]         = "sourcecode.cpp.h",
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
		}
		return types[path.getextension(node.path)] or "text"
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
			WindowedApp = "com.apple.product-type.application",
			StaticLib   = "com.apple.product-type.library.static",
			SharedLib   = "com.apple.product-type.library.dynamic",
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
			WindowedApp = "wrapper.application",
			StaticLib   = "archive.ar",
			SharedLib   = "\"compiled.mach-o.dylib\"",
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
		return (path.getextension(fname) == ".framework")
	end


--
-- Retrieves a unique 12 byte ID for an object. This function accepts and ignores two
-- parameters 'node' and 'usage', which are used by an alternative implementation of
-- this function for testing.
--
-- @returns
--    A 24-character string representing the 12 byte ID.
--

	function xcode.newid()
		return string.format("%04X%04X%04X%04X%04X%04X",
			math.random(0, 32767),
			math.random(0, 32767),
			math.random(0, 32767),
			math.random(0, 32767),
			math.random(0, 32767),
			math.random(0, 32767))
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

	function xcode.printlist(list, tag)
		if #list > 0 then
			_p(4,'%s = (', tag)
			for _, item in ipairs(list) do
				local escaped_item = item:gsub("\"", "\\\"")
				_p(5, '"%s",', escaped_item)
			end
			_p(4,');')
		end
	end


---------------------------------------------------------------------------
-- Section generator functions, in the same order in which they appear
-- in the .pbxproj file
---------------------------------------------------------------------------

	function xcode.Header()
		_p('// !$*UTF8*$!')
		_p('{')
		_p(1,'archiveVersion = 1;')
		_p(1,'classes = {')
		_p(1,'};')
		_p(1,'objectVersion = 45;')
		_p(1,'objects = {')
		_p('')
	end


	function xcode.PBXBuildFile(tr)
		_p('/* Begin PBXBuildFile section */')
		tree.traverse(tr, {
			onnode = function(node)
				if node.buildid then
					_p(2,'%s /* %s in %s */ = {isa = PBXBuildFile; fileRef = %s /* %s */; };', 
						node.buildid, node.name, xcode.getbuildcategory(node), node.id, node.name)
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
								error('relative paths are not currently supported for frameworks')
							end
							pth = nodePath
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

						-- if the parent node is virtual, it won't have a local path
						-- of its own; need to use full relative path from project
						if node.parent.isvpath then
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
					if node.path and not node.isvpath then
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
			
			_p(2,'%s /* %s */ = {', node.targetid, name)
			_p(3,'isa = PBXNativeTarget;')
			_p(3,'buildConfigurationList = %s /* Build configuration list for PBXNativeTarget "%s" */;', node.cfgsection, name)
			_p(3,'buildPhases = (')
			if hasBuildCommands('prebuildcommands') then
				_p(4,'9607AE1010C857E500CD1376 /* Prebuild */,')
			end
			_p(4,'%s /* Resources */,', node.resstageid)
			_p(4,'%s /* Sources */,', node.sourcesid)
			if hasBuildCommands('prelinkcommands') then
				_p(4,'9607AE3510C85E7E00CD1376 /* Prelink */,')
			end
			_p(4,'%s /* Frameworks */,', node.fxstageid)
			if hasBuildCommands('postbuildcommands') then
				_p(4,'9607AE3710C85E8F00CD1376 /* Postbuild */,')
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


	function xcode.PBXProject(tr)
		_p('/* Begin PBXProject section */')
		_p(2,'08FB7793FE84155DC02AAC07 /* Project object */ = {')
		_p(3,'isa = PBXProject;')
		_p(3,'buildConfigurationList = 1DEB928908733DD80010E9CD /* Build configuration list for PBXProject "%s" */;', tr.name)
		_p(3,'compatibilityVersion = "Xcode 3.2";')
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

		local function doblock(id, name, which)
			-- start with the project-level commands (most common)
			local prjcmds = tr.project[which]
			local commands = table.join(prjcmds, {})

			-- see if there are any config-specific commands to add
			for _, cfg in ipairs(tr.configs) do
				local cfgcmds = cfg[which]
				if #cfgcmds > #prjcmds then
					table.insert(commands, 'if [ "${CONFIGURATION}" = "' .. xcode.getconfigname(cfg) .. '" ]; then')
					for i = #prjcmds + 1, #cfgcmds do
						table.insert(commands, cfgcmds[i])
					end
					table.insert(commands, 'fi')
				end
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
				
		doblock("9607AE1010C857E500CD1376", "Prebuild", "prebuildcommands")
		doblock("9607AE3510C85E7E00CD1376", "Prelink", "prelinkcommands")
		doblock("9607AE3710C85E8F00CD1376", "Postbuild", "postbuildcommands")
		
		if wrapperWritten then
			_p('/* End PBXShellScriptBuildPhase section */')
		end
	end
	
	
	function xcode.PBXSourcesBuildPhase(tr)
		_p('/* Begin PBXSourcesBuildPhase section */')
		for _, target in ipairs(tr.products.children) do
			_p(2,'%s /* Sources */ = {', target.sourcesid)
			_p(3,'isa = PBXSourcesBuildPhase;')
			_p(3,'buildActionMask = 2147483647;')
			_p(3,'files = (')
			tree.traverse(tr, {
				onleaf = function(node)
					if xcode.getbuildcategory(node) == "Sources" then
						_p(4,'%s /* %s in Sources */,', node.buildid, node.name)
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


	function xcode.XCBuildConfiguration_Target(tr, target, cfg)
		local cfgname = xcode.getconfigname(cfg)
		
		_p(2,'%s /* %s */ = {', cfg.xcode.targetid, cfgname)
		_p(3,'isa = XCBuildConfiguration;')
		_p(3,'buildSettings = {')
		_p(4,'ALWAYS_SEARCH_USER_PATHS = NO;')

		if not cfg.flags.Symbols then
			_p(4,'DEBUG_INFORMATION_FORMAT = "dwarf-with-dsym";')
		end
		
		if cfg.kind ~= "StaticLib" and cfg.buildtarget.prefix ~= "" then
			_p(4,'EXECUTABLE_PREFIX = %s;', cfg.buildtarget.prefix)
		end
		
		if cfg.targetextension then
			local ext = cfg.targetextension
			ext = iif(ext:startswith("."), ext:sub(2), ext)
			_p(4,'EXECUTABLE_EXTENSION = %s;', ext)
		end

		local outdir = path.getdirectory(cfg.buildtarget.bundlepath)
		if outdir ~= "." then
			_p(4,'CONFIGURATION_BUILD_DIR = %s;', outdir)
		end

		_p(4,'GCC_DYNAMIC_NO_PIC = NO;')
		_p(4,'GCC_MODEL_TUNING = G5;')

		if tr.infoplist then
			_p(4,'INFOPLIST_FILE = "%s";', tr.infoplist.cfg.name)
		end

		installpaths = {
			ConsoleApp = '/usr/local/bin',
			WindowedApp = '"$(HOME)/Applications"',
			SharedLib = '/usr/local/lib',
			StaticLib = '/usr/local/lib',
		}
		_p(4,'INSTALL_PATH = %s;', installpaths[cfg.kind])
		
		local infoplist_file = nil
		
		for _, v in ipairs(cfg.files) do
			-- for any file named *info.plist, use it as the INFOPLIST_FILE
			if (string.find (string.lower (v), 'info.plist') ~= nil) then
				infoplist_file = string.format('$(SRCROOT)/%s', v)
			end
		end
		
		if infoplist_file ~= nil then
			_p(4,'INFOPLIST_FILE = "%s";', infoplist_file)
		end

		_p(4,'PRODUCT_NAME = "%s";', cfg.buildtarget.basename)
		_p(3,'};')
		_p(3,'name = "%s";', cfgname)
		_p(2,'};')
	end
	
	
	function xcode.XCBuildConfiguration_Project(tr, cfg)
		local cfgname = xcode.getconfigname(cfg)

		_p(2,'%s /* %s */ = {', cfg.xcode.projectid, cfgname)
		_p(3,'isa = XCBuildConfiguration;')
		_p(3,'buildSettings = {')
		
		local archs = {
			Native = "$(NATIVE_ARCH_ACTUAL)",
			x32    = "i386",
			x64    = "x86_64",
			Universal32 = "$(ARCHS_STANDARD_32_BIT)",
			Universal64 = "$(ARCHS_STANDARD_64_BIT)",
			Universal = "$(ARCHS_STANDARD_32_64_BIT)",
		}
		_p(4,'ARCHS = "%s";', archs[cfg.platform])

		_p(4,'SDKROOT = "%s";', xcode.toolset)
		
		local targetdir = path.getdirectory(cfg.buildtarget.bundlepath)
		if targetdir ~= "." then
			_p(4,'CONFIGURATION_BUILD_DIR = "$(SYMROOT)";');
		end
		
		_p(4,'CONFIGURATION_TEMP_DIR = "$(OBJROOT)";')
		
		if cfg.flags.Symbols then
			_p(4,'COPY_PHASE_STRIP = NO;')
		end
		
		_p(4,'GCC_C_LANGUAGE_STANDARD = gnu99;')
		
		if cfg.flags.NoExceptions then
			_p(4,'GCC_ENABLE_CPP_EXCEPTIONS = NO;')
		end
		
		if cfg.flags.NoRTTI then
			_p(4,'GCC_ENABLE_CPP_RTTI = NO;')
		end
		
		if _ACTION ~= "xcode4" and cfg.flags.Symbols and not cfg.flags.NoEditAndContinue then
			_p(4,'GCC_ENABLE_FIX_AND_CONTINUE = YES;')
		end
		
		if cfg.flags.NoExceptions then
			_p(4,'GCC_ENABLE_OBJC_EXCEPTIONS = NO;')
		end
		
		if cfg.flags.Optimize or cfg.flags.OptimizeSize then
			_p(4,'GCC_OPTIMIZATION_LEVEL = s;')
		elseif cfg.flags.OptimizeSpeed then
			_p(4,'GCC_OPTIMIZATION_LEVEL = 3;')
		else
			_p(4,'GCC_OPTIMIZATION_LEVEL = 0;')
		end
		
		if cfg.pchheader and not cfg.flags.NoPCH then
			_p(4,'GCC_PRECOMPILE_PREFIX_HEADER = YES;')
			_p(4,'GCC_PREFIX_HEADER = "%s";', cfg.pchheader)
		end
		
		xcode.printlist(cfg.defines, 'GCC_PREPROCESSOR_DEFINITIONS')

		_p(4,'GCC_SYMBOLS_PRIVATE_EXTERN = NO;')
		
		if cfg.flags.FatalWarnings then
			_p(4,'GCC_TREAT_WARNINGS_AS_ERRORS = YES;')
		end
		
		_p(4,'GCC_WARN_ABOUT_RETURN_TYPE = YES;')
		_p(4,'GCC_WARN_UNUSED_VARIABLE = YES;')

		xcode.printlist(cfg.includedirs, 'HEADER_SEARCH_PATHS')
		xcode.printlist(cfg.libdirs, 'LIBRARY_SEARCH_PATHS')
		
		_p(4,'OBJROOT = "%s";', cfg.objectsdir)

		_p(4,'ONLY_ACTIVE_ARCH = %s;',iif(premake.config.isdebugbuild(cfg),'YES','NO'))
		
		-- build list of "other" C/C++ flags
		local checks = {
			["-ffast-math"]          = cfg.flags.FloatFast,
			["-ffloat-store"]        = cfg.flags.FloatStrict,
			["-fomit-frame-pointer"] = cfg.flags.NoFramePointer,
		}
			
		local flags = { }
		for flag, check in pairs(checks) do
			if check then
				table.insert(flags, flag)
			end
		end

		for _, val in ipairs(premake.xcode.parameters) do
			_p(4, val ..';')
		end

		xcode.printlist(table.join(flags, cfg.buildoptions, cfg.buildoptions_c), 'OTHER_CFLAGS')
		xcode.printlist(table.join(flags, cfg.buildoptions, cfg.buildoptions_cpp), 'OTHER_CPLUSPLUSFLAGS')

		-- build list of "other" linked flags. All libraries that aren't frameworks
		-- are listed here, so I don't have to try and figure out if they are ".a"
		-- or ".dylib", which Xcode requires to list in the Frameworks section
		flags = { }
		for _, lib in ipairs(premake.getlinks(cfg, "system")) do
			if not xcode.isframework(lib) then
				table.insert(flags, "-l" .. lib)
			end
		end
		flags = table.join(flags, cfg.linkoptions)
		xcode.printlist(flags, 'OTHER_LDFLAGS')
		
		if cfg.flags.StaticRuntime then
			_p(4,'STANDARD_C_PLUS_PLUS_LIBRARY_TYPE = static;')
		end
		
		if targetdir ~= "." then
			_p(4,'SYMROOT = "%s";', targetdir)
		end
		
		if cfg.flags.ExtraWarnings then
			_p(4,'WARNING_CFLAGS = "-Wall";')
		end
		
		_p(3,'};')
		_p(3,'name = "%s";', cfgname)
		_p(2,'};')
	end


	function xcode.XCBuildConfiguration(tr)
		_p('/* Begin XCBuildConfiguration section */')
		for _, target in ipairs(tr.products.children) do
			for _, cfg in ipairs(tr.configs) do
				xcode.XCBuildConfiguration_Target(tr, target, cfg)
			end
		end
		for _, cfg in ipairs(tr.configs) do
			xcode.XCBuildConfiguration_Project(tr, cfg)
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
		_p('\trootObject = 08FB7793FE84155DC02AAC07 /* Project object */;')
		_p('}')
	end
