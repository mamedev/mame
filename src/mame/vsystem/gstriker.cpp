// license:BSD-3-Clause
// copyright-holders:Farfetch'd, David Haywood
/*** DRIVER INFO **************************************************************

Grand Striker, V Goal Soccer, World Cup '94
driver by Farfetch'd and David Haywood

Grand Striker (c)1993  Human
V Goal Soccer (c)1994  Tecmo
Tecmo World Cup '94 (c) 1994 Tecmo

******************************************************************************

    Hardware notes

Both games seem to be similar hardware, V Goal Soccer doesn't work.
the hardware is also quite similar to several other Video System games.

In particular, the sound hardware is identical to aerofgt (including the
memory mapping of the Z80, it's really just a romswap), and the sprite chip
(Fujitsu CG10103) is the same used in several Video System games (see the notes
in the video).

Grand Striker has an IRQ2 which is probably network related.

TODO:
- Finish hooking up the inputs
- Tilemap scrolling/rotation/zooming or whatever effect it needs
- Priorities are wrong. I suspect they need sprite orthogonality
- Missing mixer registers (mainly layer enable/disable)
- Tecmo World Cup '94 has missing protection emulation for draw buy-in
  (as seen by code snippet 0x42ee, referenced in other places as well)
  It's unknown how the game logic should be at current stage.
- Tecmo World Cup '94 also has no name entry whatsoever.

******************************************************************************/

/*** README INFO **************************************************************

*** ROMSET: gstriker

Grand Striker
Human 1993

This game runs on Video Systems h/w.

PCB Nos: TW-107 94V-0
         LD01-A
CPU    : MC68000P10
SND    : Zilog Z0840006PSC (Z80), YM2610, YM3016-D
OSC    : 14.31818 MHz, 20.000MHz
XTAL   : 8.000MHz
DIPs   : 8 position (x2)
RAM    : 6264 (x12), 62256 (x4), CY7C195 (x1), 6116 (x3)
PALs   : 16L8 labelled S204A (near Z80)
         16L8 labelled S205A (near VS920A)
         16L8 labelled S201A \
                       S202A  |
                       S203A /  (Near 68000)


Other  :

MC68B50P (located next to 68000)
Fujitsu MB3773 (8 pin DIP)
Fujitsu MB605E53U (160 pin PQFP, located near U2 & U4) (screen tilemap)
Fujitsu CG10103 145 (160 pin PQFP, located near U25) (sprites)
VS9209 (located near DIPs)
VS920A (located near U79) (score tilemap)

ROMs:
human-1.u58 27C240   - Main Program
human-2.u79 27C1024  - ? (near VS920A)
human-3.u87 27C010   - Sound Program
human-4.u6      27C240   - ?, maybe region specific gfx
scrgs101.u25    23C16000 - GFX
scrgs102.u24    23C16000 - GFX
scrgs103.u23    23C16000 - GFX
scrgs104.u22    23C16000 - GFX
scrgs105.u2     23C16000 - GFX   \
scrgs105.u4     23C16000 - GFX   / note, contents of these are identical.
scrgs106.u93    232001   - Sounds
scrgs107.u99    23c8000  - Sounds

*** ROMSET: vgoalsoc

V Goal Soccer
Tecmo 1994

This game runs on Video Systems h/w.

PCB No: VSIS-20V3, Tecmo No. VG63
CPU: MC68HC000P16
SND: Zilog Z0840006PSC (Z80), YM2610, YM3016-D
OSC: 14.31818 MHz (Near Z80), 32.000MHz (Near 68000), 20.000MHz (Near MCU)
DIPs: 8 position (x2)
RAM: LH5168 (x12), KM62256 (x4), CY7C195 (x1), LH5116 (x3)
PALs: 16L8 labelled S2032A (near Z80)
      16L8 labelled S2036A (near U104)
 4 x  16L8 labelled S2031A \
                    S2033A  |
                    S2034A  |  (Near 68000)
                    S2035A /


Other:

Hitachi H8/325  HD6473258P10 (Micro-controller, located next to 68000)
Fujitsu MB3773 (8 pin DIP)
Fujitsu MB605E53U (160 pin PQFP, located near U17 & U20)
Fujitsu CG10103 145 (160 pin PQFP, located next to VS9210)
VS9210 (located near U11 & U12)
VS9209 (located near DIPs)
VS920A (located near U48) (score tilemap)

ROMs:
c16_u37.u37 27C4002  - Main Program
c16_u48.u48 27C1024  - ?
c16_u65.u65 27C2001  - Sound Program
c13_u86.u86 HN62302  - Sounds
c13_u104.104    HN624116 - Sounds
c13_u20.u20     HN62418  - GFX   \
c13_u17.u17     HN62418  - GFX   / note, contents of these are identical.
c13_u11.u11     HN624116 - GFX
c13_u12.u12     HN624116 - GFX


*** ROMSET: vgoalsoca

Tecmo V Goal Soccer (c)1994 Tecmo

CPU: 68000, Z80
Sound: YM2610
Other: VS9209, VS920A, VS9210, VS920B, HD6473258P10, CG10103, CY7C195,

X1: 20
X2: 32
X3: 14.31818

Note: Same hardware as Tecmo World Cup '94, minus one VS9209 chip.

*** ROMSET: twcup94

Tecmo World Cup 94
Tecmo 1994

VSIS-20V3

   6264
   6264            H8/320         SW    SW
   6264            20MHz  13  6264
   6264   ?        68000-16   6264
   6264
   6264    ?
   6264
   6264
   6264
   6264

   U11         6264
   U12         6264
   U13
   U14         11


   U17-20                U104
           6264 6264
                      U86
   U17-20    ?                 YM2610
                   12   Z80

Frequencies: 68k is XTAL_32MHZ/2
             z80 is XTAL(20'000'000)/4

******************************************************************************/

#include "emu.h"

#include "vs9209.h"
#include "mb60553.h"
#include "vs920a.h"
#include "vsystem_spr.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/6850acia.h"
#include "machine/gen_latch.h"
#include "machine/mb3773.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


// configurable logging
#define LOG_MIXER      (1U << 1)
#define LOG_PROTECTION (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_MIXER | LOG_PROTECTION)

#include "logmacro.h"

#define LOGMIXER(...)      LOGMASKED(LOG_MIXER,      __VA_ARGS__)
#define LOGPROTECTION(...) LOGMASKED(LOG_PROTECTION, __VA_ARGS__)


