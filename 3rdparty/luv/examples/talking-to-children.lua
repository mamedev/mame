local p = require('lib/utils').prettyPrint
local uv = require('luv')

local stdout = uv.new_pipe(false)
local stderr = uv.new_pipe( false)
local stdin = uv.new_pipe(false)

local handle, pid

local function onexit(code, signal)
  p("exit", {code=code,signal=signal})
end

local function onclose()
  p("close")
end

local function onread(err, chunk)
  assert(not err, err)
  if (chunk) then
    p("data", {data=chunk})
  else
    p("end")
  end
end

local function onshutdown()
  uv.close(handle, onclose)
end

handle, pid = uv.spawn("cat", {
  stdio = {stdin, stdout, stderr}
}, onexit)

p{
  handle=handle,
  pid=pid
}

uv.read_start(stdout, onread)
uv.read_start(stderr, onread)
uv.write(stdin, "Hello World")
uv.shutdown(stdin, onshutdown)

uv.run()
uv.walk(uv.close)
uv.run()
