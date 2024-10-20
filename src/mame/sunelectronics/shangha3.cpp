// license:BSD-3-Clause
// copyright-holders: Nicola Salmoria

/***************************************************************************

Shanghai 3           (c)1993 Sunsoft     (68000     YM2149 OKI6295)
Hebereke no Popoon   (c)1994 Sunsoft     (68000 Z80 YM3438 OKI6295)
Blocken              (c)1994 KID / Visco (68000 Z80 YM3438 OKI6295)

These games use the custom blitter GA9201 KA01-0249 (120pin IC)

driver by Nicola Salmoria

TODO:
all games:
- Blitter needs to be device-ized
shangha3:
- The zoom used for the "100" floating score when you remove tiles is very
  rough.
heberpop:
- Unknown writes to sound ports 40/41
blocken:
- incomplete zoom support, and missing rotation support. Setting the game in
  Game Mode B shows a decent test case for it by starting a play.
- attract mode tries to read at 0x80000-0xfffff area, returning 0 in there
  freezes the demo play for some frames (MT #00985). For now I've returned $ff,
  but needs HW tests to check out what lies in there (maybe a ROM mirror).
- how to play screen is bogus, it basically loses sync pretty soon.

Notes:
- a Blocken PCB shot shows a 48 MHz xtal, game is definitely too slow at
  8 MHz (noticeable thru colour cycling effects)
- Confirmed OSC is 48MHz and OKI resonator is 1.056MHz.
- Hebereke no Popoon has various debug mode switches, the ones found so far:
  $30878e: bit 0 enable, active low.
           If enabled correctly all of the non gameplay screens become
       text stubs;
  $aff14: a non-zero value enables CPU usage in 2p mode.
          As a side-effect this also makes winning condition to never
      satisfy, it goes on indefinitely with the selected characters and
      input methods.

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "speaker.h"
#include "screen.h"


namespace {

class shangha3_state : public driver_device
{
public:
	shangha3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_ram(*this, "ram")
	{ }

	void shangha3(machine_config &config);

	void init_shangha3();

protected:
	virtual void video_start() override ATTR_COLD;

protected:
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_ram;

	// driver init configuration
	uint8_t m_do_shadows = 0;

	void flipscreen_w(uint8_t data);
	void gfxlist_addr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void blitter_go_w(uint16_t data);
	void irq_ack_w(uint16_t data);
	void coinctrl_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	uint8_t m_drawmode_table[16]{};
	uint16_t m_gfxlist_addr = 0;
	bitmap_ind16 m_rawbitmap;
	int m_prot_count = 0;

	uint16_t prot_r();
	void prot_w(uint16_t data);

	void program_map(address_map &map) ATTR_COLD;
};

class heberpop_state : public shangha3_state
{
public:
	heberpop_state(const machine_config &mconfig, device_type type, const char *tag) :
		shangha3_state(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void heberpop(machine_config &config);

	void init_heberpop();

protected:
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;

	void sound_program_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;

private:
	void coinctrl_w(uint8_t data);

	void program_map(address_map &map) ATTR_COLD;
};

class blocken_state : public heberpop_state
{
public:
	blocken_state(const machine_config &mconfig, device_type type, const char *tag) :
		heberpop_state(mconfig, type, tag),
		m_okibank(*this, "okibank")
	{ }

	void blocken(machine_config &config);

	void init_blocken();

private:
	required_memory_bank m_okibank;

	void coinctrl_w(uint8_t data);

	void program_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

Custom blitter GA9201 KA01-0249 (120pin IC)


This is a tile-based blitter that writes to a frame buffer. The buffer is
never cleared, so stuff drawn is left over from frame to frame until it is
overwritten.

Tiles are stored in ROM and have a fixed 16x16 size. The blitter can draw them
as single sprites (composed of one or more tiles in the horizontal direction),
or as larger sprites whose tile codes are picked from a tilemap in RAM.

Sprites can be zoomed, distorted and rotated.

Shadows are supported on pen 14 (shangha3 makes heavy use of them) but it's not
clear how they are turned on and off. heberpop definitely doesn't use them,
pen 14 is supposed to be solid black (outlines).

The chip addresses 0x100000 bytes of memory, containing both the command list,
tilemap, and spare RAM to be used by the CPU.

The command list starts at a variable point in memory, set by
gfxlist_addr_w(), and always ends at 0x0fffff.

Large sprites refer to a single tilemap starting at 0x000000, the command list
contains the coordinates of the place in the tilemap to pick data from.

The commands list is processed when blitter_go_w() is written to, so
it is not processed automatically every frame.

The commands have a fixed length of 16 words. The format is as follows:

Word | Bit(s)           | Use
-----+-fedcba9876543210-+----------------
  0  | ---------------- | unused?
  1  | xxxx------------ | high bits of tile #; for tilemaps, this is applied to all tiles
  1  | ----xxxxxxxxxxxx | low bits of tile #; probably unused for tilemaps
  2  | ---xxxxxxxxx---- | x coordinate of destination top left corner
  3  | ---xxxxxxxxx---- | y coordinate of destination top left corner
  4  | ------------x--- | 0 = use code as-is  1 = fetch codes from tilemap RAM
  4  | -------------x-- | 1 = draw "compressed" tilemap, as if tiles were 8x8 instead of 16x16
  4  | --------------x- | flip y
  4  | ---------------x | flip x
  5  | --------x------- | unknown (used by blocken)
  5  | ---------xxx---- | high bits of color #; for tilemaps, this is applied to all tiles
  5  | ------------xxxx | low bits of color #; probably unused for tilemaps
  6  | xxxxxxxxxxxxxxxx | width-1 of destination rectangle    \ if *both* of these are 0,
  7  | xxxxxxxxxxxxxxxx | height-1 of destination rectangle   / disable sprite
  8  | xxxxxxxxxxxxxxxx | x coordinate*0x10 of source top left corner (tilemaps only?)
  9  | xxxxxxxxxxxxxxxx | y coordinate*0x10 of source top left corner (tilemaps only?)
  a  | xxxxxxxxxxxxxxxx | x zoom
  b  | xxxxxxxxxxxxxxxx | rotation
  c  | xxxxxxxxxxxxxxxx | rotation
  d  | xxxxxxxxxxxxxxxx | y zoom
  e  | ---------------- | unknown
  f  | ---------------- | unknown

***************************************************************************/


