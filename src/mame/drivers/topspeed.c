// license:BSD-3-Clause
// copyright-holders:David Graves
/***************+************************************************************

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

Replace TC0140SYT with PC060HA

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
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/taitoio.h"
#include "audio/taitosnd.h"
#include "sound/2151intf.h"
#include "sound/msm5205.h"
#include "sound/flt_vol.h"
#include "includes/taitoipt.h"
#include "includes/topspeed.h"
#include "topspeed.lh"



/**********************************************************
                       CPU CONTROL
**********************************************************/

WRITE16_MEMBER(topspeed_state::cpua_ctrl_w)
{
	// Written only twice; once on startup at 0x00 then 0xc3 after init
	m_cpua_ctrl = data;
}


/**********************************************************
                       GAME INPUTS
**********************************************************/

READ8_MEMBER(topspeed_state::input_bypass_r)
{
	// Read port number
	UINT8 port = m_tc0220ioc->port_r(space, 0);
	UINT16 steer = 0xff80 + read_safe(ioport("STEER"), 0);

	switch (port)
	{
		case 0x0c:
			return steer & 0xff;

		case 0x0d:
			return steer >> 8;

		default:
			return m_tc0220ioc->portreg_r(space, offset);
	}
}

CUSTOM_INPUT_MEMBER(topspeed_state::pedal_r)
{
	static const UINT8 retval[8] = { 0,1,3,2,6,7,5,4 };
	const char *tag = (const char *)param;
	return retval[read_safe(ioport(tag), 0) & 7];
}

READ16_MEMBER(topspeed_state::motor_r)
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
			logerror("CPU #0 PC %06x: warning - read from motor cpu %03x\n", space.device().safe_pc(), offset);
			return 0;
	}
}

WRITE16_MEMBER(topspeed_state::motor_w)
{
	// Writes $900000-25 and $900200-219
	logerror("CPU #0 PC %06x: warning - write %04x to motor cpu %03x\n", space.device().safe_pc(), data, offset);
}


/*****************************************************
                        SOUND
*****************************************************/

WRITE8_MEMBER(topspeed_state::sound_bankswitch_w)
{
	membank("sndbank")->set_entry(data & 3);
}

void topspeed_state::msm5205_update(int chip)
{
	if (m_msm_reset[chip])
		return;

	UINT8 data = m_msm_rom[chip][m_msm_pos[chip]];
	msm5205_device *msm = chip ? m_msm2 : m_msm1;

	msm->data_w((m_msm_nibble[chip] ? data : data >> 4) & 0xf);

	if (m_msm_nibble[chip])
		++m_msm_pos[chip];

	m_msm_nibble[chip] ^= 1;
}

WRITE_LINE_MEMBER(topspeed_state::msm5205_1_vck)
{
	msm5205_update(0);
}

WRITE8_MEMBER(topspeed_state::msm5205_command_w)
{
	int chip = (offset >> 12) & 1;
	msm5205_device *msm = chip ? m_msm2 : m_msm1;

	switch (offset >> 8 & 0x2e)
	{
		// $b000 / $c000: Start address
		case 0x00:
			m_msm_pos[chip] = data << 8;
			break;

		// $b400 / $c400: Run
		case 0x04:
			m_msm_reset[chip] = 0;
			msm->reset_w(0);
			break;

		// $b800 / $c800: Stop
		case 0x08:
			m_msm_reset[chip] = 1;
			m_msm_nibble[chip] = 0;
			msm->reset_w(1);
			break;

		// $cc00: ? Chip 2 only
		case 0x0c:
			break;

		default:
			logerror("Unhandled MSM5205 control write to %x with %x (PC:%.4x)\n", 0xb000 + offset, data, space.device().safe_pc());
			break;
	}
}

WRITE8_MEMBER(topspeed_state::volume_w)
{
	// The volume is controlled by two Taito TC0060DCA hybrid volume modules
	filter_volume_device *filter = NULL;

	switch (offset)
	{
		case 0x000: filter = m_filter2;     break; // MSM5205 1
		case 0x200: filter = m_filter3;     break; // MSM5205 2
		case 0x400: filter = m_filter1l;    break; // YM-2151 L
		case 0x600: filter = m_filter1r;    break; // YM-2151 R
	}

	filter->flt_volume_set_volume(data / 255.0f);
}

