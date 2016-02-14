local uv = require('luv')
local cwd = uv.cwd()
package.path = cwd .. "/plugins/?.lua;" .. cwd .. "/plugins/?/init.lua"

require('weblit/app')

  .bind({
    host = "0.0.0.0",
    port = 8080
  })

  .use(require('weblit/logger'))
  .use(require('weblit/auto-headers'))
  .use(require('weblit/etag-cache'))

  .route({
    method = "GET",
    path = "/",
  }, function (req, res, go)
    res.code = 200
    res.headers["Content-Type"] = "text/html"
    res.body = "<h1>Hello!</h1>\n"
  end)

  .start()

