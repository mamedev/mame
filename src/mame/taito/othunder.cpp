// license:BSD-3-Clause
// copyright-holders:David Graves
// thanks-to:Richard Bush
/***************************************************************************

Operation Thunderbolt  (Taito)
---------------------

David Graves

(this is based on the F2 driver by Bryan McPhail, Brad Oliver, Andrew Prime,
Nicola Salmoria. Thanks to Richard Bush and the Raine team, whose open
source was very helpful in many areas particularly the sprites.)

*************************************************************************

Operation Thunderbolt
Taito, 1988

PCB Layout
----------

K1100381A
J1100166A MAIN PCB
|-----------------------------------------------------------------------|
|  VOL   VOL          TC0310FAM  YM3016 B67-13.40 16MHz 24MHz      PAL  |
|        TL074  |------|TC0310FAM TL074                                 |
|  MB3735 MB3735|TAITO |            Z80                                 |
|               |TC0140|            6264.46     26.686MHz    6116  6116 |
|               |SYT   |                                     6116  6116 |
|               |------|            YM2610                   6116  6116 |
|    ADC0808     B67-08.67                                   6116  6116 |
|          MB3771                  B67-07.44       |------|             |
|2               B67-06.66                         |TAITO |    |------| |
|8    |------|                                     |TC0020|    |TAITO | |
||    |TAITO |               62256.57 62256.55     |VAR   |    |TC0050| |
|W    |TC0220|       |------|                      |------|    |VDZ   | |
|A    |IOC   |       |TAITO |            B67-05.43             |------| |
|Y    |------|       |TC0100|  6116.56                                  |
|                    |SCN   |       6116.54          |------|           |
|    DSWA  DSWB      |------|         B67-05.43      |TAITO |           |
|                                                    |TC0050|           |
|  93C46                     |------|                |VDZ   |           |
|     |------|    B67-23.64  |TAITO |   |----|       |------|  B67-04.4 |
|     |TAITO |               |TC0320|   | 6  |                          |
| 555 |TC0110|    B67-20.63  |OBR   |   | 8  |  PAL  |------|  B67-03.3 |
|     |PCR   |               |------|   | 0  |       |TAITO |           |
|     |------|    B67-15.62   62256.52  | 0  |  PAL  |TC0050|  B67-02.2 |
| 6264.73 6264.74                       | 0  |       |VDZ   |           |
|     TC0070RGB   B67-14.61   62256.51  |----|  PAL  |------|  B67-01.1 |
|-----------------------------------------------------------------------|
Notes:
      68000 running at 12.000MHz [24/2]
      Z80 running at 4.000MHz [24/6]
      YM2610 running at 8.000MHz [16/2]
      62256.51 & 52 - Main program 68000 work RAM
      6264.46 - Sound program Z80 work RAM
      62256.55 & 57 - Background RAM, tied to TC0100SCN
      6116.56 & 54 - Sprite attribute/enable RAM, tied to TC0320OBR
      Other 6116 RAM - Sprite RAM, connected to TC0020VAR & TC0050VDZ
      6264.73 & 74 - Color RAM

      Taito Custom ICs -
                         TC0220IOC
                         TC0110PCR
                         TC0140SYT
                         TC0100SCN
                         TC0320OBR
                         TC0020VAR
                         TC0050VDZ (x3)
                         TC0070RGB (ceramic module)
                         TC0310FAM (ceramic module)
      VSync: 60.0552Hz
      HSync: 15.4938kHz

      Note: The hardware outputs reversed video because the cabinet uses a mirror.
      The video can't be flipped/reversed with the DIPs, it's fixed in hardware.

      PCB edge connector is 28-way non-JAMMA.
      The gun is NOT a light gun, it uses two 5k-ohm potentiometers for the X-Y position

               Solder | Parts
               -------+-------
               Ground | 1 Ground
               Ground | 2 Ground
                  +5v | 3 +5v
                  +5v | 4 +5v
                  -5v | 5 -5v
                  12v | 6 12v
                  Key | 7 Key
          - L Speaker | 8 + L Speaker
          - R Speaker | 9 + R Speaker
                      |10
                      |11
            1 P Start |12
            2 P Start |13
             1 P Bomb |14
          1 P Trigger |15 2 P Bomb
          2 P Trigger |16 Service Credit
               Coin B |17 Coin A
       Coin Counter B |18 Coin Counter A
                      |19
     2 P Gun Solenoid |20 1 P Gun Solenoid
          Video Green |21 Video Red
           Video Sync |22 Video Blue
   2 P Gun X Position |23 Ground
   2 P Gun Y Position |24 1 P Gun X Position
                      |25 1 P Gun Y Position
                      |26
               Ground |27 Ground
               Ground |28 Ground

*************************************************************************

Operation Thunderbolt operates on hardware very similar to the Taito Z
system, in particular the game Spacegun. The lightgun hardware in these
two (as well as the eeprom and calibration process) looks identical.

The game has 4 separate layers of graphics - one 64x64 tiled scrolling
background plane of 8x8 tiles, a similar foreground plane, a sprite plane,
and a text plane with character definitions held in ram.

The sprites are 16x8 tiles aggregated through a spritemap rom into 64x64
zoomable sprites.

The main difference is that Operation Thunderbolt uses only a single 68000
CPU, whereas Spacegun has twin 68Ks. (Operation Thunderbolt has a Z80
taking over sound duties, which Spacegun doesn't.)


custom ICs
----------
TC0020VAR     sprites??
TC0050VDZ x3  sprites??
TC0070RGB     video DAC
TC0100SCN     tilemaps
TC0110PCR     palette
TC0140SYT     main/sub CPU interface + sub cpu address decoder and I/O interface
TC0220IOC     I/O interface
TC0310FAM x2  sound volume and panning
TC0320OBR     sprites


memory map
----------
68000:

The address decoding is done by two PALs (IC37 and IC33). Part of the decoding,
and also interrupt control, is done by another PAL (IC36). Luckily this time,
the PALs HAVE been read, so the memory map is accurate :)

Address                  Dir Data             Name      Description
------------------------ --- ---------------- --------- -----------------------
000000xxxxxxxxxxxxxxxxx- R   xxxxxxxxxxxxxxxx TROM0     program ROM
000001xxxxxxxxxxxxxxxxx- R   xxxxxxxxxxxxxxxx TROM1     program ROM
00001000xxxxxxxxxxxxxxxx R/W xxxxxxxxxxxxxxxx TRAMH     work RAM
00001001-----------xxxx- R/W --------xxxxxxxx II/O      TC0220IOC
0001-----------------xxx R/W xxxxxxxxxxxxxxxx CLCS      TC0110PCR
0010--xxxxxxxxxxxxxxxxxx R/W xxxxxxxxxxxxxxxx SCCS      TC0100SCN
0011------------------xx R/W ------------xxxx SSCS      TC0140SYT
0100---------xxxxxxxxxxx R/W xxxxxxxxxxxxxxxx OBCS      TC0320OBR
0101----------------xxx- R/W --------xxxxxxxx A/DOE     ADC0808 A/D converter (for lightgun)
0110----------------000-   W ----------------           IRQ5 acknowledge (automatically enabled on reset)
0110----------------001-   W ---------------- 6PR       IRQ6 acknowledge (automatically enabled on reset)
0110----------------010-   W ---------------- n.c.
0110----------------011-   W ---------------- n.c.
0110----------------100-   W ---------------- n.c.
0110----------------101-   W ---------------- n.c.
0110----------------110-   W ---------------- n.c.
0110----------------111-   W ---------------- n.c.


Z80:

all of the address decoding is done by the TC0140SYT, which uses address bits
A0 and A9-A15, and data bits D0-D3.

Address          Dir Data     Name        Description
---------------- --- -------- ----------- -----------------------
00xxxxxxxxxxxxxx R   xxxxxxxx ROM         program ROM (TC0140SYT ROMCS0 pin)
01xxxxxxxxxxxxxx R   xxxxxxxx ROM         program ROM (banked) (TC0140SYT ROMCS0, ROMA14 and ROMA15 pins)
110xxxxxxxxxxxxx R/W xxxxxxxx SRAM        work RAM (TC0140SYT RAMCS pin)
1110000-------xx R/W xxxxxxxx OP_T        YM2610 (TC0140SYT OPX pin)
1110001--------x R/W ----xxxx             TC0140SYT control
1110010-------00   W ---xxxxx CH1_VOLR_CT TC0310FAM #1 CS2 (TC0140SYT CSA pin)
1110010-------01   W ---xxxxx CH1_VOLF_CT TC0310FAM #1 CS1 (TC0140SYT CSA pin)
1110010-------10   W ---xxxxx CH2_VOLR_CT TC0310FAM #2 CS2 (TC0140SYT CSA pin)
1110010-------11   W ---xxxxx CH2_VOLF_CT TC0310FAM #2 CS1 (TC0140SYT CSA pin)
1110011---------   W                      ??
1110100---------
1110101--------- R   -------x ROTARY 1    TC0140SYT I/OA0 pin
1110101--------- R   ------x- ROTARY 2    TC0140SYT I/OA1 pin
1110101--------- R   -----x-- ROTARY 3    TC0140SYT I/OA2 pin
1110110---------
1110111---------   W                      ??
1111000---------   W                      ??
1111001---------   W ----xxxx             TC0140SYT ROM bankswitch



Notes:
------
- The game checks an external input called ROTARY in the schematics, not
  mentioned in the manual. The ROTARY input controls the separation between
  left and right players, the game adjusts the values written to the external
  volume controller depending on it.
  Possible values are: 111 (max) 011 (high) -01 (med) --0 (low). It's a
  rotary control so only one bit is supposed to be low.

(Changed set led output to proper output port.  --HowardC)
- The outputs I'm turning on are actually the gun solenoid outputs, which
  would rattle the gun while firing.


- BM, 060108 - The original flyer for this game has screenshots which clearly
  show the background is 4 pixels to the left on several game stages (you can
  see the edge of sprites overlapping past the right edge).  Therefore I
  do not believe the TC0100SCN problem mentioned above actually exists.  The
  current emulation appears to be accurate.


Stephh's notes (based on the game M68000 code and some tests) :

1) 'othunder', 'othundrj' and 'othunduo'

  - Region stored at 0x03fffe.w
  - Sets :
      * 'othunder' : region = 0x0003
      * 'othundrj' : region = 0x0001
      * 'othunduo' : region = 0x0002
  - These 3 games are 100% the same, only region differs !
  - Coinage relies on the region (code at 0x000db2) :
      * 0x0001 (Japan) and 0x0002 (US) use TAITO_COINAGE_JAPAN_OLD
      * 0x0003 (World) and 0x0004 (licensed to xxx) use TAITO_COINAGE_WORLD
  - Notice screen only if region = 0x0001
  - According to the manual, DSWB bit 6 determines continue pricing :

    PORT_DIPNAME( 0x40, 0x40, DEF_STR( Continue_Price ) )
    PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0x40, "Same as Start" )

    However, many conditions are required to make it work due to code at 0x00e0c4 :
      * region must not be 0x0001
      * "Allow Continue" Dip Switch must be set to "Yes"
      * coinage must be 2C_1C for both slots
    This is why this Dip Switch has NO effect in the following sets :
      * 'othunder' : coinage can't be 2C_1C for the 2 slots (coin B)
      * 'othundrj' : region = 0x0001
  - DSWB bit 7 ("Language") affects parts of the texts (not the ones in "demo mode")
    but the voices are always in English regardless of the region !


2) 'othundu'

  - Region stored at 0x03fffe.w
  - Sets :
      * 'othundu' : region = 0x0002
  - Comparison with 'othunder' :
      * slightly different code at 0x023c4c
      * additional data from 0x023eee to 0x0240ed (0x0200 bytes)
      * same other notes as for 'othunder'


TODO:
-----

- The "FIRE!" arrows pointing to padlocks are not in perfect sync with the
  background scrolling. Should they?

- The quality of the zoomed sprites could probably be better. Drawing them as
  made by 16x8 tiles loses precision due to limitations of drawgfxzoom().

- Schematics show a OBPRI output to control sprite priority. This doesn't seem
  to be used however, and isn't hooked up. See tc0220ioc_w().

***************************************************************************/

