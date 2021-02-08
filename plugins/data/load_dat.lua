local datfile = {}
local db = require("data/database")

local function readret(file)
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
	return read
end


function datfile.open(file, vertag, fixupcb)
	if not db then
		return nil
	end
	local ver, dbver
	local filepath
	local fh

	for path in mame_manager.ui.options.entries.historypath:value():gmatch("([^;]+)") do
		filepath = emu.subst_env(path) .. "/" .. file
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
		return readret(file), dbver
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
			local match = line:match(vertag .. "%s*(%S+)")
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
		return readret(file), dbver
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
		fh:seek("set")
		local buffer = fh:read("a")
		db.exec("BEGIN TRANSACTION")
		db.check("beginning transaction")
		local function gmatchpos()
			local pos = 1
			local function iter()
				local tags, data
				while not data do
					local npos
					local spos, epos = buffer:find("[\n\r]$[^=]*=[^\n\r]*", pos)
					if not spos then
						return nil
					end
					npos, epos = buffer:find("[\n\r]$%w*[\n\r]+", epos)
					if not npos then
						return nil
					end
					tags = buffer:sub(spos, epos)
					spos, npos = buffer:find("[\n\r]$end[\n\r]", epos)
					if not spos then
						return nil
					end
					data = buffer:sub(epos, spos)
					pos = npos
				end
				return tags, data
			end
			return iter
		end
		for info, data in gmatchpos() do
			local tags = {}
			local infotype
			for s in info:gmatch("[\n\r]$([^\n\r]*)") do
				if s:find("=", 1, true) then
					local m1, m2 = s:match("([^=]*)=(.*)")
					for tag in m1:gmatch("[^,]+") do
						for set in m2:gmatch("[^,]+") do
							tags[#tags + 1] = { tag = tag, set = set }
						end
					end
				else
					infotype = s
					break
				end
			end

			if #tags > 0 and infotype then
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
				for num, tag in pairs(tags) do
					stmt = db.prepare("INSERT INTO \"" .. file .. "_idx\" VALUES (?, ?, ?, ?)")
					db.check("inserting into index")
					stmt:bind_values(infotype, tag.tag, tag.set, row)
					stmt:step()
					stmt:finalize()
				end
			end
		end
		db.exec("END TRANSACTION")
		db.check("ending transaction")
	end
	fh:close()

	return readret(file), ver
end

return datfile
