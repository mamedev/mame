// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi

/* PGM System (c)1997 IGS

Based on Information from ElSemi

A flexible cartridge based platform some would say was designed to compete with
SNK's NeoGeo and Capcom's CPS Hardware systems, despite its age it only uses a
68000 for the main processor and a Z80 to drive the sound, just like the two
previously mentioned systems in that respect..

Resolution is 448x224, 15 bit colour

Sound system is ICS WaveFront 2115 Wavetable midi synthesizer, used in some
actual sound cards (Turtle Beach)

Later games are encrypted.  Latest games (kov2, ddp2) include an arm7
coprocessor with an internal rom and an encrypted external rom.

Roms Contain the Following Data

Pxxxx - 68k Program
Txxxx - TX & BG Graphics (2 formats within the same rom)
Mxxxx - Music samples (8 bit mono 11025Hz)
Axxxx - Colour Data (for sprites)
Bxxxx - Masks & A Rom Colour Indexes (for sprites)

There is no rom for the Z80, the program is uploaded by the 68k

Known Games on this Platform
----------------------------


010x  - 1997  - Oriental Legend
020x  - 1997  - Dragon World 2
030x  - 1998  - The Killing Blade
040x  - 1998  - Dragon World 3
050x  - 1999? - Oriental Legend Super
060x  - 1999  - Knights of Valor, Knights of Valor Plus, Knights of Valor Super Heroes
070x  - 1999  - Photo Y2k
080x  - 1999  - Puzzle Star
090x  - 2001  - Puzzli II
100x  - 2001  - Martial Masters

120x  - 2001  - Knights of Valor 2 Plus (9 Dragons?)
130x  - 2001  - DoDonpachi II

0450x - 2002  - Demon Front (also known to be produced / sold in a single PCB version)

(add more)

TODO:

- IRQ4 generation - Puzzli 2 doesn't like this, many other games require it for coins / inputs to work.
- Verify/Implement unemulated vreg usage (b04000, b0e000)
  b04000 is 210h or 610h, bit 10 unknown
  b0e000 bit 2/0 related to VRAM access / DMA / or others?
- Zooming function is not fully verified; sprite zoom table is bit different in drgw2


Protection Devices / Co-processors
----------------------------------

IGS used a variety of additional ASIC chips on the game boards, these act as protection and
also give additional power to the board to make up for the limited power of the 68000
processor.  Some protection devices use external data roms, others have internal code only.
Most of these are not emulated correctly. In most cases the protection device supplies the
game region code..

ASIC 3:
    state based device?
    see
    machine/pgmprot_orlegend.cpp

ASIC 25 + ASIC 12
    state based device + rom overlays
    see
    machine/pgmprot_igs025_igs012.cpp

ASIC 25 + ASIC 22
    state based device + encrypted DMA device
    see
    machine/pgmprot_igs025_igs022.cpp

ASIC 25 + ASIC 28
    state based device + encrypted DMA device?
    see
    machine/pgmprot_igs025_igs028.cpp

ASIC 027A(55857F/55857G):
    ARM based CPUs with internal ROM
    see
    machine/pgmprot_igs027a_type1.cpp
    machine/pgmprot_igs027a_type2.cpp
    machine/pgmprot_igs027a_type3.cpp


there are probably more...

PCB Layout
----------

IGS PCB NO-0133-2 (Main Board)
|-------------------------------------------------------------------------------------|
|   |----------------------------|   |----------|   |----------------------------|    |
|   |----------------------------|   |----------|   |----------------------------|    |
|                                      PGM_T01S.U29  UM61256    SRM2B61256  SRM2B61256|
| |---------|  33.8688MHz   |----------|                        SRM2B61256  SRM2B61256|
| |WAVEFRONT|               |L8A0290   |   UM6164  UM6164                             |
| |ICS2115V |               |IGS023    |                 PGM_P01S.U20              SW2|
| |(PLCC84) |               |(QFP256)  |                                              |
| |         |               |          |                                              |
| |---------|        50MHz  |----------|                                              |
|    UPD6379  PGM_M01S.U18                             |----------|                   |
|VOL                                                   |MC68HC000 |          74HC132  |
|                                                      |FN20      |   20MHz  74HC132  |
|  UPC844C    |------|                                 |(PLCC68)  |                   |
|             |Z80   |                                 |          |          V3021    |
|             |PLCC44|                  PAL            |----------|                   |
|             |------|    |--------|                                      32.768kHz   |-|
|                         |IGS026  |                                                    |
|                         |(QFP144)|           |--------|                              I|
|                         |        |           |IGS026  |                              D|
|                         |--------|           |(QFP144)|                              C|
|TDA1519A    UM61256 UM61256                   |        |                              3|
|                              TD62064         |--------|                              4|
|                                                                          3.6V_BATT    |
|                                                                                     |-|
|              |----|                                           |-----|     SW3       |
|              |    |               J  A  M  M  A               |     | SW1           |
|--------------|    |-------------------------------------------|     |---------------|


IGS PCB NO-0136 (Riser)
|-------------------------------------------------------------------------------------|
|      |---------------------------------|  |---------------------------------|       |
|      |---------------------------------|  |---------------------------------|       |
|                                                                                     |
|      |---------------------------------|  |---------------------------------|       |
|      |---------------------------------|  |---------------------------------|       |
|                                                                                     |
|   |----------------------------|   |----------|   |----------------------------|    |
|---|                            |---|          |---|                            |----|
    |----------------------------|   |----------|   |----------------------------|

Notes:
      All IC's are shown.

      CPU's
      -----
         68HC000FN20 - Motorola 68000 processor, clocked at 20.000MHz (PLCC68)
         Z80         - Zilog Z0840008VSC Z80 processor, clocked at 8.468MHz (PLCC44)

      SOUND
      -----
         ICS2115     - ICS WaveFront ICS2115V Wavetable Midi Synthesizer, clocked at 33.8688MHz (PLCC84)

      RAM
      ---
         SRM2B256 - Epson SRM2B256SLMX55 8K x8 SRAM (x4, SOP28)
         UM6164   - Unicorn Microelectronics UM6164DS-12 8K x8 SRAM (x2, SOJ28)
         UM61256  - Unicorn Microelectronics UM61256FS-15 32K x8 SRAM (x3, SOJ28)

      ROMs
      ----
         PGM_M01S.U18 - 16MBit mask ROM (TSOP48)
         PGM_P01S.U20 - 1MBit  mask ROM (DIP40, socketed, equivalent to 27C1024 EPROM)
         PGM_T01S.U29 - 16MBit mask ROM (SOP44)

      CUSTOM IC's
      -----------
         IGS023 (QFP256)
         IGS026 (x2, QFP144)

      OTHER
      -----
         3.6V_BATT - 3.6V NICad battery, connected to the V3021 RTC
         IDC34     - IDC34 way flat cable plug, PGM can support 4 players max in two cabs,
                     this is jamma connector for another cab and the P3&P4
         PAL       - Atmel ATF16V8B PAL (DIP20)
         SW1       - Push button switch to enter Test Mode
         SW2       - 8 position DIP Switch (for configuration of PCB/game options)
         SW3       - SPDT switch , to clear the NVRAM and reset the whole system
         TD62064   - Toshiba NPN 50V 1.5A Quad Darlington Switch; for driving coin meters (DIP16)
         TDA1519A  - Philips 2x 6W Stereo Power AMP (SIL9)
         uPD6379   - NEC 2-channel 16-bit D/A converter 10mW typ. (SOIC8)
         uPC844C   - NEC Quad High Speed Wide Band Operational Amplifier (DIP14)
         V3021     - EM Microelectronic-Marin SA Ultra Low Power 32kHz CMOS Real Time Clock (DIP8)
         VOL       - Volume potentiometer

*/

#include "emu.h"
#include "pgm.h"

#include "pgmprot_igs025_igs012.h"
#include "pgmprot_igs025_igs022.h"
#include "pgmprot_igs025_igs028.h"
#include "pgmprot_igs027a_type1.h"
#include "pgmprot_igs027a_type2.h"
#include "pgmprot_igs027a_type3.h"
#include "pgmprot_orlegend.h"

#include "screen.h"
#include "speaker.h"

#define LOG_Z80     (1U << 1)

#define LOG_ALL     (LOG_Z80)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGZ80(...) LOGMASKED(LOG_Z80, __VA_ARGS__)

void pgm_state::coin_counter_w(u16 data)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x0001);
	machine().bookkeeping().coin_counter_w(1, data & 0x0002);
	machine().bookkeeping().coin_counter_w(2, data & 0x0004);
	machine().bookkeeping().coin_counter_w(3, data & 0x0008);
}

u8 pgm_state::z80_ram_r(offs_t offset)
{
	return m_z80_mainram[offset];
}

void pgm_state::z80_ram_w(offs_t offset, u8 data)
{
	const int pc = m_maincpu->pc();

	m_z80_mainram[offset] = data;

	if (pc != 0xf12 && pc != 0xde2 && pc != 0x100c50 && pc != 0x100b20)
		LOGZ80("Z80: write %04x, %02x (%06x)\n", offset, data, m_maincpu->pc());
}

void pgm_state::z80_reset_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGZ80("Z80: reset %04x @ %04x (%06x)\n", data, mem_mask, m_maincpu->pc());

	if (data == 0x5050)
	{
		m_ics->reset();
		m_soundcpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_soundcpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	}
	else
	{
		/* this might not be 100% correct, but several of the games (ddp2, puzzli2 etc.) expect the z80 to be turned
		off during data uploads, they write here before the upload */
		m_soundcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
}

void pgm_state::z80_ctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGZ80("Z80: ctrl %04x @ %04x (%06x)\n", data, mem_mask, m_maincpu->pc());
}

void pgm_state::m68k_l1_w(u8 data)
{
	LOGZ80("SL 1 m68.w %02x (%06x) IRQ\n", data, m_maincpu->pc());
	m_soundlatch->write(data);
	m_soundcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void pgm_state::z80_l3_w(u8 data)
{
	LOGZ80("SL 3 z80.w %02x (%04x)\n", data, m_soundcpu->pc());
	m_soundlatch3->write(data);
}


/*** Memory Maps *************************************************************/

/*** Z80 (sound CPU)**********************************************************/

void pgm_state::pgm_z80_mem(address_map &map)
{
	map(0x0000, 0xffff).ram().share("z80_mainram");
}

void pgm_state::pgm_z80_io(address_map &map)
{
	map(0x8000, 0x8003).rw("ics", FUNC(ics2115_device::read), FUNC(ics2115_device::write));
	map(0x8100, 0x81ff).r(m_soundlatch3, FUNC(generic_latch_8_device::read)).w(FUNC(pgm_state::z80_l3_w));
	map(0x8200, 0x82ff).rw(m_soundlatch, FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::write));
	map(0x8400, 0x84ff).rw("soundlatch2", FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::write));
}

/*** 68000 (main CPU) + variants for protection devices **********************/

void pgm_state::pgm_base_mem(address_map &map)
{
	map(0x700006, 0x700007).nopw(); // Watchdog?

	map(0x800000, 0x81ffff).ram().mirror(0x0e0000).share("sram"); /* Main Ram */

	map(0x900000, 0x907fff).mirror(0x0f8000).rw(m_video, FUNC(igs023_video_device::videoram_r), FUNC(igs023_video_device::videoram_w)); /* IGS023 VIDEO CHIP */
	map(0xa00000, 0xa011ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xb00000, 0xb0ffff).rw(m_video, FUNC(igs023_video_device::videoregs_r), FUNC(igs023_video_device::videoregs_w)); /* Video Regs inc. Zoom Table */
	//map(0xb01000, 0xb0103f) Zoom/Shrink table for sprites
	//map(0xb02000, 0xb02001) Background scroll Y
	//map(0xb03000, 0xb03001) Background scroll X
	//map(0xb04000, 0xb04001) Unknown #0
	//map(0xb05000, 0xb05001) Foreground scroll Y
	//map(0xb06000, 0xb06001) Foreground scroll X?
	//map(0xb0e000, 0xb0e001) Unknown #1

	map(0xc00003, 0xc00003).r(m_soundlatch, FUNC(generic_latch_8_device::read)).w(FUNC(pgm_state::m68k_l1_w));
	map(0xc00005, 0xc00005).rw("soundlatch2", FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::write));
	map(0xc00007, 0xc00007).rw("rtc", FUNC(v3021_device::read), FUNC(v3021_device::write));
	map(0xc00008, 0xc00009).w(FUNC(pgm_state::z80_reset_w));
	map(0xc0000a, 0xc0000b).w(FUNC(pgm_state::z80_ctrl_w));
	map(0xc0000d, 0xc0000d).rw(m_soundlatch3, FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::write));

	map(0xc08000, 0xc08001).portr("P1P2");
	map(0xc08002, 0xc08003).portr("P3P4");
	map(0xc08004, 0xc08005).portr("Service");
	map(0xc08006, 0xc08007).portr("DSW").w(FUNC(pgm_state::coin_counter_w));

	map(0xc10000, 0xc1ffff).rw(FUNC(pgm_state::z80_ram_r), FUNC(pgm_state::z80_ram_w)); /* Z80 Program */
}

void pgm_state::pgm_mem(address_map &map)
{
	pgm_base_mem(map);
	map(0x000000, 0x0fffff).rom();   /* BIOS ROM */
}

void pgm_state::pgm_basic_mem(address_map &map)
{
	pgm_mem(map);
	map(0x100000, 0x3fffff).bankr("bank1"); /* Game ROM */
}


/*** Input Ports *************************************************************/

/* enough for 4 players, the basic dips mapped are listed in the test mode */

INPUT_PORTS_START( pgm )
	PORT_START("P1P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("P3P4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)

	PORT_START("Service")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Test-Key 1P+2P")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service 1P+2P")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("Test-Key 3P+4P")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Service 3P+4P")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?

	PORT_START("DSW")
	PORT_SERVICE( 0x0001, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0002, 0x0002, "Music" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Voice" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Free" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Stop" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("Region")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/*** Machine Driver **********************************************************/

/* most games require IRQ4 for inputs to work, Puzzli 2 is explicit about not wanting it tho
   what is the source? */
TIMER_DEVICE_CALLBACK_MEMBER(pgm_state::interrupt)
{
	int scanline = param;

// already being generated by screen.screen_vblank().set(FUNC(pgm_state::screen_vblank));
//  if (scanline == 224)
//      m_maincpu->set_input_line(6, HOLD_LINE);

//  vblank end interrupt
	if (scanline == 0)
		if (!m_irq4_disabled) m_maincpu->set_input_line(4, HOLD_LINE);
}

void pgm_state::machine_reset()
{
	m_soundcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

u16 pgm_state::sprites_r(offs_t offset)
{
	return m_mainram[offset];
}

void pgm_state::video_start()
{
}

void pgm_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		/* first 0xa00 of main ram = sprites, seems to be buffered, DMA? */
		m_video->get_sprites();

		// vblank start interrupt
		m_maincpu->set_input_line(M68K_IRQ_6, HOLD_LINE);
	}
}

void pgm_state::pgmbase(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL); /* 20 mhz! verified on real board */
	m_maincpu->set_addrmap(AS_PROGRAM, &pgm_state::pgm_basic_mem);
	TIMER(config, "scantimer").configure_scanline(FUNC(pgm_state::interrupt), "screen", 0, 1);

	Z80(config, m_soundcpu, 33.8688_MHz_XTAL/4);
	m_soundcpu->set_addrmap(AS_PROGRAM, &pgm_state::pgm_z80_mem);
	m_soundcpu->set_addrmap(AS_IO, &pgm_state::pgm_z80_io);

	NVRAM(config, "sram", nvram_device::DEFAULT_ALL_0);

	V3021(config, "rtc");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(50_MHz_XTAL/5, 640, 0, 448, 264, 0, 224); // or 20MHz / 2? framerate verified
	screen.set_screen_update(m_video, FUNC(igs023_video_device::screen_update));
	screen.screen_vblank().set(FUNC(pgm_state::screen_vblank));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::xRGB_555, 0x1200/2);

	/*sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, "soundlatch2");
	GENERIC_LATCH_8(config, m_soundlatch3);

	IGS023_VIDEO(config, m_video, 0);
	m_video->set_palette(m_palette);
	m_video->read_spriteram_callback().set(FUNC(pgm_state::sprites_r));

	ICS2115(config, m_ics, 33.8688_MHz_XTAL);
	m_ics->irq().set_inputline("soundcpu", 0);
	m_ics->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void pgm_state::pgm(machine_config &config)
{
	pgmbase(config);
}


/*** ROM Loading *************************************************************/

/* take note of "igs023:sprmask" needed for expanding the Sprite Colour Data */

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios))

#define PGM_68K_BIOS \
	ROM_SYSTEM_BIOS( 0, "v2",     "PGM BIOS V2" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "pgm_p02s.u20",    0x00000, 0x020000, CRC(78c15fa2) SHA1(885a6558e022602cc6f482ac9667ba9f61e75092) ) /* Version 2 (Label: IGS | PGM P02S | 1P0792D1 | J992438 )*/ \
	ROM_SYSTEM_BIOS( 1, "v1",     "PGM BIOS V1" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "pgm_p01s.u20",    0x00000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) ) /* Version 1 */
#define PGM_AUDIO_BIOS \
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) )
#define PGM_VIDEO_BIOS \
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) )

