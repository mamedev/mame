// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega vector hardware

    Games supported:
        * Space Fury
        * Zektor
        * Tac/Scan
        * Eliminator
        * Star Trek

    Known bugs:
        * games run too fast

****************************************************************************

    4/25/99 - Tac-Scan sound call for coins now works. (Jim Hernandez)
    2/5/98 - Added input ports support for Tac Scan. Bonus Ships now work.
             Zektor now uses it's own input port section. (Jim Hernandez)

    Sega Vector memory map (preliminary)

    Most of the info here comes from the wiretap archive at:
    http://www.mikesarcade.com/cgi-bin/spies.cgi?action=url&type=info&page=segaxyfaq1.6.txt

     * Sega G80 Vector Simulation

    ROM Address Map
    ---------------
           Eliminator Elim4Player Space Fury  Zektor  TAC/SCAN  Star Trk
    -----+-----------+-----------+-----------+-------+---------+---------+
    0000 | 969       | 1390      | 969       | 1611  | 1711    | 1873    | CPU u25
    -----+-----------+-----------+-----------+-------+---------+---------+
    0800 | 1333      | 1347      | 960       | 1586  | 1670    | 1848    | ROM u1
    -----+-----------+-----------+-----------+-------+---------+---------+
    1000 | 1334      | 1348      | 961       | 1587  | 1671    | 1849    | ROM u2
    -----+-----------+-----------+-----------+-------+---------+---------+
    1800 | 1335      | 1349      | 962       | 1588  | 1672    | 1850    | ROM u3
    -----+-----------+-----------+-----------+-------+---------+---------+
    2000 | 1336      | 1350      | 963       | 1589  | 1673    | 1851    | ROM u4
    -----+-----------+-----------+-----------+-------+---------+---------+
    2800 | 1337      | 1351      | 964       | 1590  | 1674    | 1852    | ROM u5
    -----+-----------+-----------+-----------+-------+---------+---------+
    3000 | 1338      | 1352      | 965       | 1591  | 1675    | 1853    | ROM u6
    -----+-----------+-----------+-----------+-------+---------+---------+
    3800 | 1339      | 1353      | 966       | 1592  | 1676    | 1854    | ROM u7
    -----+-----------+-----------+-----------+-------+---------+---------+
    4000 | 1340      | 1354      | 967       | 1593  | 1677    | 1855    | ROM u8
    -----+-----------+-----------+-----------+-------+---------+---------+
    4800 | 1341      | 1355      | 968       | 1594  | 1678    | 1856    | ROM u9
    -----+-----------+-----------+-----------+-------+---------+---------+
    5000 | 1342      | 1356      |           | 1595  | 1679    | 1857    | ROM u10
    -----+-----------+-----------+-----------+-------+---------+---------+
    5800 | 1343      | 1357      |           | 1596  | 1680    | 1858    | ROM u11
    -----+-----------+-----------+-----------+-------+---------+---------+
    6000 | 1344      | 1358      |           | 1597  | 1681    | 1859    | ROM u12
    -----+-----------+-----------+-----------+-------+---------+---------+
    6800 | 1345      | 1359      |           | 1598  | 1682    | 1860    | ROM u13
    -----+-----------+-----------+-----------+-------+---------+---------+
    7000 |           | 1360      |           | 1599  | 1683    | 1861    | ROM u14
    -----+-----------+-----------+-----------+-------+---------+---------+
    7800 |                                   | 1600  | 1684    | 1862    | ROM u15
    -----+-----------+-----------+-----------+-------+---------+---------+
    8000 |                                   | 1601  | 1685    | 1863    | ROM u16
    -----+-----------+-----------+-----------+-------+---------+---------+
    8800 |                                   | 1602  | 1686    | 1864    | ROM u17
    -----+-----------+-----------+-----------+-------+---------+---------+
    9000 |                                   | 1603  | 1687    | 1865    | ROM u18
    -----+-----------+-----------+-----------+-------+---------+---------+
    9800 |                                   | 1604  | 1688    | 1866    | ROM u19
    -----+-----------+-----------+-----------+-------+---------+---------+
    A000 |                                   | 1605  | 1709    | 1867    | ROM u20
    -----+-----------+-----------+-----------+-------+---------+---------+
    A800 |                                   | 1606  | 1710    | 1868    | ROM u21
    -----+-----------+-----------+-----------+-------+---------+---------+
    B000 |                                                     | 1869    | ROM u22
    -----+-----------+-----------+-----------+-------+---------+---------+
    B800 |                                                     | 1870    | ROM u23
    -----+-----------+-----------+-----------+-------+---------+---------+

    I/O ports:
    read:

    write:

    These games all have dipswitches, but they are mapped in such a way as to make
    using them with MAME extremely difficult. I might try to implement them in the
    future.

    SWITCH MAPPINGS
    ---------------

    +------+------+------+------+------+------+------+------+
    |SW1-8 |SW1-7 |SW1-6 |SW1-5 |SW1-4 |SW1-3 |SW1-2 |SW1-1 |
    +------+------+------+------+------+------+------+------+
     F8:08 |F9:08 |FA:08 |FB:08 |F8:04 |F9:04  FA:04  FB:04    Zektor &
           |      |      |      |      |      |                Space Fury
           |      |      |      |      |      |
       1  -|------|------|------|------|------|--------------- upright
       0  -|------|------|------|------|------|--------------- cocktail
           |      |      |      |      |      |
           |  1  -|------|------|------|------|--------------- voice
           |  0  -|------|------|------|------|--------------- no voice
                  |      |      |      |      |
                  |  1   |  1  -|------|------|--------------- 5 ships
                  |  0   |  1  -|------|------|--------------- 4 ships
                  |  1   |  0  -|------|------|--------------- 3 ships
                  |  0   |  0  -|------|------|--------------- 2 ships
                                |      |      |
                                |  1   |  1  -|--------------- hardest
                                |  0   |  1  -|--------------- hard
    1 = Open                    |  1   |  0  -|--------------- medium
    0 = Closed                  |  0   |  0  -|--------------- easy

    +------+------+------+------+------+------+------+------+
    |SW2-8 |SW2-7 |SW2-6 |SW2-5 |SW2-4 |SW2-3 |SW2-2 |SW2-1 |
    +------+------+------+------+------+------+------+------+
    |F8:02 |F9:02 |FA:02 |FB:02 |F8:01 |F9:01 |FA:01 |FB:01 |
    |      |      |      |      |      |      |      |      |
    |  1   |  1   |  0   |  0   |  1   | 1    | 0    |  0   | 1 coin/ 1 play
    +------+------+------+------+------+------+------+------+

    Known problems:

    1 The games seem to run too fast. This is most noticeable
      with the speech samples in Zektor - they don't match the mouth.
      Slowing down the Z80 doesn't help and in fact hurts performance.

    2 Cocktail mode isn't implemented.

    Is 1) still valid?

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/samples.h"
#include "machine/segag80.h"
#include "includes/segag80v.h"


/*************************************
 *
 *  Constants
 *
 *************************************/

#define CPU_CLOCK           8000000
#define VIDEO_CLOCK         15468480



/*************************************
 *
 *  Machine setup and config
 *
 *************************************/

