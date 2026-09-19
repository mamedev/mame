-- license:BSD-3-Clause
-- copyright-holders:Ben Bruscella
--
-- pinviz table definition for Bally Power Play (1978).
--
-- Feature positions come from the machine's own layout (by17_pwerplay.lay), whose
-- playfield switch items carry pinviz ids. This file says what each switch is and
-- how it behaves, and takes the cabinet outline, shooter lane, lanes and flippers
-- from the shared Bally body. Solenoid roles are the ones named in the driver's
-- solenoid feature table in by17.cpp.
--
-- Three pop bumpers, two four-target drop banks standing upright either side, a
-- saucer at the top, rollover buttons up both sides and across the middle, and a
-- centre target. The playfield post that Sol 1 and Sol 17 raise and lower is not
-- simulated.

local body = require('pinviz/tables/bally_body')
local t = body.build{ lane = 19.25 }

t.lay_width = 19.0      -- the layout's panel draws the playfield, not the shooter lane
t.lay_length = 42.0
t.name = 'Power Play'
t.launch_speed = 88
t.serve_solenoids = {}
t.serve_on_flipper_enable = true
t.outhole_solenoid = 'solenoid6'
t.outhole_switch = { ':X0', 0x80 }
t.flipper_enable = 'solenoid18'

t.bumpers = {
	{ name = 'Left Thumper Bumper', switch = { ':X4', 0x80 }, r = 1.65, kick = 65 },   -- left, Sol 9
	{ name = 'Right Thumper Bumper', switch = { ':X4', 0x40 }, r = 1.65, kick = 65 },   -- right, Sol 10
	{ name = 'Bottom Thumper Bumper', switch = { ':X4', 0x20 }, r = 1.65, kick = 65 },   -- bottom, Sol 11
}

t.slings = {
	{ name = 'Left Slingshot', switch = { ':X4', 0x10 }, dir = 'u', len = 3.2, kick = 55 },   -- left, Sol 12
	{ name = 'Right Slingshot', switch = { ':X4', 0x08 }, dir = 'd', len = 3.2, kick = 55 },   -- right, Sol 14
	-- rubbers beside the drop target banks, both on the same switch
	{ name = 'Rebound Rubber', switch = { ':X0', 0x04 }, dir = 'v', len = 2.0, kick = 25 },
	{ name = 'Rebound Rubber', switch = { ':X0', 0x04 }, lay = ':2', dir = 'v', len = 2.0, kick = 25 },
	-- rubber at the top right
	{ name = 'Top Right Rubber', switch = { ':X0', 0x02 }, dir = 'v', len = 1.6, kick = 25 },
}

-- Drop target banks. Sol 13 clears the high nibble of strobe 2 (the left bank),
-- Sol 15 the low nibble (the right bank), per the driver's switch return masks.
t.targets = {
	{ name = 'Left Drop Target 1', switch = { ':X2', 0x80 }, dir = 'v', drop = true, reset = 'solenoid12' },
	{ name = 'Left Drop Target 2', switch = { ':X2', 0x40 }, dir = 'v', drop = true, reset = 'solenoid12' },
	{ name = 'Left Drop Target 3', switch = { ':X2', 0x20 }, dir = 'v', drop = true, reset = 'solenoid12' },
	{ name = 'Left Drop Target 4', switch = { ':X2', 0x10 }, dir = 'v', drop = true, reset = 'solenoid12' },
	{ name = 'Right Drop Target 1', switch = { ':X2', 0x08 }, dir = 'v', drop = true, reset = 'solenoid14' },
	{ name = 'Right Drop Target 2', switch = { ':X2', 0x04 }, dir = 'v', drop = true, reset = 'solenoid14' },
	{ name = 'Right Drop Target 3', switch = { ':X2', 0x02 }, dir = 'v', drop = true, reset = 'solenoid14' },
	{ name = 'Right Drop Target 4', switch = { ':X2', 0x01 }, dir = 'v', drop = true, reset = 'solenoid14' },
	-- centre target, the second item on the same switch as a top rollover
	{ name = 'Centre Target', switch = { ':X4', 0x01 }, lay = ':2', dir = 'h' },
}

-- the saucer holds the ball until Sol 8 kicks it back up the playfield
t.saucers = {
	{ name = 'Saucer', switch = { ':X3', 0x80 }, r = 0.9, solenoid = 'solenoid7', kick = { 0, -95 }, timeout = 6, timeout_kick = { 0, 15 } },
}

t.sensors = {
	{ name = 'Left Rollover 1', switch = { ':X3', 0x10 }, r = 0.7 },
	{ name = 'Left Rollover 2', switch = { ':X4', 0x02 }, r = 0.7 },
	{ name = 'Left Rollover 3', switch = { ':X0', 0x08 }, r = 0.7 },
	{ name = 'Right Rollover 1', switch = { ':X3', 0x20 }, r = 0.7 },
	{ name = 'Right Rollover 2', switch = { ':X4', 0x04 }, r = 0.7 },
	{ name = 'Right Rollover 3', switch = { ':X0', 0x10 }, r = 0.7 },
	{ name = 'Centre Rollover', switch = { ':X3', 0x02 }, r = 0.7 },
	{ name = 'Top Rollover', switch = { ':X4', 0x01 }, r = 0.7 },
	{ name = 'Lower Rollover Left', switch = { ':X3', 0x01 }, r = 0.7 },
	{ name = 'Lower Rollover Right', switch = { ':X3', 0x01 }, lay = ':2', r = 0.7 },
	{ name = 'Left Outlane', switch = { ':X3', 0x08 }, r = 0.7 },
	{ name = 'Right Outlane', switch = { ':X3', 0x04 }, r = 0.7 },
	{ name = 'Left Return Lane', switch = { ':X0', 0x01 }, r = 0.7 },
	{ name = 'Right Return Lane', switch = { ':X0', 0x01 }, lay = ':2', r = 0.7 },
}

t.solenoid_names = {
	[0] = 'Post Down', [1] = 'Chime 10', [2] = 'Chime 100', [3] = 'Chime 1000', [4] = 'Chime 10000',
	[5] = 'Knocker', [6] = 'Outhole', [7] = 'Saucer', [8] = 'Pop Bumper Left', [9] = 'Pop Bumper Right',
	[10] = 'Pop Bumper Bottom', [11] = 'Slingshot Left', [12] = 'Drop Target Reset Left',
	[13] = 'Slingshot Right', [14] = 'Drop Target Reset Right', [16] = 'Post Up',
	[17] = 'Coin Lockout', [18] = 'Flipper Enable Relay',
}

return t
