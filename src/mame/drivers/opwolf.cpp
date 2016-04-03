// license:GPL-2.0+
// copyright-holders:David Graves, Jarek Burczynski,Stephane Humbert
// thanks-to:Richard Bush
/****************************************************************************

Operation Wolf  (c) Taito 1987
==============

David Graves, Jarek Burczynski
C-Chip emulation by Bryan McPhail

Sources:    MAME Rastan driver
            MAME Taito F2 driver
            Raine source - many thanks to Richard Bush
              and the Raine Team.

Main CPU: MC68000 uses irq 5.
Sound   : Z80 & YM2151 & MSM5205


Operation Wolf uses similar hardware to Rainbow Islands and Rastan.
The screen layout and registers and sprites appear to be identical.

Taito TC0030CMD chip labeled B20-18 (at least for the US boards)
Taito PC060HA looks like it might be a DIP28 Fujitsu MB884x chip
There are 4 socketed PALs (DIP20 type PAL16L8ACN) labeled B20-09
      through B20-12 (not read)

OSC:  Main board: 16MHz, 12MHz & 26.686MHz
     Sound board: 8MHz (Next to Z80 & YM2151)
CPU: TS68000CP8 (Rated for 8MHz)

Gun Travel
----------

Horizontal gun travel maybe could be widened to include more
of the status bar (you can shoot enemies underneath it).

To keep the input span 0-255 a multiplier (300/256 ?)
would be used.

Operation Wolf
Taito, 1987

PCB Layouts
-----------

SOUND BOARD
J1100122A (J1100122B) \ <-- minor rev, but no difference on PCB
K1100268A (K1100268B) /
K1100268A OPERATION WOLF (sticker)
|-----------------------------------------|
|X                S              8464     |
|        VR1         PC060HA     B20-07.10|
|N       MB3735       YM2151        Z80   |
|               SEIBU YM3012          8MHz|
|               HB-41                     |
|                                         |
|        VR2                              |
|        MB3735                           |
|    TL074                                |
|T       TC0060DCA                        |
|        TC0060DCA                        |
|    TL074                                |
|                                         |
|    M5205                                |
|    384kHz                               |
|    M5205                       B20-08.21|
|-----------------------------------------|
Notes:
      Z80       - clock 4.000MHz [8/2]
      YM2151    - clock 4.000MHz [8/2]
      YM3012    - 2-channel serial input floating D/A converter
      M5205     - clock 384kHz (via resonator)
      HB-41     - Seibu HB-41 ceramic module (sound/filter related)
      MB3735    - Audio power amp IC
      TL074     - Op amp
      8464      - 8kx8 SRAM
      PC060HA   - Taito PC060HA CIU custom IC (DIP28). Manufactured by Fujitsu
      B20-07.10 - 27C512 EPROM (DIP28)
      B20-08.21 - 234000 mask ROM (DIP40)
      VR1/2     - Volume pots
      S         - 30 pin flat cable joining to main board (for main board/sound board communication)
      X         - 5 pin connector for sound output (left+right speakers)
      N         - 10 pin connector for power inputs
      T         - 18-way edge connector (single sided, pins on solder side only)

      Pinouts
      -------
              X  Connector            N  Connector      T  Connector
              ------------            ------------      ------------
              1  Right Speaker (+)    1  GND            1  GND
              2  Right Speaker (-)    2  GND            2  GND
              3  Left  Speaker (+)    3  not used       3  GND
              4  not used             4  +5 Volts       4  GND
              5  Left  Speaker (-)    5  +5 Volts       5  Optical sensor input from gun
                                      6  not used       6  not used
                                      7  -5 Volts       7  not used
                                      8  +12 Volts      8  not used
                                      9  not used       9  not used
                                      10 not used       10 not used
                                                        11 not used
                                                        12 not used
                                                        13 not used
                                                        14 not used
                                                        15 +5 Volts
                                                        16 +5 Volts
                                                        17 +5 Volts
                                                        18 +5 Volts
MAIN PCB
J1100130A
K1100288A
M4300084F OPERATION WOLF (sticker)
K1100303A OPERATION WOLF (sticker)
|----------------------------------------------------|
|         SWB SWA        S      M                    |
| 12MHz                                       16MHz  |
|H               6116                       26.686MHz|
|      TC0070RGB 6116             B20-09.19 MB3771   |
|                                                    |
|                                  |-------|B20-13.13|
|                    2018 2018     |TAITO  |         |
|G     PC050CM       2018 2018     |PC080SN|         |
|                       |-------|  |-------|         |
|   B20-18.73           |TAITO  |               DIP28|
|   B20-14.72           |PC0900J|      43256  DIP28  |
| B20-12.92             |-------|          43256     |
|               DIP28      B20-03-2.30               |
|    68000      DIP28      B20-19.29  8464  8464     |
| B20-11.85     B20-05-2.40  DIP28                   |
| B20-10.84     B20-04.39    DIP28                   |
|----------------------------------------------------|
Notes:
      68000       - clock 8.000MHz [16/2]
      6116        - 2k x8 SRAM (color RAM)
      43256       - 32k x8 SRAM (tile RAM)
      8464        - 8k x8 SRAM (work RAM)
      2018        - 2k x8 SRAM (sprite RAM)
      PC080SN     - Taito custom tilemap generator IC (connected to 43256 SRAM)
      PC0900J     - Taito custom sprite generator IC (connected to 2018 SRAM)
      DIP28       - unpopulated DIP28 socket
      B20-18.73   - Taito custom C-Chip marked 'TC0030CMD, with sticker 'B20-18'
      TC0070RGB   - Taito custom ceramic module RGB mixer IC
      PC050CM     - Taito custom ceramic module (input related functions)
      MB3771      - Fujitsu MB3771 master reset IC
      S           - 30 pin flat cable connector joining to sound PCB
      M           - 4 pin connector for gun vibration motor power and grenade button
      H           - 12 pin connector for power input
      G           - 44-way edge connector
      B20-13/14   - MN234000 mask ROM
      B20-03.30   - \
      B20-19.29   -  |
      B20-04.39   -  | 27C512 EPROM
      B20-05.40   - _|
      B20-09.19   - \
      B20-10.84   -  |
      B20-11.85   -  | MMI PAL16L8ACN
      B20-12.92   - _|

      Measurements
      ------------
      XTAL1 - 26.68558MHz
      XTAL2 - 15.99965MHz
      XTAL3 - 11.9999MHz
      VSync - 60.0551Hz
      HSync - 15.6742kHz

      Pinouts
      -------
              M  Connector               H  Connector           G  Connector
              ------------               ------------           ------------
              1  BUTTON 2 (grenade)      1  GND                Solder   Parts
              2  not used                2  GND                GND  A   1  GND
              3  +12V (to motor in gun)  3  GND          VIDEO GND  B   2  RED
              4  -12V (to motor in gun)  4  GND               BLUE  C   3  GREEN
                                         5  +5 Volts      not used  D   4  SYNC
                                         6  +5 Volts      not used  E   5  not used
                                         7  +5 Volts      not used  F   6  not used
                                         8  -5 Volts      not used  H   7  not used
                                         9  +13 Volts*      COIN B  J   8  COIN A
                                         10 not used       METER B  K   9  METER A
                                         11 +12 Volts    LOCKOUT B  L  10  LOCKOUT A
                                         12 +12 Volts         TILT  M  11  SERVICE CREDIT
                                   * leave unconnected!   not used  N  12  START
                                                          not used  P  13  not used
                                                          not used  R  14  not used
                                                          not used  S  15  not used
                                                          not used  T  16  not used
                                                          not used  U  17  not used
                                                          not used  V  18  not used
                                                          not used  W  19  not used
                                                          not used  X  20  not used
                                                          not used  Y  21  BUTTON 1 (bullets)
                                                          not used  Z  22  not used



Gun board (inside gun)

J9100095A
K9100122A
|-----------|
|CN1        |
|           |
|-----------|
Notes:
      PCB contains mostly caps/resistors and three small opto-related 8-pin ICs
      CN1 - 4 pin connector joins to connector M on main board


Stephh's notes (based on the game M68000 code and some tests) :

1) 'opwolf' and 'opwolfu'

  - Region stored at 0x03fffe.w
  - Sets :
      * 'opwolf'  : region = 0x0003
      * 'opwolfu' : region = 0x0002
  - These 2 games are 100% the same, only region and gun offsets differ !
  - Coinage relies on the region (code at 0x00bea4) :
      * 0x0001 (Japan) and 0x0002 (US) use TAITO_COINAGE_JAPAN_OLD
      * 0x0003 (World) and 0x0004 (Japan, licensed to Taito America ?) use TAITO_COINAGE_WORLD
  - Gun offsets are stored at 0x03ffaf.b (Y) and 0x03ffb1.b (X);
    these values are checked via routine at 0x000b76
  - Notice screen only if region = 0x0001
  - When "Language" Dip Switch is set to Japanese, you can also select your starting level;
    however, only the 4 first levels are available ! Correct behaviour ?
  - There's code at 0x00e090 that tests contents of 0x03ffb2.w :
    if it is 0x0000, the highscore isn't displayed, but value = 0x00ff

2) 'opwolfa'

  - Region stored at 0x03fffe.w
  - Sets :
      * 'opwolfa' : region = 0x0003
  - There is only ONE byte of difference at 0x03fff5.b with 'opwolf'
    but its effect is unknown as this address doesn't seem to be read !

3) 'opwolfb'

  - Region stored at 0x03fffe.w
  - Sets :
      * 'opwolfb' : region = 0x0003
  - Comparison with 'opwolf' :
      * gun offsets are the same (so why are they different in the INIT function ?)
      * all reference to TAITO and "Operation Wolf" have been changed or "blanked"
      * "(c) 1988 BEAR CORPORATION KOREA" / "ALL RIGHTS RESERVED"
      * ROM check test "noped" (code at 0x00bb72)
  - Notes on bootleg c-chip (similar to what is in machine/opwolf.c) :
      * always Engish language (thus the Dip Switch change to "Unused")
      * round 4 in "demo mode" instead of round 5
      * "service" button doesn't add credits (it works in the "test mode" though)
      * if you die after round 6, difficulty isn't reset for the next game


TODO
====

Need to verify Opwolf against original board: various reports
claim there are discrepancies (perhaps limitations of the fake
Z80 c-chip substitute to blame?).

There are a few unmapped writes for the sound Z80 in the log.

Unknown writes to the MSM5205 control addresses

Raine source has standard Asuka/Mofflot sprite/tile priority:
0x2000 in sprite_ctrl puts all sprites under top bg layer. But
Raine simply kludges in this value, failing to read it from a
register. So what is controlling priority.


***************************************************************************/

