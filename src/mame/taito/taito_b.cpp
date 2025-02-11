// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
/***************************************************************************

Taito B System

driver by Jarek Burczynski, with help from:
Nicola Salmoria, Brian A. Troha, stephh, Gerardo Oporto Jorrin, David Graves
heavily based on Taito F2 System driver by Brad Oliver, Andrew Prime

The board uses TC0220IOC, TC0260DAR, TC0180VCU, and TC0140SYT.
Sonic Blast Man uses TC0510NIO instead of TC0220IOC.

The palette resolution is 12 bits in some games and 15 bits in others.

TODO:
- hitice: ice trails are incorrect.
- hitice: the pixel bitmap is not cleared on startup nor during attract mode. There's a
  kludge to fix it in the first case.
- rambo3u: has a lot of unmapped writes in the VCU area (log up to end of
  round 2) [viofight also does a few]
- The eprom games could have a single io handler if it's confirmed all
  3 use a special 4 player I/O chip. Puzzle Bobble and qzshowby use TC0640FIO, as do the
  more common sets of Space Invaders DX (i.e. not spacedxo).
- can the text layer scroll? (hitice: glass walls at beginning of match, also check when the
  screen wiggles after the puck hits the wall shortly into the first round of attract mode)
- sprites are not in perfect sync with the background. Check ashura, they are almost
  tight during gameplay but completely off in attract mode.
- realpunc: missing camera emulation.

The Taito B system is a fairly flexible hardware platform. It supports 4
separate layers of graphics - one 64x64 tiled scrolling background plane
of 16x16 tiles, a similar foreground plane, a sprite plane capable of sprite
zooming and 'pageable' text plane of 8x8 tiles.

Sound is handled by a Z80 with a YM2610 or YM2610B or YM2203's connected
to it. Different sound chips - depending on game.

The memory map for each of the games is similar but not identical.


Memory map for Rastan Saga 2 / Nastar / Nastar Warrior :

CPU 1 : 68000, uses irqs 2 & 4. One of the IRQs just sets a flag which is
checked in the other IRQ routine. Could be timed to vblank...

  0x000000 - 0x07ffff : ROM
  0x200000 - 0x201fff : palette RAM, 4096 total colors (0x1000 words)
  0x400000 - 0x403fff : 64x64 foreground layer (offsets 0x0000-0x1fff tile codes; offsets 0x2000-0x3fff tile attributes)
  0x404000 - 0x407fff : 64x64 background layer (offsets 0x0000-0x1fff tile codes; offsets 0x2000-0x3fff tile attributes)
  0x408000 - 0x408fff : 64x64 text layer
  0x410000 - 0x41197f : ??k of sprite RAM (this is the range that Rastan Saga II tests at startup time)
  0x413800 - 0x413bff : foreground (line/screen) scroll RAM
  0x413c00 - 0x413fff : background (line/screen) scroll RAM

  0x600000 - 0x607fff : 32k of CPU RAM
  0x800000 - 0x800003 : communication with sound CPU via TC0140SYT
  0xa00000 - 0xa0000f : input ports and dipswitches


Notes:
 Master of Weapon has secret command to select level:
 (sequence is the same as in Metal Black):
 - boot machine with service switch pressed
 - message appears: "SERVICE SWITCH ERROR"
 - press 1p start, 1p start, 1p start, service switch, 1p start
 - message appears: "SELECT BY DOWN SW"
 - select level with joy down/up
 - press 1p start button

Other games that have this feature:
 Rastan Saga 2
 Crime City
 Violence Fight
 Rambo 3



List of known B-System games:

    Rastan Saga II                  (YM2610 sound)
    Ashura Blaster                  (YM2610 sound)
    Crime City                      (YM2610 sound)
    Rambo 3 (two different versions)(YM2610 sound)
    Tetris                          (YM2610 sound)
    Space Invaders DX               (YM2610 sound, MB87078 - electronic volume control)
    Silent Dragon                   (YM2610 sound)
    Sel Feena                       (YM2610 sound)
    Ryujin                          (YM2610 sound)

    Violence Fight                  (YM2203 sound, 1xMSM6295 )
    Hit The Ice                     (YM2203 sound, 1xMSM6295 )
    Master of Weapon                (YM2203 sound)

    Quiz Sekai wa SHOW by shobai    (YM2610-B sound, MB87078 - electronic volume control)
    Puzzle Bobble                   (YM2610-B sound, MB87078 - electronic volume control)
    Sonic Blast Man                 (YM2610-B sound)

Nastar
Taito, 1988

PCB Layout
----------

K1100419A  J1100178A
|---------------------------------------|
|68000  B81-13.31  B81-08.50  DSWA  DSWB|
|       B81-09.30  B81-10.49  TC0220IOC |
|24MHz B81-05.21 6264  6264             |
|      B81-06.22 6264  6264             |
|                           27.164MHz   |
|B81-04.15                             J|
|B81-03.14                             A|
|             TC0180VCU     6264       M|
|62256 62256                           M|
|                           TC0260DAR  A|
|62256 62256                            |
|                              MB3735   |
|62256 62256                 6264       |
|             TC0140SYT      B81-11.37  |
|62256 62256                 Z80A       |
|                       16MHz           |
|B81-02.2                               |
|B81-01.1     YM2610 YM3016  TL074 TL074|
|---------------------------------------|

Notes:
      68000 clock: 12.000MHz (24 / 2)
        Z80 clock: 4.000MHz  (16 / 4)
     YM2610 clock: 8.000MHz  (16 / 2)
            Vsync: 60Hz



Violence Fight
Taito, 1989

PCB Layout
----------

K1100511A  J1100213A
|---------------------------------------|
|C16-01.1   6264     6264     DSWA DSWB |
|C16-02.2 C16-06.22 C16-07.41 TC0220IOC |
|C16-03.3 C16-14.23 C16-11.42           |
|C16-04.4      68000        6116        |
|               / C16-08                |
|           PALS\ C16-09    TC0260DAR  J|
|   TC0180VCU                          A|
|                                      M|
|        27.164MHz               TL074 M|
|                                      A|
|        24MHz                          |
|                               YM3014B |
|            PC060HA      YM2203        |
| 62256 62256            C16-05.47      |
| 62256 62256 Z80B                MB3735|
| 62256 62256 C16-12.32                 |
| 62256 62256 6264        M6295  TL074  |
|                  4.224MHz             |
|---------------------------------------|

Notes:
      68000 clock: 12.000MHz (24 / 2)
        Z80 clock: 6.000MHz  (24 / 4)
     YM2203 clock: 3.000MHz  (24 / 8)
      M6295 clock: 1.056MHz  (4.224 / 4), sample rate = 1056000 / 132
            Vsync: 60Hz


***************************************************************************

Master of Weapon, Taito 1989
Hardware info by Guru

PCB Layout
----------

B SYSTEM
K1100424A
J1100181A
K1100426A MAST OF WEAPON (sticker)
|------------------------------------------------------------------|
|VOL 4556  Y3014    YM2203       PC060HA       CXK58257  CXK58257  |
|  MB3735           Z80                B72-10.IC32                 |
|                   2018         B72_07.IC30                       |
|                                              CXK58257  CXK58257  |
|      62064  |-----|            TMM2063                           |
|             |TC0260                                              |
|J            |DAR  |            27.164MHz     CXK58257  CXK58257  |
|A            |-----|                                              |
|M                              |--------|                         |
|M                              |TAITO   |     CXK58257  CXK58257  |
|A                              |TC0180  |                         |
|    48CR-1                     |VCU     |                         |
|    48CR-1                     |--------|              B72-02.IC6 |
|    48CR-1                                                        |
|    48CR-1         TMM2063      TMM2063                B72-01.IC5 |
|            MB3771                                                |
| 48CR-1            B72_04.IC34  B72_03.IC25 B72-09.IC23 B72-08.IC3|
|       TC0040IOC                                            24MHz |
|   SW2   SW1       B72_06.IC33  B72_12.IC24          68000        |
|------------------------------------------------------------------|
Notes:
      68000 - Motorola MC68000P12 CPU. Clock input 12.000MHz [24/2]
        Z80 - Zilog Z0840006PSC CPU. Clock input 6.000MHz [24/4]
     YM2203 - Yamaha YM2203 FM Operator Type-N sound chip. Clock input 3.000MHz [24/8]
      Y3014 - Yamaha YM3014B Serial Input Floating D/A Converter. Clock input 1.0000MHz [24/24] on pin 5
    PC060HA - Taito PC060HA CIU custom IC (68K<>Z80 communication)
  TC0260DAR - Taito TC0260DAR custom IC (palette and RGB output)
  TC0180VCU - Taito TC0180VCU custom IC (tilemap)
  TC0040IOC - Taito TC0040IOC custom IC (controls, coin lockout, coin meter, DIP switch management and master reset/watchdog)
   CXK58257 - Sony CXK58257 32kBx8-bit SRAM (video RAM)
       2018 - Motorola MCM2018 2kBx8-bit SRAM (palette RAM)
    TMM2063 - Toshiba TMM2063 8kBx8-bit SRAM (program RAM for 68000 & Z80)
      62064 - Toshiba TD62064 4-Channel Darlington Sink Driver
     MB3771 - Fujitsu MB3771 Master Reset IC
      SW1/2 - 8-position DIP switch
     48CR-1 - Custom resistor array
       4556 - NEC uPC4556 Dual Operational Amplifier
    MBB3735 - Fujitsu MB3735 Audio Power Amplifier
B72_04.IC34 \
B72_03.IC25  \ M5M27C101 128kBx8-bit EPROM (main program)
B72_06.IC33  /
B72_12.IC24 /
B72_07.IC30 - 27C512 64kBx8-bit EPROM (sound program)
 B72-02.IC6 \ 234000 512kBx8-bit mask ROM (equivalent to AM27C400, graphics)
 B72-01.IC5 /
B72-09.IC23 - MMI PAL16L8 (PCB marked 'INT')
 B72-08.IC3 - MMI PAL16L8 (PCB marked 'DEC')
B72-10.IC32 - MMI PAL16L8
      HSync - 15.1782kHz \
      VSync - 60.00000Hz / <-- actual measurements on PCB

***************************************************************************/

#include "emu.h"
#include "taito_b.h"
#include "taitoipt.h"
#include "taitosnd.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"
#include "speaker.h"


void taitob_state::bankswitch_w(uint8_t data)
{
	m_audiobank->set_entry(data & 3);
}

template<int Player>
uint16_t rambo3_state::tracky_hi_r()
{
	return m_trackx_io[Player]->read();
}

template<int Player>
uint16_t rambo3_state::tracky_lo_r()
{
	return (m_trackx_io[Player]->read() & 0xff) << 8;
}

template<int Player>
uint16_t rambo3_state::trackx_hi_r()
{
	return m_tracky_io[Player]->read();
}

template<int Player>
uint16_t rambo3_state::trackx_lo_r()
{
	return (m_tracky_io[Player]->read() & 0xff) << 8;
}

INPUT_CHANGED_MEMBER(taitob_c_state::realpunc_sensor)
{
	m_maincpu->set_input_line(4, HOLD_LINE);
}

/***************************************************************************

  Puzzle Bobble, Qzshowby, Space DX   EEPROM

***************************************************************************/

uint16_t taitob_state::eep_latch_r()
{
	return m_eep_latch;
}

void taitob_state::eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_eep_latch);

	if (ACCESSING_BITS_8_15)
	{
		data >>= 8; /*M68k byte write*/

		/* bit 0 - Unused */
		/* bit 1 - Unused */
		/* bit 2 - Eeprom data  */
		/* bit 3 - Eeprom clock */
		/* bit 4 - Eeprom reset (active low) */
		/* bit 5 - Unused */
		/* bit 6 - Unused */
		/* bit 7 - set all the time (Chip Select?) */

		/* EEPROM */
		m_eepromout_io->write(data, 0xff);
	}
}

/*************************************************************************
   The input area for the three eprom games ($500000-2f) may well be
   addressing a single i/o chip with 4 player and coin inputs as
   standard. It's confirmed that all of these use TC0640FIO.
*************************************************************************/


void taitob_state::player_12_coin_ctrl_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 0));
	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 1));
	machine().bookkeeping().coin_counter_w(0, BIT( data, 2));
	machine().bookkeeping().coin_counter_w(1, BIT( data, 3));
}

uint16_t taitob_state::player_34_coin_ctrl_r()
{
	return m_coin_word;
}

void taitob_state::player_34_coin_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_coin_word);

	/* coin counters and lockout */
	machine().bookkeeping().coin_lockout_w(2, BIT(~data, 8));
	machine().bookkeeping().coin_lockout_w(3, BIT(~data, 9));
	machine().bookkeeping().coin_counter_w(2, BIT( data, 10));
	machine().bookkeeping().coin_counter_w(3, BIT( data, 11));
}

void taitob_state::spacedxo_tc0220ioc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_tc0220ioc->write(offset, data & 0xff);
	else
	{
		/* spacedxo also writes here - bug? */
		m_tc0220ioc->write(offset, (data >> 8) & 0xff);
	}
}

void taitob_c_state::output_w(uint16_t data)
{
/*
   15 = Camera Enable?
   14 = Lamp 2?
   13 = Lamp 1?
*/
}


void taitob_state::rastsag2_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x201fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x400000, 0x47ffff).m(m_tc0180vcu, FUNC(tc0180vcu_device::tc0180vcu_memrw));
	map(0x600000, 0x607fff).ram(); /* Main RAM */ /*ashura up to 603fff only*/
	map(0x800000, 0x800001).nopr();
	map(0x800000, 0x800000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x800002, 0x800002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0xa00000, 0xa0000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0xff00);
}


void taitob_state::crimec_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x20000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0xff00);
	map(0x400000, 0x47ffff).m(m_tc0180vcu, FUNC(tc0180vcu_device::tc0180vcu_memrw));
	map(0x600000, 0x600001).nopr();
	map(0x600000, 0x600000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x600002, 0x600002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x800000, 0x801fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xa00000, 0xa0ffff).ram(); /* Main RAM */
}


void taitob_state::tetrist_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x200001).nopr();
	map(0x200000, 0x200000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x200002, 0x200002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x400000, 0x47ffff).m(m_tc0180vcu, FUNC(tc0180vcu_device::tc0180vcu_memrw));
	map(0x600000, 0x60000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0xff00);
	map(0x800000, 0x807fff).ram(); /* Main RAM */
	map(0xa00000, 0xa01fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
}

void taitob_state::tetrista_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x201fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x400000, 0x47ffff).m(m_tc0180vcu, FUNC(tc0180vcu_device::tc0180vcu_memrw));
	map(0x600000, 0x600003).rw("tc0040ioc", FUNC(tc0040ioc_device::read), FUNC(tc0040ioc_device::write)).umask16(0xff00);
	map(0x800000, 0x803fff).ram(); /* Main RAM */
	map(0xa00000, 0xa00001).nopr();
	map(0xa00000, 0xa00000).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0xa00002, 0xa00002).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
}


void hitice_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x400000, 0x47ffff).m(m_tc0180vcu, FUNC(tc0180vcu_device::tc0180vcu_memrw));
	map(0x600000, 0x60000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0xff00);
	map(0x610000, 0x610001).portr("P3_P4");
	map(0x700000, 0x700001).nopr();
	map(0x700000, 0x700000).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0x700002, 0x700002).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	map(0x800000, 0x803fff).ram(); /* Main RAM */
	map(0xa00000, 0xa01fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xb00000, 0xb7ffff).ram().w(FUNC(hitice_state::pixelram_w)).share("pixelram");
	map(0xbffff0, 0xbffff5).w(FUNC(hitice_state::pixel_scroll_w));
//  { 0xbffffa, 0xbffffb, ???
}


void rambo3_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x200001).nopr();
	map(0x200000, 0x200000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x200002, 0x200002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x400000, 0x47ffff).m(m_tc0180vcu, FUNC(tc0180vcu_device::tc0180vcu_memrw));
	map(0x600000, 0x60000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0xff00);
	map(0x600010, 0x600011).r(FUNC(rambo3_state::tracky_lo_r<0>)); /*player 1*/
	map(0x600012, 0x600013).r(FUNC(rambo3_state::tracky_hi_r<0>));
	map(0x600014, 0x600015).r(FUNC(rambo3_state::trackx_lo_r<0>));
	map(0x600016, 0x600017).r(FUNC(rambo3_state::trackx_hi_r<0>));
	map(0x600018, 0x600019).r(FUNC(rambo3_state::tracky_lo_r<1>)); /*player 2*/
	map(0x60001a, 0x60001b).r(FUNC(rambo3_state::tracky_hi_r<1>));
	map(0x60001c, 0x60001d).r(FUNC(rambo3_state::trackx_lo_r<1>));
	map(0x60001e, 0x60001f).r(FUNC(rambo3_state::trackx_hi_r<1>));
	map(0x800000, 0x803fff).ram(); /* Main RAM */
	map(0xa00000, 0xa01fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
}


void taitob_state::pbobble_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x400000, 0x47ffff).m(m_tc0180vcu, FUNC(tc0180vcu_device::tc0180vcu_memrw));
	map(0x500000, 0x50000f).rw(m_tc0640fio, FUNC(tc0640fio_device::halfword_byteswap_r), FUNC(tc0640fio_device::halfword_byteswap_w));
	map(0x500024, 0x500025).portr("P3_P4_A");        /* shown in service mode, game omits to read it */
	map(0x500026, 0x500027).rw(FUNC(taitob_state::eep_latch_r), FUNC(taitob_state::eeprom_w));
	map(0x500028, 0x500029).w(FUNC(taitob_state::player_34_coin_ctrl_w));    /* simply locks coins 3&4 out */
	map(0x50002e, 0x50002f).portr("P3_P4_B");        /* shown in service mode, game omits to read it */
	map(0x600000, 0x600003).w("mb87078", FUNC(mb87078_device::data_w)).umask16(0xff00);
	map(0x700000, 0x700001).nopr();
	map(0x700000, 0x700000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x700002, 0x700002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x800000, 0x801fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x900000, 0x90ffff).ram(); /* Main RAM */
}

/* identical to pbobble, above??? */
void taitob_state::spacedx_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x400000, 0x47ffff).m(m_tc0180vcu, FUNC(tc0180vcu_device::tc0180vcu_memrw));
	map(0x500000, 0x50000f).rw(m_tc0640fio, FUNC(tc0640fio_device::halfword_byteswap_r), FUNC(tc0640fio_device::halfword_byteswap_w));
	map(0x500024, 0x500025).portr("P3_P4_A");
	map(0x500026, 0x500027).rw(FUNC(taitob_state::eep_latch_r), FUNC(taitob_state::eeprom_w));
	map(0x500028, 0x500029).w(FUNC(taitob_state::player_34_coin_ctrl_w));    /* simply locks coins 3&4 out */
	map(0x50002e, 0x50002f).portr("P3_P4_B");
	map(0x600000, 0x600003).w("mb87078", FUNC(mb87078_device::data_w)).umask16(0xff00);
	map(0x700000, 0x700001).nopr();
	map(0x700000, 0x700000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x700002, 0x700002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x800000, 0x801fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x900000, 0x90ffff).ram(); /* Main RAM */
}

