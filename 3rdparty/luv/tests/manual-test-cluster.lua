-- This is quite the involved test.  Basically it binds
-- to a tcp port, spawns n children (one per CPU core)
-- who all listen on the same shared port and act as a
-- load balancing cluster.
-- Then N clients are spawned that connect to the cluster
-- The application itself kills the worker upon connection
-- All N workers should accept exactly one request and all close.

return require('lib/tap')(function (test)

  -- This function will be run in a child process
  local worker_code = string.dump(function ()
    local dump = require('lib/utils').dump

    local function print(...)
      local n = select('#', ...)
      local arguments = {...}
      for i = 1, n do
        arguments[i] = tostring(arguments[i])
      end

      local text = table.concat(arguments, "\t")
      text = "    " .. string.gsub(text, "\n", "\n    ")
      _G.print(text)
    end

    local function p(...)
      local n = select('#', ...)
      local arguments = { ... }

      for i = 1, n do
        arguments[i] = dump(arguments[i])
      end

      print(table.concat(arguments, "\t"))
    end

    local uv = require('luv')
    local answer = -1

    -- The parent is going to pass us the server handle over a pipe
    -- This will be our local file descriptor at PIPE_FD
    local pipe = uv.new_pipe(true)
    local pipe_fd = tonumber(os.getenv("PIPE_FD"))
    assert(uv.pipe_open(pipe, pipe_fd))

    -- Configure the server handle
    local server = uv.new_tcp()
    local done = false
    local function onconnection()
      print("NOT ACCEPTING, already done")
      if done then return end
      local client = uv.new_tcp()
      assert(uv.accept(server, client))
      p("New TCP", client, "on", server)
      p{client=client}
      assert(uv.write(client, "BYE!\n"));
      assert(uv.shutdown(client, function ()
        uv.close(client)
        uv.unref(server)
        done = true
        answer = 42
      end))
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
        assert(uv.listen(server, 0, onconnection))
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

    os.exit(answer)
  end)

  local client_code = string.dump(function ()
    local dump = require('lib/utils').dump

    local function print(...)
      local n = select('#', ...)
      local arguments = {...}
      for i = 1, n do
        arguments[i] = tostring(arguments[i])
      end

      local text = table.concat(arguments, "\t")
      text = "      " .. string.gsub(text, "\n", "\n      ")
      _G.print(text)
    end

    local function p(...)
      local n = select('#', ...)
      local arguments = { ... }

      for i = 1, n do
        arguments[i] = dump(arguments[i])
      end

      print(table.concat(arguments, "\t"))
    end

    local uv = require('luv')

    local host = os.getenv("HOST")
    local port = tonumber(os.getenv("PORT"))

    local socket = uv.new_tcp()

    assert(uv.tcp_connect(socket, host, port, function (err)
      p("client connected", {err=err})
      assert(not err, err)
    end))

    -- Start the event loop!
    uv.run()
  end)

  test("tcp cluster", function (print, p, expect, uv)

    local exepath = assert(uv.exepath())
    local cpu_count = # assert(uv.cpu_info())
    local left = cpu_count

    local server = uv.new_tcp()
    assert(uv.tcp_bind(server, "::1", 0))

    local address = uv.tcp_getsockname(server)
    p{server=server,address=address}

    print("Master process bound to TCP port " .. address.port .. " on " .. address.ip)

    local function spawnWorker()
      local pipe = uv.new_pipe(true)
      local input = uv.new_pipe(false)
      local child, pid
      child, pid = assert(uv.spawn(exepath, {
        cwd = uv.cwd(),
        stdio = {input,1,2,pipe},
        env= {"PIPE_FD=3"}
      }, expect(function (status, signal)
        p("Worker exited", {status=status,signal=signal})
        assert(status == 42, "worker should return 42")
        assert(signal == 0)
        left = left - 1
        uv.close(child)
        uv.close(input)
        uv.close(pipe)
        if left == 0 then
          p("All workers are now dead")
          uv.close(server)
        end
      end)))
      p("Spawned worker", pid, "and sending handle", server)
      assert(uv.write(input, worker_code))
      assert(uv.write2(pipe, "123", server))
      assert(uv.shutdown(input))
      assert(uv.shutdown(pipe))
    end

    local function spawnClient()
      local input = uv.new_pipe(false)
      local child, pid
      child, pid = assert(uv.spawn(exepath, {
        stdio = {input,1,2},
        cwd = uv.cwd(),
        env= {
          "HOST=" .. address.ip,
          "PORT=" .. address.port,
        }
      }, expect(function (status, signal)
        p("Client exited", {status=status,signal=signal})
        assert(status == 0)
        assert(signal == 0)
        uv.close(child)
      end, left)))
      p("Spawned client", pid)
      assert(uv.write(input, client_code))
      assert(uv.shutdown(input))
      uv.close(input)
    end

    -- Spawn a child process for each CPU core
    for _ = 1, cpu_count do
      spawnWorker()
    end

    -- Spawn the clients after a short delay
    local timer = uv.new_timer()
    uv.timer_start(timer, 1000, 0, expect(function ()
      for _ = 1, cpu_count do
        spawnClient()
      end
      uv.close(timer)
    end))

  end)
end)