namespace {

class gstriker_state : public driver_device
{
public:
	gstriker_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_spr(*this, "vsystem_spr"),
		m_bg(*this, "zoomtilemap"),
		m_tx(*this, "texttilemap"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_watchdog(*this, "watchdog"),
		m_acia(*this, "acia"),
		m_cg10103_vram(*this, "cg10103_vram"),
		m_buffered_spriteram(*this, "buffere_spriteram%u", 0U, 0x2000U, ENDIANNESS_BIG),
		m_work_ram(*this, "work_ram"),
		m_mixerregs(*this, "mixerregs"),
		m_soundbank(*this, "soundbank")
	{ }

	void base(machine_config &config);
	void twc94(machine_config &config);
	void gstriker(machine_config &config);
	void vgoal(machine_config &config);

	void init_vgoalsoc();
	void init_twcup94();
	void init_twcup94a();
	void init_twcup94b();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<vsystem_spr_device> m_spr;
	required_device<mb60553_zooming_tilemap_device> m_bg;
	required_device<vs920a_text_tilemap_device> m_tx;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<mb3773_device> m_watchdog;
	optional_device<acia6850_device> m_acia;

	required_shared_ptr<uint16_t> m_cg10103_vram;
	memory_share_array_creator<uint16_t, 2> m_buffered_spriteram;
	required_shared_ptr<uint16_t> m_work_ram;
	required_shared_ptr<uint16_t> m_mixerregs;
	required_memory_bank m_soundbank;

	enum
	{
		TECMO_WCUP94_MCU = 1,
		TECMO_WCUP94A_MCU,
		TECMO_WCUP94B_MCU,
		VGOAL_SOCCER_MCU
	};

	int m_gametype = 0;
	uint16_t m_prot_reg[2]{};

	// common
	void sh_bankswitch_w(uint8_t data);

	// vgoalsoc and twrldc
	void twcup94_prot_reg_w(uint8_t data);

	// vgoalsoc only
	uint16_t vbl_toggle_r();
	void vbl_toggle_w(uint16_t data);

	uint32_t pri_callback(uint32_t color);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	void mcu_init();
	void gstriker_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void twcup94_map(address_map &map) ATTR_COLD;
};


/*** VIDEO UPDATE/START **********************************************/

void gstriker_state::video_start()
{
	// Initialize the chip for the score plane
	m_tx->set_transparent_pen(0xf);

	// Initialize the chip for the screen plane
	m_bg->set_transparent_pen(0xf);
}

uint32_t gstriker_state::pri_callback(uint32_t color)
{
	return BIT(color, 5) ? 0 : GFX_PMASK_2;
}

void gstriker_state::screen_vblank(int state)
{
	// sprites are two frames ahead
	// TODO: probably all Video System games are (Aero Fighters definitely desyncs wrt background)
	if(state)
	{
		memcpy(m_buffered_spriteram[1], m_buffered_spriteram[0], 0x2000);
		memcpy(m_buffered_spriteram[0], m_cg10103_vram, 0x2000);
	}
}


uint32_t gstriker_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_mixerregs[8] & 0x07ff, cliprect); // complete guess, causes green behind test grid in twc94 and blue behind title screen on gstriker
	screen.priority().fill(0, cliprect);

	/*
	[4] AAAA BBBB ---- ---- sprite priority number A/B?
	[5] xxxx ---- ---- ---- background layer priority number?
	[6] xxxx ---- ---- ---- foreground layer priority number?
	*/
	LOGMIXER("%04x %04x %04x %04x %04x %04x %04x %04x | %04x %04x %04x %04x %04x %04x %04x %04x", m_mixerregs[0], m_mixerregs[1], m_mixerregs[2], m_mixerregs[3], m_mixerregs[4], m_mixerregs[5], m_mixerregs[6], m_mixerregs[7], m_mixerregs[8], m_mixerregs[9], m_mixerregs[10], m_mixerregs[11], m_mixerregs[12], m_mixerregs[13], m_mixerregs[14], m_mixerregs[15]);

	m_spr->set_pal_base((m_mixerregs[0] & 0xf000) >> 8);
	m_bg->set_pal_base((m_mixerregs[1] & 0xf000) >> 8);
	m_tx->set_pal_base((m_mixerregs[2] & 0xf000) >> 8);

	// Sandwiched screen/sprite0/score/sprite1. Surely wrong, probably needs sprite orthogonality
	m_bg->draw(screen, bitmap, cliprect, 1);
	m_tx->draw(screen, bitmap, cliprect, 2);

	m_spr->draw_sprites(m_buffered_spriteram[1], 0x2000, screen, bitmap, cliprect);

	return 0;
}


void gstriker_state::machine_start()
{
	m_soundbank->configure_entries(0, 8, memregion("audiocpu")->base(), 0x8000);

	if (m_acia.found())
	{
		m_acia->write_cts(0);
		m_acia->write_dcd(0);
	}
}

/*** SOUND RELATED ***********************************************************/


void gstriker_state::sh_bankswitch_w(uint8_t data)
{
	m_soundbank->set_entry(data & 0x07);
}

/*** GFX DECODE **************************************************************/

static GFXDECODE_START( gfx_gstriker )
	GFXDECODE_ENTRY( "fix_tiles",    0, gfx_8x8x4_packed_lsb,   0, 256 )
	GFXDECODE_ENTRY( "scroll_tiles", 0, gfx_16x16x4_packed_msb, 0, 256 )
GFXDECODE_END

static GFXDECODE_START( gfx_gstriker_spr )
	GFXDECODE_ENTRY( "sprites",      0, gfx_16x16x4_packed_msb, 0, 256 )
GFXDECODE_END


/*** MEMORY LAYOUTS **********************************************************/

void gstriker_state::twcup94_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x103fff).rw(m_bg, FUNC(mb60553_zooming_tilemap_device::vram_r), FUNC(mb60553_zooming_tilemap_device::vram_w));
	map(0x140000, 0x141fff).ram().share(m_cg10103_vram);
	map(0x180000, 0x180fff).rw(m_tx, FUNC(vs920a_text_tilemap_device::vram_r), FUNC(vs920a_text_tilemap_device::vram_w));
	map(0x181000, 0x181fff).rw(m_bg, FUNC(mb60553_zooming_tilemap_device::line_r), FUNC(mb60553_zooming_tilemap_device::line_w));
	map(0x1c0000, 0x1c0fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette").mirror(0x00f000);

	map(0x200000, 0x20000f).rw(m_bg, FUNC(mb60553_zooming_tilemap_device::regs_r), FUNC(mb60553_zooming_tilemap_device::regs_w));
	map(0x200010, 0x200011).nopw();
	map(0x200020, 0x200021).nopw();
	map(0x200040, 0x20005f).ram().share(m_mixerregs);
	map(0x200080, 0x20009f).rw("io", FUNC(vs9209_device::read), FUNC(vs9209_device::write)).umask16(0x00ff);
	map(0x2000a1, 0x2000a1).w("soundlatch", FUNC(generic_latch_8_device::write));

	map(0xffc000, 0xffffff).ram().share(m_work_ram);
}

void gstriker_state::gstriker_map(address_map &map)
{
	twcup94_map(map);
	map(0x200060, 0x200063).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
}

void gstriker_state::sound_map(address_map &map)
{
	map(0x0000, 0x77ff).rom();
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr(m_soundbank);
}

void gstriker_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0x04, 0x04).w(FUNC(gstriker_state::sh_bankswitch_w));
	map(0x08, 0x08).w("soundlatch", FUNC(generic_latch_8_device::acknowledge_w));
	map(0x0c, 0x0c).r("soundlatch", FUNC(generic_latch_8_device::read));
}