void shangha3_state::video_start()
{
	m_screen->register_screen_bitmap(m_rawbitmap);

	for (int i = 0; i < 14; i++)
		m_drawmode_table[i] = DRAWMODE_SOURCE;
	m_drawmode_table[14] = m_do_shadows ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;
	m_drawmode_table[15] = DRAWMODE_NONE;

	if (m_do_shadows)
	{
		// Prepare the shadow table
		for (int i = 0; i < 128; i++)
			m_palette->shadow_table()[i] = i + 128;
	}

	save_item(NAME(m_gfxlist_addr));
	save_item(NAME(m_rawbitmap));
}



void shangha3_state::flipscreen_w(uint8_t data)
{
	// bit 7 flips screen, the rest seems to always be set to 0x7e
	flip_screen_set(data & 0x80);

	if ((data & 0x7f) != 0x7e) popmessage("flipscreen_w %02x", data);
}

void shangha3_state::gfxlist_addr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_gfxlist_addr);
}


void shangha3_state::blitter_go_w(uint16_t data)
{
	auto profile = g_profiler.start(PROFILER_VIDEO);

	for (int offs = m_gfxlist_addr << 3; offs < m_ram.bytes() / 2; offs += 16)
	{
		int code = m_ram[offs + 1];
		int const color = m_ram[offs + 5] & 0x7f;
		int flipx = m_ram[offs + 4] & 0x01;
		int flipy = m_ram[offs + 4] & 0x02;
		int sx = (m_ram[offs + 2] & 0x1ff0) >> 4;
		if (sx >= 0x180) sx -= 0x200;
		int sy = (m_ram[offs + 3] & 0x1ff0) >> 4;
		if (sy >= 0x100) sy -= 0x200;
		int const sizex = m_ram[offs + 6];
		int const sizey = m_ram[offs + 7];
		int const zoomx = m_ram[offs + 10];
		int const zoomy = m_ram[offs + 13];

		if (flip_screen())
		{
			sx = 383 - sx - sizex;
			sy = 255 - sy - sizey;
			flipx = !flipx;
			flipy = !flipy;
		}

		if ((sizex || sizey)
				// avoid garbage on startup
		&& sizex < 512 && sizey < 256 && zoomx < 0x1f0 && zoomy < 0x1f0)
		{
			rectangle myclip;

//          if (m_ram[offs + 11] || m_ram[offs + 12])
//              logerror("offs %04x: sx %04x sy %04x zoom %04x %04x %04x %04x fx %d fy %d\n", offs, sx, sy, zoomx, m_ram[offs + 11]), m_ram[offs + 12], zoomy, flipx, flipy);

			myclip.set(sx, sx + sizex, sy, sy + sizey);
			myclip &= m_rawbitmap.cliprect();

			if (m_ram[offs + 4] & 0x08)    // tilemap
			{
				int const condensed = m_ram[offs + 4] & 0x04;

				int srcx = m_ram[offs + 8] / 16;
				int srcy = m_ram[offs + 9] / 16;
				int const dispx = srcx & 0x0f;
				int const dispy = srcy & 0x0f;

				int h = (sizey + 15) / 16 + 1;
				int w = (sizex + 15) / 16 + 1;

				if (condensed)
				{
					h *= 2;
					w *= 2;
					srcx /= 8;
					srcy /= 8;
				}
				else
				{
					srcx /= 16;
					srcy /= 16;
				}

				for (int y = 0; y < h; y++)
				{
					for (int x = 0; x < w; x++)
					{
						int dx, dy, tile;

						// TODO: zooming algo is definitely wrong for Blocken here
						if (condensed)
						{
							int const addr = ((y + srcy) & 0x1f) |
											(((x + srcx) & 0xff) << 5);
							tile = m_ram[addr];
							dx = 8 * x * (0x200 - zoomx) / 0x100 - dispx;
							dy = 8 * y * (0x200 - zoomy) / 0x100 - dispy;
						}
						else
						{
							int const addr = ((y + srcy) & 0x0f) |
											(((x + srcx) & 0xff) << 4) |
											(((y + srcy) & 0x10) << 8);
							tile = m_ram[addr];
							dx = 16 * x * (0x200 - zoomx) / 0x100 - dispx;
							dy = 16 * y * (0x200 - zoomy) / 0x100 - dispy;
						}

						if (flipx) dx = sx + sizex - 15 - dx;
						else dx = sx + dx;
						if (flipy) dy = sy + sizey - 15 - dy;
						else dy = sy + dy;

						m_gfxdecode->gfx(0)->transpen(m_rawbitmap, myclip,
								(tile & 0x0fff) | (code & 0xf000),
								(tile >> 12) | (color & 0x70),
								flipx, flipy,
								dx, dy, 15);
					}
				}
			}
			else    // sprite
			{
				if (zoomx <= 1 && zoomy <= 1)
					m_gfxdecode->gfx(0)->zoom_transtable(m_rawbitmap, myclip,
							code,
							color,
							flipx, flipy,
							sx, sy,
							0x1000000, 0x1000000,
							m_drawmode_table);
				else
				{
					int w = (sizex + 15) / 16;

					for (int x = 0; x < w; x++)
					{
						m_gfxdecode->gfx(0)->zoom_transtable(m_rawbitmap, myclip,
								code,
								color,
								flipx, flipy,
								sx + 16 * x,sy,
								(0x200 - zoomx) * 0x100, (0x200 - zoomy) * 0x100,
								m_drawmode_table);

						if ((code & 0x000f) == 0x0f)
							code = (code + 0x100) & 0xfff0;
						else
							code++;
					}
				}
			}
		}
	}
}