#include "emu.h"
#include "othunder.h"
#include "taitoipt.h"
#include "taitosnd.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/adc0808.h"
#include "machine/eepromser.h"
#include "sound/flt_vol.h"
#include "sound/ymopn.h"
#include "screen.h"
#include "speaker.h"


/***********************************************************
                INTERRUPTS
***********************************************************/

void othunder_state::irq_ack_w(offs_t offset, u16 data)
{
	switch (offset)
	{
		case 0:
			m_maincpu->set_input_line(5, CLEAR_LINE);
			break;

		case 1:
			m_maincpu->set_input_line(6, CLEAR_LINE);
			break;
	}
}

void othunder_state::vblank_w(int state)
{
	if (state)
		m_maincpu->set_input_line(5, ASSERT_LINE);
}

void othunder_state::adc_eoc_w(int state)
{
	if (state)
		m_maincpu->set_input_line(6, ASSERT_LINE);
}


/******************************************************************
                    EEPROM

This is an earlier version of the eeprom used in some TaitoB games.
The eeprom unlock command is different, and the write/clock/reset
bits are different.
******************************************************************/

void othunder_state::eeprom_w(u8 data)
{

/*              0000000x    SOL-1 (gun solenoid)
                000000x0    SOL-2 (gun solenoid)
                00000x00    OBPRI (sprite priority)
                0000x000    (unused)
                000x0000    eeprom reset (active low)
                00x00000    eeprom clock
                0x000000    eeprom in data
                x0000000    eeprom out data  */

	/* Recoil Piston Motor Status */
	m_recoil_piston[0] = BIT(data, 0);
	m_recoil_piston[1] = BIT(data, 1);

	if (data & 4)
		popmessage("OBPRI SET!");

	m_eeprom->di_write(BIT(data, 6));
	m_eeprom->clk_write(BIT(data, 5));
	m_eeprom->cs_write(BIT(data, 4));
}

