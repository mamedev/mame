// license:BSD-3-Clause
// copyright-holders:David Graves
// thanks-to:Richard Bush
/*******************************************************************************

Taito Triple Screen Games
=========================

Ninja Warriors (c) 1987 Taito
Darius 2       (c) 1989 Taito

David Graves

(this is based on the F2 driver by Bryan McPhail, Brad Oliver, Andrew Prime,
Nicola Salmoria. Thanks to Richard Bush and the Raine team, whose open
source was very helpful in many areas particularly the sprites.)

                *****

The triple screen games operate on hardware with various similarities to
the Taito F2 system, as they share some custom ics e.g. the TC0100SCN.

According to Sixtoe: "The multi-monitor systems had 2 or 3 13" screens;
one in the middle facing the player, and the other 1 or 2 on either side
mounted below and facing directly up reflecting off special semi-reflecting
mirrors, with about 1" of the graphics being overlapped on each screen.
This was the only way to get uninterrupted screens and to be able to see
through both ways. Otherwise you`d have the monitors' edges visible.
You can tell if your arcade has been cheap (like one near me) when you
look at the screens and can see black triangles on the left or right, this
means they bought ordinary mirrors and you can't see through them the
wrong way, as the semi-reflecting mirrors were extremely expensive."

For each screen the games have 3 separate layers of graphics:- one
128x64 tiled scrolling background plane of 8x8 tiles, a similar
foreground plane, and a 128x32 text plane with character definitions
held in ram.

Writing to the first TC0100SCN "writes through" to the two subsidiary
chips so that all three have identical contents. The subsidiary ones are
only addressed individually during initial memory checks, I think. (?)

There is a single sprite plane which covers all 3 screens.
The sprites are 16x16 and are not zoomable.

Twin 68000 processors are used; both have access to sprite ram and
the tilemap areas, and they communicate via 64K of shared ram.

Sound is dealt with by a Z80 controlling a YM2610. Sound commands
are written to the Z80 by the 68000 (the same as in Taito F2 games).


Tilemaps
========

TC0100SCN has tilemaps twice as wide as usual. The two BG tilemaps take
up twice the usual space, $8000 bytes each. The text tilemap takes up
the usual space, because its height is halved.

The triple palette generator (one for each screen) is probably just a
result of the way the hardware works: the colors in each are the same.


Ninja Warriors
Taito, 1987

PCB Layout
----------

J1100136A
K1100313A
SOUND BOARD
|------------------------------|
|  Z80     16MHz        YM2610 |
|M      B31-37.11              |
|  TMM2063           TC0140SYT |
|  TC0060DCA                   |
|  TC0060DCA  YM3016-F         |
|  TL074 TL074 TL074           |
|                     B31-08.19|
|                     B31-09.18|
|  MB3735   MB3735    B31-10.17|
|                     B31-11.16|
|  VOL1     VOL2               |
|                        S     |
|------------------------------|
Notes:
      Z80        - clock 4.000MHz [16/4]
      YM2610     - clock 8.000MHz [16/2]
      MB3735     - Audio power AMP
      TC006DCA   - Taito custom ceramic DAC/filter module (SIL20)
      TC0140SYT  - Taito custom sound control IC (QFP120)
      YM3016F    - Yamaha stereo audio DAC (DOIC16)
      TL074      - Texas Instruments low noise quad J-Fet op-amp (DIP14)
      M          - 25 pin connector
      S          - 30 pin flat cable joining to CPU BOARD
      TMM2063    - 8kx8 SRAM (DIP28)
      B31-37     - TC541000 mask ROM (DIP32)
      Other B31* - 234000 mask ROM (DIP40)


K1100311B
J1100134B
CPU BOARD
|--------------------------------------------------------------|
|                     2018                              2063   |
|                    TC0070RGB   TC0110PCR    B31-02.29 43256  |
|     68000             2018        TC0100SCN B31-01.28 43256  |
|                           26.686MHz                   2063   |
|                B31-16.68  16MHZ                              |
|H                     2018                             2063   |
|  B31-36.97 B31-33.87  TC0070RGB  TC011PCR   B31-02.27 43256  |
|  B31-35.96 B31-32.86  2018       TC0100SCN  B31-01.26 43256  |
|  B31-34.95 B31-31.85  B31-15.62                       2063   |
|J    DIP28     DIP28                                          |
|   43256     43256                                            |
|                                                       2063   |
|                 2018            TC0100SCN  B31-02.24  43256  |
|     B31-17.82   TC0070RGB   TC0110PCR      B31-01.24  43256  |
|                     B31-14-1.54                       2063   |
|                                                              |
|                 2018                                  43256  |
|G                                                      43256  |
|                                43256     43256     B31-13.20 |
|                         3771   B31-45.35 B31-47.32    68000  |
|     PC050CM    TC0040IOC  DSWA B31-29.34 B31-27.31           |
|                           DSWB    DIP40             B31-12.1 |
|--------------------------------------------------------------|
Notes:
      43256 - 32kx8 SRAM (DIP28)
      2063  - 8kx8 SRAM (NDIP28)
      DIP28 - Empty socket(s)
      DIP40 - Socket for mounting 'ROM 5 BOARD'
      2018  - 2kx8 SRAM (NDIP24)
      H     - 12 pin connector for power input
      J     - 15 pin connector
      G     - 22-way edge connector
      B31-12 to B31-17 - PALs


K9100162A
J9100118A
ROM 5 BOARD
|-----------------|
|B31-38.3 B31-40.6|
|B31-39.2 B31-41.5|
|                 |
| DIP40     74F139|
|-----------------|


K1100312A
J1100135A
OBJECT BOARD
|--------------------------------------------------------------|
|                                                              |
|                    B31-18.IC78                               |
|                                    TMM2064   TMM2064 TMM2064 |
|                    B31-19.IC80                               |
|                                    TMM2064                   |
|                                                              |
|                                                              |
|                             B31-20.IC119                     |
|                             B31-21.IC120                     |
|                             B31-22.IC121                     |
|                                                  B31-04.IC173|
|                                     B31-23.IC143 B31-05.IC174|
|                                     B31-24.IC144 B31-06.IC175|
|                                                  B31-07.IC176|
|                  MB81461 MB81461 MB81461 MB81461             |
|                  MB81461 MB81461 MB81461 MB81461             |
|                  MB81461 MB81461 MB81461 MB81461             |
|H                 MB81461 MB81461 MB81461 MB81461   TC0120SHT |
|                  MB81461 MB81461 MB81461 MB81461             |
|                  MB81461 MB81461 MB81461 MB81461             |
|                  MB81461 MB81461 MB81461 MB81461             |
|                  MB81461 MB81461 MB81461 MB81461             |
|--------------------------------------------------------------|
Notes:
      B31-04 to 06   - 234000 mask ROM (DIP40)
      B31-18 to 24   - PALs
      MB81461        - 64kx4 dual-port DRAM (ZIP24)
      TMM2064        - 8kx8 SRAM (DIP28)

Dumpers Notes
-------------

Ninja Warriors (JPN Ver.)
(c)1987 Taito

Sound Board
K1100313A
CPU     :Z80
Sound   :YM2610
OSC     :16000.00KHz
Other   :TC0140SYT,TC0060DCA x2
-----------------------
B31-08.19
B31-09.18
B31-10.17
B31-11.16
B31_37.11
-----------------------
CPU Board
M4300086A
K1100311A
CPU     :TS68000CP8 x2
Sound   :YM2610
OSC     :26686.00KHz,16000.00KHz
Other   :TC0040IOC,TC0070RGB x3,TC0110PCR x3,TC0100SCN x3
-----------------------
B31-01.23
B31-01.26
B31-01.28
B31-02.24
B31-02.27
B31-02.29
B31_27.31
B31_28.32
B31_29.34
B31_30.35
B31_31.85
B31_32.86
B31_33.87
B31_34.95
B31_35.96
B31_36.97
B31_38.3
B31_39.2
B31_40.6
B31_41.5
-----------------------
OBJECT Board
K1100312A
Other   :TC0120SHT
-----------------------
B31-04.173
B31-05.174
B31-06.175
B31-07.176
B31-25.38
B31-26.58


Stephh's notes (based on the game M68000 code and some tests) :

1) 'ninjaw*'

  - Region stored at 0x01fffe.w
  - Sets :
      * 'ninjaw'  : region = 0x0003
      * 'ninjawu' : region = 0x0004
      * 'ninjawj' : region = 0x0000
  - Coinage relies on the region (code at 0x0013bc) :
      * 0x0000 (Japan), 0x0001 (?) and 0x0002 (US) use TAITO_COINAGE_JAPAN_OLD
      * 0x0003 (World) and 0x0004 (licensed to xxx) use TAITO_COINAGE_WORLD
  - Notice screen only if region = 0x0000
  - According to the manual, DSWB bit 6 determines continue pricing :
                                                   ("Not Used" on Japanese manual)

    PORT_DIPNAME( 0x40, 0x00, DEF_STR( Continue_Price ) ) PORT_DIPLOCATION("SW2:7")
    PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0x00, "Same as Start" )

    However, many conditions are required to make it work due to code at 0x001404 :
      * region must not be 0x0000
      * coinage must be the same for both slots
      * coinage must be 2C_1C
    This is why this Dip Switch has NO effect in the sets we have :
      * 'ninjaw' : coinage is always different for the 2 slots
      * 'ninjawj' : region = 0x0000


2) 'darius2'

  - Region stored at 0x03fffe.w
  - Sets :
      * 'darius2' : region = 0x0001
  - Coinage relies on the region (code at 0x00f37a) :
      * 0x0000 (?), 0x0001 (Japan) and 0x0002 (US) use TAITO_COINAGE_JAPAN_OLD
      * 0x0002 (US, licensed to ROMSTAR) uses slighlty different TAITO_COINAGE_US :
        4C_3C instead of 4C_1C, same other settings otherwise
      * 0x0003 (World) uses TAITO_COINAGE_WORLD
  - Texts and game name rely on the region :
      * 0x0001 : some texts in Japanese - game name is "Darius II"
      * other : all texts in English - game name is "Sagaia"
  - Notice screen only if region = 0x0001
  - FBI logo only if region = 0x0002
  - Japan version resets score on continue, other versions don't


TODO
====

Verify 68000 clock rates. Unknown sprite bits.


Ninjaw
------

"Subwoofer" sound filtering isn't perfect.

Some enemies slide relative to the background when they should
be standing still. High cpu interleaving doesn't help much.


Darius 2
--------

"Subwoofer" sound filtering isn't perfect.

(When you lose a life or big enemies appear it's meant to create
rumbling on a subwoofer in the cabinet.)

*******************************************************************************/

