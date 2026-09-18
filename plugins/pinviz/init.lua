-- license:BSD-3-Clause
-- copyright-holders:Ben Bruscella
--
-- pinviz: a minimal pinball playfield for testing MAME pinball drivers.
--
-- The emulated machine is the real thing: the ROM, switch matrix, solenoids,
-- lamps, displays and sound. This plugin adds the one part MAME does not have,
-- a ball. It is enough physics to play a game and watch the driver react, not a
-- simulator: one ball, walls, flippers, slingshots, pop bumpers, targets and
-- lanes, drawn as lines over the left part of the window.
--
-- A table definition (plugins/pinviz/tables/<system>.lua) says where things are
-- and which input port field each one closes. Solenoid outputs from the driver
-- drive the ball the other way: serving, the outhole kick, drop target resets
-- and the flipper enable relay.
--
-- Keys: left/right flipper as named in the table (Shift keys for Centaur),
-- Space to plunge. Environment: PINVIZ_AUTOPILOT=1 plays by itself,
-- PINVIZ_LOG=1 prints the event log to the console.

local exports = {
	name = 'pinviz',
	version = '0.1.0',
	description = 'Minimal pinball playfield simulation for testing pinball drivers',
	license = 'BSD-3-Clause',
	author = { name = 'Ben Bruscella' } }

local pinviz = exports

local frame_subscription, start_subscription, stop_subscription