void othunder_state::coins_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x08);
}


/*****************************************
            SOUND
*****************************************/

void othunder_state::sound_bankswitch_w(u8 data)
{
	m_z80bank->set_entry(data & 3);
}

void othunder_state::tc0310fam_w(offs_t offset, u8 data)
{
	/* there are two TC0310FAM, one for CH1 and one for CH2 from the YM2610. The
	   PSG output is routed to both chips. */
	int voll, volr;

	m_pan[offset] = data & 0x1f;

	/* PSG output (single ANALOG OUT pin on the YM2610, but we have three channels
	   because we are using the AY-3-8910 emulation. */
	volr = (m_pan[0] + m_pan[2]) * 100 / (2 * 0x1f);
	voll = (m_pan[1] + m_pan[3]) * 100 / (2 * 0x1f);
	m_2610_0l->set_gain(voll / 100.0);
	m_2610_0r->set_gain(volr / 100.0);

	/* CH1 */
	volr = m_pan[0] * 100 / 0x1f;
	voll = m_pan[1] * 100 / 0x1f;
	m_2610_1l->set_gain(voll / 100.0);
	m_2610_1r->set_gain(volr / 100.0);

	/* CH2 */
	volr = m_pan[2] * 100 / 0x1f;
	voll = m_pan[3] * 100 / 0x1f;
	m_2610_2l->set_gain(voll / 100.0);
	m_2610_2r->set_gain(volr / 100.0);
}


/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

void othunder_state::othunder_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x08ffff).ram();
	map(0x090000, 0x09000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0x00ff);
//  map(0x09000c, 0x09000d).nopw();   /* ?? (keeps writing 0x77) */
	map(0x100000, 0x100007).rw(m_tc0110pcr, FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_rbswap_word_w));   /* palette */
	map(0x200000, 0x20ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* tilemaps */
	map(0x220000, 0x22000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x300001, 0x300001).w(m_tc0140syt, FUNC(tc0140syt_device::master_port_w));
	map(0x300003, 0x300003).rw(m_tc0140syt, FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0x400000, 0x4005ff).ram().share("spriteram");
	map(0x500000, 0x500007).rw("adc", FUNC(adc0808_device::data_r), FUNC(adc0808_device::address_offset_start_w)).umask16(0x00ff);
	map(0x600000, 0x600003).w(FUNC(othunder_state::irq_ack_w));
}


/***************************************************************************/

void othunder_state::z80_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr("z80bank");
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe003).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0xe200, 0xe200).nopr().w(m_tc0140syt, FUNC(tc0140syt_device::slave_port_w));
	map(0xe201, 0xe201).rw(m_tc0140syt, FUNC(tc0140syt_device::slave_comm_r), FUNC(tc0140syt_device::slave_comm_w));
	map(0xe400, 0xe403).w(FUNC(othunder_state::tc0310fam_w)); /* pan */
	map(0xe600, 0xe600).nopw(); /* ? */
	map(0xea00, 0xea00).portr("ROTARY");  /* rotary input */
	map(0xee00, 0xee00).nopw(); /* ? */
	map(0xf000, 0xf000).nopw(); /* ? */
	map(0xf200, 0xf200).w(FUNC(othunder_state::sound_bankswitch_w));
}



/***********************************************************
             INPUT PORTS, DIPs
***********************************************************/

