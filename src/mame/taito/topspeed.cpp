// license:BSD-3-Clause
// copyright-holders: David Graves
// thanks-to: Richard Bush

/****************************************************************************

Top Speed / Full Throttle    (c) Taito 1987
-------------------------

David Graves

Sources:        Rastan driver by Jarek Burczynski
            MAME Taito F2 & Z drivers
            Raine source - special thanks to Richard Bush
              and the Raine Team.

                *****

Top Speed / Full Throttle is the forerunner of the Taito Z system on
which Taito's driving games were based from 1988-91. (You can spot some
similarities with Continental Circus, the first of the TaitoZ games.)

The game hardware has 5 separate layers of graphics - four 64x64 tiled
scrolling background planes of 8x8 tiles (two of which are used for
drawing the road), and a sprite plane.

Taito got round the limitations of the tilemap generator they were using
(which only supports two layers) by using a pair of them.

[Trivia: Taito employed the same trick three years later, this time with
the TC0100SCN in "Thunderfox".]

Top Speed's sprites are 16x8 tiles aggregated through a RAM sprite map
area into 128x128 big sprites. (The TaitoZ system also used a similar
sprite map system, but moved the sprite map from RAM to ROM.)

Top Speed has twin 68K CPUs which communicate via $10000 bytes of
shared ram. The first 68000 handles screen, palette and sprites, and
the road. The second 68000 handles inputs/dips, and does data processing
in shared ram to relieve CPUA. There is also a Z80, which takes over
sound duties.


PCB contents (from CPU PCB photo)
-------------

XTAL: 16000.00KHZ and 26686.00KHZ
CPU: 2 * 68000-8, Z80 + Z80 CTC
Sound: YM2151, YM3012(DAC), 2 * OKI M5205
Taito: 2 * PC080SN, PC060HA, TC0040IOC, 2 * TC0060DCA, PC050CM



TODO Lists
==========

Minor black glitches on the road: these are all on the right
hand edge of the tilemap making up the "left" half: this is
the upper of the two road tilemaps so any gunk will be visible.

'Tearing' effect between the two road tilemaps is visible when
single-stepping. A sync issue?

CPUA (on all variants) could have a spin_until_int at $63a.

Motor CPU: appears to be identical to one in ChaseHQ.


Stephh's notes (based on the game M68000 code and some tests) :

1) 'topspeed' and 'topspedu'

  - All addresses are for 2nd M68000 CPU !
  - Region stored at 0x01fffe.w
  - Sets :
      * 'topspeed' : region = 0x0003
      * 'topspedu' : region = 0x0004
  - Coinage relies on the region (code at 0x00dd10) :
      * 0x0001 (Japan), 0x0002 (US) and 0x0004 (US, Romstar license) use TAITO_COINAGE_JAPAN_OLD
      * 0x0003 (World) uses TAITO_COINAGE_WORLD
  - Notice screen only if region = 0x0001
  - Game name : "Top Speed"
  - It's only possible to continue a game when you reach at least level 2
  - The "Continue Price" Dip Switch is a bit weird when set to 1C_1C :
      * coin 1 : 1C_1C (normal behaviour)
      * coin 2 : same number of credits as per Coin B settings, but twice coins
        for example, when Coin B setting is 1C_2C, when pressing COIN2,
        you'll get 2 credits, but you'll be able to continue 4 times
        before needing to insert another coin
  - There is sort of built-in cheat (code at 0x015332) :
      * set "Allow Continue" Dip Switch to "No"
      * set "Continue Price" Dip Switch to 1C_1C
      * set contents of 0x000402.b to 0x55 (be aware that this address is in ROM area)
      * you'll then be awarded infinite time :)


2) 'fullthrl'

  - All addresses are for 2nd M68000 CPU !
  - Region stored at 0x01fffe.w
  - Sets :
      * 'fullthrl' : region = 0x0001
  - Game name : "Full Throttle"
  - Same other notes as for 'topspeed'

Main board
    V  Connector                 G  Connector
    ------------                 ------------
    1  Video GND                Solder   Parts
    2  Video RED                GND  A   1  GND
    3  Video GREEN              GND  B   2  GND
    4  Video BLUE               +5V  C   3  +5V
    5  Video SYNC               +5V  D   4  +5V
                                -5V  E   5  -5V
                               +13V  F   6  +12V
    H  Connector                ---  H   7  ---
    ------------     Coin Counter 2  J   8  Coin Counter 1
    1  GND           Coin Lockout 2  K   9  Coin Lockout 1
    2  GND          Speaker CH1 [-]  L  10  Speaker CH1 [+]
    3  GND          Speaker CH2 [-]  M  11  Speaker CH2 [+]
    4  GND                 Volume 1  N  12  Volume 3
    5  +5V                      ---  P  13  Volume 2
    6  +5V               Service SW  R  14  GND
    7  +5V               Brake SW 1  S  15  GND
    8  +5V                Coin SW 2  T  16  Coin SW 1
    9  -5V               Brake SW 3  U  17  Brake SW 2
    10 ---                  Tilt SW  V  18  Nitro SW
    11 +12V             1P Start SW  W  19  Handle Center SW
    12 ---               Accel SW 1  X  20  Shift SW
                         Accel SW 3  Y  21  Accel SW 2
                                ---  Z  22  ---
                                ---  a  23  ---
                                ---  b  24  ---
                                ---  c  25  ---
                  Handle Sensor [-]  d  26  Handle Sensor [+]
                                GND  e  27  GND
                                GND  f  28  GND

Handle Sensor board
    1  Handle Sensor [-]
    2  +5V
    3  GND
    4  Handle Sensor [+]

Sound Volume Connection
    Sound Volume 1  1  ----   -\
    Sound Volume 2  2  ----   -----| short with only one pin
    Sound Volume 3  3  ----   -/   |
               GND  4  ------------|

From JP manual
    If Cabinet is Upright (see DSWA:1,2), These SWs are unused.
        Accel SW 2 (21), Accel SW 3 (Y), Brake SW 2 (17), Brake SW 3 (U) and Handle Center SW (19)

***************************************************************************/

#include "emu.h"

#include "pc080sn.h"
#include "taitoio.h"
#include "taitoipt.h"
#include "taitosnd.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "sound/flt_vol.h"
#include "sound/msm5205.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "topspeed.lh"


