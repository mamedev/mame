-- license:BSD-3-Clause
-- copyright-holders:Ben Bruscella
--
-- pinviz table definition for Bally Playboy (1978).
--
-- Feature positions come from the machine's own layout (by35_playboy.lay), whose
-- playfield switch items carry pinviz ids. This file says what each switch is and
-- how it behaves, and takes the cabinet outline, shooter lane, lanes and flippers
-- from the shared Bally body. Solenoid roles are the ones the layout names.
--
-- Three pop bumpers, a five target drop bank standing on a diagonal to the right,
-- five stand-up targets down the left, four top lanes, a centre target, and the
-- kickback grotto on the far left that Sol 8 fires back up the playfield.

local body = require('pinviz/tables/bally_body')
local t = body.build{ lane = 19.25 }

t.lay_width = 19.0      -- the layout's panel draws the playfield, not the shooter lane
t.lay_length = 42.0
t.name = 'Playboy'
t.launch_speed = 88
t.serve_solenoids = {}
t.serve_on_flipper_enable = true
t.outhole_solenoid = 'solenoid6'
t.outhole_switch = { ':X0', 0x80 }
t.flipper_enable = 'solenoid18'

t.bumpers = {
	{ name = 'Left Thumper Bumper', switch = { ':X4', 0x80 }, r = 1.65, kick = 65 },
	{ name = 'Right Thumper Bumper', switch = { ':X4', 0x40 }, r = 1.65, kick = 65 },
	{ name = 'Bottom Thumper Bumper', switch = { ':X4', 0x20 }, r = 1.65, kick = 65 },
}

t.slings = {
	{ name = 'Left Slingshot', switch = { ':X4', 0x10 }, dir = 'u', len = 3.2, kick = 55 },
	{ name = 'Right Slingshot', switch = { ':X4', 0x08 }, dir = 'd', len = 3.2, kick = 55 },
	{ name = 'Rebound Rubber', switch = { ':X4', 0x01 }, dir = 'v', len = 2.0, kick = 25 },
}

t.targets = {
	-- five drop targets on a diagonal, reset by Sol 13
	{ name = 'Drop Target 1', switch = { ':X0', 0x10 }, dir = 'v', drop = true, reset = 'solenoid12' },
	{ name = 'Drop Target 2', switch = { ':X0', 0x08 }, dir = 'v', drop = true, reset = 'solenoid12' },
	{ name = 'Drop Target 3', switch = { ':X0', 0x04 }, dir = 'v', drop = true, reset = 'solenoid12' },
	{ name = 'Drop Target 4', switch = { ':X0', 0x02 }, dir = 'v', drop = true, reset = 'solenoid12' },
	{ name = 'Drop Target 5', switch = { ':X0', 0x01 }, dir = 'v', drop = true, reset = 'solenoid12' },
	-- five stand-up targets down the left
	{ name = 'Left Target 1', switch = { ':X3', 0x10 }, dir = 'v' },
	{ name = 'Left Target 2', switch = { ':X3', 0x08 }, dir = 'v' },
	{ name = 'Left Target 3', switch = { ':X3', 0x04 }, dir = 'v' },
	{ name = 'Left Target 4', switch = { ':X3', 0x02 }, dir = 'v' },
	{ name = 'Left Target 5', switch = { ':X3', 0x01 }, dir = 'v' },
	{ name = 'Centre Target', switch = { ':X2', 0x01 }, dir = 'h' },
}

-- the kickback grotto holds the ball until Sol 8 fires it back up the playfield
t.saucers = {
	{ name = 'Kickback Grotto', switch = { ':X3', 0x80 }, r = 0.9, solenoid = 'solenoid7',
	  kick = { 6, -90 }, timeout = 5, timeout_kick = { 0, 15 } },
}

t.sensors = {
	{ name = 'Top Lane 1', switch = { ':X2', 0x10 }, r = 0.7 },
	{ name = 'Top Lane 2', switch = { ':X2', 0x08 }, r = 0.7 },
	{ name = 'Top Lane 3', switch = { ':X2', 0x04 }, r = 0.7 },
	{ name = 'Top Lane 4', switch = { ':X2', 0x02 }, r = 0.7 },
	{ name = 'Right Lane Rollover', switch = { ':X3', 0x40 }, r = 0.7 },
	{ name = 'Rollover Button', switch = { ':X3', 0x20 }, r = 0.8 },
	{ name = 'Left Outlane', switch = { ':X2', 0x40 }, r = 0.7 },
	{ name = 'Right Outlane', switch = { ':X2', 0x20 }, r = 0.7 },
	{ name = 'Left Return Lane', switch = { ':X2', 0x80 }, r = 0.7 },
	{ name = 'Right Return Lane', switch = { ':X2', 0x80 }, lay = ':2', r = 0.7 },
}

t.solenoid_names = {
	[5] = 'Knocker', [6] = 'Outhole', [7] = 'Kickback Grotto', [8] = 'Left Thumper Bumper',
	[9] = 'Right Thumper Bumper', [10] = 'Bottom Thumper Bumper', [11] = 'Left Slingshot',
	[12] = 'Drop Target Reset', [13] = 'Right Slingshot', [17] = 'Coin Lockout',
	[18] = 'Flipper Enable Relay',
}

return t
