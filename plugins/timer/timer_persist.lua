-- license:BSD-3-Clause
-- copyright-holders:Vas Crabb

local sqlite3 = require('lsqlite3')


local function check_schema(db)
	local create_statement =
		[[CREATE TABLE timer (
			driver      VARCHAR(32) NOT NULL,
			softlist    VARCHAR(24) NOT NULL DEFAULT '',
			software    VARCHAR(16) NOT NULL DEFAULT '',
			total_time  INTEGER NOT NULL DEFAULT 0,
			play_count  INTEGER NOT NULL DEFAULT 0,
			emu_sec     INTEGER NOT NULL DEFAULT 0,
			emu_nsec    INTEGER NOT NULL DEFAULT 0,
			PRIMARY KEY (driver, softlist, software));]]

	-- create table if it doesn't exist yet
	local table_found = false
	db:exec(
		[[SELECT * FROM sqlite_master WHERE type = 'table' AND name='timer';]],
		function()
			table_found = true
		end)
	if not table_found then
		emu.print_verbose('Creating timer database table')
		db:exec(create_statement)
		return
	end

	-- check recently added columns
	local have_softlist = false
	local have_emu_sec = false
	local have_emu_nsec = false
	db:exec(
		[[PRAGMA table_info(timer);]],
		function(udata, n, vals, cols)
			for i, name in ipairs(cols) do
				if name == 'name' then
					if vals[i] == 'softlist' then
						have_softlist = true
					elseif vals[i] == 'emu_sec' then
						have_emu_sec = true
					elseif vals[i] == 'emu_nsec' then
						have_emu_nsec = true
					end
					return 0
				end
			end
			return 0
		end)
	if not have_softlist then
		emu.print_verbose('Adding softlist column to timer database')
		db:exec([[ALTER TABLE timer ADD COLUMN softlist VARCHAR(24) NOT NULL DEFAULT '';]])
		local to_split = { }
		db:exec(
			[[SELECT DISTINCT software FROM timer WHERE software LIKE '%:%';]],
			function(udata, n, vals)
				table.insert(to_split, vals[1])
				return 0
			end)
		if #to_split > 0 then
			local update = db:prepare([[UPDATE timer SET softlist = ?, software = ? WHERE software = ?;]])
			for i, softname in ipairs(to_split) do
				local x, y = softname:find(':')
				local softlist = softname:sub(1, x - 1)
				local software = softname:sub(y + 1)
				update:bind_values(softlist, software, softname)
				update:step()
				update:reset()
			end
		end
	end
	if not have_emu_sec then
		emu.print_verbose('Adding emu_sec column to timer database')
		db:exec([[ALTER TABLE timer ADD COLUMN emu_sec INTEGER NOT NULL DEFAULT 0;]])
	end
	if not have_emu_nsec then
		emu.print_verbose('Adding emu_nsec column to timer database')
		db:exec([[ALTER TABLE timer ADD COLUMN emu_nsec INTEGER NOT NULL DEFAULT 0;]])
	end

	-- check the required columns are in the primary key
	local index_name
	db:exec(
		[[SELECT name FROM sqlite_master WHERE type = 'index' AND tbl_name = 'timer';]],
		function(udata, n, vals)
			index_name = vals[1]
		end)
	local index_good
	if index_name then
		local driver_indexed = false
		local softlist_indexed = false
		local software_indexed = false
		db:exec(
			string.format([[PRAGMA index_info('%s');]], index_name), -- can't use prepared statement for PRAGMA
			function(udata, n, vals, cols)
				for i, name in ipairs(cols) do
					if name == 'name' then
						if vals[i] == 'driver' then
							driver_indexed = true
						elseif vals[i] == 'softlist' then
							softlist_indexed = true
						elseif vals[i] == 'software' then
							software_indexed = true
						end
						return 0
					end
				end
				return 0
			end)
		index_good = driver_indexed and softlist_indexed and software_indexed
	end

	-- if the required columns are not indexed, migrate to a new table with desired primary key
	if not index_good then
		emu.print_verbose('Re-indexing timer database table')
		db:exec([[DROP TABLE IF EXISTS timer_old;]])
		db:exec([[ALTER TABLE timer RENAME TO timer_old;]])
		db:exec(create_statement)
		db:exec(
			[[INSERT
				INTO timer (driver, softlist, software, total_time, play_count, emu_sec, emu_nsec)
				SELECT driver, softlist, software, total_time, play_count, emu_sec, emu_nsec FROM timer_old;]])
		db:exec([[DROP TABLE timer_old;]])
	end