WRITE_LINE_MEMBER(topspeed_state::z80ctc_to0)
{
	if (m_msm2_vck2 && !state)
	{
		// CTC output is divided by 2
		if (m_msm2_vck)
		{
			m_msm2->vclk_w(1);
		}
		else
		{
			// Update on falling edge of /VCK
			UINT16 oldpos = m_msm_pos[1];

			msm5205_update(1);

			// Handle looping
			if ((oldpos >> 8) == 0x0f && ((m_msm_pos[1] >> 8) == 0x10))
			{
				m_msm_pos[1] = 0;
				m_msm2->reset_w(1);
				m_msm2->vclk_w(0);
				m_msm2->reset_w(0);
			}
			else
			{
				m_msm2->vclk_w(0);
			}
		}

		m_msm2_vck ^= 1;
	}
	m_msm2_vck2 = state;
}


/***********************************************************
                      MEMORY STRUCTURES
***********************************************************/

static ADDRESS_MAP_START( cpua_map, AS_PROGRAM, 16, topspeed_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM AM_SHARE("sharedram")
	AM_RANGE(0x500000, 0x503fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x600002, 0x600003) AM_WRITE(cpua_ctrl_w)
	AM_RANGE(0x7e0000, 0x7e0001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_device, master_port_w, 0x00ff)
	AM_RANGE(0x7e0002, 0x7e0003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w, 0x00ff)
	AM_RANGE(0x800000, 0x8003ff) AM_RAM AM_SHARE("raster_ctrl")
	AM_RANGE(0x800400, 0x80ffff) AM_RAM
	AM_RANGE(0x880000, 0x880007) AM_WRITENOP // Lamps/outputs?
	AM_RANGE(0xa00000, 0xa0ffff) AM_DEVREADWRITE("pc080sn_1", pc080sn_device, word_r, word_w)
	AM_RANGE(0xa20000, 0xa20003) AM_DEVWRITE("pc080sn_1", pc080sn_device, yscroll_word_w)
	AM_RANGE(0xa40000, 0xa40003) AM_DEVWRITE("pc080sn_1", pc080sn_device, xscroll_word_w)
	AM_RANGE(0xa50000, 0xa50003) AM_DEVWRITE("pc080sn_1", pc080sn_device, ctrl_word_w)
	AM_RANGE(0xb00000, 0xb0ffff) AM_DEVREADWRITE("pc080sn_2", pc080sn_device, word_r, word_w)
	AM_RANGE(0xb20000, 0xb20003) AM_DEVWRITE("pc080sn_2", pc080sn_device, yscroll_word_w)
	AM_RANGE(0xb40000, 0xb40003) AM_DEVWRITE("pc080sn_2", pc080sn_device, xscroll_word_w)
	AM_RANGE(0xb50000, 0xb50003) AM_DEVWRITE("pc080sn_2", pc080sn_device, ctrl_word_w)
	AM_RANGE(0xd00000, 0xd00fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe00000, 0xe0ffff) AM_RAM AM_SHARE("spritemap")
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpub_map, AS_PROGRAM, 16, topspeed_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x400000, 0X40ffff) AM_RAM AM_SHARE("sharedram")
	AM_RANGE(0x880000, 0x880001) AM_READ8(input_bypass_r, 0x00ff) AM_DEVWRITE8("tc0220ioc", tc0220ioc_device, portreg_w, 0x00ff)
	AM_RANGE(0x880002, 0x880003) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_device, port_r, port_w, 0x00ff)
	AM_RANGE(0x900000, 0x9003ff) AM_READWRITE(motor_r, motor_w)
ADDRESS_MAP_END


/***************************************************************************/