void taitob_state::spacedxo_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x100001).nopr();
	map(0x100000, 0x100000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x100002, 0x100002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x200000, 0x20000f).r(m_tc0220ioc, FUNC(tc0220ioc_device::read)).umask16(0x00ff);
	map(0x200000, 0x20000f).w(FUNC(taitob_state::spacedxo_tc0220ioc_w));
	map(0x210000, 0x210001).portr("IN3");
	map(0x220000, 0x220001).portr("IN4");
	map(0x230000, 0x230001).portr("IN5");
	map(0x300000, 0x301fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x302000, 0x303fff).readonly();
	map(0x400000, 0x40ffff).ram(); /* Main RAM */
	map(0x500000, 0x57ffff).m(m_tc0180vcu, FUNC(tc0180vcu_device::tc0180vcu_memrw));
}


void taitob_state::qzshowby_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x20000f).rw(m_tc0640fio, FUNC(tc0640fio_device::halfword_byteswap_r), FUNC(tc0640fio_device::halfword_byteswap_w));
	map(0x200024, 0x200025).portr("P3_P4_A");    /* player 3,4 start */
	map(0x200026, 0x200027).w(FUNC(taitob_state::eeprom_w));
	map(0x200028, 0x200029).rw(FUNC(taitob_state::player_34_coin_ctrl_r), FUNC(taitob_state::player_34_coin_ctrl_w));
	map(0x20002e, 0x20002f).portr("P3_P4_B");    /* player 3,4 buttons */
	map(0x400000, 0x47ffff).m(m_tc0180vcu, FUNC(tc0180vcu_device::tc0180vcu_memrw));
	map(0x600000, 0x600001).nopr();
	map(0x600000, 0x600000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x600002, 0x600002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x700000, 0x700003).w("mb87078", FUNC(mb87078_device::data_w)).umask16(0xff00);
	map(0x800000, 0x801fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x900000, 0x90ffff).ram(); /* Main RAM */
}


void taitob_state::viofight_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x200001).nopr();
	map(0x200000, 0x200000).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0x200002, 0x200002).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	map(0x400000, 0x47ffff).m(m_tc0180vcu, FUNC(tc0180vcu_device::tc0180vcu_memrw));
	map(0x600000, 0x601fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x800000, 0x80000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0xff00);
	map(0xa00000, 0xa03fff).ram(); /* Main RAM */
}


void taitob_state::masterw_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x203fff).ram(); /* Main RAM */
	map(0x400000, 0x47ffff).m(m_tc0180vcu, FUNC(tc0180vcu_device::tc0180vcu_memrw));
	map(0x600000, 0x601fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x800000, 0x800003).rw("tc0040ioc", FUNC(tc0040ioc_device::read), FUNC(tc0040ioc_device::write)).umask16(0xff00);
	map(0xa00000, 0xa00001).nopr();
	map(0xa00000, 0xa00000).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0xa00002, 0xa00002).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
}


void taitob_state::silentd_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x100001).nopr();
	map(0x100000, 0x100000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x100002, 0x100002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
//  map(0x10001a, 0x10001b).nopr(); // ??? read at $1e344
//  map(0x10001c, 0x10001d).nopr(); // ??? read at $1e356
	map(0x200000, 0x20000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
	map(0x210000, 0x210001).portr("IN3");
	map(0x220000, 0x220001).portr("IN4");
	map(0x230000, 0x230001).portr("IN5");
	map(0x240000, 0x240001).nopw(); // ???
//  map(0x240000, 0x240001).nopr(); /* read 4 times at init */
	map(0x300000, 0x301fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x400000, 0x403fff).ram(); /* Main RAM */
	map(0x500000, 0x57ffff).m(m_tc0180vcu, FUNC(tc0180vcu_device::tc0180vcu_memrw));
}


void taitob_state::selfeena_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram(); /* Main RAM */
	map(0x200000, 0x27ffff).m(m_tc0180vcu, FUNC(tc0180vcu_device::tc0180vcu_memrw));
	map(0x300000, 0x301fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x400000, 0x40000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0xff00);
	map(0x410000, 0x41000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0xff00); /* mirror address - seems to be only used for coin control */
	map(0x500000, 0x500001).nopr();
	map(0x500000, 0x500000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x500002, 0x500002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
}


void taitob_state::sbm_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram(); /* Main RAM */
	map(0x200000, 0x201fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x300000, 0x30000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_wordswap_r), FUNC(tc0510nio_device::halfword_wordswap_w));
	map(0x320000, 0x320001).nopr();
	map(0x320000, 0x320000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x320002, 0x320002).rw("tc0140syt", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x900000, 0x97ffff).m(m_tc0180vcu, FUNC(tc0180vcu_device::tc0180vcu_memrw));
}

void taitob_c_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x110000, 0x12ffff).ram();
	map(0x130000, 0x13ffff).ram(); // Check me
	map(0x180000, 0x18000f).rw(m_tc0510nio, FUNC(tc0510nio_device::halfword_wordswap_r), FUNC(tc0510nio_device::halfword_wordswap_w));
	map(0x184000, 0x184001).w(FUNC(taitob_c_state::video_ctrl_w));
	map(0x188000, 0x188001).nopr();
	map(0x188000, 0x188000).w("tc0140syt", FUNC(tc0140syt_device::master_port_w));
	map(0x188002, 0x188003).nopr();
	map(0x188002, 0x188002).w("tc0140syt", FUNC(tc0140syt_device::master_comm_w));
	map(0x18c000, 0x18c001).w(FUNC(taitob_c_state::output_w));
	map(0x200000, 0x27ffff).m(m_tc0180vcu, FUNC(tc0180vcu_device::tc0180vcu_memrw));
	map(0x280000, 0x281fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x300000, 0x300003).rw(m_hd63484, FUNC(hd63484_device::read16), FUNC(hd63484_device::write16));
//  map(0x320000, 0x320001).nop(); // ?
	map(0x320002, 0x320003).nopr();
	map(0x320002, 0x320002).w("tc0140syt", FUNC(tc0140syt_device::master_comm_w));
}

void taitob_c_state::hd63484_map(address_map &map)
{
	map(0x00000, 0x7ffff).ram();
}

void taitob_state::masterw_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr(m_audiobank);
	map(0x8000, 0x8fff).ram();
	map(0x9000, 0x9001).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xa000, 0xa000).w("ciu", FUNC(pc060ha_device::slave_port_w));
	map(0xa001, 0xa001).rw("ciu", FUNC(pc060ha_device::slave_comm_r), FUNC(pc060ha_device::slave_comm_w));
}

void taitob_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr(m_audiobank);
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe003).rw("ymsnd", FUNC(ym_generic_device::read), FUNC(ym_generic_device::write));
	map(0xe200, 0xe200).nopr().w("tc0140syt", FUNC(tc0140syt_device::slave_port_w));
	map(0xe201, 0xe201).rw("tc0140syt", FUNC(tc0140syt_device::slave_comm_r), FUNC(tc0140syt_device::slave_comm_w));
	map(0xe400, 0xe403).nopw(); /* pan */
	map(0xe600, 0xe600).nopw(); /* ? */
	map(0xea00, 0xea00).nopr();
	map(0xee00, 0xee00).nopw(); /* ? */
	map(0xf000, 0xf000).nopw(); /* ? */
	map(0xf200, 0xf200).w(FUNC(taitob_state::bankswitch_w));
}

void taitob_state::viofight_sound_map(address_map &map)
{
	masterw_sound_map(map);
	map(0xb000, 0xb001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));       /* yes, both addresses for the same chip */
}


/***********************************************************
             INPUT PORTS, DIPs
***********************************************************/

#define TAITO_B_SYSTEM_INPUT \
	PORT_START("IN2") \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT ) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )


static INPUT_PORTS_START( rastsag2 ) /* Japanese version */
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)  // all 2 "unused" in manual
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "100k only" )
	PORT_DIPSETTING(    0x08, "150k only" )
	PORT_DIPSETTING(    0x04, "200k only" )
	PORT_DIPSETTING(    0x00, "250k only" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */

	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS( 2 )

	TAITO_B_SYSTEM_INPUT
INPUT_PORTS_END

static INPUT_PORTS_START( nastar ) /* World version */
	PORT_INCLUDE(rastsag2)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_WORLD_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( nastarw ) /* USA version */
	PORT_INCLUDE(rastsag2)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( masterw )
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "500k, 1000k and 1500k" )
	PORT_DIPSETTING(    0x0c, "500k and 1000k" )
	PORT_DIPSETTING(    0x04, "500k only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x80, 0x80, "Ship Type" )         PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "Space Ship" )
	PORT_DIPSETTING(    0x00, "Hover Cycle" )

	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS( 2 )

	TAITO_B_SYSTEM_INPUT
INPUT_PORTS_END

static INPUT_PORTS_START( masterwj )
	PORT_INCLUDE(masterw)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( masterwu )
	PORT_INCLUDE(masterw)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( yukiwo )
	PORT_INCLUDE(masterw)

	PORT_MODIFY("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* This prototype doesn't seem to have the "Space Ship" option */
INPUT_PORTS_END

static INPUT_PORTS_START( crimec )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "Hi Score" )          PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Scribble" )
	PORT_DIPSETTING(    0x00, "3 Characters" )
	TAITO_DSWA_BITS_1_TO_3_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "every 80k" )
	PORT_DIPSETTING(    0x0c, "80k only" )
	PORT_DIPSETTING(    0x04, "160k only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x40, "5 Times" )
	PORT_DIPSETTING( 0x80, "8 Times" )
	PORT_DIPSETTING( 0xc0, DEF_STR( On ) )

	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS( 2 )

	TAITO_B_SYSTEM_INPUT
INPUT_PORTS_END

static INPUT_PORTS_START( crimecj )
	PORT_INCLUDE(crimec)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( crimecu )
	PORT_INCLUDE(crimec)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( tetrist )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */

	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS( 2 )

	TAITO_B_SYSTEM_INPUT
INPUT_PORTS_END

static INPUT_PORTS_START( ashura )
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "every 100k" )
	PORT_DIPSETTING(    0x0c, "every 150k" )
	PORT_DIPSETTING(    0x04, "every 200k" )
	PORT_DIPSETTING(    0x00, "every 250k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7") /* Listed as Unused in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:8") /* Listed as Unused in the manual */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS( 2 )

	TAITO_B_SYSTEM_INPUT
INPUT_PORTS_END

static INPUT_PORTS_START( ashuraj )
	PORT_INCLUDE(ashura)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( ashurau )
	PORT_INCLUDE(ashura)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( hitice )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "Cabinet Style" )     PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "4 Players")
	PORT_DIPSETTING(    0x00, "2 Players")
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW1:2" )        /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )        /* Listed as "Unused" */

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, "Timer count" )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "1 sec = 58/60" )
	PORT_DIPSETTING(    0x04, "1 sec = 56/60" )
	PORT_DIPSETTING(    0x08, "1 sec = 62/60" )
	PORT_DIPSETTING(    0x00, "1 sec = 45/60" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x80, 0x80, "Maximum credits" )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, "99" )
	PORT_DIPSETTING(    0x80, "9"  )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("P3_P4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START4 )
INPUT_PORTS_END

static INPUT_PORTS_START( hiticej )
	PORT_INCLUDE(hitice)

	PORT_MODIFY("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( rambo3 )
	PORT_START("DSWA")  /* DSW A */
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x08, 0x08, "Control" )           PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "8 way Joystick" )
	PORT_DIPSETTING(    0x00, DEF_STR( Trackball ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("TRACKX1")
	PORT_BIT( 0xffff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("TRACKY1")
	PORT_BIT( 0xffff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("TRACKX2")
	PORT_BIT( 0xffff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("TRACKY2")
	PORT_BIT( 0xffff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( rambo3u )
	PORT_INCLUDE(rambo3)

	PORT_MODIFY("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_US_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( rambo3p )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)  // all 5 "unused" in manual
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */

	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS( 2 )

	TAITO_B_SYSTEM_INPUT

	PORT_START("TRACKX1")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("TRACKY1")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("TRACKX2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("TRACKY2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


/* Helps document the input ports. */

static INPUT_PORTS_START( pbobble ) /* Missing P3&4 controls ! */
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW ) /*ok*/

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) /*ok*/
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) /*ok*/
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2) /*ok*/
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2) /*ok*/

	PORT_START("START")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START("P1_P2_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/

	PORT_START("P1_P2_B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("P3_P4_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/

	PORT_START("P3_P4_B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
INPUT_PORTS_END

static INPUT_PORTS_START( spacedxo )
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, "Match Point" )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "1500 Points" )
	PORT_DIPSETTING(    0x00, "1000 Points" )
	PORT_DIPNAME( 0x80, 0x80, "Game Type" )         PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "Double Company" )
	PORT_DIPSETTING(    0x00, "Single Company" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( qzshowby )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*unused in test mode*/
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW ) /*ok*/

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) /*ok*/
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) /*ok*/
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2) /*ok*/
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2) /*ok*/

	PORT_START("START")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START("P1_P2_A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* IPT_START1 in test mode */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* IPT_START2 in test mode */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_P2_B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("P3_P4_A")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN  ) /* IPT_START3 in test mode */
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN  ) /* IPT_START4 in test mode */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3_P4_B")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
INPUT_PORTS_END

static INPUT_PORTS_START( viofight )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)  // all 7 "unused" in manual
	TAITO_COINAGE_WORLD_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */

	PORT_START("IN0")
	TAITO_JOY_UDLR_3_BUTTONS( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_3_BUTTONS( 2 )

	TAITO_B_SYSTEM_INPUT
INPUT_PORTS_END

static INPUT_PORTS_START( viofightj )
	PORT_INCLUDE(viofight)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( viofightu )
	PORT_INCLUDE(viofight)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( silentd ) /* World Version */
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)          /* Listed as "NOT USED" in the manual and only shown as "OFF" */
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )        /* Listed as "Unused" */

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x04, 0x04, "Friendly Fire" )   PORT_DIPLOCATION("SW2:3") // "hit of players" or "invincible player mode"
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Power-Up at Stage Clear" )   PORT_DIPLOCATION("SW2:4") // "clear stage power-up"
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Regain Power buy-in" )      PORT_DIPLOCATION("SW2:5") // If enabled player can use a credit to refill his HP and get a "rage" mode for few seconds
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )

/*  Manual Shows "1 Coin Slot (Shared)" and "4 Coin Slot (1 Per Player)"

    You can play the game with 2, 3, or 4 players and the last option is a linked 4 players.
    Using bit6 and bit7&8 you end up with 1, 2 or 4 separate "Credits" on the demo screens.
    Using bits7&8 you can have 2-4 players as shown at the top of the game screens.

*/

	PORT_DIPNAME( 0x20, 0x20, "Credits" )           PORT_DIPLOCATION("SW2:6") /* Only shows 4 separate credits with 4p/1m below */
	PORT_DIPSETTING(    0x20, "Combined" )
	PORT_DIPSETTING(    0x00, "Separate" )          /* When multiple credits show, Coin B will affect p2 credits */
	PORT_DIPNAME( 0xc0, 0x80, "Cabinet Style" )     PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, "3 Players" )
	PORT_DIPSETTING(    0x80, "2 Players" )
	PORT_DIPSETTING(    0x40, "4 Players/1 Machine" )   /* with bit6, shows 4 separate credits */
	PORT_DIPSETTING(    0x00, "4 Players/2 Machines" )  /* with bit6 shows 2 separate credits */

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2) PORT_NAME ("Coin 3 2nd input")/*not sure if this is legal under MAME*/
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2) PORT_NAME ("Coin 4 2nd input")/*not sure if this is legal under MAME*/
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( silentdj )
	PORT_INCLUDE(silentd)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END

static INPUT_PORTS_START( silentdu )
	PORT_INCLUDE(silentd)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SW1)
INPUT_PORTS_END


static INPUT_PORTS_START( selfeena )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "100k only" )
	PORT_DIPSETTING(    0x08, "200k only" )
	PORT_DIPSETTING(    0x04, "300k only" )
	PORT_DIPSETTING(    0x00, "400k only" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( ryujin )
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SW1)
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN3")
	PORT_BIT(  0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT(  0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN5")
	PORT_BIT(  0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( sbm )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "Tickets" )                   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Win Bonus Tickets" )         PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, "Punch Power" )               PORT_DIPLOCATION("SW2:3,4")
	/* Modifies the power of the player's punch by a set amount. Table is at 0x0A614:
	    DIP 00: 0000 ;+00
	    DIP 01: FFFB ;-05
	    DIP 10: 0005 :+05
	    DIP 11: 000A :+10
	    For example, if the player had an initial power of 200t and Dip Switches 2-3 and 2-4 were both set, the power would be increased to 210t.
	    This bonus (or penalty) is applied before the game floors the punch power at 1t and before it caps the punch power at 300t. So, if in MAME, you hold Pad Photosensors 1 and 2 and then immediately hit Pad Photosensor 3, you'll still get a punch power of 300t even if Dip Switch 2-3 was set and Dip Switch 2-4 wasn't.
	    Just for reference, the initial power of the player's punch is calculated by taking 24000 and dividing it by the timer at 105048 in RAM. This is performed in the subroutine at 0x0A5B0 in the main CPU.
	*/
	PORT_DIPSETTING(    0x08, "Lower" )
	PORT_DIPSETTING(    0x0c, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Higher ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Highest ) )
	PORT_DIPNAME( 0x30, 0x30, "Tickets Payout" )            PORT_DIPLOCATION("SW2:5,6")
	/* Controls the power divider. The game calculates the base ticket total by taking the total power of the player's three punches and then dividing it by one of four values in the table at 0x7FFC0:
	    DIP 00: 50
	    DIP 01: 40
	    DIP 10: 80
	    DIP 11: 120
	    For example, if the player got a combined total of 640t power and dip switch 2-6 was set but 2-5 was not, they'll have a base of 8 tickets.
	*/
	PORT_DIPSETTING(    0x00, "Total Power / 120" )
	PORT_DIPSETTING(    0x10, "Total Power / 80" )
	PORT_DIPSETTING(    0x30, "Total Power / 50" )
	PORT_DIPSETTING(    0x20, "Total Power / 40" )
	PORT_DIPNAME( 0xc0, 0xc0, "Win Bonus Tickets Payout" )  PORT_DIPLOCATION("SW2:7,8")
	/* Controls the value of the round completion ticket bonus. Each round has its own ticket table: The first value is for 00, the second is for 01, third for 10, fourth for 11:
	    7FFC8: 0001 0002 0003 0002 #ROUND 1 BONUS TABLE - 01 / 02 / 03 / 02
	    7FFD0: 0002 0004 0004 0004 #ROUND 2 BONUS TABLE - 02 / 04 / 04 / 04
	    7FFD8: 0003 0006 0005 0008 #ROUND 3 BONUS TABLE - 03 / 06 / 05 / 08
	    7FFE0: 0004 0008 0006 0010 #ROUND 4 BONUS TABLE - 04 / 08 / 06 / 16
	    7FFE8: 0005 000A 0007 0020 #ROUND 5 BONUS TABLE - 05 / 10 / 07 / 32
	    For example, if the player beat round 4 and dip switch 2-7 was set but 2-8 was not, they'd have a bonus of 8 tickets.
	*/
	PORT_DIPSETTING(    0xc0, "1, 2, 3, 4, 5" )
	PORT_DIPSETTING(    0x80, "2, 4, 6, 8, 10" )
	PORT_DIPSETTING(    0x40, "3, 4, 5, 6, 7" )
	PORT_DIPSETTING(    0x00, "2, 4, 8, 16, 32" )

	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)//sound select UP
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)//sound select DOWN
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)//ok (object test)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)//ok (object test)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)//-- unused in test modes
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)//-- unused in test modes
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)//BEN IN (ticket dispenser)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)//LADY ????

	PORT_START("START")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Select")//select; ok (1P in object test)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Start")//start ; ok (2P in object test)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PHOTOSENSOR")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )       //ok
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )   //ok
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)   //ok
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)   //ok
	/* BUTTON1 ACTIVE LOW, - game thinks that punching pad has already been raised */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Pad Photosensor 1")//PHOTO 1 (punching pad photosensor 1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Pad Photosensor 2")//PHOTO 2 (punching pad photosensor 2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH,IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Pad Photosensor 3")//PHOTO 3 (punching pad photosensor 3)
	/*To simulate a punch:
	    - wait for "READY GO!" message,
	    - press button1 + button 2 (LCTRL + ALT) (you'll hear a "punching" sound),
	    - THEN  press button 3 (SPACE)
	    The time passed between the presses will be used to calculate the power of your punch.
	    The longer the time - the less power.
	*/
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Pad Photosensor 4")//PHOTO 4  ??? ACTIVE_LOW  ??? (punching pad photosensor 4)
INPUT_PORTS_END

