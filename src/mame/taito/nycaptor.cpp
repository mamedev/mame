// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/***************************************************************************
    N.Y. Captor - Taito '85

     Driver by Tomasz Slanina
****************************************************************************
  Hardware similar to Fairyland Story
  Cycle Shooting (Taito '86) is running on (almost) the same hardware
  as NY Captor, but MCU dump is missing.


What's new :
 - added bootlegs - Bronx and Colt
 - Cycle Shooting added (bigger VRAM than nycpator, different (unknown yet) banking and
   gfx control , ROMs probably are wrong mapped, gfx too)
 - Sub CPU halt (cpu #0 ,$d001)
 - Improved communication between main cpu and sound cpu ($d400 cpu#0 , $d000 sound cpu)


To do :
 - REAL bg/sprite priority system
 - cpu #0 , $d806 - only bits 0,1,2 used , something with sound control
                    (code at $62x)
 - better sound emulation (mixing ?)
 - unknown R/W
 - 13th bit in color defs (priority? bit is set for all colors in spot1,
                           for 3/4 screen cols in spot2 etc)
 - handle coin counters and coin lockout

Notes :

 $d000 - MCU data read/write
 $d001 - Watchdog ?
 $d002 - generic control  ----x--- ROM Bank

 $d800 - Dip 0            ------xx Bonus life
                                00 150000 300000 300000
                                01 100000 300000 300000
                                10  20000  80000  80000
                                11  50000 150000 200000
                          -----x-- Unknown
                          ---xx--- Lives
                             00    2
                             01    1
                             10    3
                             11    5
                          --x----- Free Play
                            0      Yes
                            1      No
                          -x------ Allow Continue
                           0       No
                           1       Yes
                          x------- Demo Sounds
                          0        Off
                          1        On

 $d801 - Dip 1            Coinage

 $d802 - Dip 2            -------x Freeze
                                 0 On
                                 1 Off
                          ------x- Training Spot
                                0  No
                                1  Yes
                          ----xx-- Difficulty
                              00   Hardest
                              01   Normal
                              10   Hard
                              11   Easy
                          ---x---- Display Coinage
                             0     No
                             1     Yes
                          --x----- Reset Damage
                            0      Every 4 Stages
                            1      Every Stage
                          -x------ No Hit
                           0       Yes
                           1       No
                          x------- Coin Slots
                          0        1
                          1        2

 $d803 - Input Port 1     -------x Start1
                          ------x- Unused
                          -----x-- Service1
                          ----x--- Tilt
                          ---x---- Coin1
                          --x----- Coin2
                          xx------ Unknown

 $d804 - Input Port 2     -------x Button1
                          xxxxxxx- Unused

 $d805 - MCU Status read  ------x- MCU has sent data to the main cpu

 $d806 - Unknown

 $d807 - MCU Status read  -------x MCU is ready to receive data from main cpu


RAM :

 $e18e - sync between 1st and 2nd Z80
 $e18d - same as a bove

 $e19d - coinage A (Word)
 $e19f - coinage B (Word)

 $e298 - spot
 $e299 - lives
 $e2a1 - damage
 $e2a2 - stage

 $e39c - video control byte stored here
 $e39d - generic control value
 $e39f - gun position update
 $e397 - same as above ?

 $e3a0 - gun Y position ($18 - $f0 = visible area)
 $e3a1 - gun X position ($08 - $c0 = visible area)
 $e3a2 - shot trigger

 $e3a8 - timer for bullets reload
 $e3aa - bullets

 $df00 R - x - beam counter
 $df01 R - y - beam counter
 $df02 R - bit 0 - beam detector

 $df03 W - video control byte


Stephh's additional notes (based on the game Z80 code and some tests) :

1) 'nycaptor' and clones

1a) 'nycaptor'

  - You need to press SERVICE1 while resetting the game to enter the "test mode".
    Note that the grid is always displayed, so it's IMPOSSIBLE to see what's in it 8(
  - When "Infinite Bullets" is set to ON, there is no timer to reload the bullets.
  - When "No Hit" Dip Switch is ON, damage is still incremented, but there are
    no test made if it reaches the limit (0x0a).
  - DSWC bit 2 also determines the precision of a shot
    (= range around a bullet where the enemies can be killed) :
      * 0 : small range (0x0a or 0x10)
      * 1 : high  range (0x0c or 0x12)
  - Even when "Coin Slots" is set to "1", pressing COIN2 will be based on "Coin B" settings.

1b) 'colt'

  - Due to patched code at 0x9fd, lives (BCD coded) settings are different than in 'nycaptor'.
    Display at start will be lives-1, but it will also display "0" when you select 100 lives.
  - The game is much harder than 'nycaptor' :
      * Due to the 4 'nop' instructions at 0x0b03, you can't get any extra life.
      * Due to patched code from 0x0bea to 0x0bf4, you lose a life as soon as you get hit
        by en enemy or when you shoot at an innocent.
        Furthermore, shooting birds kills you instead of recovering 3 "steps" of damage !
  - I can't tell if it's an ingame bug of this bootleg, but the game hangs after bonus stage
    (level 4) instead of looping back to level 1 with higher difficulty.
    The game also freezes sometimes for some unknown reasons.

2) 'cyclshtg' and clones

2a) 'cyclshtg'

  - Lives (BCD coded) settings are not read from MCU, but from table at 0x0fee.
  - Even if it isn't mentioned in the manual, DSWB bit 7 allows you to reset damage to 0
    at the end of a level (check code at 0x328d).
  - When "Infinite Bullets" is set to ON, there is no timer to reload the bullets.
    However, it's hard to notice as you don't see an indicator as in 'nycaptor'.
  - There is a leftover from 'nycaptor' in CPU1 which reads DSWA bits 0 and 1 to determine
    "Bonus Lives" settings (code at 0x009b). These values are overwritten via code in CPU 0.

2b) 'bronx'

  - Lives (BCD coded) settings are different than in 'cyclshtg' (table at 0x0fea).
  - The game is much harder than 'cyclshtg' :
      * You start with less lives with default settings (2 instead of 3).
      * Due to the 'add  a,$00' instruction at 0x1131, you can't get any extra life
        (when you get enough points, or even when you hit the "1UP" bonus).
      * Due to extra code at 0x3062, you start the game as if you had already sustained
        8 "steps" of damage. However, when you end a level, if "Damage Reset" Dip Switch
        is ON, your damage will be reset to 0.
        Furthermore, the game starts as if you had completed the 6 stages 8 times.
      * Due to the 'ld   (ix+$0e),$08' instruction at 0x32b1, you also start the
        other lives as if you had already sustained 8 "steps" of damage. However, again,
        when you end a level, if "Damage Reset" Dip Switch is ON, your damage will be reset to 0.