/* The BIOS - NOT A GAME */
ROM_START( pgm )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS

	ROM_REGION( 0x280000, "igs023", 0 ) /* 8x8 Text Layer Tiles */
	PGM_VIDEO_BIOS

	ROM_REGION( 0x200000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS

	ROM_REGION16_LE( 0x1000000, "igs023:sprcol", ROMREGION_ERASEFF ) /* Sprite Colour Data */
	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", ROMREGION_ERASEFF ) /* Sprite Masks + Colour Indexes */
ROM_END

ROM_START( orlegend )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0103.rom",    0x100000, 0x200000, CRC(d5e93543) SHA1(f081edc26514ca8354c13c7f6f89aba8e4d3e7d2) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0100.u8",     0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0100.u5",     0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "pgm_a0101.u6",     0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "pgm_a0102.u7",     0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "pgm_a0103.u8",     0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "pgm_a0104.u11",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "pgm_a0105.u12",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0100.u9",     0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "pgm_b0101.u10",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "pgm_b0102.u15",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0100.u1",     0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

ROM_START( orlegende )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "pgm_p0102.u2",    0x100000, 0x200000, CRC(4d0f6cc5) SHA1(8d41f0a712fb11a1da865f5159e5e27447b4388a) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0100.u8",     0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0100.u5",     0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "pgm_a0101.u6",     0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "pgm_a0102.u7",     0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "pgm_a0103.u8",     0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "pgm_a0104.u11",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "pgm_a0105.u12",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0100.u9",     0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "pgm_b0101.u10",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "pgm_b0102.u15",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0100.u1",     0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

ROM_START( orlegendc )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "pgm_p0101.u2",    0x100000, 0x200000, CRC(b24f0c1e) SHA1(a2cf75d739681f091c24ef78ed6fc13aa8cfe0c6) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0100.u8",     0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	/* Different labels comparing to others sets except for PGM B0102 in U15 and PGM M0100 in U1; it probably needs to be dumped */
	ROM_LOAD( "pgm_a0100-1.u5",   0x0000000, 0x400000, BAD_DUMP CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "pgm_a0101-1.u6",   0x0400000, 0x400000, BAD_DUMP CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "pgm_a0102-1.u7",   0x0800000, 0x400000, BAD_DUMP CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "pgm_a0103-1.u8",   0x0c00000, 0x400000, BAD_DUMP CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "pgm_a0104-1.u11",  0x1000000, 0x400000, BAD_DUMP CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "pgm_a0105-1.u12",  0x1400000, 0x400000, BAD_DUMP CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0100-1.u9",   0x0000000, 0x400000, BAD_DUMP CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "pgm_b0101-1.u10",  0x0400000, 0x400000, BAD_DUMP CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "pgm_b0102.u15",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0100.u1",     0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

ROM_START( orlegendca )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0101.102",    0x100000, 0x200000, CRC(7a22e1cb) SHA1(4fe0fde00521b0915146334ea7213f3eb7e2affc) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0100.u8",     0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0100.u5",     0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "pgm_a0101.u6",     0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "pgm_a0102.u7",     0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "pgm_a0103.u8",     0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "pgm_a0104.u11",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "pgm_a0105.u12",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0100.u9",     0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "pgm_b0101.u10",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "pgm_b0102.u15",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0100.u1",     0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END


/*

Oriental Legend / Xi You Shi E Zhuan (CHINA 111 Ver.)
(c)1997 IGS

Top board
---------
PCB Number: IGS PCB NO-0134-2

OLV 111 CHINA U6.U6   [5fb86373]
OLV 111 CHINA U7.U7   [6ee79faf]
OLV 111 CHINA U9.U9   [83cf09c8]
OLV 111 CHINA U11.U11 [b80ddd3c]

PGM T0100.U8

Bottom Board
------------
PCB Number: IGS PCB NO-0135

PGM A0100.U5
PGM A0101.U6
PGM A0102.U7
PGM A0103.U8
PGM A0104.U11
PGM A0105.U12

PGM B0100.U9
PGM B0101.U10
PGM B0102.U15

PGM M0100.U1

*/

ROM_START( orlegend111c )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "olv_111_china_u6.u6",     0x100001, 0x080000, CRC(5fb86373) SHA1(2fc58eff1f38754c75819fde666244b867ca4f05) )
	ROM_LOAD16_BYTE( "olv_111_china_u9.u9",     0x100000, 0x080000, CRC(83cf09c8) SHA1(959780b45326059517f3008a356657f4f3d2908f) )
	ROM_LOAD16_BYTE( "olv_111_china_u7.u7",     0x200001, 0x080000, CRC(6ee79faf) SHA1(039b4b07b8577f0d3022ae01210c00375624cb3c) )
	ROM_LOAD16_BYTE( "olv_111_china_u11.u11",   0x200000, 0x080000, CRC(b80ddd3c) SHA1(55c700ce71ffdee392e03fd9d4719542c3527132) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0100.u8",     0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0100.u5",     0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "pgm_a0101.u6",     0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "pgm_a0102.u7",     0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "pgm_a0103.u8",     0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "pgm_a0104.u11",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "pgm_a0105.u12",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0100.u9",     0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "pgm_b0101.u10",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "pgm_b0102.u15",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0100.u1",     0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

/*

Oriental Legend / Xi You Shi E Zhuan (TAIWAN 111 Ver.)
(c)1997 IGS

Top board
---------
PCB Number: IGS PCB NO-0134-2

OLV 111 TAIWAN U6.U6   [b205a733]
OLV 111 TAIWAN U7.U7   [27628e87]
OLV 111 TAIWAN U9.U9   [6d9d29b4]
OLV 111 TAIWAN U11.U11 [23f33bc9]

PGM T0100.U8

Bottom Board
------------
PCB Number: IGS PCB NO-0135-1

PGM A0100B.U5
PGM A0101B.U8
PGM A0102B.U6
PGM A0103B.U9
PGM A0104B.U7
PGM A0105B.U10

PGM B0100B.U11
PGM B0101B.U12
PGM B0102.U2

PGM M0100.U1

*/

ROM_START( orlegend111t )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "olv_111_taiwan_u6.u6",     0x100001, 0x080000, CRC(b205a733) SHA1(33f4c9162e36be4957004f80593f94fc33b163f8) )
	ROM_LOAD16_BYTE( "olv_111_taiwan_u9.u9",     0x100000, 0x080000, CRC(6d9d29b4) SHA1(29a18de7e5b58c2f3125d6dc9cc8a8186180e956) )
	ROM_LOAD16_BYTE( "olv_111_taiwan_u7.u7",     0x200001, 0x080000, CRC(27628e87) SHA1(a0effd83dc57ac72ba4f110737a075705d78e798) )
	ROM_LOAD16_BYTE( "olv_111_taiwan_u11.u11",    0x200000, 0x080000, CRC(23f33bc9) SHA1(f24490370d40d905afe8b716a3953b4e9f0aada4) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0100.u8",     0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	/* Different labels comparing to others sets except for PGM B0102 in U2 and PGM M0100 in U1; it probably needs to be dumped */
	ROM_LOAD( "pgm_a0100b.u5",    0x0000000, 0x400000, BAD_DUMP CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "pgm_a0101b.u8",    0x0400000, 0x400000, BAD_DUMP CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "pgm_a0102b.u6",    0x0800000, 0x400000, BAD_DUMP CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "pgm_a0103b.u9",    0x0c00000, 0x400000, BAD_DUMP CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "pgm_a0104b.u7",    0x1000000, 0x400000, BAD_DUMP CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "pgm_a0105b.u10",   0x1400000, 0x400000, BAD_DUMP CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0100b.u11",   0x0000000, 0x400000, BAD_DUMP CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "pgm_b0101b.u12",   0x0400000, 0x400000, BAD_DUMP CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "pgm_b0102.u2",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0100.u1",     0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END


/*

Oriental Legend / Xi You Shi E Zhuan (KOREA 111 Ver.)
(c)1997 IGS

Top board
---------
PCB Number: IGS PCB NO-0134-2

OLV 111 KOREA U6.U6   [1ff35baa]
OLV 111 KOREA U7.U7   [27628e87]
OLV 111 KOREA U9.U9   [87b6d202]
OLV 111 KOREA U11.U11 [23f33bc9]

PGM T0100.U8

Bottom Board
------------
PCB Number: IGS PCB NO-0135

PGM A0100.U5
PGM A0101.U6
PGM A0102.U7
PGM A0103.U8
PGM A0104.U11
PGM A0105.U12

PGM B0100.U9
PGM B0101.U10
PGM B0102.U15

PGM M0100.U1

*/

ROM_START( orlegend111k )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "olv_111_korea_u6.u6",     0x100001, 0x080000, CRC(1ff35baa) SHA1(f6791cc37ea468d0154b0e31a99f6f33a74bca81) )
	ROM_LOAD16_BYTE( "olv_111_korea_u9.u9",     0x100000, 0x080000, CRC(87b6d202) SHA1(32828166a645158630f79e7d493a2774e69bc265) )
	ROM_LOAD16_BYTE( "olv_111_korea_u7.u7",     0x200001, 0x080000, CRC(27628e87) SHA1(a0effd83dc57ac72ba4f110737a075705d78e798) )
	ROM_LOAD16_BYTE( "olv_111_korea_u11.u11",   0x200000, 0x080000, CRC(23f33bc9) SHA1(f24490370d40d905afe8b716a3953b4e9f0aada4) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0100.u8",     0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0100.u5",     0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "pgm_a0101.u6",     0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "pgm_a0102.u7",     0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "pgm_a0103.u8",     0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "pgm_a0104.u11",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "pgm_a0105.u12",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0100.u9",     0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "pgm_b0101.u10",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "pgm_b0102.u15",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0100.u1",     0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END


/*

Oriental Legend / Xi You Shi E Zhuan (TAIWAN 105 Ver.)
(c)1997 IGS

Top board
---------
PCB Number: IGS PCB NO-0134-2

OLV 105 TAIWAN U6.U6   [2b14331f]
OLV 105 TAIWAN U7.U7   [5712facc]
OLV 105 TAIWAN U9.U9   [ae9d71e4]
OLV 105 TAIWAN U11.U11 [40ae4d9e]

PGM T0100.U8

Bottom Board
------------
PCB Number: IGS PCB NO-0135

PGM A0100.U5
PGM A0101.U6
PGM A0102.U7
PGM A0103.U8
PGM A0104.U11
PGM A0105.U12

PGM B0100.U9
PGM B0101.U10
PGM B0102.U15

PGM M0100.U1

*/

ROM_START( orlegend105t )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "olv_105_taiwan_u6.u6",     0x100001, 0x080000, CRC(2b14331f) SHA1(3909b5480764916761895ad3f25dd4f40762a2d8) )
	ROM_LOAD16_BYTE( "olv_105_taiwan_u9.u9",     0x100000, 0x080000, CRC(ae9d71e4) SHA1(20f84c18808a80464f22c72eafe7680690274f39) )
	ROM_LOAD16_BYTE( "olv_105_taiwan_u7.u7",     0x200001, 0x080000, CRC(5712facc) SHA1(2d95ebd1703874e89ac3a206f8c1f0ece6e833e0) )
	ROM_LOAD16_BYTE( "olv_105_taiwan_u11.u11",   0x200000, 0x080000, CRC(40ae4d9e) SHA1(62d7a96438b7fe93f74753333f50e077d417971e) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0100.u8",     0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0100.u5",     0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "pgm_a0101.u6",     0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "pgm_a0102.u7",     0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "pgm_a0103.u8",     0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "pgm_a0104.u11",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "pgm_a0105.u12",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0100.u9",     0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "pgm_b0101.u10",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "pgm_b0102.u15",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0100.u1",     0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

/*

Oriental Legend / Xi You Shi E Zhuan (KOREA 105 Ver.)
(c)1997 IGS

Top board
---------
PCB Number: IGS PCB NO-0134-2
IGS PCB NO-0135

OLV 105 KOREA U6.U6   [b86703fe]
OLV 105 KOREA U7.U7   [5712facc]
OLV 105 KOREA U9.U9   [5a108e39]
OLV 105 KOREA U11.U11 [40ae4d9e]

PGM T0100.U8

Bottom Board
------------
PCB Number: IGS PCB NO-0135

PGM A0100.U5
PGM A0101.U6
PGM A0102.U7
PGM A0103.U8
PGM A0104.U11
PGM A0105.U12

PGM B0100.U9
PGM B0101.U10
PGM B0102.U15

PGM M0100.U1

*/

ROM_START( orlegend105k )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "olv_105_korea_u6.u6",     0x100001, 0x080000, CRC(b86703fe) SHA1(a3529b45efd400ecd5e76f764b528ebce46e24ab) )
	ROM_LOAD16_BYTE( "olv_105_korea_u9.u9",     0x100000, 0x080000, CRC(5a108e39) SHA1(2033f4fe3f2dfd725dac535324f58348b9ac3914) )
	ROM_LOAD16_BYTE( "olv_105_korea_u7.u7",     0x200001, 0x080000, CRC(5712facc) SHA1(2d95ebd1703874e89ac3a206f8c1f0ece6e833e0) )
	ROM_LOAD16_BYTE( "olv_105_korea_u11.u11",   0x200000, 0x080000, CRC(40ae4d9e) SHA1(62d7a96438b7fe93f74753333f50e077d417971e) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0100.u8",     0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0100.u5",     0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "pgm_a0101.u6",     0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "pgm_a0102.u7",     0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "pgm_a0103.u8",     0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "pgm_a0104.u11",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "pgm_a0105.u12",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0100.u9",     0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "pgm_b0101.u10",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "pgm_b0102.u15",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0100.u1",     0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

/*

Dragon World 2 (English / World Version)
IGS, 1997

This is a cart for the IGS PGM system.


PCB Layout
----------

IGS PCB NO-0162
|-----------------------------------------------|
| |------|                                      |
| |IGS012|       *1                    T0200.U7 |
| |      |                                      |
| |------|                                      |
|              |--------|                       |
|              |        |                       |
|              | IGS025 |  *2   V-110X.U2       |
| PAL    PAL   |        |                  PAL  |
|              |--------|                       |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      IGS012       - Custom IGS IC (QFP80)

      -- on english version
      IGS025       - Custom IGS IC (PLCC68, labelled "DRAGON II 0006")
      -- on china version
      IGS025       - Custom IGS IC (PLCC68, labelled "DRAGON II 0005")


      T0200.U7     - 32MBit MaskROM (SOP44)

      -- on english version
      V-110X.U2    - AM27C4096 4MBit EPROM (DIP42, labelled "DRAGON II V-110X")
      -- on china version
      V-110X.U2    - AM27C4096 4MBit EPROM (DIP42, labelled "DRAGON II V-100C")

      PALs         - x3, labelled "CZ U3", "CZ U4", "CZ U6"
      *1           - Unpopulated position for MX23C4100 SOP40 mask ROM
      *2           - Unpopulated position for MX23C4100 DIP40 EPROM/mask ROM


IGS PCB NO-0135
|-----------------------------------------------|
| U11    U12     U13      U14       U15      U16|
|                                               |
|                                               |
|A0200.U5                       B0200.U9        |
|        U6      U7       U8                 U10|
|                                               |
|                                               |
|74LS138         U1       U2             74LS139|
|                                               |
|-|                                           |-|
  |--------------------||---------------------|

Notes:
      This PCB contains only SOP44 mask ROMS and 2 logic IC's
      Only U5 and U9 are populated

      glitch on select screen exists on real board.

*/

ROM_START( drgw2 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "dragon_ii_v-110x.u2",    0x100000, 0x080000, CRC(1978106b) SHA1(af8a13d7783b755a58762c98bdc32cab845b2251) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0200.u7",    0x180000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION16_LE( 0x0400000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION16_LE( 0x0400000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
ROM_END

ROM_START( drgw2100x )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "dragon_ii_v-100x.u2",    0x100000, 0x080000,  CRC(5e71851d) SHA1(62052469f69daec88efd26652c1b893d6f981912) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0200.u7",    0x180000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION16_LE( 0x0400000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION16_LE( 0x0400000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
ROM_END

ROM_START( drgw2101c )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "dragon_ii_v-101c.u2",    0x100000, 0x080000, CRC(b0c592fa) SHA1(87ccfdb940303ebcf42cb2952aecae97648c1e0d) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0200.u7",    0x180000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION16_LE( 0x0400000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION16_LE( 0x0400000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
ROM_END

ROM_START( drgw2100c )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "dragon_ii_v-100c.u2",    0x100000, 0x080000, CRC(67467981) SHA1(58af01a3871b6179fe42ff471cc39a2161940043) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0200.u7",    0x180000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION16_LE( 0x0400000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION16_LE( 0x0400000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
ROM_END

ROM_START( drgw2100j )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "dragon_ii_v100j.u2",    0x100000, 0x080000, CRC(f8f8393e) SHA1(ef0db668b4e4f661d4c1e95d57afe881bcdf13cc) )
	// A cart has been found with same contents but ROM label on sticker is DRAGON II V101J.
	// Is this correct or wrong sticker applied?

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0200.u7",    0x180000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION16_LE( 0x0400000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION16_LE( 0x0400000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
ROM_END

ROM_START( drgw2100hk ) // the IGS025 has a "DRAGON-II 0004-1" sticker, the IGS012 has no per-game marking
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "dragon_ii_v-100-h.u2",    0x100000, 0x080000, CRC(c6e2e6ec) SHA1(84145dfb26857ea20efb233363f175bc9bb25b0c) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0200.u7",    0x180000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION16_LE( 0x0400000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION16_LE( 0x0400000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
ROM_END

/*

Dragon World 3 (CHINA 106 Ver.)
(c)1998 IGS

Top board
---------
PCB Number: IGS PCB NO-0189-1

DW3 V106 U12.U12 [c3f6838b]
DW3 V106 U13.U13 [28284e22]

DW3 TEXT U15.U15

PGM T0400.U18

Bottom Board
------------
PCB Number: IGS PCB NO-0178

PGM A0400.U9
PGM A0401.U10

PGM B0400.U13

PGM M0400.U1

*/

ROM_START( drgw3 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "dw3_v106_u12.u12",     0x100001, 0x080000,  CRC(c3f6838b) SHA1(c135b1d4dd62af308139d40d03c29be7508fb1e7) )
	ROM_LOAD16_BYTE( "dw3_v106_u13.u13",     0x100000, 0x080000,  CRC(28284e22) SHA1(4643a69881ddb7383ca10f3eb2aa2cf41be39e9f) )

	ROM_REGION( 0x010000, "igs022", 0 ) /* Protection Data - is it correct for this set? */
	ROM_LOAD( "dw3_text_u15.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0400.u18",   0x180000, 0x400000, CRC(b70f3357) SHA1(8733969d7d21f540f295a9f747a4bb8f0d325cf0) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0401.u10",    0x0400000, 0x400000, CRC(cab6557f) SHA1(1904dd86645eea27ac1ab8a2462b20f6531356f8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0400.u1",  0x400000, 0x400000, CRC(031eb9ce) SHA1(0673ec194732becc6648c2ae1396e894aa269f9a) )
ROM_END


ROM_START( drgw3105 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "dw3_v105_u12.u12",     0x100001, 0x080000,  CRC(c5e24318) SHA1(c6954495bbc72c3985df75aecf6afd6826c8e30e) )
	ROM_LOAD16_BYTE( "dw3_v105_u13.u13",     0x100000, 0x080000,  CRC(8d6c9d39) SHA1(cb79303ab551e91f07e11414db4254d5b161d415) )

	ROM_REGION( 0x010000, "igs022", 0 ) /* Protection Data - is it correct for this set? */
	ROM_LOAD( "dw3_v100_u15.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0400.u18",   0x180000, 0x400000, CRC(b70f3357) SHA1(8733969d7d21f540f295a9f747a4bb8f0d325cf0) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0401.u10",    0x0400000, 0x400000, CRC(cab6557f) SHA1(1904dd86645eea27ac1ab8a2462b20f6531356f8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0400.u1",  0x400000, 0x400000, CRC(031eb9ce) SHA1(0673ec194732becc6648c2ae1396e894aa269f9a) )
ROM_END

ROM_START( drgw3103 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "dragon_iii_v103j_u12.u12",     0x100001, 0x080000,  CRC(275b39a2) SHA1(8ba4d2601734c2dda3d4269fbe8f543dc3f0b212) )
	ROM_LOAD16_BYTE( "dragon_iii_v103j_u13.u13",     0x100000, 0x080000,  CRC(9aa56e8f) SHA1(c3f27d8b59adf72040a2e2c11e34f9b07efd7e9e) )

	ROM_REGION( 0x010000, "igs022", 0 ) /* Protection Data */
	ROM_LOAD( "dw3_v100_u15.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0400.u18",   0x180000, 0x400000, CRC(b70f3357) SHA1(8733969d7d21f540f295a9f747a4bb8f0d325cf0) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0401.u10",    0x0400000, 0x400000, CRC(cab6557f) SHA1(1904dd86645eea27ac1ab8a2462b20f6531356f8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0400.u1",  0x400000, 0x400000, CRC(031eb9ce) SHA1(0673ec194732becc6648c2ae1396e894aa269f9a) )
ROM_END


/*

Chuugokuryuu 3 Special (JAPAN 100 Ver.)
Alta Co./IGS, 1998

Top board
---------
PCB Number: IGS PCB-0189

8MHz Xtal
32.768kHz Xtal
UM6164 (RAM x 2)
MACH211 CPLD
IGS022 ASIC
IGS025 ASIC
1x PAL

2x 27C040 EPROMs (main 68k program)
DW3 V100 U12.U12
DW3 V100 U13.U13

1x 27C512 EPROM (protection code?)
DW3 V100 U15.U15

1x 32MBit smt mask ROM
PGM T0400.U18

Bottom Board
------------
PCB Number: IGS PCB-0178

4x 32MBit smt mask ROMs
PGM A0400.U9
PGM A0401.U10

PGM B0400.U13

PGM M0400.U1

*/

ROM_START( drgw3100 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "dw3_v100_u12.u12",     0x100001, 0x080000,  CRC(47243906) SHA1(9cd46e3cba97f049bcb238ceb6edf27a760ef831) )
	ROM_LOAD16_BYTE( "dw3_v100_u13.u13",     0x100000, 0x080000,  CRC(b7cded21) SHA1(c1ae2af2e42227503c81bbcd2bd6862aa416bd78) )

	ROM_REGION( 0x010000, "igs022", 0 ) /* Protection Data */
	ROM_LOAD( "dw3_v100_u15.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0400.u18",   0x180000, 0x400000, CRC(b70f3357) SHA1(8733969d7d21f540f295a9f747a4bb8f0d325cf0) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0401.u10",    0x0400000, 0x400000, CRC(cab6557f) SHA1(1904dd86645eea27ac1ab8a2462b20f6531356f8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0400.u1",  0x400000, 0x400000, CRC(031eb9ce) SHA1(0673ec194732becc6648c2ae1396e894aa269f9a) )
ROM_END

/*

Dragon World EX IGS PGM cart

Top board
---------
PCB Number: IGS PCB-0189-1
8MHz XTAL
2x 27C040 EPROMs at U12, U13
27C512 EPROM at U15
PAL at U17
2x 6164 SRAM at U1, U4
MACH211 at U11
IGS022 at U14
IGS025 at U16
16M SOP44 mask ROM at U18

Bottom Board
------------
PCB Number: IGS PCB-0178
2x 16M mask ROMs at U1, U10
2x 32M mask ROMs at U9, U13

*/

// seems to be an updated version of dw3, most roms are the same, but it's a sequel, not a clone.
// the non-program roms that differ are actually the same, but in the dw3 sets they're double sized with duplicate data (overdumped, or just different roms used on pcb?)
ROM_START( dwex )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "dwex_v100.u12",     0x100001, 0x080000, CRC(bc171799) SHA1(142329dffbca199f3e748a52146a03e27b36db6a) ) // V100 08/16/00 09:39:27
	ROM_LOAD16_BYTE( "dwex_v100.u13",     0x100000, 0x080000, CRC(7afe6322) SHA1(a52d71af1d6de16c5a3df23eacdab3466693ba8d) )

	ROM_REGION( 0x010000, "igs022", 0 ) /* Protection Data */
	ROM_LOAD( "dwiii_data_u15.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0x500000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0400.u18", 0x180000, 0x200000, CRC(9ecc950d) SHA1(fd97f43818a3eb18254636166871fa09bd0d6c07) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) )
	ROM_LOAD( "pgm_a0401.u10",    0x0400000, 0x200000, CRC(d36c06a4) SHA1(f192e8bfdfbe3d82a49d8f0d3cb0603e39719773) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0400.u1",  0x400000, 0x200000, CRC(42d54fd5) SHA1(ad915b514aa6cae6f72dea78e6208f40b08ceac0) )
ROM_END

ROM_START( dwex101cn )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "dwex_v101cn.u12",     0x100001, 0x080000, CRC(4f951f42) SHA1(830a943ce34c63ce418f60d913fae333377a9704) ) // V101CN (China) / V100 (other regions) 12/12/01 09:45:00
	ROM_LOAD16_BYTE( "dwex_v101cn.u13",     0x100000, 0x080000, CRC(66172511) SHA1(eb1a6fc9c22f04fcca0395a4b5c2972438c60a78) )

	ROM_REGION( 0x010000, "igs022", 0 ) /* Protection Data */
	ROM_LOAD( "dwiii_data_u15.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0x500000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0400.u18", 0x180000, 0x200000, CRC(9ecc950d) SHA1(fd97f43818a3eb18254636166871fa09bd0d6c07) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) )
	ROM_LOAD( "pgm_a0401.u10",    0x0400000, 0x200000, CRC(d36c06a4) SHA1(f192e8bfdfbe3d82a49d8f0d3cb0603e39719773) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0400.u1",  0x400000, 0x200000, CRC(42d54fd5) SHA1(ad915b514aa6cae6f72dea78e6208f40b08ceac0) )
ROM_END


/*

Sangoku Senki / Knights of Valour (HONG KONG 117 Ver.)
(c)1999 ALTA / IGS

Top board
---------
PCB Number: IGS PCB NO-0212-2

PGM P0601 V117.U1   [c4d19fe6]

PGM T0600.U11

Bottom Board
------------
PCB Number: IGS PCB NO-0213T

PGM A0600.U2
PGM A0601.U4
PGM A0602.U6
PGM A0603.U9

PGM M0600.U3

PGM B0600.U5
PGM B0601.U7

*/

ROM_START( kov )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "pgm_p0601_v117.u1",    0x100000, 0x400000, CRC(c4d19fe6) SHA1(14ef31539bfbc665e76c9703ee01b12228344052) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0600.u11",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0600.u2",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0601.u4",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0602.u6",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0603.u9",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0600.u5",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "pgm_b0601.u7",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0600.u3",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kov115 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600.115",    0x100000, 0x400000, CRC(527a2924) SHA1(7e3b166dddc5245d7b408e78437c16fd2986d1d9) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0600.u11",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0600.u2",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0601.u4",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0602.u6",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0603.u9",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0600.u5",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "pgm_b0601.u7",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0600.u3",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

/*

Sangoku Senki / Knights of Valour (HONG KONG 114 Ver.)
(c)1999 ALTA / IGS

Top board
---------
PCB Number: IGS PCB NO-0212-1

SAV111 U4.U4    [ae2f1b4e]
SAV111 U5.U5    [5fdd4aa8]
SAV111 U7.U7    [95eedf0e]
SAV111 U8.U8    [003cbf49]
SAV111 U10.U10  [d5536107]

T0600.U11

Bottom Board
------------
PCB Number: IGS PCB NO-0213T

PGM A0600.U2
PGM A0601.U4
PGM A0602.U6
PGM A0603.U9

PGM M0600.U3

PGM B0600.U5
PGM B0601.U7

*/

ROM_START( kov114 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "sav114_u4.u4",        0x100001, 0x080000, CRC(4db3d4d3) SHA1(924734ec3b3d6de21872890b9575f72f81c7b636) )
	ROM_LOAD16_BYTE( "sav114_u7.u7",        0x100000, 0x080000, CRC(b8d12b0c) SHA1(f4c02e494a479c3021b56bb87341d507104f93d5) )
	ROM_LOAD16_BYTE( "sav114_u5.u5",        0x200001, 0x080000, CRC(9e586dab) SHA1(3de948decf2b7bcbbbc37d2e6fa7a2a71f0b8d5e) )
	ROM_LOAD16_BYTE( "sav114_u8.u8",        0x200000, 0x080000, CRC(ab129997) SHA1(b0e56a09df0def3e8b584ca6f53cd3c88634653e) )
	ROM_LOAD16_WORD_SWAP( "sav114_u10.u10", 0x300000, 0x080000, CRC(8f84ecfd) SHA1(2e7f322da6c4b1d6daf7a308229f4cf2e69fda8f) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov_igs027.bin", 0x000000, 0x04000, NO_DUMP ) // IGS 027 55857E 100 9901 HONG KONG

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0600.u11",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0600.u2",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0601.u4",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0602.u6",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0603.u9",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0600.u5",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "pgm_b0601.u7",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0600.u3",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

/*

Sangoku Senki / Knights of Valour (JAPAN 111 Ver.)
(c)1999 ALTA / IGS

Top board
---------
PCB Number: IGS PCB NO-0212-1
IGS PCB NO-0213T

SAV111 U4.U4    [ae2f1b4e]
SAV111 U5.U5    [5fdd4aa8]
SAV111 U7.U7    [95eedf0e]
SAV111 U8.U8    [003cbf49]
SAV111 U10.U10  [d5536107]

PGM T0600.U11

Bottom Board
------------
PCB Number: IGS PCB NO-0213T

PGM A0600.U2
PGM A0601.U4
PGM A0602.U6
PGM A0603.U9

PGM M0600.U3

PGM B0600.U5
PGM B0601.U7

*/

ROM_START( kov111 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "sav111_u4.u4",        0x100001, 0x080000, CRC(ae2f1b4e) SHA1(2ac9d84f5dee52f374941cfd68e2b98ecad436a8) )
	ROM_LOAD16_BYTE( "sav111_u7.u7",        0x100000, 0x080000, CRC(95eedf0e) SHA1(582a54e9a1eda7ff73e20f0e69d2d50141772378) )
	ROM_LOAD16_BYTE( "sav111_u5.u5",        0x200001, 0x080000, CRC(5fdd4aa8) SHA1(43c96e21ad4f11148e1e94a59c53780b2edd43ba) )
	ROM_LOAD16_BYTE( "sav111_u8.u8",        0x200000, 0x080000, CRC(003cbf49) SHA1(fb5bea47ecae025b1b425af52cd05e061f45e377) )
	ROM_LOAD16_WORD_SWAP( "sav111_u10.u10", 0x300000, 0x080000, CRC(d5536107) SHA1(f963e015d99c1621323eecf63e773c0b9f4b6a43) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0600.u11",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0600.u2",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0601.u4",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0602.u6",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0603.u9",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0600.u5",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "pgm_b0601.u7",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0600.u3",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovplus )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "pgm_p0603_v119.u1",    0x100000, 0x400000, CRC(e4b0875d) SHA1(e8382e131b0e431406dc2a05cc1ef128302d987c) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0600.u11",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0600.u2",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0601.u4",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0602.u6",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0603.u9",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0600.u5",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "pgm_b0601.u7",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0600.u3",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END


/*

Sangoku Senki Plus / Knights of Valour Plus (Alt 119 Ver.)
(c)1999 IGS

PGM system
IGS PCB NO-0212-1
IGS PCB NO-0213T


V119.U2      [29588ef2]
V119.U3      [6750388f]
V119.U4      [8200ece6]
V119.U5      [d4101ffd]
V119.U6      [71e28f27]

T0600.U11


A0600.U2
A0601.U4
A0602.U6
A0603.U9

M0600.U3

B0600.U5
B0601.U7

*/

ROM_START( kovplusa )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "v119.u4",         0x100001, 0x080000, CRC(6750388f) SHA1(869f4ad27f2992cc62baa9a78bf7984a43ec4cc5) )
	ROM_LOAD16_BYTE( "v119.u7",         0x100000, 0x080000, CRC(d4101ffd) SHA1(a327fd56eec65b07df9305cd93ef2c46bf8e40f3) )
	ROM_LOAD16_BYTE( "v119.u5",         0x200001, 0x080000, CRC(8200ece6) SHA1(97081d2e8aed2ac6fbe5951890aecea18af5ce2e) )
	ROM_LOAD16_BYTE( "v119.u8",         0x200000, 0x080000, CRC(71e28f27) SHA1(db382807e9185f0dc17124f210165fa1b36ca6ac) )
	ROM_LOAD16_WORD_SWAP( "v119.u10",   0x300000, 0x080000, CRC(29588ef2) SHA1(17d1a308d44434cf65224a24360cf4b6e32d28f3) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0600.u11",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0600.u2",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0601.u4",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0602.u6",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0603.u9",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0600.u5",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "pgm_b0601.u7",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0600.u3",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsgqyz )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "pgm_sgqyz.rom",    0x100000, 0x400000, CRC(18e1eed9) SHA1(db18d9121bb533140957e9c58dbc38211d164b01) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsgqyz_igs027a.bin", 0x000000, 0x04000, NO_DUMP ) // bootleg is probably a different device

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0600.u11",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0600.u2",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0601.u4",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0602.u6",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "qyza0603.rom",    0x1800000, 0x0800000, CRC(c8b92220) SHA1(4f9c43970d92ac8a8f1563021022797ae8e32012) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0600.u5",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "qyzb0601.rom",    0x0800000, 0x0800000, CRC(64f55882) SHA1(ab9ac1396587c3d78d06f6ec83cab61d6a9faacd) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0600.u3",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsgqyza )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "pgm_sgqyza.rom",    0x100000, 0x400000, CRC(5a30dcb7) SHA1(64f34faf99a19c0a54899990695129c512d5a3c8) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsgqyza_igs027a.bin", 0x000000, 0x04000, NO_DUMP ) // bootleg is probably a different device

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0600.u11",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0600.u2",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0601.u4",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0602.u6",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "qyza0603.rom",    0x1800000, 0x0800000, CRC(c8b92220) SHA1(4f9c43970d92ac8a8f1563021022797ae8e32012) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0600.u5",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "qyzb0601.rom",    0x0800000, 0x0800000, CRC(64f55882) SHA1(ab9ac1396587c3d78d06f6ec83cab61d6a9faacd) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0600.u3",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsgqyzb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "pgm_sgqyzb.rom",    0x100000, 0x400000, CRC(18b8b9c0) SHA1(f4937aa21cd11af16fb50e7a75c8d4c4ed27c5cf) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsgqyzb_igs027a.bin", 0x000000, 0x04000, NO_DUMP ) // bootleg is probably a different device

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0600.u11",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0600.u2",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0601.u4",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0602.u6",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "qyza0603.rom",    0x1800000, 0x0800000, CRC(c8b92220) SHA1(4f9c43970d92ac8a8f1563021022797ae8e32012) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0600.u5",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "qyzb0601.rom",    0x0800000, 0x0800000, CRC(64f55882) SHA1(ab9ac1396587c3d78d06f6ec83cab61d6a9faacd) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0600.u3",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsh )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "pgm_p0605_v104.u1",    0x100000, 0x400000, CRC(7c78e5f3) SHA1(9b1e4bd63fb1294ebeb539966842273c8dc7683b) )
	//ROM_LOAD16_WORD_SWAP( "kovsh-v0104-u1.bin",    0x100000, 0x400000, CRC(4e2ba39b) SHA1(f3b5aa6f45cfd5a7f1e2a2e893d1652a3f23d6b8) ) // identical but the last 1MB is filled with 0xff instead of 0x00

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000,  CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0600.u11",      0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0600.u1",       0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0601.u3",       0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0602.u5",       0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0613.u7",       0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )
	ROM_LOAD( "pgm_a0604_v200.u9",  0x1a00000, 0x0400000, CRC(26b59fd3) SHA1(53219376056f4766dc5236735599d982ceb56b84) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0600.u6",       0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "pgm_b0601.u8",       0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )
	ROM_LOAD( "pgm_b0602_v200.u10", 0x0c00000, 0x0100000, CRC(9df77934) SHA1(99a3fe337c13702c9aa2373bcd1bb1befd0e2a13) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0600.u4",       0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsh103 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "pgm_p0604_v103.u1",    0x100000, 0x400000, CRC(f0b3da82) SHA1(4067beb69c049b51bce6154f4cf880600ca4de11) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000,  CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0600.u11",      0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0600.u1",       0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0601.u3",       0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0602.u5",       0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0613.u7",       0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )
	ROM_LOAD( "pgm_a0604_v200.u9",  0x1a00000, 0x0400000, CRC(26b59fd3) SHA1(53219376056f4766dc5236735599d982ceb56b84) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0600.u6",       0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "pgm_b0601.u8",       0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )
	ROM_LOAD( "pgm_b0602_v200.u10", 0x0c00000, 0x0100000, CRC(9df77934) SHA1(99a3fe337c13702c9aa2373bcd1bb1befd0e2a13) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0600.u4",       0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsh102 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600.102",    0x100000, 0x400000, CRC(fdd4fb0f) SHA1(6906ce68f37b82e52ba30e5cceb1304d9e01430a) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000,  CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0600.u11",      0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0600.u1",       0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0601.u3",       0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0602.u5",       0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0613.u7",       0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )
	ROM_LOAD( "pgm_a0604_v200.u9",  0x1a00000, 0x0400000, CRC(26b59fd3) SHA1(53219376056f4766dc5236735599d982ceb56b84) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0600.u6",       0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "pgm_b0601.u8",       0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )
	ROM_LOAD( "pgm_b0602_v200.u10", 0x0c00000, 0x0100000, CRC(9df77934) SHA1(99a3fe337c13702c9aa2373bcd1bb1befd0e2a13) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0600.u4",       0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsh101 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600.101",    0x100000, 0x400000, CRC(517a9bcf) SHA1(1ee0333aee2a7569e15bb2a1be8dd03f8b08e08c) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000,  CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0600.u11",      0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0600.u1",       0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0601.u3",       0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0602.u5",       0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0613.u7",       0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )
	ROM_LOAD( "pgm_a0604_v200.u9",  0x1a00000, 0x0400000, CRC(26b59fd3) SHA1(53219376056f4766dc5236735599d982ceb56b84) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0600.u6",       0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "pgm_b0601.u8",       0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )
	ROM_LOAD( "pgm_b0602_v200.u10", 0x0c00000, 0x0100000, CRC(9df77934) SHA1(99a3fe337c13702c9aa2373bcd1bb1befd0e2a13) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0600.u4",       0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsh100 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "kovsh-v0100-u1.bin",    0x100000, 0x400000, CRC(d145c1ca) SHA1(ce4da36791bc5eea9fe1ef6db180d789bab0bab7) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000,  CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0600.u11",      0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0600.u1",       0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0601.u3",       0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0602.u5",       0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0613.u7",       0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )
	ROM_LOAD( "pgm_a0604_v200.u9",  0x1a00000, 0x0400000, CRC(26b59fd3) SHA1(53219376056f4766dc5236735599d982ceb56b84) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0600.u6",       0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "pgm_b0601.u8",       0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )
	ROM_LOAD( "pgm_b0602_v200.u10", 0x0c00000, 0x0100000, CRC(9df77934) SHA1(99a3fe337c13702c9aa2373bcd1bb1befd0e2a13) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0600.u4",       0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovqhsgs )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "qhsg_c51.rom", 0x100000, 0x400000, CRC(e5cbac85) SHA1(4b424206387057863990b04f6d5bd0b6f754814f) ) // V300CN V303CN Nov 21 2008 19:03:49

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "qhsg_prot.c51", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "lsqh2_t01.rom",0x180000, 0x800000, CRC(d498d97f) SHA1(97a7b6d2ed1170449e7c2899448af7cbbca4c94f) )
	ROM_IGNORE( 0x800000 )  // second half identical

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "lsqh2_a01.rom", 0x0000000, 0x1000000, CRC(25ae3efd) SHA1(083d977602ddb5ad54fbdcba000cd4287de8d463) )
	ROM_LOAD( "lsqh2_a23.rom", 0x1000000, 0x1000000, CRC(7a779112) SHA1(0a7d36b3715063d8eac629b95a9bb3ecd8e54fca) )
	ROM_LOAD( "lsqh2_a45.rom", 0x2000000, 0x1000000, CRC(5d7de052) SHA1(7663b6cf09f65c4644661005a38f9aba84a32913) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "lsqh2_b01.rom", 0x0000000, 0x1000000, CRC(df7ca696) SHA1(7af3d27957a39de7e4873867c9972c05af7e7964) )

	ROM_REGION( 0xc00000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "lsqh2_m01.rom",0x400000, 0x400000, CRC(01af1b65) SHA1(6cf523fa8f1e03f974771611bb9a4e08a4d4443f) )
	ROM_IGNORE( 0x400000 )  // 400000-7fffff empty
	ROM_CONTINUE( 0x800000, 0x400000 )
	ROM_IGNORE( 0x400000 )  // c00000-ffffff empty
ROM_END

ROM_START( kovqhsgs302 ) // cart with 2010--04--16 main PCB and 2009-09-01 ROM PCB
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code, on main PCB */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "27c322.u3", 0x100000, 0x400000, CRC(4018559f) SHA1(e7e97ce89e1f563e8e08501ec3c2be6d81107253) ) // V300CN V302CN Dec 29 2010 16:31:32

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom (NXP LPC2132 ARM7 TDMI-S CPU with 64kB internal flash ROM on main PCB)  */
	ROM_LOAD( "qhsg_prot.c51", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) ) // not dumped for this set

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles, on main PCB */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t01.u8",0x180000, 0x800000, CRC(d498d97f) SHA1(97a7b6d2ed1170449e7c2899448af7cbbca4c94f) )
	ROM_IGNORE(                  0x800000 )  // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data, on ROM PCB */
	ROM_LOAD( "a01.u8",  0x0000000, 0x1000000, CRC(25ae3efd) SHA1(083d977602ddb5ad54fbdcba000cd4287de8d463) )
	ROM_LOAD( "a23.u9",  0x1000000, 0x1000000, CRC(1f51c140) SHA1(638a7449449716a2ae862a11b4333c09d369050a) )
	ROM_LOAD( "a45.u10", 0x2000000, 0x1000000, CRC(5e44fd82) SHA1(bb7b9258a37a7fc7caf14ac5b606dd0ce6d43135) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes, on ROM PCB */
	ROM_LOAD( "b01.u6", 0x0000000, 0x1000000, CRC(779825d3) SHA1(eb179111eb4f0c98d502622f094b1e5aa1f98225) )

	ROM_REGION( 0xc00000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) -, on ROM PCB */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m01.u5",0x400000, 0x400000, CRC(af5e9be0) SHA1(be9da327f2a26bf14a6bc1c0b2dd114ac13b8c74) )
	ROM_IGNORE(                  0x400000 )  // 400000-7fffff almost completely empty
	ROM_CONTINUE(      0x800000, 0x400000 )
	ROM_IGNORE(                  0x400000 )  // c00000-ffffff almost completely empty
ROM_END


/*

p0701_v105.u2

IGS PCB NO-0220-1
PGM P0701 V105
1B2687LC
C994746

*/

ROM_START( photoy2k )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "pgm_p0701_v105.u2",     0x100000, 0x200000, CRC(fab142e0) SHA1(8dc7e53b740ed68bac98c0ef7ca4943c517e6f5d) ) // 10/12/99 21:07:53

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "igs027a_photoy2k_v100_china.asic", 0x000000, 0x04000, CRC(1a0b68f6) SHA1(290441ed652f54b26ace8f59a26220881fb62084) ) // 3 bytes differ from the read in the other sets.  I think this one is GOOD and the other is bad.  This always gives the same read, so unless the actual chips is bad... TBC

	ROM_REGION( 0x280000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0700.u11",   0x180000, 0x080000, CRC(93943b4d) SHA1(3b439903853727d45d62c781af6073024eb3c5a3) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0700.u2",    0x0000000, 0x0800000, CRC(503c855b) SHA1(aa910cc33a23ac8f1c91f88da388ed92e49fa1b8) )
	ROM_LOAD( "pgm_a0701.u4",    0x0800000, 0x0800000, CRC(845e11a8) SHA1(9e35d0e6620acd023eba83f86d970e9895204767) )
	ROM_LOAD( "pgm_a0702.u3",    0x1000000, 0x0080000, CRC(42239e1b) SHA1(2b6d20958abf8a67ce525d5c8964b6d225ccaeda) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0700.u7",            0x0000000, 0x0800000, CRC(8cd027f6) SHA1(e377c64facbf657f58b8567d8b483ca067967fc0) )
	ROM_LOAD( "photo_y2k_cg_v101_u6.u6", 0x0800000, 0x0020000, CRC(da02ec3e) SHA1(7ee21d748c9b932f53e790a9040167f904fecefc) )

	ROM_REGION( 0x480000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0700.u5",    0x400000, 0x080000, CRC(acc7afce) SHA1(ac2d344ebac336f0f363bb045dd8ea4e83d1fb50) )
ROM_END


ROM_START( photoy2k104 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v104.16m",     0x100000, 0x200000, CRC(e051070f) SHA1(a5a1a8dd7542a30632501af8d02fda07475fd9aa) ) // 06/10/99 17:28:34

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "igs027a_photoy2k_v100_china_alt.asic", 0x000000, 0x04000,  CRC(6dd7f257) SHA1(1984f98a282d8b3264674f231c3b7def1757cf72) )

	ROM_REGION( 0x280000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0700.u11",   0x180000, 0x080000, CRC(93943b4d) SHA1(3b439903853727d45d62c781af6073024eb3c5a3) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0700.u2",    0x0000000, 0x0800000, CRC(503c855b) SHA1(aa910cc33a23ac8f1c91f88da388ed92e49fa1b8) )
	ROM_LOAD( "pgm_a0701.u4",    0x0800000, 0x0800000, CRC(845e11a8) SHA1(9e35d0e6620acd023eba83f86d970e9895204767) )
	ROM_LOAD( "pgm_a0702.u3",    0x1000000, 0x0080000, CRC(42239e1b) SHA1(2b6d20958abf8a67ce525d5c8964b6d225ccaeda) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0700.u7",            0x0000000, 0x0800000, CRC(8cd027f6) SHA1(e377c64facbf657f58b8567d8b483ca067967fc0) )
	ROM_LOAD( "photo_y2k_cg_v101_u6.u6", 0x0800000, 0x0020000, CRC(da02ec3e) SHA1(7ee21d748c9b932f53e790a9040167f904fecefc) )

	ROM_REGION( 0x480000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0700.u5",    0x400000, 0x080000, CRC(acc7afce) SHA1(ac2d344ebac336f0f363bb045dd8ea4e83d1fb50) )
ROM_END


ROM_START( photoy2k103j )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "photo_y2k_v103_u4.u4",  0x100001, 0x080000, CRC(c16dc699) SHA1(062d38ed32f56c1e640c5e2be046bc6e150123b1) ) // 05/10/99 13:32:52
	ROM_LOAD16_BYTE( "photo_y2k_v103_u6.u6",  0x100000, 0x080000, CRC(2e2671a4) SHA1(5f37b6d789fedeb5e291081f2908061f30875dc6) )
	ROM_LOAD16_BYTE( "photo_y2k_v103_u5.u5",  0x200001, 0x080000, CRC(97839a61) SHA1(bf34e3fab90a846baa5b5e0a3c3d9d99a603c8ee) )
	ROM_LOAD16_BYTE( "photo_y2k_v103_u8.u8",  0x200000, 0x080000, CRC(43af9664) SHA1(00dc74960cb126adfc223783b09a2787fe37625e) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "igs027a_photoy2k_v100_china.asic", 0x000000, 0x04000, CRC(1a0b68f6) SHA1(290441ed652f54b26ace8f59a26220881fb62084) )

	ROM_REGION( 0x280000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0700.u11",   0x180000, 0x080000, CRC(93943b4d) SHA1(3b439903853727d45d62c781af6073024eb3c5a3) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0700.u2",            0x0000000, 0x0800000, CRC(503c855b) SHA1(aa910cc33a23ac8f1c91f88da388ed92e49fa1b8) )
	ROM_LOAD( "pgm_a0701.u4",            0x0800000, 0x0800000, CRC(845e11a8) SHA1(9e35d0e6620acd023eba83f86d970e9895204767) )
	ROM_LOAD( "photo_yk2_cg_v101_u3.u3", 0x1000000, 0x0080000, CRC(42239e1b) SHA1(2b6d20958abf8a67ce525d5c8964b6d225ccaeda) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0700.u7",            0x0000000, 0x0800000, CRC(8cd027f6) SHA1(e377c64facbf657f58b8567d8b483ca067967fc0) )
	ROM_LOAD( "photo_y2k_cg_v101_u6.u6", 0x0800000, 0x0020000, CRC(da02ec3e) SHA1(7ee21d748c9b932f53e790a9040167f904fecefc) )

	ROM_REGION( 0x480000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "photo_y2k_sp_v102_u5.u5", 0x400000, 0x080000, CRC(acc7afce) SHA1(ac2d344ebac336f0f363bb045dd8ea4e83d1fb50) )
ROM_END


/*

Real and Fake / Photo Y2K (JPN 102 Ver.)
(c)1999 ALTA / IGS

PGM system
IGS PCB NO-0220
IGS PCB NO-0221


V102.U4      [a65eda9f]
V102.U5      [9201621b]
V102.U6      [b9ca5504]
V102.U8      [3be22b8f]

T0700.U11


A0700.U2
A0701.U4

SP_V102.U5

B0700.U7

CG_V101.U3
CG_V101.U6

*/

ROM_START( photoy2k102j )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "photo_y2k_v102_u4.u4",     0x100001, 0x080000, CRC(a65eda9f) SHA1(6307cacf4a262e781753eff14700a0455837780c) ) // 05/03/99 16:06:33
	ROM_LOAD16_BYTE( "photo_y2k_v102_u6.u6",     0x100000, 0x080000, CRC(b9ca5504) SHA1(058cf01316f233236ca9861349f515935283b75e) )
	ROM_LOAD16_BYTE( "photo_y2k_v102_u5.u5",     0x200001, 0x080000, CRC(9201621b) SHA1(1ca3ebe7eec40614bfa8b911657fa2b51f2c51a4) )
	ROM_LOAD16_BYTE( "photo_y2k_v102_u8.u8",     0x200000, 0x080000, CRC(3be22b8f) SHA1(03634fbd6a8a8369c6cb1fd6694a3784dac5bf59) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "igs027a_photoy2k_v100_china_alt.asic", 0x000000, 0x04000,  CRC(6dd7f257) SHA1(1984f98a282d8b3264674f231c3b7def1757cf72) )

	ROM_REGION( 0x280000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0700.u11",   0x180000, 0x080000, CRC(93943b4d) SHA1(3b439903853727d45d62c781af6073024eb3c5a3) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0700.u2",    0x0000000, 0x0800000, CRC(503c855b) SHA1(aa910cc33a23ac8f1c91f88da388ed92e49fa1b8) )
	ROM_LOAD( "pgm_a0701.u4",    0x0800000, 0x0800000, CRC(845e11a8) SHA1(9e35d0e6620acd023eba83f86d970e9895204767) )
	ROM_LOAD( "pgm_a0702.u3",    0x1000000, 0x0080000, CRC(42239e1b) SHA1(2b6d20958abf8a67ce525d5c8964b6d225ccaeda) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0700.u7",            0x0000000, 0x0800000, CRC(8cd027f6) SHA1(e377c64facbf657f58b8567d8b483ca067967fc0) )
	ROM_LOAD( "photo_y2k_cg_v101_u6.u6", 0x0800000, 0x0020000, CRC(da02ec3e) SHA1(7ee21d748c9b932f53e790a9040167f904fecefc) )

	ROM_REGION( 0x480000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "photo_y2k_sp_v102_u5.u5", 0x400000, 0x080000, CRC(acc7afce) SHA1(ac2d344ebac336f0f363bb045dd8ea4e83d1fb50) )
ROM_END


/*

Photo Y2K2 IGS PGM cart

Top board
---------
PCB Number: IGS PCB-0313-00T
27C160 EPROM at U1
PAL at U3
IGS027A at U4

Bottom Board
------------
PCB Number: IGS PCB-0314-00
1x 16M SOP44 mask ROM at U3
6x 64M SOP44 mask ROMs at U4, U5, U6, U7, U8, U9

*/

ROM_START( py2k2 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "y2k2_m-101xx.u1",     0x100000, 0x200000, CRC(c47795f1) SHA1(5be4af4275571932d7740c3ea0857a1f58a3f6d9) ) // 68k (encrypted) 2nd half empty...; 05/25/01 11:02:54

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "igs027a_photoy2k2.asic", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x280000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	/* no extra tilemap rom */

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1100.u6",    0x0000000, 0x0800000, CRC(e32ce499) SHA1(f84c7daa55c25a05da467b5654ebf432b7ce1754) )
	ROM_LOAD( "pgm_a1101.u7",    0x0800000, 0x0800000, CRC(4e7568bc) SHA1(bf9cc453191bd5ec9fbcce62891809f253a44267) )
	ROM_LOAD( "pgm_a1102.u8",    0x1000000, 0x0800000, CRC(6da7c143) SHA1(9408ba7722bfc8013f851aadea5e2819f5263129) )
	ROM_LOAD( "pgm_a1103.u9",    0x1800000, 0x0800000, CRC(0ebebfdc) SHA1(4faad7f97c7e734f179ec934a37e75d8d6adccf4) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1100.u4",    0x0000000, 0x0800000,  CRC(fa53d6f6) SHA1(c2da55f4b7e721fa1c63bd7f9528f261643164e8) )
	ROM_LOAD( "pgm_b1101.u5",    0x0800000, 0x0800000, CRC(001e4c81) SHA1(21119055f8fd7f831529e73ff9c97bca3987a1dc))

	ROM_REGION( 0x880000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1100.u3",    0x400000, 0x200000, CRC(fb1515f8) SHA1(90e5e5bfdac9a460445bf224952e4a536888dc1b) )
ROM_END

ROM_START( py2k2100 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "y2kii_v100.u1",     0x100000, 0x100000, CRC(7a1e36ac) SHA1(0ff9a30733ca8026e4acce45a8993c8ab4b242b5) ) // 68k (encrypted) 08/07/00 13:19:59

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "igs027a_photoy2k2.asic", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x280000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	/* no extra tilemap rom */

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1100.u6",    0x0000000, 0x0800000, CRC(e32ce499) SHA1(f84c7daa55c25a05da467b5654ebf432b7ce1754) )
	ROM_LOAD( "pgm_a1101.u7",    0x0800000, 0x0800000, CRC(4e7568bc) SHA1(bf9cc453191bd5ec9fbcce62891809f253a44267) )
	ROM_LOAD( "pgm_a1102.u8",    0x1000000, 0x0800000, CRC(6da7c143) SHA1(9408ba7722bfc8013f851aadea5e2819f5263129) )
	ROM_LOAD( "pgm_a1103.u9",    0x1800000, 0x0800000, CRC(0ebebfdc) SHA1(4faad7f97c7e734f179ec934a37e75d8d6adccf4) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1100.u4",    0x0000000, 0x0800000,  CRC(fa53d6f6) SHA1(c2da55f4b7e721fa1c63bd7f9528f261643164e8) )
	ROM_LOAD( "pgm_b1101.u5",    0x0800000, 0x0800000, CRC(001e4c81) SHA1(21119055f8fd7f831529e73ff9c97bca3987a1dc))

	ROM_REGION( 0x880000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1100.u3",    0x400000, 0x200000, CRC(fb1515f8) SHA1(90e5e5bfdac9a460445bf224952e4a536888dc1b) )
ROM_END


ROM_START( pgm3in1 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v-102cn.u3",     0x100000, 0x200000, CRC(72e06b2b) SHA1(0d72f90b1a2df5f0e8708b59d2a7c13dba998acd)) // M68K V102 08/23/04 13:03:26 (encrypted)

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "igs027a_pgm3in1.asic", 0x000000, 0x04000, NO_DUMP )

	/* No external ARM rom */

	ROM_REGION( 0x500000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "u5.u5",    0x180000, 0x200000, CRC(da375a50) SHA1(62cd2fd3dfc1897528eaa38d243d7a9526eac71b) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1100.u4",    0x0000000, 0x0800000, CRC(e32ce499) SHA1(f84c7daa55c25a05da467b5654ebf432b7ce1754) ) // == y2k2_a1100.u6
	ROM_LOAD( "pgm_a1101.u5",    0x0800000, 0x0800000, CRC(4e7568bc) SHA1(bf9cc453191bd5ec9fbcce62891809f253a44267) ) // == y2k2_a1101.u7
	ROM_LOAD( "pgm_a1102.u6",    0x1000000, 0x0800000, CRC(6da7c143) SHA1(9408ba7722bfc8013f851aadea5e2819f5263129) ) // == y2k2_a1102.u8
	ROM_LOAD( "pgm_a1103.u7",    0x1800000, 0x0800000, CRC(0ebebfdc) SHA1(4faad7f97c7e734f179ec934a37e75d8d6adccf4) ) // == y2k2_a1103.u9
	ROM_LOAD( "ext_bit_cg.u20",  0x2000000, 0x0400000, CRC(fe314754) SHA1(ae3e8bbdce852a3fa39806a5221c053dee5abfd4) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", ROMREGION_ERASE00 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1100.u8",    0x0000000, 0x0800000, CRC(fa53d6f6) SHA1(c2da55f4b7e721fa1c63bd7f9528f261643164e8) ) // == y2k2_b1100.u4
	ROM_LOAD( "pgm_b1101.u9",    0x0800000, 0x0800000, CRC(001e4c81) SHA1(21119055f8fd7f831529e73ff9c97bca3987a1dc) ) // == y2k2_b1101.u5
	ROM_LOAD( "ext_bit_map.u21", 0x0f00000, 0x0100000, CRC(fe31dca6) SHA1(825bab7342c944794514fc7fe3e41779de3b5cd4) ) // yes this loads over the empty part of u9
	ROM_IGNORE(0x0100000) // the 2nd half is empty

	ROM_REGION( 0xe80000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1100.u17",    0x400000, 0x200000, CRC(fb1515f8) SHA1(90e5e5bfdac9a460445bf224952e4a536888dc1b) ) // == y2k2_m1100.u3
	ROM_LOAD( "u16.u16",          0x600000, 0x800000, CRC(714c33e5) SHA1(5478d5247349cdfb5f835171615d6ca2e5689140) ) // check loading
ROM_END


ROM_START( pgm3in1c100 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v-100cn.u3",     0x100000, 0x200000, CRC(a39f59b4) SHA1(4eb53fb9f173cb470e16dc8f193c8909cf045e3d)) // M68K V100 07/13/04 12:09:20 (encrypted)

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "igs027a_pgm3in1.asic", 0x000000, 0x04000, NO_DUMP )

	/* No external ARM rom */

	ROM_REGION( 0x500000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "u5.u5",    0x180000, 0x200000, CRC(da375a50) SHA1(62cd2fd3dfc1897528eaa38d243d7a9526eac71b) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1100.u4",    0x0000000, 0x0800000, CRC(e32ce499) SHA1(f84c7daa55c25a05da467b5654ebf432b7ce1754) ) // == y2k2_a1100.u6
	ROM_LOAD( "pgm_a1101.u5",    0x0800000, 0x0800000, CRC(4e7568bc) SHA1(bf9cc453191bd5ec9fbcce62891809f253a44267) ) // == y2k2_a1101.u7
	ROM_LOAD( "pgm_a1102.u6",    0x1000000, 0x0800000, CRC(6da7c143) SHA1(9408ba7722bfc8013f851aadea5e2819f5263129) ) // == y2k2_a1102.u8
	ROM_LOAD( "pgm_a1103.u7",    0x1800000, 0x0800000, CRC(0ebebfdc) SHA1(4faad7f97c7e734f179ec934a37e75d8d6adccf4) ) // == y2k2_a1103.u9
	ROM_LOAD( "ext_bit_cg.u20",  0x2000000, 0x0400000, CRC(fe314754) SHA1(ae3e8bbdce852a3fa39806a5221c053dee5abfd4) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", ROMREGION_ERASE00 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1100.u8",    0x0000000, 0x0800000, CRC(fa53d6f6) SHA1(c2da55f4b7e721fa1c63bd7f9528f261643164e8) ) // == y2k2_b1100.u4
	ROM_LOAD( "pgm_b1101.u9",    0x0800000, 0x0800000, CRC(001e4c81) SHA1(21119055f8fd7f831529e73ff9c97bca3987a1dc) ) // == y2k2_b1101.u5
	ROM_LOAD( "ext_bit_map.u21", 0x0f00000, 0x0100000, CRC(fe31dca6) SHA1(825bab7342c944794514fc7fe3e41779de3b5cd4) ) // yes this loads over the empty part of u9
	ROM_IGNORE(0x0100000) // the 2nd half is empty

	ROM_REGION( 0xe80000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1100.u17",    0x400000, 0x200000, CRC(fb1515f8) SHA1(90e5e5bfdac9a460445bf224952e4a536888dc1b) ) // == y2k2_m1100.u3
	ROM_LOAD( "u16.u16",          0x600000, 0x800000, CRC(714c33e5) SHA1(5478d5247349cdfb5f835171615d6ca2e5689140) ) // check loading
ROM_END


/*

The Killing Blade (English / World Version)
IGS, 1998

This is a cart for the IGS PGM system.


PCB Layout
----------

IGS PCB NO-0179
|-----------------------------------------------|
|                      8MHz  |--------|         |
|            32.768kHz|----| |        |T0300.U14|
|6164  6164           |IGS | | IGS025 |         |
|                     |022 | |        |         |
|*                    |----| |--------|         |
|                                               |
|           U2     U3     U4     U5     U6      |
| PAL   PAL                                PAL  |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      IGS022       - Custom IGS IC (QFP100)
      IGS025       - Custom IGS IC (PLCC68, labelled "ENGLISH")
      T0300.U14    - 32MBit MaskROM (SOP44, labelled "T0300")
      6164         - x2, 8K x8 SRAM (SOJ28)
      U2           - 27C512 512KBit EPROM (DIP28, labelled "KB U2 V104")
      U3           - 27C4000 4MBit EPROM (DIP32, labelled "KB U3 V104")
      U4           - 27C4000 4MBit EPROM (DIP32, labelled "KB U4 V104")
      U5           - 27C4000 4MBit EPROM (DIP32, labelled "KB U5 V104")
      U6           - 27C4000 4MBit EPROM (DIP32, labelled "KB U6 V104")
      PALs         - x3, labelled "DH U8", "DH U1", "DH U7"
      *            - Unpopulated position for DIP42 EPROM/mask ROM (labelled "P0300")


IGS PCB NO-0178
|-----------------------------------------------|
| U9    U10   U11    U12     U13     U14     U15|
|                                               |
|                                               |
|                                               |
| U1    U2                         74LS138      |
|                                  74LS139      |
|             U3     U4      U5              U8 |
|                                               |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|

Notes:
      U1           - 32MBit mask ROM (SOP44, labelled "M0300")
      U2           - 32MBit mask ROM (SOP44, labelled "A0307")
      U3           - 16MBit mask ROM (DIP42, labelled "A0302")
      U4           - 16MBit mask ROM (DIP42, labelled "A0304")
      U5           - 16MBit mask ROM (DIP42, labelled "A0305")
      U8           - 16MBit mask ROM (DIP42, labelled "B0301")
      U9           - 32MBit mask ROM (SOP44, labelled "A0300")
      U10          - 32MBit mask ROM (SOP44, labelled "A0301")
      U11          - 32MBit mask ROM (SOP44, labelled "A0303")
      U12          - 32MBit mask ROM (SOP44, labelled "A0306")
      U13          - 32MBit mask ROM (SOP44, labelled "B0300")
      U14          - 32MBit mask ROM (SOP44, labelled "B0302")
      U15          - 32MBit mask ROM (SOP44, labelled "B0303")

*/



/*
 the text on the chip are
-----------------
IGS
PGM P0300 V109
1A0577Y3
J982846
-----------------
*/

ROM_START( killbld )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0300_v109.u9", 0x100000, 0x200000, CRC(2fcee215) SHA1(855281a9090bfdf3da9f4d50c121765131a13400) )
	// also found with 4 smaller ROMs
	// p0300_v109.u9 [even 1/2] kb_u3_v109.u3              IDENTICAL CRC(fe028dd5) SHA1(a865639e6ce9a5ad7100bc0445a58b0465bfe8a6)
	// p0300_v109.u9 [odd 1/2]  kb_u6_v109.u6              IDENTICAL CRC(e50dba01) SHA1(0103192a877ab18c4da0b3448a1dd9e088c7a740)
	// p0300_v109.u9 [even 2/2] kb_u4_v109.u4              IDENTICAL CRC(6ac58bb3) SHA1(5e911d1490f6a32f90e150ea933d8679302e0f61)
	// p0300_v109.u9 [odd 2/2]  kb_u5_v109.u5              IDENTICAL CRC(7dabf576) SHA1(a9fb7fb4c487752597793962ef2bc17ae244fc0a)

	ROM_REGION( 0x010000, "igs022", 0 ) /* Protection Data */
	ROM_LOAD( "kb_u2_v109.u2", 0x000000, 0x010000,  CRC(de3eae63) SHA1(03af767ef764055bda528b5cc6a24b9e1218cca8) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0300.u14",    0x180000, 0x400000, CRC(0922f7d9) SHA1(4302b4b7369e13f315fad14f7d6cad1321101d24) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0300.u9",   0x0000000, 0x0400000,  CRC(3f9455d3) SHA1(3718ce00ad93975383aafc14e5a74dc297b011a1) )
	ROM_LOAD( "pgm_a0301.u10",  0x0400000, 0x0400000,  CRC(92776889) SHA1(6d677837fefff47bfd1c6166322f69f89989a5e2) )
	ROM_LOAD( "pgm_a0303.u11",  0x0800000, 0x0400000,  CRC(33f5cc69) SHA1(9cacd5058d4bb25b77f71658bbbbd4b38d0a6b6a) )
	ROM_LOAD( "pgm_a0306.u12",  0x0c00000, 0x0400000,  CRC(cc018a8e) SHA1(37752d46f238fb57c0ab5a4f96b1e013f2077347) )
	ROM_LOAD( "pgm_a0307.u2",   0x1000000, 0x0400000,  CRC(bc772e39) SHA1(079cc42a190cb916f02b59bca8fa90e524acefe9) )
//  ROM_LOAD( "pgm_a0302.u3",   0x1400000, 0x0200000,  CRC(a4810e38) SHA1(c31fe641feab2c93795fc35bf71d4f37af1056d4) ) // from lord of gun! unused..
//  ROM_LOAD( "pgm_a0304.u4",   0x1600000, 0x0200000,  CRC(3096de1c) SHA1(d010990d21cfda9cb8ab5b4bc0e329c23b7719f5) ) // from lord of gun! unused..
//  ROM_LOAD( "pgm_a0305.u5",   0x1800000, 0x0200000,  CRC(2234531e) SHA1(58a82e31a1c0c1a4dd026576319f4e7ecffd140e) ) // from lord of gun! unused..

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0300.u13",    0x0000000, 0x0400000, CRC(7f876981) SHA1(43555a200929ad5ecc42137fc9aeb42dc4f50d20) )
	ROM_LOAD( "pgm_b0302.u14",    0x0400000, 0x0400000, CRC(eea9c502) SHA1(04b3972c7111ea59a3cceab6ad124080c4ce3520) )
	ROM_LOAD( "pgm_b0303.u15",    0x0800000, 0x0200000, CRC(77a9652e) SHA1(2342f643d37945fbda224a5034c013796e5134ca) )
//  ROM_LOAD( "pgm_b0301.u8",     0x0a00000, 0x0200000, CRC(400abe33) SHA1(20de1eb626424ea41bd55eb3cecd6b50be744ee0) ) // from lord of gun! unused..

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0300.u1",     0x400000, 0x400000, CRC(93159695) SHA1(50c5976c9b681bd3d1ebefa3bfa9fe6e72dcb96f) )
ROM_END

ROM_START( killbld104 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "kb_u3_v104.u3",     0x100001, 0x080000, CRC(6db1d719) SHA1(804002f014d275aaf0368fb7f904938fe4ac07ee) )
	ROM_LOAD16_BYTE( "kb_u6_v104.u6",     0x100000, 0x080000, CRC(31ecc978) SHA1(82666d534e4151775063af6d39f575faba0f1047) )
	ROM_LOAD16_BYTE( "kb_u4_v104.u4",     0x200001, 0x080000, CRC(1ed8b2e7) SHA1(331c037640cfc1fe743cd0e65a1156c470b3303e) )
	ROM_LOAD16_BYTE( "kb_u5_v104.u5",     0x200000, 0x080000, CRC(a0bafc29) SHA1(b20db7c16353c6f87ed3c08c9d037b07336711f1) )

	ROM_REGION( 0x010000, "igs022", 0 ) /* Protection Data */
	ROM_LOAD( "kb_u2_v104.u2", 0x000000, 0x010000,  CRC(c970f6d5) SHA1(399fc6f80262784c566363c847dc3fdc4fb37494) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0300.u14",    0x180000, 0x400000, CRC(0922f7d9) SHA1(4302b4b7369e13f315fad14f7d6cad1321101d24) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0300.u9",   0x0000000, 0x0400000,  CRC(3f9455d3) SHA1(3718ce00ad93975383aafc14e5a74dc297b011a1) )
	ROM_LOAD( "pgm_a0301.u10",  0x0400000, 0x0400000,  CRC(92776889) SHA1(6d677837fefff47bfd1c6166322f69f89989a5e2) )
	ROM_LOAD( "pgm_a0303.u11",  0x0800000, 0x0400000,  CRC(33f5cc69) SHA1(9cacd5058d4bb25b77f71658bbbbd4b38d0a6b6a) )
	ROM_LOAD( "pgm_a0306.u12",  0x0c00000, 0x0400000,  CRC(cc018a8e) SHA1(37752d46f238fb57c0ab5a4f96b1e013f2077347) )
	ROM_LOAD( "pgm_a0307.u2",   0x1000000, 0x0400000,  CRC(bc772e39) SHA1(079cc42a190cb916f02b59bca8fa90e524acefe9) )
//  ROM_LOAD( "pgm_a0302.u3",   0x1400000, 0x0200000,  CRC(a4810e38) SHA1(c31fe641feab2c93795fc35bf71d4f37af1056d4) ) // from lord of gun! unused..
//  ROM_LOAD( "pgm_a0304.u4",   0x1600000, 0x0200000,  CRC(3096de1c) SHA1(d010990d21cfda9cb8ab5b4bc0e329c23b7719f5) ) // from lord of gun! unused..
//  ROM_LOAD( "pgm_a0305.u5",   0x1800000, 0x0200000,  CRC(2234531e) SHA1(58a82e31a1c0c1a4dd026576319f4e7ecffd140e) ) // from lord of gun! unused..

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0300.u13",    0x0000000, 0x0400000, CRC(7f876981) SHA1(43555a200929ad5ecc42137fc9aeb42dc4f50d20) )
	ROM_LOAD( "pgm_b0302.u14",    0x0400000, 0x0400000, CRC(eea9c502) SHA1(04b3972c7111ea59a3cceab6ad124080c4ce3520) )
	ROM_LOAD( "pgm_b0303.u15",    0x0800000, 0x0200000, CRC(77a9652e) SHA1(2342f643d37945fbda224a5034c013796e5134ca) )
//  ROM_LOAD( "pgm_b0301.u8",     0x0a00000, 0x0200000, CRC(400abe33) SHA1(20de1eb626424ea41bd55eb3cecd6b50be744ee0) ) // from lord of gun! unused..

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0300.u1",     0x400000, 0x400000, CRC(93159695) SHA1(50c5976c9b681bd3d1ebefa3bfa9fe6e72dcb96f) )
ROM_END

ROM_START( killbld106 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "kb_u3_v106.u3",     0x100001, 0x080000, CRC(33b9111a) SHA1(26875a9e502af9a36d13077cb7b07f9d28773d72) )
	ROM_LOAD16_BYTE( "kb_u6_v106.u6",     0x100000, 0x080000, CRC(1c957bd7) SHA1(2bb54915166b4a62148de043b2c2088d39b91f14) )
	ROM_LOAD16_BYTE( "kb_u4_v106.u4",     0x200001, 0x080000, CRC(169bbaaf) SHA1(a1833d3fd024c43ba7642f13e83a5b7b66631136) )
	ROM_LOAD16_BYTE( "kb_u5_v106.u5",     0x200000, 0x080000, CRC(df85abd4) SHA1(f9e37f76c7af8a8492bd1fd22d8b3fbda194ed03) )

	ROM_REGION( 0x010000, "igs022", 0 ) /* Protection Data */
	ROM_LOAD( "kb_u2_v106.u2", 0x000000, 0x010000,  CRC(5df8cf51) SHA1(d82e281a43015da653fc37e97f52943e03a07112) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0300.u14",    0x180000, 0x400000, CRC(0922f7d9) SHA1(4302b4b7369e13f315fad14f7d6cad1321101d24) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0300.u9",   0x0000000, 0x0400000,  CRC(3f9455d3) SHA1(3718ce00ad93975383aafc14e5a74dc297b011a1) )
	ROM_LOAD( "pgm_a0301.u10",  0x0400000, 0x0400000,  CRC(92776889) SHA1(6d677837fefff47bfd1c6166322f69f89989a5e2) )
	ROM_LOAD( "pgm_a0303.u11",  0x0800000, 0x0400000,  CRC(33f5cc69) SHA1(9cacd5058d4bb25b77f71658bbbbd4b38d0a6b6a) )
	ROM_LOAD( "pgm_a0306.u12",  0x0c00000, 0x0400000,  CRC(cc018a8e) SHA1(37752d46f238fb57c0ab5a4f96b1e013f2077347) )
	ROM_LOAD( "pgm_a0307.u2",   0x1000000, 0x0400000,  CRC(bc772e39) SHA1(079cc42a190cb916f02b59bca8fa90e524acefe9) )
//  ROM_LOAD( "pgm_a0302.u3",   0x1400000, 0x0200000,  CRC(a4810e38) SHA1(c31fe641feab2c93795fc35bf71d4f37af1056d4) ) // from lord of gun! unused..
//  ROM_LOAD( "pgm_a0304.u4",   0x1600000, 0x0200000,  CRC(3096de1c) SHA1(d010990d21cfda9cb8ab5b4bc0e329c23b7719f5) ) // from lord of gun! unused..
//  ROM_LOAD( "pgm_a0305.u5",   0x1800000, 0x0200000,  CRC(2234531e) SHA1(58a82e31a1c0c1a4dd026576319f4e7ecffd140e) ) // from lord of gun! unused..

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0300.u13",    0x0000000, 0x0400000, CRC(7f876981) SHA1(43555a200929ad5ecc42137fc9aeb42dc4f50d20) )
	ROM_LOAD( "pgm_b0302.u14",    0x0400000, 0x0400000, CRC(eea9c502) SHA1(04b3972c7111ea59a3cceab6ad124080c4ce3520) )
	ROM_LOAD( "pgm_b0303.u15",    0x0800000, 0x0200000, CRC(77a9652e) SHA1(2342f643d37945fbda224a5034c013796e5134ca) )
//  ROM_LOAD( "pgm_b0301.u8",     0x0a00000, 0x0200000, CRC(400abe33) SHA1(20de1eb626424ea41bd55eb3cecd6b50be744ee0) ) // from lord of gun! unused..

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0300.u1",     0x400000, 0x400000, CRC(93159695) SHA1(50c5976c9b681bd3d1ebefa3bfa9fe6e72dcb96f) )
ROM_END

ROM_START( killbld100 ) // was dumped from a Taiwanese board
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "kb_u3_v100.u3",     0x100001, 0x080000, CRC(ba3233a9) SHA1(d41f00c0c83ead8567b2e4ebf3fd7b8525f06b73) )
	ROM_LOAD16_BYTE( "kb_u6_v100.u6",     0x100000, 0x080000, CRC(55ae1d39) SHA1(931411c0d7d02a636010bb5f7f312906de32d839) )
	ROM_LOAD16_BYTE( "kb_u4_v100.u4",     0x200001, 0x080000, CRC(d56a8407) SHA1(1c43c62ec9e3b11f77d9c14647797493f7b8f960) )
	ROM_LOAD16_BYTE( "kb_u5_v100.u5",     0x200000, 0x080000, CRC(99afff2b) SHA1(4167efc81daf365e632177bb26650c95dcc32ccb) )

	ROM_REGION( 0x010000, "igs022", 0 ) /* Protection Data */
	ROM_LOAD( "kb_u2_v100.u2", 0x000000, 0x010000,  CRC(6fbbdcb7) SHA1(2928b66cd605d38a5d4e5876890e52af89dfadc4) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0300.u14",    0x180000, 0x400000, CRC(0922f7d9) SHA1(4302b4b7369e13f315fad14f7d6cad1321101d24) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0300.u9",   0x0000000, 0x0400000,  CRC(3f9455d3) SHA1(3718ce00ad93975383aafc14e5a74dc297b011a1) )
	ROM_LOAD( "pgm_a0301.u10",  0x0400000, 0x0400000,  CRC(92776889) SHA1(6d677837fefff47bfd1c6166322f69f89989a5e2) )
	ROM_LOAD( "pgm_a0303.u11",  0x0800000, 0x0400000,  CRC(33f5cc69) SHA1(9cacd5058d4bb25b77f71658bbbbd4b38d0a6b6a) )
	ROM_LOAD( "pgm_a0306.u12",  0x0c00000, 0x0400000,  CRC(cc018a8e) SHA1(37752d46f238fb57c0ab5a4f96b1e013f2077347) )
	ROM_LOAD( "pgm_a0307.u2",   0x1000000, 0x0400000,  CRC(bc772e39) SHA1(079cc42a190cb916f02b59bca8fa90e524acefe9) )
//  ROM_LOAD( "pgm_a0302.u3",   0x1400000, 0x0200000,  CRC(a4810e38) SHA1(c31fe641feab2c93795fc35bf71d4f37af1056d4) ) // from lord of gun! unused..
//  ROM_LOAD( "pgm_a0304.u4",   0x1600000, 0x0200000,  CRC(3096de1c) SHA1(d010990d21cfda9cb8ab5b4bc0e329c23b7719f5) ) // from lord of gun! unused..
//  ROM_LOAD( "pgm_a0305.u5",   0x1800000, 0x0200000,  CRC(2234531e) SHA1(58a82e31a1c0c1a4dd026576319f4e7ecffd140e) ) // from lord of gun! unused..

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0300.u13",    0x0000000, 0x0400000, CRC(7f876981) SHA1(43555a200929ad5ecc42137fc9aeb42dc4f50d20) )
	ROM_LOAD( "pgm_b0302.u14",    0x0400000, 0x0400000, CRC(eea9c502) SHA1(04b3972c7111ea59a3cceab6ad124080c4ce3520) )
	ROM_LOAD( "pgm_b0303.u15",    0x0800000, 0x0200000, CRC(77a9652e) SHA1(2342f643d37945fbda224a5034c013796e5134ca) )
//  ROM_LOAD( "pgm_b0301.u8",     0x0a00000, 0x0200000, CRC(400abe33) SHA1(20de1eb626424ea41bd55eb3cecd6b50be744ee0) ) // from lord of gun! unused..

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0300.u1",     0x400000, 0x400000, CRC(93159695) SHA1(50c5976c9b681bd3d1ebefa3bfa9fe6e72dcb96f) )
ROM_END


/*
Puzzle Star
IGS, 1999

Cart for IGS PGM system. This game is a 'columns' type game.

PCB Layout
----------

IGS PCB NO- T0236
|-----------------------------------------------|
|                        U6 U7                  |
|         |-------|                             |
|         |IGS027A|                             |
|         |       |                     T0800.U5|
|         |       |                             |
|         |-------|                             |
|          U1_V100MG.U1                         |
|          U2_V100MG.U2   U3   PAL              |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      IGS027A       - Custom IGS IC, ARM7/9? based CPU (QFP120, stamped 'IGS027A' & labelled 'ENGLISH')
      T0800.U5      - 16MBit MaskROM (DIP42)
      U1_V100MG.U1  - MX27C4000 512K x8 EPROM (DIP32, labelled 'PuzzleStar U1 V100MG')
      U2_V100MG.U2  - MX27C4000 512K x8 EPROM (DIP32, labelled 'PuzzleStar U2 V100MG')
      PAL           - Atmel ATF22V10B PAL (DIP24, labelled 'EA U4')
      U3            - Unpopulated position for 32MBit mask ROM (DIP42)
      U6, U7        - Unpopulated position for 74LS245 logic chip (x2)


IGS PCB NO- T0237
|-----------------------------------------------|
|                                               |
|                                               |
|                                               |
|                                               |
|       A0800.U1   M0800.U2   B0800.U3          |
|                                               |
|                                               |
|                                               |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      U1 - 32MBit MaskROM (DIP42)
      U2 - 32MBit MaskROM (DIP42)
      U3 - 16MBit MaskROM (DIP42)
*/

ROM_START( puzlstar ) // V100MG 09/30/99 11:39:23
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "puzzlestar_u1_v100mg.u1",     0x100000, 0x080000, CRC(c6ee43d7) SHA1(2419414ded556b5e4868d51b0da5dfce374d6bc0) )
	ROM_LOAD16_BYTE( "puzzlestar_u2_v100mg.u2",     0x100001, 0x080000, CRC(42aa03ce) SHA1(8e1666cc3ded98179a3e4854973604172921bbd6) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "puzlstar_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x500000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0800.u5",    0x180000, 0x200000, CRC(f9d84e59) SHA1(80ec77025ac5bf355b1a60f2a678dd4c56071f6b) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0800.u1",    0x0000000, 0x0400000, CRC(e1e6ec40) SHA1(390432431f144ef63424a426582b311765a61771) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0800.u3",    0x0000000, 0x0200000, CRC(52e7bef5) SHA1(a678251b7e46a1016d0afc1d8d5c9928008ad5b1) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0800.u2",    0x400000, 0x400000,  CRC(e1a46541) SHA1(6fe9de5700d8638374734d80551dcedb62975140) )
ROM_END

ROM_START( puzlstara ) // V100MG 09/20/99 15:16:02
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "v100mg.u1",     0x100000, 0x080000, CRC(5788b77d) SHA1(7770aae6e686da92b2623c977d1bc8f019f48267) )
	ROM_LOAD16_BYTE( "v100mg.u2",     0x100001, 0x080000, CRC(4c79d979) SHA1(3b92052a35994f2b3dd164930154184c45d5e2d0) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "puzlstar_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x500000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0800.u5",    0x180000, 0x200000, CRC(f9d84e59) SHA1(80ec77025ac5bf355b1a60f2a678dd4c56071f6b) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0800.u1",    0x0000000, 0x0400000, CRC(e1e6ec40) SHA1(390432431f144ef63424a426582b311765a61771) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0800.u3",    0x0000000, 0x0200000, CRC(52e7bef5) SHA1(a678251b7e46a1016d0afc1d8d5c9928008ad5b1) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0800.u2",    0x400000, 0x400000,  CRC(e1a46541) SHA1(6fe9de5700d8638374734d80551dcedb62975140) )
ROM_END


/*

Oriental Legend Super
IGS, 1998

This is a cart for the IGS PGM system.

PCB Layout
----------
IGS PCB NO-0191-1
|-----------------------------------------------|
|6264                 8MHz|--------|            |
|6264                     |        |   T0500.U18|
|                         | IGS025 |            |
|                 |-----| |        |   T0501.U19|
|                 | IGS | |--------|            |
|                 | 028 |                       |
|        *1       |-----|           V101.U1     |
|              V101.U2   V101.U4  PAL      PAL  |
|  V101.U6          V101.U3   V101.U5           |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      IGS028       - Custom IGS IC (QFP100)
      IGS025       - Custom IGS IC (PLCC68, labelled "KOREA")
      T0500.U18    - 32MBit MaskROM (SOP44)
      T0501.U19    - 16MBit MaskROM (SOP44)
      V101.U1      - MX27C4096 4MBit EPROM (DIP40)
      V101.U2/3/4/5- MX27C4000 4MBit EPROM (DIP32)
      PALs         - x2, labelled "CW-2 U8", "CW-2 U7"
      6264         - 8K x8 SRAM
      *1           - Unpopulated position for SOP44 mask ROM labelled "P0500"


IGS PCB NO-0135
|-----------------------------------------------|
|A0504.U11        A0506.U13     B0502.U15       |
|         A0505.U12         U14        B0503.U16|
|                                               |
|A0500.U5                       B0500.U9        |
|         A0501.U6       A0503.U8      B0501.U10|
|                 A0502.U7                      |
|                                               |
|74LS138          M0500.U1               74LS139|
|                           U2                  |
|-|                                           |-|
  |--------------------||---------------------|

Notes:
      This PCB contains only SOP44 MaskROMS and 2 logic IC's
      U2 and U14 are not populated.
      All are 32MBit except M0500 which is 16MBit.

*/

ROM_START( olds103t )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0500.v103",0x100000, 0x400000, CRC(17e32e14) SHA1(b8f731087af2c59fe5b1da31f1cb055d35c8b440) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0500.u18",   0x180000, 0x400000, CRC(d881726c) SHA1(a82517e665996f7b7017c940f1fcf016fccb65c2) )
	ROM_LOAD( "pgm_t0501.u19",   0x580000, 0x200000, CRC(d2106864) SHA1(65d827135b87d82196433aea3279608ee263feca) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0500.u5",    0x0000000, 0x400000, CRC(80a59197) SHA1(7d0108e2f0d0347d43ace2d96c0388202c05fdfb) )
	ROM_LOAD( "pgm_a0501.u6",    0x0400000, 0x400000, CRC(98c931b1) SHA1(9b70d1a7beb4c3a0c1436c25fd1fb76e47539538) )
	ROM_LOAD( "pgm_a0502.u7",    0x0800000, 0x400000, CRC(c3fcdf1d) SHA1(107585fd103fcd0af0fb7db283be4f7c7058aef7) )
	ROM_LOAD( "pgm_a0503.u8",    0x0c00000, 0x400000, CRC(066dffec) SHA1(f023032a7202b7090fb609a39e0f19018e664bf3) )
	ROM_LOAD( "pgm_a0504.u11",   0x1000000, 0x400000, CRC(45337583) SHA1(c954d0e5bf7fa99c90b0d154e7119d2b0c461f1c) )
	ROM_LOAD( "pgm_a0505.u12",   0x1400000, 0x400000, CRC(5b8cf3a5) SHA1(856d1e47b5d9a66dcfbdc74a51ed646fd7d96a35) )
	ROM_LOAD( "pgm_a0506.u13",   0x1800000, 0x400000, CRC(087ac60c) SHA1(3d5bf7dd40c8a3c1224cf82e12410ca904c0c5db) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0500.u9",    0x0000000, 0x400000, CRC(cde07f74) SHA1(1fe90db7c57faa28f3a054b2c07909bef89e3efb) )
	ROM_LOAD( "pgm_b0501.u10",   0x0400000, 0x400000, CRC(1546c2e9) SHA1(a7b9c8b44203db54a59d49fe469bb52bba807ba2) )
	ROM_LOAD( "pgm_b0502.u15",   0x0800000, 0x400000, CRC(e97b31c3) SHA1(1a7ca4f6c8644e84a33ae41cd4637f21046b14c5) )
	ROM_LOAD( "pgm_b0503.u16",   0x0c00000, 0x400000, CRC(e41d98e4) SHA1(f80b27fcee81762993e09bf1b3cad6e85274760c) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0500.u1",    0x400000, 0x200000, CRC(37928cdd) SHA1(e80498cabc2a6a54d4f3ebcb097d4b3fad96fe55) )
ROM_END

ROM_START( olds )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "sp_v101_u2.u2",      0x100001, 0x080000,   CRC(08eb9661) SHA1(105946e72e562adb1a9fd794ca0fd2c91967eb56) )
	ROM_LOAD16_BYTE( "sp_v101_u3.u3",      0x100000, 0x080000,   CRC(0a358c1e) SHA1(95c7c3f069c5d05001e22535750f6b3cd7de105f) )
	ROM_LOAD16_BYTE( "sp_v101_u4.u4",      0x200001, 0x080000,   CRC(766570e0) SHA1(e7c3f5664ec69b662b82c2e1375555db7305390c) )
	ROM_LOAD16_BYTE( "sp_v101_u5.u5",      0x200000, 0x080000,   CRC(58662e12) SHA1(2b39bd847e9c4968a8e77a2f3cec77cf323ceee3) )
	ROM_LOAD16_WORD_SWAP( "sp_v101_u1.u1", 0x300000, 0x080000,    CRC(2b2f4f1e) SHA1(67b97cf8cc7f517d67cd45588addd2ad8e24612a) )

	ROM_REGION32_LE( 0x010000, "user1", 0 ) /* IGS028 Protection Data */
	ROM_LOAD( "sp_v101_u6.u6", 0x000000, 0x010000,  CRC(097046bc) SHA1(6d75db85cf4c79b63e837897785c253014b2126d) )

	ROM_REGION( 0x4000, "user2", ROMREGION_ERASEFF ) /* its a dump of the shared protection rom/ram from pcb. */
	// clearly not for this revision
	ROM_LOAD( "ram_dump", 0x000000, 0x04000, CRC(280cfb4e) SHA1(cd2bdcaa21347952c2bf38b105a204d327fde39e) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0500.u18",   0x180000, 0x400000, CRC(d881726c) SHA1(a82517e665996f7b7017c940f1fcf016fccb65c2) )
	ROM_LOAD( "pgm_t0501.u19",   0x580000, 0x200000, CRC(d2106864) SHA1(65d827135b87d82196433aea3279608ee263feca) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0500.u5",    0x0000000, 0x400000, CRC(80a59197) SHA1(7d0108e2f0d0347d43ace2d96c0388202c05fdfb) )
	ROM_LOAD( "pgm_a0501.u6",    0x0400000, 0x400000, CRC(98c931b1) SHA1(9b70d1a7beb4c3a0c1436c25fd1fb76e47539538) )
	ROM_LOAD( "pgm_a0502.u7",    0x0800000, 0x400000, CRC(c3fcdf1d) SHA1(107585fd103fcd0af0fb7db283be4f7c7058aef7) )
	ROM_LOAD( "pgm_a0503.u8",    0x0c00000, 0x400000, CRC(066dffec) SHA1(f023032a7202b7090fb609a39e0f19018e664bf3) )
	ROM_LOAD( "pgm_a0504.u11",   0x1000000, 0x400000, CRC(45337583) SHA1(c954d0e5bf7fa99c90b0d154e7119d2b0c461f1c) )
	ROM_LOAD( "pgm_a0505.u12",   0x1400000, 0x400000, CRC(5b8cf3a5) SHA1(856d1e47b5d9a66dcfbdc74a51ed646fd7d96a35) )
	ROM_LOAD( "pgm_a0506.u13",   0x1800000, 0x400000, CRC(087ac60c) SHA1(3d5bf7dd40c8a3c1224cf82e12410ca904c0c5db) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0500.u9",    0x0000000, 0x400000, CRC(cde07f74) SHA1(1fe90db7c57faa28f3a054b2c07909bef89e3efb) )
	ROM_LOAD( "pgm_b0501.u10",   0x0400000, 0x400000, CRC(1546c2e9) SHA1(a7b9c8b44203db54a59d49fe469bb52bba807ba2) )
	ROM_LOAD( "pgm_b0502.u15",   0x0800000, 0x400000, CRC(e97b31c3) SHA1(1a7ca4f6c8644e84a33ae41cd4637f21046b14c5) )
	ROM_LOAD( "pgm_b0503.u16",   0x0c00000, 0x400000, CRC(e41d98e4) SHA1(f80b27fcee81762993e09bf1b3cad6e85274760c) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0500.u1",    0x400000, 0x200000, CRC(37928cdd) SHA1(e80498cabc2a6a54d4f3ebcb097d4b3fad96fe55) )
ROM_END

ROM_START( olds100 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "sp_v100_u2.u2",      0x100001, 0x080000,  CRC(517c2a06) SHA1(bbf5b311fac9b0bb4d4129c0561e5e24f6963fa2) )
	ROM_LOAD16_BYTE( "sp_v100_u3.u3",      0x100000, 0x080000,  CRC(d0e2b741) SHA1(2e671dbb4320d1f0c059b35efd33cdea26f12131) )
	ROM_LOAD16_BYTE( "sp_v100_u4.u4",      0x200001, 0x080000,  CRC(32a6bdbd) SHA1(a93d7f4eae722a58eca9ec351ad5890cefda56f0) )
	ROM_LOAD16_BYTE( "sp_v100_u5.u5",      0x200000, 0x080000,  CRC(b4a1cafb) SHA1(b2fccd480ede93f58ad043387b18b898152f06ef) )
	ROM_LOAD16_WORD_SWAP( "sp_v100_u1.u1", 0x300000, 0x080000,  CRC(37ea4e75) SHA1(a94fcb89da3394a43d360f885419677f511d2580) )

	ROM_REGION( 0x4000, "user2", ROMREGION_ERASEFF ) /* its a dump of the shared protection rom/ram from pcb. */
	// used to simulate encrypted DMA protection device for now ..
	ROM_LOAD( "ram_dump", 0x000000, 0x04000, CRC(280cfb4e) SHA1(cd2bdcaa21347952c2bf38b105a204d327fde39e) )

	ROM_REGION32_LE( 0x010000, "user1", 0 ) /* IGS028 Protection Data */
	ROM_LOAD( "sp_v100_u6.u6", 0x000000, 0x010000,  CRC(e7613dda) SHA1(0d7c043b90e2f9a36a45066f22e3e305dc716676) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0500.u18",   0x180000, 0x400000, CRC(d881726c) SHA1(a82517e665996f7b7017c940f1fcf016fccb65c2) )
	ROM_LOAD( "pgm_t0501.u19",   0x580000, 0x200000, CRC(d2106864) SHA1(65d827135b87d82196433aea3279608ee263feca) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0500.u5",    0x0000000, 0x400000, CRC(80a59197) SHA1(7d0108e2f0d0347d43ace2d96c0388202c05fdfb) )
	ROM_LOAD( "pgm_a0501.u6",    0x0400000, 0x400000, CRC(98c931b1) SHA1(9b70d1a7beb4c3a0c1436c25fd1fb76e47539538) )
	ROM_LOAD( "pgm_a0502.u7",    0x0800000, 0x400000, CRC(c3fcdf1d) SHA1(107585fd103fcd0af0fb7db283be4f7c7058aef7) )
	ROM_LOAD( "pgm_a0503.u8",    0x0c00000, 0x400000, CRC(066dffec) SHA1(f023032a7202b7090fb609a39e0f19018e664bf3) )
	ROM_LOAD( "pgm_a0504.u11",   0x1000000, 0x400000, CRC(45337583) SHA1(c954d0e5bf7fa99c90b0d154e7119d2b0c461f1c) )
	ROM_LOAD( "pgm_a0505.u12",   0x1400000, 0x400000, CRC(5b8cf3a5) SHA1(856d1e47b5d9a66dcfbdc74a51ed646fd7d96a35) )
	ROM_LOAD( "pgm_a0506.u13",   0x1800000, 0x400000, CRC(087ac60c) SHA1(3d5bf7dd40c8a3c1224cf82e12410ca904c0c5db) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0500.u9",    0x0000000, 0x400000, CRC(cde07f74) SHA1(1fe90db7c57faa28f3a054b2c07909bef89e3efb) )
	ROM_LOAD( "pgm_b0501.u10",   0x0400000, 0x400000, CRC(1546c2e9) SHA1(a7b9c8b44203db54a59d49fe469bb52bba807ba2) )
	ROM_LOAD( "pgm_b0502.u15",   0x0800000, 0x400000, CRC(e97b31c3) SHA1(1a7ca4f6c8644e84a33ae41cd4637f21046b14c5) )
	ROM_LOAD( "pgm_b0503.u16",   0x0c00000, 0x400000, CRC(e41d98e4) SHA1(f80b27fcee81762993e09bf1b3cad6e85274760c) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0500.u1",    0x400000, 0x200000, CRC(37928cdd) SHA1(e80498cabc2a6a54d4f3ebcb097d4b3fad96fe55) )
ROM_END

/* this is the set which the protection ram dump seems to be for.. */
ROM_START( olds100a )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	/* this rom had a lame hack applied to it by the dumper, this was removed, hopefully it is correct now */
	ROM_LOAD16_WORD_SWAP( "pgm_p0500_v100.u24",    0x100000, 0x400000, CRC(8981fc87) SHA1(678d6705d06b99bca5951ff77708adadc4c4396b) )

	/* ROM label SP 西遊記 DATA on sticker */
	ROM_REGION32_LE( 0x010000, "user1", 0 ) /* IGS028 Protection Data */
	ROM_LOAD( "sp_data.u6", 0x000000, 0x010000,  CRC(e7613dda) SHA1(0d7c043b90e2f9a36a45066f22e3e305dc716676) )

	ROM_REGION( 0x4000, "user2", ROMREGION_ERASEFF ) /* its a dump of the shared protection rom/ram from pcb. */
	// used to simulate encrypted DMA protection device for now ..
	ROM_LOAD( "ram_dump", 0x000000, 0x04000, CRC(280cfb4e) SHA1(cd2bdcaa21347952c2bf38b105a204d327fde39e) )

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0500.u18",   0x180000, 0x400000, CRC(d881726c) SHA1(a82517e665996f7b7017c940f1fcf016fccb65c2) )
	ROM_LOAD( "pgm_t0501.u19",   0x580000, 0x200000, CRC(d2106864) SHA1(65d827135b87d82196433aea3279608ee263feca) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0500.u5",    0x0000000, 0x400000, CRC(80a59197) SHA1(7d0108e2f0d0347d43ace2d96c0388202c05fdfb) )
	ROM_LOAD( "pgm_a0501.u6",    0x0400000, 0x400000, CRC(98c931b1) SHA1(9b70d1a7beb4c3a0c1436c25fd1fb76e47539538) )
	ROM_LOAD( "pgm_a0502.u7",    0x0800000, 0x400000, CRC(c3fcdf1d) SHA1(107585fd103fcd0af0fb7db283be4f7c7058aef7) )
	ROM_LOAD( "pgm_a0503.u8",    0x0c00000, 0x400000, CRC(066dffec) SHA1(f023032a7202b7090fb609a39e0f19018e664bf3) )
	ROM_LOAD( "pgm_a0504.u11",   0x1000000, 0x400000, CRC(45337583) SHA1(c954d0e5bf7fa99c90b0d154e7119d2b0c461f1c) )
	ROM_LOAD( "pgm_a0505.u12",   0x1400000, 0x400000, CRC(5b8cf3a5) SHA1(856d1e47b5d9a66dcfbdc74a51ed646fd7d96a35) )
	ROM_LOAD( "pgm_a0506.u13",   0x1800000, 0x400000, CRC(087ac60c) SHA1(3d5bf7dd40c8a3c1224cf82e12410ca904c0c5db) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0500.u9",    0x0000000, 0x400000, CRC(cde07f74) SHA1(1fe90db7c57faa28f3a054b2c07909bef89e3efb) )
	ROM_LOAD( "pgm_b0501.u10",   0x0400000, 0x400000, CRC(1546c2e9) SHA1(a7b9c8b44203db54a59d49fe469bb52bba807ba2) )
	ROM_LOAD( "pgm_b0502.u15",   0x0800000, 0x400000, CRC(e97b31c3) SHA1(1a7ca4f6c8644e84a33ae41cd4637f21046b14c5) )
	ROM_LOAD( "pgm_b0503.u16",   0x0c00000, 0x400000, CRC(e41d98e4) SHA1(f80b27fcee81762993e09bf1b3cad6e85274760c) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0500.u1",    0x400000, 0x200000, CRC(37928cdd) SHA1(e80498cabc2a6a54d4f3ebcb097d4b3fad96fe55) )
ROM_END


ROM_START( kov2 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v107_u18.u18",      0x100000, 0x400000, CRC(661a5b2c) SHA1(125054fabc93d4f4cba869c3e6adf863650d30cf) ) // 05/10/01 14:24:08 V107

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v102_u19.u19",   0x000000, 0x200000,   CRC(462e2980) SHA1(3da7c3d2c65b59f50c78be1c25922b71d40f6080) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t1200.u27",  0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1200.u1",   0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "pgm_a1201.u4",   0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "pgm_a1202.u6",   0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "pgm_a1203.u8",   0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "pgm_a1204.u10",  0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1200.u5",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "pgm_b1201.u7",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1200.u3",   0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2106 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v106_u18.u18",    0x100000, 0x400000, CRC(40051ad9) SHA1(ba2ddf267fe688d5dfed575aeeccbab10135b37b) ) // 02/27/01 13:26:46 V106

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v102_u19.u19",   0x000000, 0x200000,   CRC(462e2980) SHA1(3da7c3d2c65b59f50c78be1c25922b71d40f6080) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t1200.u27",  0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1200.u1",   0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "pgm_a1201.u4",   0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "pgm_a1202.u6",   0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "pgm_a1203.u8",   0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "pgm_a1204.u10",  0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1200.u5",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "pgm_b1201.u7",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1200.u3",   0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2104 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v104_u18.u18",    0x100000, 0x400000, CRC(0fdb050b) SHA1(8ac704459e1a17d0956dd3529e81b6bcd8abf361) ) // 01/19/01 10:19:38 V104

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v102_u19.u19",   0x000000, 0x200000,   CRC(462e2980) SHA1(3da7c3d2c65b59f50c78be1c25922b71d40f6080) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t1200.u27",  0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1200.u1",   0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "pgm_a1201.u4",   0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "pgm_a1202.u6",   0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "pgm_a1203.u8",   0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "pgm_a1204.u10",  0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1200.u5",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "pgm_b1201.u7",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1200.u3",   0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2103 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v103_u18.u18",    0x100000, 0x400000, CRC(98c32f76) SHA1(ec7e35e8071bb7097e415493be4e40be0ca19fd6) ) // 12/28/00 15:09:31 V103

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v101_u19.u19",   0x000000, 0x200000, CRC(8c35f2fe) SHA1(d4858f97fcfad0f342fccbc9cf1590276cc3c69c) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t1200.u27",  0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1200.u1",   0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "pgm_a1201.u4",   0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "pgm_a1202.u6",   0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "pgm_a1203.u8",   0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "pgm_a1204.u10",  0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1200.u5",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "pgm_b1201.u7",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1200.u3",   0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov21022 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v102_u18.u18",    0x100000, 0x400000, CRC(a2489c37) SHA1(77ea7cdec211848296dafd45bee1d042133ea2a6) ) // 12/14/00 10:33:36 V102

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v102_u19.u19",   0x000000, 0x200000,   CRC(462e2980) SHA1(3da7c3d2c65b59f50c78be1c25922b71d40f6080) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t1200.u27",  0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1200.u1",   0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "pgm_a1201.u4",   0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "pgm_a1202.u6",   0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "pgm_a1203.u8",   0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "pgm_a1204.u10",  0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1200.u5",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "pgm_b1201.u7",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1200.u3",   0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2102 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v102_u18.u18",    0x100000, 0x400000, CRC(a2489c37) SHA1(77ea7cdec211848296dafd45bee1d042133ea2a6) ) // 12/14/00 10:33:36 V102

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v101_u19.u19",   0x000000, 0x200000, CRC(8c35f2fe) SHA1(d4858f97fcfad0f342fccbc9cf1590276cc3c69c) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t1200.u27",  0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1200.u1",   0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "pgm_a1201.u4",   0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "pgm_a1202.u6",   0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "pgm_a1203.u8",   0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "pgm_a1204.u10",  0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1200.u5",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "pgm_b1201.u7",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1200.u3",   0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2101 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v101_u18.u18",    0x100000, 0x400000, CRC(c9926f35) SHA1(a9c72d0c5d239164107894c7d3fffe4af29ed201) ) // 12/07/00 16:40:30 V101

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v101_u19.u19",   0x000000, 0x200000, CRC(8c35f2fe) SHA1(d4858f97fcfad0f342fccbc9cf1590276cc3c69c) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t1200.u27",  0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1200.u1",   0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "pgm_a1201.u4",   0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "pgm_a1202.u6",   0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "pgm_a1203.u8",   0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "pgm_a1204.u10",  0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1200.u5",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "pgm_b1201.u7",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1200.u3",   0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2100 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v100_u18.u18",    0x100000, 0x400000, CRC(86205879) SHA1(f73d5b70b41d39be1cac75e474b025de2cce0b01) ) // 11/29/00 11:03:08 V100

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v100_u19.u19",   0x000000, 0x200000,   CRC(edd59922) SHA1(09b14f20f685944a93292c83e5830849aade42c9) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t1200.u27",  0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1200.u1",   0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "pgm_a1201.u4",   0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "pgm_a1202.u6",   0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "pgm_a1203.u8",   0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "pgm_a1204.u10",  0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1200.u5",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "pgm_b1201.u7",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1200.u3",   0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END


ROM_START( kov2p )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v205_32m.u8",    0x100000, 0x400000, CRC(3a2cc0de) SHA1(d7511478b34bfb03b2fb5b8268b60502d05b9414) ) // 04/25/02 17:48:27 M205XX

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2p_igs027a_china.bin", 0x000000, 0x04000, CRC(19a0bd95) SHA1(83e9f22512832a51d41c588debe8be7adb3b1df7) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v200_16m.u23",   0x000000, 0x200000,  CRC(16a0c11f) SHA1(ce449cef76ebd5657d49b57951e2eb0f132e203e) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t1200.u21",  0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1200.u1",   0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "pgm_a1201.u4",   0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "pgm_a1202.u6",   0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "pgm_a1203.u8",   0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "pgm_a1204.u10",  0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1200.u5",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "pgm_b1201.u7",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1200.u3",   0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2p204 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v204_32m.u8",    0x100000, 0x400000, CRC(583e0650) SHA1(2e5656dd9c6cba9f84af9baa3f5f70cdccf9db47) ) // 08/28/01 09:11:49 M204XX

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2p_igs027a_china.bin", 0x000000, 0x04000, CRC(19a0bd95) SHA1(83e9f22512832a51d41c588debe8be7adb3b1df7) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v200_16m.u23",   0x000000, 0x200000,  CRC(16a0c11f) SHA1(ce449cef76ebd5657d49b57951e2eb0f132e203e) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t1200.u21",  0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1200.u1",   0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "pgm_a1201.u4",   0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "pgm_a1202.u6",   0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "pgm_a1203.u8",   0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "pgm_a1204.u10",  0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1200.u5",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "pgm_b1201.u7",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1200.u3",   0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2p203 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v203_32m.u8",    0x100000, 0x400000, CRC(11416886) SHA1(00088165893ed0b5fb8bbac3def0edeb9ff0c4fd) ) // 08/13/01 16:57:32 M203XX

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2p_igs027a_china.bin", 0x000000, 0x04000, CRC(19a0bd95) SHA1(83e9f22512832a51d41c588debe8be7adb3b1df7) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v200_16m.u23",   0x000000, 0x200000,  CRC(16a0c11f) SHA1(ce449cef76ebd5657d49b57951e2eb0f132e203e) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t1200.u21",  0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1200.u1",   0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "pgm_a1201.u4",   0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "pgm_a1202.u6",   0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "pgm_a1203.u8",   0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "pgm_a1204.u10",  0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1200.u5",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "pgm_b1201.u7",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1200.u3",   0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2p202 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v202_32m.u8",    0x100000, 0x400000, CRC(e9b5aa0c) SHA1(39a776c8501e8557d305cfa56c997f9adeb6bcd2) ) // 07/09/01 11:03:50 M202XX

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2p_igs027a_china.bin", 0x000000, 0x04000, CRC(19a0bd95) SHA1(83e9f22512832a51d41c588debe8be7adb3b1df7) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v200_16m.u23",   0x000000, 0x200000,  CRC(16a0c11f) SHA1(ce449cef76ebd5657d49b57951e2eb0f132e203e) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t1200.u21",  0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1200.u1",   0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "pgm_a1201.u4",   0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "pgm_a1202.u6",   0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "pgm_a1203.u8",   0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "pgm_a1204.u10",  0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1200.u5",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "pgm_b1201.u7",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1200.u3",   0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2p200 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v200_32m.u8",    0x100000, 0x400000, CRC(9a09fd61) SHA1(690a0b323e1f275a08ba84febf93bf8edc2d0802) ) // 06/18/01  22:59:12 M200XX

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2p_igs027a_china.bin", 0x000000, 0x04000, CRC(19a0bd95) SHA1(83e9f22512832a51d41c588debe8be7adb3b1df7) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v200_16m.u23",   0x000000, 0x200000,  CRC(16a0c11f) SHA1(ce449cef76ebd5657d49b57951e2eb0f132e203e) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t1200.u21",  0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1200.u1",   0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "pgm_a1201.u4",   0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "pgm_a1202.u6",   0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "pgm_a1203.u8",   0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "pgm_a1204.u10",  0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1200.u5",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "pgm_b1201.u7",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1200.u3",   0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

/*

Do Donpachi II
IGS (Cave license), 2001

This is a PGM cart containing not a lot....
5x SOP44 mask ROMs (4x 64M, 1x 32M)
2x EPROMs (1x 1M, 1x 16M)
2x PALs (labelled FN U14 and FN U15)
1x custom IGS027A (QFP120)
3x RAMs WINBOND W24257AJ-8N
Some logic IC's, resistors, caps etc.

*/

/*
  For ddp2 the China and Japan internal ASIC27 roms were dumped, and confirmed to differ only by the byte at offset 0x2882
  we're assuming this to be true for other regions, this is a fair assumption based on the evidence we have.
*/

#define DDP2_ASIC027_CHINA \
	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */ \
	ROM_LOAD( "ddp2_igs027a_china.bin", 0x000000, 0x04000, CRC(8c566319) SHA1(bb001d8ada56bf446f9ab88e00936501652daf11) ) /* 00 */

#define DDP2_ASIC027_TAIWAN \
	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */ \
	ROM_LOAD( "ddp2_igs027a_taiwan.bin", 0x000000, 0x04000, CRC(1dd34bdc) SHA1(4f2ee38ee37d5cc63b2a63ef51bd392e4f18d836) ) /* 01 */

#define DDP2_ASIC027_JAPAN \
	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */ \
	ROM_LOAD( "ddp2_igs027a_japan.bin", 0x000000, 0x04000, CRC(742d34d2) SHA1(4491c08f3cefef2933ad5a741f4bb05cc2f3e1a0) ) /* 02 */

#define DDP2_ASIC027_KOREA \
	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */ \
	ROM_LOAD( "ddp2_igs027a_korea.bin", 0x000000, 0x04000, CRC(e5a81c17) SHA1(cfe4e28a44a1b3a5c1c9e303941b335dbde7dd8d) ) /* 03 */

#define DDP2_ASIC027_HONG_KONG \
	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */ \
	ROM_LOAD( "ddp2_igs027a_hong_kong.bin", 0x000000, 0x04000, CRC(a7d1cace) SHA1(b9391da52c8234ca0182484f33b5242827c51c76) ) /* 04 */

#define DDP2_ASIC027_WORLD \
	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */ \
	ROM_LOAD( "ddp2_igs027a_world.bin", 0x000000, 0x04000, CRC(3654e20b) SHA1(e00ef1d51efe66354e3eaf4750d9d819c74ddfbf) ) /* 05 */

#define DDP2_COMMON_ROMS \
	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */ \
	ROM_LOAD( "v100_210.u23",   0x000000, 0x20000, CRC(06c3dd29) SHA1(20c9479f158467fc2037dcf162b6c6be18c91d46) ) \
	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */ \
	PGM_VIDEO_BIOS \
	ROM_LOAD( "pgm_t1300.u21",  0x180000, 0x800000, CRC(e748f0cb) SHA1(5843bee3a17c33648ce904af2b98c6a90aff7393) ) \
	ROM_REGION16_LE( 0x1000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */ \
	ROM_LOAD( "pgm_a1300.u1",   0x0000000, 0x0800000, CRC(fc87a405) SHA1(115c21ecc56997652e527c92654076870bc9fa51) ) /* FIXED BITS (xxxxxxxx1xxxxxxx) */ \
	ROM_LOAD( "pgm_a1301.u2",   0x0800000, 0x0800000, CRC(0c8520da) SHA1(390317857ae5baa94a4cc042874b00a811f06a63) ) /* FIXED BITS (xxxxxxxx1xxxxxxx) */ \
	ROM_REGION16_LE( 0x0800000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */ \
	ROM_LOAD( "pgm_b1300.u7",   0x0000000, 0x0800000,  CRC(ef646604) SHA1(d737ff513792962f18df88c2caa9dd71de449079) ) \
	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */ \
	PGM_AUDIO_BIOS \
	ROM_LOAD( "pgm_m1300.u5",   0x400000, 0x400000, CRC(82d4015d) SHA1(d4cdc1aec1c97cf23ff7a20ccaad822962e66ffa) )

#define DDP2_PROGRAM_102 \
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */ \
	PGM_68K_BIOS \
	ROM_LOAD16_WORD_SWAP( "v102.u8", 0x100000, 0x200000, CRC(5a9ea040) SHA1(51eaec46c368f7cfc5245e64896092f52b1193e0) ) // 09/10/01
#define DDP2_PROGRAM_101 \
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */ \
	PGM_68K_BIOS \
	ROM_LOAD16_WORD_SWAP( "v101_16m.u8", 0x100000, 0x200000, CRC(5e5786fd) SHA1(c6fc2956b5dc6a97c0d7d808a8c58aa21fa023b9) ) // 07/10/01
#define DDP2_PROGRAM_100 \
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */ \
	PGM_68K_BIOS \
	ROM_LOAD16_WORD_SWAP( "v100_u8.u8", 0x100000, 0x200000, CRC(0c8aa8ea) SHA1(57e33224622607a1df8daabf26ba063cf8a6d3fc) ) // 05/21/01


ROM_START( ddp2 )
	DDP2_PROGRAM_102
	DDP2_ASIC027_WORLD
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2101 )
	DDP2_PROGRAM_101
	DDP2_ASIC027_WORLD
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2100 )
	DDP2_PROGRAM_100
	DDP2_ASIC027_WORLD
	DDP2_COMMON_ROMS
ROM_END


ROM_START( ddp2hk )
	DDP2_PROGRAM_102
	DDP2_ASIC027_HONG_KONG
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2101hk )
	DDP2_PROGRAM_101
	DDP2_ASIC027_HONG_KONG
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2100hk )
	DDP2_PROGRAM_100
	DDP2_ASIC027_HONG_KONG
	DDP2_COMMON_ROMS
ROM_END


ROM_START( ddp2k )
	DDP2_PROGRAM_102
	DDP2_ASIC027_KOREA
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2101k )
	DDP2_PROGRAM_101
	DDP2_ASIC027_KOREA
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2100k )
	DDP2_PROGRAM_100
	DDP2_ASIC027_KOREA
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2j )
	DDP2_PROGRAM_102
	DDP2_ASIC027_JAPAN
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2101j )
	DDP2_PROGRAM_101
	DDP2_ASIC027_JAPAN
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2100j )
	DDP2_PROGRAM_100
	DDP2_ASIC027_JAPAN
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2t )
	DDP2_PROGRAM_102
	DDP2_ASIC027_TAIWAN
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2101t )
	DDP2_PROGRAM_101
	DDP2_ASIC027_TAIWAN
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2100t )
	DDP2_PROGRAM_100
	DDP2_ASIC027_TAIWAN
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2c )
	DDP2_PROGRAM_102
	DDP2_ASIC027_CHINA
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2101c )
	DDP2_PROGRAM_101
	DDP2_ASIC027_CHINA
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2100c )
	DDP2_PROGRAM_100
	DDP2_ASIC027_CHINA
	DDP2_COMMON_ROMS
ROM_END




/*

Dragon World 2001 IGS PGM cart

Top board
---------
PCB Number: IGS PCB-0349-01-FL
22MHz OSC
2x 27C4096 EPROMs at U12, U22
1x 27C160 EPROM at U11
2x PALs at U4, U6
3x 62256 SRAM at U1, U23, U24
IGS027A at U7

Bottom Board
------------
PCB Number: IGS PCB-0350-00T-FL-A
4x 27C160 EPROMs at U2, U3, U7, U9

*/

ROM_START( dw2001 )
	/* All ROMs labels are on stickers */
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "dw2001_u22.u22", 0x100000, 0x80000, CRC(5cabed92) SHA1(d513e353c5c4695b16228e0bda9388c396aa4a81) ) // 02/21/01 16:05:16

	ROM_REGION( 0x4000, "prot", ROMREGION_ERASEFF ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dw2001_igs027a_japan.bin", 0x000000, 0x04000, CRC(3a79159b) SHA1(0d693c798ce24c6a749669be8c7b1e4633409e49) )

	ROM_REGION32_LE( 0x4000000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "dw2001_u12.u12", 0x000000, 0x80000, CRC(973db1ab) SHA1(cc35e1a8534fa5d59d888f530769bae4e08c62ca) ) // external ARM data rom (encrypted)

	ROM_REGION( 0x500000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "dw2001_u11.u11",    0x180000, 0x200000, CRC(b27cf093) SHA1(7c5736a3d72b89742da1c92b2604d66e48b95e56) )

	ROM_REGION16_LE( 0x0400000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "dw2001_u2.u2",    0x000000, 0x200000, CRC(d11c733c) SHA1(8faad32e8e215631a2263bdd51a9ae434540d028) )
	ROM_LOAD( "dw2001_u3.u3",    0x200000, 0x200000, CRC(1435aef2) SHA1(582eb9f6415c89418401be7ebad041adeb600515) )

	ROM_REGION16_LE( 0x0200000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "dw2001_u9.u9",    0x000000, 0x200000, CRC(ccbca572) SHA1(4d3512e82cb65e5cdfcc6cb18deec9f4a6dd350a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "dw2001_u7.u7",      0x200000, 0x200000, CRC(4ea62f21) SHA1(318f8a1ff5d4ff029a1c4133fe7acc2fc185d112) )
ROM_END

ROM_START( dwpc )
	/* All ROMs labels are on stickers */
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "dwpc_v110cn_u22.u22", 0x100000, 0x80000, CRC(64f22362) SHA1(5f7c58498ae5cbec1c36eaa65c75287c36b6bd52) ) // 03/19/02 11:13:16

	ROM_REGION( 0x4000, "prot", ROMREGION_ERASEFF ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dw2001_igs027a_japan.bin", 0x000000, 0x04000, BAD_DUMP CRC(3a79159b) SHA1(0d693c798ce24c6a749669be8c7b1e4633409e49) ) // this was dumped for the Japanese version

	ROM_REGION32_LE( 0x4000000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "dwpc_v110cn_u12.u12", 0x000000, 0x80000, CRC(5bb1ee6a) SHA1(7844e7eb8c10a5aeb18c6057d9dc2fee7ff822ba) ) // external ARM data rom (encrypted)

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "dwpc_v110cn_u11.u11",    0x180000, 0x400000, CRC(db219cb8) SHA1(8af5a8dac8db93a7720675c64212293d7eab106d) )

	ROM_REGION16_LE( 0x0400000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "dwpc_v101xx_u2.u2",    0x000000, 0x200000, CRC(48b2f407) SHA1(9b3c0f315805aaca72c9dc3a35406f775856a8bb) )
	ROM_LOAD( "dwpc_v101xx_u3.u3",    0x200000, 0x200000, CRC(3bb45a97) SHA1(5c5cd5a241ed25dbb83690cb879472b60cd03260) )

	ROM_REGION16_LE( 0x0200000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "dwpc_v101xx_u9.u9",    0x000000, 0x200000, CRC(481b89b1) SHA1(6a241dc3b4a53ce320f3f17e972ffffe46adda4f) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "dwpc_v101xx_u7.u7",      0x200000, 0x200000, CRC(5cf9bada) SHA1(c5868a31e09e6909c724411a402d8964c29584fc) )
ROM_END

ROM_START( dwpc101j )
	/* All ROMs labels are on stickers */
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "dwpc_v101jp_u22.u22", 0x100000, 0x80000, CRC(b93027c0) SHA1(602e5f651ccb63e6465ebd7762d8d2dcf7d54077) ) // 09/26/01 10:23:26

	ROM_REGION( 0x4000, "prot", ROMREGION_ERASEFF ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dw2001_igs027a_japan.bin", 0x000000, 0x04000, CRC(3a79159b) SHA1(0d693c798ce24c6a749669be8c7b1e4633409e49) )

	ROM_REGION32_LE( 0x4000000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "dwpc_v100jp_u12.u12", 0x000000, 0x80000, CRC(0d112126) SHA1(2b569b8ef974d1d9906cc052eee63b869c8d4fa4) ) // external ARM data rom (encrypted)

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "dwpc_v100jp_u11.u11",    0x180000, 0x400000, CRC(c29d8831) SHA1(0ae93ef31fffc244111f636d47e5762d3ba23fe5) )

	ROM_REGION16_LE( 0x0400000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "dwpc_v100jp_u2.u2",    0x000000, 0x200000, CRC(e7115763) SHA1(f1bf06e9434a3b962166849f51b9dc3a74d7f2a4) )
	ROM_LOAD( "dwpc_v100jp_u3.u3",    0x200000, 0x200000, CRC(49c184a4) SHA1(320504adf596c38db56247e9cef02e7c7a363ccb) )

	ROM_REGION16_LE( 0x0200000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "dwpc_v100jp_u9.u9",    0x000000, 0x200000, CRC(412b9913) SHA1(52fc42a966575e02991aa92382b855744f44854a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "dwpc_v100jp_u7.u7",      0x200000, 0x200000, CRC(5cf9bada) SHA1(c5868a31e09e6909c724411a402d8964c29584fc) )
ROM_END

/*

Puzzli 2
IGS, 2001

Cart for IGS PGM system. The layout of the PCB is virtually identical to Puzzle Star.

PCB Layout
----------

IGS PCB NO- 0259
|-----------------------------------------------|
|                        U6 U7                  |
|         |-------|                             |
|         |IGS027A|                             |
|         |       |                     T0900.U9|
|         |       |                             |
|         |-------|                             |
|          2SP_V200.U3                          |
|          2SP_V200.U4    U5   PAL              |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      IGS027A     - Custom IGS IC, ARM7/9? based CPU (QFP120, stamped 'IGS027A')
      T0900.U9    - 16MBit MaskROM (SOP44)
      2SP_V200.U3 - MX27C4000 512K x8 EPROM (DIP32, labelled '2SP V200 U3')
      2SP_V200.U4 - MX27C4000 512K x8 EPROM (DIP32, labelled '2SP V200 U4')
      PAL         - AMD PALCE22V10 PAL (DIP24, labelled 'EL U8')
      U5          - Unpopulated position for 16MBit MaskROM (DIP42)
      U6, U7      - Unpopulated position for 74LS245 logic chip (x2)


IGS PCB NO- 0258
|-----------------------------------------------|
|                                               |
|                                               |
|                                               |
|                                               |
|   *    M0900.U2   A0900.U3   B0900.U4         |
|                                               |
|                                               |
|                                               |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      *  - Unpopulated position for Oki MSM27C3202CZ 32MBit MaskROM (TSOP48 Type II)
      U2 - 32MBit MaskROM (DIP42, Byte mode)
      U3 - 32MBit MaskROM (SOP44)
      U4 - 16MBit MaskROM (SOP44)

*/

ROM_START( puzzli2 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v100_u5.u5",     0x100000, 0x200000, CRC(1abb4595) SHA1(860bb49efc3cb55b6b9846f5ab787d6fd586432d) ) // PUZZLI-2 V0001

	ROM_REGION( 0x4000, "prot", ROMREGION_ERASEFF ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "puzzli2_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x500000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0900.u9",    0x180000, 0x200000, CRC(70615611) SHA1(a46d4aa71396947b427f9ba4ba0e636876c09d6b) )

	ROM_REGION16_LE( 0x0400000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0900.u3",    0x0000000, 0x0400000, CRC(14911251) SHA1(e0d10ef50c408dbcf0907f81d4f0e49aeb651a6c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION16_LE( 0x0200000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0900.u4",    0x0000000, 0x0200000,  CRC(6f0638b6) SHA1(14b315fe9e80b3314bb63487e6ea9ce04c9703bd) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0900.u2",    0x400000, 0x400000, CRC(9ea7af2e) SHA1(d2593d391a93c5cf5a554750c32886dea6599b3d) )
ROM_END


ROM_START( puzzli2s )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "2sp_v200.u3",     0x100000, 0x080000, CRC(2a5ba8a6) SHA1(4c87b849fd6f39152e3e2ef699b78ce24b3fb6d0) ) // PUZZLI-2 V200
	ROM_LOAD16_BYTE( "2sp_v200.u4",     0x100001, 0x080000, CRC(fa5c86c1) SHA1(11c219722b891b775c0f7f9bc8276cdd8f74d657) )

	ROM_REGION( 0x4000, "prot", ROMREGION_ERASEFF ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "puzzli2_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x500000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0900.u9",    0x180000, 0x200000, CRC(70615611) SHA1(a46d4aa71396947b427f9ba4ba0e636876c09d6b) )

	ROM_REGION16_LE( 0x0400000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0900.u3",    0x0000000, 0x0400000, CRC(14911251) SHA1(e0d10ef50c408dbcf0907f81d4f0e49aeb651a6c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION16_LE( 0x0200000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0900.u4",    0x0000000, 0x0200000,  CRC(6f0638b6) SHA1(14b315fe9e80b3314bb63487e6ea9ce04c9703bd) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0900.u2",    0x400000, 0x400000, CRC(9ea7af2e) SHA1(d2593d391a93c5cf5a554750c32886dea6599b3d) )
ROM_END


/*

Martial Masters
IGS, 2001

Cart for IGS PGM system. This game is a straight rip-off of any of the
late side-by-side fighting games made by SNK or Capcom such as King Of Fighters
or Super Street Fighter II etc


PCB Layout
----------

IGS PCB-0293-01
|-----------------------------------------------|
| 62256              62256         IGS027A      |
| 62256                                         |
|                      *                        |
|                                               |
| PAL                                           |
|                                               |
| PAL             V102_16M.U10  T1000.U3        |
|                                               |
|                 V104_32M.U9              22MHz|
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      62256        - 32K x8 SRAM (SOJ28)
      IGS027A      - Custom IGS IC, ARM7 based CPU with internal 64K ROM (QFP120)
      T1000.U3     - 23C6410 64MBit MaskROM (SOP44)
      V102_16M.U10 - MX29F1610MC 16MBit SOP44 FlashROM mounted onto a tiny DIP42 to SOP44 adapter board
                     (manufactured by IGS) which is plugged into a standard DIP42 socket. This chip was
                     read directly on the adapter as a 27C160 EPROM. The socket is wired to accept 32MBit
                     DIP42 EPROMs.
      V104_32M.U9  - M27C3202CZ 32MBit TSOP48 Type II OTP MaskROM mounted onto a tiny DIP42 to TSOP48 Type II
                     adapter board (manufactured by IGS) which is plugged into a standard DIP42 socket. This
                     chip was read directly on the adapter as a 27C322 EPROM. The socket is wired to accept
                     32MBit DIP42 EPROMs.
      *            - Unpopulated position for 62256 SRAM


IGS PCB-0292-00
|-----------------------------------------------|
| A1000.U3         A1002.U6           A1004.U10 |
|          A1001.U4         A1003.U8            |
|                                               |
|                                               |
|                                               |
|                                               |
|                  M1001.U7           B1001.U11 |
|          M1000.U5         B1000.U9            |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|



*/

ROM_START( martmast )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v104_32m.u9",    0x100000, 0x400000, CRC(cfd9dff4) SHA1(328eaf6ac49a73265ee4e0f992b1b1312f49877b) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "martial_masters_v102_usa.asic", 0x000000, 0x04000, CRC(a6c0828c) SHA1(0a5bda56dca264c3c7ff7698b8f699563f203c4d) ) // not verified, could be bad

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v102_16m.u10",    0x000000, 0x200000,  CRC(18b745e6) SHA1(7bcb58dd3a2d6072f492cf0dd7181cb061c1f49d) ) // maybe incorrect size, probably needs redump

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t1000.u3",    0x180000, 0x800000, CRC(bbf879b5) SHA1(bd9a6aea34ad4001e89e62ff4b7a2292eb833c00) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1000.u3",    0x0000000, 0x0800000, CRC(43577ac8) SHA1(6eea8b455985d5bac74dcc9943cdc3c0902de6cc) )
	ROM_LOAD( "pgm_a1001.u4",    0x0800000, 0x0800000, CRC(fe7a476f) SHA1(a8c7f1f0dd3e53141aed6d927eb88a3ceebb81e4) )
	ROM_LOAD( "pgm_a1002.u6",    0x1000000, 0x0800000, CRC(62e33d38) SHA1(96163d583e25073594f8413ce263e56b66bd69a1) )
	ROM_LOAD( "pgm_a1003.u8",    0x1800000, 0x0800000, CRC(b2c4945a) SHA1(7b18287a2db56db3651cfd4deb607af53522fefd) )
	ROM_LOAD( "pgm_a1004.u10",   0x2000000, 0x0400000, CRC(9fd3f5fd) SHA1(057531f91062be51589c6cf8f4170089b9be6380) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1000.u9",    0x0000000, 0x0800000,  CRC(c5961f6f) SHA1(a68060b10edbd084cbde79d2ed1c9084777beb10) )
	ROM_LOAD( "pgm_b1001.u11",   0x0800000, 0x0800000,  CRC(0b7e1c06) SHA1(545e15e0087f8621d593fecd8b4013f7ca311686) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1000.u5",    0x400000, 0x800000, CRC(ed407ae8) SHA1(a6e9c09b39c13e8fb7fbc89fa9f823cbeb66e901) )
	ROM_LOAD( "pgm_m1001.u7",    0xc00000, 0x400000, CRC(662d2d48) SHA1(2fcc3099d9c04456cae3b13035fb28eaf709e7d8) )
ROM_END

ROM_START( martmast104c )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v104_32m.u9",    0x100000, 0x400000, CRC(cfd9dff4) SHA1(328eaf6ac49a73265ee4e0f992b1b1312f49877b) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "martial_masters_v101_cn.asic", 0x000000, 0x04000, CRC(b3e25b7d) SHA1(6147d7ee2e11636521df1bb96ed5da8ad21b2a57) ) // not verified, could be bad

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v102_16m.u10",    0x000000, 0x200000,  CRC(18b745e6) SHA1(7bcb58dd3a2d6072f492cf0dd7181cb061c1f49d) ) // maybe incorrect size, probably needs redump

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t1000.u3",    0x180000, 0x800000, CRC(bbf879b5) SHA1(bd9a6aea34ad4001e89e62ff4b7a2292eb833c00) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1000.u3",    0x0000000, 0x0800000, CRC(43577ac8) SHA1(6eea8b455985d5bac74dcc9943cdc3c0902de6cc) )
	ROM_LOAD( "pgm_a1001.u4",    0x0800000, 0x0800000, CRC(fe7a476f) SHA1(a8c7f1f0dd3e53141aed6d927eb88a3ceebb81e4) )
	ROM_LOAD( "pgm_a1002.u6",    0x1000000, 0x0800000, CRC(62e33d38) SHA1(96163d583e25073594f8413ce263e56b66bd69a1) )
	ROM_LOAD( "pgm_a1003.u8",    0x1800000, 0x0800000, CRC(b2c4945a) SHA1(7b18287a2db56db3651cfd4deb607af53522fefd) )
	ROM_LOAD( "pgm_a1004.u10",   0x2000000, 0x0400000, CRC(9fd3f5fd) SHA1(057531f91062be51589c6cf8f4170089b9be6380) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1000.u9",    0x0000000, 0x0800000,  CRC(c5961f6f) SHA1(a68060b10edbd084cbde79d2ed1c9084777beb10) )
	ROM_LOAD( "pgm_b1001.u11",   0x0800000, 0x0800000,  CRC(0b7e1c06) SHA1(545e15e0087f8621d593fecd8b4013f7ca311686) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1000.u5",    0x400000, 0x800000, CRC(ed407ae8) SHA1(a6e9c09b39c13e8fb7fbc89fa9f823cbeb66e901) )
	ROM_LOAD( "pgm_m1001.u7",    0xc00000, 0x400000, CRC(662d2d48) SHA1(2fcc3099d9c04456cae3b13035fb28eaf709e7d8) )
ROM_END

ROM_START( martmast103c )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v103_32m.u9",    0x100000, 0x400000, CRC(df5ffbe9) SHA1(cd37900b3741707f81077ca07565997031409131) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "martial_masters_v101_cn.asic", 0x000000, 0x04000, CRC(b3e25b7d) SHA1(6147d7ee2e11636521df1bb96ed5da8ad21b2a57) ) // v102 on the PCB the maincpu ROM was dumped from. v102 Chinese ASIC hasn't been dumped yet.

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v102_16m.u10",    0x000000, 0x200000,  CRC(18b745e6) SHA1(7bcb58dd3a2d6072f492cf0dd7181cb061c1f49d) ) // maybe incorrect size, probably needs redump

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t1000.u3",    0x180000, 0x800000, CRC(bbf879b5) SHA1(bd9a6aea34ad4001e89e62ff4b7a2292eb833c00) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1000.u3",    0x0000000, 0x0800000, CRC(43577ac8) SHA1(6eea8b455985d5bac74dcc9943cdc3c0902de6cc) )
	ROM_LOAD( "pgm_a1001.u4",    0x0800000, 0x0800000, CRC(fe7a476f) SHA1(a8c7f1f0dd3e53141aed6d927eb88a3ceebb81e4) )
	ROM_LOAD( "pgm_a1002.u6",    0x1000000, 0x0800000, CRC(62e33d38) SHA1(96163d583e25073594f8413ce263e56b66bd69a1) )
	ROM_LOAD( "pgm_a1003.u8",    0x1800000, 0x0800000, CRC(b2c4945a) SHA1(7b18287a2db56db3651cfd4deb607af53522fefd) )
	ROM_LOAD( "pgm_a1004.u10",   0x2000000, 0x0400000, CRC(9fd3f5fd) SHA1(057531f91062be51589c6cf8f4170089b9be6380) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1000.u9",    0x0000000, 0x0800000,  CRC(c5961f6f) SHA1(a68060b10edbd084cbde79d2ed1c9084777beb10) )
	ROM_LOAD( "pgm_b1001.u11",   0x0800000, 0x0800000,  CRC(0b7e1c06) SHA1(545e15e0087f8621d593fecd8b4013f7ca311686) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1000.u5",    0x400000, 0x800000, CRC(ed407ae8) SHA1(a6e9c09b39c13e8fb7fbc89fa9f823cbeb66e901) )
	ROM_LOAD( "pgm_m1001.u7",    0xc00000, 0x400000, CRC(662d2d48) SHA1(2fcc3099d9c04456cae3b13035fb28eaf709e7d8) )
ROM_END

ROM_START( martmast102c )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "mm_v102_u9.u9",    0x100000, 0x400000, CRC(bb24b92a) SHA1(442cb9e3f51727be82f71c078c5c3e49dc1a23f0) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "martial_masters_v101_cn.asic", 0x000000, 0x04000, CRC(b3e25b7d) SHA1(6147d7ee2e11636521df1bb96ed5da8ad21b2a57) ) // not verified, could be bad

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "mm_v101_u10.u10", 0x000000, 0x400000,   CRC(41b9497c) SHA1(a941abbb938fb769112eaad374cc513a71aaa556) ) // double size wrt to other sets

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t1000.u3",    0x180000, 0x800000, CRC(bbf879b5) SHA1(bd9a6aea34ad4001e89e62ff4b7a2292eb833c00) )

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1000.u3",    0x0000000, 0x0800000, CRC(43577ac8) SHA1(6eea8b455985d5bac74dcc9943cdc3c0902de6cc) )
	ROM_LOAD( "pgm_a1001.u4",    0x0800000, 0x0800000, CRC(fe7a476f) SHA1(a8c7f1f0dd3e53141aed6d927eb88a3ceebb81e4) )
	ROM_LOAD( "pgm_a1002.u6",    0x1000000, 0x0800000, CRC(62e33d38) SHA1(96163d583e25073594f8413ce263e56b66bd69a1) )
	ROM_LOAD( "pgm_a1003.u8",    0x1800000, 0x0800000, CRC(b2c4945a) SHA1(7b18287a2db56db3651cfd4deb607af53522fefd) )
	ROM_LOAD( "pgm_a1004.u10",   0x2000000, 0x0400000, CRC(9fd3f5fd) SHA1(057531f91062be51589c6cf8f4170089b9be6380) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1000.u9",    0x0000000, 0x0800000,  CRC(c5961f6f) SHA1(a68060b10edbd084cbde79d2ed1c9084777beb10) )
	ROM_LOAD( "pgm_b1001.u11",   0x0800000, 0x0800000,  CRC(0b7e1c06) SHA1(545e15e0087f8621d593fecd8b4013f7ca311686) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1000.u5",    0x400000, 0x800000, CRC(ed407ae8) SHA1(a6e9c09b39c13e8fb7fbc89fa9f823cbeb66e901) )
	ROM_LOAD( "pgm_m1001.u7",    0xc00000, 0x400000, CRC(662d2d48) SHA1(2fcc3099d9c04456cae3b13035fb28eaf709e7d8) )
ROM_END

/*

Demon Front
IGS, 2002

Cart for IGS PGM system. This game is a straight rip-off of Metal Slug.

PCB Layout
----------

IGS PCB-0387-02-FV
|-----------------------------------------------|
| BS616LV1010                      IGS027A      |
| BS616LV1010                                   |
|                                               |
|                              *     BS616LV1010|
|            PAL  PAL                           |
|                                               |
| V102_16M.U5        V101_32M.U26               |
|                                        PAL    |
|                             T04501.U29   22MHz|
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      BS616LV1010  - 64K x16 SRAM (TSOP44)
      IGS027A      - Custom IGS IC, ARM7 based CPU (QFP120)
      T04501.U29   - 23C6410 64MBit MaskROM (SOP44)
      V102_16M.U5  - 27C160 16MBit EPROM (DIP42)
      V101_32M.U26 - 27C322 32MBit EPROM (DIP42)
      *            - Unpopulated position for 29F1610 16MBit SOP44 FlashROM, linked to IGS027A


IGS PCB-0390-00-FV-A
|-----------------------------------------------|
| A04501.U3  A04502.U4  A04503.U6   U8*     U10*|
|                                               |
|                                               |
|                                               |
|                                               |
|     W04501.U5   U7*    B04501.U9   B04502.U11 |
|                                               |
|                                               |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      *  - Unpopulated SOP44 ROM position.

*/


ROM_START( dmnfrnt ) // same romset has also been found on a single PCB board (set dmnfrnta in FBNeo)
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v105_16m.u5",    0x100000, 0x200000, CRC(bda083bd) SHA1(58d6438737a2c43aa8bbcb7f34fb51375b781b1c) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dmnfrnt_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v105_32m.u26", 0x000000, 0x400000,  CRC(c798c2ef) SHA1(91e364c33b935293fa765ca521cdb67ac45ec70f) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "igs_t04501w064.u29",    0x180000, 0x800000, CRC(900eaaac) SHA1(4033cb7b28fcadb92d5af3ea7fdd1c22747618fd) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "igs_a04501w064.u3",     0x0000000, 0x0800000, CRC(9741bea6) SHA1(e3e904249be228628c8c2bd3495cda23586dc048) )
	ROM_LOAD( "igs_a04502w064.u4",     0x0800000, 0x0800000, CRC(e104f405) SHA1(124b3deed3e838f8bae6c7d78bdd788859597585) )
	ROM_LOAD( "igs_a04503w064.u6",     0x1000000, 0x0800000, CRC(bfd5cfe3) SHA1(fbe4c0a2987c2036df707b86597d78124ee2e665) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "igs_b04501w064.u9",     0x0000000, 0x0800000,  CRC(29320b7d) SHA1(59c78805e666f912df201c34616744f46057937b) )
	ROM_LOAD( "igs_b04502w016.u11",    0x0800000, 0x0200000,  CRC(578c00e9) SHA1(14235cc8b0f8c7dd659512f017a2d4aacd91d89d) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "igs_w04501b064.u5",     0x400000, 0x800000, CRC(3ab58137) SHA1(b221f7e551ff0bfa3fd97b6ebedbac69442a66e9) )
ROM_END

ROM_START( dmnfrnt103 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v103_16m.u5",    0x100000, 0x200000, CRC(2ddafb3d) SHA1(c7d22e007952459de6d23a42ce32aab67b493fc3) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dmnfrnt_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v103_32m.u26", 0x000000, 0x400000, CRC(e78383a3) SHA1(7ae99e93489e79fb1e4240124d22b6002fb7fe18) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "igs_t04501w064.u29",    0x180000, 0x800000, CRC(900eaaac) SHA1(4033cb7b28fcadb92d5af3ea7fdd1c22747618fd) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "igs_a04501w064.u3",     0x0000000, 0x0800000, CRC(9741bea6) SHA1(e3e904249be228628c8c2bd3495cda23586dc048) )
	ROM_LOAD( "igs_a04502w064.u4",     0x0800000, 0x0800000, CRC(e104f405) SHA1(124b3deed3e838f8bae6c7d78bdd788859597585) )
	ROM_LOAD( "igs_a04503w064.u6",     0x1000000, 0x0800000, CRC(bfd5cfe3) SHA1(fbe4c0a2987c2036df707b86597d78124ee2e665) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "igs_b04501w064.u9",     0x0000000, 0x0800000,  CRC(29320b7d) SHA1(59c78805e666f912df201c34616744f46057937b) )
	ROM_LOAD( "igs_b04502w016.u11",    0x0800000, 0x0200000,  CRC(578c00e9) SHA1(14235cc8b0f8c7dd659512f017a2d4aacd91d89d) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "igs_w04501b064.u5",     0x400000, 0x800000, CRC(3ab58137) SHA1(b221f7e551ff0bfa3fd97b6ebedbac69442a66e9) )
ROM_END

ROM_START( dmnfrnt102 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v102_16m.u5",    0x100000, 0x200000, CRC(3d4d481a) SHA1(95953b8f31343389405cc722b4177ff5adf67b62) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dmnfrnt_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v101_32m.u26", 0x000000, 0x400000,  CRC(93965281) SHA1(89da198aaa7ca759cb96b5f18859a477e55fd590) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "igs_t04501w064.u29",    0x180000, 0x800000, CRC(900eaaac) SHA1(4033cb7b28fcadb92d5af3ea7fdd1c22747618fd) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "igs_a04501w064.u3",     0x0000000, 0x0800000, CRC(9741bea6) SHA1(e3e904249be228628c8c2bd3495cda23586dc048) )
	ROM_LOAD( "igs_a04502w064.u4",     0x0800000, 0x0800000, CRC(e104f405) SHA1(124b3deed3e838f8bae6c7d78bdd788859597585) )
	ROM_LOAD( "igs_a04503w064.u6",     0x1000000, 0x0800000, CRC(bfd5cfe3) SHA1(fbe4c0a2987c2036df707b86597d78124ee2e665) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "igs_b04501w064.u9",     0x0000000, 0x0800000,  CRC(29320b7d) SHA1(59c78805e666f912df201c34616744f46057937b) )
	ROM_LOAD( "igs_b04502w016.u11",    0x0800000, 0x0200000,  CRC(578c00e9) SHA1(14235cc8b0f8c7dd659512f017a2d4aacd91d89d) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "igs_w04501b064.u5",     0x400000, 0x800000, CRC(3ab58137) SHA1(b221f7e551ff0bfa3fd97b6ebedbac69442a66e9) )
ROM_END

ROM_START( dmnfrntpcb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p02s.u42",    0x00000, 0x020000, CRC(78c15fa2) SHA1(885a6558e022602cc6f482ac9667ba9f61e75092) ) /* IGS PGM P02S 1A3708A1A0 S002838  (uses standard PGM v2 bios) */
	ROM_LOAD16_WORD_SWAP( "demon_front_v107kr_u43.u43",    0x100000, 0x200000,  CRC(671d8a31) SHA1(a0c2af67d7c56b4b355883892a47640fc72408a1) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dmnfrnt_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "demon_front_v107kr_u62.u62",     0x000000, 0x400000, CRC(cb94772e) SHA1(4213600be41fd9ea295dd308920b1d89b635724f) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "igs_t04501w064.u71",    0x180000, 0x800000, CRC(900eaaac) SHA1(4033cb7b28fcadb92d5af3ea7fdd1c22747618fd) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "igs_a04501w064.u30",    0x0000000, 0x0800000, CRC(9741bea6) SHA1(e3e904249be228628c8c2bd3495cda23586dc048) )
	ROM_LOAD( "igs_a04502w064.u31",    0x0800000, 0x0800000, CRC(e104f405) SHA1(124b3deed3e838f8bae6c7d78bdd788859597585) )
	ROM_LOAD( "igs_a04503w064.u32",    0x1000000, 0x0800000, CRC(bfd5cfe3) SHA1(fbe4c0a2987c2036df707b86597d78124ee2e665) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "igs_b04501w064.u40",    0x0000000, 0x0800000,  CRC(29320b7d) SHA1(59c78805e666f912df201c34616744f46057937b) )
	ROM_LOAD( "igs_b04502w016.u41",   0x0800000, 0x0200000,  CRC(578c00e9) SHA1(14235cc8b0f8c7dd659512f017a2d4aacd91d89d) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "igs_w04501b064.u8",    0x400000, 0x800000, CRC(3ab58137) SHA1(b221f7e551ff0bfa3fd97b6ebedbac69442a66e9) )
ROM_END

// the (readable part of) internal roms have been verified on
// Japan PCB
// Overseas Cart
// the two dumps differed by the region byte and internal checksum byte

ROM_START( theglad )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v101_u6.u6",      0x100000, 0x080000, CRC(f799e866) SHA1(dccc3c903357c40c3cf85ac0ae8fc12fb0f853a6) ) // V101 05/15/03 09:00:32

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "theglad_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "theglad_igs027a_v100_overseas.bin", 0x0188, 0x3e78, CRC(02fe6f52) SHA1(0b0ddf4507856cfc5b7d4ef7e4c5375254c2a024) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v107_u26.u26", 0x000000, 0x200000,  CRC(f7c61357) SHA1(52d31c464dfc83c5371b078cb6b73c0d0e0d57e3) ) // 06/06/03 16:17:27 V107

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "igs_t04601w64m.u33",   0x180000, 0x800000, CRC(e5dab371) SHA1(2e3c93958eb0326b6b84b95c2168626f26bbac76) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "igs_a04601w64m.u2",    0x0000000, 0x0800000,  CRC(d9b2e004) SHA1(8e1882b800fe9f12d7d49303e7417ba5b6f8ef85) )
	ROM_LOAD( "igs_a04602w64m.u4",    0x0800000, 0x0800000,  CRC(14f22308) SHA1(7fad54704e8c97eab723f53dfb50fb3e7bb606d2) )
	ROM_LOAD( "igs_a04603w64m.u6",    0x1000000, 0x0800000,  CRC(8f621e17) SHA1(b0f87f378e0115d0c95017ca0f1b0d508827a7c6) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "igs_b04601w64m.u11",   0x0000000, 0x0800000, CRC(ee72bccf) SHA1(73c25fe659f6c903447066e4ef83d2f580449d76) )
	ROM_LOAD( "igs_b04602w32m.u12",   0x0800000, 0x0400000, CRC(7dba9c38) SHA1(a03d509274e8f6a500a7ebe2da5aab8bed4e7f2f) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "igs_w04601b64m.u1",    0x400000, 0x800000, CRC(5f15ddb3) SHA1(c38dcef8e06802a84e42a7fc9fa505475fc3ac65) )
ROM_END

ROM_START( theglad100 ) // is this actually a pre-v100 proto?
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "u6.rom",       0x100000, 0x080000, CRC(14c85212) SHA1(8d2489708e176a2c460498a13173be01f645b79e) ) // 01/06/03 09:27:02

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "theglad_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "theglad_igs027a_older.bin", 0x0188, 0x3e78, BAD_DUMP CRC(02fe6f52) SHA1(0b0ddf4507856cfc5b7d4ef7e4c5375254c2a024) ) // this is wrong for this set, we patch it to work

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "u2.rom", 0x000000, 0x200000,  CRC(c7bcf2ae) SHA1(10bc012c83987f594d5375a51bc4be2e17568a81) ) // 01/16/03 10:39:25 V100

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "igs_t04601w64m.u33",   0x180000, 0x800000, CRC(e5dab371) SHA1(2e3c93958eb0326b6b84b95c2168626f26bbac76) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "igs_a04601w64m.u2",    0x0000000, 0x0800000,  CRC(d9b2e004) SHA1(8e1882b800fe9f12d7d49303e7417ba5b6f8ef85) )
	ROM_LOAD( "igs_a04602w64m.u4",    0x0800000, 0x0800000,  CRC(14f22308) SHA1(7fad54704e8c97eab723f53dfb50fb3e7bb606d2) )
	ROM_LOAD( "igs_a04603w64m.u6",    0x1000000, 0x0800000,  CRC(8f621e17) SHA1(b0f87f378e0115d0c95017ca0f1b0d508827a7c6) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "igs_b04601w64m.u11",   0x0000000, 0x0800000, CRC(ee72bccf) SHA1(73c25fe659f6c903447066e4ef83d2f580449d76) )
	ROM_LOAD( "igs_b04602w32m.u12",   0x0800000, 0x0400000, CRC(7dba9c38) SHA1(a03d509274e8f6a500a7ebe2da5aab8bed4e7f2f) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "igs_w04601b64m.u1",    0x400000, 0x800000, CRC(5f15ddb3) SHA1(c38dcef8e06802a84e42a7fc9fa505475fc3ac65) )
ROM_END


ROM_START( theglad101 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v100.u6",       0x100000, 0x080000, CRC(bcf3b172) SHA1(df7e2808c0341be0a59eefa852c857a3a919223e) ) // 02/25/03 17:42:51

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "theglad_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "theglad_igs027a_v100_overseas.bin", 0x0188, 0x3e78, CRC(02fe6f52) SHA1(0b0ddf4507856cfc5b7d4ef7e4c5375254c2a024) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v101.u26", 0x000000, 0x200000, CRC(23faec02) SHA1(9065d55c2a14e6889e735a452bbc32530056645a) ) // 03/13/03 14:06:44 V101

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "igs_t04601w64m.u33",   0x180000, 0x800000, CRC(e5dab371) SHA1(2e3c93958eb0326b6b84b95c2168626f26bbac76) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "igs_a04601w64m.u2",    0x0000000, 0x0800000,  CRC(d9b2e004) SHA1(8e1882b800fe9f12d7d49303e7417ba5b6f8ef85) )
	ROM_LOAD( "igs_a04602w64m.u4",    0x0800000, 0x0800000,  CRC(14f22308) SHA1(7fad54704e8c97eab723f53dfb50fb3e7bb606d2) )
	ROM_LOAD( "igs_a04603w64m.u6",    0x1000000, 0x0800000,  CRC(8f621e17) SHA1(b0f87f378e0115d0c95017ca0f1b0d508827a7c6) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "igs_b04601w64m.u11",   0x0000000, 0x0800000, CRC(ee72bccf) SHA1(73c25fe659f6c903447066e4ef83d2f580449d76) )
	ROM_LOAD( "igs_b04602w32m.u12",   0x0800000, 0x0400000, CRC(7dba9c38) SHA1(a03d509274e8f6a500a7ebe2da5aab8bed4e7f2f) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "igs_w04601b64m.u1",    0x400000, 0x800000, CRC(5f15ddb3) SHA1(c38dcef8e06802a84e42a7fc9fa505475fc3ac65) )
ROM_END

ROM_START( theglad104 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v100.u6",       0x100000, 0x080000, CRC(bcf3b172) SHA1(df7e2808c0341be0a59eefa852c857a3a919223e) ) // 02/25/03 17:42:51

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "theglad_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "theglad_igs027a_v100_overseas.bin", 0x0188, 0x3e78, BAD_DUMP CRC(02fe6f52) SHA1(0b0ddf4507856cfc5b7d4ef7e4c5375254c2a024) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v104_u26.u26", 0x000000, 0x200000,  CRC(81b5df6d) SHA1(63ab9806be458cfe9e5561606fd200c599dcb527) ) // 04/02/03 09:39:46 V104

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "igs_t04601w64m.u33",   0x180000, 0x800000, CRC(e5dab371) SHA1(2e3c93958eb0326b6b84b95c2168626f26bbac76) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "igs_a04601w64m.u2",    0x0000000, 0x0800000,  CRC(d9b2e004) SHA1(8e1882b800fe9f12d7d49303e7417ba5b6f8ef85) )
	ROM_LOAD( "igs_a04602w64m.u4",    0x0800000, 0x0800000,  CRC(14f22308) SHA1(7fad54704e8c97eab723f53dfb50fb3e7bb606d2) )
	ROM_LOAD( "igs_a04603w64m.u6",    0x1000000, 0x0800000,  CRC(8f621e17) SHA1(b0f87f378e0115d0c95017ca0f1b0d508827a7c6) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "igs_b04601w64m.u11",   0x0000000, 0x0800000, CRC(ee72bccf) SHA1(73c25fe659f6c903447066e4ef83d2f580449d76) )
	ROM_LOAD( "igs_b04602w32m.u12",   0x0800000, 0x0400000, CRC(7dba9c38) SHA1(a03d509274e8f6a500a7ebe2da5aab8bed4e7f2f) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "igs_w04601b64m.u1",    0x400000, 0x800000, CRC(5f15ddb3) SHA1(c38dcef8e06802a84e42a7fc9fa505475fc3ac65) )
ROM_END

ROM_START( thegladpcb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "bios.u42",      0x000000, 0x020000, CRC(517cf7a2) SHA1(f5720b29e3be6ec22be03a768618cb2a1aa4ade7) ) // V0001-001J
	ROM_LOAD16_WORD_SWAP( "v100_u43.u43",  0x100000, 0x080000, CRC(bcf3b172) SHA1(df7e2808c0341be0a59eefa852c857a3a919223e) ) // 02/25/03 17:42:51

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "thegladpcb_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "thegladpcb_igs027a_v100_japan.bin", 0x0188, 0x3e78, CRC(d7f06e2d) SHA1(9c3aca7a487f5329d84731e2c63d5ed591bf9d24) )    // from 'thegladpcb set'

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v101_u62.u62", 0x000000, 0x200000, CRC(23faec02) SHA1(9065d55c2a14e6889e735a452bbc32530056645a) ) // 03/13/03 14:06:44 V101

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.u72",         0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // standard PGM tx bios
	ROM_LOAD( "igs_t04601w64m.u71",   0x180000, 0x800000, CRC(e5dab371) SHA1(2e3c93958eb0326b6b84b95c2168626f26bbac76) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "igs_a04601w64m.u30",   0x0000000, 0x0800000,  CRC(d9b2e004) SHA1(8e1882b800fe9f12d7d49303e7417ba5b6f8ef85) )
	ROM_LOAD( "igs_a04602w64m.u31",   0x0800000, 0x0800000,  CRC(14f22308) SHA1(7fad54704e8c97eab723f53dfb50fb3e7bb606d2) )
	ROM_LOAD( "igs_a04603w64m.u32",   0x1000000, 0x0800000,  CRC(8f621e17) SHA1(b0f87f378e0115d0c95017ca0f1b0d508827a7c6) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "igs_b04601w64m.u40",   0x0000000, 0x0800000, CRC(ee72bccf) SHA1(73c25fe659f6c903447066e4ef83d2f580449d76) )
	ROM_LOAD( "igs_b04602w32m.u41",   0x0800000, 0x0400000, CRC(7dba9c38) SHA1(a03d509274e8f6a500a7ebe2da5aab8bed4e7f2f) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.u4",          0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // standard PGM sample bios
	ROM_LOAD( "igs_w04601b64m.u8",    0x400000, 0x800000, CRC(5f15ddb3) SHA1(c38dcef8e06802a84e42a7fc9fa505475fc3ac65) )
	ROM_LOAD( "wave_u29.u29",         0xc00000, 0x200000, CRC(51acb395) SHA1(65a2ecd3de2ff782f2aa0f0f905f9b18323aea64) ) // extra ROM on the PCB version for the Japanese music
ROM_END

ROM_START( thegladpcba )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "bios.u42",       0x000000, 0x020000, CRC(517cf7a2) SHA1(f5720b29e3be6ec22be03a768618cb2a1aa4ade7) ) // V0001-001J
	ROM_LOAD16_WORD_SWAP( "v100_u43.u43",  0x100000, 0x080000, CRC(bcf3b172) SHA1(df7e2808c0341be0a59eefa852c857a3a919223e) ) // 02/25/03 17:42:51

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "thegladpcb_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "thegladpcb_igs027a_v100_japan.bin", 0x0188, 0x3e78, CRC(d7f06e2d) SHA1(9c3aca7a487f5329d84731e2c63d5ed591bf9d24) )    // from 'thegladpcb set'

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v100_u62.u62", 0x000000, 0x200000, CRC(0f3f511e) SHA1(28dd8d27495cec86e968a3ea549c5b30513dbb6e) ) // 02/25/03 16:32:21 V100

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.u72",         0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // standard PGM tx bios
	ROM_LOAD( "igs_t04601w64m.u71",   0x180000, 0x800000, CRC(e5dab371) SHA1(2e3c93958eb0326b6b84b95c2168626f26bbac76) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "igs_a04601w64m.u30",   0x0000000, 0x0800000,  CRC(d9b2e004) SHA1(8e1882b800fe9f12d7d49303e7417ba5b6f8ef85) )
	ROM_LOAD( "igs_a04602w64m.u31",   0x0800000, 0x0800000,  CRC(14f22308) SHA1(7fad54704e8c97eab723f53dfb50fb3e7bb606d2) )
	ROM_LOAD( "igs_a04603w64m.u32",   0x1000000, 0x0800000,  CRC(8f621e17) SHA1(b0f87f378e0115d0c95017ca0f1b0d508827a7c6) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "igs_b04601w64m.u40",   0x0000000, 0x0800000, CRC(ee72bccf) SHA1(73c25fe659f6c903447066e4ef83d2f580449d76) )
	ROM_LOAD( "igs_b04602w32m.u41",   0x0800000, 0x0400000, CRC(7dba9c38) SHA1(a03d509274e8f6a500a7ebe2da5aab8bed4e7f2f) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.u4",          0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // standard PGM sample bios
	ROM_LOAD( "igs_w04601b64m.u8",    0x400000, 0x800000, CRC(5f15ddb3) SHA1(c38dcef8e06802a84e42a7fc9fa505475fc3ac65) )
	ROM_LOAD( "wave_u29.u29",         0xc00000, 0x200000, CRC(51acb395) SHA1(65a2ecd3de2ff782f2aa0f0f905f9b18323aea64) ) // extra ROM on the PCB version for the Japanese music
ROM_END

ROM_START( oldsplus )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v-205cn.u10",   0x100000, 0x400000, CRC(923f7246) SHA1(818ade79e9724f5a2b0cc5a647ae5d4ee0374799) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "oldsplus_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x800000, "user1", ROMREGION_ERASE00 )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "igs_t05301w064.u2",   0x180000, 0x800000, CRC(8257bbb0) SHA1(b48067b7e7081a15fddf21739b641d677c2df3d9) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "igs_a05301w064.u3",   0x0000000, 0x0800000, CRC(57946fd2) SHA1(5d79bc71a1881f3099821a9b255a5f271e0eeff6) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "igs_a05302w064.u4",   0x0800000, 0x0800000, CRC(3459a0b8) SHA1(94ab6f980b5582f1db9bb12019d03f0b6e0a06df) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "igs_a05303w064.u6",   0x1000000, 0x0800000, CRC(13475d85) SHA1(4683a3bf304fdc15ffb1c61b7957ad68b023fa33) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "igs_a05304w064.u8",   0x1800000, 0x0800000, CRC(f03ef7a6) SHA1(c18b1b622b430d5e031e65daa6819b84c3e12ef5) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "igs_b05301w064.u9",   0x0000000, 0x0800000, CRC(fd98f503) SHA1(02046ab1aa89f63bff149003d9d61776e025a92a) )
	ROM_LOAD( "igs_b05302w064.u11",  0x0800000, 0x0800000, CRC(9f6094a8) SHA1(69f6f2003ab975eae13ea6b5c2ffa40df6e6bdf6) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "igs_w05301b032.u5",   0x400000, 0x400000, CRC(86ec83bc) SHA1(067cb7ec449eacd1f49298f45a364368934db5dd) )
ROM_END

ROM_START( oldsplus203 ) // only program ROM provided for this set
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v-203cn.u10",   0x100000, 0x400000, CRC(c728cadc) SHA1(7e6acd2535b1700fdefc048bd521c305d3170108) )

	ROM_REGION( 0x4000, "prot", 0 ) // ARM protection ASIC - internal ROM
	ROM_LOAD( "oldsplus_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x800000, "user1", ROMREGION_ERASE00 )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "igs_t05301w064.u2",   0x180000, 0x800000, CRC(8257bbb0) SHA1(b48067b7e7081a15fddf21739b641d677c2df3d9) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "igs_a05301w064.u3",   0x0000000, 0x0800000, CRC(57946fd2) SHA1(5d79bc71a1881f3099821a9b255a5f271e0eeff6) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "igs_a05302w064.u4",   0x0800000, 0x0800000, CRC(3459a0b8) SHA1(94ab6f980b5582f1db9bb12019d03f0b6e0a06df) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "igs_a05303w064.u6",   0x1000000, 0x0800000, CRC(13475d85) SHA1(4683a3bf304fdc15ffb1c61b7957ad68b023fa33) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "igs_a05304w064.u8",   0x1800000, 0x0800000, CRC(f03ef7a6) SHA1(c18b1b622b430d5e031e65daa6819b84c3e12ef5) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "igs_b05301w064.u9",   0x0000000, 0x0800000, CRC(fd98f503) SHA1(02046ab1aa89f63bff149003d9d61776e025a92a) )
	ROM_LOAD( "igs_b05302w064.u11",  0x0800000, 0x0800000, CRC(9f6094a8) SHA1(69f6f2003ab975eae13ea6b5c2ffa40df6e6bdf6) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "igs_w05301b032.u5",   0x400000, 0x400000, CRC(86ec83bc) SHA1(067cb7ec449eacd1f49298f45a364368934db5dd) )
ROM_END

ROM_START( kovshp )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600h_101.rom",   0x100000, 0x400000, CRC(e1d89a19) SHA1(30e11c145652d03464b14d3cd09e4f35fff6120e) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) ) // this is the kovsh ARM rom, we intercept and modify protection calls

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0600.u2",       0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )
	// also seen split in 2 smaller ROMs
	// "s&k_fntcg_u3.u3" - t0600.rom    [1/2]  IDENTICAL - CRC(164b3c94) SHA1(f00ea66886ca6bff74bbeaa49e7f5c75c275d5d7)
	// "s&k_fntcg_u7.u7" - t0600.rom    [2/2]  IDENTICAL - CRC(b1fae5e8) SHA1(88b84879b5ce9c29081647186b3a1b003efe6dcc)

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0600.u3",       0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0601.u4",       0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0602.u6",       0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "igs_a05401w064.u8",  0x1800000, 0x0800000, CRC(4fd3413e) SHA1(5e8f3e421342bf558c77e59635f9b5d713e825c2) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0600.u9",       0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "igs_b05401w064.u11", 0x0800000, 0x0800000, CRC(60999757) SHA1(118cf058e67858958bcb931e14f5d160c7de87cc) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0600.u5",       0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovshp100 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "s__s_m-100cn.u10",   0x100000, 0x400000, CRC(e251e8e4) SHA1(af5b7c81632a39e1450d932951bed634c76b84e8) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) ) // this is the kovsh ARM rom, we intercept and modify protection calls

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0600.u2",       0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0600.u3",       0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0601.u4",       0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0602.u6",       0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "igs_a05401w064.u8",  0x1800000, 0x0800000, CRC(4fd3413e) SHA1(5e8f3e421342bf558c77e59635f9b5d713e825c2) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0600.u9",       0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "igs_b05401w064.u11", 0x0800000, 0x0800000, CRC(60999757) SHA1(118cf058e67858958bcb931e14f5d160c7de87cc) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0600.u5",       0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END



ROM_START( kovytzy )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "ytzy_v201cn.rom",   0x100000, 0x400000, CRC(f3705ea0) SHA1(e31ad474d0c2364311d21a8ce37d49919c7b999c) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) ) // this is the kovsh ARM rom, we intercept and modify protection calls

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgm_t0600.u2",       0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a0600.u3",       0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0601.u4",       0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "pgm_a0602.u6",       0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "igs_a05401w064.u8",  0x1800000, 0x0800000, CRC(4fd3413e) SHA1(5e8f3e421342bf558c77e59635f9b5d713e825c2) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b0600.u9",       0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "igs_b05401w064.u11", 0x0800000, 0x0800000, CRC(60999757) SHA1(118cf058e67858958bcb931e14f5d160c7de87cc) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m0600.u5",       0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovshxas )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "assg_v202cn.rom",   0x100000, 0x400000, CRC(3b7b627f) SHA1(b331148501f9349fbd5882fb3f184f6304e58646) ) // V202CN Oct 6 2008 09:59:26

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) ) // this is the kovsh ARM rom, we intercept and modify protection calls

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603as.rom",    0x1800000, 0x0800000, CRC(7057b37e) SHA1(85a19f23303b4d581c4fa315b2c204af92fcb424) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601as.rom",    0x0800000, 0x0800000, CRC(3784fb49) SHA1(7e85fe5b5fb8746f1321c03ad2350d2a58969d7a) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END


ROM_START( kovlsqh )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "lsqh_v200cn.rom", 0x100000, 0x400000, CRC(9935a27a) SHA1(3075935293172466c4bd997dcb67f864ae26493e) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) ) // this is the kovsh ARM rom, we intercept and modify protection calls

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "lsqh2_t01.rom",0x180000, 0x800000, CRC(d498d97f) SHA1(97a7b6d2ed1170449e7c2899448af7cbbca4c94f) )
	ROM_IGNORE( 0x800000 )  // second half identical

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "lsqh2_a01.rom", 0x0000000, 0x1000000, CRC(25ae3efd) SHA1(083d977602ddb5ad54fbdcba000cd4287de8d463) )
	ROM_LOAD( "lsqh2_a23.rom", 0x1000000, 0x1000000, CRC(7a779112) SHA1(0a7d36b3715063d8eac629b95a9bb3ecd8e54fca) )
	ROM_LOAD( "lsqh2_a45.rom", 0x2000000, 0x1000000, CRC(5d7de052) SHA1(7663b6cf09f65c4644661005a38f9aba84a32913) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "lsqh2_b01.rom", 0x0000000, 0x1000000, CRC(df7ca696) SHA1(7af3d27957a39de7e4873867c9972c05af7e7964) )

	ROM_REGION( 0xc00000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "lsqh2_m01.rom",0x400000, 0x400000, CRC(01af1b65) SHA1(6cf523fa8f1e03f974771611bb9a4e08a4d4443f) )
	ROM_IGNORE( 0x400000 )  // 400000-7fffff empty
	ROM_CONTINUE( 0x800000, 0x400000 )
	ROM_IGNORE( 0x400000 )  // c00000-ffffff empty