namespace {

class topspeed_state : public driver_device
{
public:
	topspeed_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_spritemap(*this, "spritemap")
		, m_raster_ctrl(*this, "raster_ctrl")
		, m_spriteram(*this, "spriteram")
		, m_sharedram(*this, "sharedram")
		, m_sndbank(*this, "sndbank")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_subcpu(*this, "subcpu")
		, m_msm(*this, "msm%u", 1U)
		, m_pc080sn(*this, "pc080sn_%u", 1U)
		, m_tc0040ioc(*this, "tc0040ioc")
		, m_filter1l(*this, "filter1l")
		, m_filter1r(*this, "filter1r")
		, m_filter2(*this, "filter2")
		, m_filter3(*this, "filter3")
		, m_gfxdecode(*this, "gfxdecode")
		, m_gas(*this, "GAS")
		, m_brake(*this, "BRAKE")
		, m_steer(*this, "STEER")
		, m_msm_rom(*this, "adpcm_%u", 0U)
	{ }

	void topspeed(machine_config &config);

	ioport_value gas_pedal_r();
	ioport_value brake_pedal_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_shared_ptr<u16> m_spritemap;
	required_shared_ptr<u16> m_raster_ctrl;
	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr<u16> m_sharedram;
	required_memory_bank m_sndbank;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device_array<msm5205_device, 2> m_msm;
	required_device_array<pc080sn_device, 2> m_pc080sn;
	required_device<tc0040ioc_device> m_tc0040ioc;
	required_device<filter_volume_device> m_filter1l;
	required_device<filter_volume_device> m_filter1r;
	required_device<filter_volume_device> m_filter2;
	required_device<filter_volume_device> m_filter3;
	required_device<gfxdecode_device> m_gfxdecode;
	required_ioport m_gas;
	required_ioport m_brake;
	required_ioport m_steer;

	// Misc
	u16  m_cpua_ctrl = 0;
	s32  m_ioc220_port = 0;

	// ADPCM
	required_region_ptr_array<u8, 2> m_msm_rom;
	u16  m_msm_pos[2]{};
	u8   m_msm_reset[2]{};
	u8   m_msm_nibble[2]{};
	u8   m_msm2_vck = 0;
	u8   m_msm2_vck2 = 0;

#ifdef MAME_DEBUG
	u8   m_dislayer[5] = { 0, 0, 0, 0, 0 };
#endif

	void msm5205_update(int chip);

	void cpua_ctrl_w(u16 data);
	u8 input_bypass_r();
	u16 motor_r(offs_t offset);
	void motor_w(offs_t offset, u16 data);
	void coins_w(u8 data);

	void msm5205_command_w(offs_t offset, u8 data);
	void msm5205_1_vck(int state);
	void z80ctc_to0(int state);
	void volume_w(offs_t offset, u8 data);

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void cpua_map(address_map &map) ATTR_COLD;
	void cpub_map(address_map &map) ATTR_COLD;
	void z80_io(address_map &map) ATTR_COLD;
	void z80_prg(address_map &map) ATTR_COLD;
};


/****************************************************************************

                                     NOTES

Raster line color control
-------------------------

Used to make the road move. Each word controls one pixel row.

0x800000 - 0x1ff  raster color control for one road tilemap
0x800200 - 0x3ff  raster color control for the other

Road tile colors are (all?) in the range 0x100-103. Top road section
(tilemap at 0xa08000) uses 0x100 and 0x101. Bottom section
(tilemap at 0xb00000) uses 0x102 and 0x103. This would allow colors
on left and right side of road to be different. In practice it seems
Taito didn't take advantage of this.

Each tilemap is usually all one color value. Every now and then (10s
or so) the value alternates. This seems to be determined by whether
the current section of road has white lines in the middle. (0x101/3
gives white lines.)

The raster line color control area has groups of four values which
cascade down through it so the road colors cascade down the screen.

There are three known groups (start is arbitrary; the cycles repeat ad
infinitum or until a different cycle starts; values given are from bottom
to top of screen):

(i) White lines in center of road

12  %10010
1f  %11111
00  %00000
0d  %01101

(ii) No lines in center of road

08  %01000
0c  %01100
1a  %11010
1e  %11110

(iii) Under bridge or in tunnel [note almost identical to (i)]

ffe0    %00000
ffed    %01101
fff2    %10010
ffef    %01111

(iv) Unknown 4th group for tunnels in later parts of the game that have
no white lines, analogous to (ii) ?


Correlating with screenshots suggests that these bits refer to:

x....  road body ?
.x...  lines in road center and inner edge
..x..  lines at road outer edge
...x.  outside road ?
....x  ???


Actual gfx tiles used for the road only use colors 1-5. Palette offsets:

(0 = transparency)
1 = lines in road center
2 = road edge (inner)
3 = road edge (outer)
4 = road body
5 = outside road

Each palette block contains three possible sets of 5 colors. Entries 1-5
(standard), 6-10 (alternate), 11-15 (tunnels).

In tunnels only 11-15 are used. Outside tunnels there is a choice between
the standard colors and the alternate colors. The road body could in theory
take a standard color while 'outside the road' took on an alternate. But
in practice the game is using a very limited choice of raster control words,
so we don't know.

Need to test whether sections of the road with unknown raster control words
(tunnels late in the game without central white lines) are correct against
a real machine.

Also are the 'prelines' shortly before white road lines appear correct?



CHECK screen inits at $1692

These suggest that rowscroll areas are all 0x1000 long and there are TWO
for each tilemap layer.

256 rows => 256 words => 0x200 bytes. So probably the inits are far too long.

Maybe the second area for each layer contains colscroll ?

****************************************************************************/


/****************************************************************************

                                     SPRITES

    Layout 8 bytes per sprite
    -------------------------

    +0x00   xxxxxxx. ........   Zoom Y
            .......x xxxxxxxx   Y

    +0x02   x....... ........   Flip Y
            ........ .xxxxxxx   Zoom X

    +0x04   x....... ........   Priority
            .x...... ........   Flip X
            ..x..... ........   Unknown
            .......x xxxxxxxx   X

    +0x06   xxxxxxxx ........   Color
            ........ xxxxxxxx   Tile number

********************************************************************************/

void topspeed_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const u32 primasks[2] = { 0xff00, 0xfffc };  // Sprites are over bottom layer or under top layer