***************************************************************************/

#include "emu.h"
#include "nycaptor.h"
#include "taitoipt.h"

#include "cpu/m6805/m6805.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "screen.h"
#include "speaker.h"


//#define USE_MCU


void nycaptor_state::sub_cpu_halt_w(uint8_t data)
{
	m_subcpu->set_input_line(INPUT_LINE_HALT, (data) ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t nycaptor_state::nycaptor_b_r()
{
	return 1;
}

uint8_t nycaptor_state::nycaptor_by_r()
{
	int port = ioport("LIGHTY")->read();

	if (m_gametype == 1)
		port = 255 - port;

	return port - 8;
}

uint8_t nycaptor_state::nycaptor_bx_r()
{
	return (ioport("LIGHTX")->read() + 0x27) | 1;
}


void nycaptor_state::sound_cpu_reset_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data&1 )? ASSERT_LINE : CLEAR_LINE);
}

uint8_t nycaptor_state::nycaptor_mcu_status_r1()
{
	/* bit 1 = when 1, mcu has sent data to the main cpu */
	return (CLEAR_LINE != m_bmcu->mcu_semaphore_r()) ? 2 : 0;
}

uint8_t nycaptor_state::nycaptor_mcu_status_r2()
{
	/* bit 0 = when 1, mcu is ready to receive data from main cpu */
	return (CLEAR_LINE != m_bmcu->host_semaphore_r()) ? 0 : 1;
}

uint8_t nycaptor_state::sound_status_r()
{
	return (m_soundlatch->pending_r() ? 1 : 0) | (m_soundlatch2->pending_r() ? 2 : 0);
}



void nycaptor_state::nmi_disable_w(uint8_t data)
{
	m_soundnmi->in_w<1>(0);
}

void nycaptor_state::nmi_enable_w(uint8_t data)
{
	m_soundnmi->in_w<1>(1);
}

void nycaptor_state::unk_w(uint8_t data)
{
}

uint8_t nycaptor_state::nycaptor_generic_control_r()
{
	return m_generic_control_reg;
}

void nycaptor_state::nycaptor_generic_control_w(uint8_t data)
{
	m_generic_control_reg = data;
	membank("bank1")->set_entry((data&0x08)>>3);
}

void nycaptor_state::nycaptor_master_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xc7ff).ram().w(FUNC(nycaptor_state::nycaptor_videoram_w)).share("videoram");
	map(0xd000, 0xd000).rw(m_bmcu, FUNC(taito68705_mcu_device::data_r), FUNC(taito68705_mcu_device::data_w));
	map(0xd001, 0xd001).w(FUNC(nycaptor_state::sub_cpu_halt_w));
	map(0xd002, 0xd002).rw(FUNC(nycaptor_state::nycaptor_generic_control_r), FUNC(nycaptor_state::nycaptor_generic_control_w));   /* bit 3 - memory bank at 0x8000-0xbfff */
	map(0xd400, 0xd400).r(m_soundlatch2, FUNC(generic_latch_8_device::read)).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xd401, 0xd401).nopr();
	map(0xd403, 0xd403).w(FUNC(nycaptor_state::sound_cpu_reset_w));
	map(0xd800, 0xd800).portr("DSWA");
	map(0xd801, 0xd801).portr("DSWB");
	map(0xd802, 0xd802).portr("DSWC");
	map(0xd803, 0xd803).portr("IN0");
	map(0xd804, 0xd804).portr("IN1");
	map(0xd805, 0xd805).r(FUNC(nycaptor_state::nycaptor_mcu_status_r1));
	map(0xd806, 0xd806).r(FUNC(nycaptor_state::sound_status_r));
	map(0xd807, 0xd807).r(FUNC(nycaptor_state::nycaptor_mcu_status_r2));
	map(0xdc00, 0xdc9f).ram().share("spriteram");
	map(0xdca0, 0xdcbf).ram().w(FUNC(nycaptor_state::nycaptor_scrlram_w)).share("scrlram");
	map(0xdce1, 0xdce1).nopw();
	map(0xdd00, 0xdeff).rw(FUNC(nycaptor_state::nycaptor_palette_r), FUNC(nycaptor_state::nycaptor_palette_w));
	map(0xdf03, 0xdf03).rw(FUNC(nycaptor_state::nycaptor_gfxctrl_r), FUNC(nycaptor_state::nycaptor_gfxctrl_w));
	map(0xe000, 0xffff).ram().share("sharedram");
}

void nycaptor_state::nycaptor_slave_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram().w(FUNC(nycaptor_state::nycaptor_videoram_w)).share("videoram");
	map(0xd800, 0xd800).portr("DSWA");
	map(0xd801, 0xd801).portr("DSWB");
	map(0xd802, 0xd802).portr("DSWC");
	map(0xd803, 0xd803).portr("IN0");
	map(0xd804, 0xd804).portr("IN1");
	map(0xdc00, 0xdc9f).ram().share("spriteram");
	map(0xdca0, 0xdcbf).w(FUNC(nycaptor_state::nycaptor_scrlram_w)).share("scrlram");

	map(0xdd00, 0xdeff).rw(FUNC(nycaptor_state::nycaptor_palette_r), FUNC(nycaptor_state::nycaptor_palette_w));
	map(0xdf00, 0xdf00).r(FUNC(nycaptor_state::nycaptor_bx_r));
	map(0xdf01, 0xdf01).r(FUNC(nycaptor_state::nycaptor_by_r));
	map(0xdf02, 0xdf02).r(FUNC(nycaptor_state::nycaptor_b_r));
	map(0xdf03, 0xdf03).r(FUNC(nycaptor_state::nycaptor_gfxctrl_r)).nopw();/* ? gfx control ? */
	map(0xe000, 0xffff).ram().share("sharedram");
}

void nycaptor_state::sound_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xc801).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0xc802, 0xc803).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0xc900, 0xc90d).w(m_msm, FUNC(msm5232_device::write));
	map(0xca00, 0xca00).nopw();
	map(0xcb00, 0xcb00).nopw();
	map(0xcc00, 0xcc00).nopw();
	map(0xd000, 0xd000).r(m_soundlatch, FUNC(generic_latch_8_device::read)).w(m_soundlatch2, FUNC(generic_latch_8_device::write));
	map(0xd200, 0xd200).nopr().w(FUNC(nycaptor_state::nmi_enable_w));
	map(0xd400, 0xd400).w(FUNC(nycaptor_state::nmi_disable_w));
	map(0xd600, 0xd600).w("dac", FUNC(dac_byte_interface::data_w)); //otherwise no girl's scream in cycle shooting, see MT03975
	map(0xe000, 0xefff).noprw();
}


/* Cycle Shooting */


uint8_t nycaptor_state::cyclshtg_mcu_status_r()
{
	return 0xff;
}