static INPUT_PORTS_START( sbmj )
	PORT_INCLUDE(sbm)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( realpunc )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "Difficulty 1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x02, "Difficulty 2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Difficulty 3" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,IPT_BUTTON1 ) PORT_NAME("Safety switch")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,IPT_BUTTON2 ) PORT_NAME("Pad Photosensor 1 (N)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(taitob_c_state::realpunc_sensor), 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,IPT_BUTTON3 ) PORT_NAME("Pad Photosensor 2 (U)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(taitob_c_state::realpunc_sensor), 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,IPT_BUTTON4 ) PORT_NAME("Pad Photosensor 3 (D)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(taitob_c_state::realpunc_sensor), 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,IPT_UNKNOWN )
INPUT_PORTS_END


/*
    Games that use the mb87078 are: pbobble, spacedx and qzshowby
    schems are not available, but from the writes I guess that
    they only use channel 1
    The sound chips' volume altered with the mb87078 are:
    ym2610 in spacedx,
    ym2610b in pbobble,qzshowby,

    Both ym2610 and ym2610b generate 3 (PSG like) + 2 (fm left,right) channels.
    I use mixer_set_volume() to emulate the effect.
*/
void taitob_state::mb87078_gain_changed(offs_t offset, uint8_t data)
{
	if (offset == 1)
	{
		device_sound_interface *sound;
		m_ym->interface(sound);
		sound->set_output_gain(0, data / 100.0);
		sound->set_output_gain(1, data / 100.0);
		sound->set_output_gain(2, data / 100.0);
		//popmessage("MB87078 gain ch#%i percent=%i", offset, data);
	}
}


void taitob_state::machine_start()
{
	m_audiobank->configure_entries(0, 4, memregion("audiocpu")->base(), 0x4000);

	save_item(NAME(m_eep_latch));
	save_item(NAME(m_coin_word));
}

void taitob_state::machine_reset()
{
	m_eep_latch = 0;
	m_coin_word = 0;
}


void taitob_state::rastsag2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);   /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitob_state::rastsag2_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 4);  /* 4 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &taitob_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(taitob_state::player_12_coin_ctrl_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(taitob_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 4096);

	TC0180VCU(config, m_tc0180vcu, 27.164_MHz_XTAL / 4);
	m_tc0180vcu->set_fb_colorbase(0x40);
	m_tc0180vcu->set_bg_colorbase(0xc0);
	m_tc0180vcu->set_fg_colorbase(0x80);
	m_tc0180vcu->set_tx_colorbase(0x00);
	m_tc0180vcu->set_palette(m_palette);
	m_tc0180vcu->inth_callback().set_inputline(m_maincpu, M68K_IRQ_4, HOLD_LINE);
	m_tc0180vcu->intl_callback().set_inputline(m_maincpu, M68K_IRQ_2, HOLD_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2610_device &ymsnd(YM2610(config, m_ym, 16_MHz_XTAL / 2));  /* 8 MHz */
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


void taitob_state::masterw(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);   /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitob_state::masterw_map);

	Z80(config, m_audiocpu, 24_MHz_XTAL / 4);  /* 6 MHz Z80B */
	m_audiocpu->set_addrmap(AS_PROGRAM, &taitob_state::masterw_sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	tc0040ioc_device &tc0040ioc(TC0040IOC(config, "tc0040ioc", 0));
	tc0040ioc.read_0_callback().set_ioport("DSWA");
	tc0040ioc.read_1_callback().set_ioport("DSWB");
	tc0040ioc.read_2_callback().set_ioport("IN0");
	tc0040ioc.read_3_callback().set_ioport("IN1");
	tc0040ioc.write_4_callback().set(FUNC(taitob_state::player_12_coin_ctrl_w));
	tc0040ioc.read_7_callback().set_ioport("IN2");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(taitob_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 4096);

	TC0180VCU(config, m_tc0180vcu, 27.164_MHz_XTAL / 4);
	m_tc0180vcu->set_fb_colorbase(0x10);
	m_tc0180vcu->set_bg_colorbase(0x30);
	m_tc0180vcu->set_fg_colorbase(0x20);
	m_tc0180vcu->set_tx_colorbase(0x00);
	m_tc0180vcu->set_palette(m_palette);
	m_tc0180vcu->inth_callback().set_inputline(m_maincpu, M68K_IRQ_5, HOLD_LINE);
	m_tc0180vcu->intl_callback().set_inputline(m_maincpu, M68K_IRQ_4, HOLD_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, m_ym, 24_MHz_XTAL / 8));  /* 3 MHz */
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.port_a_write_callback().set_membank(m_audiobank).mask(0x03);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 0.25);
	ymsnd.add_route(2, "mono", 0.25);
	ymsnd.add_route(3, "mono", 0.80);

	pc060ha_device &ciu(PC060HA(config, "ciu", 0));
	ciu.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	ciu.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


void taitob_state::tetrist(machine_config &config) /* Nastar conversion kit with slightly different memory map */
{
	rastsag2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &taitob_state::tetrist_map);
	m_palette->set_format(palette_device::RRRRGGGGBBBBRGBx, 4096);
}


void taitob_state::tetrista(machine_config &config) /* Master of Weapon conversion kit with slightly different memory map */
{
	masterw(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &taitob_state::tetrista_map);
	m_palette->set_format(palette_device::RRRRGGGGBBBBRGBx, 4096);
}


void taitob_state::ashura(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);   /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitob_state::rastsag2_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 4);  /* 4 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &taitob_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(taitob_state::player_12_coin_ctrl_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(taitob_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 4096);

	TC0180VCU(config, m_tc0180vcu, 27.164_MHz_XTAL / 4);
	m_tc0180vcu->set_fb_colorbase(0x40);
	m_tc0180vcu->set_bg_colorbase(0xc0);
	m_tc0180vcu->set_fg_colorbase(0x80);
	m_tc0180vcu->set_tx_colorbase(0x00);
	m_tc0180vcu->set_palette(m_palette);
	m_tc0180vcu->inth_callback().set_inputline(m_maincpu, M68K_IRQ_4, HOLD_LINE);
	m_tc0180vcu->intl_callback().set_inputline(m_maincpu, M68K_IRQ_2, HOLD_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2610_device &ymsnd(YM2610(config, m_ym, 16_MHz_XTAL / 2));  /* 8 MHz */
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


void taitob_state::crimec(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);   /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitob_state::crimec_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 4);  /* 4 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &taitob_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(taitob_state::player_12_coin_ctrl_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(taitob_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 4096);

	TC0180VCU(config, m_tc0180vcu, 27.164_MHz_XTAL / 4);
	m_tc0180vcu->set_fb_colorbase(0x80);
	m_tc0180vcu->set_bg_colorbase(0x00);
	m_tc0180vcu->set_fg_colorbase(0x40);
	m_tc0180vcu->set_tx_colorbase(0xc0);
	m_tc0180vcu->set_palette(m_palette);
	m_tc0180vcu->inth_callback().set_inputline(m_maincpu, M68K_IRQ_5, HOLD_LINE);
	m_tc0180vcu->intl_callback().set_inputline(m_maincpu, M68K_IRQ_3, HOLD_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2610_device &ymsnd(YM2610(config, m_ym, 16_MHz_XTAL / 2));  /* 8 MHz */
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


void hitice_state::hitice(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);   /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &hitice_state::main_map);

	Z80(config, m_audiocpu, 24_MHz_XTAL / 4);  /* 6 MHz Z80B */
	m_audiocpu->set_addrmap(AS_PROGRAM, &hitice_state::viofight_sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(hitice_state::player_12_coin_ctrl_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(hitice_state::screen_update_hitice));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 4096);

	TC0180VCU(config, m_tc0180vcu, 27.164_MHz_XTAL / 4); // nominally "6.6 MHZ"
	m_tc0180vcu->set_fb_colorbase(0x40);
	m_tc0180vcu->set_bg_colorbase(0xc0);
	m_tc0180vcu->set_fg_colorbase(0x80);
	m_tc0180vcu->set_tx_colorbase(0x00);
	m_tc0180vcu->set_palette(m_palette);
	m_tc0180vcu->inth_callback().set_inputline(m_maincpu, M68K_IRQ_4, HOLD_LINE);
	m_tc0180vcu->intl_callback().set_inputline(m_maincpu, M68K_IRQ_6, HOLD_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, m_ym, 24_MHz_XTAL / 8));  /* 3 MHz */
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.port_a_write_callback().set_membank(m_audiobank).mask(0x03);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 0.25);
	ymsnd.add_route(2, "mono", 0.25);
	ymsnd.add_route(3, "mono", 0.80);

	okim6295_device &oki(OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "mono", 0.50);

	pc060ha_device &ciu(PC060HA(config, "ciu", 0));
	ciu.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	ciu.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


void rambo3_state::rambo3p(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);   /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &rambo3_state::main_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 4); /* verified on pcb */
	m_audiocpu->set_addrmap(AS_PROGRAM, &rambo3_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(rambo3_state::player_12_coin_ctrl_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(rambo3_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 4096);

	TC0180VCU(config, m_tc0180vcu, 27.164_MHz_XTAL / 4);
	m_tc0180vcu->set_fb_colorbase(0x40);
	m_tc0180vcu->set_bg_colorbase(0xc0);
	m_tc0180vcu->set_fg_colorbase(0x80);
	m_tc0180vcu->set_tx_colorbase(0x00);
	m_tc0180vcu->set_palette(m_palette);
	m_tc0180vcu->inth_callback().set_inputline(m_maincpu, M68K_IRQ_6, HOLD_LINE);
	m_tc0180vcu->intl_callback().set_inputline(m_maincpu, M68K_IRQ_1, HOLD_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 16_MHz_XTAL / 2));   /* verified on pcb */
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


void rambo3_state::rambo3(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);   /* 12MHz verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &rambo3_state::main_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 4); /* 4MHz verified on pcb */
	m_audiocpu->set_addrmap(AS_PROGRAM, &rambo3_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(rambo3_state::player_12_coin_ctrl_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(rambo3_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 4096);

	TC0180VCU(config, m_tc0180vcu, 27.164_MHz_XTAL / 4);
	m_tc0180vcu->set_fb_colorbase(0x10);
	m_tc0180vcu->set_bg_colorbase(0x30);
	m_tc0180vcu->set_fg_colorbase(0x20);
	m_tc0180vcu->set_tx_colorbase(0x00);
	m_tc0180vcu->set_palette(m_palette);
	m_tc0180vcu->inth_callback().set_inputline(m_maincpu, M68K_IRQ_6, HOLD_LINE);
	m_tc0180vcu->intl_callback().set_inputline(m_maincpu, M68K_IRQ_1, HOLD_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 16_MHz_XTAL / 2));  /* 8 MHz verified on pcb */
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


void taitob_state::pbobble(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);   /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitob_state::pbobble_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 2);  /* 4 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &taitob_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	EEPROM_93C46_16BIT(config, "eeprom");

	TC0640FIO(config, m_tc0640fio, 0);
	m_tc0640fio->read_0_callback().set_ioport("SERVICE");
	m_tc0640fio->read_1_callback().set_ioport("COIN");
	m_tc0640fio->read_2_callback().set_ioport("START");
	m_tc0640fio->read_3_callback().set_ioport("P1_P2_A");
	m_tc0640fio->write_4_callback().set(FUNC(taitob_state::player_12_coin_ctrl_w));
	m_tc0640fio->read_7_callback().set_ioport("P1_P2_B");

	MB87078(config, m_mb87078);
	m_mb87078->gain_changed().set(FUNC(taitob_state::mb87078_gain_changed));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(taitob_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 4096);

	TC0180VCU(config, m_tc0180vcu, 27.164_MHz_XTAL / 4);
	m_tc0180vcu->set_fb_colorbase(0x80);
	m_tc0180vcu->set_bg_colorbase(0x00);
	m_tc0180vcu->set_fg_colorbase(0x40);
	m_tc0180vcu->set_tx_colorbase(0xc0);
	m_tc0180vcu->set_palette(m_palette);
	m_tc0180vcu->inth_callback().set_inputline(m_maincpu, M68K_IRQ_3, HOLD_LINE);
	m_tc0180vcu->intl_callback().set_inputline(m_maincpu, M68K_IRQ_5, HOLD_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2610b_device &ymsnd(YM2610B(config, "ymsnd", 16_MHz_XTAL / 2));  /* 8 MHz */
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


void taitob_state::spacedx(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);   /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitob_state::spacedx_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 4);  /* 4 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &taitob_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	EEPROM_93C46_16BIT(config, "eeprom");

	TC0640FIO(config, m_tc0640fio, 0);
	m_tc0640fio->read_0_callback().set_ioport("SERVICE");
	m_tc0640fio->read_1_callback().set_ioport("COIN");
	m_tc0640fio->read_2_callback().set_ioport("START");
	m_tc0640fio->read_3_callback().set_ioport("P1_P2_A");
	m_tc0640fio->write_4_callback().set(FUNC(taitob_state::player_12_coin_ctrl_w));
	m_tc0640fio->read_7_callback().set_ioport("P1_P2_B");

	MB87078(config, m_mb87078);
	m_mb87078->gain_changed().set(FUNC(taitob_state::mb87078_gain_changed));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(taitob_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 4096);

	TC0180VCU(config, m_tc0180vcu, 27.164_MHz_XTAL / 4);
	m_tc0180vcu->set_fb_colorbase(0x80);
	m_tc0180vcu->set_bg_colorbase(0x00);
	m_tc0180vcu->set_fg_colorbase(0x40);
	m_tc0180vcu->set_tx_colorbase(0xc0);
	m_tc0180vcu->set_palette(m_palette);
	m_tc0180vcu->inth_callback().set_inputline(m_maincpu, M68K_IRQ_3, HOLD_LINE);
	m_tc0180vcu->intl_callback().set_inputline(m_maincpu, M68K_IRQ_5, HOLD_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 16_MHz_XTAL / 2));  /* 8 MHz */
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


void taitob_state::spacedxo(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);   /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitob_state::spacedxo_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 4);  /* 4 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &taitob_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(taitob_state::player_12_coin_ctrl_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(taitob_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 4096);

	TC0180VCU(config, m_tc0180vcu, 27.164_MHz_XTAL / 4);
	m_tc0180vcu->set_fb_colorbase(0x10);
	m_tc0180vcu->set_bg_colorbase(0x30);
	m_tc0180vcu->set_fg_colorbase(0x20);
	m_tc0180vcu->set_tx_colorbase(0x00);
	m_tc0180vcu->set_palette(m_palette);
	m_tc0180vcu->inth_callback().set_inputline(m_maincpu, M68K_IRQ_6, HOLD_LINE);
	m_tc0180vcu->intl_callback().set_inputline(m_maincpu, M68K_IRQ_4, HOLD_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 16_MHz_XTAL / 2));  /* 8 MHz */
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


void taitob_state::qzshowby(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16000000);   /* 16 MHz according to the readme*/
	m_maincpu->set_addrmap(AS_PROGRAM, &taitob_state::qzshowby_map);

	Z80(config, m_audiocpu, 4000000);  /* 4 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &taitob_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	EEPROM_93C46_16BIT(config, "eeprom");

	TC0640FIO(config, m_tc0640fio, 0);
	m_tc0640fio->read_0_callback().set_ioport("SERVICE");
	m_tc0640fio->read_1_callback().set_ioport("COIN");
	m_tc0640fio->read_2_callback().set_ioport("START");
	m_tc0640fio->read_3_callback().set_ioport("P1_P2_A");
	m_tc0640fio->write_4_callback().set(FUNC(taitob_state::player_12_coin_ctrl_w));
	m_tc0640fio->read_7_callback().set_ioport("P1_P2_B");

	MB87078(config, m_mb87078);
	m_mb87078->gain_changed().set(FUNC(taitob_state::mb87078_gain_changed));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(taitob_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 4096);

	TC0180VCU(config, m_tc0180vcu, 27.164_MHz_XTAL / 4);
	m_tc0180vcu->set_fb_colorbase(0x80);
	m_tc0180vcu->set_bg_colorbase(0x00);
	m_tc0180vcu->set_fg_colorbase(0x40);
	m_tc0180vcu->set_tx_colorbase(0xc0);
	m_tc0180vcu->set_palette(m_palette);
	m_tc0180vcu->inth_callback().set_inputline(m_maincpu, M68K_IRQ_3, HOLD_LINE);
	m_tc0180vcu->intl_callback().set_inputline(m_maincpu, M68K_IRQ_5, HOLD_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2610b_device &ymsnd(YM2610B(config, "ymsnd", 8000000));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


void taitob_state::viofight(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);   /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitob_state::viofight_map);

	Z80(config, m_audiocpu, 24_MHz_XTAL / 4);  /* 6 MHz verified */
	m_audiocpu->set_addrmap(AS_PROGRAM, &taitob_state::viofight_sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(taitob_state::player_12_coin_ctrl_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(taitob_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 4096);

	TC0180VCU(config, m_tc0180vcu, 27.164_MHz_XTAL / 4);
	m_tc0180vcu->set_fb_colorbase(0x10);
	m_tc0180vcu->set_bg_colorbase(0x30);
	m_tc0180vcu->set_fg_colorbase(0x20);
	m_tc0180vcu->set_tx_colorbase(0x00);
	m_tc0180vcu->set_palette(m_palette);
	m_tc0180vcu->inth_callback().set_inputline(m_maincpu, M68K_IRQ_4, HOLD_LINE);
	m_tc0180vcu->intl_callback().set_inputline(m_maincpu, M68K_IRQ_1, HOLD_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, m_ym, 24_MHz_XTAL / 8));  /* 3 MHz */
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.port_a_write_callback().set_membank(m_audiobank).mask(0x03);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 0.25);
	ymsnd.add_route(2, "mono", 0.25);
	ymsnd.add_route(3, "mono", 0.80);

	okim6295_device &oki(OKIM6295(config, "oki", 4.224_MHz_XTAL / 4, okim6295_device::PIN7_HIGH)); // 1.056MHz clock frequency, but pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "mono", 0.50);

	pc060ha_device &ciu(PC060HA(config, "ciu", 0));
	ciu.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	ciu.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