INPUT_CHANGED_MEMBER(segag80v_state::service_switch)
{
	/* pressing the service switch sends an NMI */
	if (newval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


void segag80v_state::machine_start()
{
	/* register for save states */
	save_item(NAME(m_mult_data));
	save_item(NAME(m_mult_result));
	save_item(NAME(m_spinner_select));
	save_item(NAME(m_spinner_sign));
	save_item(NAME(m_spinner_count));
}



/*************************************
 *
 *  RAM writes/decryption
 *
 *************************************/

offs_t segag80v_state::decrypt_offset(address_space &space, offs_t offset)
{
	/* ignore anything but accesses via opcode $32 (LD $(XXYY),A) */
	offs_t pc = space.device().safe_pcbase();
	if ((UINT16)pc == 0xffff || space.read_byte(pc) != 0x32)
		return offset;

	/* fetch the low byte of the address and munge it */
	return (offset & 0xff00) | (*m_decrypt)(pc, space.read_byte(pc + 1));
}

WRITE8_MEMBER(segag80v_state::mainram_w)
{
	m_mainram[decrypt_offset(space, offset)] = data;
}

WRITE8_MEMBER(segag80v_state::usb_ram_w){ m_usb->ram_w(space, decrypt_offset(m_maincpu->space(AS_PROGRAM), offset), data); }
WRITE8_MEMBER(segag80v_state::vectorram_w)
{
	m_vectorram[decrypt_offset(space, offset)] = data;
}



/*************************************
 *
 *  Input port access
 *
 *************************************/

inline UINT8 segag80v_state::demangle(UINT8 d7d6, UINT8 d5d4, UINT8 d3d2, UINT8 d1d0)
{
	return ((d7d6 << 7) & 0x80) | ((d7d6 << 2) & 0x40) |
			((d5d4 << 5) & 0x20) | ((d5d4 << 0) & 0x10) |
			((d3d2 << 3) & 0x08) | ((d3d2 >> 2) & 0x04) |
			((d1d0 << 1) & 0x02) | ((d1d0 >> 4) & 0x01);
}


READ8_MEMBER(segag80v_state::mangled_ports_r)
{
	/* The input ports are odd. Neighboring lines are read via a mux chip  */
	/* one bit at a time. This means that one bank of DIP switches will be */
	/* read as two bits from each of 4 ports. For this reason, the input   */
	/* ports have been organized logically, and are demangled at runtime.  */
	/* 4 input ports each provide 8 bits of information. */
	UINT8 d7d6 = ioport("D7D6")->read();
	UINT8 d5d4 = ioport("D5D4")->read();
	UINT8 d3d2 = ioport("D3D2")->read();
	UINT8 d1d0 = ioport("D1D0")->read();
	int shift = offset & 3;
	return demangle(d7d6 >> shift, d5d4 >> shift, d3d2 >> shift, d1d0 >> shift);
}



/*************************************
 *
 *  Spinner control emulation
 *
 *************************************/

WRITE8_MEMBER(segag80v_state::spinner_select_w)
{
	m_spinner_select = data;
}


READ8_MEMBER(segag80v_state::spinner_input_r)
{
	INT8 delta;

	if (m_spinner_select & 1)
		return ioport("FC")->read();

/*
 * The values returned are always increasing.  That is, regardless of whether
 * you turn the spinner left or right, the self-test should always show the
 * number as increasing. The direction is only reflected in the least
 * significant bit.
 */

	/* I'm sure this can be further simplified ;-) BW */
	delta = ioport("SPINNER")->read();
	if (delta != 0)
	{
		m_spinner_sign = (delta >> 7) & 1;
		m_spinner_count += abs(delta);
	}
	return ~((m_spinner_count << 1) | m_spinner_sign);
}



/*************************************
 *
 *  Eliminator 4-player controls
 *
 *************************************/

CUSTOM_INPUT_MEMBER(segag80v_state::elim4_joint_coin_r)
{
	return (ioport("COINS")->read() & 0xf) != 0xf;
}


READ8_MEMBER(segag80v_state::elim4_input_r)
{
	UINT8 result = 0;

	/* bit 3 enables demux */
	if (m_spinner_select & 8)
	{
		/* Demux bit 0-2. Only 6 and 7 are connected */
		switch (m_spinner_select & 7)
		{
			case 6:
				/* player 3 & 4 controls */
				result = ioport("FC")->read();
				break;
			case 7:
				/* the 4 coin inputs */
				result = ioport("COINS")->read();
				break;
		}
	}

	/* LS240 has inverting outputs */
	return (result ^ 0xff);
}



/*************************************
 *
 *  Multiplier
 *
 *************************************/

WRITE8_MEMBER(segag80v_state::multiply_w)
{
	m_mult_data[offset] = data;
	if (offset == 1)
		m_mult_result = m_mult_data[0] * m_mult_data[1];
}


READ8_MEMBER(segag80v_state::multiply_r)
{
	UINT8 result = m_mult_result;
	m_mult_result >>= 8;
	return result;
}



/*************************************
 *
 *  Misc other I/O
 *
 *************************************/

WRITE8_MEMBER(segag80v_state::coin_count_w)
{
	coin_counter_w(machine(), 0, (data >> 7) & 1);
	coin_counter_w(machine(), 1, (data >> 6) & 1);
}


WRITE8_MEMBER(segag80v_state::unknown_w)
{
	/* writing an 0x04 here enables interrupts */
	/* some games write 0x00/0x01 here as well */
	if (data != 0x00 && data != 0x01 && data != 0x04)
		osd_printf_debug("%04X:unknown_w = %02X\n", space.device().safe_pc(), data);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

/* complete memory map derived from schematics */
static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, segag80v_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM     /* CPU board ROM */
	AM_RANGE(0x0800, 0xbfff) AM_ROM     /* PROM board ROM area */
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(mainram_w) AM_SHARE("mainram")
	AM_RANGE(0xe000, 0xefff) AM_RAM_WRITE(vectorram_w) AM_SHARE("vectorram")
ADDRESS_MAP_END


/* complete memory map derived from schematics */
static ADDRESS_MAP_START( main_portmap, AS_IO, 8, segag80v_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xbc, 0xbc) /* AM_READ ??? */
	AM_RANGE(0xbd, 0xbe) AM_WRITE(multiply_w)
	AM_RANGE(0xbe, 0xbe) AM_READ(multiply_r)
	AM_RANGE(0xbf, 0xbf) AM_WRITE(unknown_w)

	AM_RANGE(0xf9, 0xf9) AM_MIRROR(0x04) AM_WRITE(coin_count_w)
	AM_RANGE(0xf8, 0xfb) AM_READ(mangled_ports_r)
	AM_RANGE(0xfc, 0xfc) AM_READ_PORT("FC")
ADDRESS_MAP_END



/*************************************
 *
 *  Generic Port definitions
 *
 *************************************/

static INPUT_PORTS_START( g80v_generic )
	PORT_START("D7D6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)  /* P1.5 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )                 /* n/c */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )                 /* n/c */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )                 /* n/c */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(3)  /* P1.8 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.13 */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.14 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )                 /* n/c */

	PORT_START("D5D4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )               /* P1.10 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )                 /* P1.15 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.16 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.17 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.18 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )                 /* P1.19 */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.20 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.21 */

	PORT_START("D3D2")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW1:8" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x01, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x02, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x04, "SW1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x08, "SW1:1" )

	PORT_START("D1D0")
	PORT_DIPNAME( 0x0f, 0x03, DEF_STR( Coin_A )) PORT_DIPLOCATION("SW2:8,7,6,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x09, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x0a, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x0b, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x0c, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x0d, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x0f, "1 Coin/2 Credits 4/9" )
	PORT_DIPSETTING(    0x0e, "1 Coin/2 Credits 5/11" )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ))
	PORT_DIPNAME( 0xf0, 0x30, DEF_STR( Coin_B )) PORT_DIPLOCATION("SW2:4,3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x90, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0xa0, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0xb0, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0xc0, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0xd0, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0xf0, "1 Coin/2 Credits 4/9" )
	PORT_DIPSETTING(    0xe0, "1 Coin/2 Credits 5/11" )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_6C ))

	PORT_START("FC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )                /* P1.23 */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )                /* P1.24 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )                /* P1.25 */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )                /* P1.26 */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )                /* P1.27 */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )                /* P1.28 */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )                /* P1.29 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )                /* P1.30 */

	PORT_START("SERVICESW")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_HIGH ) PORT_CHANGED_MEMBER(DEVICE_SELF, segag80v_state,service_switch, 0)
INPUT_PORTS_END



/*************************************
 *
 *  Specific Port definitions
 *
 *************************************/

static INPUT_PORTS_START( elim2 )
	PORT_INCLUDE( g80v_generic )

	PORT_MODIFY("D7D6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )

	PORT_MODIFY("D5D4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)

	PORT_MODIFY("D3D2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ))       PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ))       // This switch is not documented in the manual
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))
	//"SW1:7" unused                                    // Unused according to manual
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ))         PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ))    PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ))          // This option is not documented in the manual
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ))
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ))
	PORT_DIPSETTING(    0x30, DEF_STR( Hardest ))
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Bonus_Life ))    PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0xc0, DEF_STR( None ))
	PORT_DIPSETTING(    0x80, "10000" )
	PORT_DIPSETTING(    0x40, "20000" )
	PORT_DIPSETTING(    0x00, "30000" )

	PORT_MODIFY("FC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( elim2c )
	PORT_INCLUDE( elim2 )

	PORT_MODIFY("D7D6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("D5D4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_MODIFY("FC")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( elim4 )
	PORT_INCLUDE( g80v_generic )

	PORT_MODIFY("D7D6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, segag80v_state,elim4_joint_coin_r, NULL)   /* combination of all four coin inputs */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_MODIFY("D5D4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )

	PORT_MODIFY("D3D2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ))       PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ))       // This switch is not documented in the manual
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))
	//"SW1:7" unused                                    // This switch is not documented in the manual
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ))         PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ))    PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ))          // This option is not documented in the manual
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ))
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ))
	PORT_DIPSETTING(    0x30, DEF_STR( Hardest ))
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Bonus_Life ))    PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0xc0, DEF_STR( None ))
	PORT_DIPSETTING(    0x80, "10000" )
	PORT_DIPSETTING(    0x40, "20000" )
	PORT_DIPSETTING(    0x00, "30000" )

	PORT_MODIFY("D1D0")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:8" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:4" )
	PORT_DIPNAME( 0xe0, 0x00, DEF_STR( Coin_A ))        PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ))
	PORT_DIPSETTING(    0xc0, DEF_STR( 7C_1C ))
	PORT_DIPSETTING(    0xa0, DEF_STR( 6C_1C ))
	PORT_DIPSETTING(    0x80, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(    0x60, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))

	PORT_MODIFY("FC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(1)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( spacfury )
	PORT_INCLUDE( g80v_generic )

	PORT_MODIFY("D5D4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_MODIFY("D3D2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ))       PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ))   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ))         PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ))    PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ))
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ))
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ))
	PORT_DIPSETTING(    0x30, DEF_STR( Hardest ))
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Bonus_Life ))    PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x40, "20000" )
	PORT_DIPSETTING(    0x80, "30000" )
	PORT_DIPSETTING(    0xc0, "40000" )

	PORT_MODIFY("FC")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( zektor )
	PORT_INCLUDE( g80v_generic )

	PORT_MODIFY("D3D2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ))       PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ))   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ))         PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ))    PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ))
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ))
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ))
	PORT_DIPSETTING(    0x30, DEF_STR( Hardest ))
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Bonus_Life ))    PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( None ))          // These switches are not documented in the manual
	PORT_DIPSETTING(    0xc0, "10000" )
	PORT_DIPSETTING(    0x80, "20000" )
	PORT_DIPSETTING(    0x40, "30000" )

	PORT_MODIFY("D5D4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.15 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.19 */

	PORT_MODIFY("FC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 )

	PORT_START("SPINNER")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(3) PORT_RESET
INPUT_PORTS_END


static INPUT_PORTS_START( tacscan )
	PORT_INCLUDE( g80v_generic )

	PORT_MODIFY("D3D2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ))       PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ))   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))           // This switch isn't documented in the manual
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x0c, 0x0c, "Number of Ships" )       PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "6" )
	PORT_DIPSETTING(    0x0c, "8" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ))    PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ))
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ))
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ))          // This option isn't documented in the manual
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Hard ))
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Bonus_Life ))    PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( None ))
	PORT_DIPSETTING(    0xc0, "10000" )
	PORT_DIPSETTING(    0x80, "20000" )
	PORT_DIPSETTING(    0x40, "30000" )

	PORT_MODIFY("D5D4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.15 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.19 */

	PORT_MODIFY("FC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 )

	PORT_START("SPINNER")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_RESET
INPUT_PORTS_END


static INPUT_PORTS_START( startrek )
	PORT_INCLUDE( g80v_generic )

	PORT_MODIFY("D3D2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ))       PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ))       // This switch isn't documented in the manual
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ))   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x0c, 0x0c, "Photon Torpedoes" )      PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ))    PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ))
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ))
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ))
	PORT_DIPSETTING(    0x30, "Tournament" )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Bonus_Life ))    PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x40, "20000" )
	PORT_DIPSETTING(    0x80, "30000" )
	PORT_DIPSETTING(    0xc0, "40000" )

	PORT_MODIFY("D5D4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.15 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )                 /* P1.19 */


	PORT_MODIFY("FC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 )

	PORT_START("SPINNER")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_RESET
INPUT_PORTS_END



/*************************************
 *
 *  Eliminator sound interfaces
 *
 *************************************/

static const char *const elim_sample_names[] =
{
	"*elim2",
	"elim1",
	"elim2",
	"elim3",
	"elim4",
	"elim5",
	"elim6",
	"elim7",
	"elim8",
	"elim9",
	"elim10",
	"elim11",
	"elim12",
	nullptr   /* end of array */
};


/*************************************
 *
 *  Space Fury sound interfaces
 *
 *************************************/

static const char *const spacfury_sample_names[] =
{
	"*spacfury",
	/* Sound samples */
	"sfury1",
	"sfury2",
	"sfury3",
	"sfury4",
	"sfury5",
	"sfury6",
	"sfury7",
	"sfury8",
	"sfury9",
	"sfury10",
	nullptr   /* end of array */
};

/*************************************
 *
 *  Zektor sound interfaces
 *
 *************************************/

static const char *const zektor_sample_names[] =
{
	"*zektor",
	"elim1",  /*  0 fireball */
	"elim2",  /*  1 bounce */
	"elim3",  /*  2 Skitter */
	"elim4",  /*  3 Eliminator */
	"elim5",  /*  4 Electron */
	"elim6",  /*  5 fire */
	"elim7",  /*  6 thrust */
	"elim8",  /*  7 Electron */
	"elim9",  /*  8 small explosion */
	"elim10", /*  9 med explosion */
	"elim11", /* 10 big explosion */
	nullptr
};


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( g80v_base, segag80v_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CPU_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", segag80v_state,  irq0_line_hold)


	/* video hardware */

	MCFG_SCREEN_ADD("screen", VECTOR)
	MCFG_SCREEN_REFRESH_RATE(40)
	MCFG_SCREEN_SIZE(400, 300)
	MCFG_SCREEN_VISIBLE_AREA(512, 1536, 640-32, 1408+32)
	MCFG_SCREEN_UPDATE_DRIVER(segag80v_state, screen_update_segag80v)

	MCFG_VECTOR_ADD("vector")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( elim2, g80v_base )

	/* custom sound board */
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(8)
	MCFG_SAMPLES_NAMES(elim_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( spacfury, g80v_base )

	/* custom sound board */
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(8)
	MCFG_SAMPLES_NAMES(spacfury_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	/* speech board */
	MCFG_FRAGMENT_ADD(sega_speech_board)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( zektor, g80v_base )

	/* custom sound board */
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(8)
	MCFG_SAMPLES_NAMES(zektor_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_SOUND_ADD("aysnd", AY8910, CPU_CLOCK/2/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)

	/* speech board */
	MCFG_FRAGMENT_ADD(sega_speech_board)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( tacscan, g80v_base )

	/* universal sound board */
	MCFG_SEGAUSB_ADD("usbsnd")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( startrek, g80v_base )

	/* speech board */
	MCFG_FRAGMENT_ADD(sega_speech_board)

	/* universal sound board */
	MCFG_SEGAUSB_ADD("usbsnd")
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( elim2 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "969.cpu-u25",   0x0000, 0x0800, CRC(411207f2) SHA1(2a082be4052b5d8f365abd0a51ea805d270d1189) )
	ROM_LOAD( "1333.prom-u1",  0x0800, 0x0800, CRC(fd2a2916) SHA1(431d340c0c9257d66f5851a591861bcefb600cec) )
	ROM_LOAD( "1334.prom-u2",  0x1000, 0x0800, CRC(79eb5548) SHA1(d951de5c0ab94fdb6e58207ee9a147674dd74220) )
	ROM_LOAD( "1335.prom-u3",  0x1800, 0x0800, CRC(3944972e) SHA1(59c84cf23898adb7e434c5802dbb821c79099890) )
	ROM_LOAD( "1336.prom-u4",  0x2000, 0x0800, CRC(852f7b4d) SHA1(6db45b9d11374f4cadf185aec81f33c0040bc001) )
	ROM_LOAD( "1337.prom-u5",  0x2800, 0x0800, CRC(cf932b08) SHA1(f0b61ca8266fd3de7522244c9b1587eecd24a4f1) )
	ROM_LOAD( "1338.prom-u6",  0x3000, 0x0800, CRC(99a3f3c9) SHA1(aa7d4805c70311ebe24ff70fcc32c0e2a7c4488a) )
	ROM_LOAD( "1339.prom-u7",  0x3800, 0x0800, CRC(d35f0fa3) SHA1(752f14b298604a9b91e94cd6d5d291ef33f27ec0) )
	ROM_LOAD( "1340.prom-u8",  0x4000, 0x0800, CRC(8fd4da21) SHA1(f30627dd1fbcc12bb587742a9072bbf38ba48401) )
	ROM_LOAD( "1341.prom-u9",  0x4800, 0x0800, CRC(629c9a28) SHA1(cb7df14ea1bb2d7997bfae1ca70b47763c73298a) )
	ROM_LOAD( "1342.prom-u10", 0x5000, 0x0800, CRC(643df651) SHA1(80c5da44b5d2a7d97c7ba0067f773eb645a9d432) )
	ROM_LOAD( "1343.prom-u11", 0x5800, 0x0800, CRC(d29d70d2) SHA1(ee2cd752b99ebd522eccf5e71d02c31479acfdf5) )
	ROM_LOAD( "1344.prom-u12", 0x6000, 0x0800, CRC(c5e153a3) SHA1(7e805573aeed01e3d4ed477870800dd7ecad7a1b) )
	ROM_LOAD( "1345.prom-u13", 0x6800, 0x0800, CRC(40597a92) SHA1(ee1ae2b424c38b40d2cbeda4aba3328e6d3f9c81) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "s-c.xyt-u39",   0x0000, 0x0400, CRC(56484d19) SHA1(61f43126fdcfc230638ed47085ae037a098e6781) )   /* sine table */
	ROM_LOAD( "pr-82.cpu-u15", 0x0400, 0x0020, CRC(c609b79e) SHA1(49dbcbb607079a182d7eb396c0da097166ea91c9) )   /* CPU board addressing */
ROM_END

ROM_START( elim2a )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "969.cpu-u25",    0x0000, 0x0800, CRC(411207f2) SHA1(2a082be4052b5d8f365abd0a51ea805d270d1189) )
	ROM_LOAD( "1158.prom-u1",   0x0800, 0x0800, CRC(a40ac3a5) SHA1(9cf707e3439def17390ae16b49552fb1996a6335) )
	ROM_LOAD( "1159.prom-u2",   0x1000, 0x0800, CRC(ff100604) SHA1(1636337c702473b5a567832a622b0c09bd1e2aba) )
	ROM_LOAD( "1160a.prom-u3",  0x1800, 0x0800, CRC(ebfe33bd) SHA1(226da36becd278d34030f564fef61851819d2324) )
	ROM_LOAD( "1161a.prom-u4",  0x2000, 0x0800, CRC(03d41db3) SHA1(da9e618314c01b56b9d66abd14f1e5bf928fff54) )
	ROM_LOAD( "1162a.prom-u5",  0x2800, 0x0800, CRC(f2c7ece3) SHA1(86a9099ce97439cd849dc32ed2c164a1be14e4e7) )
	ROM_LOAD( "1163a.prom-u6",  0x3000, 0x0800, CRC(1fc58b00) SHA1(732c57781cd45cd301b2337b6879ff811d9692f3) )
	ROM_LOAD( "1164a.prom-u7",  0x3800, 0x0800, CRC(f37480d1) SHA1(3d7fac05d60083ddcd51c0190078c89a39f79a91) )
	ROM_LOAD( "1165a.prom-u8",  0x4000, 0x0800, CRC(328819f8) SHA1(ed5e3488399b4481e69f623404a28515524af60a) )
	ROM_LOAD( "1166a.prom-u9",  0x4800, 0x0800, CRC(1b8e8380) SHA1(d56ccc4fac9c8149ebef4939ba401372d69bf022) )
	ROM_LOAD( "1167a.prom-u10", 0x5000, 0x0800, CRC(16aa3156) SHA1(652a547ff1cb4ede507418b392e28f30a3cc179c) )
	ROM_LOAD( "1168a.prom-u11", 0x5800, 0x0800, CRC(3c7c893a) SHA1(73d2835833a6d40f6a9b0a87364af48a449d9674) )
	ROM_LOAD( "1169a.prom-u12", 0x6000, 0x0800, CRC(5cee23b1) SHA1(66f6fc6163148608296e3d25abb194559a2f5179) )
	ROM_LOAD( "1170a.prom-u13", 0x6800, 0x0800, CRC(8cdacd35) SHA1(f24f8a74cb4b8452ddbd42e61d3b0366bbee7f98) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "s-c.xyt-u39",    0x0000, 0x0400, CRC(56484d19) SHA1(61f43126fdcfc230638ed47085ae037a098e6781) )  /* sine table */
	ROM_LOAD( "pr-82.cpu-u15",  0x0400, 0x0020, CRC(c609b79e) SHA1(49dbcbb607079a182d7eb396c0da097166ea91c9) )  /* CPU board addressing */
ROM_END

ROM_START( elim2c )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "969t.cpu-u25",  0x0000, 0x0800, CRC(896a615c) SHA1(542386196eca9fd822e36508e173201ee8a962ed) )
	ROM_LOAD( "1200.prom-u1",  0x0800, 0x0800, CRC(590beb6a) SHA1(307c33cbc0b90f290aac302366e3ce4f70e5265e) )
	ROM_LOAD( "1201.prom-u2",  0x1000, 0x0800, CRC(fed32b30) SHA1(51fba99d3bf543318ebe70ee1aa91e3171767d6f) )
	ROM_LOAD( "1202.prom-u3",  0x1800, 0x0800, CRC(0a2068d0) SHA1(90acf1e78f5c3266d1fbc31470ad4d6a8cb43fe8) )
	ROM_LOAD( "1203.prom-u4",  0x2000, 0x0800, CRC(1f593aa2) SHA1(aaad927174fa806d2c602b5672b1396eb9ec50fa) )
	ROM_LOAD( "1204.prom-u5",  0x2800, 0x0800, CRC(046f1030) SHA1(632ac37b84007f169ce72877d8089538413ba20b) )
	ROM_LOAD( "1205.prom-u6",  0x3000, 0x0800, CRC(8d10b870) SHA1(cc91a06c6b0e1697c399700bc351384360ecd5a3) )
	ROM_LOAD( "1206.prom-u7",  0x3800, 0x0800, CRC(7f6c5afa) SHA1(0e684c654cfe2365e7d21e7bccb25f1ddb883770) )
	ROM_LOAD( "1207.prom-u8",  0x4000, 0x0800, CRC(6cc74d62) SHA1(3392e5cd177885be7391a2699164f39302554d26) )
	ROM_LOAD( "1208.prom-u9",  0x4800, 0x0800, CRC(cc37a631) SHA1(084ecc6b0179fe4f984131d057d5de5382443911) )
	ROM_LOAD( "1209.prom-u10", 0x5000, 0x0800, CRC(844922f8) SHA1(0ad201fce2eaa7dde77d8694d226aad8b9a46ea7) )
	ROM_LOAD( "1210.prom-u11", 0x5800, 0x0800, CRC(7b289783) SHA1(5326ca94b5197ef99db4ea3b28051090f0d7a9ce) )
	ROM_LOAD( "1211.prom-u12", 0x6000, 0x0800, CRC(17349db7) SHA1(8e7ee1fbf153a36a13f3252ca4c588df531b56ec) )
	ROM_LOAD( "1212.prom-u13", 0x6800, 0x0800, CRC(152cf376) SHA1(56c3141598b8bac81e85b1fc7052fdd19cd95609) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "s-c.xyt-u39",   0x0000, 0x0400, CRC(56484d19) SHA1(61f43126fdcfc230638ed47085ae037a098e6781) )   /* sine table */
	ROM_LOAD( "pr-82.cpu-u15", 0x0400, 0x0020, CRC(c609b79e) SHA1(49dbcbb607079a182d7eb396c0da097166ea91c9) )   /* CPU board addressing */
ROM_END

ROM_START( elim4 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "1390.cpu-u25",  0x0000, 0x0800, CRC(97010c3e) SHA1(b07db05abf48461b633bbabe359a973a5bc6da13) )
	ROM_LOAD( "1347.prom-u1",  0x0800, 0x0800, CRC(657d7320) SHA1(ef8a637d94dfa8b9dfa600269d914d635e597a9c) )
	ROM_LOAD( "1348.prom-u2",  0x1000, 0x0800, CRC(b15fe578) SHA1(d53773a5f7ec3c130d4ff75a5348a9f37c82c7c8) )
	ROM_LOAD( "1349.prom-u3",  0x1800, 0x0800, CRC(0702b586) SHA1(9847172872419c475d474ff09612c38b867e15af) )
	ROM_LOAD( "1350.prom-u4",  0x2000, 0x0800, CRC(4168dd3b) SHA1(1f26877c63cd7983dfa9a869e0442e8a213f382f) )
	ROM_LOAD( "1351.prom-u5",  0x2800, 0x0800, CRC(c950f24c) SHA1(497a9aa7b9d040a4ff7b3f938093edec2218120d) )
	ROM_LOAD( "1352.prom-u6",  0x3000, 0x0800, CRC(dc8c91cc) SHA1(c99224c7e57dfce9440771f78ce90ea576feed2a) )
	ROM_LOAD( "1353.prom-u7",  0x3800, 0x0800, CRC(11eda631) SHA1(8ba926268762d491d28d5629d5a310b1accca47d) )
	ROM_LOAD( "1354.prom-u8",  0x4000, 0x0800, CRC(b9dd6e7a) SHA1(ab6780f0abe99a5ef76746d45384e80399c6d611) )
	ROM_LOAD( "1355.prom-u9",  0x4800, 0x0800, CRC(c92c7237) SHA1(18aad6618df51a1980775a3aaa4447205453a8af) )
	ROM_LOAD( "1356.prom-u10", 0x5000, 0x0800, CRC(889b98e3) SHA1(23661149e7ffbdbc2c95920d13e9b8b24f86cd9a) )
	ROM_LOAD( "1357.prom-u11", 0x5800, 0x0800, CRC(d79248a5) SHA1(e58062d5c4e5f6fe8d70dd9b55d46a57137c9a64) )
	ROM_LOAD( "1358.prom-u12", 0x6000, 0x0800, CRC(c5dabc77) SHA1(2dc59e627f40fefefc206f2e9d070a62606e44fc) )
	ROM_LOAD( "1359.prom-u13", 0x6800, 0x0800, CRC(24c8e5d8) SHA1(d0ae6e1dfd05d170c49837760369f04df4eaa14f) )
	ROM_LOAD( "1360.prom-u14", 0x7000, 0x0800, CRC(96d48238) SHA1(76a7b49081cd2d0dd1976077aa66b6d5ae5b2b43) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "s-c.xyt-u39",   0x0000, 0x0400, CRC(56484d19) SHA1(61f43126fdcfc230638ed47085ae037a098e6781) )   /* sine table */
	ROM_LOAD( "pr-82.cpu-u15", 0x0400, 0x0020, CRC(c609b79e) SHA1(49dbcbb607079a182d7eb396c0da097166ea91c9) )   /* CPU board addressing */
ROM_END

ROM_START( elim4p )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "1390.cpu-u25",  0x0000, 0x0800, CRC(97010c3e) SHA1(b07db05abf48461b633bbabe359a973a5bc6da13) )
	ROM_LOAD( "sw1.prom-u1",   0x0800, 0x0800, CRC(5350b8eb) SHA1(def9192971d1943e45cea1845b1d8c8e2a01bc38) )
	ROM_LOAD( "sw2.prom-u2",   0x1000, 0x0800, CRC(44f45465) SHA1(e3139878602864509803dabc0f9c278e4b856431) )
	ROM_LOAD( "sw3.prom-u3",   0x1800, 0x0800, CRC(5b692c3c) SHA1(6cd1361e9f063af1f175baed466cc2667b776a52) )
	ROM_LOAD( "sw4.prom-u4",   0x2000, 0x0800, CRC(0b78dd00) SHA1(f48c4bdd5fc2e818107b036aa6eddebf46a0e964) )
	ROM_LOAD( "sw5.prom-u5",   0x2800, 0x0800, CRC(8b3795f1) SHA1(1bcd12791e45dd14c7541e6fe3798a8159b6c11b) )
	ROM_LOAD( "sw6.prom-u6",   0x3000, 0x0800, CRC(4304b503) SHA1(2bc7a702d43092818ecb713fa0bac476c272e3a0) )
	ROM_LOAD( "sw7.prom-u7",   0x3800, 0x0800, CRC(3cb4a604) SHA1(868c3c1bead99c2e6857d1c2eef02d84e0e87f29) )
	ROM_LOAD( "sw8.prom-u8",   0x4000, 0x0800, CRC(bdc55223) SHA1(47ca7485c9e2878cbcb92d93a022f7d74a6d13df) )
	ROM_LOAD( "sw9.prom-u9",   0x4800, 0x0800, CRC(f6ca1bf1) SHA1(e4dc6bd6486dff2d0e8a93e5c7649093107cde46) )
	ROM_LOAD( "swa.prom-u10",  0x5000, 0x0800, CRC(12373f7f) SHA1(685c1202345ae8ef53fa61b7254ce04efd94a12b) )
	ROM_LOAD( "swb.prom-u11",  0x5800, 0x0800, CRC(d1effc6b) SHA1(b72cd14642f26ac50fbe6199d121b0278588ca22) )
	ROM_LOAD( "swc.prom-u12",  0x6000, 0x0800, CRC(bf361ab3) SHA1(23e3396dc937c0a19d0d312d1de3443b43807d91) )
	ROM_LOAD( "swd.prom-u13",  0x6800, 0x0800, CRC(ae2c88e5) SHA1(b0833051f543529502e05fb183effa9f817757fb) )
	ROM_LOAD( "swe.prom-u14",  0x7000, 0x0800, CRC(ec4cc343) SHA1(00e107eaf530ce6bec2afffd7d7bedd7763cfb17) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "s-c.xyt-u39",   0x0000, 0x0400, CRC(56484d19) SHA1(61f43126fdcfc230638ed47085ae037a098e6781) )   /* sine table */
	ROM_LOAD( "pr-82.cpu-u15", 0x0400, 0x0020, CRC(c609b79e) SHA1(49dbcbb607079a182d7eb396c0da097166ea91c9) )   /* CPU board addressing */
ROM_END


ROM_START( spacfury ) /* Revision C */
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "969c.cpu-u25",    0x0000, 0x0800, CRC(411207f2) SHA1(2a082be4052b5d8f365abd0a51ea805d270d1189) )
	ROM_LOAD( "960c.prom-u1",    0x0800, 0x0800, CRC(d071ab7e) SHA1(c7d2429e4fa77988d7ac62bc68f876ffb7467838) )
	ROM_LOAD( "961c.prom-u2",    0x1000, 0x0800, CRC(aebc7b97) SHA1(d0a0328ed34de9bd2c83da4ddc2d017e2b5a8bdc) )
	ROM_LOAD( "962c.prom-u3",    0x1800, 0x0800, CRC(dbbba35e) SHA1(0400d1ba09199d19f5b8aa5bb1a85ed27930822d) )
	ROM_LOAD( "963c.prom-u4",    0x2000, 0x0800, CRC(d9e9eadc) SHA1(1ad228d65dca48d084bbac358af80882886e7a40) )
	ROM_LOAD( "964c.prom-u5",    0x2800, 0x0800, CRC(7ed947b6) SHA1(c0fd7ed74a87cc422a42e2a4f9eb947f5d5d9fed) )
	ROM_LOAD( "965c.prom-u6",    0x3000, 0x0800, CRC(d2443a22) SHA1(45e5d43eae89e25370bb8e8db2b664642a238eb9) )
	ROM_LOAD( "966c.prom-u7",    0x3800, 0x0800, CRC(1985ccfc) SHA1(8c5931519b976c82a94d17279cc919b4baad5bb7) )
	ROM_LOAD( "967c.prom-u8",    0x4000, 0x0800, CRC(330f0751) SHA1(07ae52fdbfa2cc326f88dc76c3dc8e145b592863) )
	ROM_LOAD( "968c.prom-u9",    0x4800, 0x0800, CRC(8366eadb) SHA1(8e4cb30a730237da2e933370faf5eaa1a41cacbf) )

	ROM_REGION( 0x0800, "audiocpu", 0 )
	ROM_LOAD( "808c.speech-u7",  0x0000, 0x0800, CRC(b779884b) SHA1(ac07e99717a1f51b79f3e43a5d873ebfa0559320) )

	ROM_REGION( 0x4000, "speech", 0 )
	ROM_LOAD( "970c.speech-u6",  0x0000, 0x1000, CRC(979d8535) SHA1(1ed097e563319ca6d2b7df9875ce7ee921eae468) )
	ROM_LOAD( "971c.speech-u5",  0x1000, 0x1000, CRC(022dbd32) SHA1(4e0504b5ccc28094078912673c49571cf83804ab) )
	ROM_LOAD( "972c.speech-u4",  0x2000, 0x1000, CRC(fad9346d) SHA1(784e5ab0fb00235cfd733c502baf23960923504f) )

	ROM_REGION( 0x0440, "proms", 0 )
	ROM_LOAD( "s-c.xyt-u39",     0x0000, 0x0400, CRC(56484d19) SHA1(61f43126fdcfc230638ed47085ae037a098e6781) ) /* sine table */
	ROM_LOAD( "pr-82.cpu-u15",   0x0400, 0x0020, CRC(c609b79e) SHA1(49dbcbb607079a182d7eb396c0da097166ea91c9) ) /* CPU board addressing */
	ROM_LOAD( "6331.speech-u30", 0x0420, 0x0020, CRC(adcb81d0) SHA1(74b0efc7e8362b0c98e54a6107981cff656d87e1) ) /* speech board addressing */
ROM_END

ROM_START( spacfurya ) /* Revision A */
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "969a.cpu-u25",    0x0000, 0x0800, CRC(896a615c) SHA1(542386196eca9fd822e36508e173201ee8a962ed) )
	ROM_LOAD( "960a.prom-u1",    0x0800, 0x0800, CRC(e1ea7964) SHA1(9c84c525973fcf1437b062d98195272723249d02) )
	ROM_LOAD( "961a.prom-u2",    0x1000, 0x0800, CRC(cdb04233) SHA1(6f8d2fe6d46d04ebe94b7943006d63b24c88ed5a) )
	ROM_LOAD( "962a.prom-u3",    0x1800, 0x0800, CRC(5f03e632) SHA1(c6e8d72ba13ab05ec01a78502948a73c21e0fd69) )
	ROM_LOAD( "963a.prom-u4",    0x2000, 0x0800, CRC(45a77b44) SHA1(91f4822b89ec9c16c67c781a11fabfa4b9914660) )
	ROM_LOAD( "964a.prom-u5",    0x2800, 0x0800, CRC(ba008f8b) SHA1(24f5bef240ae2bcfd5b1d95f51b3599f79518b56) )
	ROM_LOAD( "965a.prom-u6",    0x3000, 0x0800, CRC(78677d31) SHA1(ed5810aa4bddbfe36a6ff9992dd0cb58cce66836) )
	ROM_LOAD( "966a.prom-u7",    0x3800, 0x0800, CRC(a8a51105) SHA1(f5e0fa662552f50fa6905f579d4c678b790ffa96) )
	ROM_LOAD( "967a.prom-u8",    0x4000, 0x0800, CRC(d60f667d) SHA1(821271ec1918e22ed29a5b1f4b0182765ef5ba10) )
	ROM_LOAD( "968a.prom-u9",    0x4800, 0x0800, CRC(aea85b6a) SHA1(8778ff0be34cd4fd5b8f6f76c64bfca68d4d240e) )

	ROM_REGION( 0x0800, "audiocpu", 0 )
	ROM_LOAD( "808a.speech-u7",  0x0000, 0x0800, CRC(5988c767) SHA1(3b91a8cd46aa7e714028cc40f700fea32287afb1) )

	ROM_REGION( 0x4000, "speech", 0 )
	ROM_LOAD( "970.speech-u6",   0x0000, 0x1000, CRC(f3b47b36) SHA1(6ae0b627349664140a7f70799645b368e452d69c) )
	ROM_LOAD( "971.speech-u5",   0x1000, 0x1000, CRC(e72bbe88) SHA1(efadf8aa448c289cf4d0cf1831255b9ac60820f2) )
	ROM_LOAD( "972.speech-u4",   0x2000, 0x1000, CRC(8b3da539) SHA1(3a0c4af96a2116fc668a340534582776b2018663) )

	ROM_REGION( 0x0440, "proms", 0 )
	ROM_LOAD( "s-c.xyt-u39",     0x0000, 0x0400, CRC(56484d19) SHA1(61f43126fdcfc230638ed47085ae037a098e6781) ) /* sine table */
	ROM_LOAD( "pr-82.cpu-u15",   0x0400, 0x0020, CRC(c609b79e) SHA1(49dbcbb607079a182d7eb396c0da097166ea91c9) ) /* CPU board addressing */
	ROM_LOAD( "6331.speech-u30", 0x0420, 0x0020, CRC(adcb81d0) SHA1(74b0efc7e8362b0c98e54a6107981cff656d87e1) ) /* speech board addressing */
ROM_END

ROM_START( spacfuryb ) /* Revision B */
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "969a.cpu-u25",    0x0000, 0x0800, CRC(896a615c) SHA1(542386196eca9fd822e36508e173201ee8a962ed) )
	ROM_LOAD( "960b.prom-u1",    0x0800, 0x0800, CRC(8a99b63f) SHA1(4b9ec152e0fad50afeea11f5d61331f3211da606) )
	ROM_LOAD( "961b.prom-u2",    0x1000, 0x0800, CRC(c72c1609) SHA1(b489423b52a15275b63f6b01b9aa75ad1ce777b5) )
	ROM_LOAD( "962b.prom-u3",    0x1800, 0x0800, CRC(7ffc338d) SHA1(2c37049657305c465e3a5301e0be9f1afc9333c0) )
	ROM_LOAD( "963b.prom-u4",    0x2000, 0x0800, CRC(4fe0bd88) SHA1(d1902c8b2c2577fb49894aeac4c9d6b8cf38f2f6) )
	ROM_LOAD( "964b.prom-u5",    0x2800, 0x0800, CRC(09b359db) SHA1(e1d6af48680dc0f34068ee6e916650dac738e280) )
	ROM_LOAD( "965b.prom-u6",    0x3000, 0x0800, CRC(7c1f9b71) SHA1(ebe29a558e7239b4f0bc49a1fe92e5f1903edce3) )
	ROM_LOAD( "966b.prom-u7",    0x3800, 0x0800, CRC(8933b852) SHA1(dabb219195a668893c82ccc80ed09989f7fd83af) )
	ROM_LOAD( "967b.prom-u8",    0x4000, 0x0800, CRC(82b5768d) SHA1(823d8c0a537bad62e8186f88f8d02a0f3dc6da0f) )
	ROM_LOAD( "968b.prom-u9",    0x4800, 0x0800, CRC(fea68f02) SHA1(83bef40dfaac014b7929239d81075335ff8fd506) )

	ROM_REGION( 0x0800, "audiocpu", 0 )
	ROM_LOAD( "808a.speech-u7",  0x0000, 0x0800, CRC(5988c767) SHA1(3b91a8cd46aa7e714028cc40f700fea32287afb1) )

	ROM_REGION( 0x4000, "speech", 0 )
	ROM_LOAD( "970.speech-u6",   0x0000, 0x1000, CRC(f3b47b36) SHA1(6ae0b627349664140a7f70799645b368e452d69c) )
	ROM_LOAD( "971.speech-u5",   0x1000, 0x1000, CRC(e72bbe88) SHA1(efadf8aa448c289cf4d0cf1831255b9ac60820f2) )
	ROM_LOAD( "972.speech-u4",   0x2000, 0x1000, CRC(8b3da539) SHA1(3a0c4af96a2116fc668a340534582776b2018663) )

	ROM_REGION( 0x0440, "proms", 0 )
	ROM_LOAD( "s-c.xyt-u39",     0x0000, 0x0400, CRC(56484d19) SHA1(61f43126fdcfc230638ed47085ae037a098e6781) ) /* sine table */
	ROM_LOAD( "pr-82.cpu-u15",   0x0400, 0x0020, CRC(c609b79e) SHA1(49dbcbb607079a182d7eb396c0da097166ea91c9) ) /* CPU board addressing */
	ROM_LOAD( "6331.speech-u30", 0x0420, 0x0020, CRC(adcb81d0) SHA1(74b0efc7e8362b0c98e54a6107981cff656d87e1) ) /* speech board addressing */