/* Define clocks based on actual OSC on the PCB */

#define CPU_CLOCK       (XTAL_16MHz / 2)    /* clock for 68000 */
#define SOUND_CPU_CLOCK     (XTAL_8MHz / 2)     /* clock for Z80 sound CPU */

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "includes/taitoipt.h"
#include "audio/taitosnd.h"
#include "sound/2151intf.h"
#include "sound/msm5205.h"
#include "includes/opwolf.h"

READ16_MEMBER(opwolf_state::cchip_r)
{
	return m_cchip_ram[offset];
}

WRITE16_MEMBER(opwolf_state::cchip_w)
{
	m_cchip_ram[offset] = data &0xff;
}

/**********************************************************
                GAME INPUTS
**********************************************************/

#define P1X_PORT_TAG     "P1X"
#define P1Y_PORT_TAG     "P1Y"

READ16_MEMBER(opwolf_state::opwolf_in_r)
{
	static const char *const inname[2] = { "IN0", "IN1" };
	return ioport(inname[offset])->read();
}

READ16_MEMBER(opwolf_state::opwolf_dsw_r)
{
	static const char *const dswname[2] = { "DSWA", "DSWB" };
	return ioport(dswname[offset])->read();
}

READ16_MEMBER(opwolf_state::opwolf_lightgun_r)
{
	static const char *const dswname[2] = { "IN2", "IN3" };
	return ioport(dswname[offset])->read();
}

READ8_MEMBER(opwolf_state::z80_input1_r)
{
	return ioport("IN0")->read();   /* irrelevant mirror ? */
}