/*** INPUT PORTS *************************************************************/

static INPUT_PORTS_START( gstriker_generic )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 ) // "Test"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // vbl?

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)   // "Spare"

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)   // "Spare"
INPUT_PORTS_END

static INPUT_PORTS_START( gstriker )
	PORT_INCLUDE( gstriker_generic )

	// defaults are confirmed from the jp manual
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, "2 Players VS CPU Game" ) PORT_DIPLOCATION("SW1:5") // "Cooperation Coin"
	PORT_DIPSETTING(    0x10, "1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Credits" )
	PORT_DIPNAME( 0x20, 0x20, "Player VS Player Game" ) PORT_DIPLOCATION("SW1:6") // "Competitive Coin"
	PORT_DIPSETTING(    0x20, "1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "New Challenger" ) PORT_DIPLOCATION("SW1:7") // buy-in on linked cab only according to manual
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Cabinet Type" ) PORT_DIPLOCATION("SW1:8") // "Cabinet Type"
	PORT_DIPSETTING(    0x00, "1 Player" )
	PORT_DIPSETTING(    0x80, "2 Players" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x06, 0x04, "Player(s) VS CPU Time" ) PORT_DIPLOCATION("SW2:2,3") // "Tournament  Time"
	PORT_DIPSETTING(    0x06, "1:30" )
	PORT_DIPSETTING(    0x04, "2:00" )
	PORT_DIPSETTING(    0x02, "3:00" )
	PORT_DIPSETTING(    0x00, "4:00" )
	PORT_DIPNAME( 0x18, 0x10, "Player VS Player Time" ) PORT_DIPLOCATION("SW2:4,5") // "Competitive Time"
	PORT_DIPSETTING(    0x18, "2:00" )
	PORT_DIPSETTING(    0x10, "3:00" )
	PORT_DIPSETTING(    0x08, "4:00" )
	PORT_DIPSETTING(    0x00, "5:00" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6") // "Demo Sound"
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Communication Mode" ) PORT_DIPLOCATION("SW2:7") // "Master/Slave"
	PORT_DIPSETTING(    0x40, "Master" )
	PORT_DIPSETTING(    0x00, "Slave" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" ) // "Self Test Mode"
INPUT_PORTS_END

static INPUT_PORTS_START( twcup94 )
	PORT_INCLUDE( gstriker_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Pass")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Shoot")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Pass")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Shoot")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )

	PORT_DIPNAME( 0xc0, 0xc0, "Play Time" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, "P v CPU 1:00, P v P 1:30" )
	PORT_DIPSETTING(    0xc0, "P v CPU 1:30, P v P 2:00" )
	PORT_DIPSETTING(    0x40, "P v CPU 2:00, P v P 2:30" )
	PORT_DIPSETTING(    0x80, "P v CPU 2:30, P v P 3:00" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Show Dip Configuration" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Countdown" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "54 sec" )
	PORT_DIPSETTING(    0x00, "60 sec" )
	PORT_DIPNAME( 0x20, 0x20, "Start credit" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7")
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( vgoalsoc )
	PORT_INCLUDE( gstriker_generic )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x80, "A" )
	PORT_DIPSETTING(    0xc0, "B" )
	PORT_DIPSETTING(    0x40, "C" )
	PORT_DIPSETTING(    0x00, "D" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Player VS CPU Time" ) PORT_DIPLOCATION("SW2:1,2") // no cooperative
	PORT_DIPSETTING(    0x02, "1:00" )
	PORT_DIPSETTING(    0x03, "1:30" )
	PORT_DIPSETTING(    0x01, "2:00" )
	PORT_DIPSETTING(    0x00, "2:30" )
	PORT_DIPNAME( 0x0c, 0x0c, "Player VS Player Time" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "1:30" )
	PORT_DIPSETTING(    0x0c, "2:00" )
	PORT_DIPSETTING(    0x04, "2:30" )
	PORT_DIPSETTING(    0x00, "3:00" )
	PORT_DIPNAME( 0x10, 0x10, "Countdown" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "54 sec" )
	PORT_DIPSETTING(    0x00, "60 sec" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x80, "Start credit" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )
INPUT_PORTS_END

/*** MACHINE DRIVER **********************************************************/

void gstriker_state::base(machine_config &config)
{
	Z80(config, m_audiocpu, 8_MHz_XTAL / 2); // 4 MHz ???
	m_audiocpu->set_addrmap(AS_PROGRAM, &gstriker_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &gstriker_state::sound_io_map);

	vs9209_device &io(VS9209(config, "io", 0));
	io.porta_input_cb().set_ioport("P1");
	io.portb_input_cb().set_ioport("P2");
	io.portc_input_cb().set_ioport("SYSTEM");
	io.portd_input_cb().set_ioport("DSW1");
	io.porte_input_cb().set_ioport("DSW2");
	io.porth_input_cb().set("soundlatch", FUNC(generic_latch_8_device::pending_r)).lshift(0);
	io.porth_output_cb().set("watchdog", FUNC(mb3773_device::write_line_ck)).bit(3);

	MB3773(config, m_watchdog, 0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
//  m_screen->set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(5000)); // hand-tuned, it needs a bit
	m_screen->set_size(64*8, 64*8);
	m_screen->set_visarea(0*8, 40*8-1, 0*8, 28*8-1);
	m_screen->set_screen_update(FUNC(gstriker_state::screen_update));
	m_screen->screen_vblank().set(FUNC(gstriker_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gstriker);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x800);

	MB60553(config, m_bg, 0);
	m_bg->set_gfxdecode_tag(m_gfxdecode);
	m_bg->set_gfx_region(1);

	VS920A(config, m_tx, 0);
	m_tx->set_gfxdecode_tag(m_gfxdecode);
	m_tx->set_gfx_region(0);

	VSYSTEM_SPR(config, m_spr, 0, m_palette, gfx_gstriker_spr);
	m_spr->set_pri_cb(FUNC(gstriker_state::pri_callback));
	m_spr->set_pal_mask(0x1f);
	m_spr->set_transpen(0);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	generic_latch_8_device &soundlatch(GENERIC_LATCH_8(config, "soundlatch"));
	soundlatch.data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	soundlatch.set_separate_acknowledge(true);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 8_MHz_XTAL));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}

void gstriker_state::gstriker(machine_config &config)
{
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &gstriker_state::gstriker_map);
	m_maincpu->set_vblank_int("screen", FUNC(gstriker_state::irq1_line_hold));

	base(config);

	ACIA6850(config, m_acia, 0);
	m_acia->irq_handler().set_inputline(m_maincpu, M68K_IRQ_2);
	//m_acia->txd_handler().set("link", FUNC(rs232_port_device::write_txd));
	//m_acia->rts_handler().set("link", FUNC(rs232_port_device::write_rts));
}