uint32_t shangha3_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_rawbitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


// this looks like a simple protection check
/*
write    read
78 78 -> 0
9b 10 -> 1
9b 20 -> 3
9b 40 -> 7
9b 80 -> f
08    -> e
10    -> c
20    -> 8
40    -> 0
*/
uint16_t shangha3_state::prot_r()
{
	static const int result[] = { 0x0, 0x1, 0x3, 0x7, 0xf, 0xe, 0xc, 0x8, 0x0};

	logerror("PC %04x: read 20004e\n", m_maincpu->pc());

	return result[m_prot_count++ % 9];
}

void shangha3_state::prot_w(uint16_t data)
{
	logerror("PC %04x: write %02x to 20004e\n", m_maincpu->pc(), data);
}

void shangha3_state::coinctrl_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 2));
	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 2));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
}

void heberpop_state::coinctrl_w(uint8_t data)
{
	// the Oki ROM bank is selected by the main CPU!
	m_oki->set_rom_bank(BIT(data, 3));

	shangha3_state::coinctrl_w(data);
}

void blocken_state::coinctrl_w(uint8_t data)
{
	// the Oki ROM bank is selected by the main CPU!
	m_okibank->set_entry((data >> 4) & 3);

	shangha3_state::coinctrl_w(data);
}


void shangha3_state::irq_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(4, CLEAR_LINE);
}