static ADDRESS_MAP_START( z80_prg, AS_PROGRAM, 8, topspeed_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("sndbank")
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9001) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_DEVWRITE("tc0140syt", tc0140syt_device, slave_port_w)
	AM_RANGE(0xa001, 0xa001) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, slave_comm_r, slave_comm_w)
	AM_RANGE(0xb000, 0xcfff) AM_WRITE(msm5205_command_w)
	AM_RANGE(0xd000, 0xdfff) AM_WRITE(volume_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80_io, AS_IO, 8, topspeed_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("ctc", z80ctc_device, read, write)
ADDRESS_MAP_END


/***********************************************************
                    INPUT PORTS, DIPs
***********************************************************/

static INPUT_PORTS_START( topspeed )
	// 0x880000 (port 0) -> 0x400852 (-$77ae,A5) (shared RAM)
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(    0x03, "Deluxe" )    // analog pedals, racing wheel, motor (tilt disabled)
	PORT_DIPSETTING(    0x02, "Standard" )  // digital pedals, continuous wheel
//  PORT_DIPSETTING(    0x01, "Standard" )
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
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, topspeed_state, pedal_r, "BRAKE") PORT_CONDITION("DSWA", 0x03, NOTEQUALS, 0x02)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_NAME("Brake Switch") PORT_CONDITION("DSWA", 0x03, EQUALS, 0x02)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_NAME("Nitro")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE2 ) PORT_NAME("Calibrate") // ?
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON4 ) PORT_NAME("Shifter") PORT_TOGGLE
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, topspeed_state, pedal_r, "GAS") PORT_CONDITION("DSWA", 0x03, NOTEQUALS, 0x02)
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
	{ 0, 8, 16, 24 },
	{ 32, 33, 34, 35, 36, 37, 38, 39, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8    // every sprite takes 64 consecutive bytes
};

static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	RGN_FRAC(1,1),
	4,      // 4 bits per pixel
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    // every sprite takes 32 consecutive bytes
};

static GFXDECODE_START( topspeed )
	GFXDECODE_ENTRY( "gfx2", 0x0, tile16x8_layout,  0, 256 )    // Sprite parts
	GFXDECODE_ENTRY( "gfx1", 0x0, charlayout,  0, 512 )         // Sprites & playfield
	// Road Lines gfxdecodable ?
GFXDECODE_END


/***********************************************************
                     MACHINE DRIVERS
***********************************************************/

void topspeed_state::machine_start()
{
	membank("sndbank")->configure_entries(0, 4, memregion("audiocpu")->base() + 0xc000, 0x4000);

	m_msm_rom[0] = memregion("adpcm")->base();
	m_msm_rom[1] = memregion("adpcm")->base() + 0x10000;

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
	m_msm1->reset_w(1);
	m_msm2->reset_w(1);
	m_msm2_vck = 0;
	m_msm2_vck2 = 0;
}


