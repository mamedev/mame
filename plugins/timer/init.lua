-- license:BSD-3-Clause
-- copyright-holders:Carl
require('lfs')
local sqlite3 = require('lsqlite3')
local exports = {}
exports.name = "timer"
exports.version = "0.0.2"
exports.description = "Game play timer"
exports.license = "The BSD 3-Clause License"
exports.author = { name = "Carl" }

local timer = exports

function timer.startplugin()
	local dir = emu.subst_env(manager.options.entries.homepath:value())
	local timer_db = dir .. "/timer/timer.db"
	local timer_started = false
	local total_time = 0
	local start_time = 0
	local play_count = 0

	local function save()
		total_time = total_time + (os.time() - start_time)

		local db = assert(sqlite3.open(timer_db))

		local insert_stmt = assert( db:prepare("INSERT OR IGNORE INTO timer VALUES (?, ?, 0, 0)") )
		insert_stmt:bind_values(emu.romname(), emu.softname())
		insert_stmt:step()
		insert_stmt:reset()

		local update_stmt = assert( db:prepare("UPDATE timer SET total_time=?, play_count=? WHERE driver=? AND software=?") )
		update_stmt:bind_values(total_time, play_count,emu.romname(), emu.softname())
		update_stmt:step()
		update_stmt:reset()

		assert(db:close() == sqlite3.OK)
	end


	emu.register_start(function()
		local file
		if timer_started then
			save()
		end
		timer_started = true
		lfs.mkdir(dir .. '/timer')
		local db = assert(sqlite3.open(timer_db))
		local found=false
		db:exec([[select * from sqlite_master where name='timer';]], function(...) found=true return 0 end)
		if not found then
			db:exec[[  CREATE TABLE timer (
						driver      VARCHAR(32) PRIMARY KEY,
						software    VARCHAR(40),
						total_time  INTEGER NOT NULL,
						play_count  INTEGER NOT NULL
					  ); ]]
		end

		local stmt, row
		stmt = db:prepare("SELECT total_time, play_count FROM timer WHERE driver = ? AND software = ?")
		stmt:bind_values(emu.romname(), emu.softname())
		if (stmt:step() == sqlite3.ROW) then
			row = stmt:get_named_values()
			play_count = row.play_count
			total_time = row.total_time
		else
			play_count = 0
			total_time = 0
		end

		assert(db:close() == sqlite3.OK)

		start_time = os.time()
		play_count = play_count + 1
	end)

	emu.register_stop(function()
		timer_started = false
		save()
		total_time = 0
		play_count = 0
	end)

	local function sectohms(time)
		local hrs = math.floor(time / 3600)
		local min = math.floor((time % 3600) / 60)
		local sec = time % 60
		return string.format("%03d:%02d:%02d", hrs, min, sec)
	end

	local lastupdate

	local function menu_populate()
		lastupdate = os.time()
		local time = lastupdate - start_time
		return
			{{ _("Current time"), sectohms(time), "off" },
			 { _("Total time"), sectohms(total_time + time), "off" },
			 { _("Play Count"), play_count, "off" }},
			nil,
			"idle"
	end

	local function menu_callback(index, event)
		return os.time() > lastupdate
	end

	emu.register_menu(menu_callback, menu_populate, _("Timer"))
end

return exports