void shangha3_state::program_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x100fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x200000, 0x200001).portr("INPUTS");
	map(0x200002, 0x200003).portr("SYSTEM");
	map(0x200008, 0x200009).w(FUNC(shangha3_state::blitter_go_w));
	map(0x20000a, 0x20000b).w(FUNC(shangha3_state::irq_ack_w));
	map(0x20000c, 0x20000c).w(FUNC(shangha3_state::coinctrl_w));
	map(0x20001f, 0x20001f).r("aysnd", FUNC(ym2149_device::data_r));
	map(0x20002f, 0x20002f).w("aysnd", FUNC(ym2149_device::data_w));
	map(0x20003f, 0x20003f).w("aysnd", FUNC(ym2149_device::address_w));
	map(0x20004e, 0x20004f).rw(FUNC(shangha3_state::prot_r), FUNC(shangha3_state::prot_w));
	map(0x20006f, 0x20006f).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x300000, 0x30ffff).ram().share(m_ram); // gfx & work RAM
	map(0x340001, 0x340001).w(FUNC(shangha3_state::flipscreen_w));
	map(0x360000, 0x360001).w(FUNC(shangha3_state::gfxlist_addr_w));
}

void heberpop_state::program_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x100fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x200000, 0x200001).portr("INPUTS");
	map(0x200002, 0x200003).portr("SYSTEM");
	map(0x200004, 0x200005).portr("DSW");
	map(0x200008, 0x200009).w(FUNC(heberpop_state::blitter_go_w));
	map(0x20000a, 0x20000b).w(FUNC(heberpop_state::irq_ack_w));
	map(0x20000d, 0x20000d).w(FUNC(heberpop_state::coinctrl_w));
	map(0x20000f, 0x20000f).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x300000, 0x30ffff).ram().share(m_ram); // gfx & work RAM
	map(0x340001, 0x340001).w(FUNC(heberpop_state::flipscreen_w));
	map(0x360000, 0x360001).w(FUNC(heberpop_state::gfxlist_addr_w));
	map(0x800000, 0xb7ffff).rom().region("blitter", 0);
}

void blocken_state::program_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x100001).portr("INPUTS");
	map(0x100002, 0x100003).portr("SYSTEM").nopw(); // w -> unknown purpose
	map(0x100004, 0x100005).portr("DSW");
	map(0x100008, 0x100009).w(FUNC(blocken_state::blitter_go_w));
	map(0x10000a, 0x10000b).nopr().w(FUNC(blocken_state::irq_ack_w)); // r -> unknown purpose (value doesn't matter, left-over?)
	map(0x10000d, 0x10000d).w(FUNC(blocken_state::coinctrl_w));
	map(0x10000f, 0x10000f).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x200000, 0x200fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x300000, 0x30ffff).ram().share(m_ram); // gfx & work RAM
	map(0x340001, 0x340001).w(FUNC(blocken_state::flipscreen_w));
	map(0x360000, 0x360001).w(FUNC(blocken_state::gfxlist_addr_w));
	map(0x800000, 0xb7ffff).rom().region("blitter", 0);
}


void heberpop_state::sound_program_map(address_map &map)
{
	map(0x0000, 0xf7ff).rom();
	map(0xf800, 0xffff).ram();
}

void heberpop_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ymsnd", FUNC(ym3438_device::read), FUNC(ym3438_device::write));
	map(0x80, 0x80).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xc0, 0xc0).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

/* $00000-$20000 stays the same in all sound banks,
   the second half of the bank is what gets switched */
void blocken_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr(m_okibank);
}

static INPUT_PORTS_START( shangha3 )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE(0x0020, IP_ACTIVE_LOW)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW1") // DIP switch locations assigned as per service mode
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2") // DIP switch locations assigned as per service mode
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Base Time" )         PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x04, "70 sec" )
	PORT_DIPSETTING(    0x0c, "80 sec" )
	PORT_DIPSETTING(    0x08, "90 sec" )
	PORT_DIPSETTING(    0x00, "100 sec" )
	PORT_DIPNAME( 0x30, 0x30, "Additional Time" )       PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x10, "4 sec" )
	PORT_DIPSETTING(    0x30, "5 sec" )
	PORT_DIPSETTING(    0x20, "6 sec" )
	PORT_DIPSETTING(    0x00, "7 sec" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( heberpop )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") // vblank?? has to toggle
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") // vblank?? has to toggle

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE(0x0020, IP_ACTIVE_LOW)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") // DIP switch locations assigned as per service mode
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Allow Diagonal Moves" )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( blocken )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") // vblank?? has to toggle
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") // vblank?? has to toggle

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE )  // keeping this pressed on boot generates "BAD DIPSW"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") // DIP switch locations assigned as per service mode
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Game Type" )         PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, "A" )
	PORT_DIPSETTING(      0x0000, "B" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Players ) )      PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0030, "1" )
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0010, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_7C ) )
INPUT_PORTS_END



