// license:BSD-3-Clause
// copyright-holders: Dan Boris, Mirko Buffoni

/***************************************************************************

    Atari Cloak & Dagger hardware

    Games supported:
        * Cloak & Dagger

****************************************************************************

    Master processor

    IRQ: 4 IRQ's per frame at even intervals, 4th IRQ is at start of VBLANK

    000-3FF    Working RAM
    400-7FF    Playfield RAM
    800-FFF    Communication RAM (shared with slave processor)

    1000-100F  Pokey 1
    1008 (R)   bit 7 = Start 2 players
           bit 6 = Start 1 player

    1800-180F  Pokey 2
    1808(R)    Dipswitches

    2000 (R):  Joysticks
    2200 (R):  Player 2 joysticks (for cocktail version)

    2400 (R)   bit 0: Vertical Blank
           bit 1: Self test switch
           bit 2: Left Coin
           bit 3: Right Coin
           bit 4: Cocktail mode
           bit 5: Aux Coin
           bit 6: Player 2 Igniter button
           bit 7: Player 1 Igniter button

    2600 (W) Custom Write (this has something to do with positioning of the display out, I ignore it)

    2800-29FF: (R/W) non-volatile RAM
    3000-30FF: (R/W) Motion RAM
    3200-327F: (W) Color RAM, Address bit 6 becomes the 9th bit of color RAM

    3800: (W) Right Coin Counter
    3801: (W) Left Coin Counter
    3803: (W) Cocktail Output
    3806: (W) Start 2 LED
    3807: (W) Start 1 LED

    3A00: (W) Watchdog reset
    3C00: (W) Reset IRQ
    3E00: (W) bit 0: Enable NVRAM

    4000 - FFFF ROM
        4000-5FFF  136023.501
        6000-7FFF  136023.502
        8000-BFFF  136023.503
        C000-FFFF  136023.504


    Slave processor

    IRQ: 1 IRQ per frame at start of VBLANK

    0000-0007: Working RAM
    0008-000A, 000C-000E: (R/W) bit 0,1,2: Store to/Read From Bit Map RAM

    0008: Decrement X/Increment Y
    0009: Decrement Y
    000A: Decrement X
    000B: Set bitmap X coordinate
    000C: Increment X/Increment Y  <-- Yes this is correct
    000D: Increment Y
    000E: Increment X
    000F: Set bitmap Y coordinate

    0010-07FF: Working RAM
    0800-0FFF: Communication RAM (shared with master processor)

    1000 (W): Reset IRQ
    1200 (W):  bit 0: Swap bit maps
           bit 1: Clear bit map
    1400 (W): Custom Write (this has something to do with positioning of the display out, I ignore it)

    2000-FFFF: Program ROM
        2000-3FFF: 136023.509
        4000-5FFF: 136023.510
        6000-7FFF: 136023.511
        8000-9FFF: 136023.512
        A000-BFFF: 136023.513
        C000-DFFF: 136023.514
        E000-EFFF: 136023.515


    Motion object ROM: 136023.307,136023.308
    Playfield ROM: 136023.306,136023.305


    2008-07
    Dip locations and suggested settings verified with manual.

    Note: The master CPU prints a 'SLAVE COM BAD' message on startup
    before it actually tests the slave CPU. The message only remains
    visible if the slave CPU fails to respond.

****************************************************************************/