#include "emu.h"

#include "taitoio.h"
#include "taitoipt.h"
#include "taitosnd.h"

#include "tc0100scn.h"
#include "tc0110pcr.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/flt_vol.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "ninjaw.lh"


namespace {

class ninjaw_state : public driver_device
{
public:
	ninjaw_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_tc0140syt(*this, "tc0140syt"),
		m_tc0100scn(*this, "tc0100scn_%u", 1),
		m_tc0110pcr(*this, "tc0110pcr_%u", 1),
		m_2610_l(*this, "2610.%u.l", 1),
		m_2610_r(*this, "2610.%u.r", 1),
		m_gfxdecode(*this, "gfxdecode_%u", 1),
		m_spriteram(*this, "spriteram"),
		m_z80bank(*this, "z80bank")
	{ }

	void darius2(machine_config &config);
	void ninjaw(machine_config &config);

protected:
	virtual void device_post_load() override;
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<tc0140syt_device> m_tc0140syt;
	required_device_array<tc0100scn_device, 3> m_tc0100scn;
	required_device_array<tc0110pcr_device, 3> m_tc0110pcr;
	required_device_array<filter_volume_device, 2> m_2610_l;
	required_device_array<filter_volume_device, 2> m_2610_r;
	required_device_array<gfxdecode_device, 3> m_gfxdecode;

	/* memory pointers */
	required_shared_ptr<u16> m_spriteram;

	/* memory regions */
	required_memory_bank m_z80bank;

	/* misc */
	u16 m_cpua_ctrl = 0;
	int m_pandata[4]{};

	void coin_control_w(u8 data);
	void cpua_ctrl_w(u16 data);
	void sound_bankswitch_w(u8 data);
	void pancontrol_w(offs_t offset, u8 data);
	void tc0100scn_triple_screen_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u32 screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) { return update_screen(screen, bitmap, cliprect, 36 * 8, 0); }
	u32 screen_update_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) { return update_screen(screen, bitmap, cliprect, 36 * 8, 1); }
	u32 screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) { return update_screen(screen, bitmap, cliprect, 36 * 8, 2); }
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int x_offs, int y_offs, int chip);
	void parse_control();
	u32 update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs, int chip);
	void darius2_master_map(address_map &map);
	void darius2_slave_map(address_map &map);
	void ninjaw_master_map(address_map &map);
	void ninjaw_slave_map(address_map &map);
	void sound_map(address_map &map);
};


/*******************************************************************************
        SUBWOOFER (SOUND)
*******************************************************************************/
#if 0

class subwoofer_device : public device_t, public device_sound_interface
{
public:
	subwoofer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	~subwoofer_device() {}

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;
};

extern const device_type SUBWOOFER;

const device_type SUBWOOFER = device_creator<subwoofer_device>;

subwoofer_device::subwoofer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SUBWOOFER, "Subwoofer", tag, owner, clock),
	device_sound_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void subwoofer_device::device_start()
{
	/* Adjust the lowpass filter of the first three YM2610 channels */

	/* The 150 Hz is a common top frequency played by a generic */
	/* subwoofer, the real Arcade Machine may differ */

	mixer_set_lowpass_frequency(0, 20);
	mixer_set_lowpass_frequency(1, 20);
	mixer_set_lowpass_frequency(2, 20);

	return 0;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void subwoofer_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	outputs[0].fill(0);
}

#endif


/*******************************************************************************
        SPRITE DRAW ROUTINE
*******************************************************************************/

void ninjaw_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int x_offs, int y_offs, int chip)
{
#ifdef MAME_DEBUG
	int unknown = 0;
#endif

	static const u32 primask[2] =
	{
		GFX_PMASK_4, // draw sprites with priority 0 which are over the mid layer
		(GFX_PMASK_4 | GFX_PMASK_2) // draw sprites with priority 1 which are under the mid layer
	};

	for (int offs = 0; offs < (m_spriteram.bytes() / 2); offs += 4)
	{
		int data = m_spriteram[offs + 2];
		const u32 tilenum = data & 0x7fff;

		if (!tilenum)
			continue;

		data = m_spriteram[offs + 0];
		int x = (data - 32) & 0x3ff;    /* aligns sprites on rock outcrops and sewer hole */

		data = m_spriteram[offs + 1];
		int y = (data - 0) & 0x1ff;

		/*
		    The purpose of the bit at data&0x8 (below) is unknown, but it is set
		    on Darius explosions, some enemy missiles and at least 1 boss.
		    It is most likely another priority bit but as there are no obvious
		    visual problems it will need checked against the original pcb.

		    There is a report this bit is set when the player intersects
		    the tank sprite in Ninja Warriors however I was unable to repro
		    this or find any use of this bit in that game.

		    Bit&0x8000 is set on some sprites in later levels of Darius
		    but is again unknown, and there is no obvious visual problem.
		*/
		data = m_spriteram[offs + 3];
		const bool flipx = (data & 0x1);
		const bool flipy = (data & 0x2) >> 1;
		const int priority = (data & 0x4) >> 2; // 1 = low
		/* data&0x8 - unknown */
		const u32 color = (data & 0x7f00) >> 8;
		/* data&0x8000 - unknown */

#ifdef MAME_DEBUG
		if (data & 0x80f0) unknown |= (data &0x80f0);
#endif

		x -= x_offs;
		y += y_offs;

		/* sprite wrap: coords become negative at high values */
		if (x > 0x3c0) x -= 0x400;
		if (y > 0x180) y -= 0x200;

		const int curx = x;
		const int cury = y;
		const u32 code = tilenum;

		m_gfxdecode[chip]->gfx(0)->prio_transpen(bitmap,cliprect,
				code, color,
				flipx, flipy,
				curx, cury,
				screen.priority(), primask[priority],
				0);
	}

#ifdef MAME_DEBUG
	if (unknown)
		popmessage("unknown sprite bits: %04x",unknown);
#endif
}


/*******************************************************************************
        SCREEN REFRESH
*******************************************************************************/

u32 ninjaw_state::update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs, int chip)
{
	tc0100scn_device *tc0100scn = m_tc0100scn[chip];
	xoffs *= chip;
	u8 layer[3];

	tc0100scn->tilemap_update();

	layer[0] = m_tc0100scn[0]->bottomlayer();
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	screen.priority().fill(0, cliprect);
	/* chip 0 does tilemaps on the left, chip 1 center, chip 2 the right */
	// draw bottom layer
	u8 nodraw = tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);    /* left */

	/* Ensure screen blanked even when bottom layers not drawn due to disable bit */
	if (nodraw)
		bitmap.fill(m_tc0110pcr[chip]->black_pen(), cliprect);

	// draw middle layer
	tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);

	// draw top(text) layer
	tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	/* Sprites can be under/over the layer below text layer */
	draw_sprites(screen, bitmap, cliprect, xoffs, 8, chip);

	return 0;
}


