local sql = require("lsqlite3")
local datfile = {}
local db
do
	local dbpath = lfs.env_replace(mame_manager:ui():options().entries.historypath:value():match("([^;]+)"))
	db = sql.open(dbpath .. "/history.db")
	if not db then
		lfs.mkdir(dbpath)
		db = sql.open(dbpath .. "/history.db")
	end
end
if db then
	local found = false
	db:exec("select * from sqllite_master where name = version", function() found = true return 0 end)
	if not found then
		db:exec([[
			CREATE TABLE version (
				version VARCHAR NOT NULL,
				datfile VARCHAR UNIQUE NOT NULL)]])
	end
end

function datfile.open(file, vertag, fixupcb)
	if not db then
		return nil
	end
	local function read(tag1, tag2, set)
		local data
		local stmt = db:prepare("SELECT f.data FROM \"" .. file .. "_idx\" AS fi, \"" .. file .. [["
		 AS f WHERE fi.type = ? AND fi.val = ? AND fi.romset = ? AND f.rowid = fi.data]])
		stmt:bind_values(tag1, tag2, set)
		if stmt:step() == sql.ROW then
			data = stmt:get_value(0)
		end
		stmt:finalize()
		return data
	end

	local ver, dbver
	local filepath
	local fh

	for path in mame_manager:ui():options().entries.historypath:value():gmatch("([^;]+)") do
		filepath = lfs.env_replace(path) .. "/" .. file
		fh = io.open(filepath, "r")
		if fh then
			break
		end
	end
	-- remove unsafe chars from file for use in sql statement
	file = file:gsub("[^%w%._]", "")

	local stmt = db:prepare("SELECT version FROM version WHERE datfile = ?")
	stmt:bind_values(file)
	if stmt:step() == sql.ROW then
		dbver = stmt:get_value(0)
	end
	stmt:finalize()

	if not fh and dbver then
		-- data in database but missing file, just use what we have
		return read, dbver
	elseif not fh then
		return nil
	elseif not dbver then
		db:exec("CREATE TABLE \"" .. file .. [[_idx" (
				type VARCHAR NOT NULL,
				val VARCHAR NOT NULL,
				romset VARCHAR NOT NULL,
				data INTEGER NOT NULL)]])
		db:exec("CREATE TABLE \"" .. file .. "\" (data CLOB NOT NULL)")
		db:exec("CREATE INDEX \"typeval_" .. file .. "\" ON \"" .. file .. "_idx\"(type, val)")
	end

	if vertag then
		for line in fh:lines() do
			local match = line:match(vertag .. "%s*([^%s]+)")
			if match then
				ver = match
				break
			end
		end
	else
		-- use file ctime for version
		ver = tostring(lfs.attributes(filepath, "change"))
	end
	if ver == dbver then
		return read, dbver
	end

	if dbver then
		db:exec("DELETE FROM \"" .. file .. "\"")
		db:exec("DELETE FROM \"" .. file .. "_idx\"")
		stmt = db:prepare("UPDATE version SET version = ? WHERE datfile = ?")
	else
		stmt = db:prepare("INSERT INTO version VALUES (?, ?)")
	end
	stmt:bind_values(ver, file)
	stmt:step()
	stmt:finalize()

	do
		local inblock = false
		fh:seek("set")
		local buffer = fh:read("a")
		db:exec("BEGIN TRANSACTION")
		local function gmatchpos()
			local pos = 1
			local function iter()
				local tag1, tag2, data, start, inblock = false
				while not data do
					local spos, epos, match = buffer:find("\n($[^\n]*)", pos)
					if not spos then
						return nil
					end
					if match ~= "$end" and not inblock then
						if not tag1 then
							tag1 = match
						else
							tag2 = match
							start = epos + 1
							inblock = true
						end
					elseif inblock == true then
						data = buffer:sub(start, spos)
						inblock = false
					end
					pos = epos
				end
				return tag1, tag2, data
			end
			return iter
		end
		for info1, info2, data in gmatchpos() do
			local tag, set = info1:match("^%$([^%s=]+)=?([^%s]*)")
			if set and set ~= "" then
				local tags = {}
				local sets = {}
				tag:gsub("([^,]+)", function(s) tags[#tags + 1] = s end)
				set:gsub("([^,]+)", function(s) sets[#sets + 1] = s end)
				if #tags > 0 and #sets > 0 then
					local tag1 = info2:match("^$([^%s]*)")
					if fixupcb then
						data = fixupcb(data)
					end
					stmt = db:prepare("INSERT INTO \"" .. file .. "\" VALUES (?)")
					stmt:bind_values(data)
					stmt:step()
					local row = stmt:last_insert_rowid()
					stmt:finalize()
					for num1, tag2 in pairs(tags) do
						for num2, set in pairs(sets) do
							if fixupcb then
								fixupcb(data)
							end
							stmt = db:prepare("INSERT INTO \"" .. file .. "_idx\" VALUES (?, ?, ?, ?)")
							stmt:bind_values(tag1, tag2, set, row)
							stmt:step()
							stmt:finalize()
						end
					end
				end
			end
		end
		db:exec("END TRANSACTION")
	end
	fh:close()

	return read, ver
end

return datfile