void gstriker_state::twc94(machine_config &config)
{
	M68000(config, m_maincpu, 32_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &gstriker_state::twcup94_map);
	m_maincpu->set_vblank_int("screen", FUNC(gstriker_state::irq1_line_hold));

	base(config);

	m_audiocpu->set_clock(20_MHz_XTAL / 4);

	subdevice<vs9209_device>("io")->porth_output_cb().append(FUNC(gstriker_state::twcup94_prot_reg_w));
}


void gstriker_state::vgoal(machine_config &config)
{
	twc94(config);
	m_spr->set_transpen(0xf); // different vs. the other games, TODO: find register
}




/*** ROM LOADING *************************************************************/

ROM_START( gstriker )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "human-1.u58",  0x00000, 0x80000, CRC(45cf4857) SHA1(8133a9a7bdd547cc3d69140a68a1a5a7341e9f5b) )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "human-3_27c1001.u87",  0x00000, 0x20000, CRC(2f28c01e) SHA1(63829ad7969d197b2f2c87cb88bdb9e9880ed2d6) )

	ROM_REGION( 0x20000, "fix_tiles", 0 ) // score
	ROM_LOAD( "human-2_27c1024.u79",  0x00000, 0x20000, CRC(a981993b) SHA1(ed92c7581d2b84a8628744dd5f8a2266c45dcd5b) )

	ROM_REGION( 0x200000, "scroll_tiles", 0 )
	ROM_LOAD( "human_scr-gs-105_m531602c-44_3405356.u2",  0x00000, 0x200000, CRC(d584b568) SHA1(64c5e4fdbb859873e51f62d8f5314598108270ef) )
	ROM_LOAD( "human_scr-gs-105_m531602c-44_3405356.u4",  0x00000, 0x200000, CRC(d584b568) SHA1(64c5e4fdbb859873e51f62d8f5314598108270ef) ) // same content, different position on board

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD( "human_scr-gs-101_m531602c-40_3405351.u25", 0x000000, 0x200000, CRC(becaea24) SHA1(e96fca863f49f50992f56c7defa5a69599608785) )
	ROM_LOAD( "human_scr-gs-102_m531602c-41_3405355.u24", 0x200000, 0x200000, CRC(0dae7aba) SHA1(304f336994be33fa8239c13e6fd9967c06f97d5c) )
	ROM_LOAD( "human_scr-gs-103_m531602c-42_3405353.u23", 0x400000, 0x200000, CRC(3448fe92) SHA1(c4c2d2d5610795aff6633b0601ff484897598904) )
	ROM_LOAD( "human_scr-gs-104_m531602c-43_3405354.u22", 0x600000, 0x200000, CRC(0ac33e5a) SHA1(9d7717d80f2c6817bac3fad50c39e04f0aa94255) )
	ROM_LOAD( "human-4_27c240.u6",                        0xf80000, 0x080000, CRC(a990f9bb) SHA1(7ce31d4c650eb244e2ab285f253a98d6613b7dc8) ) // extra European team flags

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "human_scr-gs-106_m532001b-16_3402370.u93", 0x00000, 0x040000, CRC(93c9868c) SHA1(dcecb34e46405155e35aaf134b8547430d23f5a7) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "scrgs107.u99", 0x00000, 0x100000, CRC(ecc0a01b) SHA1(239e832b7d22925460a8f44eb82e782cd13aba49) )

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "pal16l8.s201a.u52",   0x0000, 0x0104, CRC(724faf0f) SHA1(755fad09d188af58efce733a9f1256b1abc7c360) )
	ROM_LOAD( "pal16l8.s202a.u74",   0x0200, 0x0104, CRC(ad5c4722) SHA1(0aad71b73c6674e15596b7de59160a5156a4118d) )
	ROM_LOAD( "pal16l8.s203a.u75",   0x0400, 0x0104, CRC(ad197e2d) SHA1(e0691b79b8433285a0bafea1d52b0166f6417c20) )
	ROM_LOAD( "pal16l8.s204a.u89",   0x0600, 0x0104, CRC(eb997577) SHA1(504a2499c8a96c74607d06aefb0a062612a78b38) )
	ROM_LOAD( "pal16l8.s205a.u109",  0x0800, 0x0104, CRC(0d644e59) SHA1(bb8f4ab47d7bc9b9b37f636f8fa9c419f17630ad) )
ROM_END

ROM_START( gstrikera )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "human-1_27c4002.u58",  0x00000, 0x80000, CRC(7cf45320) SHA1(4127c93fe5f863cecf0a005c66129c0eb660f5dd) )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "human-3_27c1001.u87",  0x00000, 0x20000, CRC(2f28c01e) SHA1(63829ad7969d197b2f2c87cb88bdb9e9880ed2d6) )

	ROM_REGION( 0x20000, "fix_tiles", 0 ) // score
	ROM_LOAD( "human-2_27c1024.u79",  0x00000, 0x20000, CRC(a981993b) SHA1(ed92c7581d2b84a8628744dd5f8a2266c45dcd5b) )

	ROM_REGION( 0x200000, "scroll_tiles", 0 )
	ROM_LOAD( "human_scr-gs-105_m531602c-44_3405356.u2",  0x00000, 0x200000, CRC(d584b568) SHA1(64c5e4fdbb859873e51f62d8f5314598108270ef) )
	ROM_LOAD( "human_scr-gs-105_m531602c-44_3405356.u4",  0x00000, 0x200000, CRC(d584b568) SHA1(64c5e4fdbb859873e51f62d8f5314598108270ef) ) // same content, different position on board

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD( "human_scr-gs-101_m531602c-40_3405351.u25", 0x000000, 0x200000, CRC(becaea24) SHA1(e96fca863f49f50992f56c7defa5a69599608785) )
	ROM_LOAD( "human_scr-gs-102_m531602c-41_3405355.u24", 0x200000, 0x200000, CRC(0dae7aba) SHA1(304f336994be33fa8239c13e6fd9967c06f97d5c) )
	ROM_LOAD( "human_scr-gs-103_m531602c-42_3405353.u23", 0x400000, 0x200000, CRC(3448fe92) SHA1(c4c2d2d5610795aff6633b0601ff484897598904) )
	ROM_LOAD( "human_scr-gs-104_m531602c-43_3405354.u22", 0x600000, 0x200000, CRC(0ac33e5a) SHA1(9d7717d80f2c6817bac3fad50c39e04f0aa94255) )
	ROM_LOAD( "human-4_27c240.u6",                        0xf80000, 0x080000, CRC(a990f9bb) SHA1(7ce31d4c650eb244e2ab285f253a98d6613b7dc8) ) // extra European team flags

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "human_scr-gs-106_m532001b-16_3402370.u93", 0x00000, 0x040000, CRC(93c9868c) SHA1(dcecb34e46405155e35aaf134b8547430d23f5a7) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "scrgs107.u99", 0x00000, 0x100000, CRC(ecc0a01b) SHA1(239e832b7d22925460a8f44eb82e782cd13aba49) )

	// PALs were protected on this version, used the ones from the "gstriker" set
	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "pal16l8.s201a.u52",   0x0000, 0x0104, CRC(724faf0f) SHA1(755fad09d188af58efce733a9f1256b1abc7c360) )
	ROM_LOAD( "pal16l8.s202a.u74",   0x0200, 0x0104, CRC(ad5c4722) SHA1(0aad71b73c6674e15596b7de59160a5156a4118d) )
	ROM_LOAD( "pal16l8.s203a.u75",   0x0400, 0x0104, CRC(ad197e2d) SHA1(e0691b79b8433285a0bafea1d52b0166f6417c20) )
	ROM_LOAD( "pal16l8.s204a.u89",   0x0600, 0x0104, CRC(eb997577) SHA1(504a2499c8a96c74607d06aefb0a062612a78b38) )
	ROM_LOAD( "pal16l8.s205a.u109",  0x0800, 0x0104, CRC(0d644e59) SHA1(bb8f4ab47d7bc9b9b37f636f8fa9c419f17630ad) )