static GFXDECODE_START( gfx_shangha3 )
	GFXDECODE_ENTRY( "blitter", 0, gfx_16x16x4_packed_lsb, 0, 128 )
GFXDECODE_END


void shangha3_state::shangha3(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 48_MHz_XTAL / 3); // TMP68HC000N-16
	m_maincpu->set_addrmap(AS_PROGRAM, &shangha3_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(shangha3_state::irq4_line_assert));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
//  m_screen->set_refresh_hz(60);
//  m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
//  m_screen->set_size(24*16, 16*16);
//  m_screen->set_visarea(0*16, 24*16-1, 1*16, 15*16-1);
	m_screen->set_raw(48_MHz_XTAL / 6, 512, 0, 24 * 16, 263, 1 * 16, 15 * 16); // refresh rate is unknown
	m_screen->set_screen_update(FUNC(shangha3_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_shangha3);

	PALETTE(config, m_palette).set_format(palette_device::RGBx_555, 2048).enable_shadows();

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2149_device &aysnd(YM2149(config, "aysnd", 48_MHz_XTAL / 32)); // 1.5MHz
	aysnd.port_a_read_callback().set_ioport("DSW1");
	aysnd.port_b_read_callback().set_ioport("DSW2");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.30);

	OKIM6295(config, m_oki, 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH); // pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void heberpop_state::heberpop(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 48_MHz_XTAL / 3); // TMP68HC000N-16 like the others??
	m_maincpu->set_addrmap(AS_PROGRAM, &heberpop_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(heberpop_state::irq4_line_assert));

	Z80(config, m_audiocpu, 48_MHz_XTAL / 8);  // 6 MHz ???
	m_audiocpu->set_addrmap(AS_PROGRAM, &heberpop_state::sound_program_map);
	m_audiocpu->set_addrmap(AS_IO, &heberpop_state::sound_io_map);  // NMI triggered by YM3438

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
//  m_screen->set_refresh_hz(60);
//  m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
//  m_screen->set_size(24*16, 16*16);
//  m_screen->set_visarea(0*16, 24*16-1, 1*16, 15*16-1);
	m_screen->set_raw(48_MHz_XTAL / 6, 512, 0, 24 * 16, 263, 1 * 16, 15 * 16); // refresh rate is unknown
	m_screen->set_screen_update(FUNC(heberpop_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_shangha3);

	PALETTE(config, m_palette).set_format(palette_device::RGBx_555, 2048).enable_shadows();

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	ym3438_device &ymsnd(YM3438(config, "ymsnd", 48_MHz_XTAL / 6)); // 8 MHz?
	ymsnd.irq_handler().set_inputline("audiocpu", INPUT_LINE_NMI);
	ymsnd.add_route(0, "mono", 0.40);
	ymsnd.add_route(1, "mono", 0.40);

	OKIM6295(config, m_oki, 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH); // pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void blocken_state::blocken(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 48_MHz_XTAL / 3); // TMP68HC000N-16
	m_maincpu->set_addrmap(AS_PROGRAM, &blocken_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(blocken_state::irq4_line_assert));

	Z80(config, m_audiocpu, 48_MHz_XTAL / 8);   // 6 MHz?
	m_audiocpu->set_addrmap(AS_PROGRAM, &blocken_state::sound_program_map);
	m_audiocpu->set_addrmap(AS_IO, &blocken_state::sound_io_map);  // NMI triggered by YM3438

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
//  m_screen->set_refresh_hz(60);
//  m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
//  m_screen->set_size(24*16, 16*16);
//  m_screen->set_visarea(0*16, 24*16-1, 1*16, 15*16-1);
	m_screen->set_raw(48_MHz_XTAL / 6, 512, 0, 24 * 16, 263, 1 * 16, 15 * 16); // refresh rate is unknown
	m_screen->set_screen_update(FUNC(blocken_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_shangha3);

	PALETTE(config, m_palette).set_format(palette_device::RGBx_555, 2048).enable_shadows();

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	ym3438_device &ymsnd(YM3438(config, "ymsnd", 48_MHz_XTAL / 6)); // 8 MHz?
	ymsnd.irq_handler().set_inputline("audiocpu", INPUT_LINE_NMI);
	ymsnd.add_route(0, "mono", 0.40);
	ymsnd.add_route(1, "mono", 0.40);

	OKIM6295(config, m_oki, 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->set_addrmap(0, &blocken_state::oki_map);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/


/*

Shanghai 3  (c)  1993 Sunsoft

SUN04C
+-------------------------------------+
|        M6295 IC75                   |
|    1.056MHz  SW2                    |
| VOL  YM2149F SW1                    |
|                                     |
|                                     |
|J                    M M             |
|A                    2 2             |
|M        M                           |
|M        1    M    +------+     IC48*|
|A             3    |GA9201|     IC47*|
|         M         | KA01 |     IC46*|
|         1    M    | 0249 |     IC44*|
|              3    +------+          |
|    68000                        I   |
| I I                             C   |
| C C 48MHz                       4   |
| 3 2                             3   |
+-------------------------------------+

   CPU: TMP68HC000-16
 Sound: YM2149F, OKI M6295
 Video: GA9201 KA01-0249 (QFP120)
   OSC: 48MHz, 1.056MHz (resonator)
Memory: M1 = TMM2018AP-45 (2K x 8 SRAM)
        M2 = LH52B256D-70LL (32K x 8 SRAM)
        M3 = TC514280BJL-70 (256K x 4 DRAM)
 Other: SW1 & SW2 - 8-position dipswitch
        VOL - Volume pot

* = unpopulated 32 pin ROM sockets silkscreened 27C040

NOTE: For the "World" set, it differs from the US set (besides the US set having the data repeated) by 2 bytes.

  0xB8B == 0x00 for world, 0x12 for US set (flag to show FBI warning screen)
0x32800 == 0xB9 for world, 0xCB for US set (checksum adjustment)

*/

ROM_START( shangha3 ) // PCB labeled SUN04C - Has two additional tiles sets to choose from.
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ic3",  0x00000, 0x40000, CRC(4a9cdcfd) SHA1(c27b767ef2de90b36095b49baae9fa514f461c2c) ) // ST M27C2001 EPROM with no label
	ROM_LOAD16_BYTE( "ic2",  0x00001, 0x40000, CRC(714bfdbc) SHA1(0ce611624e8a5e28cba5443b63b8872eed9f68fc) ) // ST M27C2001 EPROM with no label

	ROM_REGION16_BE( 0x400000, "blitter", 0 )
	ROM_LOAD( "s3j_char-a1.ic43", 0x000000, 0x200000, CRC(2dbf9d17) SHA1(dd94ddc4bb02ab544aa3f89b614afc46678cc48d) ) // 42pin mask ROM
	ROM_LOAD( "27c4000.ic44",     0x200000, 0x080000, CRC(6344ffb7) SHA1(06bc5bcf94973ec152e7abf9cc658ef319eb4b65) ) // Korean fonts, vs mode how to play etc? (probably for Korean program ROMs we don't have, but was on World board)

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "s3j_v10.ic75", 0x00000, 0x40000, CRC(f0cdc86a) SHA1(b1017a9841a56e0f5d2714f550f64ed1f4e238e6) )
ROM_END

ROM_START( shangha3u ) // PCB labeled SUN04C - Shows FBI "Winners Don't Use Drugs" splash screen (once). Has two additional tiles sets to choose from.
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s3u_ic3_v1.0.ic3",  0x00000, 0x80000, CRC(53ef4988) SHA1(63f098d95865928a553e945fe60dea79aa16c603) ) // ST M27C4001 EPROM labeled S3U IC3 V1.0
	ROM_LOAD16_BYTE( "s3u_ic2_v1.0.ic2",  0x00001, 0x80000, CRC(fdea0232) SHA1(8983a646412df01b6bc66994700796e7b7fcbb61) ) // ST M27C4001 EPROM labeled S3U IC2 V1.0
	// both program ROMs are double sized with the identical halves

	ROM_REGION16_BE( 0x200000, "blitter", 0 )
	ROM_LOAD( "s3j_char-a1.ic43", 0x000000, 0x200000, CRC(2dbf9d17) SHA1(dd94ddc4bb02ab544aa3f89b614afc46678cc48d) ) // 42pin mask ROM

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s3j_ic75v1.0.ic75", 0x00000, 0x80000, CRC(a8136d8c) SHA1(8028bda5642c2546c1ac8da78dbff4084829f03b) ) // 27C4001 labeled S3J IC75V1.0 with 1st & 2nd halves == s3j_v10.ic75
ROM_END

ROM_START( shangha3up ) // PCB labeled SUN04 with a sticker labeled PCB 001, a prototyping version of the later SUN04C
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "syan3u_evn_10-7.ic3",  0x00000, 0x40000, CRC(a1f5275a) SHA1(71a024205bd5e6385bd9d746c339f0327bd1c1d6) ) // ST M27C2001 EPROM hand written label:  SYAN3U  EVN 10/7
	ROM_LOAD16_BYTE( "syan3u_odd_10-7.ic2",  0x00001, 0x40000, CRC(fe3960bf) SHA1(545473260d959b8ed8145263d54f5f4523a844c4) ) // ST M27C2001 EPROM hand written label:  SYAN3U  ODD 10/7

	ROM_REGION16_BE( 0x200000, "blitter", 0 ) // same data as the 42 pin mask S3J CHAR-A1
	ROM_LOAD( "s3j_chr-a1_1_sum_53b1_93.9.20.ic80", 0x000000, 0x80000, CRC(fcaf795b) SHA1(312d85f39087564d67f12e0287f508b94b1493af) ) // HN27C4001 hand written label:  S3J-CHR-A1  #1  SUM: 53B1  93.9.20
	ROM_LOAD( "s3j_chr-a1_2_sum_0e32_93.9.20.ic81", 0x080000, 0x80000, CRC(5a564f50) SHA1(34ca2ecd7101961e657034082802d89db5b4b7bd) ) // HN27C4001 hand written label:  S3J-CHR-A1  #2  SUM: 0E32  93.9.20
	ROM_LOAD( "s3j_chr-a1_3_sum_0d9a_93.9.20.ic82", 0x100000, 0x80000, CRC(2b333c69) SHA1(6e720de5d222be25857ab18902636587e8c6afb8) ) // HN27C4001 hand written label:  S3J-CHR-A1  #3  SUM: 0D9A  93.9.20
	ROM_LOAD( "s3j_chr-a1_4_sum_27e7_93.9.20.ic83", 0x180000, 0x80000, CRC(19be7039) SHA1(53839460b53144120cc2f68992c054062efa939b) ) // HN27C4001 hand written label:  S3J-CHR-A1  #4  SUM: 27E7  93.9.20

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "pcm_9-16_0166.ic75", 0x00000, 0x40000, CRC(f0cdc86a) SHA1(b1017a9841a56e0f5d2714f550f64ed1f4e238e6) ) // Hand written label:  <kanji for Shanghai III>  PCM  9/16 0166
ROM_END

ROM_START( shangha3j ) // PCB labeled SUN04C
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s3j_v11.ic3",  0x00000, 0x40000, CRC(e98ce9c8) SHA1(359e117aebb644d7b235add7e71ed6891243d451) )
	ROM_LOAD16_BYTE( "s3j_v11.ic2",  0x00001, 0x40000, CRC(09174620) SHA1(1d1639c07895f715facfe153fbdb6ae0f3cdd876) )

	ROM_REGION16_BE( 0x200000, "blitter", 0 )
	ROM_LOAD( "s3j_char-a1.ic43", 0x000000, 0x200000, CRC(2dbf9d17) SHA1(dd94ddc4bb02ab544aa3f89b614afc46678cc48d) ) // 42pin mask ROM

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "s3j_v10.ic75", 0x00000, 0x40000, CRC(f0cdc86a) SHA1(b1017a9841a56e0f5d2714f550f64ed1f4e238e6) )
ROM_END

ROM_START( heberpop ) // PCB labeled SUN-06
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hbp_ic31.ic31",  0x00000, 0x80000, CRC(c430d264) SHA1(4be12b1fa90da09047db3a31171ffda8ab8bd851) )
	ROM_LOAD16_BYTE( "hbp_ic32.ic32",  0x00001, 0x80000, CRC(bfa555a8) SHA1(754f581554022b98ba8e78ee96f846faa2cedc69) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "hbp_ic34_v1.0.ic34",  0x00000, 0x10000, CRC(0cf056c6) SHA1(9992cd3879d9a57fcb784fc1e11d6b6d87e5a366) )

	ROM_REGION16_BE( 0x380000, "blitter", ROMREGION_ERASEFF )
	ROM_LOAD( "hbp_ic98_v1.0.ic98",   0x000000, 0x80000, CRC(a599100a) SHA1(f2e517256a42b3fa4a047bbe742d714f568cc117) )
	ROM_LOAD( "hbp_ic99_v1.0.ic99",   0x080000, 0x80000, CRC(fb8bb12f) SHA1(78c1fec1371d312e113d92803dd59acc36604989) )
	ROM_LOAD( "hbp_ic100_v1.0.ic100", 0x100000, 0x80000, CRC(05a0f765) SHA1(4f44cf367c3697eb6c245297c9d05160d7d94e24) )
	ROM_LOAD( "hbp_ic101_v1.0.ic101", 0x180000, 0x80000, CRC(151ba025) SHA1(b6ebe60872957a2625e666d53a5a4bc941a1f21c) )
	ROM_LOAD( "hbp_ic102_v1.0.ic102", 0x200000, 0x80000, CRC(2b5e341a) SHA1(c7ad2dafb3433296c117978434e1699290267891) )
	ROM_LOAD( "hbp_ic103_v1.0.ic103", 0x280000, 0x80000, CRC(efa0e745) SHA1(fc1d52d35b3c902d8b25403b0e13f86a04039bc4) )
	ROM_LOAD( "hbp_ic104_v1.0.ic104", 0x300000, 0x80000, CRC(bb896bbb) SHA1(4311876628beb82cbacdab4d055c3738e74241b0) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "hbp_ic53_v1.0.ic53",  0x00000, 0x80000, CRC(a4483aa0) SHA1(be301d8ac6d69f5c3fdbcb85bd557090e46da1ff) )
