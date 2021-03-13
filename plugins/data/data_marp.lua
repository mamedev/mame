-- get marp high score file from http://replay.marpirc.net/txt/scores3.htm
local dat = {}
local db = require("data/database")
local ver, info

local function init()
	local filepath
	local dbver
	local fh
	local file = "scores3.htm"

	for path in mame_manager.ui.options.entries.historypath:value():gmatch("([^;]+)") do
		filepath = emu.subst_env(path) .. "/" .. file
		fh = io.open(filepath, "r")
		if fh then
			break
		end
	end

	local stmt = db.prepare("SELECT version FROM version WHERE datfile = ?")
	db.check("reading marp version")
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
		db.exec("CREATE TABLE \"" .. file .. [[" (
		romset VARCHAR NOT NULL,
		data CLOB NOT NULL)]])
		db.check("creating marp table")
	end

	for line in fh:lines() do
		local match = line:match("Top Scores from the MAME Action Replay Page %(([%w :]+)%)")
		if match then
			ver = match
			break
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
		db.check("deleting marp")
		stmt = db.prepare("UPDATE version SET version = ? WHERE datfile = ?")
		db.check("updating marp version")
	else
		stmt = db.prepare("INSERT INTO version VALUES (?, ?)")
		db.check("inserting marp version")
	end
	stmt:bind_values(ver, file)
	stmt:step()
	stmt:finalize()

	fh:seek("set")
	local buffer = fh:read("a")
	db.exec("BEGIN TRANSACTION")
	db.check("beginning marp transation")

	local function gmatchpos()
		local pos = 1
		local set, data = ""
		local function iter()
			local lastset = set
			while true do
				local spos, scr, plyr, stype, ltype
				local url = ""
				spos, pos, set, stype, scr, plyr, ltype = buffer:find("\n%s*([%w_]*)%-?(%w-) :%s*(%d+) [;:] ([^:]+): [^%[\n]*%[?([%w ]*)[^\n]*", pos)
				if not spos then
					return nil
				end
				if set ~= "" then
					if ltype ~= "" then
						url = ltype .. "\t\n"
					else
						url = ""
					end
					url = url .. "http://replay.marpirc.net/r/" .. set .. ((stype ~= "") and ("-" .. stype) or "") .. "\t\n"
				end
				if set ~= "" and lastset and lastset ~= set then
					local lastdata = data
					data = url .. plyr .. "\t" .. scr .. "\n"
					return lastset, lastdata
				end
				data = data .. ((url ~= "") and ("\n" .. url) or "") .. plyr .. "\t" .. scr .. "\n"
			end
		end
		return iter
	end

	for set, data in gmatchpos() do
		stmt = db.prepare("INSERT INTO \"" .. file .. "\" VALUES (?, ?)")
		db.check("inserting marp values")
		stmt:bind_values(set, data)
		stmt:step()
		stmt:finalize()
	end
	fh:close()
	db.exec("END TRANSACTION")
	db.check("ending marp transation")
end

if db then
	init()
end

function dat.check(set, softlist)
	if softlist or not ver or not db then
		return nil
	end
	info = nil
	local stmt = db.prepare("SELECT data FROM \"scores3.htm\" AS f WHERE romset = ?")
	db.check("reading marp data")
	stmt:bind_values(set)
	if stmt:step() == db.ROW then
		info = "#j2\n" .. stmt:get_value(0)
	end
	stmt:finalize()
	return info and _("MARPScore") or nil
end

function dat.get()
	return info
end

function dat.ver()
	return ver
end

return dat