void taitob_state::silentd(machine_config &config) /* ET910000B PCB */
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);   /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitob_state::silentd_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 4);  /* 4 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &taitob_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(taitob_state::player_12_coin_ctrl_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(taitob_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 4096);

	TC0180VCU(config, m_tc0180vcu, 27.164_MHz_XTAL / 4);
	m_tc0180vcu->set_fb_colorbase(0x10);
	m_tc0180vcu->set_bg_colorbase(0x30);
	m_tc0180vcu->set_fg_colorbase(0x20);
	m_tc0180vcu->set_tx_colorbase(0x00);
	m_tc0180vcu->set_palette(m_palette);
	m_tc0180vcu->inth_callback().set_inputline(m_maincpu, M68K_IRQ_6, HOLD_LINE);
	m_tc0180vcu->intl_callback().set_inputline(m_maincpu, M68K_IRQ_4, HOLD_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2610b_device &ymsnd(YM2610B(config, "ymsnd", 16_MHz_XTAL / 2));  /* 8 MHz */
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


void taitob_state::selfeena(machine_config &config) /* ET910000A PCB */
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);   /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitob_state::selfeena_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 4);  /* 4 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &taitob_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(taitob_state::player_12_coin_ctrl_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(taitob_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 4096);

	TC0180VCU(config, m_tc0180vcu, 27.164_MHz_XTAL / 4);
	m_tc0180vcu->set_fb_colorbase(0x10);
	m_tc0180vcu->set_bg_colorbase(0x30);
	m_tc0180vcu->set_fg_colorbase(0x20);
	m_tc0180vcu->set_tx_colorbase(0x00);
	m_tc0180vcu->set_palette(m_palette);
	m_tc0180vcu->inth_callback().set_inputline(m_maincpu, M68K_IRQ_6, HOLD_LINE);
	m_tc0180vcu->intl_callback().set_inputline(m_maincpu, M68K_IRQ_4, HOLD_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 16_MHz_XTAL / 2));  /* 8 MHz */
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}

#if 0
void taitob_state::ryujin_patch(void)
{
	uint16_t *rom = (uint16_t*)memregion("maincpu")->base();
	rom[ 0x62/2 ] = 1;
	//0 (already in rom) - Taito Corporation 1993
	//1 - Taito America corp with blue FBI logo
}
#endif


#if 0
void taitob_state::sbm_patch(void)
{
	uint16_t *rom = (uint16_t*)memregion("maincpu")->base();
	rom[ 0x7ffff/2 ] = 2; //US version
}
#endif

void taitob_state::sbm(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000);   /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitob_state::sbm_map);

	Z80(config, m_audiocpu, 4000000);  /* 4 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &taitob_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	TC0510NIO(config, m_tc0510nio, 0);
	m_tc0510nio->read_0_callback().set_ioport("DSWA");
	m_tc0510nio->read_1_callback().set_ioport("DSWB");
	m_tc0510nio->read_2_callback().set_ioport("JOY");
	m_tc0510nio->read_3_callback().set_ioport("START");
	m_tc0510nio->write_4_callback().set(FUNC(taitob_state::player_12_coin_ctrl_w));
	m_tc0510nio->read_7_callback().set_ioport("PHOTOSENSOR");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(taitob_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 4096);

	TC0180VCU(config, m_tc0180vcu, 27.164_MHz_XTAL / 4);
	m_tc0180vcu->set_fb_colorbase(0x40);
	m_tc0180vcu->set_bg_colorbase(0xc0);
	m_tc0180vcu->set_fg_colorbase(0x80);
	m_tc0180vcu->set_tx_colorbase(0x00);
	m_tc0180vcu->set_palette(m_palette);
	m_tc0180vcu->inth_callback().set_inputline(m_maincpu, M68K_IRQ_4, HOLD_LINE);
	m_tc0180vcu->intl_callback().set_inputline(m_maincpu, M68K_IRQ_5, HOLD_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2610b_device &ymsnd(YM2610B(config, "ymsnd", 8000000));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


void taitob_c_state::realpunc(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &taitob_c_state::main_map);

	Z80(config, m_audiocpu, 6000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &taitob_c_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	TC0510NIO(config, m_tc0510nio, 0);
	m_tc0510nio->read_0_callback().set_ioport("DSWA");
	m_tc0510nio->read_1_callback().set_ioport("DSWB");
	m_tc0510nio->read_2_callback().set_ioport("IN0");
	m_tc0510nio->read_3_callback().set_ioport("IN1");
	m_tc0510nio->write_4_callback().set(FUNC(taitob_c_state::player_12_coin_ctrl_w));
	m_tc0510nio->read_7_callback().set_ioport("IN2");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(taitob_c_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 4096);

	HD63484(config, m_hd63484, 0);
	m_hd63484->set_addrmap(0, &taitob_c_state::hd63484_map);
	m_hd63484->set_auto_configure_screen(false);

	TC0180VCU(config, m_tc0180vcu, 27.164_MHz_XTAL / 4);
	m_tc0180vcu->set_fb_colorbase(0x40);
	m_tc0180vcu->set_bg_colorbase(0xc0);
	m_tc0180vcu->set_fg_colorbase(0x80);
	m_tc0180vcu->set_tx_colorbase(0x00);
	m_tc0180vcu->set_palette(m_palette);
	m_tc0180vcu->inth_callback().set_inputline(m_maincpu, M68K_IRQ_2, HOLD_LINE);
	m_tc0180vcu->intl_callback().set_inputline(m_maincpu, M68K_IRQ_3, HOLD_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2610b_device &ymsnd(YM2610B(config, "ymsnd", 8000000));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 1.0);
	ymsnd.add_route(2, "mono", 1.0);

	tc0140syt_device &tc0140syt(TC0140SYT(config, "tc0140syt", 0));
	tc0140syt.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	tc0140syt.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( rastsag2 )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "b81-08.50", 0x00000, 0x20000, CRC(d6da9169) SHA1(33d74315754576e6f879059de033f96f9003f819) )
	ROM_LOAD16_BYTE( "b81-07.31", 0x00001, 0x20000, CRC(8edf17d7) SHA1(b0c03002ed520abffefd55d4969d0ed4fcf3a3a4) )
	ROM_LOAD16_BYTE( "b81-10.49", 0x40000, 0x20000, CRC(53f34344) SHA1(9930c3fd9c17f7d9b654221da3896d0ff5778c97) )
	ROM_LOAD16_BYTE( "b81-09.30", 0x40001, 0x20000, CRC(630d34af) SHA1(42452111b10f1d543e03661012dda879218dea62) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "b81-11.37", 0x00000, 0x10000, CRC(3704bf09) SHA1(a0a37b23e3df482c5f5aa43825345ea8affbba34) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "b81-03.14", 0x000000, 0x080000, CRC(551b75e6) SHA1(5b8388ee2c6262f359c9e6d04c951ea8dc3901c9) )
	ROM_LOAD( "b81-04.15", 0x080000, 0x080000, CRC(cf734e12) SHA1(4201a74468058761454515738fbf3a7b22a66e00) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "b81-02.2", 0x00000, 0x80000, CRC(20ec3b86) SHA1(fcdcc7f0a09feb824d8d73b1af0aae7ec30fd1ed) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "b81-01.1", 0x00000, 0x80000, CRC(b33f796b) SHA1(6cdb32f56283acdf20eb46a1e658e3bd7c97978c) )
ROM_END

ROM_START( nastarw )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "b81-08.50", 0x00000, 0x20000, CRC(d6da9169) SHA1(33d74315754576e6f879059de033f96f9003f819) )
	ROM_LOAD16_BYTE( "b81-12.31", 0x00001, 0x20000, CRC(f9d82741) SHA1(f5f3a1101d92b6c241e819dcdcdcdc4b125140f7) )
	ROM_LOAD16_BYTE( "b81-10.49", 0x40000, 0x20000, CRC(53f34344) SHA1(9930c3fd9c17f7d9b654221da3896d0ff5778c97) )
	ROM_LOAD16_BYTE( "b81-09.30", 0x40001, 0x20000, CRC(630d34af) SHA1(42452111b10f1d543e03661012dda879218dea62) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "b81-11.37", 0x00000, 0x10000, CRC(3704bf09) SHA1(a0a37b23e3df482c5f5aa43825345ea8affbba34) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "b81-03.14", 0x000000, 0x080000, CRC(551b75e6) SHA1(5b8388ee2c6262f359c9e6d04c951ea8dc3901c9) )
	ROM_LOAD( "b81-04.15", 0x080000, 0x080000, CRC(cf734e12) SHA1(4201a74468058761454515738fbf3a7b22a66e00) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "b81-02.2", 0x00000, 0x80000, CRC(20ec3b86) SHA1(fcdcc7f0a09feb824d8d73b1af0aae7ec30fd1ed) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "b81-01.1", 0x00000, 0x80000, CRC(b33f796b) SHA1(6cdb32f56283acdf20eb46a1e658e3bd7c97978c) )
ROM_END

ROM_START( nastar )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "b81-08.50", 0x00000, 0x20000, CRC(d6da9169) SHA1(33d74315754576e6f879059de033f96f9003f819) )
	ROM_LOAD16_BYTE( "b81-13.31", 0x00001, 0x20000, CRC(60d176fb) SHA1(fbe3a0603bcd23e565b0d474a63742d20a3ce8cc) )
	ROM_LOAD16_BYTE( "b81-10.49", 0x40000, 0x20000, CRC(53f34344) SHA1(9930c3fd9c17f7d9b654221da3896d0ff5778c97) )
	ROM_LOAD16_BYTE( "b81-09.30", 0x40001, 0x20000, CRC(630d34af) SHA1(42452111b10f1d543e03661012dda879218dea62) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "b81-11.37", 0x00000, 0x10000, CRC(3704bf09) SHA1(a0a37b23e3df482c5f5aa43825345ea8affbba34) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "b81-03.14", 0x000000, 0x080000, CRC(551b75e6) SHA1(5b8388ee2c6262f359c9e6d04c951ea8dc3901c9) )
	ROM_LOAD( "b81-04.15", 0x080000, 0x080000, CRC(cf734e12) SHA1(4201a74468058761454515738fbf3a7b22a66e00) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "b81-02.2", 0x00000, 0x80000, CRC(20ec3b86) SHA1(fcdcc7f0a09feb824d8d73b1af0aae7ec30fd1ed) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "b81-01.1", 0x00000, 0x80000, CRC(b33f796b) SHA1(6cdb32f56283acdf20eb46a1e658e3bd7c97978c) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ampal16l8-b81-05.21",  0x0000, 0x0104, CRC(922fd368) SHA1(6258c64a16c64de5c9c29e325a5e1f9695698b1f) )
	ROM_LOAD( "ampal16l8-b81-06a.22", 0x0200, 0x0104, CRC(bb1cec84) SHA1(fc7a8286687508b4e62b9754dba95f33336b8214) )
ROM_END

ROM_START( crimec )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "b99-07.40", 0x00000, 0x20000, CRC(26e886e6) SHA1(b7d0024a084216c1139eb6705f1b8143902cbed3) )
	ROM_LOAD16_BYTE( "b99-05.29", 0x00001, 0x20000, CRC(ff7f9a9d) SHA1(c1897a141eea423879d2792640a9ee85636ed5be) )
	ROM_LOAD16_BYTE( "b99-06.39", 0x40000, 0x20000, CRC(1f26aa92) SHA1(10ab253812db83204c136d01d865063a2210cb92) )
	ROM_LOAD16_BYTE( "b99-14.28", 0x40001, 0x20000, CRC(71c8b4d7) SHA1(55068c9cac75200f564b10f98f322e30aaa6849e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "b99-08.45", 0x00000, 0x10000, CRC(26135451) SHA1(651c77285eb12a13a2fe4069031c6f01150ecba4) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "b99-02.18", 0x000000, 0x080000, CRC(2a5d4a26) SHA1(94bdfca9365970a80a639027b195b71cebc5ab9c) )
	ROM_LOAD( "b99-01.19", 0x080000, 0x080000, CRC(a19e373a) SHA1(2208c9142473dc2218fd8b97fd6d0c861aeba011) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "b99-03.37", 0x000000, 0x080000, CRC(dda10df7) SHA1(ffbe1423794035e6f049fddb096b7282610b7cee) )
ROM_END

ROM_START( crimecu )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "b99-07.40", 0x00000, 0x20000, CRC(26e886e6) SHA1(b7d0024a084216c1139eb6705f1b8143902cbed3) )
	ROM_LOAD16_BYTE( "b99-05.29", 0x00001, 0x20000, CRC(ff7f9a9d) SHA1(c1897a141eea423879d2792640a9ee85636ed5be) )
	ROM_LOAD16_BYTE( "b99-06.39", 0x40000, 0x20000, CRC(1f26aa92) SHA1(10ab253812db83204c136d01d865063a2210cb92) )
	ROM_LOAD16_BYTE( "b99-13.28", 0x40001, 0x20000, CRC(06cf8441) SHA1(0395fd1f38366bd56b4b53e435dc7c3676d9e9bf) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "b99-08.45", 0x00000, 0x10000, CRC(26135451) SHA1(651c77285eb12a13a2fe4069031c6f01150ecba4) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "b99-02.18", 0x000000, 0x080000, CRC(2a5d4a26) SHA1(94bdfca9365970a80a639027b195b71cebc5ab9c) )
	ROM_LOAD( "b99-01.19", 0x080000, 0x080000, CRC(a19e373a) SHA1(2208c9142473dc2218fd8b97fd6d0c861aeba011) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "b99-03.37", 0x000000, 0x080000, CRC(dda10df7) SHA1(ffbe1423794035e6f049fddb096b7282610b7cee) )
ROM_END

ROM_START( crimecj )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "b99-07.40", 0x00000, 0x20000, CRC(26e886e6) SHA1(b7d0024a084216c1139eb6705f1b8143902cbed3) )
	ROM_LOAD16_BYTE( "b99-05.29", 0x00001, 0x20000, CRC(ff7f9a9d) SHA1(c1897a141eea423879d2792640a9ee85636ed5be) )
	ROM_LOAD16_BYTE( "b99-06.39", 0x40000, 0x20000, CRC(1f26aa92) SHA1(10ab253812db83204c136d01d865063a2210cb92) )
	ROM_LOAD16_BYTE( "b99-15.28", 0x40001, 0x20000, CRC(e8c1e56d) SHA1(ba0dc181db843e26676223f54cb121df738be987) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "b99-08.45", 0x00000, 0x10000, CRC(26135451) SHA1(651c77285eb12a13a2fe4069031c6f01150ecba4) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "b99-02.18", 0x000000, 0x080000, CRC(2a5d4a26) SHA1(94bdfca9365970a80a639027b195b71cebc5ab9c) )
	ROM_LOAD( "b99-01.19", 0x080000, 0x080000, CRC(a19e373a) SHA1(2208c9142473dc2218fd8b97fd6d0c861aeba011) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "b99-03.37", 0x000000, 0x080000, CRC(dda10df7) SHA1(ffbe1423794035e6f049fddb096b7282610b7cee) )
ROM_END

ROM_START( ashura )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c43-15.50", 0x00000, 0x20000, CRC(5d05d6c6) SHA1(43bcf6dbfa982595154ebb2b478415b63dfcb7aa) )
	ROM_LOAD16_BYTE( "c43-13.31", 0x00001, 0x20000, CRC(75b7d877) SHA1(54ffb35fdc84c86a3187291e2f1862a1a9152812) )
	ROM_LOAD16_BYTE( "c43-14.49", 0x40000, 0x20000, CRC(ede7f37d) SHA1(3ed744885f2aaba5c4e6f4d77ed33d12f0290968) )
	ROM_LOAD16_BYTE( "c43-12.30", 0x40001, 0x20000, CRC(b08a4ba0) SHA1(dad644bcaa240bf0d7393153ab0d0e9bf1d620b2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "c43-16",  0x00000, 0x10000, CRC(cb26fce1) SHA1(7cc60326455c7bb2ca543ddfd4002462cc079837) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "c43-02",  0x00000, 0x80000, CRC(105722ae) SHA1(1de5d396d2a4d5948544082c471a15ca1b8e756c) )
	ROM_LOAD( "c43-03",  0x80000, 0x80000, CRC(426606ba) SHA1(961ec0a9dc18044adae433337bfa89d951c5207c) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "c43-01",  0x00000, 0x80000, CRC(db953f37) SHA1(252591b676366d4828acb20c77aa9960ad9b367e) )
ROM_END

ROM_START( ashuraj )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c43-07-1.50", 0x00000, 0x20000, CRC(d5ceb20f) SHA1(59bc9468d7f43d3b1388c2915fafeba7e2cf13a5) )
	ROM_LOAD16_BYTE( "c43-05-1.31", 0x00001, 0x20000, CRC(a6f3bb37) SHA1(6959f3bcbcd2d5b13c95dcfd7a536541dcab49f8) )
	ROM_LOAD16_BYTE( "c43-06-1.49", 0x40000, 0x20000, CRC(0f331802) SHA1(e7ed01b0d664c4db6ea9acc54b57e674e10685aa) )
	ROM_LOAD16_BYTE( "c43-04-1.30", 0x40001, 0x20000, CRC(e06a2414) SHA1(77ebe1e61b6303f328757de2c90fae6588026414) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "c43-16",  0x00000, 0x10000, CRC(cb26fce1) SHA1(7cc60326455c7bb2ca543ddfd4002462cc079837) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "c43-02",  0x00000, 0x80000, CRC(105722ae) SHA1(1de5d396d2a4d5948544082c471a15ca1b8e756c) )
	ROM_LOAD( "c43-03",  0x80000, 0x80000, CRC(426606ba) SHA1(961ec0a9dc18044adae433337bfa89d951c5207c) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "c43-01",  0x00000, 0x80000, CRC(db953f37) SHA1(252591b676366d4828acb20c77aa9960ad9b367e) )
ROM_END

ROM_START( ashurau )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c43-11.50", 0x00000, 0x20000, CRC(d5aefc9b) SHA1(0fdfa166942ea70a77f956d16fd3b31225044c54) )
	ROM_LOAD16_BYTE( "c43-09.31", 0x00001, 0x20000, CRC(e91d0ab1) SHA1(992f63ebba538e624c695c1a868b14f4baa66ab9) )
	ROM_LOAD16_BYTE( "c43-10.49", 0x40000, 0x20000, CRC(c218e7ea) SHA1(d9b19ad26206238f8417efe3b80c020fea0dd573) )
	ROM_LOAD16_BYTE( "c43-08.30", 0x40001, 0x20000, CRC(5ef4f19f) SHA1(864e3a4a4e92adaa63249debad6292c528289fbe) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "c43-16",  0x00000, 0x10000, CRC(cb26fce1) SHA1(7cc60326455c7bb2ca543ddfd4002462cc079837) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "c43-02",  0x00000, 0x80000, CRC(105722ae) SHA1(1de5d396d2a4d5948544082c471a15ca1b8e756c) )
	ROM_LOAD( "c43-03",  0x80000, 0x80000, CRC(426606ba) SHA1(961ec0a9dc18044adae433337bfa89d951c5207c) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "c43-01",  0x00000, 0x80000, CRC(db953f37) SHA1(252591b676366d4828acb20c77aa9960ad9b367e) )
ROM_END

ROM_START( tetrist ) // Nastar / Nastar Warrior / Rastan Saga 2 conversion with graphics and sound ROMs left in place
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c12-03.50", 0x000000, 0x020000, CRC(38f1ed41) SHA1(d11194dd6686e6eba8c481bb0f9662041ca396ed) )
	ROM_LOAD16_BYTE( "c12-02.31", 0x000001, 0x020000, CRC(ed9530bc) SHA1(84c324e4ef0c5c3af04ea000ad3e9c319bd9f2a2) )
	ROM_LOAD16_BYTE( "c12-05.49", 0x040000, 0x020000, CRC(128e9927) SHA1(227b4a43074b66c9ba6f4497eb329fbcc5e3f52b) )
	ROM_LOAD16_BYTE( "c12-04.30", 0x040001, 0x020000, CRC(5da7a319) SHA1(0c903b3274f6eafe24c8b5ef476dc5e8e3131b20) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "c12-06.37", 0x00000, 0x10000, CRC(f2814b38) SHA1(846d3cc7a6f1cfbfd5661d6942f24330d21f91f0) )

	ROM_REGION( 0x100000, "tc0180vcu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100000, "gfx2", 0 )
	/* b81-03.14 & b81-04.15 are present on the original board and are actually from Nastar
	  the game doesn't use any tiles from here but the roms must be present on the board to avoid
	  tile 0 being solid and obscuring the bitmap (however if we load them in the correct region
	  unwanted tiles from here are shown after gameover which is wrong) - There is an undumped PAL
	      C12-01 that controls this effect.
	 */
	ROM_LOAD( "b81-03.14", 0x000000, 0x080000, CRC(551b75e6) SHA1(5b8388ee2c6262f359c9e6d04c951ea8dc3901c9) )
	ROM_LOAD( "b81-04.15", 0x080000, 0x080000, CRC(cf734e12) SHA1(4201a74468058761454515738fbf3a7b22a66e00) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "b81-02.2", 0x00000, 0x80000, CRC(20ec3b86) SHA1(fcdcc7f0a09feb824d8d73b1af0aae7ec30fd1ed) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "b81-01.1", 0x00000, 0x80000, CRC(b33f796b) SHA1(6cdb32f56283acdf20eb46a1e658e3bd7c97978c) )