ROM_END

ROM_START( kovlsqh2 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "lsqh2_prg.rom",0x100000, 0x400000, CRC(d71e3d50) SHA1(bda78648bc176b0ded74118a8e340ee661bb930d) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) ) // this is the kovsh ARM rom, we intercept and modify protection calls

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "lsqh2_t01.rom",0x180000, 0x800000, CRC(d498d97f) SHA1(97a7b6d2ed1170449e7c2899448af7cbbca4c94f) )
	ROM_IGNORE( 0x800000 )  // second half identical

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "lsqh2_a01.rom", 0x0000000, 0x1000000, CRC(25ae3efd) SHA1(083d977602ddb5ad54fbdcba000cd4287de8d463) )
	ROM_LOAD( "lsqh2_a23.rom", 0x1000000, 0x1000000, CRC(7a779112) SHA1(0a7d36b3715063d8eac629b95a9bb3ecd8e54fca) )
	ROM_LOAD( "lsqh2_a45.rom", 0x2000000, 0x1000000, CRC(5d7de052) SHA1(7663b6cf09f65c4644661005a38f9aba84a32913) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "lsqh2_b01.rom", 0x0000000, 0x1000000, CRC(df7ca696) SHA1(7af3d27957a39de7e4873867c9972c05af7e7964) )

	ROM_REGION( 0xc00000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "lsqh2_m01.rom",0x400000, 0x400000, CRC(01af1b65) SHA1(6cf523fa8f1e03f974771611bb9a4e08a4d4443f) )
	ROM_IGNORE( 0x400000 )  // 400000-7fffff empty
	ROM_CONTINUE( 0x800000, 0x400000 )
	ROM_IGNORE( 0x400000 )  // c00000-ffffff empty