static INPUT_PORTS_START( othunder )
	/* 0x090000 -> 0x08a000 */
	PORT_START("DSWA")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	TAITO_DSWA_BITS_2_TO_3
	TAITO_COINAGE_WORLD

	/* 0x090002 -> 0x08a002 */
	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Magazines/Rockets" )
	PORT_DIPSETTING(    0x0c, "5/3" )
	PORT_DIPSETTING(    0x08, "6/4" )
	PORT_DIPSETTING(    0x04, "7/5" )
	PORT_DIPSETTING(    0x00, "8/6" )
	PORT_DIPNAME( 0x30, 0x30, "Bullets per Magazine" )
	PORT_DIPSETTING(    0x00, "30" )
	PORT_DIPSETTING(    0x10, "35" )
	PORT_DIPSETTING(    0x30, "40" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                        /* see notes */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")   /* unused */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* speed of 13 is compromise between moving aim around screen fast
	   enough and being accurate enough not to miss targets. 20 is too
	   inaccurate, and 10 is too slow. */

	PORT_START("P1X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("P1Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_CROSSHAIR(Y, 1.0, -0.057, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13) PORT_PLAYER(1)

	PORT_START("P2X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("P2Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_CROSSHAIR(Y, 1.0, -0.057, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13) PORT_PLAYER(2)

	/* rotary volume control */
	PORT_START("ROTARY")
	PORT_CONFNAME( 0x07, 0x07, "Stereo Separation" )
	PORT_CONFSETTING(    0x07, "Maximum" )
	PORT_CONFSETTING(    0x03, DEF_STR( High ) )
	PORT_CONFSETTING(    0x01, DEF_STR( Medium ) )
	PORT_CONFSETTING(    0x00, DEF_STR( Low ) )
INPUT_PORTS_END

static INPUT_PORTS_START( othunderj )
	PORT_INCLUDE( othunder )

	PORT_MODIFY( "DSWA" )
	TAITO_COINAGE_JAPAN_OLD

	PORT_MODIFY( "DSWB" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )
INPUT_PORTS_END

static INPUT_PORTS_START( othunderu )
	PORT_INCLUDE( othunder )

	PORT_MODIFY( "DSWA" )
	TAITO_COINAGE_JAPAN_OLD

	PORT_MODIFY( "DSWB" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Continue_Price ) )        /* see notes */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, "Same as Start" )
INPUT_PORTS_END



/***********************************************************
                GFX DECODING
***********************************************************/

static const gfx_layout tile16x8_layout =
{
	16,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,16) },
	{ STEP16(0,1) },
	{ STEP8(0,16*4) },
	64*8
};

static GFXDECODE_START( gfx_othunder )
	GFXDECODE_ENTRY( "sprites", 0, tile16x8_layout, 0, 256 )   /* sprite parts */
GFXDECODE_END


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

void othunder_state::machine_start()
{
	m_recoil_piston.resolve();

	m_z80bank->configure_entries(0, 4, memregion("audiocpu")->base(), 0x4000);

	save_item(NAME(m_pan));
}

void othunder_state::othunder(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 24_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &othunder_state::othunder_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL/2/2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &othunder_state::z80_sound_map);

	EEPROM_93C46_16BIT(config, m_eeprom);

	adc0808_device &adc(ADC0808(config, "adc", 16_MHz_XTAL/2/2/8));
	adc.eoc_callback().set(FUNC(othunder_state::adc_eoc_w));
	adc.in_callback<0>().set_ioport("P1X");
	adc.in_callback<1>().set_ioport("P1Y");
	adc.in_callback<2>().set_ioport("P2X");
	adc.in_callback<3>().set_ioport("P2Y");

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::do_read)).lshift(7);
	m_tc0220ioc->write_3_callback().set(FUNC(othunder_state::eeprom_w));
	m_tc0220ioc->write_4_callback().set(FUNC(othunder_state::coins_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(26'686'000) / 4, 424, 0, 320, 262, 16, 256); // same as taito_z.cpp
	screen.set_screen_update(FUNC(othunder_state::screen_update));
	screen.set_palette(m_tc0110pcr);
	screen.screen_vblank().set(FUNC(othunder_state::vblank_w));

	GFXDECODE(config, m_gfxdecode, m_tc0110pcr, gfx_othunder);

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_offsets(4, 0);
	m_tc0100scn->set_palette(m_tc0110pcr);

	TC0110PCR(config, m_tc0110pcr, 0);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 16000000/2));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "2610.0l", 0.25);
	ymsnd.add_route(0, "2610.0r", 0.25);
	ymsnd.add_route(1, "2610.1l", 1.0);
	ymsnd.add_route(1, "2610.1r", 1.0);
	ymsnd.add_route(2, "2610.2l", 1.0);
	ymsnd.add_route(2, "2610.2r", 1.0);

	FILTER_VOLUME(config, "2610.0l").add_route(ALL_OUTPUTS, "speaker", 1.0);
	FILTER_VOLUME(config, "2610.0r").add_route(ALL_OUTPUTS, "speaker", 1.0);
	FILTER_VOLUME(config, "2610.1l").add_route(ALL_OUTPUTS, "speaker", 1.0);
	FILTER_VOLUME(config, "2610.1r").add_route(ALL_OUTPUTS, "speaker", 1.0);
	FILTER_VOLUME(config, "2610.2l").add_route(ALL_OUTPUTS, "speaker", 1.0);
	FILTER_VOLUME(config, "2610.2r").add_route(ALL_OUTPUTS, "speaker", 1.0);

	TC0140SYT(config, m_tc0140syt, 0);
	m_tc0140syt->nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	m_tc0140syt->reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


/***************************************************************************
                    DRIVERS
***************************************************************************/