	// Most of spriteram is not used by the 68000: rest is scratch space for the h/w perhaps ?
	for (int offs = 0; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		const u16 data = m_spriteram[offs + 2];

		const int tilenum = m_spriteram[offs + 3] & 0xff;
		const u32 color = (m_spriteram[offs + 3] & 0xff00) >> 8;
		const int flipx = (data & 0x4000) >> 14;
		const int flipy = (m_spriteram[offs + 1] & 0x8000) >> 15;
		int x = data & 0x1ff;
		int y = m_spriteram[offs] & 0x1ff;
		int zoomx = (m_spriteram[offs + 1] & 0x7f);
		int zoomy = (m_spriteram[offs] & 0xfe00) >> 9;
		const int priority = (data & 0x8000) >> 15;
//      unknown = (data & 0x2000) >> 13;

		// End of sprite list
		if (y == 0x180)
			break;

		const int map_offset = tilenum << 7;

		zoomx += 1;
		zoomy += 1;

		y += 3 + (128-zoomy);

		// treat coords as signed
		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;

		int bad_chunks = 0;

		for (int sprite_chunk = 0; sprite_chunk < 128; sprite_chunk++)
		{
			const int k = sprite_chunk % 8;   // 8 sprite chunks per row
			const int j = sprite_chunk / 8;   // 16 rows

			// pick tiles back to front for x and y flips
			const int px = (flipx) ?  (7 - k) : (k);
			const int py = (flipy) ? (15 - j) : (j);

			const u16 code = m_spritemap[map_offset + (py << 3) + px];

			if (code & 0x8000)
			{
				bad_chunks += 1;
				continue;
			}

			const int curx = x + ((k * zoomx) / 8);
			const int cury = y + ((j * zoomy) / 16);

			const int zx = x + (((k + 1) * zoomx) / 8) - curx;
			const int zy = y + (((j + 1) * zoomy) / 16) - cury;

			m_gfxdecode->gfx(0)->prio_zoom_transpen(bitmap, cliprect,
					code,
					color,
					flipx, flipy,
					curx, cury,
					zx << 12, zy << 13,
					screen.priority(), primasks[priority], 0);
		}

		if (bad_chunks)
			logerror("Sprite number %04x had %02x invalid chunks\n", tilenum, bad_chunks);
	}
}


/***************************************************************************/

u32 topspeed_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[4];

#ifdef MAME_DEBUG
	if (machine().input().code_pressed_once (KEYCODE_V))
	{
		m_dislayer[0] ^= 1;
		popmessage("bg: %01x", m_dislayer[0]);
	}

	if (machine().input().code_pressed_once (KEYCODE_B))
	{
		m_dislayer[1] ^= 1;
		popmessage("fg: %01x", m_dislayer[1]);
	}

	if (machine().input().code_pressed_once (KEYCODE_N))
	{
		m_dislayer[2] ^= 1;
		popmessage("bg2: %01x", m_dislayer[2]);
	}

	if (machine().input().code_pressed_once (KEYCODE_M))
	{
		m_dislayer[3] ^= 1;
		popmessage("fg2: %01x", m_dislayer[3]);
	}

	if (machine().input().code_pressed_once (KEYCODE_C))
	{
		m_dislayer[4] ^= 1;
		popmessage("sprites: %01x", m_dislayer[4]);
	}
#endif

	m_pc080sn[0]->tilemap_update();
	m_pc080sn[1]->tilemap_update();

	// Tilemap layer priority seems hardwired (the order is odd, too)
	layer[0] = 1;
	layer[1] = 0;
	layer[2] = 1;
	layer[3] = 0;

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

#ifdef MAME_DEBUG
	if (m_dislayer[3] == 0)
#endif
	m_pc080sn[1]->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);

#ifdef MAME_DEBUG
	if (m_dislayer[2] == 0)
#endif
	m_pc080sn[1]->tilemap_draw_special(screen, bitmap, cliprect, layer[1], 0, 2, m_raster_ctrl);

#ifdef MAME_DEBUG
	if (m_dislayer[1] == 0)
#endif
	m_pc080sn[0]->tilemap_draw_special(screen, bitmap, cliprect, layer[2], 0, 4, m_raster_ctrl + 0x100);

#ifdef MAME_DEBUG
	if (m_dislayer[0] == 0)
#endif
	m_pc080sn[0]->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 8);

#ifdef MAME_DEBUG
	if (m_dislayer[4] == 0)
#endif

	draw_sprites(screen, bitmap, cliprect);
	return 0;
}


/**********************************************************
                       CPU CONTROL
**********************************************************/

void topspeed_state::cpua_ctrl_w(u16 data)
{
	// Written only twice; once on startup at 0x00 then 0xc3 after init
	m_cpua_ctrl = data;
}


/**********************************************************
                       GAME INPUTS
**********************************************************/

u8 topspeed_state::input_bypass_r()
{
	// Read port number
	const u8 port = m_tc0040ioc->port_r();
	const u16 steer = 0xff80 + m_steer->read();

	switch (port)
	{
		case 0x0c:
			return steer & 0xff;

		case 0x0d:
			return steer >> 8;

		default:
			return m_tc0040ioc->portreg_r();
	}
}

ioport_value topspeed_state::gas_pedal_r()
{
	static constexpr u8 retval[8] = { 0, 1, 3, 2, 6, 7, 5, 4 };
	return retval[m_gas->read() & 7];
}

ioport_value topspeed_state::brake_pedal_r()
{
	static constexpr u8 retval[8] = { 0, 1, 3, 2, 6, 7, 5, 4 };
	return retval[m_brake->read() & 7];
}

// TODO: proper motorcpu hook-up

u16 topspeed_state::motor_r(offs_t offset)
{
	switch (offset)
	{
		case 0x0:   // Motor status?
			return (machine().rand() & 0xff);

		case 0x101: // Motor CPU status?
			return 0x55;

		case 0x141: // Left limit data
		case 0x142: // Right limit data
		case 0x143: // Horizontal center data

		case 0x144: // Upper limit data
		case 0x145: // Lower limit data
		case 0x146: // Vertical center data

		case 0x147: // Horizontal motor position
		case 0x148: // Vertical motor position
			return 0;

		default:
			logerror("CPU #0 PC %06x: warning - read from motor cpu %03x\n", m_subcpu->pc(), offset);
			return 0;
	}
}

