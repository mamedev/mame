

local createServer = require('coro-net').createServer
local wrapper = require('coro-wrapper')
local readWrap, writeWrap = wrapper.reader, wrapper.writer
local httpCodec = require('http-codec')
--local tlsWrap = require('coro-tls').wrap
local parseQuery = require('querystring').parse

-- Ignore SIGPIPE if it exists on platform
local uv = require('luv')
if uv.constants.SIGPIPE then
  uv.new_signal():start("sigpipe")
end

local server = {}
local handlers = {}
local bindings = {}

-- Provide a nice case insensitive interface to headers.
local headerMeta = {
  __index = function (list, name)
    if type(name) ~= "string" then
      return rawget(list, name)
    end
    name = name:lower()
    for i = 1, #list do
      local key, value = unpack(list[i])
      if key:lower() == name then return value end
    end
  end,
  __newindex = function (list, name, value)
    -- non-string keys go through as-is.
    if type(name) ~= "string" then
      return rawset(list, name, value)
    end
    -- First remove any existing pairs with matching key
    local lowerName = name:lower()
    for i = #list, 1, -1 do
      if list[i][1]:lower() == lowerName then
        table.remove(list, i)
      end
    end
    -- If value is nil, we're done
    if value == nil then return end
    -- Otherwise, set the key(s)
    if (type(value) == "table") then
      -- We accept a table of strings
      for i = 1, #value do
        rawset(list, #list + 1, {name, tostring(value[i])})
      end
    else
      -- Or a single value interperted as string
      rawset(list, #list + 1, {name, tostring(value)})
    end
  end,
}

local function handleRequest(head, input, socket)
  local req = {
    socket = socket,
    method = head.method,
    path = head.path,
    headers = setmetatable({}, headerMeta),
    version = head.version,
    keepAlive = head.keepAlive,
    body = input
  }
  for i = 1, #head do
    req.headers[i] = head[i]
  end

  local res = {
    code = 404,
    headers = setmetatable({}, headerMeta),
    body = "Not Found\n",
  }

  local function run(i)
    local success, err = pcall(function ()
      i = i or 1
      local go = i < #handlers
        and function ()
          return run(i + 1)
        end
        or function () end
      return handlers[i](req, res, go)
    end)
    if not success then
      res.code = 500
      res.headers = setmetatable({}, headerMeta)
      res.body = err
      print(err)
    end
  end
  run(1)

  local out = {
    code = res.code,
    keepAlive = res.keepAlive,
  }
  for i = 1, #res.headers do
    out[i] = res.headers[i]
  end
  return out, res.body, res.upgrade
end

local function handleConnection(rawRead, rawWrite, socket)

  -- Speak in HTTP events
  local read, updateDecoder = readWrap(rawRead, httpCodec.decoder())
  local write, updateEncoder = writeWrap(rawWrite, httpCodec.encoder())

  for head in read do
    local parts = {}
    for chunk in read do
      if #chunk > 0 then
        parts[#parts + 1] = chunk
      else
        break
      end
    end
    local res, body, upgrade = handleRequest(head, #parts > 0 and table.concat(parts) or nil, socket)
    write(res)
    if upgrade then
      return upgrade(read, write, updateDecoder, updateEncoder, socket)
    end
    write(body)
    if not (res.keepAlive and head.keepAlive) then
      break
    end
  end
  write()

end

function server.bind(options)
  if not options.host then
    options.host = "127.0.0.1"
  end
  if not options.port then
    options.port = require('uv').getuid() == 0 and
      (options.tls and 443 or 80) or
      (options.tls and 8443 or 8080)
  end
  bindings[#bindings + 1] = options
  return server
end

function server.use(handler)
  handlers[#handlers + 1] = handler
  return server
end


function server.start()
  if #bindings == 0 then
    server.bind({})
  end
  for i = 1, #bindings do
    local options = bindings[i]
    createServer(options, function (rawRead, rawWrite, socket)
      --local tls = options.tls
      --if tls then
        --rawRead, rawWrite = tlsWrap(rawRead, rawWrite, {
         -- server = true,
          --key = assert(tls.key, "tls key required"),
          --cert = assert(tls.cert, "tls cert required"),
        --})
      --end
      return handleConnection(rawRead, rawWrite, socket)
    end)
    print("HTTP server listening at http" .. (options.tls and "s" or "") .. "://" .. options.host .. (options.port == (options.tls and 443 or 80) and "" or ":" .. options.port) .. "/")
  end
  return server
end

local quotepattern = '(['..("%^$().[]*+-?"):gsub("(.)", "%%%1")..'])'
local function escape(str)
    return str:gsub(quotepattern, "%%%1")
end

local function compileGlob(glob)
  local parts = {"^"}
  for a, b in glob:gmatch("([^*]*)(%**)") do
    if #a > 0 then
      parts[#parts + 1] = escape(a)
    end
    if #b > 0 then
      parts[#parts + 1] = "(.*)"
    end
  end
  parts[#parts + 1] = "$"
  local pattern = table.concat(parts)
  return function (string)
    return string and string:match(pattern)
  end
end

local function compileRoute(route)
  local parts = {"^"}
  local names = {}
  for a, b, c, d in route:gmatch("([^:]*):([_%a][_%w]*)(:?)([^:]*)") do
    if #a > 0 then
      parts[#parts + 1] = escape(a)
    end
    if #c > 0 then
      parts[#parts + 1] = "(.*)"
    else
      parts[#parts + 1] = "([^/]*)"
    end
    names[#names + 1] = b
    if #d > 0 then
      parts[#parts + 1] = escape(d)
    end
  end
  if #parts == 1 then
    return function (string)
      if string == route then return {} end
    end
  end
  parts[#parts + 1] = "$"
  local pattern = table.concat(parts)
  return function (string)
    local matches = {string:match(pattern)}
    if #matches > 0 then
      local results = {}
      for i = 1, #matches do
        results[i] = matches[i]
        results[names[i]] = matches[i]
      end
      return results
    end
  end
end

function server.route(options, handler)
  local method = options.method
  local path = options.path and compileRoute(options.path)
  local host = options.host and compileGlob(options.host)
  local filter = options.filter
  server.use(function (req, res, go)
    if method and req.method ~= method then return go() end
    if host and not host(req.headers.host) then return go() end
    if filter and not filter(req) then return go() end
    local params
    if path then
      local pathname, query = req.path:match("^([^?]*)%??(.*)")
      params = path(pathname)
      if not params then return go() end
      if #query > 0 then
        req.query = parseQuery(query)
      end
    end
    req.params = params or {}
    return handler(req, res, go)
  end)
  return server
end

return server