uint8_t nycaptor_state::cyclshtg_mcu_r()
{
	return 7;
}

void nycaptor_state::cyclshtg_mcu_w(uint8_t data)
{
}

uint8_t nycaptor_state::cyclshtg_mcu_status_r1()
{
	return machine().rand();
}

void nycaptor_state::cyclshtg_generic_control_w(uint8_t data)
{
	m_generic_control_reg = data;
	membank("bank1")->set_entry((data >> 2) & 3);

	// shared palette data gets overwritten in colt without this
	if (m_gametype == 2)
		m_subcpu->set_input_line(INPUT_LINE_RESET, BIT(data, 1) ? CLEAR_LINE : ASSERT_LINE);
}


void nycaptor_state::cyclshtg_master_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xcfff).ram().w(FUNC(nycaptor_state::nycaptor_videoram_w)).share("videoram");
	map(0xd000, 0xd000).rw(FUNC(nycaptor_state::cyclshtg_mcu_r), FUNC(nycaptor_state::cyclshtg_mcu_w));
	map(0xd001, 0xd001).w(FUNC(nycaptor_state::sub_cpu_halt_w));
	map(0xd002, 0xd002).rw(FUNC(nycaptor_state::nycaptor_generic_control_r), FUNC(nycaptor_state::cyclshtg_generic_control_w));
	map(0xd400, 0xd400).r(m_soundlatch2, FUNC(generic_latch_8_device::read)).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xd403, 0xd403).w(FUNC(nycaptor_state::sound_cpu_reset_w));
	map(0xd800, 0xd800).portr("DSWA");
	map(0xd801, 0xd801).portr("DSWB");
	map(0xd802, 0xd802).portr("DSWC");
	map(0xd803, 0xd803).portr("IN0");
	map(0xd804, 0xd804).portr("IN1");
	map(0xd805, 0xd805).r(FUNC(nycaptor_state::cyclshtg_mcu_status_r));
	map(0xd806, 0xd806).r(FUNC(nycaptor_state::sound_status_r));
	map(0xd807, 0xd807).r(FUNC(nycaptor_state::cyclshtg_mcu_status_r));
	map(0xdc00, 0xdc9f).ram().share("spriteram");
	map(0xdca0, 0xdcbf).ram().w(FUNC(nycaptor_state::nycaptor_scrlram_w)).share("scrlram");
	map(0xdce1, 0xdce1).nopw();
	map(0xdd00, 0xdeff).rw(FUNC(nycaptor_state::nycaptor_palette_r), FUNC(nycaptor_state::nycaptor_palette_w));
	map(0xdf03, 0xdf03).rw(FUNC(nycaptor_state::nycaptor_gfxctrl_r), FUNC(nycaptor_state::nycaptor_gfxctrl_w));
	map(0xe000, 0xffff).ram().share("sharedram");
}

void nycaptor_state::cyclshtg_slave_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram().w(FUNC(nycaptor_state::nycaptor_videoram_w)).share("videoram");
	map(0xd800, 0xd800).portr("DSWA");
	map(0xd801, 0xd801).portr("DSWB");
	map(0xd802, 0xd802).portr("DSWC");
	map(0xd803, 0xd803).portr("IN0");
	map(0xd804, 0xd804).portr("IN1");
	map(0xdc00, 0xdc9f).ram().share("spriteram");
	map(0xdca0, 0xdcbf).w(FUNC(nycaptor_state::nycaptor_scrlram_w)).share("scrlram");
	map(0xdd00, 0xdeff).rw(FUNC(nycaptor_state::nycaptor_palette_r), FUNC(nycaptor_state::nycaptor_palette_w));
	map(0xdf00, 0xdf00).r(FUNC(nycaptor_state::nycaptor_bx_r));
	map(0xdf01, 0xdf01).r(FUNC(nycaptor_state::nycaptor_by_r));
	map(0xdf02, 0xdf02).r(FUNC(nycaptor_state::nycaptor_b_r));
	map(0xdf03, 0xdf03).r(FUNC(nycaptor_state::nycaptor_gfxctrl_r));
	map(0xdf03, 0xdf03).nopw();
	map(0xe000, 0xffff).ram().share("sharedram");
}

uint8_t nycaptor_state::unk_r()
{
	return machine().rand();
}

void nycaptor_state::bronx_master_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xcfff).ram().w(FUNC(nycaptor_state::nycaptor_videoram_w)).share("videoram");
	map(0xd000, 0xd000).r(FUNC(nycaptor_state::cyclshtg_mcu_r)).nopw();
	map(0xd001, 0xd001).w(FUNC(nycaptor_state::sub_cpu_halt_w));
	map(0xd002, 0xd002).rw(FUNC(nycaptor_state::nycaptor_generic_control_r), FUNC(nycaptor_state::cyclshtg_generic_control_w));
	map(0xd400, 0xd400).r(m_soundlatch2, FUNC(generic_latch_8_device::read)).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xd401, 0xd401).r(FUNC(nycaptor_state::unk_r));
	map(0xd403, 0xd403).w(FUNC(nycaptor_state::sound_cpu_reset_w));
	map(0xd800, 0xd800).portr("DSWA");
	map(0xd801, 0xd801).portr("DSWB");
	map(0xd802, 0xd802).portr("DSWC");
	map(0xd803, 0xd803).portr("IN0");
	map(0xd804, 0xd804).portr("IN1");
	map(0xd805, 0xd805).r(FUNC(nycaptor_state::cyclshtg_mcu_status_r));
	map(0xd806, 0xd806).r(FUNC(nycaptor_state::sound_status_r));
	map(0xd807, 0xd807).r(FUNC(nycaptor_state::cyclshtg_mcu_status_r));
	map(0xdc00, 0xdc9f).ram().share("spriteram");
	map(0xdca0, 0xdcbf).ram().w(FUNC(nycaptor_state::nycaptor_scrlram_w)).share("scrlram");
	map(0xdd00, 0xdeff).rw(FUNC(nycaptor_state::nycaptor_palette_r), FUNC(nycaptor_state::nycaptor_palette_w));
	map(0xdf03, 0xdf03).rw(FUNC(nycaptor_state::nycaptor_gfxctrl_r), FUNC(nycaptor_state::nycaptor_gfxctrl_w));
	map(0xe000, 0xffff).ram().share("sharedram");
}

