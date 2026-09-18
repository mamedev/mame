# pinviz

A minimal pinball playfield for testing MAME pinball drivers.

The emulated machine is the real thing: the ROM, switch matrix, solenoids,
lamps, displays and sound. This plugin adds the one part MAME does not have,
a ball. It is enough physics to play a game and watch the driver react, not a
simulator: one ball, walls, flippers, slingshots, pop bumpers, drop and
stand-up targets, saucers, lanes, one-way gates and the outhole, drawn over
the playfield panel.

    mame centaur -plugin pinviz

Keys: left and right Shift for the flippers (or whatever the table names),
Space to plunge. The usual MAME keys coin up and start the game.

Over a layout the plugin keeps the playfield clear and shows one status line at
the top of the picture. The full event log goes to the console instead, because a
log panel on the picture covers the flippers and the outhole, which is the part
worth watching.

Environment variables:

- `PINVIZ_AUTOPILOT=1` plays by itself, which turns the plugin into a driver
  regression test.
- `PINVIZ_LOG=1` prints every switch closure and solenoid fire to the console.
- `PINVIZ_FLIPTEST=<speed>` drops balls onto the left flipper at that speed in
  inches per second, flips so the bat is mid sweep as the ball arrives, and
  reports how many were hit, how many passed through, and the speed and angle
  the shots left at. A ball that went back up the table was hit, even if it has
  come down again by the end of the trial.
- `PINVIZ_DUMP=1` prints the table's walls and its resolved features once at
  startup, so the geometry can be checked outside the plugin: a rubber laid
  across a rail, or anything reaching past the layout's playfield panel.
- `PINVIZ_FLIPSPREAD=0` makes that test drop every ball at one point on the bat
  instead of five. A run is then repeatable and any spread in the exit speeds is
  the physics rather than the contact point.

## Where a table comes from

Two sources, so that neither repeats the other.

**The machine's layout says where things are.** A per-game layout marks its
playfield panel with items whose ids are `pinviz:pfx` and `pinviz:pfy`, and each
playfield switch with an item whose id is `pinviz:<port>:<mask>`. A second item
for the same switch gets a `:2` suffix, a third `:3`, and so on. pinviz reads
those positions when the machine starts. The layout is then the single place a
playfield is drawn, and adding the ids to an existing layout is most of the work
of supporting a machine. `src/mame/layout/by35_centaur.lay` and the two by17
layouts are examples.

**The table in `tables/<system>.lua` says what each switch is**, and supplies
what a picture cannot: walls, rails, flippers, physics constants, and which
solenoid serves the ball, kicks the outhole, resets a drop target bank or
releases a saucer. A feature identifies its switch, its kind, and for a segment
an orientation within the layout item (`h`, `v`, `d`, `u`) and a length in
inches. A feature may also carry its own coordinates, for a machine whose layout
has no ids; Playboy does that.

A layout that draws only the playfield leaves no room for the shooter lane, so a
table can set `lay_width` and `lay_length` to say how much of itself the panel
covers. The rest of the table, the lane included, sits outside the picture.

A layout should not paint the walls and rails itself. pinviz draws those from the
table, and a layout that draws its own will disagree with them as soon as either
changes. Every layout here paints only its panel border and leaves the rest to
the plugin.

`tables/bally_body.lua` holds the parts every machine on Bally's solid state
cabinet shares: the outline, the shooter lane and its gate, the outlane and
inlane rails, and the flippers. A table for one of those machines takes the
skeleton and adds only its own features.

## Machines

| Table | Layout supplies positions | Notes |
|---|---|---|
| `centaur` | yes | Geometry traced from a playfield photograph. |
| `playboy` | yes | |
| `matahari` | yes | Walls come from the shared Bally body, not the real playfield. |
| `pwerplay` | yes | Same. Plays a full five ball game. |
| `hs_l4` | yes | Williams System 11. Switches and solenoids are from Williams' tech chart; the layout arranges the real features rather than tracing the playfield. |

A machine with a ball trough will not start a game until the trough reads full, so
a table can list its trough switches. pinviz then holds one closed per ball at
home, hands one to the shooter lane when the feeder solenoid fires, and takes one
back when the outhole kicker returns a drained ball. High Speed needs this.

A table can also set `output_prefix`, because not every driver calls its outputs
`solenoid<n>`: System 11 publishes `out0` to `out85`.

High Speed shows FACTORY SETTING on a fresh NVRAM and needs one reset before it
will accept a coin, as the `s11.cpp` header says.

For Playboy, Mata Hari, Power Play and High Speed only the switch positions are real. The
walls are a generic cabinet, so the ball does not travel the paths it would on
the machine, and play is unevenly distributed: Mata Hari's ball spends most of
its time on the two upper pop bumpers. Tightening those needs a playfield
reference for each, the way Centaur's did.

## Checking it still works

Two checks, split by whether they need a machine.

`scripts/build/check_pinviz.py` compares each table against the layout its
machine uses, and needs neither a build nor a ROM, so CI runs it. A table names
a switch and the layout marks that switch with an id; nothing at build or run
time ties the two together, so an id that is renamed or dropped shows up only as
a feature quietly missing from the table. It also catches an id that appears
twice, whose position would be ambiguous, and a layout with no playfield extent
markers, and it reports ids no table uses yet.

`plugins/pinviz/selftest.py --rompath roms` exercises the plugin on every machine
that has a table and needs both, so it is run by hand:

- geometry, from `PINVIZ_DUMP`: no rubber or target may cross a wall, since the
  acute corner that leaves holds a ball against both surfaces, and every feature
  must have a position
- flippers, from `PINVIZ_FLIPTEST`: nothing may pass through the bat at any speed
  a playfield can produce, and a shot should leave faster than it arrived
- play, from `PINVIZ_AUTOPILOT`: the machine must serve balls and see switch
  closures, with no Lua errors

Pass `--quick` for shorter runs, or machine names to narrow it down.
