// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Nicola Salmoria
/***************************************************************************

Buggy Challenge - (c) 1984 Taito Corporation

driver by Ernesto Corvi and Nicola Salmoria

TODO:
- I'm almost sure that I'm not handling the zoom x ROM table correctly. Gives
  reasonable results, though. I'm confident that the zoom y table handling is
  correct.
- Tilemap and sprite placement might not be accurate, there aren't many
  references.
- The gradient sky is completely wrong - it's more of a placeholder to show
  that it's supposed to be there. It is supposed to skew along with the
  background, and the gradient can move around (the latter doesn't seem to
  be used except for making it cover the whole screen on the title screen,
  and start at the middle during gameplay)
  Update: stage 2 is supposed to have a different gradient, how/where
  this is located is unknown (pen 0x20?)
- Video driver is largely unoptimized
- Support for the 7630's controlling the sound chip outputs (bass/treble,
  volume) is completely missing.
- The sound Z80 seems to write answers for the main Z80, but the latter doesn't
  seem to read them.
- videoram and spriteram garbage occurs when entering into cross hatch test and exiting.
  Game attempts to transfer content of videoram into spriteram/scrollram, then transfer
  back again into videoram. Maybe the host CPU cannot read contents of VRAM at all?

Notes:
- There is also a 4-channel version of the sound board for the cockpit
  cabinet (ROMs not dumped)


Memory Map
----------
0000 - 3fff = ROM A22-04 (23)
4000 - 7fff = ROM A22-05 (22)
8000 - 87ff = RAM (36)
8800 - 8fff = RAM (35)

c800 - cbff = videoram
cc00 - cfff = videoram

d100 = /ANYOUT
    bit7 = lamp
    bit6 = lockout
    bit4 = OJMODE (sprite palette bank)
    bit3 = SKY OFF
    bit2 = /SN3OFF
    bit1 = flip screen X
    bit0 = flip screen Y
d200 = bank switch
    bit2 = Bank Select bit 1
    bit1 = Bank Select bit 0
    bit0 = EA13 (high/low part of banked ROM)
d300 = /TRESET (Watchdog reset?)
d301 = No name?
    bit6 = FLPF2 (W-6)
    bit5 = FLPE2 (W-5)
    bit4 = FLPD2 (W-4)
    bot2 = FLPF1 (W-3)
    bit1 = FLPE1 (W-2)
    bit0 = FLPD1 (W-1)
d302 - bit 0 = /RESET line on the 68705

d304 - d307 = SCCON1 to SCCON4

d613 = /SoundCS = /RESET line on all audio CPUs

d700 - d7ff = ( /VCRRQ - palette ram )

d800 - d8ff /ScrollRQ (S37)
da00 - daff /ScrollRQ (S37)
db00 - dbff /ScrollRQ (S37)

dcxx = /SPOSI (S36)

2008-07
Dip locations and factory settings verified from dip listing

Clock information:
Xtal = 48mhz
QA output = "24M" = 24mhz
QB output = "1/2CLK" = 12mhz
QC output = "CLK" = 6mhz
"1/2phi" = 24M / 3 = 8mhz

The z80B main cpu is clocked by (depending on a jumper) either "1/2CLK"/2 OR "1/2PHI"/2, so either 6mhz or 4mhz.
Schematics show the jumper set to the 6mhz setting.

***************************************************************************/

#include "emu.h"
#include "includes/buggychl.h"

#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "machine/watchdog.h"
#include "speaker.h"

#include "buggychl.lh"


WRITE8_MEMBER(buggychl_state::bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x07);   // shall we check if data&7 < # banks?
}

WRITE8_MEMBER(buggychl_state::sound_enable_w)
{
	machine().sound().system_enable(data & 1);
}

READ8_MEMBER(buggychl_state::mcu_status_r)
{
	// bit 0 = when 1, MCU is ready to receive data from main CPU
	// bit 1 = when 1, MCU has sent data to the main CPU
	return
		((CLEAR_LINE == m_bmcu->host_semaphore_r()) ? 0x01 : 0x00) |
		((CLEAR_LINE != m_bmcu->mcu_semaphore_r()) ? 0x02 : 0x00);
}

