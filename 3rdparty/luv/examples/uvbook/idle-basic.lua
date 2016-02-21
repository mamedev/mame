local uv = require('luv')

local counter = 0
local idle = uv.new_idle()
idle:start(function()
    counter = counter + 1
    if counter >= 10e6 then
        idle:stop()
    end
end)

print("Idling...")
uv.run('default')
uv.loop_close()