void nycaptor_state::bronx_slave_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xcfff).ram().w(FUNC(nycaptor_state::nycaptor_videoram_w)).share("videoram");
	map(0xd800, 0xd800).portr("DSWA");
	map(0xd801, 0xd801).portr("DSWB");
	map(0xd802, 0xd802).portr("DSWC");
	map(0xd803, 0xd803).portr("IN0");
	map(0xd804, 0xd804).portr("IN1");
	map(0xd805, 0xd805).r(FUNC(nycaptor_state::cyclshtg_mcu_status_r1));
	map(0xd807, 0xd807).r(FUNC(nycaptor_state::cyclshtg_mcu_status_r));
	map(0xdc00, 0xdc9f).ram().share("spriteram");
	map(0xdca0, 0xdcbf).w(FUNC(nycaptor_state::nycaptor_scrlram_w)).share("scrlram");
	map(0xdd00, 0xdeff).rw(FUNC(nycaptor_state::nycaptor_palette_r), FUNC(nycaptor_state::nycaptor_palette_w));
	map(0xdf00, 0xdf00).r(FUNC(nycaptor_state::nycaptor_bx_r));
	map(0xdf01, 0xdf01).r(FUNC(nycaptor_state::nycaptor_by_r));
	map(0xdf02, 0xdf02).r(FUNC(nycaptor_state::nycaptor_b_r));
	map(0xdf03, 0xdf03).rw(FUNC(nycaptor_state::nycaptor_gfxctrl_r), FUNC(nycaptor_state::nycaptor_gfxctrl_w));
	map(0xe000, 0xffff).ram().share("sharedram");
}

void nycaptor_state::bronx_slave_io_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("user1", 0);
}


/* verified from Z80 code */
static INPUT_PORTS_START( nycaptor )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SWA:1,2")  // table at 0x00e5 in CPU1 - see notes for 'colt'
	PORT_DIPSETTING(    0x02, "20k 80k 80k+" )
	PORT_DIPSETTING(    0x03, "50k 150k 200k+" )
	PORT_DIPSETTING(    0x01, "100k 300k 300k+" )
	PORT_DIPSETTING(    0x00, "150k 300k 300k+" )
	PORT_DIPNAME( 0x04, 0x04, "Infinite Bullets")           PORT_DIPLOCATION("SWA:3")  // see notes
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )            PORT_DIPLOCATION("SWA:4,5")  // values are read from the MCU
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SWB:1,2,3,4")  // DIPs are reversed from usual (e.g. 1C 1C = all on) but confirmed correct from manual
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
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SWB:5,6,7,8")  // DIPs are reversed from usual (e.g. 1C 1C = all on) but confirmed correct from manual
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

	PORT_START("DSWC")
	PORT_DIPNAME( 0x01, 0x01, "Freeze" )                    PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Training Spot" )             PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SWC:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" )           PORT_DIPLOCATION("SWC:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Reset Damage" )              PORT_DIPLOCATION("SWC:6")
	PORT_DIPSETTING(    0x20, "Every Stage" )
	PORT_DIPSETTING(    0x00, "Every 4 Stages" )
	PORT_DIPNAME( 0x40, 0x40, "No Hit (Cheat)")             PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )                PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )            // IPT_START2 in some similar Taito games (eg: 'flstory')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )            // "I/O ERROR" if active - code at 0x083d
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )            // "I/O ERROR" if active - code at 0x083d

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LIGHTX")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHTY")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)
INPUT_PORTS_END

/* verified from Z80 code */
static INPUT_PORTS_START( colt )
	PORT_INCLUDE( nycaptor )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )    PORT_DIPLOCATION("SWA:4,5")  // see notes
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x00, "100" )
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( cyclshtg )
	PORT_START("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SWA:1")
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SWA:2")
	TAITO_DSWA_BITS_2_TO_3_LOC (SWA)
	TAITO_COINAGE_JAPAN_OLD_LOC (SWA)  // coinage B isn't mentioned in the manual

	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC (SWB)
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SWB:3,4")  // table at 0x100f - see notes for 'bronx'
	PORT_DIPSETTING(    0x0c, "150k 350k 200k+" )
	PORT_DIPSETTING(    0x08, "200k 500k 300k+" )
	PORT_DIPSETTING(    0x04, "300k 700k 400k+" )
	PORT_DIPSETTING(    0x00, "400k 900k 500k+" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )            PORT_DIPLOCATION("SWB:5,6")  // see notes
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:7" )
	PORT_DIPNAME( 0x80, 0x80, "Reset Damage (Cheat)" )      PORT_DIPLOCATION("SWB:8")  // see notes
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SWC:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SWC:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWC:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SWC:4" )
	PORT_DIPNAME( 0x10, 0x10, "Infinite Bullets" )          PORT_DIPLOCATION("SWC:5")  // see notes
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWC:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWC:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWC:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )            // IPT_START2 in some similar Taito games (eg: 'flstory')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LIGHTX")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHTY")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)
INPUT_PORTS_END

/* verified from Z80 code */
static INPUT_PORTS_START( bronx )
	PORT_INCLUDE( cyclshtg )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )   PORT_DIPLOCATION("SWB:5,6")  // see notes
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0,
			16*8+3, 16*8+2, 16*8+1, 16*8+0, 16*8+8+3, 16*8+8+2, 16*8+8+1, 16*8+8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16 },
	64*8
};

static GFXDECODE_START( gfx_nycaptor )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 16 )//16 colors
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 256, 16 )//palette 2, 16 colors
GFXDECODE_END




void nycaptor_state::machine_start()
{
	if (m_gametype == 0)
		membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x4000);
	else
		membank("bank1")->configure_entries(0, 4, memregion("maincpu")->base() + 0x10000, 0x4000);

	save_item(NAME(m_generic_control_reg));

	save_item(NAME(m_char_bank));
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_gfxctrl));
}

void nycaptor_state::machine_reset()
{
//  MACHINE_RESET_CALL_MEMBER(ta7630);

	m_generic_control_reg = 0;

	m_char_bank = 0;
	m_palette_bank = 0;
	m_gfxctrl = 0;
}