ROM_END


ROM_START( kovlsjb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "lsjb_prg.rom", 0x100000, 0x400000, CRC(adf06b37) SHA1(be3c0af64de374046d28492ac49c01da1ec78e40) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) ) // this is the kovsh ARM rom, we intercept and modify protection calls

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "lsqh2_t01.rom",0x180000, 0x800000, CRC(d498d97f) SHA1(97a7b6d2ed1170449e7c2899448af7cbbca4c94f) )
	ROM_IGNORE( 0x800000 )  // second half identical

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "lsqh2_a01.rom", 0x0000000, 0x1000000, CRC(25ae3efd) SHA1(083d977602ddb5ad54fbdcba000cd4287de8d463) )
	ROM_LOAD( "lsqh2_a23.rom", 0x1000000, 0x1000000, CRC(7a779112) SHA1(0a7d36b3715063d8eac629b95a9bb3ecd8e54fca) )
	ROM_LOAD( "lsqh2_a45.rom", 0x2000000, 0x1000000, CRC(5d7de052) SHA1(7663b6cf09f65c4644661005a38f9aba84a32913) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "lsqh2_b01.rom", 0x0000000, 0x1000000, CRC(df7ca696) SHA1(7af3d27957a39de7e4873867c9972c05af7e7964) )

	ROM_REGION( 0xc00000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "lsqh2_m01.rom",0x400000, 0x400000, CRC(01af1b65) SHA1(6cf523fa8f1e03f974771611bb9a4e08a4d4443f) )
	ROM_IGNORE( 0x400000 )  // 400000-7fffff empty
	ROM_CONTINUE( 0x800000, 0x400000 )
	ROM_IGNORE( 0x400000 )  // c00000-ffffff empty
