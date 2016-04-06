xml = require("xml_conv")
json = dofile("../json/init.lua")

function readAll(file)
    local f = io.open(file, "rb")
    local content = f:read("*all")
    f:close()
    return content
end

print(json.stringify(xml.conv_cheat(readAll(arg[1]))))