ROM_END


ROM_START( zektor )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "1611.cpu-u25",    0x0000, 0x0800, CRC(6245aa23) SHA1(815f3c7edad9c290b719a60964085e90e7268112) )
	ROM_LOAD( "1586.prom-u1",    0x0800, 0x0800, CRC(efeb4fb5) SHA1(b337179c01870c953b8d38c20263802e9a7936d3) )
	ROM_LOAD( "1587.prom-u2",    0x1000, 0x0800, CRC(daa6c25c) SHA1(061e390775b6dd24f85d51951267bca4339a3845) )
	ROM_LOAD( "1588.prom-u3",    0x1800, 0x0800, CRC(62b67dde) SHA1(831bad0f5a601d6859f69c70d0962c970d92db0e) )
	ROM_LOAD( "1589.prom-u4",    0x2000, 0x0800, CRC(c2db0ba4) SHA1(658773f2b56ea805d7d678e300f9bbc896fbf176) )
	ROM_LOAD( "1590.prom-u5",    0x2800, 0x0800, CRC(4d948414) SHA1(f60d295b0f8f798126dbfdc197943d8511238390) )
	ROM_LOAD( "1591.prom-u6",    0x3000, 0x0800, CRC(b0556a6c) SHA1(84b481cc60dc3df3a1cf18b1ece4c70bcc7bb5a1) )
	ROM_LOAD( "1592.prom-u7",    0x3800, 0x0800, CRC(750ecadf) SHA1(83ddd482230fbf6cf78a054fb4abd5bc8aec3ec8) )
	ROM_LOAD( "1593.prom-u8",    0x4000, 0x0800, CRC(34f8850f) SHA1(d93594e529aca8d847c9f1e9055f1840f6069fb2) )
	ROM_LOAD( "1594.prom-u9",    0x4800, 0x0800, CRC(52b22ab2) SHA1(c8f822a1a54081cfc88149c97b4dc19aa745a8d5) )
	ROM_LOAD( "1595.prom-u10",   0x5000, 0x0800, CRC(a704d142) SHA1(95c1249a8efd1a69972ffd7a4da76a0bca5095d9) )
	ROM_LOAD( "1596.prom-u11",   0x5800, 0x0800, CRC(6975e33d) SHA1(3f12037edd6f1b803b5f864789f4b88958ac9578) )
	ROM_LOAD( "1597.prom-u12",   0x6000, 0x0800, CRC(d48ab5c2) SHA1(3f4faf4b131b120b30cd4e73ff34d5cd7ef6c47a) )
	ROM_LOAD( "1598.prom-u13",   0x6800, 0x0800, CRC(ab54a94c) SHA1(9dd57b4b6e46d46922933128d9786df011c6133d) )
	ROM_LOAD( "1599.prom-u14",   0x7000, 0x0800, CRC(c9d4f3a5) SHA1(8516914b49fad85222cbdd9a43609834f5d0f13d) )
	ROM_LOAD( "1600.prom-u15",   0x7800, 0x0800, CRC(893b7dbc) SHA1(136135f0be2e8dddfa0d21a5f4119ee4685c4866) )
	ROM_LOAD( "1601.prom-u16",   0x8000, 0x0800, CRC(867bdf4f) SHA1(5974d32d878206abd113f74ba20fa5276cf21a6f) )
	ROM_LOAD( "1602.prom-u17",   0x8800, 0x0800, CRC(bd447623) SHA1(b8d255aeb32096891379330c5b8adf1d151d70c2) )
	ROM_LOAD( "1603.prom-u18",   0x9000, 0x0800, CRC(9f8f10e8) SHA1(ffe9d872d9011b3233cb06d966852319f9e4cd01) )
	ROM_LOAD( "1604.prom-u19",   0x9800, 0x0800, CRC(ad2f0f6c) SHA1(494a224905b1dac58b3b50f65a8be986b68b06f2) )
	ROM_LOAD( "1605.prom-u20",   0xa000, 0x0800, CRC(e27d7144) SHA1(5b82fda797d86e11882d1f9738a59092c5e3e7d8) )
	ROM_LOAD( "1606.prom-u21",   0xa800, 0x0800, CRC(7965f636) SHA1(5c8720beedab4979a813ce7f0e8961c863973ff7) )

	ROM_REGION( 0x0800, "audiocpu", 0 )
	ROM_LOAD( "1607.speech-u7",  0x0000, 0x0800, CRC(b779884b) SHA1(ac07e99717a1f51b79f3e43a5d873ebfa0559320) )

	ROM_REGION( 0x4000, "speech", 0 )
	ROM_LOAD( "1608.speech-u6",  0x0000, 0x1000, CRC(637e2b13) SHA1(8a470f9a8a722f7ced340c4d32b4cf6f05b3e848) )
	ROM_LOAD( "1609.speech-u5",  0x1000, 0x1000, CRC(675ee8e5) SHA1(e314482028b8925ad02e833a1d22224533d0a683) )
	ROM_LOAD( "1610.speech-u4",  0x2000, 0x1000, CRC(2915c7bd) SHA1(3ed98747b5237aa1b3bab6866292370dc2c7655a) )

	ROM_REGION( 0x0440, "proms", 0 )
	ROM_LOAD( "s-c.xyt-u39",     0x0000, 0x0400, CRC(56484d19) SHA1(61f43126fdcfc230638ed47085ae037a098e6781) ) /* sine table */
	ROM_LOAD( "pr-82.cpu-u15",   0x0400, 0x0020, CRC(c609b79e) SHA1(49dbcbb607079a182d7eb396c0da097166ea91c9) ) /* CPU board addressing */
	ROM_LOAD( "6331.speech-u30", 0x0420, 0x0020, CRC(adcb81d0) SHA1(74b0efc7e8362b0c98e54a6107981cff656d87e1) ) /* speech board addressing */
