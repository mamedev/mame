return require('lib/tap')(function (test)

  -- This tests using timers for a simple timeout.
  -- It also tests the handle close callback and
  test("simple timeout", function (print, p, expect, uv)
    local timer = uv.new_timer()
    local function onclose()
      p("closed", timer)
    end
    local function ontimeout()
      p("timeout", timer)
      uv.close(timer, expect(onclose))
    end
    uv.timer_start(timer, 10, 0, expect(ontimeout))
  end)

  -- This is like the previous test, but using repeat.
  test("simple interval", function (print, p, expect, uv)
    local timer = uv.new_timer()
    local count = 3
    local onclose = expect(function ()
      p("closed", timer)
    end)
    local function oninterval()
      p("interval", timer)
      count = count - 1
      if count == 0 then
        uv.close(timer, onclose)
      end
    end
    uv.timer_start(timer, 10, 10, oninterval)
  end)

  -- Test two concurrent timers
  -- There is a small race condition, but there are 100ms of wiggle room.
  -- 400ms is halfway between 100+200ms and 100+400ms
  test("timeout with interval", function (print, p, expect, uv)
    local a = uv.new_timer()
    local b = uv.new_timer()
    uv.timer_start(a, 400, 0, expect(function ()
      p("timeout", a)
      uv.timer_stop(b)
      uv.close(a)
      uv.close(b)
    end))
    uv.timer_start(b, 100, 200, expect(function ()
      p("interval", b)
    end, 2))
  end)

  -- This advanced test uses the rest of the uv_timer_t functions
  -- to create an interval that shrinks over time.
  test("shrinking interval", function (print, p, expect, uv)
    local timer = uv.new_timer()
    uv.timer_start(timer, 10, 0, expect(function ()
      local r = uv.timer_get_repeat(timer)
      p("interval", timer, r)
      if r == 0 then
        uv.timer_set_repeat(timer, 8)
        uv.timer_again(timer)
      elseif r == 2 then
        uv.timer_stop(timer)
        uv.close(timer)
      else
        uv.timer_set_repeat(timer, r / 2)
      end
    end, 4))
  end)

  test("shrinking interval using methods", function (print, p, expect, uv)
    local timer = uv.new_timer()
    timer:start(10, 0, expect(function ()
      local r = timer:get_repeat()
      p("interval", timer, r)
      if r == 0 then
        timer:set_repeat(8)
        timer:again()
      elseif r == 2 then
        timer:stop()
        timer:close()
      else
        timer:set_repeat(r / 2)
      end
    end, 4))
  end)

end)