READ8_MEMBER(opwolf_state::z80_input2_r)
{
	return ioport("IN0")->read();   /* needed for coins */
}


/******************************************************
                SOUND
******************************************************/

WRITE8_MEMBER(opwolf_state::sound_bankswitch_w)
{
	membank("z80bank")->set_entry(data & 0x03);
}

/***********************************************************
             MEMORY STRUCTURES
***********************************************************/


static ADDRESS_MAP_START( opwolf_map, AS_PROGRAM, 16, opwolf_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x0f0000, 0x0f07ff) AM_MIRROR(0xf000) AM_READ(opwolf_cchip_data_r)
	AM_RANGE(0x0f0802, 0x0f0803) AM_MIRROR(0xf000) AM_READ(opwolf_cchip_status_r)
	AM_RANGE(0x0ff000, 0x0ff7ff) AM_WRITE(opwolf_cchip_data_w)
	AM_RANGE(0x0ff802, 0x0ff803) AM_WRITE(opwolf_cchip_status_w)
	AM_RANGE(0x0ffc00, 0x0ffc01) AM_WRITE(opwolf_cchip_bank_w)
	AM_RANGE(0x100000, 0x107fff) AM_RAM
	AM_RANGE(0x200000, 0x200fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x380000, 0x380003) AM_READ(opwolf_dsw_r)          /* dip switches */
	AM_RANGE(0x380000, 0x380003) AM_WRITE(opwolf_spritectrl_w)  // usually 0x4, changes when you fire
	AM_RANGE(0x3a0000, 0x3a0003) AM_READ(opwolf_lightgun_r)     /* lightgun, read at $11e0/6 */
	AM_RANGE(0x3c0000, 0x3c0001) AM_WRITENOP                    /* watchdog ?? */
	AM_RANGE(0x3e0000, 0x3e0001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_device, master_port_w, 0xff00)
	AM_RANGE(0x3e0002, 0x3e0003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w, 0xff00)
	AM_RANGE(0xc00000, 0xc0ffff) AM_DEVREADWRITE("pc080sn", pc080sn_device, word_r, word_w)
	AM_RANGE(0xc10000, 0xc1ffff) AM_WRITEONLY                   /* error in init code (?) */
	AM_RANGE(0xc20000, 0xc20003) AM_DEVWRITE("pc080sn", pc080sn_device, yscroll_word_w)
	AM_RANGE(0xc40000, 0xc40003) AM_DEVWRITE("pc080sn", pc080sn_device, xscroll_word_w)
	AM_RANGE(0xc50000, 0xc50003) AM_DEVWRITE("pc080sn", pc080sn_device, ctrl_word_w)
	AM_RANGE(0xd00000, 0xd03fff) AM_DEVREADWRITE("pc090oj", pc090oj_device, word_r, word_w)  /* sprite ram */
ADDRESS_MAP_END


static ADDRESS_MAP_START( opwolfb_map, AS_PROGRAM, 16, opwolf_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x0f0008, 0x0f000b) AM_READ(opwolf_in_r)           /* coins and buttons */
	AM_RANGE(0x0ff000, 0x0fffff) AM_READWRITE(cchip_r,cchip_w)
	AM_RANGE(0x100000, 0x107fff) AM_RAM
	AM_RANGE(0x200000, 0x200fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x380000, 0x380003) AM_READ(opwolf_dsw_r)          /* dip switches */
	AM_RANGE(0x380000, 0x380003) AM_WRITE(opwolf_spritectrl_w)  // usually 0x4, changes when you fire
	AM_RANGE(0x3a0000, 0x3a0003) AM_READ(opwolf_lightgun_r)     /* lightgun, read at $11e0/6 */
	AM_RANGE(0x3c0000, 0x3c0001) AM_WRITENOP                    /* watchdog ?? */
	AM_RANGE(0x3e0000, 0x3e0001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_device, master_port_w, 0xff00)
	AM_RANGE(0x3e0002, 0x3e0003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w, 0xff00)
	AM_RANGE(0xc00000, 0xc0ffff) AM_DEVREADWRITE("pc080sn", pc080sn_device, word_r, word_w)
	AM_RANGE(0xc10000, 0xc1ffff) AM_WRITEONLY                   /* error in init code (?) */
	AM_RANGE(0xc20000, 0xc20003) AM_DEVWRITE("pc080sn", pc080sn_device, yscroll_word_w)
	AM_RANGE(0xc40000, 0xc40003) AM_DEVWRITE("pc080sn", pc080sn_device, xscroll_word_w)
	AM_RANGE(0xc50000, 0xc50003) AM_DEVWRITE("pc080sn", pc080sn_device, ctrl_word_w)
	AM_RANGE(0xd00000, 0xd03fff) AM_DEVREADWRITE("pc090oj", pc090oj_device, word_r, word_w)  /* sprite ram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( opwolfp_map, AS_PROGRAM, 16, opwolf_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x107fff) AM_RAM
	
	AM_RANGE(0x200000, 0x200fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x380000, 0x380003) AM_READ(opwolf_dsw_r)          /* dip switches */
	AM_RANGE(0x380000, 0x380003) AM_WRITE(opwolf_spritectrl_w)  // usually 0x4, changes when you fire
	AM_RANGE(0x3a0000, 0x3a0003) AM_READ(opwolf_lightgun_r)     /* lightgun, read at $11e0/6 (AND INPUTS) */
	AM_RANGE(0x3c0000, 0x3c0001) AM_WRITENOP                    /* watchdog ?? */
	AM_RANGE(0x3e0000, 0x3e0001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_device, master_port_w, 0xff00)
	AM_RANGE(0x3e0002, 0x3e0003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w, 0xff00)
	AM_RANGE(0xc00000, 0xc0ffff) AM_DEVREADWRITE("pc080sn", pc080sn_device, word_r, word_w)
	AM_RANGE(0xc10000, 0xc1ffff) AM_WRITEONLY                   /* error in init code (?) */
	AM_RANGE(0xc20000, 0xc20003) AM_DEVWRITE("pc080sn", pc080sn_device, yscroll_word_w)
	AM_RANGE(0xc40000, 0xc40003) AM_DEVWRITE("pc080sn", pc080sn_device, xscroll_word_w)
	AM_RANGE(0xc50000, 0xc50003) AM_DEVWRITE("pc080sn", pc080sn_device, ctrl_word_w)
	AM_RANGE(0xd00000, 0xd03fff) AM_DEVREADWRITE("pc090oj", pc090oj_device, word_r, word_w)  /* sprite ram */
