// license:BSD-3-Clause
// copyright-holders:R. Belmont, Nicola Salmoria
/***************************************************************************

 Lethal Enforcers
 (c) 1992 Konami
 Driver by R. Belmont and Nicola Salmoria.

 This hardware is exceptionally weird - they have a bunch of chips intended
 for use with a 68000 hooked up to an 8-bit CPU.  So everything is bankswitched
 like crazy.

 LETHAL ENFORCERS
 KONAMI 1993

            84256         053245A
                191A03.A4     6116     191A04.A8 191A05.A10
                     054539             053244A   053244A
       Z80B  191A02.F4
                6116
                                          191A06.C9
                               7C185
                               7C185
                007644            5116
                054000            4464
                4464              4464
                191E01.U4         5116      054157  054157
                63C09EP           054156
       24MHZ                                 191A07.V8 191A08.V10
                                             191A09.V9 191A10.V10



---


Lethal Enforcers (c) Konami 1992
GX191 PWB353060A

Dump of USA program ROMs only.

Label   CRC32       Location    Code        Chip Type
  1   [72b843cc]      F4        Z80     TMS 27C512 (64k)
  6   [1b6b8f16]      U4        6309        ST 27C4001 (512k)

At offset 0x3FD03 in 6_usa.u4 is "08/17/92 21:38"

Run down of PCB:
Main CPU:  HD63C09EP
    OSC 24.00000MHz near 6309

Sound CPU:  Z80 (Zilog Z0840006PSC)
    OSC 18.43200MHz near Z80, 054968A & 054539

Konami Custom chips:

054986A (sound latch + Z80 memory mapper/banker + output DAC)
054539  (sound)
054000  (collision/protection)
053244A (x2) (sprites)
053245A (sprites)
054156 (tilemaps)
054157 (x2) (tilemaps)
007324 (???)

All other ROMs surface mounted (not included):

Label   Printed*    Position
191 A03 Mask16M-8bit - Near 054986A & 054539 - Sound - Also labeled as 056046

191A04  Mask8M-16bit \ Near 053244A (x2) & 05245A - Sprites
191A05  Mask8M-16bit /
191 A06 Mask8M-16bit - Also labeled as 056049

191A07  Mask8M-16bitx4 \
191A08  Mask8M-16bitx4  | Near 054157 (x2) & 054156 - Tiles
191A09  Mask8M-16bitx4  |
191A10  Mask8M-16bitx4 /

* This info is printed/silk-screened on to the PCB for assembly information?


4 way Dip Switch

---------------------------------------------------
 DipSwitch Title   |   Function   | 1 | 2 | 3 | 4 |
---------------------------------------------------
   Sound Output    |    Stereo    |off|           |
                   |   Monaural   |on |           |
---------------------------------------------------
  Coin Mechanism   |   Common     |   |off|       |
                   | Independent  |   |on |       |
---------------------------------------------------
    Game Type      |   Street     |       |off|   |
                   |   Arcade     |       |on |   |
---------------------------------------------------
    Language*      |   English    |           |off|
                   |   Spanish    |           |on |
---------------------------------------------------
     Default Settings             |off|off|on |off|
---------------------------------------------------
 NOTE: Listed as "NOT USED" in UK / World Manual  |
---------------------------------------------------

Push Button Test Switch


Memory map (from the schematics)

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
000xxxxxxxxxxxxx R   xxxxxxxx PROM      program ROM (banked)
001xxxxxxxxxxxxx R/W xxxxxxxx WRAM      work RAM
010000--00xxxxxx   W xxxxxxxx VREG      054156 control
010000--01--xxxx   W xxxxxxxx VSCG      054157 control
010000--1000---- R/W -------- AFR       watchdog reset
010000--1001----   W          SDON      sound enable?
010000--1010                  CCLR      ?
010000--1011----              n.c.
010000--11-000--   W ------xx COIN1/2   coin counters
010000--11-000--   W ----xx-- CKO1/2    coin enables?
010000--11-000--   W ---x---- VRD       \ related to reading graphics ROMs?
010000--11-000--   W --x----- CRDB      /
010000--11-001--   W -----xxx EEP       EEPROM DI, CS, CLK
010000--11-001--   W ----x--- MUT       sound mute?
010000--11-001--   W ---x---- CBNK      bank switch 4400-7FFF region between palette and 053245/054156
010000--11-001--   W --x----- n.c.
010000--11-001--   W xx------ SHD0/1    shadow control
010000--11-010--   W -----xxx PCU1/XBA  palette bank (tilemap A)
010000--11-010--   W -xxx---- PCU1/XBB  palette bank (tilemap B)
010000--11-011--   W -----xxx PCU2/XBC  palette bank (tilemap C)
010000--11-011--   W -xxx---- PCU2/XBD  palette bank (tilemap D)
010000--11-100--   W -----xxx PCU3/XBO  palette bank (sprites)
010000--11-100--   W -xxx---- PCU3/XBK  palette bank (background?)
010000--11-101xx R   xxxxxxxx POG       gun inputs
010000--11-11000 R   xxxxxxxx SW        dip switches, EEPROM DO, R/B
010000--11-11001 R   xxxxxxxx SW        inputs
010000--11-11010 R   xxxxxxxx SW        unused inputs (crossed out in schematics)
010000--11-11011 R   xx------ HHI1/2    gun input ready?
010000--11-11011 R   -------x NCPU      ?
010000--11-111--   W --xxxxxx BREG      ROM bank select
010010--00------              n.c.
010010--01---xxx R/W xxxxxxxx OREG      053244/053245 control
010010--10-xxxxx R/W xxxxxxxx HIP       054000
010010--11       R/W xxxxxxxx PAR       sound communication
010100xxxxxxxxxx R/W xxxxxxxx OBJ       053245 sprite RAM
011xxxxxxxxxxxxx R/W xxxxxxxx VRAM      054156 video RAM
1xxxxxxxxxxxxxxx R   xxxxxxxx PROM      program ROM


Lethal Enforcers
Konami 1992

PCB Layout
----------
GX191 PWB353060A
|--------------------------------------------------------|
|LA4705           191A03.A4  |------|191A04.A8 191A05.A10|
|CN6     84256     |------|  |053245A                    |
|         18.432MHz|054539|  |      |  |------| |------| |
| 054986A          |      |  |      |  |053244A |053244A |
|                  |      |  |------|  |      | |      | |
|                  |------|            |      | |      | |
|     Z80B        191A02.F4   LH5116   |------| |------| |
|                  LH5116                                |
|                                        191A06.G9       |
|                                                        |
|                                                        |
|J 051550                                                |
|A                            CY7C185                    |
|M 052535                     CY7C185                    |
|M 052535                                                |
|A 052535                                                |
|                                                        |
|                                                        |
|                                                        |
|                  054884     LH5116                     |
|                             MN4464                     |
|     005273                  MN4464   |------| |------| |
|     005273                  MN4464   |054157| |054157| |
| DSW(4)                      LH5116   |      | |      | |
|      ER5911.Q2   007644     |------| |      | |      | |
|TEST_SW           054000     |054156| |------| |------| |
|                  MN4464     |      |                   |
|                191UAD01.U4  |      |                   |
|                   007324    |------|                   |
|  CN8                              191A07.V8  191A08.V10|
|  CN7   24MHz   HD63C09EP          191A09.X8  191A10.X10|
|--------------------------------------------------------|
Notes:
      63C09EP - Clock 3.000MHz [24/8]
      Z80B    - Clock 6.000MHz [24/4]
      84256   - Fujitsu 84256 32kx8 SRAM (DIP28)
      LH5116  - Sharp LH5116 2kx8 SRAM (DIP24)
      CY7C185 - Cypress CY7C185 8kx8 SRAM (DIP28)
      MN4464  - Panasonic MN4464 8kx8 SRAM (DIP28)
      ER5911  - EEPROM (128 bytes)
      CN6     - 4 pin connector for stereo sound output
      CN7/CN8 - 4 pin connectors for standard light guns
                Pin numbering from left to right is 4 3 2 1
                Pin 1 - Opto
                Pin 2 - Ground
                Pin 3 - Trigger
                Pin 4 - +5v
      191*    - EPROM/mask ROM
      LA4705  - 15W 2-channel BTL audio power AMP

      Custom Chips
      ------------
      054000  - Collision/protection
      007324  - Resistor array package containing eight 150 ohm resistors. The IC looks like a DIP16 logic chip
                but with an epoxy top. The schematics show it connected to the 6309 data lines (D0-D7), main
                8k program RAM (D0-D7) and the 054000. It is a simple resistor array (x8)
      007644  - ? (DIP22)
      054157  \
      054156  / Tilemap generators
      053244A \
      053245A / Sprite generators
      054539  - 8-Channel ADPCM sound generator. Clock input 18.432MHz. Clock outputs 18.432/4 & 18.432/8
      052535  - Video DAC (one for each R,G,B video signal)
      051550  - EMI filter for credit/coin counter
      005273  - Resistor array for gun trigger and 1 player/2 player start
      054884  - MMI PAL16L8
      054986A - Audio DAC/filter + sound latch + Z80 memory mapper/banker (large ceramic SDIP64 module)
                This module contains several surface mounted capacitors and resistors, 4558 OP amp,
                Analog Devices AD1868 dual 18-bit audio DAC and a Konami 054321 QFP44 IC.

      Sync Measurements
      -----------------
      HSync - 15.2038kHz
      VSync - 59.6380Hz


note:

Lethal Enforcers has two sprite rendering chips working in parallel with their
output mixed to give 6bpp, and two tilemap rendering chips working in parallel
to give 8bpp. We currently cheat, using just one of each device but using
alternate gfx layouts. Emulating it accurately will require separating the
"front end" chips (053245, 054156) from the "back end" chips (053244, 054157)
as only the latter are doubled.

mirror not set up correctly

maybe some sprite placement issues

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m6809/hd6309.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/k054539.h"
#include "includes/lethal.h"

#define MAIN_CLOCK      XTAL_24MHz
#define SOUND_CLOCK     XTAL_18_432MHz


static const char *const gunnames[] = { "LIGHT0_X", "LIGHT0_Y", "LIGHT1_X", "LIGHT1_Y" };

/* a = 1, 2 = player # */
#define GUNX( a ) (( ( ioport(gunnames[2 * (a - 1)])->read() * 287 ) / 0xff ) + 16)
#define GUNY( a ) (( ( ioport(gunnames[2 * (a - 1) + 1])->read() * 223 ) / 0xff ) + 10)