void nycaptor_state::nycaptor(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 8000000/2);      /* ??? */
	m_maincpu->set_addrmap(AS_PROGRAM, &nycaptor_state::nycaptor_master_map);
	m_maincpu->set_vblank_int("screen", FUNC(nycaptor_state::irq0_line_hold));

	Z80(config, m_subcpu, 8000000/2);
	m_subcpu->set_addrmap(AS_PROGRAM, &nycaptor_state::nycaptor_slave_map);
	m_subcpu->set_vblank_int("screen", FUNC(nycaptor_state::irq0_line_hold));   /* IRQ generated by ??? */

	Z80(config, m_audiocpu, 8000000/2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &nycaptor_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(nycaptor_state::irq0_line_hold), attotime::from_hz(2*60));  /* IRQ generated by ??? */

	TAITO68705_MCU(config, m_bmcu, 2000000);

	config.set_maximum_quantum(attotime::from_hz(6000));  /* 100 CPU slices per frame - a high value to ensure proper synchronization of the CPUs */

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(nycaptor_state::screen_update_nycaptor));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_nycaptor);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 512);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch).data_pending_callback().set(m_soundnmi, FUNC(input_merger_device::in_w<0>));

	INPUT_MERGER_ALL_HIGH(config, m_soundnmi).output_handler().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	GENERIC_LATCH_8(config, m_soundlatch2);

	ay8910_device &ay1(AY8910(config, "ay1", 8000000/4));
	ay1.port_a_write_callback().set(FUNC(nycaptor_state::unk_w));
	ay1.port_b_write_callback().set(FUNC(nycaptor_state::unk_w));
	ay1.add_route(ALL_OUTPUTS, "speaker", 0.15);

	ay8910_device &ay2(AY8910(config, "ay2", 8000000/4));
	ay2.port_a_write_callback().set(FUNC(nycaptor_state::unk_w));
	ay2.port_b_write_callback().set(FUNC(nycaptor_state::unk_w));
	ay2.add_route(ALL_OUTPUTS, "speaker", 0.15);

	MSM5232(config, m_msm, 2000000);
	m_msm->set_capacitors(1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6); /* 1 uF capacitors (match the sample, not verified, standard) */
	m_msm->add_route(0, "speaker", 1.0);   // pin 28  2'-1
	m_msm->add_route(1, "speaker", 1.0);   // pin 29  4'-1
	m_msm->add_route(2, "speaker", 1.0);   // pin 30  8'-1
	m_msm->add_route(3, "speaker", 1.0);   // pin 31 16'-1
	m_msm->add_route(4, "speaker", 1.0);   // pin 36  2'-2
	m_msm->add_route(5, "speaker", 1.0);   // pin 35  4'-2
	m_msm->add_route(6, "speaker", 1.0);   // pin 34  8'-2
	m_msm->add_route(7, "speaker", 1.0);   // pin 33 16'-2
	// pin 1 SOLO  8'       not mapped
	// pin 2 SOLO 16'       not mapped
	// pin 22 Noise Output  not mapped

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
}

void nycaptor_state::cyclshtg(machine_config &config)
{
	Z80(config, m_maincpu, 8000000/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &nycaptor_state::cyclshtg_master_map);
	m_maincpu->set_vblank_int("screen", FUNC(nycaptor_state::irq0_line_hold));

	Z80(config, m_subcpu, 8000000/2);
	m_subcpu->set_addrmap(AS_PROGRAM, &nycaptor_state::cyclshtg_slave_map);
	m_subcpu->set_addrmap(AS_IO, &nycaptor_state::bronx_slave_io_map);
	m_subcpu->set_vblank_int("screen", FUNC(nycaptor_state::irq0_line_hold));

	Z80(config, m_audiocpu, 8000000/2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &nycaptor_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(nycaptor_state::irq0_line_hold), attotime::from_hz(2*60));

#ifdef USE_MCU
	TAITO68705_MCU(config, m_bmcu, 2000000);
#endif

	config.set_maximum_quantum(attotime::from_hz(60));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(nycaptor_state::screen_update_nycaptor));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_nycaptor);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 512);

	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch).data_pending_callback().set(m_soundnmi, FUNC(input_merger_device::in_w<0>));

	INPUT_MERGER_ALL_HIGH(config, m_soundnmi).output_handler().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	GENERIC_LATCH_8(config, m_soundlatch2);

	ay8910_device &ay1(AY8910(config, "ay1", 8000000/4));
	ay1.port_a_write_callback().set(FUNC(nycaptor_state::unk_w));
	ay1.port_b_write_callback().set(FUNC(nycaptor_state::unk_w));
	ay1.add_route(ALL_OUTPUTS, "speaker", 0.15);

	ay8910_device &ay2(AY8910(config, "ay2", 8000000/4));
	ay2.port_a_write_callback().set(FUNC(nycaptor_state::unk_w));
	ay2.port_b_write_callback().set(FUNC(nycaptor_state::unk_w));
	ay2.add_route(ALL_OUTPUTS, "speaker", 0.15);

	MSM5232(config, m_msm, 2000000);
	m_msm->set_capacitors(1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6); /* 1 uF capacitors (match the sample, not verified, standard) */
	m_msm->add_route(0, "speaker", 1.0);   // pin 28  2'-1
	m_msm->add_route(1, "speaker", 1.0);   // pin 29  4'-1
	m_msm->add_route(2, "speaker", 1.0);   // pin 30  8'-1
	m_msm->add_route(3, "speaker", 1.0);   // pin 31 16'-1
	m_msm->add_route(4, "speaker", 1.0);   // pin 36  2'-2
	m_msm->add_route(5, "speaker", 1.0);   // pin 35  4'-2
	m_msm->add_route(6, "speaker", 1.0);   // pin 34  8'-2
	m_msm->add_route(7, "speaker", 1.0);   // pin 33 16'-2
	// pin 1 SOLO  8'       not mapped
	// pin 2 SOLO 16'       not mapped
	// pin 22 Noise Output  not mapped

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
}


void nycaptor_state::bronx(machine_config &config)
{
	Z80(config, m_maincpu, 8000000/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &nycaptor_state::bronx_master_map);
	m_maincpu->set_vblank_int("screen", FUNC(nycaptor_state::irq0_line_hold));

	Z80(config, m_subcpu, 8000000/2);
	m_subcpu->set_addrmap(AS_PROGRAM, &nycaptor_state::bronx_slave_map);
	m_subcpu->set_addrmap(AS_IO, &nycaptor_state::bronx_slave_io_map);
	m_subcpu->set_vblank_int("screen", FUNC(nycaptor_state::irq0_line_hold));

	Z80(config, m_audiocpu, 8000000/2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &nycaptor_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(nycaptor_state::irq0_line_hold), attotime::from_hz(2*60));

	config.set_maximum_quantum(attotime::from_hz(120));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(nycaptor_state::screen_update_nycaptor));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_nycaptor);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 512);

	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch).data_pending_callback().set(m_soundnmi, FUNC(input_merger_device::in_w<0>));

	INPUT_MERGER_ALL_HIGH(config, m_soundnmi).output_handler().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	GENERIC_LATCH_8(config, m_soundlatch2);

	ay8910_device &ay1(AY8910(config, "ay1", 8000000/4));
	ay1.port_a_write_callback().set(FUNC(nycaptor_state::unk_w));
	ay1.port_b_write_callback().set(FUNC(nycaptor_state::unk_w));
	ay1.add_route(ALL_OUTPUTS, "speaker", 0.15);

	ay8910_device &ay2(AY8910(config, "ay2", 8000000/4));
	ay2.port_a_write_callback().set(FUNC(nycaptor_state::unk_w));
	ay2.port_b_write_callback().set(FUNC(nycaptor_state::unk_w));
	ay2.add_route(ALL_OUTPUTS, "speaker", 0.15);

	MSM5232(config, m_msm, 2000000);
	m_msm->set_capacitors(1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6); /* 1 uF capacitors (match the sample, not verified, standard) */
	m_msm->add_route(0, "speaker", 1.0);   // pin 28  2'-1
	m_msm->add_route(1, "speaker", 1.0);   // pin 29  4'-1
	m_msm->add_route(2, "speaker", 1.0);   // pin 30  8'-1
	m_msm->add_route(3, "speaker", 1.0);   // pin 31 16'-1
	m_msm->add_route(4, "speaker", 1.0);   // pin 36  2'-2
	m_msm->add_route(5, "speaker", 1.0);   // pin 35  4'-2
	m_msm->add_route(6, "speaker", 1.0);   // pin 34  8'-2
	m_msm->add_route(7, "speaker", 1.0);   // pin 33 16'-2
	// pin 1 SOLO  8'       not mapped
	// pin 2 SOLO 16'       not mapped
	// pin 22 Noise Output  not mapped

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
}



