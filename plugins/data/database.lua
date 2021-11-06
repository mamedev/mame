local sql = require("lsqlite3")
local datfile = {}
local db

local function check_db(msg)
	if db:errcode() > sql.OK then
		emu.print_error(string.format("Error: %s (%s - %s)", msg, db:errcode(), db:errmsg()))
	end
end

do
	local dbpath = emu.subst_env(mame_manager.ui.options.entries.historypath:value():match("([^;]+)"))
	db = sql.open(dbpath .. "/history.db")
	if not db then
		lfs.mkdir(dbpath)
		db = sql.open(dbpath .. "/history.db")
		if not db then
			emu.print_error("Unable to create history.db")
			return false
		end
		check_db("opening database")
	end
end

if db then
	local found = false
	db:exec("select * from sqlite_master where name = 'version'", function() found = true return 0 end)
	check_db("checking for 'version' table")
	if not found then
		db:exec([[
			CREATE TABLE version (
				version VARCHAR NOT NULL,
				datfile VARCHAR UNIQUE NOT NULL)]])
		check_db("creating 'version' table")
	end
end

local dbtable = { prepare = function(...) return db:prepare(...) end,
		  exec = function(...) return db:exec(...) end, ROW = sql.ROW, check = check_db }

return db and dbtable or false