ROM_END

ROM_START( kovlsjba )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "lsjba_prg.rom",0x100000, 0x400000, CRC(8b42f456) SHA1(48796e48f6f1a5f68442cf15a6b195095d443a35) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) ) // this is the kovsh ARM rom, we intercept and modify protection calls

	ROM_REGION( 0xa00000, "igs023",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "lsqh2_t01.rom",0x180000, 0x800000, CRC(d498d97f) SHA1(97a7b6d2ed1170449e7c2899448af7cbbca4c94f) )
	ROM_IGNORE( 0x800000 )  // second half identical

	ROM_REGION16_LE( 0x4000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "lsqh2_a01.rom", 0x0000000, 0x1000000, CRC(25ae3efd) SHA1(083d977602ddb5ad54fbdcba000cd4287de8d463) )
	ROM_LOAD( "lsqh2_a23.rom", 0x1000000, 0x1000000, CRC(7a779112) SHA1(0a7d36b3715063d8eac629b95a9bb3ecd8e54fca) )
	ROM_LOAD( "lsqh2_a45.rom", 0x2000000, 0x1000000, CRC(5d7de052) SHA1(7663b6cf09f65c4644661005a38f9aba84a32913) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "lsqh2_b01.rom", 0x0000000, 0x1000000, CRC(df7ca696) SHA1(7af3d27957a39de7e4873867c9972c05af7e7964) )

	ROM_REGION( 0xc00000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "lsqh2_m01.rom",0x400000, 0x400000, CRC(01af1b65) SHA1(6cf523fa8f1e03f974771611bb9a4e08a4d4443f) )
	ROM_IGNORE( 0x400000 )  // 400000-7fffff empty
	ROM_CONTINUE( 0x800000, 0x400000 )
	ROM_IGNORE( 0x400000 )  // c00000-ffffff empty