ROM_START( othunder )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code */
	ROM_LOAD16_BYTE( "b67-20-1.ic63", 0x00000, 0x20000, CRC(851a453b) SHA1(48b8c379e78cd79463f1e24dc23816a97cf819b8) )
	ROM_LOAD16_BYTE( "b67-23-1.ic64", 0x00001, 0x20000, CRC(6e4f3d56) SHA1(99a29c0cbea0aea42355a036aa7f2174ca997872) )
	ROM_LOAD16_BYTE( "b67-14.ic61",   0x40000, 0x20000, CRC(7f3dd724) SHA1(2f2eeae0ee31e20082237b9a947c6848771eb73c) )
	ROM_LOAD16_BYTE( "b67-15.ic62",   0x40001, 0x20000, CRC(e84f62d0) SHA1(3b4a55a14dee7d592467fde9a75bde64deabd27d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "b67-13.ic40",   0x00000, 0x10000, CRC(2936b4b1) SHA1(39b41643464dd89e456ab6eb15a0ff0aef30afde) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b67-06.ic66", 0x00000, 0x80000, CRC(b9a38d64) SHA1(7ae8165b444d9da6ccdbc4a769535bcbb6738aaa) )     /* SCN */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b67-01.ic1", 0x00000, 0x80000, CRC(81ad9acb) SHA1(d9ad3f6332c6ca6b9872da57526a8158a3cf5b2f) ) /* OBJ: each rom has 1 bitplane, forming 16x8 tiles */
	ROM_LOAD64_WORD_SWAP( "b67-02.ic2", 0x00002, 0x80000, CRC(c20cd2fb) SHA1(b015e1fe167e19826aa451b45cd143d66a6db83c) )
	ROM_LOAD64_WORD_SWAP( "b67-03.ic3", 0x00004, 0x80000, CRC(bc9019ed) SHA1(7eddc83d71be97ce6637e6b35c226d58e6c39c3f) )
	ROM_LOAD64_WORD_SWAP( "b67-04.ic4", 0x00006, 0x80000, CRC(2af4c8af) SHA1(b2ae7aad0c59ffc368811f4bd5546dbb6860f9a9) )

	ROM_REGION16_LE( 0x80000, "sprmap_rom", 0 )
	ROM_LOAD16_WORD( "b67-05.ic43", 0x00000, 0x80000, CRC(9593e42b) SHA1(54b5538c302a1734ff4b752ab87a8c45d5c6b23d) )  /* index used to create 64x64 sprites on the fly */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "b67-08.ic67", 0x00000, 0x80000, CRC(458f41fb) SHA1(acca7c95acd1ae7a1cc51fb7fe644ad6d00ff5ac) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b67-07.ic44", 0x00000, 0x80000, CRC(4f834357) SHA1(f34705ce64870a8b24ec2639505079cc031fb719) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "plhs18p8b-b67-09.ic15", 0x0000, 0x0149, CRC(62035487) SHA1(5d9538ea9eabff324d274772b1e1fc9a9aec9100) )
	ROM_LOAD( "pal16l8a-b67-11.ic36",  0x0200, 0x0104, CRC(3177fb06) SHA1(c128277fe03342d9ec8da3c6e08a404a3f010547) )
	ROM_LOAD( "pal20l8b-b67-12.ic37",  0x0400, 0x0144, CRC(a47c2798) SHA1(8c963efd416b3f6586cb12afb9417dc95c2bc574) )
	ROM_LOAD( "pal20l8b-b67-10.ic33",  0x0600, 0x0144, CRC(4ced09c7) SHA1(519e6152cc5e4cb3ec24c4dc09101dddf22988aa) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "93c46_eeprom-othunder.ic86", 0x0000, 0x0080, CRC(3729b844) SHA1(f6bb41d293d1e47214f8b2d147991404f3278ebf) )
ROM_END

ROM_START( othundero )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code */
	ROM_LOAD16_BYTE( "b67-20.ic63",   0x00000, 0x20000, CRC(21439ea2) SHA1(d5b5a194e9698cf43513c0d56146772e8132ab07) )
	ROM_LOAD16_BYTE( "b67-23.ic64",   0x00001, 0x20000, CRC(789e9daa) SHA1(15bb0eec68aeea0b9f55889566338c9ce0ac9b5e) )
	ROM_LOAD16_BYTE( "b67-14.ic61",   0x40000, 0x20000, CRC(7f3dd724) SHA1(2f2eeae0ee31e20082237b9a947c6848771eb73c) )
	ROM_LOAD16_BYTE( "b67-15.ic62",   0x40001, 0x20000, CRC(e84f62d0) SHA1(3b4a55a14dee7d592467fde9a75bde64deabd27d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "b67-13.ic40",   0x00000, 0x10000, CRC(2936b4b1) SHA1(39b41643464dd89e456ab6eb15a0ff0aef30afde) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b67-06.ic66", 0x00000, 0x80000, CRC(b9a38d64) SHA1(7ae8165b444d9da6ccdbc4a769535bcbb6738aaa) )     /* SCN */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b67-01.ic1", 0x00000, 0x80000, CRC(81ad9acb) SHA1(d9ad3f6332c6ca6b9872da57526a8158a3cf5b2f) ) /* OBJ: each rom has 1 bitplane, forming 16x8 tiles */
	ROM_LOAD64_WORD_SWAP( "b67-02.ic2", 0x00002, 0x80000, CRC(c20cd2fb) SHA1(b015e1fe167e19826aa451b45cd143d66a6db83c) )
	ROM_LOAD64_WORD_SWAP( "b67-03.ic3", 0x00004, 0x80000, CRC(bc9019ed) SHA1(7eddc83d71be97ce6637e6b35c226d58e6c39c3f) )
	ROM_LOAD64_WORD_SWAP( "b67-04.ic4", 0x00006, 0x80000, CRC(2af4c8af) SHA1(b2ae7aad0c59ffc368811f4bd5546dbb6860f9a9) )

	ROM_REGION16_LE( 0x80000, "sprmap_rom", 0 )
	ROM_LOAD16_WORD( "b67-05.ic43", 0x00000, 0x80000, CRC(9593e42b) SHA1(54b5538c302a1734ff4b752ab87a8c45d5c6b23d) )  /* index used to create 64x64 sprites on the fly */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "b67-08.ic67", 0x00000, 0x80000, CRC(458f41fb) SHA1(acca7c95acd1ae7a1cc51fb7fe644ad6d00ff5ac) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b67-07.ic44", 0x00000, 0x80000, CRC(4f834357) SHA1(f34705ce64870a8b24ec2639505079cc031fb719) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "plhs18p8b-b67-09.ic15", 0x0000, 0x0149, CRC(62035487) SHA1(5d9538ea9eabff324d274772b1e1fc9a9aec9100) )
	ROM_LOAD( "pal16l8a-b67-11.ic36",  0x0200, 0x0104, CRC(3177fb06) SHA1(c128277fe03342d9ec8da3c6e08a404a3f010547) )
	ROM_LOAD( "pal20l8b-b67-12.ic37",  0x0400, 0x0144, CRC(a47c2798) SHA1(8c963efd416b3f6586cb12afb9417dc95c2bc574) )
	ROM_LOAD( "pal20l8b-b67-10.ic33",  0x0600, 0x0144, CRC(4ced09c7) SHA1(519e6152cc5e4cb3ec24c4dc09101dddf22988aa) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "93c46_eeprom-othunder.ic86", 0x0000, 0x0080, CRC(3729b844) SHA1(f6bb41d293d1e47214f8b2d147991404f3278ebf) )
ROM_END

ROM_START( othunderu )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code */
	ROM_LOAD16_BYTE( "b67-20-1.ic63", 0x00000, 0x20000, CRC(851a453b) SHA1(48b8c379e78cd79463f1e24dc23816a97cf819b8) )
	ROM_LOAD16_BYTE( "b67-22-1.ic64", 0x00001, 0x20000, CRC(19480dc0) SHA1(8bbc982c89f0878e7639330970df5aa93ecbb083) )
	ROM_LOAD16_BYTE( "b67-14.ic61",   0x40000, 0x20000, CRC(7f3dd724) SHA1(2f2eeae0ee31e20082237b9a947c6848771eb73c) )
	ROM_LOAD16_BYTE( "b67-15.ic62",   0x40001, 0x20000, CRC(e84f62d0) SHA1(3b4a55a14dee7d592467fde9a75bde64deabd27d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "b67-13.ic40",   0x00000, 0x10000, CRC(2936b4b1) SHA1(39b41643464dd89e456ab6eb15a0ff0aef30afde) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b67-06.ic66", 0x00000, 0x80000, CRC(b9a38d64) SHA1(7ae8165b444d9da6ccdbc4a769535bcbb6738aaa) )     /* SCN */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b67-01.ic1", 0x00000, 0x80000, CRC(81ad9acb) SHA1(d9ad3f6332c6ca6b9872da57526a8158a3cf5b2f) ) /* OBJ: each rom has 1 bitplane, forming 16x8 tiles */
	ROM_LOAD64_WORD_SWAP( "b67-02.ic2", 0x00002, 0x80000, CRC(c20cd2fb) SHA1(b015e1fe167e19826aa451b45cd143d66a6db83c) )
	ROM_LOAD64_WORD_SWAP( "b67-03.ic3", 0x00004, 0x80000, CRC(bc9019ed) SHA1(7eddc83d71be97ce6637e6b35c226d58e6c39c3f) )
	ROM_LOAD64_WORD_SWAP( "b67-04.ic4", 0x00006, 0x80000, CRC(2af4c8af) SHA1(b2ae7aad0c59ffc368811f4bd5546dbb6860f9a9) )

	ROM_REGION16_LE( 0x80000, "sprmap_rom", 0 )
	ROM_LOAD16_WORD( "b67-05.ic43", 0x00000, 0x80000, CRC(9593e42b) SHA1(54b5538c302a1734ff4b752ab87a8c45d5c6b23d) )  /* index used to create 64x64 sprites on the fly */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "b67-08.ic67", 0x00000, 0x80000, CRC(458f41fb) SHA1(acca7c95acd1ae7a1cc51fb7fe644ad6d00ff5ac) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b67-07.ic44", 0x00000, 0x80000, CRC(4f834357) SHA1(f34705ce64870a8b24ec2639505079cc031fb719) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "plhs18p8b-b67-09.ic15", 0x0000, 0x0149, CRC(62035487) SHA1(5d9538ea9eabff324d274772b1e1fc9a9aec9100) )
	ROM_LOAD( "pal16l8a-b67-11.ic36",  0x0200, 0x0104, CRC(3177fb06) SHA1(c128277fe03342d9ec8da3c6e08a404a3f010547) )
	ROM_LOAD( "pal20l8b-b67-12.ic37",  0x0400, 0x0144, CRC(a47c2798) SHA1(8c963efd416b3f6586cb12afb9417dc95c2bc574) )
	ROM_LOAD( "pal20l8b-b67-10.ic33",  0x0600, 0x0144, CRC(4ced09c7) SHA1(519e6152cc5e4cb3ec24c4dc09101dddf22988aa) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "93c46_eeprom-othunder.ic86", 0x0000, 0x0080, CRC(3729b844) SHA1(f6bb41d293d1e47214f8b2d147991404f3278ebf) )
ROM_END

ROM_START( othunderua )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code */
	ROM_LOAD16_BYTE( "b67-20.ic63",   0x00000, 0x20000, CRC(21439ea2) SHA1(d5b5a194e9698cf43513c0d56146772e8132ab07) )
	ROM_LOAD16_BYTE( "b67-22.ic64",   0x00001, 0x20000, CRC(0f99ad3c) SHA1(dd6c9e822470ca867ec01e642443a871e879bae5) )
	ROM_LOAD16_BYTE( "b67-14.ic61",   0x40000, 0x20000, CRC(7f3dd724) SHA1(2f2eeae0ee31e20082237b9a947c6848771eb73c) )
	ROM_LOAD16_BYTE( "b67-15.ic62",   0x40001, 0x20000, CRC(e84f62d0) SHA1(3b4a55a14dee7d592467fde9a75bde64deabd27d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "b67-13.ic40",   0x00000, 0x10000, CRC(2936b4b1) SHA1(39b41643464dd89e456ab6eb15a0ff0aef30afde) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b67-06.ic66", 0x00000, 0x80000, CRC(b9a38d64) SHA1(7ae8165b444d9da6ccdbc4a769535bcbb6738aaa) )     /* SCN */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b67-01.ic1", 0x00000, 0x80000, CRC(81ad9acb) SHA1(d9ad3f6332c6ca6b9872da57526a8158a3cf5b2f) ) /* OBJ: each rom has 1 bitplane, forming 16x8 tiles */
	ROM_LOAD64_WORD_SWAP( "b67-02.ic2", 0x00002, 0x80000, CRC(c20cd2fb) SHA1(b015e1fe167e19826aa451b45cd143d66a6db83c) )
	ROM_LOAD64_WORD_SWAP( "b67-03.ic3", 0x00004, 0x80000, CRC(bc9019ed) SHA1(7eddc83d71be97ce6637e6b35c226d58e6c39c3f) )
	ROM_LOAD64_WORD_SWAP( "b67-04.ic4", 0x00006, 0x80000, CRC(2af4c8af) SHA1(b2ae7aad0c59ffc368811f4bd5546dbb6860f9a9) )

	ROM_REGION16_LE( 0x80000, "sprmap_rom", 0 )
	ROM_LOAD16_WORD( "b67-05.ic43", 0x00000, 0x80000, CRC(9593e42b) SHA1(54b5538c302a1734ff4b752ab87a8c45d5c6b23d) )  /* index used to create 64x64 sprites on the fly */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "b67-08.ic67", 0x00000, 0x80000, CRC(458f41fb) SHA1(acca7c95acd1ae7a1cc51fb7fe644ad6d00ff5ac) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b67-07.ic44", 0x00000, 0x80000, CRC(4f834357) SHA1(f34705ce64870a8b24ec2639505079cc031fb719) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "plhs18p8b-b67-09.ic15", 0x0000, 0x0149, CRC(62035487) SHA1(5d9538ea9eabff324d274772b1e1fc9a9aec9100) )
	ROM_LOAD( "pal16l8a-b67-11.ic36",  0x0200, 0x0104, CRC(3177fb06) SHA1(c128277fe03342d9ec8da3c6e08a404a3f010547) )
	ROM_LOAD( "pal20l8b-b67-12.ic37",  0x0400, 0x0144, CRC(a47c2798) SHA1(8c963efd416b3f6586cb12afb9417dc95c2bc574) )
	ROM_LOAD( "pal20l8b-b67-10.ic33",  0x0600, 0x0144, CRC(4ced09c7) SHA1(519e6152cc5e4cb3ec24c4dc09101dddf22988aa) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "93c46_eeprom-othunder.ic86", 0x0000, 0x0080, CRC(3729b844) SHA1(f6bb41d293d1e47214f8b2d147991404f3278ebf) )
ROM_END

ROM_START( othunderj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code */
	ROM_LOAD16_BYTE( "b67-20.ic63",   0x00000, 0x20000, CRC(21439ea2) SHA1(d5b5a194e9698cf43513c0d56146772e8132ab07) )
	ROM_LOAD16_BYTE( "b67-21.ic64",   0x00001, 0x20000, CRC(9690fc86) SHA1(4e695554fc9cc91c5f8cff95dc290333bb56d571) )
	ROM_LOAD16_BYTE( "b67-14.ic61",   0x40000, 0x20000, CRC(7f3dd724) SHA1(2f2eeae0ee31e20082237b9a947c6848771eb73c) )
	ROM_LOAD16_BYTE( "b67-15.ic62",   0x40001, 0x20000, CRC(e84f62d0) SHA1(3b4a55a14dee7d592467fde9a75bde64deabd27d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "b67-13.ic40",   0x00000, 0x10000, CRC(2936b4b1) SHA1(39b41643464dd89e456ab6eb15a0ff0aef30afde) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b67-06.ic66", 0x00000, 0x80000, CRC(b9a38d64) SHA1(7ae8165b444d9da6ccdbc4a769535bcbb6738aaa) )     /* SCN */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b67-01.ic1", 0x00000, 0x80000, CRC(81ad9acb) SHA1(d9ad3f6332c6ca6b9872da57526a8158a3cf5b2f) ) /* OBJ: each rom has 1 bitplane, forming 16x8 tiles */
	ROM_LOAD64_WORD_SWAP( "b67-02.ic2", 0x00002, 0x80000, CRC(c20cd2fb) SHA1(b015e1fe167e19826aa451b45cd143d66a6db83c) )
	ROM_LOAD64_WORD_SWAP( "b67-03.ic3", 0x00004, 0x80000, CRC(bc9019ed) SHA1(7eddc83d71be97ce6637e6b35c226d58e6c39c3f) )
	ROM_LOAD64_WORD_SWAP( "b67-04.ic4", 0x00006, 0x80000, CRC(2af4c8af) SHA1(b2ae7aad0c59ffc368811f4bd5546dbb6860f9a9) )

	ROM_REGION16_LE( 0x80000, "sprmap_rom", 0 )
	ROM_LOAD16_WORD( "b67-05.ic43", 0x00000, 0x80000, CRC(9593e42b) SHA1(54b5538c302a1734ff4b752ab87a8c45d5c6b23d) )  /* index used to create 64x64 sprites on the fly */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "b67-08.ic67", 0x00000, 0x80000, CRC(458f41fb) SHA1(acca7c95acd1ae7a1cc51fb7fe644ad6d00ff5ac) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b67-07.ic44", 0x00000, 0x80000, CRC(4f834357) SHA1(f34705ce64870a8b24ec2639505079cc031fb719) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "plhs18p8b-b67-09.ic15", 0x0000, 0x0149, CRC(62035487) SHA1(5d9538ea9eabff324d274772b1e1fc9a9aec9100) )
	ROM_LOAD( "pal16l8a-b67-11.ic36",  0x0200, 0x0104, CRC(3177fb06) SHA1(c128277fe03342d9ec8da3c6e08a404a3f010547) )
	ROM_LOAD( "pal20l8b-b67-12.ic37",  0x0400, 0x0144, CRC(a47c2798) SHA1(8c963efd416b3f6586cb12afb9417dc95c2bc574) )
	ROM_LOAD( "pal20l8b-b67-10.ic33",  0x0600, 0x0144, CRC(4ced09c7) SHA1(519e6152cc5e4cb3ec24c4dc09101dddf22988aa) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "93c46_eeprom-othunder.ic86", 0x0000, 0x0080, CRC(3729b844) SHA1(f6bb41d293d1e47214f8b2d147991404f3278ebf) )
ROM_END

ROM_START( othunderjsc ) // SC stands for Shopping Center. It was put in a smaller, single player cabinet aimed at children
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 512K for 68000 code */
	ROM_LOAD16_BYTE( "b67-24.ic63",   0x00000, 0x20000, CRC(18670e0b) SHA1(d848623046905471e3f859db6f179e1e4b3e4d51) ) // 27C1001
	ROM_LOAD16_BYTE( "b67-25.ic64",   0x00001, 0x20000, CRC(3d422991) SHA1(22d18fdd9c4511774e0a4b246a293d27f7498735) ) // 27C1001
	ROM_LOAD16_BYTE( "b67-14.ic61",   0x40000, 0x20000, CRC(7f3dd724) SHA1(2f2eeae0ee31e20082237b9a947c6848771eb73c) )
	ROM_LOAD16_BYTE( "b67-15.ic62",   0x40001, 0x20000, CRC(e84f62d0) SHA1(3b4a55a14dee7d592467fde9a75bde64deabd27d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "b67-13.ic40",   0x00000, 0x10000, CRC(2936b4b1) SHA1(39b41643464dd89e456ab6eb15a0ff0aef30afde) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "b67-06.ic66", 0x00000, 0x80000, CRC(b9a38d64) SHA1(7ae8165b444d9da6ccdbc4a769535bcbb6738aaa) )     /* SCN */

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "b67-01.ic1", 0x00000, 0x80000, CRC(81ad9acb) SHA1(d9ad3f6332c6ca6b9872da57526a8158a3cf5b2f) ) /* OBJ: each rom has 1 bitplane, forming 16x8 tiles */
	ROM_LOAD64_WORD_SWAP( "b67-02.ic2", 0x00002, 0x80000, CRC(c20cd2fb) SHA1(b015e1fe167e19826aa451b45cd143d66a6db83c) )
	ROM_LOAD64_WORD_SWAP( "b67-03.ic3", 0x00004, 0x80000, CRC(bc9019ed) SHA1(7eddc83d71be97ce6637e6b35c226d58e6c39c3f) )
	ROM_LOAD64_WORD_SWAP( "b67-04.ic4", 0x00006, 0x80000, CRC(2af4c8af) SHA1(b2ae7aad0c59ffc368811f4bd5546dbb6860f9a9) )

	ROM_REGION16_LE( 0x80000, "sprmap_rom", 0 )
	ROM_LOAD16_WORD( "b67-05.ic43", 0x00000, 0x80000, CRC(9593e42b) SHA1(54b5538c302a1734ff4b752ab87a8c45d5c6b23d) )  /* index used to create 64x64 sprites on the fly */

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "b67-08.ic67", 0x00000, 0x80000, CRC(458f41fb) SHA1(acca7c95acd1ae7a1cc51fb7fe644ad6d00ff5ac) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "b67-07.ic44", 0x00000, 0x80000, CRC(4f834357) SHA1(f34705ce64870a8b24ec2639505079cc031fb719) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "plhs18p8b-b67-09.ic15", 0x0000, 0x0149, CRC(62035487) SHA1(5d9538ea9eabff324d274772b1e1fc9a9aec9100) )
	ROM_LOAD( "pal16l8a-b67-11.ic36",  0x0200, 0x0104, CRC(3177fb06) SHA1(c128277fe03342d9ec8da3c6e08a404a3f010547) )
	ROM_LOAD( "pal20l8b-b67-12.ic37",  0x0400, 0x0144, CRC(a47c2798) SHA1(8c963efd416b3f6586cb12afb9417dc95c2bc574) )
	ROM_LOAD( "pal20l8b-b67-10.ic33",  0x0600, 0x0144, CRC(4ced09c7) SHA1(519e6152cc5e4cb3ec24c4dc09101dddf22988aa) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "93c46_eeprom-othunder.ic86", 0x0000, 0x0080, CRC(3729b844) SHA1(f6bb41d293d1e47214f8b2d147991404f3278ebf) )
ROM_END

GAME( 1988, othunder,    0,        othunder, othunder,  othunder_state, empty_init, ORIENTATION_FLIP_X, "Taito Corporation Japan",   "Operation Thunderbolt (World, rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, othundero,   othunder, othunder, othunder,  othunder_state, empty_init, ORIENTATION_FLIP_X, "Taito Corporation Japan",   "Operation Thunderbolt (World)",        MACHINE_SUPPORTS_SAVE )
GAME( 1988, othunderu,   othunder, othunder, othunderu, othunder_state, empty_init, ORIENTATION_FLIP_X, "Taito America Corporation", "Operation Thunderbolt (US, rev 1)",    MACHINE_SUPPORTS_SAVE )
GAME( 1988, othunderua,  othunder, othunder, othunderu, othunder_state, empty_init, ORIENTATION_FLIP_X, "Taito America Corporation", "Operation Thunderbolt (US)",           MACHINE_SUPPORTS_SAVE )
GAME( 1988, othunderj,   othunder, othunder, othunderj, othunder_state, empty_init, ORIENTATION_FLIP_X, "Taito Corporation",         "Operation Thunderbolt (Japan)",        MACHINE_SUPPORTS_SAVE )
GAME( 1988, othunderjsc, othunder, othunder, othunderj, othunder_state, empty_init, ORIENTATION_FLIP_X, "Taito Corporation",         "Operation Thunderbolt (Japan, SC)",    MACHINE_SUPPORTS_SAVE )