/*******************************************************************************
        MISC. CONTROL
*******************************************************************************/

void ninjaw_state::parse_control()   /* assumes Z80 sandwiched between 68Ks */
{
	/* bit 0 enables cpu B */
	/* however this fails when recovering from a save state
	   if cpu B is disabled !! */
	m_subcpu->set_input_line(INPUT_LINE_RESET, (m_cpua_ctrl & 0x1) ? CLEAR_LINE : ASSERT_LINE);

}

void ninjaw_state::cpua_ctrl_w(u16 data)
{
	if ((data &0xff00) && ((data &0xff) == 0))
		data = data >> 8;
	m_cpua_ctrl = data;

	parse_control();

	logerror("CPU #0 PC %06x: write %04x to cpu control\n", m_maincpu->pc(), data);
}


void ninjaw_state::coin_control_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x08);
}


/*******************************************************************************
        SOUND
*******************************************************************************/

void ninjaw_state::sound_bankswitch_w(u8 data)
{
	m_z80bank->set_entry(data & 7);
}

/**** sound pan control ****/

void ninjaw_state::pancontrol_w(offs_t offset, u8 data)
{
	filter_volume_device *flt = nullptr;
	offset &= 3;
	offset ^= 1;

	switch (offset)
	{
		case 0: flt = m_2610_l[0]; break;
		case 1: flt = m_2610_r[0]; break;
		case 2: flt = m_2610_l[1]; break;
		case 3: flt = m_2610_r[1]; break;
	}

	m_pandata[offset] = (float)data * (100.f / 255.0f);
	//popmessage(" pan %02x %02x %02x %02x", m_pandata[0], m_pandata[1], m_pandata[2], m_pandata[3] );
	flt->set_gain(m_pandata[offset] / 100.0);
}


void ninjaw_state::tc0100scn_triple_screen_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_tc0100scn[0]->ram_w(offset, data, mem_mask);
	m_tc0100scn[1]->ram_w(offset, data, mem_mask);
	m_tc0100scn[2]->ram_w(offset, data, mem_mask);
}


/*******************************************************************************
        MEMORY STRUCTURES
*******************************************************************************/

void ninjaw_state::ninjaw_master_map(address_map &map)
{
	map(0x000000, 0x0bffff).rom();
	map(0x0c0000, 0x0cffff).ram();                                                     /* main ram */
	map(0x200000, 0x200003).rw("tc0040ioc", FUNC(tc0040ioc_device::read), FUNC(tc0040ioc_device::write)).umask16(0x00ff);
	map(0x210000, 0x210001).w(FUNC(ninjaw_state::cpua_ctrl_w));
	map(0x220001, 0x220001).w(m_tc0140syt, FUNC(tc0140syt_device::master_port_w));
	map(0x220003, 0x220003).rw(m_tc0140syt, FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x240000, 0x24ffff).ram().share("share1");
	map(0x260000, 0x263fff).ram().share("spriteram");
	map(0x280000, 0x293fff).r(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r)).w(FUNC(ninjaw_state::tc0100scn_triple_screen_w)); /* tilemaps (1st screen/all screens) */
	map(0x2a0000, 0x2a000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x2c0000, 0x2d3fff).rw(m_tc0100scn[1], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));      /* tilemaps (2nd screen) */
	map(0x2e0000, 0x2e000f).rw(m_tc0100scn[1], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x300000, 0x313fff).rw(m_tc0100scn[2], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));      /* tilemaps (3rd screen) */
	map(0x320000, 0x32000f).rw(m_tc0100scn[2], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x340000, 0x340007).rw(m_tc0110pcr[0], FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));        /* palette (1st screen) */
	map(0x350000, 0x350007).rw(m_tc0110pcr[1], FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));        /* palette (2nd screen) */
	map(0x360000, 0x360007).rw(m_tc0110pcr[2], FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));        /* palette (3rd screen) */
}

// NB there could be conflicts between which cpu writes what to the
// palette, as our interleaving won't match the original board.

void ninjaw_state::ninjaw_slave_map(address_map &map)
{
	map(0x000000, 0x05ffff).rom();
	map(0x080000, 0x08ffff).ram(); /* main ram */
	map(0x200000, 0x200003).rw("tc0040ioc", FUNC(tc0040ioc_device::read), FUNC(tc0040ioc_device::write)).umask16(0x00ff);
	map(0x240000, 0x24ffff).ram().share("share1");
	map(0x260000, 0x263fff).ram().share("spriteram");
	map(0x280000, 0x293fff).r(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r)).w(FUNC(ninjaw_state::tc0100scn_triple_screen_w)); /* tilemaps (1st screen/all screens) */
	map(0x340000, 0x340007).rw(m_tc0110pcr[0], FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));        /* palette (1st screen) */
	map(0x350000, 0x350007).rw(m_tc0110pcr[1], FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));        /* palette (2nd screen) */
	map(0x360000, 0x360007).rw(m_tc0110pcr[2], FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));        /* palette (3rd screen) */
}

void ninjaw_state::darius2_master_map(address_map &map)
{
	map(0x000000, 0x0bffff).rom();
	map(0x0c0000, 0x0cffff).ram();                         /* main ram */
	map(0x200000, 0x200003).rw("tc0040ioc", FUNC(tc0040ioc_device::read), FUNC(tc0040ioc_device::write)).umask16(0x00ff);
	map(0x210000, 0x210001).w(FUNC(ninjaw_state::cpua_ctrl_w));
	map(0x220001, 0x220001).w(m_tc0140syt, FUNC(tc0140syt_device::master_port_w));
	map(0x220003, 0x220003).rw(m_tc0140syt, FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x240000, 0x24ffff).ram().share("share1");
	map(0x260000, 0x263fff).ram().share("spriteram");
	map(0x280000, 0x293fff).r(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r)).w(FUNC(ninjaw_state::tc0100scn_triple_screen_w)); /* tilemaps (1st screen/all screens) */
	map(0x2a0000, 0x2a000f).rw(m_tc0100scn[0], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x2c0000, 0x2d3fff).rw(m_tc0100scn[1], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));      /* tilemaps (2nd screen) */
	map(0x2e0000, 0x2e000f).rw(m_tc0100scn[1], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x300000, 0x313fff).rw(m_tc0100scn[2], FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));      /* tilemaps (3rd screen) */
	map(0x320000, 0x32000f).rw(m_tc0100scn[2], FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x340000, 0x340007).rw(m_tc0110pcr[0], FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));        /* palette (1st screen) */
	map(0x350000, 0x350007).rw(m_tc0110pcr[1], FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));        /* palette (2nd screen) */
	map(0x360000, 0x360007).rw(m_tc0110pcr[2], FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_word_w));        /* palette (3rd screen) */
}

void ninjaw_state::darius2_slave_map(address_map &map)
{
	map(0x000000, 0x05ffff).rom();
	map(0x080000, 0x08ffff).ram();                                                     /* main ram */
	map(0x200000, 0x200003).rw("tc0040ioc", FUNC(tc0040ioc_device::read), FUNC(tc0040ioc_device::write)).umask16(0x00ff);
	map(0x240000, 0x24ffff).ram().share("share1");
	map(0x260000, 0x263fff).ram().share("spriteram");
	map(0x280000, 0x293fff).r(m_tc0100scn[0], FUNC(tc0100scn_device::ram_r)).w(FUNC(ninjaw_state::tc0100scn_triple_screen_w)); /* tilemaps (1st screen/all screens) */
}


/******************************************************************************/

void ninjaw_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr("z80bank");
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe003).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0xe200, 0xe200).nopr().w(m_tc0140syt, FUNC(tc0140syt_device::slave_port_w));
	map(0xe201, 0xe201).rw(m_tc0140syt, FUNC(tc0140syt_device::slave_comm_r), FUNC(tc0140syt_device::slave_comm_w));
	map(0xe400, 0xe403).w(FUNC(ninjaw_state::pancontrol_w)); /* pan */
	map(0xea00, 0xea00).nopr();
	map(0xee00, 0xee00).nopw(); /* ? */
	map(0xf000, 0xf000).nopw(); /* ? */
	map(0xf200, 0xf200).w(FUNC(ninjaw_state::sound_bankswitch_w));
}


/*******************************************************************************
        INPUT PORTS, DIPs
*******************************************************************************/

