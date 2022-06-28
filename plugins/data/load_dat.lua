local datfile = {}

local db = require('data/database')

local function readret(file, tablename)
	local query = db.prepare(
		string.format(
			[[SELECT f.data
				FROM "%s_idx" AS fi LEFT JOIN "%s" AS f ON fi.data = f.rowid
				WHERE fi.type = ? AND fi.val = ? AND fi.romset = ?;]],
			tablename, tablename))
	local function read(tag, val, set)
		query:bind_values(tag, val, set)
		local data
		while not data do
			local status = query:step()
			if status == db.ROW then
				data = query:get_value(0)
			elseif status == db.DONE then
				break
			elseif status ~= db.BUSY then
				db.check(string.format('reading %s data', file))
				break
			end
		end
		query:reset()
		return data
	end
	return read
end


function datfile.open(file, vertag, fixupcb)
	if not db then
		return nil
	end

	local fh, filepath, tablename, dbver = db.open_data_file(file)
	if not fh then
		if dbver then
			-- data in database but missing file, just use what we have
			return readret(file, tablename), dbver
		else
			return nil
		end
	end

	local ver
	if vertag then
		-- scan file for version
		for line in fh:lines() do
			local match = line:match(vertag .. '%s*(%S+)')
			if match then
				ver = match
				break
			end
		end
	end
	if not ver then
		-- fall back to file modification time for version
		ver = tostring(lfs.attributes(filepath, 'change'))
	end
	if ver == dbver then
		fh:close()
		return readret(file, tablename), dbver
	end

	if not dbver then
		db.exec(
			string.format(
				[[CREATE TABLE "%s_idx" (
					type VARCHAR NOT NULL,
					val VARCHAR NOT NULL,
					romset VARCHAR NOT NULL,
					data INTEGER NOT NULL);]],
				tablename))
		db.check(string.format('creating %s index table', file))
		db.exec(string.format([[CREATE TABLE "%s" (data CLOB NOT NULL);]], tablename))
		db.check(string.format('creating %s data table', file))
		db.exec(
			string.format(
				[[CREATE INDEX "typeval_%s" ON "%s_idx" (type, val, romset);]],
				tablename, tablename))
		db.check(string.format('creating %s type/value index', file))
	end

	db.exec([[BEGIN TRANSACTION;]])
	if not db.check(string.format('starting %s transaction', file)) then
		fh:close()
		if dbver then
			return readret(file, tablename), dbver
		else
			return nil
		end
	end

	-- clean out previous data and update the version
	if dbver then
		db.exec(string.format([[DELETE FROM "%s";]], tablename))
		if not db.check(string.format('deleting previous %s data', file)) then
			db.exec([[ROLLBACK TRANSACTION;]])
			fh:close()
			return readret(file, tablename), dbver
		end
		db.exec(string.format([[DELETE FROM "%s_idx";]], tablename))
		if not db.check(string.format('deleting previous %s data', file)) then
			db.exec([[ROLLBACK TRANSACTION;]])
			fh:close()
			return readret(file, tablename), dbver
		end
	end
	db.set_version(file, ver)
	if not db.check(string.format('updating %s version', file)) then
		db.exec([[ROLLBACK TRANSACTION;]])
		fh:close()
		if dbver then
			return readret(file, tablename), dbver
		else
			return nil
		end
	end

	local dataquery = db.prepare(
		string.format([[INSERT INTO "%s" (data) VALUES (?);]], tablename))
	local indexquery = db.prepare(
		string.format(
			[[INSERT INTO "%s_idx" (type, val, romset, data) VALUES (?, ?, ?, ?)]],
			tablename))

	fh:seek('set')
	local buffer = fh:read('a')

	local function gmatchpos()
		local pos = 1
		local function iter()
			local tags, data
			while not data do
				local npos
				local spos, epos = buffer:find('[\n\r]$[^=\n\r]*=[^\n\r]*', pos)
				if not spos then
					return nil
				end
				npos, epos = buffer:find('[\n\r]$%w+%s*[\n\r]+', epos)
				if not npos then
					return nil
				end
				tags = buffer:sub(spos, epos)
				spos, npos = buffer:find('[\n\r]$[^=\n\r]*=[^\n\r]*', epos)
				if not spos then
					return nil
				end
				data = buffer:sub(epos, spos)
				pos = spos
			end
			return tags, data
		end
		return iter
	end

	for info, data in gmatchpos() do
		local tags = {}
		local infotype
		info = info:gsub(utf8.char(0xfeff), '') -- remove byte order marks
		data = data:gsub(utf8.char(0xfeff), '')
		for s in info:gmatch('[\n\r]$([^\n\r]*)') do
			if s:find('=', 1, true) then
				local m1, m2 = s:match('([^=]*)=(.*)')
				for tag in m1:gmatch('[^,]+') do
					for set in m2:gmatch('[^,]+') do
						table.insert(tags, { tag = tag, set = set })
					end
				end
			else
				infotype = s
				break
			end
		end

		data = data:gsub('[\n\r]$end%s*[\n\r]$%w+%s*[\n\r]', '\n')
		data = data:gsub('[\n\r]$end%s*[\n\r].-[\n\r]$%w+%s*[\n\r]', '\n')
		data = data:gsub('[\n\r]$end%s*[\n\r].*', '')

		if (#tags > 0) and infotype then
			data = data:gsub('\r', '') -- strip carriage returns
			if fixupcb then
				data = fixupcb(data)
			end

			dataquery:bind_values(data)
			local row
			while true do
				local status = dataquery:step()
				if status == db.DONE then
					row = dataquery:last_insert_rowid();
					break
				elseif status == db.BUSY then
					emu.print_error(string.format('Database busy: inserting %s data', file))
					dataquery:finalize()
					indexquery:finalize()
					db.exec([[ROLLBACK TRANSACTION;]])
					fh:close()
					if dbver then
						return readret(file, tablename), dbver
					else
						return nil
					end
				elseif result ~= db.ROW then
					db.check(string.format('inserting %s data', file))
					break
				end
			end
			dataquery:reset()

			if row then
				for num, tag in pairs(tags) do
					indexquery:bind_values(infotype, tag.tag, tag.set, row)
					while true do
						local status = indexquery:step()
						if status == db.DONE then
							break
						elseif status == db.BUSY then
							emu.print_error(string.format('Database busy: inserting %s data', file))
							dataquery:finalize()
							indexquery:finalize()
							db.exec([[ROLLBACK TRANSACTION;]])
							fh:close()
							if dbver then
								return readret(file, tablename), dbver
							else
								return nil
							end
						elseif result ~= db.ROW then
							db.check(string.format('inserting %s data', file))
							break
						end
					end
					indexquery:reset()
				end
			end
		end
	end

	dataquery:finalize()
	indexquery:finalize()

	fh:close()
	db.exec([[COMMIT TRANSACTION;]])
	if not db.check(string.format('committing %s transaction', file)) then
		db.exec([[ROLLBACK TRANSACTION;]])
		if dbver then
			return readret(file, tablename), dbver
		else
			return nil
		end
	end

	return readret(file, tablename), ver
end

return datfile
