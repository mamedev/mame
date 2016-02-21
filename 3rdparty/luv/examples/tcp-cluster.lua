
-- This function will be run in a child process
local child_code = string.dump(function ()
  local p = require('lib/utils').prettyPrint
  local uv = require('luv')

  -- The parent is going to pass us the server handle over a pipe
  -- This will be our local file descriptor at PIPE_FD
  local pipe = uv.new_pipe(true)
  local pipe_fd = tonumber(os.getenv("PIPE_FD"))
  assert(uv.pipe_open(pipe, pipe_fd))

  -- Configure the server handle
  local server = uv.new_tcp()
  local function onconnection()
    local client = uv.new_tcp()
    uv.accept(server, client)
    p("New TCP", client, "on", server)
    p{client=client}
    uv.write(client, "BYE!\n");
    uv.shutdown(client, function ()
      uv.close(client)
      uv.close(server)
    end)
  end

  -- Read the server handle from the parent
  local function onread(err, data)
    p("onread", {err=err,data=data})
    assert(not err, err)
    if uv.pipe_pending_count(pipe) > 0 then
      local pending_type = uv.pipe_pending_type(pipe)
      p("pending_type", pending_type)
      assert(pending_type == "tcp")
      assert(uv.accept(pipe, server))
      assert(uv.listen(server, 128, onconnection))
      p("Received server handle from parent process", server)
    elseif data then
      p("ondata", data)
    else
      p("onend", data)
    end
  end
  uv.read_start(pipe, onread)

  -- Start the event loop!
  uv.run()
end)

local p = require('lib/utils').prettyPrint
local uv = require('luv')

local exepath = assert(uv.exepath())
local cpu_count = # assert(uv.cpu_info())

local server = uv.new_tcp()
assert(uv.tcp_bind(server, "::1", 1337))
print("Master process bound to TCP port 1337 on ::1")


local function onexit(status, signal)
  p("Child exited", {status=status,signal=signal})
end

local function spawnChild()
  local pipe = uv.new_pipe(true)
  local input = uv.new_pipe(false)
  local _, pid = assert(uv.spawn(exepath, {
    stdio = {input,1,2,pipe},
    env= {"PIPE_FD=3"}
  }, onexit))
  uv.write(input, child_code)
  uv.shutdown(input)
  p("Spawned child", pid, "and sending handle", server)
  assert(uv.write2(pipe, "123", server))
  assert(uv.shutdown(pipe))
end

-- Spawn a child process for each CPU core
for _ = 1, cpu_count do
  spawnChild()
end

uv.run()
