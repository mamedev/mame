-- license:BSD-3-Clause
-- copyright-holders:Ben Bruscella
--
-- The parts of a playfield that every machine on a given cabinet shares: the
-- outline, the shooter lane and its one way gate, the outlane and inlane rails,
-- and the flippers. Bally's solid state cabinet kept the same body for years, so
-- a table can take this skeleton and add only its own features.
--
-- Units are inches, origin top left, y down the table towards the flippers.

local M = {}

-- opts.width, opts.length  playfield size, defaults to the Bally 20.25 x 42 body
-- opts.lane                left edge of the shooter lane
-- opts.flipper_y           height of the flipper pivots
-- opts.flipper_gap         distance of each pivot from the centre line
function M.build(opts)
	opts = opts or {}
	local W = opts.width or 20.25
	local L = opts.length or 42.0
	local LANE = opts.lane or 19.25
	local CX = LANE / 2
	local fy = opts.flipper_y or 38.6
	local fgap = opts.flipper_gap or 3.1
	local function mirror(x) return LANE - x end

	-- Top of the playfield. Bally's body is a flattened arch, not a semicircle: a
	-- semicircle is so tall that a plunged ball rides it all the way round and down
	-- the far wall, missing the playfield entirely.
	local walls = {}
	do
		local top = { { 0, 6.0 }, { 0.6, 3.6 }, { 2.0, 1.9 }, { 4.2, 0.9 }, { 7.0, 0.5 }, { 10.2, 0.4 },
			{ 13.5, 0.55 }, { 16.0, 0.95 }, { 18.0, 1.8 }, { 19.4, 3.2 }, { W, 5.5 } }
		for i = 1, #top - 1 do
			local a, b = top[i], top[i + 1]
			walls[#walls + 1] = { a[1] * W / 20.25, a[2], b[1] * W / 20.25, b[2] }
		end
	end

	local more = {
		-- sides and the shooter lane
		{ 0, W / 2, 0, L }, { W, W / 2, W, L },
		{ LANE, W / 2 + 1.6, LANE, L },
		-- outlane dividers, inlane guides, sling undersides
		{ 1.9, 27.0, 1.9, 36.0 }, { mirror(1.9), 27.0, mirror(1.9), 36.0 },
		{ 3.9, 29.5, 3.9, 31.4 }, { mirror(3.9), 29.5, mirror(3.9), 31.4 },
		{ 3.9, 35.4, 5.6, 35.4 }, { mirror(3.9), 35.4, mirror(5.6), 35.4 },
		-- inlane rails onto the flippers; a ball in the inlane reaches the flipper
		-- while a ball in the outlane drops past it. The rail stops just short of the
		-- pivot and a little above it: run all the way in and the rail's end and the
		-- bat form an acute corner at the pivot, and a ball that rolls into it is held
		-- there by both.
		{ 1.9, 36.0, CX - fgap - 0.55, fy - 0.30 },
		{ mirror(1.9), 36.0, mirror(CX - fgap - 0.55), fy - 0.30 },
		-- deflector at the top of the shooter lane, which turns a plunged ball into
		-- the playfield instead of letting it ride the top wall to the far side
		{ LANE + 0.1, 5.4, LANE - 2.6, 7.4 },
	}
	for _, s in ipairs(more) do walls[#walls + 1] = s end

	return {
		width = W, length = L,
		ball_radius = 0.53,
		gravity = 43.0,
		rolling_friction = 0.12,
		wall_restitution = 0.45,

		walls = walls,
		-- one way flap over the top of the shooter lane, sloping into the playfield
		gates = { { W, W / 2 + 0.2, LANE - 0.4, W / 2 + 1.6 } },

		serve_position = { LANE + 0.5, L - 2.5 },
		launch_speed = 115,
		drain_y = L - 0.7,
		drain_x = { 0, LANE },

		flippers = {
			{ pivot = { CX - fgap, fy }, length = 3.1, rest = 32, up = -28, key = 'KEYCODE_LSHIFT' },
			{ pivot = { mirror(CX - fgap), fy }, length = 3.1, rest = 148, up = 208, key = 'KEYCODE_RSHIFT' },
		},

		mirror = mirror,
	}
end

return M
