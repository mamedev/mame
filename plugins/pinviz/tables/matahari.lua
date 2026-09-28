-- license:BSD-3-Clause
-- copyright-holders:Ben Bruscella
--
-- pinviz table definition for Bally Mata Hari (1978).
--
-- Feature positions come from the machine's own layout (by17_matahari.lay), whose
-- playfield switch items carry pinviz ids. This file says what each switch is and
-- how it behaves, and takes the cabinet outline, shooter lane, lanes and flippers
-- from the shared Bally body. Solenoid roles are the ones named in the driver's
-- solenoid feature table in by17.cpp.
--
-- Four pop bumpers across the middle, two four-target drop banks angled down and
-- outwards, a saucer at the top, two top rollover lanes, side rollovers, and the
-- usual in and out lanes.

local body = require('pinviz/tables/bally_body')
local t = body.build{ lane = 19.25 }

t.lay_width = 19.0      -- the layout's panel draws the playfield, not the shooter lane
t.lay_length = 42.0
t.name = 'Mata Hari'
t.launch_speed = 88
t.serve_solenoids = {}
t.serve_on_flipper_enable = true
t.outhole_solenoid = 'solenoid6'
t.outhole_switch = { ':X0', 0x80 }
t.flipper_enable = 'solenoid18'

t.bumpers = {
	{ name = 'Left Top Thumper Bumper', switch = { ':X4', 0x80 }, r = 1.65, kick = 65 },   -- left top, Sol 9
	{ name = 'Right Top Thumper Bumper', switch = { ':X4', 0x40 }, r = 1.65, kick = 65 },   -- right top, Sol 10
	{ name = 'Left Bottom Thumper Bumper', switch = { ':X4', 0x20 }, r = 1.65, kick = 65 },   -- left bottom, Sol 8
	{ name = 'Right Bottom Thumper Bumper', switch = { ':X4', 0x10 }, r = 1.65, kick = 65 },   -- right bottom, Sol 11
}

t.slings = {
	{ name = 'Left Slingshot', switch = { ':X4', 0x08 }, dir = 'u', len = 3.2, kick = 55 },   -- left, Sol 12
	{ name = 'Right Slingshot', switch = { ':X4', 0x04 }, dir = 'd', len = 3.2, kick = 55 },   -- right, Sol 14
	-- rubbers beside the drop target banks, both on the same switch
	{ name = 'Rebound Rubber', switch = { ':X3', 0x04 }, dir = 'v', len = 2.0, kick = 25 },
	{ name = 'Rebound Rubber', switch = { ':X3', 0x04 }, lay = ':2', dir = 'v', len = 2.0, kick = 25 },
}

-- Drop target banks. Sol 13 clears the high nibble of strobe 2 (the left bank),
-- Sol 15 the low nibble (the right bank), per the driver's switch return masks.
t.targets = {
	{ name = 'Left Drop Target 1', switch = { ':X2', 0x80 }, dir = 'h', drop = true, reset = 'solenoid12' },
	{ name = 'Left Drop Target 2', switch = { ':X2', 0x40 }, dir = 'h', drop = true, reset = 'solenoid12' },
	{ name = 'Left Drop Target 3', switch = { ':X2', 0x20 }, dir = 'h', drop = true, reset = 'solenoid12' },
	{ name = 'Left Drop Target 4', switch = { ':X2', 0x10 }, dir = 'h', drop = true, reset = 'solenoid12' },
	{ name = 'Right Drop Target 1', switch = { ':X2', 0x08 }, dir = 'h', drop = true, reset = 'solenoid14' },
	{ name = 'Right Drop Target 2', switch = { ':X2', 0x04 }, dir = 'h', drop = true, reset = 'solenoid14' },
	{ name = 'Right Drop Target 3', switch = { ':X2', 0x02 }, dir = 'h', drop = true, reset = 'solenoid14' },
	{ name = 'Right Drop Target 4', switch = { ':X2', 0x01 }, dir = 'h', drop = true, reset = 'solenoid14' },
}

-- the saucer holds the ball until Sol 1 kicks it back up the playfield
t.saucers = {
	{ name = 'Saucer', switch = { ':X3', 0x80 }, r = 1.0, solenoid = 'solenoid0', kick = { 0, -95 }, timeout = 6, timeout_kick = { 0, 15 } },
}

t.sensors = {
	{ name = 'Top Lane Left', switch = { ':X3', 0x40 }, r = 0.7 },   -- top lane left
	{ name = 'Top Lane Right', switch = { ':X3', 0x20 }, r = 0.7 },   -- top lane right
	{ name = 'Left Side Rollover', switch = { ':X3', 0x10 }, r = 0.7 },   -- side rollover left
	{ name = 'Right Side Rollover', switch = { ':X3', 0x08 }, r = 0.7 },   -- side rollover right
	{ name = 'Left Outlane', switch = { ':X4', 0x02 }, r = 0.7 },   -- left outlane
	{ name = 'Right Outlane', switch = { ':X4', 0x01 }, r = 0.7 },   -- right outlane
	{ name = 'Left Return Lane', switch = { ':X3', 0x02 }, r = 0.7 },   -- left return lane
	{ name = 'Right Return Lane', switch = { ':X3', 0x01 }, r = 0.7 },   -- right return lane
}

t.solenoid_names = {
	[0] = 'Saucer', [1] = 'Chime 10', [2] = 'Chime 100', [3] = 'Chime 1000', [4] = 'Chime 10000',
	[5] = 'Knocker', [6] = 'Outhole', [7] = 'Pop Bumper Left Bottom', [8] = 'Pop Bumper Left Top',
	[9] = 'Pop Bumper Right Top', [10] = 'Pop Bumper Right Bottom', [11] = 'Slingshot Left',
	[12] = 'Drop Target Reset Left', [13] = 'Slingshot Right', [14] = 'Drop Target Reset Right',
	[17] = 'Coin Lockout', [18] = 'Flipper Enable Relay',
}

return t
