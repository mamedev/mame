-- license:BSD-3-Clause
-- copyright-holders:Ben Bruscella
--
-- pinviz table definition for Williams High Speed (1986), System 11.
--
-- Switch numbers and solenoid functions are Williams' own, from the machine's
-- tech chart: 64 switches on eight strobes, 16 controlled solenoids and six
-- special solenoids. The System 11 driver publishes all of them as out0 to
-- out21, where outN is solenoid N+1 for the controlled ones, and the specials
-- land on out16 to out21.
--
-- Feature positions come from the machine's layout (s11_hs.lay). That layout
-- arranges the real features sensibly rather than tracing the real playfield,
-- so this plays like High Speed's parts list, not like High Speed.

local W, L = 20.25, 42.0
local LANE = 18.6
local CX = 9.3
local function mirror(x) return 2 * CX - x end

local walls = {}
do
	-- flattened arch, as on the Bally body; Williams' cabinet top is similar
	local top = { { 0, 6.0 }, { 0.6, 3.6 }, { 2.0, 1.9 }, { 4.2, 0.9 }, { 7.0, 0.5 }, { 10.2, 0.4 },
		{ 13.5, 0.55 }, { 16.0, 0.95 }, { 18.0, 1.8 }, { 19.4, 3.2 }, { W, 5.5 } }
	for i = 1, #top - 1 do walls[#walls + 1] = { top[i][1], top[i][2], top[i + 1][1], top[i + 1][2] } end
