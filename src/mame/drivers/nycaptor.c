// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/***************************************************************************
    N.Y. Captor - Taito '85

     Driver by Tomasz Slanina  analog [at] op.pl
****************************************************************************
  Hardware similar to Fairyland Story
  Cycle Shooting (Taito '86) is running on (almost) the same hardware
  as NY Captor, but MCU dump is missing.


What's new :
 - added bootlegs - Bronx and Colt
 - Cycle Shooting added (bigger VRAM than nycpator, dfferent (unknwn yet) banking and
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
    The game also frezees sometimes for some unknown reasons.

2) 'cyclshtg' and clones

2a) 'cyclshtg'

  - Lives (BCD coded) settings are not read from MCU, but from table at 0x0fee.
  - Even if it isn't mentionned in the manual, DSWB bit 7 allows you to reset damage to 0
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

//#define USE_MCU


#include "emu.h"
#include "cpu/m6805/m6805.h"
#include "cpu/z80/z80.h"
#include "includes/nycaptor.h"
#include "includes/taitoipt.h"
#include "sound/ay8910.h"
#include "sound/dac.h"


WRITE8_MEMBER(nycaptor_state::sub_cpu_halt_w)
{
	m_subcpu->set_input_line(INPUT_LINE_HALT, (data) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(nycaptor_state::from_snd_r)
{
	return m_snd_data;
}

WRITE8_MEMBER(nycaptor_state::to_main_w)
{
	m_snd_data = data;
}

READ8_MEMBER(nycaptor_state::nycaptor_b_r)
{
	return 1;
}

READ8_MEMBER(nycaptor_state::nycaptor_by_r)
{
	int port = ioport("LIGHTY")->read();

	if (m_gametype == 1)
		port = 255 - port;

	return port - 8;
}

READ8_MEMBER(nycaptor_state::nycaptor_bx_r)
{
	return (ioport("LIGHTX")->read() + 0x27) | 1;
}


WRITE8_MEMBER(nycaptor_state::sound_cpu_reset_w)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data&1 )? ASSERT_LINE : CLEAR_LINE);
}


MACHINE_RESET_MEMBER(nycaptor_state,ta7630)
{
	int i;
	double db           = 0.0;
	double db_step      = 0.50; /* 0.50 dB step (at least, maybe more) */
	double db_step_inc  = 0.275;

	for (i = 0; i < 16; i++)
	{
		double max = 100.0 / pow(10.0, db/20.0 );
		m_vol_ctrl[15 - i] = max;
		/*logerror("vol_ctrl[%x] = %i (%f dB)\n", 15 - i, m_vol_ctrl[15 - i], db);*/
		db += db_step;
		db_step += db_step_inc;
	}
}

TIMER_CALLBACK_MEMBER(nycaptor_state::nmi_callback)
{
	if (m_sound_nmi_enable)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	else
		m_pending_nmi = 1;
}

WRITE8_MEMBER(nycaptor_state::sound_command_w)
{
	soundlatch_byte_w(space, 0, data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(nycaptor_state::nmi_callback),this), data);
}

WRITE8_MEMBER(nycaptor_state::nmi_disable_w)
{
	m_sound_nmi_enable = 0;
}

WRITE8_MEMBER(nycaptor_state::nmi_enable_w)
{
	m_sound_nmi_enable = 1;

	if (m_pending_nmi)
	{
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		m_pending_nmi = 0;
	}
}

WRITE8_MEMBER(nycaptor_state::unk_w)
{
}

READ8_MEMBER(nycaptor_state::nycaptor_generic_control_r)
{
	return m_generic_control_reg;
}

WRITE8_MEMBER(nycaptor_state::nycaptor_generic_control_w)
{
	m_generic_control_reg = data;
	membank("bank1")->set_entry((data&0x08)>>3);
}

