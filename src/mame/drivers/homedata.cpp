// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Nicola Salmoria
/***************************************************************************

Homedata Games

driver by Phil Stroffolino and Nicola Salmoria


*1987 X77 Mahjong Hourouki Part1 -Seisyun Hen-
*1987 X72 Mahjong Hourouki Gaiden
 1988     Mahjong Joshi Pro-wres -Give up 5 byou mae-
*1988 A74 Mahjong Hourouki Okite
*1988 X80 Mahjong Clinic
*1988 M81 Mahjong Rokumeikan
*1988 J82 Reikai Doushi / Chinese Exorcist
*1989 X83 Mahjong Kojin Kyouju (Private Teacher)
 1989     Battle Cry (not released in Japan)
*1989 X90 Mahjong Vitamin C
*1989 X91 Mahjong Yougo no Kiso Tairyoku
*1990 X02 Mahjong Lemon Angel
*1991 X07 Mahjong Kinjirareta Asobi -Ike Ike Kyoushi no Yokubou-

Games from other companies:

*1991 M15 Mahjong Ikagadesuka     (c) Mitchell
*19??     Mahjong Jogakuen        (c) Windom


These games use only tilemaps for graphics.  These tilemaps are organized into
two pages (a visible page and a backbuffer) which are automatically swapped by the
hardware at vblank.

Some of the tiles are written directly by the CPU, others are written by a "blitter"
which unpacks RLE data from a ROM.


In games using the uPD7807CW, the coprocessor manages input ports and sound/music.


Notes:

- To access service mode in the older mahjong games, keep F2 pressed during boot.
  Service mode doesn't work in hourouki because it needs an additional "check" ROM.

- The "help" button some games ask you to press is the start button.

- The games can change visible area at runtime. The meaning of the control registers
  isn't understood, but it supported enough to give the correct visible area to all
  games.
  mjkinjas sets the registers to values different from all the other games; it also
  has a 11MHz xtal instead of the 9MHz of all the others, so the two things are
  probably related.

- the ROM for the uPD7807 is divided in 4 0x10000 banks. The first three just
  contain a simple sample player, followed by the PCM data. The fourth bank contains
  the main program. The program literally changes banks under its own feet. However,
  the portions of code executed while the bank switch happens are identical in all
  banks, so it still works with MAME's standard MRA8_BANK handling.


TODO:
- Dip switches! They might be right for mjhokite, but I haven't verified the other
  games.

- I'm not sure service mode in the newer mahjong games is working as it's supposed to.
  dip switch changes are not reported, and keypresses only work after you insert a coin.

- Sound sfxs don't work in mjikaga. CPU communication issue?
  Also note that bit 2 of bankswitch_w() and bit 7 of pteacher_blitter_bank_w() might
  have some other function, since the ROMs are smaller.

- wrong gfx in mrokumei at the beginning of a game. It is selecting the wrong gfx bank;
  the bank handling seems correct in all other games, so I don't know what's wrong here.

- mjikaga shows some imperfections with horizontal placement of the screen.

- in attract mode, hourouki draws a horizontal black bar on the bottom right side of
  the display.


----------------------------------------------------------------------------
Mahjong Hourouki
(c)1987 Home Data

Board:  A77-PWB-A-(A)

CPU:    68B09E Z80-A
Sound:  SN76489AN DAC
OSC:    9.000MHz 16.000MHz
Custom: GX61A01

----------------------------------------------------------------------------
Mahjong Hourouki Gaiden
(c)1987 Home Data

Board:  A77-PWB-A-(A)

CPU:    68B09E Z80-A
Sound:  SN76489AN DAC
OSC:    9.000MHz 16.000MHz
Custom: GX61A01

----------------------------------------------------------------------------
Mahjong Hourouki Okite
(c)1988 Homedata

Almost same board as "Mahjong Clinic"

Board:  X77-PWB-A-(A) A74 PWB-B

CPU:    Hitachi HD68B09EP (location 14G), Sharp LH0080A (Z80A, location 10K)
Sound:  SN76489 DAC?
OSC:    16.000MHz (OSC1) 9.000MHz (OSC2)
Custom: HOMEDATA GX61A01 102 8728KK (100pin PQFP, location 8C)

----------------------------------------------------------------------------
Mahjong Rokumeikan
(c)1988 Home Data

Board:  A74-PWB-A-(A) (main) A74 PWB-B     (sub)

CPU:    68B09E Z80-A
Sound:  SN76489AN DAC
OSC:    9.000MHz 16.000MHz
Custom: GX61A01

----------------------------------------------------------------------------
----------------------------------------------------------------------------
Reikai Doushi (Chinese Exorcist)
aka Last Apostle Puppet Show (US)
(c)1988 HOME DATA

CPU   : 68B09E
SOUND : YM2203
OSC.  : 16.000MHz 9.000MHz

----------------------------------------------------------------------------
----------------------------------------------------------------------------
Mahjong Kojinkyouju (Private Teacher)
(c)1989 HOME DATA

Board:  X73-PWB-A(C)

CPU:    6809 uPC324C
Sound:  SN76489
OSC:    16.000MHz 9.000MHz

----------------------------------------------------------------------------
Mahjong Vitamin C
(c)1989 Home Data

Board:  X73-PWB-A(A)

CPU:    68B09E uPD7807CW(?)
Sound:  SN76489AN DAC
OSC:    9.000MHz 16.000MHz
Custom: GX61A01

----------------------------------------------------------------------------
Mahjong-yougo no Kisotairyoku
(c)1989 Home Data

Board:  X83-PWB-A(A)

CPU:    68B09E uPD7807CW(?)
Sound:  SN76489AN DAC
OSC:    9.000MHz 16.000MHz
Custom: GX61A01

----------------------------------------------------------------------------
Mahjong Kinjirareta Asobi
(c)1990 Home Data

Board:  X83-PWB-A(A)

CPU:    68B09E uPD7807CW
Sound:  SN76489AN
        DAC
OSC:    11.000MHz 16.000MHz
Custom: GX61A01

----------------------------------------------------------------------------
Mahjong Jogakuen
(c)19?? Windom

Board:  X83-PWB-A(A)

CPU:    68B09E uPD7807CW(?)
Sound:  SN76489AN DAC
OSC:    9.000MHz 16.000MHz
Custom: GX61A01

----------------------------------------------------------------------------
----------------------------------------------------------------------------
Mahjong Lemon Angel
(c)1990 Homedata

Board:  X83-PWB-A(A)

CPU:    Fujitsu MBL68B09E (16G)
        (surface scratched 64pin DIP device on location 12G) [probably uPD7807CW]
Sound:  SN76489
OSC:    16.0000MHz (XTAL1) 9.000MHz (XTAL2)
Custom: HOMEDATA GX61A01 102 8842KK

----------------------------------------------------------------------------
Mahjong Ikagadesuka
(c)1991 Mitchell

Board:  X83-PWB-A(A)

CPU:    68B09E uPD7807CW
Sound:  SN76489AN DAC
OSC:    9.000MHz 16.000MHz
Custom: GX61A01

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/m6809.h"
#include "cpu/upd7810/upd7810.h"
#include "includes/homedata.h"
#include "sound/dac.h"
#include "sound/2203intf.h"

INTERRUPT_GEN_MEMBER(homedata_state::homedata_irq)
{
	m_vblank = 1;
	device.execute().set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(homedata_state::upd7807_irq)
{
	generic_pulse_irq_line(device.execute(), UPD7810_INTF1, 1);
}


/********************************************************************************

  Older Mahjong games:

  single CPU, handling SN76489 + DAC and inputs

 ********************************************************************************/

READ8_MEMBER(homedata_state::mrokumei_keyboard_r)
{
	int res = 0x3f,i;
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4" };

	/* offset 0 is player 1, offset 1 player 2 (not supported) */
	if (offset == 0)
	{
		for (i = 0; i < 5; i++)
		{
			if (m_keyb & (1 << i))
			{
				res = ioport(keynames[i])->read() & 0x3f;
				break;
			}
		}
	}

	if (offset == 0)
	{
		/* bit 7: visible page
		 * bit 6: vblank
		 * other bits are inputs
		 */
		res |= m_visible_page << 7;

		if (m_vblank)
			res |= 0x40;

		m_vblank = 0;
	}

	return res;
}

WRITE8_MEMBER(homedata_state::mrokumei_keyboard_select_w)
{
	m_keyb = data;
}


READ8_MEMBER(homedata_state::mrokumei_sound_io_r)
{
	if (m_sndbank & 4)
		return(soundlatch_byte_r(space, 0));
	else
		return memregion("audiocpu")->base()[0x10000 + offset + (m_sndbank & 1) * 0x10000];
}

WRITE8_MEMBER(homedata_state::mrokumei_sound_bank_w)
{
	/* bit 0 = ROM bank
	   bit 2 = ROM or soundlatch
	 */
	m_sndbank = data;
}

WRITE8_MEMBER(homedata_state::mrokumei_sound_io_w)
{
	switch (offset & 0xff)
	{
		case 0x40:
			m_dac->write_signed8(data);
			break;
		default:
			logerror("%04x: I/O write to port %04x\n", space.device().safe_pc(), offset);
			break;
	}
}

WRITE8_MEMBER(homedata_state::mrokumei_sound_cmd_w)
{
	soundlatch_byte_w(space, offset, data);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}


/********************************************************************************

  Reikai Doushi:

  slave uPD7807, handling YM2203 + DAC and inputs

 ********************************************************************************/

READ8_MEMBER(homedata_state::reikaids_upd7807_porta_r)
{
	return m_upd7807_porta;
}

WRITE8_MEMBER(homedata_state::reikaids_upd7807_porta_w)
{
	m_upd7807_porta = data;
}

WRITE8_MEMBER(homedata_state::reikaids_upd7807_portc_w)
{
	/* port C layout:
	   7 coin counter
	   6 to main CPU (data)
	   5 YM2203 write clock
	   4 YM2203 read clock
	   3 YM2203 address (0 = register select 1 = data)
	   2 to main CPU (status)
	   1 \ ROM bank
	   0 /
	  */
//  logerror("%04x: port C wr %02x (STATUS %d DATA %d)\n", space.device().safe_pc(), data, BIT(data, 2), BIT(data, 6));

	membank("bank2")->set_entry(data & 0x03);

	machine().bookkeeping().coin_counter_w(0, ~data & 0x80);

	if (BIT(m_upd7807_portc, 5) && !BIT(data, 5))   /* write clock 1->0 */
		m_ymsnd->write(space, BIT(data, 3), m_upd7807_porta);

	if (BIT(m_upd7807_portc, 4) && !BIT(data, 4))   /* read clock 1->0 */
		m_upd7807_porta = m_ymsnd->read(space, BIT(data, 3));

	m_upd7807_portc = data;
}

READ8_MEMBER(homedata_state::reikaids_io_r)
{
	int res = ioport("IN2")->read();    // bit 4 = coin, bit 5 = service

	res |= BIT(m_upd7807_portc, 2) * 0x01;      // bit 0 = upd7807 status
	res |= BIT(m_upd7807_portc, 6) * 0x02;      // bit 1 = upd7807 data
	if (m_vblank)
		res |= 0x04;                // bit 2 = vblank
	res |= m_visible_page * 0x08;   // bit 3 = visible page

	m_vblank = 0;

	//logerror("%04x: io_r %02x\n", space.device().safe_pc(), res);

	return res;
}

READ8_MEMBER(homedata_state::reikaids_snd_command_r)
{
	//logerror("%04x: sndmcd_r (%02x)\n", space.device().safe_pc(), m_snd_command);
	return m_snd_command;
}

WRITE8_MEMBER(homedata_state::reikaids_snd_command_w)
{
	m_snd_command = data;
	//logerror("%04x: coprocessor_command_w %02x\n", space.device().safe_pc(), data);
}


/********************************************************************************

  Newer Mahjong games:

  slave uPD7807, handling SN76489 + DAC and inputs

 ********************************************************************************/

WRITE8_MEMBER(homedata_state::pteacher_snd_command_w)
{
	//logerror("%04x: snd_command_w %02x\n", space.device().safe_pc(), data);
	m_from_cpu = data;
}

READ8_MEMBER(homedata_state::pteacher_snd_r)
{
	//logerror("%04x: pteacher_snd_r %02x\n",space.device().safe_pc(),to_cpu);
	return m_to_cpu;
}

READ8_MEMBER(homedata_state::pteacher_io_r)
{
	/* bit 6: !vblank
	 * bit 7: visible page
	 * other bits seem unused
	 */

	int res = (m_visible_page ^ 1) << 7;

	if (!m_vblank)
		res |= 0x40;

	m_vblank = 0;

	return res;
}

READ8_MEMBER(homedata_state::pteacher_keyboard_r)
{
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5" };
	int dips = ioport("DSW")->read();

	//  logerror("%04x: keyboard_r with port A = %02x\n",space.device().safe_pc(),upd7807_porta);

	if (m_upd7807_porta & 0x80)
	{
		/* player 1 + dip switches */
		int row = (m_upd7807_porta & 0x07);
		return ioport(keynames[row])->read() | (((dips >> row) & 1) << 5);  // 0-5
	}
	if (m_upd7807_porta & 0x08)
	{
		/* player 2 (not supported) + dip switches */
		int row = ((m_upd7807_porta >> 4) & 0x07);
		return 0xdf | (((dips >> (row + 5)) & 1) << 5); // 6-11
	}

	return 0xff;
}

READ8_MEMBER(homedata_state::pteacher_upd7807_porta_r)
{
	if (!BIT(m_upd7807_portc, 6))
		m_upd7807_porta = m_from_cpu;
	else
		logerror("%04x: read PA with PC *not* clear\n", space.device().safe_pc());

	return m_upd7807_porta;
}

WRITE8_MEMBER(homedata_state::pteacher_snd_answer_w)
{
	m_to_cpu = data;
	//logerror("%04x: to_cpu = %02x\n", space.device().safe_pc(), m_to_cpu);
}

WRITE8_MEMBER(homedata_state::pteacher_upd7807_porta_w)
{
	m_upd7807_porta = data;
}

WRITE8_MEMBER(homedata_state::pteacher_upd7807_portc_w)
{
	/* port C layout:
	   7 coin counter
	   6 enable message from main CPU on port A
	   5 clock latch for 76489 command
	   4
	   3 \ ROM bank
	   2 /
	   1 input (service)
	   0 input (coin)
	  */

	//  logerror("%04x: port C wr %02x\n", space.device().safe_pc(), data);

	membank("bank2")->set_entry((data & 0x0c) >> 2);

	machine().bookkeeping().coin_counter_w(0, ~data & 0x80);

	if (BIT(m_upd7807_portc, 5) && !BIT(data, 5))   /* clock 1->0 */
		m_sn->write(space, 0, m_upd7807_porta);

	m_upd7807_portc = data;
}

/********************************************************************************/


WRITE8_MEMBER(homedata_state::bankswitch_w)
{
	int last_bank = (memregion("maincpu")->bytes() - 0x10000) / 0x4000;

	/* last bank is fixed and is #0 for us, other banks start from #1 (hence data+1 below)*/
	if (data < last_bank)
		membank("bank1")->set_entry(data + 1);
	else
		membank("bank1")->set_entry(0);
}


/********************************************************************************/


static ADDRESS_MAP_START( mrokumei_map, AS_PROGRAM, 8, homedata_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM_WRITE(mrokumei_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4000, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x6fff) AM_RAM /* work ram */
	AM_RANGE(0x7000, 0x77ff) AM_RAM /* hourouki expects this to act as RAM */
	AM_RANGE(0x7800, 0x7800) AM_RAM /* only used to store the result of the ROM check */
	AM_RANGE(0x7801, 0x7802) AM_READ(mrokumei_keyboard_r)   // also vblank and active page
	AM_RANGE(0x7803, 0x7803) AM_READ_PORT("IN0")            // coin, service
	AM_RANGE(0x7804, 0x7804) AM_READ_PORT("DSW1")           // DSW1
	AM_RANGE(0x7805, 0x7805) AM_READ_PORT("DSW2")           // DSW2
	AM_RANGE(0x7ff0, 0x7ffd) AM_WRITEONLY AM_SHARE("vreg")
	AM_RANGE(0x7ffe, 0x7ffe) AM_READNOP // ??? read every vblank, value discarded
	AM_RANGE(0x8000, 0x8000) AM_WRITE(mrokumei_blitter_start_w) // in some games also ROM bank switch to access service ROM
	AM_RANGE(0x8001, 0x8001) AM_WRITE(mrokumei_keyboard_select_w)
	AM_RANGE(0x8002, 0x8002) AM_WRITE(mrokumei_sound_cmd_w)
	AM_RANGE(0x8003, 0x8003) AM_DEVWRITE("snsnd", sn76489a_device, write)
	AM_RANGE(0x8006, 0x8006) AM_WRITE(homedata_blitter_param_w)
	AM_RANGE(0x8007, 0x8007) AM_WRITE(mrokumei_blitter_bank_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mrokumei_sound_map, AS_PROGRAM, 8, homedata_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xfffc, 0xfffd) AM_WRITENOP    /* stack writes happen here, but there's no RAM */
	AM_RANGE(0x8080, 0x8080) AM_WRITE(mrokumei_sound_bank_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( mrokumei_sound_io_map, AS_IO, 8, homedata_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(mrokumei_sound_io_r, mrokumei_sound_io_w) /* read address is 16-bit, write address is only 8-bit */
ADDRESS_MAP_END

/********************************************************************************/

static ADDRESS_MAP_START( reikaids_map, AS_PROGRAM, 8, homedata_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM_WRITE(reikaids_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4000, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x6fff) AM_RAM /* work RAM */
	AM_RANGE(0x7800, 0x7800) AM_RAM /* behaves as normal RAM */
	AM_RANGE(0x7801, 0x7801) AM_READ_PORT("IN0")
	AM_RANGE(0x7802, 0x7802) AM_READ_PORT("IN1")
	AM_RANGE(0x7803, 0x7803) AM_READ(reikaids_io_r) // coin, blitter, upd7807
	AM_RANGE(0x7ff0, 0x7ffd) AM_WRITEONLY AM_SHARE("vreg")
	AM_RANGE(0x7ffe, 0x7ffe) AM_WRITE(reikaids_blitter_bank_w)
	AM_RANGE(0x7fff, 0x7fff) AM_WRITE(reikaids_blitter_start_w)
	AM_RANGE(0x8000, 0x8000) AM_WRITE(bankswitch_w)
	AM_RANGE(0x8002, 0x8002) AM_WRITE(reikaids_snd_command_w)
	AM_RANGE(0x8005, 0x8005) AM_WRITE(reikaids_gfx_bank_w)
	AM_RANGE(0x8006, 0x8006) AM_WRITE(homedata_blitter_param_w)
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( reikaids_upd7807_map, AS_PROGRAM, 8, homedata_state )
	AM_RANGE(0x0000, 0xfeff) AM_ROMBANK("bank2")    /* External ROM (Banked) */
	AM_RANGE(0xff00, 0xffff) AM_RAM /* Internal RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( reikaids_upd7807_io_map, AS_IO, 8, homedata_state )
	AM_RANGE(UPD7807_PORTA, UPD7807_PORTA) AM_READWRITE(reikaids_upd7807_porta_r, reikaids_upd7807_porta_w)
	AM_RANGE(UPD7807_PORTB, UPD7807_PORTB) AM_DEVWRITE("dac", dac_device, write_signed8)
	AM_RANGE(UPD7807_PORTC, UPD7807_PORTC) AM_WRITE(reikaids_upd7807_portc_w)
	AM_RANGE(UPD7807_PORTT, UPD7807_PORTT) AM_READ(reikaids_snd_command_r)
ADDRESS_MAP_END

/**************************************************************************/


static ADDRESS_MAP_START( pteacher_map, AS_PROGRAM, 8, homedata_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM_WRITE(mrokumei_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4000, 0x5eff) AM_RAM
	AM_RANGE(0x5f00, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x6fff) AM_RAM /* work ram */
	AM_RANGE(0x7800, 0x7800) AM_RAM /* behaves as normal RAM */
	AM_RANGE(0x7801, 0x7801) AM_READ(pteacher_io_r) // vblank, visible page
	AM_RANGE(0x7ff2, 0x7ff2) AM_READ(pteacher_snd_r)
	AM_RANGE(0x7ff0, 0x7ffd) AM_WRITEONLY AM_SHARE("vreg")
	AM_RANGE(0x7fff, 0x7fff) AM_WRITE(pteacher_blitter_start_w)
	AM_RANGE(0x8000, 0x8000) AM_WRITE(bankswitch_w)
	AM_RANGE(0x8002, 0x8002) AM_WRITE(pteacher_snd_command_w)
	AM_RANGE(0x8005, 0x8005) AM_WRITE(pteacher_blitter_bank_w)
	AM_RANGE(0x8006, 0x8006) AM_WRITE(homedata_blitter_param_w)
	AM_RANGE(0x8007, 0x8007) AM_WRITE(pteacher_gfx_bank_w)
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pteacher_upd7807_map, AS_PROGRAM, 8, homedata_state )
	AM_RANGE(0x0000, 0x0000) AM_WRITE(pteacher_snd_answer_w)
	AM_RANGE(0x0000, 0xfeff) AM_ROMBANK("bank2")    /* External ROM (Banked) */
	AM_RANGE(0xff00, 0xffff) AM_RAM /* Internal RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( pteacher_upd7807_io_map, AS_IO, 8, homedata_state )
	AM_RANGE(UPD7807_PORTA, UPD7807_PORTA) AM_READWRITE(pteacher_upd7807_porta_r, pteacher_upd7807_porta_w)
	AM_RANGE(UPD7807_PORTB, UPD7807_PORTB) AM_DEVWRITE("dac", dac_device, write_signed8)
	AM_RANGE(UPD7807_PORTC, UPD7807_PORTC) AM_READ_PORT("COIN") AM_WRITE(pteacher_upd7807_portc_w)
	AM_RANGE(UPD7807_PORTT, UPD7807_PORTT) AM_READ(pteacher_keyboard_r)
ADDRESS_MAP_END

/**************************************************************************/


static INPUT_PORTS_START( mjhokite )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, "Initial Score" )
	PORT_DIPSETTING(    0x10, "1000" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xe0, "1 (easiest)" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0xa0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x60, "5" )
	PORT_DIPSETTING(    0x40, "6" )
	PORT_DIPSETTING(    0x20, "7" )
	PORT_DIPSETTING(    0x00, "8 (hardest)" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Girl Voice" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x02, 0x02, "Freeze?" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )   // doesn't work in all games
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1                              )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( reikaids )
	PORT_START("IN0")   // 0x7801
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT(  0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT(  0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) /* punch */
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) /* kick */
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) /* jump */
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")   // 0x7802
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT(  0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT(  0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* punch */
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) /* kick */
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) /* jump */
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")   // 0x7803
	PORT_BIT(  0x01, IP_ACTIVE_HIGH,IPT_SPECIAL ) /* coprocessor status */
	PORT_BIT(  0x02, IP_ACTIVE_HIGH,IPT_SPECIAL ) /* coprocessor data */
	PORT_BIT(  0x04, IP_ACTIVE_HIGH,IPT_SPECIAL ) /* vblank */
	PORT_BIT(  0x08, IP_ACTIVE_HIGH,IPT_SPECIAL ) /* visible page */
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_COIN1    )
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "20k then every 60k" )
	PORT_DIPSETTING(    0x04, "30k then every 80k" )
	PORT_DIPSETTING(    0x02, "20k" )
	PORT_DIPSETTING(    0x00, "30k" )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x20, "Time" )
	PORT_DIPSETTING(    0x20, "60" )
	PORT_DIPSETTING(    0x00, "45" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_5C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( battlcry )
	PORT_INCLUDE( reikaids )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, "Up To Stage 1" )
	PORT_DIPSETTING(    0x06, "Up To Stage 2" )
	PORT_DIPSETTING(    0x01, "Up To Stage 3" )
	PORT_DIPSETTING(    0x05, "Up To Stage 4" )
	PORT_DIPSETTING(    0x03, "Up To Stage 5" )
	PORT_DIPSETTING(    0x07, "Up To Stage 6" )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x20, 0x20, "Time" )
	PORT_DIPSETTING(    0x20, "90" )
	PORT_DIPSETTING(    0x00, "120" )
	PORT_DIPNAME( 0x40, 0x40, "Test Mode" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Background Music" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END


static INPUT_PORTS_START( mj_keyboard )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* dip switch (handled separately) */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* dip switch (handled separately) */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* dip switch (handled separately) */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* dip switch (handled separately) */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* dip switch (handled separately) */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* dip switch (handled separately) */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( pteacher )
	PORT_START("DSW")   /* dip switches (handled by pteacher_keyboard_r) */
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0100, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_INCLUDE( mj_keyboard )
INPUT_PORTS_END

static INPUT_PORTS_START( lemnangl )
	PORT_INCLUDE( pteacher )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, "In-Game BGM" )           PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Female Voices" )         PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0020, 0x0020, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, 0x0040, "SW1:7" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_SERVICE_DIPLOC(0x0100, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( jogakuen )
	PORT_START("DSW")   /* dip switches (handled by pteacher_keyboard_r) */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0400, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_INCLUDE( mj_keyboard )
INPUT_PORTS_END

static INPUT_PORTS_START( mjikaga )
	PORT_START("DSW")   /* dip switches (handled by pteacher_keyboard_r) */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_INCLUDE( mj_keyboard )
INPUT_PORTS_END


/**************************************************************************/


static const gfx_layout char_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( mrokumei )
	GFXDECODE_ENTRY( "gfx1", 0, char_layout, 0x6000, 0x100 )
	GFXDECODE_ENTRY( "gfx2", 0, char_layout, 0x7000, 0x100 )
GFXDECODE_END

static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static GFXDECODE_START( reikaids )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 0x6000, 0x20 )
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout, 0x4000, 0x20 )
	GFXDECODE_ENTRY( "gfx3", 0, tile_layout, 0x2000, 0x20 )
	GFXDECODE_ENTRY( "gfx4", 0, tile_layout, 0x0000, 0x20 )
GFXDECODE_END

static GFXDECODE_START( pteacher )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 0x0000, 0x40 )
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout, 0x4000, 0x40 )
GFXDECODE_END

static const gfx_layout tile_layout_4bpp_hi =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static const gfx_layout tile_layout_4bpp_lo =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static GFXDECODE_START( lemnangl )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout_4bpp_hi, 0x0000, 0x200 )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout_4bpp_lo, 0x2000, 0x200 )
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout_4bpp_lo, 0x4000, 0x200 )
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout_4bpp_hi, 0x6000, 0x200 )
GFXDECODE_END


MACHINE_START_MEMBER(homedata_state,homedata)
{
	m_sn = machine().device<sn76489a_device>("snsnd");

	save_item(NAME(m_visible_page));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_blitter_bank));
	save_item(NAME(m_blitter_param_count));
	save_item(NAME(m_blitter_param));
	save_item(NAME(m_vblank));
	save_item(NAME(m_sndbank));
	save_item(NAME(m_keyb));
	save_item(NAME(m_snd_command));
}

MACHINE_START_MEMBER(homedata_state,reikaids)
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 8, &ROM[0xc000], 0x4000);
	membank("bank2")->configure_entries(0, 4, memregion("audiocpu")->base(), 0x10000);

	MACHINE_START_CALL_MEMBER(homedata);

	save_item(NAME(m_upd7807_porta));
	save_item(NAME(m_upd7807_portc));

	save_item(NAME(m_reikaids_which));
	save_item(NAME(m_gfx_bank));
}

MACHINE_START_MEMBER(homedata_state,pteacher)
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 4, &ROM[0xc000], 0x4000);
	membank("bank2")->configure_entries(0, 4, memregion("audiocpu")->base(), 0x10000);

	MACHINE_START_CALL_MEMBER(homedata);

	save_item(NAME(m_upd7807_porta));
	save_item(NAME(m_upd7807_portc));

	save_item(NAME(m_gfx_bank));
	save_item(NAME(m_to_cpu));
	save_item(NAME(m_from_cpu));
}

MACHINE_RESET_MEMBER(homedata_state,homedata)
{
	m_visible_page = 0;
	m_flipscreen = 0;
	m_blitter_bank = 0;
	m_blitter_param_count = 0;
	m_blitter_param[0] = 0;
	m_blitter_param[1] = 0;
	m_blitter_param[2] = 0;
	m_blitter_param[3] = 0;
	m_vblank = 0;
	m_sndbank = 0;
	m_keyb = 0;
	m_snd_command = 0;
}

MACHINE_RESET_MEMBER(homedata_state,pteacher)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* on reset, ports are set as input (high impedance), therefore 0xff output */
	pteacher_upd7807_portc_w(space, 0, 0xff);

	MACHINE_RESET_CALL_MEMBER(homedata);

	m_upd7807_porta = 0;
	m_gfx_bank[0] = 0;
	m_gfx_bank[1] = 0;
	m_to_cpu = 0;
	m_from_cpu = 0;
}

MACHINE_RESET_MEMBER(homedata_state,reikaids)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* on reset, ports are set as input (high impedance), therefore 0xff output */
	reikaids_upd7807_portc_w(space, 0, 0xff);

	MACHINE_RESET_CALL_MEMBER(homedata);

	m_reikaids_which = m_priority;  // m_priority is set in DRIVER_INIT
	m_upd7807_porta = 0;
	m_gfx_bank[0] = 0;
	m_gfx_bank[1] = 0;  // this is not used by reikaids
}

static MACHINE_CONFIG_START( mrokumei, homedata_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 16000000/4)  /* 4MHz ? */
	MCFG_CPU_PROGRAM_MAP(mrokumei_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", homedata_state,  homedata_irq) /* also triggered by the blitter */

	MCFG_CPU_ADD("audiocpu", Z80, 16000000/4)   /* 4MHz ? */
	MCFG_CPU_PROGRAM_MAP(mrokumei_sound_map)
	MCFG_CPU_IO_MAP(mrokumei_sound_io_map)

	MCFG_MACHINE_START_OVERRIDE(homedata_state,homedata)
	MCFG_MACHINE_RESET_OVERRIDE(homedata_state,homedata)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	// visible area can be changed at runtime
	MCFG_SCREEN_VISIBLE_AREA(0*8, 54*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(homedata_state, screen_update_mrokumei)
	MCFG_SCREEN_VBLANK_DRIVER(homedata_state, screen_eof_homedata)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mrokumei)
	MCFG_PALETTE_ADD("palette", 0x8000)

	MCFG_PALETTE_INIT_OWNER(homedata_state,mrokumei)
	MCFG_VIDEO_START_OVERRIDE(homedata_state,mrokumei)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("snsnd", SN76489A, 16000000/4)     // SN76489AN actually
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/**************************************************************************/

static MACHINE_CONFIG_START( reikaids, homedata_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 16000000/4)  /* 4MHz ? */
	MCFG_CPU_PROGRAM_MAP(reikaids_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", homedata_state,  homedata_irq) /* also triggered by the blitter */

	MCFG_CPU_ADD("audiocpu", UPD7807, 8000000)  /* ??? MHz (max speed for the 7807 is 12MHz) */
	MCFG_CPU_PROGRAM_MAP(reikaids_upd7807_map)
	MCFG_CPU_IO_MAP(reikaids_upd7807_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", homedata_state,  upd7807_irq)

	MCFG_QUANTUM_TIME(attotime::from_hz(30000)) // very high interleave required to sync for startup tests

	MCFG_MACHINE_START_OVERRIDE(homedata_state,reikaids)
	MCFG_MACHINE_RESET_OVERRIDE(homedata_state,reikaids)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 16, 256-1-16)
	MCFG_SCREEN_UPDATE_DRIVER(homedata_state, screen_update_reikaids)
	MCFG_SCREEN_VBLANK_DRIVER(homedata_state, screen_eof_homedata)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", reikaids)
	MCFG_PALETTE_ADD("palette", 0x8000)

	MCFG_PALETTE_INIT_OWNER(homedata_state,reikaids)
	MCFG_VIDEO_START_OVERRIDE(homedata_state,reikaids)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 3000000)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 0.25)
	MCFG_SOUND_ROUTE(2, "mono", 0.25)
	MCFG_SOUND_ROUTE(3, "mono", 1.0)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END


