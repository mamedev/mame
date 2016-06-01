if #arg ~= 2 then
	print("usage: lua sort_hiscore.lua hiscore.dat mame.lst")
	return
end

local datfile = io.open(arg[1])

if not datfile then
	return
end

local entries = {{}}
local namelist = false
local comment = false
local entry = entries[1]
for line in datfile:lines() do
	local function next_entry()
		entries[#entries + 1] = {}
		comment = false
		namelist = false
		return entries[#entries]
	end

	local function additem(table, item)
		if not entry[table] then
			entry[table] = {}
		end
		entry[table][#entry[table] + 1 ] = item
	end

	local function clean(data)
		return data:match("^([^%s;]+)")
	end

	if line:match("^%w") then
		if not comment and not namelist then
			entry = next_entry()
		end
		namelist = true
	else
		namelist = false
	end
	if line:match("^%s*$") then
		entry = next_entry()
	end
	line = line:match("^%s*.-%s*$")
	local char = line:sub(1,1)
	if char == ";" then
		additem("comment", line)
		comment = true
	else
		comment = false
	end
	if char:match("%w") then
		additem("name", clean(line):sub(1, -2))
	end
	if char == "@" then
		additem("data", clean(line))
	end
end

datfile:close()

local lstfile = io.open(arg[2])

local list = {}
local src = "error"
for line in lstfile:lines() do
	if not line:match("^[%s/*]") then
		if line:sub(1,1) == "@" then
			src = line:match("^@source:(.*)")
		else
			local set = line:match("^([^%s/]*)")
			if set and set ~= "" then
				list[set] = src
			end
		end
	end
end

lstfile:close()

local sorted = {}
local comments = 0

for num, entry in ipairs(entries) do
	if not entry.name then
		if entry.comment then
			if entry.comment[1]:sub(2,4) ~= "@s:" then
				comments = comments + 1
				table.insert(sorted, comments, entry)
			end
		end
	else
		table.sort(entry.name)
		entry.src = "source not found"
		for num, name in ipairs(entry.name) do
			name = name:match("[^,]*")
			if not list[name] then
				entry.name[num] = entry.name[num] .. ":  ; missing"
			else
				entry.name[num] = entry.name[num] .. ":"
				entry.src = list[name]
			end
		end
		sorted[#sorted + 1] = entry
	end
end

table.sort(sorted, function(a,b) if a.src and b.src then return a.src < b.src else return false end end)

src = "error";

for num, entry in ipairs(sorted) do
	local function printall(table)
		for num, str in ipairs(table) do
			print(str)
		end
	end
	if entry.src and entry.src ~= src then
		print(";@s:" .. entry.src .. "\n")
		src = entry.src
	end
	if entry.comment then
		printall(entry.comment)
	end
	if entry.name then
		printall(entry.name)
		printall(entry.data)
	end
	print("\n")
end