ROM_END

ROM_START( tetrista ) // Master of Weapon conversion with graphics ROMs left in place
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c35-04.33", 0x000000, 0x020000, CRC(fa6e42ff) SHA1(1c586243aaf57b46338f22ae0fcdba2897ccb98a) )
	ROM_LOAD16_BYTE( "c35-03.24", 0x000001, 0x020000, CRC(aebd8539) SHA1(5230c0513581513ba971da55c04da8ba451a16e2) )
	ROM_LOAD16_BYTE( "c35-02.34", 0x040000, 0x020000, CRC(128e9927) SHA1(227b4a43074b66c9ba6f4497eb329fbcc5e3f52b) ) // ==c12-05
	ROM_LOAD16_BYTE( "c35-01.25", 0x040001, 0x020000, CRC(5da7a319) SHA1(0c903b3274f6eafe24c8b5ef476dc5e8e3131b20) ) // ==c12-04

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "c35-05.30", 0x00000, 0x10000, CRC(785c63fb) SHA1(13db76d8ce52ff21bfda0866c9c6b52147c6fc9d) )

	ROM_REGION( 0x100000, "tc0180vcu", ROMREGION_ERASE00 )

	ROM_REGION( 0x100000, "gfx2", 0 )
	/* b72-01.5 & b72-02.6 are present on the original board and are actually from Master of Weapon
	  the game doesn't use any tiles from here but the roms must be present on the board to avoid
	  tile 0 being solid and obscuring the bitmap (however if we load them in the correct region
	  unwanted tiles from here are shown after gameover which is wrong) - There is an undumped PAL
	      C35-06 that controls this effect.
	 */
	ROM_LOAD( "b72-02.6", 0x000000, 0x080000, CRC(843444eb) SHA1(2b466045f882996c80e0090009ee957e11d32825) )
	ROM_LOAD( "b72-01.5", 0x080000, 0x080000, CRC(a24ac26e) SHA1(895715a2bb0cb15334cba2283bd228b4fc08cd0c) )
ROM_END

ROM_START( hitice ) /* 4 Player version */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c59-10.42", 0x00000, 0x20000, CRC(e4ffad15) SHA1(87da85e1489fe57bd012177a70434152e5475009) )
	ROM_LOAD16_BYTE( "c59-12.64", 0x00001, 0x20000, CRC(a080d7af) SHA1(9c68b78fbcc42a2f748d1b7f84f138be79f7c0c9) )
	ROM_LOAD16_BYTE( "c59-09.41", 0x40000, 0x10000, CRC(e243e3b0) SHA1(a7daf96ef70e9a92bb3ee4a151ce674a187c15a2) )
	ROM_LOAD16_BYTE( "c59-11.63", 0x40001, 0x10000, CRC(4d4dfa52) SHA1(8ecd62dc2a2f35850340469afa54862b46053ce0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "c59-08.50",  0x00000, 0x10000, CRC(d3cbc10b) SHA1(75305e264300e0ebd15ada45a6d222fee75bd8e4) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "c59-03.12",  0x00000, 0x80000, CRC(9e513048) SHA1(4ffa63c01a25e912dd218b7b2deaf5ad1a53659a) )
	ROM_LOAD( "c59-02.13",  0x80000, 0x80000, CRC(affb5e07) SHA1(afe92268c78ab5565d2913672e25f3136a15f534) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "c59-01.30",  0x00000, 0x20000, CRC(46ae291d) SHA1(d36ab48cfa393a6a1eae5caa2a397087eb098f7f) )

	ROM_REGION( 0x800, "plds", 0 )
	ROM_LOAD( "pal20l8b-c59-04.25", 0x000, 0x144, CRC(2ebcf07c) SHA1(b73396fff8cde51e8a429843cd6dc3386f777f3b) )
	ROM_LOAD( "pal16l8b-c59-05.26", 0x200, 0x104, CRC(37b67c5c) SHA1(a4bf3532774bcd285a6e0e24a9e9a3b28684f724) )
	ROM_LOAD( "pal20l8b-c59-06.53", 0x400, 0x144, CRC(bf176875) SHA1(d7073ff7bf8f905dc8a6d3cf51543a572fa87f2f) )
	ROM_LOAD( "pal16r4b-c59-07.61", 0x600, 0x104, CRC(cf64bd95) SHA1(5acada8bd6da40b5342bdd7ec494ee0e615492f0) )
ROM_END

ROM_START( hiticerb ) /* 4 Player version */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c59-10.42", 0x00000, 0x20000, CRC(e4ffad15) SHA1(87da85e1489fe57bd012177a70434152e5475009) )
	ROM_LOAD16_BYTE( "c59-12.64", 0x00001, 0x20000, CRC(a080d7af) SHA1(9c68b78fbcc42a2f748d1b7f84f138be79f7c0c9) )
	ROM_LOAD16_BYTE( "c59-09.41", 0x40000, 0x10000, CRC(e243e3b0) SHA1(a7daf96ef70e9a92bb3ee4a151ce674a187c15a2) )
	ROM_LOAD16_BYTE( "c59-11.63", 0x40001, 0x10000, CRC(4d4dfa52) SHA1(8ecd62dc2a2f35850340469afa54862b46053ce0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "c59-08.50",  0x00000, 0x10000, CRC(d3cbc10b) SHA1(75305e264300e0ebd15ada45a6d222fee75bd8e4) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD16_BYTE( "ic5.bin",  0x00000, 0x20000, CRC(55698bbc) SHA1(ba0e0de12e3e52ef026d37d667060e8ba6925340) )
	ROM_LOAD16_BYTE( "ic1.bin",  0x40000, 0x20000, CRC(fca01c22) SHA1(94d7d521ea6fdab752939c1f2087b16a2b2922a8) )
	ROM_LOAD16_BYTE( "ic6.bin",  0x00001, 0x20000, CRC(a7cb863f) SHA1(f749d38c2164bb9833e653bc1253a336c0043daa) )
	ROM_LOAD16_BYTE( "ic2.bin",  0x40001, 0x20000, CRC(f1c09244) SHA1(68d67db5398c92699022f86fa53a1cbda344c375) )
	ROM_LOAD16_BYTE( "ic8.bin",  0x80000, 0x20000, CRC(7cd3aa29) SHA1(78279db96b4f913b60a866d4fcbde46e5608fe0d) )
	ROM_LOAD16_BYTE( "ic4.bin",  0xc0000, 0x20000, CRC(d0f2fd35) SHA1(e705e6d889a01bca6a33efcb36ef13a5f43bcd17) )
	ROM_LOAD16_BYTE( "ic7.bin",  0x80001, 0x20000, CRC(6b2979c7) SHA1(6154cee3796aea637114a771e4c773013b28b362) )
	ROM_LOAD16_BYTE( "ic3.bin",  0xc0001, 0x20000, CRC(51621004) SHA1(86980acdba4380f8682ace0e0f0543312304d451) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "ic30.bin",  0x00000, 0x20000, CRC(46ae291d) SHA1(d36ab48cfa393a6a1eae5caa2a397087eb098f7f) )

	ROM_REGION( 0x800, "plds", 0 )
	ROM_LOAD( "pal20l8b-c59-04.25", 0x000, 0x144, CRC(2ebcf07c) SHA1(b73396fff8cde51e8a429843cd6dc3386f777f3b) )
	ROM_LOAD( "pal16l8b-c59-05.26", 0x200, 0x104, CRC(37b67c5c) SHA1(a4bf3532774bcd285a6e0e24a9e9a3b28684f724) )
	ROM_LOAD( "pal20l8b-c59-06.53", 0x400, 0x144, CRC(bf176875) SHA1(d7073ff7bf8f905dc8a6d3cf51543a572fa87f2f) )
	ROM_LOAD( "pal16r4b-c59-07.61", 0x600, 0x104, CRC(cf64bd95) SHA1(5acada8bd6da40b5342bdd7ec494ee0e615492f0) )
ROM_END

ROM_START( hiticej ) /* 2 Player version */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c59-23.42", 0x00000, 0x20000, CRC(01958fcc) SHA1(adaf9b0a4658d4d8eb8cdd343b40643b4c05d09e) )
	ROM_LOAD16_BYTE( "c59-25.64", 0x00001, 0x20000, CRC(71984c76) SHA1(2e8bbfd01b0f229db5f10563a0864e8a2d1a515f) )
	ROM_LOAD16_BYTE( "c59-22.41", 0x40000, 0x10000, CRC(c2c86140) SHA1(8d285a50786c91d28004a30854bbc40cf7d0de4b) )
	ROM_LOAD16_BYTE( "c59-24.63", 0x40001, 0x10000, CRC(26c8f409) SHA1(c10f5504e4521c64e2410cef504ff7079c7d4e70) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "c59-08.50",  0x00000, 0x10000, CRC(d3cbc10b) SHA1(75305e264300e0ebd15ada45a6d222fee75bd8e4) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "c59-03.12",  0x00000, 0x80000, CRC(9e513048) SHA1(4ffa63c01a25e912dd218b7b2deaf5ad1a53659a) )
	ROM_LOAD( "c59-02.13",  0x80000, 0x80000, CRC(affb5e07) SHA1(afe92268c78ab5565d2913672e25f3136a15f534) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "c59-01.30",  0x00000, 0x20000, CRC(46ae291d) SHA1(d36ab48cfa393a6a1eae5caa2a397087eb098f7f) )

	ROM_REGION( 0x800, "plds", 0 )
	ROM_LOAD( "pal20l8b-c59-04.25", 0x000, 0x144, CRC(2ebcf07c) SHA1(b73396fff8cde51e8a429843cd6dc3386f777f3b) )
	ROM_LOAD( "pal16l8b-c59-05.26", 0x200, 0x104, CRC(37b67c5c) SHA1(a4bf3532774bcd285a6e0e24a9e9a3b28684f724) )
	ROM_LOAD( "pal20l8b-c59-06.53", 0x400, 0x144, CRC(bf176875) SHA1(d7073ff7bf8f905dc8a6d3cf51543a572fa87f2f) )
	ROM_LOAD( "pal16r4b-c59-07.61", 0x600, 0x104, CRC(cf64bd95) SHA1(5acada8bd6da40b5342bdd7ec494ee0e615492f0) )
ROM_END

ROM_START( rambo3 )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "b93-11.rom0e.ic9",  0x00000, 0x20000, CRC(1cc42247) SHA1(e5a226a0016ec329fc23046c426c6303e452ef1d) )
	ROM_LOAD16_BYTE( "b93-14.rom0o.ic23", 0x00001, 0x20000, CRC(7d917c21) SHA1(2850c46d6bdabfb76c40a7dc78ebc14b69ce95c9) )
	ROM_LOAD16_BYTE( "b93-07.rom1e.ic8",  0x40000, 0x20000, CRC(c973ff6f) SHA1(d11f289f8559602783d97b831182e8c37954980f) )
	ROM_LOAD16_BYTE( "b93-06-rom1o.ic22", 0x40001, 0x20000, CRC(a83d3fd5) SHA1(4f07d969af3b25e4ab8d0f2c03e0c0c8ada02991) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "b93-10.ic36", 0x00000, 0x10000, CRC(b18bc020) SHA1(5f3a108ad1f652103dda14672223529077a0924b) )

	ROM_REGION( 0x200000, "tc0180vcu", 0 )
	ROM_LOAD( "b93-03.ic3",  0x000000, 0x80000, CRC(f5808c41) SHA1(73e129e87d7e240f96cb06d484fb19cd0ef49721) )
	ROM_LOAD( "b93-04.ic4",  0x080000, 0x80000, CRC(c57831ce) SHA1(85c203a858df34fe8663b2b16447d328cdb4145e) )
	ROM_LOAD( "b93-02.ic2",  0x100000, 0x80000, CRC(c55fcf54) SHA1(6a26ed2541be9e3341f20e74cc49b5366ce7d424) )
	ROM_LOAD( "b93-01.ic1",  0x180000, 0x80000, CRC(9dd014c6) SHA1(0f046d9de57db0272810adde7d49cc348b78f1f7) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "b93-05.roma.ic46", 0x00000, 0x80000, CRC(0179dc40) SHA1(89feb708618ae7fa96883473d5c7a09dcc6f452a) )

	ROM_REGION( 0x400, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "b93-08-pal16l8b.ic11", 0x000, 0x104, CRC(6b1e5259) SHA1(47fd718680e7feadb20c45fbad0c3cb572d2937f) )
	ROM_LOAD( "b93-09-pal16l8b.ic25", 0x200, 0x104, CRC(e63d9041) SHA1(4dcd6949825615b0c1370bea50e709da1b265a6c) )
ROM_END

ROM_START( rambo3u )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "ramb3-11.bin",  0x00000, 0x20000, CRC(1cc42247) SHA1(e5a226a0016ec329fc23046c426c6303e452ef1d) )
	ROM_LOAD16_BYTE( "ramb3-13.bin",  0x00001, 0x20000, CRC(0a964cb7) SHA1(332fe23c33b1400c628e0c491f3e00820bde6696) )
	ROM_LOAD16_BYTE( "ramb3-07.bin",  0x40000, 0x20000, CRC(c973ff6f) SHA1(d11f289f8559602783d97b831182e8c37954980f) )
	ROM_LOAD16_BYTE( "ramb3-06.bin",  0x40001, 0x20000, CRC(a83d3fd5) SHA1(4f07d969af3b25e4ab8d0f2c03e0c0c8ada02991) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "ramb3-10.bin", 0x00000, 0x10000, CRC(b18bc020) SHA1(5f3a108ad1f652103dda14672223529077a0924b) )

	ROM_REGION( 0x200000, "tc0180vcu", 0 )
	ROM_LOAD( "ramb3-03.bin",  0x000000, 0x80000, CRC(f5808c41) SHA1(73e129e87d7e240f96cb06d484fb19cd0ef49721) )
	ROM_LOAD( "ramb3-04.bin",  0x080000, 0x80000, CRC(c57831ce) SHA1(85c203a858df34fe8663b2b16447d328cdb4145e) )
	ROM_LOAD( "ramb3-01.bin",  0x100000, 0x80000, CRC(c55fcf54) SHA1(6a26ed2541be9e3341f20e74cc49b5366ce7d424) )
	ROM_LOAD( "ramb3-02.bin",  0x180000, 0x80000, CRC(9dd014c6) SHA1(0f046d9de57db0272810adde7d49cc348b78f1f7) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "ramb3-05.bin", 0x00000, 0x80000, CRC(0179dc40) SHA1(89feb708618ae7fa96883473d5c7a09dcc6f452a) )
ROM_END

ROM_START( rambo3p ) /* Is this set a prototype or possible bootleg? */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "r3-0e.rom",  0x00000, 0x10000, CRC(3efa4177) SHA1(5e4995e34b92f625f7825238dfbc9e76d4090871) )
	ROM_LOAD16_BYTE( "r3-0o.rom",  0x00001, 0x10000, CRC(55c38d92) SHA1(4f712b4eb20ee176da83a5f1154d5890d1360398) )

	/*NOTE: there is a hole in address space here */

	ROM_LOAD16_BYTE( "r3-1e.rom" , 0x40000, 0x20000, CRC(40e363c7) SHA1(9907def4736fbff15cf769a762bf1811f890d1c7) )
	ROM_LOAD16_BYTE( "r3-1o.rom" , 0x40001, 0x20000, CRC(7f1fe6ab) SHA1(31231747982d9c42f693f650bc137794b438c2b7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "r3-00.rom", 0x00000, 0x10000, CRC(df7a6ed6) SHA1(68f7f17c9ead6aee653b02de234ec54b519907d7) )

	ROM_REGION( 0x200000, "tc0180vcu", 0 )
	ROM_LOAD16_BYTE( "r3-ch1ll.rom", 0x000000, 0x020000, CRC(c86ea5fc) SHA1(daf89340bb5d6ae57ef6faedb3f86c944c68fc45) )
	ROM_LOAD16_BYTE( "r3-ch1hl.rom", 0x040000, 0x020000, CRC(7525eb92) SHA1(f691a000580d078f207ea6e9bc8a527e74bc20e5) )
	ROM_LOAD16_BYTE( "r3-ch3ll.rom", 0x080000, 0x020000, CRC(abe54b1e) SHA1(4b6d0c694d9c1fcdec87b70fbadee9fb3455b42c) )
	ROM_LOAD16_BYTE( "r3-ch3hl.rom", 0x0c0000, 0x020000, CRC(80e5647e) SHA1(744e9dcc651b80ef4b3cc29ed0ebb223bb65f106) )

	ROM_LOAD16_BYTE( "r3-ch1lh.rom", 0x000001, 0x020000, CRC(75568cf0) SHA1(78cb940fafb6e01a018d6823636b398a898e988a) )
	ROM_LOAD16_BYTE( "r3-ch1hh.rom", 0x040001, 0x020000, CRC(e39cff37) SHA1(79680526759013f8641e82c27b3afc184c06f059) )
	ROM_LOAD16_BYTE( "r3-ch3lh.rom", 0x080001, 0x020000, CRC(5a155c04) SHA1(9472ef6474275ac5993c2afb222cfbc8d848bd86) )
	ROM_LOAD16_BYTE( "r3-ch3hh.rom", 0x0c0001, 0x020000, CRC(abe58fdb) SHA1(6429caa8680255c46457ab0ad6c0f610291d8a92) )

	ROM_LOAD16_BYTE( "r3-ch0ll.rom", 0x100000, 0x020000, CRC(b416f1bf) SHA1(8ae546c8286d616a991766eb97c3d281dbafd944) )
	ROM_LOAD16_BYTE( "r3-ch0hl.rom", 0x140000, 0x020000, CRC(a4cad36d) SHA1(f8327bcc490cb66703de9bbcd931d964609ae822) )
	ROM_LOAD16_BYTE( "r3-ch2ll.rom", 0x180000, 0x020000, CRC(d0ce3051) SHA1(0dc57f53b82d8ae15106df8a08b404c076b9fcef) )
	ROM_LOAD16_BYTE( "r3-ch2hl.rom", 0x1c0000, 0x020000, CRC(837d8677) SHA1(6a3b36399e89c8ff031b25bcf62704197d711a5b) )

	ROM_LOAD16_BYTE( "r3-ch0lh.rom", 0x100001, 0x020000, CRC(76a330a2) SHA1(4324bceca7bf5155f7b2543748989e2d364a7e97) )
	ROM_LOAD16_BYTE( "r3-ch0hh.rom", 0x140001, 0x020000, CRC(4dc69751) SHA1(2abf657dc951dc263b53a2c7a4ed2f4aaf5f9a98) )
	ROM_LOAD16_BYTE( "r3-ch2lh.rom", 0x180001, 0x020000, CRC(df3bc48f) SHA1(6747a453da4bca0b837f4ef1f1bbe871f15332ed) )
	ROM_LOAD16_BYTE( "r3-ch2hh.rom", 0x1c0001, 0x020000, CRC(bf37dfac) SHA1(27e825bd0a4d7ae65714fefeb6fedac501984ba9) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "r3-a1.rom", 0x00000, 0x20000, CRC(4396fa19) SHA1(cb6d983f210249676c500723041d74fa3fdc517d) )
	ROM_LOAD( "r3-a2.rom", 0x20000, 0x20000, CRC(41fe53a8) SHA1(1723046111d0115d3f64c3111c50d51306e88ad0) )
	ROM_LOAD( "r3-a3.rom", 0x40000, 0x20000, CRC(e89249ba) SHA1(cd94492a0643e9e1e25b121160914822a6a7723e) )
	ROM_LOAD( "r3-a4.rom", 0x60000, 0x20000, CRC(9cf4c21b) SHA1(756fc6bbc798a39a18eab3829e032cac8fe3f8ed) )
ROM_END

ROM_START( pbobble )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "pb-ic18.ic18", 0x00000, 0x40000, CRC(5de14f49) SHA1(91d537748f26e19a5c32de4b8dad341750de39ef) )
	ROM_LOAD16_BYTE( "pb-ic2.ic2",   0x00001, 0x40000, CRC(2abe07d1) SHA1(2bb78b606a7341d74cced0447f8bd5ccc635cc4c) )

	ROM_REGION( 0x20000, "audiocpu", 0 )     /* 128k for Z80 code */
	ROM_LOAD( "pb-ic27.ic27", 0x00000, 0x20000, CRC(26efa4c4) SHA1(795af8f6d23c2cbe2c811ec9ab1f14a4eee3f99e) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "pb-ic14.ic14", 0x00000, 0x80000, CRC(55f90ea4) SHA1(793c79e5b72171124368ad09dd31235252c541f5) )
	ROM_LOAD( "pb-ic9.ic9",   0x80000, 0x80000, CRC(3253aac9) SHA1(916d85aa96e2914630833292a0655b0389b4a39b) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "pb-ic15.ic15", 0x000000, 0x100000, CRC(0840cbc4) SHA1(1adbd7aef44fa80832f63dfb8efdf69fd7256a57) )