ROM_END


ROM_START( tacscan )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "1711a.cpu-u25",  0x0000, 0x0800, CRC(0da13158) SHA1(256c5441a4841441501c9b7bcf09e0e99e8dd671) )
	ROM_LOAD( "1670c.prom-u1",  0x0800, 0x0800, CRC(98de6fd5) SHA1(f22c215d7558e00366fec5092abb51c670468f8c) )
	ROM_LOAD( "1671a.prom-u2",  0x1000, 0x0800, CRC(dc400074) SHA1(70093ef56e0784173a06da1ac781bb9d8c4e7fc5) )
	ROM_LOAD( "1672a.prom-u3",  0x1800, 0x0800, CRC(2caf6f7e) SHA1(200119260f78bb1c5389707b3ceedfbc1ae43549) )
	ROM_LOAD( "1673a.prom-u4",  0x2000, 0x0800, CRC(1495ce3d) SHA1(3189f8061961d90a52339c855c06e81f4537fb2b) )
	ROM_LOAD( "1674a.prom-u5",  0x2800, 0x0800, CRC(ab7fc5d9) SHA1(b2d9241d83d175ead4da36d7311a41a5f972e06a) )
	ROM_LOAD( "1675a.prom-u6",  0x3000, 0x0800, CRC(cf5e5016) SHA1(78a3f1e4a905515330d4737ac38576ac6e0d8611) )
	ROM_LOAD( "1676a.prom-u7",  0x3800, 0x0800, CRC(b61a3ab3) SHA1(0f4ef5c7fe299ad20fa4637260282a733f1cf461) )
	ROM_LOAD( "1677a.prom-u8",  0x4000, 0x0800, CRC(bc0273b1) SHA1(8e8d8830f17b9fa6d45d98108ca02d90c29de574) )
	ROM_LOAD( "1678b.prom-u9",  0x4800, 0x0800, CRC(7894da98) SHA1(2de7c121ad847e51a10cb1b81aec84cc44a3d04c) )
	ROM_LOAD( "1679a.prom-u10", 0x5000, 0x0800, CRC(db865654) SHA1(db4d5675b53ff2bbaf70090fd064e98862f4ad33) )
	ROM_LOAD( "1680a.prom-u11", 0x5800, 0x0800, CRC(2c2454de) SHA1(74101806439c9faeba88ffe573fa4f93ffa0ba3c) )
	ROM_LOAD( "1681a.prom-u12", 0x6000, 0x0800, CRC(77028885) SHA1(bc981620ebbfbe4e32b3b4d00504475634454c57) )
	ROM_LOAD( "1682a.prom-u13", 0x6800, 0x0800, CRC(babe5cf1) SHA1(26219b7a26f818fee2fe579ec6fb0b16c6bf056f) )
	ROM_LOAD( "1683a.prom-u14", 0x7000, 0x0800, CRC(1b98b618) SHA1(19854cb2741ba37c11ae6d429fa6c17ff930f5e5) )
	ROM_LOAD( "1684a.prom-u15", 0x7800, 0x0800, CRC(cb3ded3b) SHA1(f1e886f4f71b0f6f2c11fb8b4921c3452fc9b2c0) )
	ROM_LOAD( "1685a.prom-u16", 0x8000, 0x0800, CRC(43016a79) SHA1(ee22c1fe0c8df90d9215175104f8a796c3d2aed3) )
	ROM_LOAD( "1686a.prom-u17", 0x8800, 0x0800, CRC(a4397772) SHA1(cadc95b869f5bf5dba7f03dfe5ae64a50899cced) )
	ROM_LOAD( "1687a.prom-u18", 0x9000, 0x0800, CRC(002f3bc4) SHA1(7f3795a05d5651c90cdcd4d00c46d05178b433ea) )
	ROM_LOAD( "1688a.prom-u19", 0x9800, 0x0800, CRC(0326d87a) SHA1(3a5ea4526db417b9e00b24b019c1c6016773c9e7) )
	ROM_LOAD( "1709a.prom-u20", 0xa000, 0x0800, CRC(f35ed1ec) SHA1(dce95a862af0c6b67fb76b99fee0523d53b7551c) )
	ROM_LOAD( "1710a.prom-u21", 0xa800, 0x0800, CRC(6203be22) SHA1(89731c7c88d0125a11368d707f566eb53c783266) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "s-c.xyt-u39",    0x0000, 0x0400, CRC(56484d19) SHA1(61f43126fdcfc230638ed47085ae037a098e6781) )  /* sine table */
	ROM_LOAD( "pr-82.cpu-u15",  0x0400, 0x0020, CRC(c609b79e) SHA1(49dbcbb607079a182d7eb396c0da097166ea91c9) )  /* CPU board addressing */
ROM_END


ROM_START( startrek )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "1873.cpu-u25",    0x0000, 0x0800, CRC(be46f5d9) SHA1(fadf13042d31b0dacf02a3166545c946f6fd3f33) )
	ROM_LOAD( "1848.prom-u1",    0x0800, 0x0800, CRC(65e3baf3) SHA1(0c081ed6c8be0bb5eb3d5769ac1f0b8fe4735d11) )
	ROM_LOAD( "1849.prom-u2",    0x1000, 0x0800, CRC(8169fd3d) SHA1(439d4b857083ae40df7d7f53c36ec13b05d86a86) )
	ROM_LOAD( "1850.prom-u3",    0x1800, 0x0800, CRC(78fd68dc) SHA1(fb56567458807d9becaacac11091931af9889620) )
	ROM_LOAD( "1851.prom-u4",    0x2000, 0x0800, CRC(3f55ab86) SHA1(f75ce0c56e22e8758dd1f5ce9ac00f5f41b13465) )
	ROM_LOAD( "1852.prom-u5",    0x2800, 0x0800, CRC(2542ecfb) SHA1(7cacee44670768e9fae1024f172b867193d2ea4a) )
	ROM_LOAD( "1853.prom-u6",    0x3000, 0x0800, CRC(75c2526a) SHA1(6e86b30fcdbe7622ab873092e7a7a46d8bad790f) )
	ROM_LOAD( "1854.prom-u7",    0x3800, 0x0800, CRC(096d75d0) SHA1(26e90c296b00239a6cde4ec5e80cccd7bb36bcbd) )
	ROM_LOAD( "1855.prom-u8",    0x4000, 0x0800, CRC(bc7b9a12) SHA1(6dc60e380dc5790cd345b06c064ea7d69570aadb) )
	ROM_LOAD( "1856.prom-u9",    0x4800, 0x0800, CRC(ed9fe2fb) SHA1(5d56e8499cb4f54c5e76a9231c53d95777777e05) )
	ROM_LOAD( "1857.prom-u10",   0x5000, 0x0800, CRC(28699d45) SHA1(c133eb4fc13987e634d3789bfeaf9e03196f8fd3) )
	ROM_LOAD( "1858.prom-u11",   0x5800, 0x0800, CRC(3a7593cb) SHA1(7504f960507579d043b7ee20fb8fd2610399ff4b) )
	ROM_LOAD( "1859.prom-u12",   0x6000, 0x0800, CRC(5b11886b) SHA1(b0fb6e912953822242501943f7214e4af6ab7891) )
	ROM_LOAD( "1860.prom-u13",   0x6800, 0x0800, CRC(62eb96e6) SHA1(51d1f5e48e3e21147584ace61b8832ad892cb6e2) )
	ROM_LOAD( "1861.prom-u14",   0x7000, 0x0800, CRC(99852d1d) SHA1(eaea6a99f0a7f0292db3ea19649b5c1be45b9507) )
	ROM_LOAD( "1862.prom-u15",   0x7800, 0x0800, CRC(76ce27b2) SHA1(8fa8d73aa4dcf3709ecd057bad3278fac605988c) )
	ROM_LOAD( "1863.prom-u16",   0x8000, 0x0800, CRC(dd92d187) SHA1(5a11cdc91bb7b36ea98503892847d8dbcedfe95a) )
	ROM_LOAD( "1864.prom-u17",   0x8800, 0x0800, CRC(e37d3a1e) SHA1(15d949989431dcf1e0406f1e3745f3ee91012ff5) )
	ROM_LOAD( "1865.prom-u18",   0x9000, 0x0800, CRC(b2ec8125) SHA1(70982c614471614f6b490ae2d65faec0eff2ac37) )
	ROM_LOAD( "1866.prom-u19",   0x9800, 0x0800, CRC(6f188354) SHA1(e99946467090b68559c2b54ad2e85204b71a459f) )
	ROM_LOAD( "1867.prom-u20",   0xa000, 0x0800, CRC(b0a3eae8) SHA1(51a0855753dc2d4fe1a05bd54fa958beeab35299) )
	ROM_LOAD( "1868.prom-u21",   0xa800, 0x0800, CRC(8b4e2e07) SHA1(11f7de6327abf88012854417224b38a2352a9dc7) )
	ROM_LOAD( "1869.prom-u22",   0xb000, 0x0800, CRC(e5663070) SHA1(735944c2b924964f72f3bb3d251a35ea2aef3d15) )
	ROM_LOAD( "1870.prom-u23",   0xb800, 0x0800, CRC(4340616d) SHA1(e93686a29377933332523425532d102e30211111) )

	ROM_REGION( 0x0800, "audiocpu", 0 )
	ROM_LOAD( "1670.speech-u7",  0x0000, 0x0800, CRC(b779884b) SHA1(ac07e99717a1f51b79f3e43a5d873ebfa0559320) )

	ROM_REGION( 0x4000, "speech", 0 )
	ROM_LOAD( "1871.speech-u6",  0x0000, 0x1000, CRC(03713920) SHA1(25a0158cab9983248e91133f96d1849c9e9bcbd2) )
	ROM_LOAD( "1872.speech-u5",  0x1000, 0x1000, CRC(ebb5c3a9) SHA1(533b6f0499b311f561cf7aba14a7f48ca7c47321) )

	ROM_REGION( 0x0440, "proms", 0 )
	ROM_LOAD( "s-c.xyt-u39",     0x0000, 0x0400, CRC(56484d19) SHA1(61f43126fdcfc230638ed47085ae037a098e6781) ) /* sine table */
	ROM_LOAD( "pr-82.cpu-u15",   0x0400, 0x0020, CRC(c609b79e) SHA1(49dbcbb607079a182d7eb396c0da097166ea91c9) ) /* CPU board addressing */
	ROM_LOAD( "6331.speech-u30", 0x0420, 0x0020, CRC(adcb81d0) SHA1(74b0efc7e8362b0c98e54a6107981cff656d87e1) ) /* speech board addressing */
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(segag80v_state,elim2)
{
	address_space &iospace = m_maincpu->space(AS_IO);

	/* configure security */
	m_decrypt = segag80_security(70);

	/* configure sound */
	iospace.install_write_handler(0x3e, 0x3e, write8_delegate(FUNC(segag80v_state::elim1_sh_w),this));
	iospace.install_write_handler(0x3f, 0x3f, write8_delegate(FUNC(segag80v_state::elim2_sh_w),this));
}


