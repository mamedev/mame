--
-- base/bake.lua
--
-- Takes all the configuration information provided by the project scripts
-- and stored in the solution->project->block hierarchy and flattens it all
-- down into one object per configuration. These objects are cached with the
-- project, and can be retrieved by calling the getconfig() or eachconfig().
--
-- Copyright (c) 2008-2011 Jason Perkins and the Premake project
--

	premake.bake = { }
	local bake = premake.bake


-- do not copy these fields into the configurations

	local nocopy =
	{
		blocks    = true,
		keywords  = true,
		projects  = true,
		__configs = true,
	}

-- do not cascade these fields from projects to configurations

	local nocascade =
	{
		makesettings = true,
	}

-- leave these paths as absolute, rather than converting to project relative

	local keeprelative =
	{
		basedir  = true,
		location = true,
	}



--
-- Returns a list of all of the active terms from the current environment.
-- See the docs for configuration() for more information about the terms.
--

	function premake.getactiveterms()
		local terms = { _action = _ACTION:lower(), os = os.get() }

		-- add option keys or values
		for key, value in pairs(_OPTIONS) do
			if value ~= "" then
				table.insert(terms, value:lower())
			else
				table.insert(terms, key:lower())
			end
		end

		return terms
	end


--
-- Test a single configuration block keyword against a list of terms.
-- The terms are a mix of key/value pairs. The keyword is tested against
-- the values; on a match, the corresponding key is returned. This
-- enables testing for required values in iskeywordsmatch(), below.
--

	function premake.iskeywordmatch(keyword, terms)
		-- is it negated?
		if keyword:startswith("not ") then
			return not premake.iskeywordmatch(keyword:sub(5), terms)
		end

		for _, pattern in ipairs(keyword:explode(" or ")) do
			for termkey, term in pairs(terms) do
				if term:match(pattern) == term then
					return termkey
				end
			end
		end
	end



--
-- Checks a set of configuration block keywords against a list of terms.
-- The required flag is used by the file configurations: only blocks
-- with a term that explictly matches the filename get applied; more
-- general blocks are skipped over (since they were already applied at
-- the config level).
--

	function premake.iskeywordsmatch(keywords, terms)
		local hasrequired = false
		for _, keyword in ipairs(keywords) do
			local matched = premake.iskeywordmatch(keyword, terms)
			if not matched then
				return false
			end
			if matched == "required" then
				hasrequired = true
			end
		end

		if terms.required and not hasrequired then
			return false
		else
			return true
		end
	end


--
-- Converts path fields from absolute to location-relative paths.
--
-- @param location
--    The base location, paths will be relative to this directory.
-- @param obj
--    The object containing the fields to be adjusted.
--

	local function adjustpaths(location, obj)
		function adjustpathlist(list)
			for i, p in ipairs(list) do
				list[i] = path.getrelative(location, p)
			end
		end

		for name, value in pairs(obj) do
			local field = premake.fields[name]
			if field and value and not keeprelative[name] then
				if field.kind == "path" then
					obj[name] = path.getrelative(location, value)
				elseif field.kind == "dirlist" or field.kind == "filelist" then
					adjustpathlist(value)
				elseif field.kind == "keypath" then
					for k,v in pairs(value) do
						adjustpathlist(v)
					end
				end
			end
		end
	end



--
-- Merge all of the fields from one object into another. String values are overwritten,
-- while list values are merged. Fields listed in premake.nocopy are skipped.
--
-- @param dest
--    The destination object, to contain the merged settings.
-- @param src
--    The source object, containing the settings to added to the destination.
--

	local function mergefield(kind, dest, src)
		local tbl = dest or { }
		if kind == "keyvalue" or kind == "keypath" then
			for key, value in pairs(src) do
				tbl[key] = mergefield("list", tbl[key], value)
			end
		else
			for _, item in ipairs(src) do
				if not tbl[item] then
					table.insert(tbl, item)
					tbl[item] = item
				end
			end
		end
		return tbl
	end

	local function removevalues(tbl, removes)
		for i=#tbl,1,-1 do
			for _, pattern in ipairs(removes) do
				if pattern == tbl[i] then
					table.remove(tbl, i)
					break
				end
			end
		end
	end

	local function mergeobject(dest, src)
		-- if there's nothing to add, quick out
		if not src then
			return
		end

		for fieldname, value in pairs(src) do
			if not nocopy[fieldname] then
				-- fields that are included in the API are merged...
				local field = premake.fields[fieldname]
				if field then
					if type(value) == "table" then
						dest[fieldname] = mergefield(field.kind, dest[fieldname], value)
						if src.removes then
							removes = src.removes[fieldname]
							if removes then
								removevalues(dest[fieldname], removes)
							end
						end
					else
						dest[fieldname] = value
					end

				-- ...everything else is just copied as-is
				else
					dest[fieldname] = value
				end
			end
		end
	end