static ADDRESS_MAP_START( nycaptor_master_map, AS_PROGRAM, 8, nycaptor_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(nycaptor_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd000, 0xd000) AM_READWRITE(nycaptor_mcu_r, nycaptor_mcu_w)
	AM_RANGE(0xd001, 0xd001) AM_WRITE(sub_cpu_halt_w)
	AM_RANGE(0xd002, 0xd002) AM_READWRITE(nycaptor_generic_control_r, nycaptor_generic_control_w)   /* bit 3 - memory bank at 0x8000-0xbfff */
	AM_RANGE(0xd400, 0xd400) AM_READWRITE(from_snd_r, sound_command_w)
	AM_RANGE(0xd401, 0xd401) AM_READNOP
	AM_RANGE(0xd403, 0xd403) AM_WRITE(sound_cpu_reset_w)
	AM_RANGE(0xd800, 0xd800) AM_READ_PORT("DSWA")
	AM_RANGE(0xd801, 0xd801) AM_READ_PORT("DSWB")
	AM_RANGE(0xd802, 0xd802) AM_READ_PORT("DSWC")
	AM_RANGE(0xd803, 0xd803) AM_READ_PORT("IN0")
	AM_RANGE(0xd804, 0xd804) AM_READ_PORT("IN1")
	AM_RANGE(0xd805, 0xd805) AM_READ(nycaptor_mcu_status_r1)
	AM_RANGE(0xd806, 0xd806) AM_READNOP /* unknown ?sound? */
	AM_RANGE(0xd807, 0xd807) AM_READ(nycaptor_mcu_status_r2)
	AM_RANGE(0xdc00, 0xdc9f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xdca0, 0xdcbf) AM_RAM_WRITE(nycaptor_scrlram_w) AM_SHARE("scrlram")
	AM_RANGE(0xdce1, 0xdce1) AM_WRITENOP
	AM_RANGE(0xdd00, 0xdeff) AM_READWRITE(nycaptor_palette_r, nycaptor_palette_w)
	AM_RANGE(0xdf03, 0xdf03) AM_READWRITE(nycaptor_gfxctrl_r, nycaptor_gfxctrl_w)
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("sharedram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( nycaptor_slave_map, AS_PROGRAM, 8, nycaptor_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(nycaptor_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd800, 0xd800) AM_READ_PORT("DSWA")
	AM_RANGE(0xd801, 0xd801) AM_READ_PORT("DSWB")
	AM_RANGE(0xd802, 0xd802) AM_READ_PORT("DSWC")
	AM_RANGE(0xd803, 0xd803) AM_READ_PORT("IN0")
	AM_RANGE(0xd804, 0xd804) AM_READ_PORT("IN1")
	AM_RANGE(0xdc00, 0xdc9f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xdca0, 0xdcbf) AM_WRITE(nycaptor_scrlram_w) AM_SHARE("scrlram")

	AM_RANGE(0xdd00, 0xdeff) AM_READWRITE(nycaptor_palette_r, nycaptor_palette_w)
	AM_RANGE(0xdf00, 0xdf00) AM_READ(nycaptor_bx_r)
	AM_RANGE(0xdf01, 0xdf01) AM_READ(nycaptor_by_r)
	AM_RANGE(0xdf02, 0xdf02) AM_READ(nycaptor_b_r)
	AM_RANGE(0xdf03, 0xdf03) AM_READ(nycaptor_gfxctrl_r) AM_WRITENOP/* ? gfx control ? */
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("sharedram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( nycaptor_sound_map, AS_PROGRAM, 8, nycaptor_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xc801) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0xc802, 0xc803) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0xc900, 0xc90d) AM_DEVWRITE("msm", msm5232_device, write)
	AM_RANGE(0xca00, 0xca00) AM_WRITENOP
	AM_RANGE(0xcb00, 0xcb00) AM_WRITENOP
	AM_RANGE(0xcc00, 0xcc00) AM_WRITENOP
	AM_RANGE(0xd000, 0xd000) AM_READ(soundlatch_byte_r) AM_WRITE(to_main_w)
	AM_RANGE(0xd200, 0xd200) AM_READNOP AM_WRITE(nmi_enable_w)
	AM_RANGE(0xd400, 0xd400) AM_WRITE(nmi_disable_w)
	AM_RANGE(0xd600, 0xd600) AM_WRITENOP
	AM_RANGE(0xe000, 0xefff) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( nycaptor_m68705_map, AS_PROGRAM, 8, nycaptor_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(nycaptor_68705_port_a_r, nycaptor_68705_port_a_w)
	AM_RANGE(0x0001, 0x0001) AM_READWRITE(nycaptor_68705_port_b_r, nycaptor_68705_port_b_w)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(nycaptor_68705_port_c_r, nycaptor_68705_port_c_w)
	AM_RANGE(0x0004, 0x0004) AM_WRITE(nycaptor_68705_ddr_a_w)
	AM_RANGE(0x0005, 0x0005) AM_WRITE(nycaptor_68705_ddr_b_w)
	AM_RANGE(0x0006, 0x0006) AM_WRITE(nycaptor_68705_ddr_c_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END


/* Cycle Shooting */


READ8_MEMBER(nycaptor_state::cyclshtg_mcu_status_r)
{
	return 0xff;
}

READ8_MEMBER(nycaptor_state::cyclshtg_mcu_r)
{
	return 7;
}

WRITE8_MEMBER(nycaptor_state::cyclshtg_mcu_w)
{
}

READ8_MEMBER(nycaptor_state::cyclshtg_mcu_status_r1)
{
	return machine().rand();
}

WRITE8_MEMBER(nycaptor_state::cyclshtg_generic_control_w)
{
	m_generic_control_reg = data;
	membank("bank1")->set_entry((data >> 2) & 3);
}


static ADDRESS_MAP_START( cyclshtg_master_map, AS_PROGRAM, 8, nycaptor_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xcfff) AM_RAM_WRITE(nycaptor_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd000, 0xd000) AM_READWRITE(cyclshtg_mcu_r, cyclshtg_mcu_w)
	AM_RANGE(0xd001, 0xd001) AM_WRITE(sub_cpu_halt_w)
	AM_RANGE(0xd002, 0xd002) AM_READWRITE(nycaptor_generic_control_r, cyclshtg_generic_control_w)
	AM_RANGE(0xd400, 0xd400) AM_READWRITE(from_snd_r, sound_command_w)
	AM_RANGE(0xd403, 0xd403) AM_WRITE(sound_cpu_reset_w)
	AM_RANGE(0xd800, 0xd800) AM_READ_PORT("DSWA")
	AM_RANGE(0xd801, 0xd801) AM_READ_PORT("DSWB")
	AM_RANGE(0xd802, 0xd802) AM_READ_PORT("DSWC")
	AM_RANGE(0xd803, 0xd803) AM_READ_PORT("IN0")
	AM_RANGE(0xd804, 0xd804) AM_READ_PORT("IN1")
	AM_RANGE(0xd805, 0xd805) AM_READ(cyclshtg_mcu_status_r)
	AM_RANGE(0xd806, 0xd806) AM_READNOP
	AM_RANGE(0xd807, 0xd807) AM_READ(cyclshtg_mcu_status_r)
	AM_RANGE(0xdc00, 0xdc9f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xdca0, 0xdcbf) AM_RAM_WRITE(nycaptor_scrlram_w) AM_SHARE("scrlram")
	AM_RANGE(0xdce1, 0xdce1) AM_WRITENOP
	AM_RANGE(0xdd00, 0xdeff) AM_READWRITE(nycaptor_palette_r, nycaptor_palette_w)
	AM_RANGE(0xdf03, 0xdf03) AM_READWRITE(nycaptor_gfxctrl_r, nycaptor_gfxctrl_w)
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("sharedram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( cyclshtg_slave_map, AS_PROGRAM, 8, nycaptor_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM_WRITE(nycaptor_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd800, 0xd800) AM_READ_PORT("DSWA")
	AM_RANGE(0xd801, 0xd801) AM_READ_PORT("DSWB")
	AM_RANGE(0xd802, 0xd802) AM_READ_PORT("DSWC")
	AM_RANGE(0xd803, 0xd803) AM_READ_PORT("IN0")
	AM_RANGE(0xd804, 0xd804) AM_READ_PORT("IN1")
	AM_RANGE(0xdc00, 0xdc9f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xdca0, 0xdcbf) AM_WRITE(nycaptor_scrlram_w) AM_SHARE("scrlram")
	AM_RANGE(0xdd00, 0xdeff) AM_READWRITE(nycaptor_palette_r, nycaptor_palette_w)
	AM_RANGE(0xdf00, 0xdf00) AM_READ(nycaptor_bx_r)
	AM_RANGE(0xdf01, 0xdf01) AM_READ(nycaptor_by_r)
	AM_RANGE(0xdf02, 0xdf02) AM_READ(nycaptor_b_r)
	AM_RANGE(0xdf03, 0xdf03) AM_READ(nycaptor_gfxctrl_r)
	AM_RANGE(0xdf03, 0xdf03) AM_WRITENOP
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("sharedram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( cyclshtg_sound_map, AS_PROGRAM, 8, nycaptor_state )
	AM_RANGE(0xd600, 0xd600) AM_DEVWRITE("dac", dac_device, write_signed8) //otherwise no girl's scream, see MT03975
	AM_IMPORT_FROM( nycaptor_sound_map )
ADDRESS_MAP_END

READ8_MEMBER(nycaptor_state::unk_r)
{
	return machine().rand();
}

static ADDRESS_MAP_START( bronx_master_map, AS_PROGRAM, 8, nycaptor_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xcfff) AM_RAM_WRITE(nycaptor_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd000, 0xd000) AM_READ(cyclshtg_mcu_r) AM_WRITENOP
	AM_RANGE(0xd001, 0xd001) AM_WRITE(sub_cpu_halt_w)
	AM_RANGE(0xd002, 0xd002) AM_READWRITE(nycaptor_generic_control_r, cyclshtg_generic_control_w)
	AM_RANGE(0xd400, 0xd400) AM_READWRITE(from_snd_r, sound_command_w)
	AM_RANGE(0xd401, 0xd401) AM_READ(unk_r)
	AM_RANGE(0xd403, 0xd403) AM_WRITE(sound_cpu_reset_w)
	AM_RANGE(0xd800, 0xd800) AM_READ_PORT("DSWA")
	AM_RANGE(0xd801, 0xd801) AM_READ_PORT("DSWB")
	AM_RANGE(0xd802, 0xd802) AM_READ_PORT("DSWC")
	AM_RANGE(0xd803, 0xd803) AM_READ_PORT("IN0")
	AM_RANGE(0xd804, 0xd804) AM_READ_PORT("IN1")
	AM_RANGE(0xd805, 0xd805) AM_READ(cyclshtg_mcu_status_r)
	AM_RANGE(0xd806, 0xd806) AM_READNOP
	AM_RANGE(0xd807, 0xd807) AM_READ(cyclshtg_mcu_status_r)
	AM_RANGE(0xdc00, 0xdc9f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xdca0, 0xdcbf) AM_RAM_WRITE(nycaptor_scrlram_w) AM_SHARE("scrlram")
	AM_RANGE(0xdd00, 0xdeff) AM_READWRITE(nycaptor_palette_r, nycaptor_palette_w)
	AM_RANGE(0xdf03, 0xdf03) AM_READWRITE(nycaptor_gfxctrl_r, nycaptor_gfxctrl_w)
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("sharedram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( bronx_slave_map, AS_PROGRAM, 8, nycaptor_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM_WRITE(nycaptor_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd800, 0xd800) AM_READ_PORT("DSWA")
	AM_RANGE(0xd801, 0xd801) AM_READ_PORT("DSWB")
	AM_RANGE(0xd802, 0xd802) AM_READ_PORT("DSWC")
	AM_RANGE(0xd803, 0xd803) AM_READ_PORT("IN0")
	AM_RANGE(0xd804, 0xd804) AM_READ_PORT("IN1")
	AM_RANGE(0xd805, 0xd805) AM_READ(cyclshtg_mcu_status_r1)
	AM_RANGE(0xd807, 0xd807) AM_READ(cyclshtg_mcu_status_r)
	AM_RANGE(0xdc00, 0xdc9f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xdca0, 0xdcbf) AM_WRITE(nycaptor_scrlram_w) AM_SHARE("scrlram")
	AM_RANGE(0xdd00, 0xdeff) AM_READWRITE(nycaptor_palette_r, nycaptor_palette_w)
	AM_RANGE(0xdf00, 0xdf00) AM_READ(nycaptor_bx_r)
	AM_RANGE(0xdf01, 0xdf01) AM_READ(nycaptor_by_r)
	AM_RANGE(0xdf02, 0xdf02) AM_READ(nycaptor_b_r)
	AM_RANGE(0xdf03, 0xdf03) AM_READWRITE(nycaptor_gfxctrl_r, nycaptor_gfxctrl_w)
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("sharedram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( bronx_slave_io_map, AS_IO, 8, nycaptor_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END


/* verified from Z80 code */
static INPUT_PORTS_START( nycaptor )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )       /* table at 0x00e5 in CPU1 - see notes for 'colt' */
	PORT_DIPSETTING(    0x02, "20k 80k 80k+" )
	PORT_DIPSETTING(    0x03, "50k 150k 200k+" )
	PORT_DIPSETTING(    0x01, "100k 300k 300k+" )
	PORT_DIPSETTING(    0x00, "150k 300k 300k+" )
	PORT_DIPNAME( 0x04, 0x04, "Infinite Bullets")           /* see notes */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )            /* values are read from the MCU */
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
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
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
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
	PORT_DIPNAME( 0x01, 0x01, "Freeze" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Training Spot" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Reset Damage" )
	PORT_DIPSETTING(    0x20, "Every Stage" )
	PORT_DIPSETTING(    0x00, "Every 4 Stages" )
	PORT_DIPNAME( 0x40, 0x40, "No Hit (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )            /* IPT_START2 is some similar Taito games (eg: 'flstory') */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )            /* "I/O ERROR" if active - code at 0x083d */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )            /* "I/O ERROR" if active - code at 0x083d */

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
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )            /* see notes */
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x00, "100" )
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( cyclshtg )
	PORT_START("DSWA")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	TAITO_DSWA_BITS_2_TO_3
	TAITO_COINAGE_JAPAN_OLD                                 /* coinage B isn't mentionned in the manual */

	PORT_START("DSWB")
	TAITO_DIFFICULTY
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Bonus_Life ) )       /* table at 0x100f - see notes for 'bronx' */
	PORT_DIPSETTING(    0x0c, "150k 350k 200k+" )
	PORT_DIPSETTING(    0x08, "200k 500k 300k+" )
	PORT_DIPSETTING(    0x04, "300k 700k 400k+" )
	PORT_DIPSETTING(    0x00, "400k 900k 500k+" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )            /* see notes */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, "Reset Damage (Cheat)" )      /* see notes */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWC")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x10, 0x10, "Infinite Bullets" )          /* see notes */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )            /* IPT_START2 is some similar Taito games (eg: 'flstory') */
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
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )            /* see notes */
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

static GFXDECODE_START( nycaptor )
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
	save_item(NAME(m_sound_nmi_enable));
	save_item(NAME(m_pending_nmi));
	save_item(NAME(m_snd_data));
	save_item(NAME(m_vol_ctrl));

	save_item(NAME(m_char_bank));
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_gfxctrl));

	save_item(NAME(m_port_a_in));
	save_item(NAME(m_port_a_out));
	save_item(NAME(m_ddr_a));
	save_item(NAME(m_port_b_in));
	save_item(NAME(m_port_b_out));
	save_item(NAME(m_ddr_b));
	save_item(NAME(m_port_c_in));
	save_item(NAME(m_port_c_out));
	save_item(NAME(m_ddr_c));
	save_item(NAME(m_mcu_sent));
	save_item(NAME(m_main_sent));
	save_item(NAME(m_from_main));
	save_item(NAME(m_from_mcu));
}