WRITE8_MEMBER(lethal_state::control2_w)
{
	/* bit 0 is data */
	/* bit 1 is cs (active low) */
	/* bit 2 is clock (active high) */
	/* bit 3 is "MUT" on the schematics (audio mute?) */
	/* bit 4 bankswitches the 4400-7fff region: 0 = registers, 1 = palette RAM ("CBNK" on schematics) */
	/* bit 6 is "SHD0" (some kind of shadow control) */
	/* bit 7 is "SHD1" (ditto) */

	m_cur_control2 = data;

	m_bank4000->set_bank(BIT(m_cur_control2, 4));

	ioport("EEPROMOUT")->write(m_cur_control2, 0xff);
}

INTERRUPT_GEN_MEMBER(lethal_state::lethalen_interrupt)
{
	if (m_k056832->is_irq_enabled(0))
		device.execute().set_input_line(HD6309_IRQ_LINE, HOLD_LINE);
}

WRITE8_MEMBER(lethal_state::sound_cmd_w)
{
	soundlatch_byte_w(space, 0, data);
}

WRITE8_MEMBER(lethal_state::sound_irq_w)
{
	m_soundcpu->set_input_line(0, HOLD_LINE);
}

READ8_MEMBER(lethal_state::sound_status_r)
{
	return 0xf;
}

