local exports = {}
exports.name = "creationix/coro-wrapper"
exports.version = "1.0.0-1"
exports.homepage = "https://github.com/luvit/lit/blob/master/deps/coro-wrapper.lua"
exports.description = "An adapter for applying decoders to coro-streams."
exports.tags = {"coro", "decoder", "adapter"}
exports.license = "MIT"
exports.author = { name = "Tim Caswell" }

function exports.reader(read, decode)
  local buffer = ""
  return function ()
    while true do
      local item, extra = decode(buffer)
      if item then
        buffer = extra
        return item
      end
      local chunk = read()
      if not chunk then return end
      buffer = buffer .. chunk
    end
  end,
  function (newDecode)
    decode = newDecode
  end
end

function exports.writer(write, encode)
  return function (item)
    if not item then
      return write()
    end
    return write(encode(item))
  end,
  function (newEncode)
    encode = newEncode
  end
end

return exports