/*

    TODO:

    - is bitmap drawing in service mode correct?
    - real CPU speeds (Pokey speeds verified with comparison to recording)
    - custom write

*/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "machine/nvram.h"
#include "machine/rescap.h"
#include "machine/watchdog.h"
#include "sound/pokey.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class cloak_state : public driver_device
{
public:
	cloak_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_slave(*this, "slave"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_bitmap_videoram(*this, "bitmap_videoram%u", 1U, 256U * 256U, ENDIANNESS_LITTLE)
	{ }

	void cloak(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_slave;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	memory_share_array_creator<uint8_t, 2> m_bitmap_videoram;
	std::unique_ptr<uint16_t[]> m_palette_ram;

	uint8_t m_nvram_enabled = 0;
	uint8_t m_bitmap_videoram_selected = 0;
	uint8_t m_bitmap_videoram_address_x = 0;
	uint8_t m_bitmap_videoram_address_y = 0;
	tilemap_t *m_bg_tilemap = nullptr;

	template <uint8_t Which> void coin_counter_w(int state);
	void custom_w(uint8_t data);
	void irq_reset_0_w(uint8_t data);
	void irq_reset_1_w(uint8_t data);
	void nvram_enable_w(uint8_t data);
	void paletteram_w(offs_t offset, uint8_t data);
	void clearbmp_w(uint8_t data);
	uint8_t graph_processor_r(offs_t offset);
	void graph_processor_w(offs_t offset, uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void adjust_xy(int offset);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_pen(int i);
	void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void master_map(address_map &map) ATTR_COLD;
	void slave_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  CLOAK & DAGGER uses RAM to dynamically
  create the palette. The resolution is 9 bit (3 bits per gun). The palette
  contains 64 entries, but it is accessed through a memory windows 128 bytes
  long: writing to the first 64 bytes sets the MSB of the red component to 0,
  while writing to the last 64 bytes sets it to 1.

  Colors 0-15  Character mapped graphics
  Colors 16-31 Bitmapped graphics (2 palettes selected by 128H)
  Colors 32-47 Sprites
  Colors 48-63 not used

  These are the exact resistor values from the schematics:

  bit 8 -- diode |< -- pullup 1 kohm -- 2.2 kohm resistor -- pulldown 100 pf -- RED
        -- diode |< -- pullup 1 kohm -- 4.7 kohm resistor -- pulldown 100 pf -- RED
        -- diode |< -- pullup 1 kohm -- 10  kohm resistor -- pulldown 100 pf -- RED
        -- diode |< -- pullup 1 kohm -- 2.2 kohm resistor -- pulldown 100 pf -- GREEN
        -- diode |< -- pullup 1 kohm -- 4.7 kohm resistor -- pulldown 100 pf -- GREEN
        -- diode |< -- pullup 1 kohm -- 10  kohm resistor -- pulldown 100 pf -- GREEN
        -- diode |< -- pullup 1 kohm -- 2.2 kohm resistor -- pulldown 100 pf -- BLUE
        -- diode |< -- pullup 1 kohm -- 4.7 kohm resistor -- pulldown 100 pf -- BLUE
  bit 0 -- diode |< -- pullup 1 kohm -- 10  kohm resistor -- pulldown 100 pf -- BLUE

***************************************************************************/

void cloak_state::paletteram_w(offs_t offset, uint8_t data)
{
	m_palette_ram[offset & 0x3f] = ((offset & 0x40) << 2) | data;
	set_pen(offset & 0x3f);
}


void cloak_state::set_pen(int i)
{
	static const int resistances[3] = { 10000, 4700, 2200 };
	double weights[3];

	// compute the color output resistor weights
	compute_resistor_weights(0, 255, -1.0,
								3, resistances, weights, 0, 1000,
								0, nullptr, nullptr, 0, 0,
								0, nullptr, nullptr, 0, 0);

	int bit0, bit1, bit2;

	// red component
	bit0 = (~m_palette_ram[i] >> 6) & 0x01;
	bit1 = (~m_palette_ram[i] >> 7) & 0x01;
	bit2 = (~m_palette_ram[i] >> 8) & 0x01;
	int const r = combine_weights(weights, bit0, bit1, bit2);

	// green component
	bit0 = (~m_palette_ram[i] >> 3) & 0x01;
	bit1 = (~m_palette_ram[i] >> 4) & 0x01;
	bit2 = (~m_palette_ram[i] >> 5) & 0x01;
	int const g = combine_weights(weights, bit0, bit1, bit2);

	// blue component
	bit0 = (~m_palette_ram[i] >> 0) & 0x01;
	bit1 = (~m_palette_ram[i] >> 1) & 0x01;
	bit2 = (~m_palette_ram[i] >> 2) & 0x01;
	int const b = combine_weights(weights, bit0, bit1, bit2);

	m_palette->set_pen_color(i, rgb_t(r, g, b));
}

void cloak_state::clearbmp_w(uint8_t data)
{
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());

	m_bitmap_videoram_selected = data & 0x01;

	if (data & 0x02)    // clear
		memset(m_bitmap_videoram[m_bitmap_videoram_selected], 0, 256*256);
}

void cloak_state::adjust_xy(int offset)
{
	switch (offset)
	{
		case 0x00:  m_bitmap_videoram_address_x--; m_bitmap_videoram_address_y++; break;
		case 0x01:  m_bitmap_videoram_address_y--; break;
		case 0x02:  m_bitmap_videoram_address_x--; break;
		case 0x04:  m_bitmap_videoram_address_x++; m_bitmap_videoram_address_y++; break;
		case 0x05:  m_bitmap_videoram_address_y++; break;
		case 0x06:  m_bitmap_videoram_address_x++; break;
	}
}

uint8_t cloak_state::graph_processor_r(offs_t offset)
{
	uint8_t const ret = m_bitmap_videoram[!m_bitmap_videoram_selected][(m_bitmap_videoram_address_y << 8) | m_bitmap_videoram_address_x];

	adjust_xy(offset);

	return ret;
}

void cloak_state::graph_processor_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x03: m_bitmap_videoram_address_x = data; break;
		case 0x07: m_bitmap_videoram_address_y = data; break;
		default:
			m_bitmap_videoram[m_bitmap_videoram_selected][(m_bitmap_videoram_address_y << 8) | m_bitmap_videoram_address_x] = data & 0x0f;

			adjust_xy(offset);
			break;
	}
}

void cloak_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(cloak_state::get_bg_tile_info)
{
	int const code = m_videoram[tile_index];

	tileinfo.set(0, code, 0, 0);
}

void cloak_state::video_start()
{
	constexpr uint8_t NUM_PENS = 0x40;

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cloak_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_palette_ram = std::make_unique<uint16_t[]>(NUM_PENS);

	save_item(NAME(m_bitmap_videoram_address_x));
	save_item(NAME(m_bitmap_videoram_address_y));
	save_item(NAME(m_bitmap_videoram_selected));
	save_pointer(NAME(m_palette_ram), NUM_PENS);
	// save_item(NAME(m_nvram_enabled));
}

void cloak_state::draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			pen_t const pen = m_bitmap_videoram[!m_bitmap_videoram_selected][(y << 8) | x] & 0x07;

			if (pen)
				bitmap.pix(y, (x - 6) & 0xff) = 0x10 | ((x & 0x80) >> 4) | pen;
		}
}

