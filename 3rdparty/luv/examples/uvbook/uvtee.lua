local uv = require('luv')

if not arg[1] then
    print(string.format("please run %s filename",arg[0]))
    return
end


local stdin = uv.new_tty(0, true)
local stdout = uv.new_tty(1, true)
--local stdin_pipe = uv.new_pipe(false)
--uv.pipe_open(stdin_pipe,0)

local fname = arg[1]

uv.fs_open(fname, 'w+', tonumber('644', 8), function(err,fd)
    if err then
        print("error opening file:"..err)
    else
        local fpipe = uv.new_pipe(false)
        uv.pipe_open(fpipe, fd)
    
        uv.read_start(stdin, function(err,chunk)
            if err then
                print('Read error: '..err)
            else
                uv.write(stdout,chunk)
                uv.write(fpipe,chunk)
            end
        end);
    end
end)

uv.run('default')
uv.loop_close()
