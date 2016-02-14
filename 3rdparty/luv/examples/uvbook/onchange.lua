local uv = require('luv')

if #arg==0 then
    print(string.format("Usage: %s <command> <file1> [file2 ...]",arg[0]));
    return
end

for i=1,#arg do
    local fse = uv.new_fs_event()
    assert(uv.fs_event_start(fse,arg[i],{
        --"watch_entry"=true,"stat"=true,
        recursive=true
    },function (err,fname,status)
        if(err) then
            print("Error "..err)
        else
            print(string.format('Change detected in %s',
                uv.fs_event_getpath(fse)))
            for k,v in pairs(status) do
                print(k,v)
            end                
            print('file changed:'..(fname and fname or ''))
        end
    end))
    
end

uv.run('default')
uv.loop_close()