--
-- Merges the settings from a solution's or project's list of configuration blocks,
-- for all blocks that match the provided set of environment terms.
--
-- @param dest
--    The destination object, to contain the merged settings.
-- @param obj
--    The solution or project object being collapsed.
-- @param basis
--    "Root" level settings, from the solution, which act as a starting point for
--    all of the collapsed settings built during this call.
-- @param terms
--    A list of keywords to filter the configuration blocks; only those that
--    match will be included in the destination.
-- @param cfgname
--    The name of the configuration being collapsed. May be nil.
-- @param pltname
--    The name of the platform being collapsed. May be nil.
--

	local function merge(dest, obj, basis, terms, cfgname, pltname)
		-- the configuration key is the merged configuration and platform names
		local key = cfgname or ""
		pltname = pltname or "Native"
		if pltname ~= "Native" then
			key = key .. pltname
		end

		-- add the configuration and platform to the block filter terms
		terms.config = (cfgname or ""):lower()
		terms.platform = pltname:lower()

		-- build the configuration base by merging the solution and project level settings
		local cfg = {}
		mergeobject(cfg, basis[key])

		adjustpaths(obj.location, cfg)
		mergeobject(cfg, obj)

		-- add `kind` to the filter terms
		if (cfg.kind) then
			terms['kind']=cfg.kind:lower()
		end

		-- now add in any blocks that match the filter terms
		for _, blk in ipairs(obj.blocks) do
			if (premake.iskeywordsmatch(blk.keywords, terms))then
				mergeobject(cfg, blk)
				if (cfg.kind and not cfg.terms.kind) then
					cfg.terms['kind'] = cfg.kind:lower()
					terms['kind'] = cfg.kind:lower()
				end
			end
		end

		-- package it all up and add it to the result set
		cfg.name      = cfgname
		cfg.platform  = pltname
		for k,v in pairs(terms) do
			cfg.terms[k] =v
		end
		dest[key] = cfg
	end



--
-- Collapse a solution or project object down to a canonical set of configuration settings,
-- keyed by configuration block/platform pairs, and taking into account the current
-- environment settings.
--
-- @param obj
--    The solution or project to be collapsed.
-- @param basis
--    "Root" level settings, from the solution, which act as a starting point for
--    all of the collapsed settings built during this call.
-- @returns
--    The collapsed list of settings, keyed by configuration block/platform pair.
--

	local function collapse(obj, basis)
		local result = {}
		basis = basis or {}

		-- find the solution, which contains the configuration and platform lists
		local sln = obj.solution or obj

		-- build a set of configuration filter terms; only those configuration blocks
		-- with a matching set of keywords will be included in the merged results
		local terms = premake.getactiveterms()

		-- build a project-level configuration.
		merge(result, obj, basis, terms)--this adjusts terms

		-- now build configurations for each build config/platform pair
		for _, cfgname in ipairs(sln.configurations) do
			local terms_local = {}
			for k,v in pairs(terms)do terms_local[k]=v end
			merge(result, obj, basis, terms_local, cfgname, "Native")--terms cam also be adjusted here
			for _, pltname in ipairs(sln.platforms or {}) do
				if pltname ~= "Native" then
					merge(result, obj, basis,terms_local, cfgname, pltname)--terms also here
				end
			end
		end

		return result
	end