// the schematics show that the two sound semaphore latch bits are actually flipped backwards when read by the sound cpu
//   vs when read by the main cpu.
//   Given the other schematic errors, and the fact that the sound board schematic is for the wrong pcb, is this even correct?
//   It isn't even obvious if the maincpu or sound cpu read the semaphores at all, ever.
// a cpu write to soundlatch sets ic12.2 so /Q is low, so cpu bit 1 and sound bit 0 read as clear
// a sound write to soundlatch2 clears ic12.1 so /Q is high, so cpu bit 0 and sound bit 1 read as set
// a cpu read of soundlatch2 sets ic12.1 so /Q is low, so cpu bit 0 and sound bit 1 read as clear
// a sound read of soundlatch clears ic12.2 so /Q is high, so cpu bit 1 and sound bit 0 read as set
// ic12.1 is set and ic12.2 is cleared by /SRESET
READ8_MEMBER(buggychl_state::sound_status_main_r)
{
	return (m_soundlatch2->pending_r() ? 1 : 0) | (m_soundlatch->pending_r() ? 0 : 2);
}

READ8_MEMBER(buggychl_state::sound_status_sound_r)
{
	return (m_soundlatch2->pending_r() ? 2 : 0) | (m_soundlatch->pending_r() ? 0 : 1);
}

