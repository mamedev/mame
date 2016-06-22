--
-- Name:        ninja_base.lua
-- Purpose:     Define the ninja action.
-- Author:      Stuart Carnie (stuart.carnie at gmail.com)
--

local ninja = premake.ninja

function ninja.esc(value)
	value = value:gsub("%$", "$$") -- TODO maybe there is better way
	value = value:gsub(":", "$:")
	value = value:gsub("\n", "$\n")
	value = value:gsub(" ", "$ ")
	return value
end

-- in some cases we write file names in rule commands directly
-- so we need to propely escape them
function ninja.shesc(value)
	if type(value) == "table" then
		local result = {}
		local n = #value
		for i = 1, n do
			table.insert(result, ninja.shesc(value[i]))
		end
		return result
	end

	if value:find(" ") then
		return "\"" .. value .. "\""
	end

	return value
end

function ninja.list(value)
	if #value > 0 then
		return " " .. table.concat(value, " ")
	else
		return ""
	end
end

-- generate all build files for every project configuration
function ninja.generate_project(prj)
	ninja.generate_cpp(prj)
end

local function innerget(self, key)
	return rawget(getmetatable(self), key) or self.__inner[key]
end

local prj_proxy = { __index = innerget }

local cfg_proxy = { __index = innerget }

function new_prj_proxy(prj)
	prj = prj.project or prj
	
	local v = { __inner = prj }
	
	local __configs = {}
	for key, cfg in pairs(prj.__configs) do
		if key ~= "" then
			__configs[key] = ninja.get_proxy("cfg", cfg)
		else
			__configs[key] = cfg
		end
	end
	v.__configs = __configs
	
	return setmetatable(v, prj_proxy)
end

local function rebasekeys(t, keys, old, new)
	for _,key in ipairs(keys) do
		t[key] = path.rebase(t[key], old, new)
	end
	return t
end

local function rebasearray(t, old, new)
	local res = { }
	for _,f in ipairs(t) do
		table.insert(res, path.rebase(f, old, new))
	end
	return res
end

function new_cfg_proxy(cfg)
	local keys = { "directory", "fullpath", "bundlepath" }
	
	local old = cfg.location
	local new = path.join(cfg.location, cfg.shortname)
	local v = {
		__inner     = cfg,
		location    = new,
		objectsdir  = path.rebase(cfg.objectsdir, old, new),
		buildtarget = rebasekeys(table.deepcopy(cfg.buildtarget), keys, old, new),
		linktarget  = rebasekeys(table.deepcopy(cfg.buildtarget), keys, old, new),
	}
	
	v.files           = rebasearray(cfg.files, old, new)
	v.includedirs     = rebasearray(cfg.includedirs, old, new)
	v.userincludedirs = rebasearray(cfg.userincludedirs, old, new)
	
	return setmetatable(v, cfg_proxy)
end

function cfg_proxy:getprojectfilename(fullpath)
	local name = self.project.name .. ".ninja"
	
	if fullpath ~= nil then
		return path.join(self.location, name)
	end
	
	return name
end

function cfg_proxy:getoutputfilename()
	return path.join(self.buildtarget.directory, self.buildtarget.name)
end

local proxy_cache = { 
	prj = { new = new_prj_proxy }, 
	cfg = { new = new_cfg_proxy },
}

function get_proxy(cache, obj)
	if not cache[obj] then
		cache[obj] = cache.new(obj)
	end
	return cache[obj]
end

function ninja.get_proxy(typ, obj)
	if not proxy_cache[typ] then
		error("invalid proxy type")
	end
	
	return get_proxy(proxy_cache[typ], obj)
end

