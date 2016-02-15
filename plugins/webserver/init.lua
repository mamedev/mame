local exports = {}
exports.name = "webserver"
exports.version = "1.0.0"
exports.description = "A simple web server"
exports.license = "MIT"
exports.author = { name = "Miodrag Milanovic" }

local ws = exports

local app = require('weblit/app')

function ws.startplugin()
	app.bind({
		host = "0.0.0.0",
		port = 8080
	})

	app.use(require('weblit/logger'))
	app.use(require('weblit/auto-headers'))
	app.use(require('weblit/etag-cache'))

	app.route({
		method = "GET",
		path = "/",
	}, function (req, res, go)
		res.code = 200
		res.headers["Content-Type"] = "text/html"
		res.body = "<h1>Hello!</h1>\n"
	end)

	app.start()
end

return exports