DRIVER_INIT_MEMBER(segag80v_state,elim4)
{
	address_space &iospace = m_maincpu->space(AS_IO);

	/* configure security */
	m_decrypt = segag80_security(76);

	/* configure sound */
	iospace.install_write_handler(0x3e, 0x3e, write8_delegate(FUNC(segag80v_state::elim1_sh_w),this));
	iospace.install_write_handler(0x3f, 0x3f, write8_delegate(FUNC(segag80v_state::elim2_sh_w),this));

	/* configure inputs */
	iospace.install_write_handler(0xf8, 0xf8, write8_delegate(FUNC(segag80v_state::spinner_select_w),this));
	iospace.install_read_handler(0xfc, 0xfc, read8_delegate(FUNC(segag80v_state::elim4_input_r),this));
}


DRIVER_INIT_MEMBER(segag80v_state,spacfury)
{
	address_space &iospace = m_maincpu->space(AS_IO);

	/* configure security */
	m_decrypt = segag80_security(64);

	/* configure sound */
	iospace.install_write_handler(0x38, 0x38, write8_delegate(FUNC(speech_sound_device::data_w), (speech_sound_device*)m_speech));
	iospace.install_write_handler(0x3b, 0x3b, write8_delegate(FUNC(speech_sound_device::control_w), (speech_sound_device*)m_speech));
	iospace.install_write_handler(0x3e, 0x3e, write8_delegate(FUNC(segag80v_state::spacfury1_sh_w),this));
	iospace.install_write_handler(0x3f, 0x3f, write8_delegate(FUNC(segag80v_state::spacfury2_sh_w),this));
}


DRIVER_INIT_MEMBER(segag80v_state,zektor)
{
	address_space &iospace = m_maincpu->space(AS_IO);
	ay8910_device *ay8910 = machine().device<ay8910_device>("aysnd");

	/* configure security */
	m_decrypt = segag80_security(82);

	/* configure sound */
	iospace.install_write_handler(0x38, 0x38, write8_delegate(FUNC(speech_sound_device::data_w), (speech_sound_device*)m_speech));
	iospace.install_write_handler(0x3b, 0x3b, write8_delegate(FUNC(speech_sound_device::control_w), (speech_sound_device*)m_speech));
	iospace.install_write_handler(0x3c, 0x3d, write8_delegate(FUNC(ay8910_device::address_data_w), ay8910));
	iospace.install_write_handler(0x3e, 0x3e, write8_delegate(FUNC(segag80v_state::zektor1_sh_w),this));
	iospace.install_write_handler(0x3f, 0x3f, write8_delegate(FUNC(segag80v_state::zektor2_sh_w),this));

	/* configure inputs */
	iospace.install_write_handler(0xf8, 0xf8, write8_delegate(FUNC(segag80v_state::spinner_select_w),this));
	iospace.install_read_handler(0xfc, 0xfc, read8_delegate(FUNC(segag80v_state::spinner_input_r),this));
}


DRIVER_INIT_MEMBER(segag80v_state,tacscan)
{
	address_space &pgmspace = m_maincpu->space(AS_PROGRAM);
	address_space &iospace = m_maincpu->space(AS_IO);

	/* configure security */
	m_decrypt = segag80_security(76);

	/* configure sound */
	iospace.install_readwrite_handler(0x3f, 0x3f, read8_delegate(FUNC(usb_sound_device::status_r), (usb_sound_device*)m_usb), write8_delegate(FUNC(usb_sound_device::data_w), (usb_sound_device*)m_usb));
	pgmspace.install_read_handler(0xd000, 0xdfff, read8_delegate(FUNC(usb_sound_device::ram_r), (usb_sound_device*)m_usb));
	pgmspace.install_write_handler(0xd000, 0xdfff, write8_delegate(FUNC(segag80v_state::usb_ram_w),this));

	/* configure inputs */
	iospace.install_write_handler(0xf8, 0xf8, write8_delegate(FUNC(segag80v_state::spinner_select_w),this));
	iospace.install_read_handler(0xfc, 0xfc, read8_delegate(FUNC(segag80v_state::spinner_input_r),this));
}


DRIVER_INIT_MEMBER(segag80v_state,startrek)
{
	address_space &pgmspace = m_maincpu->space(AS_PROGRAM);
	address_space &iospace = m_maincpu->space(AS_IO);

	/* configure security */
	m_decrypt = segag80_security(64);

	/* configure sound */
	iospace.install_write_handler(0x38, 0x38, write8_delegate(FUNC(speech_sound_device::data_w), (speech_sound_device*)m_speech));
	iospace.install_write_handler(0x3b, 0x3b, write8_delegate(FUNC(speech_sound_device::control_w), (speech_sound_device*)m_speech));

	iospace.install_readwrite_handler(0x3f, 0x3f, read8_delegate(FUNC(usb_sound_device::status_r), (usb_sound_device*)m_usb), write8_delegate(FUNC(usb_sound_device::data_w), (usb_sound_device*)m_usb));
	pgmspace.install_read_handler(0xd000, 0xdfff, read8_delegate(FUNC(usb_sound_device::ram_r), (usb_sound_device*)m_usb));
	pgmspace.install_write_handler(0xd000, 0xdfff, write8_delegate(FUNC(segag80v_state::usb_ram_w),this));

	/* configure inputs */
	iospace.install_write_handler(0xf8, 0xf8, write8_delegate(FUNC(segag80v_state::spinner_select_w),this));
	iospace.install_read_handler(0xfc, 0xfc, read8_delegate(FUNC(segag80v_state::spinner_input_r),this));
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

//    YEAR, NAME,      PARENT,   MACHINE,  INPUT,    INIT,     MONITOR,                     COMPANY,FULLNAME,FLAGS
GAME( 1981, elim2,     0,        elim2,    elim2, segag80v_state,    elim2,    ORIENTATION_FLIP_Y,          "Gremlin", "Eliminator (2 Players, set 1)", MACHINE_IMPERFECT_SOUND )
GAME( 1981, elim2a,    elim2,    elim2,    elim2, segag80v_state,    elim2,    ORIENTATION_FLIP_Y,          "Gremlin", "Eliminator (2 Players, set 2)", MACHINE_IMPERFECT_SOUND )
GAME( 1981, elim2c,    elim2,    elim2,    elim2c, segag80v_state,   elim2,    ORIENTATION_FLIP_Y,          "Gremlin", "Eliminator (2 Players, cocktail)", MACHINE_IMPERFECT_SOUND )
GAME( 1981, elim4,     elim2,    elim2,    elim4, segag80v_state,    elim4,    ORIENTATION_FLIP_Y,          "Gremlin", "Eliminator (4 Players)", MACHINE_IMPERFECT_SOUND )
GAME( 1981, elim4p,    elim2,    elim2,    elim4, segag80v_state,    elim4,    ORIENTATION_FLIP_Y,          "Gremlin", "Eliminator (4 Players, prototype)", MACHINE_IMPERFECT_SOUND )
GAME( 1981, spacfury,  0,        spacfury, spacfury, segag80v_state, spacfury, ORIENTATION_FLIP_Y,          "Sega", "Space Fury (revision C)", MACHINE_IMPERFECT_SOUND )
GAME( 1981, spacfurya, spacfury, spacfury, spacfury, segag80v_state, spacfury, ORIENTATION_FLIP_Y,          "Sega", "Space Fury (revision A)", MACHINE_IMPERFECT_SOUND )
GAME( 1981, spacfuryb, spacfury, spacfury, spacfury, segag80v_state, spacfury, ORIENTATION_FLIP_Y,          "Sega", "Space Fury (revision B)", MACHINE_IMPERFECT_SOUND )
GAME( 1982, zektor,    0,        zektor,   zektor, segag80v_state,   zektor,   ORIENTATION_FLIP_Y,          "Sega", "Zektor (revision B)", MACHINE_IMPERFECT_SOUND )
GAME( 1982, tacscan,   0,        tacscan,  tacscan, segag80v_state,  tacscan,  ORIENTATION_FLIP_X ^ ROT270, "Sega", "Tac/Scan", MACHINE_IMPERFECT_SOUND )
GAME( 1982, startrek,  0,        startrek, startrek, segag80v_state, startrek, ORIENTATION_FLIP_Y,          "Sega", "Star Trek", MACHINE_IMPERFECT_SOUND )
