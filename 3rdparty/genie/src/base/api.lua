--
-- api.lua
-- Implementation of the solution, project, and configuration APIs.
-- Copyright (c) 2002-2011 Jason Perkins and the Premake project
--

premake.fields = {}


premake.check_paths = false

--
-- Check to see if a value exists in a list of values, using a
-- case-insensitive match. If the value does exist, the canonical
-- version contained in the list is returned, so future tests can
-- use case-sensitive comparisions.
--

	function premake.checkvalue(value, allowed)
		if (allowed) then
			if (type(allowed) == "function") then
				return allowed(value)
			else
				for _,v in ipairs(allowed) do
					if (value:lower() == v:lower()) then
						return v
					end
				end
				return nil, "invalid value '" .. value .. "'"
			end
		else
			return value
		end
	end



--
-- Retrieve the current object of a particular type from the session. The
-- type may be "solution", "container" (the last activated solution or
-- project), or "config" (the last activated configuration). Returns the
-- requested container, or nil and an error message.
--

	function premake.getobject(t)
		local container

		if (t == "container" or t == "solution") then
			container = premake.CurrentContainer
		else
			container = premake.CurrentConfiguration
		end

		if t == "solution" then
			if typex(container) == "project" then
				container = container.solution
			end
			if typex(container) ~= "solution" then
				container = nil
			end
		end

		local msg
		if (not container) then
			if (t == "container") then
				msg = "no active solution or project"
			elseif (t == "solution") then
				msg = "no active solution"
			else
				msg = "no active solution, project, or configuration"
			end
		end

		return container, msg
	end



--
-- Adds values to an array field.
--
-- @param obj
--    The object containing the field.
-- @param fieldname
--    The name of the array field to which to add.
-- @param values
--    The value(s) to add. May be a simple value or an array
--    of values.
-- @param allowed
--    An optional list of allowed values for this field.
-- @return
--    The value of the target field, with the new value(s) added.
--

	function premake.setarray(obj, fieldname, value, allowed)
		obj[fieldname] = obj[fieldname] or {}

		local function add(value, depth)
			if type(value) == "table" then
				for _,v in ipairs(value) do
					add(v, depth + 1)
				end
			else
				value, err = premake.checkvalue(value, allowed)
				if not value then
					error(err, depth)
				end
				table.insert(obj[fieldname], value)
			end
		end

		if value then
			add(value, 5)
		end

		return obj[fieldname]
	end



--
-- Adds table value to array of tables
--
	function premake.settable(obj, fieldname, value, allowed)
		obj[fieldname] = obj[fieldname] or {}
		table.insert(obj[fieldname], value)
		return obj[fieldname]
	end