void topspeed_state::motor_w(offs_t offset, u16 data)
{
	// Writes $900000-25 and $900200-219
	logerror("CPU #0 PC %06x: warning - write %04x to motor cpu %03x\n", m_subcpu->pc(), data, offset);
}

void topspeed_state::coins_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x08);
}


/*****************************************************
                        SOUND
*****************************************************/

void topspeed_state::msm5205_update(int chip)
{
	if (m_msm_reset[chip])
		return;

	const u8 data = m_msm_rom[chip][m_msm_pos[chip]];

	m_msm[chip]->data_w((m_msm_nibble[chip] ? data : data >> 4) & 0xf);

	if (m_msm_nibble[chip])
		++m_msm_pos[chip];

	m_msm_nibble[chip] ^= 1;
}

void topspeed_state::msm5205_1_vck(int state)
{
	msm5205_update(0);
}

void topspeed_state::msm5205_command_w(offs_t offset, u8 data)
{
	const int chip = (offset >> 12) & 1;

	switch (offset >> 8 & 0x2e)
	{
		// $b000 / $c000: Start address
		case 0x00:
			m_msm_pos[chip] = data << 8;
			break;

		// $b400 / $c400: Run
		case 0x04:
			m_msm_reset[chip] = 0;
			m_msm[chip]->reset_w(0);
			break;

		// $b800 / $c800: Stop
		case 0x08:
			m_msm_reset[chip] = 1;
			m_msm_nibble[chip] = 0;
			m_msm[chip]->reset_w(1);
			break;

		// $cc00: ? Chip 2 only
		case 0x0c:
			break;

		default:
			logerror("Unhandled MSM5205 control write to %x with %x (PC:%.4x)\n", 0xb000 + offset, data, m_audiocpu->pc());
			break;
	}
}

void topspeed_state::volume_w(offs_t offset, u8 data)
{
	// The volume is controlled by two Taito TC0060DCA hybrid volume modules
	filter_volume_device *filter = nullptr;

	switch (offset)
	{
		case 0x000: filter = m_filter2;     break; // MSM5205 1
		case 0x200: filter = m_filter3;     break; // MSM5205 2
		case 0x400: filter = m_filter1l;    break; // YM-2151 L
		case 0x600: filter = m_filter1r;    break; // YM-2151 R
	}

	filter->set_gain(data / 255.0f);
}

void topspeed_state::z80ctc_to0(int state)
{
	if (m_msm2_vck2 && !state)
	{
		// CTC output is divided by 2
		if (m_msm2_vck)
		{
			m_msm[1]->vclk_w(1);
		}
		else
		{
			// Update on falling edge of /VCK
			const u16 oldpos = m_msm_pos[1];

			msm5205_update(1);

			// Handle looping
			if ((oldpos >> 8) == 0x0f && ((m_msm_pos[1] >> 8) == 0x10))
			{
				m_msm_pos[1] = 0;
				m_msm[1]->reset_w(1);
				m_msm[1]->vclk_w(0);
				m_msm[1]->reset_w(0);
			}
			else
			{
				m_msm[1]->vclk_w(0);
			}
		}

		m_msm2_vck ^= 1;
	}
	m_msm2_vck2 = state;
}


/***********************************************************
                      MEMORY STRUCTURES
***********************************************************/

void topspeed_state::cpua_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x400000, 0x40ffff).ram().share(m_sharedram);
	map(0x500000, 0x503fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x600002, 0x600003).w(FUNC(topspeed_state::cpua_ctrl_w));
	map(0x7e0000, 0x7e0001).nopr();
	map(0x7e0001, 0x7e0001).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0x7e0003, 0x7e0003).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	map(0x800000, 0x8003ff).ram().share(m_raster_ctrl);
	map(0x800400, 0x80ffff).ram();
	map(0x880000, 0x880007).nopw(); // Lamps/outputs?
	map(0xa00000, 0xa0ffff).rw(m_pc080sn[0], FUNC(pc080sn_device::word_r), FUNC(pc080sn_device::word_w));
	map(0xa20000, 0xa20003).w(m_pc080sn[0], FUNC(pc080sn_device::yscroll_word_w));
	map(0xa40000, 0xa40003).w(m_pc080sn[0], FUNC(pc080sn_device::xscroll_word_w));
	map(0xa50000, 0xa50003).w(m_pc080sn[0], FUNC(pc080sn_device::ctrl_word_w));
	map(0xb00000, 0xb0ffff).rw(m_pc080sn[1], FUNC(pc080sn_device::word_r), FUNC(pc080sn_device::word_w));
	map(0xb20000, 0xb20003).w(m_pc080sn[1], FUNC(pc080sn_device::yscroll_word_w));
	map(0xb40000, 0xb40003).w(m_pc080sn[1], FUNC(pc080sn_device::xscroll_word_w));
	map(0xb50000, 0xb50003).w(m_pc080sn[1], FUNC(pc080sn_device::ctrl_word_w));
	map(0xd00000, 0xd00fff).ram().share(m_spriteram);
	map(0xe00000, 0xe0ffff).ram().share(m_spritemap);
}

void topspeed_state::cpub_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x400000, 0x40ffff).ram().share(m_sharedram);
	map(0x880001, 0x880001).r(FUNC(topspeed_state::input_bypass_r)).w(m_tc0040ioc, FUNC(tc0040ioc_device::portreg_w)).umask16(0x00ff);
	map(0x880003, 0x880003).rw(m_tc0040ioc, FUNC(tc0040ioc_device::port_r), FUNC(tc0040ioc_device::port_w));
	map(0x900000, 0x9003ff).rw(FUNC(topspeed_state::motor_r), FUNC(topspeed_state::motor_w));
}


/***************************************************************************/

void topspeed_state::z80_prg(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr(m_sndbank);
	map(0x8000, 0x8fff).ram();
	map(0x9000, 0x9001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xa000, 0xa000).w("ciu", FUNC(pc060ha_device::slave_port_w));
	map(0xa001, 0xa001).rw("ciu", FUNC(pc060ha_device::slave_comm_r), FUNC(pc060ha_device::slave_comm_w));
	map(0xb000, 0xcfff).w(FUNC(topspeed_state::msm5205_command_w));
	map(0xd000, 0xdfff).w(FUNC(topspeed_state::volume_w));
}

void topspeed_state::z80_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}