void cloak_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = (0x100 / 4) - 1; offs >= 0; offs--)
	{
		int const code = m_spriteram[offs + 64] & 0x7f;
		int flipx = m_spriteram[offs + 64] & 0x80;
		int flipy = 0;
		int sx = m_spriteram[offs + 192];
		int sy = 240 - m_spriteram[offs];

		if (flip_screen())
		{
			sx -= 9;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, 0, flipx, flipy, sx, sy, 0);
	}
}

uint32_t cloak_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_bitmap(bitmap, cliprect);
	draw_sprites(bitmap, cliprect);
	return 0;
}


/*************************************
 *
 *  Output ports
 *
 *************************************/

template <uint8_t Which>
void cloak_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}

void cloak_state::custom_w(uint8_t data)
{
}

void cloak_state::irq_reset_0_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

void cloak_state::irq_reset_1_w(uint8_t data)
{
	m_slave->set_input_line(0, CLEAR_LINE);
}

void cloak_state::nvram_enable_w(uint8_t data)
{
	m_nvram_enabled = data & 0x01; // TODO: set but never used?
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void cloak_state::master_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x0400, 0x07ff).ram().w(FUNC(cloak_state::videoram_w)).share(m_videoram);
	map(0x0800, 0x0fff).ram().share("shared_ram");
	map(0x1000, 0x100f).rw("pokey1", FUNC(pokey_device::read), FUNC(pokey_device::write));       // DSW0 also
	map(0x1800, 0x180f).rw("pokey2", FUNC(pokey_device::read), FUNC(pokey_device::write));       // DSW1 also
	map(0x2000, 0x2000).portr("P1");
	map(0x2200, 0x2200).portr("P2");
	map(0x2400, 0x2400).portr("SYSTEM");
	map(0x2600, 0x2600).w(FUNC(cloak_state::custom_w));
	map(0x2800, 0x29ff).ram().share("nvram");
	map(0x2f00, 0x2fff).noprw();
	map(0x3000, 0x30ff).ram().share(m_spriteram);
	map(0x3200, 0x327f).w(FUNC(cloak_state::paletteram_w));
	map(0x3800, 0x3807).w("outlatch", FUNC(ls259_device::write_d7));
	map(0x3a00, 0x3a00).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x3c00, 0x3c00).w(FUNC(cloak_state::irq_reset_0_w));
	map(0x3e00, 0x3e00).w(FUNC(cloak_state::nvram_enable_w));
	map(0x4000, 0xffff).rom();
}


/*************************************
 *
 *  Slave CPU memory handlers
 *
 *************************************/

void cloak_state::slave_map(address_map &map)
{
	map(0x0000, 0x0007).ram();
	map(0x0008, 0x000f).rw(FUNC(cloak_state::graph_processor_r), FUNC(cloak_state::graph_processor_w));
	map(0x0010, 0x07ff).ram();
	map(0x0800, 0x0fff).ram().share("shared_ram");
	map(0x1000, 0x1000).w(FUNC(cloak_state::irq_reset_1_w));
	map(0x1200, 0x1200).w(FUNC(cloak_state::clearbmp_w));
	map(0x1400, 0x1400).w(FUNC(cloak_state::custom_w));
	map(0x2000, 0xffff).rom();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( cloak )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )     // player 2 controls, not used

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )     // cocktail mode switch, not used
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )     // player 2 button 1, not used
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("START")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )     // not connected
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_UNUSED )    // pulled high
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, "Credits" ) PORT_DIPLOCATION("5A:!7,!8")
	PORT_DIPSETTING(    0x02, "1 Credit/1 Game" )
	PORT_DIPSETTING(    0x01, "1 Credit/2 Games" )
	PORT_DIPSETTING(    0x03, "2 Credits/1 Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("5A:!5,!6")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("5A:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )    // "5A:!3" - it must be OFF according to manual.
	PORT_DIPNAME( 0x40, 0x00, "Demo Freeze Mode" ) PORT_DIPLOCATION("5A:!2")    // when active, press button 1 to freeze
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )    // "5A:!1" - Not Used according to manual.
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	256,
	4,
	{ 0, 1, 2, 3 },
	{ 0x1000*8+0, 0x1000*8+4, 0, 4, 0x1000*8+8, 0x1000*8+12, 8, 12 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout spritelayout =
{
	8,16,
	128,
	4,
	{ 0, 1, 2, 3 },
	{ 0x1000*8+0, 0x1000*8+4, 0, 4, 0x1000*8+8, 0x1000*8+12, 8, 12 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	16*16
};

static GFXDECODE_START( gfx_cloak )
	GFXDECODE_ENTRY( "chars",   0, charlayout,     0,  1 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,  32,  1 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void cloak_state::cloak(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 1'000'000);     // 1 MHz ????
	m_maincpu->set_addrmap(AS_PROGRAM, &cloak_state::master_map);
	m_maincpu->set_periodic_int(FUNC(cloak_state::irq0_line_hold), attotime::from_hz(4*60));

	M6502(config, m_slave, 1'250'000);       // 1.25 MHz ????
	m_slave->set_addrmap(AS_PROGRAM, &cloak_state::slave_map);
	m_slave->set_periodic_int(FUNC(cloak_state::irq0_line_hold), attotime::from_hz(2*60));

	config.set_maximum_quantum(attotime::from_hz(1000));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	ls259_device &outlatch(LS259(config, "outlatch")); // 10B
	outlatch.q_out_cb<0>().set(FUNC(cloak_state::coin_counter_w<1>)); // right
	outlatch.q_out_cb<1>().set(FUNC(cloak_state::coin_counter_w<0>)); // left
	outlatch.q_out_cb<3>().set(FUNC(cloak_state::flip_screen_set));
	outlatch.q_out_cb<5>().set_nop();   // ???
	outlatch.q_out_cb<6>().set_output("led1").invert(); // START LED 2
	outlatch.q_out_cb<7>().set_output("led0").invert(); // START LED 1

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 3*8, 32*8-1);
	m_screen->set_screen_update(FUNC(cloak_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cloak);
	PALETTE(config, m_palette).set_entries(64);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	// more low pass filters ==> DISCRETE processing
	pokey_device &pokey1(POKEY(config, "pokey1", XTAL(10'000'000) / 8));      // Accurate to recording
	pokey1.allpot_r().set_ioport("START");
	pokey1.set_output_opamp_low_pass(RES_K(1), CAP_U(0.047), 5.0);
	pokey1.add_route(ALL_OUTPUTS, "mono", 0.50);

	pokey_device &pokey2(POKEY(config, "pokey2", XTAL(10'000'000) / 8));      // Accurate to recording
	pokey2.allpot_r().set_ioport("DSW");
	pokey2.set_output_opamp_low_pass(RES_K(1), CAP_U(0.022), 5.0);
	pokey2.add_route(ALL_OUTPUTS, "mono", 0.50);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( cloak )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136023-501.bin",      0x4000, 0x2000, CRC(c2dbef1b) SHA1(3bab091afb846ea5f09200e3b44dc8dd430993fe) )
	ROM_LOAD( "136023-502.bin",      0x6000, 0x2000, CRC(316d0c7b) SHA1(58e50661c077415d9465d85c015b8238b4552304) )
	ROM_LOAD( "136023-503.bin",      0x8000, 0x4000, CRC(b9c291a6) SHA1(b3e310110c6d76fa11c44561eb8281aec5f2d1ae) )
	ROM_LOAD( "136023-504.bin",      0xc000, 0x4000, CRC(d014a1c0) SHA1(76c375ccbd0956779942a72ad2e3c3b8c6203ab3) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "136023-509.bin",      0x2000, 0x2000, CRC(46c021a4) SHA1(8ed7efca766611d433a4fec16ae9c05131a157f4) )
	ROM_LOAD( "136023-510.bin",      0x4000, 0x2000, CRC(8c9cf017) SHA1(19e404354418f95ed7079420fe51110f30e349ed) )
	ROM_LOAD( "136023-511.bin",      0x6000, 0x2000, CRC(66fd8a34) SHA1(9597a02a20113baea656ad5d581311abc2865fb1) )
	ROM_LOAD( "136023-512.bin",      0x8000, 0x2000, CRC(48c8079e) SHA1(b4b74115e58d67de2f50c1a6a39f34f116f0df29) )
	ROM_LOAD( "136023-513.bin",      0xa000, 0x2000, CRC(13f1cbab) SHA1(c016db6c0ca6d72ca8425d807d95f43dc87e6788) )
	ROM_LOAD( "136023-514.bin",      0xc000, 0x2000, CRC(6f8c7991) SHA1(bd6f838b224abed78fbdb7da17baa892289fc2f2) )
	ROM_LOAD( "136023-515.bin",      0xe000, 0x2000, CRC(835438a0) SHA1(27f320b74e7bdb29d4bc505432c96284f482cacc) )

	ROM_REGION( 0x2000, "chars", 0 )
	ROM_LOAD( "136023-105.bin",      0x0000, 0x1000, CRC(ee443909) SHA1(802c5839be9e9e33c75ca7318043ecdb7b82f721) )
	ROM_LOAD( "136023-106.bin",      0x1000, 0x1000, CRC(d708b132) SHA1(d57acdcfb7b3de65f0162bdc041efff4c7eeff18) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "136023-107.bin",      0x0000, 0x1000, CRC(c42c84a4) SHA1(6f241e772f8b46c8d3acad2e967f1ab530886b11) )
	ROM_LOAD( "136023-108.bin",      0x1000, 0x1000, CRC(4fe13d58) SHA1(b21a32b2ff5363ab35fd1438344a04deb4077dbc) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "136023-116.3n",  0x000, 0x100, CRC(ef2668e5) SHA1(442df346a40b1e20d599ed9877c699e00ab518ba) ) // 82S129 Vertical timing PROM
ROM_END


ROM_START( cloaksp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136023-501.bin",      0x4000, 0x2000, CRC(c2dbef1b) SHA1(3bab091afb846ea5f09200e3b44dc8dd430993fe) )
	ROM_LOAD( "136023-502.bin",      0x6000, 0x2000, CRC(316d0c7b) SHA1(58e50661c077415d9465d85c015b8238b4552304) )
	ROM_LOAD( "136023-503.bin",      0x8000, 0x4000, CRC(b9c291a6) SHA1(b3e310110c6d76fa11c44561eb8281aec5f2d1ae) )
	ROM_LOAD( "136023-804.bin",      0xc000, 0x4000, CRC(994899c7) SHA1(3835777ce3f3aaff2ce3520c71d16d4c86743a20) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "136023-509.bin",      0x2000, 0x2000, CRC(46c021a4) SHA1(8ed7efca766611d433a4fec16ae9c05131a157f4) )
	ROM_LOAD( "136023-510.bin",      0x4000, 0x2000, CRC(8c9cf017) SHA1(19e404354418f95ed7079420fe51110f30e349ed) )
	ROM_LOAD( "136023-511.bin",      0x6000, 0x2000, CRC(66fd8a34) SHA1(9597a02a20113baea656ad5d581311abc2865fb1) )
	ROM_LOAD( "136023-512.bin",      0x8000, 0x2000, CRC(48c8079e) SHA1(b4b74115e58d67de2f50c1a6a39f34f116f0df29) )
	ROM_LOAD( "136023-513.bin",      0xa000, 0x2000, CRC(13f1cbab) SHA1(c016db6c0ca6d72ca8425d807d95f43dc87e6788) )
	ROM_LOAD( "136023-514.bin",      0xc000, 0x2000, CRC(6f8c7991) SHA1(bd6f838b224abed78fbdb7da17baa892289fc2f2) )
	ROM_LOAD( "136023-515.bin",      0xe000, 0x2000, CRC(835438a0) SHA1(27f320b74e7bdb29d4bc505432c96284f482cacc) )

	ROM_REGION( 0x2000, "chars", 0 )
	ROM_LOAD( "136023-105.bin",      0x0000, 0x1000, CRC(ee443909) SHA1(802c5839be9e9e33c75ca7318043ecdb7b82f721) )
	ROM_LOAD( "136023-106.bin",      0x1000, 0x1000, CRC(d708b132) SHA1(d57acdcfb7b3de65f0162bdc041efff4c7eeff18) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "136023-107.bin",      0x0000, 0x1000, CRC(c42c84a4) SHA1(6f241e772f8b46c8d3acad2e967f1ab530886b11) )
	ROM_LOAD( "136023-108.bin",      0x1000, 0x1000, CRC(4fe13d58) SHA1(b21a32b2ff5363ab35fd1438344a04deb4077dbc) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "136023-116.3n",  0x000, 0x100, CRC(ef2668e5) SHA1(442df346a40b1e20d599ed9877c699e00ab518ba) ) // 82S129 Vertical timing PROM
ROM_END


ROM_START( cloakfr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136023-501.bin",      0x4000, 0x2000, CRC(c2dbef1b) SHA1(3bab091afb846ea5f09200e3b44dc8dd430993fe) )
	ROM_LOAD( "136023-502.bin",      0x6000, 0x2000, CRC(316d0c7b) SHA1(58e50661c077415d9465d85c015b8238b4552304) )
	ROM_LOAD( "136023-503.bin",      0x8000, 0x4000, CRC(b9c291a6) SHA1(b3e310110c6d76fa11c44561eb8281aec5f2d1ae) )
	ROM_LOAD( "136023-704.bin",      0xc000, 0x4000, CRC(bf225ea0) SHA1(d1e45d6071eb819e6a5075b61014e24a451fb443) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "136023-509.bin",      0x2000, 0x2000, CRC(46c021a4) SHA1(8ed7efca766611d433a4fec16ae9c05131a157f4) )
	ROM_LOAD( "136023-510.bin",      0x4000, 0x2000, CRC(8c9cf017) SHA1(19e404354418f95ed7079420fe51110f30e349ed) )
	ROM_LOAD( "136023-511.bin",      0x6000, 0x2000, CRC(66fd8a34) SHA1(9597a02a20113baea656ad5d581311abc2865fb1) )
	ROM_LOAD( "136023-512.bin",      0x8000, 0x2000, CRC(48c8079e) SHA1(b4b74115e58d67de2f50c1a6a39f34f116f0df29) )
	ROM_LOAD( "136023-513.bin",      0xa000, 0x2000, CRC(13f1cbab) SHA1(c016db6c0ca6d72ca8425d807d95f43dc87e6788) )
	ROM_LOAD( "136023-514.bin",      0xc000, 0x2000, CRC(6f8c7991) SHA1(bd6f838b224abed78fbdb7da17baa892289fc2f2) )
	ROM_LOAD( "136023-515.bin",      0xe000, 0x2000, CRC(835438a0) SHA1(27f320b74e7bdb29d4bc505432c96284f482cacc) )

	ROM_REGION( 0x2000, "chars", 0 )
	ROM_LOAD( "136023-105.bin",      0x0000, 0x1000, CRC(ee443909) SHA1(802c5839be9e9e33c75ca7318043ecdb7b82f721) )
	ROM_LOAD( "136023-106.bin",      0x1000, 0x1000, CRC(d708b132) SHA1(d57acdcfb7b3de65f0162bdc041efff4c7eeff18) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "136023-107.bin",      0x0000, 0x1000, CRC(c42c84a4) SHA1(6f241e772f8b46c8d3acad2e967f1ab530886b11) )
	ROM_LOAD( "136023-108.bin",      0x1000, 0x1000, CRC(4fe13d58) SHA1(b21a32b2ff5363ab35fd1438344a04deb4077dbc) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "136023-116.3n",  0x000, 0x100, CRC(ef2668e5) SHA1(442df346a40b1e20d599ed9877c699e00ab518ba) ) // 82S129 Vertical timing PROM
ROM_END


ROM_START( cloakgr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136023-501.bin",      0x4000, 0x2000, CRC(c2dbef1b) SHA1(3bab091afb846ea5f09200e3b44dc8dd430993fe) )
	ROM_LOAD( "136023-502.bin",      0x6000, 0x2000, CRC(316d0c7b) SHA1(58e50661c077415d9465d85c015b8238b4552304) )
	ROM_LOAD( "136023-503.bin",      0x8000, 0x4000, CRC(b9c291a6) SHA1(b3e310110c6d76fa11c44561eb8281aec5f2d1ae) )
	ROM_LOAD( "136023-604.bin",      0xc000, 0x4000, CRC(7ac66aea) SHA1(fbeb3bb2756275aabb5fee1aa44c6f32da159bb6) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "136023-509.bin",      0x2000, 0x2000, CRC(46c021a4) SHA1(8ed7efca766611d433a4fec16ae9c05131a157f4) )
	ROM_LOAD( "136023-510.bin",      0x4000, 0x2000, CRC(8c9cf017) SHA1(19e404354418f95ed7079420fe51110f30e349ed) )
	ROM_LOAD( "136023-511.bin",      0x6000, 0x2000, CRC(66fd8a34) SHA1(9597a02a20113baea656ad5d581311abc2865fb1) )
	ROM_LOAD( "136023-512.bin",      0x8000, 0x2000, CRC(48c8079e) SHA1(b4b74115e58d67de2f50c1a6a39f34f116f0df29) )
	ROM_LOAD( "136023-513.bin",      0xa000, 0x2000, CRC(13f1cbab) SHA1(c016db6c0ca6d72ca8425d807d95f43dc87e6788) )
	ROM_LOAD( "136023-514.bin",      0xc000, 0x2000, CRC(6f8c7991) SHA1(bd6f838b224abed78fbdb7da17baa892289fc2f2) )
	ROM_LOAD( "136023-515.bin",      0xe000, 0x2000, CRC(835438a0) SHA1(27f320b74e7bdb29d4bc505432c96284f482cacc) )

	ROM_REGION( 0x2000, "chars", 0 )
	ROM_LOAD( "136023-105.bin",      0x0000, 0x1000, CRC(ee443909) SHA1(802c5839be9e9e33c75ca7318043ecdb7b82f721) )
	ROM_LOAD( "136023-106.bin",      0x1000, 0x1000, CRC(d708b132) SHA1(d57acdcfb7b3de65f0162bdc041efff4c7eeff18) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "136023-107.bin",      0x0000, 0x1000, CRC(c42c84a4) SHA1(6f241e772f8b46c8d3acad2e967f1ab530886b11) )
	ROM_LOAD( "136023-108.bin",      0x1000, 0x1000, CRC(4fe13d58) SHA1(b21a32b2ff5363ab35fd1438344a04deb4077dbc) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "136023-116.3n",  0x000, 0x100, CRC(ef2668e5) SHA1(442df346a40b1e20d599ed9877c699e00ab518ba) ) // 82S129 Vertical timing PROM
ROM_END


ROM_START( agentx4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136401-023.bin",      0x4000, 0x2000, CRC(f7edac86) SHA1(80e8738460d408343cbae2a99bcdfd92664d13bb) )
	ROM_LOAD( "136402-023.bin",      0x6000, 0x2000, CRC(db5e2382) SHA1(15196a588cb676254ed19fe019e4bab3708d5636) )
	ROM_LOAD( "136403-023.bin",      0x8000, 0x4000, CRC(87de01b4) SHA1(5c79edc132a3b460a636e223387a8bbde54833dc) )
	ROM_LOAD( "136404-023.bin",      0xc000, 0x4000, CRC(b97219dc) SHA1(39d1fcc76e112840f20443e2125478a33a102afc) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "136409-023.bin",      0x2000, 0x2000, CRC(add4a749) SHA1(885a7175179d3e4201af24217d27f100479edc1b) )
	ROM_LOAD( "136410-023.bin",      0x4000, 0x2000, CRC(b1e1c074) SHA1(ac0b39887c4e21db06160e0d097d17e00c47c784) )
	ROM_LOAD( "136411-023.bin",      0x6000, 0x2000, CRC(6d0c1ee5) SHA1(62c7214df7cd4f5426d7fa2b28243acc31b356f2) )
	ROM_LOAD( "136412-023.bin",      0x8000, 0x2000, CRC(815af543) SHA1(0ec8deb28d5f022e13c900d2b845f50fede1ef8e) )
	ROM_LOAD( "136413-023.bin",      0xa000, 0x2000, CRC(2ad9e622) SHA1(ac5f5ce4cb0a04dff28d1575d0525a01b17364d9) )
	ROM_LOAD( "136414-023.bin",      0xc000, 0x2000, CRC(cadf9ab0) SHA1(d5c4c86124033f21e065e5cd9cf7a1b31d3c11a6) )
	ROM_LOAD( "136415-023.bin",      0xe000, 0x2000, CRC(f5024961) SHA1(99a8b02932b497588a011fe051465407531f8a0f) )

	ROM_REGION( 0x2000, "chars", 0 )
	ROM_LOAD( "136105-023.bin",      0x0000, 0x1000, CRC(ee443909) SHA1(802c5839be9e9e33c75ca7318043ecdb7b82f721) )
	ROM_LOAD( "136106-023.bin",      0x1000, 0x1000, CRC(d708b132) SHA1(d57acdcfb7b3de65f0162bdc041efff4c7eeff18) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "136107-023.bin",      0x0000, 0x1000, CRC(c42c84a4) SHA1(6f241e772f8b46c8d3acad2e967f1ab530886b11) )
	ROM_LOAD( "136108-023.bin",      0x1000, 0x1000, CRC(4fe13d58) SHA1(b21a32b2ff5363ab35fd1438344a04deb4077dbc) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "136023-116.3n",  0x000, 0x100, CRC(ef2668e5) SHA1(442df346a40b1e20d599ed9877c699e00ab518ba) ) // 82S129 Vertical timing PROM
ROM_END


ROM_START( agentx3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136301-023.bin",      0x4000, 0x2000, CRC(fba1d9de) SHA1(e8a72531f912a6ff1cd1c67a5c3fd0d18ede54f6) )
	ROM_LOAD( "136302-023.bin",      0x6000, 0x2000, CRC(e5694c72) SHA1(45e143048f2bbfcb1a71eab63ccd61e2a227066c) )
	ROM_LOAD( "136303-023.bin",      0x8000, 0x4000, CRC(70ef51c5) SHA1(4a2a15dc05eaf8481dfdb18e1aea81efb8f7e1d6) )
	ROM_LOAD( "136304-023.bin",      0xc000, 0x4000, CRC(f4a86cda) SHA1(438c60e91948468f4ce3bafd471bc68b1a728c3b) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "136309-023.bin",      0x2000, 0x2000, CRC(1c04282d) SHA1(ec50645220dae7f65aa2c0252088cb53de2e4610) )
	ROM_LOAD( "136310-023.bin",      0x4000, 0x2000, CRC(a61eaa88) SHA1(6f00d5d8338a311b33a8ff748c13d25fa01feba7) )
	ROM_LOAD( "136311-023.bin",      0x6000, 0x2000, CRC(a670f4b4) SHA1(235828c34710c901bffef85f208bc0c5abfd527a) )
	ROM_LOAD( "136312-023.bin",      0x8000, 0x2000, CRC(e955af62) SHA1(a32862ed36db3ca0cdee7aadfb63938672e4fc14) )
	ROM_LOAD( "136313-023.bin",      0xa000, 0x2000, CRC(b4b46d9d) SHA1(2eb67a4705e84c15231c15caa4e3e566b1ebfeb2) )
	ROM_LOAD( "136314-023.bin",      0xc000, 0x2000, CRC(3138a3b2) SHA1(553f247a72fce176d80db7e4c7624cdb73c8c078) )
	ROM_LOAD( "136315-023.bin",      0xe000, 0x2000, CRC(d12f5523) SHA1(53a7e4e360bd8c21c0e9bf408b6dd231f4fe025c) )

	ROM_REGION( 0x2000, "chars", 0 )
	ROM_LOAD( "136105-023.bin",      0x0000, 0x1000, CRC(ee443909) SHA1(802c5839be9e9e33c75ca7318043ecdb7b82f721) )
	ROM_LOAD( "136106-023.bin",      0x1000, 0x1000, CRC(d708b132) SHA1(d57acdcfb7b3de65f0162bdc041efff4c7eeff18) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "136107-023.bin",      0x0000, 0x1000, CRC(c42c84a4) SHA1(6f241e772f8b46c8d3acad2e967f1ab530886b11) )
	ROM_LOAD( "136108-023.bin",      0x1000, 0x1000, CRC(4fe13d58) SHA1(b21a32b2ff5363ab35fd1438344a04deb4077dbc) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "136023-116.3n",  0x000, 0x100, CRC(ef2668e5) SHA1(442df346a40b1e20d599ed9877c699e00ab518ba) ) // 82S129 Vertical timing PROM
ROM_END


ROM_START( agentx2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136201-023.bin",      0x4000, 0x2000, CRC(e6c7041f) SHA1(dd253b2a1417498b16c2986f7e2d885cfc11f8e9) )
	ROM_LOAD( "136202-023.bin",      0x6000, 0x2000, CRC(4c94929e) SHA1(b67640cfdb7ccda80b8569d2c8607d0f0103226e) )
	ROM_LOAD( "136203-023.bin",      0x8000, 0x4000, CRC(c7a59697) SHA1(a1735873290956c50fea34787003fe2ef9e23b6e) )
	ROM_LOAD( "136204-023.bin",      0xc000, 0x4000, CRC(e6e06a9c) SHA1(91976a73742d5cb61f59e28917f5eb923c9ee8f5) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "136209-023.bin",      0x2000, 0x2000, CRC(319772d8) SHA1(ef4cdb5f7d100410bc406b61857083c12cde21f6) )
	ROM_LOAD( "136210-023.bin",      0x4000, 0x2000, CRC(6e95f628) SHA1(987a76447b6bda00223a8c94bdb741dce944c61f) )
	ROM_LOAD( "136211-023.bin",      0x6000, 0x2000, CRC(8d936132) SHA1(461e1924e4fce337889377912311291947c432be) )
	ROM_LOAD( "136212-023.bin",      0x8000, 0x2000, CRC(9a3074c8) SHA1(7edb677eb5815aa0417da239d76e7b9b29977711) )
	ROM_LOAD( "136213-023.bin",      0xa000, 0x2000, CRC(15984981) SHA1(caff62801039139b8e186d619b4f7565b3ddabc8) )
	ROM_LOAD( "136214-023.bin",      0xc000, 0x2000, CRC(dba311ec) SHA1(a825f013211e6eaac4c18a589b38a416dc633954) )
	ROM_LOAD( "136215-023.bin",      0xe000, 0x2000, CRC(dc20c185) SHA1(5a1d3e65e543fe295c2f179f8b5c2193065af581) )

	ROM_REGION( 0x2000, "chars", 0 )
	ROM_LOAD( "136105-023.bin",      0x0000, 0x1000, CRC(ee443909) SHA1(802c5839be9e9e33c75ca7318043ecdb7b82f721) )
	ROM_LOAD( "136106-023.bin",      0x1000, 0x1000, CRC(d708b132) SHA1(d57acdcfb7b3de65f0162bdc041efff4c7eeff18) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "136107-023.bin",      0x0000, 0x1000, CRC(c42c84a4) SHA1(6f241e772f8b46c8d3acad2e967f1ab530886b11) )
	ROM_LOAD( "136108-023.bin",      0x1000, 0x1000, CRC(4fe13d58) SHA1(b21a32b2ff5363ab35fd1438344a04deb4077dbc) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "136023-116.3n",  0x000, 0x100, CRC(ef2668e5) SHA1(442df346a40b1e20d599ed9877c699e00ab518ba) ) // 82S129 Vertical timing PROM
ROM_END


ROM_START( agentx1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136101-023.bin",      0x4000, 0x2000, CRC(a12b9c22) SHA1(40fef20782bdb05ef5ef81327e69286abab6449c) )
	ROM_LOAD( "136102-023.bin",      0x6000, 0x2000, CRC(e65d30df) SHA1(bad65c2bb434c43bd72a0b7f88adea4150e3cd66) )
	ROM_LOAD( "136103-023.bin",      0x8000, 0x4000, CRC(c6f8a128) SHA1(feaf860b2226796643c107ac5821b0a9988d3367) )
	ROM_LOAD( "136104-023.bin",      0xc000, 0x4000, CRC(db002945) SHA1(52c0d0e94e718e51cd744d3ceb10e1443f41a3d5) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "136109-023.bin",      0x2000, 0x2000, CRC(31487f5d) SHA1(86d519fb7f352101780c2ed1a87cbabb2ff0dcd4) )
	ROM_LOAD( "136110-023.bin",      0x4000, 0x2000, CRC(1ee38ecb) SHA1(44d3d7c0cffbdeb57231037cfbf7e4054dfc9349) )
	ROM_LOAD( "136111-023.bin",      0x6000, 0x2000, CRC(ca6a6b0c) SHA1(3d8b62a6a7480032bce74d3a5dbdaa52fc8b33c0) )
	ROM_LOAD( "136112-023.bin",      0x8000, 0x2000, CRC(933051bc) SHA1(4a6e04b268d874333cfa01783f1fc6603ef58a99) )
	ROM_LOAD( "136113-023.bin",      0xa000, 0x2000, CRC(1706a674) SHA1(4aca238151d1b30aa22df5ea9525f16df2067a7d) )
	ROM_LOAD( "136114-023.bin",      0xc000, 0x2000, CRC(7c7c905d) SHA1(eb417c5e80f1a3169c1385d7c843a3defa584b6e) )
	ROM_LOAD( "136115-023.bin",      0xe000, 0x2000, CRC(7f36710c) SHA1(6d412f22e723999c641dde18a47c8d90d2ed07a3) )

	ROM_REGION( 0x2000, "chars", 0 )
	ROM_LOAD( "136105-023.bin",      0x0000, 0x1000, CRC(ee443909) SHA1(802c5839be9e9e33c75ca7318043ecdb7b82f721) )
	ROM_LOAD( "136106-023.bin",      0x1000, 0x1000, CRC(d708b132) SHA1(d57acdcfb7b3de65f0162bdc041efff4c7eeff18) )

	ROM_REGION( 0x2000, "sprites", 0 )
	ROM_LOAD( "136107-023.bin",      0x0000, 0x1000, CRC(c42c84a4) SHA1(6f241e772f8b46c8d3acad2e967f1ab530886b11) )
	ROM_LOAD( "136108-023.bin",      0x1000, 0x1000, CRC(4fe13d58) SHA1(b21a32b2ff5363ab35fd1438344a04deb4077dbc) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "136023-116.3n",  0x000, 0x100, CRC(ef2668e5) SHA1(442df346a40b1e20d599ed9877c699e00ab518ba) ) // 82S129 Vertical timing PROM
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, cloak,   0,     cloak, cloak, cloak_state, empty_init, ROT0, "Atari", "Cloak & Dagger (rev 5)",     MACHINE_SUPPORTS_SAVE )
GAME( 1983, cloaksp, cloak, cloak, cloak, cloak_state, empty_init, ROT0, "Atari", "Cloak & Dagger (Spanish)",   MACHINE_SUPPORTS_SAVE )
GAME( 1983, cloakfr, cloak, cloak, cloak, cloak_state, empty_init, ROT0, "Atari", "Cloak & Dagger (French)",    MACHINE_SUPPORTS_SAVE )
GAME( 1983, cloakgr, cloak, cloak, cloak, cloak_state, empty_init, ROT0, "Atari", "Cloak & Dagger (German)",    MACHINE_SUPPORTS_SAVE )
GAME( 1983, agentx4, cloak, cloak, cloak, cloak_state, empty_init, ROT0, "Atari", "Agent X (prototype, rev 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, agentx3, cloak, cloak, cloak, cloak_state, empty_init, ROT0, "Atari", "Agent X (prototype, rev 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, agentx2, cloak, cloak, cloak, cloak_state, empty_init, ROT0, "Atari", "Agent X (prototype, rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, agentx1, cloak, cloak, cloak, cloak_state, empty_init, ROT0, "Atari", "Agent X (prototype, rev 1)", MACHINE_SUPPORTS_SAVE )