end
local more = {
	{ 0, 6.0, 0, L }, { W, 5.5, W, L },
	{ LANE, 9.0, LANE, L },                                     -- shooter lane
	{ LANE + 0.1, 5.0, LANE - 2.6, 7.0 },                       -- lane exit deflector
	-- outlane dividers and inlane rails
	{ 2.6, 25.0, 2.6, 31.5 }, { mirror(2.6), 25.0, mirror(2.6), 31.5 },
	{ 2.6, 31.5, CX - 3.95, 34.3 }, { mirror(2.6), 31.5, mirror(CX - 3.95), 34.3 },
	-- sling undersides
	{ 3.6, 31.0, 5.4, 31.0 }, { mirror(3.6), 31.0, mirror(5.4), 31.0 },
	-- apron edges that funnel a drained ball towards the outhole
	{ 0, 36.4, CX - 2.4, 39.6 }, { LANE, 36.4, mirror(CX - 2.4), 39.6 },
}
for _, s in ipairs(more) do walls[#walls + 1] = s end

return {
	name = 'High Speed',
	width = W, length = L,
	ball_radius = 0.53,
	gravity = 43.0,
	rolling_friction = 0.12,
	wall_restitution = 0.45,

	walls = walls,
	gates = { { W, 5.6, LANE - 0.4, 7.2 } },
	posts = { { CX, 17.4, 0.45 }, { 4.4, 22.0, 0.4 }, { mirror(4.4), 22.0, 0.4 } },

	-- System 11 publishes its solenoids and lamps as out0 to out85
	output_prefix = 'out',

	-- Sol 2 feeds the shooter lane from the trough, Sol 1 kicks the outhole into it.
	-- The game will not start until the trough reads full.
	trough = { { ':X1', 0x08 }, { ':X1', 0x04 }, { ':X1', 0x02 } },   -- trough 3, 2, 1
	serve_solenoids = { 'out1' },
	serve_position = { LANE + 0.7, 39.0 },
	launch_speed = 96,
	outhole_solenoid = 'out0',
	outhole_switch = { ':X1', 0x01 },
	drain_y = 40.6,
	drain_x = { 0, LANE },   -- anything off the playfield reaches the outhole

	flippers = {
		{ pivot = { CX - 3.4, 34.6 }, length = 3.1, rest = 32, up = -28, key = 'KEYCODE_LSHIFT' },
		{ pivot = { mirror(CX - 3.4), 34.6 }, length = 3.1, rest = 148, up = 208, key = 'KEYCODE_RSHIFT' },
	},

	slings = {
		{ name = 'Left Slingshot', switch = { ':X6', 0x01 }, dir = 'u', len = 3.2, kick = 55 },
		{ name = 'Right Slingshot', switch = { ':X6', 0x02 }, dir = 'd', len = 3.2, kick = 55 },
	},

	bumpers = {
		{ name = 'Upper Left Jet Bumper', switch = { ':X4', 0x01 }, r = 1.575, kick = 65 },
		{ name = 'Lower Left Jet Bumper', switch = { ':X4', 0x02 }, r = 1.575, kick = 65 },
		{ name = 'Right Jet Bumper', switch = { ':X4', 0x04 }, r = 1.575, kick = 65 },
	},

	targets = {
		-- three stoplight banks, red yellow green; these are stand-up targets
		{ name = 'Upper Left Stoplight Red', switch = { ':X2', 0x01 }, dir = 'v' },
		{ name = 'Upper Left Stoplight Yellow', switch = { ':X2', 0x02 }, dir = 'v' },
		{ name = 'Upper Left Stoplight Green', switch = { ':X2', 0x04 }, dir = 'v' },
		{ name = 'Lower Left Stoplight Red', switch = { ':X1', 0x10 }, dir = 'v' },
		{ name = 'Lower Left Stoplight Yellow', switch = { ':X1', 0x20 }, dir = 'v' },
		{ name = 'Lower Left Stoplight Green', switch = { ':X1', 0x40 }, dir = 'v' },
		{ name = 'Right Stoplight Red', switch = { ':X2', 0x20 }, dir = 'v' },
		{ name = 'Right Stoplight Yellow', switch = { ':X2', 0x40 }, dir = 'v' },
		{ name = 'Right Stoplight Green', switch = { ':X2', 0x80 }, dir = 'v' },
		-- six standup target arrows across the upper playfield
		{ name = 'Standup Target Arrow 1', switch = { ':X3', 0x01 }, dir = 'h' },
		{ name = 'Standup Target Arrow 2', switch = { ':X3', 0x02 }, dir = 'h' },
		{ name = 'Standup Target Arrow 3', switch = { ':X3', 0x04 }, dir = 'h' },
		{ name = 'Standup Target Arrow 4', switch = { ':X3', 0x08 }, dir = 'h' },
		{ name = 'Standup Target Arrow 5', switch = { ':X3', 0x10 }, dir = 'h' },
		{ name = 'Standup Target Arrow 6', switch = { ':X3', 0x20 }, dir = 'h' },
	},

	-- the eject hole and the four hideouts all hold the ball for a kicker
	saucers = {
		{ name = 'Eject Hole', switch = { ':X1', 0x80 }, r = 0.9, solenoid = 'out2', kick = { 0, -80 }, timeout = 6, timeout_kick = { 0, 14 } },
		{ name = 'Upper Left Hideout', switch = { ':X4', 0x40 }, r = 0.8, solenoid = 'out6', kick = { 22, 10 }, timeout = 6, timeout_kick = { 8, 12 } },
		{ name = 'Lower Left Hideout', switch = { ':X4', 0x80 }, r = 0.8, solenoid = 'out6', kick = { 22, -10 }, timeout = 6, timeout_kick = { 8, 12 } },
		{ name = 'Upper Right Hideout', switch = { ':X5', 0x40 }, r = 0.8, solenoid = 'out7', kick = { -22, 10 }, timeout = 6, timeout_kick = { -8, 12 } },
		{ name = 'Lower Right Hideout', switch = { ':X5', 0x80 }, r = 0.8, solenoid = 'out7', kick = { -22, -10 }, timeout = 6, timeout_kick = { -8, 12 } },
	},

	sensors = {
		{ name = 'Left Ramp', switch = { ':X5', 0x02 }, r = 0.7 },
		{ name = 'Right Ramp', switch = { ':X5', 0x04 }, r = 0.7 },
		{ name = 'Left Spinner', switch = { ':X5', 0x08 }, r = 0.7 },
		{ name = 'Centre Spinner', switch = { ':X5', 0x10 }, r = 0.7 },
		{ name = 'Right Spinner', switch = { ':X5', 0x20 }, r = 0.7 },
		{ name = 'Left Star Rollover', switch = { ':X6', 0x04 }, r = 0.8 },
		{ name = 'Right Star Rollover', switch = { ':X6', 0x08 }, r = 0.8 },
		{ name = 'Left Return Lane', switch = { ':X2', 0x08 }, r = 0.7 },
		{ name = 'Right Return Lane', switch = { ':X2', 0x10 }, r = 0.7 },
		{ name = 'Left Outlane', switch = { ':X3', 0x40 }, r = 0.7 },
		{ name = 'Right Outlane', switch = { ':X3', 0x80 }, r = 0.7 },
		{ name = 'Ball Shooter Lane', switch = { ':X4', 0x08 }, r = 0.7 },
	},

	-- outN is solenoid N+1, so these are the tech chart's numbers less one. The six
	-- special solenoids are left as letters: High Speed fires its slingshots and jet
	-- bumpers from their own switches in hardware, not from the CPU, so which letter
	-- drives which coil cannot be confirmed by watching the machine play.
	solenoid_names = {
		[0] = 'Outhole Kicker', [1] = 'Ball Shooter Lane Feeder', [2] = 'Eject Hole',
		[3] = 'Police Light Relay', [4] = 'Flasher 2', [5] = 'Flasher 3',
		[6] = 'Left Hideout', [7] = 'Right Hideout', [8] = 'Flasher 1',
		[9] = 'Insert Board Flashers', [10] = 'General Illumination Relay', [11] = 'Flasher 4',
		[12] = 'Ramp Gates', [13] = 'Kickback', [14] = 'Knocker', [15] = 'Coin Lockout Relay',
		[16] = 'Special D', [17] = 'Special B', [18] = 'Special C',
		[19] = 'Special A', [20] = 'Special E', [21] = 'Special F',
	},
}