static INPUT_PORTS_START( ninjaw )
	/* 0x200000 (port 0) -> 0x0c2291.b and 0x24122c (shared RAM) */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	TAITO_DSWA_BITS_2_TO_3_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	/* 0x200000 (port 1) -> 0x0c2290.b and 0x24122e (shared RAM) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )        /* Manual shows switches 3, 4, 5, 6 & 8 as not used */
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )        /* see notes */
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )                /* Stops working if this is high */
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Freeze") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN1")
	TAITO_JOY_DUAL_UDRL( 1, 2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END

/* Can't use PORT_INCLUDE because of PORT_DIPLOCATION */
static INPUT_PORTS_START( ninjawj )
	PORT_INCLUDE(ninjaw)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END

/* Can't use PORT_INCLUDE because of PORT_DIPLOCATION */
static INPUT_PORTS_START( darius2 )
	PORT_INCLUDE(ninjaw)

	/* 0x200000 (port 0) -> 0x0c2002 (-$5ffe,A5) and 0x0c2006 (-$5ffa,A5) */
	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "Difficulty Enhancement" ) PORT_DIPLOCATION("SW1:1")    /* code at 0x00c20e */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) ) // Easy  Medium  Hard  Hardest  // Japan factory default = "Off"
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )  // Easy- Medium+ Hard+ Hardest+ // "Easy-" is easier than "Easy". "Medium+","Hard+" and "hardest+" are harder than "Medium","Hard" and "hardest".
	PORT_DIPNAME( 0x02, 0x02, "Auto Fire" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Fast" )
	TAITO_DSWA_BITS_2_TO_3_LOC(SW1)
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)

	/* 0x200000 (port 1) -> 0x0c2004 (-$5ffc,A5) and 0x0c2008 (-$5ff8,A5) */
	PORT_MODIFY("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "every 700k" )
	PORT_DIPSETTING(    0x08, "every 800k" )
	PORT_DIPSETTING(    0x04, "every 900k" )
	PORT_DIPSETTING(    0x00, "every 1000k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END


/*******************************************************************************
        GFX DECODING (Thanks to Raine for the obj decoding)
*******************************************************************************/

static const gfx_layout tilelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ STEP4(0,4) },    /* pixel bits separated, jump 4 to get to next one */
	{ STEP4(3,-1), STEP4(4*4+3,-1), STEP4(4*4*2*8+3,-1), STEP4(4*4*2*8+4*4+3,-1) },
	{ STEP8(0,4*4*2), STEP8(4*4*2*8*2,4*4*2) },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( gfx_ninjaw )
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 0, 256 )   /* sprites */
GFXDECODE_END


/*******************************************************************************
        MACHINE DRIVERS
--------------------------------------------------------------------------------
Ninjaw: high interleaving of 100, but doesn't stop enemies
"sliding" when they should be standing still relative
to the scrolling background.

Darius2: arbitrary interleaving of 10 to keep cpus synced.
*******************************************************************************/

void ninjaw_state::device_post_load()
{
	parse_control();
}

void ninjaw_state::machine_start()
{
	m_z80bank->configure_entries(0, 8, memregion("audiocpu")->base(), 0x4000);

	save_item(NAME(m_cpua_ctrl));
	save_item(NAME(m_pandata));
}

void ninjaw_state::machine_reset()
{
	m_cpua_ctrl = 0xff;
	memset(m_pandata, 0, sizeof(m_pandata));

	/**** mixer control enable ****/
	machine().sound().system_mute(false);  /* mixer enabled */
}

void ninjaw_state::ninjaw(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16000000/2);  /* 8 MHz ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &ninjaw_state::ninjaw_master_map);
	m_maincpu->set_vblank_int("lscreen", FUNC(ninjaw_state::irq4_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", 16000000/4));    /* 16/4 MHz ? */
	audiocpu.set_addrmap(AS_PROGRAM, &ninjaw_state::sound_map);

	M68000(config, m_subcpu, 16000000/2);  /* 8 MHz ? */
	m_subcpu->set_addrmap(AS_PROGRAM, &ninjaw_state::ninjaw_slave_map);
	m_subcpu->set_vblank_int("lscreen", FUNC(ninjaw_state::irq4_line_hold));

	// TODO: if CPUs are unsynched then seldomly stages loads up with no enemies
	//       Let's use a better timer (was 6000 before) based off actual CPU timing.
	//       Might as well bump the divider in case the bug still occurs before resorting to perfect CPU.
	config.set_maximum_quantum(attotime::from_hz(16000000/1024));  /* CPU slices */
	//config.m_perfect_cpu_quantum = subtag("maincpu");

	tc0040ioc_device &tc0040ioc(TC0040IOC(config, "tc0040ioc", 0));
	tc0040ioc.read_0_callback().set_ioport("DSWA");
	tc0040ioc.read_1_callback().set_ioport("DSWB");
	tc0040ioc.read_2_callback().set_ioport("IN0");
	tc0040ioc.read_3_callback().set_ioport("IN1");
	tc0040ioc.write_4_callback().set(FUNC(ninjaw_state::coin_control_w));
	tc0040ioc.read_7_callback().set_ioport("IN2");

	/* video hardware */
	GFXDECODE(config, m_gfxdecode[0], m_tc0110pcr[0], gfx_ninjaw);
	GFXDECODE(config, m_gfxdecode[1], m_tc0110pcr[1], gfx_ninjaw);
	GFXDECODE(config, m_gfxdecode[2], m_tc0110pcr[2], gfx_ninjaw);

	config.set_default_layout(layout_ninjaw);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(60);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	lscreen.set_size(36*8, 32*8);
	lscreen.set_visarea(0*8, 36*8-1, 3*8, 31*8-1);
	lscreen.set_screen_update(FUNC(ninjaw_state::screen_update_left));
	lscreen.set_palette(m_tc0110pcr[0]);

	screen_device &mscreen(SCREEN(config, "mscreen", SCREEN_TYPE_RASTER));
	mscreen.set_refresh_hz(60);
	mscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	mscreen.set_size(36*8, 32*8);
	mscreen.set_visarea(0*8, 36*8-1, 3*8, 31*8-1);
	mscreen.set_screen_update(FUNC(ninjaw_state::screen_update_middle));
	mscreen.set_palette(m_tc0110pcr[1]);

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(60);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	rscreen.set_size(36*8, 32*8);
	rscreen.set_visarea(0*8, 36*8-1, 3*8, 31*8-1);
	rscreen.set_screen_update(FUNC(ninjaw_state::screen_update_right));
	rscreen.set_palette(m_tc0110pcr[2]);

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(22, 0);
	m_tc0100scn[0]->set_multiscr_xoffs(0);
	m_tc0100scn[0]->set_multiscr_hack(0);
	m_tc0100scn[0]->set_palette(m_tc0110pcr[0]);

	TC0110PCR(config, m_tc0110pcr[0], 0);

	TC0100SCN(config, m_tc0100scn[1], 0);
	m_tc0100scn[1]->set_offsets(22, 0);
	m_tc0100scn[1]->set_multiscr_xoffs(2);
	m_tc0100scn[1]->set_multiscr_hack(1);
	m_tc0100scn[1]->set_palette(m_tc0110pcr[1]);

	TC0110PCR(config, m_tc0110pcr[1], 0);

	TC0100SCN(config, m_tc0100scn[2], 0);
	m_tc0100scn[2]->set_offsets(22, 0);
	m_tc0100scn[2]->set_multiscr_xoffs(4);
	m_tc0100scn[2]->set_multiscr_hack(1);
	m_tc0100scn[2]->set_palette(m_tc0110pcr[2]);

	TC0110PCR(config, m_tc0110pcr[2], 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	SPEAKER(config, "subwoofer").seat();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 16000000/2));
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(0, "subwoofer", 0.25);
	ymsnd.add_route(1, "2610.1.l", 1.0);
	ymsnd.add_route(1, "2610.1.r", 1.0);
	ymsnd.add_route(2, "2610.2.l", 1.0);
	ymsnd.add_route(2, "2610.2.r", 1.0);

	FILTER_VOLUME(config, "2610.1.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "2610.1.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);

//  SUBWOOFER(config, "subwoofer", 0);

	TC0140SYT(config, m_tc0140syt, 0);
	m_tc0140syt->nmi_callback().set_inputline("audiocpu", INPUT_LINE_NMI);
	m_tc0140syt->reset_callback().set_inputline("audiocpu", INPUT_LINE_RESET);
}


