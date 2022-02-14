local dat = {}
local db = require("data/database")
local ver, info
local file = "history.xml"

local function init()
	local filepath
	local dbver
	local fh

	for path in mame_manager.ui.options.entries.historypath:value():gmatch("([^;]+)") do
		filepath = emu.subst_env(path) .. "/" .. file
		fh = io.open(filepath, "r")
		if fh then
			break
		end
	end

	-- check for old history table
	local stmt = db.prepare("SELECT version FROM version WHERE datfile = ?")
	stmt:bind_values("history.dat")
	if stmt:step() == db.ROW then
		stmt:finalize()
		db.exec("DROP TABLE \"history.dat\"")
		db.exec("DROP TABLE \"history.dat_idx\"")
		stmt = db.prepare("DELETE FROM version WHERE datfile = ?")
		stmt:bind_values("history.dat")
		stmt:step()
	end
	stmt:finalize()


	local stmt = db.prepare("SELECT version FROM version WHERE datfile = ?")
	db.check("reading history version")
	stmt:bind_values(file)
	if stmt:step() == db.ROW then
		dbver = stmt:get_value(0)
	end
	stmt:finalize()

	if not fh and dbver then
		-- data in database but missing file, just use what we have
		ver = dbver
		return
	elseif not fh then
		return
	elseif not dbver then
		db.exec("CREATE TABLE \"" .. file .. [[_idx" (
				name VARCHAR NOT NULL,
				list VARCHAR,
				data INTEGER NOT NULL)]])
		db.check("creating index")
		db.exec("CREATE TABLE \"" .. file .. "\" (data CLOB NOT NULL)")
		db.check("creating table")
		db.exec("CREATE INDEX \"name_" .. file .. "\" ON \"" .. file .. "_idx\"(name)")
		db.check("creating system index")
	end

	for line in fh:lines() do
		local match = line:match("<history([^>]*)>")
		if match then
			match = match:match("version=\"([^\"]*)\"")
			if match then
				ver = match
				break
			end
		end
	end

	if not ver then
		fh:close()
		return
	end

	if ver == dbver then
		fh:close()
		return
	end

	if dbver then
		db.exec("DELETE FROM \"" .. file .. "\"")
		db.check("deleting history")
		db.exec("DELETE FROM \"" .. file .. "_idx\"")
		db.check("deleting index")
		stmt = db.prepare("UPDATE version SET version = ? WHERE datfile = ?")
		db.check("updating history version")
	else
		stmt = db.prepare("INSERT INTO version VALUES (?, ?)")
		db.check("inserting history version")
	end
	stmt:bind_values(ver, file)
	stmt:step()
	stmt:finalize()

	fh:seek("set")
	local buffer = fh:read("a")
	db.exec("BEGIN TRANSACTION")
	db.check("beginning history transation")

	local lasttag
	local entry = {}
	local rowid
	local slaxml = require("xml")

	local parser = slaxml:parser{
		startElement = function(name)
			lasttag = name
			if name == "entry" then
				entry = {}
				rowid = nil
			elseif (name == "system") or (name == "item") then
				entry[#entry + 1] = {}
			end
		end,
		attribute = function(name, value)
			if (name == "name") or (name == "list") then
				entry[#entry][name] = value
			end
		end,
		text = function(text, cdata)
			if lasttag == "text" then
				text = text:gsub("\r", "") -- strip crs
				stmt = db.prepare("INSERT INTO \"" .. file .. "\" VALUES (?)")
				db.check("inserting values")
				stmt:bind_values(text)
				stmt:step()
				rowid = stmt:last_insert_rowid()
				stmt:finalize()
			end
		end,
		closeElement = function(name)
			if name == "entry" then
				for num, entry in pairs(entry) do
					stmt = db.prepare("INSERT INTO \"" .. file .. "_idx\" VALUES (?, ?, ?)")
					db.check("inserting into index")
					stmt:bind_values(entry.name, entry.list, rowid)
					stmt:step()
					stmt:finalize()
				end
			end
		end
	}
	parser:parse(buffer, {stripWhitespace=true})
	fh:close()
	db.exec("END TRANSACTION")
	db.check("ending history transation")
end

if db then
	init()
end

function dat.check(set, softlist)
	if not ver or not db then
		return nil
	end
	info = nil
	local stmt
	if softlist then
		stmt = db.prepare("SELECT f.data FROM \"" .. file .. "_idx\" AS fi, \"" .. file .. [["
			AS f WHERE fi.name = ? AND fi.list = ? AND f.rowid = fi.data]])
		stmt:bind_values(set, softlist)
	else
		stmt = db.prepare("SELECT f.data FROM \"" .. file .. "_idx\" AS fi, \"" .. file .. [["
			AS f WHERE fi.name = ? AND fi.list ISNULL AND f.rowid = fi.data]])
		stmt:bind_values(set)
	end
	if stmt:step() == db.ROW then
		info = stmt:get_value(0)
	end
	stmt:finalize()
	return info and _p("plugin-data", "History") or nil
end

function dat.get()
	return info
end

function dat.ver()
	return ver
end

return dat