ADDRESS_MAP_END


/***************************************************************************
    This extra Z80 substitutes for the c-chip in the bootleg
 */

static ADDRESS_MAP_START( opwolfb_sub_z80_map, AS_PROGRAM, 8, opwolf_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8800, 0x8800) AM_READ(z80_input1_r)  /* read at PC=$637: poked to $c004 */
	AM_RANGE(0x9000, 0x9000) AM_WRITENOP            /* unknown write, 0 then 1 each interrupt */
	AM_RANGE(0x9800, 0x9800) AM_READ(z80_input2_r)  /* read at PC=$631: poked to $c005 */
	AM_RANGE(0xa000, 0xa000) AM_WRITENOP    /* IRQ acknowledge (unimplemented) */
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("cchip_ram")
ADDRESS_MAP_END


/***************************************************************************/


//static UINT8 adpcm_d[0x08];
//0 - start ROM offset LSB
//1 - start ROM offset MSB
//2 - end ROM offset LSB
//3 - end ROM offset MSB
//start & end need to be multiplied by 16 to get a proper _byte_ address in adpcm ROM
//4 - always zero write (start trigger ?)
//5 - different values
//6 - different values

void opwolf_state::machine_start()
{
	save_item(NAME(m_sprite_ctrl));
	save_item(NAME(m_sprites_flipscreen));

	save_item(NAME(m_adpcm_b));
	save_item(NAME(m_adpcm_c));
	save_item(NAME(m_adpcm_pos));
	save_item(NAME(m_adpcm_end));
}

MACHINE_RESET_MEMBER(opwolf_state,opwolf)
{
	m_adpcm_b[0] = m_adpcm_b[1] = 0;
	m_adpcm_c[0] = m_adpcm_c[1] = 0;
	m_adpcm_pos[0] = m_adpcm_pos[1] = 0;
	m_adpcm_end[0] = m_adpcm_end[1] = 0;
	m_adpcm_data[0] = m_adpcm_data[1] = -1;

	m_sprite_ctrl = 0;
	m_sprites_flipscreen = 0;

	m_msm1->reset_w(1);
	m_msm2->reset_w(1);
}

void opwolf_state::opwolf_msm5205_vck(msm5205_device *device,int chip)
{
	if (m_adpcm_data[chip] != -1)
	{
		device->data_w(m_adpcm_data[chip] & 0x0f);
		m_adpcm_data[chip] = -1;
		if (m_adpcm_pos[chip] == m_adpcm_end[chip])
			device->reset_w(1);
	}
	else
	{
		m_adpcm_data[chip] = memregion("adpcm")->base()[m_adpcm_pos[chip]];
		m_adpcm_pos[chip] = (m_adpcm_pos[chip] + 1) & 0x7ffff;
		device->data_w(m_adpcm_data[chip] >> 4);
	}
}
WRITE_LINE_MEMBER(opwolf_state::opwolf_msm5205_vck_1)
{
	opwolf_msm5205_vck(m_msm1, 0);
}
WRITE_LINE_MEMBER(opwolf_state::opwolf_msm5205_vck_2)
{
	opwolf_msm5205_vck(m_msm2, 1);
}

WRITE8_MEMBER(opwolf_state::opwolf_adpcm_b_w)
{
	int start;
	int end;

	m_adpcm_b[offset] = data;

	if (offset == 0x04) //trigger ?
	{
		start = m_adpcm_b[0] + m_adpcm_b[1] * 256;
		end   = m_adpcm_b[2] + m_adpcm_b[3] * 256;
		start *= 16;
		end   *= 16;
		m_adpcm_pos[0] = start;
		m_adpcm_end[0] = end;
		m_msm1->reset_w(0);
	}

//  logerror("CPU #1     b00%i-data=%2x   pc=%4x\n",offset,data,space.device().safe_pc() );
}


WRITE8_MEMBER(opwolf_state::opwolf_adpcm_c_w)
{
	int start;
	int end;

	m_adpcm_c[offset] = data;

	if (offset == 0x04) //trigger ?
	{
		start = m_adpcm_c[0] + m_adpcm_c[1] * 256;
		end   = m_adpcm_c[2] + m_adpcm_c[3] * 256;
		start *= 16;
		end   *= 16;
		m_adpcm_pos[1] = start;
		m_adpcm_end[1] = end;
		m_msm2->reset_w(0);
	}

//  logerror("CPU #1     c00%i-data=%2x   pc=%4x\n",offset,data,space.device().safe_pc() );
}


WRITE8_MEMBER(opwolf_state::opwolf_adpcm_d_w)
{
//   logerror("CPU #1         d00%i-data=%2x   pc=%4x\n",offset,data,space.device().safe_pc() );
}

WRITE8_MEMBER(opwolf_state::opwolf_adpcm_e_w)
{
//  logerror("CPU #1         e00%i-data=%2x   pc=%4x\n",offset,data,space.device().safe_pc() );
}

static ADDRESS_MAP_START( opwolf_sound_z80_map, AS_PROGRAM, 8, opwolf_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("z80bank")
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9001) AM_DEVREADWRITE("ymsnd", ym2151_device,read,write)
	AM_RANGE(0x9002, 0x9100) AM_READNOP
	AM_RANGE(0xa000, 0xa000) AM_DEVWRITE("tc0140syt", tc0140syt_device, slave_port_w)
	AM_RANGE(0xa001, 0xa001) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, slave_comm_r, slave_comm_w)
	AM_RANGE(0xb000, 0xb006) AM_WRITE(opwolf_adpcm_b_w)
	AM_RANGE(0xc000, 0xc006) AM_WRITE(opwolf_adpcm_c_w)
	AM_RANGE(0xd000, 0xd000) AM_WRITE(opwolf_adpcm_d_w)
	AM_RANGE(0xe000, 0xe000) AM_WRITE(opwolf_adpcm_e_w)
ADDRESS_MAP_END