ROM_END


ROM_START( killbldp )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v300xx_u6.u6",     0x100000, 0x080000, CRC(b7fb8ec9) SHA1(e71b2d74269a82c7155b9818821156e128b68b28) ) // V300 05-09-16 11:58:20

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	/* the first 0x268 bytes of this are EXECUTE ONLY in the original chip, attempting to read them even via the original CPU just returns what is on the bus */
//  ROM_LOAD( "killbldp_igs027a.bin", 0x000000, 0x04000, CRC(c7868d90) SHA1(335c99933a38b77fcfc3f8004063f35124364f3e) ) // this is the original rom with the first 0x268 bytes from the bootleg - but it doesn't work?
	/* there are some differences around 0x2e80, investigate - maybe above is badly dumped?, padding at 0x3ac0 is also different */
	ROM_LOAD( "killbldp_igs027a_alt.bin", 0x000000, 0x04000, CRC(98316b06) SHA1(09be9fad24d68980a0a5beae60ced48012286216) ) // from a bootleg

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v300xx_u26.u26", 0x000000, 0x200000,  CRC(144388c8) SHA1(d7469df077c1a674129f18210584ba4d05a61888) ) // 05-09-16 23:52:32 V300

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "igs_t05701w032.u33", 0x180000, 0x400000, CRC(567c714f) SHA1(b25b20e1ec9f077d6f7b9d41723a68d0d461bef2) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "igs_a05701w064.u3",  0x0000000, 0x0800000, CRC(8c0c992c) SHA1(28391e50ca4400060676f1524bd49ede373292da) )
	ROM_LOAD( "igs_a05702w064.u4",  0x0800000, 0x0800000, CRC(7e5b0f27) SHA1(9e8d69f34c30216925fcb7af87f8b37f703317e7) )
	ROM_LOAD( "igs_a05703w064.u6",  0x1000000, 0x0800000, CRC(accbdb44) SHA1(d59b2452c7a5b4e666473dc973b73a0f2b4edc13) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "igs_b05701w064.u9",  0x0000000, 0x0800000, CRC(a20cdcef) SHA1(029a49971adf1e72ab556a207172bdfbd0b86b03) )
	ROM_LOAD( "igs_b05702w016.u11", 0x0800000, 0x0200000, CRC(fe7457df) SHA1(d66b1b31102b0210f9faf40e1473cd1511ccaf1f) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "igs_w05701b032.u5",  0x400000, 0x400000, CRC(2d3ae593) SHA1(b9c1d2994be95ba974bc134a3bf115bc9c9c9c16) )
ROM_END

ROM_START( svg )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "svg_v200_u30.u30",      0x100000, 0x080000, CRC(34c18f3f) SHA1(42d1edd0dcfaa5e44861c6a1d4cb24f51ba23de8) ) // V200 09/12/05 16:42:51

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "svg_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "svg_igs027a_v200_china.bin", 0x0188, 0x3e78, CRC(72b73169) SHA1(ffc0caea855ab4b01beb3aebd0bf17187c66c22c) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	/* eproms with no labels stickers */
	ROM_LOAD( "epr.u26", 0x000000, 0x400000, CRC(46826ec8) SHA1(ad1daf6f615fb8d748ce7f98f19dd3bf22f79fba) ) // 10/11/05 10:07:20 V201
	ROM_LOAD( "epr.u36", 0x400000, 0x400000, CRC(fa5f3901) SHA1(8ab7c6763df4f752b50ed2197063f58046b32ddb) )

	ROM_REGION( 0x500000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "igs_t05601w016.u29", 0x180000, 0x200000, CRC(03e110dc) SHA1(41c8f286e9303b24ba6235b341371c298226fb6a) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "igs_a05601w064.u3",  0x0000000, 0x0800000, CRC(ea6453e4) SHA1(b5c82edafa8008ad59b5f2219511947d078d446e) )
	ROM_LOAD( "igs_a05602w064.u4",  0x0800000, 0x0800000, CRC(6d00621b) SHA1(55a4bc357e14b975b0234a9cd49e2224f509dad9) )
	ROM_LOAD( "igs_a05603w064.u6",  0x1000000, 0x0800000, CRC(7b71c64f) SHA1(bec7c7edf0634cf8351a54abb867c56af08ad2c3) )
	ROM_LOAD( "igs_a05604w032.u8",  0x1800000, 0x0400000, CRC(9452a567) SHA1(01fdb8e1f131603843ef4c49ab76d7a56b2d6414) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "igs_b05601w064.u9",  0x0000000, 0x0800000, CRC(35c0a489) SHA1(a7d5527da01f8eaa7499fb6785b57094521bce97) )
	ROM_LOAD( "igs_b05602w064.u11", 0x0800000, 0x0800000, CRC(8aad3f85) SHA1(da6996d901d42b3a2ba7019ad014bb938a5e328b) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "igs_w05601b064.u5",  0x400000, 0x800000, CRC(bfe61a71) SHA1(e682ca8d57ca51c4d72f64fc091161f3dbdce871) )
	ROM_LOAD( "igs_w05602b032.u7",  0xc00000, 0x400000, CRC(0685166d) SHA1(64dac49abd2a46d5fb58c678027aa5e23d672dc4) )
ROM_END

ROM_START( svgtw )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v101tw_u30.u30",      0x100000, 0x080000, CRC(8d0405e4) SHA1(b6175c9ffeaac531d28e7845cb34c673476e286a) ) // V100 03/14/05 20:04:08

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */ // marked H1
	ROM_LOAD( "svgpcb_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "svgcpb_igs027a_v100_japan.bin", 0x0188, 0x3e78, CRC(7a59da5d) SHA1(d67ba465db40ca716b4b901b1c8e762716fb954e) ) // this is from the Japan set, the cart this came from was Taiwan

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v101tw_u26.u26", 0x000000, 0x400000, CRC(cc24f542) SHA1(623ed398d2eeea229833d92eb4fb6492133202b3) ) // 06/20/05 11:36:15 V100
	ROM_LOAD( "v101tw_u36.u36", 0x400000, 0x400000, CRC(f18283e2) SHA1(15323c5f816a0bf6f510311eb49d485ccf713cf7) )

	ROM_REGION( 0x500000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "igs_t05601w016.u29", 0x180000, 0x200000, CRC(03e110dc) SHA1(41c8f286e9303b24ba6235b341371c298226fb6a) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "igs_a05601w064.u3",  0x0000000, 0x0800000, CRC(ea6453e4) SHA1(b5c82edafa8008ad59b5f2219511947d078d446e) )
	ROM_LOAD( "igs_a05602w064.u4",  0x0800000, 0x0800000, CRC(6d00621b) SHA1(55a4bc357e14b975b0234a9cd49e2224f509dad9) )
	ROM_LOAD( "igs_a05603w064.u6",  0x1000000, 0x0800000, CRC(7b71c64f) SHA1(bec7c7edf0634cf8351a54abb867c56af08ad2c3) )
	ROM_LOAD( "igs_a05604w032.u8",  0x1800000, 0x0400000, CRC(9452a567) SHA1(01fdb8e1f131603843ef4c49ab76d7a56b2d6414) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "igs_b05601w064.u9",  0x0000000, 0x0800000, CRC(35c0a489) SHA1(a7d5527da01f8eaa7499fb6785b57094521bce97) )
	ROM_LOAD( "igs_b05602w064.u11", 0x0800000, 0x0800000, CRC(8aad3f85) SHA1(da6996d901d42b3a2ba7019ad014bb938a5e328b) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "igs_w05601b064.u5",  0x400000, 0x800000, CRC(bfe61a71) SHA1(e682ca8d57ca51c4d72f64fc091161f3dbdce871) )
	ROM_LOAD( "igs_w05602b032.u7",  0xc00000, 0x400000, CRC(0685166d) SHA1(64dac49abd2a46d5fb58c678027aa5e23d672dc4) )
ROM_END

ROM_START( svghk )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v101hk_u30.u30",      0x100000, 0x080000, CRC(8d0405e4) SHA1(b6175c9ffeaac531d28e7845cb34c673476e286a) ) // V100 03/14/05 20:04:08

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */ // marked H1
	ROM_LOAD( "svgpcb_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "svgcpb_igs027a_v100_japan.bin", 0x0188, 0x3e78, CRC(7a59da5d) SHA1(d67ba465db40ca716b4b901b1c8e762716fb954e) ) // this is from the Japan set, the cart this came from was Hong Kong

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v101hk_u26.u26", 0x000000, 0x400000, CRC(42b0d5a9) SHA1(1dd2b6530f982dbb720e4c017fca7f90cf441f57) ) // 06/20/05 11:36:15 V100
	ROM_LOAD( "v101hk_u36.u36", 0x400000, 0x400000, CRC(bf15a47a) SHA1(822aadb222d4f0278b2e2375cb59d8b35667f5ef) )

	ROM_REGION( 0x500000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "igs_t05601w016.u29", 0x180000, 0x200000, CRC(03e110dc) SHA1(41c8f286e9303b24ba6235b341371c298226fb6a) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "igs_a05601w064.u3",  0x0000000, 0x0800000, CRC(ea6453e4) SHA1(b5c82edafa8008ad59b5f2219511947d078d446e) )
	ROM_LOAD( "igs_a05602w064.u4",  0x0800000, 0x0800000, CRC(6d00621b) SHA1(55a4bc357e14b975b0234a9cd49e2224f509dad9) )
	ROM_LOAD( "igs_a05603w064.u6",  0x1000000, 0x0800000, CRC(7b71c64f) SHA1(bec7c7edf0634cf8351a54abb867c56af08ad2c3) )
	ROM_LOAD( "igs_a05604w032.u8",  0x1800000, 0x0400000, CRC(9452a567) SHA1(01fdb8e1f131603843ef4c49ab76d7a56b2d6414) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "igs_b05601w064.u9",  0x0000000, 0x0800000, CRC(35c0a489) SHA1(a7d5527da01f8eaa7499fb6785b57094521bce97) )
	ROM_LOAD( "igs_b05602w064.u11", 0x0800000, 0x0800000, CRC(8aad3f85) SHA1(da6996d901d42b3a2ba7019ad014bb938a5e328b) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "igs_w05601b064.u5",  0x400000, 0x800000, CRC(bfe61a71) SHA1(e682ca8d57ca51c4d72f64fc091161f3dbdce871) )
	ROM_LOAD( "igs_w05602b032.u7",  0xc00000, 0x400000, CRC(0685166d) SHA1(64dac49abd2a46d5fb58c678027aa5e23d672dc4) )
ROM_END

ROM_START( svgpcb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "svg_bios.u49",        0x000000, 0x020000, CRC(3346401f) SHA1(28bd730b6026c1e521c95072d33c7bdcd19c1460) )
	ROM_LOAD16_WORD_SWAP( "svg_v100jp_u50.u50",  0x100000, 0x080000, CRC(8d0405e4) SHA1(b6175c9ffeaac531d28e7845cb34c673476e286a) ) // V100 03/14/05 20:04:08

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "svgpcb_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "svgcpb_igs027a_v100_japan.bin", 0x0188, 0x3e78, CRC(7a59da5d) SHA1(d67ba465db40ca716b4b901b1c8e762716fb954e) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "svg_v100jp_u64.u64", 0x000000, 0x400000, CRC(399d4a8b) SHA1(b120e8386a259e6fd7941acf3c33cf288eda616c) ) // 05/12/05 15:31:35 V100
	ROM_LOAD( "svg_v100jp_u65.u65", 0x400000, 0x400000, CRC(6e1c33b1) SHA1(66f26b2f4c0b3dcf6d1bb1df48e2ddbcc9d9432d) )

	ROM_REGION( 0x500000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS // IGS PGM T01S 1B8558A1 M002146
	ROM_LOAD( "igs_t05601w016.u70", 0x180000, 0x200000, CRC(03e110dc) SHA1(41c8f286e9303b24ba6235b341371c298226fb6a) ) // IGS T05601W016 2C35 B270 2L464103 B050924

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "igs_a05601w064.u44", 0x0000000, 0x0800000, CRC(ea6453e4) SHA1(b5c82edafa8008ad59b5f2219511947d078d446e) ) // IGS A05601W064 44BF 2ECD 2E153602A1 S050914
	ROM_LOAD( "igs_a05602w064.u45", 0x0800000, 0x0800000, CRC(6d00621b) SHA1(55a4bc357e14b975b0234a9cd49e2224f509dad9) ) // IGS A05602W064 85C1 8A1F 2E153602A2 S050914
	ROM_LOAD( "igs_a05603w064.u46", 0x1000000, 0x0800000, CRC(7b71c64f) SHA1(bec7c7edf0634cf8351a54abb867c56af08ad2c3) ) // IGS A05603W064 8EC7 329A 2E153602A3 S050914
	ROM_LOAD( "igs_a05604w032.u47", 0x1800000, 0x0400000, CRC(9452a567) SHA1(01fdb8e1f131603843ef4c49ab76d7a56b2d6414) ) // IGS A05604W032 55F5 4CDB SL529808 S050912

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "igs_b05601w064.u61", 0x0000000, 0x0800000, CRC(35c0a489) SHA1(a7d5527da01f8eaa7499fb6785b57094521bce97) ) // IGS B05601W064 EED5 E656 2E350003A1 S050914
	ROM_LOAD( "igs_b05602w064.u62", 0x0800000, 0x0800000, CRC(8aad3f85) SHA1(da6996d901d42b3a2ba7019ad014bb938a5e328b) ) // IGS B05602W064 DD65 3D89 2E350004A2 S050914

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS // IGS PGM M01S 1BB278A1 E002146
	ROM_LOAD( "igs_w05601b064.u30", 0x400000, 0x800000, CRC(bfe61a71) SHA1(e682ca8d57ca51c4d72f64fc091161f3dbdce871) ) // IGS W05601B064 2530 FBF6 2E350004A3 S050914
	ROM_LOAD( "igs_w05602b032.u31", 0xc00000, 0x400000, CRC(0685166d) SHA1(64dac49abd2a46d5fb58c678027aa5e23d672dc4) ) // IGS w05602B032 1BC2 90D3 2K453504 S050912
ROM_END


ROM_START( happy6 )
	/* All ROMs labels are on stickers */
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "s101xx_u5.u5",      0x100000, 0x080000, CRC(aa4646e3) SHA1(e6772cc480ddd3e1d199364b1f2ff93b973e6842) ) // V101 03/17/04 11:26:48

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	// data before 0x188 is read-protected and cannot be read even with a trojan (as with most 2001/2+ IGS titles)
//  ROM_LOAD( "happy6_igs027a.bin", 0x000000, 0x04000, NO_DUMP )
	ROM_LOAD( "happy6_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "happy6_igs027a_v100_china.bin", 0x0188, 0x3e78, CRC(ed530445) SHA1(05c92d649701be2541557b1334dd6c820ca1009e) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v-102cn_u26.u26", 0x000000, 0x400000, CRC(310510fb) SHA1(e0e80a04e9f7bf27e6581a8935c960bad33bb6de) ) // 03/16/04 14:29:17 V102

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t01w64m_u29.u29", 0x180000, 0x800000, CRC(2d3feb8b) SHA1(9832b1c46b1ee73febf5c5c8913859f4e0581665) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a01w64m_u5.u5",   0x0000000, 0x0800000, CRC(bbaa3df3) SHA1(a72268d3989e96d571242279922291d0dc72db28) )
	ROM_LOAD( "a02w64m_u6.u6",   0x0800000, 0x0800000, CRC(f8c9cd36) SHA1(d9613a83bcc2364492fa922fde1c4f0d07b3009c) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b01w64m_u19.u19", 0x0000000, 0x0800000, CRC(73f5f225) SHA1(507126fa96dcec967bdbc0978d79fbce9d25db37) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w01w64m_u17.u17", 0x400000, 0x800000, CRC(7e23e2be) SHA1(5e920b8e480f00b6666292d4a56039b40af18141) )
ROM_END

ROM_START( happy6101 )
	/* All ROMs labels are on stickers */
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v100cn_u5.u5",      0x100000, 0x080000, CRC(a25418e8) SHA1(acd7e7b69956cb4ce8e26c6420cb97bb4bf404e7) ) // V100 12/22/03 15:35:33

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	// data before 0x188 is read-protected and cannot be read even with a trojan (as with most 2001/2+ IGS titles)
//  ROM_LOAD( "happy6_igs027a.bin", 0x000000, 0x04000, NO_DUMP )
	ROM_LOAD( "happy6_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "happy6_igs027a_v100_china.bin", 0x0188, 0x3e78, CRC(ed530445) SHA1(05c92d649701be2541557b1334dd6c820ca1009e) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v101cn_u26.u26", 0x000000, 0x400000, CRC(4a48ca1c) SHA1(3bebc091787903d45cb84c7302046602a903f59c) ) // 01/09/04 19:51:11 V101

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t01w64m_u29.u29", 0x180000, 0x800000, CRC(2d3feb8b) SHA1(9832b1c46b1ee73febf5c5c8913859f4e0581665) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a01w64m_u5.u5",   0x0000000, 0x0800000, CRC(bbaa3df3) SHA1(a72268d3989e96d571242279922291d0dc72db28) )
	ROM_LOAD( "a02w64m_u6.u6",   0x0800000, 0x0800000, CRC(f8c9cd36) SHA1(d9613a83bcc2364492fa922fde1c4f0d07b3009c) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b01w64m_u19.u19", 0x0000000, 0x0800000, CRC(73f5f225) SHA1(507126fa96dcec967bdbc0978d79fbce9d25db37) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w01w64m_u17.u17", 0x400000, 0x800000, CRC(7e23e2be) SHA1(5e920b8e480f00b6666292d4a56039b40af18141) )
ROM_END

ROM_START( happy6100hk )
	/* All ROMs labels are on stickers */
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v100hk_u5.u5",      0x100000, 0x080000, CRC(a25418e8) SHA1(acd7e7b69956cb4ce8e26c6420cb97bb4bf404e7) ) // V100 12/22/03 15:35:33

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	// data before 0x188 is read-protected and cannot be read even with a trojan (as with most 2001/2+ IGS titles)
//  ROM_LOAD( "happy6_igs027a.bin", 0x000000, 0x04000, NO_DUMP )
	ROM_LOAD( "happy6_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "happy6_igs027a_v100_china.bin", 0x0188, 0x3e78, BAD_DUMP CRC(ed530445) SHA1(05c92d649701be2541557b1334dd6c820ca1009e) ) // needs the HK version

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v100hk_u26.u26", 0x000000, 0x400000, CRC(8f2feb1f) SHA1(acbc6620a296e8a6819bf088886bcbfc329f286d) ) // 12/22/03 11:28:36 V100

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t01w64m_u29.u29", 0x180000, 0x800000, CRC(2d3feb8b) SHA1(9832b1c46b1ee73febf5c5c8913859f4e0581665) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a01w64m_u5.u5",   0x0000000, 0x0800000, CRC(bbaa3df3) SHA1(a72268d3989e96d571242279922291d0dc72db28) )
	ROM_LOAD( "a02w64m_u6.u6",   0x0800000, 0x0800000, CRC(f8c9cd36) SHA1(d9613a83bcc2364492fa922fde1c4f0d07b3009c) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b01w64m_u19.u19", 0x0000000, 0x0800000, CRC(73f5f225) SHA1(507126fa96dcec967bdbc0978d79fbce9d25db37) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w01w64m_u17.u17", 0x400000, 0x800000, CRC(7e23e2be) SHA1(5e920b8e480f00b6666292d4a56039b40af18141) )
ROM_END

ROM_START( happy6100cn )
	/* All ROMs labels are on stickers */
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v100cn_u5.u5",      0x100000, 0x080000, CRC(a25418e8) SHA1(acd7e7b69956cb4ce8e26c6420cb97bb4bf404e7) ) // V100 12/22/03 15:35:33

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	// data before 0x188 is read-protected and cannot be read even with a trojan (as with most 2001/2+ IGS titles)
//  ROM_LOAD( "happy6_igs027a.bin", 0x000000, 0x04000, NO_DUMP )
	ROM_LOAD( "happy6_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "happy6_igs027a_v100_china.bin", 0x0188, 0x3e78, CRC(ed530445) SHA1(05c92d649701be2541557b1334dd6c820ca1009e) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v100cn_u26.u26", 0x000000, 0x400000, CRC(9c29e482) SHA1(17a054cb4ab0663e3eba9661c3a9d4dfb7dad010) ) // 12/22/03 11:28:36 V100

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t01w64m_u29.u29", 0x180000, 0x800000, CRC(2d3feb8b) SHA1(9832b1c46b1ee73febf5c5c8913859f4e0581665) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a01w64m_u5.u5",   0x0000000, 0x0800000, CRC(bbaa3df3) SHA1(a72268d3989e96d571242279922291d0dc72db28) )
	ROM_LOAD( "a02w64m_u6.u6",   0x0800000, 0x0800000, CRC(f8c9cd36) SHA1(d9613a83bcc2364492fa922fde1c4f0d07b3009c) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b01w64m_u19.u19", 0x0000000, 0x0800000, CRC(73f5f225) SHA1(507126fa96dcec967bdbc0978d79fbce9d25db37) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w01w64m_u17.u17", 0x400000, 0x800000, CRC(7e23e2be) SHA1(5e920b8e480f00b6666292d4a56039b40af18141) )
ROM_END

/* all known revisions of ketsui have roms marked v100, even when the actual game revision is upgraded */

