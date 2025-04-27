// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/*
Sega GP World hardware
Driver by Andrew Gardner with help from Daphne Source.

Notes:
    - GP World is a rare game that came in a huge cabinet with two monitors, side by side.
      The image from the laserdisc was stretched to an 8x3 aspect and graphics were overlayed
      on top.
    - The hardware is similar to Astron Belt but somewhat more powerful.


ToDo:
    - Hook up start lamp to mame lamp system
    - Do the gear shifter right
    - What's up with the pedals being tied together?
    - Daphne says this game is dual monitor.  Is it?
    - Finish sprite drawing - looks awfully similar to system1 and suprloco.
    - Palette!
    - Still some undocumented reads and writes - likely dealing with I/O.
    - Convert to tilemaps.

Dumping Notes:
    GP World by Sega - Dumped by Matteo Marioni on Dec, 21 2001

    834-5515-01
    GP WORLD P.
    SEGA No. 123743

    Chip            Label
    82S123          PR6146, PR6147
    27128           EPR-6162A (the "A" letter is handwritten), EPR-6163, EPR-6164, EPR-6157, EPR-6155, EPR-6153
                        EPR-6151, EPR-6149, EPR-6158, EPR-6156, EPR-6154, EPR-6152, EPR-6150
    2732            EPR-6148
    82S129          PR-5501 (located on video overlay pcb [834-5175])

    (not dumped)
    DMPAL16R6JC     315-5072.ic9, 315-5071.ic10
    DMPAL12H6JC     315-5070.ic97
*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ldv1000.h"
#include "emupal.h"
#include "speaker.h"


namespace {

class gpworld_state : public driver_device
{
public:
	gpworld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_laserdisc(*this, "laserdisc"),
		m_sprite_ram(*this, "sprite_ram"),
		m_palette_ram(*this, "palette_ram"),
		m_tile_ram(*this, "tile_ram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{
	}

	void gpworld(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void driver_start() override;

	void mainmem(address_map &map) ATTR_COLD;
	void mainport(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_stop);

	void ldp_write(uint8_t data);
	void misc_io_write(uint8_t data);
	void brake_gas_write(uint8_t data);
	void palette_write(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_callback);
	void draw_tiles(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_pixel(bitmap_rgb32 &bitmap, const rectangle &cliprect, int x, int y, int color, int flip);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	emu_timer *m_irq_stop_timer = nullptr;

	required_device<pioneer_ldv1000_device> m_laserdisc;
	required_shared_ptr<uint8_t> m_sprite_ram;
	required_shared_ptr<uint8_t> m_palette_ram;
	required_shared_ptr<uint8_t> m_tile_ram;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t m_nmi_enable = 0;
	uint8_t m_start_lamp = 0;
	uint8_t m_ldp_read_latch = 0;
	uint8_t m_ldp_write_latch = 0;
	uint8_t m_brake_gas = 0;
	uint8_t ldp_read();
	uint8_t pedal_in();
};


/* Assumed to be the same as segald hardware */
#define GUESSED_CLOCK (5000000)





/* VIDEO GOODS */
void gpworld_state::draw_tiles(bitmap_rgb32 &bitmap,const rectangle &cliprect)
{
	uint8_t characterX, characterY;

	/* Temporarily set to 64 wide to accommodate two screens */
	for (characterX = 0; characterX < 64; characterX++)
	{
		for (characterY = 0; characterY < 32; characterY++)
		{
			int current_screen_character = (characterY*64) + characterX;

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, m_tile_ram[current_screen_character],
					characterY, 0, 0, characterX*8, characterY*8, 0);
		}
	}
}

void gpworld_state::draw_pixel(bitmap_rgb32 &bitmap, const rectangle &cliprect, int x, int y, int color, int flip)
{
	if (flip)
	{
		x = bitmap.width() - x - 1;
		y = bitmap.height() - y - 1;
	}

	if (cliprect.contains(x, y))
		bitmap.pix(y, x) = m_palette->pen(color);
}

void gpworld_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const int SPR_Y_TOP     = 0;
	const int SPR_Y_BOTTOM  = 1;
	const int SPR_X_LO      = 2;
	const int SPR_X_HI      = 3;
	const int SPR_SKIP_LO   = 4;
	const int SPR_SKIP_HI   = 5;
	const int SPR_GFXOFS_LO = 6;
	const int SPR_GFXOFS_HI = 7;
	int flip = flip_screen();

	uint8_t *GFX = memregion("gfx2")->base();

	/* Heisted from Daphne which heisted it from MAME */
	for (int i = 0; i < 0x800; i += 8)
	{
		uint8_t *spr_reg = m_sprite_ram + i;

		if (spr_reg[SPR_Y_BOTTOM] && spr_reg[SPR_X_LO] != 0xff)
		{
			int src  = spr_reg[SPR_GFXOFS_LO] + (spr_reg[SPR_GFXOFS_HI] << 8);
			int skip = spr_reg[SPR_SKIP_LO] + (spr_reg[SPR_SKIP_HI] << 8);

			int height = spr_reg[SPR_Y_BOTTOM] - spr_reg[SPR_Y_TOP];
			int sy = spr_reg[SPR_Y_TOP] + 1;
			int sx = spr_reg[SPR_X_LO] + ((spr_reg[SPR_X_HI] & 0x01) << 8) ;

			int sprite_color = (spr_reg[SPR_X_HI] >> 4) & 0x0f;
			int sprite_bank  = (spr_reg[SPR_X_HI] >> 1) & 0x07;

/*
            logerror("%x - %x = %x\n", spr_reg[SPR_Y_BOTTOM], spr_reg[SPR_Y_TOP], height);
            logerror("Draw Sprite #%x with src %x, skip %x, height %x, y %x, x %x\n", i/8, src, skip, height, sy, sx);

            logerror("%02x %02x %02x %02x %02x %02x %02x %02x\n", spr_reg[SPR_Y_TOP], spr_reg[SPR_Y_BOTTOM], spr_reg[SPR_X_LO], spr_reg[SPR_X_HI],
                                                                  spr_reg[SPR_SKIP_LO], spr_reg[SPR_SKIP_HI], spr_reg[SPR_GFXOFS_LO], spr_reg[SPR_GFXOFS_HI]);
            draw_pixel(bitmap,cliprect,sx,sy,0xffffffff,flip);
*/

			for (int row = 0; row < height; row++)
			{
				int src2 = src + skip;
				src = src2;

				int x = sx;
				int y = sy+row;

				while (1)
				{
					uint8_t data_lo   = GFX[(src2 & 0x7fff) | (sprite_bank << 16)];
					uint8_t data_high = GFX[(src2 & 0x7fff) | 0x8000 | (sprite_bank << 16)];

					uint8_t pixel1 = data_high >> 0x04;
					uint8_t pixel2 = data_high & 0x0f;
					uint8_t pixel3 = data_lo >> 0x04;
					uint8_t pixel4 = data_lo & 0x0f;

					/* we'll see if this is still applicable */
					if (src & 0x8000)
					{
						uint8_t temp_pixel;
						temp_pixel = pixel1;
						pixel1 = pixel4;
						pixel4 = temp_pixel;

						temp_pixel = pixel2;
						pixel2 = pixel3;
						pixel3 = temp_pixel;

						src2--;
					}
					else
					{
						src2++;
					}

					/* Daphne says "don't draw the pixel if it's black". */
					draw_pixel(bitmap, cliprect, x+0, y, m_palette->pen_color(pixel1 + sprite_color * 0x10 + 0x200), flip);
					draw_pixel(bitmap, cliprect, x+1, y, m_palette->pen_color(pixel2 + sprite_color * 0x10 + 0x200), flip);
					draw_pixel(bitmap, cliprect, x+2, y, m_palette->pen_color(pixel3 + sprite_color * 0x10 + 0x200), flip);
					draw_pixel(bitmap, cliprect, x+3, y, m_palette->pen_color(pixel4 + sprite_color * 0x10 + 0x200), flip);

					x += 4;

					/* stop drawing when the sprite data is 0xf */
					if ((data_lo & 0x0f) == 0x0f && !(src & 0x8000))
					{
						break;
					}
					else if ((src & 0x8000) && (data_high & 0xf0) == 0xf0)
					{
						break;
					}
				}
			}
		}
	}
}


uint32_t gpworld_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	draw_tiles(bitmap, cliprect);
	draw_sprites(bitmap, cliprect);

	return 0;
}


void gpworld_state::machine_start()
{
	m_irq_stop_timer = timer_alloc(FUNC(gpworld_state::irq_stop), this);
}


/* MEMORY HANDLERS */
/* READS */
uint8_t gpworld_state::ldp_read()
{
	return m_ldp_read_latch;
}

uint8_t gpworld_state::pedal_in()
{
	if (m_brake_gas)
		return  ioport("INACCEL")->read();

	return  ioport("INBRAKE")->read();

}

/* WRITES */
void gpworld_state::ldp_write(uint8_t data)
{
	m_ldp_write_latch = data;
}

void gpworld_state::misc_io_write(uint8_t data)
{
	m_start_lamp = (data & 0x04) >> 1;
	m_nmi_enable = (data & 0x40) >> 6;
	/*  dunno      = (data & 0x80) >> 7; */ //coin counter???

	logerror("NMI : %x (0x%x)\n", m_nmi_enable, data);
}

void gpworld_state::brake_gas_write(uint8_t data)
{
	m_brake_gas = data & 0x01;
}

void gpworld_state::palette_write(offs_t offset, uint8_t data)
{
	/* This is all just a (bad) guess */
	int pal_index, r, g, b, a;

	m_palette_ram[offset] = data;

	/* "Round down" to the nearest palette entry */
	pal_index = offset & 0xffe;

	g = (m_palette_ram[pal_index]   & 0xf0) << 0;
	b = (m_palette_ram[pal_index]   & 0x0f) << 4;
	r = (m_palette_ram[pal_index+1] & 0x0f) << 4;
	a = (m_palette_ram[pal_index+1] & 0x80) ? 0 : 255;  /* guess */

	/* logerror("PAL WRITE index : %x  rgb : %d %d %d (real %x) at %x\n", pal_index, r,g,b, data, offset); */

	m_palette->set_pen_color((pal_index & 0xffe) >> 1, rgb_t(a, r, g, b));
}

/* PROGRAM MAP */
void gpworld_state::mainmem(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram().share("sprite_ram");
	map(0xc800, 0xcfff).ram().w(FUNC(gpworld_state::palette_write)).share("palette_ram"); /* The memory test reads at 0xc800 */
	map(0xd000, 0xd7ff).ram().share("tile_ram");
	map(0xd800, 0xd800).rw(FUNC(gpworld_state::ldp_read), FUNC(gpworld_state::ldp_write));
/*  map(0xd801, 0xd801).r(FUNC(gpworld_state::???)); */
	map(0xda00, 0xda00).portr("INWHEEL"); //8255 here....
/*  map(0xda01, 0xda01).w(FUNC(gpworld_state::???)); */                 /* These inputs are interesting - there are writes and reads all over these addr's */
	map(0xda02, 0xda02).w(FUNC(gpworld_state::brake_gas_write));               /*bit 0 select gas/brake input */
	map(0xda20, 0xda20).r(FUNC(gpworld_state::pedal_in));

	map(0xe000, 0xffff).ram();                              /* Potentially not all work RAM? */
}


/* I/O MAP */
void gpworld_state::mainport(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).w(FUNC(gpworld_state::misc_io_write));
	map(0x80, 0x80).portr("IN0");
	map(0x81, 0x81).portr("IN1");
	map(0x82, 0x82).portr("DSW1");
	map(0x83, 0x83).portr("DSW2");
}


/* PORTS */
static INPUT_PORTS_START( gpworld )
	PORT_START("IN0")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "SHIFT" ) PORT_CODE( KEYCODE_Q )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )     /* maybe? it's not listed in the test screen. */
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INWHEEL")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "SLIGHT RIGHT" ) PORT_CODE( KEYCODE_Y )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "MEDIUM RIGHT" ) PORT_CODE( KEYCODE_U )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "STRONG RIGHT" ) PORT_CODE( KEYCODE_I )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "FIERCE RIGHT" ) PORT_CODE( KEYCODE_O )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "SLIGHT LEFT" ) PORT_CODE( KEYCODE_T )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "MEDIUM LEFT" ) PORT_CODE( KEYCODE_R )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "STRONG LEFT" ) PORT_CODE( KEYCODE_E )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "FIERCE LEFT" ) PORT_CODE( KEYCODE_W )

	PORT_START("INACCEL")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("INBRAKE")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("DSW1")
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x50, "6 Coins/4 Credits" )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/1 Credit (again)" )
	PORT_DIPSETTING(    0x30, "5 Coins/6 Credits" )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x60, "2 Coins/3 Credits (again)" )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x05, "6 Coins/4 Credits" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/1 Credit (again)" )
	PORT_DIPSETTING(    0x03, "5 Coins/6 Credits" )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, "2 Coins/3 Credits (again)" )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x80, "Start Year #" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x80, "1984" )
	PORT_DIPSETTING(    0x00, "1985" )
	PORT_DIPNAME( 0x01, 0x01, "Steering Difficulty" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x01, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x02, "Rank to advance" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x02, "Fourth" )
	PORT_DIPSETTING(    0x00, "Third" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

TIMER_CALLBACK_MEMBER(gpworld_state::irq_stop)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

INTERRUPT_GEN_MEMBER(gpworld_state::vblank_callback)
{
	/* Do an NMI if the enabled bit is set */
	if (m_nmi_enable)
	{
		m_laserdisc->data_w(m_ldp_write_latch);
		m_ldp_read_latch = m_laserdisc->data_r();
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}

	/* The time the IRQ line stays high is set just long enough to happen after the NMI - hacky? */
	device.execute().set_input_line(0, ASSERT_LINE);
	m_irq_stop_timer->adjust(attotime::from_usec(100));
}

static const gfx_layout gpworld_tile_layout =
{
	8,8,
	0x800/8,
	2,
	{ 0x800*8, 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_gpworld )
	GFXDECODE_ENTRY("gfx1", 0, gpworld_tile_layout, 0x0, 0x100)
GFXDECODE_END

/* DRIVER */
void gpworld_state::gpworld(machine_config &config)
{
	/* main cpu */
	Z80(config, m_maincpu, GUESSED_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &gpworld_state::mainmem);
	m_maincpu->set_addrmap(AS_IO, &gpworld_state::mainport);
	m_maincpu->set_vblank_int("screen", FUNC(gpworld_state::vblank_callback));


	PIONEER_LDV1000(config, m_laserdisc, 0);
	m_laserdisc->set_overlay(512, 256, FUNC(gpworld_state::screen_update));
	m_laserdisc->add_route(0, "speaker", 1.0, 0);
	m_laserdisc->add_route(1, "speaker", 1.0, 1);

	/* video hardware */
	m_laserdisc->add_ntsc_screen(config, "screen");

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gpworld);
	PALETTE(config, m_palette).set_entries(1024);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();
}


ROM_START( gpworld )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "epr6162a.ic51", 0x0000, 0x4000, CRC(70e42574) SHA1(2fa50c7a67a2efb6b2c313850ace40e42d18b0a8) )
	ROM_LOAD( "epr6163.ic67",  0x4000, 0x4000, CRC(49539e46) SHA1(7cfd5b6b356c3fa5439e6fe3ac2e6a097b722a2c) )
	ROM_LOAD( "epr6164.ic83",  0x8000, 0x4000, CRC(7f0e6853) SHA1(c255ac6e4b61faa8da9b5aa70f12c868b81acfe1) )

	/* Tiles */
	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "epr6148.ic18", 0x0000, 0x1000, CRC(a4b11cf5) SHA1(9697494335089b13071d773812eec373ef5b358c) )

	/* Sprites */
	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "epr6149.ic111", 0x00000, 0x4000, CRC(7e6c4797) SHA1(acf934e1f3f55a0d1cd1630f6a78f9954f1ec53f) )
	ROM_LOAD( "epr6151.ic110", 0x04000, 0x4000, CRC(26d72b96) SHA1(759ef85877edfc37f2a1a242bf1c9b0e8d5e9a88) )
	ROM_LOAD( "epr6150.ic128", 0x08000, 0x4000, CRC(6837e095) SHA1(b2ac8341fcf0037d186b0759227597643ca0336d) )
	ROM_LOAD( "epr6152.ic127", 0x0c000, 0x4000, CRC(86939fe2) SHA1(07527297262297562e052672976fd6f7124f5c7b) )
	ROM_LOAD( "epr6153.ic109", 0x10000, 0x4000, CRC(f52b7c1b) SHA1(326c6b46e85ab12b43ae2b6b7a49055fe5c31431) )
	ROM_LOAD( "epr6155.ic108", 0x14000, 0x4000, CRC(a020bd03) SHA1(13cd81dc2df185d9ec989e69261d275b46e44075) )
	ROM_LOAD( "epr6154.ic126", 0x18000, 0x4000, CRC(bbf5d7db) SHA1(3dc50fa0dfe285cc741d9d6b9bac4a0ff6e3877f) )
	ROM_LOAD( "epr6156.ic125", 0x1c000, 0x4000, CRC(2c03c64c) SHA1(fa31e49385e004b6fb81c8d54cc6b64dfe1358d2) )
	ROM_LOAD( "epr6157.ic107", 0x20000, 0x4000, CRC(cdd31036) SHA1(722d843a1c524b0a689ab73bb7367c4ca33fc983) )
	/*                 ic106 Unpopulated? */
	ROM_LOAD( "epr6158.ic124", 0x28000, 0x4000, CRC(d15ac707) SHA1(170e0a851c3e845330f776a53ca619d16d025dd7) )
	/*                 ic123 Unpopulated? */

	/* Misc PROMs */
	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "pr6146.ic2",  0x000, 0x020, CRC(d10801a0) SHA1(89e9ac0d9c9eee6efd5455a3416c436ceda8f632) )
	ROM_LOAD( "pr6147.ic28", 0x020, 0x100, CRC(b7173df9) SHA1(044beda43cb1793033021a08b3ee3441d5ffe6c3) )
	ROM_LOAD( "pr5501.ic14", 0x120, 0x100, CRC(1bdf71d4) SHA1(ac52e948cce6df4abb7543c08e2c6454efd63e79) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "gpworld", 0, NO_DUMP )
ROM_END


void gpworld_state::driver_start()
{
	m_nmi_enable = 0;
	m_start_lamp = 0;
	m_brake_gas = 0;
	m_ldp_write_latch = m_ldp_read_latch = 0;
}

} // anonymous namespace


/*    YEAR  NAME      PARENT   MACHINE  INPUT    STATE          INIT        MONITOR  COMPANY  FULLNAME     FLAGS) */
GAME( 1984, gpworld,  0,       gpworld, gpworld, gpworld_state, empty_init, ROT0,    "Sega",  "GP World",  MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
