Set_mt = {}

function Set(t)
	local set = {}
	for k, l in ipairs(t) do 
		set[l] = true 
	end
	setmetatable(set, Set_mt)
	return set
end

function Set_mt.union(a, b)
	local res = Set{}
	for k in pairs(a) do res[k] = true end
	for k in pairs(b) do res[k] = true end
	return res
end

function Set_mt.intersection(a, b)
	local res = Set{}
	for k in pairs(a) do
		res[k] = b[k]
	end
	return res
end

local function get_cache(a)
	if not rawget(a, "__cache") then
		rawset(a, "__cache", Set_mt.totable(a))
	end
	return rawget(a, "__cache")
end

function Set_mt.len(a)
	return #get_cache(a)
end

function Set_mt.implode(a, sep)
	return table.concat(get_cache(a), sep)
end

function Set_mt.totable(a)
	local res = {}
	for k in pairs(a) do
		table.insert(res, k)
	end
	return res
end

function Set_mt.difference(a, b)
	if getmetatable(b) ~= Set_mt then
		if type(b) ~= "table" then
			error(type(b).." is not a Set or table")
		end
		b=Set(b)
	end

	local res = Set{}
	for k in pairs(a) do
		res[k] = not b[k] or nil
	end
	rawset(res, "__cache", nil)
	return res
end

function Set_mt.__index(a, i)
	if type(i) == "number" then
		return get_cache(a)[i]
	end
	return Set_mt[i] or rawget(a, i)
end

Set_mt.__add = Set_mt.union
Set_mt.__mul = Set_mt.intersection
Set_mt.__sub = Set_mt.difference
Set_mt.__len = Set_mt.len
Set_mt.__concat = Set_mt.implode