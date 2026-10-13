local datfile = {}

local db = require('data/database')

local function readret(file, tablename)
	local query = db.prepare(
		string.format(
			[[SELECT f.data
				FROM "%s_idx" AS fi LEFT JOIN "%s" AS f ON fi.data = f.rowid
				WHERE fi.type = ? AND fi.val = ? AND fi.romset = ?;]],
			tablename, tablename))
	local db_row = db.ROW
	local db_done = db.DONE
	local db_busy = db.BUSY
	local query_bind = query.bind_values
	local query_step = query.step
	local query_get = query.get_value
	local query_reset = query.reset
	local function read(tag, val, set)
		query_bind(query, tag, val, set)
		local data
		while not data do
			local status = query_step(query)
			if status == db_row then
				data = query_get(query, 0)
			elseif status == db_done then
				break
			elseif status ~= db_busy then
				db.check(string.format('reading %s data', file))
				break
			end
		end
		query_reset(query)
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

	local str_find = string.find
	local str_match = string.match
	local str_gmatch = string.gmatch
	local str_sub = string.sub
	local str_gsub = string.gsub
	local str_format = string.format
	local table_ins = table.insert

	local ver
	if vertag then
		-- scan file for version
		for line in fh:lines() do
			local match = str_match(line, vertag .. '%s*(%S+)')
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
			str_format(
				[[CREATE TABLE "%s_idx" (
					type VARCHAR NOT NULL,
					val VARCHAR NOT NULL,
					romset VARCHAR NOT NULL,
					data INTEGER NOT NULL);]],
				tablename))
		db.check(str_format('creating %s index table', file))
		db.exec(str_format([[CREATE TABLE "%s" (data CLOB NOT NULL);]], tablename))
		db.check(str_format('creating %s data table', file))
		db.exec(
			str_format(
				[[CREATE INDEX "typeval_%s" ON "%s_idx" (type, val, romset);]],
				tablename, tablename))
		db.check(str_format('creating %s type/value index', file))
	end

	db.exec([[BEGIN TRANSACTION;]])
	if not db.check(str_format('starting %s transaction', file)) then
		fh:close()
		if dbver then
			return readret(file, tablename), dbver
		else
			return nil
		end
	end

	-- clean out previous data and update the version
	if dbver then
		db.exec(str_format([[DELETE FROM "%s";]], tablename))
		if not db.check(str_format('deleting previous %s data', file)) then
			db.exec([[ROLLBACK TRANSACTION;]])
			fh:close()
			return readret(file, tablename), dbver
		end
		db.exec(str_format([[DELETE FROM "%s_idx";]], tablename))
		if not db.check(str_format('deleting previous %s data', file)) then
			db.exec([[ROLLBACK TRANSACTION;]])
			fh:close()
			return readret(file, tablename), dbver
		end
	end
	db.set_version(file, ver)
	if not db.check(str_format('updating %s version', file)) then
		db.exec([[ROLLBACK TRANSACTION;]])
		fh:close()
		if dbver then
			return readret(file, tablename), dbver
		else
			return nil
		end
	end

	local dataquery = db.prepare(
		str_format([[INSERT INTO "%s" (data) VALUES (?);]], tablename))
	local indexquery = db.prepare(
		str_format(
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
				local spos, epos = str_find(buffer, '[\n\r]$[^=\n\r]*=[^\n\r]*', pos)
				if not spos then
					return nil
				end
				npos, epos = str_find(buffer, '[\n\r]$%w+%s*[\n\r]+', epos)
				if not npos then
					return nil
				end
				tags = str_sub(buffer, spos, epos)
				spos, npos = str_find(buffer, '[\n\r]$[^=\n\r]*=[^\n\r]*', epos)
				if not spos then
					return nil
				end
				data = str_sub(buffer, epos, spos)
				pos = spos
			end
			return tags, data
		end
		return iter
	end

	for rawinfo, rawdata in gmatchpos() do
		local tags = {}
		local infotype
		local info = str_gsub(rawinfo, utf8.char(0xfeff), '') -- remove byte order marks
		local data = str_gsub(rawdata, utf8.char(0xfeff), '')
		for s in str_gmatch(info, '[\n\r]$([^\n\r]*)') do
			if str_find(s, '=', 1, true) then
				local m1, m2 = str_match(s, '([^=]*)=(.*)')
				for tag in str_gmatch(m1, '[^,]+') do
					for set in str_gmatch(m2, '[^,]+') do
						table_ins(tags, { tag = tag, set = set })
					end
				end
			else
				infotype = s
				break
			end
		end

		data = str_gsub(data, '[\n\r]$end%s*[\n\r]$%w+%s*[\n\r]', '\n')
		data = str_gsub(data, '[\n\r]$end%s*[\n\r].-[\n\r]$%w+%s*[\n\r]', '\n')
		data = str_gsub(data, '[\n\r]$end%s*[\n\r].*', '')

		if (#tags > 0) and infotype then
			data = str_gsub(data, '\r', '') -- strip carriage returns
			if fixupcb then
				data = fixupcb(data)
			end

			local db_done = db.DONE
			local db_busy = db.BUSY
			local db_row = db.ROW
			local query_bind = dataquery.bind_values
			local query_step = dataquery.step
			local query_reset = dataquery.reset
			query_bind(dataquery, data)
			local row
			while true do
				local status = query_step(dataquery)
				if status == db_done then
					row = dataquery:last_insert_rowid();
					break
				elseif status == db_busy then
					emu.print_error(str_format('Database busy: inserting %s data', file))
					dataquery:finalize()
					indexquery:finalize()
					db.exec([[ROLLBACK TRANSACTION;]])
					fh:close()
					if dbver then
						return readret(file, tablename), dbver
					else
						return nil
					end
				elseif result ~= db_row then
					db.check(str_format('inserting %s data', file))
					break
				end
			end
			query_reset(dataquery)

			if row then
				query_bind = indexquery.bind_values
				query_step = indexquery.step
				query_reset = indexquery.reset
				for num, tag in pairs(tags) do
					query_bind(indexquery, infotype, tag.tag, tag.set, row)
					while true do
						local status = query_step(indexquery)
						if status == db_done then
							break
						elseif status == db_busy then
							emu.print_error(str_format('Database busy: inserting %s data', file))
							dataquery:finalize()
							indexquery:finalize()
							db.exec([[ROLLBACK TRANSACTION;]])
							fh:close()
							if dbver then
								return readret(file, tablename), dbver
							else
								return nil
							end
						elseif result ~= db_row then
							db.check(str_format('inserting %s data', file))
							break
						end
					end
					query_reset(indexquery)
				end
			end
		end
	end

	dataquery:finalize()
	indexquery:finalize()

	fh:close()
	db.exec([[COMMIT TRANSACTION;]])
	if not db.check(str_format('committing %s transaction', file)) then
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
