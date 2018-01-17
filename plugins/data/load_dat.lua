local datfile = {}
local db = require("data/database")

function datfile.open(file, vertag, fixupcb)
	if not db then
		return nil
	end
	local function read(tag1, tag2, set)
		local data
		local stmt = db.prepare("SELECT f.data FROM \"" .. file .. "_idx\" AS fi, \"" .. file .. [["
		 AS f WHERE fi.type = ? AND fi.val = ? AND fi.romset = ? AND f.rowid = fi.data]])
		db.check("reading " .. tag1 .. " - " .. tag2 .. " - " .. set)
		stmt:bind_values(tag1, tag2, set)
		if stmt:step() == db.ROW then
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

	local stmt = db.prepare("SELECT version FROM version WHERE datfile = ?")
	db.check("reading version")
	stmt:bind_values(file)
	if stmt:step() == db.ROW then
		dbver = stmt:get_value(0)
	end
	stmt:finalize()

	if not fh and dbver then
		-- data in database but missing file, just use what we have
		return read, dbver
	elseif not fh then
		return nil
	elseif not dbver then
		db.exec("CREATE TABLE \"" .. file .. [[_idx" (
				type VARCHAR NOT NULL,
				val VARCHAR NOT NULL,
				romset VARCHAR NOT NULL,
				data INTEGER NOT NULL)]])
		db.check("creating index")
		db.exec("CREATE TABLE \"" .. file .. "\" (data CLOB NOT NULL)")
		db.check("creating table")
		db.exec("CREATE INDEX \"typeval_" .. file .. "\" ON \"" .. file .. "_idx\"(type, val)")
		db.check("creating typeval index")
	end

	if vertag then
		for line in fh:lines() do
			local match = line:match(vertag .. "%s*([^%s]+)")
			if match then
				ver = match
				break
			end
		end
	end
	if not ver then
		-- use file ctime for version
		ver = tostring(lfs.attributes(filepath, "change"))
	end
	if ver == dbver then
		fh:close()
		return read, dbver
	end

	if dbver then
		db.exec("DELETE FROM \"" .. file .. "\"")
		db.check("deleting")
		db.exec("DELETE FROM \"" .. file .. "_idx\"")
		db.check("deleting index")
		stmt = db.prepare("UPDATE version SET version = ? WHERE datfile = ?")
		db.check("updating version")
	else
		stmt = db.prepare("INSERT INTO version VALUES (?, ?)")
		db.check("inserting version")
	end
	stmt:bind_values(ver, file)
	stmt:step()
	stmt:finalize()

	do
		local inblock = false
		fh:seek("set")
		local buffer = fh:read("a")
		db.exec("BEGIN TRANSACTION")
		db.check("beginning transaction")
		local function gmatchpos()
			local pos = 1
			local function iter()
				local tag1, tag2, data, start, inblock = false
				while not data do
					local spos, epos, match = buffer:find("\n($[^\n\r]*)", pos)
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
					data = data:gsub("\r", "") -- strip crs
					if fixupcb then
						data = fixupcb(data)
					end
					stmt = db.prepare("INSERT INTO \"" .. file .. "\" VALUES (?)")
					db.check("inserting values")
					stmt:bind_values(data)
					stmt:step()
					local row = stmt:last_insert_rowid()
					stmt:finalize()
					for num1, tag2 in pairs(tags) do
						for num2, set in pairs(sets) do
							if fixupcb then
								fixupcb(data)
							end
							stmt = db.prepare("INSERT INTO \"" .. file .. "_idx\" VALUES (?, ?, ?, ?)")
							db.check("inserting into index")
							stmt:bind_values(tag1, tag2, set, row)
							stmt:step()
							stmt:finalize()
						end
					end
				end
			end
		end
		db.exec("END TRANSACTION")
		db.check("ending transaction")
	end
	fh:close()

	return read, ver
end

return datfile
