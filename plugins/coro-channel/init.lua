local exports = {}
exports.name = "creationix/coro-channel"
exports.version = "1.2.0"
exports.homepage = "https://github.com/luvit/lit/blob/master/deps/coro-channel.lua"
exports.description = "An adapter for wrapping uv streams as coro-streams and chaining filters."
exports.tags = {"coro", "adapter"}
exports.license = "MIT"
exports.author = { name = "Tim Caswell" }

local function wrapRead(socket)
  local paused = true
  local queue = {}
  local waiting
  local onRead

  function onRead(err, chunk)
    local data = err and {nil, err} or {chunk}
    if waiting then
      local thread = waiting
      waiting = nil
      assert(coroutine.resume(thread, unpack(data)))
    else
      queue[#queue + 1] = data
      if not paused then
        paused = true
        assert(socket:read_stop())
      end
    end
  end

  return function ()
    if #queue > 0 then
      return unpack(table.remove(queue, 1))
    end
    if paused then
      paused = false
      assert(socket:read_start(onRead))
    end
    waiting = coroutine.running()
    return coroutine.yield()
  end

end

local function wrapWrite(socket)

  local function wait()
    local thread = coroutine.running()
    return function (err)
      assert(coroutine.resume(thread, err))
    end
  end

  local function shutdown()
    socket:shutdown(wait())
    coroutine.yield()
    if not socket:is_closing() then
      socket:close()
    end
  end

  return function (chunk)
    if chunk == nil then
      return shutdown()
    end
    assert(socket:write(chunk, wait()))
    local err = coroutine.yield()
    return not err, err
  end

end

exports.wrapRead = wrapRead
exports.wrapWrite = wrapWrite

-- Given a raw uv_stream_t userdata, return coro-friendly read/write functions.
function exports.wrapStream(socket)
  return wrapRead(socket), wrapWrite(socket)
end


function exports.chain(...)
  local args = {...}
  local nargs = select("#", ...)
  return function (read, write)
    local threads = {} -- coroutine thread for each item
    local waiting = {} -- flag when waiting to pull from upstream
    local boxes = {}   -- storage when waiting to write to downstream
    for i = 1, nargs do
      threads[i] = coroutine.create(args[i])
      waiting[i] = false
      local r, w
      if i == 1 then
        r = read
      else
        function r()
          local j = i - 1
          if boxes[j] then
            local data = boxes[j]
            boxes[j] = nil
            assert(coroutine.resume(threads[j]))
            return unpack(data)
          else
            waiting[i] = true
            return coroutine.yield()
          end
        end
      end
      if i == nargs then
        w = write
      else
        function w(...)
          local j = i + 1
          if waiting[j] then
            waiting[j] = false
            assert(coroutine.resume(threads[j], ...))
          else
            boxes[i] = {...}
            coroutine.yield()
          end
        end
      end
      assert(coroutine.resume(threads[i], r, w))
    end
  end
end

return exports