--
-- Computes a unique objects directory for every configuration, using the
-- following choices:
--   [1] -> the objects directory as set in the project of config
--   [2] -> [1] + the platform name
--   [3] -> [2] + the configuration name
--   [4] -> [3] + the project name
--

	local function builduniquedirs()
		local num_variations = 4

		-- Start by listing out each possible object directory for each configuration.
		-- Keep a count of how many times each path gets used across the session.
		local cfg_dirs = {}
		local hit_counts = {}

		for sln in premake.solution.each() do
			for _, prj in ipairs(sln.projects) do
				for _, cfg in pairs(prj.__configs) do

					local dirs = { }
					dirs[1] = path.getabsolute(path.join(cfg.location, cfg.objdir or cfg.project.objdir or "obj"))
					dirs[2] = path.join(dirs[1], iif(cfg.platform == "Native", "", cfg.platform))
					dirs[3] = path.join(dirs[2], cfg.name)
					dirs[4] = path.join(dirs[3], cfg.project.name)
					cfg_dirs[cfg] = dirs

					-- configurations other than the root should bias toward a more
					-- description path, including the platform or config name
					local start = iif(cfg.name, 2, 1)
					for v = start, num_variations do
						local d = dirs[v]
						hit_counts[d] = (hit_counts[d] or 0) + 1
					end

				end
			end
		end

		-- Now assign an object directory to each configuration, skipping those
		-- that are in use somewhere else in the session
		for sln in premake.solution.each() do
			for _, prj in ipairs(sln.projects) do
				for _, cfg in pairs(prj.__configs) do

					local dir
					local start = iif(cfg.name, 2, 1)
					for v = start, iif(cfg.flags.SingleOutputDir==true,num_variations-1,num_variations) do
						dir = cfg_dirs[cfg][v]
						if hit_counts[dir] == 1 then break end
					end
					cfg.objectsdir = path.getrelative(cfg.location, dir)
				end
			end
		end

	end