/* Main cpu address map ( * = used within this section; x = don't care )
           |           |           |
15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 *  *                                             R  74LS139 @ ic53
 0  0  *  *  *  *  *  *  *  *  *  *  *  *  *  *   R  ROM (ic23)
 0  1  *  *  *  *  *  *  *  *  *  *  *  *  *  *   R  ROM (ic22)
 1  *  *  *                                       RW 74LS138 @ ic66
 1  0  0  0  0  *  *  *  *  *  *  *  *  *  *  *   RW SRAM (ic36)
 1  0  0  0  1  *  *  *  *  *  *  *  *  *  *  *   RW SRAM (ic35)
 1  0  0  1  *  *  *  *  *  *  *  *  *  *  *  *   RW  /STYLRQ
 (TODO: finish above, its quite complicated)
 1  0  1  *  *  *  *  *  *  *  *  *  *  *  *  *   R  /EXROMRD and /CDRRQW, banked ROM reads (generates a waitstate)
 1  1  0  0  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?   ?  (unknown, cut off on schematic)
 1  1  0  1  0  0  0  0  ?  ?  ?  ?  ?  ?  ?  ?   W  HORIZON
 1  1  0  1  0  0  0  1  x  x  x  x  x  x  x  x   W  ANY OUT (lamp d7, lockout d6, unused d5, ojmode d4, skyoff d3, sn31/4off d2, hinv d1, vinv d0)
 1  1  0  1  0  0  1  0  x  x  x  x  x  x  x  x   W  BANKSWITCH (banking, rom selected on d2, d1, upper/lower half of rom on d0)
 1  1  0  1  0  0  1  1  x  x  x  x  x  *  *  *   W  74LS138 @ ic39
 1  1  0  1  0  0  1  1  x  x  x  x  x  0  0  0   W  /TRESET (watchdog reset)
 1  1  0  1  0  0  1  1  x  x  x  x  x  0  0  1   W  FLPD1,E1,F1,D2,E2,F2 on d0-d5 respectively
 1  1  0  1  0  0  1  1  x  x  x  x  x  0  1  0   W  /SRESET (value of d0 latched; this is the mcu reset and resets the mcu semaphores as well)
 1  1  0  1  0  0  1  1  x  x  x  x  x  0  1  1   W  STYLBANK (d4 controls latch at v-ic25.1)
 1  1  0  1  0  0  1  1  x  x  x  x  x  1  0  0   W  SCCON1
 1  1  0  1  0  0  1  1  x  x  x  x  x  1  0  1   W  SCCON2
 1  1  0  1  0  0  1  1  x  x  x  x  x  1  1  0   W  SCCON3
 1  1  0  1  0  0  1  1  x  x  x  x  x  1  1  1   W  SCCON4
 (the four ports above are probably for connecting to a bezel score display, almost identical to that of Grand Champion; see https://ia800501.us.archive.org/16/items/ArcadeGameManualGrandchampion/grandchampion.pdf pdf pages 54 and 55)
 1  1  0  1  0  1  0  0  x  x  x  x  x  x  *  *   RW  SEQRQ 74ls155 @ ic42
 1  1  0  1  0  1  0  0  x  x  x  x  x  x  0  0   W  Write to MCU in latch and set ic43.1  semaphore
 1  1  0  1  0  1  0  0  x  x  x  x  x  x  0  0   R  Read from MCU out latch and clear ic43.2 semaphore
 1  1  0  1  0  1  0  0  x  x  x  x  x  x  0  1   R  Read semaphores : /ic43.0 in d0 and ic43.1 in d1
 1  1  0  1  0  1  0  0  x  x  x  x  x  x  1  x   OPEN BUS
 1  1  0  1  0  1  0  1  *  *  *  *  *  *  *  *   RW  OBJRQ (read/write obj SRAM vb-ic34)
 1  1  0  1  0  1  1  0  x  x  x  *  *            W  74LS139 @ ic52
 1  1  0  1  0  1  1  0  x  x  x  0  0  x  *  *   R  INPUTA
 (16 inputs read here in the 4 bytes)
 1  1  0  1  0  1  1  0  x  x  x  0  1  x  *  *   R  INPUTB
 (16 inputs read here in the 4 bytes)
 1  1  0  1  0  1  1  0  x  x  x  1  0  x  *  *   *  SOUNDCS 74ls155 @ s-ic40
 1  1  0  1  0  1  1  0  x  x  x  1  0  x  0  0   R  Read from Sound out latch, set s-ic12.1
 1  1  0  1  0  1  1  0  x  x  x  1  0  x  0  1   R  Read sound semaphores: /s-ic12.1 in d0 and /s-ic12.2 in d1
 1  1  0  1  0  1  1  0  x  x  x  1  0  x  1  x   R  OPEN BUS
 1  1  0  1  0  1  1  0  x  x  x  1  0  x  0  0   W  Write to Sound in latch, set s-ic12.2
 1  1  0  1  0  1  1  0  x  x  x  1  0  x  0  1   W  OPEN BUS
 1  1  0  1  0  1  1  0  x  x  x  1  0  x  1  0   W  OPEN BUS
 1  1  0  1  0  1  1  0  x  x  x  1  0  x  1  1   W  SNDRESET (value of d0 latched; if high, this sets s-ic12.1, clears s-ic12.2, clears soundnmi enable, clears sound control latch, resets sound z80(s), zeroes all dac input latches, resets ay-3-8910 chips, and resets the waitstate request)
 1  1  0  1  0  1  1  0  x  x  x  1  1  x  x  x   W  ACCELCL
 1  1  0  1  0  1  1  1  x  x  *  *  *  *  *  *   RW /VCRRQ
 (TODO: palette sram)
 1  1  0  1  1  0  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?   ?  /SCROLRQ
 1  1  0  1  1  1  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?   ?  /S_POSI
 1  1  1  0  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?   ?  (unknown, cut off on schematic)
 1  1  1  1  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?  ?   ?  (unknown, cut off on schematic)
*/
ADDRESS_MAP_START(buggychl_state::buggychl_map)
	AM_RANGE(0x0000, 0x3fff) AM_ROM /* A22-04 (23) */
	AM_RANGE(0x4000, 0x7fff) AM_ROM /* A22-05 (22) */
	AM_RANGE(0x8000, 0x87ff) AM_RAM /* 6116 SRAM (36) */
	AM_RANGE(0x8800, 0x8fff) AM_RAM /* 6116 SRAM (35) */
	AM_RANGE(0x9000, 0x9fff) AM_WRITE(buggychl_sprite_lookup_w)
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank1") AM_WRITE(buggychl_chargen_w) AM_SHARE("charram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xd000, 0xd000) AM_WRITENOP // ???
	AM_RANGE(0xd100, 0xd100) AM_MIRROR(0x00ff) AM_WRITE(buggychl_ctrl_w)
	AM_RANGE(0xd200, 0xd200) AM_MIRROR(0x00ff) AM_WRITE(bankswitch_w)
	AM_RANGE(0xd300, 0xd300) AM_MIRROR(0x00f8) AM_DEVWRITE("watchdog", watchdog_timer_device, reset_w)
	// d301 = flp stuff, unused?
	// d302 = mcu reset latched d0
	AM_RANGE(0xd303, 0xd303) AM_MIRROR(0x00f8) AM_WRITE(buggychl_sprite_lookup_bank_w)
	AM_RANGE(0xd304, 0xd307) AM_WRITENOP // d304-d307 is SCCON, which seems to be for a bezel mounted 7seg score/time display like Grand Champion has
	AM_RANGE(0xd400, 0xd400) AM_MIRROR(0x00fc) AM_DEVREADWRITE("bmcu", taito68705_mcu_device, data_r, data_w)
	AM_RANGE(0xd401, 0xd401) AM_MIRROR(0x00fc) AM_READ(mcu_status_r)
	AM_RANGE(0xd500, 0xd57f) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xd600, 0xd600) AM_MIRROR(0x00e4) AM_READ_PORT("DSW1")
	AM_RANGE(0xd601, 0xd601) AM_MIRROR(0x00e4) AM_READ_PORT("DSW2")
	AM_RANGE(0xd602, 0xd602) AM_MIRROR(0x00e4) AM_READ_PORT("DSW3")
	AM_RANGE(0xd603, 0xd603) AM_MIRROR(0x00e4) AM_READ_PORT("IN0")    /* player inputs */
	AM_RANGE(0xd608, 0xd608) AM_MIRROR(0x00e4) AM_READ_PORT("WHEEL")
	AM_RANGE(0xd609, 0xd609) AM_MIRROR(0x00e4) AM_READ_PORT("IN1")    /* coin + accelerator */
