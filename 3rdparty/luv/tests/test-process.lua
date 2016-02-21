return require('lib/tap')(function (test)

  test("test disable_stdio_inheritance", function (print, p, expect, uv)
    uv.disable_stdio_inheritance()
  end)

  test("process stdout", function (print, p, expect, uv)
    local stdout = uv.new_pipe(false)

    local handle, pid
    handle, pid = uv.spawn(uv.exepath(), {
      args = {"-e", "print 'Hello World'"},
      stdio = {nil, stdout},
    }, expect(function (code, signal)
      p("exit", {code=code, signal=signal})
      uv.close(handle)
    end))

    p{
      handle=handle,
      pid=pid
    }

    uv.read_start(stdout, expect(function (err, chunk)
      p("stdout", {err=err,chunk=chunk})
      assert(not err, err)
      uv.close(stdout)
    end))

  end)

  if _G.isWindows then return end

  test("spawn and kill by pid", function (print, p, expect, uv)
    local handle, pid
    handle, pid = uv.spawn("sleep", {
      args = {1},
    }, expect(function (status, signal)
      p("exit", handle, {status=status,signal=signal})
      assert(status == 0)
      assert(signal == 2)
      uv.close(handle)
    end))
    p{handle=handle,pid=pid}
    uv.kill(pid, "sigint")
  end)

  test("spawn and kill by handle", function (print, p, expect, uv)
    local handle, pid
    handle, pid = uv.spawn("sleep", {
      args = {1},
    }, expect(function (status, signal)
      p("exit", handle, {status=status,signal=signal})
      assert(status == 0)
      assert(signal == 15)
      uv.close(handle)
    end))
    p{handle=handle,pid=pid}
    uv.process_kill(handle, "sigterm")
  end)

  test("invalid command", function (print, p, expect, uv)
    local handle, err
    handle, err = uv.spawn("ksjdfksjdflkjsflksdf", {}, function(exit, code)
      assert(false)
    end)
    assert(handle == nil)
    assert(err)
  end)

  test("process stdio", function (print, p, expect, uv)
    local stdin = uv.new_pipe(false)
    local stdout = uv.new_pipe(false)

    local handle, pid
    handle, pid = uv.spawn("cat", {
      stdio = {stdin, stdout},
    }, expect(function (code, signal)
      p("exit", {code=code, signal=signal})
      uv.close(handle)
    end))

    p{
      handle=handle,
      pid=pid
    }

    uv.read_start(stdout, expect(function (err, chunk)
      p("stdout", {err=err,chunk=chunk})
      assert(not err, err)
      uv.close(stdout)
    end))

    uv.write(stdin, "Hello World")
    uv.shutdown(stdin, expect(function ()
      uv.close(stdin)
    end))

  end)

end)