ROM_END

ROM_START( bublbust ) // Purportedly a location test version, but has the same version number and build date as the released Japanese set below
	ROM_REGION( 0x80000, "maincpu", 0 ) // 512k for 68000 code - located on Taito C0503005B main board
	ROM_LOAD16_BYTE( "p.bobble_prg_h_kyotsu_6-15_c681.ic18", 0x00000, 0x40000, CRC(ba83e398) SHA1(2512ea5a805f0e5c470c4d216d679f3dc3c4aa82) ) // hand written label P.ボブル PRG H 共通 6/15 C681
	ROM_LOAD16_BYTE( "p.bobble_prg_l_usa_6-15_e9e1.ic2",     0x00001, 0x40000, CRC(a12cb3f2) SHA1(06271d3a0af101a959e6e777f1f9e99bae1e6076) ) // hand written label P.ボブル PRG L USA 6/15 E9E1

	ROM_REGION( 0x20000, "audiocpu", 0 ) // 128k for Z80 code - located on Taito C0503005B main board
	ROM_LOAD( "prg_snd.ic27", 0x00000, 0x20000, CRC(2f288fe0) SHA1(4ba707f4b3a1ca0e573652d1c733ee889f9fef8a) ) // hand written label PRG SND IC 27 - no checksum listed

	ROM_REGION( 0x100000, "tc0180vcu", 0 ) // located on Taito C0503002A ROM PCB
	ROM_LOAD16_BYTE( "p.bobble_chr-1l_6-14_eaa0.ic11_ch_1_0l", 0x00000, 0x40000, CRC(8d3fa2f0) SHA1(3d36944e35081740469df61a40e538eb94ea2ef6) ) // hand written label P.ボブル CHR 1L 6/14 EAA0
	ROM_LOAD16_BYTE( "p.bobble_chr-1h_6-14_1515.ic12_ch_1_0h", 0x00001, 0x40000, CRC(a2eb4d32) SHA1(571811a543af50cc5e0f1d7aad9fc388919e93e6) ) // hand written label P.ボブル CHR 1H 6/14 1515
	ROM_LOAD16_BYTE( "p.bobble_chr-0l_6-14_86d5.ic7_ch_0_0l",  0x80000, 0x40000, CRC(80f1aab0) SHA1(f5cdb93aa702fb8ba91eaf8f470b6e2a61a581b5) ) // hand written label P.ボブル CHR 0L 6/14 86D5
	ROM_LOAD16_BYTE( "p.bobble_chr-0h_6-14_d180.ic8_ch_0_0h",  0x80001, 0x40000, CRC(d773cac8) SHA1(b008a21c39650a28cba402cd1c4de0567e275bc9) ) // hand written label P.ボブル CHR 0H 6/14 D180 (label torn, checksum illegible besides final 0)

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // located on Taito C0503002A ROM PCB - Half the size of the Japanese ADPCMA sample ROM
	ROM_LOAD( "cr40-kaigai-7bec.ic15_ach_0", 0x00000, 0x80000, CRC(f203ae52) SHA1(d41b880ef17eea6614e2911318db5cd070473381) ) // hand written label CR40 海外 7BEC
ROM_END