/***************************************************************************
  Game driver(s)
***************************************************************************/

ROM_START( nycaptor )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "a50_04",   0x00000, 0x4000, CRC(33d971a3) SHA1(8bf6cb8d799739dc6f115d352453af278d58de9a) )
	ROM_LOAD( "a50_03",   0x04000, 0x4000, CRC(8557fa44) SHA1(5639ec2ac21ae94c416c01bd7c0dae722cc14598) )
	ROM_LOAD( "a50_02",   0x10000, 0x4000, CRC(9697b898) SHA1(28b92963264b867ca4452dad7dcbbb8c8247d2f5) )
	ROM_LOAD( "a50_01",   0x14000, 0x4000, CRC(0965f84a) SHA1(22698446f52f5d29632aa982b9be87a9bf86fbef) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "a50_05-2", 0x0000, 0x4000, CRC(7796655a) SHA1(74cf84bf6f4dfee57ef6236280c0d64c1a156ffe) )
	ROM_LOAD( "a50_06",   0x4000, 0x4000, CRC(450d6783) SHA1(e652bf83fb6bdd8152dbafb05bb695259c2619cc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a50_15",   0x0000, 0x4000, CRC(f8a604e5) SHA1(8fae920fd09584b5e5ccd0db8b8934b393a23d50) )
	ROM_LOAD( "a50_16",   0x4000, 0x4000, CRC(fc24e11d) SHA1(ce1a1d7b809fa0f5f5e7a462047374b1b3f621c6) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )
	ROM_LOAD( "a50_17",   0x0000, 0x0800, CRC(69fe08dc) SHA1(9bdac3e835f63bbb8806892169d89f43d447df21) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a50_07",   0x00000, 0x4000, CRC(b3b330cf) SHA1(8330e4831b8d0068c0367417db2e27fded7c56fe) )
	ROM_LOAD( "a50_09",   0x04000, 0x4000, CRC(22e2232e) SHA1(3b43114a5511b00e45cd67073c3f833fcf098fb6) )
	ROM_LOAD( "a50_11",   0x08000, 0x4000, CRC(2c04ad4f) SHA1(a6272f91f583d46dd4a3fe863482b39406c70e97) )
	ROM_LOAD( "a50_13",   0x0c000, 0x4000, CRC(9940e78d) SHA1(8607808e6ff76808ae155d671c3179bfe7d2639b) )
	ROM_LOAD( "a50_08",   0x10000, 0x4000, CRC(3c31be14) SHA1(a1ed382fbd374609022535bd0d78bc9e84edd63b) )
	ROM_LOAD( "a50_10",   0x14000, 0x4000, CRC(ce84dc5a) SHA1(8723310f3e25820ef7490c16759ebcb8354dde85) )
	ROM_LOAD( "a50_12",   0x18000, 0x4000, CRC(3fb4cfa3) SHA1(b1c5f7b0297c59dc93420d31e0ef2b1125dbb9db) )
	ROM_LOAD( "a50_14",   0x1c000, 0x4000, CRC(24b2f1bf) SHA1(4757aec2e4b99ce33d993ce1e19ee46a4eb76e86) )
ROM_END

// note, a mix of a80 and a97 codes, are there multiple versions of this game? the a97 gfx ROM doesn't match the bronx bootleg, so maybe the bootleg is based off the a80 version?
ROM_START( cyclshtg )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "a97_01.i17",   0x00000, 0x4000, CRC(686fac1a) SHA1(46d17cb98f064413bb76c5d869f8061d2771cda0) )
	ROM_LOAD( "a97_02.i16",   0x04000, 0x4000, CRC(48a812f9) SHA1(8ab18cb8d6a8b7ce1ed1a4009f5435ce4b0937b4) )
	ROM_LOAD( "a97_03.u15",   0x10000, 0x4000, BAD_DUMP CRC(67ad3067) SHA1(2e355653e91c093abe7db0a3d55d5a3f95c4a2e3) ) // first half of data is missing compared to bronx (rest not 100% identical when decrypted, so can't use bootleg data)
	ROM_LOAD( "a97_04.u14",   0x14000, 0x4000, BAD_DUMP CRC(804e6445) SHA1(5b6771c5729faf62d5002d090c0b9c5ca5cb9ad6) ) // ^

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "a97_05.u22",   0x0000, 0x4000, CRC(fdc36c4f) SHA1(cae2d3f07c5bd6de9d40ff7d385b999e7dc9ce82) )
	ROM_LOAD( "a80_06.u23",   0x4000, 0x4000, CRC(2769c5ab) SHA1(b8f5a4a8c70c8d37d5e92b37faa0e25b287b3fb2) )

	ROM_REGION( 0x08000, "user1", 0 )
	ROM_LOAD( "a97_06.i24",   0x4000, 0x4000, BAD_DUMP CRC(c0473a54) SHA1(06fa7345a44a72995146e973c2cd7a14499f4310) ) // see above

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a80_16.i26",   0x0000, 0x4000, CRC(ce171a48) SHA1(e5ae9bb22f58c8857737bc6f5317866819a4e4d1) )
	ROM_LOAD( "a80_17.i25",   0x4000, 0x4000, CRC(a90b7bbc) SHA1(bd5c96861a59a1f84bb5032775b1c70efdb7066f) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 )
	ROM_LOAD( "a80_18",       0x0000, 0x0800, NO_DUMP ) /* Missing */

	ROM_REGION( 0x20000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a80_11.u11",   0x00000, 0x4000, CRC(29e1293b) SHA1(106204ec46fae5d3b5e4a4d423bc0e886a637c61) )
	ROM_LOAD( "a80_10.u10",   0x04000, 0x4000, CRC(345f576c) SHA1(fee5b2167bcd0fdc21c0a7b22ffdf7506a24baee) )
	ROM_LOAD( "a80_09.u9",    0x08000, 0x4000, CRC(3ef06dff) SHA1(99bbd32ae89a6becac9e1bb8a34a834d81890444) )
	ROM_LOAD( "a97_07.u8",    0x0c000, 0x4000, CRC(8f2baf57) SHA1(bb4e5b69477c51536dfd95ea59c7281159f3bcc7) )
	ROM_LOAD( "a80_15.u39",   0x10000, 0x4000, CRC(2cefb47d) SHA1(3bef20c9c0c4f9237a327da3cbc9a7bbf63771ea) )
	ROM_LOAD( "a80_14.u34",   0x14000, 0x4000, CRC(91642de8) SHA1(531974fc147d25e9feada89bc82d5df62ec9d446) )
	ROM_LOAD( "a80_13.u33",   0x18000, 0x4000, CRC(96a67c6b) SHA1(3bed4313d6b8d554b35abb1515ab94e78c2718c5) )
	ROM_LOAD( "a80_12.u23",   0x1c000, 0x4000, CRC(9ff04c85) SHA1(a5edc50bbe6e2c976895c97400c75088bc90a1fc) )
ROM_END

/*
Bronx (Cycle Shooting bootleg)
Taito, 1986

The hardware is an almost exact copy of the original Taito hardware,
minus the 68705 Microcontroller :-)

PCB Layout
----------

Top Board

 |----------------------------------|
|-|     VOL        VOL              |
| | TDA2003    TDA2003       LM3900 |
| |                                 |
| |                                 |
| |                                 |
| |            LM3900               |
| |                                 |
| |                                 |
|-|                            M5232|
 |                AY-3-8910         |
 |                    AY-3-8910     |
 |                                  |
 |                                  |
 |                                  |
 |                                  |
 |                             2016 |
 |             Z80  16    17    8MHz|
 |----------------------------------|
Notes:
      Z80 running at 4.000MHz
      M5253 running at 2.000MHz
      AY-3-8910's running at 2.000MHz
      2016 - 2k x8 SRAM (DIP24)


Middle Board

|----------------------------------------------------|
|                   74     Z80   ROM1       04  8MHz |
|          |-|                              74       |
|          | |       139   ROM7  ROM2                |
|          | |                              240      |
|          | |             ROM6  ROM3       155      |
|          | |    245                                |
|          | |             ROM5  ROM4       139      |
|          | |    245                               |-|
|2         | |                                      | |
|2         |-|    175    6264       Z80             | |
|W                                                  | |
|A                                                  | |
|Y             251      251                         | |
|                                                   | |
|              251                                  | |
|                                                   |-|
|              251      251  244    244        393   |
|                                                    |
|                       253  244    PAL        02    |
|                                                    |
|                       251  32     107        00    |
|                            74     74               |
|    DSWC     DSWB     DSWA         244              |
|----------------------------------------------------|
Notes:
      Both Z80s running at 4.000MHz
      6264 - 8k x8 SRAM (DIP28)


Bottom Board

|----------------------------------------------------|
|   18.432MHz                                        |
|                                                    |
|                                            2016    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                         6148                      |-|
|1                                                  | |
|8                        6148                      | |
|W                                                  | |
|A                                                  | |
|Y                                6148              | |
|                                                   | |
|    8        12                  6148              | |
|                                                   |-|
|    9        13                  6148               |
|                                                    |
|    10       14          6148    6148               |
|                                                    |
|    11       15          6148                       |
|                                                    |
|                         6148                       |
|----------------------------------------------------|
Notes:
      2016 - 2k x8 SRAM (DIP24)
      6148 - 1k x4 SRAM (DIP18)
*/

ROM_START( bronx )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1.bin",   0x00000, 0x4000, CRC(399b5063) SHA1(286e0d1a0c1a41060e87f2dadf199f38c787d40f) )
	ROM_LOAD( "2.bin",   0x04000, 0x4000, CRC(3b5f9756) SHA1(7df0e4808799ca6a7809bd9ac3a1689a18ae1cdb) )
	ROM_LOAD( "3.bin",   0x10000, 0x8000, CRC(d2ffd3ce) SHA1(06d237a4aa46e37192bd94e2db361c62c35d469c) )
	ROM_LOAD( "4.bin",   0x18000, 0x8000, CRC(20cf148d) SHA1(49f49f9e58d7aa5690ff828b746ab856c71b0d9c) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "5.bin",   0x0000, 0x4000, CRC(19a1c665) SHA1(38f5f47f22740a75cf0ce6d0406368c2c86e0021) )
	ROM_LOAD( "6.bin",   0x4000, 0x4000, CRC(2769c5ab) SHA1(b8f5a4a8c70c8d37d5e92b37faa0e25b287b3fb2) )

	ROM_REGION( 0x08000, "user1", 0 )
	ROM_LOAD( "7.bin",   0x00000, 0x8000, CRC(463f9f62) SHA1(8e1fa8f78d230d32502422078599e9e9af889a92) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a80_16.i26",   0x0000, 0x4000, CRC(ce171a48) SHA1(e5ae9bb22f58c8857737bc6f5317866819a4e4d1) )
	ROM_LOAD( "a80_17.i25",   0x4000, 0x4000, CRC(a90b7bbc) SHA1(bd5c96861a59a1f84bb5032775b1c70efdb7066f) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a80_11.u11",   0x00000, 0x4000, CRC(29e1293b) SHA1(106204ec46fae5d3b5e4a4d423bc0e886a637c61) )
	ROM_LOAD( "a80_10.u10",   0x04000, 0x4000, CRC(345f576c) SHA1(fee5b2167bcd0fdc21c0a7b22ffdf7506a24baee) )
	ROM_LOAD( "a80_09.u9",    0x08000, 0x4000, CRC(3ef06dff) SHA1(99bbd32ae89a6becac9e1bb8a34a834d81890444) )
	ROM_LOAD( "8.bin",        0x0c000, 0x4000, CRC(2b778d24) SHA1(caca7a18743a4bb657a7c5691d93de0ccb867003) )

	ROM_LOAD( "a80_15.u39",   0x10000, 0x4000, CRC(2cefb47d) SHA1(3bef20c9c0c4f9237a327da3cbc9a7bbf63771ea) )
	ROM_LOAD( "a80_14.u34",   0x14000, 0x4000, CRC(91642de8) SHA1(531974fc147d25e9feada89bc82d5df62ec9d446) )
	ROM_LOAD( "a80_13.u33",   0x18000, 0x4000, CRC(96a67c6b) SHA1(3bed4313d6b8d554b35abb1515ab94e78c2718c5) )
	ROM_LOAD( "a80_12.u23",   0x1c000, 0x4000, CRC(9ff04c85) SHA1(a5edc50bbe6e2c976895c97400c75088bc90a1fc) )
ROM_END

/*
Colt (NY Captor bootleg)
Terminator,1986

The hardware is an almost exact copy of the original Taito hardware,
minus the 68705 Microcontroller :-)
The hardware is exactly the same as the bootleg Cycle Shooting hardware.

PCB Layout
----------

Top Board

 |----------------------------------|
|-|     VOL        VOL              |
| | TDA2003    TDA2003       LM3900 |
| |                                 |
| |                                 |
| |                                 |
| |            LM3900               |
| |                                 |
| |                                 |
|-|                            M5232|
 |                AY-3-8910         |
 |                    AY-3-8910     |
 |                                  |
 |                                  |
 |                                  |
 |                                  |
 |                             2016 |
 |             Z80  15    16    8MHz|
 |----------------------------------|
Notes:
      Z80 running at 4.000MHz
      M5253 running at 2.000MHz
      AY-3-8910's running at 2.000MHz
      2016 - 2k x8 SRAM (DIP24)


Middle Board

|----------------------------------------------------|
|                          Z80     04           8MHz |
|          |-|                                       |
|          | |              *      03                |
|          | |                                       |
|          | |              06     02                |
|          | |                                       |
|          | |              05     01                |
|          | |                                      |-|
|2         | |                                      | |
|2         |-|           6264     Z80               | |
|W                                                  | |
|A                                                  | |
|Y                                                  | |
|                                                   | |
|                                                   | |
|                                                   |-|
|                                                    |
|                                PAL                 |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|    DSWC  DSWB  DSWA                                |
|----------------------------------------------------|
Notes:
      Both Z80s running at 4.000MHz
      6264 - 8k x8 SRAM (DIP28)
      * - socket not populated

Bottom Board

|----------------------------------------------------|
|   18.432MHz                                        |
|                                                    |
|                                            2016    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                         6148                      |-|
|1                                                  | |
|8                        6148                      | |
|W                                                  | |
|A                                                  | |
|Y                                6148              | |
|                                                   | |
|    13       14                  6148              | |
|                                                   |-|
|    11       12                  6148               |
|                                                    |
|    09       10          6148    6148               |
|                                                    |
|    07       08          6148                       |
|                                                    |
|                         6148                       |
|----------------------------------------------------|
Notes:
      2016 - 2k x8 SRAM (DIP24)
      6148 - 1k x4 SRAM (DIP18)

*/
ROM_START( colt )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "04.bin",   0x00000, 0x4000, CRC(dc61fdb2) SHA1(94fdd95082936b2445008aee60381ebe35385b4a) )
	ROM_LOAD( "03.bin",   0x04000, 0x4000, CRC(5835b8b1) SHA1(25a48660f8fb166f996133fb9113d1566dbae281) )
	ROM_LOAD( "02.bin",   0x10000, 0x4000, CRC(89c99a28) SHA1(1a4fdb5c13569699dfbf2bde0aeeb5e7fcc22ef9) )
	ROM_RELOAD(           0x14000, 0x4000)
	ROM_LOAD( "01.bin",   0x18000, 0x4000, CRC(9b0948f3) SHA1(a55e09243640ec56aa22e4b6d47165b02b880eb7) )
	ROM_RELOAD(           0x1c000, 0x4000)

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "05.bin",   0x0000, 0x4000, CRC(2b6e017a) SHA1(60715e1c6fbcdd2c0e114035c342ba587dfc1b4b) )
	ROM_LOAD( "a50_06",   0x4000, 0x4000, CRC(450d6783) SHA1(e652bf83fb6bdd8152dbafb05bb695259c2619cc) )

	ROM_REGION( 0x08000, "user1", ROMREGION_ERASE00 ) //empty

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a50_15",   0x0000, 0x4000, CRC(f8a604e5) SHA1(8fae920fd09584b5e5ccd0db8b8934b393a23d50) )
	ROM_LOAD( "a50_16",   0x4000, 0x4000, CRC(fc24e11d) SHA1(ce1a1d7b809fa0f5f5e7a462047374b1b3f621c6) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a50_07",   0x00000, 0x4000, CRC(b3b330cf) SHA1(8330e4831b8d0068c0367417db2e27fded7c56fe) )
	ROM_LOAD( "a50_09",   0x04000, 0x4000, CRC(22e2232e) SHA1(3b43114a5511b00e45cd67073c3f833fcf098fb6) )
	ROM_LOAD( "a50_11",   0x08000, 0x4000, CRC(2c04ad4f) SHA1(a6272f91f583d46dd4a3fe863482b39406c70e97) )
	ROM_LOAD( "a50_13",   0x0c000, 0x4000, CRC(9940e78d) SHA1(8607808e6ff76808ae155d671c3179bfe7d2639b) )
	ROM_LOAD( "08.bin",   0x10000, 0x4000, CRC(9c0689f3) SHA1(7cecabd03f4973ab97e9dde041fcc9220f7aa812) )
	ROM_LOAD( "a50_10",   0x14000, 0x4000, CRC(ce84dc5a) SHA1(8723310f3e25820ef7490c16759ebcb8354dde85) )
	ROM_LOAD( "a50_12",   0x18000, 0x4000, CRC(3fb4cfa3) SHA1(b1c5f7b0297c59dc93420d31e0ef2b1125dbb9db) )
	ROM_LOAD( "a50_14",   0x1c000, 0x4000, CRC(24b2f1bf) SHA1(4757aec2e4b99ce33d993ce1e19ee46a4eb76e86) )
ROM_END

void nycaptor_state::init_nycaptor()
{
	m_gametype = 0;
}

void nycaptor_state::init_cyclshtg()
{
	m_gametype = 1;
}

void nycaptor_state::init_bronx()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x20000; i++)
		rom[i] = bitswap<8>(rom[i], 0, 1, 2, 3, 4, 5, 6, 7);

	m_gametype = 1;
}

void nycaptor_state::init_colt()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x20000; i++)
		rom[i] = bitswap<8>(rom[i], 0, 1, 2, 3, 4, 5, 6, 7);

	m_gametype = 2;
}

GAME( 1985, nycaptor, 0,        nycaptor, nycaptor, nycaptor_state, init_nycaptor, ROT0,  "Taito",   "N.Y. Captor (rev 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, colt,     nycaptor, bronx,    colt,     nycaptor_state, init_colt,     ROT0,  "bootleg", "Colt",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1986, cyclshtg, 0,        cyclshtg, cyclshtg, nycaptor_state, init_cyclshtg, ROT90, "Taito",   "Cycle Shooting",      MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1986, bronx,    cyclshtg, bronx,    bronx,    nycaptor_state, init_bronx,    ROT90, "bootleg", "Bronx",               MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