ROM_END

ROM_START( gstrikerj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "human1.u58",  0x00000, 0x80000, CRC(dce0549c) SHA1(5805a81ddae6bec5b6cc47dc1dbcbe2a81d2c033) )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "human3.u87",  0x00000, 0x20000, CRC(2f28c01e) SHA1(63829ad7969d197b2f2c87cb88bdb9e9880ed2d6) )

	ROM_REGION( 0x20000, "fix_tiles", 0 ) // score
	ROM_LOAD( "human2.u79",  0x00000, 0x20000, CRC(9ad17eb3) SHA1(614b2630e02745f675b1791a514a90131264d545) )

	ROM_REGION( 0x200000, "scroll_tiles", 0 )
	ROM_LOAD( "human_scr-gs-105_m531602c-44_3405356.u2",  0x00000, 0x200000, CRC(d584b568) SHA1(64c5e4fdbb859873e51f62d8f5314598108270ef) )
	ROM_LOAD( "human_scr-gs-105_m531602c-44_3405356.u4",  0x00000, 0x200000, CRC(d584b568) SHA1(64c5e4fdbb859873e51f62d8f5314598108270ef) ) // same content, different position on board

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD( "human_scr-gs-101_m531602c-40_3405351.u25", 0x000000, 0x200000, CRC(becaea24) SHA1(e96fca863f49f50992f56c7defa5a69599608785) )
	ROM_LOAD( "human_scr-gs-102_m531602c-41_3405355.u24", 0x200000, 0x200000, CRC(0dae7aba) SHA1(304f336994be33fa8239c13e6fd9967c06f97d5c) )
	ROM_LOAD( "human_scr-gs-103_m531602c-42_3405353.u23", 0x400000, 0x200000, CRC(3448fe92) SHA1(c4c2d2d5610795aff6633b0601ff484897598904) )
	ROM_LOAD( "human_scr-gs-104_m531602c-43_3405354.u22", 0x600000, 0x200000, CRC(0ac33e5a) SHA1(9d7717d80f2c6817bac3fad50c39e04f0aa94255) )
	// u6 is NOT populated on the JPN version

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "human_scr-gs-106_m532001b-16_3402370.u93", 0x00000, 0x040000, CRC(93c9868c) SHA1(dcecb34e46405155e35aaf134b8547430d23f5a7) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "scrgs107.u99", 0x00000, 0x100000, CRC(ecc0a01b) SHA1(239e832b7d22925460a8f44eb82e782cd13aba49) )

	// PALs were protected on this version, used the ones from the "gstriker" set
	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "pal16l8.s201a.u52",   0x0000, 0x0104, CRC(724faf0f) SHA1(755fad09d188af58efce733a9f1256b1abc7c360) )
	ROM_LOAD( "pal16l8.s202a.u74",   0x0200, 0x0104, CRC(ad5c4722) SHA1(0aad71b73c6674e15596b7de59160a5156a4118d) )
	ROM_LOAD( "pal16l8.s203a.u75",   0x0400, 0x0104, CRC(ad197e2d) SHA1(e0691b79b8433285a0bafea1d52b0166f6417c20) )
	ROM_LOAD( "pal16l8.s204a.u89",   0x0600, 0x0104, CRC(eb997577) SHA1(504a2499c8a96c74607d06aefb0a062612a78b38) )
	ROM_LOAD( "pal16l8.s205a.u109",  0x0800, 0x0104, CRC(0d644e59) SHA1(bb8f4ab47d7bc9b9b37f636f8fa9c419f17630ad) )
ROM_END


// these were bruteforced from secured pal16l8 devices found on a twcup94a set, probably the same for all sets?
#define TWCUP94_PLD_DEVICES \
	ROM_LOAD( "s2031a.u39", 0x0000, 0x0117, CRC(66f6020f) SHA1(b44a9ad51c1987bab14fb044b3ee37d73ec96fa7) ) \
	ROM_LOAD( "s2032a.u64", 0x0200, 0x0117, CRC(e186728e) SHA1(c6ad476566d48585944e7f7889667899f654619b) ) \
	ROM_LOAD( "s2033a.u66", 0x0400, 0x0117, CRC(672aa79b) SHA1(2e1f0643e537d6040855478f1c5b4a9f117458fe) ) \
	ROM_LOAD( "s2034a.u67", 0x0600, 0x0117, CRC(92ebeafd) SHA1(3bf5fd1f12934c3b7076dd1f31820bbb4c4b2bd2) ) \
	ROM_LOAD( "s2035a.u68", 0x0800, 0x0117, CRC(e3fe7bc9) SHA1(339adcfa3128f466fb5a216f53b098e6fd9d7d2b) ) \
	ROM_LOAD( "s2036a.u79", 0x1000, 0x0117, CRC(20a4c0c5) SHA1(2bef5fca2f17877f23a4c8c5c183f8895f3d18c6) )

ROM_START( vgoalsoc )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "c16_u37.u37",  0x00000, 0x80000, CRC(18c05440) SHA1(0fc78ee0ba6d7817d4a93a80f668f193c352c00d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "c16_u65.u65",  0x000000, 0x040000, CRC(2f7bf23c) SHA1(1a1a06f57bbac59807679e3762cb2f23ab1ad35e) )

	ROM_REGION( 0x20000, "mcu", 0 )
	ROM_LOAD( "vgoalsoc_hd6473258p10", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x20000, "fix_tiles", 0 ) // score
	ROM_LOAD( "c16_u48.u48",  0x000000, 0x020000, CRC(ca059e7f) SHA1(2fa48b0fec1210575f3a1ecee7d2aec0af3fa9c4) )

	ROM_REGION( 0x100000, "scroll_tiles", 0 ) // screen
	ROM_LOAD( "c13_u20.u20",  0x000000, 0x100000, CRC(bc6e07e8) SHA1(3f164165a2eed909aaf38d1ae23a622482d39f96) )
	ROM_LOAD( "c13_u17.u17",  0x000000, 0x100000, CRC(bc6e07e8) SHA1(3f164165a2eed909aaf38d1ae23a622482d39f96) ) // same content, different position on board

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "c13_u11.u11",  0x000000, 0x200000, CRC(76d09f27) SHA1(ffef83954426f9e56bbe2d98b32cea675c063fab) )
	ROM_LOAD( "c13_u12.u12",  0x200000, 0x200000, CRC(a3874419) SHA1(c9fa283106ada3419e311f400fcf4251b32318c4) )

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "c13_u86.u86",  0x000000, 0x040000, CRC(4b76a162) SHA1(38dcb7536662f5f520e59f3ff746b42e9df789d2) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "c13_u104.104", 0x000000, 0x200000, CRC(8437b6f8) SHA1(79f183dcbf3cde5c77e086e4fdd8341809396e37) )

	ROM_REGION( 0x1200, "plds", 0 ) // from twcup94a set
	TWCUP94_PLD_DEVICES
