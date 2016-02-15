local child_code = string.dump(function ()
  local uv = require('luv')
  local signal = uv.new_signal()
  uv.ref(signal)
  uv.signal_start(signal, "sigint", function ()
    uv.unref(signal)
  end)
  uv.run()
  os.exit(7)
end)

return require('lib/tap')(function (test)

  if _G.isWindows then return end

  test("Catch SIGINT", function (print, p, expect, uv)
    local child, pid
    local input = uv.new_pipe(false)
    child, pid = assert(uv.spawn(uv.exepath(), {
      args = {"-"},
      -- cwd = uv.cwd(),
      stdio = {input,1,2}
    }, expect(function (code, signal)
      p("exit", {pid=pid,code=code,signal=signal})
      assert(code == 7)
      assert(signal == 0)
      uv.close(input)
      uv.close(child)
    end)))
    uv.write(input, child_code)
    uv.shutdown(input)
    local timer = uv.new_timer()
    uv.timer_start(timer, 200, 0, expect(function ()
      print("Sending child SIGINT")
      uv.process_kill(child, "sigint")
      uv.close(timer)
    end))
  end)

end)
