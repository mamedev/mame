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

return function() return db, sql end