/***********************************************************
             INPUT PORTS, DIPs
***********************************************************/


CUSTOM_INPUT_MEMBER(opwolf_state::opwolf_gun_x_r )
{
  /* P1X - Have to remap 8 bit input value, into 0-319 visible range */
	int scaled = (ioport(P1X_PORT_TAG)->read() * 320 ) / 256;
	return (scaled + 0x15 + m_opwolf_gun_xoffs);
}

CUSTOM_INPUT_MEMBER(opwolf_state::opwolf_gun_y_r )
{
	return (ioport(P1Y_PORT_TAG)->read() - 0x24 + m_opwolf_gun_yoffs);
}



static INPUT_PORTS_START( opwolf )
	/* 0x380000 -> 0x0ff028 (-$fd8,A5) (C-chip) */
	PORT_START("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) \
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	TAITO_COINAGE_WORLD_LOC(SW1)

	/* 0x380002 -> 0x0ff02a (-$fd6,A5) (C-chip) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, "Ammo Magazines at Start" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPNAME( 0x40, 0x00, "Discount When Continuing" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	
	PORT_START("IN2")
	PORT_BIT( 0x01ff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, opwolf_state, opwolf_gun_x_r, NULL)
	PORT_BIT( 0xfe00, IP_ACTIVE_LOW,  IPT_UNUSED )
	
	PORT_START("IN3")
	PORT_BIT( 0x01ff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, opwolf_state, opwolf_gun_y_r, NULL)
	PORT_BIT( 0xfe00, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START(P1X_PORT_TAG)  /* P1X (span allows you to shoot enemies behind status bar) */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START(P1Y_PORT_TAG)  /* P1Y (span allows you to be slightly offscreen) */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)



INPUT_PORTS_END



static INPUT_PORTS_START( opwolfp )
	PORT_INCLUDE( opwolf )


	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_MODIFY("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW,  IPT_UNUSED )



	PORT_MODIFY("IN2")
	/* 0x0000 - 0x01ff is GUNX */
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0xc000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN3")
	/* 0x0000 - 0x01ff is GUNY */
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0xf800, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x10, 0x10, "Display Hit Percentage (Cheat)" ) PORT_DIPLOCATION("SW2:5") // probably a cheat / debug feature as it's not in the final game
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Infinite Health (Cheat)" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Infinite Ammo (Cheat)" ) PORT_DIPLOCATION("SW2:7") // is 'Discount when Continuing' in final release
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, "English (invalid)" ) // game hangs on course screen (confirmed on hardware where it watchdog resets)
INPUT_PORTS_END


static INPUT_PORTS_START( opwolfu )
	PORT_INCLUDE( opwolf )

	PORT_MODIFY( "DSWA" )
	TAITO_COINAGE_JAPAN_OLD
INPUT_PORTS_END

static INPUT_PORTS_START( opwolfb )
	PORT_INCLUDE( opwolf )

	PORT_MODIFY( "DSWB" )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                        /* see notes */
INPUT_PORTS_END


/**************************************************************
                GFX DECODING
**************************************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4, 10*4, 11*4, 8*4, 9*4, 14*4, 15*4, 12*4, 13*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout charlayout_b =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static const gfx_layout tilelayout_b =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4, 8*4, 9*4, 10*4, 11*4, 12*4, 13*4, 14*4, 15*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( opwolf )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,  0, 128 )   /* sprites */
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  0, 128 )   /* scr tiles */
GFXDECODE_END

static GFXDECODE_START( opwolfb )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout_b,  0, 128 ) /* sprites */
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_b,  0, 128 ) /* scr tiles */
GFXDECODE_END


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

static MACHINE_CONFIG_START( opwolf, opwolf_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, CPU_CLOCK ) /* 8 MHz */
	MCFG_CPU_PROGRAM_MAP(opwolf_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", opwolf_state,  irq5_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, SOUND_CPU_CLOCK ) /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(opwolf_sound_z80_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))   /* 10 CPU slices per frame - enough for the sound CPU to read all commands */

	MCFG_MACHINE_RESET_OVERRIDE(opwolf_state,opwolf)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(opwolf_state, screen_update_opwolf)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", opwolf)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)

	MCFG_DEVICE_ADD("pc080sn", PC080SN, 0)
	MCFG_PC080SN_GFX_REGION(1)
	MCFG_PC080SN_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD("pc090oj", PC090OJ, 0)
	MCFG_PC090OJ_GFXDECODE("gfxdecode")
	MCFG_PC090OJ_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", SOUND_CPU_CLOCK )  /* 4 MHz */
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_YM2151_PORT_WRITE_HANDLER(WRITE8(opwolf_state,sound_bankswitch_w))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.75)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.75)

	MCFG_SOUND_ADD("msm1", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(opwolf_state, opwolf_msm5205_vck_1)) /* VCK function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8 kHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.60)

	MCFG_SOUND_ADD("msm2", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(opwolf_state, opwolf_msm5205_vck_2)) /* VCK function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8 kHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.60)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( opwolfp, opwolf )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu") /* 8 MHz */
	MCFG_CPU_PROGRAM_MAP(opwolfp_map)
MACHINE_CONFIG_END



static MACHINE_CONFIG_START( opwolfb, opwolf_state ) /* OSC clocks unknown for the bootleg, but changed to match original sets */

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, CPU_CLOCK ) /* 8 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(opwolfb_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", opwolf_state,  irq5_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, SOUND_CPU_CLOCK ) /* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(opwolf_sound_z80_map)

	MCFG_CPU_ADD("sub", Z80, SOUND_CPU_CLOCK )  /* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(opwolfb_sub_z80_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", opwolf_state,  irq0_line_hold)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))   /* 10 CPU slices per frame - enough for the sound CPU to read all commands */


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(opwolf_state, screen_update_opwolf)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", opwolfb)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)

	MCFG_DEVICE_ADD("pc080sn", PC080SN, 0)
	MCFG_PC080SN_GFX_REGION(1)
	MCFG_PC080SN_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD("pc090oj", PC090OJ, 0)
	MCFG_PC090OJ_GFXDECODE("gfxdecode")
	MCFG_PC090OJ_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", SOUND_CPU_CLOCK )
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_YM2151_PORT_WRITE_HANDLER(WRITE8(opwolf_state,sound_bankswitch_w))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.75)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.75)

	MCFG_SOUND_ADD("msm1", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(opwolf_state, opwolf_msm5205_vck_1)) /* VCK function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8 kHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.60)

	MCFG_SOUND_ADD("msm2", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(opwolf_state, opwolf_msm5205_vck_2)) /* VCK function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8 kHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.60)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END


/***************************************************************************
                    DRIVERS
***************************************************************************/