//  AM_RANGE(0xd60a, 0xd60a) AM_MIRROR(0x00e4) // other inputs, not used?
//  AM_RANGE(0xd60b, 0xd60b) AM_MIRROR(0x00e4) // other inputs, not used?
	AM_RANGE(0xd610, 0xd610) AM_MIRROR(0x00e4) AM_DEVREAD("soundlatch2", generic_latch_8_device, read) AM_DEVWRITE("soundlatch", generic_latch_8_device, write)
	AM_RANGE(0xd611, 0xd611) AM_MIRROR(0x00e4) AM_READ(sound_status_main_r)
//  AM_RANGE(0xd613, 0xd613) AM_MIRROR(0x00e4) AM_WRITE(sound_reset_w)
	AM_RANGE(0xd618, 0xd618) AM_MIRROR(0x00e7) AM_WRITENOP    /* accelerator clear; TODO: should we emulate the proper quadrature counter here? */
	AM_RANGE(0xd700, 0xd7ff) AM_DEVWRITE("palette", palette_device, write8) AM_SHARE("palette")
	AM_RANGE(0xd820, 0xd83f) AM_RAM // TODO
	AM_RANGE(0xd840, 0xd85f) AM_WRITEONLY AM_SHARE("scrollv")
	AM_RANGE(0xdb00, 0xdbff) AM_WRITEONLY AM_SHARE("scrollh")
	AM_RANGE(0xdc04, 0xdc04) AM_WRITEONLY /* should be fg scroll */
	AM_RANGE(0xdc06, 0xdc06) AM_WRITE(buggychl_bg_scrollx_w)
ADDRESS_MAP_END

/* The schematics for buggy challenge has the wrong sound board schematic attached to it.
  (The schematic is for an unknown taito game, possibly never released.)
   The final buggy challenge sound board is more similar to Fairyland Story sound
   hardware, except it has two YM2149 chips instead of one, and much less ROM space. */