--
-- Adds values to an array-of-directories field of a solution/project/configuration.
-- `fields` is an array of containers/fieldname pairs to add the results to. All
-- values are converted to absolute paths before being stored.
--
-- Only the result of the first field given is returned.
--

	local function domatchedarray(fields, value, matchfunc)
		local result = { }

		function makeabsolute(value, depth)
			if (type(value) == "table") then
				for _, item in ipairs(value) do
					makeabsolute(item, depth + 1)
				end
			elseif type(value) == "string" then
				if value:find("*") then
					local arr = matchfunc(value);
					if (premake.check_paths) and (#arr == 0) then
						error("Can't find matching files for pattern :" .. value)
					end
					makeabsolute(arr, depth + 1)
				else
					table.insert(result, path.getabsolute(value))
				end
			else
				error("Invalid value in list: expected string, got " .. type(value), depth)
			end
		end

		makeabsolute(value, 3)

		local retval = {}

		for index, field in ipairs(fields) do
			local ctype = field[1]
			local fieldname = field[2]
			local array = premake.setarray(ctype, fieldname, result)

			if index == 1 then
				retval = array
			end
		end

		return retval
	end

	function premake.setdirarray(fields, value)
		return domatchedarray(fields, value, os.matchdirs)
	end

	function premake.setfilearray(fields, value)
		return domatchedarray(fields, value, os.matchfiles)
	end


--
-- Adds values to a key-value field of a solution/project/configuration. `ctype`
-- specifies the container type (see premake.getobject) for the field.
--

	function premake.setkeyvalue(ctype, fieldname, values)
		local container, err = premake.getobject(ctype)
		if not container then
			error(err, 4)
		end

		if not container[fieldname] then
			container[fieldname] = {}
		end

		if type(values) ~= "table" then
			error("invalid value; table expected", 4)
		end

		local field = container[fieldname]

		for key,value in pairs(values) do
			if not field[key] then
				field[key] = {}
			end
			table.insertflat(field[key], value)
		end

		return field
	end


--
-- Set a new value for a string field of a solution/project/configuration. `ctype`
-- specifies the container type (see premake.getobject) for the field.
--

	function premake.setstring(ctype, fieldname, value, allowed)
		-- find the container for this value
		local container, err = premake.getobject(ctype)
		if (not container) then
			error(err, 4)
		end

		-- if a value was provided, set it
		if (value) then
			value, err = premake.checkvalue(value, allowed)
			if (not value) then
				error(err, 4)
			end

			container[fieldname] = value
		end

		return container[fieldname]
	end

--
-- Removes a value from an array
--
	function premake.remove(fieldname, value)
		local cfg = premake.CurrentConfiguration
		cfg.removes = cfg.removes or {}
		cfg.removes[fieldname] = premake.setarray(cfg.removes, fieldname, value)
	end


--
-- The getter/setter implemention.
--

	local function accessor(name, value)
		local kind    = premake.fields[name].kind
		local scope   = premake.fields[name].scope
		local allowed = premake.fields[name].allowed

		if (kind == "string" or kind == "path") and value then
			if type(value) ~= "string" then
				error("string value expected", 3)
			end
		end

		-- find the container for the value
		local container, err = premake.getobject(scope)
		if (not container) then
			error(err, 3)
		end

		if kind == "string" then
			return premake.setstring(scope, name, value, allowed)
		elseif kind == "path" then
			if value then value = path.getabsolute(value) end
			return premake.setstring(scope, name, value)
		elseif kind == "list" then
			return premake.setarray(container, name, value, allowed)
		elseif kind == "table" then
			return premake.settable(container, name, value, allowed)
		elseif kind == "dirlist" then
			return premake.setdirarray({{container, name}}, value)
		elseif kind == "filelist" or kind == "absolutefilelist" then
			-- HACK: If we're adding files, we should also add them to the project's
			-- `allfiles` field. This is to support files being added per config.
			local fields = {{container, name}}
			if name == "files" then
				local prj, err = premake.getobject("container")
				if (not prj) then
					error(err, 2)
				end
				-- The first config block for the project is always the project's
				-- global config. See the `project` function.
				table.insert(fields, {prj.blocks[1], "allfiles"})
			end
			return premake.setfilearray(fields, value)
		elseif kind == "keyvalue" or kind == "keypath" then
			return premake.setkeyvalue(scope, name, value)
		end
	end

--
-- Project object constructors.
--

	function configuration(terms)
		if not terms then
			return premake.CurrentConfiguration
		end

		local container, err = premake.getobject("container")
		if (not container) then
			error(err, 2)
		end

		local cfg = { }
		cfg.terms = table.flatten({terms})

		table.insert(container.blocks, cfg)
		premake.CurrentConfiguration = cfg

		-- create a keyword list using just the indexed keyword items. This is a little
		-- confusing: "terms" are what the user specifies in the script, "keywords" are
		-- the Lua patterns that result. I'll refactor to better names.
		cfg.keywords = { }
		for _, word in ipairs(cfg.terms) do
			table.insert(cfg.keywords, path.wildcards(word):lower())
		end

		-- initialize list-type fields to empty tables
		for name, field in pairs(premake.fields) do
			if (field.kind ~= "string" and field.kind ~= "path") then
				cfg[name] = { }
			end
		end

		return cfg
	end

--
-- Creates a single group element
-- @param name
--    the display name of the group
-- @param sln
--    the solution to add the group to
-- @param parent
--    the parent of this group, can be nil
-- @param inpath
--    the full path to this group, lower case only
-- @returns
--    the group object
--

	local function creategroup(name, sln, curpath, parent, inpath)

		local group = {}

		-- attach a type
		setmetatable(group, {
			__type = "group"
		})

		-- add to master list keyed by both name and index
		table.insert(sln.groups, group)
		sln.groups[inpath] = group

		-- add to the parent's child list
		if parent ~= nil then
			table.insert(parent.groups, group)
		end

		group.solution = sln
		group.name = name
		group.uuid = os.uuid(curpath)
		group.parent = parent
		group.projects = { }
		group.groups = { }
		return group
	end

--
-- Creates all groups that exist in a given group hierarchy
-- @param inpath
--    the path to create groups from (i.e. "Examples/Simple")
-- @param sln
--    the solution to add the groups to
-- @returns
--    the group object for the deepest folder
--

	local function creategroupsfrompath(inpath, sln)
		if inpath == nil then return nil end

		-- Split groups in hierarchy
		inpath = path.translate(inpath, "/")
		local groups = string.explode(inpath, "/")

		-- Each part of the hierarchy may already exist
		local curpath = ""
		local lastgroup = nil
		for i, v in ipairs(groups) do
			curpath = curpath .. "/" .. v:lower()

			local group = sln.groups[curpath]
			if group == nil then
				group = creategroup(v, sln, curpath, lastgroup, curpath)
			end
			lastgroup = group
		end

		return lastgroup
	end

	local function createproject(name, sln, isUsage)
		local prj = {}

		-- attach a type
		setmetatable(prj, {
			__type = "project",
		})

		-- add to master list keyed by both name and index
		table.insert(sln.projects, prj)
		if(isUsage) then
			--If we're creating a new usage project, and there's already a project
			--with our name, then set us as the usage project for that project.
			--Otherwise, set us as the project in that slot.
			if(sln.projects[name]) then
				sln.projects[name].usageProj = prj;
			else
				sln.projects[name] = prj
			end
		else
			--If we're creating a regular project, and there's already a project
			--with our name, then it must be a usage project. Set it as our usage project
			--and set us as the project in that slot.
			if(sln.projects[name]) then
				prj.usageProj = sln.projects[name];
			end

			sln.projects[name] = prj
		end

		local group = creategroupsfrompath(premake.CurrentGroup, sln)

		if group ~= nil then
			table.insert(group.projects, prj)
		end

		prj.solution       = sln
		prj.name           = name
		prj.basedir        = os.getcwd()
		prj.uuid           = os.uuid(prj.name)
		prj.blocks         = { }
		prj.usage          = isUsage
		prj.group          = group

		return prj;
	end

	function usage(name)
		if (not name) then
			--Only return usage projects.
			if(typex(premake.CurrentContainer) ~= "project") then return nil end
			if(not premake.CurrentContainer.usage) then return nil end
			return premake.CurrentContainer
		end

		-- identify the parent solution
		local sln
		if (typex(premake.CurrentContainer) == "project") then
			sln = premake.CurrentContainer.solution
		else
			sln = premake.CurrentContainer
		end
		if (typex(sln) ~= "solution") then
			error("no active solution", 2)
		end

		-- if this is a new project, or the project in that slot doesn't have a usage, create it
		if((not sln.projects[name]) or
			((not sln.projects[name].usage) and (not sln.projects[name].usageProj))) then
			premake.CurrentContainer = createproject(name, sln, true)
		else
			premake.CurrentContainer = iff(sln.projects[name].usage,
			sln.projects[name], sln.projects[name].usageProj)
		end

		-- add an empty, global configuration to the project
		configuration { }

		return premake.CurrentContainer
	end

	function project(name)
		if (not name) then
			--Only return non-usage projects
			if(typex(premake.CurrentContainer) ~= "project") then return nil end
			if(premake.CurrentContainer.usage) then return nil end
			return premake.CurrentContainer
		end

		-- identify the parent solution
		local sln
		if (typex(premake.CurrentContainer) == "project") then
			sln = premake.CurrentContainer.solution
		else
			sln = premake.CurrentContainer
		end
		if (typex(sln) ~= "solution") then
			error("no active solution", 2)
		end

		-- if this is a new project, or the old project is a usage project, create it
		if((not sln.projects[name]) or sln.projects[name].usage) then
			premake.CurrentContainer = createproject(name, sln)
		else
			premake.CurrentContainer = sln.projects[name];
		end

		-- add an empty, global configuration to the project
		configuration { }

		return premake.CurrentContainer
	end


	function solution(name)
		if not name then
			if typex(premake.CurrentContainer) == "project" then
				return premake.CurrentContainer.solution
			else
				return premake.CurrentContainer
			end
		end

		premake.CurrentContainer = premake.solution.get(name)
		if (not premake.CurrentContainer) then
			premake.CurrentContainer = premake.solution.new(name)
		end

		-- add an empty, global configuration
		configuration { }

		return premake.CurrentContainer
	end


	function group(name)
		if not name then
			return premake.CurrentGroup
		end
		premake.CurrentGroup = name

		return premake.CurrentGroup
	end

	function importvsproject(location)
		if string.find(_ACTION, "vs") ~= 1 then
			error("Only available for visual studio actions")
		end

		sln, err = premake.getobject("solution")
		if not sln then
			error(err)
		end

		local group = creategroupsfrompath(premake.CurrentGroup, sln)

		local project = {}
		project.location = location
		project.group = group
		project.flags = {}

		table.insert(sln.importedprojects, project)
    end

--
-- Define a new action.
--
-- @param a
--    The new action object.
--

	function newaction(a)
		premake.action.add(a)
	end


--
-- Define a new option.
--
-- @param opt
--    The new option object.
--

	function newoption(opt)
		premake.option.add(opt)
	end


--
-- Enable file level configuration
-- this makes project generation slower for large projects
--

	function enablefilelevelconfig()
		premake._filelevelconfig = true
	end



--
-- Define a new API field.
-- Build the getter/setter functions from its metadata.
--
-- @param a
--    The new field/API object.
--

function newapifield(field)
	premake.fields[field.name] = field

	_G[field.name] = function(value)
		return accessor(field.name, value)
	end

	-- list value types get a remove() call too
	if field.kind == "list"
	or field.kind == "dirlist"
	or field.kind == "filelist"
	or field.kind == "absolutefilelist"
	then
		if  field.name ~= "removefiles"
		and field.name ~= "files" then
			_G["remove"..field.name] = function(value)
				premake.remove(field.name, value)
			end
		end
	end
end

--
-- This builds the API functions from their metadata.
--

	newapifield {
		name  = "archivesplit_size",
		kind  = "string",
		scope = "config",
	}

	newapifield {
		name  = "basedir",
		kind  = "path",
		scope = "container",
	}

	newapifield {
		name  = "buildaction",
		kind  = "string",
		scope = "config",
		allowed = {
			"Compile",
			"Copy",
			"Embed",
			"None"
		}
	}

	newapifield {
		name  = "buildoptions",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "buildoptions_asm",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "buildoptions_c",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "buildoptions_cpp",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "buildoptions_objc",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "buildoptions_objcpp",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "buildoptions_vala",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "clrreferences",
		kind = "list",
		scope = "container",
	}

	newapifield {
		name  = "configurations",
		kind  = "list",
		scope = "solution",
	}

	newapifield {
		name  = "custombuildtask",
		kind  = "table",
		scope = "config",
	}

	newapifield {
		name  = "debugcmd",
		kind = "string",
		scope = "config",
	}

	newapifield {
		name  = "debugargs",
		kind = "list",
		scope = "config",
	}

	newapifield {
		name  = "debugdir",
		kind = "path",
		scope = "config",
	}

	newapifield {
		name  = "debugenvs" ,
		kind = "list",
		scope = "config",
	}

	newapifield {
		name  = "defines",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "deploymentoptions",
		kind  = "list",
		scope = "config",
		usagecopy = true,
	}

	newapifield {
		name  = "dependency",
		kind  = "table",
		scope = "config",
	}

	newapifield {
		name  = "deploymode",
		kind = "string",
		scope = "config",
	}

	newapifield {
		name  = "excludes",
		kind  = "filelist",
		scope = "config",
	}

	newapifield {
		name  = "forcenative",
		kind = "filelist",
		scope = "config",
	}

	newapifield {
		name  = "nopch",
		kind  = "filelist",
		scope = "config",
	}

	newapifield {
		name  = "files",
		kind  = "filelist",
		scope = "config",
	}

	newapifield {
		name  = "removefiles",
		kind  = "filelist",
		scope = "config",
	}

	newapifield {
		name  = "flags",
		kind  = "list",
		scope = "config",
		isflags = true,
		usagecopy = true,
		allowed = function(value)
			local allowed_flags = {
				AntBuildDebuggable = 1,
				C7DebugInfo = 1,
				Cpp11 = 1,
				Cpp14 = 1,
				Cpp17 = 1,
				Cpp20 = 1,
				CppLatest = 1,
				DebugEnvsDontMerge = 1,
				DebugEnvsInherit = 1,
				DeploymentContent = 1,
				EnableMinimalRebuild = 1,
				EnableSSE = 1,
				EnableSSE2 = 1,
				EnableAVX = 1,
				EnableAVX2 = 1,
				PedanticWarnings = 1,
				ExtraWarnings = 1,
				FatalWarnings = 1,
				FloatFast = 1,
				FloatStrict = 1,
				FullSymbols = 1,
				GenerateMapFiles = 1,
				Hotpatchable = 1,
				LinkSupportCircularDependencies = 1,
				Managed = 1,
				MinimumWarnings = 1,
				NativeWChar = 1,
				No64BitChecks = 1,
				NoBufferSecurityCheck = 1,
				NoEditAndContinue = 1,
				NoExceptions = 1,
				NoFramePointer = 1,
				NoImportLib = 1,
				NoIncrementalLink = 1,
				NoJMC = 1,
				NoManifest = 1,
				NoMultiProcessorCompilation = 1,
				NoNativeWChar = 1,
				NoOptimizeLink = 1,
				NoPCH = 1,
				NoRTTI = 1,
				NoRuntimeChecks = 1,
				NoWinMD = 1,    -- explicitly disables Windows Metadata
				NoWinRT = 1,    -- explicitly disables Windows Runtime Extension
				FastCall = 1,
				StdCall = 1,
				SingleOutputDir = 1,
				ObjcARC = 1,
				Optimize = 1,
				OptimizeSize = 1,
				OptimizeSpeed = 1,
				DebugRuntime = 1,
				ReleaseRuntime = 1,
				SEH = 1,
				StaticRuntime = 1,
				Symbols = 1,
				Unicode = 1,
				UnitySupport = 1,
				Unsafe = 1,
				UnsignedChar = 1,
				UseFullPaths = 1,
				UseLDResponseFile = 1,
				UseObjectResponseFile = 1,
				WinMain = 1
			}

			local englishToAmericanSpelling =
			{
				nooptimiselink = 'nooptimizelink',
				optimise = 'optimize',
				optimisesize = 'optimizesize',
				optimisespeed = 'optimizespeed',
			}

			local lowervalue = value:lower()
			lowervalue = englishToAmericanSpelling[lowervalue] or lowervalue
			for v, _ in pairs(allowed_flags) do
				if v:lower() == lowervalue then
					return v
				end
			end
			return nil, "invalid flag"
		end,
	}

	newapifield {
		name  = "framework",
		kind = "string",
		scope = "container",
		allowed = {
			"1.0",
			"1.1",
			"2.0",
			"3.0",
			"3.5",
			"4.0",
			"4.5",
			"4.5.1",
			"4.5.2",
			"4.6",
			"4.6.1",
			"4.6.2",
		}
	}

	newapifield {
		name  = "iostargetplatformversion",
		kind  = "string",
		scope = "project",
	}

	newapifield {
		name  = "macostargetplatformversion",
		kind  = "string",
		scope = "project",
	}

	newapifield {
		name  = "tvostargetplatformversion",
		kind  = "string",
		scope = "project",
	}

	newapifield {
		name  = "visionostargetplatformversion",
		kind  = "string",
		scope = "project",
	}

	newapifield {
		name  = "windowstargetplatformversion",
		kind  = "string",
		scope = "project",
	}

	newapifield {
		name  = "windowstargetplatformminversion",
		kind = "string",
		scope = "project",
	}

	newapifield {
		name  = "forcedincludes",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "imagepath",
		kind = "path",
		scope = "config",
	}

	newapifield {
		name  = "imageoptions",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "implibdir",
		kind  = "path",
		scope = "config",
	}

	newapifield {
		name  = "implibextension",
		kind  = "string",
		scope = "config",
	}

	newapifield {
		name  = "implibname",
		kind  = "string",
		scope = "config",
	}

	newapifield {
		name  = "implibprefix",
		kind  = "string",
		scope = "config",
	}

	newapifield {
		name  = "implibsuffix",
		kind  = "string",
		scope = "config",
	}

	newapifield {
		name  = "includedirs",
		kind  = "dirlist",
		scope = "config",
		usagecopy = true,
	}

	newapifield {
		name  = "systemincludedirs",
		kind  = "dirlist",
		scope = "config",
		usagecopy = true,
	}

	newapifield {
		name  = "userincludedirs",
		kind  = "dirlist",
		scope = "config",
		usagecopy = true,
	}

	newapifield {
		name  = "usingdirs",
		kind  = "dirlist",
		scope = "config",
		usagecopy = true,
	}

	newapifield {
		name  = "kind",
		kind  = "string",
		scope = "config",
		allowed = {
			"ConsoleApp",
			"WindowedApp",
			"StaticLib",
			"SharedLib",
			"Bundle",
		}
	}

	newapifield {
		name  = "language",
		kind  = "string",
		scope = "container",
		allowed = {
			"C",
			"C++",
			"C#",
			"Vala",
			"Swift",
		}
	}

	newapifield {
		name  = "libdirs",
		kind  = "dirlist",
		scope = "config",
		linkagecopy = true,
	}

	newapifield {
		name  = "linkoptions",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "links",
		kind  = "list",
		scope = "config",
		allowed = function(value)
			-- if library name contains a '/' then treat it as a path to a local file
			if value:find('/', nil, true) then
				value = path.getabsolute(value)
			end
			return value
		end,
		linkagecopy = true,
		mergecopiestotail = true,
	}

	newapifield {
		name  = "location",
		kind  = "path",
		scope = "container",
	}

	newapifield {
		name  = "makesettings",
		kind = "list",
		scope = "config",
	}


	newapifield {
		name  = "messageskip",
		kind  = "list",
		scope = "solution",
		isflags = true,
		usagecopy = true,
		allowed = function(value)
			local allowed_messages = {
				SkipCreatingMessage = 1,
				SkipBuildingMessage = 1,
				SkipCleaningMessage = 1,
			}

			local lowervalue = value:lower()
			for v, _ in pairs(allowed_messages) do
				if v:lower() == lowervalue then
					return v
				end
			end
			return nil, "invalid message to skip"
		end,
	}

	newapifield {
		name  = "msgarchiving",
		kind  = "string",
		scope = "config",
	}

	newapifield {
		name  = "msgcompile",
		kind  = "string",
		scope = "config",
	}

	newapifield {
		name  = "msgprecompile",
		kind  = "string",
		scope = "config",
	}

	newapifield {
		name  = "msgcompile_objc",
		kind  = "string",
		scope = "config",
	}

	newapifield {
		name  = "msgresource",
		kind  = "string",
		scope = "config",
	}

	newapifield {
		name  = "msglinking",
		kind  = "string",
		scope = "config",
	}

	newapifield {
		name  = "objdir",
		kind  = "path",
		scope = "config",
	}

	newapifield {
		name  = "options",
		kind  = "list",
		scope = "container",
		isflags = true,
		usagecopy = true,
		allowed = function(value)
			local allowed_options = {
				ForceCPP = 1,
				ArchiveSplit = 1,
				SkipBundling = 1,
				XcodeLibrarySchemes = 1,
				XcodeSchemeNoConfigs = 1,
			}

			local lowervalue = value:lower()
			for v, _ in pairs(allowed_options) do
				if v:lower() == lowervalue then
					return v
				end
			end
			return nil, "invalid option"
		end,
	}

	newapifield {
		name  = "pchheader",
		kind  = "string",
		scope = "config",
	}

	newapifield {
		name  = "pchsource",
		kind  = "path",
		scope = "config",
	}

	newapifield {
		name  = "platforms",
		kind  = "list",
		scope = "solution",
		allowed = table.keys(premake.platforms),
	}

	newapifield {
		name  = "postbuildcommands",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "prebuildcommands",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "postcompiletasks",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "prelinkcommands",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "propertysheets",
		kind  = "dirlist",
		scope = "config",
	}

	newapifield {
		name  = "pullmappingfile",
		kind  = "path",
		scope = "config",
	}

	newapifield {
		name  = "applicationdatadir",
		kind  = "path",
		scope = "config",
	}

	newapifield {
		name  = "finalizemetasource",
		kind  = "path",
		scope = "config",
	}

	newapifield {
		name  = "resdefines",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "resincludedirs",
		kind  = "dirlist",
		scope = "config",
	}

	newapifield {
		name  = "resoptions",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "sdkreferences",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "startproject",
		kind  = "string",
		scope = "solution",
	}

	newapifield {
		name  = "targetdir",
		kind  = "path",
		scope = "config",
	}

	newapifield {
		name  = "targetsubdir",
		kind  = "string",
		scope = "config",
	}

	newapifield {
		name  = "targetextension",
		kind  = "string",
		scope = "config",
	}

	newapifield {
		name  = "targetname",
		kind  = "string",
		scope = "config",
	}

	newapifield {
		name  = "targetprefix",
		kind  = "string",
		scope = "config",
	}

	newapifield {
		name  = "targetsuffix",
		kind  = "string",
		scope = "config",
	}

	newapifield {
		name  = "trimpaths",
		kind = "dirlist",
		scope = "config",
	}

	newapifield {
		name  = "uuid",
		kind  = "string",
		scope = "container",
		allowed = function(value)
			local ok = true
			if (#value ~= 36) then ok = false end
			for i=1,36 do
				local ch = value:sub(i,i)
				if (not ch:find("[ABCDEFabcdef0123456789-]")) then ok = false end
			end
			if (value:sub(9,9) ~= "-")   then ok = false end
			if (value:sub(14,14) ~= "-") then ok = false end
			if (value:sub(19,19) ~= "-") then ok = false end
			if (value:sub(24,24) ~= "-") then ok = false end
			if (not ok) then
				return nil, "invalid UUID"
			end
			return value:upper()
		end
	}

	newapifield {
		name  = "uses",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "vapidirs",
		kind  = "dirlist",
		scope = "config",
	}

	newapifield {
		name  = "vpaths",
		kind = "keypath",
		scope = "container",
	}

	newapifield {
		name  = "vsimportreferences",
		kind = "filelist",
		scope = "container",
	}

	newapifield {
		name  = "dpiawareness",
		kind = "string",
		scope = "config",
		allowed = {
			"None",
			"High",
			"HighPerMonitor",
		}
	}

	newapifield {
		name  = "xcodeprojectopts",
		kind = "table",
		scope = "config",
	}

	newapifield {
		name  = "xcodetargetopts",
		kind = "table",
		scope = "config",
	}

	newapifield {
		name  = "xcodescriptphases",
		kind  = "table",
		scope = "config",
	}

	newapifield {
		name  = "xcodecopyresources",
		kind  = "table",
		scope = "project",
	}

	newapifield {
		name  = "xcodecopyframeworks",
		kind  = "filelist",
		scope = "project",
	}

	newapifield {
		name  = "wholearchive",
		kind  = "list",
		scope = "config",
	}

		-- swift options
	newapifield {
		name  = "swiftmodulemaps",
		kind  = "filelist",
		scope = "config",
	}

	newapifield {
		name  = "buildoptions_swift",
		kind  = "list",
		scope = "config",
	}

	newapifield {
		name  = "linkoptions_swift",
		kind  = "list",
		scope = "config",
	}

		-- Tegra Android options
	newapifield {
		name  = "androidtargetapi",
		kind = "string",
		scope = "config",
	}

	newapifield {
		name  = "androidminapi",
		kind = "string",
		scope = "config",
	}

	newapifield {
		name  = "androidarch",
		kind = "string",
		scope = "config",
		allowed = {
			"armv7-a",
			"armv7-a-hard",
			"arm64-v8a",
			"x86",
			"x86_64",
		}
	}

	newapifield {
		name  = "androidndktoolchainversion",
		kind = "string",
		scope = "config",
	}

	newapifield {
		name  = "androidstltype",
		kind = "string",
		scope = "config",
	}

	newapifield {
		name  = "androidcppstandard",
		kind = "string",
		scope = "config",
		allowed = {
			"c++98",
			"c++11",
			"c++1y",
			"gnu++98",
			"gnu++11",
			"gnu++1y",
		}
	}

	newapifield {
		name  = "androidlinker",
		kind = "string",
		scope = "config",
		allowed = {
			"bfd",
			"gold",
		}
	}

	newapifield {
		name  = "androiddebugintentparams",
		kind = "list",
		scope = "config",
	}

	newapifield {
		name  = "antbuildjavasourcedirs",
		kind = "dirlist",
		scope = "config",
	}

	newapifield {
		name  = "antbuildjardirs",
		kind = "dirlist",
		scope = "config",
	}

	newapifield {
		name  = "antbuildjardependencies",
		kind = "list",
		scope = "config",
	}

	newapifield {
		name  = "antbuildnativelibdirs",
		kind = "dirlist",
		scope = "config",
	}

	newapifield {
		name  = "antbuildnativelibdependencies",
		kind = "list",
		scope = "config",
	}

	newapifield {
		name  = "antbuildassetsdirs",
		kind = "dirlist",
		scope = "config",
	}

	newapifield {
		name  = "postsolutioncallbacks",
		kind  = "list",
		scope = "solution",
	}

	newapifield {
		name  = "postprojectcallbacks",
		kind  = "list",
		scope = "project",
	}

--
-- End of API
--