--
-- Pre-computes the build and link targets for a configuration.
--

	local function buildtargets()
		for sln in premake.solution.each() do
			for _, prj in ipairs(sln.projects) do
				for _, cfg in pairs(prj.__configs) do
					-- determine which conventions the target should follow for this config
					local pathstyle = premake.getpathstyle(cfg)
					local namestyle = premake.getnamestyle(cfg)

					-- build the targets
					cfg.buildtarget = premake.gettarget(cfg, "build", pathstyle, namestyle, cfg.system)
					cfg.linktarget  = premake.gettarget(cfg, "link",  pathstyle, namestyle, cfg.system)
					if pathstyle == "windows" then
						cfg.objectsdir = path.translate(cfg.objectsdir, "\\")
					end

				end
			end
		end
	end

  	local function getCfgKind(cfg)
  		if(cfg.kind) then
  			return cfg.kind;
  		end

  		if(cfg.project.__configs[""] and cfg.project.__configs[""].kind) then
  			return cfg.project.__configs[""].kind;
  		end

  		return nil
  	end

  	local function getprojrec(dstArray, foundList, cfg, cfgname, searchField, bLinkage)
  		if(not cfg) then return end

  		local foundUsePrjs = {};
  		for _, useName in ipairs(cfg[searchField]) do
  			local testName = useName:lower();
  			if((not foundList[testName])) then
  				local theProj = nil;
  				local theUseProj = nil;
  				for _, prj in ipairs(cfg.project.solution.projects) do
  					if (prj.name:lower() == testName) then
  						if(prj.usage) then
  							theUseProj = prj;
  						else
  							theProj = prj;
  						end
  					end
  				end

  				--Must connect to a usage project.
  				if(theUseProj) then
  					foundList[testName] = true;
  					local prjEntry = {
  						name = testName,
  						proj = theProj,
  						usageProj = theUseProj,
  						bLinkageOnly = bLinkage,
  					};
  					dstArray[testName] = prjEntry;
  					table.insert(foundUsePrjs, theUseProj);
  				end
  			end
  		end

  		for _, usePrj in ipairs(foundUsePrjs) do
  			--Links can only recurse through static libraries.
  			if((searchField ~= "links") or
  				(getCfgKind(usePrj.__configs[cfgname]) == "StaticLib")) then
  				getprojrec(dstArray, foundList, usePrj.__configs[cfgname],
  					cfgname, searchField, bLinkage);
  			end
  		end
  	end

  --
  -- This function will recursively get all projects that the given configuration has in its "uses"
  -- field. The return values are a list of tables. Each table in that list contains the following:
  --		name = The lowercase name of the project.
  --		proj = The project. Can be nil if it is usage-only.
  --		usageProj = The usage project. Can't be nil, as using a project that has no
  -- 			usage project is not put into the list.
  --		bLinkageOnly = If this is true, then only the linkage information should be copied.
  -- The recursion will only look at the "uses" field on *usage* projects.
  -- This function will also add projects to the list that are mentioned in the "links"
  -- field of usage projects. These will only copy linker information, but they will recurse.
  -- through other "links" fields.
  --
  	local function getprojectsconnections(cfg, cfgname)
  		local dstArray = {};
  		local foundList = {};
  		foundList[cfg.project.name:lower()] = true;

  		--First, follow the uses recursively.
  		getprojrec(dstArray, foundList, cfg, cfgname, "uses", false);

  		--Next, go through all of the usage projects and recursively get their links.
  		--But only if they're not already there. Get the links as linkage-only.
  		local linkArray = {};
  		for prjName, prjEntry in pairs(dstArray) do
  			getprojrec(linkArray, foundList, prjEntry.usageProj.__configs[cfgname], cfgname,
  				"links", true);
  		end

  		--Copy from linkArray into dstArray.
  		for prjName, prjEntry in pairs(linkArray) do
  			dstArray[prjName] = prjEntry;
  		end

  		return dstArray;
  	end


  	local function isnameofproj(cfg, strName)
  		local sln = cfg.project.solution;
  		local strTest = strName:lower();
  		for prjIx, prj in ipairs(sln.projects) do
  			if (prj.name:lower() == strTest) then
  				return true;
  			end
  		end

  		return false;
  	end


  --
  -- Copies the field from dstCfg to srcCfg.
  --
  	local function copydependentfield(srcCfg, dstCfg, strSrcField)
  		local srcField = premake.fields[strSrcField];
  		local strDstField = strSrcField;

  		if type(srcCfg[strSrcField]) == "table" then
  			--handle paths.
  			if (srcField.kind == "dirlist" or srcField.kind == "filelist") and
  				(not keeprelative[strSrcField]) then
  				for i,p in ipairs(srcCfg[strSrcField]) do
  					table.insert(dstCfg[strDstField],
  						path.rebase(p, srcCfg.project.location, dstCfg.project.location))
  				end
  			else
  				if(strSrcField == "links") then
  					for i,p in ipairs(srcCfg[strSrcField]) do
  						if(not isnameofproj(dstCfg, p)) then
  							table.insert(dstCfg[strDstField], p)
  						else
  							printf("Failed to copy '%s' from proj '%s'.",
  								p, srcCfg.project.name);
  						end
  					end
  				else
  					for i,p in ipairs(srcCfg[strSrcField]) do
  						table.insert(dstCfg[strDstField], p)
  					end
  				end
  			end
  		else
  			if(srcField.kind == "path" and (not keeprelative[strSrcField])) then
  				dstCfg[strDstField] = path.rebase(srcCfg[strSrcField],
  					prj.location, dstCfg.project.location);
  			else
  				dstCfg[strDstField] = srcCfg[strSrcField];
  			end
  		end
  	end


  --
  -- This function will take the list of project entries and apply their usage project data
  -- to the given configuration. It will copy compiling information for the projects that are
  -- not listed as linkage-only. It will copy the linking information for projects only if
  -- the source project is not a static library. It won't copy linking information
  -- if the project is in this solution; instead it will add that project to the configuration's
  -- links field, expecting that Premake will handle the rest.
  --
  	local function copyusagedata(cfg, cfgname, linkToProjs)
  		local myPrj = cfg.project;
  		local bIsStaticLib = (getCfgKind(cfg) == "StaticLib");

  		for prjName, prjEntry in pairs(linkToProjs) do
  			local srcPrj = prjEntry.usageProj;
  			local srcCfg = srcPrj.__configs[cfgname];

  			for name, field in pairs(premake.fields) do
  				if(srcCfg[name]) then
  					if(field.usagecopy) then
  						if(not prjEntry.bLinkageOnly) then
  							copydependentfield(srcCfg, cfg, name)
  						end
  					elseif(field.linkagecopy) then
  						--Copy the linkage data if we're building a non-static thing
  						--and this is a pure usage project. If it's not pure-usage, then
  						--we will simply put the project's name in the links field later.
  						if((not bIsStaticLib) and (not prjEntry.proj)) then
  							copydependentfield(srcCfg, cfg, name)
  						end
  					end
  				end
  			end

  			if((not bIsStaticLib) and prjEntry.proj) then
  				table.insert(cfg.links, prjEntry.proj.name);
  			end
  		end
  	end