ADDRESS_MAP_START(buggychl_state::sound_map)
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x4800, 0x4801) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x4802, 0x4803) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x4810, 0x481d) AM_DEVWRITE("msm", msm5232_device, write)
	AM_RANGE(0x4820, 0x4820) AM_WRITE(ta7630_volbal_msm_w) /* VOL/BAL   for the 7630 on the MSM5232 output */
	AM_RANGE(0x4830, 0x4830) AM_RAM /* TRBL/BASS for the 7630 on the MSM5232 output */
	AM_RANGE(0x5000, 0x5000) AM_DEVREAD("soundlatch", generic_latch_8_device, read) AM_DEVWRITE("soundlatch2", generic_latch_8_device, write)
	AM_RANGE(0x5001, 0x5001) AM_READ(sound_status_sound_r) AM_DEVWRITE("soundnmi", input_merger_device, in_set<1>)
	AM_RANGE(0x5002, 0x5002) AM_DEVWRITE("soundnmi", input_merger_device, in_clear<1>)
	AM_RANGE(0x5003, 0x5003) AM_WRITE(sound_enable_w) // unclear what this actually controls
	AM_RANGE(0xe000, 0xefff) AM_ROM /* space for diagnostics ROM */
ADDRESS_MAP_END

/******************************************************************************/

// accelerator is 4-bit, we need to convert it here so that it doesn't clash with other inputs in IN1 (known i/o framework fault)
CUSTOM_INPUT_MEMBER( buggychl_state::pedal_in_r )
{
	return m_pedal_input->read() >> 4;
}


static INPUT_PORTS_START( buggychl )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Game Over Bonus" ) PORT_DIPLOCATION("SW1:1,2")   /* Arks/Flags/Fuel */
	PORT_DIPSETTING(    0x03, "2000/1000/50" )
	PORT_DIPSETTING(    0x02, "1000/500/30" )
	PORT_DIPSETTING(    0x01, "500/200/10" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )         /* 1300 units of fuel */
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )       /* 1200 units of fuel */
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )         /* 1100 units of fuel */
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )      /* 1000 units of fuel */
	PORT_SERVICE_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )        /* Only listed as OFF in the manual */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Start button needed" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" )        /* Only listed as OFF in the manual */
	PORT_DIPNAME( 0x04, 0x04, "Fuel loss (Cheat)") PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Crash only" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )        /* Only listed as OFF in the manual */
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_TOGGLE PORT_NAME("P1 Gear Shift")  /* shift */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Test Button") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, buggychl_state, pedal_in_r, nullptr)

	PORT_START("PEDAL")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00, 0xff) PORT_NAME("P1 Pedal") PORT_SENSITIVITY(100) PORT_KEYDELTA(15)   /* accelerator */

	PORT_START("WHEEL") /* wheel */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_REVERSE
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	256,
	4,
	{ 3*0x800*8, 2*0x800*8, 0x800*8, 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,1,
	RGN_FRAC(1,8),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ RGN_FRAC(1,8)+7, RGN_FRAC(1,8)+6, RGN_FRAC(1,8)+5, RGN_FRAC(1,8)+4, RGN_FRAC(1,8)+3, RGN_FRAC(1,8)+2, RGN_FRAC(1,8)+1, RGN_FRAC(1,8)+0,
			7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0 },
	8
};

static GFXDECODE_START( buggychl )
	GFXDECODE_ENTRY( nullptr,           0, charlayout,   0, 8 ) /* decoded at runtime */
	/* sprites are drawn pixel by pixel by draw_sprites() */
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0, 8 )
GFXDECODE_END


WRITE8_MEMBER(buggychl_state::ta7630_volbal_msm_w)
{
	m_ta7630->set_device_volume(m_msm, data >> 4);
}

WRITE8_MEMBER(buggychl_state::ta7630_volbal_ay1_w)
{
	/* VOL/BAL   for the 7630 on this 8910 output */
	m_ta7630->set_device_volume(m_ay1, data >> 4);
}

WRITE8_MEMBER(buggychl_state::port_b_0_w)
{
	/* TRBL/BASS for the 7630 on this 8910 output */
}

WRITE8_MEMBER(buggychl_state::ta7630_volbal_ay2_w)
{
	/* VOL/BAL   for the 7630 on this 8910 output */
	m_ta7630->set_device_volume(m_ay2, data >> 4);
}

WRITE8_MEMBER(buggychl_state::port_b_1_w)
{
	/* TRBL/BASS for the 7630 on this 8910 output */
}