void ninjaw_state::darius2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16000000/2);  /* 8 MHz ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &ninjaw_state::darius2_master_map);
	m_maincpu->set_vblank_int("lscreen", FUNC(ninjaw_state::irq4_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", 16000000/4));    /* 4 MHz ? */
	audiocpu.set_addrmap(AS_PROGRAM, &ninjaw_state::sound_map);

	M68000(config, m_subcpu, 16000000/2);  /* 8 MHz ? */
	m_subcpu->set_addrmap(AS_PROGRAM, &ninjaw_state::darius2_slave_map);
	m_subcpu->set_vblank_int("lscreen", FUNC(ninjaw_state::irq4_line_hold));

	config.set_maximum_quantum(attotime::from_hz(16000000/1024));  /* CPU slices */
	//config.m_perfect_cpu_quantum = subtag("maincpu");

	tc0040ioc_device &tc0040ioc(TC0040IOC(config, "tc0040ioc", 0));
	tc0040ioc.read_0_callback().set_ioport("DSWA");
	tc0040ioc.read_1_callback().set_ioport("DSWB");
	tc0040ioc.read_2_callback().set_ioport("IN0");
	tc0040ioc.read_3_callback().set_ioport("IN1");
	tc0040ioc.write_4_callback().set(FUNC(ninjaw_state::coin_control_w));
	tc0040ioc.read_7_callback().set_ioport("IN2");

	/* video hardware */
	GFXDECODE(config, m_gfxdecode[0], m_tc0110pcr[0], gfx_ninjaw);
	GFXDECODE(config, m_gfxdecode[1], m_tc0110pcr[1], gfx_ninjaw);
	GFXDECODE(config, m_gfxdecode[2], m_tc0110pcr[2], gfx_ninjaw);

	config.set_default_layout(layout_ninjaw);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(60);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	lscreen.set_size(36*8, 32*8);
	lscreen.set_visarea(0*8, 36*8-1, 3*8, 31*8-1);
	lscreen.set_screen_update(FUNC(ninjaw_state::screen_update_left));
	lscreen.set_palette(m_tc0110pcr[0]);

	screen_device &mscreen(SCREEN(config, "mscreen", SCREEN_TYPE_RASTER));
	mscreen.set_refresh_hz(60);
	mscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	mscreen.set_size(36*8, 32*8);
	mscreen.set_visarea(0*8, 36*8-1, 3*8, 31*8-1);
	mscreen.set_screen_update(FUNC(ninjaw_state::screen_update_middle));
	mscreen.set_palette(m_tc0110pcr[1]);

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(60);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	rscreen.set_size(36*8, 32*8);
	rscreen.set_visarea(0*8, 36*8-1, 3*8, 31*8-1);
	rscreen.set_screen_update(FUNC(ninjaw_state::screen_update_right));
	rscreen.set_palette(m_tc0110pcr[2]);

	TC0100SCN(config, m_tc0100scn[0], 0);
	m_tc0100scn[0]->set_offsets(22, 0);
	m_tc0100scn[0]->set_multiscr_xoffs(0);
	m_tc0100scn[0]->set_multiscr_hack(0);
	m_tc0100scn[0]->set_palette(m_tc0110pcr[0]);

	TC0110PCR(config, m_tc0110pcr[0], 0);

	TC0100SCN(config, m_tc0100scn[1], 0);
	m_tc0100scn[1]->set_offsets(22, 0);
	m_tc0100scn[1]->set_multiscr_xoffs(2);
	m_tc0100scn[1]->set_multiscr_hack(1);
	m_tc0100scn[1]->set_palette(m_tc0110pcr[1]);

	TC0110PCR(config, m_tc0110pcr[1], 0);

	TC0100SCN(config, m_tc0100scn[2], 0);
	m_tc0100scn[2]->set_offsets(22, 0);
	m_tc0100scn[2]->set_multiscr_xoffs(4);
	m_tc0100scn[2]->set_multiscr_hack(1);
	m_tc0100scn[2]->set_palette(m_tc0110pcr[2]);

	TC0110PCR(config, m_tc0110pcr[2], 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	SPEAKER(config, "subwoofer").seat();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 16000000/2));
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(0, "subwoofer", 0.25);
	ymsnd.add_route(1, "2610.1.l", 1.0);
	ymsnd.add_route(1, "2610.1.r", 1.0);
	ymsnd.add_route(2, "2610.2.l", 1.0);
	ymsnd.add_route(2, "2610.2.r", 1.0);

	FILTER_VOLUME(config, "2610.1.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "2610.1.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.l").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, "2610.2.r").add_route(ALL_OUTPUTS, "rspeaker", 1.0);

//  SUBWOOFER(config, "subwoofer", 0);

	TC0140SYT(config, m_tc0140syt, 0);
	m_tc0140syt->nmi_callback().set_inputline("audiocpu", INPUT_LINE_NMI);
	m_tc0140syt->reset_callback().set_inputline("audiocpu", INPUT_LINE_RESET);
}


/*******************************************************************************
        ROM DEFINITIONS
*******************************************************************************/