ROM_END

ROM_START( vgoalsoca )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "vgoalc16.u37", 0x00000, 0x80000, CRC(775ef300) SHA1(d0ab1c13a19ce646c6edfc25a0c0994989560cbc) )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "c16_u65.u65",  0x000000, 0x040000, CRC(2f7bf23c) SHA1(1a1a06f57bbac59807679e3762cb2f23ab1ad35e) )

	ROM_REGION( 0x20000, "mcu", 0 )
	ROM_LOAD( "vgoalsoc_hd6473258p10", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x20000, "fix_tiles", 0 )
	ROM_LOAD( "c16_u48.u48",  0x000000, 0x020000, CRC(ca059e7f) SHA1(2fa48b0fec1210575f3a1ecee7d2aec0af3fa9c4) )

	ROM_REGION( 0x100000, "scroll_tiles", 0 )
	ROM_LOAD( "c13_u20.u20",  0x000000, 0x100000, CRC(bc6e07e8) SHA1(3f164165a2eed909aaf38d1ae23a622482d39f96) )
	ROM_LOAD( "c13_u17.u17",  0x000000, 0x100000, CRC(bc6e07e8) SHA1(3f164165a2eed909aaf38d1ae23a622482d39f96) ) // same content, different position on board

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "c13_u11.u11",  0x000000, 0x200000, CRC(76d09f27) SHA1(ffef83954426f9e56bbe2d98b32cea675c063fab) )
	ROM_LOAD( "c13_u12.u12",  0x200000, 0x200000, CRC(a3874419) SHA1(c9fa283106ada3419e311f400fcf4251b32318c4) )

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "c13_u86.u86",  0x000000, 0x040000, CRC(4b76a162) SHA1(38dcb7536662f5f520e59f3ff746b42e9df789d2) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "c13_u104.104", 0x000000, 0x200000, CRC(8437b6f8) SHA1(79f183dcbf3cde5c77e086e4fdd8341809396e37) )

	ROM_REGION( 0x1200, "plds", 0 ) // from twcup94a set
	TWCUP94_PLD_DEVICES
ROM_END

ROM_START( twcup94 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "13.u37", 0x00000, 0x80000, CRC(42adb463) SHA1(ec7bcb684489b56f81ab851a9d8f42d54679363b) )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "12.u65", 0x000000, 0x040000, CRC(f316e7fc) SHA1(a2215605518e7293774735371c65abcead99bd88) )

	ROM_REGION( 0x20000, "mcu", 0 )
	ROM_LOAD( "twcup94_hd6473258p10", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x20000, "fix_tiles", 0 )
	ROM_LOAD( "11.u48", 0x000000, 0x020000, CRC(37d6dcb6) SHA1(679dd8b615497fff23c4638d413b5d4a724d3f2a) )

	ROM_REGION( 0x200000, "scroll_tiles", 0 )
	ROM_LOAD( "u17", 0x000000, 0x200000, CRC(a5e40a61) SHA1(a2cb452fb069862570870653b29b045d12caf062) )
	ROM_LOAD( "u20", 0x000000, 0x200000, CRC(a5e40a61) SHA1(a2cb452fb069862570870653b29b045d12caf062) )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD( "u11", 0x000000, 0x200000, CRC(dd93fd45) SHA1(26491815b5443fe6d8b1ef4d795c5151fd75c101) )
	ROM_LOAD( "u12", 0x200000, 0x200000, CRC(8e3c9bd2) SHA1(bfd23157c836148a3860ccea5191f656fdd98ef4) )
	ROM_LOAD( "u13", 0x400000, 0x200000, CRC(8db6b3a9) SHA1(9422cd5d6fb57a7eaa7a13bdf4ccee1f8b57f773) )
	ROM_LOAD( "u14", 0x600000, 0x200000, CRC(89739c31) SHA1(29cd779bfe93448fb6cbfe6f8e3661dd659c0d21) )

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "u86", 0x000000, 0x040000, CRC(775f45dc) SHA1(1a740dd880d9f873e93dfc096fbcae1784b4f522) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "u104", 0x000000, 0x100000, CRC(df07d0af) SHA1(356560e164ff222bc9004fe202f829c93244a6c9) )

	ROM_REGION( 0x1200, "plds", 0 ) // from twcup94a set
	TWCUP94_PLD_DEVICES
ROM_END

ROM_START( twcup94a )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "twrdc94a_13.u37", 0x00000, 0x80000, CRC(08f314ee) SHA1(3fca5050f5bcd60533d3bd9dea81ba631a98bfd6) )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "twrdc94a_12.u65", 0x000000, 0x040000, CRC(c131f5a4) SHA1(d8cc7c463ad628f6f052489a73b97f998532738d) )

	ROM_REGION( 0x20000, "mcu", 0 )
	ROM_LOAD( "twcup94_hd6473258p10", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x20000, "fix_tiles", 0 )
	ROM_LOAD( "twrdc94a_11.u48", 0x000000, 0x020000, CRC(37d6dcb6) SHA1(679dd8b615497fff23c4638d413b5d4a724d3f2a) )

	ROM_REGION( 0x200000, "scroll_tiles", 0 )
	ROM_LOAD( "u17", 0x000000, 0x200000, CRC(a5e40a61) SHA1(a2cb452fb069862570870653b29b045d12caf062) )
	ROM_LOAD( "u20", 0x000000, 0x200000, CRC(a5e40a61) SHA1(a2cb452fb069862570870653b29b045d12caf062) )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD( "u11", 0x000000, 0x200000, CRC(dd93fd45) SHA1(26491815b5443fe6d8b1ef4d795c5151fd75c101) )
	ROM_LOAD( "u12", 0x200000, 0x200000, CRC(8e3c9bd2) SHA1(bfd23157c836148a3860ccea5191f656fdd98ef4) )
	ROM_LOAD( "u13", 0x400000, 0x200000, CRC(8db6b3a9) SHA1(9422cd5d6fb57a7eaa7a13bdf4ccee1f8b57f773) )
	ROM_LOAD( "u14", 0x600000, 0x200000, CRC(89739c31) SHA1(29cd779bfe93448fb6cbfe6f8e3661dd659c0d21) )

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "u86", 0x000000, 0x040000, CRC(775f45dc) SHA1(1a740dd880d9f873e93dfc096fbcae1784b4f522) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "u104", 0x000000, 0x100000, CRC(df07d0af) SHA1(356560e164ff222bc9004fe202f829c93244a6c9) )

	ROM_REGION( 0x1200, "plds", 0 )
	TWCUP94_PLD_DEVICES