WRITE8_MEMBER(lethal_state::le_bankswitch_w)
{
	membank("bank1")->set_entry(data);
}

READ8_MEMBER(lethal_state::guns_r)
{
	switch (offset)
	{
		case 0:
			return GUNX(1) >> 1;
		case 1:
			if ((GUNY(1)<=0x0b) || (GUNY(1)>=0xe8))
				return 0;
			else
				return (232 - GUNY(1));
		case 2:
			return GUNX(2) >> 1;
		case 3:
			if ((GUNY(2)<=0x0b) || (GUNY(2)>=0xe8))
				return 0;
			else
				return (232 - GUNY(2));
	}

	return 0;
}

READ8_MEMBER(lethal_state::gunsaux_r)
{
	int res = 0;

	if (GUNX(1) & 1) res |= 0x80;
	if (GUNX(2) & 1) res |= 0x40;

	return res;
}

static ADDRESS_MAP_START( le_main, AS_PROGRAM, 8, lethal_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROMBANK("bank1")
	AM_RANGE(0x2000, 0x3fff) AM_RAM             // work RAM
	AM_RANGE(0x4000, 0x403f) AM_DEVWRITE("k056832", k056832_device, write)
	AM_RANGE(0x4040, 0x404f) AM_DEVWRITE("k056832", k056832_device, b_w)
	AM_RANGE(0x4080, 0x4080) AM_READNOP     // watchdog
	AM_RANGE(0x4090, 0x4090) AM_READNOP
	AM_RANGE(0x40a0, 0x40a0) AM_READNOP
	AM_RANGE(0x40c4, 0x40c4) AM_WRITE(control2_w)
	AM_RANGE(0x40c8, 0x40d0) AM_WRITE(lethalen_palette_control) // PCU1-PCU3 on the schematics
	AM_RANGE(0x40d4, 0x40d7) AM_READ(guns_r)
	AM_RANGE(0x40d8, 0x40d8) AM_READ_PORT("DSW")
	AM_RANGE(0x40d9, 0x40d9) AM_READ_PORT("INPUTS")
	AM_RANGE(0x40db, 0x40db) AM_READ(gunsaux_r)     // top X bit of guns
	AM_RANGE(0x40dc, 0x40dc) AM_WRITE(le_bankswitch_w)
	AM_RANGE(0x4000, 0x43ff) AM_UNMAP // first 0x400 bytes of palette RAM are inaccessible
	AM_RANGE(0x4000, 0x7fff) AM_DEVICE("bank4000", address_map_bank_device, amap8)
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION("maincpu", 0x38000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bank4000_map, AS_PROGRAM, 8, lethal_state )
	// VRD = 0 or 1, CBNK = 0
	AM_RANGE(0x0840, 0x084f) AM_MIRROR(0x8000) AM_DEVREADWRITE("k053244", k05324x_device, k053244_r, k053244_w)
	AM_RANGE(0x0880, 0x089f) AM_MIRROR(0x8000) AM_DEVREADWRITE("k054000", k054000_device, read, write)
	AM_RANGE(0x08c6, 0x08c6) AM_MIRROR(0x8000) AM_WRITE(sound_cmd_w)
	AM_RANGE(0x08c7, 0x08c7) AM_MIRROR(0x8000) AM_WRITE(sound_irq_w)
	AM_RANGE(0x08ca, 0x08ca) AM_MIRROR(0x8000) AM_READ(sound_status_r)
	AM_RANGE(0x1000, 0x17ff) AM_MIRROR(0x8000) AM_DEVREADWRITE("k053244", k05324x_device, k053245_r, k053245_w)

	// VRD = 0, CBNK = 0
	AM_RANGE(0x2000, 0x27ff) AM_DEVREADWRITE("k056832", k056832_device, ram_code_lo_r, ram_code_lo_w)
	AM_RANGE(0x2800, 0x2fff) AM_DEVREADWRITE("k056832", k056832_device, ram_code_hi_r, ram_code_hi_w)
	AM_RANGE(0x3000, 0x37ff) AM_DEVREADWRITE("k056832", k056832_device, ram_attr_lo_r, ram_attr_lo_w)
	AM_RANGE(0x3800, 0x3fff) AM_DEVREADWRITE("k056832", k056832_device, ram_attr_hi_r, ram_attr_hi_w)

	// VRD = 1, CBNK = 0 or 1
	AM_RANGE(0xa000, 0xbfff) AM_MIRROR(0x4000) AM_UNMAP // AM_DEVREAD("k056832", k056832_device, rom_byte_r)

	// CBNK = 1; partially overlaid when VRD = 1
	AM_RANGE(0x4000, 0x7fff) AM_MIRROR(0x8000) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
ADDRESS_MAP_END

static ADDRESS_MAP_START( le_sound, AS_PROGRAM, 8, lethal_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xfa2f) AM_DEVREADWRITE("k054539", k054539_device, read, write)
	AM_RANGE(0xfc00, 0xfc00) AM_WRITE(soundlatch2_byte_w)
	AM_RANGE(0xfc02, 0xfc02) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xfc03, 0xfc03) AM_READNOP