ROM_START( ket )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketsui_v100.u38", 0x000000, 0x200000, CRC(dfe62f3b) SHA1(baa58d1ce47a707f84f65779ac0689894793e9d9) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "cave_t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION16_LE( 0x1000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "cave_a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION16_LE( 0x0800000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "cave_m04701b032.u17",   0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv",  0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( ket1 ) // only difference between this and ket1 is the rom fill on the unused area
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketsui_v100_alt_fill.u38", 0x000000, 0x200000, CRC(e140f8a4) SHA1(34fd25f8896935503d7537e89a4cd174e8995070) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "cave_t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION16_LE( 0x1000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "cave_a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION16_LE( 0x0800000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "cave_m04701b032.u17",   0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv",  0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( keta )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketsui_prg_revised.bin", 0x000000, 0x200000, CRC(69fcf5eb) SHA1(f726e251b4daa2f8d717e32000d4d7abc71c710d) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "cave_t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION16_LE( 0x1000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "cave_a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION16_LE( 0x0800000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "cave_m04701b032.u17",   0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv",  0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( ketb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketsui_prg_original.bin", 0x000000, 0x200000, CRC(cca5e153) SHA1(b653feaa2004c379312def6b1613c3497f654ddf) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "cave_t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION16_LE( 0x1000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "cave_a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION16_LE( 0x0800000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "cave_m04701b032.u17",   0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv",  0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END


ROM_START( ketarr10 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketarr_v100.u38", 0x000000, 0x200000, CRC(d4c7a8ab) SHA1(65d104d17bd4fd03a2b44297a003ba03d746c7ee) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "cave_t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION16_LE( 0x1000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "cave_a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION16_LE( 0x0800000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "cave_m04701b032.u17",   0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv", 0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END


ROM_START( ketarrf )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketarrf_v100.u38", 0x000000, 0x200000, CRC(6ad17aa4) SHA1(791bd1a107433a3811c8a79ea26a73e66ddd296f) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "cave_t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION16_LE( 0x1000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "cave_a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION16_LE( 0x0800000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "cave_m04701b032.u17",   0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv", 0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( ketarr15 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketarr15_v100.u38", 0x000000, 0x200000, CRC(552a7d95) SHA1(4f3fb13f34d58a7482e1d26623d38aa0b54ca8dd) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "cave_t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION16_LE( 0x1000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "cave_a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION16_LE( 0x0800000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "cave_m04701b032.u17",   0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv", 0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( ketarrs15 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketarrs15_v100.u38", 0x000000, 0x200000, CRC(a95e71e0) SHA1(182c12e3581ebb20176d8abca41ee62aadcd63e0) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "cave_t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION16_LE( 0x1000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "cave_a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION16_LE( 0x0800000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "cave_m04701b032.u17",   0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv", 0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( ketarr151 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketarr151_v100.u38", 0x000000, 0x200000, CRC(2b7c030d) SHA1(9aaba1242d7ce29915a31d40341da82985927f9d) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "cave_t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION16_LE( 0x1000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "cave_a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION16_LE( 0x0800000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "cave_m04701b032.u17",   0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv", 0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( ketarrs151 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketarrs151_v100.u38", 0x000000, 0x200000, CRC(35c984e4) SHA1(d4517f318de0c40a3b30e41374f33bb355581434) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "cave_t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION16_LE( 0x1000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "cave_a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION16_LE( 0x0800000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "cave_m04701b032.u17",   0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv", 0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( ketarr )
	ROM_REGION( 0x600000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "ketarr17_v100.u38", 0x000000, 0x200000, CRC(2cb80b89) SHA1(e1aa072b8344890486e11795e02703aa2d234bb1) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "cave_t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION16_LE( 0x1000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "cave_a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION16_LE( 0x0800000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "cave_m04701b032.u17",   0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv", 0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

/*

Usually a Demon Front cart is used as donor for the Ketsui conversion.Anyway these can be the donor carts:

Knight of Valour 2
Demon Front
Dodonpachi 2 Bee Storm
Martial Master
Gladiator
Killing Blade

*/

ROM_START( ketbl ) // this assumes a Dodonpachi 2 Bee Storm cart was used
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	// the program rom in the cartridge actually ends up mapping OVER the motherboard bios rom, you don't get the PGM splash etc.
	ROM_LOAD16_WORD_SWAP( "ketsui_u1.bin", 0x000000, 0x200000, CRC(391767b4) SHA1(722e364d3a8982a40df8f898e995212a1a6fee35) )
	ROM_CONTINUE(0x000000,0x200000) // first half is empty, use 2nd half

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */ // leftover from original game
	ROM_LOAD( "ddp2_igs027a_japan.bin", 0x000000, 0x04000, CRC(742d34d2) SHA1(4491c08f3cefef2933ad5a741f4bb05cc2f3e1a0) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */ // leftover from original game
	ROM_LOAD( "v100.u23", 0x000000, 0x20000, CRC(06c3dd29) SHA1(20c9479f158467fc2037dcf162b6c6be18c91d46) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION16_LE( 0x1000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION16_LE( 0x0800000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m04701b032.u17",    0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1
ROM_END

ROM_START( espgal )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "espgaluda_v100.u38", 0x000000, 0x200000, CRC(08ecec34) SHA1(bce2e7fb9105ed51603d09cbd3a9eeb5b8f47ee2) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "espgal_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "t01s.u18", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "cave_t04801w064.u19",   0x180000, 0x800000, CRC(6021c79e) SHA1(fbc340dafb18aa3094de29b881318a5a9794e4bc) ) //text-1

	ROM_REGION16_LE( 0x1000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04801w064.u7",    0x0000000, 0x0800000, CRC(26dd4932) SHA1(9bbabb5a53cb5ba88397cc2c258980f3b70314ce) ) //image-1
	ROM_LOAD( "cave_a04802w064.u8",    0x0800000, 0x0800000, CRC(0e6bf7a9) SHA1(a7541e2b5a0df2bc62a5b347e54dbc2ed1922db2) ) //image-2

	ROM_REGION16_LE( 0x0800000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04801w064.u1",    0x0000000, 0x0800000, CRC(98dce13a) SHA1(61d48b7117459f7babc022b68231f6928177a71d) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, same as ketsui but not "PGM" string on PCB */
	ROM_LOAD( "cave_w04801b032.u17",   0x400000, 0x400000, CRC(60298536) SHA1(6b7333f16cce778c5725dbdf75a5446f0906397a) ) //music-1
ROM_END

ROM_START( espgalbl ) // this assumes a Dodonpachi 2 Bee Storm cart was used
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	// the program rom in the cartridge actually ends up mapping OVER the motherboard bios rom, you don't get the PGM splash etc.
	ROM_LOAD16_WORD_SWAP( "espgaluda_u8.bin", 0x000000, 0x400000, CRC(6a92dd52) SHA1(d4b694c88deaeebb7b1c0ddbf29a06380b06426f) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */ // leftover from original game
	ROM_LOAD( "ddp2_igs027a_japan.bin", 0x000000, 0x04000, CRC(742d34d2) SHA1(4491c08f3cefef2933ad5a741f4bb05cc2f3e1a0) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */ // leftover from original game
	ROM_LOAD( "v100.u23", 0x000000, 0x20000, CRC(06c3dd29) SHA1(20c9479f158467fc2037dcf162b6c6be18c91d46) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t04801w064.u19",   0x180000, 0x800000, CRC(6021c79e) SHA1(fbc340dafb18aa3094de29b881318a5a9794e4bc) ) //text-1

	ROM_REGION16_LE( 0x1000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04801w064.u7",    0x0000000, 0x0800000, CRC(26dd4932) SHA1(9bbabb5a53cb5ba88397cc2c258980f3b70314ce) ) //image-1
	ROM_LOAD( "a04802w064.u8",    0x0800000, 0x0800000, CRC(0e6bf7a9) SHA1(a7541e2b5a0df2bc62a5b347e54dbc2ed1922db2) ) //image-2

	ROM_REGION16_LE( 0x0800000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04801w064.u1",    0x0000000, 0x0800000, CRC(98dce13a) SHA1(61d48b7117459f7babc022b68231f6928177a71d) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w04801b032.u17",    0x400000, 0x400000, CRC(60298536) SHA1(6b7333f16cce778c5725dbdf75a5446f0906397a) ) //music-1
ROM_END


ROM_START( ddp3 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "ddp3_v101_16m.u36",  0x100000, 0x200000,CRC(fba2180e) SHA1(59c1a76243e587c07215c8a76649401ef0bff7c7) ) // marked 怒首領蜂(dodonpachi) 3 V101 16M

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp3_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "cave_t04401w064.u19", 0x180000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) ) //image-1
	ROM_LOAD( "cave_a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) ) //image-2

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04401w064.u1",  0x0000000, 0x0800000, CRC(17731c9d) SHA1(0e0aa0ec01035323985ac8e08228a0fd6edf6689) ) //bitmap-1

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "cave_m04401b032.u17", 0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ddp3_defaults.nv",  0x0000000, 0x020000, CRC(571e96c0) SHA1(348940c77ca348213331b85b9b1d3aabb96a528a) )
ROM_END

ROM_START( ddpdoj )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",   0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "ddp3_v101.u36",  0x100000, 0x200000, CRC(195b5c1e) SHA1(f18d791c034b0a3d85888a92fb5d326ee3deb04f) ) // yes this one was actually marked v101 which goes against the standard Cave marking system

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp3_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "cave_t04401w064.u19", 0x180000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) ) //image-1
	ROM_LOAD( "cave_a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) ) //image-2

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04401w064.u1",  0x0000000, 0x0800000, CRC(17731c9d) SHA1(0e0aa0ec01035323985ac8e08228a0fd6edf6689) ) //bitmap-1

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "cave_m04401b032.u17", 0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ddp3_defaults.nv",  0x0000000, 0x020000, CRC(571e96c0) SHA1(348940c77ca348213331b85b9b1d3aabb96a528a) )
ROM_END


ROM_START( ddpdoja )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "ddp3_d_d_1_0.u36",  0x100000, 0x200000, CRC(5d3f85ba) SHA1(4c24ea206140863d456179750366921442e1d2b8) ) // marked v100

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp3_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "cave_t04401w064.u19", 0x180000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) ) //image-1
	ROM_LOAD( "cave_a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) ) //image-2

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04401w064.u1",  0x0000000, 0x0800000, CRC(17731c9d) SHA1(0e0aa0ec01035323985ac8e08228a0fd6edf6689) ) //bitmap-1

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "cave_m04401b032.u17", 0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ddp3_defaults.nv",  0x0000000, 0x020000, CRC(571e96c0) SHA1(348940c77ca348213331b85b9b1d3aabb96a528a) )
ROM_END

ROM_START( ddpdojb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "dd_v100.u36",  0x100000, 0x200000, CRC(7da0c1e4) SHA1(aca2fe35ba0ab3628900fa2aba2d22fc4fd7046d) ) // marked v100

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp3_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "cave_t04401w064.u19", 0x180000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) ) //image-1
	ROM_LOAD( "cave_a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) ) //image-2

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04401w064.u1",  0x0000000, 0x0800000, CRC(17731c9d) SHA1(0e0aa0ec01035323985ac8e08228a0fd6edf6689) ) //bitmap-1

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "cave_m04401b032.u17", 0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ddp3_defaults.nv",  0x0000000, 0x020000, CRC(571e96c0) SHA1(348940c77ca348213331b85b9b1d3aabb96a528a) )
ROM_END

ROM_START( ddpdojp )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgmbios.u20.27c210",    0x00000, 0x020000, CRC(1d2a7c15) SHA1(025a9f2bb64887699bf7ccab0f2ccfc55c3ad75c) )
	ROM_LOAD16_WORD_SWAP( "ca008.cod_prom.u13.27c322",  0x100000, 0x400000, CRC(2ba7fa3b) SHA1(c4c5425a2455cb95555d94bbf8afc83cf0b140e8) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "fnt_cg.0_cgrom0.u8.27c322",   0x180000, 0x0400000, CRC(1805e198) SHA1(23db96eddfb47dfef9dcfab52ae2582ad3c0fc90) )
	ROM_LOAD( "fnt_cg.1_cgrom1.u10.27c322",  0x580000, 0x0400000, CRC(f06ce99c) SHA1(24482e2bb2485855e300b6b3e07962d4f0a6ab83) )

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "sp_cg.0_imcs0.u11.27c322",  0x0000000, 0x0400000, CRC(c66bdd8e) SHA1(dde26821a3ef8ad5c3c29e11c55f8f6953c085dd) )
	ROM_LOAD( "sp_cg.1_imcs1.u13.27c322",  0x0400000, 0x0400000, CRC(d30eac89) SHA1(78b18ff71136df0ffadb87fc53b60943dd62bcdd) )
	ROM_LOAD( "sp_cg.2_imcs2.u15.27c322",  0x0800000, 0x0400000, CRC(f31b010e) SHA1(1987e9cd2abcf57be990853fbd305474e58da5b8) )
	ROM_LOAD( "sp_cg.3_imcs3.u17.27c322",  0x0c00000, 0x0400000, CRC(01ec23f5) SHA1(ff22642f914b0f8911a221df1e9073bdc9434660) )

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "sp_mp.0_bitcs0.u5.27c322",  0x0000000, 0x0400000, CRC(e30494a4) SHA1(12d216252a916be4fd8d77b89497cee9b04861e5) )
	ROM_LOAD( "sp_mp.1_bitcs1.u6.27c322",  0x0400000, 0x0400000, CRC(0239daec) SHA1(506c4d8a4bfa988d81565c47f742f1b06175915e) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "sndmask.rom_mcs1.u3.27c322",  0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) )

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ddp3_defaults.nv",  0x0000000, 0x020000, CRC(571e96c0) SHA1(348940c77ca348213331b85b9b1d3aabb96a528a) )
ROM_END



/* this expects Magic values in NVRAM to boot */
// latest revision, BL version at 2nd ROM half was updated, require different NVRAM protection values, still display same 3-dot version text as older set
ROM_START( ddpdojblk )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "ddb10_10_8_434f.u45",  0x100000, 0x200000, CRC(d21561db) SHA1(66a0103bc5f17b28736b562e32807271a5afa261) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp3_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "cave_t04401w064.u19", 0x180000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) ) //image-1
	ROM_LOAD( "cave_a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) ) //image-2

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04401w064.u1",  0x0000000, 0x0800000, CRC(17731c9d) SHA1(0e0aa0ec01035323985ac8e08228a0fd6edf6689) ) //bitmap-1

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "cave_m04401b032.u17", 0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* NVRAM with factory programmed values - needed to boot */
	ROM_LOAD( "ddp3blk_defaults.nv",  0x0000000, 0x020000, CRC(c2282720) SHA1(80b7662a7577883dabd043b6500ae244379047c2) )
ROM_END

ROM_START( ddpdojblka )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "ddb_1dot.u45",  0x100000, 0x200000, CRC(265f26cd) SHA1(91abc7fc4722f3d01d76a4c1ae14c4132e4e576c) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp3_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "cave_t04401w064.u19", 0x180000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) ) //image-1
	ROM_LOAD( "cave_a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) ) //image-2

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04401w064.u1",  0x0000000, 0x0800000, CRC(17731c9d) SHA1(0e0aa0ec01035323985ac8e08228a0fd6edf6689) ) //bitmap-1

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "cave_m04401b032.u17", 0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* NVRAM with factory programmed values - needed to boot */
	ROM_LOAD( "ddp3blk_defaults.nv",  0x0000000, 0x020000, CRC(a1651904) SHA1(5b80d3c4c764895c40953a66161d4dd84f742604) )
ROM_END

ROM_START( ddpdojblkb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "ddb10.u45",  0x100000, 0x200000, CRC(72b35510) SHA1(9a432e5e1ebe61aafd737b6acc905653e5af0d38) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp3_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "cave_t04401w064.u19", 0x180000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "cave_a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) ) //image-1
	ROM_LOAD( "cave_a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) ) //image-2

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "cave_b04401w064.u1",  0x0000000, 0x0800000, CRC(17731c9d) SHA1(0e0aa0ec01035323985ac8e08228a0fd6edf6689) ) //bitmap-1

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "cave_m04401b032.u17", 0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* NVRAM with factory programmed values - needed to boot */
	ROM_LOAD( "ddp3blk_defaults.nv",  0x0000000, 0x020000, CRC(a1651904) SHA1(5b80d3c4c764895c40953a66161d4dd84f742604) )
ROM_END

// bootleg on a converted KOVSH cart
ROM_START( ddpdojblkbl )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "ddp_doj_u1.bin",  0x100000, 0x400000, CRC(eb4ab06a) SHA1(187c37e5319395e36a1cf3626b53e08df615cc0c) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000,  CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xa00000, "igs023", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t04401w064.u19",0x180000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1

	ROM_REGION16_LE( 0x2000000, "igs023:sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) ) //image-1
	ROM_LOAD( "a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) ) //image-2

	ROM_REGION16_LE( 0x1000000, "igs023:sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04401w064_corrupt.u1",  0x0000000, 0x0800000, CRC(8cbff066) SHA1(eef1cd566bc70ebf45f047e56026803d5c1dac43) ) //bitmap-1 - bootlegs were based off a corrupt dump of the ROM

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m04401b032.u17",  0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) ) //music-1
ROM_END


/*** Init Stuff **************************************************************/

void pgm_state::pgm_basic_init(bool set_bank)
{
	u8 *ROM = memregion("maincpu")->base();
	if (set_bank) membank("bank1")->set_base(&ROM[0x100000]);
}

void pgm_state::init_pgm()
{
	pgm_basic_init();
}


/*** GAME ********************************************************************/

GAME( 1997, pgm,          0,         pgm,                   pgm,       pgm_state,       init_pgm,         ROT0,   "IGS", "PGM (Polygame Master) System BIOS", MACHINE_IS_BIOS_ROOT )

/* -----------------------------------------------------------------------------------------------------------------------
   Working (at least one set of the game is fully working)
   -----------------------------------------------------------------------------------------------------------------------*/

//西游释厄传/Xīyóu shì è zhuàn (China; Simplified Chinese)
//西遊釋厄傳/Xīyóu shì è zhuàn (Taiwan; Traditional Chinese)
// the version numbering on these is a mess... date strings from ROM (and in some cases even those are missing..)
GAME( 1997, orlegend,     pgm,       pgm_asic3,              orlegend,  pgm_asic3_state,   init_orlegend, ROT0,   "IGS", "Oriental Legend / Xiyou Shi E Zhuan (ver. 126)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )                 // V0001 01/14/98 18:16:38 - runs as World
GAME( 1997, orlegende,    orlegend,  pgm_asic3,              orlegend,  pgm_asic3_state,   init_orlegend, ROT0,   "IGS", "Oriental Legend / Xiyou Shi E Zhuan (ver. 112)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )                 // V0001 07/14/97 11:19:45 - runs as World
GAME( 1997, orlegendc,    orlegend,  pgm_asic3,              orlegend,  pgm_asic3_state,   init_orlegend, ROT0,   "IGS", "Oriental Legend / Xiyou Shi E Zhuan (ver. 112, Chinese Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )  // V0001 05/05/97 10:08:21 - runs as World, Korea, China
GAME( 1997, orlegendca,   orlegend,  pgm_asic3,              orlegend,  pgm_asic3_state,   init_orlegend, ROT0,   "IGS", "Oriental Legend / Xiyou Shi E Zhuan (ver. ???, Chinese Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )  // V0001 04/02/97 13:35:43 - runs as HongKong, China, China
GAME( 1997, orlegend111c, orlegend,  pgm_asic3,              orlegend,  pgm_asic3_state,   init_orlegend, ROT0,   "IGS", "Oriental Legend / Xiyou Shi E Zhuan (ver. 111, Chinese Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )  // V0001 no date!          - runs as HongKong, China, China
GAME( 1997, orlegend111t, orlegend,  pgm_asic3,              orlegendt, pgm_asic3_state,   init_orlegend, ROT0,   "IGS", "Oriental Legend / Xiyou Shi E Zhuan (ver. 111, Taiwanese Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )// V0001 no date! - needs a different protection sequence
GAME( 1997, orlegend111k, orlegend,  pgm_asic3,              orlegendk, pgm_asic3_state,   init_orlegend, ROT0,   "IGS", "Oriental Legend / Xiyou Shi E Zhuan (ver. 111, Korean Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )   // not checked
GAME( 1997, orlegend105t, orlegend,  pgm_asic3,              orlegendt, pgm_asic3_state,   init_orlegend, ROT0,   "IGS", "Oriental Legend / Xiyou Shi E Zhuan (ver. 105, Taiwanese Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )// V0000 no date! - needs a different protection sequence
GAME( 1997, orlegend105k, orlegend,  pgm_asic3,              orlegendk, pgm_asic3_state,   init_orlegend, ROT0,   "IGS", "Oriental Legend / Xiyou Shi E Zhuan (ver. 105, Korean Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )  //  V0000 no date!          - runs as Korea

//Dragon World II
//中國龍II/Zhōngguó lóng II (China, Taiwan, Japan; Traditional Chinese only in title screen)
//東方之珠II/Dung1Fong1 Zi1 Zyu1 II (Hong Kong)/dongbang jiju II (Korea(undumped))
GAME( 1997, drgw2,        pgm,       pgm_012_025_drgw2,      pgm,       pgm_012_025_state, init_drgw2,    ROT0,   "IGS", "Dragon World II (ver. 110X, Export)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1997, drgw2100x,    drgw2,     pgm_012_025_drgw2,      pgm,       pgm_012_025_state, init_dw2v100x, ROT0,   "IGS", "Dragon World II (ver. 100X, Export)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1997, drgw2100j,    drgw2,     pgm_012_025_drgw2,      pgm,       pgm_012_025_state, init_drgw2j,   ROT0,   "IGS (Alta license)", "Chuugokuryuu II (ver. 100J, Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1997, drgw2101c,    drgw2,     pgm_012_025_drgw2,      pgm,       pgm_012_025_state, init_drgw2c101,ROT0,   "IGS", "Zhongguo Long II (ver. 101C, China)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1997, drgw2100c,    drgw2,     pgm_012_025_drgw2,      pgm,       pgm_012_025_state, init_drgw2c,   ROT0,   "IGS", "Zhongguo Long II (ver. 100C, China)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1997, drgw2100hk,   drgw2,     pgm_012_025_drgw2,      pgm,       pgm_012_025_state, init_drgw2hk,  ROT0,   "IGS", "Dungfong Zi Zyu II (ver. 100H, Hong Kong)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // (region is shown as Hokg Kong, Dragon World 3 is the same)

//The Killing Blade
//傲剑狂刀/Ào jiàn kuáng dāo (China, Hong Kong; Simplified Chinese)
//傲劍狂刀/Ào jiàn kuáng dāo (Taiwan; Traditional Chinese)
GAME( 1998, killbld,      pgm,       pgm_022_025_killbld,    killbld,   pgm_022_025_state, init_killbld,  ROT0,   "IGS", "The Killing Blade / Ao Jian Kuang Dao (ver. 109, Chinese Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1998, killbld104,   killbld,   pgm_022_025_killbld,    killbld,   pgm_022_025_state, init_killbld,  ROT0,   "IGS", "The Killing Blade / Ao Jian Kuang Dao (ver. 104)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1998, killbld106,   killbld,   pgm_022_025_killbld,    killbld,   pgm_022_025_state, init_killbld,  ROT0,   "IGS", "The Killing Blade / Ao Jian Kuang Dao (ver. 106)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1998, killbld100,   killbld,   pgm_022_025_killbld,    killbld,   pgm_022_025_state, init_killbld,  ROT0,   "IGS", "The Killing Blade / Ao Jian Kuang Dao (ver. 100)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */

//中國龍3/Zhōngguó lóng 3 (China, Taiwan, Japan; Traditional chinese only in title screen)
//東方之珠3/Dung1Fong1 Zi1 Zyu1 3 (Hong Kong)/dongbang jiju 3 (Korea)
// these seem playable but the DMA mode transfering 68k code to RAM is not emulated so there could still be problems
// when set to Japan it has the extra subtitle and so gets referred to as Dragon World 3 Special / Chuugokuryuu 3 Special.  The earliest versions seem to only contain the code for the Japanese region, presumably the support for other regions was added later.
GAME( 1998, drgw3,        pgm,       pgm_022_025_dw3,        dw3,       pgm_022_025_state, init_drgw3,    ROT0,   "IGS", "Dragon World 3 / Zhongguo Long 3 / Dungfong Zi Zyu 3 / Chuugokuryuu 3 Special (ver. 106)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1998, drgw3105,     drgw3,     pgm_022_025_dw3,        dw3,       pgm_022_025_state, init_drgw3,    ROT0,   "IGS", "Dragon World 3 / Zhongguo Long 3 / Dungfong Zi Zyu 3 / Chuugokuryuu 3 Special (ver. 105)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1998, drgw3103,     drgw3,     pgm_022_025_dw3,        dw3,       pgm_022_025_state, init_drgw3,    ROT0,   "IGS", "Chuugokuryuu 3 Special (Japan, ver. 103)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // Japan only, has an extra game mode option!
GAME( 1998, drgw3100,     drgw3,     pgm_022_025_dw3,        dw3j,      pgm_022_025_state, init_drgw3,    ROT0,   "IGS", "Chuugokuryuu 3 Special (Japan, ver. 100)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // ^

// Share title screen graphics and region as drgw3
GAME( 2000, dwex,         pgm,       pgm_022_025_dw3,        dw3,       pgm_022_025_state, init_drgw3,    ROT0,   "IGS", "Dragon World 3 EX / Zhongguo Long 3 EX / Dungfong Zi Zyu 3 EX / Chuugokuryuu 3 EX (ver. 100)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 2001, dwex101cn,    dwex,      pgm_022_025_dw3,        dw3,       pgm_022_025_state, init_drgw3,    ROT0,   "IGS", "Dragon World 3 EX / Zhongguo Long 3 EX / Dungfong Zi Zyu 3 EX / Chuugokuryuu 3 EX (ver. 101CN)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */

//超級比一比/Chāojí bǐ yī bǐ (Taiwan)
//大家来找碴/Dàjiā lái zhǎo chá/Daai6gaa1 Loi4 Zaau2 Caa4 (China, Hong Kong)
//リアルアンドフェイク/Riaruandofeiku(Real and Fake) (Japan)
// region provided by internal ARM rom
GAME( 1999, photoy2k,     pgm,       pgm_arm_type1,          photoy2k,  pgm_arm_type1_state, init_photoy2k, ROT0,   "IGS", "Photo Y2K / Chaoji Bi Yi Bi / Dajia Lai Zhao Cha / Real and Fake (ver. 105)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1999, photoy2k104,  photoy2k,  pgm_arm_type1,          photoy2k,  pgm_arm_type1_state, init_photoy2k, ROT0,   "IGS", "Photo Y2K / Chaoji Bi Yi Bi / Dajia Lai Zhao Cha / Real and Fake (ver. 104)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1999, photoy2k103j, photoy2k,  pgm_arm_type1,          photoy2kj, pgm_arm_type1_state, init_photoy2k, ROT0,   "IGS", "Photo Y2K / Chaoji Bi Yi Bi / Dajia Lai Zhao Cha / Real and Fake (ver. 103, Japanese Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1999, photoy2k102j, photoy2k,  pgm_arm_type1,          photoy2k,  pgm_arm_type1_state, init_photoy2k, ROT0,   "IGS", "Photo Y2K / Chaoji Bi Yi Bi / Dajia Lai Zhao Cha / Real and Fake (ver. 102, Japanese Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */

//三国战纪风云再起/Sānguó zhàn jì Fēngyún zàiqǐ (China, Hong Kong; Simplified Chinese)
//三國戰紀風雲再起/Sānguó zhàn jì Fēngyún zàiqǐ (Taiwan; Traditional Chinese)
//三國戰紀 Superheroes/Sangoku-Senki Superheroes (Japan; Traditional Chinese)
// region provided by internal ARM rom
GAME( 1999, kovsh,        pgm,       pgm_arm_type1,          kovsh,     pgm_arm_type1_state, init_kovsh,      ROT0,   "IGS", "Knights of Valour Super Heroes / Sanguo Zhan Ji Fengyun Zaiqi / Sangoku Senki Super Heroes (ver. 104, CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k: SANGO EX V104 03/24/00 11:15:25, ARM: China internal ROM
GAME( 1999, kovsh103,     kovsh,     pgm_arm_type1,          kovsh,     pgm_arm_type1_state, init_kovsh,      ROT0,   "IGS", "Knights of Valour Super Heroes / Sanguo Zhan Ji Fengyun Zaiqi / Sangoku Senki Super Heroes (ver. 103, CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k: SANGO EX V103 01/18/00 17:37:12, ARM: China internal ROM
GAME( 1999, kovsh102,     kovsh,     pgm_arm_type1,          kovsh,     pgm_arm_type1_state, init_kovsh,      ROT0,   "IGS", "Knights of Valour Super Heroes / Sanguo Zhan Ji Fengyun Zaiqi / Sangoku Senki Super Heroes (ver. 102, CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k: SANGO EX V102 12/23/99 15:17:57, ARM: China internal ROM
GAME( 1999, kovsh101,     kovsh,     pgm_arm_type1,          kovsh,     pgm_arm_type1_state, init_kovsh,      ROT0,   "IGS", "Knights of Valour Super Heroes / Sanguo Zhan Ji Fengyun Zaiqi / Sangoku Senki Super Heroes (ver. 101, CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k: SANGO EX V101 12/20/99 10:59:05, ARM: China internal ROM
GAME( 1999, kovsh100,     kovsh,     pgm_arm_type1,          kovsh,     pgm_arm_type1_state, init_kovsh,      ROT0,   "IGS", "Knights of Valour Super Heroes / Sanguo Zhan Ji Fengyun Zaiqi / Sangoku Senki Super Heroes (ver. 100, CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k: SANGO EX V100 12/06/99 13:36:04, ARM: China internal ROM
// 拳皇三国特別版/Quánhuáng sānguó Tèbié bǎn
// nasty modern asian bootleg of Knights of Valour Super Heroes with characters ripped from SNK's The King of Fighters series!
GAME( 2008, kovqhsgs,     kovsh,     pgm_arm_type1,          kovsh,     pgm_arm_type1_state, init_kovqhsgs,   ROT0,   "bootleg", "Quanhuang Sanguo Tebie Ban (bootleg of Knights of Valour Super Heroes, V303CN, Nov 21 2008 19:03:49)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2010, kovqhsgs302,  kovsh,     pgm_arm_type1,          kovsh,     pgm_arm_type1_state, init_kovqhsgs,   ROT0,   "bootleg", "Quanhuang Sanguo Tebie Ban (bootleg of Knights of Valour Super Heroes, V302CN, Dec 29 2010 16:31:32)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // needs decryption, actual title is probably different

//三國戰紀2/Sānguó zhàn jì 2 (Traditional Chinese only in title screen)
// region provided by internal ARM rom
GAME( 2000, kov2,         pgm,       pgm_arm_type2,          kov2,      pgm_arm_type2_state, init_kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sanguo Zhan Ji 2 / Sangoku Senki 2 (ver. 107, 102, 100HK)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 05/10/01 14:24:08 V107 (Ext. Arm V102, Int. Arm V100HK)
GAME( 2000, kov2106,      kov2,      pgm_arm_type2,          kov2,      pgm_arm_type2_state, init_kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sanguo Zhan Ji 2 / Sangoku Senki 2 (ver. 106, 102, 100HK)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 02/27/01 13:26:46 V106 (Ext. Arm V102, Int. Arm V100HK)
GAME( 2000, kov2104,      kov2,      pgm_arm_type2,          kov2,      pgm_arm_type2_state, init_kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sanguo Zhan Ji 2 / Sangoku Senki 2 (ver. 104, 102, 100HK)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 01/19/01 10:19:38 V104 (Ext. Arm V102, Int. Arm V100HK)
GAME( 2000, kov2103,      kov2,      pgm_arm_type2,          kov2,      pgm_arm_type2_state, init_kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sanguo Zhan Ji 2 / Sangoku Senki 2 (ver. 103, 101, 100HK)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 12/28/00 15:09:31 V103 (Ext. Arm V101, Int. Arm V100HK)
GAME( 2000, kov21022,     kov2,      pgm_arm_type2,          kov2,      pgm_arm_type2_state, init_kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sanguo Zhan Ji 2 / Sangoku Senki 2 (ver. 102, 102, 100HK)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 12/14/00 10:33:36 V102 (Ext. Arm V102, Int. Arm V100HK)
GAME( 2000, kov2102,      kov2,      pgm_arm_type2,          kov2,      pgm_arm_type2_state, init_kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sanguo Zhan Ji 2 / Sangoku Senki 2 (ver. 102, 101, 100HK)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 12/14/00 10:33:36 V102 (Ext. Arm V101, Int. Arm V100HK)
GAME( 2000, kov2101,      kov2,      pgm_arm_type2,          kov2,      pgm_arm_type2_state, init_kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sanguo Zhan Ji 2 / Sangoku Senki 2 (ver. 101, 101, 100HK)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 11/29/00 11:03:08 V100 (Ext. Arm V100, Int. Arm V100HK)
GAME( 2000, kov2100,      kov2,      pgm_arm_type2,          kov2,      pgm_arm_type2_state, init_kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sanguo Zhan Ji 2 / Sangoku Senki 2 (ver. 100, 100, 100HK)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 11/29/00 11:03:08 V100 (Ext. Arm V100, Int. Arm V100HK)

//三國戰紀2 Nine Dragons/Sānguó zhàn jì 2 Nine Dragons (Overseas)
//三國戰紀2群雄争霸/Sānguó zhàn jì 2 Qúnxióng zhēngbà/Saam1Qwok3 Zin3 Gei2 2 Kwan4Hung4 Zang1Baa3 (China/Hong Kong; Mixed Traditional and Simplified Chinese)
//三國戰紀2飛龍在天/Sānguó zhàn jì 2 Fēilóng zài tiān (Taiwan; Traditional Chinese)
//三國戰紀武将争霸/Sangoku-Senki Bushō Souha (Japan; Mixed Traditional and Simplified Chinese, Busyou Souha in title screen)
// region provided by internal ARM rom (we only have a China internal ROM)
GAME( 2001, kov2p,        pgm,       pgm_arm_type2,          kov2,      pgm_arm_type2_state, init_kov2p,      ROT0,   "IGS", "Knights of Valour 2 Plus - Nine Dragons / Sanguo Zhan Ji 2 Qunxiong Zhengba / Sanguo Zhan Ji 2 Feilong Zai Tian / Sangoku Senki Busyou Souha (ver. M205XX, 200, 100CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 04/25/02  17:48:27 M205XX
GAME( 2001, kov2p204,     kov2p,     pgm_arm_type2,          kov2,      pgm_arm_type2_state, init_kov2p,      ROT0,   "IGS", "Knights of Valour 2 Plus - Nine Dragons / Sanguo Zhan Ji 2 Qunxiong Zhengba / Sanguo Zhan Ji 2 Feilong Zai Tian / Sangoku Senki Busyou Souha (ver. M204XX, 200, 100CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 08/28/01  09:11:49 M204XX
GAME( 2001, kov2p203,     kov2p,     pgm_arm_type2,          kov2,      pgm_arm_type2_state, init_kov2p,      ROT0,   "IGS", "Knights of Valour 2 Plus - Nine Dragons / Sanguo Zhan Ji 2 Qunxiong Zhengba / Sanguo Zhan Ji 2 Feilong Zai Tian / Sangoku Senki Busyou Souha (ver. M203XX, 200, 100CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 08/13/01  16:57:32 M203XX
GAME( 2001, kov2p202,     kov2p,     pgm_arm_type2,          kov2,      pgm_arm_type2_state, init_kov2p,      ROT0,   "IGS", "Knights of Valour 2 Plus - Nine Dragons / Sanguo Zhan Ji 2 Qunxiong Zhengba / Sanguo Zhan Ji 2 Feilong Zai Tian / Sangoku Senki Busyou Souha (ver. M202XX, 200, 100CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 07/09/01  11:03:50 M202XX
GAME( 2001, kov2p200,     kov2p,     pgm_arm_type2,          kov2,      pgm_arm_type2_state, init_kov2p,      ROT0,   "IGS", "Knights of Valour 2 Plus - Nine Dragons / Sanguo Zhan Ji 2 Qunxiong Zhengba / Sanguo Zhan Ji 2 Feilong Zai Tian / Sangoku Senki Busyou Souha (ver. M200XX, 200, 100CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 06/18/01  22:59:12 M200XX

//形意拳/Xíng yì quán/Sin ī ken(Japanese label)
// region provided by internal ARM rom
GAME( 2001, martmast,     pgm,       pgm_arm_type2_22m,      martmast,  pgm_arm_type2_state, init_martmast,   ROT0,   "IGS", "Martial Masters / Xing Yi Quan (ver. 104, 102, 102US)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k V104, Ext Arm 102, Int Arm 102US
GAME( 2001, martmast104c, martmast,  pgm_arm_type2_22m,      martmast,  pgm_arm_type2_state, init_martmast,   ROT0,   "IGS", "Martial Masters / Xing Yi Quan (ver. 104, 102, 101CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k V104, Ext Arm 102, Int Arm 101CN
GAME( 2001, martmast103c, martmast,  pgm_arm_type2_22m,      martmast,  pgm_arm_type2_state, init_martmast,   ROT0,   "IGS", "Martial Masters / Xing Yi Quan (ver. 103, 102, 101CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k V103, Ext Arm 102, Int Arm 101CN (actually 102 CN on the PCB, needs to be dumped)
GAME( 2001, martmast102c, martmast,  pgm_arm_type2_22m,      martmast,  pgm_arm_type2_state, init_martmast,   ROT0,   "IGS", "Martial Masters / Xing Yi Quan (ver. 102, 101, 101CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k V102, Ext Arm 101, Int Arm 101CN

//蜂暴/Fēng bào (Chinese, Title call excepting Japan)/Fung1 Bou6 (Hong Kong, Jyutping)
// region provided by internal ARM rom
GAME( 2001, ddp2,         pgm,       pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (World, ver. 102)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2101,      ddp2,      pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (World, ver. 101)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2100,      ddp2,      pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (World, ver. 100)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2hk,       ddp2,      pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS", "DoDonPachi II - Fung Bou (Hong Kong, ver. 102)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2101hk,    ddp2,      pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS", "DoDonPachi II - Fung Bou (Hong Kong, ver. 101)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2100hk,    ddp2,      pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS", "DoDonPachi II - Fung Bou (Hong Kong, ver. 100)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2k,        ddp2,      pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (Korea, ver. 102)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2101k,     ddp2,      pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (Korea, ver. 101)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2100k,     ddp2,      pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (Korea, ver. 100)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2j,        ddp2,      pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS (Cave license)", "DoDonPachi II (Japan, ver. 102)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2101j,     ddp2,      pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS (Cave license)", "DoDonPachi II (Japan, ver. 101)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2100j,     ddp2,      pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS (Cave license)", "DoDonPachi II (Japan, ver. 100)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2t,        ddp2,      pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS", "DoDonPachi - Feng Bao (Taiwan, ver. 102)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2101t,     ddp2,      pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS", "DoDonPachi - Feng Bao (Taiwan, ver. 101)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2100t,     ddp2,      pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS", "DoDonPachi - Feng Bao (Taiwan, ver. 100)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2c,        ddp2,      pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS", "DoDonPachi - Feng Bao (China, ver. 102)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2101c,     ddp2,      pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS", "DoDonPachi - Feng Bao (China, ver. 101)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2100c,     ddp2,      pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,       ROT270, "IGS", "DoDonPachi - Feng Bao (China, ver. 100)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )


// japan region only? service mode calls it Dragon World 2001 so I'm leaving that title in the description
GAME( 2001, dw2001,       pgm,       pgm_arm_type2_22m,      dw2001,    pgm_arm_type2_state, init_dw2001,     ROT0,   "IGS", "Chuugokuryuu 2001 [Dragon World 2001] (V100 02/21/01 16:05:16, Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 02/21/01 16:05:16

// japan region only? service mode calls it Dragon World Pretty Chance so I'm leaving that title in the description
// english / chinese version also exists
GAME( 2001, dwpc,         pgm,       pgm_arm_type2_22m,      dw2001,    pgm_arm_type2_state, init_dwpc,       ROT0,   "IGS", "Zhongguo Long Pretty Chance [Dragon World Pretty Chance] (V110 03/19/02 11:13:16, China)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 03/19/02 11:13:16, needs proper ARM internal ROM dump, currently hacked, needs reset before working
GAME( 2001, dwpc101j,     dwpc,      pgm_arm_type2_22m,      dw2001,    pgm_arm_type2_state, init_dwpc101j,   ROT0,   "IGS", "Chuugokuryuu Pretty Chance [Dragon World Pretty Chance] (V101 09/26/01 10:23:26, Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 09/26/01 10:23:26

//魔域戰線/Móyù zhànxiàn (Traditional Chinese)
//魔域战线/Móyù zhànxiàn (Simplified Chinese)
// we bypass the internal ARM rom on these, ideally it should still be dumped tho! the region screens show a blank string where the internal ROM revision would otherwise be displayed
// ARM version strings don't match 100% with labels... for 68k ROMs I'm using the build time / date stamp from near the start of the rom, there are some slightly different time stamps later
GAME( 2002, dmnfrnt,      pgm,       pgm_arm_type3_22m,      pgm,       pgm_arm_type3_state, init_dmnfrnt,    ROT0,   "IGS", "Demon Front / Moyu Zhanxian (68k label V105, ROM M105XX 08/05/02) (ARM label V105, ROM 08/05/02 S105XX)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k time: 10:24:11 ARM time: 10:33:23
GAME( 2002, dmnfrnt103,   dmnfrnt,   pgm_arm_type3_22m,      pgm,       pgm_arm_type3_state, init_dmnfrnt,    ROT0,   "IGS", "Demon Front / Moyu Zhanxian (68k label V103, ROM M103XX 07/05/02) (ARM label V103, ROM 07/05/02 S103XX)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k time: 14:43:13 ARM time: 11:04:24
GAME( 2002, dmnfrnt102,   dmnfrnt,   pgm_arm_type3_22m,      pgm,       pgm_arm_type3_state, init_dmnfrnt,    ROT0,   "IGS", "Demon Front / Moyu Zhanxian (68k label V102, ROM M102XX 06/19/02) (ARM label V102, ROM 05/24/02 S101XX)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k time: 13:44:08 ARM time: 13:04:31  (from the strings it looks like V102 only upgraded the 68k ROM)
GAME( 2002, dmnfrntpcb,   dmnfrnt,   pgm_arm_type3_24m,      pgm,       pgm_arm_type3_state, init_dmnfrnt,    ROT0,   "IGS", "Demon Front V1.1 / Moyu Zhanxian V1.1 (68k label V107KR, ROM M107KR 11/03/03) (ARM label V106KR, ROM 10/16/03 S106KR) (JAMMA PCB)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // works but reports version mismatch (wants internal rom version and region to match external?)


/* these don't use an External ARM rom, and don't have any weak internal functions which would allow the internal ROM to be read out */
GAME( 2002, ddp3,         0,         pgm_arm_type1_cave,     ddp3,      pgm_arm_type1_state, init_ddp3,     ROT270, "Cave (AMI license)", "DoDonPachi III (World, 2002.05.15 Master Ver)",                MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, ddpdoj,       ddp3,      pgm_arm_type1_cave,     ddp3,      pgm_arm_type1_state, init_ddp3,     ROT270, "Cave (AMI license)", "DoDonPachi Dai-Ou-Jou (Japan, 2002.04.05.Master Ver, 68k Label V101)",  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // is there a v101 without the . after 05?
GAME( 2002, ddpdoja,      ddp3,      pgm_arm_type1_cave,     ddp3,      pgm_arm_type1_state, init_ddp3,     ROT270, "Cave (AMI license)", "DoDonPachi Dai-Ou-Jou (Japan, 2002.04.05.Master Ver, 68k Label V100)",  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, ddpdojb,      ddp3,      pgm_arm_type1_cave,     ddp3,      pgm_arm_type1_state, init_ddp3,     ROT270, "Cave (AMI license)", "DoDonPachi Dai-Ou-Jou (Japan, 2002.04.05 Master Ver)",       MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, ddpdojp,      ddp3,      pgm,                    ddp3,      pgm_state,           init_pgm,      ROT270, "Cave (AMI license)", "DoDonPachi Dai-Ou-Jou (Japan, 2002.04.05 Master Ver, location test)",       MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // unprotected, but still has strings related to the protection ASIC
GAME( 2002, ddpdojblk,    ddp3,      pgm_arm_type1_cave,     ddp3,      pgm_arm_type1_state, init_ddp3,     ROT270, "Cave (AMI license)", "DoDonPachi Dai-Ou-Jou Black Label (Japan, 2002.10.07.Black Ver, newer)",        MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // Displays "2002.04.05.Master Ver" (old) or "2002.10.07.Black Ver" (new)
GAME( 2002, ddpdojblka,   ddp3,      pgm_arm_type1_cave,     ddp3,      pgm_arm_type1_state, init_ddp3,     ROT270, "Cave (AMI license)", "DoDonPachi Dai-Ou-Jou Black Label (Japan, 2002.10.07.Black Ver, older)",        MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // Displays "2002.04.05.Master Ver" (old) or "2002.10.07.Black Ver" (new)
GAME( 2002, ddpdojblkb,   ddp3,      pgm_arm_type1_cave,     ddp3,      pgm_arm_type1_state, init_ddp3,     ROT270, "Cave (AMI license)", "DoDonPachi Dai-Ou-Jou Black Label (Japan, 2002.10.07 Black Ver)",        MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // Displays "2002.04.05.Master Ver" (old) or "2002.10.07 Black Ver" (new)
GAME( 2012, ddpdojblkbl,  ddp3,      pgm_arm_type1,          pgm,       pgm_arm_type1_state, init_kovsh,    ROT270, "bootleg",            "DoDonPachi Dai-Ou-Jou Black Label (Japan, 2002.10.07 Black Ver., bootleg Knights of Valour Super Heroes conversion)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // the extra . in the revision has been added by bootlegger

// the exact text of the 'version' shows which revision of the game it is; the newest has 2 '.' symbols in the string, the oldest, none.
// the only difference between 'ket' and 'ket1' is the ROM fill at 0x1443bc-0x1c88cd, on ket1 it seems to be randomized / garbage data, on ket it's all 0xff, both have been seen on more than one PCB.
GAME( 2002, ket,          0,         pgm_arm_type1_cave,     ddp3,      pgm_arm_type1_state, init_ket,      ROT270, "Cave (AMI license)", "Ketsui: Kizuna Jigoku Tachi (2003/01/01. Master Ver.)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, ket1,         ket,       pgm_arm_type1_cave,     ddp3,      pgm_arm_type1_state, init_ket,      ROT270, "Cave (AMI license)", "Ketsui: Kizuna Jigoku Tachi (2003/01/01. Master Ver.) (alt rom fill)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, keta,         ket,       pgm_arm_type1_cave,     ddp3,      pgm_arm_type1_state, init_ket,      ROT270, "Cave (AMI license)", "Ketsui: Kizuna Jigoku Tachi (2003/01/01 Master Ver.)",  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, ketb,         ket,       pgm_arm_type1_cave,     ddp3,      pgm_arm_type1_state, init_ket,      ROT270, "Cave (AMI license)", "Ketsui: Kizuna Jigoku Tachi (2003/01/01 Master Ver)",   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, ketbl,        ket,       pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,     ROT270, "bootleg",            "Ketsui: Kizuna Jigoku Tachi (2003/01/01. Master Ver., bootleg cartridge conversion)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )


// these are modern hacks, some of them have been seen on original PCBs, also reportedly on a bootleg PCB with mostly original components but the ARM replaced with a custom chip.
// this is a significantly reworked version of the game
GAME( 2014, ketarr,       ket,       pgm_arm_type1_cave,     espgal,    pgm_arm_type1_state, init_ket,      ROT270, "hack (trap15)", "Ketsui: Kizuna Jigoku Tachi (2014/07/16 ARRANGE 1.7 VER) (hack)", MACHINE_SUPPORTS_SAVE )
GAME( 2012, ketarr151,    ket,       pgm_arm_type1_cave,     espgal,    pgm_arm_type1_state, init_ket,      ROT270, "hack (trap15)", "Ketsui: Kizuna Jigoku Tachi (2012/06/26 ARRANGE 1.51 VER) (hack)", MACHINE_SUPPORTS_SAVE ) // this apparently crashes on an original PGM PCB when displaying the text after starting a game, find out why and reproduce the issue in MAME.
GAME( 2012, ketarr15,     ket,       pgm_arm_type1_cave,     espgal,    pgm_arm_type1_state, init_ket,      ROT270, "hack (trap15)", "Ketsui: Kizuna Jigoku Tachi (2012/06/26 ARRANGE 1.5 VER) (hack)", MACHINE_SUPPORTS_SAVE )
GAME( 2012, ketarr10,     ket,       pgm_arm_type1_cave,     espgal,    pgm_arm_type1_state, init_ket,      ROT270, "hack (trap15)", "Ketsui: Kizuna Jigoku Tachi (2012/04/17 ARRANGE VER) (hack)", MACHINE_SUPPORTS_SAVE )

// these simplify the scoring system
GAME( 2012, ketarrs151,   ket,       pgm_arm_type1_cave,     ddp3,      pgm_arm_type1_state, init_ket,      ROT270, "hack (trap15)", "Ketsui: Kizuna Jigoku Tachi (2012/06/27 MR.STOIC 1.51 VER) (hack)", MACHINE_SUPPORTS_SAVE )
GAME( 2012, ketarrs15,    ket,       pgm_arm_type1_cave,     ddp3,      pgm_arm_type1_state, init_ket,      ROT270, "hack (trap15)", "Ketsui: Kizuna Jigoku Tachi (2012/06/27 MR.STOIC 1.5 VER) (hack)", MACHINE_SUPPORTS_SAVE )

// this has the 'programmed slowdown' removed.
GAME( 2012, ketarrf,      ket,       pgm_arm_type1_cave,     ddp3,      pgm_arm_type1_state, init_ket,      ROT270, "hack (trap15)", "Ketsui: Kizuna Jigoku Tachi (2012/04/17 FAST. VER) (hack)", MACHINE_SUPPORTS_SAVE )

// this version is stupid, it just simulates what happens if the protection chip isn't returning proper values
// ROM_LOAD16_WORD_SWAP( "ketarrb_v100.u38", 0x000000, 0x200000, CRC(ec7a4f92) SHA1(6351fb386586956fbdb5f0730c481fb539cc267a) )
// GAME( 2002, ketarrb,    ket,       pgm_arm_type1_cave,   ddp3,    pgm_arm_type1_state, init_ket,      ROT270, "trap15", "Ketsui: Kizuna Jigoku Tachi (2012/04/17 BACK. VER)", MACHINE_SUPPORTS_SAVE )


GAME( 2003, espgal,       0,         pgm_arm_type1_cave,     espgal,    pgm_arm_type1_state, init_espgal,   ROT270, "Cave (AMI license)", "Espgaluda (2003/10/15 Master Ver)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2003, espgalbl,     espgal,    pgm_arm_type2,          pgm,       pgm_arm_type2_state, init_ddp2,     ROT270, "bootleg",            "Espgaluda (2003/10/15 Master Ver, bootleg cartridge conversion)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

//泡泡鱼/Pào pào yú (China)
//(i can't found any similar Traditional Chinese characters on Taiwan region title screen)
// protection simulated, but should be correct
GAME( 1999, puzzli2,      pgm,       pgm_arm_type1_sim,      puzzli2,   pgm_arm_type1_state, init_puzzli2,  ROT0,   "IGS (Metro license)", "Puzzli 2 / Pao Pao Yu (ver. 100)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // ROM label is V100 ( V0001, 11/22/99 09:27:58 in program ROM )
GAME( 2001, puzzli2s,     puzzli2,   pgm_arm_type1_sim,      puzzli2,   pgm_arm_type1_state, init_puzzli2,  ROT0,   "IGS (Metro license)", "Puzzli 2 Super / Pao Pao Yu Super (ver. 200)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )  // ( V200, 12/28/01 12:53:34 in program ROM )

//傲劍狂刀加强版/Ào jiàn kuáng dāo Jiāqiáng bǎn
GAME( 2005, killbldp,     pgm,       pgm_arm_type3_33_8688m, pgm,       pgm_arm_type3_state, init_killbldp, ROT0,   "IGS", "The Killing Blade Plus / Ao Jian Kuang Dao Jiaqiang Ban (China, ver. 300)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* using internal rom from bootleg */

//神劍伏魔录/Shén jiàn fú mó lù/San4 Gim3 Fuk6 Mo1 Luk6 (China, Hong Kong; Mixed Traditional and Simplified Chinese)
//神劍風雲/Shén jiàn fēngyún (Taiwan and Title call; Traditional Chinese)
//신검의 풍운/singeom-ui pung-un (Korea)
//闘幻狂 Road of the Sword/Tōgenkyō Road of the Sword (Japan)
// we're using a partial dump of the internal rom (sans the execute only area) with handcrafted startup code..
// all 3 68k roms still have V100 strings, but are clearly different builds, there don't appear to be build string dates in them.  Two of the external ARM roms are marked V100 but are different builds, the single PCB v100 appears to be a later revision than the cart V100 as it shares the internal ROM with the V107 cart version, the v100 cart has a different internal ROM
GAME( 2003, theglad,      pgm,       pgm_arm_type3_22m,      theglad,   pgm_arm_type3_state, init_theglad,  ROT0,   "IGS", "The Gladiator / Shen Jian Fu Mo Lu / Shen Jian Fengyun (M68k label V101) (ARM label V107, ROM 06/06/03 SHEN JIAN V107)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // ARM time: 16:17:27
GAME( 2003, theglad104,   theglad,   pgm_arm_type3_22m,      theglad,   pgm_arm_type3_state, init_theglad,  ROT0,   "IGS", "The Gladiator / Shen Jian Fu Mo Lu / Shen Jian Fengyun (M68k label V100) (ARM label V104, ROM 04/02/03 SHEN JIAN V104)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // ARM time: 09:39:46
GAME( 2003, theglad101,   theglad,   pgm_arm_type3_22m,      theglad,   pgm_arm_type3_state, init_theglad,  ROT0,   "IGS", "The Gladiator / Shen Jian Fu Mo Lu / Shen Jian Fengyun (M68k label V100) (ARM label V101, ROM 03/13/03 SHEN JIAN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // ARM time: 14:06:44
// the v100 68k ROM on this is older than the v101 set, this set also uses a different internal ROM to everything else, must be a very early release, maybe pre v100 proto with v100 strings?
GAME( 2003, theglad100,   theglad,   pgm_arm_type3_22m,      theglad,   pgm_arm_type3_state, init_theglada, ROT0,   "IGS", "The Gladiator / Shen Jian Fu Mo Lu / Shen Jian Fengyun (M68k label V100) (ARM label V100, ROM 01/16/03 SHEN JIAN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* need correct internal rom of IGS027A - we currently patch the one we have */ // ARM time: 10:39:25
// newer than ARM V100 Cart, older than ARM V101 Cart, same 68k rom as V101 Cart.
GAME( 2003, thegladpcb,   theglad,   pgm_arm_type3_33m,      pgm,       pgm_arm_type3_state, init_theglad,  ROT0,   "IGS (Alta/AMI license)", "Tougenkyou - Road of the Sword (M68k label V100) (ARM label V101, ROM 03/13/03 SHEN JIAN) (Japan, JAMMA PCB)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )// ARM time: 16:17:27 // PCB version only released in Japan?
GAME( 2003, thegladpcba,  theglad,   pgm_arm_type3_33m,      pgm,       pgm_arm_type3_state, init_theglad,  ROT0,   "IGS (Alta/AMI license)", "Tougenkyou - Road of the Sword (M68k label V100) (ARM label V100, ROM 02/25/03 SHEN JIAN) (Japan, JAMMA PCB)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )// ARM time: 16:32:21 // PCB version only released in Japan?

//圣魔世纪/Shèng mó shìjì (China)
//聖魔世紀/Shèng mó shìjì (Taiwan, Hong Kong)
GAME( 2005, svg,          pgm,       pgm_arm_type3_33m,      svg,       pgm_arm_type3_state,  init_svg,     ROT0,   "IGS / Idea Factory", "S.V.G. - Spectral vs Generation / Sheng Mo Shiji (M68k label V200) (ARM label V200, ROM 10/11/05 S.V.G V201)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // ARM label was 200, but it's code rev 201? // ARM time: 10:07:20
GAME( 2005, svghk,        svg,       pgm_arm_type3_33m,      svghk,     pgm_arm_type3_state,  init_svgpcb,  ROT0,   "IGS / Idea Factory", "S.V.G. - Spectral vs Generation / Sheng Mo Shiji (M68k label V101HK) (ARM label V101HK, ROM 06/20/05 S.V.G V100)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k label was 101 but it's same as v100
GAME( 2005, svgtw,        svg,       pgm_arm_type3_33m,      svgtw,     pgm_arm_type3_state,  init_svgpcb,  ROT0,   "IGS / Idea Factory", "S.V.G. - Spectral vs Generation / Sheng Mo Shiji (M68k label V101TW) (ARM label V101TW, ROM 06/20/05 S.V.G V100)", MACHINE_NOT_WORKING ) // 68k label was 101 but it's same as v100
GAME( 2005, svgpcb,       svg,       pgm_arm_type3_33m,      svgpcb,    pgm_arm_type3_state,  init_svgpcb,  ROT0,   "IGS / Idea Factory (AMI license)", "S.V.G. - Spectral vs Generation / Sheng Mo Shiji (M68k label V100JP) (ARM label V100JP, ROM 05/12/05 S.V.G V100) (Japan, JAMMA PCB)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )// ARM time: 15:31:35 // PCB version only released in Japan?

//欢乐六合一/Huānlè liùhé yī (China, Singapore)
//歡樂六合一/Huānlè liùhé yī (Taiwan, Hong Kong, Oversea)
GAME( 2004, happy6,       pgm,       pgm_arm_type3_24m,      happy6,    pgm_arm_type3_state,  init_happy6,  ROT0,   "IGS", "Huanle Liuhe Yi (Happy 6-in-1) (M68K ver. V101, ARM ver. V102CN)",   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2004, happy6101,    happy6,    pgm_arm_type3_24m,      happy6,    pgm_arm_type3_state,  init_happy6,  ROT0,   "IGS", "Huanle Liuhe Yi (Happy 6-in-1) (M68K ver. V100, ARM ver. V101CN)",   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2004, happy6100hk,  happy6,    pgm_arm_type3_24m,      happy6hk,  pgm_arm_type3_state,  init_happy6,  ROT0,   "IGS", "Huanle Liuhe Yi (Happy 6-in-1) (M68K ver. V100HK, ARM ver. V100HK)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2004, happy6100cn,  happy6,    pgm_arm_type3_24m,      happy6,    pgm_arm_type3_state,  init_happy6,  ROT0,   "IGS", "Huanle Liuhe Yi (Happy 6-in-1) (M68K ver. V100, ARM ver. V100CN)",   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

//超級比一比2/Chāojí bǐ yī bǐ 2 (Taiwan)
//大家来找碴2/Dàjiā lái zhǎo chá 2/Daai6gaa1 Loi4 Zaau2 Caa4 2 (China, Hong Kong)
//リアルアンドフェイク 2 Photo Y2K/Riaruandofeiku(Real and Fake) 2 Photo Y2K (Japan)
GAME( 2001, py2k2,        pgm,       pgm_arm_type1_sim,      py2k2,     pgm_arm_type1_state,  init_py2k2,    ROT0,   "IGS", "Photo Y2K 2 / Chaoji Bi Yi Bi 2 / Dajia Lai Zhao Cha 2 / Real and Fake 2 Photo Y2K (M101XX 05/25/01 11:02:54)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )  /* need internal rom of IGS027A */
GAME( 2000, py2k2100,     py2k2,     pgm_arm_type1_sim,      py2k2,     pgm_arm_type1_state,  init_py2k2,    ROT0,   "IGS", "Photo Y2K 2 / Chaoji Bi Yi Bi 2 / Dajia Lai Zhao Cha 2 / Real and Fake 2 Photo Y2K (ver. 100, Japanese Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )  /* need internal rom of IGS027A */

/* -----------------------------------------------------------------------------------------------------------------------
   Partially Working, playable, but some imperfections
   -----------------------------------------------------------------------------------------------------------------------*/

//西游释厄传Super/Xīyóu shì è zhuàn Super (China; Simplified Chinese)
//西遊釋厄傳Super/Xīyóu shì è zhuàn Super (Taiwan; Traditional Chinese)
GAME( 1998, olds,         pgm,       pgm_028_025_ol,         olds,      pgm_028_025_state,    init_olds,    ROT0,   "IGS", "Oriental Legend Super / Xiyou Shi E Zhuan Super (ver. 101, Korean Board)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1998, olds100,      olds,      pgm_028_025_ol,         olds,      pgm_028_025_state,    init_olds,    ROT0,   "IGS", "Oriental Legend Special / Xiyou Shi E Zhuan Super (ver. 100, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1998, olds100a,     olds,      pgm_028_025_ol,         olds,      pgm_028_025_state,    init_olds,    ROT0,   "IGS", "Oriental Legend Special / Xiyou Shi E Zhuan Super (ver. 100, set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device; OPCODE 1111 error at startup */
// This version was specially made for a Chinese online gaming company. While it may not be entirely suitable for MAME, it can give some insight into how protection should work.
GAME( 1998, olds103t,     olds,      pgm,                    pgm,       pgm_state,            init_pgm,     ROT0,   "bootleg", "Xiyou Shi E Zhuan Super (ver. 103, China, Tencent) (unprotected)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // without overseas region

//三国战纪/Sānguó zhàn jì (China, Hong Kong; Simplified Chinese)
//三國戰紀/Sānguó zhàn jì (Taiwan, Japan; Traditional Chinese)
GAME( 1999, kov,          pgm,       pgm_arm_type1_sim,      sango,     pgm_arm_type1_state,  init_kov,     ROT0,   "IGS", "Knights of Valour / Sanguo Zhan Ji / Sangoku Senki (ver. 117, Hong Kong)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */                 // V0008 04/27/99 10:33:33
GAME( 1999, kov115,       kov,       pgm_arm_type1_sim,      sango,     pgm_arm_type1_state,  init_kov,     ROT0,   "IGS", "Knights of Valour / Sanguo Zhan Ji / Sangoku Senki (ver. 115)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */                 // V0006 02/22/99 11:53:18
GAME( 1999, kov114,       kov,       pgm_arm_type1_sim,      sango,     pgm_arm_type1_state,  init_kov,     ROT0,   "IGS", "Knights of Valour / Sanguo Zhan Ji / Sangoku Senki (ver. 114, Hong Kong)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 1999, kov111,       kov,       pgm_arm_type1_sim,      sango,     pgm_arm_type1_state,  init_kov,     ROT0,   "IGS", "Knights of Valour / Sanguo Zhan Ji / Sangoku Senki (ver. 111, Japanese Board)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */ // V0002 01/31/99 01:54:16

//三国战纪正宗Plus/Sānguó zhàn jì Zhèngzōng Plus (China/Hong Kong; Simplified Chinese)
//三國戰紀正宗Plus/Sānguó zhàn jì Zhèngzōng Plus (Taiwan, Japan; Traditional Chinese)
// no PLUS on screen when set to Korea
GAME( 1999, kovplus,      pgm,       pgm_arm_type1_sim,      sango,     pgm_arm_type1_state,  init_kov,     ROT0,   "IGS", "Knights of Valour Plus / Sanguo Zhan Ji Zhengzong Plus / Sangoku Senki Masamune Plus (ver. 119, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 1999, kovplusa,     kovplus,   pgm_arm_type1_sim,      sango,     pgm_arm_type1_state,  init_kov,     ROT0,   "IGS", "Knights of Valour Plus / Sanguo Zhan Ji Zhengzong Plus / Sangoku Senki Masamune Plus (ver. 119, set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

//三國群英传正宗Plus/Sānguó qúnyīng zhuàn Zhèngzōng Plus (Mixed Simplified and Traditional Chinese in title screen)
// modified title screen is only visible for china region, so use that by default.  Character select portraits don't seem quite right (different protection?)
GAME( 1999, kovsgqyz,     kovplus,   pgm_arm_type1_sim,      sango_ch,  pgm_arm_type1_state,  init_kovboot, ROT0,   "bootleg", "Sanguo Qunying Zhuan Zhengzong Plus (bootleg of Knights of Valour Plus, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 1999, kovsgqyza,    kovplus,   pgm_arm_type1_sim,      sango_ch,  pgm_arm_type1_state,  init_kovboot, ROT0,   "bootleg", "Sanguo Qunying Zhuan Zhengzong Plus (bootleg of Knights of Valour Plus, set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 1999, kovsgqyzb,    kovplus,   pgm_arm_type1_sim,      sango_ch,  pgm_arm_type1_state,  init_kovboot, ROT0,   "bootleg", "Sanguo Qunying Zhuan Zhengzong Plus (bootleg of Knights of Valour Plus, set 3)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */


/* -----------------------------------------------------------------------------------------------------------------------
   NOT Working (mostly due to needing internal protection roms dumped)
   -----------------------------------------------------------------------------------------------------------------------*/

//魔幻星座/Móhuàn xīngzuò
GAME( 1999, puzlstar,     pgm,       pgm_arm_type1_sim,      pstar,     pgm_arm_type1_state,  init_pstar,    ROT0,   "IGS (Metro license)", "Puzzle Star / Mohuan Xingzuo (ver. 100MG, 09/30/99 build)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 1999, puzlstara,    puzlstar,  pgm_arm_type1_sim,      pstar,     pgm_arm_type1_state,  init_pstar,    ROT0,   "IGS (Metro license)", "Puzzle Star / Mohuan Xingzuo (ver. 100MG, 09/20/99 build)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

//閃亮三合一/Shǎn liàng sān hé yī (Taiwan, Hong Kong, Oversea)
//闪亮三合一/Shǎn liàng sān hé yī (China)
GAME( 2004, pgm3in1,      pgm,       pgm_arm_type1_sim,      pgm3in1,   pgm_arm_type1_state,  init_pgm3in1,  ROT0,   "IGS", "Shan Liang San He Yi (Flash 3-in-1) (V102 08/23/04 13:03:26)", MACHINE_NOT_WORKING )  /* need internal rom of IGS027A */
GAME( 2004, pgm3in1c100,  pgm3in1,   pgm_arm_type1_sim,      pgm3in1,   pgm_arm_type1_state,  init_pgm3in1,  ROT0,   "IGS", "Shan Liang San He Yi (Flash 3-in-1) (V100 07/13/04 12:09:20)", MACHINE_NOT_WORKING )  /* need internal rom of IGS027A */


/* Games below this point are known to have an 'execute only' internal ROM area covering an area at the start of the internal ROM.  This can't be read when running code from either internal or external ROM space. */

//西游释厄传群魔乱舞/Xīyóu shì è zhuàn Qúnmóluànwǔ (China, Japan; Simplified Chinese)
//西遊釋厄傳群魔亂舞/Xīyóu shì è zhuàn Qúnmóluànwǔ (Hong Kong, World, Taiwan; Traditional Chinese)
//Oriental Legend 2/손오공 2/Son Ogong 2 (Korea)
// Simulation doesn't seem 100% so marked as NOT WORKING.  Probably wasn't released in all specified regions (protection device internal ROM supplies the region)  "Oriental Ex" is the identifier string used in test mode.
GAME( 2004, oldsplus,     pgm,           pgm_arm_type1_sim,      oldsplus,  pgm_arm_type1_state, init_oldsplus, ROT0,   "IGS", "Oriental Legend 2 (Korea) / Xiyou Shi E Zhuan Qunmoluanwu (World, China, Japan, Hong Kong, Taiwan) (ver. 205) [Oriental Ex]", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 2004, oldsplus203,  oldsplus,      pgm_arm_type1_sim,      oldsplus,  pgm_arm_type1_state, init_oldsplus, ROT0,   "IGS", "Oriental Legend 2 (Korea) / Xiyou Shi E Zhuan Qunmoluanwu (World, China, Japan, Hong Kong, Taiwan) (ver. 203) [Oriental Ex]", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

// we use the kovsh ARM rom for this, intercepting commands and changing them to match it, doesn't seem 100% correct tho so I'm leaving it as NOT WORKING; for example the ARM rom supplies addresses of Z80 music data sections, which have moved causing incorrect music, some damage rates could be different too.
// the game logo remains stuck on the screen during gameplay, but videos of the original board suggest this happens on real hardware as well
// if the internal ROM can't be extracted (likely case, execute only area and NO chance of custom code execution at all due to lack of external ROM) then a reference simulator should probably be written based on the actual
// kovsh code, tweaked based on tests done with this specific board to catch any different behaviors.  These all seem to be for China only, they don't work as expected when set to other regions.

//三国战记乱世枭雄/Sānguó zhàn jì Luànshì xiāoxióng (China; Simplified Chinese, Official?)
//三國戰記亂世梟雄/Sānguó zhàn jì Luànshì xiāoxióng (Traditional Chinese, Official?)
GAME( 2004, kovshp,       pgm,       pgm_arm_type1,          kovsh,     pgm_arm_type1_state, init_kovshp,   ROT0,   "IGS", "Knights of Valour Super Heroes Plus / Sanguo Zhan Ji Luanshi Xiaoxiong (ver. 101)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 2004, kovshp100,    kovshp,    pgm_arm_type1,          kovsh,     pgm_arm_type1_state, init_kovshp,   ROT0,   "IGS", "Knights of Valour Super Heroes Plus / Sanguo Zhan Ji Luanshi Xiaoxiong (ver. 100)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
// First(possibly) major updated version of kovsh with 3 game modes and new characters, Version is V201 and identification string is "SANGO EX+" instead "SANGO EX".
// 一统中原/Yītǒng zhōngyuán is game mode(this mode is similar as kovsh), and ROM labels too?
GAME( 1999, kovytzy,      pgm,       pgm_arm_type1,          kovsh,     pgm_arm_type1_state, init_kovshp,   ROT0,   "IGS", "Knights of Valour Super Heroes / Sanguo Zhan Ji Fengyun Zaiqi / Sangoku Senki Super Heroes (SANGO EX+) (ver. 201 'Yitong Zhongyuan', China)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
//傲世三国/Àoshì sānguó
// this bootleg is very close to kovshp
GAME( 2008, kovshxas,     kovshp,    pgm_arm_type1,          kovsh,     pgm_arm_type1_state, init_kovshxas, ROT0,   "bootleg", "Aoshi Sanguo (bootleg of Knights of Valour Super Heroes Plus, V202CN, Oct 6 2008 09:59:26)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
// these should be bootlegs of kovshp, but have further command changes in their ARMs and have a lot of code shuffled around, bootlegs of a different revision?
//乱世拳皇/Luànshì quánhuáng
GAME( 200?, kovlsqh,      kovshp,    pgm_arm_type1,          kovsh,     pgm_arm_type1_state, init_kovlsqh2, ROT0,   "bootleg", "Luanshi Quanhuang (bootleg of Knights of Valour Super Heroes Plus, ver. 200CN)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 200?, kovlsqh2,     kovshp,    pgm_arm_type1,          kovsh,     pgm_arm_type1_state, init_kovlsqh2, ROT0,   "bootleg", "Luanshi Quanhuang 2 (bootleg of Knights of Valour Super Heroes Plus, ver. 200CN)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
//乱世街霸/Luànshì jiē bà
GAME( 200?, kovlsjb,      kovshp,    pgm_arm_type1,          kovsh,     pgm_arm_type1_state, init_kovlsqh2, ROT0,   "bootleg", "Luanshi Jie Ba (bootleg of Knights of Valour Super Heroes Plus, ver. 200CN, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 200?, kovlsjba,     kovshp,    pgm_arm_type1,          kovsh,     pgm_arm_type1_state, init_kovlsqh2, ROT0,   "bootleg", "Luanshi Jie Ba (bootleg of Knights of Valour Super Heroes Plus, ver. 200CN, set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