void nycaptor_state::machine_reset()
{
	MACHINE_RESET_CALL_MEMBER(ta7630);

	m_generic_control_reg = 0;
	m_sound_nmi_enable = 0;
	m_pending_nmi = 0;
	m_snd_data = 0;

	m_char_bank = 0;
	m_palette_bank = 0;
	m_gfxctrl = 0;

	m_port_a_in = 0;
	m_port_a_out = 0;
	m_ddr_a = 0;
	m_port_b_in = 0;
	m_port_b_out = 0;
	m_ddr_b = 0;
	m_port_c_in = 0;
	m_port_c_out = 0;
	m_ddr_c = 0;
	m_mcu_sent = 0;
	m_main_sent = 0;
	m_from_main = 0;
	m_from_mcu = 0;

	memset(m_vol_ctrl, 0, sizeof(m_vol_ctrl));
}

static MACHINE_CONFIG_START( nycaptor, nycaptor_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,8000000/2)      /* ??? */
	MCFG_CPU_PROGRAM_MAP(nycaptor_master_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", nycaptor_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80,8000000/2)
	MCFG_CPU_PROGRAM_MAP(nycaptor_slave_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", nycaptor_state,  irq0_line_hold)   /* IRQ generated by ??? */

	MCFG_CPU_ADD("audiocpu", Z80,8000000/2)
	MCFG_CPU_PROGRAM_MAP(nycaptor_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(nycaptor_state, irq0_line_hold, 2*60)  /* IRQ generated by ??? */

	MCFG_CPU_ADD("mcu", M68705,2000000)
	MCFG_CPU_PROGRAM_MAP(nycaptor_m68705_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))  /* 100 CPU slices per frame - an high value to ensure proper synchronization of the CPUs */


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(nycaptor_state, screen_update_nycaptor)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", nycaptor)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 8000000/4)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(nycaptor_state, unk_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(nycaptor_state, unk_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay2", AY8910, 8000000/4)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(nycaptor_state, unk_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(nycaptor_state, unk_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("msm", MSM5232, 2000000)
	MCFG_MSM5232_SET_CAPACITORS(0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6) /* 0.65 (???) uF capacitors (match the sample, not verified) */
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

static MACHINE_CONFIG_START( cyclshtg, nycaptor_state )

	MCFG_CPU_ADD("maincpu", Z80,8000000/2)
	MCFG_CPU_PROGRAM_MAP(cyclshtg_master_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", nycaptor_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80,8000000/2)
	MCFG_CPU_PROGRAM_MAP(cyclshtg_slave_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", nycaptor_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80,8000000/2)
	MCFG_CPU_PROGRAM_MAP(cyclshtg_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(nycaptor_state, irq0_line_hold, 2*60)

#ifdef USE_MCU
	MCFG_CPU_ADD("mcu", M68705,2000000)
	MCFG_CPU_PROGRAM_MAP(nycaptor_m68705_map)
#endif

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(nycaptor_state, screen_update_nycaptor)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", nycaptor)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)


	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 8000000/4)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(nycaptor_state, unk_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(nycaptor_state, unk_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay2", AY8910, 8000000/4)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(nycaptor_state, unk_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(nycaptor_state, unk_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("msm", MSM5232, 2000000)
	MCFG_MSM5232_SET_CAPACITORS(0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6) /* 0.65 (???) uF capacitors (match the sample, not verified) */
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

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( bronx, nycaptor_state )

	MCFG_CPU_ADD("maincpu", Z80,8000000/2)
	MCFG_CPU_PROGRAM_MAP(bronx_master_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", nycaptor_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80,8000000/2)
	MCFG_CPU_PROGRAM_MAP(bronx_slave_map)
	MCFG_CPU_IO_MAP(bronx_slave_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", nycaptor_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80,8000000/2)
	MCFG_CPU_PROGRAM_MAP(cyclshtg_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(nycaptor_state, irq0_line_hold, 2*60)

	MCFG_QUANTUM_TIME(attotime::from_hz(120))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(nycaptor_state, screen_update_nycaptor)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", nycaptor)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)


	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 8000000/4)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(nycaptor_state, unk_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(nycaptor_state, unk_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay2", AY8910, 8000000/4)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(nycaptor_state, unk_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(nycaptor_state, unk_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("msm", MSM5232, 2000000)
	MCFG_MSM5232_SET_CAPACITORS(0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6) /* 0.65 (???) uF capacitors (match the sample, not verified) */
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

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



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

	ROM_REGION( 0x0800, "mcu", 0 )
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

ROM_START( cyclshtg )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "a97_01.i17",   0x00000, 0x4000, CRC(686fac1a) SHA1(46d17cb98f064413bb76c5d869f8061d2771cda0) )
	ROM_LOAD( "a97_02.i16",   0x04000, 0x4000, CRC(48a812f9) SHA1(8ab18cb8d6a8b7ce1ed1a4009f5435ce4b0937b4) )
	ROM_LOAD( "a97_03.u15",   0x10000, 0x4000, CRC(67ad3067) SHA1(2e355653e91c093abe7db0a3d55d5a3f95c4a2e3) )
	ROM_LOAD( "a97_04.u14",   0x14000, 0x4000, CRC(804e6445) SHA1(5b6771c5729faf62d5002d090c0b9c5ca5cb9ad6) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "a97_05.u22",   0x0000, 0x4000, CRC(fdc36c4f) SHA1(cae2d3f07c5bd6de9d40ff7d385b999e7dc9ce82) )
	ROM_LOAD( "a80_06.u23",   0x4000, 0x4000, CRC(2769c5ab) SHA1(b8f5a4a8c70c8d37d5e92b37faa0e25b287b3fb2) )
	ROM_LOAD( "a97_06.i24",   0x8000, 0x4000, CRC(c0473a54) SHA1(06fa7345a44a72995146e973c2cd7a14499f4310) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a80_16.i26",   0x0000, 0x4000, CRC(ce171a48) SHA1(e5ae9bb22f58c8857737bc6f5317866819a4e4d1) )
	ROM_LOAD( "a80_17.i25",   0x4000, 0x4000, CRC(a90b7bbc) SHA1(bd5c96861a59a1f84bb5032775b1c70efdb7066f) )

	ROM_REGION( 0x0800, "cpu3", 0 )
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
	ROM_LOAD( "8.bin",          0x0c000, 0x4000, CRC(2b778d24) SHA1(caca7a18743a4bb657a7c5691d93de0ccb867003) )

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
	ROM_LOAD( "01.bin",   0x18000, 0x4000, CRC(9b0948f3) SHA1(a55e09243640ec56aa22e4b6d47165b02b880eb7) )
	ROM_COPY( "maincpu",   0x10000, 0x014000, 0x04000 )
	ROM_COPY( "maincpu",   0x18000, 0x01c000, 0x04000 )

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

DRIVER_INIT_MEMBER(nycaptor_state,nycaptor)
{
	m_gametype = 0;
}

DRIVER_INIT_MEMBER(nycaptor_state,cyclshtg)
{
	m_gametype = 1;
}

DRIVER_INIT_MEMBER(nycaptor_state,bronx)
{
	int i;
	UINT8 *rom = memregion("maincpu")->base();

	for (i = 0; i < 0x20000; i++)
		rom[i] = BITSWAP8(rom[i], 0, 1, 2, 3, 4, 5, 6, 7);

	m_gametype = 1;
}

DRIVER_INIT_MEMBER(nycaptor_state,colt)
{
	int i;
	UINT8 *rom = memregion("maincpu")->base();

	for (i = 0; i < 0x20000; i++)
		rom[i] = BITSWAP8(rom[i], 0, 1, 2, 3, 4, 5, 6, 7);

	m_gametype = 2;
}

GAME( 1985, nycaptor, 0,        nycaptor, nycaptor, nycaptor_state, nycaptor, ROT0,  "Taito",   "N.Y. Captor", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, cyclshtg, 0,        cyclshtg, cyclshtg, nycaptor_state, cyclshtg, ROT90, "Taito",   "Cycle Shooting", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
/* bootlegs */
GAME( 1986, bronx,    cyclshtg, bronx,    bronx, nycaptor_state,    bronx,    ROT90, "bootleg", "Bronx", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, colt,     nycaptor, bronx,    colt, nycaptor_state,     colt,     ROT0,  "bootleg", "Colt", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
