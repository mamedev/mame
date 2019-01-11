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
		return data:match("^([^%s;]+)"):lower()
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
local sindex = {}
local comments = ""

for num, entry in pairs(entries) do
	if not entry.name then
		if entry.comment then
			if entry.comment[1]:sub(2,4) ~= "@s:" then
				comments = comments .. table.concat(entry.comment, "\n") .. "\n"
			end
		end
	else
		table.sort(entry.name)
		entry.src = "source not found"
		for num, name in pairs(entry.name) do
			name = name:match("[^,]*")
			if not list[name] then
				entry.name[num] = entry.name[num] .. ":  ; missing"
			else
				entry.name[num] = entry.name[num] .. ":"
				entry.src = list[name]
			end
		end
		entry.data = table.concat(entry.data, "\n")
		if entry.comment then
			entry.comment = table.concat(entry.comment, "\n")
		end
		sorted[#sorted + 1] = entry
		if not sindex[entry.src] then
			sindex[entry.src] = {}
		end
		sindex[entry.src][#sorted] = entry
	end
end

for src, entries in pairs(sindex) do
	for num1, entry in pairs(entries) do
		for num2, entry2 in pairs(entries) do
			if entry.name and entry ~= entry2 and entry.data == entry2.data then
				for num3, name in pairs(entry2.name) do
					entry.name[#entry.name + 1] = name
				end
				if entry2.comment then
					if not entry.comment then
						entry.comment = entry2.comment
					elseif entry.comment ~= entry2.comment then
						entry.comment = entry.comment .. "\n" .. entry2.comment
					end
				end
				sorted[num2] = {}
				entries[num2] = {}
			end
		end
	end
end

local nindex = {}

for num1, entry in pairs(sorted) do
	if entry.name then
		for num2, name in pairs(entry.name) do
			local curname = name:match("[^:]*")
			if nindex[curname] then
				if nindex[curname] == entry then
					entry.name[num2] = ""
				else
					print(curname, "duplicate name")
				end
			else
				nindex[curname] = entry
			end
		end
	end
end

-- copyright 2010 Uli Schlachter GPLv2
local function stable_sort(list, comp)
	-- A table could contain non-integer keys which we have to ignore.
	local num = 0
	for k, v in ipairs(list) do
		num = num + 1
	end

	if num <= 1 then
		-- Nothing to do
		return
	end

	-- Sort until everything is sorted :)
	local sorted = false
	local n = num
	while not sorted do
		sorted = true
		for i = 1, n - 1 do
			-- Two equal elements won't be swapped -> we are stable
			if comp(list[i+1], list[i]) then
				local tmp = list[i]
				list[i] = list[i+1]
				list[i+1] = tmp

				sorted = false
			end
		end
		-- The last element is now guaranteed to be in the right spot
		n = n - 1
	end
end
stable_sort(sorted, function(a,b)
	if a.src and b.src then
		return a.src < b.src
	elseif not a.src then
		return true
	else
		return false
	end
end)

local src = "error";

print(comments)

for num, entry in ipairs(sorted) do
	if entry.name then
		if entry.src and entry.src ~= src then
			print(";@s:" .. entry.src .. "\n")
			src = entry.src
		end
		if entry.comment then
			print(entry.comment)
		end
		print((table.concat(entry.name, "\n"):gsub("\n+","\n"):gsub("\n$","")))
		print(entry.data)
		print("\n")
	end
end
