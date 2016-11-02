function premake.qbs.list(indent, name, table)
	if #table > 0 then
		_p(indent, '%s: [', name)
		for _, item in ipairs(table) do
			_p(indent+1, '"%s",', item:gsub('"', '\\"'))
		end
		_p(indent+1, ']')
	end
end