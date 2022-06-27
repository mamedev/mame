local sql = require('lsqlite3')
local db

local function check_db_result(msg)
	if db:errcode() > sql.OK then
		emu.print_error(string.format('Error: %s (%s - %s)', msg, db:errcode(), db:errmsg()))
		return false
	end
	return true
end

local function settings_path()
	return emu.subst_env(manager.machine.options.entries.homepath:value():match('([^;]+)')) .. '/data'
end

local function check_version_table()
	local found = false
	db:exec(
			[[SELECT COUNT(*) FROM sqlite_master WHERE name = 'version' AND type = 'table';]],
			function (udata, cols, values, names)
				found = tonumber(values[1]) > 0
				return 0
			end)
	if check_db_result('checking for "version" table') and (not found) then
		db:exec(
				[[CREATE TABLE version (
					datfile VARCHAR NOT NULL,
					version VARCHAR NOT NULL,
					PRIMARY KEY (datfile));]])
		found = check_db_result('creating "version" table')
	end
	if not found then
		db:close()
		db = nil
	end
end

local function open_existing()
	db = sql.open(settings_path() .. '/history.db', sql.OPEN_READWRITE)
	if db then
		check_version_table()
	end
	return db
end

local function open_create()
	-- first try to create settings directory
	local dir = settings_path()
	local attr = lfs.attributes(dir)
	if (not attr) and (not lfs.mkdir(dir)) then
		emu.print_error(string.format('Error creating data plugin settings directory "%s"', dir))
	elseif attr and (attr.mode ~= 'directory') then
		emu.print_error(string.format('Error opening data plugin database: "%s" is not a directory', dir))
	else
		-- now try to open the database
		local dbpath = dir .. '/history.db'
		db = sql.open(dbpath, sql.OPEN_READWRITE | sql.OPEN_CREATE)
		if not db then
			emu.print_error(string.format('Error opening data plugin database "%s"', dbpath))
		else
			check_version_table()
		end
	end
	return db
end


local dbtable = {
	ROW = sql.ROW,
	BUSY = sql.BUSY,
	DONE = sql.DONE,
	check = check_db_result }

function dbtable.sanitize_name(name)
	return name:gsub('[^%w%."]', '_')
end

function dbtable.get_version(filename)
	-- don't try to create the database here, just return nil if it doesn't exist
	if (not db) and (not open_existing()) then
		return nil
	end
	local query = db:prepare([[SELECT version FROM version WHERE datfile = ?;]])
	query:bind_values(filename)
	local result
	while result == nil do
		local status = query:step()
		if status == sql.ROW then
			result = query:get_value(0)
		elseif status ~= sql.BUSY then
			break
		end
	end
	query:finalize()
	return result
end

function dbtable.set_version(filename, version)
	if (not db) and (not open_create()) then
		return nil
	end
	local query
	if version ~= nil then
		query = db:prepare(
			[[INSERT INTO version (datfile, version) VALUES (?1, ?2)
				ON CONFLICT(datfile) DO UPDATE set version = ?2;]])
		query:bind_values(filename, version)
	else
		query = db:prepare([[DELETE FROM version WHERE datfile = ?1;]])
		query:bind_values(filename)
	end
	local result
	while result == nil do
		local status = query:step()
		if status == sqlite3.DONE then
			result = true
		elseif result ~= sqlite3.ROW then
			result = false
		end
	end
	query:finalize()
	return result
end

function dbtable.prepare(...)
	if (not db) and open_create() then
		dbtable.prepare = function (...) return db:prepare(...) end
	end
	if db then
		return db:prepare(...)
	else
		return nil
	end
end

function dbtable.exec(...)
	if (not db) and open_create() then
		dbtable.exec = function (...) return db:exec(...) end
	end
	if db then
		return db:exec(...)
	else
		return nil
	end
end

function dbtable.open_data_file(file)
	local fh, filepath
	for path in mame_manager.ui.options.entries.historypath:value():gmatch('([^;]+)') do
		filepath = emu.subst_env(path) .. '/' .. file
		fh = io.open(filepath, 'r')
		if fh then
			break
		end
	end
	return fh, filepath, dbtable.sanitize_name(file), dbtable.get_version(file)
end

return dbtable