ROM_START( spacedx )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "d89-06.ic18", 0x00000, 0x40000, CRC(7122751e) SHA1(4b4eb58af28f1988ff102251407449d0affbd4c2) )
	ROM_LOAD16_BYTE( "d89-xx.ic2",  0x00001, 0x40000, CRC(56b0be6c) SHA1(37e3e28a23fb4af35bdf7c751e4c3a743e505c46) ) // need to verify / correct ROM label

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "d89-07.ic27", 0x00000, 0x10000, CRC(bd743401) SHA1(bf1ff2255bbd79be21855814d52daced71fbe198) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "d89-02.ic14", 0x00000, 0x80000, CRC(c36544b9) SHA1(6bd5257dfb27532621b75f43e31aa351ad2192a2) )
	ROM_LOAD( "d89-01.ic9",  0x80000, 0x80000, CRC(fffa0660) SHA1(de1abe1b1e9d14405b5663103ea4a6119fce7cc5) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "d89-03.ic15", 0x00000, 0x80000, CRC(218f31a4) SHA1(9f52b9fa8f02003888180524a6e9ee7c9230f55d) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "pal16l8-d72-05.ic37",   0x0000, 0x0104, CRC(c3d4cb7e) SHA1(1f3453a543dd98d02183595d66c67773fbf0ed07) ) /* Matches D72-05 in qzshowby */
	ROM_LOAD( "pal16l8-d72-06.ic50",   0x0200, 0x0104, CRC(e96b7f37) SHA1(568087d0ab0ed55814deccc11630d3e26f765450) ) /* Differs from D72-06 in qzshowby */
	ROM_LOAD( "palce20v8-d72-07.ic28", 0x0400, 0x0157, CRC(6359e64c) SHA1(83786f047aef591eb147a16a282f5312b36bc489) ) /* Matches D72-07 in qzshowby */
	ROM_LOAD( "palce20v8-d72-09.ic47", 0x0600, 0x0157, CRC(de1760fd) SHA1(332156699408e5b0a698f031c01f8aa85c3d5d32) ) /* Differs from D72-09 in qzshowby */
	ROM_LOAD( "palce16v8-d72-10.ic12", 0x0800, 0x0117, CRC(a5181ba2) SHA1(8315d6efa26be2ed98d4c0b39a196033789ab947) ) /* Matches D72-10 in qzshowby */
	ROM_LOAD( "pal20l8b-d89-04.ic40",  0x0a00, 0x0144, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( spacedxj )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "d89-06.ic18", 0x00000, 0x40000, CRC(7122751e) SHA1(4b4eb58af28f1988ff102251407449d0affbd4c2) )
	ROM_LOAD16_BYTE( "d89-05.ic2",  0x00001, 0x40000, CRC(be1638af) SHA1(5d28af674dd355159602e323800419a33e0b77d2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "d89-07.ic27", 0x00000, 0x10000, CRC(bd743401) SHA1(bf1ff2255bbd79be21855814d52daced71fbe198) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "d89-02.ic14", 0x00000, 0x80000, CRC(c36544b9) SHA1(6bd5257dfb27532621b75f43e31aa351ad2192a2) )
	ROM_LOAD( "d89-01.ic9" , 0x80000, 0x80000, CRC(fffa0660) SHA1(de1abe1b1e9d14405b5663103ea4a6119fce7cc5) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "d89-03.ic15", 0x00000, 0x80000, CRC(218f31a4) SHA1(9f52b9fa8f02003888180524a6e9ee7c9230f55d) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "pal16l8-d72-05.ic37",   0x0000, 0x0104, CRC(c3d4cb7e) SHA1(1f3453a543dd98d02183595d66c67773fbf0ed07) ) /* Matches D72-05 in qzshowby */
	ROM_LOAD( "pal16l8-d72-06.ic50",   0x0200, 0x0104, CRC(e96b7f37) SHA1(568087d0ab0ed55814deccc11630d3e26f765450) ) /* Differs from D72-06 in qzshowby */
	ROM_LOAD( "palce20v8-d72-07.ic28", 0x0400, 0x0157, CRC(6359e64c) SHA1(83786f047aef591eb147a16a282f5312b36bc489) ) /* Matches D72-07 in qzshowby */
	ROM_LOAD( "palce20v8-d72-09.ic47", 0x0600, 0x0157, CRC(de1760fd) SHA1(332156699408e5b0a698f031c01f8aa85c3d5d32) ) /* Differs from D72-09 in qzshowby */
	ROM_LOAD( "palce16v8-d72-10.ic12", 0x0800, 0x0117, CRC(a5181ba2) SHA1(8315d6efa26be2ed98d4c0b39a196033789ab947) ) /* Matches D72-10 in qzshowby */
	ROM_LOAD( "pal20l8b-d89-04.ic40",  0x0a00, 0x0144, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( spacedxo ) // different PCB type, similar to Silent Dragon
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "d89-08.bin",  0x00000, 0x20000, CRC(0c2fe7f9) SHA1(a0773c059c8ff2c9e367e0fb460d7e5f9a762ab1) )
	ROM_LOAD16_BYTE( "d89-09.bin",  0x00001, 0x20000, CRC(7f0a0ba4) SHA1(479df027929201997aeebbea5901a0a494f2dd61) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "d89-07.ic27", 0x00000, 0x10000, CRC(bd743401) SHA1(bf1ff2255bbd79be21855814d52daced71fbe198) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "d89-12.bin",0x00000, 0x80000, CRC(53df86f1) SHA1(f03d77dd54eb455462133a29dd8fec007abedcfd) )
	ROM_LOAD( "d89-13.bin",0x80000, 0x80000, CRC(c44c1352) SHA1(78a04fe0ade6e8f9e6bbda7652a54a79b6208fdd) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "d89-03.ic15", 0x00000, 0x80000, CRC(218f31a4) SHA1(9f52b9fa8f02003888180524a6e9ee7c9230f55d) )
ROM_END

ROM_START( qzshowby )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1M for 68000 code */
	ROM_LOAD16_BYTE( "d72-13.bin", 0x00000, 0x80000, CRC(a867759f) SHA1(ab06f42d58ae099fe5c1f810786c2a3e5a667e8e) )
	ROM_LOAD16_BYTE( "d72-12.bin", 0x00001, 0x80000, CRC(522c09a7) SHA1(2ceeb7ac24bb621630cc996381e57501f9ea672e) )

	ROM_REGION( 0x20000, "audiocpu", 0 )     /* 128k for Z80 code */
	ROM_LOAD( "d72-11.bin", 0x00000, 0x20000, CRC(2ca046e2) SHA1(983620e657d729e1441d509f18141bb3bb581855) )

	ROM_REGION( 0x400000, "tc0180vcu", 0 )
	ROM_LOAD( "d72-03.bin", 0x000000, 0x200000, CRC(1de257d0) SHA1(df03b1fb5cd69e2d2eb2088f96f26b0ea9756fb7) )
	ROM_LOAD( "d72-02.bin", 0x200000, 0x200000, CRC(bf0da640) SHA1(2b2493904ed0b94dc12b56dae71cc5c25701aef9) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "d72-01.bin", 0x00000, 0x200000, CRC(b82b8830) SHA1(4b2dca16fe072a5ee51de5cf40637e3f1b39f695) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "pal16l8-d72-05.bin",   0x0000, 0x0104, CRC(c3d4cb7e) SHA1(1f3453a543dd98d02183595d66c67773fbf0ed07) )
	ROM_LOAD( "pal16l8-d72-06.bin",   0x0200, 0x0104, CRC(27580efc) SHA1(11b3c0e2b344926dd068672a952574f06989d30a) ) /* This one or spacedx's D72-06 is a bad dump, should match */
	ROM_LOAD( "palce20v8-d72-07.bin", 0x0400, 0x0157, CRC(6359e64c) SHA1(83786f047aef591eb147a16a282f5312b36bc489) )
	ROM_LOAD( "palce20v8-d72-08.bin", 0x0600, 0x0157, CRC(746a6474) SHA1(f6c45ff53a01c03b1fc622dc161843b5faf0d2e4) )
	ROM_LOAD( "palce20v8-d72-09.bin", 0x0800, 0x0157, CRC(9f680800) SHA1(2fa41ead85136e851d465432a7b9d3ec848c7a22) ) /* This one or spacedx's D72-09 is a bad dump, should match */
	ROM_LOAD( "palce16v8-d72-10.bin", 0x0a00, 0x0117, CRC(a5181ba2) SHA1(8315d6efa26be2ed98d4c0b39a196033789ab947) )
ROM_END

ROM_START( viofight )
	ROM_REGION( 0x080000, "maincpu", 0 )     /* 1M for 68000 code */
	ROM_LOAD16_BYTE( "c16-11.42", 0x00000, 0x10000, CRC(23dbd388) SHA1(488f928826d16b201dcc4b491b09955d0af91f19) )
	ROM_LOAD16_BYTE( "c16-14.23", 0x00001, 0x10000, CRC(dc934f6a) SHA1(36d7b10478f2b97d0521edb84f1f4fa5a11f962b) )
	ROM_LOAD16_BYTE( "c16-07.41", 0x40000, 0x20000, CRC(64d1d059) SHA1(643ac7fa5076147b24810a8e1b925dfe09f75864) )
	ROM_LOAD16_BYTE( "c16-06.22", 0x40001, 0x20000, CRC(043761d8) SHA1(4587cadd73b628b4b9ac5c537cec20f90fb4959d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 128k for Z80 code */
	ROM_LOAD( "c16-12.32", 0x00000, 0x10000, CRC(6fb028c7) SHA1(a808d82e872914f994652e95dca3fcad00ba02fc) )

	ROM_REGION( 0x200000, "tc0180vcu", 0 )
	// ROM dumps are confirmed good. Some bad GFX (MAME gfxdecode: 0000,0001, 1000,1001, 2000,2001, 3000,3001).
	// See 3rd stage tiger fight background for example, see MT8048 for more information.
	ROM_LOAD( "c16-01.1", 0x000000, 0x080000, CRC(7059ce83) SHA1(1e6825ab944254cd4ba6574762172245b3352319) )
	ROM_LOAD( "c16-02.2", 0x080000, 0x080000, CRC(b458e905) SHA1(b712cbf4a4015e1fc2243871fe753e230f0172c2) )
	ROM_LOAD( "c16-03.3", 0x100000, 0x080000, CRC(515a9431) SHA1(836be28614326d093be8841617cca83cef8d55cc) )
	ROM_LOAD( "c16-04.4", 0x180000, 0x080000, CRC(ebf285e2) SHA1(0f806e42778e28e9687d85b2601ee08dd786869b) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "c16-05.47", 0x000000, 0x80000, CRC(a49d064a) SHA1(f9ed675cfaae69b68c99c7dce7c2a457b5b5c293) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16l8b-c16-08.ic34", 0x0000, 0x0104, CRC(9456d278) SHA1(cd5986f260aac0ae1df1cea6dbd1dbee82536105) )
	ROM_LOAD( "pal16l8b-c16-09.ic35", 0x0200, 0x0104, CRC(0965baab) SHA1(3f704cd42d5277e9ba0b0680a2722488fc4cc630) )
ROM_END

ROM_START( viofightu )
	ROM_REGION( 0x080000, "maincpu", 0 )     /* 1M for 68000 code */
	ROM_LOAD16_BYTE( "c16-11.42", 0x00000, 0x10000, CRC(23dbd388) SHA1(488f928826d16b201dcc4b491b09955d0af91f19) )
	ROM_LOAD16_BYTE( "c16-13.23", 0x00001, 0x10000, CRC(ab947ffc) SHA1(103023cee4b20afa5086ba60522ea5aa723aebef) )
	ROM_LOAD16_BYTE( "c16-07.41", 0x40000, 0x20000, CRC(64d1d059) SHA1(643ac7fa5076147b24810a8e1b925dfe09f75864) )
	ROM_LOAD16_BYTE( "c16-06.22", 0x40001, 0x20000, CRC(043761d8) SHA1(4587cadd73b628b4b9ac5c537cec20f90fb4959d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 128k for Z80 code */
	ROM_LOAD( "c16-12.32", 0x00000, 0x10000, CRC(6fb028c7) SHA1(a808d82e872914f994652e95dca3fcad00ba02fc) )

	ROM_REGION( 0x200000, "tc0180vcu", 0 )
	ROM_LOAD( "c16-01.1", 0x000000, 0x080000, CRC(7059ce83) SHA1(1e6825ab944254cd4ba6574762172245b3352319) )
	ROM_LOAD( "c16-02.2", 0x080000, 0x080000, CRC(b458e905) SHA1(b712cbf4a4015e1fc2243871fe753e230f0172c2) )
	ROM_LOAD( "c16-03.3", 0x100000, 0x080000, CRC(515a9431) SHA1(836be28614326d093be8841617cca83cef8d55cc) )
	ROM_LOAD( "c16-04.4", 0x180000, 0x080000, CRC(ebf285e2) SHA1(0f806e42778e28e9687d85b2601ee08dd786869b) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "c16-05.47", 0x000000, 0x80000, CRC(a49d064a) SHA1(f9ed675cfaae69b68c99c7dce7c2a457b5b5c293) )

	ROM_REGION( 0x00400, "plds", 0 )
	ROM_LOAD( "pal16l8b-c16-08.ic34", 0x0000, 0x0104, CRC(9456d278) SHA1(cd5986f260aac0ae1df1cea6dbd1dbee82536105) )
	ROM_LOAD( "pal16l8b-c16-09.ic35", 0x0200, 0x0104, CRC(0965baab) SHA1(3f704cd42d5277e9ba0b0680a2722488fc4cc630) )
ROM_END

ROM_START( viofightj )
	ROM_REGION( 0x080000, "maincpu", 0 )     /* 1M for 68000 code */
	ROM_LOAD16_BYTE( "c16-11.42", 0x00000, 0x10000, CRC(23dbd388) SHA1(488f928826d16b201dcc4b491b09955d0af91f19) )
	ROM_LOAD16_BYTE( "c16-10.23", 0x00001, 0x10000, CRC(329d2e46) SHA1(044e8a283e3bdd3d64dbeb9b6982088e967b10ff) )
	ROM_LOAD16_BYTE( "c16-07.41", 0x40000, 0x20000, CRC(64d1d059) SHA1(643ac7fa5076147b24810a8e1b925dfe09f75864) )
	ROM_LOAD16_BYTE( "c16-06.22", 0x40001, 0x20000, CRC(043761d8) SHA1(4587cadd73b628b4b9ac5c537cec20f90fb4959d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 128k for Z80 code */
	ROM_LOAD( "c16-12.32", 0x00000, 0x10000, CRC(6fb028c7) SHA1(a808d82e872914f994652e95dca3fcad00ba02fc) )

	ROM_REGION( 0x200000, "tc0180vcu", 0 )
	ROM_LOAD( "c16-01.1", 0x000000, 0x080000, CRC(7059ce83) SHA1(1e6825ab944254cd4ba6574762172245b3352319) )
	ROM_LOAD( "c16-02.2", 0x080000, 0x080000, CRC(b458e905) SHA1(b712cbf4a4015e1fc2243871fe753e230f0172c2) )
	ROM_LOAD( "c16-03.3", 0x100000, 0x080000, CRC(515a9431) SHA1(836be28614326d093be8841617cca83cef8d55cc) )
	ROM_LOAD( "c16-04.4", 0x180000, 0x080000, CRC(ebf285e2) SHA1(0f806e42778e28e9687d85b2601ee08dd786869b) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "c16-05.47", 0x000000, 0x80000, CRC(a49d064a) SHA1(f9ed675cfaae69b68c99c7dce7c2a457b5b5c293) )

	ROM_REGION( 0x00400, "plds", 0 )
	ROM_LOAD( "pal16l8b-c16-08.ic34", 0x0000, 0x0104, CRC(9456d278) SHA1(cd5986f260aac0ae1df1cea6dbd1dbee82536105) )
	ROM_LOAD( "pal16l8b-c16-09.ic35", 0x0200, 0x0104, CRC(0965baab) SHA1(3f704cd42d5277e9ba0b0680a2722488fc4cc630) )
ROM_END

ROM_START( masterw )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "b72_06.33", 0x00000, 0x20000, CRC(ae848eff) SHA1(8715e64c5e03097aff5bf1a27e3809619a7731f0) )
	ROM_LOAD16_BYTE( "b72_12.24", 0x00001, 0x20000, CRC(7176ce70) SHA1(f3462ab9fe7e118b16fbe31d8caca27452280bf9) )
	ROM_LOAD16_BYTE( "b72_04.34", 0x40000, 0x20000, CRC(141e964c) SHA1(324e881317a3bf9885c81bb53cdc3de782ec2952) )
	ROM_LOAD16_BYTE( "b72_03.25", 0x40001, 0x20000, CRC(f4523496) SHA1(2c3e9d014ace1ae5127f432292f8d19c3a0ae1b0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "b72_07.30", 0x00000, 0x10000, CRC(2b1a946f) SHA1(cc9512e44bd92020ab5a53716b6399b7a6cde76d) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "b72-02.6", 0x000000, 0x080000, CRC(843444eb) SHA1(2b466045f882996c80e0090009ee957e11d32825) )
	ROM_LOAD( "b72-01.5", 0x080000, 0x080000, CRC(a24ac26e) SHA1(895715a2bb0cb15334cba2283bd228b4fc08cd0c) )

	ROM_REGION( 0x00600, "plds", 0 )
	ROM_LOAD( "b72-08.ic3",  0x0000, 0x0104, CRC(1501a44a) SHA1(4dcda59238f17ce9d8654c27f45e217e6460fc03) )
	ROM_LOAD( "b72-09.ic23", 0x0200, 0x0104, CRC(a1d19d49) SHA1(5ca983147ef57240f6b6a05a07a821f72bb13001) )
	ROM_LOAD( "b72-10.ic32", 0x0400, 0x0104, CRC(20b0450b) SHA1(235ebbfd5b4cd179bb4d87e30d0e449028bb1df6) )
ROM_END

ROM_START( masterwu )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "b72_06.33", 0x00000, 0x20000, CRC(ae848eff) SHA1(8715e64c5e03097aff5bf1a27e3809619a7731f0) )
	ROM_LOAD16_BYTE( "b72_11.24", 0x00001, 0x20000, CRC(0671fee6) SHA1(6bec65d5e6704b4ec62c91f814675841ae9316a0) )
	ROM_LOAD16_BYTE( "b72_04.34", 0x40000, 0x20000, CRC(141e964c) SHA1(324e881317a3bf9885c81bb53cdc3de782ec2952) )
	ROM_LOAD16_BYTE( "b72_03.25", 0x40001, 0x20000, CRC(f4523496) SHA1(2c3e9d014ace1ae5127f432292f8d19c3a0ae1b0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "b72_07.30", 0x00000, 0x10000, CRC(2b1a946f) SHA1(cc9512e44bd92020ab5a53716b6399b7a6cde76d) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "b72-02.6", 0x000000, 0x080000, CRC(843444eb) SHA1(2b466045f882996c80e0090009ee957e11d32825) )
	ROM_LOAD( "b72-01.5", 0x080000, 0x080000, CRC(a24ac26e) SHA1(895715a2bb0cb15334cba2283bd228b4fc08cd0c) )

	ROM_REGION( 0x00600, "plds", 0 )
	ROM_LOAD( "b72-08.ic3",  0x0000, 0x0104, CRC(1501a44a) SHA1(4dcda59238f17ce9d8654c27f45e217e6460fc03) )
	ROM_LOAD( "b72-09.ic23", 0x0200, 0x0104, CRC(a1d19d49) SHA1(5ca983147ef57240f6b6a05a07a821f72bb13001) )
	ROM_LOAD( "b72-10.ic32", 0x0400, 0x0104, CRC(20b0450b) SHA1(235ebbfd5b4cd179bb4d87e30d0e449028bb1df6) )
ROM_END

ROM_START( masterwj )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "b72_06.33", 0x00000, 0x20000, CRC(ae848eff) SHA1(8715e64c5e03097aff5bf1a27e3809619a7731f0) )
	ROM_LOAD16_BYTE( "b72_05.24", 0x00001, 0x20000, CRC(9f78af5c) SHA1(5f3ca663c7257f5cec071907b1526fff3ab07b20) )
	ROM_LOAD16_BYTE( "b72_04.34", 0x40000, 0x20000, CRC(141e964c) SHA1(324e881317a3bf9885c81bb53cdc3de782ec2952) )
	ROM_LOAD16_BYTE( "b72_03.25", 0x40001, 0x20000, CRC(f4523496) SHA1(2c3e9d014ace1ae5127f432292f8d19c3a0ae1b0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "b72_07.30", 0x00000, 0x10000, CRC(2b1a946f) SHA1(cc9512e44bd92020ab5a53716b6399b7a6cde76d) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "b72-02.6", 0x000000, 0x080000, CRC(843444eb) SHA1(2b466045f882996c80e0090009ee957e11d32825) )
	ROM_LOAD( "b72-01.5", 0x080000, 0x080000, CRC(a24ac26e) SHA1(895715a2bb0cb15334cba2283bd228b4fc08cd0c) )

	ROM_REGION( 0x00600, "plds", 0 )
	ROM_LOAD( "b72-08.ic3",  0x0000, 0x0104, CRC(1501a44a) SHA1(4dcda59238f17ce9d8654c27f45e217e6460fc03) )
	ROM_LOAD( "b72-09.ic23", 0x0200, 0x0104, CRC(a1d19d49) SHA1(5ca983147ef57240f6b6a05a07a821f72bb13001) )
	ROM_LOAD( "b72-10.ic32", 0x0400, 0x0104, CRC(20b0450b) SHA1(235ebbfd5b4cd179bb4d87e30d0e449028bb1df6) )
ROM_END

ROM_START( yukiwo ) /* Prototype of Master of Weapon */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "ic33-rom0e.bin", 0x00000, 0x20000, CRC(a0dd51d9) SHA1(a4740bf08e26e1e576344c95d945df5d970738f2) )
	ROM_LOAD16_BYTE( "ic24-e882.bin",  0x00001, 0x20000, CRC(d66f29d4) SHA1(0854f1a0943a20693e6cd02825666e39b4fe28ca) )
	ROM_LOAD16_BYTE( "ic34-rom1e.bin", 0x40000, 0x10000, CRC(5ab7bc95) SHA1(393edefda5657853dccc21fa7848239789dbca65) )
	ROM_LOAD16_BYTE( "ic25-rom10.bin", 0x40001, 0x10000, CRC(0571b986) SHA1(aba9ac5fa4bd86cfa037baf34980b6a72417836b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "ic30-snd.bin", 0x00000, 0x08000, CRC(8632adb7) SHA1(a746ed0e7e2c2c216874b5729d59d7ed37689e4f) ) // TMM27256AD-20
	ROM_RELOAD(0x8000, 0x8000)

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD16_BYTE( "ic1-a010.bin", 0x000001, 0x020000, CRC(0030dce2) SHA1(649e0caf83b259814e1f0c2f6de40ebd8ea1a639) )
	ROM_LOAD16_BYTE( "ic5-9df1.bin", 0x000000, 0x020000, CRC(0507b908) SHA1(72f9913ddec37cd4e6b0b611619d0195302eb7e8) )
	ROM_LOAD16_BYTE( "ic3-1305.bin", 0x080001, 0x020000, CRC(8772b1a6) SHA1(961c8da0fcc3ce9dd334d7bcae75d8b93ded0bb4) )
	ROM_LOAD16_BYTE( "ic7-7c16.bin", 0x080000, 0x020000, CRC(a366bffd) SHA1(0b68163c73a8a020edbcd016b175d269ce2ad825) )
	ROM_LOAD16_BYTE( "ic2-6588.bin", 0x040001, 0x020000, CRC(25e79bc2) SHA1(f5b88ced9816338f45eb3430f7df0ecd5326321d) )
	ROM_LOAD16_BYTE( "ic6-6f3f.bin", 0x040000, 0x020000, CRC(77afcf80) SHA1(93886ad1bddc3bce578f575598e55bdd6e31c5ee) )
	ROM_LOAD16_BYTE( "ic4-9e5e.bin", 0x0c0001, 0x020000, CRC(3b30166b) SHA1(9450bcd30cf903cc0ae6c5f1cf2735ddd457e5f0) )
	ROM_LOAD16_BYTE( "ic8-e28a.bin", 0x0c0000, 0x020000, CRC(1b3db354) SHA1(a8481799182e5a06e105a4600679d4544317a350) )
ROM_END

ROM_START( silentd ) /* Silkscreened PCB number ET910000B */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "east-12-1.ic32", 0x00000, 0x20000, CRC(5883d362) SHA1(21c3af053fa92c26f119466ecd655697cc72ff3a) )
	ROM_LOAD16_BYTE( "east-15-1.ic10", 0x00001, 0x20000, CRC(8c0a72ae) SHA1(2199c4d6b87913ffb24cdccd6ca5176a97132baa) )
	ROM_LOAD16_BYTE( "east-11.ic31",   0x40000, 0x20000, CRC(35da4428) SHA1(5374bd97ad58aa2d67404cb05c862bb3aba40d6a) )
	ROM_LOAD16_BYTE( "east-09.ic9",    0x40001, 0x20000, CRC(2f05b14a) SHA1(f9ae935612e95d8ac2596af1728a6062569e9a42) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "east-13.ic15", 0x00000, 0x10000, CRC(651861ab) SHA1(f94a120b70a4d59e17a6e120ca461b1f37587c0c) )

	ROM_REGION( 0x400000, "tc0180vcu", 0 )
	ROM_LOAD( "east-04.ic28", 0x000000, 0x100000, CRC(53237217) SHA1(fec044c26b8c99235f88c8be0d9ac63b81a3a094) )
	ROM_LOAD( "east-06.ic29", 0x100000, 0x100000, CRC(e6e6dfa7) SHA1(913fa9a21cea175a1af87023144ebc98b3b0f33b) )
	ROM_LOAD( "east-03.ic39", 0x200000, 0x100000, CRC(1b9b2846) SHA1(d9c87e130bc3baa949d8a8738daad648fcf284df) )
	ROM_LOAD( "east-05.ic40", 0x300000, 0x100000, CRC(e02472c5) SHA1(35572610f6823ec980a928a75abd689197ebe207) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "east-01.ic1", 0x00000, 0x80000, CRC(b41fff1a) SHA1(54920d13fa2b3000eedab9d0050a299ae743c663) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "east-02.ic3", 0x00000, 0x80000, CRC(e0de5c39) SHA1(75d0e193d882e67921c216c3293454e34304d25e) )
ROM_END

ROM_START( silentdj ) /* Silkscreened PCB number ET910000B */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "east-12-1.ic32", 0x00000, 0x20000, CRC(5883d362) SHA1(21c3af053fa92c26f119466ecd655697cc72ff3a) )
	ROM_LOAD16_BYTE( "east-10-1.ic10", 0x00001, 0x20000, CRC(584306fc) SHA1(961cb6aaa426e3d83c499d101ef369b86a84c5d8) )
	ROM_LOAD16_BYTE( "east-11.ic31",   0x40000, 0x20000, CRC(35da4428) SHA1(5374bd97ad58aa2d67404cb05c862bb3aba40d6a) )
	ROM_LOAD16_BYTE( "east-09.ic9",    0x40001, 0x20000, CRC(2f05b14a) SHA1(f9ae935612e95d8ac2596af1728a6062569e9a42) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "east-13.ic15", 0x00000, 0x10000, CRC(651861ab) SHA1(f94a120b70a4d59e17a6e120ca461b1f37587c0c) )

	ROM_REGION( 0x400000, "tc0180vcu", 0 )
	ROM_LOAD( "east-04.ic28", 0x000000, 0x100000, CRC(53237217) SHA1(fec044c26b8c99235f88c8be0d9ac63b81a3a094) )
	ROM_LOAD( "east-06.ic29", 0x100000, 0x100000, CRC(e6e6dfa7) SHA1(913fa9a21cea175a1af87023144ebc98b3b0f33b) )
	ROM_LOAD( "east-03.ic39", 0x200000, 0x100000, CRC(1b9b2846) SHA1(d9c87e130bc3baa949d8a8738daad648fcf284df) )
	ROM_LOAD( "east-05.ic40", 0x300000, 0x100000, CRC(e02472c5) SHA1(35572610f6823ec980a928a75abd689197ebe207) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "east-01.ic1", 0x00000, 0x80000, CRC(b41fff1a) SHA1(54920d13fa2b3000eedab9d0050a299ae743c663) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "east-02.ic3", 0x00000, 0x80000, CRC(e0de5c39) SHA1(75d0e193d882e67921c216c3293454e34304d25e) )
ROM_END

ROM_START( silentdu ) /* Dumped from an original Taito PCB (ET910000B) */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "east-12-1.ic32", 0x00000, 0x20000, CRC(5883d362) SHA1(21c3af053fa92c26f119466ecd655697cc72ff3a) )
	ROM_LOAD16_BYTE( "east-14-1.ic10", 0x00001, 0x20000, CRC(3267bcd5) SHA1(358a717d0cdd22d84eb0d928c36e4e72a40c2882) )
	ROM_LOAD16_BYTE( "east-11.ic31",   0x40000, 0x20000, CRC(35da4428) SHA1(5374bd97ad58aa2d67404cb05c862bb3aba40d6a) )
	ROM_LOAD16_BYTE( "east-09.ic9",    0x40001, 0x20000, CRC(2f05b14a) SHA1(f9ae935612e95d8ac2596af1728a6062569e9a42) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "east-13.ic15", 0x00000, 0x10000, CRC(651861ab) SHA1(f94a120b70a4d59e17a6e120ca461b1f37587c0c) )

	ROM_REGION( 0x400000, "tc0180vcu", 0 )
	ROM_LOAD( "east-04.ic28", 0x000000, 0x100000, CRC(53237217) SHA1(fec044c26b8c99235f88c8be0d9ac63b81a3a094) )
	ROM_LOAD( "east-06.ic29", 0x100000, 0x100000, CRC(e6e6dfa7) SHA1(913fa9a21cea175a1af87023144ebc98b3b0f33b) )
	ROM_LOAD( "east-03.ic39", 0x200000, 0x100000, CRC(1b9b2846) SHA1(d9c87e130bc3baa949d8a8738daad648fcf284df) )
	ROM_LOAD( "east-05.ic40", 0x300000, 0x100000, CRC(e02472c5) SHA1(35572610f6823ec980a928a75abd689197ebe207) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )
	ROM_LOAD( "east-01.ic1", 0x00000, 0x80000, CRC(b41fff1a) SHA1(54920d13fa2b3000eedab9d0050a299ae743c663) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )
	ROM_LOAD( "east-02.ic3", 0x00000, 0x80000, CRC(e0de5c39) SHA1(75d0e193d882e67921c216c3293454e34304d25e) )
ROM_END

ROM_START( selfeena ) /* Silkscreened PCB number ET910000A */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "se-02.27", 0x00000, 0x20000, CRC(08f0c8e3) SHA1(2279ebfcd0cd05eec9c7a01ca7db82fcdc3b7ca7) )
	ROM_LOAD16_BYTE( "se-01.26", 0x00001, 0x20000, CRC(a06ca64b) SHA1(6bfd3c5faf169678ba9bb1c483901b6c06605faf) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "se-03.39",0x00000, 0x10000, CRC(675998be) SHA1(16e59a9b7dc46aeee0022cb73cf6feced6796c35) )

	ROM_REGION( 0x100000, "tc0180vcu", 0 )
	ROM_LOAD( "se-04.2",  0x000000, 0x80000, CRC(920ad100) SHA1(69cd2af6218db90632f09a131d2956ab69034643) )
	ROM_LOAD( "se-05.1",  0x080000, 0x80000, CRC(d297c995) SHA1(e5ad5a8ce222621c9156c2949916bee6b3099c4e) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "se-06.11", 0x00000, 0x80000, CRC(80d5e772) SHA1(bee4982a3d65210ff86495e36a0b656934b00c7d) )
ROM_END

ROM_START( ryujin ) /* Silkscreened PCB number ET910000B */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "rjn_02.ic32", 0x00000, 0x20000, CRC(5fd353d5) SHA1(76c5e173f5945f9d69f805961ffc190f4f1532d7) ) /* Yes!, these were labeled RJN 02 & RJN 01 (unlike the rest labeled RUJ xx) */
	ROM_LOAD16_BYTE( "rjn_01.ic10", 0x00001, 0x20000, CRC(f775e4b6) SHA1(1b85b217daf4784e35e0beb088bee229244f1207) )
	ROM_LOAD16_BYTE( "ruj_04.ic31", 0x40000, 0x20000, CRC(0c153cab) SHA1(16fac3863c1394c9f41173174a4aca20cded6278) ) /* ET910000B PCB uses different IC locations */
	ROM_LOAD16_BYTE( "ruj_03.ic9",  0x40001, 0x20000, CRC(7695f89c) SHA1(755eb7ef40da190d55de80ee5e0e0a537c22e5f1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "ruj_05.ic15",0x00000, 0x10000, CRC(95270b16) SHA1(c1ad76268679cf198e9f1514360f280b73e49ab5) )

	ROM_REGION( 0x200000, "tc0180vcu", 0 )
	ROM_LOAD( "ryujin-07.ic28", 0x000000, 0x100000, CRC(34f50980) SHA1(432384bd283389bca17611602eb310726c9d78a4) )
	ROM_LOAD( "ryujin-06.ic39", 0x100000, 0x100000, CRC(1b85ff34) SHA1(5ad259e6f7aa4a0c08975da73bf41400495f2e61) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "ryujin-08.ic1", 0x00000, 0x80000, CRC(480d040d) SHA1(50add2f304ef34f7f45f25a2a2cf0568d58259ad) )

	ROM_REGION( 0x00400, "plds", 0 )
	ROM_LOAD( "pal16l8b-east-07.ic46", 0x0000, 0x0104, CRC(0cb80f64) SHA1(53c38fc795d05277172391d7994c96d8c66b49c4) )
	ROM_LOAD( "pal16l8b-east-08.ic47", 0x0200, 0x0104, CRC(bdce045f) SHA1(cc67b2afa1b57345a4b91f5fafd99cae5286827a) )
ROM_END

ROM_START( ryujina ) /* Silkscreened PCB number ET910000A */
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "ruj_02.27", 0x00000, 0x20000, CRC(0d223aee) SHA1(33f5498a650b244c5a4a22415408a269da597abf) )
	ROM_LOAD16_BYTE( "ruj_01.26", 0x00001, 0x20000, CRC(c6bcdd1e) SHA1(d8620995ad1bc256eab4ed7e1c549e8b6ec5c3fb) )
	ROM_LOAD16_BYTE( "ruj_04.29", 0x40000, 0x20000, CRC(0c153cab) SHA1(16fac3863c1394c9f41173174a4aca20cded6278) )
	ROM_LOAD16_BYTE( "ruj_03.28", 0x40001, 0x20000, CRC(7695f89c) SHA1(755eb7ef40da190d55de80ee5e0e0a537c22e5f1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "ruj_05.39",0x00000, 0x10000, CRC(95270b16) SHA1(c1ad76268679cf198e9f1514360f280b73e49ab5) )

	ROM_REGION( 0x200000, "tc0180vcu", 0 )
	ROM_LOAD( "ryujin-07.2", 0x000000, 0x100000, CRC(34f50980) SHA1(432384bd283389bca17611602eb310726c9d78a4) )
	ROM_LOAD( "ryujin-06.1", 0x100000, 0x100000, CRC(1b85ff34) SHA1(5ad259e6f7aa4a0c08975da73bf41400495f2e61) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "ryujin-08.11", 0x00000, 0x80000, CRC(480d040d) SHA1(50add2f304ef34f7f45f25a2a2cf0568d58259ad) )
ROM_END

ROM_START( sbm )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "c69-20-2.ic10", 0x00000, 0x20000, CRC(225952a3) SHA1(d95ef971929d4a8c19193e80499052f57d205001) )
	ROM_LOAD16_BYTE( "c69-22-2.ic12", 0x00001, 0x20000, CRC(d900ce83) SHA1(b86c3e7c2f9f2dbd415b7a355e5978de46fc3564) )
	ROM_LOAD16_BYTE( "c69-19-2.ic9" , 0x40000, 0x20000, CRC(d6cfacfb) SHA1(0f910e92a81e0d7ac35d0ffc799dd723f64d47d1) )
	ROM_LOAD16_BYTE( "c69-25-2.ic11", 0x40001, 0x20000, CRC(70903898) SHA1(879412e3c385533fd0bf1e47956e3e9bea78378d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "c69-31.ic28",0x00000, 0x10000, CRC(c999f753) SHA1(5d944b492893a0bb605743e2928c802427a85fbf) )

	ROM_REGION( 0x400000, "tc0180vcu", 0 )
	ROM_LOAD       ( "c69-01.ic5", 0x000000, 0x100000, CRC(521fabe3) SHA1(f81303688ac014145a7e9325affc4a0f86755ba9) )
	ROM_LOAD16_BYTE( "c69-13.ic2", 0x100000, 0x020000, CRC(d1550884) SHA1(f3a65827c45959fe5ac4bfc92831a8bed76bf287) )
	ROM_LOAD16_BYTE( "c69-12.ic1", 0x100001, 0x020000, CRC(eb56582c) SHA1(41b77704aeaecf3eaa3f87e97c0b59fd00e5349e) )
	/* 140000-1fffff empty */
	ROM_LOAD       ( "c69-02.ic6", 0x200000, 0x100000, CRC(f0e20d35) SHA1(af67f39498f68523ece4cd91045456092038e0a4) )
	ROM_LOAD16_BYTE( "c69-15.ic4", 0x300000, 0x020000, CRC(9761d316) SHA1(f03216bbade96948ff433a925e8bffb8760b4101) )
	ROM_LOAD16_BYTE( "c69-14.ic3", 0x300001, 0x020000, CRC(0ed0272a) SHA1(03b15654213ff71ffc96d3a87657bdeb724e9269) )
	/* 340000-3fffff empty */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c69-26.ic36", 0x00000, 0x80000, CRC(8784058b) SHA1(c3d9c620704fb5e80719e996e97f6191a5bd9f8c) )

	ROM_REGION( 0x1800, "plds", 0 )
	ROM_LOAD( "c69-04.ic6",  0x000, 0x104, CRC(80498715) SHA1(076c26f365b74f3b6adb2c4c1a3add5a46a257ea) ) // 16l8
	ROM_LOAD( "c69-05.ic25", 0x200, 0x104, CRC(35e345b4) SHA1(befbea944c645dafb143d8dba83fcc16f1edd3e6) ) // 16l8
	ROM_LOAD( "c69-06.ic17", 0x400, 0x144, CRC(3988e5d1) SHA1(24e03455fd9e653e3fc901bef2c3f3ea59f04ea5) ) // 20l8
ROM_END

ROM_START( sbmj )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "c69-20-1.10", 0x00000, 0x20000, CRC(b40e4910) SHA1(1436990b8c3c1d0763283133b2ce9d47cd65f825) )
	ROM_LOAD16_BYTE( "c69-22-1.12", 0x00001, 0x20000, CRC(ecbcf830) SHA1(49f04a198d327866b26b978a302ddc7aea5ac6de) )
	ROM_LOAD16_BYTE( "c69-19-1.9" , 0x40000, 0x20000, CRC(5719c158) SHA1(e1cdf89695e1dba75f92e449d08fa6df57e9b388) )
	ROM_LOAD16_BYTE( "c69-21-1.11", 0x40001, 0x20000, CRC(73562394) SHA1(ef9cf4718db05d9b99d6e2630e8f6ad248f30b9a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "c69-23.28",0x00000, 0x10000, CRC(b2fce13e) SHA1(ecdb08482af80fd04cc8b67157e11ca61ce06437) )

	ROM_REGION( 0x400000, "tc0180vcu", 0 )
	ROM_LOAD       ( "c69-01.ic5", 0x000000, 0x100000, CRC(521fabe3) SHA1(f81303688ac014145a7e9325affc4a0f86755ba9) )
	ROM_LOAD16_BYTE( "c69-13.ic2", 0x100000, 0x020000, CRC(d1550884) SHA1(f3a65827c45959fe5ac4bfc92831a8bed76bf287) )
	ROM_LOAD16_BYTE( "c69-12.ic1", 0x100001, 0x020000, CRC(eb56582c) SHA1(41b77704aeaecf3eaa3f87e97c0b59fd00e5349e) )
	/* 140000-1fffff empty */
	ROM_LOAD       ( "c69-02.ic6", 0x200000, 0x100000, CRC(f0e20d35) SHA1(af67f39498f68523ece4cd91045456092038e0a4) )
	ROM_LOAD16_BYTE( "c69-15.ic4", 0x300000, 0x020000, CRC(9761d316) SHA1(f03216bbade96948ff433a925e8bffb8760b4101) )
	ROM_LOAD16_BYTE( "c69-14.ic3", 0x300001, 0x020000, CRC(0ed0272a) SHA1(03b15654213ff71ffc96d3a87657bdeb724e9269) )
	/* 340000-3fffff empty */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c69-03.36", 0x00000, 0x80000, CRC(63e6b6e7) SHA1(72574ca7505eee15fabc4996f253505d9dd65898) )
ROM_END

ROM_START( realpunc )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "d76_05.47", 0x00000, 0x80000, CRC(879b7e6a) SHA1(2b06fb4b92d4c23edba97974161da1cb88e0daf5) )
	ROM_LOAD16_BYTE( "d76_18.48", 0x00001, 0x80000, CRC(46ed7a9f) SHA1(5af7f23e79b9a947f15d36fe54111aa76bc1037b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for Z80 code */
	ROM_LOAD( "d76_06.106",0x00000, 0x10000, CRC(72c799fd) SHA1(ab086be38b890152b33f0c4e33d0f02d0a5321bc) )

	ROM_REGION( 0x400000, "tc0180vcu", 0 )
	ROM_LOAD( "d76_02.76", 0x000000, 0x100000, CRC(57691b93) SHA1(570dbefda40f8be5f1da58c5433b8a8084f49cac) )
	ROM_LOAD( "d76_03.45", 0x200000, 0x100000, CRC(9f0aefd8) SHA1(d516c64baabd268f99dc5e67b7adf135b4eb45fd) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 )      /* ADPCM samples */
	ROM_LOAD( "d76_01.93", 0x000000, 0x200000, CRC(2bc265f2) SHA1(409b822989e2aad50872f80f5160d4909c42206c) )
ROM_END

ROM_START( realpuncj )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "d76_05.47", 0x00000, 0x80000, CRC(879b7e6a) SHA1(2b06fb4b92d4c23edba97974161da1cb88e0daf5) )
	ROM_LOAD16_BYTE( "d76_04.48", 0x00001, 0x80000, CRC(bb8ab32d) SHA1(60e04d406bbd3e1b66edde15c6e8d4df4d32703b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for Z80 code */
	ROM_LOAD( "d76_06.106",0x00000, 0x10000, CRC(72c799fd) SHA1(ab086be38b890152b33f0c4e33d0f02d0a5321bc) )

	ROM_REGION( 0x400000, "tc0180vcu", 0 )
	ROM_LOAD( "d76_02.76", 0x000000, 0x100000, CRC(57691b93) SHA1(570dbefda40f8be5f1da58c5433b8a8084f49cac) )
	ROM_LOAD( "d76_03.45", 0x200000, 0x100000, CRC(9f0aefd8) SHA1(d516c64baabd268f99dc5e67b7adf135b4eb45fd) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 )      /* ADPCM samples */
	ROM_LOAD( "d76_01.93", 0x000000, 0x200000, CRC(2bc265f2) SHA1(409b822989e2aad50872f80f5160d4909c42206c) )
ROM_END

GAME( 1989, masterw,  0,       masterw,  masterw,   taitob_state, empty_init, ROT270, "Taito Corporation Japan",   "Master of Weapon (World)",  MACHINE_SUPPORTS_SAVE )
GAME( 1989, masterwu, masterw, masterw,  masterwu,  taitob_state, empty_init, ROT270, "Taito America Corporation", "Master of Weapon (US)",     MACHINE_SUPPORTS_SAVE )
GAME( 1989, masterwj, masterw, masterw,  masterwj,  taitob_state, empty_init, ROT270, "Taito Corporation",         "Master of Weapon (Japan)",  MACHINE_SUPPORTS_SAVE )
GAME( 1989, yukiwo,   masterw, masterw,  yukiwo,    taitob_state, empty_init, ROT270, "Taito Corporation Japan",   "Yukiwo (World, prototype)", MACHINE_SUPPORTS_SAVE )

GAME( 1988, nastar,   0,       rastsag2, nastar,    taitob_state, empty_init, ROT0,   "Taito Corporation Japan",   "Nastar (World)",        MACHINE_SUPPORTS_SAVE )
GAME( 1988, nastarw,  nastar,  rastsag2, nastarw,   taitob_state, empty_init, ROT0,   "Taito America Corporation", "Nastar Warrior (US)",   MACHINE_SUPPORTS_SAVE )
GAME( 1988, rastsag2, nastar,  rastsag2, rastsag2,  taitob_state, empty_init, ROT0,   "Taito Corporation",         "Rastan Saga 2 (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, rambo3,   0,       rambo3,   rambo3,    rambo3_state, empty_init, ROT0,   "Taito Europe Corporation",  "Rambo III (Europe)",         MACHINE_SUPPORTS_SAVE )
GAME( 1989, rambo3u,  rambo3,  rambo3,   rambo3u,   rambo3_state, empty_init, ROT0,   "Taito America Corporation", "Rambo III (US)",             MACHINE_SUPPORTS_SAVE )
GAME( 1989, rambo3p,  rambo3,  rambo3p,  rambo3p,   rambo3_state, empty_init, ROT0,   "Taito Europe Corporation",  "Rambo III (Europe, Proto?)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, crimec,   0,       crimec,   crimec,    taitob_state, empty_init, ROT0,   "Taito Corporation Japan",   "Crime City (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, crimecu,  crimec,  crimec,   crimecu,   taitob_state, empty_init, ROT0,   "Taito America Corporation", "Crime City (US)",    MACHINE_SUPPORTS_SAVE )
GAME( 1989, crimecj,  crimec,  crimec,   crimecj,   taitob_state, empty_init, ROT0,   "Taito Corporation",         "Crime City (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1988, tetrist,  tetris,  tetrist,  tetrist,   taitob_state, empty_init, ROT0,   "Sega", "Tetris (Japan, Taito B-System, Nastar Conversion Kit)",           MACHINE_SUPPORTS_SAVE )
GAME( 1988, tetrista, tetris,  tetrista, tetrist,   taitob_state, empty_init, ROT0,   "Sega", "Tetris (Japan, Taito B-System, Master of Weapon Conversion Kit)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, viofight, 0,       viofight, viofight,  taitob_state, empty_init, ROT0,   "Taito Corporation Japan",   "Violence Fight (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, viofightu,viofight,viofight, viofightu, taitob_state, empty_init, ROT0,   "Taito America Corporation", "Violence Fight (US)",    MACHINE_SUPPORTS_SAVE )
GAME( 1989, viofightj,viofight,viofight, viofightj, taitob_state, empty_init, ROT0,   "Taito Corporation",         "Violence Fight (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, ashura,   0,       ashura,   ashura,    taitob_state, empty_init, ROT270, "Taito Corporation Japan",   "Ashura Blaster (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, ashuraj,  ashura,  ashura,   ashuraj,   taitob_state, empty_init, ROT270, "Taito Corporation",         "Ashura Blaster (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, ashurau,  ashura,  ashura,   ashurau,   taitob_state, empty_init, ROT270, "Taito America Corporation", "Ashura Blaster (US)",    MACHINE_SUPPORTS_SAVE )

GAME( 1990, hitice,   0,       hitice,   hitice,    hitice_state, empty_init, ROT0,   "Taito Corporation (Williams license)",     "Hit the Ice (US)",                   MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1990, hiticerb, hitice,  hitice,   hitice,    hitice_state, empty_init, ROT0,   "Taito Corporation (Williams license)",     "Hit the Ice (US, with riser board)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1990, hiticej,  hitice,  hitice,   hiticej,   hitice_state, empty_init, ROT0,   "Taito Corporation (licensed from Midway)", "Hit the Ice (Japan)",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1991, selfeena, 0,       selfeena, selfeena,  taitob_state, empty_init, ROT0,   "East Technology", "Sel Feena", MACHINE_SUPPORTS_SAVE )

GAME( 1992, silentd,  0,       silentd,  silentd,   taitob_state, empty_init, ROT0,   "Taito Corporation Japan",   "Silent Dragon (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, silentdj, silentd, silentd,  silentdj,  taitob_state, empty_init, ROT0,   "Taito Corporation",         "Silent Dragon (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, silentdu, silentd, silentd,  silentdu,  taitob_state, empty_init, ROT0,   "Taito America Corporation", "Silent Dragon (US)",    MACHINE_SUPPORTS_SAVE )

GAME( 1993, ryujin,   0,       silentd,  ryujin,    taitob_state, empty_init, ROT270, "Taito Corporation", "Ryu Jin (Japan, ET910000B PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, ryujina,  ryujin,  selfeena, ryujin,    taitob_state, empty_init, ROT270, "Taito Corporation", "Ryu Jin (Japan, ET910000A PCB)", MACHINE_SUPPORTS_SAVE )

GAME( 1993, qzshowby, 0,       qzshowby, qzshowby,  taitob_state, empty_init, ROT0,   "Taito Corporation", "Quiz Sekai wa SHOW by shobai (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1994, pbobble,  0,       pbobble,  pbobble,   taitob_state, empty_init, ROT0,   "Taito Corporation",         "Puzzle Bobble (Japan, B-System)", MACHINE_SUPPORTS_SAVE ) // PUZZLE  VER 2.0J   1994/06/15  15:28:30
GAME( 1994, bublbust, pbobble, pbobble,  pbobble,   taitob_state, empty_init, ROT0,   "Taito America Corporation", "Bubble Buster (USA, B-System)",   MACHINE_SUPPORTS_SAVE ) // PUZZLE  VER 2.0A   1994/06/15  15:28:30 - Location test but same build as Japan release?

GAME( 1994, spacedx,  0,       spacedx,  pbobble,   taitob_state, empty_init, ROT0,   "Taito Corporation", "Space Invaders DX (US, v2.1)",    MACHINE_SUPPORTS_SAVE )
GAME( 1994, spacedxj, spacedx, spacedx,  pbobble,   taitob_state, empty_init, ROT0,   "Taito Corporation", "Space Invaders DX (Japan, v2.1)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, spacedxo, spacedx, spacedxo, spacedxo,  taitob_state, empty_init, ROT0,   "Taito Corporation", "Space Invaders DX (Japan, v2.0)", MACHINE_SUPPORTS_SAVE )

// Sonic Blast Man is a ticket dispensing game. (Japanese version however does not dispense them, only US does - try the "sbm_patch" in the machine_config).
// It is a bit different from other games running on this system, in that it has a punching pad that player needs to punch to hit the enemy.
GAME( 1990, sbm,      0,       sbm,      sbm,       taitob_state,   empty_init, ROT0, "Taito Corporation",       "Sonic Blast Man (US)",    MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL )
GAME( 1990, sbmj,     sbm,     sbm,      sbmj,      taitob_state,   empty_init, ROT0, "Taito Corporation",       "Sonic Blast Man (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL )
GAME( 1994, realpunc, 0,       realpunc, realpunc,  taitob_c_state, empty_init, ROT0, "Taito Corporation Japan", "Real Puncher (World, v2.12O)",   MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL )
GAME( 1994, realpuncj,realpunc,realpunc, realpunc,  taitob_c_state, empty_init, ROT0, "Taito Corporation Japan", "Real Puncher (Japan, v2.12J)",   MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL )
