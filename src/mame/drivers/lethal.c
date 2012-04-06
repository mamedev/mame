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
010000--00xxxxxx   W xxxxxxxx VREG      056832 control
010000--01--xxxx   W xxxxxxxx VSCG      056832 control
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
010000--11-001--   W ---x---- CBNK      bank switch 4800-7FFF region between palette and 053245/056832
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
010010--01---xxx R/W xxxxxxxx OREG      053244
010010--10-xxxxx R/W xxxxxxxx HIP       054000
010010--11       R/W xxxxxxxx PAR       sound communication
010100xxxxxxxxxx R/W xxxxxxxx OBJ       053245
011xxxxxxxxxxxxx R/W xxxxxxxx VRAM      056832
1xxxxxxxxxxxxxxx R   xxxxxxxx PROM      program ROM


note:

lethal enforcers has 2 sprite rendering chips working in parallel mixing
data together to give 6bpp.. we cheat by using a custom function in
konamiic.c and a fixed 6bpp decode.

mirror not set up correctly

maybe some sprite placement issues

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "cpu/m6809/m6809.h"
#include "cpu/hd6309/hd6309.h"
#include "cpu/z80/z80.h"
#include "machine/eeprom.h"
#include "sound/k054539.h"
#include "includes/lethal.h"

#define MAIN_CLOCK		XTAL_24MHz
#define SOUND_CLOCK		XTAL_18_432MHz


static const char *const gunnames[] = { "LIGHT0_X", "LIGHT0_Y", "LIGHT1_X", "LIGHT1_Y" };

/* a = 1, 2 = player # */
#define GUNX( a ) (( ( input_port_read(machine(), gunnames[2 * (a - 1)]) * 287 ) / 0xff ) + 16)
#define GUNY( a ) (( ( input_port_read(machine(), gunnames[2 * (a - 1) + 1]) * 223 ) / 0xff ) + 10)

static const eeprom_interface eeprom_intf =
{
	7,			/* address bits */
	8,			/* data bits */
	"011000",		/* read command */
	"011100",		/* write command */
	"0100100000000",	/* erase command */
	"0100000000000",	/* lock command */
	"0100110000000" 	/* unlock command */
};

WRITE8_MEMBER(lethal_state::control2_w)
{
	/* bit 0 is data */
	/* bit 1 is cs (active low) */
	/* bit 2 is clock (active high) */
	/* bit 3 is "MUT" on the schematics (audio mute?) */
	/* bit 4 bankswitches the 4800-4fff region: 0 = registers, 1 = RAM ("CBNK" on schematics) */
	/* bit 6 is "SHD0" (some kind of shadow control) */
	/* bit 7 is "SHD1" (ditto) */

	m_cur_control2 = data;

	input_port_write(machine(), "EEPROMOUT", m_cur_control2, 0xff);
}

static INTERRUPT_GEN(lethalen_interrupt)
{
	lethal_state *state = device->machine().driver_data<lethal_state>();

	if (k056832_is_irq_enabled(state->m_k056832, 0))
		device_set_input_line(device, HD6309_IRQ_LINE, HOLD_LINE);
}

WRITE8_MEMBER(lethal_state::sound_cmd_w)
{
	soundlatch_w(space, 0, data);
}

WRITE8_MEMBER(lethal_state::sound_irq_w)
{
	device_set_input_line(m_audiocpu, 0, HOLD_LINE);
}

READ8_MEMBER(lethal_state::sound_status_r)
{
	return 0xf;
}

static void sound_nmi( device_t *device )
{
	lethal_state *state = device->machine().driver_data<lethal_state>();
	device_set_input_line(state->m_audiocpu, INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER(lethal_state::le_bankswitch_w)
{
	memory_set_bank(machine(), "bank1", data);
}

READ8_MEMBER(lethal_state::le_4800_r)
{

	if (m_cur_control2 & 0x10)	// RAM enable
	{
		return m_generic_paletteram_8[offset];
	}
	else
	{
		if (offset < 0x0800)
		{
			switch (offset)
			{
				case 0x40:
				case 0x41:
				case 0x42:
				case 0x43:
				case 0x44:
				case 0x45:
				case 0x46:
				case 0x47:
				case 0x48:
				case 0x49:
				case 0x4a:
				case 0x4b:
				case 0x4c:
				case 0x4d:
				case 0x4e:
				case 0x4f:
					return k053244_r(m_k053244, offset - 0x40);

				case 0x80:
				case 0x81:
				case 0x82:
				case 0x83:
				case 0x84:
				case 0x85:
				case 0x86:
				case 0x87:
				case 0x88:
				case 0x89:
				case 0x8a:
				case 0x8b:
				case 0x8c:
				case 0x8d:
				case 0x8e:
				case 0x8f:
				case 0x90:
				case 0x91:
				case 0x92:
				case 0x93:
				case 0x94:
				case 0x95:
				case 0x96:
				case 0x97:
				case 0x98:
				case 0x99:
				case 0x9a:
				case 0x9b:
				case 0x9c:
				case 0x9d:
				case 0x9e:
				case 0x9f:
					return k054000_r(m_k054000, offset - 0x80);

				case 0xca:
					return sound_status_r(space, 0);
			}
		}
		else if (offset < 0x1800)
			return k053245_r(m_k053244, (offset - 0x0800) & 0x07ff);
		else if (offset < 0x2000)
			return k056832_ram_code_lo_r(m_k056832, offset - 0x1800);
		else if (offset < 0x2800)
			return k056832_ram_code_hi_r(m_k056832, offset - 0x2000);
		else if (offset < 0x3000)
			return k056832_ram_attr_lo_r(m_k056832, offset - 0x2800);
		else // (offset < 0x3800)
			return k056832_ram_attr_hi_r(m_k056832, offset - 0x3000);
	}

	return 0;
}

WRITE8_MEMBER(lethal_state::le_4800_w)
{

	if (m_cur_control2 & 0x10)	// RAM enable
	{
		paletteram_xBBBBBGGGGGRRRRR_be_w(space, offset, data);
	}
	else
	{
		if (offset < 0x0800)
		{
			switch (offset)
			{
				case 0xc6:
					sound_cmd_w(space, 0, data);
					break;

				case 0xc7:
					sound_irq_w(space, 0, data);
					break;

				case 0x40:
				case 0x41:
				case 0x42:
				case 0x43:
				case 0x44:
				case 0x45:
				case 0x46:
				case 0x47:
				case 0x48:
				case 0x49:
				case 0x4a:
				case 0x4b:
				case 0x4c:
				case 0x4d:
				case 0x4e:
				case 0x4f:
					k053244_w(m_k053244, offset - 0x40, data);
					break;

				case 0x80:
				case 0x81:
				case 0x82:
				case 0x83:
				case 0x84:
				case 0x85:
				case 0x86:
				case 0x87:
				case 0x88:
				case 0x89:
				case 0x8a:
				case 0x8b:
				case 0x8c:
				case 0x8d:
				case 0x8e:
				case 0x8f:
				case 0x90:
				case 0x91:
				case 0x92:
				case 0x93:
				case 0x94:
				case 0x95:
				case 0x96:
				case 0x97:
				case 0x98:
				case 0x99:
				case 0x9a:
				case 0x9b:
				case 0x9c:
				case 0x9d:
				case 0x9e:
				case 0x9f:
					k054000_w(m_k054000, offset - 0x80, data);
					break;

				default:
					logerror("Unknown LE 48xx register write: %x to %x (PC=%x)\n", data, offset, cpu_get_pc(&space.device()));
					break;
			}
		}
		else if (offset < 0x1800)
			k053245_w(m_k053244, (offset - 0x0800) & 0x07ff, data);
		else if (offset < 0x2000)
			k056832_ram_code_lo_w(m_k056832, offset - 0x1800, data);
		else if (offset < 0x2800)
			k056832_ram_code_hi_w(m_k056832, offset - 0x2000, data);
		else if (offset < 0x3000)
			k056832_ram_attr_lo_w(m_k056832, offset - 0x2800, data);
		else // (offset < 0x3800)
			k056832_ram_attr_hi_w(m_k056832, offset - 0x3000, data);
	}
}

// use one more palette entry for the BG color
WRITE8_MEMBER(lethal_state::le_bgcolor_w)
{
	paletteram_xBBBBBGGGGGRRRRR_be_w(space, 0x3800 + offset, data);
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
			break;
		case 2:
			return GUNX(2) >> 1;
		case 3:
			if ((GUNY(2)<=0x0b) || (GUNY(2)>=0xe8))
				return 0;
			else
				return (232 - GUNY(2));
			break;
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
	AM_RANGE(0x2000, 0x3fff) AM_RAM				// work RAM
	AM_RANGE(0x4000, 0x403f) AM_DEVWRITE_LEGACY("k056832", k056832_w)
	AM_RANGE(0x4040, 0x404f) AM_DEVWRITE_LEGACY("k056832", k056832_b_w)
	AM_RANGE(0x4080, 0x4080) AM_READNOP		// watchdog
	AM_RANGE(0x4090, 0x4090) AM_READNOP
	AM_RANGE(0x40a0, 0x40a0) AM_READNOP
	AM_RANGE(0x40c4, 0x40c4) AM_WRITE(control2_w)
	AM_RANGE(0x40c8, 0x40d0) AM_WRITE(lethalen_palette_control)	// PCU1-PCU3 on the schematics
	AM_RANGE(0x40d4, 0x40d7) AM_READ(guns_r)
	AM_RANGE(0x40d8, 0x40d8) AM_READ_PORT("DSW")
	AM_RANGE(0x40d9, 0x40d9) AM_READ_PORT("INPUTS")
	AM_RANGE(0x40db, 0x40db) AM_READ(gunsaux_r)		// top X bit of guns
	AM_RANGE(0x40dc, 0x40dc) AM_WRITE(le_bankswitch_w)
	AM_RANGE(0x47fe, 0x47ff) AM_WRITE(le_bgcolor_w)		// BG color
	AM_RANGE(0x4800, 0x7fff) AM_READWRITE(le_4800_r, le_4800_w) // bankswitched: RAM and registers
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( le_sound, AS_PROGRAM, 8, lethal_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xfa2f) AM_DEVREADWRITE("k054539", k054539_device, read, write)
	AM_RANGE(0xfc00, 0xfc00) AM_WRITE(soundlatch2_w)
	AM_RANGE(0xfc02, 0xfc02) AM_READ(soundlatch_r)
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_device, read_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )	/* it must be 1 ? */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR(Language) )
	PORT_DIPSETTING(    0x10, DEF_STR(English) )
	PORT_DIPSETTING(    0x00, DEF_STR(Spanish) )
	PORT_DIPNAME( 0x20, 0x00, "Game Type" )
	PORT_DIPSETTING(    0x20, "Street" )
	PORT_DIPSETTING(    0x00, "Arcade" )
	PORT_DIPNAME( 0x40, 0x40, "Coin Mechanism" )
	PORT_DIPSETTING(    0x40, "Common" )
	PORT_DIPSETTING(    0x00, "Independent" )
	PORT_DIPNAME( 0x0080, 0x0080, "Sound Output" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Mono ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Stereo ) )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, write_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_cs_line)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_clock_line)

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( lethalej )
	PORT_INCLUDE( lethalen )

	PORT_MODIFY("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_REVERSE

	PORT_MODIFY("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_REVERSE

	PORT_MODIFY("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_REVERSE

	PORT_MODIFY("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_REVERSE
INPUT_PORTS_END


static const gfx_layout lethal_6bpp =
{
	16,16,
	RGN_FRAC(1,2),
	6,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0, 8, 0, 24, 16 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
	8*32+0, 8*32+1, 8*32+2, 8*32+3, 8*32+4, 8*32+5, 8*32+6, 8*32+7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	128*8
};

/* we use this decode instead of the one done by the sprite video start due to it being 6bpp */
static GFXDECODE_START( lethal )
	GFXDECODE_ENTRY( "gfx2", 0, lethal_6bpp,   0x000/*0x400*/, 256  ) /* sprites tiles */
GFXDECODE_END


/* sound */

static const k054539_interface k054539_config =
{
	NULL,
	NULL,
	sound_nmi
};

static MACHINE_START( lethalen )
{
	lethal_state *state = machine.driver_data<lethal_state>();
	UINT8 *ROM = machine.region("maincpu")->base();

	memory_configure_bank(machine, "bank1", 0, 0x20, &ROM[0x10000], 0x2000);
	memory_set_bank(machine, "bank1", 0);

	state->m_generic_paletteram_8.allocate(0x3800 + 0x02);

	state->m_maincpu = machine.device("maincpu");
	state->m_audiocpu = machine.device("soundcpu");
	state->m_k054539 = machine.device("k054539");
	state->m_k053244 = machine.device("k053244");
	state->m_k056832 = machine.device("k056832");
	state->m_k054000 = machine.device("k054000");

	state->save_item(NAME(state->m_cur_control2));
	state->save_item(NAME(state->m_sprite_colorbase));
	state->save_item(NAME(state->m_layer_colorbase));
}

static MACHINE_RESET( lethalen )
{
	lethal_state *state = machine.driver_data<lethal_state>();
	UINT8 *prgrom = (UINT8 *)machine.region("maincpu")->base();
	int i;

	memory_set_bankptr(machine, "bank2", &prgrom[0x48000]);
	/* force reset again to read proper reset vector */
	machine.device("maincpu")->reset();

	for (i = 0; i < 4; i++)
		state->m_layer_colorbase[i] = 0;

	state->m_sprite_colorbase = 0;
	state->m_cur_control2 = 0;
}

static const k056832_interface lethalen_k056832_intf =
{
	"gfx1", 1,
	K056832_BPP_8LE,
	1, 0,
	KONAMI_ROM_DEINTERLEAVE_NONE,
	lethalen_tile_callback, "none"
};

static const k05324x_interface lethalen_k05324x_intf =
{
	"gfx3", 2,
	NORMAL_PLANE_ORDER,
	95, 0,
	KONAMI_ROM_DEINTERLEAVE_2,
	lethalen_sprite_callback
};

static const k05324x_interface lethalej_k05324x_intf =
{
	"gfx3", 2,
	NORMAL_PLANE_ORDER,
	-105, 0,
	KONAMI_ROM_DEINTERLEAVE_2,
	lethalen_sprite_callback
};

static MACHINE_CONFIG_START( lethalen, lethal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD6309, MAIN_CLOCK/2)    /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(le_main)
	MCFG_CPU_VBLANK_INT("screen", lethalen_interrupt)

	MCFG_CPU_ADD("soundcpu", Z80, MAIN_CLOCK/4)  /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(le_sound)

	MCFG_MACHINE_START(lethalen)
	MCFG_MACHINE_RESET(lethalen)

	MCFG_EEPROM_ADD("eeprom", eeprom_intf)

	MCFG_GFXDECODE(lethal)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.62)  /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(216, 504-1, 16, 240-1)
	MCFG_SCREEN_UPDATE_STATIC(lethalen)

	MCFG_PALETTE_LENGTH(7168+1)

	MCFG_VIDEO_START(lethalen)

	MCFG_K056832_ADD("k056832", lethalen_k056832_intf)
	MCFG_K053244_ADD("k053244", lethalen_k05324x_intf)
	MCFG_K054000_ADD("k054000")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_K054539_ADD("k054539", 48000, k054539_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( lethalej, lethalen )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(224, 512-1, 16, 240-1)

	MCFG_DEVICE_REMOVE("k053244")
	MCFG_K053244_ADD("k053244", lethalej_k05324x_intf)
MACHINE_CONFIG_END

ROM_START( lethalen )	// US version UAE
	ROM_REGION( 0x50000, "maincpu", 0 )
	/* main program */
	ROM_LOAD( "191uae01.u4",    0x10000,  0x40000,  CRC(dca340e3) SHA1(8efbba0e3a459bcfe23c75c584bf3a4ce25148bb) )

	ROM_REGION( 0x020000, "soundcpu", 0 )
	/* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x000000, 0x010000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )
	ROM_RELOAD(         0x010000, 0x010000 )

	ROM_REGION( 0x400000, "gfx1", 0 )
	/* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "gfx2", ROMREGION_ERASE00 )
	/* sprites - fake 6bpp decode is done from here */
	ROM_LOAD( "191a04", 0x000000, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD( "191a05", 0x100000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "gfx3", ROMREGION_ERASE00 )
	ROM_COPY("gfx2",0,0, 0x200000)

	ROM_REGION( 0x200000, "gfx4", ROMREGION_ERASE00 )
	ROM_COPY("gfx2",0x200000,0, 0x200000)

	ROM_REGION( 0x200000, "k054539", 0 )
	/* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethalen.nv", 0x0000, 0x0080, CRC(6e7224e6) SHA1(86dea9262d55e58b573d397d0fea437c58728707) )
ROM_END

ROM_START( lethalenj )	// Japan version JAD
	ROM_REGION( 0x50000, "maincpu", 0 )
	/* main program */
	ROM_LOAD( "191jad01.u4",    0x10000,  0x40000, CRC(160a25c0) SHA1(1d3ed5a158e461a73c079fe24a8e9d5e2a87e126) )

	ROM_REGION( 0x020000, "soundcpu", 0 )
	/* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x000000, 0x010000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )
	ROM_RELOAD(         0x010000, 0x010000 )

	ROM_REGION( 0x400000, "gfx1", 0 )
	/* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "gfx2", ROMREGION_ERASE00 )
	/* sprites - fake 6bpp decode is done from here */
	ROM_LOAD( "191a04", 0x000000, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD( "191a05", 0x100000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "gfx3", ROMREGION_ERASE00 )
	ROM_COPY("gfx2",0,0, 0x200000)

	ROM_REGION( 0x200000, "gfx4", ROMREGION_ERASE00 )
	ROM_COPY("gfx2",0x200000,0, 0x200000)

	ROM_REGION( 0x200000, "k054539", 0 )
	/* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethalenj.nv", 0x0000, 0x0080, CRC(20b28f2f) SHA1(53d212f2c006729a01dfdb49cb36b67b9425172e) )
ROM_END

ROM_START( lethalenux )	// US version ?, proto / hack?, very different to other sets
	ROM_REGION( 0x50000, "maincpu", 0 )
	/* main program */
	ROM_LOAD( "191xxx01.u4",    0x10000,  0x40000, CRC(a3b9e790) SHA1(868b422850be129952c8b11c3c4aa730d8ea1544) ) // hacked? fails rom test, verified on multiple boards

	ROM_REGION( 0x020000, "soundcpu", 0 )
	/* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x000000, 0x010000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )
	ROM_RELOAD(         0x010000, 0x010000 )

	ROM_REGION( 0x400000, "gfx1", 0 )
	/* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "gfx2", ROMREGION_ERASE00 )
	/* sprites - fake 6bpp decode is done from here */
	ROM_LOAD( "191a04", 0x000000, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD( "191a05", 0x100000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "gfx3", ROMREGION_ERASE00 )
	ROM_COPY("gfx2",0,0, 0x200000)

	ROM_REGION( 0x200000, "gfx4", ROMREGION_ERASE00 )
	ROM_COPY("gfx2",0x200000,0, 0x200000)

	ROM_REGION( 0x200000, "k054539", 0 )
	/* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethalenux.nv", 0x0000, 0x0080, CRC(5d69c39d) SHA1(e468df829ee5094792289f9166d7e39b638ab70d) )
ROM_END

ROM_START( lethaleneab )	// Euro ver. EAB
	ROM_REGION( 0x50000, "maincpu", 0 )
	/* main program */
	ROM_LOAD( "191eab01.u4",    0x10000,  0x40000, CRC(d7ce111e) SHA1(e56137a0ba7664f09b5d05bb39ec6eb4d1e412c7) )

	ROM_REGION( 0x020000, "soundcpu", 0 )
	/* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x000000, 0x010000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )
	ROM_RELOAD(         0x010000, 0x010000 )

	ROM_REGION( 0x400000, "gfx1", 0 )
	/* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "gfx2", ROMREGION_ERASE00 )
	/* sprites - fake 6bpp decode is done from here */
	ROM_LOAD( "191a04", 0x000000, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD( "191a05", 0x100000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "gfx3", ROMREGION_ERASE00 )
	ROM_COPY("gfx2",0,0, 0x200000)

	ROM_REGION( 0x200000, "gfx4", ROMREGION_ERASE00 )
	ROM_COPY("gfx2",0x200000,0, 0x200000)

	ROM_REGION( 0x200000, "k054539", 0 )
	/* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethaleneab.nv", 0x0000, 0x0080, CRC(4e9bb34d) SHA1(9502583bc9f5f6fc5bba333869398b24bf154b73) )
ROM_END

ROM_START( lethaleneae )	// Euro ver. EAE
	ROM_REGION( 0x50000, "maincpu", 0 )
	/* main program */
	ROM_LOAD( "191eae01.u4",    0x10000,  0x40000, CRC(c6a3c6ac) SHA1(96a209a3a5b4af40af36bd7090c59a74f8c8df59) )

	ROM_REGION( 0x020000, "soundcpu", 0 )
	/* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x000000, 0x010000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )
	ROM_RELOAD(         0x010000, 0x010000 )

	ROM_REGION( 0x400000, "gfx1", 0 )
	/* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "gfx2", ROMREGION_ERASE00 )
	/* sprites - fake 6bpp decode is done from here */
	ROM_LOAD( "191a04", 0x000000, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD( "191a05", 0x100000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "gfx3", ROMREGION_ERASE00 )
	ROM_COPY("gfx2",0,0, 0x200000)

	ROM_REGION( 0x200000, "gfx4", ROMREGION_ERASE00 )
	ROM_COPY("gfx2",0x200000,0, 0x200000)

	ROM_REGION( 0x200000, "k054539", 0 )
	/* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethaleneae.nv", 0x0000, 0x0080, CRC(eb369a67) SHA1(6c67294669614e96de5efb38372dbed435ee04d3) )
ROM_END

ROM_START( lethalenua )	// *might* be UAA (writes UA to Eeprom)
	ROM_REGION( 0x50000, "maincpu", 0 )
	/* main program */
	ROM_LOAD( "6_usa.u4",    0x10000,  0x40000, CRC(ab6b8f16) SHA1(8de6c429a6e71144270e79d18ad47b5aad13fe04) )

	ROM_REGION( 0x020000, "soundcpu", 0 )
	/* Z80 sound program */
	ROM_LOAD( "191a02.f4", 0x000000, 0x010000, CRC(72b843cc) SHA1(b44b2f039358c26fa792d740639b66a5c8bf78e7) )
	ROM_RELOAD(         0x010000, 0x010000 )

	ROM_REGION( 0x400000, "gfx1", 0 )
	/* tilemaps */
	ROM_LOAD32_WORD( "191a08", 0x000002, 0x100000, CRC(555bd4db) SHA1(d2e55796b4ab2306ae549fa9e7288e41eaa8f3de) )
	ROM_LOAD32_WORD( "191a10", 0x000000, 0x100000, CRC(2fa9bf51) SHA1(1e4ec56b41dfd8744347a7b5799e3ebce0939adc) )
	ROM_LOAD32_WORD( "191a07", 0x200002, 0x100000, CRC(1dad184c) SHA1(b2c4a8e48084005056aef2c8eaccb3d2eca71b73) )
	ROM_LOAD32_WORD( "191a09", 0x200000, 0x100000, CRC(e2028531) SHA1(63ccce7855d829763e9e248a6c3eb6ea89ab17ee) )

	ROM_REGION( 0x400000, "gfx2", ROMREGION_ERASE00 )
	/* sprites - fake 6bpp decode is done from here */
	ROM_LOAD( "191a04", 0x000000, 0x100000, CRC(5c3eeb2b) SHA1(33ea8b3968b78806334b5a0aab3a2c24e45c604e) )
	ROM_LOAD( "191a05", 0x100000, 0x100000, CRC(f2e3b58b) SHA1(0bbc2fe87a4fd00b5073a884bcfebcf9c2c402ad) )
	ROM_LOAD( "191a06", 0x200000, 0x100000, CRC(ee11fc08) SHA1(ec6dd684e8261b181d65b8bf1b9e97da5c4468f7) )

	ROM_REGION( 0x200000, "gfx3", ROMREGION_ERASE00 )
	ROM_COPY("gfx2",0,0, 0x200000)

	ROM_REGION( 0x200000, "gfx4", ROMREGION_ERASE00 )
	ROM_COPY("gfx2",0x200000,0, 0x200000)

	ROM_REGION( 0x200000, "k054539", 0 )
	/* K054539 samples */
	ROM_LOAD( "191a03", 0x000000, 0x200000, CRC(9b13fbe8) SHA1(19b02dbd9d6da54045b0ba4dfe7b282c72745c9c))

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "lethalenua.nv", 0x0000, 0x0080, CRC(f71ad1c3) SHA1(04c7052d0895797af8a06183b8a877795bf2dbb3) )
ROM_END


static DRIVER_INIT( lethalen )
{
	konamid_rom_deinterleave_2_half(machine, "gfx2");
	konamid_rom_deinterleave_2(machine, "gfx4");
}

GAME( 1992, lethalen,   0,        lethalen, lethalen, lethalen, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver UAE, 11/19/92 15:04)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE ) // writes UE to eeprom
GAME( 1992, lethalenua, lethalen, lethalen, lethalen, lethalen, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver unknown, US, 08/17/92 21:38)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE ) // UAA? (writes UA to eeprom)
GAME( 1992, lethalenux, lethalen, lethalen, lethalen, lethalen, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver unknown, US, 08/06/92 15:11, hacked/proto?)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE ) // writes UA to eeprom but earlier than suspected UAA set, might be a proto, might be hacked, fails rom test, definitely a good dump, another identical set was found in Italy
GAME( 1992, lethaleneab,lethalen, lethalen, lethalen, lethalen, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver EAB, 10/14/92 19:53)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE ) // writes EC to eeprom?!
GAME( 1992, lethaleneae,lethalen, lethalen, lethalen, lethalen, ORIENTATION_FLIP_Y, "Konami", "Lethal Enforcers (ver EAE, 11/19/92 16:24)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE ) // writes EE to eeprom
// different mirror / display setup
GAME( 1992, lethalenj,  lethalen, lethalej, lethalej, lethalen, ORIENTATION_FLIP_X, "Konami", "Lethal Enforcers (ver JAD, 12/04/92 17:16)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE ) // writes JC to eeprom?!
