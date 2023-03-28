local exports = {
	name = "autopatch",
	version = "1",
	description = "Automatically apply corresponding patch",
	license = "CC0",
	author = { name = "jflatt" }}

local autopatch = exports

function autopatch.startplugin()
	emu.register_start(function()
		--check if patch file exists
		local basePath = manager.plugins['autopatch'].directory
		local romName = emu.romname()
		local softName = emu.softname()
		local fullPath = basePath .. '/' .. romName .. '/' .. softName .. '.ips'
		local file = io.open(fullPath, 'rb')

		if file then
		    --test that it's a valid IPS patch
		    local patchTest = file:read(5)
		    if not patchTest then
			file:close()
			emu.print_verbose("Could not read header")
			return
		    end
		    if patchTest ~= 'PATCH' then
			file:close()
			emu.print_verbose("Invalid header")
			return
		    end

		    local romRegion = ({
			--['nes'] = ':nes_slot:cart:chr_rom',
			['snes'] = ':snsslot:cart:rom',
			['genesis'] = ':mdslot:cart:rom',
			['megadriv'] = ':mdslot:cart:rom',
		    })[romName];
		    
		    --read IPS file
		    while true do
			--each offset chunk is 3 bytes, and the EOF marker is 3 bytes
			local offsetBytes = file:read(3)
			if not offsetBytes then
			    file:close()
			    emu.print_verbose("Could not read offset")
			    return
			end
			if offsetBytes == 'EOF' then
			    emu.print_verbose("End of file found")
			    break
			end
			local offset = string.unpack('>I3', offsetBytes)

			--read in the data length
			local lenBytes = file:read(2)
			local length = string.unpack('>I2', lenBytes)
			
			if length == 0 then
			    --It's an RLE chunk
			    local runBytes = file:read(2) --run count
			    local run = string.unpack('>I2', runBytes)
			    local value = file:read(1) --byte to repeat
			    
			    --write to rom contents
			    for i = offset, offset + run, 1 do
				manager.machine.memory.regions[romRegion]:write_u8(i, value)
			    end
			elseif length > 0 then
			    --It's a normal chunk
			    local value = file:read(length)

			    --write to rom contents
			    for i = 0, length - 1, 1 do
				manager.machine.memory.regions[romRegion]:write_u8(offset + i, value:byte(i + 1))
			    end
			else
			    file:close()
			    emu.print_verbose("Bad IPS file")
			    return
			end
		    end

		    file:close()
		    local msg = "Auto patch plugin using " .. fullPath
		    emu.print_info(msg)
		    manager.machine:popmessage(msg)
		end
	end)
end

return exports