ROM_START( opwolf )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "b20-05-02.40",  0x00000, 0x10000, CRC(3ffbfe3a) SHA1(e41257e6af18bab4e36267a0c25a6aaa742972d2) )
	ROM_LOAD16_BYTE( "b20-03-02.30",  0x00001, 0x10000, CRC(fdabd8a5) SHA1(866ec6168489024b8d157f2d5b1553d7f6e3d9b7) )
	ROM_LOAD16_BYTE( "b20-04.39",     0x20000, 0x10000, CRC(216b4838) SHA1(2851cae00bb3e32e20f35fdab8ed6f149e658363) )
	ROM_LOAD16_BYTE( "b20-20.29",     0x20001, 0x10000, CRC(d244431a) SHA1(cb6c1d330a526f05c205f68247328161b8d4a1ba) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "b20-07.10",  0x00000, 0x10000, CRC(45c7ace3) SHA1(06f7393f6b973b7735c27e8380cb4148650cfc16) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b20-13.13",  0x00000, 0x80000, CRC(f6acdab1) SHA1(716b94ab3fa330ecf22df576f6a9f47a49c7554a) )    /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b20-14.72",  0x00000, 0x80000, CRC(89f889e5) SHA1(1592f6ce4fbb75e33d6ab957e5b90242a7a7a8c4) )    /* Sprites (16 x 16) */

	ROM_REGION( 0x80000, "adpcm", 0 )   /* ADPCM samples */
	ROM_LOAD( "b20-08.21",  0x00000, 0x80000, CRC(f3e19c64) SHA1(39d48645f776c9c2ade537d959ecc6f9dc6dfa1b) )
ROM_END

/* I don't know what this set changes.  There is a single byte changed near the end of the roms, just after the
   copyright strings, however, it is not the region byte.  This set came from a 100% legitimate Taito PCB with
   original labels.  It would be easy just to write this off as a bad read / hacked rom but the bootleg version
   has exactly the same change and the label is different (b20-17 instead of b20-20) so this seems unlikely */

