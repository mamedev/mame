local exports = {}
exports.name = "creationix/coro-net"
exports.version = "1.1.1-1"
exports.dependencies = {
  "creationix/coro-channel@1.2.0"
}
exports.homepage = "https://github.com/luvit/lit/blob/master/deps/coro-net.lua"
exports.description = "An coro style client and server helper for tcp and pipes."
exports.tags = {"coro", "tcp", "pipe", "net"}
exports.license = "MIT"
exports.author = { name = "Tim Caswell" }

local uv = require('luv')
local wrapStream = require('coro-channel').wrapStream

local function makeCallback(timeout)
  local thread = coroutine.running()
  local timer, done
  if timeout then
    timer = uv.new_timer()
    timer:start(timeout, 0, function ()
      if done then return end
      done = true
      timer:close()
      return assert(coroutine.resume(thread, nil, "timeout"))
    end)
  end
  return function (err, data)
    if done then return end
    done = true
    if timer then timer:close() end
    if err then
      return assert(coroutine.resume(thread, nil, err))
    end
    return assert(coroutine.resume(thread, data or true))
  end
end
exports.makeCallback = makeCallback

local function normalize(options)
  local t = type(options)
  if t == "string" then
    options = {path=options}
  elseif t == "number" then
    options = {port=options}
  elseif t ~= "table" then
    assert("Net options must be table, string, or number")
  end
  if options.port or options.host then
    return true,
      options.host or "127.0.0.1",
      assert(options.port, "options.port is required for tcp connections")
  elseif options.path then
    return false, options.path
  else
    error("Must set either options.path or options.port")
  end
end

function exports.connect(options)
  local socket, success, err
  local isTcp, host, port = normalize(options)
  if isTcp then
    assert(uv.getaddrinfo(host, port, {
      socktype = options.socktype or "stream",
      family = options.family or "inet",
    }, makeCallback(options.timeout)))
    local res
    res, err = coroutine.yield()
    if not res then return nil, err end
    socket = uv.new_tcp()
    socket:connect(res[1].addr, res[1].port, makeCallback(options.timeout))
  else
    socket = uv.new_pipe(false)
    socket:connect(host, makeCallback(options.timeout))
  end
  success, err = coroutine.yield()
  if not success then return nil, err end
  local read, write = wrapStream(socket)
  return read, write, socket
end

function exports.createServer(options, onConnect)
  local server
  local isTcp, host, port = normalize(options)
  if isTcp then
    server = uv.new_tcp()
    assert(server:bind(host, port))
  else
    server = uv.new_pipe(false)
    assert(server:bind(host))
  end
  assert(server:listen(256, function (err)
    assert(not err, err)
    local socket = isTcp and uv.new_tcp() or uv.new_pipe(false)
    server:accept(socket)
    coroutine.wrap(function ()
      local success, failure = xpcall(function ()
        local read, write = wrapStream(socket)
        return onConnect(read, write, socket)
      end, debug.traceback)
      if not success then
        print(failure)
      end
      if not socket:is_closing() then
        socket:close()
      end
    end)()
  end))
  return server
end

return exports