--
-- Main function, controls the process of flattening the configurations.
--

	function premake.bake.buildconfigs()

		-- convert project path fields to be relative to project location
		for sln in premake.solution.each() do
			for _, prj in ipairs(sln.projects) do
				prj.location = prj.location or sln.location or prj.basedir
				adjustpaths(prj.location, prj)
				for _, blk in ipairs(prj.blocks) do
					adjustpaths(prj.location, blk)
				end
			end
			sln.location = sln.location or sln.basedir
		end

		-- collapse configuration blocks, so that there is only one block per build
		-- configuration/platform pair, filtered to the current operating environment
		for sln in premake.solution.each() do
			local basis = collapse(sln)
			for _, prj in ipairs(sln.projects) do
				prj.__configs = collapse(prj, basis)
				for _, cfg in pairs(prj.__configs) do
					bake.postprocess(prj, cfg)
				end
			end
		end

		-- This loop finds the projects that a configuration is connected to
		-- via its "uses" field. It will then copy any usage project information from that
		-- usage project to the configuration in question.
		for sln in premake.solution.each() do
			for prjIx, prj in ipairs(sln.projects) do
				if(not prj.usage) then
					for cfgname, cfg in pairs(prj.__configs) do
						local usesPrjs = getprojectsconnections(cfg, cfgname);
						copyusagedata(cfg, cfgname, usesPrjs)
					end
				end
			end
		end

		-- Remove all usage projects.
		for sln in premake.solution.each() do
			local removeList = {};
			for index, prj in ipairs(sln.projects) do
				if(prj.usage) then
					table.insert(removeList, 1, index); --Add in reverse order.
				end
			end

			for _, index in ipairs(removeList) do
				table.remove(sln.projects, index);
			end
		end

		-- assign unique object directories to each configuration
		builduniquedirs()

		-- walk it again and build the targets and unique directories
		buildtargets(cfg)

	end


--
-- Post-process a project configuration, applying path fix-ups and other adjustments
-- to the "raw" setting data pulled from the project script.
--
-- @param prj
--    The project object which contains the configuration.
-- @param cfg
--    The configuration object to be fixed up.
--

	function premake.bake.postprocess(prj, cfg)
		cfg.project   = prj
		cfg.shortname = premake.getconfigname(cfg.name, cfg.platform, true)
		cfg.longname  = premake.getconfigname(cfg.name, cfg.platform)

		-- set the project location, if not already set
		cfg.location = cfg.location or cfg.basedir

		-- figure out the target system
		local platform = premake.platforms[cfg.platform]
		if platform.iscrosscompiler then
			cfg.system = cfg.platform
		else
			cfg.system = os.get()
		end

		-- adjust the kind as required by the target system
		if cfg.kind == "SharedLib" and platform.nosharedlibs then
			cfg.kind = "StaticLib"
		end

		-- remove excluded files from the file list
		local removefiles = cfg.removefiles
		if _ACTION == 'gmake' then
			removefiles = table.join(removefiles, cfg.excludes)
		end
		local files = {}
		for _, fname in ipairs(cfg.files) do
			if not table.icontains(removefiles, fname) then
				table.insert(files, fname)
			end
		end
		cfg.files = files

		-- fixup the data
		for name, field in pairs(premake.fields) do
			-- re-key flag fields for faster lookups
			if field.isflags then
				local values = cfg[name]
				for _, flag in ipairs(values) do values[flag] = true end
			end
		end

		-- build configuration objects for all files
		-- TODO: can I build this as a tree instead, and avoid the extra
		-- step of building it later?
		cfg.__fileconfigs = { }
		for _, fname in ipairs(cfg.files) do
			local fcfg = {}

			-- Only do this if the script has called enablefilelevelconfig()
			if premake._filelevelconfig then
				cfg.terms.required = fname:lower()
				for _, blk in ipairs(cfg.project.blocks) do
					-- BK - `iskeywordsmatch` call is super slow for large projects...
					if (premake.iskeywordsmatch(blk.keywords, cfg.terms)) then
						mergeobject(fcfg, blk)
					end
				end
			end

			-- add indexed by name and integer
			-- TODO: when everything is converted to trees I won't need
			-- to index by name any longer
			fcfg.name = fname
			cfg.__fileconfigs[fname] = fcfg
			table.insert(cfg.__fileconfigs, fcfg)
		end
	end