/**************************************************************************/

static MACHINE_CONFIG_START( pteacher, homedata_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 16000000/4)  /* 4MHz ? */
	MCFG_CPU_PROGRAM_MAP(pteacher_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", homedata_state,  homedata_irq) /* also triggered by the blitter */

	MCFG_CPU_ADD("audiocpu", UPD7807, 9000000)  /* 9MHz ? */
	MCFG_CPU_PROGRAM_MAP(pteacher_upd7807_map)
	MCFG_CPU_IO_MAP(pteacher_upd7807_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", homedata_state,  upd7807_irq)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))  // should be enough

	MCFG_MACHINE_START_OVERRIDE(homedata_state,pteacher)
	MCFG_MACHINE_RESET_OVERRIDE(homedata_state,pteacher)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	// visible area can be changed at runtime
	MCFG_SCREEN_VISIBLE_AREA(0*8, 54*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(homedata_state, screen_update_pteacher)
	MCFG_SCREEN_VBLANK_DRIVER(homedata_state, screen_eof_homedata)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pteacher)
	MCFG_PALETTE_ADD("palette", 0x8000)

	MCFG_PALETTE_INIT_OWNER(homedata_state,pteacher)
	MCFG_VIDEO_START_OVERRIDE(homedata_state,pteacher)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("snsnd", SN76489A, 16000000/4)     // SN76489AN actually
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mjkinjas, pteacher )

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_CLOCK(11000000)    /* 11MHz ? */
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( lemnangl, pteacher )

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", lemnangl)

	MCFG_VIDEO_START_OVERRIDE(homedata_state,lemnangl)
MACHINE_CONFIG_END

static INPUT_PORTS_START( mirderby )
INPUT_PORTS_END


static ADDRESS_MAP_START( cpu0_map, AS_PROGRAM, 8, homedata_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu1_map, AS_PROGRAM, 8, homedata_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM // videoram
	AM_RANGE(0x4000, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x6fff) AM_RAM /* work ram */
	AM_RANGE(0x7000, 0x77ff) AM_RAM
	//0x7ff0 onward is the blitter
	AM_RANGE(0x7ffe, 0x7ffe) AM_READNOP //watchdog
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


READ8_MEMBER(homedata_state::mirderby_prot_r)
{
	m_prot_data&=0x7f;
	return m_prot_data++;
}

WRITE8_MEMBER(homedata_state::mirderby_prot_w)
{
	m_prot_data = data;
}


static ADDRESS_MAP_START( cpu2_map, AS_PROGRAM, 8, homedata_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM_WRITE(mrokumei_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4000, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x6fff) AM_RAM /* work ram */
	AM_RANGE(0x7000, 0x77ff) AM_RAM
	AM_RANGE(0x7800, 0x7800) AM_READWRITE(mirderby_prot_r, mirderby_prot_w) // protection check? (or sound comms?)
	AM_RANGE(0x7ffe, 0x7ffe) AM_READNOP //watchdog
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END





static const gfx_layout mirderbychar_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( mirderby )
	GFXDECODE_ENTRY( "gfx1", 0, mirderbychar_layout, 0x0000, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0, mirderbychar_layout, 0x0000, 0x10 )
GFXDECODE_END

/*   Miracle Derby - Ascot

   - has the same GX61A01 custom (blitter?) as homedata.c and a 'similar' CPU setup (this has more CPUs)
     and similar board / rom numbering (X**-)

     The drivers can probably be merged later, although the current per-game handling of the blitter in
     homedata.c should be looked at.



        Notes from Stefan Lindberg:

        Eprom "x70_a04.5g" had wires attached to it, pin 2 and 16 was joined and pin 1,32,31,30 was joined, i
        removed them and read the eprom as the type it was (D27c1000D).

        Measured frequencies:
        MBL68B09E = 2mhz
        MBL68B09E = 2mhz
        z80 = 4mhz
        YM2203 = 2mhz

        See included PCB pics.



        Roms:

        Name              Size     CRC32         Chip Type
        ---------------------------------------------------------------------------------
        x70a07.8l         256      0x7d4c9712    82s129
        x70a08.7l         256      0xc4e77174    82s129
        x70a09.6l         256      0xd0187957    82s129
        x70_a03.8g        32768    0x4e298b2d    27c256
        x70_a04.5g        131072   0x14392fdb    D27c1000D
        x70_a11.1g        32768    0xb394eef7    27c256
        x70_b02.12e       32768    0x76c9bb6f    27c256
        x70_c01.14e       65536    0xd79d072d    27c512



*/

/* clocks are 16mhz and 9mhz */

static MACHINE_CONFIG_START( mirderby, homedata_state )

	MCFG_CPU_ADD("maincpu", M6809, 16000000/8)  /* 2 Mhz */
	MCFG_CPU_PROGRAM_MAP(cpu2_map)


	/* basic machine hardware */
	MCFG_CPU_ADD("cpu0", Z80, 16000000/4)   /* 4 Mhz */
	MCFG_DEVICE_DISABLE()
	MCFG_CPU_PROGRAM_MAP(cpu0_map)

	MCFG_CPU_ADD("cpu1", M6809, 16000000/8) /* 2 Mhz */
	MCFG_CPU_PROGRAM_MAP(cpu1_map)
	MCFG_DEVICE_DISABLE()
	//MCFG_CPU_VBLANK_INT("screen", mirderby_irq)


	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 54*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(homedata_state, screen_update_mirderby)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mirderby)
	MCFG_PALETTE_ADD("palette", 0x8000)

	MCFG_PALETTE_INIT_OWNER(homedata_state,mirderby)
	MCFG_VIDEO_START_OVERRIDE(homedata_state,mirderby)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 2000000)
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 0.25)
	MCFG_SOUND_ROUTE(2, "mono", 0.25)
	MCFG_SOUND_ROUTE(3, "mono", 1.0)
MACHINE_CONFIG_END

/**************************************************************************/


ROM_START( hourouki )
	ROM_REGION( 0x010000, "maincpu", 0 ) /* 6809 Code */
	ROM_LOAD( "x77f01.bin", 0x08000, 0x8000, CRC(cd3197b8) SHA1(7dad9ce57a83d675a8a9a4e06df360c22100fe4b) )

	ROM_REGION( 0x30000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "x77a10.bin", 0x00000, 0x20000, CRC(dc1d616b) SHA1(93b8dfe1566556e9621c0d5f3998b31874f74a28) )
	ROM_RELOAD(             0x10000, 0x20000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "x77a03.bin", 0, 0x20000, CRC(5960cde8) SHA1(857b56b304423102a5f1fc88ba4a2386d012bf4d) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "x77a04.bin", 0x00000, 0x20000, CRC(fd348e59) SHA1(df0eb4d24e3e778e7a06b7fd7fa4e077fa0ebf82) )
	ROM_LOAD( "x77a05.bin", 0x20000, 0x20000, CRC(3f76c8af) SHA1(2b41a9a06f8ccbb9f3879c742fa8af4424837fb2) )

	ROM_REGION( 0x010000, "proms", 0 )  /* static palette */
	ROM_LOAD16_BYTE( "x77e06.bin", 0x00000, 0x8000, CRC(63607fe5) SHA1(50c756d741117bd0ea8a877bcb1f025e02cc6d29) )
	ROM_LOAD16_BYTE( "x77e07.bin", 0x00001, 0x8000, CRC(79fcfc57) SHA1(4311a6bd8aa2dcad2b4fa5a9091c27dd74df62eb) )

	ROM_REGION( 0x20000, "user1", 0 ) /* blitter data */
	ROM_LOAD( "x77a08.bin", 0x0000, 0x20000, CRC(22bde229) SHA1(8b44fa895f77b5c95d798172225a030fa0c04126) )
ROM_END

ROM_START( mhgaiden )
	ROM_REGION( 0x010000, "maincpu", 0 ) /* 6809 Code */
	ROM_LOAD( "x72e01.bin", 0x08000, 0x8000, CRC(98cfa53e) SHA1(dd08f5f9ff9d4a9e01e531247fcb17a8407ca8b6) )

	ROM_REGION( 0x30000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "x72b10.bin", 0x00000, 0x20000, CRC(00ebbc45) SHA1(9e7ade202bf37a86153a38d705ae26a72732d2bb) )
	ROM_RELOAD(             0x10000, 0x20000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "x72b03.bin", 0, 0x20000, CRC(9019936f) SHA1(5b98ad1b4c0b141bc84fc17bea49784883c5775a) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "x72b04.bin", 0x00000, 0x20000, CRC(37e3e779) SHA1(7011159dee2c643c3fd6ffbbd0849f63933279ad) )
	ROM_LOAD( "x72b05.bin", 0x20000, 0x20000, CRC(aa5ce6f6) SHA1(d97b08fce68a69b1445d5ab28c0c97fabb5f264b) )

	ROM_REGION( 0x010000, "proms", 0 )  /* static palette */
	ROM_LOAD16_BYTE( "x72c06.bin", 0x00000, 0x8000, CRC(b57fb589) SHA1(21b2fc33b9045a4ffa9e4b0bd598fd3416c0adbf) )
	ROM_LOAD16_BYTE( "x72c07.bin", 0x00001, 0x8000, CRC(2aadb285) SHA1(90af8541092fbb116b6d6eaf1511b49a2f9bceaf) )

	ROM_REGION( 0x20000, "user1", 0 ) /* blitter data */
	ROM_LOAD( "x72b08.bin", 0x0000, 0x20000, CRC(be312d23) SHA1(f2c18d6372c2f819248976c67abe0ddcd5cc1de1) )
ROM_END

ROM_START( mjhokite )
	ROM_REGION( 0x010000, "maincpu", 0 ) /* 6809 Code */
	ROM_LOAD( "a74_g01.6g", 0x08000, 0x8000, CRC(409cc501) SHA1(6e9ab81198a5a54489cca8b6dcafb67995351207) )

	ROM_REGION( 0x30000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "a74_a10.11k", 0x00000, 0x20000, CRC(2252f3ec) SHA1(018aaad087354b05b120aa42db572ed13f690f88) )
	ROM_RELOAD(              0x10000, 0x20000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "a74_a03.1g", 0, 0x20000, CRC(bf801b74) SHA1(fd9b7a4590fdeba90423a7ceccd8dfbb8826460b) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "a74_a040.bin", 0x00000, 0x20000, CRC(b7a4ddbd) SHA1(70b9e44da7f32120fe86b23e431c3ae1eb36ef1a) )
	ROM_LOAD( "a74_a050.bin", 0x20000, 0x20000, CRC(c1718d39) SHA1(e37e2338b7e9409d9d542d30f363d07c7a873602) )
	ROM_LOAD( "a74_a041.bin", 0x40000, 0x20000, CRC(c6a6407d) SHA1(7421f4ae8b2959d16114cadc9098156e5a97b36f) )
	ROM_LOAD( "a74_a051.bin", 0x60000, 0x20000, CRC(74522b81) SHA1(6bd9655005f36887bec7c127f467e765447acd8f) )

	ROM_REGION( 0x010000, "proms", 0 )  /* static palette */
	ROM_LOAD16_BYTE( "a74_a06.1l", 0x00000, 0x8000, CRC(df057dd3) SHA1(0afae441b43a87e04511b8e652d2b03f48d8f705) )
	ROM_LOAD16_BYTE( "a74_a07.1m", 0x00001, 0x8000, CRC(3c230167) SHA1(5d57f614bf07baa8b5c96f1d14241ff7c66806c1) )

	ROM_REGION( 0x20000, "user1", 0 ) /* blitter data */
	ROM_LOAD( "a74_a08.13a", 0x0000, 0x20000, CRC(dffdd855) SHA1(91469a997a6a9f74f1b84c127f30f5b0e2f974ac) )
ROM_END

ROM_START( mjclinic )
	ROM_REGION( 0x010000, "maincpu", 0 ) /* 6809 Code */
	ROM_LOAD( "x80_g01.6g", 0x08000, 0x8000, CRC(787b4fb5) SHA1(d1708979d209113b604f6d0973fe14a0c4348351) )

	ROM_REGION( 0x30000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "x80_a10.11k", 0x00000, 0x20000, CRC(afedbadf) SHA1(e2f101b59c0d23f9dc9b057c41d496fc3223cbb8) )
	ROM_RELOAD(              0x10000, 0x20000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "x80_a03.1g", 0, 0x20000, CRC(34b63c89) SHA1(98383ce8e464ecc53677c060338cc03aa6238a0e) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "x80_a040.bin", 0x00000, 0x20000, CRC(6f18a8cf) SHA1(af8de2bd8b2157ceaa8192ead51ac4489c464ab2) )
	ROM_LOAD( "x80_a050.bin", 0x20000, 0x20000, CRC(6b1ec3a9) SHA1(dc7a857bb102325ebf2522369a231260c2e02c89) )
	ROM_LOAD( "x80_a041.bin", 0x40000, 0x20000, CRC(f70bb001) SHA1(3b29bb0bd155e97d3ccc72a8a07046c676c8452d) )
	ROM_LOAD( "x80_a051.bin", 0x60000, 0x20000, CRC(c7469cb8) SHA1(bcf7021667e943991865fdbc9620f00b09e1db3e) )

	ROM_REGION( 0x010000, "proms", 0 )  /* static palette */
	ROM_LOAD16_BYTE( "x80_a06.1l", 0x00000, 0x8000, CRC(c1f9b2fb) SHA1(abe17e2b3b880f91564e32c246116c33e2884889) )
	ROM_LOAD16_BYTE( "x80_a07.1m", 0x00001, 0x8000, CRC(e3120152) SHA1(b4a778a9b91e204e2c068dff9a40bd29eccf04a5) )

	ROM_REGION( 0x20000, "user1", 0 ) /* blitter data */
	ROM_LOAD( "x80_a08.13a", 0x0000, 0x20000, CRC(174e8ec0) SHA1(a5075fe4bba9403ef9e0636d5f3f66aad8b2bc54) )
ROM_END

ROM_START( mrokumei )
	ROM_REGION( 0x010000, "maincpu", 0 ) /* 6809 Code */
	ROM_LOAD( "m81d01.bin", 0x08000, 0x8000, CRC(6f81a78a) SHA1(5e16327b04b065ed4e39a147b18711902dba6384) )

	ROM_REGION( 0x30000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "m81a10.bin", 0x00000, 0x20000, CRC(0866b2d3) SHA1(37a726830476e372db906382e1d0601c461c7c10) )
	ROM_RELOAD(             0x10000, 0x20000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "m81a03.bin", 0, 0x20000, CRC(4f96e6d2) SHA1(9e6375ee543215691b28c914a8374c1d75c6681f) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "m81a40.bin", 0x00000, 0x20000, CRC(f07c6a91) SHA1(1e8b07817bb3661211b22c8c0ee35424b3ee8774) )
	ROM_LOAD( "m81a50.bin", 0x20000, 0x20000, CRC(5ef0d7f2) SHA1(9d0af693b95d941c4b5702c483aee9d33c2fb886) )
	ROM_LOAD( "m81a41.bin", 0x40000, 0x20000, CRC(9332b879) SHA1(fc70a6acf816564c1c90c2f9aa644f702a38eae1) )
	ROM_LOAD( "m81a51.bin", 0x60000, 0x20000, CRC(dda3ae30) SHA1(a0ba05cc46a4b2869ddf544c538e40a40f3babd5) )

	ROM_REGION( 0x010000, "proms", 0 )  /* static palette */
	ROM_LOAD16_BYTE( "m81b06.bin", 0x00000, 0x8000, CRC(96665d39) SHA1(9173791831555e69b8938d85340a08dd73012f8d) )
	ROM_LOAD16_BYTE( "m81b07.bin", 0x00001, 0x8000, CRC(14f39690) SHA1(821f9de1b28b9d7844fc1b002c7ee3bbdda7f905) )

	ROM_REGION( 0x20000, "user1", 0 ) /* blitter data */
	ROM_LOAD( "m81a08.bin", 0x0000, 0x20000, CRC(dba706b9) SHA1(467c8c3e12cd64002d2516dd117bc87d03448f49) )
ROM_END


ROM_START( reikaids )
	ROM_REGION( 0x02c000, "maincpu", 0 ) /* 6809 Code */
	ROM_LOAD( "j82c01.bin", 0x010000, 0x01c000, CRC(50fcc451) SHA1(ad717b8300f0903ef136569cf933b8af0e67eb6b) )
	ROM_CONTINUE(           0x00c000, 0x004000             )

	ROM_REGION( 0x40000, "audiocpu", 0) /* uPD7807 code */
	ROM_LOAD( "x82a04.bin", 0x000000, 0x040000, CRC(52c9028a) SHA1(9d5e37b2f741d5c0e64ba3d674a72330058b96f2) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "x82a13.bin",  0x000000, 0x80000, CRC(954c8844) SHA1(94e09009d0ad40a62f2a92cd5bd500ffe2c4650a) )
	ROM_LOAD( "x82a14.bin",  0x080000, 0x80000, CRC(a748305e) SHA1(ee01025820a0915621d2c056d65e67a84a5f724c) )
	ROM_LOAD( "x82a15.bin",  0x100000, 0x80000, CRC(c50f7047) SHA1(13fcd1abaf88c759e747cf5f2c94a522276945c9) )
	ROM_LOAD( "x82a16.bin",  0x180000, 0x80000, CRC(b270094a) SHA1(edd4f7e3fd082c12758d82ea6eba2a060398b9ef) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "x82a09.bin",  0x000000, 0x80000, CRC(c496b187) SHA1(8ec4e09469c9f425681671f5ddb2d2d8768af42b) )
	ROM_LOAD( "x82a10.bin",  0x080000, 0x80000, CRC(4243fe28) SHA1(01260543a6f61f46ba56840c1b96ecedf637451c) )

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD( "x82a08.bin",  0x000000, 0x80000, CRC(51cfd790) SHA1(208f00b1ccf420fe891dd30684481bf871c1aa63) )

	ROM_REGION( 0x080000, "gfx4", 0 )
	ROM_LOAD( "x82a05.bin",  0x000000, 0x80000, CRC(fb65e0e0) SHA1(d560091cfad17af6539913db1279c62a680de4fc) )

	ROM_REGION( 0x010000, "proms", 0 )  /* static palette */
	ROM_LOAD16_BYTE( "e82a18.bin", 0x00000, 0x8000, CRC(1f52a7aa) SHA1(55bbba5bfad1eee4872939d23ac643aa3074b3cf) )
	ROM_LOAD16_BYTE( "e82a17.bin", 0x00001, 0x8000, CRC(f91d77a1) SHA1(a650a68e7e75719819fd04dda86d7fc8849dfe7d) )

	ROM_REGION( 0x40000, "user1", 0 ) /* blitter data */
	ROM_LOAD( "x82a02.bin", 0x00000, 0x040000, CRC(90fe700f) SHA1(bf7f9955a2cb1af43a272bf3366ff8c09ff6f7e6) )

	ROM_REGION( 0x0100, "user2", 0 )
	ROM_LOAD( "x82a19.bin", 0x0000, 0x0100, CRC(7ed947b4) SHA1(40c74a17976fab5d7f9da367083764934bb87281) )  // priority (not used)
ROM_END


ROM_START( battlcry )
	ROM_REGION( 0x02c000, "maincpu", 0 ) /* 6809 Code */
	ROM_LOAD( "s88e01.j13", 0x010000, 0x01c000, CRC(b08438fe) SHA1(41a0fcdabee449081840848c45983984d7153d1b) )
	ROM_CONTINUE(           0x00c000, 0x004000             )

	ROM_REGION( 0x40000, "audiocpu", 0) /* uPD7807 code */
	ROM_LOAD( "s88b04.f20", 0x000000, 0x040000, CRC(c54b5a5e) SHA1(421082af349b170d74f5214d8b5eed44db472749) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "s88c13.e2",  0x000000, 0x80000, CRC(8103f007) SHA1(fcc51e57a1df572ad00000e03db1f5f949dea619) )
	ROM_LOAD( "s88c14.e1",  0x080000, 0x80000, CRC(a7b5fafb) SHA1(7a9448bf13ee680c99bc19a6fac7a14d6ce8ddfc) )
	ROM_LOAD( "s88c15.f2",  0x100000, 0x80000, CRC(597d833b) SHA1(d0c22981b105c1b4b37eae64964af8b683cc4687) )
	ROM_LOAD( "s88c16.f1",  0x180000, 0x80000, CRC(9ca1c1a1) SHA1(30bbb47503631295a16bdd77a5bc08681dd7c63e) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "s88c09.e5",  0x000000, 0x80000, CRC(8f77ac3e) SHA1(5903a167ddf69284035967876ef46d7d09582574) )
	ROM_LOAD( "s88c10.e4",  0x080000, 0x80000, CRC(cb44371e) SHA1(96ac9aa0034152ba88afc373475928ad6d9ecd29) )

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD( "s88c08.e6",  0x000000, 0x80000, CRC(a989cfc2) SHA1(11c39a2ddd5e4be150d2f2ce332e312907df2377) )

	ROM_REGION( 0x080000, "gfx4", 0 )
	ROM_LOAD( "s88c05.e7",  0x000000, 0x80000, CRC(e7f13340) SHA1(05b0f3ca369c95d4fd50cd9617fc044ad7bdf0d3) )

	ROM_REGION( 0x010000, "proms", 0 )  /* static palette */
	ROM_LOAD16_BYTE( "s88b18.f10", 0x00000, 0x8000, CRC(fa432edc) SHA1(55c01b6a1175539facdfdd0c3c49d878a59156a4) )
	ROM_LOAD16_BYTE( "s88b17.f9",  0x00001, 0x8000, CRC(7c55568e) SHA1(1e599cd00abe7b67bcb0c8d3f0c467a99ef79658) )

	ROM_REGION( 0x40000, "user1", 0 ) /* blitter data */
	ROM_LOAD( "s88b02.f19", 0x00000, 0x040000, CRC(7044a542) SHA1(8efaa512f62fe9a37d2474c435c549118c019d67) )

	ROM_REGION( 0x0100, "user2", 0 )
	ROM_LOAD( "s88a19.l5", 0x0000, 0x0100, CRC(c8ead41e) SHA1(d1e733691de9f9b71c9724de73086d36f381fc74) )   // priority (not used)
ROM_END



ROM_START( mjkojink )
	ROM_REGION( 0x01c000, "maincpu", 0 ) /* 6809 Code */
	ROM_LOAD( "x83j01.16e", 0x010000, 0xc000, CRC(91f90376) SHA1(d452f538f4a1b774640ced49f0ab2784b112e8ba) )
	ROM_CONTINUE(           0x00c000, 0x4000             )

	ROM_REGION( 0x40000, "audiocpu", 0) /* uPD7807 code */
	ROM_LOAD( "x83b02.9g",  0x00000, 0x40000, CRC(46a11578) SHA1(4ff7797808610b4bb0550be71acc49bbd8556fad) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "x83b14.1f",  0, 0x40000, CRC(2bcd7557) SHA1(1263a038a294fb521bdf315e8ac1198a42f68415) )
	ROM_LOAD32_BYTE( "x83b15.3f",  1, 0x40000, CRC(7d780e22) SHA1(f887f1a8e5fb65e560cb4987ae21a0c9100ebff0) )
	ROM_LOAD32_BYTE( "x83b16.4f",  2, 0x40000, CRC(5420a3f2) SHA1(0161eb2218d3e9cc018c3c2623de58855ab1a608) )
	ROM_LOAD32_BYTE( "x83b17.6f",  3, 0x40000, CRC(96bcdf83) SHA1(5568c15016a9cd68909aaf29a0bbe6659fdcefc9) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "x83b10.1c",  0, 0x40000, CRC(500bfeea) SHA1(cf60f6158611bfb360c7c97597a9d416530bec9e) )
	ROM_LOAD32_BYTE( "x83b11.3c",  1, 0x40000, CRC(2ef77717) SHA1(7d09dc3d73acaf910141598959fdc3501e17ddde) )
	ROM_LOAD32_BYTE( "x83b12.4c",  2, 0x40000, CRC(2035009d) SHA1(8a2aadcc49ac7e68dfabe5ea66b607459a89045d) )
	ROM_LOAD32_BYTE( "x83b13.6c",  3, 0x40000, CRC(53800df2) SHA1(5f18052a2d6afd27ff898ab597bb4e92fad238d7) )

	ROM_REGION( 0x010000, "proms", 0 )  /* static palette */
	ROM_LOAD16_BYTE( "x83a19.4k", 0x00000, 0x8000, CRC(d29c9ef0) SHA1(c4e2c0c3c9c7abee1a965d6842956cdf98a76ad4) )
	ROM_LOAD16_BYTE( "x83a18.3k", 0x00001, 0x8000, CRC(c3351952) SHA1(036be91ca428c5df016dd777bd658cb00f44ee3c) )

	ROM_REGION( 0x40000, "user1", 0 ) /* blitter data */
	ROM_LOAD( "x83b03.12e", 0x0000, 0x40000, CRC(4ba8b5ec) SHA1(cee77583f2f7b7fdba7e0f17e4d1244891488d36) )
ROM_END

ROM_START( vitaminc )
	ROM_REGION( 0x01c000, "maincpu", 0 ) /* 6809 Code */
	ROM_LOAD( "x90e01.bin", 0x010000, 0xc000, CRC(bc982525) SHA1(30f5e9ab27f799b895a3d979109e331603d94249) )
	ROM_CONTINUE(           0x00c000, 0x4000             )

	ROM_REGION( 0x40000, "audiocpu", 0) /* uPD7807 code */
	ROM_LOAD( "x90a02.bin", 0x00000, 0x40000, CRC(811f540a) SHA1(21993e99835a8995da28c24565b8e5dcc7aeb23e) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "x90a14.bin", 0, 0x40000, CRC(4b49d182) SHA1(2658175a23641cf6a4c989940ec2ce31b64ad53e) )
	ROM_LOAD32_BYTE( "x90a15.bin", 1, 0x40000, CRC(5e9016c2) SHA1(5255c1f05f1308ba6fe8b6655eb251a6e4a30a37) )
	ROM_LOAD32_BYTE( "x90a16.bin", 2, 0x40000, CRC(b8843000) SHA1(6d7b02334603f6abd64edcba8fd2f082839c4811) )
	ROM_LOAD32_BYTE( "x90a17.bin", 3, 0x40000, CRC(d74a843c) SHA1(0a0236390f47bb96bf39baaba9961130b26f70d7) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "x90a10.bin", 0, 0x40000, CRC(ee9fa36f) SHA1(c8864af92553f44180158dc1b34c9e1b81dfcd7f) )
	ROM_LOAD32_BYTE( "x90a11.bin", 1, 0x40000, CRC(b77d9ef4) SHA1(bb49b0dc28385c7052ff65ffc2c8446b37081965) )
	ROM_LOAD32_BYTE( "x90a12.bin", 2, 0x40000, CRC(da6a65d1) SHA1(3ab3a9e9c76dfc02579bf2fc390c23d24fe5a901) )
	ROM_LOAD32_BYTE( "x90a13.bin", 3, 0x40000, CRC(4da4553b) SHA1(da6fd2cfd5b027afe7e900f90128a25779acb15f) )

	ROM_REGION( 0x010000, "proms", 0 )  /* static palette */
	ROM_LOAD16_BYTE( "x90b19.bin", 0x00000, 0x8000, CRC(d0022cfb) SHA1(f384964a09fe6c1f8a993f5da67d81a23df22879) )
	ROM_LOAD16_BYTE( "x90b18.bin", 0x00001, 0x8000, CRC(fe1de95d) SHA1(e94282065b3c69de7c9ea214c752745c18c38cb7) )

	ROM_REGION( 0x40000, "user1", 0 ) /* blitter data */
	ROM_LOAD( "x90a03.bin", 0x0000, 0x40000, CRC(35d5b4e6) SHA1(1ea4d03946aad33d33a817f83d20e7ad8faace6d) )
ROM_END

ROM_START( mjyougo )
	ROM_REGION( 0x01c000, "maincpu", 0 ) /* 6809 Code */
	ROM_LOAD( "x91c01.bin", 0x010000, 0xc000, CRC(e28e8c21) SHA1(8039d764fb48269f0cab549c5a8861c05ecb1ef1) )
	ROM_CONTINUE(           0x00c000, 0x4000             )

	ROM_REGION( 0x40000, "audiocpu", 0) /* uPD7807 code */
	ROM_LOAD( "x91a02.bin", 0x00000, 0x40000, CRC(995b1399) SHA1(262f3d7ccffdaa578466d390d790f89186b3c993) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "x91a14.bin", 0, 0x40000, CRC(b165fbe8) SHA1(eaad2fb26aedd07c4ec0a8bf91cb10f32c76cb83) )
	ROM_LOAD32_BYTE( "x91a15.bin", 1, 0x40000, CRC(9b60bf2e) SHA1(13c1efbdc1f9f88f0d51dfd76c0128a490f63527) )
	ROM_LOAD32_BYTE( "x91a16.bin", 2, 0x40000, CRC(db4a1655) SHA1(f57c7faaf7632a13af4a7b18aa8f81a2042e4eb8) )
	ROM_LOAD32_BYTE( "x91a17.bin", 3, 0x40000, CRC(4f35ec3b) SHA1(5bf2d9eed69cc5e9358d95c0fba8b2b916274288) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "x91a10.bin", 0, 0x40000, CRC(cb364158) SHA1(5255756bfa1145b2773720a39e88fdb67f26e2e5) )
	ROM_LOAD32_BYTE( "x91a11.bin", 1, 0x40000, CRC(f3655577) SHA1(14eb0eda35015718f2e08da10d00ac8a381b4400) )
	ROM_LOAD32_BYTE( "x91a12.bin", 2, 0x40000, CRC(149e8f86) SHA1(660c05deabb0ce43b2f5b936c035df1a2b029fc6) )
	ROM_LOAD32_BYTE( "x91a13.bin", 3, 0x40000, CRC(59f7a140) SHA1(daad9433742b6292c8299935483f07f171436d17) )

	ROM_REGION( 0x010000, "proms", 0 )  /* static palette */
	ROM_LOAD16_BYTE( "x91a19.bin", 0x00000, 0x8000, CRC(f63493df) SHA1(0e436bffb03fa6218363ee205c4fe5f4e16a24cc) )
	ROM_LOAD16_BYTE( "x91a18.bin", 0x00001, 0x8000, CRC(b3541265) SHA1(d8e79c2856bb81e9ad9b79c012d64663a8997bbd) )

	ROM_REGION( 0x40000, "user1", 0 ) /* blitter data */
	ROM_LOAD( "x91a03.bin", 0x0000, 0x40000, CRC(4863caa2) SHA1(6ef511cb4d184d4705eb0fd3eb3d82daad49564a) )
ROM_END

ROM_START( mjkinjas )
	ROM_REGION( 0x01c000, "maincpu", 0 ) /* 6809 Code */
	ROM_LOAD( "x07c01.bin", 0x010000, 0xc000, CRC(e6534904) SHA1(59c092f0369fc893763ad4b96551e0b4c2430a6a) )
	ROM_CONTINUE(           0x00c000, 0x4000             )

	ROM_REGION( 0x40000, "audiocpu", 0) /* uPD7807 code */
	ROM_LOAD( "x07a02.bin", 0x00000, 0x40000, CRC(31396a5b) SHA1(c444f0a651da70c050a4c69bd09c31fc80dbf1de) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "x07a14.bin", 0, 0x80000, CRC(02829ede) SHA1(227d2bcbaafab19ab2c3ff9dad17a0792d114466) )
	ROM_LOAD32_BYTE( "x07a15.bin", 1, 0x80000, CRC(9c8b55db) SHA1(2142c0a6a6b89b2e07ac50be5c5af159610a1922) )
	ROM_LOAD32_BYTE( "x07a16.bin", 2, 0x80000, CRC(7898a340) SHA1(a04d71c5b964509d591659f8f09ea5f38080c0ae) )
	ROM_LOAD32_BYTE( "x07a17.bin", 3, 0x80000, CRC(bf1f6540) SHA1(94465a2dc20bc2b35d5518a485025b721ef87c49) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "x07a10.bin", 0, 0x80000, CRC(3bfab66e) SHA1(149d90d6fd1357b407bd9f2fe495ee28d55c9122) )
	ROM_LOAD32_BYTE( "x07a11.bin", 1, 0x80000, CRC(e8f610e3) SHA1(52970a56e67f6b36e1693c06207cbf14ebb96931) )
	ROM_LOAD32_BYTE( "x07a12.bin", 2, 0x80000, CRC(911f0972) SHA1(90b511725de3a226326ddc39106071230f3d3bc6) )
	ROM_LOAD32_BYTE( "x07a13.bin", 3, 0x80000, CRC(59be4c77) SHA1(1ba858c6b1e91753204e10738ca685a4df550d64) )

	ROM_REGION( 0x010000, "proms", 0 )  /* static palette */
	ROM_LOAD16_BYTE( "x07a19.bin", 0x00000, 0x8000, CRC(7acabdf8) SHA1(90e39c1dd3e32c057ab8bfcd82d022231a06847c) )
	ROM_LOAD16_BYTE( "x07a18.bin", 0x00001, 0x8000, CRC(d247bd5a) SHA1(298d62395ea7c687ed509863aaf3d8b758743cd8) )

	ROM_REGION( 0x80000, "user1", 0 ) /* blitter data */
	ROM_LOAD( "x07a03.bin", 0x0000, 0x80000, CRC(f5ff3e72) SHA1(e3489a3b8988677866aadd41cb99f146217022ce) )
ROM_END

ROM_START( jogakuen )
	ROM_REGION( 0x01c000, "maincpu", 0 ) /* 6809 Code */
	ROM_LOAD( "a01.bin",    0x010000, 0xc000, CRC(a189490a) SHA1(0d9f6389d4b16c3b885cdc8be20b19db25812aad) )
	ROM_CONTINUE(           0x00c000, 0x4000             )

	ROM_REGION( 0x40000, "audiocpu", 0) /* uPD7807 code */
	ROM_LOAD( "a02.bin",    0x00000, 0x40000, CRC(033add6c) SHA1(fc6b9333722228ba4270b1ba520e32e624b251c2) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "a14.bin",    0, 0x80000, CRC(27ad91d7) SHA1(6d96b030a7f1f8c90fb31582b72acbbc759d1e2f) )
	ROM_LOAD32_BYTE( "a15.bin",    1, 0x80000, CRC(e3b2753b) SHA1(c7923da2a718b70c4359d2726e8f8744f55e0389) )
	ROM_LOAD32_BYTE( "a16.bin",    2, 0x80000, CRC(6e2c61fc) SHA1(57b33614b4968d5f88741ff0a52c9a1f28a14da7) )
	ROM_LOAD32_BYTE( "a17.bin",    3, 0x80000, CRC(2f79d467) SHA1(b82df890cb12c33f6cc58676c02595be7614fd12) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "a10.bin",    0, 0x80000, CRC(a453759a) SHA1(62b7a41ea97bc808f3250505a9ffeaf53f1b9308) )
	ROM_LOAD32_BYTE( "a11.bin",    1, 0x80000, CRC(252cf007) SHA1(7b3dd558ca00f6855ac037f30e348265a9778f7d) )
	ROM_LOAD32_BYTE( "a12.bin",    2, 0x80000, CRC(5db85eb5) SHA1(01291b98676a8f5116739f66e517fe05f806514a) )
	ROM_LOAD32_BYTE( "a13.bin",    3, 0x80000, CRC(fe04d5b7) SHA1(965c095ce4f8f494d91370fe798be9214c2195f2) )

	ROM_REGION( 0x010000, "proms", 0 )  /* static palette */
	ROM_LOAD16_BYTE( "a19.bin",    0x00000, 0x8000, CRC(9a3d9d5e) SHA1(1ab20fc5b7ab1c2508c76b0051923446c409e074) )
	ROM_LOAD16_BYTE( "a18.bin",    0x00001, 0x8000, CRC(3289edd4) SHA1(7db4fbda8a22c64dc29ce6b4b63204cebd641351) )

	ROM_REGION( 0x40000, "user1", 0 ) /* blitter data */
	ROM_LOAD( "a03.bin",    0x0000, 0x40000, CRC(bb1507ab) SHA1(465f45c9cae2d4e064b784cc5ba25b60839e4b5f) )
ROM_END


ROM_START( lemnangl )
	ROM_REGION( 0x01c000, "maincpu", 0 ) /* 6809 Code */
	ROM_LOAD( "x02_d01.16e", 0x010000, 0xc000, CRC(4c2fae05) SHA1(86516399bd1eb1565b446dfa0f9a974bde6f9af2) )
	ROM_CONTINUE(            0x00c000, 0x4000             )

	ROM_REGION( 0x40000, "audiocpu", 0) /* uPD7807 code */
	ROM_LOAD( "x02a02.9g",  0x00000, 0x40000, CRC(e9aa8c80) SHA1(6db1345e20d53d8c69cebcac3b2a973fbcaa0e63) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "x02a14.1f",  0, 0x40000, CRC(4aa2397b) SHA1(5716de828e5e8735d5e6be0f7cfa95c52b37418a) )
	ROM_LOAD32_BYTE( "x02a15.3f",  1, 0x40000, CRC(d01986e2) SHA1(5889d771d5e13a12e650ee2e4885616373dcf828) )
	ROM_LOAD32_BYTE( "x02a16.4f",  2, 0x40000, CRC(16fca216) SHA1(e6715888053748a6da93dc087535da8e0b1ba702) )
	ROM_LOAD32_BYTE( "x02a17.6f",  3, 0x40000, CRC(7a6a96e7) SHA1(732e6f5ce0cac966aa8b6cfed3f39bb3ea30f57e) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "x02a10.1c",  0, 0x40000, CRC(e7164f57) SHA1(06f80ff72f12f2d821899c73051315bfa7b9bf7a) )
	ROM_LOAD32_BYTE( "x02a11.3c",  1, 0x40000, CRC(73fb5d3d) SHA1(99a26ae495e6768756c354330a6a859cfd1b9781) )
	ROM_LOAD32_BYTE( "x02a12.4c",  2, 0x40000, CRC(fc3a254a) SHA1(bf8172c00446b348b4432c71d92e8567ba50ab98) )
	ROM_LOAD32_BYTE( "x02a13.6c",  3, 0x40000, CRC(9f63e7e0) SHA1(4bd8077a3700ccbc4c4a358342b1f9fd4cba8f10) )

	ROM_REGION( 0x010000, "proms", 0 )  /* static palette */
	ROM_LOAD16_BYTE( "x02_b19.5k", 0x00000, 0x8000, CRC(f75959bc) SHA1(59a1debc28263a7f5f9b775817232fffc6e63ac4) )
	ROM_LOAD16_BYTE( "x02_b18.3k", 0x00001, 0x8000, CRC(3f1510b1) SHA1(a9892beb3d911ee91d7dd276d9c84d14ba3253d8) )

	ROM_REGION( 0x40000, "user1", 0 ) /* blitter data */
	ROM_LOAD( "x02a03.12e", 0x0000, 0x40000, CRC(02ef0378) SHA1(6223a019fc7794872dd49151952c56892295a779) )
ROM_END

ROM_START( mjikaga )
	ROM_REGION( 0x01c000, "maincpu", 0 ) /* 6809 Code */
	ROM_LOAD( "m15a01.bin", 0x010000, 0xc000, CRC(938cc4fb) SHA1(f979c6eee0b72bf53be8c7ebbc4e1dc05bd447d4) )
	ROM_CONTINUE(           0x00c000, 0x4000             )

	ROM_REGION( 0x40000, "audiocpu", 0) /* uPD7807 code */
	ROM_LOAD( "m15a02.bin", 0x00000, 0x40000, CRC(375933dd) SHA1(e813f02e53dc892714cd0e81301606600b72535c) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "m15a14.bin", 0, 0x40000, CRC(a685c452) SHA1(819486730cdd89572f5c6b6176e9694cf99a4264) )
	ROM_LOAD32_BYTE( "m15a15.bin", 1, 0x40000, CRC(44153914) SHA1(f8149c2e5412eb2a93dd16bfcb4f618aeddf3ff9) )
	ROM_LOAD32_BYTE( "m15a16.bin", 2, 0x40000, CRC(a4b0b8ac) SHA1(f6fd54c8c7ded3f7502c07b208e5f317669bc3f0) )
	ROM_LOAD32_BYTE( "m15a17.bin", 3, 0x40000, CRC(bb9cb2ef) SHA1(4df3eb57b46cb3ca25d938bb54205487b2c06b6d) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "m15a10.bin", 0, 0x40000, CRC(0aeed38e) SHA1(9becd3d5d32ca9678e9211480139c3a1beabb98d) )
	ROM_LOAD32_BYTE( "m15a11.bin", 1, 0x40000, CRC(a305e6e6) SHA1(a2c978050e70deb0244c660db800cba42eb3e2d0) )
	ROM_LOAD32_BYTE( "m15a12.bin", 2, 0x40000, CRC(946b3f55) SHA1(17451cbd7b0c828444aaf2ff170f3a7104596c1d) )
	ROM_LOAD32_BYTE( "m15a13.bin", 3, 0x40000, CRC(d9196955) SHA1(fa8fddbcd91b12d6cb918c0761ea478969a5a795) )

	ROM_REGION( 0x010000, "proms", 0 )  /* static palette */
	ROM_LOAD16_BYTE( "m15a19.bin", 0x00000, 0x8000, CRC(2f247acf) SHA1(468fee5a16c98751524e21ad0f608fc85c95ab86) )
	ROM_LOAD16_BYTE( "m15a18.bin", 0x00001, 0x8000, CRC(2648ca07) SHA1(e4f5ed62b014a6e397ce611c2c592f795d112219) )

	ROM_REGION( 0x40000, "user1", 0 ) /* blitter data */
	ROM_LOAD( "m15a03.bin", 0x0000, 0x40000, CRC(07e2e8f8) SHA1(61eed47c4136733059c1d96e98cadb8ac9078f95) )
ROM_END

ROM_START( mjprivat )
	ROM_REGION( 0x01c000, "maincpu", 0 ) /* 6809 Code */
	ROM_LOAD( "311d01.16e", 0x010000, 0xc000, CRC(3b4f4676) SHA1(e308febb9c8d35b495b83b37daff7c56deba78fb) )
	ROM_CONTINUE(           0x00c000, 0x4000             )

	ROM_REGION( 0x40000, "audiocpu", 0) /* uPD7807 code */
	ROM_LOAD( "311a02.9g", 0x00000, 0x40000, CRC(ae1d360a) SHA1(26a1e8d654ea2e4393a5267f0d9dab62149b4112) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "311a14.1f", 0, 0x40000, CRC(e8741ece) SHA1(ba380c287236668d8ed8583fb9735ed88d2eb41b) )
	ROM_LOAD32_BYTE( "311a15.3f", 1, 0x40000, CRC(51a5c226) SHA1(d527cc542a507fda6b55aec51e76a2dd019b9560) )
	ROM_LOAD32_BYTE( "311a16.4f", 2, 0x40000, CRC(f4419416) SHA1(0e8aa0ef51e449354dce3c8563c5eee230039dc6) )
	ROM_LOAD32_BYTE( "311a17.6f", 3, 0x40000, CRC(1edf5715) SHA1(717181ef596aac6fb556da75ac495f44b763840e) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "311a10.1c", 0, 0x40000, CRC(6b15a5fc) SHA1(db19c4c8fae31357bd72821a74b277b7f9abccb4) )
	ROM_LOAD32_BYTE( "311a11.3c", 1, 0x40000, CRC(b5faaf51) SHA1(7c6c72101d677d5f2e41ea6dc6490828f41d844c) )
	ROM_LOAD32_BYTE( "311a12.4c", 2, 0x40000, CRC(ab26bb73) SHA1(3645d3f6767b0bcb43ce2e3b2ef3bdca37d446cf) )
	ROM_LOAD32_BYTE( "311a13.6c", 3, 0x40000, CRC(f2f8f9a0) SHA1(d6b5dd68c85f9d506147f5ffeb60bdfad2bf0195) )

	ROM_REGION( 0x010000, "proms", 0 )  /* static palette */
	ROM_LOAD16_BYTE( "311a19.4k", 0x00000, 0x8000, CRC(dbb5569a) SHA1(e23a32667adc8eade7555adbac18dac268f08d3e) )
	ROM_LOAD16_BYTE( "311a18.3k", 0x00001, 0x8000, CRC(a5ca7723) SHA1(d074c5c09a50abdb2b0ffdeacfc077a91b9204bf) )

	ROM_REGION( 0x40000, "user1", 0 ) /* blitter data */
	ROM_LOAD( "311b03.12e", 0x0000, 0x40000, CRC(5722c341) SHA1(694e63261d91da48c0ed14a44fbc6c9c74b055d9) )
ROM_END

ROM_START( mirderby )
	ROM_REGION( 0x8000, "cpu0", 0 ) /* Z80 Code */
	ROM_LOAD( "x70_a11.1g", 0x2000, 0x6000, CRC(b394eef7) SHA1(a646596d09b90eda44aaf8ccbf8f3fccfd3d5dad) ) // first 0x6000 bytes are blank!
	ROM_CONTINUE(0x0000, 0x2000) // main z80 code is here

	ROM_REGION( 0x10000, "cpu1", 0 ) /* M6809 code */
	ROM_LOAD( "x70_c01.14e", 0x00000, 0x10000, CRC(d79d072d) SHA1(8e189931de9c4eb520c1ec2d0898d8eaba0f01b5) )

	ROM_REGION( 0x10000, "maincpu", 0 ) /* M6809 code */
	ROM_LOAD( "x70_b02.12e", 0x8000, 0x8000, CRC(76c9bb6f) SHA1(dd8893f3082d33d366247295e9531f8879c219c5) )

	ROM_REGION( 0x8000, "gfx1", 0 ) // horse gfx
	ROM_LOAD( "x70_a03.8g", 0x0000, 0x8000, CRC(4e298b2d) SHA1(ae78327d1f30c8d19ef772b82803dab4d6b7b919))

	ROM_REGION( 0x20000, "gfx2", 0 ) // fonts etc.
	ROM_LOAD( "x70_a04.5g", 0x0000, 0x20000, CRC(14392fdb) SHA1(dafdce473b2d2ebbdbf49fbd12f85c1ad69b2877) )

	ROM_REGION( 0x300, "proms", 0 ) /* colours */
	ROM_LOAD( "x70a07.8l", 0x000, 0x100, CRC(7d4c9712) SHA1(fe2a89841fdf5e4fd6cd41478ad2f29d28bed54d) )
	ROM_LOAD( "x70a08.7l", 0x100, 0x100, CRC(c4e77174) SHA1(ada238ded69f01b4daeb0159a2c5c422977bb95e) )
	ROM_LOAD( "x70a09.6l", 0x200, 0x100, CRC(d0187957) SHA1(6b36c1bccad24708cfa2fc78da08313f9bcfdbc0) )
ROM_END



DRIVER_INIT_MEMBER(homedata_state,jogakuen)
{
	/* it seems that Mahjong Jogakuen runs on the same board as the others,
	   but with just these two addresses swapped. Instead of creating a new
	   MachineDriver, I just fix them here. */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8007, 0x8007, write8_delegate(FUNC(homedata_state::pteacher_blitter_bank_w),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x8005, 0x8005, write8_delegate(FUNC(homedata_state::pteacher_gfx_bank_w),this));
}

DRIVER_INIT_MEMBER(homedata_state,mjikaga)
{
	/* Mahjong Ikagadesuka is different as well. */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x7802, 0x7802, read8_delegate(FUNC(homedata_state::pteacher_snd_r),this));
	m_audiocpu->space(AS_PROGRAM).install_write_handler(0x0123, 0x0123, write8_delegate(FUNC(homedata_state::pteacher_snd_answer_w),this));
}

DRIVER_INIT_MEMBER(homedata_state,reikaids)
{
	m_priority = 0;
}

DRIVER_INIT_MEMBER(homedata_state,battlcry)
{
	m_priority = 1; /* priority and initial value for bank write */
}

DRIVER_INIT_MEMBER(homedata_state,mirderby)
{
}


GAME( 1987, hourouki, 0, mrokumei, mjhokite, driver_device,  0,          ROT0, "Home Data", "Mahjong Hourouki Part 1 - Seisyun Hen (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1987, mhgaiden, 0, mrokumei, mjhokite, driver_device,  0,          ROT0, "Home Data", "Mahjong Hourouki Gaiden (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, mjhokite, 0, mrokumei, mjhokite, driver_device,  0,          ROT0, "Home Data", "Mahjong Hourouki Okite (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, mjclinic, 0, mrokumei, mjhokite, driver_device,  0,          ROT0, "Home Data", "Mahjong Clinic (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, mrokumei, 0, mrokumei, mjhokite, driver_device,  0,          ROT0, "Home Data", "Mahjong Rokumeikan (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1988, reikaids, 0, reikaids, reikaids, homedata_state, reikaids,   ROT0, "Home Data", "Reikai Doushi (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, battlcry, 0, reikaids, battlcry, homedata_state, battlcry,   ROT0, "Home Data", "Battlecry", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1989, mjkojink, 0, pteacher, pteacher, driver_device,  0,          ROT0, "Home Data", "Mahjong Kojinkyouju (Private Teacher) (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, vitaminc, 0, pteacher, pteacher, driver_device,  0,          ROT0, "Home Data", "Mahjong Vitamin C (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, mjyougo,  0, pteacher, pteacher, driver_device,  0,          ROT0, "Home Data", "Mahjong-yougo no Kisotairyoku (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, mjkinjas, 0, mjkinjas, pteacher, driver_device,  0,          ROT0, "Home Data", "Mahjong Kinjirareta Asobi (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1992?,jogakuen, 0, pteacher, jogakuen, homedata_state, jogakuen,   ROT0, "Windom",    "Mahjong Jogakuen (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, lemnangl, 0, lemnangl, lemnangl, driver_device,  0,          ROT0, "Home Data", "Mahjong Lemon Angel (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, mjprivat, 0, lemnangl, pteacher, driver_device,  0,          ROT0, "Matoba",    "Mahjong Private (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1991?,mjikaga,  0, lemnangl, mjikaga,  homedata_state, mjikaga,    ROT0, "Mitchell",  "Mahjong Ikaga Desu ka (Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1988, mirderby, 0, mirderby, mirderby, homedata_state, mirderby,   ROT0, "Home Data?", "Miracle Derby - Ascot", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
