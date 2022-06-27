-- get marp high score file from http://replay.marpirc.net/txt/scores3.htm
local dat = {}

local db = require("data/database")
local ver, info

local function init()
	local file = 'scores3.htm'

	local fh, filepath, tablename, dbver = db.open_data_file(file)
	if not fh then
		if dbver then
			-- data in database but missing file, just use what we have
			ver = dbver
		end
		return
	end

	-- scan file for version
	for line in fh:lines() do
		local match = line:match('Top Scores from the MAME Action Replay Page %(([%w :]+)%)')
		if match then
			ver = match
			break
		end
	end
	if (not ver) or (ver == dbver) then
		fh:close()
		ver = dbver
		return
	end

	if not dbver then
		db.exec(
			string.format(
				[[CREATE TABLE "%s" (
					romset VARCHAR NOT NULL,
					data CLOB NOT NULL,
					PRIMARY KEY(romset));]],
				tablename))
		db.check('creating MARP table')
	end

	db.exec([[BEGIN TRANSACTION;]])
	if not db.check('starting MARP transaction') then
		fh:close()
		ver = dbver
		return
	end

	-- clean out previous data and update the version
	if dbver then
		db.exec(string.format([[DELETE FROM "%s";]], tablename))
		if not db.check('deleting previous MARP data') then
			db.exec([[ROLLBACK TRANSACTION;]])
			fh:close()
			ver = dbver
			return
		end
	end
	if not db.set_version(file, ver) then
		db.check('updating MARP version')
		db.exec([[ROLLBACK TRANSACTION;]])
		fh:close()
		ver = dbver
		return
	end

	fh:seek('set')
	local buffer = fh:read('a')

	local function gmatchpos()
		local pos = 1
		local set = ''
		local data = ''
		local function iter()
			local lastset = set
			while true do
				local spos, scr, plyr, stype, ltype
				spos, pos, set, stype, scr, plyr, ltype = buffer:find('\n%s*([%w_]*)%-?(%w-) :%s*(%d+) [;:] ([^:]+): [^%[\n]*%[?([%w ]*)[^\n]*', pos)
				if not spos then
					return nil
				end

				local url = ''
				if set ~= '' then
					if ltype ~= '' then
						url = ltype .. '\t\n'
					end
					url = url .. 'http://replay.marpirc.net/r/' .. set
					if stype ~= '' then
						url = url .. '-' .. stype
					end
					url = url .. '\t\n'
				end

				if (set ~= '') and (lastset ~= set) then
					local lastdata = data
					data = url .. plyr .. '\t' .. scr .. '\n'
					return lastset, lastdata
				end

				if url ~= '' then
					data = data .. '\n' .. url
				end
				data = data .. plyr .. '\t' .. scr .. '\n'
			end
		end
		return iter
	end

	local query = db.prepare(
		string.format([[INSERT INTO "%s" (romset, data) VALUES (?, ?)]], tablename))
	for set, data in gmatchpos() do
		query:bind_values(set, data)
		while true do
			local status = query:step()
			if status == db.DONE then
				break
			elseif status == db.BUSY then
				emu.print_error('Database busy: inserting MARP data')
				query:finalize()
				db.exec([[ROLLBACK TRANSACTION;]])
				fh:close()
				ver = dbver
				return
			elseif result ~= db.ROW then
				db.check('inserting MARP data')
				break
			end
		end
		query:reset()
	end
	query:finalize()

	fh:close()
	db.exec([[COMMIT TRANSACTION;]])
	if not db.check('committing MARP transaction') then
		db.exec([[ROLLBACK TRANSACTION;]])
		ver = dbver
	end
end

if db then
	init()
end

function dat.check(set, softlist)
	if softlist or (not ver) then
		return nil
	end

	info = nil
	local query = db.prepare([[SELECT data FROM "scores3.htm" WHERE romset = ?;]])
	query:bind_values(set)
	while not info do
		local status = query:step()
		if status == db.ROW then
			info = '#j2\n' .. query:get_value(0)
		elseif status == db.DONE then
			break
		elseif status ~= db.BUSY then
			db.check('reading MARP data')
			break
		end
	end
	query:finalize()
	return info and _p('plugin-data', 'MARPScore') or nil
end

function dat.get()
	return info
end

function dat.ver()
	return ver
end

return dat
