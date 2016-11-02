--
-- xcode_project.lua
-- Generate an Xcode C/C++ project.
-- Copyright (c) 2009 Jason Perkins and the Premake project
--

	local xcode = premake.xcode
	local tree = premake.tree

--
-- Create a tree corresponding to what is shown in the Xcode project browser
-- pane, with nodes for files and folders, resources, frameworks, and products.
--
-- @param prj
--    The project being generated.
-- @returns
--    A tree, loaded with metadata, which mirrors Xcode's view of the project.
--

	function xcode.buildprjtree(prj)
		local tr = premake.project.buildsourcetree(prj, true)

		-- create a list of build configurations and assign IDs
		tr.configs = {}
		for _, cfgname in ipairs(prj.solution.configurations) do
			for _, platform in ipairs(prj.solution.xcode.platforms) do
				local cfg = premake.getconfig(prj, cfgname, platform)
				cfg.xcode = {}
				cfg.xcode.targetid = xcode.newid(prj.xcode.projectnode, cfgname)
				cfg.xcode.projectid = xcode.newid(tr, cfgname)
				table.insert(tr.configs, cfg)
			end
		end

		-- convert localized resources from their filesystem layout (English.lproj/MainMenu.xib)
		-- to Xcode's display layout (MainMenu.xib/English).
		tree.traverse(tr, {
			onbranch = function(node)
				if path.getextension(node.name) == ".lproj" then
					local lang = path.getbasename(node.name)  -- "English", "French", etc.

					-- create a new language group for each file it contains
					for _, filenode in ipairs(node.children) do
						local grpnode = node.parent.children[filenode.name]
						if not grpnode then
							grpnode = tree.insert(node.parent, tree.new(filenode.name))
							grpnode.kind = "vgroup"
						end

						-- convert the file node to a language node and add to the group
						filenode.name = path.getbasename(lang)
						tree.insert(grpnode, filenode)
					end

					-- remove this directory from the tree
					tree.remove(node)
				end
			end
		})

		-- fix .xcassets files, they should be treated as a file, not a folder
		tree.traverse(tr, {
			onbranch = function(node)
				if path.getextension(node.name) == ".xcassets" then
					node.children = {}
				end
			end
		})

		-- the special folder "Frameworks" lists all linked frameworks
		tr.frameworks = tree.new("Frameworks")
		for cfg in premake.eachconfig(prj) do
			for _, link in ipairs(premake.getlinks(cfg, "system", "fullpath")) do
				local name = path.getname(link)
				if xcode.isframework(name) and not tr.frameworks.children[name] then
					node = tree.insert(tr.frameworks, tree.new(name))
					node.path = link
				end
			end
		end

		-- only add it to the tree if there are frameworks to link
		if #tr.frameworks.children > 0 then
			tree.insert(tr, tr.frameworks)
		end

		-- the special folder "Products" holds the target produced by the project; this
		-- is populated below
		tr.products = tree.insert(tr, tree.new("Products"))

		-- the special folder "Projects" lists sibling project dependencies
		tr.projects = tree.new("Projects")
		for _, dep in ipairs(premake.getdependencies(prj, "sibling", "object")) do
			-- create a child node for the dependency's xcodeproj
			local xcpath = xcode.getxcodeprojname(dep)
			local xcnode = tree.insert(tr.projects, tree.new(path.getname(xcpath)))
			xcnode.path = xcpath
			xcnode.project = dep
			xcnode.productgroupid = xcode.newid(xcnode, "prodgrp")
			xcnode.productproxyid = xcode.newid(xcnode, "prodprox")
			xcnode.targetproxyid  = xcode.newid(xcnode, "targprox")
			xcnode.targetdependid = xcode.newid(xcnode, "targdep")

			-- create a grandchild node for the dependency's link target
			local cfg = premake.getconfig(dep, prj.configurations[1])
			node = tree.insert(xcnode, tree.new(cfg.linktarget.name))
			node.path = cfg.linktarget.fullpath
			node.cfg = cfg
		end

		if #tr.projects.children > 0 then
			tree.insert(tr, tr.projects)
		end

		-- Final setup
		tree.traverse(tr, {
			onnode = function(node)
				-- assign IDs to every node in the tree
				node.id = xcode.newid(node)

				-- assign build IDs to buildable files
				if xcode.getbuildcategory(node) then
					node.buildid = xcode.newid(node, "build")
				end

				-- remember key files that are needed elsewhere
				if string.endswith(node.name, "Info.plist") then
					tr.infoplist = node
				end
				if string.endswith(node.name, ".entitlements") then
					tr.entitlements = node
				end
			end
		}, true)

		-- Plug in the product node into the Products folder in the tree. The node
		-- was built in xcode.preparesolution() in xcode_common.lua; it contains IDs
		-- that are necessary for inter-project dependencies
		node = tree.insert(tr.products, prj.xcode.projectnode)
		node.kind = "product"
		node.path = node.cfg.buildtarget.fullpath
		node.cfgsection = xcode.newid(node, "cfg")
		node.resstageid = xcode.newid(node, "rez")
		node.sourcesid  = xcode.newid(node, "src")
		node.fxstageid  = xcode.newid(node, "fxs")

		return tr
	end


--
-- Generate an Xcode .xcodeproj for a Premake project.
--
-- @param prj
--    The Premake project to generate.
--

	function premake.xcode.project(prj)
		local tr = xcode.buildprjtree(prj)
		xcode.Header(tr)
		xcode.PBXBuildFile(tr)
		xcode.PBXContainerItemProxy(tr)
		xcode.PBXFileReference(tr,prj)
		xcode.PBXFrameworksBuildPhase(tr)
		xcode.PBXGroup(tr)
		xcode.PBXNativeTarget(tr)
		xcode.PBXProject(tr)
		xcode.PBXReferenceProxy(tr)
		xcode.PBXResourcesBuildPhase(tr)
		xcode.PBXShellScriptBuildPhase(tr)
		xcode.PBXSourcesBuildPhase(tr,prj)
		xcode.PBXVariantGroup(tr)
		xcode.PBXTargetDependency(tr)
		xcode.XCBuildConfiguration(tr, prj)
		xcode.XCBuildConfigurationList(tr)
		xcode.Footer(tr)
	end
