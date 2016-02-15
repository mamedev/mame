return require('lib/tap')(function (test)
  test("test threadpool", function(print,p,expect,_uv)
    p('Please be patient, the test cost a lots of time')
    local count = 1000 --for memleaks dected
    local step = 0
    local ctx
    ctx = _uv.new_work(
        function(n,s) --work,in threadpool
            local uv = require('luv')
            local t = uv.thread_self()
            uv.sleep(100)
            return n,n*n, tostring(uv.thread_self()),s
        end,
        function(n,r,id, s)
            assert(n*n==r)
            if step < count then
                _uv.queue_work(ctx,n,s)
                step = step + 1
                if (step % 100==0) then
                    p(string.format('run %d%%', math.floor(step*100/count)))
                end
            end
        end    --after work, in loop thread
    )
    local ls = string.rep('-',4096)

    _uv.queue_work(ctx,2,ls)
    _uv.queue_work(ctx,4,ls)
    _uv.queue_work(ctx,6,ls)
    _uv.queue_work(ctx,-2,ls)
    _uv.queue_work(ctx,-11,ls)
    _uv.queue_work(ctx,2,ls)
    _uv.queue_work(ctx,4,ls)
    _uv.queue_work(ctx,6,ls)
    _uv.queue_work(ctx,-2,ls)
    _uv.queue_work(ctx,-11,ls)
    _uv.queue_work(ctx,2,ls)
    _uv.queue_work(ctx,4,ls)
    _uv.queue_work(ctx,6,ls)
    _uv.queue_work(ctx,-2,ls)
    _uv.queue_work(ctx,-11,ls)
    _uv.queue_work(ctx,2,ls)
    _uv.queue_work(ctx,4,ls)
    _uv.queue_work(ctx,6,ls)
    _uv.queue_work(ctx,-2,ls)
    _uv.queue_work(ctx,-11,ls)
  end)
end)