/***********************************************************
                    INPUT PORTS, DIPs
***********************************************************/

static INPUT_PORTS_START( topspeed )
	// 0x880000 (port 0) -> 0x400852 (-$77ae,A5) (shared RAM)
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(    0x03, "Deluxe" )    // analog pedals, racing wheel, motor (tilt disabled)
	PORT_DIPSETTING(    0x02, "Standard" )  // digital pedals, continuous wheel
	PORT_DIPSETTING(    0x01, "Standard" )
	PORT_DIPSETTING(    0x00, "Mini" )      // analog pedals, racing wheel
	TAITO_DSWA_BITS_2_TO_3_LOC(SWA)
	TAITO_COINAGE_WORLD_LOC(SWA)

	// 0x880000 (port 1) -> 0x400850 (-$77b0,A5) (shared RAM)
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, "Initial Time" )              PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x00, "40 seconds" )
	PORT_DIPSETTING(    0x04, "50 seconds" )
	PORT_DIPSETTING(    0x0c, "60 seconds" )
	PORT_DIPSETTING(    0x08, "70 seconds" )
	PORT_DIPNAME( 0x30, 0x30, "Nitros" )                    PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Continue_Price ) )   PORT_DIPLOCATION("SWB:8") // "KEEP OFF" in manual, see notes
	PORT_DIPSETTING(    0x80, "Same as Start" )
	PORT_DIPSETTING(    0x00, "Half of Start" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(topspeed_state, brake_pedal_r) PORT_CONDITION("DSWA", 0x03, NOTEQUALS, 0x02)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_NAME("Brake Switch") PORT_CONDITION("DSWA", 0x03, EQUALS, 0x02)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_NAME("Nitro")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE2 ) PORT_NAME("Calibrate") // ?
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON4 ) PORT_NAME("Shifter") PORT_TOGGLE
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(topspeed_state, gas_pedal_r) PORT_CONDITION("DSWA", 0x03, NOTEQUALS, 0x02)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_NAME("Gas Switch") PORT_CONDITION("DSWA", 0x03, EQUALS, 0x02)

	PORT_START("IN2")   // Unused

	PORT_START("STEER")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_NAME("Steering Wheel") PORT_CONDITION("DSWA", 0x03, NOTEQUALS, 0x02)    // racing wheel (absolute)
	PORT_BIT( 0xffff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(2)  PORT_NAME("Steering Wheel") PORT_CONDITION("DSWA", 0x03, EQUALS,    0x02)    // continuous (relative)

	PORT_START("GAS")
	PORT_BIT( 0x07, 0x00, IPT_PEDAL )  PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_NAME("Gas Pedal") PORT_CONDITION("DSWA", 0x03, NOTEQUALS, 0x02)

	PORT_START("BRAKE")
	PORT_BIT( 0x07, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_NAME("Brake Pedal") PORT_CONDITION("DSWA", 0x03, NOTEQUALS, 0x02)
INPUT_PORTS_END

static INPUT_PORTS_START( fullthrl )
	PORT_INCLUDE(topspeed)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)
INPUT_PORTS_END


/**************************************************************
                        GFX DECODING
**************************************************************/

static const gfx_layout tile16x8_layout =
{
	16,8,   // 16*8 sprites
	RGN_FRAC(1,1),
	4,      // 4 bits per pixel
	{ STEP4(0,16) },
	{ STEP16(0,1) },
	{ STEP8(0,16*4) },
	16*8*4    // every sprite takes 64 consecutive bytes
};

static GFXDECODE_START( gfx_topspeed )
	GFXDECODE_ENTRY( "sprites", 0x0, tile16x8_layout,      0, 256 ) // Sprite parts
	// Road Lines gfxdecodable ?
GFXDECODE_END

static GFXDECODE_START( gfx_topspeed_tmap )
	GFXDECODE_ENTRY( "pc080sn", 0x0, gfx_8x8x4_packed_msb, 0, 512 ) // Playfield
GFXDECODE_END


/***********************************************************
                     MACHINE DRIVERS
***********************************************************/

void topspeed_state::machine_start()
{
	m_sndbank->configure_entry(0, memregion("audiocpu")->base() + 0x10000);
	m_sndbank->configure_entries(1, 3, memregion("audiocpu")->base() + 0x4000, 0x4000);

	save_item(NAME(m_cpua_ctrl));
	save_item(NAME(m_ioc220_port));
	save_item(NAME(m_msm_pos));
	save_item(NAME(m_msm_reset));
	save_item(NAME(m_msm_nibble));
	save_item(NAME(m_msm2_vck));
	save_item(NAME(m_msm2_vck2));
}

void topspeed_state::machine_reset()
{
	m_cpua_ctrl = 0;
	m_ioc220_port = 0;

	m_msm_reset[0] = 0;
	m_msm_reset[1] = 0;
	m_msm[0]->reset_w(1);
	m_msm[1]->reset_w(1);
	m_msm2_vck = 0;
	m_msm2_vck2 = 0;
}


void topspeed_state::topspeed(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(16'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &topspeed_state::cpua_map);
	m_maincpu->set_vblank_int("screen", FUNC(topspeed_state::irq6_line_hold));

	M68000(config, m_subcpu, XTAL(16'000'000) / 2);
	m_subcpu->set_addrmap(AS_PROGRAM, &topspeed_state::cpub_map);
	m_subcpu->set_vblank_int("screen", FUNC(topspeed_state::irq5_line_hold));

	Z80(config, m_audiocpu, XTAL(16'000'000) / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &topspeed_state::z80_prg);
	m_audiocpu->set_addrmap(AS_IO, &topspeed_state::z80_io);

	z80ctc_device& ctc(Z80CTC(config, "ctc", XTAL(16'000'000) / 4));
	ctc.zc_callback<0>().set(FUNC(topspeed_state::z80ctc_to0));

	PC080SN(config, m_pc080sn[0], 0, "palette", gfx_topspeed_tmap);
	m_pc080sn[0]->set_offsets(0, 8);

	PC080SN(config, m_pc080sn[1], 0, "palette", gfx_topspeed_tmap);
	m_pc080sn[1]->set_offsets(0, 8);

	pc060ha_device &ciu(PC060HA(config, "ciu", 0));
	ciu.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	ciu.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);

	TC0040IOC(config, m_tc0040ioc, 0);
	m_tc0040ioc->read_0_callback().set_ioport("DSWA");
	m_tc0040ioc->read_1_callback().set_ioport("DSWB");
	m_tc0040ioc->read_2_callback().set_ioport("IN0");
	m_tc0040ioc->read_3_callback().set_ioport("IN1");
	m_tc0040ioc->write_4_callback().set(FUNC(topspeed_state::coins_w));
	m_tc0040ioc->read_7_callback().set_ioport("IN2");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60.0532); // Measured on real hardware
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 2*8, 32*8-1);
	screen.set_screen_update(FUNC(topspeed_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_topspeed);
	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 8192);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 16_MHz_XTAL / 4));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.port_write_handler().set_membank(m_sndbank).mask(0x03);
	ymsnd.add_route(0, "filter1l", 1.0);
	ymsnd.add_route(1, "filter1r", 1.0);

	MSM5205(config, m_msm[0], XTAL(384'000));
	m_msm[0]->vck_legacy_callback().set(FUNC(topspeed_state::msm5205_1_vck));
	m_msm[0]->set_prescaler_selector(msm5205_device::S48_4B);   // 8 kHz, 4-bit
	m_msm[0]->add_route(ALL_OUTPUTS, "filter2", 1.0);

	MSM5205(config, m_msm[1], XTAL(384'000));
	m_msm[1]->set_prescaler_selector(msm5205_device::SEX_4B);   // Slave mode, 4-bit
	m_msm[1]->add_route(ALL_OUTPUTS, "filter3", 1.0);

	FILTER_VOLUME(config, "filter1l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "filter1r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	FILTER_VOLUME(config, "filter2").add_route(ALL_OUTPUTS, "lspeaker", 1.0).add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	FILTER_VOLUME(config, "filter3").add_route(ALL_OUTPUTS, "lspeaker", 1.0).add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}



/***************************************************************************
                                DRIVERS

Note: driver does NOT make use of the zoom sprite tables rom.
***************************************************************************/

ROM_START( topspeed )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 128K for 68000 code (CPU A)
	ROM_LOAD16_BYTE( "b14-67-1.9",  0x000000, 0x10000, CRC(23f17616) SHA1(653ab6537f2e5898a77060c82b776852ab1f2b51) )
	ROM_LOAD16_BYTE( "b14-68-1.11", 0x000001, 0x10000, CRC(835659d9) SHA1(e99967f795c3c6e14bad7a66315640ca5db43c72) )
	ROM_LOAD16_BYTE( "b14-54.24",   0x080000, 0x20000, CRC(172924d5) SHA1(4a963f2e816f4b1c5acc6d38e99a68d3baeee8c6) ) // 4 data ROMs
	ROM_LOAD16_BYTE( "b14-52.26",   0x080001, 0x20000, CRC(e1b5b2a1) SHA1(8e2b992dcd5dc2317594c0187a22767aa626edee) )
	ROM_LOAD16_BYTE( "b14-55.23",   0x0c0000, 0x20000, CRC(a1f15499) SHA1(72f99108713773782fc72aae5a3f6e9e2a1e347c) )
	ROM_LOAD16_BYTE( "b14-53.25",   0x0c0001, 0x20000, CRC(04a04f5f) SHA1(09c15c33967bb141cc504b70d01c154bedb7fa33) )

	ROM_REGION( 0x20000, "subcpu", 0 ) // 128K for 68000 code (CPU B)
	ROM_LOAD16_BYTE( "b14-69.80",   0x000000, 0x10000, CRC(d652e300) SHA1(b559bdb564d96da4c656dc7b2c88dae84c4861ae) )
	ROM_LOAD16_BYTE( "b14-70.81",   0x000001, 0x10000, CRC(b720592b) SHA1(13298b498a198dcc1a56e533d106545dd77e1bbc) )

	ROM_REGION( 0x14000, "audiocpu", ROMREGION_ERASE00 ) // Z80 sound CPU
	ROM_LOAD( "b14-25.67",          0x000000, 0x10000, CRC(9eab28ef) SHA1(9a90f2c1881f4664d6d6241f3bc57faeaf150ffc) )

	ROM_REGION( 0x8000, "motorcpu", 0 )
	ROM_LOAD( "27c256.ic17",   0x0000, 0x8000, CRC(e52dfee1) SHA1(6e58e18eb2de3c899b950a4307ea21cd23683657) )

	ROM_REGION( 0x40000, "pc080sn", 0 ) // SCR tiles
	ROM_LOAD16_BYTE( "b14-06.52",   0x000000, 0x20000, CRC(b4e2536e) SHA1(c1960ee25b37b1444ec99082521c4858edcf3484) )
	ROM_LOAD16_BYTE( "b14-07.54",   0x000001, 0x20000, CRC(c6025fff) SHA1(439ed85b0160bfd6c06fd42990124a292b2e3c14) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_BYTE( "b14-48.16",   0x000007, 0x20000, CRC(30c7f265) SHA1(3e52e2aabf2c456d0b57d9414f99bd942bafc887) ) // OBJ, bitplane 3
	ROM_LOAD64_BYTE( "b14-49.12",   0x100007, 0x20000, CRC(32ba4265) SHA1(f468243d923726b7eff78d9bc55a3a092f211a24) )
	ROM_LOAD64_BYTE( "b14-50.8",    0x000006, 0x20000, CRC(ec1ef311) SHA1(4cfa06aec9535f2044b763b071f73d23ca8ba354) )
	ROM_LOAD64_BYTE( "b14-51.4",    0x100006, 0x20000, CRC(35041c5f) SHA1(71602267736396516366a8abf535db82acaa1c23) )

	ROM_LOAD64_BYTE( "b14-44.15",   0x000005, 0x20000, CRC(9f6c030e) SHA1(bb278fdcc29530685aa2e76da0712195f6ab0f5f) ) // OBJ, bitplane 2
	ROM_LOAD64_BYTE( "b14-45.11",   0x100005, 0x20000, CRC(63e4ce03) SHA1(92e3f45754676dd15691e48c0d37490c1a3ec328) )
	ROM_LOAD64_BYTE( "b14-46.7",    0x000004, 0x20000, CRC(d489adf2) SHA1(9f77916594d5ed05b79d7e8d8f534eb39f65edae) )
	ROM_LOAD64_BYTE( "b14-47.3",    0x100004, 0x20000, CRC(b3a1f75b) SHA1(050dd3313b5392d131c5a62c544260b83af0b8ab) )

	ROM_LOAD64_BYTE( "b14-40.14",   0x000003, 0x20000, CRC(fa2a3cb3) SHA1(1e102ae6e916fda046a154b89056a18b724d51a3) ) // OBJ, bitplane 1
	ROM_LOAD64_BYTE( "b14-41.10",   0x100003, 0x20000, CRC(09455a14) SHA1(dc703e1f9c4f16e330796e9945799e1038ce503b) )
	ROM_LOAD64_BYTE( "b14-42.6",    0x000002, 0x20000, CRC(ab51f53c) SHA1(0ed9a2e607b0bd2b43b47e3ed29b00a8d8a09f25) )
	ROM_LOAD64_BYTE( "b14-43.2",    0x100002, 0x20000, CRC(1e6d2b38) SHA1(453cd818a6cd8b238c72cc880c811227609767b8) )

	ROM_LOAD64_BYTE( "b14-36.13",   0x000001, 0x20000, CRC(20a7c1b8) SHA1(053c6b733a5c33b9259dfc754ce30a880905bb11) ) // OBJ, bitplane 0
	ROM_LOAD64_BYTE( "b14-37.9",    0x100001, 0x20000, CRC(801b703b) SHA1(dfbe276bd484815a7e69589eb56d54bc6e12e301) )
	ROM_LOAD64_BYTE( "b14-38.5",    0x000000, 0x20000, CRC(de0c213e) SHA1(1313b2051e906d22edb55f4d45d3a424b31ca2a2) )
	ROM_LOAD64_BYTE( "b14-39.1",    0x100000, 0x20000, CRC(798c28c5) SHA1(d2a8b9f84b3760f3800c5760ecee7ddcbafa6d6e) )

	ROM_REGION( 0x10000, "user1", 0 ) // Zoom tables for zoom sprite h/w
	ROM_LOAD( "b14-30.88",          0x000000, 0x10000, CRC(dccb0c7f) SHA1(42f0af72f559133b74912a4478e1323062be4b77) )

	ROM_REGION( 0x2000, "user2", 0 ) // Unknown (Sharp LH763J-70 64kx8 OTP ROM)
	ROM_LOAD( "b14-31.90",          0x000000, 0x02000, CRC(5c6b013d) SHA1(6d02d4560076213b6fb6fe856143bb533090603e) )

	ROM_REGION( 0x10000, "adpcm_0", 0 ) // ADPCM samples
	ROM_LOAD( "b14-28.103",         0x000000, 0x10000, CRC(df11d0ae) SHA1(259e1e6cc7ab100bfdb60e3d7a6bb46acb6fe2ea) )

	ROM_REGION( 0x10000, "adpcm_1", 0 ) // ADPCM samples
	ROM_LOAD( "b14-29.109",         0x000000, 0x10000, CRC(7ad983e7) SHA1(a3515caf93d6dab86de06ee52d6a13a456507dbe) )
ROM_END

ROM_START( topspeedu )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 128K for 68000 code (CPU A)
	ROM_LOAD16_BYTE     ( "b14-23", 0x00000, 0x10000, CRC(dd0307fd) SHA1(63218a707c78b3c785d1741dabdc511a76f12af1) )
	ROM_LOAD16_BYTE     ( "b14-24", 0x00001, 0x10000, CRC(acdf08d4) SHA1(506d48d27fc26684a3f884919665cf65a1b3062f) )
	ROM_LOAD16_WORD_SWAP( "b14-05", 0x80000, 0x80000, CRC(6557e9d8) SHA1(ff528b27fcaef5c181f5f3a56d6a41b935cf07e1) ) // Data ROM

	ROM_REGION( 0x20000, "subcpu", 0 ) // 128K for 68000 code (CPU B)
	ROM_LOAD16_BYTE( "b14-26",      0x00000, 0x10000, CRC(659dc872) SHA1(0a168122fe6324510c830e21a56eace9c8a2c189) )
	ROM_LOAD16_BYTE( "b14-56",      0x00001, 0x10000, CRC(d165cf1b) SHA1(bfbb8699c5671d3841d4057678ef4085c1927684) )

	ROM_REGION( 0x14000, "audiocpu", ROMREGION_ERASE00 ) // Z80 sound CPU
	ROM_LOAD( "b14-25.67",          0x00000, 0x10000, CRC(9eab28ef) SHA1(9a90f2c1881f4664d6d6241f3bc57faeaf150ffc) )

	ROM_REGION( 0x8000, "motorcpu", 0 )
	ROM_LOAD( "27c256.ic17",   0x0000, 0x8000, CRC(e52dfee1) SHA1(6e58e18eb2de3c899b950a4307ea21cd23683657) )

	ROM_REGION( 0x40000, "pc080sn", 0 ) // SCR tiles
	ROM_LOAD16_BYTE( "b14-06.52",   0x00000, 0x20000, CRC(b4e2536e) SHA1(c1960ee25b37b1444ec99082521c4858edcf3484) )
	ROM_LOAD16_BYTE( "b14-07.54",   0x00001, 0x20000, CRC(c6025fff) SHA1(439ed85b0160bfd6c06fd42990124a292b2e3c14) )

	ROM_REGION( 0x200000, "sprites", 0 ) // OBJ: each rom has 1 bitplane, forming 16x8 tiles
	ROM_LOAD64_WORD_SWAP( "b14-01", 0x00000, 0x80000, CRC(84a56f37) SHA1(926bcae5bd75a4172de2a2078718b2940c5c1966) )
	ROM_LOAD64_WORD_SWAP( "b14-02", 0x00002, 0x80000, CRC(6889186b) SHA1(3c38e281e8bf416a401c76ebb2d8ca95d09974b6) )
	ROM_LOAD64_WORD_SWAP( "b14-03", 0x00004, 0x80000, CRC(d1ed9e71) SHA1(26a6b2ca5bf6d70ad87f5c40c8e94ec542a2ec04) )
	ROM_LOAD64_WORD_SWAP( "b14-04", 0x00006, 0x80000, CRC(b63f0519) SHA1(e9a6b49effba0cae1ae3536a8584d3efa34ca8c3) )

	ROM_REGION( 0x10000, "user1", 0 ) // Zoom tables for zoom sprite h/w
	ROM_LOAD( "b14-30.88",          0x00000, 0x10000, CRC(dccb0c7f) SHA1(42f0af72f559133b74912a4478e1323062be4b77) )

	ROM_REGION( 0x2000, "user2", 0 ) // Unknown (Sharp LH763J-70 64kx8 OTP ROM)
	ROM_LOAD( "b14-31.90",          0x00000, 0x02000, CRC(5c6b013d) SHA1(6d02d4560076213b6fb6fe856143bb533090603e) )

	ROM_REGION( 0x10000, "adpcm_0", 0 ) // ADPCM samples
	ROM_LOAD( "b14-28.103",         0x00000, 0x10000, CRC(df11d0ae) SHA1(259e1e6cc7ab100bfdb60e3d7a6bb46acb6fe2ea) )

	ROM_REGION( 0x10000, "adpcm_1", 0 ) // ADPCM samples
	ROM_LOAD( "b14-29.109",         0x00000, 0x10000, CRC(7ad983e7) SHA1(a3515caf93d6dab86de06ee52d6a13a456507dbe) )
ROM_END

ROM_START( fullthrl )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 128K for 68000 code (CPU A) - Later revision?
	ROM_LOAD16_BYTE     ( "b14-67", 0x00000, 0x10000, CRC(284c943f) SHA1(e4720b138052d9cbf1290aeca8f9dd7fe2cffcc5) )
	ROM_LOAD16_BYTE     ( "b14-68", 0x00001, 0x10000, CRC(54cf6196) SHA1(0e86a7bf7d43526222160f4cd09f8d29fa9abdc4) )
	ROM_LOAD16_WORD_SWAP( "b14-05", 0x80000, 0x80000, CRC(6557e9d8) SHA1(ff528b27fcaef5c181f5f3a56d6a41b935cf07e1) ) // Data ROM

	ROM_REGION( 0x20000, "subcpu", 0 ) // 128K for 68000 code (CPU B)
	ROM_LOAD16_BYTE( "b14-69.80",   0x00000, 0x10000, CRC(d652e300) SHA1(b559bdb564d96da4c656dc7b2c88dae84c4861ae) )
	ROM_LOAD16_BYTE( "b14-71",      0x00001, 0x10000, CRC(f7081727) SHA1(f0ab6ce9975dd7a1fadd439fd3dfd2f1bf88796c) )

	ROM_REGION( 0x14000, "audiocpu", ROMREGION_ERASE00 ) // Z80 sound CPU
	ROM_LOAD( "b14-25.67",          0x00000, 0x10000, CRC(9eab28ef) SHA1(9a90f2c1881f4664d6d6241f3bc57faeaf150ffc) )

	ROM_REGION( 0x8000, "motorcpu", 0 )
	ROM_LOAD( "27c256.ic17",   0x0000, 0x8000, CRC(e52dfee1) SHA1(6e58e18eb2de3c899b950a4307ea21cd23683657) )

	ROM_REGION( 0x40000, "pc080sn", 0 ) // SCR tiles
	ROM_LOAD16_BYTE( "b14-06.52",   0x00000, 0x20000, CRC(b4e2536e) SHA1(c1960ee25b37b1444ec99082521c4858edcf3484) )
	ROM_LOAD16_BYTE( "b14-07.54",   0x00001, 0x20000, CRC(c6025fff) SHA1(439ed85b0160bfd6c06fd42990124a292b2e3c14) )

	ROM_REGION( 0x200000, "sprites", 0 ) // OBJ: each rom has 1 bitplane, forming 16x8 tiles
	ROM_LOAD64_WORD_SWAP( "b14-01", 0x00000, 0x80000, CRC(84a56f37) SHA1(926bcae5bd75a4172de2a2078718b2940c5c1966) )
	ROM_LOAD64_WORD_SWAP( "b14-02", 0x00002, 0x80000, CRC(6889186b) SHA1(3c38e281e8bf416a401c76ebb2d8ca95d09974b6) )
	ROM_LOAD64_WORD_SWAP( "b14-03", 0x00004, 0x80000, CRC(d1ed9e71) SHA1(26a6b2ca5bf6d70ad87f5c40c8e94ec542a2ec04) )
	ROM_LOAD64_WORD_SWAP( "b14-04", 0x00006, 0x80000, CRC(b63f0519) SHA1(e9a6b49effba0cae1ae3536a8584d3efa34ca8c3) )

	ROM_REGION( 0x10000, "user1", 0 ) // Zoom tables for zoom sprite h/w
	ROM_LOAD( "b14-30.88",          0x00000, 0x10000, CRC(dccb0c7f) SHA1(42f0af72f559133b74912a4478e1323062be4b77) )

	ROM_REGION( 0x2000, "user2", 0 ) // Unknown (Sharp LH763J-70 64kx8 OTP ROM)
	ROM_LOAD( "b14-31.90",          0x00000, 0x02000, CRC(5c6b013d) SHA1(6d02d4560076213b6fb6fe856143bb533090603e) )

	ROM_REGION( 0x10000, "adpcm_0", 0 ) // ADPCM samples
	ROM_LOAD( "b14-28.103",         0x00000, 0x10000, CRC(df11d0ae) SHA1(259e1e6cc7ab100bfdb60e3d7a6bb46acb6fe2ea) )

	ROM_REGION( 0x10000, "adpcm_1", 0 ) // ADPCM samples
	ROM_LOAD( "b14-29.109",         0x00000, 0x10000, CRC(7ad983e7) SHA1(a3515caf93d6dab86de06ee52d6a13a456507dbe) )
ROM_END

} // anonymous namespace


GAMEL( 1987, topspeed,  0,        topspeed, topspeed, topspeed_state, empty_init, ROT0, "Taito Corporation Japan",                     "Top Speed (World, rev 1)", MACHINE_SUPPORTS_SAVE, layout_topspeed )
GAMEL( 1987, topspeedu, topspeed, topspeed, fullthrl, topspeed_state, empty_init, ROT0, "Taito America Corporation (Romstar license)", "Top Speed (US)",           MACHINE_SUPPORTS_SAVE, layout_topspeed )
GAMEL( 1987, fullthrl,  topspeed, topspeed, fullthrl, topspeed_state, empty_init, ROT0, "Taito Corporation",                           "Full Throttle (Japan)",    MACHINE_SUPPORTS_SAVE, layout_topspeed )
