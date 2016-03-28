-- hiscore.lua
-- by borgar@borgar.net, WTFPL license
-- 
-- This uses MAME's built-in Lua scripting to implment
-- high-score saving with hiscore.dat infom just as older
-- builds did in the past.
-- 

local exports = {}
exports.name = "hiscore"
exports.version = "1.0.0"
exports.description = "Hiscore"
exports.license = "WTFPL license"
exports.author = { name = "borgar@borgar.net" }
local hiscore = exports

local hiscore_plugin_path = ""

function hiscore.set_folder(path)
	hiscore_plugin_path = path
end

function hiscore.startplugin()

	hiscoredata_path = "hiscore.dat";
	hiscore_path = "hi";

	current_checksum = 0;
	default_checksum = 0;

	scores_have_been_read = false;
	mem_check_passed = false;
	current_game = "";

	positions = {};

	function parse_table ( dsting )
	  local _table = {};
	  for line in string.gmatch(dsting, '([^\n]+)') do
		local cpu, mem;
		cputag, space, offs, len, chk_st, chk_ed = string.match(line, '^@([^,]+),([^,]+),([^,]+),([^,]+),([^,]+),([^,]+)');
		cpu = manager:machine().devices[cputag];
		if not cpu then
		  return nil;
		end
		mem = cpu.spaces[space];
		if not mem then
		  return nil;
		end
		_table[ #_table + 1 ] = {
		  mem = mem,
		  addr = tonumber(offs, 16),
		  size = tonumber(len, 16),
		  c_start = tonumber(chk_st, 16),
		  c_end = tonumber(chk_ed, 16),
		};
	  end
	  return _table;
	end


	function read_hiscore_dat ()
	  file = io.open( hiscoredata_path, "r" );
	  if not file then
		file = io.open( hiscore_plugin_path .. "/hiscore.dat", "r" );
	  end
	  if emu.softname() ~= "" then
		rm_match = '^' .. emu.romname() .. ',' .. emu.softname() .. ':';
	  else
	    rm_match = '^' .. emu.romname() .. ':';
      end
	  cluster = "";
	  current_is_match = false;
	  if file then
		repeat
		  line = file:read("*l");
		  if line then
			-- remove comments
			line = line:gsub( '[ \t\r\n]*;.+$', '' );
			-- handle lines
			if string.find(line, '^@') then -- data line
			  if current_is_match then
				cluster = cluster .. "\n" .. line;
			  end
			elseif string.find(line, rm_match) then --- match this game
			  current_is_match = true;
			elseif string.find(line, '^[a-z0-9_]+:') then --- some game
			  if current_is_match and string.len(cluster) > 0 then
				break; -- we're done
			  end
			else --- empty line or garbage
			  -- noop
			end
		  end
		until not line;
		file:close();
	  end
	  return cluster;
	end


	function check_mem ( posdata )
	  if #posdata < 1 then
		return false;
	  end
	  for ri,row in ipairs(posdata) do
		-- must pass mem check
		if row["c_start"] ~= row["mem"]:read_u8(row["addr"]) then
		  return false;
		end
		if row["c_end"] ~= row["mem"]:read_u8(row["addr"]+row["size"]-1) then
		  return false;
		end
	  end
	  return true;
	end


	function get_file_name ()
	  if emu.softname() ~= "" then
	  	r = hiscore_path .. '/' .. emu.romname() .. "_" .. emu.softname() .. ".hi";
	  else
	  	r = hiscore_path .. '/' .. emu.romname() .. ".hi";
	  end
	  return r;
	end


	function write_scores ( posdata )
	print("write_scores")
	  local output = io.open(get_file_name(), "wb");
	  if not output then
		-- attempt to create the directory, and try again
		os.execute( "mkdir " .. hiscore_path );
		output = io.open(get_file_name(), "wb");
	  end
	  print("write_scores output")
	  if output then
		for ri,row in ipairs(posdata) do
		  t = {};
		  for i=0,row["size"]-1 do
			t[i+1] = row["mem"]:read_u8(row["addr"] + i)
		  end
		  output:write(string.char(table.unpack(t)));
		end
		output:close();
	  end
	  print("write_scores end")
	end


	function read_scores ( posdata )
	  local input = io.open(get_file_name(), "rb");
	  if input then
		for ri,row in ipairs(posdata) do
		  local str = input:read(row["size"]);
		  for i=0,row["size"]-1 do
			local b = str:sub(i+1,i+1):byte();
			row["mem"]:write_u8( row["addr"] + i, b );
		  end
		end
		input:close();
		return true;
	  end
	  return false;
	end


	function check_scores ( posdata )
	  local r = 0;
	  -- commonly the first entry will be for the entire table
	  -- so it will only trigger a write once a player enters
	  -- his/her name in.
	  row = positions[1];
	  for i=0,row["size"]-1 do
		r = r + row["mem"]:read_u8( row["addr"] + i );
	  end
	  return r;
	end


	function init ()
	  if not scores_have_been_read then
		if check_mem( positions ) then
		  default_checksum = check_scores( positions );
		  if read_scores( positions ) then
			print( "scores read", "OK" );
		  else
			-- likely there simply isn't a .hi file around yet
			print( "scores read", "FAIL" );
		  end
		  scores_have_been_read = true;
		  current_checksum = check_scores( positions );
		  mem_check_passed = true;
		else
		  -- memory check can fail while the game is still warming up
		  -- TODO: only allow it to fail N many times
		end
	  end
	end


	local last_write_time = -10;
	function tick ()
	  -- set up scores if they have been
	  init();
	  -- only allow save check to run when 
	  if mem_check_passed then
		-- The reason for this complicated mess is that
		-- MAME does expose a hook for "exit". Once it does,
		-- this should obviously just be done when the emulator
		-- shuts down (or reboots).
		local checksum = check_scores( positions );
		if checksum ~= current_checksum and checksum ~= default_checksum then
		  -- 5 sec grace time so we don't clobber io and cause
		  -- latency. This would be bad as it would only ever happen
		  -- to players currently reaching a new highscore
		  if emu.time() > last_write_time + 5 then
			write_scores( positions );
			current_checksum = checksum;
			last_write_time = emu.time();
			-- print( "SAVE SCORES EVENT!", last_write_time );
		  end
		end
	  end
	end

	function reset()
	  -- the notifier will still be attached even if the running game has no hiscore.dat entry
	  if mem_check_passed and (emu.romname() == current_game) then
		local checksum = check_scores(positions)
		if checksum ~= current_checksum and checksum ~= default_checksum then
		  write_scores(positions)
		end
	  end
	  current_game = ""
	  mem_check_passed = false
   	  scores_have_been_read = false;
	end
	
	emu.register_start(function()
		current_game = ""
		mem_check_passed = false
	   	scores_have_been_read = false;
	  	print("Starting " .. emu.gamename())
		-- check if we've just soft reset
		-- reset() -- there's no way to reliably save scores after a soft reset currently 
		current_game = emu.romname()
		dat = read_hiscore_dat()
		if dat and dat ~= "" then
			print( "found hiscore.dat entry for " .. emu.romname() );
			positions = parse_table( dat );
			if not positions then
				print("hiscore.dat parse error");
				return;
			end
			emu.sethook( tick, "frame" );
			emu.register_stop(function()
				reset()
			end)
		end		

	end)
    
end

return exports