ROM_END

ROM_START( twcup94b )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "twrdc94b_13.u37", 0x00000, 0x80000, CRC(00059e88) SHA1(0da18d7f6ede7c6b50e45e0c8f7b70516b974fc3) )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "twrdc94a_12.u65", 0x000000, 0x040000, CRC(c131f5a4) SHA1(d8cc7c463ad628f6f052489a73b97f998532738d) )

	ROM_REGION( 0x20000, "mcu", 0 )
	ROM_LOAD( "twcup94_hd6473258p10", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x20000, "fix_tiles", 0 )
	ROM_LOAD( "11.u48", 0x000000, 0x020000, CRC(37d6dcb6) SHA1(679dd8b615497fff23c4638d413b5d4a724d3f2a) )

	ROM_REGION( 0x200000, "scroll_tiles", 0 )
	ROM_LOAD( "u17", 0x000000, 0x200000, CRC(a5e40a61) SHA1(a2cb452fb069862570870653b29b045d12caf062) )
	ROM_LOAD( "u20", 0x000000, 0x200000, CRC(a5e40a61) SHA1(a2cb452fb069862570870653b29b045d12caf062) )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD( "u11", 0x000000, 0x200000, CRC(dd93fd45) SHA1(26491815b5443fe6d8b1ef4d795c5151fd75c101) )
	ROM_LOAD( "u12", 0x200000, 0x200000, CRC(8e3c9bd2) SHA1(bfd23157c836148a3860ccea5191f656fdd98ef4) )
	ROM_LOAD( "u13", 0x400000, 0x200000, CRC(8db6b3a9) SHA1(9422cd5d6fb57a7eaa7a13bdf4ccee1f8b57f773) )
	ROM_LOAD( "u14", 0x600000, 0x200000, CRC(89739c31) SHA1(29cd779bfe93448fb6cbfe6f8e3661dd659c0d21) )

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "u86", 0x000000, 0x040000, CRC(775f45dc) SHA1(1a740dd880d9f873e93dfc096fbcae1784b4f522) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "u104", 0x000000, 0x100000, CRC(df07d0af) SHA1(356560e164ff222bc9004fe202f829c93244a6c9) )

	ROM_REGION( 0x1200, "plds", 0 ) // from twcup94a set
	TWCUP94_PLD_DEVICES
ROM_END



/******************************************************************************************
Simple protection check concept. The M68k writes a command and the MCU
returns the PC at address 0xffc000.
The problem is that only the concept is easy, beating this protection requires a good
amount of time without a trojan...

Misc Notes:
-Protection routine is at 0x890
-An original feature of this game is that if you enter into service mode the game gives you
the possibility to test various stuff on a pre-registered play such as the speed or
the zooming. To use it, you should use Player 2 Start button to show the test screens
or to advance into the tests.
******************************************************************************************/
#define PC(_num_)\
		m_work_ram[0x000 / 2] = (_num_ & 0xffff0000) >> 16;\
		m_work_ram[0x002 / 2] = (_num_ & 0x0000ffff) >> 0;


void gstriker_state::twcup94_prot_reg_w(uint8_t data)
{
	m_prot_reg[1] = m_prot_reg[0];
	m_prot_reg[0] = data;

	// Command byte is also written to VS9209 port F, which is set for input only.
	// Does the MCU somehow strobe it out of there?
	uint8_t mcu_data = m_work_ram[0x00f / 2] & 0x00ff;

	if( ((m_prot_reg[1] & 4) == 0) && ((m_prot_reg[0] & 4) == 4) )
	{
		switch( m_gametype )
		{
			case TECMO_WCUP94_MCU:
				switch (mcu_data)
				{
					#define NULL_SUB 0x0000828e
					case 0x53: PC(0x00000a4c); break; // boot -> main loop

					/*
					    68 and 62 could be sprite or sound changes, or ?
					    68(),61()
					    if( !carry )
					    {
					        68(),65()
					    }
					    else
					    {
					        62(),72()
					    }
					*/
					case 0x68: PC(NULL_SUB); break; // time up doesn't block long enough for pk shootout
					case 0x61: PC(0x00003af4); break; // after time up, pk shootout???
					case 0x65: PC(0x00003f26); break;

					// 62->72
					case 0x62: PC(NULL_SUB); break; // after losing shootout, continue ???
					case 0x72: PC(0x0000409e); break; // game over

					/*
					    Attract mode is pre programmed loop called from main
					    that runs through top11->demoplay
					    (NOTE: sprites for demo play are being drawn at 0x141000,
					    this address is used in a few places, and there's some activity
					    further up around 0x1410b0.)

					    The loop begins with three prot calls:
					    one always present (may be diversion to 0x0010dc8 unreachable code
					    and prot cases 6a,79,6f) and two alternating calls.
					    The loop is 6e -> [6b|69] -> top11 -> (4 segment)playdemo

					    These are the likely suspects for attract mode:
					    0x0010E28 red Tecmo on black
					    0x0010EEC bouncing ball and player with game title
					    0x00117A2 single segment demo play with player sprites at 0x140000
					    0x001120A sliding display of player photos
					    0x0010DC8 unreachable code at end of attract loop with cases 6a,79,6f

					*/
					case 0x6e: PC(0x00010e28); break; // loop
					case 0x6b: PC(0x00010eec); break; // attract even
					case 0x69: PC(0x0001120a); break; // attract odd

					// In "continue" screen
					// if( w@FFE078 & 80) 75
					// *** after 75 beq
					case 0x75: PC(0x005088); break; // match adder, and check if limit is reached for ending

					// unreachable code at end of attract loop 6a->79->6f
					case 0x6a: PC(NULL_SUB); break;
					case 0x79: PC(NULL_SUB); break;
					case 0x6f: PC(NULL_SUB); break;

					default:
						LOGPROTECTION("Unknown MCU CMD %04x\n", mcu_data);
						PC(NULL_SUB);
						break;

					#undef NULL_SUB
				}
				break;

			// same as above but with +0x10 displacement offsets
			case TECMO_WCUP94A_MCU:

				switch (mcu_data)
				{
					#define NULL_SUB 0x0000829e
					case 0x53: PC(0x00000a5c); break; // POST

					case 0x68: PC(NULL_SUB); break; // time up doesn't block long enough for pk shootout
					case 0x61: PC(0x00003b04); break; // after time up, pk shootout???
					case 0x65: PC(0x00003f36); break;

					case 0x62: PC(NULL_SUB); break; // after losing shootout, continue ???
					case 0x72: PC(0x000040ae); break; // game over

					case 0x75: PC(0x005098); break; // match adder, and check if limit is reached for ending

					// attract mode
					case 0x6e: PC(0x00010e38); break; // loop
					case 0x6b: PC(0x00010efc); break; // attract even
					case 0x69: PC(0x0001121a); break; // attract odd

					default:
						LOGPROTECTION("Unknown MCU CMD %04x\n", mcu_data);
						PC(NULL_SUB);
						break;

					#undef NULL_SUB
				}
				break;

			// Variable displacements (newer set?)
			case TECMO_WCUP94B_MCU:

				switch (mcu_data)
				{
					#define NULL_SUB (0x00830a)
					case 0x53: PC(0x000a80); break; // POST

					case 0x68: PC(NULL_SUB); break; // time up doesn't block long enough for pk shootout
					case 0x61: PC(0x003b72); break; // after time up, pk shootout???
					case 0x65: PC(0x003fa4); break;

					case 0x62: PC(NULL_SUB); break; // after losing shootout, continue ???
					case 0x72: PC(0x411c); break; // game over

					case 0x75: PC(0x5106); break; // match adder, and check if limit is reached for ending

					// attract mode
					case 0x6e: PC(0x00010ef0); break; // loop
					case 0x6b: PC(0x00010fb4); break; // attract even
					case 0x69: PC(0x000112d2); break; // attract odd

					default:
						LOGPROTECTION("Unknown MCU CMD %04x\n", mcu_data);
						PC(NULL_SUB);
						break;

					#undef NULL_SUB
				}
				break;


			case VGOAL_SOCCER_MCU:
				switch (mcu_data)
				{
					case 0x33: PC(0x00063416); break; // *after game over, is this right?
					case 0x3d: PC(0x0006275c); break; // after sprite ram init, team select
					case 0x42: PC(0x0006274e); break; // after press start, init sprite ram
					case 0x43: PC(0x0006a000); break; // POST
					case 0x50: PC(0x00001900); break; // enter main loop
					case 0x65: PC(0x0006532c); break; // results
					case 0x70: PC(0x00063416); break; // *attract loop ends, what should happen after "standings" display?
					case 0x74: PC(0x000650d8); break; // after time up, show scores and continue
					case 0x79: PC(0x0006072e); break; // after select, start match

					default:
						LOGPROTECTION("Unknown MCU CMD %04x\n",mcu_data);
						PC(0x00000586); // rts
						break;
				}
				break;
		}
	}
}