ROM_START( ninjaw )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 256K for 68000 CPUA code */
	ROM_LOAD16_BYTE( "b31_45.35", 0x00000, 0x10000, CRC(107902c3) SHA1(026f71a918059e3374ae262304a2ee1270f5c5bd) ) /* For these 2 roms:   Revised code base compared to the Japanese set below? */
	ROM_LOAD16_BYTE( "b31_47.32", 0x00001, 0x10000, CRC(bd536b1e) SHA1(39c86cbb3a33fc77a0141b5648a1aca862e0a5fd) ) /* For these 2 roms:   higher rom numbers seem to indicate that is the case  */
	ROM_LOAD16_BYTE( "b31_29.34", 0x20000, 0x10000, CRC(f2941a37) SHA1(cf1f231d9caddc903116a8b654f49181ca459697) )
	ROM_LOAD16_BYTE( "b31_27.31", 0x20001, 0x10000, CRC(2f3ff642) SHA1(7d6775b51d96b459b163d8fde2385b0e3f5242ca) )

	ROM_LOAD16_BYTE( "b31_41.5", 0x40000, 0x20000, CRC(0daef28a) SHA1(7c7e16b0eebc589ab99f62ddb98b372596ff5ae6) )   /* data roms ? */
	ROM_LOAD16_BYTE( "b31_39.2", 0x40001, 0x20000, CRC(e9197c3c) SHA1(a7f0ef2b3c4258c09edf05284fec45832a8fb147) )
	ROM_LOAD16_BYTE( "b31_40.6", 0x80000, 0x20000, CRC(2ce0f24e) SHA1(39632397ac7e8457607c32c31fccf1c08d4b2621) )
	ROM_LOAD16_BYTE( "b31_38.3", 0x80001, 0x20000, CRC(bc68cd99) SHA1(bb31ea589339c9f9b61e312e1024b5c8410cdb43) )

	ROM_REGION( 0x60000, "sub", 0 ) /* 384K for 68000 CPUB code */
	ROM_LOAD16_BYTE( "b31_33.87", 0x00000, 0x10000, CRC(6ce9af44) SHA1(486e332af238c211c3f64f7ead114282661687c4) )
	ROM_LOAD16_BYTE( "b31_36.97", 0x00001, 0x10000, CRC(ba20b0d4) SHA1(fb3dcb7681a95087afac9aa9393765d786243486) )
	ROM_LOAD16_BYTE( "b31_32.86", 0x20000, 0x10000, CRC(e6025fec) SHA1(071f83a9ddebe67bd6c6c2505318e177895163ee) )
	ROM_LOAD16_BYTE( "b31_35.96", 0x20001, 0x10000, CRC(70d9a89f) SHA1(20f846beb052fd8cddcf00c3e42e3304e102a87b) )
	ROM_LOAD16_BYTE( "b31_31.85", 0x40000, 0x10000, CRC(837f47e2) SHA1(88d596f01566456ba18a01afd0a6a7c121d3ca88) )
	ROM_LOAD16_BYTE( "b31_34.95", 0x40001, 0x10000, CRC(d6b5fb2a) SHA1(e3ae0d7ec62740465a90e4939b10341d3866d860) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "b31_37.11",  0x00000, 0x20000, CRC(0ca5799d) SHA1(6485dde076d15b69b9ee65880dda57ad4f8d129c) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "b31-07.176", 0x000000, 0x80000, CRC(33568cdb) SHA1(87abf56bbbd3659a1bd3e6ce9e43176be7950b41) )   /* OBJ */
	ROM_LOAD16_WORD_SWAP( "b31-06.175", 0x080000, 0x80000, CRC(0d59439e) SHA1(54d844492888e7fe2c3bc61afe64f8d47fdee8dc) )
	ROM_LOAD16_WORD_SWAP( "b31-05.174", 0x100000, 0x80000, CRC(0a1fc9fb) SHA1(a5d6975fd4f7e689c8cafd7c9cd3787797955779) )
	ROM_LOAD16_WORD_SWAP( "b31-04.173", 0x180000, 0x80000, CRC(2e1e4cb5) SHA1(4733cfc015a68e021108a9e1e8ea807b0e7eac7a) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )
	ROM_LOAD16_WORD_SWAP( "b31-01.23", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) ) /* SCR (screen 1) */
	ROM_LOAD16_WORD_SWAP( "b31-02.24", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )

	ROM_REGION( 0x100000, "tc0100scn_2", 0 )
	ROM_LOAD16_WORD_SWAP( "b31-01.26", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) ) /* SCR (screen 2) */
	ROM_LOAD16_WORD_SWAP( "b31-02.27", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )

	ROM_REGION( 0x100000, "tc0100scn_3", 0 )
	ROM_LOAD16_WORD_SWAP( "b31-01.28", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) ) /* SCR (screen 3) */
	ROM_LOAD16_WORD_SWAP( "b31-02.29", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b31-09.18", 0x000000, 0x80000, CRC(60a73382) SHA1(0ddeb86fcd4d19a58e62bf8564f996d17e36e5c5) )
	ROM_LOAD( "b31-10.17", 0x080000, 0x80000, CRC(c6434aef) SHA1(3348ce87882e3f668aa85bbb517975ec1fc9b6fd) )
	ROM_LOAD( "b31-11.16", 0x100000, 0x80000, CRC(8da531d4) SHA1(525dfab0a0729e9fb6f0e4c8187bf4ce16321b20) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b31-08.19", 0x000000, 0x80000, CRC(a0a1f87d) SHA1(6b0f8094f3a3ef1ced76984e333e22a17c51af29) )

	ROM_REGION( 0x01000, "user1", 0 )   /* unknown roms */
	ROM_LOAD( "b31-25.38", 0x00000, 0x200, CRC(a0b4ba48) SHA1(dc9a46366a0cbf63a609f177c3d3ba9675416662) )
	ROM_LOAD( "b31-26.58", 0x00000, 0x200, CRC(13e5fe15) SHA1(c973c7965954a2a0b427908f099592ed89cf0ff0) )
ROM_END

ROM_START( ninjaw1 )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 256K for 68000 CPUA code */
	ROM_LOAD16_BYTE( "b31_30.35", 0x00000, 0x10000, CRC(056edd9f) SHA1(8922cede80b31ce0f7a00c8cab13d835464c6058) ) /* For these 2 roms:  Same code base as the Japanese set below */
	ROM_LOAD16_BYTE( "b31_43.32", 0x00001, 0x10000, CRC(56ae37a6) SHA1(ddd5be455682df2c63721facee813be652863aa5) ) /* For these 2 roms:  original Taito rom, 1 byte region change */
	ROM_LOAD16_BYTE( "b31_29.34", 0x20000, 0x10000, CRC(f2941a37) SHA1(cf1f231d9caddc903116a8b654f49181ca459697) )
	ROM_LOAD16_BYTE( "b31_27.31", 0x20001, 0x10000, CRC(2f3ff642) SHA1(7d6775b51d96b459b163d8fde2385b0e3f5242ca) )

	ROM_LOAD16_BYTE( "b31_41.5", 0x40000, 0x20000, CRC(0daef28a) SHA1(7c7e16b0eebc589ab99f62ddb98b372596ff5ae6) )   /* data roms ? */
	ROM_LOAD16_BYTE( "b31_39.2", 0x40001, 0x20000, CRC(e9197c3c) SHA1(a7f0ef2b3c4258c09edf05284fec45832a8fb147) )
	ROM_LOAD16_BYTE( "b31_40.6", 0x80000, 0x20000, CRC(2ce0f24e) SHA1(39632397ac7e8457607c32c31fccf1c08d4b2621) )
	ROM_LOAD16_BYTE( "b31_38.3", 0x80001, 0x20000, CRC(bc68cd99) SHA1(bb31ea589339c9f9b61e312e1024b5c8410cdb43) )

	ROM_REGION( 0x60000, "sub", 0 ) /* 384K for 68000 CPUB code */
	ROM_LOAD16_BYTE( "b31_33.87", 0x00000, 0x10000, CRC(6ce9af44) SHA1(486e332af238c211c3f64f7ead114282661687c4) )
	ROM_LOAD16_BYTE( "b31_36.97", 0x00001, 0x10000, CRC(ba20b0d4) SHA1(fb3dcb7681a95087afac9aa9393765d786243486) )
	ROM_LOAD16_BYTE( "b31_32.86", 0x20000, 0x10000, CRC(e6025fec) SHA1(071f83a9ddebe67bd6c6c2505318e177895163ee) )
	ROM_LOAD16_BYTE( "b31_35.96", 0x20001, 0x10000, CRC(70d9a89f) SHA1(20f846beb052fd8cddcf00c3e42e3304e102a87b) )
	ROM_LOAD16_BYTE( "b31_31.85", 0x40000, 0x10000, CRC(837f47e2) SHA1(88d596f01566456ba18a01afd0a6a7c121d3ca88) )
	ROM_LOAD16_BYTE( "b31_34.95", 0x40001, 0x10000, CRC(d6b5fb2a) SHA1(e3ae0d7ec62740465a90e4939b10341d3866d860) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "b31_37.11",  0x00000, 0x20000, CRC(0ca5799d) SHA1(6485dde076d15b69b9ee65880dda57ad4f8d129c) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "b31-07.176", 0x000000, 0x80000, CRC(33568cdb) SHA1(87abf56bbbd3659a1bd3e6ce9e43176be7950b41) )   /* OBJ */
	ROM_LOAD16_WORD_SWAP( "b31-06.175", 0x080000, 0x80000, CRC(0d59439e) SHA1(54d844492888e7fe2c3bc61afe64f8d47fdee8dc) )
	ROM_LOAD16_WORD_SWAP( "b31-05.174", 0x100000, 0x80000, CRC(0a1fc9fb) SHA1(a5d6975fd4f7e689c8cafd7c9cd3787797955779) )
	ROM_LOAD16_WORD_SWAP( "b31-04.173", 0x180000, 0x80000, CRC(2e1e4cb5) SHA1(4733cfc015a68e021108a9e1e8ea807b0e7eac7a) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )
	ROM_LOAD16_WORD_SWAP( "b31-01.23", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) ) /* SCR (screen 1) */
	ROM_LOAD16_WORD_SWAP( "b31-02.24", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )

	ROM_REGION( 0x100000, "tc0100scn_2", 0 )
	ROM_LOAD16_WORD_SWAP( "b31-01.26", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) ) /* SCR (screen 2) */
	ROM_LOAD16_WORD_SWAP( "b31-02.27", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )

	ROM_REGION( 0x100000, "tc0100scn_3", 0 )
	ROM_LOAD16_WORD_SWAP( "b31-01.28", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) ) /* SCR (screen 3) */
	ROM_LOAD16_WORD_SWAP( "b31-02.29", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b31-09.18", 0x000000, 0x80000, CRC(60a73382) SHA1(0ddeb86fcd4d19a58e62bf8564f996d17e36e5c5) )
	ROM_LOAD( "b31-10.17", 0x080000, 0x80000, CRC(c6434aef) SHA1(3348ce87882e3f668aa85bbb517975ec1fc9b6fd) )
	ROM_LOAD( "b31-11.16", 0x100000, 0x80000, CRC(8da531d4) SHA1(525dfab0a0729e9fb6f0e4c8187bf4ce16321b20) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b31-08.19", 0x000000, 0x80000, CRC(a0a1f87d) SHA1(6b0f8094f3a3ef1ced76984e333e22a17c51af29) )

	ROM_REGION( 0x01000, "user1", 0 )   /* unknown roms */
	ROM_LOAD( "b31-25.38", 0x00000, 0x200, CRC(a0b4ba48) SHA1(dc9a46366a0cbf63a609f177c3d3ba9675416662) )
	ROM_LOAD( "b31-26.58", 0x00000, 0x200, CRC(13e5fe15) SHA1(c973c7965954a2a0b427908f099592ed89cf0ff0) )
ROM_END

ROM_START( ninjawu )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 256K for 68000 CPUA code */
	ROM_LOAD16_BYTE( "b31_49.35", 0x00000, 0x10000, CRC(d38b6391) SHA1(4711e059531566b779e95619c47621fdbfba2e56) ) /* For these 2 roms:   Revised code base compared to the Japanese set below? */
	ROM_LOAD16_BYTE( "b31_48.32", 0x00001, 0x10000, CRC(4b5bb3d8) SHA1(b0e2059e0fe682ef8152690d93392bdd4fda8149) ) /* For these 2 roms:   higher rom numbers seem to indicate that is the case  */
	ROM_LOAD16_BYTE( "b31_29.34", 0x20000, 0x10000, CRC(f2941a37) SHA1(cf1f231d9caddc903116a8b654f49181ca459697) )
	ROM_LOAD16_BYTE( "b31_27.31", 0x20001, 0x10000, CRC(2f3ff642) SHA1(7d6775b51d96b459b163d8fde2385b0e3f5242ca) )

	ROM_LOAD16_BYTE( "b31_41.5", 0x40000, 0x20000, CRC(0daef28a) SHA1(7c7e16b0eebc589ab99f62ddb98b372596ff5ae6) )   /* data roms ? */
	ROM_LOAD16_BYTE( "b31_39.2", 0x40001, 0x20000, CRC(e9197c3c) SHA1(a7f0ef2b3c4258c09edf05284fec45832a8fb147) )
	ROM_LOAD16_BYTE( "b31_40.6", 0x80000, 0x20000, CRC(2ce0f24e) SHA1(39632397ac7e8457607c32c31fccf1c08d4b2621) )
	ROM_LOAD16_BYTE( "b31_38.3", 0x80001, 0x20000, CRC(bc68cd99) SHA1(bb31ea589339c9f9b61e312e1024b5c8410cdb43) )

	ROM_REGION( 0x60000, "sub", 0 ) /* 384K for 68000 CPUB code */
	ROM_LOAD16_BYTE( "b31_33.87", 0x00000, 0x10000, CRC(6ce9af44) SHA1(486e332af238c211c3f64f7ead114282661687c4) )
	ROM_LOAD16_BYTE( "b31_36.97", 0x00001, 0x10000, CRC(ba20b0d4) SHA1(fb3dcb7681a95087afac9aa9393765d786243486) )
	ROM_LOAD16_BYTE( "b31_32.86", 0x20000, 0x10000, CRC(e6025fec) SHA1(071f83a9ddebe67bd6c6c2505318e177895163ee) )
	ROM_LOAD16_BYTE( "b31_35.96", 0x20001, 0x10000, CRC(70d9a89f) SHA1(20f846beb052fd8cddcf00c3e42e3304e102a87b) )
	ROM_LOAD16_BYTE( "b31_31.85", 0x40000, 0x10000, CRC(837f47e2) SHA1(88d596f01566456ba18a01afd0a6a7c121d3ca88) )
	ROM_LOAD16_BYTE( "b31_34.95", 0x40001, 0x10000, CRC(d6b5fb2a) SHA1(e3ae0d7ec62740465a90e4939b10341d3866d860) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "b31_37.11",  0x00000, 0x20000, CRC(0ca5799d) SHA1(6485dde076d15b69b9ee65880dda57ad4f8d129c) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "b31-07.176", 0x000000, 0x80000, CRC(33568cdb) SHA1(87abf56bbbd3659a1bd3e6ce9e43176be7950b41) )   /* OBJ */
	ROM_LOAD16_WORD_SWAP( "b31-06.175", 0x080000, 0x80000, CRC(0d59439e) SHA1(54d844492888e7fe2c3bc61afe64f8d47fdee8dc) )
	ROM_LOAD16_WORD_SWAP( "b31-05.174", 0x100000, 0x80000, CRC(0a1fc9fb) SHA1(a5d6975fd4f7e689c8cafd7c9cd3787797955779) )
	ROM_LOAD16_WORD_SWAP( "b31-04.173", 0x180000, 0x80000, CRC(2e1e4cb5) SHA1(4733cfc015a68e021108a9e1e8ea807b0e7eac7a) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )
	ROM_LOAD16_WORD_SWAP( "b31-01.23", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) ) /* SCR (screen 1) */
	ROM_LOAD16_WORD_SWAP( "b31-02.24", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )

	ROM_REGION( 0x100000, "tc0100scn_2", 0 )
	ROM_LOAD16_WORD_SWAP( "b31-01.26", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) ) /* SCR (screen 2) */
	ROM_LOAD16_WORD_SWAP( "b31-02.27", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )

	ROM_REGION( 0x100000, "tc0100scn_3", 0 )
	ROM_LOAD16_WORD_SWAP( "b31-01.28", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) ) /* SCR (screen 3) */
	ROM_LOAD16_WORD_SWAP( "b31-02.29", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b31-09.18", 0x000000, 0x80000, CRC(60a73382) SHA1(0ddeb86fcd4d19a58e62bf8564f996d17e36e5c5) )
	ROM_LOAD( "b31-10.17", 0x080000, 0x80000, CRC(c6434aef) SHA1(3348ce87882e3f668aa85bbb517975ec1fc9b6fd) )
	ROM_LOAD( "b31-11.16", 0x100000, 0x80000, CRC(8da531d4) SHA1(525dfab0a0729e9fb6f0e4c8187bf4ce16321b20) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b31-08.19", 0x000000, 0x80000, CRC(a0a1f87d) SHA1(6b0f8094f3a3ef1ced76984e333e22a17c51af29) )

	ROM_REGION( 0x01000, "user1", 0 )   /* unknown roms */
	ROM_LOAD( "b31-25.38", 0x00000, 0x200, CRC(a0b4ba48) SHA1(dc9a46366a0cbf63a609f177c3d3ba9675416662) )
	ROM_LOAD( "b31-26.58", 0x00000, 0x200, CRC(13e5fe15) SHA1(c973c7965954a2a0b427908f099592ed89cf0ff0) )
ROM_END

ROM_START( ninjawj )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 256K for 68000 CPUA code */
	ROM_LOAD16_BYTE( "b31_30.35", 0x00000, 0x10000, CRC(056edd9f) SHA1(8922cede80b31ce0f7a00c8cab13d835464c6058) )
	ROM_LOAD16_BYTE( "b31_28.32", 0x00001, 0x10000, CRC(cfa7661c) SHA1(a7a6abb33a514d910e3198d5acbd4c31b2434b6c) )
	ROM_LOAD16_BYTE( "b31_29.34", 0x20000, 0x10000, CRC(f2941a37) SHA1(cf1f231d9caddc903116a8b654f49181ca459697) )
	ROM_LOAD16_BYTE( "b31_27.31", 0x20001, 0x10000, CRC(2f3ff642) SHA1(7d6775b51d96b459b163d8fde2385b0e3f5242ca) )

	ROM_LOAD16_BYTE( "b31_41.5", 0x40000, 0x20000, CRC(0daef28a) SHA1(7c7e16b0eebc589ab99f62ddb98b372596ff5ae6) )   /* data roms ? */
	ROM_LOAD16_BYTE( "b31_39.2", 0x40001, 0x20000, CRC(e9197c3c) SHA1(a7f0ef2b3c4258c09edf05284fec45832a8fb147) )
	ROM_LOAD16_BYTE( "b31_40.6", 0x80000, 0x20000, CRC(2ce0f24e) SHA1(39632397ac7e8457607c32c31fccf1c08d4b2621) )
	ROM_LOAD16_BYTE( "b31_38.3", 0x80001, 0x20000, CRC(bc68cd99) SHA1(bb31ea589339c9f9b61e312e1024b5c8410cdb43) )

	ROM_REGION( 0x60000, "sub", 0 ) /* 384K for 68000 CPUB code */
	ROM_LOAD16_BYTE( "b31_33.87", 0x00000, 0x10000, CRC(6ce9af44) SHA1(486e332af238c211c3f64f7ead114282661687c4) )
	ROM_LOAD16_BYTE( "b31_36.97", 0x00001, 0x10000, CRC(ba20b0d4) SHA1(fb3dcb7681a95087afac9aa9393765d786243486) )
	ROM_LOAD16_BYTE( "b31_32.86", 0x20000, 0x10000, CRC(e6025fec) SHA1(071f83a9ddebe67bd6c6c2505318e177895163ee) )
	ROM_LOAD16_BYTE( "b31_35.96", 0x20001, 0x10000, CRC(70d9a89f) SHA1(20f846beb052fd8cddcf00c3e42e3304e102a87b) )
	ROM_LOAD16_BYTE( "b31_31.85", 0x40000, 0x10000, CRC(837f47e2) SHA1(88d596f01566456ba18a01afd0a6a7c121d3ca88) )
	ROM_LOAD16_BYTE( "b31_34.95", 0x40001, 0x10000, CRC(d6b5fb2a) SHA1(e3ae0d7ec62740465a90e4939b10341d3866d860) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "b31_37.11",  0x00000, 0x20000, CRC(0ca5799d) SHA1(6485dde076d15b69b9ee65880dda57ad4f8d129c) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "b31-07.176", 0x000000, 0x80000, CRC(33568cdb) SHA1(87abf56bbbd3659a1bd3e6ce9e43176be7950b41) )   /* OBJ */
	ROM_LOAD16_WORD_SWAP( "b31-06.175", 0x080000, 0x80000, CRC(0d59439e) SHA1(54d844492888e7fe2c3bc61afe64f8d47fdee8dc) )
	ROM_LOAD16_WORD_SWAP( "b31-05.174", 0x100000, 0x80000, CRC(0a1fc9fb) SHA1(a5d6975fd4f7e689c8cafd7c9cd3787797955779) )
	ROM_LOAD16_WORD_SWAP( "b31-04.173", 0x180000, 0x80000, CRC(2e1e4cb5) SHA1(4733cfc015a68e021108a9e1e8ea807b0e7eac7a) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )
	ROM_LOAD16_WORD_SWAP( "b31-01.23", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) ) /* SCR (screen 1) */
	ROM_LOAD16_WORD_SWAP( "b31-02.24", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )

	ROM_REGION( 0x100000, "tc0100scn_2", 0 )
	ROM_LOAD16_WORD_SWAP( "b31-01.26", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) ) /* SCR (screen 2) */
	ROM_LOAD16_WORD_SWAP( "b31-02.27", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )

	ROM_REGION( 0x100000, "tc0100scn_3", 0 )
	ROM_LOAD16_WORD_SWAP( "b31-01.28", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) ) /* SCR (screen 3) */
	ROM_LOAD16_WORD_SWAP( "b31-02.29", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )

	ROM_REGION( 0x180000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "b31-09.18", 0x000000, 0x80000, CRC(60a73382) SHA1(0ddeb86fcd4d19a58e62bf8564f996d17e36e5c5) )
	ROM_LOAD( "b31-10.17", 0x080000, 0x80000, CRC(c6434aef) SHA1(3348ce87882e3f668aa85bbb517975ec1fc9b6fd) )
	ROM_LOAD( "b31-11.16", 0x100000, 0x80000, CRC(8da531d4) SHA1(525dfab0a0729e9fb6f0e4c8187bf4ce16321b20) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b31-08.19", 0x000000, 0x80000, CRC(a0a1f87d) SHA1(6b0f8094f3a3ef1ced76984e333e22a17c51af29) )

	ROM_REGION( 0x01000, "user1", 0 )   /* unknown roms */
	ROM_LOAD( "b31-25.38", 0x00000, 0x200, CRC(a0b4ba48) SHA1(dc9a46366a0cbf63a609f177c3d3ba9675416662) )
	ROM_LOAD( "b31-26.58", 0x00000, 0x200, CRC(13e5fe15) SHA1(c973c7965954a2a0b427908f099592ed89cf0ff0) )
ROM_END

ROM_START( darius2 )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 256K for 68000 CPUA code */
	ROM_LOAD16_BYTE( "c07-32-1", 0x00000, 0x10000, CRC(216c8f6a) SHA1(493b0779b99a228911f56ef9d2d4a3945683bec0) )
	ROM_LOAD16_BYTE( "c07-29-1", 0x00001, 0x10000, CRC(48de567f) SHA1(cdf50052933cd2603fd4374e8bae8b30a6c690b5) )
	ROM_LOAD16_BYTE( "c07-31-1", 0x20000, 0x10000, CRC(8279d2f8) SHA1(bd3c80a024a58e4b554f4867f56d7f5741eb3031) )
	ROM_LOAD16_BYTE( "c07-30-1", 0x20001, 0x10000, CRC(6122e400) SHA1(2f68a423f9db8d69ab74453f8cef755f703cc94c) )

	ROM_LOAD16_BYTE( "c07-27",   0x40000, 0x20000, CRC(0a6f7b6c) SHA1(0ed915201fbc0bf94fdcbef8dfd021cebe87474f) )   /* data roms ? */
	ROM_LOAD16_BYTE( "c07-25",   0x40001, 0x20000, CRC(059f40ce) SHA1(b05a96580edb66221af2f222df74a020366ce3ea) )
	ROM_LOAD16_BYTE( "c07-26",   0x80000, 0x20000, CRC(1f411242) SHA1(0fca5d864c1925473d0058e4cf81ad926f56cb14) )
	ROM_LOAD16_BYTE( "c07-24",   0x80001, 0x20000, CRC(486c9c20) SHA1(9e98fcc1777f044d69cc93eda674501b3be26097) )

	ROM_REGION( 0x60000, "sub", 0 ) /* 384K for 68000 CPUB code */
	ROM_LOAD16_BYTE( "c07-35-1", 0x00000, 0x10000, CRC(dd8c4723) SHA1(e17159f894ee661a84ccd53e2d00ee78f2b46196) )
	ROM_LOAD16_BYTE( "c07-38-1", 0x00001, 0x10000, CRC(46afb85c) SHA1(a08fb9fd2bf0929a5599ab015680fa663f1d4fe6) )
	ROM_LOAD16_BYTE( "c07-34-1", 0x20000, 0x10000, CRC(296984b8) SHA1(3ba28e293c9d3ce01ee2f8ae2c2aa450fe021d30) )
	ROM_LOAD16_BYTE( "c07-37-1", 0x20001, 0x10000, CRC(8b7d461f) SHA1(c783491ca23223dc58fa7e8f408407b9a10cbce4) )
	ROM_LOAD16_BYTE( "c07-33-1", 0x40000, 0x10000, CRC(2da03a3f) SHA1(f1f2de82e0addc5e19c8935e4f5810896691118f) )
	ROM_LOAD16_BYTE( "c07-36-1", 0x40001, 0x10000, CRC(02cf2b1c) SHA1(c94a64f26f94f182cfe2b6edb37e4ce35a0f681b) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c07-28",  0x00000, 0x20000, CRC(da304bc5) SHA1(689b4f329d9a640145f82e12dff3dd1fcf8a28c8) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "c07-01", 0x00000, 0x80000, CRC(3cf0f050) SHA1(f5a1f7e327a2617fb95ce2837e72945fd7447346) )    /* OBJ */
	ROM_LOAD16_WORD_SWAP( "c07-02", 0x80000, 0x80000, CRC(75d16d4b) SHA1(795423278b66eca41accce1f8a4425d65af7b629) )

	ROM_REGION( 0x100000, "tc0100scn_1", 0 )
	ROM_LOAD16_WORD_SWAP( "c07-03.12", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) ) /* SCR (screen 1) */
	ROM_LOAD16_WORD_SWAP( "c07-04.11", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

	// The actual board duplicates the SCR gfx ROMs for each TC0100SCNs; TODO : ic position
	ROM_REGION( 0x100000, "tc0100scn_2", 0 )
	ROM_COPY( "tc0100scn_1", 0x000000, 0x000000, 0x100000 )    /* SCR (screen 2) */

	ROM_REGION( 0x100000, "tc0100scn_3", 0 )
	ROM_COPY( "tc0100scn_1", 0x000000, 0x000000, 0x100000 )    /* SCR (screen 3) */

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )  /* ADPCM samples */
	ROM_LOAD( "c07-10.95", 0x00000, 0x80000, CRC(4bbe0ed9) SHA1(081b73c4e4d4fa548445e5548573099bcb1e9213) )
	ROM_LOAD( "c07-11.96", 0x80000, 0x80000, CRC(3c815699) SHA1(0471ff5b0c0da905267f2cee52fd68c8661cccc9) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c07-12.107", 0x00000, 0x80000, CRC(e0b71258) SHA1(0258e308b643d723475824752ebffc4ea29d1ac4) )
ROM_END

} // anonymous namespace


/*******************************************************************************
        DRIVERS
*******************************************************************************/

//    YEAR, NAME,     PARENT, MACHINE, INPUT,   CLASS,        INIT,       MONITOR, COMPANY,                     FULLNAME, FLAGS
GAME( 1987, ninjaw,   0,      ninjaw,  ninjaw,  ninjaw_state, empty_init, ROT0,    "Taito Corporation Japan",   "The Ninja Warriors (World, later version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1987, ninjaw1,  ninjaw, ninjaw,  ninjaw,  ninjaw_state, empty_init, ROT0,    "Taito Corporation Japan",   "The Ninja Warriors (World, earlier version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1987, ninjawj,  ninjaw, ninjaw,  ninjawj, ninjaw_state, empty_init, ROT0,    "Taito Corporation",         "The Ninja Warriors (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1987, ninjawu,  ninjaw, ninjaw,  ninjawj, ninjaw_state, empty_init, ROT0,    "Taito Corporation America (licensed to Romstar)", "The Ninja Warriors (US, Romstar license)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND ) /* Uses same coinage as World, see notes */
GAME( 1989, darius2,  0,      darius2, darius2, ninjaw_state, empty_init, ROT0,    "Taito Corporation",         "Darius II (triple screen) (Japan, rev 1)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