ROM_START( opwolfa )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "b20-05-02.40",  0x00000, 0x10000, CRC(3ffbfe3a) SHA1(e41257e6af18bab4e36267a0c25a6aaa742972d2) )
	ROM_LOAD16_BYTE( "b20-03-02.30",  0x00001, 0x10000, CRC(fdabd8a5) SHA1(866ec6168489024b8d157f2d5b1553d7f6e3d9b7) )
	ROM_LOAD16_BYTE( "b20-04.39",     0x20000, 0x10000, CRC(216b4838) SHA1(2851cae00bb3e32e20f35fdab8ed6f149e658363) )
	ROM_LOAD16_BYTE( "b20-17.29",     0x20001, 0x10000, CRC(6043188e) SHA1(3a6f4836b1c19d37713f5714a947276baf1df50c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "b20-07.10",  0x00000, 0x10000, CRC(45c7ace3) SHA1(06f7393f6b973b7735c27e8380cb4148650cfc16) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b20-13.13",  0x00000, 0x80000, CRC(f6acdab1) SHA1(716b94ab3fa330ecf22df576f6a9f47a49c7554a) )    /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b20-14.72",  0x00000, 0x80000, CRC(89f889e5) SHA1(1592f6ce4fbb75e33d6ab957e5b90242a7a7a8c4) )    /* Sprites (16 x 16) */

	ROM_REGION( 0x80000, "adpcm", 0 )   /* ADPCM samples */
	ROM_LOAD( "b20-08.21",  0x00000, 0x80000, CRC(f3e19c64) SHA1(39d48645f776c9c2ade537d959ecc6f9dc6dfa1b) )
ROM_END

ROM_START( opwolfj )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "b20-05-02.40",  0x00000, 0x10000, CRC(3ffbfe3a) SHA1(e41257e6af18bab4e36267a0c25a6aaa742972d2) )
	ROM_LOAD16_BYTE( "b20-03-02.30",  0x00001, 0x10000, CRC(fdabd8a5) SHA1(866ec6168489024b8d157f2d5b1553d7f6e3d9b7) )
	ROM_LOAD16_BYTE( "b20-04.39",     0x20000, 0x10000, CRC(216b4838) SHA1(2851cae00bb3e32e20f35fdab8ed6f149e658363) )
	ROM_LOAD16_BYTE( "b20-18.29",     0x20001, 0x10000, CRC(fd202470) SHA1(3108c14953d2f50d861946e9f646813b7050b58a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "b20-07.10",  0x00000, 0x10000, CRC(45c7ace3) SHA1(06f7393f6b973b7735c27e8380cb4148650cfc16) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b20-13.13",  0x00000, 0x80000, CRC(f6acdab1) SHA1(716b94ab3fa330ecf22df576f6a9f47a49c7554a) )    /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b20-14.72",  0x00000, 0x80000, CRC(89f889e5) SHA1(1592f6ce4fbb75e33d6ab957e5b90242a7a7a8c4) )    /* Sprites (16 x 16) */

	ROM_REGION( 0x80000, "adpcm", 0 )   /* ADPCM samples */
	ROM_LOAD( "b20-08.21",  0x00000, 0x80000, CRC(f3e19c64) SHA1(39d48645f776c9c2ade537d959ecc6f9dc6dfa1b) )
ROM_END

ROM_START( opwolfu ) /* Taito TC0030 C-Chip labeled B20-18 (yes, it has a specific label on it) */
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "b20-05-02.40",  0x00000, 0x10000, CRC(3ffbfe3a) SHA1(e41257e6af18bab4e36267a0c25a6aaa742972d2) )
	ROM_LOAD16_BYTE( "b20-03-02.30",  0x00001, 0x10000, CRC(fdabd8a5) SHA1(866ec6168489024b8d157f2d5b1553d7f6e3d9b7) )
	ROM_LOAD16_BYTE( "b20-04.39",     0x20000, 0x10000, CRC(216b4838) SHA1(2851cae00bb3e32e20f35fdab8ed6f149e658363) )
	ROM_LOAD16_BYTE( "b20-19.29",     0x20001, 0x10000, CRC(b71bc44c) SHA1(5b404bd7630f01517ab98bda40ca43c11268035a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "b20-07.10",  0x00000, 0x10000, CRC(45c7ace3) SHA1(06f7393f6b973b7735c27e8380cb4148650cfc16) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b20-13.13",  0x00000, 0x80000, CRC(f6acdab1) SHA1(716b94ab3fa330ecf22df576f6a9f47a49c7554a) )    /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b20-14.72",  0x00000, 0x80000, CRC(89f889e5) SHA1(1592f6ce4fbb75e33d6ab957e5b90242a7a7a8c4) )    /* Sprites (16 x 16) */

	ROM_REGION( 0x80000, "adpcm", 0 )   /* ADPCM samples */
	ROM_LOAD( "b20-08.21",  0x00000, 0x80000, CRC(f3e19c64) SHA1(39d48645f776c9c2ade537d959ecc6f9dc6dfa1b) )
ROM_END

/*
Prototype board
There is no C-CHIP and TC0070RGB module is replaced by three PC040DA DAC, 68000 CPU is socketed as well as RAMs, the PGA customs and YM2151, YM3012 and PC060HA on sound board.
Labels on the 68k and Z80 roms are handwritten with checksums, GFX roms are the final MASK roms.
*/

ROM_START( opwolfp )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "ic40",     0x00000, 0x10000, CRC(81f56008) SHA1(8ff02c088ef325a1920b29651672bad2d2d2a7a2) )
	ROM_LOAD16_BYTE( "ic30",     0x00001, 0x10000, CRC(d90cebb2) SHA1(c36070c20dfad0e1c56c4ac016a115e9e9601ecb) )
	ROM_LOAD16_BYTE( "ic39",     0x20000, 0x10000, CRC(aeef5cfc) SHA1(3c75d1df80db6ae2d690fdf23b3c44b21056d24a) )
	ROM_LOAD16_BYTE( "ic29",     0x20001, 0x10000, CRC(5ce89249) SHA1(be22fd57e29114cde66cf4339c26740b3e0f830f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "ic10",  0x00000, 0x10000, CRC(684b40dd) SHA1(0546e01cf2c76b9c60730a14835cdeaaec21d26f) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b20-13.13",  0x00000, 0x80000, CRC(f6acdab1) SHA1(716b94ab3fa330ecf22df576f6a9f47a49c7554a) )    /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b20-06.ic72",  0x00000, 0x80000, CRC(89f889e5) SHA1(1592f6ce4fbb75e33d6ab957e5b90242a7a7a8c4) )    /* Sprites (16 x 16) */ // same content as b20-14.72 despite different label (confirmed)

	ROM_REGION( 0x80000, "adpcm", 0 )   /* ADPCM samples */
	ROM_LOAD( "b20-08.21",  0x00000, 0x80000, CRC(f3e19c64) SHA1(39d48645f776c9c2ade537d959ecc6f9dc6dfa1b) )
ROM_END


ROM_START( opwolfb )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "opwlfb.12",  0x00000, 0x10000, CRC(d87e4405) SHA1(de8a7763acd57293fbbff609e949ecd66c0f9234) )
	ROM_LOAD16_BYTE( "opwlfb.10",  0x00001, 0x10000, CRC(9ab6f75c) SHA1(85310258ca005ffb031e8d6b3f43c3d1fc29ef14) )
	ROM_LOAD16_BYTE( "opwlfb.13",  0x20000, 0x10000, CRC(61230c6e) SHA1(942764aec0c55ba00df8dbb54e127b73e24192ae) )
	ROM_LOAD16_BYTE( "opwlfb.11",  0x20001, 0x10000, CRC(342e318d) SHA1(a52918d16884ca42b2a3b910bc71bfd81b45f1ab) )

	ROM_REGION( 0x10000, "audiocpu", 0 )      /* sound cpu */
	ROM_LOAD( "opwlfb.30",  0x00000, 0x08000, CRC(0669b94c) SHA1(f10894a6fad8ed144a528db696436b58f62ddee4) )

	ROM_REGION( 0x10000, "sub", 0 )      /* c-chip substitute Z80 */
	ROM_LOAD( "opwlfb.09",   0x00000, 0x08000, CRC(ab27a3dd) SHA1(cf589e7a9ccf3e86020b86f917fb91f3d8ba7512) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "opwlfb.08",   0x00000, 0x10000, CRC(134d294e) SHA1(bd05169dbd761c2944f0ac51c1ec114577777452) )    /* SCR tiles (8 x 8) */
	ROM_LOAD16_BYTE( "opwlfb.06",   0x20000, 0x10000, CRC(317d0e66) SHA1(70298c0ef5243f481b18f904be9404527d1d99d5) )
	ROM_LOAD16_BYTE( "opwlfb.07",   0x40000, 0x10000, CRC(e1c4095e) SHA1(d5f1d26d6612e78001002f92de670e68e00c6f9e) )
	ROM_LOAD16_BYTE( "opwlfb.05",   0x60000, 0x10000, CRC(fd9e72c8) SHA1(7a76f57641c3f0198565cd163188b581253173b2) )
	ROM_LOAD16_BYTE( "opwlfb.04",   0x00001, 0x10000, CRC(de0ca98d) SHA1(066e89ec0c64da14bdcd2b337f95c0de5de33c11) )
	ROM_LOAD16_BYTE( "opwlfb.02",   0x20001, 0x10000, CRC(6231fdd0) SHA1(1c830c106cf3c94a8d06ed2fff030a5d516ab6d6) )
	ROM_LOAD16_BYTE( "opwlfb.03",   0x40001, 0x10000, CRC(ccf8ba80) SHA1(8366f5ef0de885e5241567d1a083d98a8a2875d9) )
	ROM_LOAD16_BYTE( "opwlfb.01",   0x60001, 0x10000, CRC(0a65f256) SHA1(4dfcd3cb138a87d002eb65a02f94e33f4d07676d) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "opwlfb.14",   0x00000, 0x10000, CRC(663786eb) SHA1(a25710f6c16158e51d0934f184390a01ff0a614a) )    /* Sprites (16 x 16) */
	ROM_LOAD16_BYTE( "opwlfb.15",   0x20000, 0x10000, CRC(315b8aa9) SHA1(4a904e5532421d933e4c401c03c958eb32b15e03) )
	ROM_LOAD16_BYTE( "opwlfb.16",   0x40000, 0x10000, CRC(e01099e3) SHA1(4c5391d71978f72c57c140e58a767e138acdce12) )
	ROM_LOAD16_BYTE( "opwlfb.17",   0x60000, 0x10000, CRC(56fbe61d) SHA1(0e4dce8ee981bdd851e500fa9dca5d40908e142f) )
	ROM_LOAD16_BYTE( "opwlfb.18",   0x00001, 0x10000, CRC(de9ab08e) SHA1(ef674c965f35efaf747f1ddbf9e9164fcceb0c1c) )
	ROM_LOAD16_BYTE( "opwlfb.19",   0x20001, 0x10000, CRC(645cf85e) SHA1(91c244c2e238b61c8b2f39e5fa01cc23ebbfe2ce) )
	ROM_LOAD16_BYTE( "opwlfb.20",   0x40001, 0x10000, CRC(d80b9cc6) SHA1(b189f35eb206da1ab313620e251e6bb10edeee04) )
	ROM_LOAD16_BYTE( "opwlfb.21",   0x60001, 0x10000, CRC(97d25157) SHA1(cfb3f76ed860d90235dc0e32919a5ec3d3e683dd) )

	ROM_REGION( 0x80000, "adpcm", 0 )   /* ADPCM samples (interleaved) */
	ROM_LOAD16_BYTE( "opwlfb.29",   0x00000, 0x10000, CRC(05a9eac0) SHA1(26eb1acc65aeb759920b35bcbcac6d6c2789584c) )
	ROM_LOAD16_BYTE( "opwlfb.28",   0x20000, 0x10000, CRC(281b2175) SHA1(3789e58da682041226f70eba87b31876cb206906) )
	ROM_LOAD16_BYTE( "opwlfb.27",   0x40000, 0x10000, CRC(441211a6) SHA1(82e84ae90765df5f7f6b6f32a2bb52ac40132f8d) )
	ROM_LOAD16_BYTE( "opwlfb.26",   0x60000, 0x10000, CRC(86d1d42d) SHA1(9d63e9e35fa51d8e6eac30556ba5a4dca7c14418) )
	ROM_LOAD16_BYTE( "opwlfb.25",   0x00001, 0x10000, CRC(85b87f58) SHA1(f26cf4ab8f9d30d1b1ac84be328ca821524b234e) )
	ROM_LOAD16_BYTE( "opwlfb.24",   0x20001, 0x10000, CRC(8efc5d4d) SHA1(21068d7fcfe293d99ad9f999d84483bf1a49ec6d) )
	ROM_LOAD16_BYTE( "opwlfb.23",   0x40001, 0x10000, CRC(a874c703) SHA1(c9d6074265f5d5028c69c81eaba29fa178943341) )
	ROM_LOAD16_BYTE( "opwlfb.22",   0x60001, 0x10000, CRC(9228481f) SHA1(8160f919f5e6a347c915a2bd7488b488fe2401bc) )