ADDRESS_MAP_END

static INPUT_PORTS_START( lethalen )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, do_read)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, ready_read)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR(Language) )       PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x10, DEF_STR(English) )
	PORT_DIPSETTING(    0x00, DEF_STR(Spanish) )
	PORT_DIPNAME( 0x20, 0x00, "Game Type" )         PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x20, "Street" )
	PORT_DIPSETTING(    0x00, "Arcade" )
	PORT_DIPNAME( 0x40, 0x40, "Coin Mechanism" )        PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x40, "Common" )
	PORT_DIPSETTING(    0x00, "Independent" )
	PORT_DIPNAME( 0x80, 0x80, "Sound Output" )      PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Mono ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Stereo ) )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, di_write)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, cs_write)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, clk_write)

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( lethalenj )
	PORT_INCLUDE( lethalen )

		PORT_MODIFY("DSW")  /* Normal DIPs appear to do nothing for Japan region - wrong location?  Set to unknown */
		PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSW:4")
		PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW:3")
		PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW:2")
		PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW:1")

		PORT_MODIFY("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_REVERSE

	PORT_MODIFY("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_REVERSE

	PORT_MODIFY("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_REVERSE

	PORT_MODIFY("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_REVERSE
INPUT_PORTS_END

static INPUT_PORTS_START( lethalene ) /* European region does not have non-english Language option */
	PORT_INCLUDE( lethalen )

	PORT_MODIFY("DSW")
		PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "DSW:4")
INPUT_PORTS_END


void lethal_state::machine_start()
{
	membank("bank1")->configure_entries(0, 0x20, memregion("maincpu")->base(), 0x2000);
	membank("bank1")->set_entry(0);

	save_item(NAME(m_cur_control2));
	save_item(NAME(m_layer_colorbase));
	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_back_colorbase));
}

void lethal_state::machine_reset()
{
	for (int i = 0; i < 4; i++)
		m_layer_colorbase[i] = 0;

	m_sprite_colorbase = 0;
	m_back_colorbase = 0;
	m_cur_control2 = 0;
	m_bank4000->set_bank(0);
}

static MACHINE_CONFIG_START( lethalen, lethal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD6309, MAIN_CLOCK/2)    /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(le_main)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", lethal_state,  lethalen_interrupt)

	MCFG_CPU_ADD("soundcpu", Z80, MAIN_CLOCK/4)  /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(le_sound)

	MCFG_DEVICE_ADD("bank4000", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(bank4000_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x4000)

	MCFG_EEPROM_SERIAL_ER5911_8BIT_ADD("eeprom")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.62)  /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(216, 504-1, 16, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(lethal_state, screen_update_lethalen)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ORIENTATION_FLIP_Y)

	MCFG_PALETTE_ADD("palette", 8192)
	MCFG_PALETTE_ENABLE_SHADOWS()
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("k056832", K056832, 0)
	MCFG_K056832_CB(lethal_state, tile_callback)
	MCFG_K056832_CONFIG("gfx1", 0, K056832_BPP_8LE, 1, 0, "none")
	MCFG_K056832_GFXDECODE("gfxdecode")
	MCFG_K056832_PALETTE("palette")

	MCFG_DEVICE_ADD("k053244", K053244, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K05324X_BPP(6)
	MCFG_K05324X_OFFSETS(95, 0)
	MCFG_K05324X_CB(lethal_state, sprite_callback)

	MCFG_K054000_ADD("k054000")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_DEVICE_ADD("k054539", K054539, XTAL_18_432MHz)
	MCFG_K054539_TIMER_HANDLER(INPUTLINE("soundcpu", INPUT_LINE_NMI))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( lethalej, lethalen )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(224, 512-1, 16, 240-1)
	MCFG_SCREEN_ORIENTATION(ORIENTATION_FLIP_X)

	MCFG_DEVICE_MODIFY("k053244")
	MCFG_K05324X_OFFSETS(-105, 0)
MACHINE_CONFIG_END

ROM_START( lethalen )   // US version UAE
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "191uae01.u4", 0x00000, 0x40000, CRC(dca340e3) SHA1(8efbba0e3a459bcfe23c75c584bf3a4ce25148bb) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x00000, 0x10000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "k053244", ROMREGION_ERASE00 )   /* sprites */
	ROM_LOAD32_WORD( "191a05", 0x000000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD32_WORD( "191a04", 0x000002, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD32_WORD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethalenue.nv", 0x0000, 0x0080, CRC(6e7224e6) SHA1(86dea9262d55e58b573d397d0fea437c58728707) )
ROM_END

ROM_START( lethalenub ) // US version UAB
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "191uab01.u4", 0x00000, 0x40000, CRC(2afd7528) SHA1(65ce4a54fe96ad38d39d335b5d3a644a495c7e31) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x00000, 0x10000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "k053244", ROMREGION_ERASE00 )   /* sprites */
	ROM_LOAD32_WORD( "191a05", 0x000000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD32_WORD( "191a04", 0x000002, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD32_WORD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethalenub.nv", 0x0000, 0x0080, CRC(14c6c6e5) SHA1(8a498b5322266df25fb24d1b7bd7937de459d207) )
ROM_END

ROM_START( lethalenua ) // US version UAA
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "191uaa01.u4", 0x00000, 0x40000, CRC(ab6b8f16) SHA1(8de6c429a6e71144270e79d18ad47b5aad13fe04) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x00000, 0x10000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "k053244", ROMREGION_ERASE00 )   /* sprites */
	ROM_LOAD32_WORD( "191a05", 0x000000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD32_WORD( "191a04", 0x000002, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD32_WORD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethalenua.nv", 0x0000, 0x0080, CRC(f71ad1c3) SHA1(04c7052d0895797af8a06183b8a877795bf2dbb3) )
ROM_END

ROM_START( lethalenux ) // US version ?, proto / hack?, very different to other sets
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "191xxx01.u4", 0x00000, 0x40000, CRC(a3b9e790) SHA1(868b422850be129952c8b11c3c4aa730d8ea1544) ) // hacked? fails rom test, verified on multiple boards

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x00000, 0x10000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "k053244", ROMREGION_ERASE00 )   /* sprites */
	ROM_LOAD32_WORD( "191a05", 0x000000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD32_WORD( "191a04", 0x000002, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD32_WORD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethalenux.nv", 0x0000, 0x0080, CRC(5d69c39d) SHA1(e468df829ee5094792289f9166d7e39b638ab70d) )
ROM_END

ROM_START( lethaleneab )    // Euro ver. EAB
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "191eab01.u4", 0x00000, 0x40000, CRC(d7ce111e) SHA1(e56137a0ba7664f09b5d05bb39ec6eb4d1e412c7) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x00000, 0x10000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "k053244", ROMREGION_ERASE00 )   /* sprites */
	ROM_LOAD32_WORD( "191a05", 0x000000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD32_WORD( "191a04", 0x000002, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD32_WORD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethaleneab.nv", 0x0000, 0x0080, CRC(4e9bb34d) SHA1(9502583bc9f5f6fc5bba333869398b24bf154b73) )
ROM_END

ROM_START( lethaleneae )    // Euro ver. EAE
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "191eae01.u4",    0x00000,  0x40000, CRC(c6a3c6ac) SHA1(96a209a3a5b4af40af36bd7090c59a74f8c8df59) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x00000, 0x10000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "k053244", ROMREGION_ERASE00 )   /* sprites */
	ROM_LOAD32_WORD( "191a05", 0x000000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD32_WORD( "191a04", 0x000002, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD32_WORD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethaleneae.nv", 0x0000, 0x0080, CRC(eb369a67) SHA1(6c67294669614e96de5efb38372dbed435ee04d3) )
ROM_END

ROM_START( lethalenj )  // Japan version JAD
	ROM_REGION( 0x40000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "191jad01.u4",    0x00000,  0x40000, CRC(160a25c0) SHA1(1d3ed5a158e461a73c079fe24a8e9d5e2a87e126) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x00000, 0x10000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "k053244", ROMREGION_ERASE00 )   /* sprites */
	ROM_LOAD32_WORD( "191a05", 0x000000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD32_WORD( "191a04", 0x000002, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD32_WORD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethalenj.nv", 0x0000, 0x0080, CRC(20b28f2f) SHA1(53d212f2c006729a01dfdb49cb36b67b9425172e) )
ROM_END


GAME( 1992, lethalen,   0,        lethalen, lethalen,  driver_device, 0, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver UAE, 11/19/92 15:04)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // writes UE to eeprom
GAME( 1992, lethalenub, lethalen, lethalen, lethalen,  driver_device, 0, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver UAB, 09/01/92 11:12)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // writes UB to eeprom
GAME( 1992, lethalenua, lethalen, lethalen, lethalen,  driver_device, 0, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver UAA, 08/17/92 21:38)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // writes UA to eeprom
GAME( 1992, lethalenux, lethalen, lethalen, lethalen,  driver_device, 0, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver unknown, US, 08/06/92 15:11, hacked/proto?)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // writes UA to eeprom but earlier than suspected UAA set, might be a proto, might be hacked, fails rom test, definitely a good dump, another identical set was found in Italy
GAME( 1992, lethaleneae,lethalen, lethalen, lethalene, driver_device, 0, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver EAE, 11/19/92 16:24)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // writes EE to eeprom
GAME( 1992, lethaleneab,lethalen, lethalen, lethalene, driver_device, 0, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver EAB, 10/14/92 19:53)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // writes EC to eeprom?!

// different mirror / display setup
GAME( 1992, lethalenj,  lethalen, lethalej, lethalenj, driver_device, 0, ORIENTATION_FLIP_X, "Konami", "Lethal Enforcers (ver JAD, 12/04/92 17:16)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // writes JC to eeprom?!
