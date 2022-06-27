local dat = {}

local db = require('data/database')
local ver, info
local file = 'history.xml'
local tablename

local function init()
	-- check for old history table
	if db.get_version('history.dat') then
		db.exec([[DROP TABLE "history.dat";]])
		db.exec([[DROP TABLE "history.dat_idx";]])
		db.set_version('history.dat', nil)
	end

	local fh, filepath, dbver
	fh, filepath, tablename, dbver = db.open_data_file(file)
	if not fh then
		if dbver then
			-- data in database but missing file, just use what we have
			ver = dbver
		end
		return
	end

	-- scan file for version
	for line in fh:lines() do
		local match = line:match('<history([^>]*)>')
		if match then
			match = match:match('version="([^"]*)"')
			if match then
				ver = match
				break
			end
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
				[[CREATE TABLE "%s_idx" (
					name VARCHAR NOT NULL,
					list VARCHAR NOT NULL,
					data INTEGER NOT NULL);]],
				tablename))
		db.check(string.format('creating %s index table', file))
		db.exec(string.format([[CREATE TABLE "%s" (data CLOB NOT NULL);]], tablename))
		db.check(string.format('creating %s data table', file))
		db.exec(
			string.format(
				[[CREATE INDEX "namelist_%s" ON "%s_idx" (name, list);]],
				tablename, tablename))
		db.check(string.format('creating %s name/list index', file))
	end

	local slaxml = require('xml')

	db.exec([[BEGIN TRANSACTION;]])
	if not db.check(string.format('starting %s transaction', file)) then
		fh:close()
		ver = dbver
		return
	end

	-- clean out previous data and update the version
	if dbver then
		db.exec(string.format([[DELETE FROM "%s";]], tablename))
		if not db.check(string.format('deleting previous %s data', file)) then
			db.exec([[ROLLBACK TRANSACTION;]])
			fh:close()
			ver = dbver
			return
		end
		db.exec(string.format([[DELETE FROM "%s_idx";]], tablename))
		if not db.check(string.format('deleting previous %s data', file)) then
			db.exec([[ROLLBACK TRANSACTION;]])
			fh:close()
			ver = dbver
			return
		end
	end
	db.set_version(file, ver)
	if not db.check(string.format('updating %s version', file)) then
		db.exec([[ROLLBACK TRANSACTION;]])
		fh:close()
		ver = dbver
		return
	end

	fh:seek('set')
	local buffer = fh:read('a')

	local lasttag
	local entry = {}
	local rowid

	local dataquery = db.prepare(
		string.format([[INSERT INTO "%s" (data) VALUES (?);]], tablename))
	local indexquery = db.prepare(
		string.format([[INSERT INTO "%s_idx" (name, list, data) VALUES (?, ?, ?);]], tablename))

	local parser = slaxml:parser{
		startElement = function(name)
			lasttag = name
			if name == 'entry' then
				entry = {}
				rowid = nil
			elseif (name == 'system') or (name == 'item') then
				table.insert(entry, {})
			end
		end,
		attribute = function(name, value)
			if (name == 'name') or (name == 'list') then
				entry[#entry][name] = value
			end
		end,
		text = function(text, cdata)
			if lasttag == 'text' then
				text = text:gsub('\r', '') -- strip carriage returns
				dataquery:bind_values(text)
				while true do
					local status = dataquery:step()
					if status == db.DONE then
						rowid = dataquery:last_insert_rowid();
						break
					elseif result == db.BUSY then
						emu.print_error(string.format('Database busy: inserting %s data', file))
						-- FIXME: how to abort parse and roll back?
						break
					elseif result ~= db.ROW then
						db.check(string.format('inserting %s data', file))
						break
					end
				end
				dataquery:reset()
			end
		end,
		closeElement = function(name)
			if (name == 'entry') and rowid then
				for num, entry in pairs(entry) do
					indexquery:bind_values(entry.name, entry.list or '', rowid)
					while true do
						local status = indexquery:step()
						if status == db.DONE then
							break
						elseif status == db.BUSY then
							emu.print_error(string.format('Database busy: inserting %s data', file))
							-- FIXME: how to abort parse and roll back?
							break
						elseif result ~= db.ROW then
							db.check(string.format('inserting %s data', file))
							break
						end
					end
					indexquery:reset()
				end
			end
		end
	}

	parser:parse(buffer, { stripWhitespace = true })
	dataquery:finalize()
	indexquery:finalize()
	fh:close()

	db.exec([[COMMIT TRANSACTION;]])
	if not db.check(string.format('committing %s transaction', file)) then
		db.exec([[ROLLBACK TRANSACTION;]])
		ver = dbver
	end
end

if db then
	init()
end

function dat.check(set, softlist)
	if not ver then
		return nil
	end

	info = nil
	local query = db.prepare(
		string.format(
			[[SELECT f.data
				FROM "%s_idx" AS fi LEFT JOIN "%s" AS f ON fi.data = f.rowid
				WHERE fi.name = ? AND fi.list = ?;]],
			tablename, tablename))
	query:bind_values(set, softlist or '')
	while not info do
		local status = query:step()
		if status == db.ROW then
			info = query:get_value(0)
		elseif status == db.DONE then
			break
		elseif status ~= db.BUSY then
			db.check(string.format('reading %s data', file))
			break
		end
	end
	query:finalize()
	return info and _p('plugin-data', 'History') or nil
end

function dat.get()
	return info
end

function dat.ver()
	return ver
end

return dat