ROM_END

ROM_START( blocken ) // PCB labeled KID-07
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ic31j.bin",    0x00000, 0x20000, CRC(ec8de2a3) SHA1(09a6b8c1b656b17ab3d1fc057902487e4f94cf02) )
	ROM_LOAD16_BYTE( "ic32j.bin",    0x00001, 0x20000, CRC(79b96240) SHA1(c1246bd4b91fa45c581a8fdf90cc6beb85adf8ec) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ic34.bin",     0x00000, 0x10000, CRC(23e446ff) SHA1(82c03b45b337696b0f8293c446544d7ee080d415) )

	ROM_REGION16_BE( 0x380000, "blitter", ROMREGION_ERASEFF )
	ROM_LOAD( "ic98j.bin",    0x000000, 0x80000, CRC(35dda273) SHA1(95850d12ca1557c14bc471ddf925aaf423313ff0) )
	ROM_LOAD( "ic99j.bin",    0x080000, 0x80000, CRC(ce43762b) SHA1(e1c51ea0b54b5febdee127619e15f1cda650cb4c) )
	// 100000-1fffff empty
	ROM_LOAD( "ic100j.bin",   0x200000, 0x80000, CRC(a34786fd) SHA1(7d4879cbaa055c2ddbe6d20dd946bf0e3e069d4d) )
	// 280000-37ffff empty

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "ic53.bin",     0x00000, 0x80000, CRC(86108c56) SHA1(aa405fa2eec5cc178ef6226f229a12dac09504f0) )
ROM_END