function pinviz.startplugin()

	local sim = nil            -- simulation state, nil when the running machine has no table
	local autopilot = os.getenv('PINVIZ_AUTOPILOT') == '1'
	local fliptest = tonumber(os.getenv('PINVIZ_FLIPTEST') or '0')   -- ball speed for the flipper test, 0 = off
	local console_log = os.getenv('PINVIZ_LOG') == '1'
	-- the flipper test drops the ball at five points along the bat; set to 0 to drop it
	-- at one point, which makes a run repeatable and any spread left the physics
	local flipspread = os.getenv('PINVIZ_FLIPSPREAD') ~= '0'

	local SUBSTEPS = 12
	local FLIPPER_THICKNESS = 0.22   -- half width of the bat, inches
	local FLIPPER_RESTITUTION = 0.35 -- flipper rubber, bouncier than a wall
	local FLIPPER_FRICTION = 0.12    -- drag along the rubber, gives a ball met off centre some curve
	local FRAME_DT = 1 / 60
	local SWITCH_FRAMES = 5     -- how long a hit holds a matrix switch closed
	local BALL_RESTITUTION_MIN_SPEED = 2.0
	-- A pinball leaves the plunger at about 100 in/s and a flipper can send it faster,
	-- but nothing on a playfield reaches this. Clamping keeps a bad contact, where the
	-- ball is squeezed between two surfaces and pumped, from throwing it off the table.
	local MAX_SPEED = 260

	-- --------------------------------------------------------------------
	-- helpers
	-- --------------------------------------------------------------------

	local function load_table(name)
		local ok, tbl = pcall(require, 'pinviz/tables/' .. name)
		if ok and type(tbl) == 'table' then
			return tbl
		end
		return nil
	end

	local function output_proxy(name)
		local root = manager.machine.devices[':']
		return root:output(name)
	end

	local function field(sw)
		local port = manager.machine.ioport.ports[sw[1]]
		if not port then return nil end
		return port:field(sw[2])
	end

	local function log(s, msg)
		local t = manager.machine.time.seconds + (manager.machine.time.msec / 1000)
		local line = string.format('%5.1f %s', t, msg)
		table.insert(s.events, 1, line)
		if #s.events > 14 then table.remove(s.events) end
		if console_log then print('[pinviz] ' .. line) end
	end

	-- --------------------------------------------------------------------
	-- switch handling: a hit closes a switch for a few frames
	-- --------------------------------------------------------------------

	local function pulse_switch(s, sw, name)
		local key = sw[1] .. '/' .. sw[2]
		local p = s.pulses[key]
		if not p then
			local f = field(sw)
			if not f then return end
			p = { field = f, frames = 0, name = name }
			s.pulses[key] = p
		end
		if p.frames == 0 then
			p.field:set_value(1)
			log(s, 'sw  ' .. (name or key))
			if name then s.flash[name] = 12 end
		end
		p.frames = SWITCH_FRAMES
	end

	local function hold_switch(s, sw, on)
		local f = field(sw)
		if not f then return end
		if on then f:set_value(1) else f:clear_value() end
	end

	local function update_pulses(s)
		for key, p in pairs(s.pulses) do
			if p.frames > 0 then
				p.frames = p.frames - 1
				if p.frames == 0 then p.field:clear_value() end
			end
		end
	end

	-- --------------------------------------------------------------------
	-- solenoid outputs: rising edges drive the ball
	-- --------------------------------------------------------------------

	local function watch_output(s, name)
		if not s.outputs[name] then
			s.outputs[name] = { proxy = output_proxy(name), last = 0, rose = false }
		end
	end

	local function update_outputs(s)
		for name, o in pairs(s.outputs) do
			local v = o.proxy:get() or 0
			o.rose = (v ~= 0) and (o.last == 0)
			o.last = v
			if o.rose then
				local prefix = s.tbl.output_prefix or 'solenoid'
				local idx = tonumber(name:match('^' .. prefix .. '(%d+)$'))
				local label = idx and s.tbl.solenoid_names and s.tbl.solenoid_names[idx]
				log(s, 'sol ' .. (label or name))
				if label then s.flash[label] = 12 end
			end
		end
	end

	local function rose(s, name)
		local o = s.outputs[name]
		return o and o.rose
	end

	local function output_on(s, name)
		local o = s.outputs[name]
		return o and o.last ~= 0
	end

	-- --------------------------------------------------------------------
	-- positions from the machine's layout
	--
	-- A per-game layout can carry the playfield picture. Items with ids
	-- "pinviz:pfx" and "pinviz:pfy" mark the playfield panel's extent, and an
	-- item with id "pinviz:<port>:<mask>" (a second one "...:2") marks where the
	-- switch is. The table then only has to say what each switch is.
	-- --------------------------------------------------------------------

	-- width to height ratio of the layout view as MAME is showing it
	local function view_aspect()
		local ok, a = pcall(function() return manager.machine.render.ui_target.current_view.effective_aspect end)
		if ok and a and a > 0 then return a end
		return 0
	end

	local function layout_reader(tbl)
		local view = manager.machine.render.ui_target.current_view
		if not view then return nil end
		local function item(id)
			local ok, it = pcall(function() return view.items[id] end)
			if ok then return it end
			return nil
		end
		local pfx, pfy = item('pinviz:pfx'), item('pinviz:pfy')
		if not (pfx and pfy) then return nil end
		local bx, by = pfx.bounds, pfy.bounds
		local x0, xw = bx.x0, bx.x1 - bx.x0
		local y0, yh = by.y0, by.y1 - by.y0
		if xw <= 0 or yh <= 0 then return nil end
		-- how much of the table the layout's panel covers. A mockup layout often draws
		-- only the playfield and leaves no room for the shooter lane, so a table can say
		-- the panel is narrower than the table is.
		local lw = tbl.lay_width or tbl.width
		local ll = tbl.lay_length or tbl.length
		local r = { x0 = x0, xw = xw, y0 = y0, yh = yh, lw = lw, ll = ll }
		-- playfield inches to UI coordinates, matching the layout's own stretch
		function r.to_ui(x, y) return x0 + x / lw * xw, y0 + y / ll * yh end
		-- rectangle of a switch item in inches: centre and half sizes
		function r.rect(sw, suffix)
			local port = sw[1]:gsub('^:', '')
			local id = string.format('pinviz:%s:0x%02x', port, sw[2]) .. (suffix or '')
			local it = item(id)
			if not it then return nil end
			local b = it.bounds
			local ix0, ix1 = (b.x0 - x0) / xw * lw, (b.x1 - x0) / xw * lw
			local iy0, iy1 = (b.y0 - y0) / yh * ll, (b.y1 - y0) / yh * ll
			return { cx = (ix0 + ix1) / 2, cy = (iy0 + iy1) / 2, hw = (ix1 - ix0) / 2, hh = (iy1 - iy0) / 2, x0 = ix0, y0 = iy0, x1 = ix1, y1 = iy1 }
		end
		return r
	end

	-- give each feature coordinates: from the layout when it has the switch, else
	-- from the numbers in the table. Segment features take an orientation letter
	-- (h, v, d, u) and an optional length in inches.
	local function place_features(s)
		local tbl, lay = s.tbl, s.lay
		local placed, kept, dropped = 0, 0, 0
		local function segment_from(rc, f)
			local dir, len = f.dir or 'h', f.len
			local x0, y0, x1, y1
			-- a rubber that lies along a rail is not the diagonal of the little switch
			-- symbol that marks it, so a table may give the angle itself, in degrees
			-- clockwise from the x axis, the same convention as the flipper angles
			if f.angle then
				local a = math.rad(f.angle)
				local half = (len or math.sqrt((rc.x1 - rc.x0) ^ 2 + (rc.y1 - rc.y0) ^ 2)) / 2
				f[1], f[2] = rc.cx - math.cos(a) * half, rc.cy - math.sin(a) * half
				f[3], f[4] = rc.cx + math.cos(a) * half, rc.cy + math.sin(a) * half
				return
			end
			if dir == 'v' then x0, y0, x1, y1 = rc.cx, rc.y0, rc.cx, rc.y1
			elseif dir == 'd' then x0, y0, x1, y1 = rc.x0, rc.y0, rc.x1, rc.y1
			elseif dir == 'u' then x0, y0, x1, y1 = rc.x0, rc.y1, rc.x1, rc.y0
			else x0, y0, x1, y1 = rc.x0, rc.cy, rc.x1, rc.cy end
			if len then
				local dx, dy = x1 - x0, y1 - y0
				local l = math.sqrt(dx * dx + dy * dy)
				if l > 1e-6 then
					local k = len / l / 2
					x0, y0, x1, y1 = rc.cx - dx * k, rc.cy - dy * k, rc.cx + dx * k, rc.cy + dy * k
				end
			end
			f[1], f[2], f[3], f[4] = x0, y0, x1, y1
		end
		local function circle_from(rc, f)
			f[1], f[2] = rc.cx, rc.cy
			f[3] = f.r or f[3] or math.max(rc.hw, rc.hh)
		end
		local function name_from_port(f)
			if f.name or not f.switch then return end
			local port = manager.machine.ioport.ports[f.switch[1]]
			local fld = port and port:field(f.switch[2])
			if fld then f.name = fld.name end
		end
		local function resolve(list, kind)
			local out = {}
			for _, f in ipairs(list or {}) do
				name_from_port(f)
				local rc = lay and f.switch and lay.rect(f.switch, f.lay)
				if rc then
					if kind == 'circle' then circle_from(rc, f) else segment_from(rc, f) end
					emu.print_verbose(string.format('pinviz: %-28s from layout at %.1f,%.1f %s', f.name or '?', f[1], f[2], kind == 'circle' and string.format('r=%.2f', f[3]) or string.format('to %.1f,%.1f', f[3], f[4])))
					placed = placed + 1
					out[#out + 1] = f
				elseif f[1] then
					kept = kept + 1
					out[#out + 1] = f
				else
					dropped = dropped + 1
					log(s, 'no position for ' .. (f.name or '?'))
				end
			end
			return out
		end
		tbl.sensors = resolve(tbl.sensors, 'circle')
		tbl.bumpers = resolve(tbl.bumpers, 'circle')
		tbl.saucers = resolve(tbl.saucers, 'circle')
		tbl.targets = resolve(tbl.targets, 'segment')
		tbl.slings = resolve(tbl.slings, 'segment')
		return placed, kept, dropped
	end

	-- --------------------------------------------------------------------
	-- ball and geometry
	-- --------------------------------------------------------------------

	-- push the ball out of a segment and reflect it. sv is the surface velocity
	-- (moving flippers), kick an extra outward speed (slings, bumpers).
	local function collide_segment(b, x0, y0, x1, y1, e, svx, svy, kick, thick)
		local dx, dy = x1 - x0, y1 - y0
		local len2 = dx * dx + dy * dy
		if len2 < 1e-9 then return false end
		local t = ((b.x - x0) * dx + (b.y - y0) * dy) / len2
		if t < 0 then t = 0 elseif t > 1 then t = 1 end
		local cx, cy = x0 + t * dx, y0 + t * dy
		local nx, ny = b.x - cx, b.y - cy
		local d = math.sqrt(nx * nx + ny * ny)
		local reach = b.r + (thick or 0)
		if d >= reach or d < 1e-6 then return false end
		nx, ny = nx / d, ny / d
		b.x, b.y = cx + nx * reach, cy + ny * reach
		local rvx, rvy = b.vx - (svx or 0), b.vy - (svy or 0)
		local vn = rvx * nx + rvy * ny
		if vn < 0 then
			b.vx = b.vx - (1 + e) * vn * nx
			b.vy = b.vy - (1 + e) * vn * ny
		end
		if kick and kick > 0 then
			local cur = b.vx * nx + b.vy * ny
			if cur < kick then
				b.vx = b.vx + (kick - cur) * nx
				b.vy = b.vy + (kick - cur) * ny
			end
		end
		return true, nx, ny, t
	end

	-- shortest distance between the ball's travel this substep (p0->p1) and the bat
	-- segment (a->b); returns the closest point on the bat and the parameter along it.
	local function seg_seg_closest(p0x, p0y, p1x, p1y, ax, ay, bx, by)
		local ux, uy = p1x - p0x, p1y - p0y
		local vx, vy = bx - ax, by - ay
		local wx, wy = p0x - ax, p0y - ay
		local a = ux * ux + uy * uy
		local bb = ux * vx + uy * vy
		local c = vx * vx + vy * vy
		local d = ux * wx + uy * wy
		local e = vx * wx + vy * wy
		local den = a * c - bb * bb
		local sc, tc
		if den < 1e-9 then sc = 0 else sc = (bb * e - c * d) / den end
		sc = math.max(0, math.min(1, sc))
		tc = (bb * sc + e) / (c < 1e-9 and 1 or c)
		tc = math.max(0, math.min(1, tc))
		sc = (bb * tc - d) / (a < 1e-9 and 1 or a)
		sc = math.max(0, math.min(1, sc))
		local qx, qy = p0x + sc * ux, p0y + sc * uy       -- closest point on the ball path
		local rx, ry = ax + tc * vx, ay + tc * vy         -- closest point on the bat
		local dx, dy = qx - rx, qy - ry
		return math.sqrt(dx * dx + dy * dy), rx, ry, tc, qx, qy
	end

	local function collide_circle(b, cx, cy, cr, e, kick)
		local nx, ny = b.x - cx, b.y - cy
		local d = math.sqrt(nx * nx + ny * ny)
		local rr = cr + b.r
		if d >= rr or d < 1e-6 then return false end
		nx, ny = nx / d, ny / d
		b.x, b.y = cx + nx * rr, cy + ny * rr
		local vn = b.vx * nx + b.vy * ny
		if vn < 0 then
			b.vx = b.vx - (1 + e) * vn * nx
			b.vy = b.vy - (1 + e) * vn * ny
		end
		if kick and kick > 0 then
			local cur = b.vx * nx + b.vy * ny
			if cur < kick then
				b.vx = b.vx + (kick - cur) * nx
				b.vy = b.vy + (kick - cur) * ny
			end
		end
		return true
	end

	local function flipper_ends(f)
		local a = math.rad(f.angle)
		return f.pivot[1], f.pivot[2], f.pivot[1] + f.length * math.cos(a), f.pivot[2] + f.length * math.sin(a)
	end

	local function new_ball(s, x, y, vx, vy)
		s.ball = { x = x, y = y, vx = vx or 0, vy = vy or 0, r = s.tbl.ball_radius }
	end

	-- Machines with a ball trough will not start until the trough reads full. The
	-- trough holds its switches closed for the balls that are home: the outhole
	-- kicker returns a drained ball to it, and the shooter lane feeder takes one out.
	local function trough_update(s)
		local tr = s.tbl.trough
		if not tr then return end
		for i, sw in ipairs(tr) do hold_switch(s, sw, i <= s.trough_balls) end
	end

	local function serve(s, why)
		local p = s.tbl.serve_position
		new_ball(s, p[1], p[2], 0, 0)
		s.state = 'shooter'
		s.shooter_frames = 0
		log(s, 'ball to shooter lane (' .. why .. ')')
	end

	local function plunge(s)
		local b = s.ball
		local spd = s.tbl.launch_speed * (0.9 + 0.2 * math.random())
		b.vx, b.vy = 0, -spd
		s.state = 'play'
		log(s, 'plunge')
	end

	-- --------------------------------------------------------------------
	-- one physics step
	-- --------------------------------------------------------------------

	local function step(s, dt)
		local tbl, b = s.tbl, s.ball
		if not b or s.state ~= 'play' then return end

		-- gravity down the incline, rolling friction
		local px, py = b.x, b.y   -- where the ball was at the start of this substep

		b.vy = b.vy + tbl.gravity * dt
		local damp = 1 - tbl.rolling_friction * dt
		b.vx, b.vy = b.vx * damp, b.vy * damp

		b.x = b.x + b.vx * dt
		b.y = b.y + b.vy * dt

		-- flippers move a little each substep towards where the buttons want them
		for _, f in ipairs(s.flippers) do
			f.prev_angle = f.angle
			local delta = f.target - f.angle
			local move = f.rate * dt
			if math.abs(delta) <= move then f.angle = f.target else f.angle = f.angle + (delta > 0 and move or -move) end
			f.omega = math.rad(f.angle - f.prev_angle) / dt
		end

		local e = tbl.wall_restitution

		for _, w in ipairs(tbl.walls) do
			collide_segment(b, w[1], w[2], w[3], w[4], e)
		end
		for _, p in ipairs(tbl.posts or {}) do
			collide_circle(b, p[1], p[2], p[3], 0.6)
		end
		-- one way gates (shooter lane top): solid for a ball on the blocked side of the
		-- directed segment (positive cross product), open from the other side
		for _, g in ipairs(tbl.gates or {}) do
			local cross = (g[3] - g[1]) * (b.y - g[2]) - (g[4] - g[2]) * (b.x - g[1])
			if cross > 0 then
				collide_segment(b, g[1], g[2], g[3], g[4], 0.3)
			end
		end

		for i, f in ipairs(s.flippers) do
			local x0, y0, x1, y1 = flipper_ends(f)
			local reach = b.r + FLIPPER_THICKNESS
			local dx, dy = x1 - x0, y1 - y0
			local l2 = dx * dx + dy * dy
			local hx, hy, hrx, hry, ht    -- contact: ball point, bat point, position along the bat

			-- the usual case, the ball where it is now against the bat where it is now,
			-- resolved from its own position like any other surface
			local tt = l2 > 1e-9 and ((b.x - x0) * dx + (b.y - y0) * dy) / l2 or 0
			tt = math.max(0, math.min(1, tt))
			local crx, cry = x0 + tt * dx, y0 + tt * dy
			if math.sqrt((b.x - crx) ^ 2 + (b.y - cry) ^ 2) < reach then
				hx, hy, hrx, hry, ht = b.x, b.y, crx, cry, tt
			else
				-- nothing touching now, so look for a contact this substep stepped over:
				-- a fast ball through the bat, or a fast bat past the ball. The bat is
				-- tested where it started as well as where it ended. Only this case uses
				-- the ball's path, because only here has the ball gone somewhere it
				-- should not have reached.
				local sd, srx, sry, st, sqx, sqy = seg_seg_closest(px, py, b.x, b.y, x0, y0, x1, y1)
				if f.prev_angle ~= f.angle then
					local pa = math.rad(f.prev_angle)
					local ex, ey = f.pivot[1] + f.length * math.cos(pa), f.pivot[2] + f.length * math.sin(pa)
					local d2, rx2, ry2, t2, qx2, qy2 = seg_seg_closest(px, py, b.x, b.y, f.pivot[1], f.pivot[2], ex, ey)
					if d2 < sd then sd, srx, sry, st, sqx, sqy = d2, rx2, ry2, t2, qx2, qy2 end
				end
				if sd < reach then hx, hy, hrx, hry, ht = sqx, sqy, srx, sry, st end
			end

			if hx then
				-- put the ball just off the bat on the approach side
				local nx, ny = hx - hrx, hy - hry
				local nl = math.sqrt(nx * nx + ny * ny)
				if nl < 1e-6 then
					local l = math.sqrt(l2)
					nx, ny = -dy / l, dx / l
					if (px - hrx) * nx + (py - hry) * ny < 0 then nx, ny = -nx, -ny end
				else nx, ny = nx / nl, ny / nl end
				b.x, b.y = hrx + nx * reach, hry + ny * reach
				-- One impulse, taken against the ball's velocity relative to the bat
				-- surface at the contact: a bat sweeping into the ball throws it, a bat
				-- at rest only bounces it, and nothing is added on top. Because the ball
				-- then leaves faster than the bat, the remaining substeps of a long flip
				-- find it moving away and leave it alone instead of pumping it.
				local d = ht * f.length
				local a = math.rad(f.angle)
				local svx, svy = -f.omega * d * math.sin(a), f.omega * d * math.cos(a)
				local rvx, rvy = b.vx - svx, b.vy - svy
				local vn = rvx * nx + rvy * ny
				if vn < 0 then
					b.vx = b.vx - (1 + FLIPPER_RESTITUTION) * vn * nx
					b.vy = b.vy - (1 + FLIPPER_RESTITUTION) * vn * ny
					-- a little drag along the rubber, so a ball met off centre picks up
					-- some sideways speed rather than sliding free
					local tx, ty = -ny, nx
					local vt = rvx * tx + rvy * ty
					b.vx = b.vx - FLIPPER_FRICTION * vt * tx
					b.vy = b.vy - FLIPPER_FRICTION * vt * ty
				end
			end
		end

		for i, sl in ipairs(tbl.slings or {}) do
			local hit = collide_segment(b, sl[1], sl[2], sl[3], sl[4], 0.5, 0, 0, sl.kick)
			if hit then pulse_switch(s, sl.switch, sl.name) end
		end

		for i, bp in ipairs(tbl.bumpers or {}) do
			local hit = collide_circle(b, bp[1], bp[2], bp[3], 0.5, bp.kick)
			if hit then pulse_switch(s, bp.switch, bp.name) end
		end

		for i, tg in ipairs(tbl.targets or {}) do
			if not s.dropped[i] then
				local hit = collide_segment(b, tg[1], tg[2], tg[3], tg[4], 0.6)
				if hit then
					pulse_switch(s, tg.switch, tg.name)
					if tg.drop then
						s.dropped[i] = true
						log(s, 'target down: ' .. tg.name)
					end
				end
			end
		end

		for i, sc in ipairs(tbl.saucers or {}) do
			local dx, dy = b.x - sc[1], b.y - sc[2]
			if s.saucer_cooldown == 0 and (dx * dx + dy * dy) < (sc[3] * sc[3]) then
				b.x, b.y, b.vx, b.vy = sc[1], sc[2], 0, 0
				s.state = 'saucer'
				s.saucer = i
				hold_switch(s, sc.switch, true)
				log(s, 'ball in ' .. sc.name)
				return
			end
		end

		for i, sn in ipairs(tbl.sensors or {}) do
			local dx, dy = b.x - sn[1], b.y - sn[2]
			local inside = (dx * dx + dy * dy) < (sn[3] * sn[3])
			if inside and not s.in_sensor[i] then
				pulse_switch(s, sn.switch, sn.name)
			end
			s.in_sensor[i] = inside
		end

		-- keep the ball to a speed the real thing could reach
		local sp2 = b.vx * b.vx + b.vy * b.vy
		if sp2 > MAX_SPEED * MAX_SPEED then
			local k = MAX_SPEED / math.sqrt(sp2)
			b.vx, b.vy = b.vx * k, b.vy * k
		end

		-- keep the ball on the table
		if b.x < b.r then b.x, b.vx = b.r, math.abs(b.vx) end
		if b.x > tbl.width - b.r then b.x, b.vx = tbl.width - b.r, -math.abs(b.vx) end
		if b.y < b.r then b.y, b.vy = b.r, math.abs(b.vy) end

		-- last resort: if the ball has left the table despite all of the above, treat
		-- it as drained rather than letting it fall for ever
		if b.y > tbl.length + 1 or b.y < -1 then
			b.x = math.min(math.max(b.x, b.r), tbl.width - b.r)
			b.y, b.vx, b.vy = tbl.drain_y or (tbl.length - 1), 0, 0
			s.state = 'outhole'
			hold_switch(s, tbl.outhole_switch, true)
			log(s, 'ball left the table, counted as drained')
			return
		end

		-- a ball that has stopped against geometry gets a nudge, as a player would
		local speed2 = b.vx * b.vx + b.vy * b.vy
		if speed2 < 0.25 then
			s.still = (s.still or 0) + 1
			if s.still > 3 * 60 * SUBSTEPS then
				b.vx = b.vx + (math.random() - 0.5) * 40
				b.vy = b.vy - 20 - math.random() * 20
				s.still = 0
				log(s, 'nudge (ball stuck)')
			end
		else
			s.still = 0
		end

		local in_drain_zone = b.y > tbl.drain_y
		if tbl.drain_x then in_drain_zone = in_drain_zone and b.x >= tbl.drain_x[1] and b.x <= tbl.drain_x[2] end
		if in_drain_zone then
			s.state = 'outhole'
			b.vx, b.vy = 0, 0
			b.y = tbl.drain_y
			hold_switch(s, tbl.outhole_switch, true)
			log(s, 'drain, ball in outhole')
		end
	end

	-- --------------------------------------------------------------------
	-- per frame: inputs, outputs, physics, drawing
	-- --------------------------------------------------------------------

	local function update_flippers(s)
		-- the flipper test drives the bat itself, so it does not wait for the game to
		-- energise the relay
		local enabled = fliptest > 0 or s.tbl.flipper_enable == nil or output_on(s, s.tbl.flipper_enable)
		local input = manager.machine.input
		for i, f in ipairs(s.flippers) do
			local pressed = enabled and (input:code_pressed(f.code) or ((autopilot or fliptest > 0) and s.auto_flip[i] > 0))
			f.target = pressed and f.up or f.rest
			f.rate = 1400 -- degrees per second
		end
		s.flippers_enabled = enabled
	end

	-- Flipper regression test: drop a fast ball onto the left flipper, flip when it
	-- arrives, and count whether it was hit or passed through the bat.
	local function update_fliptest(s)
		local ft = s.ft
		if not ft then ft = { phase = 0, frames = 0, hits = 0, through = 0, trials = 0, exits = {}, angs = {} }; s.ft = ft end
		ft.frames = ft.frames + 1
		local f = s.flippers[1]
		if ft.phase == 0 and ft.frames > 60 then
			new_ball(s, f.pivot[1] + 1.6 + (flipspread and (ft.trials % 5) * 0.3 or 0.6), f.pivot[2] - 6, 0, fliptest)
			s.state = 'play'; s.flippers_enabled = true; ft.phase = 1; ft.frames = 0
		elseif ft.phase == 1 then
			-- flip early enough that the bat is mid sweep when the ball arrives, which
			-- is what a player does; the lead scales with the ball's speed
			if s.ball.y > f.pivot[2] - 1.2 - fliptest * 0.020 then s.auto_flip[1] = 10; ft.phase = 2; ft.frames = 0 end
			if ft.frames > 120 then ft.phase = 3 end
		elseif ft.phase == 2 then
			-- the shot is whatever the bat threw: record the ball the moment it is
			-- travelling back up the table, so exits can be compared trial to trial
			if not ft.exit and s.ball and s.ball.vy < 0 then
				local b = s.ball
				ft.exit = math.sqrt(b.vx * b.vx + b.vy * b.vy)
				ft.exit_ang = math.deg(math.atan(b.vx, -b.vy))
			end
			if ft.frames > 40 then ft.phase = 3 end
		elseif ft.phase == 3 then
			ft.trials = ft.trials + 1
			-- a ball the bat never sent back up the table is one it passed through; one
			-- that went up and has already come down again was still hit
			if ft.exit then ft.hits = ft.hits + 1 else ft.through = ft.through + 1 end
			if ft.exit then
				ft.exits[#ft.exits + 1] = ft.exit
				ft.angs[#ft.angs + 1] = ft.exit_ang
			end
			if ft.trials % 10 == 0 then
				local n = #ft.exits
				local lo, hi, sum = math.huge, -math.huge, 0
				local alo, ahi = math.huge, -math.huge
				for i = 1, n do
					lo = math.min(lo, ft.exits[i]); hi = math.max(hi, ft.exits[i]); sum = sum + ft.exits[i]
					alo = math.min(alo, ft.angs[i]); ahi = math.max(ahi, ft.angs[i])
				end
				print(string.format('[fliptest] speed %.0f in/s: trials %d hit %d through %d | exit n=%d mean %.0f range %.0f-%.0f spread %.0f | angle %.0f to %.0f deg',
					fliptest, ft.trials, ft.hits, ft.through, n, n > 0 and sum / n or 0, lo, hi, hi - lo, alo, ahi))
			end
			ft.exit, ft.exit_ang = nil, nil
			ft.phase = 0; ft.frames = 0
		end
	end

	-- Before it can play a game the autopilot has to start one. Coin and start are
	-- ordinary matrix switches, found by their input type rather than by tag, since
	-- every driver puts them somewhere different: 17 to 19 are the coin chutes and 7
	-- or 42 the start button. It keeps coining and starting while no ball has come.
	local function find_credit_switches(s)
		if s.credit_looked then return end
		s.credit_looked = true
		for tag, port in pairs(manager.machine.ioport.ports) do
			for _, fld in pairs(port.fields) do
				local ty = fld.type
				if not s.coin_sw and (ty == 17 or ty == 18 or ty == 19) then s.coin_sw = { tag, fld.mask } end
				if not s.start_sw and (ty == 7 or ty == 42) then s.start_sw = { tag, fld.mask } end
			end
		end
	end

	local function update_credit(s)
		find_credit_switches(s)
		if s.state ~= 'trough' or s.ball then s.credit_frames = 0; return end
		s.credit_frames = (s.credit_frames or 0) + 1
		-- give the machine a few seconds to finish its power up test first
		if s.credit_frames < 5 * 60 then return end
		local step = math.floor((s.credit_frames - 5 * 60) / 45) % 4
		if (s.credit_frames - 5 * 60) % 45 ~= 0 then return end
		if step < 3 then
			if s.coin_sw then pulse_switch(s, s.coin_sw, 'Coin') end
		elseif s.start_sw then
			pulse_switch(s, s.start_sw, 'Start')
		end
	end

	local function update_autopilot(s)
		update_credit(s)
		local b = s.ball
		if not b or s.state ~= 'play' then return end
		local cx = s.tbl.width / 2
		if b.vy > 0 and b.y > s.tbl.length - 9 then
			local i = (b.x < cx) and 1 or 2
			if s.auto_flip[i] == 0 and math.random() < 0.15 then s.auto_flip[i] = 8 end
		end
	end

	local function process_frame()
		local s = sim
		if not s then return end
		local tbl = s.tbl

		update_outputs(s)
		update_pulses(s)
		for k, v in pairs(s.flash) do s.flash[k] = v - 1; if s.flash[k] <= 0 then s.flash[k] = nil end end
		-- a flip request is a countdown of frames, raised by the autopilot or by the
		-- flipper test; it has to come down for both, or the bat sticks up for good
		for i = 1, #s.flippers do s.auto_flip[i] = math.max(0, s.auto_flip[i] - 1) end
		if fliptest > 0 then update_fliptest(s) else update_autopilot(s) end
		update_flippers(s)
		if fliptest > 0 then s.flippers_enabled = true end

		-- drop target resets
		for i, tg in ipairs(tbl.targets or {}) do
			if tg.drop and s.dropped[i] and rose(s, tg.reset) then
				s.dropped[i] = false
			end
		end

		-- serving and the outhole
		for _, name in ipairs(tbl.serve_solenoids or {}) do
			if rose(s, name) and s.state == 'trough' then
				if tbl.trough then
					if s.trough_balls > 0 then
						s.trough_balls = s.trough_balls - 1
						trough_update(s)
						serve(s, name)
					end
				else
					serve(s, name)
				end
			end
		end
		s.saucer_cooldown = math.max(0, s.saucer_cooldown - 1)
		if s.state == 'saucer' then
			local sc = tbl.saucers[s.saucer]
			s.saucer_frames = s.saucer_frames + 1
			-- if the game never fires the kicker (feature not lit) the ball rolls on
			local timeout = s.saucer_frames > (sc.timeout or 6) * 60
			if rose(s, sc.solenoid) or timeout then
				hold_switch(s, sc.switch, false)
				local kick = timeout and (sc.timeout_kick or { 0, 15 }) or sc.kick
				s.ball.vx, s.ball.vy = kick[1], kick[2]
				s.state = 'play'
				s.saucer_cooldown = 30
				log(s, (timeout and 'rolls out of ' or 'kicked out of ') .. sc.name)
			end
		else
			s.saucer_frames = 0
		end
		-- the flipper relay is pulsed during the power-up self test; only a relay that
		-- stays on means a game is in progress
		if s.flippers_enabled then s.relay_frames = s.relay_frames + 1 else s.relay_frames = 0 end
		if tbl.serve_on_flipper_enable and s.state == 'trough' and s.relay_frames == 30 then
			serve(s, 'flippers enabled')
		end
		if s.state == 'outhole' and rose(s, tbl.outhole_solenoid) then
			hold_switch(s, tbl.outhole_switch, false)
			if tbl.trough then
				-- the outhole kicker returns the ball to the trough; the shooter lane
				-- feeder takes it from there when the game is ready
				s.trough_balls = math.min(#tbl.trough, s.trough_balls + 1)
				trough_update(s)
				s.state = 'trough'
				log(s, 'outhole kick, ball back in the trough')
			else
				-- on this hardware the outhole kick delivers the ball to the shooter lane
				serve(s, 'outhole kick')
			end
		end
		-- a machine with a modelled trough serves only when its feeder fires; without
		-- one, fall back to serving a ball once the flippers have been live a while
		if not tbl.trough and s.state == 'trough' and s.relay_frames > 30 then
			s.idle_frames = s.idle_frames + 1
			if s.idle_frames > 4 * 60 then
				serve(s, 'game in progress, no ball')
			end
		else
			s.idle_frames = 0
		end

		-- A ball at rest in open play is caught in a corner between two surfaces, each
		-- holding it against the other. Real machines have a player to nudge them; here
		-- a small push downhill frees it and the drain or the flippers take over.
		if s.state == 'play' and s.ball then
			local b = s.ball
			if math.sqrt(b.vx * b.vx + b.vy * b.vy) < 3.0 then
				s.stuck_frames = (s.stuck_frames or 0) + 1
				if s.stuck_frames > 90 then
					log(s, string.format('ball stuck at (%.1f, %.1f), nudged', b.x, b.y))
					-- downhill, and towards the middle of the table rather than further
					-- into the corner it is wedged in: a ball balanced on the end of an
					-- apron rail came straight back otherwise
					b.vy = b.vy + 18
					b.vx = b.vx + (b.x < tbl.width / 2 and 9 or -9)
					s.stuck_frames = 0
				end
			else
				s.stuck_frames = 0
			end
		else
			s.stuck_frames = 0
		end

		if s.state == 'shooter' then
			s.shooter_frames = s.shooter_frames + 1
			local input = manager.machine.input
			if input:code_pressed(s.plunge_code) or (autopilot and s.flippers_enabled) or (s.flippers_enabled and s.shooter_frames > 3 * 60) then
				plunge(s)
			end
		end

		local dt = FRAME_DT / SUBSTEPS
		for i = 1, SUBSTEPS do step(s, dt) end

		if console_log then
			s.log_frames = (s.log_frames or 0) + 1
			if s.log_frames % 180 == 0 and s.ball then
				print(string.format('[pinviz] ball %s at (%.1f, %.1f) v=(%.1f, %.1f)', s.state, s.ball.x, s.ball.y, s.ball.vx, s.ball.vy))
			end
		end
	end

	local function frame_done()
		-- leave MAME's own menus and messages readable
		if sim and not manager.ui.menu_active then sim.draw() end
	end

	-- --------------------------------------------------------------------
	-- drawing
	-- --------------------------------------------------------------------

	local function make_drawer(s)
		local tbl = s.tbl
		local ui = manager.machine.render.ui_container
		local target = manager.machine.render.ui_target

		local C_PANEL, C_FELT, C_EDGE = 0xFF0A0D16, 0xFF12203A, 0xFF3A5A8A
		local C_WALL, C_POST, C_SLING, C_BUMPER = 0xFFD0D8E8, 0xFF9AA4B8, 0xFFFF9A3C, 0xFFFF4A4A
		local C_BUMPER_IN, C_TARGET, C_DROP, C_DOWN = 0xFF7A1A1A, 0xFF5CFF7A, 0xFF4AC8FF, 0x603A6A88
		local C_SENSOR, C_GATE, C_FLIP, C_FLIP_OFF = 0xFF8C8CFF, 0xFF6A6AAA, 0xFFFFD93C, 0xFF6E5C24
		local C_BALL, C_BALL_RIM, C_FLASH = 0xFFF4F4F4, 0xFF9AD0FF, 0xFFFFFFFF
		local C_TEXT, C_TEXT_DIM, C_TITLE = 0xFFE6ECF5, 0xFF8F9BB0, 0xFFFFD93C

		local function flashing(name) return name and s.flash[name] end

		return function()
			local W, H = target.width, target.height
			if W <= 0 or H <= 0 then W, H = 640, 480 end
			local overlay = s.lay ~= nil
			local to
			-- radii in window coordinates that come out circular on screen: a normalised
			-- unit spans W pixels across and H pixels down, so the x radius has to be the
			-- y radius times H/W
			local circular
			if overlay then
				-- The layout's item bounds are normalised to the view, but draw_box and
				-- draw_line take coordinates normalised to the window. MAME letterboxes the
				-- view inside the window when their aspects differ, so without this
				-- correction the overlay is drawn offset by up to half a letterbox bar and
				-- only a window with the view's own aspect lines up.
				local va = view_aspect()
				local ta = W / H
				local sx, sy, ox, oy = 1, 1, 0, 0
				if va > 0 and ta > va then sx = va / ta; ox = (1 - sx) / 2
				elseif va > 0 and ta < va then sy = ta / va; oy = (1 - sy) / 2 end
				local raw = s.lay.to_ui
				to = function(x, y)
					local ux, uy = raw(x, y)
					return ox + ux * sx, oy + uy * sy
				end
				local per_inch_y = s.lay.yh * sy / s.lay.ll
				circular = function(inches)
					local ry = inches * per_inch_y
					return ry * H / W, ry
				end
			else
				local panel_w = 0.5 * W
				local margin = 0.02 * H
				local ppi = math.min((panel_w * 0.72) / tbl.width, (H - 2 * margin) / tbl.length)
				local ox, oy = margin, margin
				to = function(x, y) return (ox + x * ppi) / W, (oy + y * ppi) / H end
				circular = function(inches) return inches * ppi / W, inches * ppi / H end
			end

			local ax, ay = to(0, 0)
			local bx, by = to(tbl.width, tbl.length)
			if not overlay then
				-- opaque panel over the layout, then the playfield
				ui:draw_box(0, 0, 0.5, 1, C_PANEL, C_PANEL)
				ui:draw_box(ax, ay, bx, by, C_EDGE, C_FELT)
			end

			-- Over a layout the picture belongs inside the playfield panel. A table can
			-- reach past it, because these Bally panels draw the playfield but not the
			-- shooter lane beside it, and anything drawn past the edge lands on the next
			-- panel along. Everything is clipped to the panel, so a ball up the shooter
			-- lane is simply out of shot rather than sitting on the backbox.
			local clip_l, clip_t, clip_r, clip_b = 0, 0, 1, 1
			if overlay then
				clip_l, clip_t = to(0, 0)
				clip_r, clip_b = to(s.lay.lw, s.lay.ll)
			end
			local function wline(ax0, ay0, ax1, ay1, c)
				-- Liang-Barsky against the panel
				local dx, dy = ax1 - ax0, ay1 - ay0
				local t0, t1 = 0, 1
				local p = { -dx, dx, -dy, dy }
				local q = { ax0 - clip_l, clip_r - ax0, ay0 - clip_t, clip_b - ay0 }
				for i = 1, 4 do
					if p[i] == 0 then
						if q[i] < 0 then return end
					else
						local r = q[i] / p[i]
						if p[i] < 0 then
							if r > t1 then return elseif r > t0 then t0 = r end
						else
							if r < t0 then return elseif r < t1 then t1 = r end
						end
					end
				end
				ui:draw_line(ax0 + t0 * dx, ay0 + t0 * dy, ax0 + t1 * dx, ay0 + t1 * dy, c)
			end
			local function wbox(bx0, by0, bx1, by1, c)
				bx0, by0 = math.max(bx0, clip_l), math.max(by0, clip_t)
				bx1, by1 = math.min(bx1, clip_r), math.min(by1, clip_b)
				if bx1 > bx0 and by1 > by0 then ui:draw_box(bx0, by0, bx1, by1, c, c) end
			end
			local function line(x0, y0, x1, y1, c)
				local ux0, uy0 = to(x0, y0)
				local ux1, uy1 = to(x1, y1)
				wline(ux0, uy0, ux1, uy1, c)
			end
			local function thick(x0, y0, x1, y1, c, w)
				local dx, dy = x1 - x0, y1 - y0
				local len = math.sqrt(dx * dx + dy * dy)
				if len < 1e-6 then return end
				local nx, ny = -dy / len * w, dx / len * w
				line(x0, y0, x1, y1, c)
				line(x0 + nx, y0 + ny, x1 + nx, y1 + ny, c)
				line(x0 - nx, y0 - ny, x1 - nx, y1 - ny, c)
			end
			local function circle(x, y, r, c, n)
				n = n or 14
				local cx, cy = to(x, y)
				local rx, ry = circular(r)
				local px, py
				for i = 0, n do
					local a = (i / n) * 2 * math.pi
					local qx, qy = cx + rx * math.cos(a), cy + ry * math.sin(a)
					if px then wline(px, py, qx, qy, c) end
					px, py = qx, qy
				end
			end
			local function disc(x, y, r, c, rows)
				rows = rows or 10
				local cx, cy = to(x, y)
				local rx, ry = circular(r)
				for k = -rows, rows - 1 do
					local wy0 = cy + (k / rows) * ry
					local wy1 = cy + ((k + 1) / rows) * ry
					local hw = rx * math.sqrt(math.max(0, 1 - ((k + 0.5) / rows) ^ 2))
					wbox(cx - hw, wy0, cx + hw, wy1, c)
				end
			end

			-- over a layout the walls are the only thing showing the playfield shape, so
			-- draw them solid; a muted blue keeps them behind the features
			local c_wall = overlay and 0xFF6E86A8 or C_WALL
			for _, w in ipairs(tbl.walls) do line(w[1], w[2], w[3], w[4], c_wall) end
			for _, p in ipairs(tbl.posts or {}) do disc(p[1], p[2], p[3], C_POST, 4) end
			for _, g in ipairs(tbl.gates or {}) do line(g[1], g[2], g[3], g[4], C_GATE) end

			for _, sl in ipairs(tbl.slings or {}) do
				thick(sl[1], sl[2], sl[3], sl[4], flashing(sl.name) and C_FLASH or C_SLING, 0.12)
			end
			for _, bp in ipairs(tbl.bumpers or {}) do
				local hot = flashing(bp.name)
				disc(bp[1], bp[2], bp[3], hot and C_BUMPER or C_BUMPER_IN, 10)
				circle(bp[1], bp[2], bp[3], hot and C_FLASH or C_BUMPER, 16)
				disc(bp[1], bp[2], bp[3] * 0.35, hot and C_FLASH or C_BUMPER, 4)
			end
			for i, tg in ipairs(tbl.targets or {}) do
				local c
				if tg.drop and s.dropped[i] then c = C_DOWN
				elseif flashing(tg.name) then c = C_FLASH
				else c = tg.drop and C_DROP or C_TARGET end
				thick(tg[1], tg[2], tg[3], tg[4], c, 0.08)
			end
			for _, sn in ipairs(tbl.sensors or {}) do
				circle(sn[1], sn[2], sn[3] * 0.6, flashing(sn.name) and C_FLASH or C_SENSOR, 10)
			end
			for _, sc in ipairs(tbl.saucers or {}) do
				circle(sc[1], sc[2], sc[3], flashing(sc.name) and C_FLASH or C_SLING, 12)
			end

			for _, f in ipairs(s.flippers) do
				local x0, y0, x1, y1 = flipper_ends(f)
				local c = s.flippers_enabled and C_FLIP or C_FLIP_OFF
				thick(x0, y0, x1, y1, c, FLIPPER_THICKNESS)
				disc(x0, y0, 0.32, c, 4)
			end

			if s.ball then
				local b = s.ball
				disc(b.x, b.y, b.r, C_BALL, 8)
				circle(b.x, b.y, b.r, C_BALL_RIM, 12)
			end

			local lh = 0.03
			if overlay then
				-- One compact line at the very top of the picture. The playfield itself is
				-- left clear: a log panel over the bottom would cover the flippers and the
				-- outhole, which is the part worth watching. PINVIZ_LOG=1 prints the full
				-- log to the console.
				local e = s.events[1] or ''
				if #e > 30 then e = e:sub(1, 30) end
				local text = string.format('%s  %s  %s', s.state, s.flippers_enabled and 'flippers on' or 'flippers off', e)
				ui:draw_box(ax, ay, bx, ay + lh, 0xB0101820, 0xB0101820)
				ui:draw_text(ax + 0.004, ay + 0.004, text, C_TEXT)
			else
				-- text column to the right of the table
				local tx = bx + 0.012
				ui:draw_text(tx, ay, 'pinviz ' .. tbl.name, C_TITLE)
				ui:draw_text(tx, ay + lh, 'ball ' .. s.state, C_TEXT)
				ui:draw_text(tx, ay + 2 * lh, 'flippers ' .. (s.flippers_enabled and 'on' or 'off'), C_TEXT)
				local y = ay + 4 * lh
				for i = 1, math.min(#s.events, 14) do
					local e = s.events[i]
					if #e > 30 then e = e:sub(1, 30) end
					ui:draw_text(tx, y, e, i == 1 and C_TEXT or C_TEXT_DIM)
					y = y + lh
				end
				ui:draw_text(tx, by - lh, 'Shift flip  Space plunge', C_TEXT_DIM)
			end
		end
	end

	-- --------------------------------------------------------------------
	-- lifecycle
	-- --------------------------------------------------------------------

	local function start()
		sim = nil
		local sysname = manager.machine.system.name
		local tbl = load_table(sysname)
		if not tbl then
			emu.print_info('pinviz: no table for ' .. sysname)
			return
		end
		math.randomseed(1)
		local s = {
			tbl = tbl, ball = nil, state = 'trough', events = {}, pulses = {}, outputs = {},
			dropped = {}, in_sensor = {}, flippers = {}, auto_flip = {}, idle_frames = 0,
			shooter_frames = 0, flippers_enabled = false, saucer_cooldown = 0, saucer_frames = 0, relay_frames = 0,
			flash = {}, trough_balls = 0,
		}
		s.lay = layout_reader(tbl)
		local placed, kept, dropped = place_features(s)
		-- PINVIZ_DUMP=1 prints the table's geometry once, so the walls and the resolved
		-- features can be checked outside the plugin, for rubbers laid across a rail and
		-- other corners a ball can wedge into
		if os.getenv('PINVIZ_DUMP') == '1' then
			for _, w in ipairs(tbl.walls or {}) do
				print(string.format('[dump] wall %.3f %.3f %.3f %.3f', w[1], w[2], w[3], w[4]))
			end
			for _, kind in ipairs({ 'slings', 'targets' }) do
				for _, f in ipairs(tbl[kind] or {}) do
					if f[3] and f[4] then
						print(string.format('[dump] %s %.3f %.3f %.3f %.3f %s', kind, f[1], f[2], f[3], f[4], f.name or '?'))
					end
				end
			end
			for _, kind in ipairs({ 'bumpers', 'sensors', 'saucers' }) do
				for _, f in ipairs(tbl[kind] or {}) do
					print(string.format('[dump] %s %.3f %.3f %.3f %s', kind, f[1], f[2], f[3] or 0, f.name or '?'))
				end
			end
			for _, f in ipairs(s.flippers) do
				print(string.format('[dump] flipper %.3f %.3f %.3f %.3f', f.pivot[1], f.pivot[2], f.length, f.rest))
			end
		end
		emu.print_info(string.format('pinviz: %d features placed from the layout, %d from the table, %d without a position', placed, kept, dropped))
		local input = manager.machine.input
		for i, f in ipairs(tbl.flippers) do
			s.flippers[i] = { pivot = f.pivot, length = f.length, rest = f.rest, up = f.up,
				angle = f.rest, prev_angle = f.rest, target = f.rest, rate = 1400, omega = 0, code = input:code_from_token(f.key) }
			s.auto_flip[i] = 0
		end
		s.plunge_code = input:code_from_token('KEYCODE_SPACE')
		for _, name in ipairs(tbl.serve_solenoids or {}) do watch_output(s, name) end
		watch_output(s, tbl.outhole_solenoid)
		if tbl.flipper_enable then watch_output(s, tbl.flipper_enable) end
		for _, tg in ipairs(tbl.targets or {}) do if tg.reset then watch_output(s, tg.reset) end end
		for _, sc in ipairs(tbl.saucers or {}) do watch_output(s, sc.solenoid) end
		if tbl.solenoid_names then
			local prefix = tbl.output_prefix or 'solenoid'
			for idx, _ in pairs(tbl.solenoid_names) do watch_output(s, prefix .. idx) end
		end
		if tbl.trough then
			s.trough_balls = #tbl.trough
			trough_update(s)
		end
		s.draw = make_drawer(s)
		sim = s
		emu.print_info('pinviz: table ' .. tbl.name .. ' loaded' .. (autopilot and ' (autopilot)' or ''))
	end

	local function stop()
		if sim then
			for _, p in pairs(sim.pulses) do p.field:clear_value() end
			hold_switch(sim, sim.tbl.outhole_switch, false)
			for _, sc in ipairs(sim.tbl.saucers or {}) do hold_switch(sim, sc.switch, false) end
			for _, sw in ipairs(sim.tbl.trough or {}) do hold_switch(sim, sw, false) end
		end
		sim = nil
	end

	start_subscription = emu.add_machine_reset_notifier(start)
	stop_subscription = emu.add_machine_stop_notifier(stop)
	frame_subscription = emu.add_machine_frame_notifier(process_frame)
	emu.register_frame_done(frame_done, 'pinviz')
end

return exports