end


local function open_database()
	-- make sure settings directory exists
	local dir = emu.subst_env(manager.machine.options.entries.homepath:value():match('([^;]+)')) .. '/timer'
	local attr = lfs.attributes(dir)
	if not attr then
		lfs.mkdir(dir)
	elseif attr.mode ~= 'directory' then
		emu.print_error(string.format('Error opening timer database: "%s" is not a directory', dir))
		return nil
	end

	-- open database
	local filename = dir .. '/timer.db'
	local db = sqlite3.open(filename)
	if not db then
		emu.print_error(string.format('Error opening timer database file "%s"', filename))
		return nil
	end

	-- make sure schema is up-to-date
	check_schema(db)
	return db
end


local function get_software()
	local softname = emu.softname()
	local i, j = softname:find(':')
	if i then
		return softname:sub(1, i - 1), softname:sub(j + 1)
	else
		-- FIXME: need a way to get the implicit software list when no colon in the option value
		return '', softname
	end
end


local function get_current(db)
	local statement = db:prepare(
		[[SELECT
			total_time, play_count, emu_sec, emu_nsec
			FROM timer
			WHERE driver = ? AND softlist = ? AND software = ?;]])
	statement:bind_values(emu.romname(), get_software())
	local result
	if statement:step() == sqlite3.ROW then
		result = statement:get_named_values()
	end
	statement:reset()
	return result
end


local lib = { }

function lib:start_session()
	-- open database
	local db = open_database()
	if not db then
		return 0, 0, emu.attotime()
	end

	-- get existing values
	local row = get_current(db)
	local update
	if row then
		update = db:prepare(
			[[UPDATE timer
				SET play_count = play_count + 1
				WHERE driver = ? AND softlist = ? AND software = ?;]])
	else
		row = { total_time = 0, play_count = 0, emu_sec = 0, emu_nsec = 0 }
		update = db:prepare(
			[[INSERT
				INTO timer (driver, softlist, software, total_time, play_count, emu_sec, emu_nsec)
				VALUES (?, ?, ?, 0, 1, 0, 0);]])
	end
	update:bind_values(emu.romname(), get_software())
	update:step()
	update:reset()
	return row.total_time, row.play_count + 1, emu.attotime.from_seconds(row.emu_sec) + emu.attotime.from_nsec(row.emu_nsec)
end

function lib:update_totals(start)
	-- open database
	local db = open_database()
	if not db then
		return
	end

	-- get existing values
	local row = get_current(db)
	if not row then
		row = { total_time = 0, play_count = 1, emu_sec = 0, emu_nsec = 0 }
	end

	-- calculate new totals
	local emu_total = emu.attotime.from_seconds(row.emu_sec) + emu.attotime.from_nsec(row.emu_nsec) + manager.machine.time
	row.total_time = os.time() - start + row.total_time
	row.emu_sec = emu_total.seconds
	row.emu_nsec = emu_total.nsec

	-- update database
	local update = db:prepare(
		[[INSERT OR REPLACE
			INTO timer (driver, softlist, software, total_time, play_count, emu_sec, emu_nsec)
			VALUES (?, ?, ?, ?, ?, ?, ?);]])
	local softlist, software = get_software()
	update:bind_values(emu.romname(), softlist, software, row.total_time, row.play_count, row.emu_sec, row.emu_nsec)
	update:step()
	update:reset()

	-- close database
	if db:close() ~= sqlite3.OK then
		emu.print_error('Error closing timer database')
	end
end

return lib