ROM_END


DRIVER_INIT_MEMBER(opwolf_state,opwolf)
{
	UINT16* rom = (UINT16*)memregion("maincpu")->base();

	m_opwolf_region = rom[0x03fffe / 2] & 0xff;

	opwolf_cchip_init();

	// World & US version have different gun offsets, presumably slightly different gun hardware
	m_opwolf_gun_xoffs = 0xec - (rom[0x03ffb0 / 2] & 0xff);
	m_opwolf_gun_yoffs = 0x1c - (rom[0x03ffae / 2] & 0xff);

	membank("z80bank")->configure_entries(0, 4, memregion("audiocpu")->base(), 0x4000);
}


DRIVER_INIT_MEMBER(opwolf_state,opwolfb)
{
	UINT16* rom = (UINT16*)memregion("maincpu")->base();

	m_opwolf_region = rom[0x03fffe / 2] & 0xff;

	/* bootleg needs different range of raw gun coords */
	m_opwolf_gun_xoffs = -2;
	m_opwolf_gun_yoffs = 17;

	membank("z80bank")->configure_entries(0, 4, memregion("audiocpu")->base(), 0x4000);
}

DRIVER_INIT_MEMBER(opwolf_state,opwolfp)
{
	UINT16* rom = (UINT16*)memregion("maincpu")->base();

	m_opwolf_region = rom[0x03fffe / 2] & 0xff;

	m_opwolf_gun_xoffs = 5;
	m_opwolf_gun_yoffs = 30;

	membank("z80bank")->configure_entries(0, 4, memregion("audiocpu")->base(), 0x4000);
}



/*    year  rom       parent    machine   inp       init */
GAME( 1987, opwolf,   0,        opwolf,   opwolf,  opwolf_state,  opwolf,   ROT0, "Taito Corporation Japan", "Operation Wolf (World, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1987, opwolfa,  opwolf,   opwolf,   opwolf,  opwolf_state,  opwolf,   ROT0, "Taito Corporation Japan", "Operation Wolf (World, set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1987, opwolfj,  opwolf,   opwolf,   opwolfu, opwolf_state,  opwolf,   ROT0, "Taito Corporation", "Operation Wolf (Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1987, opwolfu,  opwolf,   opwolf,   opwolfu, opwolf_state,  opwolf,   ROT0, "Taito America Corporation", "Operation Wolf (US)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1987, opwolfb,  opwolf,   opwolfb,  opwolfb, opwolf_state,  opwolfb,  ROT0, "bootleg (Bear Corporation Korea)", "Operation Bear (bootleg of Operation Wolf)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1987, opwolfp,  opwolf,   opwolfp,  opwolfp, opwolf_state,  opwolfp,  ROT0, "Taito Corporation", "Operation Wolf (Japan, prototype)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // unprotected
