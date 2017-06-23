local jsoncheat = {}

function jsoncheat.filename(name)
	return name .. ".json"
end

function jsoncheat.conv_cheat(data)
	local json = require("json")
	return json.parse(data)
end

return jsoncheat
