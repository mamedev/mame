return require('lib/tap')(function (test)

  local function bench(uv, p, count, fn)
    collectgarbage()
    local before
    local notify = count / 8
    for i = 1, count do
      fn()
      if i % notify == 0 then
        uv.run()
        collectgarbage()
        local now = uv.resident_set_memory()
        if not before then before = now end
        p(i, now)
      end
    end
    uv.run()
    collectgarbage()
    local after = uv.resident_set_memory()
    p{
      before = before,
      after = after,
    }
    assert(after < before * 1.5)
  end

  test("fs-write", function (print, p, expect, uv)
    bench(uv, p, 0x7000, function ()
      local path = "_test_"
      local fd = assert(uv.fs_open(path, "w", 438))
      uv.fs_write(fd, "Hello World\n", -1)
      uv.fs_write(fd, {"with\n", "more\n", "lines\n"}, -1)
      uv.fs_close(fd)
      uv.fs_unlink(path)
    end)
  end)

  test("lots-o-timers", function (print, p, expect, uv)
    bench(uv, p, 0x10000, function ()
      local timer = uv.new_timer()
      uv.close(timer)
    end)
  end)

  test("lots-o-timers with canceled callbacks", function (print, p, expect, uv)
    bench(uv, p, 0x10000, function ()
      local timer = uv.new_timer()
      uv.timer_start(timer, 100, 100, function ()
      end)
      uv.timer_stop(timer)
      uv.close(timer, function ()
      end)
      uv.run()
    end)
  end)

  test("lots-o-timers with real timeouts", function (print, p, expect, uv)
    bench(uv, p, 0x500, function ()
      local timer = uv.new_timer()
      uv.timer_start(timer, 10, 0, expect(function ()
        uv.timer_stop(timer)
        uv.close(timer, function ()
        end)
      end))
    end)
  end)

  test("reading file async", function (print, p, expect, uv)
    local mode = tonumber("644", 8)
    bench(uv, p, 0x500, function ()
      local onOpen, onStat, onRead, onClose
      local fd, stat

      onOpen = expect(function (err, result)
        assert(not err, err)
        fd = result
        uv.fs_fstat(fd, onStat)
      end)

      onStat = expect(function (err, result)
        assert(not err, err)
        stat = result
        uv.fs_read(fd, stat.size, 0, onRead)
      end)

      onRead = expect(function (err, data)
        assert(not err, err)
        assert(#data == stat.size)
        uv.fs_close(fd, onClose)
      end)

      onClose = expect(function (err)
        assert(not err, err)
      end)

      assert(uv.fs_open("README.md", "r", mode, onOpen))
    end)
  end)

  test("reading file sync", function (print, p, expect, uv)
    local mode = tonumber("644", 8)
    bench(uv, p, 0x2000, function ()
      local fd = assert(uv.fs_open("README.md", "r", mode))
      local stat = assert(uv.fs_fstat(fd))
      local data = assert(uv.fs_read(fd, stat.size, 0))
      assert(#data == stat.size)
      assert(uv.fs_close(fd))
    end)
  end)

  test("invalid file", function (print, p, expect, uv)
    local mode = tonumber("644", 8)
    bench(uv, p, 0x1500, function ()
      local req = uv.fs_open("BAD_FILE", "r", mode, expect(function (err, fd)
        assert(not fd)
        assert(err)
      end))
    end)
  end)

  test("invalid file sync", function (print, p, expect, uv)
    local mode = tonumber("644", 8)
    bench(uv, p, 0x20000, function ()
      local fd, err = uv.fs_open("BAD_FILE", "r", mode)
      assert(not fd)
      assert(err)
    end)
  end)

  test("invalid spawn args", function (print, p, expect, uv)
    -- Regression test for #73
    bench(uv, p, 0x10000, function ()
      local ret, err = pcall(function ()
        return uv.spawn("ls", {
          args = {"-l", "-h"},
          stdio = {0, 1, 2},
          env = {"EXTRA=true"},
          gid = false, -- Should be integer
        })
      end)
      assert(not ret)
      assert(err)
    end)
  end)

  test("stream writing with string and array", function (print, p, expect, uv)
    local port = 0
    local server = uv.new_tcp()
    local data
    local count = 0x800
    server:unref()
    server:bind("127.0.0.1", port)
    server:listen(128, expect(function (err)
      assert(not err, err)
      local client = uv.new_tcp()
      server:accept(client)
      client:write(data)
      client:read_start(expect(function (err, data)
        assert(not err, err)
        assert(data)
        client:close()
      end))
    end, count))
    local address = server:getsockname()
    bench(uv, p, count, function ()
      data = string.rep("Hello", 500)
      local socket = uv.new_tcp()
      socket:connect(address.ip, address.port, expect(function (err)
        assert(not err, err)
        socket:read_start(expect(function (err, chunk)
          assert(not err, err)
          assert(chunk)
          local data = {}
          for i = 0, 100 do
            data[i + 1] = string.rep(string.char(i), 100)
          end
          socket:write(data)
          socket:close()
        end))
      end))
      uv.run()
    end)
    server:close()
  end)

end)
