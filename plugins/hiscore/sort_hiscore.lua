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

for num1, entry in ipairs(sorted) do
	if entry.name then
		for num2, name in ipairs(entry.name) do
			local curname = name:match("[^:]*")
			for num3, entry2 in ipairs(sorted) do
				if entry2.name and entry.src == entry2.src and entry ~= entry2 then
					for num4, name2 in ipairs(entry2.name) do
						if curname == name2:match("[^:]*") then
							print(name, "duplicate name")
						end
					end
				end
			end
		end
	end
end

for num1, entry in ipairs(sorted) do
	if entry.data then
		for num2, entry2 in ipairs(sorted) do
			if entry2.data and entry.src == entry2.src and entry ~= entry2 and #entry.data == #entry2.data then
				for i = 1, #entry2.data do
					if entry.data[i] ~= entry2.data[i] then
						break
					end
					if i == #entry2.data then
						for num3, name in ipairs(entry2.name) do
							entry.name[#entry.name + 1] = name
						end
						if entry2.comment then
							for num3, comment in ipairs(entry2.comment) do
								if not entry.comment then
									entry.comment = {}
								end
								entry.comment[#entry.comment + 1] = comment
							end
						end
						sorted[num2] = 	{}
					end
				end
			end
		end
	end
end

-- copyright 2010 Uli Schlachter GPLv2
function stable_sort(list, comp)
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
stable_sort(sorted, function(a,b) if a.src and b.src then return a.src < b.src else return false end end)

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