void shangha3_state::init_shangha3()
{
	m_do_shadows = 1;

	save_item(NAME(m_prot_count));
}

void heberpop_state::init_heberpop()
{
	m_do_shadows = 0;

	// sound CPU runs in IM 0
	m_audiocpu->set_input_line_vector(0, 0xff);  // Z80 - RST 38h
}

void blocken_state::init_blocken()
{
	init_heberpop();
	m_okibank->configure_entries(0, 4, memregion("oki")->base(), 0x20000);
}

} // anonymous namespace


GAME( 1993, shangha3,   0,        shangha3, shangha3, shangha3_state, init_shangha3, ROT0, "Sunsoft",         "Shanghai III (World)",         MACHINE_SUPPORTS_SAVE )
GAME( 1993, shangha3u,  shangha3, shangha3, shangha3, shangha3_state, init_shangha3, ROT0, "Sunsoft",         "Shanghai III (US)",            MACHINE_SUPPORTS_SAVE )
GAME( 1993, shangha3up, shangha3, shangha3, shangha3, shangha3_state, init_shangha3, ROT0, "Sunsoft",         "Shanghai III (US, prototype)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, shangha3j,  shangha3, shangha3, shangha3, shangha3_state, init_shangha3, ROT0, "Sunsoft",         "Shanghai III (Japan)",         MACHINE_SUPPORTS_SAVE )
GAME( 1994, heberpop,   0,        heberpop, heberpop, heberpop_state, init_heberpop, ROT0, "Sunsoft / Atlus", "Hebereke no Popoon (Japan)",   MACHINE_SUPPORTS_SAVE )
GAME( 1994, blocken,    0,        blocken,  blocken,  blocken_state,  init_blocken,  ROT0, "Visco / KID",     "Blocken (Japan)",              MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