void buggychl_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 6, &ROM[0x10000], 0x2000);

	save_item(NAME(m_sprite_lookup));
	save_item(NAME(m_sl_bank));
	save_item(NAME(m_bg_clip_on));
	save_item(NAME(m_sky_on));
	save_item(NAME(m_sprite_color_base));
	save_item(NAME(m_bg_scrollx));
}

void buggychl_state::machine_reset()
{
	m_sl_bank = 0;
	m_bg_clip_on = 0;
	m_sky_on = 0;
	m_sprite_color_base = 0;
	m_bg_scrollx = 0;
}

MACHINE_CONFIG_START(buggychl_state::buggychl)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL(48'000'000)/8) /* 6 MHz according to schematics, though it can be jumpered for 4MHz as well */
	MCFG_CPU_PROGRAM_MAP(buggychl_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", buggychl_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL(8'000'000)/2) /* 4 MHz according to schematics */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(buggychl_state, irq0_line_hold, ((((XTAL(8'000'000)/2)/2)/256)/64)) // timer irq
	// schematics shows a 61.035 (x2?) Hz, similar to flstory.cpp and other Taito MSM5232 based games.
	// apparently schematics also shows a switch for the timer irq that makes it to run at half speed, no idea where this is located.
	/* audiocpu nmi is caused by (main->sound semaphore)&&(sound_nmi_enabled), identical to bubble bobble. */

	// schematics show a secondary sound z80 cpu as well, running at the same speed as the audiocpu; unclear if actually populated, or if it only existed on a certain hardware release (cocktail deluxe version?)

	MCFG_DEVICE_ADD("bmcu", TAITO68705_MCU,8000000/2)  /* 4 MHz */


	MCFG_WATCHDOG_ADD("watchdog")
	MCFG_WATCHDOG_VBLANK_INIT("screen", 128); // typical Taito 74ls392

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	// derived from ladyfrog.cpp, causes glitches?
//  MCFG_SCREEN_RAW_PARAMS( XTAL(8'000'000), 510, 0, 256, 262, 2*8, 30*8 ) // pixel clock appears to run at 8 MHz
	MCFG_SCREEN_UPDATE_DRIVER(buggychl_state, screen_update_buggychl)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", buggychl)
	MCFG_PALETTE_ADD("palette", 128+128)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_PALETTE_INIT_OWNER(buggychl_state, buggychl)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_GENERIC_LATCH_8_ADD("soundlatch")
	MCFG_GENERIC_LATCH_DATA_PENDING_CB(DEVWRITELINE("soundnmi", input_merger_device, in_w<0>))

	MCFG_INPUT_MERGER_ALL_HIGH("soundnmi")
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(INPUTLINE("audiocpu", INPUT_LINE_NMI))

	MCFG_GENERIC_LATCH_8_ADD("soundlatch2")

	MCFG_TA7630_ADD("ta7630")

	MCFG_SOUND_ADD("ay1", YM2149, XTAL(8'000'000)/4)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(buggychl_state, ta7630_volbal_ay1_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(buggychl_state, port_b_0_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("ay2", YM2149, XTAL(8'000'000)/4)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(buggychl_state, ta7630_volbal_ay2_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(buggychl_state, port_b_1_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("msm", MSM5232, XTAL(8'000'000)/4)
	MCFG_MSM5232_SET_CAPACITORS(0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6) /* default 0.39 uF capacitors (not verified) */
	MCFG_SOUND_ROUTE(0, "mono", 1.0)    // pin 28  2'-1
	MCFG_SOUND_ROUTE(1, "mono", 1.0)    // pin 29  4'-1
	MCFG_SOUND_ROUTE(2, "mono", 1.0)    // pin 30  8'-1
	MCFG_SOUND_ROUTE(3, "mono", 1.0)    // pin 31 16'-1
	MCFG_SOUND_ROUTE(4, "mono", 1.0)    // pin 36  2'-2
	MCFG_SOUND_ROUTE(5, "mono", 1.0)    // pin 35  4'-2
	MCFG_SOUND_ROUTE(6, "mono", 1.0)    // pin 34  8'-2
	MCFG_SOUND_ROUTE(7, "mono", 1.0)    // pin 33 16'-2
	// pin 1 SOLO  8'       not mapped
	// pin 2 SOLO 16'       not mapped
	// pin 22 Noise Output  not mapped
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( buggychl )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "a22-04-2.23", 0x00000, 0x4000, CRC(16445a6a) SHA1(5ce7b0b1aeb3b6cd400965467f913558f39c251f) )
	ROM_LOAD( "a22-05-2.22", 0x04000, 0x4000, CRC(d57430b2) SHA1(3e5b8c21a342d8e26c12a78535748073bc5b8742) )
	ROM_LOAD( "a22-01.3",    0x10000, 0x4000, CRC(af3b7554) SHA1(fd4f5a6cf9253f64c7e86d566802a02baae3b379) ) /* banked */
	ROM_LOAD( "a22-02.2",    0x14000, 0x4000, CRC(b8a645fb) SHA1(614a0656dee0cfa1d7e16ec1e0138a423ecaf18b) ) /* banked */
	ROM_LOAD( "a22-03.1",    0x18000, 0x4000, CRC(5f45d469) SHA1(3a1b9ab2d57c06bfffb1271583944c90d3f6b5a2) ) /* banked */

	ROM_REGION( 0x10000, "audiocpu", 0 )  /* sound Z80 */
	ROM_LOAD( "a22-24.28",   0x00000, 0x4000, CRC(1e7f841f) SHA1(2dc0787b08d32acb78291b689c02dbb83d04d08c) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )  /* 8k for the microcontroller */
	ROM_LOAD( "a22-19.31",   0x00000, 0x0800, CRC(06a71df0) SHA1(28183e6769e1471e7f28dc2a9f5b54e14b7ef339) )

	ROM_REGION( 0x20000, "gfx1", 0 )    /* sprites */
	ROM_LOAD( "a22-06.111",  0x00000, 0x4000, CRC(1df91b17) SHA1(440d33bf984042fb4eac8f17bb385992ccdc6113) )
	ROM_LOAD( "a22-07.110",  0x04000, 0x4000, CRC(2f0ab9b7) SHA1(07b98e23d12da834d522e29fe7891503dc258b05) )
	ROM_LOAD( "a22-08.109",  0x08000, 0x4000, CRC(49cb2134) SHA1(f9998617c097b90be7257ba6fc1e46ff9e1f8916) )
	ROM_LOAD( "a22-09.108",  0x0c000, 0x4000, CRC(e682e200) SHA1(3e2b5dd97e4f522f208d331f6903c69d49555b61) )
	ROM_LOAD( "a22-10.107",  0x10000, 0x4000, CRC(653b7e25) SHA1(70c69288438caf6725c6d96ff75cdc82af005b2b) )
	ROM_LOAD( "a22-11.106",  0x14000, 0x4000, CRC(8057b55c) SHA1(9eeb4980cb14fb1c9b6f3aeff4e0aef1338fc71c) )
	ROM_LOAD( "a22-12.105",  0x18000, 0x4000, CRC(8b365b24) SHA1(a306c1f6fe1f5563602ab424f1b4f6ac17d1e47d) )
	ROM_LOAD( "a22-13.104",  0x1c000, 0x4000, CRC(2c6d68fe) SHA1(9e1a0e44ae2b9986d0ebff49a0fd4df3e8a7f4e7) )

	ROM_REGION( 0x4000, "gfx2", 0 ) /* sprite zoom tables */
	ROM_LOAD( "a22-14.59",   0x0000, 0x2000, CRC(a450b3ef) SHA1(42646bfaed19ea01ffe06996bb6c2fd6c70076d6) ) /* vertical */
	ROM_LOAD( "a22-15.115",  0x2000, 0x1000, CRC(337a0c14) SHA1(2aa6814f74497c5c55bf7098d7f6f5508845e36c) ) /* horizontal */
	ROM_LOAD( "a22-16.116",  0x3000, 0x1000, CRC(337a0c14) SHA1(2aa6814f74497c5c55bf7098d7f6f5508845e36c) ) /* horizontal */
ROM_END

ROM_START( buggychlt )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "bu04.bin",    0x00000, 0x4000, CRC(f90ab854) SHA1(d4536c98be35de3d888548e2de15f8435ca4f08c) )
	ROM_LOAD( "bu05.bin",    0x04000, 0x4000, CRC(543d0949) SHA1(b7b0b0319f5376e7cfcfd0e8a4fa6fea566e0206) )
	ROM_LOAD( "a22-01.3",    0x10000, 0x4000, CRC(af3b7554) SHA1(fd4f5a6cf9253f64c7e86d566802a02baae3b379) ) /* banked */
	ROM_LOAD( "a22-02.2",    0x14000, 0x4000, CRC(b8a645fb) SHA1(614a0656dee0cfa1d7e16ec1e0138a423ecaf18b) ) /* banked */
	ROM_LOAD( "a22-03.1",    0x18000, 0x4000, CRC(5f45d469) SHA1(3a1b9ab2d57c06bfffb1271583944c90d3f6b5a2) ) /* banked */

	ROM_REGION( 0x10000, "audiocpu", 0 )  /* sound Z80 */
	ROM_LOAD( "a22-24.28",   0x00000, 0x4000, CRC(1e7f841f) SHA1(2dc0787b08d32acb78291b689c02dbb83d04d08c) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )  /* 8k for the microcontroller */
	ROM_LOAD( "a22-19.31",   0x00000, 0x0800, CRC(06a71df0) SHA1(28183e6769e1471e7f28dc2a9f5b54e14b7ef339) )

	ROM_REGION( 0x20000, "gfx1", 0 )    /* sprites */
	ROM_LOAD( "a22-06.111",  0x00000, 0x4000, CRC(1df91b17) SHA1(440d33bf984042fb4eac8f17bb385992ccdc6113) )
	ROM_LOAD( "a22-07.110",  0x04000, 0x4000, CRC(2f0ab9b7) SHA1(07b98e23d12da834d522e29fe7891503dc258b05) )
	ROM_LOAD( "a22-08.109",  0x08000, 0x4000, CRC(49cb2134) SHA1(f9998617c097b90be7257ba6fc1e46ff9e1f8916) )
	ROM_LOAD( "a22-09.108",  0x0c000, 0x4000, CRC(e682e200) SHA1(3e2b5dd97e4f522f208d331f6903c69d49555b61) )
	ROM_LOAD( "a22-10.107",  0x10000, 0x4000, CRC(653b7e25) SHA1(70c69288438caf6725c6d96ff75cdc82af005b2b) )
	ROM_LOAD( "a22-11.106",  0x14000, 0x4000, CRC(8057b55c) SHA1(9eeb4980cb14fb1c9b6f3aeff4e0aef1338fc71c) )
	ROM_LOAD( "a22-12.105",  0x18000, 0x4000, CRC(8b365b24) SHA1(a306c1f6fe1f5563602ab424f1b4f6ac17d1e47d) )
	ROM_LOAD( "a22-13.104",  0x1c000, 0x4000, CRC(2c6d68fe) SHA1(9e1a0e44ae2b9986d0ebff49a0fd4df3e8a7f4e7) )

	ROM_REGION( 0x4000, "gfx2", 0 ) /* sprite zoom tables */
	ROM_LOAD( "a22-14.59",   0x0000, 0x2000, CRC(a450b3ef) SHA1(42646bfaed19ea01ffe06996bb6c2fd6c70076d6) ) /* vertical */
	ROM_LOAD( "a22-15.115",  0x2000, 0x1000, CRC(337a0c14) SHA1(2aa6814f74497c5c55bf7098d7f6f5508845e36c) ) /* horizontal */
	ROM_LOAD( "a22-16.116",  0x3000, 0x1000, CRC(337a0c14) SHA1(2aa6814f74497c5c55bf7098d7f6f5508845e36c) ) /* horizontal */
ROM_END


GAMEL( 1984, buggychl, 0,        buggychl, buggychl, buggychl_state, 0, ROT270, "Taito Corporation",                  "Buggy Challenge",          MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE, layout_buggychl )
GAMEL( 1984, buggychlt,buggychl, buggychl, buggychl, buggychl_state, 0, ROT270, "Taito Corporation (Tecfri license)", "Buggy Challenge (Tecfri)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE, layout_buggychl )
