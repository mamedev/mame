// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Alien Command (c) 1993 Jaleco

Actually same HW as the Cisco Heat ones.

TODO:
- Understand what "devices" area needs to make this working.
\- Possibly the upper switches controls the UFO's and the lower switches the astronauts.
- 3D Artworks for the UFO's,Astronauts etc.
- Merge sprite chip with jaleco/cischeat.cpp
\- Uses zooming during attract, and needs pdrawgfx.

Notes:
- The real HW is a redemption machine with two guns, similar to namco/cgang.cpp.

bp 237c4,1,{curpc+=0x10;g} ; to skip initial error checks
                           ; $f009a & $f009c SW state

m68k irq table vectors
lev 1 : 0x64 : 0000 04f0 - rte
lev 2 : 0x68 : 0000 044a - vblank
lev 3 : 0x6c : 0000 0484 - "dynamic color change" (?)
lev 4 : 0x70 : 0000 04f0 - rte
lev 5 : 0x74 : 0000 04f0 - rte
lev 6 : 0x78 : 0000 04f0 - rte
lev 7 : 0x7c : 0000 04f0 - rte

===================================================================================================

Jaleco Alien Command
Redemption Video Game with Guns

2/7/99

Hardware Specs: 68000 at 12Mhz and OKI6295

JALMR17  BIN       524,288  02-07-99  1:17a JALMR17.BIN
JALCF2   BIN     1,048,576  02-07-99  1:10a JALCF2.BIN
JALCF3   BIN       131,072  02-07-99  1:12a JALCF3.BIN
JALCF4   BIN       131,072  02-07-99  1:13a JALCF4.BIN
JALCF5   BIN       524,288  02-07-99  1:15a JALCF5.BIN
JALCF6   BIN       131,072  02-07-99  1:14a JALCF6.BIN
JALGP1   BIN       524,288  02-07-99  1:21a JALGP1.BIN
JALGP2   BIN       524,288  02-07-99  1:24a JALGP2.BIN
JALGP3   BIN       524,288  02-07-99  1:20a JALGP3.BIN
JALGP4   BIN       524,288  02-07-99  1:23a JALGP4.BIN
JALGP5   BIN       524,288  02-07-99  1:19a JALGP5.BIN
JALGP6   BIN       524,288  02-07-99  1:23a JALGP6.BIN
JALGP7   BIN       524,288  02-07-99  1:19a JALGP7.BIN
JALGP8   BIN       524,288  02-07-99  1:22a JALGP8.BIN
JALMR14  BIN       524,288  02-07-99  1:17a JALMR14.BIN
JALCF1   BIN     1,048,576  02-07-99  1:11a JALCF1.BIN


**************************************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/timer.h"
#include "sound/okim6295.h"

#include "ms1_tmap.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "acommand.lh"

namespace {

class acommand_state : public driver_device
{
public:
	acommand_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_bgtmap(*this, "bgtmap")
		, m_txtmap(*this, "txtmap")
		, m_spriteram(*this, "spriteram")
		, m_oki(*this, "oki%u", 1U)
		, m_digits(*this, "digit%u", 0U)
	{ }

	void acommand(machine_config &config);

private:
	void oki_bank_w(uint8_t data);
	void output_7seg0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void output_7seg1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void output_lamps_w(uint16_t data);

	uint16_t ext_devices_0_r();
	void ext_devices_0_w(uint16_t data);
	uint16_t ext_devices_1_r();
	void ext_devices_1_w(uint16_t data);
	void ext_devices_2_w(uint16_t data);

	uint8_t m_drawmode_table[16]{};
	void draw_sprites(bitmap_ind16 &bitmap , const rectangle &cliprect, int priority1, int priority2);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);

	void main_map(address_map &map) ATTR_COLD;

	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<megasys1_tilemap_device> m_bgtmap;
	required_device<megasys1_tilemap_device> m_txtmap;
	required_shared_ptr<uint16_t> m_spriteram;
	required_device_array<okim6295_device, 2> m_oki;
	output_finder<8> m_digits;

	uint16_t m_7seg0 = 0;
	uint16_t m_7seg1 = 0;
	uint16_t m_ufo_lane[5]{};
	uint8_t m_boss_door = 0;
};

void acommand_state::machine_start()
{
	m_digits.resolve();
}

// TODO: copied over from cischeat_draw_sprites, merge in common device
void acommand_state::video_start()
{
	for (int i = 0; i < 16; i++)
		m_drawmode_table[i] = DRAWMODE_SOURCE;

	m_drawmode_table[ 0] = DRAWMODE_SHADOW;
	m_drawmode_table[15] = DRAWMODE_NONE;
}

#define SHRINK(_org_,_fact_) ( ( ( (_org_) << 16 ) * (_fact_ & 0x01ff) ) / 0x80 )

void acommand_state::draw_sprites(bitmap_ind16 &bitmap , const rectangle &cliprect, int priority1, int priority2)
{
	int x, sx, flipx, xzoom, xscale, xdim, xnum, xstart, xend, xinc;
	int y, sy, flipy, yzoom, yscale, ydim, ynum, ystart, yend, yinc;
	int code, attr, color, size, shadow;

	int min_priority, max_priority, high_sprites;

	uint16_t      *source =   m_spriteram;
	const uint16_t    *finish =   source + 0x1000/2;

	/* Move the priority values in place */
	high_sprites = (priority1 >= 16) | (priority2 >= 16);
	priority1 = (priority1 & 0x0f) * 0x100;
	priority2 = (priority2 & 0x0f) * 0x100;

	if (priority1 < priority2)  {   min_priority = priority1;   max_priority = priority2; }
	else                        {   min_priority = priority2;   max_priority = priority1; }

	for (; source < finish; source += 0x10/2 )
	{
		size    =   source[ 0 ];
		if (size & 0x1000)  continue;

		/* number of tiles */
		xnum    =   ( (size & 0x0f) >> 0 ) + 1;
		ynum    =   ( (size & 0xf0) >> 4 ) + 1;

		xzoom   =   source[ 1 ];
		yzoom   =   source[ 2 ];
		flipx   =   xzoom & 0x1000;
		flipy   =   yzoom & 0x1000;

		sx      =   source[ 3 ];
		sy      =   source[ 4 ];
		// TODO: was & 0x1ff with 0x200 as sprite wrap sign, looks incorrect with Grand Prix Star
		//       during big car on side view in attract mode (a tyre gets stuck on the right of the screen)
		//       this arrangement works with both games (otherwise Part 2 gets misaligned bleachers sprites)
		sx      =   (sx & 0x7ff);
		sy      =   (sy & 0x7ff);
		if(sx & 0x400)
			sx -= 0x800;
		if(sy & 0x400)
			sy -= 0x800;

		/* use fixed point values (16.16), for accuracy */
		sx <<= 16;
		sy <<= 16;

		/* dimension of a tile after zoom */
		{
			xdim    =   SHRINK(16,xzoom);
			ydim    =   SHRINK(16,yzoom);
		}

		if ( ( (xdim / 0x10000) == 0 ) || ( (ydim / 0x10000) == 0) )    continue;

		/* the y pos passed to the hardware is the that of the last line,
		   we need the y pos of the first line  */
		sy -= (ydim * ynum);

		code    =   source[ 6 ];
		attr    =   source[ 7 ];
		color   =   attr & 0x007f;
		shadow  =   attr & 0x1000;

		/* high byte is a priority information */
		if ( ((attr & 0x700) < min_priority) || ((attr & 0x700) > max_priority) )
			continue;

		if ( high_sprites && !(color & 0x80) )
			continue;

		xscale = xdim / 16;
		yscale = ydim / 16;


		/* let's approximate to the nearest greater integer value
		   to avoid holes in between tiles */
		if (xscale & 0xffff)    xscale += (1<<16)/16;
		if (yscale & 0xffff)    yscale += (1<<16)/16;


		if (flipx)  { xstart = xnum-1;  xend = -1;    xinc = -1; }
		else        { xstart = 0;       xend = xnum;  xinc = +1; }

		if (flipy)  { ystart = ynum-1;  yend = -1;    yinc = -1; }
		else        { ystart = 0;       yend = ynum;  yinc = +1; }

		m_drawmode_table[ 0] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		for (y = ystart; y != yend; y += yinc)
		{
			for (x = xstart; x != xend; x += xinc)
			{
				m_gfxdecode->gfx(0)->zoom_transtable(bitmap,cliprect,code++,color,flipx,flipy,
							(sx + x * xdim) / 0x10000, (sy + y * ydim) / 0x10000,
							xscale, yscale, m_drawmode_table);
			}
		}
	}
}

uint32_t acommand_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// reference has black pen background, as weird it might sound
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_bgtmap->draw(screen, bitmap, cliprect, 0, 0);
	// TODO: isn't this supposed to use pdrawgfx?
	draw_sprites(bitmap,cliprect,15,3);
	draw_sprites(bitmap,cliprect,2,2);
	draw_sprites(bitmap,cliprect,1,0);
	draw_sprites(bitmap,cliprect,0+16,0+16);
	m_txtmap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


/*************************************
 *
 * I/O
 *
 ************************************/

void acommand_state::oki_bank_w(uint8_t data)
{
	m_oki[0]->set_rom_bank(data & 0x3);
	m_oki[1]->set_rom_bank((data & 0x30) >> 4);
}


/*                                    0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f*/
static const uint8_t led_fill[0x10] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x00,0x00,0x00,0x00,0x00,0x00 };

void acommand_state::output_7seg0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_7seg0);

	// nibble 0,1,2: left 7segs, nibble 3: right 7seg 1st digit
	for (int i = 0; i < 4; i++)
		m_digits[i] = led_fill[m_7seg0 >> (i*4) & 0xf];
}

void acommand_state::output_7seg1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_7seg1);

	// nibble 0,1: right 7seg 2nd,3rd digit
	for (int i = 0; i < 2; i++)
		m_digits[i+4] = led_fill[m_7seg1 >> (i*4) & 0xf];

	// other: ?
}

/*
    "Upper switch / Under Switch"
    xx-x ---- xx-x xx-x
    -x-- ---- ---- ---- Catch Switch - 3
    --x- ---- ---- ---- Lower Switch - 3
    ---x ---- ---- ---- Upper Switch - 3
    ---- -x-- ---- ---- Catch Switch - 2
    ---- --x- ---- ---- Lower Switch - 2
    ---- ---x ---- ---- Upper Switch - 2
    ---- ---- -x-- ---- Catch Switch - 1
    ---- ---- --x- ---- Lower Switch - 1 (active high)
    ---- ---- ---x ---- Upper Switch - 1 (active low)
    ---- ---- ---- --xx Boss Door - Motor
    state of UFO lanes:
    0x0
    0x1
    0x2
    0x3
    0x4
    0x5 ufo lane limit switch
    0x6
    0x7 astronaut switch or jamming
    0x8
    0x9 ufo lane switch or motor
    0xa astronaut switch or jamming
    0xb ufo lane switch or motor
    0xc ""
    0xd ufo lane limit switch
    0xe astronaut switch or jamming
    0xf ""
*/
uint16_t acommand_state::ext_devices_0_r()
{
	return 0xfffc | m_boss_door;
}

void acommand_state::ext_devices_0_w(uint16_t data)
{
	logerror("%04x EXT 0\n",data);
	m_boss_door = data & 3;
	m_ufo_lane[0] = (data >> 8) & 0x1f;
}

/*
    ---- ---- --x- ---- Lower Switch - 5
    ---- ---- ---x ---- Upper Switch - 5
    ---- ---- ---- --x- Lower Switch - 4 (active high)
    ---- ---- ---- ---x Upper Switch - 4 (active low)
*/
uint16_t acommand_state::ext_devices_1_r()
{
	return 0xffff;
}

void acommand_state::ext_devices_1_w(uint16_t data)
{
	//logerror("%04x EXT 1\n",data);
	m_ufo_lane[1] = (data >> 0) & 0x1f;
	m_ufo_lane[2] = (data >> 8) & 0x1f;
}

void acommand_state::ext_devices_2_w(uint16_t data)
{
	//logerror("%04x EXT 2\n",data);
	m_ufo_lane[3] = (data >> 0) & 0x1f;
	m_ufo_lane[4] = (data >> 8) & 0x1f;
}

void acommand_state::output_lamps_w(uint16_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 6));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 7));

	// --xx --xx lamps
}

void acommand_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x082000, 0x082005).w(m_bgtmap, FUNC(megasys1_tilemap_device::scroll_w));
	map(0x082100, 0x082105).w(m_txtmap, FUNC(megasys1_tilemap_device::scroll_w));
	map(0x082208, 0x082209).noprw(); // watchdog
	map(0x0a0000, 0x0a3fff).ram().w(m_bgtmap, FUNC(megasys1_tilemap_device::write)).share("bgtmap");
	map(0x0b0000, 0x0b3fff).ram().w(m_txtmap, FUNC(megasys1_tilemap_device::write)).share("txtmap");
	map(0x0b8000, 0x0bffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x0f0000, 0x0f7fff).ram();
	map(0x0f8000, 0x0f8fff).ram().share("spriteram");
	map(0x0f9000, 0x0fffff).ram();

	map(0x100001, 0x100001).w(FUNC(acommand_state::oki_bank_w));
	map(0x100008, 0x100009).portr("IN0").w(FUNC(acommand_state::output_lamps_w));
	map(0x100014, 0x100017).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write)).umask16(0x00ff);
	map(0x100018, 0x10001b).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write)).umask16(0x00ff);

	map(0x100040, 0x100041).rw(FUNC(acommand_state::ext_devices_0_r), FUNC(acommand_state::ext_devices_0_w));
	map(0x100044, 0x100045).rw(FUNC(acommand_state::ext_devices_1_r), FUNC(acommand_state::ext_devices_1_w));
	map(0x100048, 0x100049).w(FUNC(acommand_state::ext_devices_2_w));

	map(0x100050, 0x100051).w(FUNC(acommand_state::output_7seg0_w));
	map(0x100054, 0x100055).w(FUNC(acommand_state::output_7seg1_w));
	map(0x10005c, 0x10005d).portr("DSW");
}

static INPUT_PORTS_START( acommand )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0020, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0040, 0x0040, "IN0" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Ticket Dispenser - 1" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Ticket Dispenser - 2")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW3:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, 0x0040, "SW3:7" )
	// Overrides Coinage
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPUNKNOWN_DIPLOC( 0x0100, 0x0100, "SW4:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW4:2" )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW4:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW4:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW4:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW4:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW4:8" )
INPUT_PORTS_END

static GFXDECODE_START( gfx_acommand )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_8x8x4_col_2x2_group_packed_msb, 0x1800, 256 )
GFXDECODE_END

TIMER_DEVICE_CALLBACK_MEMBER(acommand_state::scanline_cb)
{
	int scanline = param;

	if(scanline == 240) // vblank-out irq
		m_maincpu->set_input_line(2, HOLD_LINE);

	if(scanline == 0) // vblank-in irq? (update palette and layers)
		m_maincpu->set_input_line(3, HOLD_LINE);
}

void acommand_state::acommand(machine_config &config)
{
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &acommand_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(acommand_state::scanline_cb), "screen", 0, 1);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	// assume same as armchmp2
	screen.set_raw(XTAL(12'000'000)/2,396,0,256,256,16,240);
	screen.set_screen_update(FUNC(acommand_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_acommand);
	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 0x4000);

	MEGASYS1_TILEMAP(config, m_bgtmap, m_palette, 0x0f00);
	MEGASYS1_TILEMAP(config, m_txtmap, m_palette, 0x2700);

	// assume amplified stereo
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	OKIM6295(config, m_oki[0], 2112000, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki[0]->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	m_oki[0]->add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	OKIM6295(config, m_oki[1], 2112000, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki[1]->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	m_oki[1]->add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( acommand )
	ROM_REGION( 0x040000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jalcf3.bin",   0x000000, 0x020000, CRC(f031abf7) SHA1(e381742fd6a6df4ddae42ddb3a074a55dc550b3c) )
	ROM_LOAD16_BYTE( "jalcf4.bin",   0x000001, 0x020000, CRC(dd0c0540) SHA1(3e788fcb30ae725bd0ec9b57424e3946db1e946f) )

	ROM_REGION( 0x20000, "txtmap", 0 ) /* BG0 */
	ROM_LOAD( "jalcf6.bin",   0x000000, 0x020000, CRC(442173d6) SHA1(56c02bc2761967040127977ecabe844fc45e2218) )

	ROM_REGION( 0x080000, "bgtmap", 0 ) /* BG1 */
	ROM_LOAD( "jalcf5.bin",   0x000000, 0x080000, CRC(ff0be97f) SHA1(5ccab778318dec30849d7b7f25091d4aab8bde32) )

	ROM_REGION( 0x400000, "gfx3", 0 ) /* SPR */
	ROM_LOAD16_BYTE( "jalgp1.bin",   0x000000, 0x080000, CRC(c4aeeae2) SHA1(ee0d3dd93a604f8e1a96b55c4a1cd001d49f1157) )
	ROM_LOAD16_BYTE( "jalgp2.bin",   0x000001, 0x080000, CRC(f0e4e80e) SHA1(08252ef8b5e309cce2d4654410142f4ae9e3ef22) )
	ROM_LOAD16_BYTE( "jalgp3.bin",   0x100000, 0x080000, CRC(7acebd83) SHA1(64be95186d62003b637fcdf45a9c0b7aab182116) )
	ROM_LOAD16_BYTE( "jalgp4.bin",   0x100001, 0x080000, CRC(6a6b72f3) SHA1(3ba359b1a89eb3f6664ed83d93f79d7f895d4222) )
	ROM_LOAD16_BYTE( "jalgp5.bin",   0x200000, 0x080000, CRC(65ab751d) SHA1(f2cb8701eb8c3567a1d03248e6918c5a7b5df939) )
	ROM_LOAD16_BYTE( "jalgp6.bin",   0x200001, 0x080000, CRC(24e3ab23) SHA1(d1431688e1518ba52935f6ab44b815975bec4c27) )
	ROM_LOAD16_BYTE( "jalgp7.bin",   0x300000, 0x080000, CRC(44b71098) SHA1(a6ec2573f9a266d4f8f315f6e99b12525011f512) )
	ROM_LOAD16_BYTE( "jalgp8.bin",   0x300001, 0x080000, CRC(ce0b7838) SHA1(46e34971cb62565a3948d8c0a18086648c32e13b) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* M6295 samples */
	ROM_LOAD( "jalcf2.bin",   0x000000, 0x100000, CRC(b982fd97) SHA1(35ee5b1b9be762ccfefda24d73e329ceea876deb) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* M6295 samples */
	ROM_LOAD( "jalcf1.bin",   0x000000, 0x100000, CRC(24af21d3) SHA1(f68ab81a6c833b57ae9eef916a1c8578f3d893dd) )

	ROM_REGION( 0x100000, "user1", 0 ) /* ? these two below are identical*/
	ROM_LOAD( "jalmr14.bin",   0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) )
	ROM_LOAD( "jalmr17.bin",   0x080000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) )
ROM_END

} // anonymous namespace

GAMEL( 1994, acommand, 0, acommand, acommand, acommand_state, empty_init, ROT0, "Jaleco", "Alien Command (v2.1)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL, layout_acommand )
