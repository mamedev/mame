return require('lib/tap')(function (test)

  test("test pass async between threads", function(p, p, expect, uv)
    local before = os.time()
    local async
    async = uv.new_async(expect(function (a,b,c)
      p('in async notify callback')
      p(a,b,c)
      assert(a=='a')
      assert(b==true)
      assert(c==250)
      uv.close(async)
    end))
    local args = {500, 'string', nil, false, 5, "helloworld",async}
    local unpack = unpack or table.unpack
    uv.new_thread(function(num,s,null,bool,five,hw,asy)
	  local uv = require'luv'
      assert(type(num) == "number")
      assert(type(s) == "string")
      assert(null == nil)
      assert(bool == false)
      assert(five == 5)
      assert(hw == 'helloworld')
      assert(type(asy)=='userdata')
      assert(uv.async_send(asy,'a',true,250)==0)
      uv.sleep(1000)
    end, unpack(args)):join()
    local elapsed = (os.time() - before) * 1000
    assert(elapsed >= 1000, "elapsed should be at least delay ")
  end)

end)