static MACHINE_CONFIG_START( topspeed, topspeed_state )

	// basic machine hardware
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz / 2)
	MCFG_CPU_PROGRAM_MAP(cpua_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", topspeed_state, irq6_line_hold)

	MCFG_CPU_ADD("subcpu", M68000, XTAL_16MHz / 2)
	MCFG_CPU_PROGRAM_MAP(cpub_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", topspeed_state, irq5_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(z80_prg)
	MCFG_CPU_IO_MAP(z80_io)

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL_16MHz / 4)
	MCFG_Z80CTC_ZC0_CB(WRITELINE(topspeed_state, z80ctc_to0))

	MCFG_DEVICE_ADD("pc080sn_1", PC080SN, 0)
	MCFG_PC080SN_GFX_REGION(1)
	MCFG_PC080SN_OFFSETS(0, 8)
	MCFG_PC080SN_GFXDECODE("gfxdecode")
	MCFG_PC080SN_PALETTE("palette")

	MCFG_DEVICE_ADD("pc080sn_2", PC080SN, 0)
	MCFG_PC080SN_GFX_REGION(1)
	MCFG_PC080SN_OFFSETS(0, 8)
	MCFG_PC080SN_GFXDECODE("gfxdecode")
	MCFG_PC080SN_PALETTE("palette")

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")

	MCFG_DEVICE_ADD("tc0220ioc", TC0220IOC, 0)
	MCFG_TC0220IOC_READ_0_CB(IOPORT("DSWA"))
	MCFG_TC0220IOC_READ_1_CB(IOPORT("DSWB"))
	MCFG_TC0220IOC_READ_2_CB(IOPORT("IN0"))
	MCFG_TC0220IOC_READ_3_CB(IOPORT("IN1"))
	MCFG_TC0220IOC_READ_7_CB(IOPORT("IN2"))

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.0532) // Measured on real hardware
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(topspeed_state, screen_update_topspeed)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", topspeed)
	MCFG_PALETTE_ADD("palette", 8192)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	// sound hardware
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", XTAL_16MHz / 4)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_YM2151_PORT_WRITE_HANDLER(WRITE8(topspeed_state, sound_bankswitch_w))
	MCFG_SOUND_ROUTE(0, "filter1l", 1.0)
	MCFG_SOUND_ROUTE(1, "filter1r", 1.0)

	MCFG_SOUND_ADD("msm1", MSM5205, XTAL_384kHz)
	MCFG_MSM5205_VCLK_CB(WRITELINE(topspeed_state, msm5205_1_vck)) // VCK function
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      // 8 kHz, 4-bit
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "filter2", 1.0)

	MCFG_SOUND_ADD("msm2", MSM5205, XTAL_384kHz)
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_SEX_4B)      // Slave mode, 4-bit
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "filter3", 1.0)

	MCFG_FILTER_VOLUME_ADD("filter1l", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("filter1r", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_FILTER_VOLUME_ADD("filter2", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_FILTER_VOLUME_ADD("filter3", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END



/***************************************************************************
                                DRIVERS

Note: driver does NOT make use of the zoom sprite tables rom.
***************************************************************************/

ROM_START( topspeed )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 128K for 68000 code (CPU A)
	ROM_LOAD16_BYTE( "b14-67-1.9",  0x00000, 0x10000, CRC(23f17616) SHA1(653ab6537f2e5898a77060c82b776852ab1f2b51) )
	ROM_LOAD16_BYTE( "b14-68-1.11", 0x00001, 0x10000, CRC(835659d9) SHA1(e99967f795c3c6e14bad7a66315640ca5db43c72) )
	ROM_LOAD16_BYTE( "b14-54.24",   0x80000, 0x20000, CRC(172924d5) SHA1(4a963f2e816f4b1c5acc6d38e99a68d3baeee8c6) ) // 4 data ROMs
	ROM_LOAD16_BYTE( "b14-52.26",   0x80001, 0x20000, CRC(e1b5b2a1) SHA1(8e2b992dcd5dc2317594c0187a22767aa626edee) )
	ROM_LOAD16_BYTE( "b14-55.23",   0xc0000, 0x20000, CRC(a1f15499) SHA1(72f99108713773782fc72aae5a3f6e9e2a1e347c) )
	ROM_LOAD16_BYTE( "b14-53.25",   0xc0001, 0x20000, CRC(04a04f5f) SHA1(09c15c33967bb141cc504b70d01c154bedb7fa33) )

	ROM_REGION( 0x20000, "subcpu", 0 ) // 128K for 68000 code (CPU B)
	ROM_LOAD16_BYTE( "b14-69.80",   0x00000, 0x10000, CRC(d652e300) SHA1(b559bdb564d96da4c656dc7b2c88dae84c4861ae) )
	ROM_LOAD16_BYTE( "b14-70.81",   0x00001, 0x10000, CRC(b720592b) SHA1(13298b498a198dcc1a56e533d106545dd77e1bbc) )

	ROM_REGION( 0x1c000, "audiocpu", 0 ) // Z80 sound CPU
	ROM_LOAD( "b14-25.67", 0x00000, 0x04000, CRC(9eab28ef) SHA1(9a90f2c1881f4664d6d6241f3bc57faeaf150ffc) )
	ROM_CONTINUE(          0x10000, 0x0c000 ) // Banked stuff

	ROM_REGION( 0x40000, "gfx1", 0 ) // SCR tiles
	ROM_LOAD16_BYTE( "b14-07.54",   0x00000, 0x20000, CRC(c6025fff) SHA1(439ed85b0160bfd6c06fd42990124a292b2e3c14) )
	ROM_LOAD16_BYTE( "b14-06.52",   0x00001, 0x20000, CRC(b4e2536e) SHA1(c1960ee25b37b1444ec99082521c4858edcf3484) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROMX_LOAD( "b14-48.16", 0x000003, 0x20000, CRC(30c7f265) SHA1(3e52e2aabf2c456d0b57d9414f99bd942bafc887) , ROM_SKIP(7) ) // OBJ, bitplane 3
	ROMX_LOAD( "b14-49.12", 0x100003, 0x20000, CRC(32ba4265) SHA1(f468243d923726b7eff78d9bc55a3a092f211a24) , ROM_SKIP(7) )
	ROMX_LOAD( "b14-50.8",  0x000007, 0x20000, CRC(ec1ef311) SHA1(4cfa06aec9535f2044b763b071f73d23ca8ba354) , ROM_SKIP(7) )
	ROMX_LOAD( "b14-51.4",  0x100007, 0x20000, CRC(35041c5f) SHA1(71602267736396516366a8abf535db82acaa1c23) , ROM_SKIP(7) )

	ROMX_LOAD( "b14-44.15", 0x000002, 0x20000, CRC(9f6c030e) SHA1(bb278fdcc29530685aa2e76da0712195f6ab0f5f) , ROM_SKIP(7) ) // OBJ, bitplane 2
	ROMX_LOAD( "b14-45.11", 0x100002, 0x20000, CRC(63e4ce03) SHA1(92e3f45754676dd15691e48c0d37490c1a3ec328) , ROM_SKIP(7) )
	ROMX_LOAD( "b14-46.7",  0x000006, 0x20000, CRC(d489adf2) SHA1(9f77916594d5ed05b79d7e8d8f534eb39f65edae) , ROM_SKIP(7) )
	ROMX_LOAD( "b14-47.3",  0x100006, 0x20000, CRC(b3a1f75b) SHA1(050dd3313b5392d131c5a62c544260b83af0b8ab) , ROM_SKIP(7) )

	ROMX_LOAD( "b14-40.14", 0x000001, 0x20000, CRC(fa2a3cb3) SHA1(1e102ae6e916fda046a154b89056a18b724d51a3) , ROM_SKIP(7) ) // OBJ, bitplane 1
	ROMX_LOAD( "b14-41.10", 0x100001, 0x20000, CRC(09455a14) SHA1(dc703e1f9c4f16e330796e9945799e1038ce503b) , ROM_SKIP(7) )
	ROMX_LOAD( "b14-42.6",  0x000005, 0x20000, CRC(ab51f53c) SHA1(0ed9a2e607b0bd2b43b47e3ed29b00a8d8a09f25) , ROM_SKIP(7) )
	ROMX_LOAD( "b14-43.2",  0x100005, 0x20000, CRC(1e6d2b38) SHA1(453cd818a6cd8b238c72cc880c811227609767b8) , ROM_SKIP(7) )

	ROMX_LOAD( "b14-36.13", 0x000000, 0x20000, CRC(20a7c1b8) SHA1(053c6b733a5c33b9259dfc754ce30a880905bb11) , ROM_SKIP(7) ) // OBJ, bitplane 0
	ROMX_LOAD( "b14-37.9",  0x100000, 0x20000, CRC(801b703b) SHA1(dfbe276bd484815a7e69589eb56d54bc6e12e301) , ROM_SKIP(7) )
	ROMX_LOAD( "b14-38.5",  0x000004, 0x20000, CRC(de0c213e) SHA1(1313b2051e906d22edb55f4d45d3a424b31ca2a2) , ROM_SKIP(7) )
	ROMX_LOAD( "b14-39.1",  0x100004, 0x20000, CRC(798c28c5) SHA1(d2a8b9f84b3760f3800c5760ecee7ddcbafa6d6e) , ROM_SKIP(7) )

	ROM_REGION( 0x10000, "user1", 0 ) // Zoom tables for zoom sprite h/w
	ROM_LOAD( "b14-30.88", 0x00000, 0x10000, CRC(dccb0c7f) SHA1(42f0af72f559133b74912a4478e1323062be4b77) )

	ROM_REGION( 0x2000, "user2", 0 ) // Unknown (Sharp LH763J-70 64kx8 OTP ROM)
	ROM_LOAD( "b14-31.90",  0x0000,  0x2000,  CRC(5c6b013d) SHA1(6d02d4560076213b6fb6fe856143bb533090603e) )

	ROM_REGION( 0x20000, "adpcm", 0 ) // ADPCM samples
	ROM_LOAD( "b14-28.103",  0x00000, 0x10000, CRC(df11d0ae) SHA1(259e1e6cc7ab100bfdb60e3d7a6bb46acb6fe2ea) )
	ROM_LOAD( "b14-29.109",  0x10000, 0x10000, CRC(7ad983e7) SHA1(a3515caf93d6dab86de06ee52d6a13a456507dbe) )
ROM_END

ROM_START( topspeedu )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 128K for 68000 code (CPU A)
	ROM_LOAD16_BYTE     ( "b14-23", 0x00000, 0x10000, CRC(dd0307fd) SHA1(63218a707c78b3c785d1741dabdc511a76f12af1) )
	ROM_LOAD16_BYTE     ( "b14-24", 0x00001, 0x10000, CRC(acdf08d4) SHA1(506d48d27fc26684a3f884919665cf65a1b3062f) )
	ROM_LOAD16_WORD_SWAP( "b14-05", 0x80000, 0x80000, CRC(6557e9d8) SHA1(ff528b27fcaef5c181f5f3a56d6a41b935cf07e1) ) // Data ROM

	ROM_REGION( 0x20000, "subcpu", 0 ) // 128K for 68000 code (CPU B)
	ROM_LOAD16_BYTE( "b14-26", 0x00000, 0x10000, CRC(659dc872) SHA1(0a168122fe6324510c830e21a56eace9c8a2c189) )
	ROM_LOAD16_BYTE( "b14-56", 0x00001, 0x10000, CRC(d165cf1b) SHA1(bfbb8699c5671d3841d4057678ef4085c1927684) )

	ROM_REGION( 0x1c000, "audiocpu", 0 ) // Z80 sound CPU
	ROM_LOAD( "b14-25.67", 0x00000, 0x04000, CRC(9eab28ef) SHA1(9a90f2c1881f4664d6d6241f3bc57faeaf150ffc) )
	ROM_CONTINUE(          0x10000, 0x0c000 ) // Banked stuff

	ROM_REGION( 0x40000, "gfx1", 0 ) // SCR tiles
	ROM_LOAD16_BYTE( "b14-07.54", 0x00000, 0x20000, CRC(c6025fff) SHA1(439ed85b0160bfd6c06fd42990124a292b2e3c14) )
	ROM_LOAD16_BYTE( "b14-06.52", 0x00001, 0x20000, CRC(b4e2536e) SHA1(c1960ee25b37b1444ec99082521c4858edcf3484) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // OBJ: each rom has 1 bitplane, forming 16x8 tiles
	ROM_LOAD32_BYTE( "b14-01", 0x00000, 0x80000, CRC(84a56f37) SHA1(926bcae5bd75a4172de2a2078718b2940c5c1966) )
	ROM_LOAD32_BYTE( "b14-02", 0x00001, 0x80000, CRC(6889186b) SHA1(3c38e281e8bf416a401c76ebb2d8ca95d09974b6) )
	ROM_LOAD32_BYTE( "b14-03", 0x00002, 0x80000, CRC(d1ed9e71) SHA1(26a6b2ca5bf6d70ad87f5c40c8e94ec542a2ec04) )
	ROM_LOAD32_BYTE( "b14-04", 0x00003, 0x80000, CRC(b63f0519) SHA1(e9a6b49effba0cae1ae3536a8584d3efa34ca8c3) )

	ROM_REGION( 0x10000, "user1", 0 ) // Zoom tables for zoom sprite h/w
	ROM_LOAD( "b14-30.88", 0x00000, 0x10000, CRC(dccb0c7f) SHA1(42f0af72f559133b74912a4478e1323062be4b77) )

	ROM_REGION( 0x2000, "user2", 0 ) // Unknown (Sharp LH763J-70 64kx8 OTP ROM)
	ROM_LOAD( "b14-31.90", 0x0000,  0x2000,  CRC(5c6b013d) SHA1(6d02d4560076213b6fb6fe856143bb533090603e) )

	ROM_REGION( 0x20000, "adpcm", 0 ) // ADPCM samples
	ROM_LOAD( "b14-28.103", 0x00000, 0x10000, CRC(df11d0ae) SHA1(259e1e6cc7ab100bfdb60e3d7a6bb46acb6fe2ea) )
	ROM_LOAD( "b14-29.109", 0x10000, 0x10000, CRC(7ad983e7) SHA1(a3515caf93d6dab86de06ee52d6a13a456507dbe) )
ROM_END

ROM_START( fullthrl )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 128K for 68000 code (CPU A) - Later revision?
	ROM_LOAD16_BYTE     ( "b14-67", 0x00000, 0x10000, CRC(284c943f) SHA1(e4720b138052d9cbf1290aeca8f9dd7fe2cffcc5) )
	ROM_LOAD16_BYTE     ( "b14-68", 0x00001, 0x10000, CRC(54cf6196) SHA1(0e86a7bf7d43526222160f4cd09f8d29fa9abdc4) )
	ROM_LOAD16_WORD_SWAP( "b14-05", 0x80000, 0x80000, CRC(6557e9d8) SHA1(ff528b27fcaef5c181f5f3a56d6a41b935cf07e1) ) // Data ROM

	ROM_REGION( 0x20000, "subcpu", 0 ) // 128K for 68000 code (CPU B)
	ROM_LOAD16_BYTE( "b14-69.80", 0x00000, 0x10000, CRC(d652e300) SHA1(b559bdb564d96da4c656dc7b2c88dae84c4861ae) )
	ROM_LOAD16_BYTE( "b14-71",    0x00001, 0x10000, CRC(f7081727) SHA1(f0ab6ce9975dd7a1fadd439fd3dfd2f1bf88796c) )

	ROM_REGION( 0x1c000, "audiocpu", 0 ) // Z80 sound CPU
	ROM_LOAD( "b14-25.67", 0x00000, 0x04000, CRC(9eab28ef) SHA1(9a90f2c1881f4664d6d6241f3bc57faeaf150ffc) )
	ROM_CONTINUE(          0x10000, 0x0c000 ) // Banked stuff

	ROM_REGION( 0x40000, "gfx1", 0 ) // SCR tiles
	ROM_LOAD16_BYTE( "b14-07.54", 0x00000, 0x20000, CRC(c6025fff) SHA1(439ed85b0160bfd6c06fd42990124a292b2e3c14) )
	ROM_LOAD16_BYTE( "b14-06.52", 0x00001, 0x20000, CRC(b4e2536e) SHA1(c1960ee25b37b1444ec99082521c4858edcf3484) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // OBJ: each rom has 1 bitplane, forming 16x8 tiles
	ROM_LOAD32_BYTE( "b14-01", 0x00000, 0x80000, CRC(84a56f37) SHA1(926bcae5bd75a4172de2a2078718b2940c5c1966) )
	ROM_LOAD32_BYTE( "b14-02", 0x00001, 0x80000, CRC(6889186b) SHA1(3c38e281e8bf416a401c76ebb2d8ca95d09974b6) )
	ROM_LOAD32_BYTE( "b14-03", 0x00002, 0x80000, CRC(d1ed9e71) SHA1(26a6b2ca5bf6d70ad87f5c40c8e94ec542a2ec04) )
	ROM_LOAD32_BYTE( "b14-04", 0x00003, 0x80000, CRC(b63f0519) SHA1(e9a6b49effba0cae1ae3536a8584d3efa34ca8c3) )

	ROM_REGION( 0x10000, "user1", 0 ) // Zoom tables for zoom sprite h/w
	ROM_LOAD( "b14-30.88", 0x00000, 0x10000, CRC(dccb0c7f) SHA1(42f0af72f559133b74912a4478e1323062be4b77) )

	ROM_REGION( 0x2000, "user2", 0 ) // Unknown (Sharp LH763J-70 64kx8 OTP ROM)
	ROM_LOAD( "b14-31.90", 0x0000,  0x2000,  CRC(5c6b013d) SHA1(6d02d4560076213b6fb6fe856143bb533090603e) )

	ROM_REGION( 0x20000, "adpcm", 0 ) // ADPCM samples
	ROM_LOAD( "b14-28.103", 0x00000, 0x10000, CRC(df11d0ae) SHA1(259e1e6cc7ab100bfdb60e3d7a6bb46acb6fe2ea) )
	ROM_LOAD( "b14-29.109", 0x10000, 0x10000, CRC(7ad983e7) SHA1(a3515caf93d6dab86de06ee52d6a13a456507dbe) )
ROM_END


GAMEL( 1987, topspeed, 0,        topspeed, topspeed, driver_device, 0, ROT0, "Taito Corporation Japan",                     "Top Speed (World)",     MACHINE_SUPPORTS_SAVE, layout_topspeed )
GAMEL( 1987, topspeedu,topspeed, topspeed, fullthrl, driver_device, 0, ROT0, "Taito America Corporation (Romstar license)", "Top Speed (US)",        MACHINE_SUPPORTS_SAVE, layout_topspeed )
GAMEL( 1987, fullthrl, topspeed, topspeed, fullthrl, driver_device, 0, ROT0, "Taito Corporation",                           "Full Throttle (Japan)", MACHINE_SUPPORTS_SAVE, layout_topspeed )
