# weblit

A web framework for luv (ported from luvit/lit)

Weblit is a collection of lit packages that together form a nice web framework.

## weblit/app

This is the core of the framework.  It's export value is the app itself.  The
config functions can be chained off this config for super terse syntax.

```lua
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
    path = "/do/:user/:action",
    domain = "*.myapp.io"
  }, function (req, res, go)
    -- Handle route
  end)

  .start()

```

### bind(options)

Use this to configure your server.  You can bind to multiple addresses and
ports. For example, the same server can listen on port `8080` using normal HTTP
while also listening on port `8443` using HTTPS.

```lua
-- Listen on port 8080 internally using plain HTTP.
.bind({
  host = "127.0.0.1",
  port = 8080
})

-- Also listen on port 8443 externally using encrypted HTTPS.
.bind({
  host = "0.0.0.0",
  port = 8443,
  tls = {
    cert = module:load("cert.pem"),
    key = module:load("key.pem",
  }
})
```

The `host` option defaults to `"127.0.0.1"`.  The default port depends on if
you're running as root and if the connection is TLS encrypted.

      | Root | User
------|-----:|------:
HTTP  | 80   | 8080
HTTPS | 442  | 8443


### use(middleware)

This adds a raw middleware to the chain.  It's signature is:

```lua
.use(function (req, res, go)
  -- Log the request table
  p("request", req)
  -- Hand off to the next layer.
  return go()
end)
```

The `req` table will contain information about the HTTP request.  This includes
several fields:

 - `socket` - The raw libuv `uv_tty_t` socket.
 - `method` - The HTTP request method verb like `GET` or `POST`.
 - `path` - The raw HTTP request path (including query string).
 - `headers` - A list of headers.  Each header is a table with two entries for
   key and value.  For convenience, there are special `__index` and
   `__newindex` metamethods that let you treat this like a case insensitive
   key/value store.
 - `version` - The HTTP version (Usually either `1.0` or `1.1`).
 - `keepAlive` - A flag telling you if this should be a keepalive connection.
 - `body` - The request body as a string.  In the future, this may also be a stream.

The `res` table also has some conventions used to form the response a piece at a
time.  Initially it contains:

 - `code` - The response status code. Initially contains `404`.
 - `headers` - Another special headers table like in `req`.
 - `body` - The response body to send. Initially contains `"Not Found\n"`.

The `go` function is to be called if you wish to not handle a request.  This
allows other middleware layers to get a chance to respond to the request.  Use a
tail call if there is nothing more to do.

Otherwise do further processing after `go` returns.  At this point, all inner
layers have finished and a response is ready in `res`.

### route(options, middleware)

Route is like use, but allows you to pre-filter the requests before the middleware
is called.

```lua
.route({
  method = "PUT",
  path = "/upload/:username"
}, function (req, res, go)
  local url = saveFile(req.params.username, req.body)
  res.code = 201
  res.headers.Location = url
end)
```

The route options accept several parameters:

 - `method` - This is a simple filter on a specific HTTP method verb.
 - `path` - This is either an exact match or can contain patterns.  Segments
   looking like `:name` will match single path segments while `:name:` will
   match multiple segments.  The matches will go into `req.params`.  Also any
   query string will be stripped off, parsed out, and stored in `req.query`.
 - `host` - Will filter against the `Host` header.  This can be an exact match
   or a glob match like `*.mydomain.org`.
 - `filter` - Filter is a custom lua function that accepts `req` and returns
   `true` or `false`.

If the request matches all the requirements, then the middleware is called the
same as with `use`.

### start

Bind to the port(s), listen on the socket(s) and start accepting connections.

## weblit/logger

This is a simple middleware that logs the request method, url and user agent.
It also includes the response status code.

Make sure to use it at the top of your middleware chain so that it's able to see
the final response code sent to the client.

```lua
.use(require('weblit/logger'))
```

## weblit/auto-headers

This implements lots of conventions and useful defaults that help your app
implement a proper HTTP server.

You should always use this near the top of the list.  The only middleware that
goes before this is the logger.


```lua
.use(require('weblit/auto-headers'))
```

## weblit/etag-cache

This caches responses in memory keyed by etag.  If there is no etag, but there
is a response body, it will use the body to generate an etag.

Put this in your list after auto-headers, but before custom server logic.

```lua
.use(require('weblit/etag-cache'))
```

## weblit/static

This middleware serves static files to the user.  Use this to serve your client-
side web assets.

Usage is pretty simplistic for now.

```lua
local static = require('weblit/static')
app.use(static("path/to/static/assets"))
```

If you want to only match a sub-path, use the router.

```lua
app.route({
  path = "/blog/:path:"
}, static(pathJoin(module.dir, "articles")))
```

The `path` param will be used if it exists and the full path will be used
otherwise.

## weblit/websocket

This implements a websocket upgrade handler.  You can choose the subprotocol and
other routing information.

```lua
app.websocket({
  path = "/v2/socket", -- Prefix for matching
  protocol = "virgo/2.0", -- Restrict to a websocket sub-protocol
}, function (req, read, write)
  -- Log the request headers
  p(req)
  -- Log and echo all messages
  for message in read do
    write(message)
  end
  -- End the stream
  write()
end)
```


## weblit

This is the metapackage that simply includes the other modules.

It exposes the other modules as a single exports table.

```lua
exports.app = require('weblit/app')
exports.autoHeaders = require('weblit/auto-headers')
exports.etagCache = require('weblit/etag-cache')
exports.logger = require('weblit/logger')
exports.static = require('weblit/static')
exports.websocket = require('weblit/websocket')
```
