local tap_module = {}

local os = require("os")

local counter = 1
local failed  = false

function tap_module.ok(assert_true, desc)
   local msg = ( assert_true and "ok " or "not ok " ) .. counter
   if ( not assert_true ) then
      failed = true
   end
   if ( desc ) then
      msg = msg .. " - " .. desc
   end
   print(msg)
   counter = counter + 1
end

function tap_module.exit()
   os.exit(failed and 1 or 0)
end

return tap_module
