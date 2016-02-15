return require('lib/tap')(function (test)

  test("test thread create", function(print, p, expect, uv)
    local delay = 1000
    local before = os.time()
    local thread = uv.new_thread(function(delay)
      require('luv').sleep(delay)
    end,delay)
    uv.thread_join(thread)
    local elapsed = (os.time() - before) * 1000
    p({
      delay = delay,
      elapsed = elapsed
    })
    assert(elapsed >= delay, "elapsed should be at least delay ")
  end)

  test("test thread create with arguments", function(print, p, expect, uv)
    local before = os.time()
    local args = {500, 'string', nil, false, 5, "helloworld"}
    local unpack = unpack or table.unpack
    uv.new_thread(function(num,s,null,bool,five,hw)
      assert(type(num) == "number")
      assert(type(s) == "string")
      assert(null == nil)
      assert(bool == false)
      assert(five == 5)
      assert(hw == 'helloworld')
      require('luv').sleep(1000)
    end, unpack(args)):join()
    local elapsed = (os.time() - before) * 1000
    assert(elapsed >= 1000, "elapsed should be at least delay ")
  end)

  test("test thread sleep msecs in main thread", function(print, p, expect, uv)
    local delay = 1000
    local before = os.time()
    uv.sleep(delay)
    local elapsed = (os.time() - before) * 1000
    p({
      delay = delay,
      elapsed = elapsed
    })
    assert(elapsed >= delay, "elapsed should be at least delay ")
  end)

end)