/*
    vgoalsoc uses a set of programmable timers.
    There is a code implementation for at 00065f00 that appears to have
    been RTSed out.
    I'm guessing it was replaced with an external implementation.

    This does indicate though that the protection could be performing
    other more complicated functions.

    The tick count is usually set to 0x3c => it's driven off vblank?
    More likely these timers are driven entirely by the MCU.
*/
//m_work_ram[ (0xffe900 - 0xffc00) ]
#define COUNTER1_ENABLE m_work_ram[0x2900 / 2] >> 8
#define COUNTER2_ENABLE (m_work_ram[0x2900 / 2] & 0xff)
#define TICK_1 m_work_ram[0x2908 / 2]
#define TICKCOUNT_1 m_work_ram[0x290a / 2]
#define TICK_2 m_work_ram[0x290c / 2]
#define TICKCOUNT_3 m_work_ram[0x290e / 2]
#define COUNTER_1 m_work_ram[0x2928 / 2]
#define COUNTER_2 m_work_ram[0x292a / 2]
uint16_t gstriker_state::vbl_toggle_r()
{
	return 0xff;
}

void gstriker_state::vbl_toggle_w(uint16_t data)
{
	if (COUNTER1_ENABLE == 1)
	{
		TICK_1 = (TICK_1 - 1) & 0xff;   // 8bit
		if (TICK_1 <= 0)
		{
			TICK_1 = TICKCOUNT_1;
			COUNTER_1 = (COUNTER_1 - 1);// & 0xff; has to be 16bit for continue timer.
		}
	}

	if (COUNTER2_ENABLE == 2)
	{
		TICK_2  = (TICK_2 - 1) & 0xff;
		if (TICK_2 <= 0)
		{
			TICK_2 = TICKCOUNT_3;
			COUNTER_2 = (COUNTER_2 - 1);// & 0xff;
		}
	}
}

void gstriker_state::mcu_init()
{
	save_item(NAME(m_prot_reg));
}

void gstriker_state::init_twcup94()
{
	m_gametype = TECMO_WCUP94_MCU;
	mcu_init();
}

void gstriker_state::init_twcup94a()
{
	m_gametype = TECMO_WCUP94A_MCU;
	mcu_init();
}

void gstriker_state::init_twcup94b()
{
	m_gametype = TECMO_WCUP94B_MCU;
	mcu_init();
}

void gstriker_state::init_vgoalsoc()
{
	m_gametype = VGOAL_SOCCER_MCU;
	mcu_init();

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x200090, 0x200091, write16smo_delegate(*this, FUNC(gstriker_state::vbl_toggle_w))); // vblank toggle
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x200090, 0x200091, read16smo_delegate(*this, FUNC(gstriker_state::vbl_toggle_r)));
}

} // anonymous namespace


/*** GAME DRIVERS ************************************************************/

GAME( 1993, gstriker,  0,        gstriker, gstriker, gstriker_state, empty_init, ROT0, "Human", "Grand Striker (Europe, Oceania)", MACHINE_NOT_WORKING | MACHINE_NODEVICE_LAN | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1993, gstrikera, gstriker, gstriker, gstriker, gstriker_state, empty_init, ROT0, "Human", "Grand Striker (Americas)",        MACHINE_NOT_WORKING | MACHINE_NODEVICE_LAN | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1993, gstrikerj, gstriker, gstriker, gstriker, gstriker_state, empty_init, ROT0, "Human", "Grand Striker (Japan)",           MACHINE_NOT_WORKING | MACHINE_NODEVICE_LAN | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

// Similar, but not identical hardware, appear to be protected by an MCU
GAME( 1994, vgoalsoc,  0,        vgoal, vgoalsoc, gstriker_state, init_vgoalsoc, ROT0, "Tecmo", "V Goal Soccer (Europe)",         MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // has ger/hol/arg/bra/ita/eng/spa/fra
GAME( 1994, vgoalsoca, vgoalsoc, vgoal, vgoalsoc, gstriker_state, init_vgoalsoc, ROT0, "Tecmo", "V Goal Soccer (US/Japan/Korea)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // has ger/hol/arg/bra/ita/kor/usa/jpn
GAME( 1994, twcup94,   0,        twc94, twcup94,  gstriker_state, init_twcup94,  ROT0, "Tecmo", "Tecmo World Cup '94 (set 1)",    MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, twcup94a,  twcup94,  twc94, twcup94,  gstriker_state, init_twcup94a, ROT0, "Tecmo", "Tecmo World Cup '94 (set 2)",    MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, twcup94b,  twcup94,  twc94, twcup94,  gstriker_state, init_twcup94b, ROT0, "Tecmo", "Tecmo World Cup '94 (set 3)",    MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
