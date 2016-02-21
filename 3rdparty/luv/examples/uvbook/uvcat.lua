local uv = require('luv')


local fname = arg[1] and arg[1] or arg[0]

uv.fs_open(fname, 'r', tonumber('644', 8), function(err,fd)
    if err then
        print("error opening file:"..err)
    else
        local stat = uv.fs_fstat(fd)
        local off = 0
        local block = 10
        
        local function on_read(err,chunk)
            if(err) then
                print("Read error: "..err);
            elseif #chunk==0 then
                uv.fs_close(fd)
            else
                off = block + off
                uv.fs_write(1,chunk,-1,function(err,chunk)
                    if err then
                        print("Write error: "..err)
                    else
                        uv.fs_read(fd, block, off, on_read)
                    end
                end)
            end
        end
        uv.fs_read(fd, block, off, on_read)
    end
end)



uv.run('default')
uv.loop_close()
