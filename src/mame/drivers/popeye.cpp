// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Couriersud
// thanks-to: Marc Lafontaine
/***************************************************************************

Popeye  (c) 1982 Nintendo

To enter service mode, reset keeping the service button pressed.

Notes:
- The main set has a protection device mapped at E000/E001. The second set
  (which is the same revision of the program code) has the protection disabled
  in a very clean way, so I don't know if it's an original (without the
  protection device to save costs), or a very well done bootleg.
- The bootleg derives from a different revision of the program code which we
  don't have.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/popeye.h"
#include "machine/netlist.h"
#include "netlist/devices/net_lib.h"

/* This is the output stage of the audio circuit.
 * It is solely an impedance changer and could be left away
 */

static NETLIST_START(nl_popeye_imp_changer)
	RES(R62, 510000)
	RES(R63, 100)
	RES(R64, 510000)
	RES(R65, 2100)
	RES(R66, 330)
	RES(R67, 51)

	QBJT_EB(Q8, "2SC1815")
	QBJT_EB(Q9, "2SA1015")

	NET_C(V5, R62.1, Q8.C, R66.1)
	NET_C(R62.2, R64.1, R63.1, C7.2)
	NET_C(R63.2, Q8.B)
	NET_C(Q8.E, R65.1, Q9.B)
	NET_C(R66.2, Q9.E, R67.1)

	NET_C(GND, Q9.C, R65.2, R64.2)
NETLIST_END()

static NETLIST_START(nl_popeye)

	/* register hard coded netlists */

	LOCAL_SOURCE(nl_popeye_imp_changer)

	/* Standard stuff */

	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-5)
	ANALOG_INPUT(V5, 5)

	/* AY 8910 internal resistors */

	RES(R_AY1_1, 1000);
	RES(R_AY1_2, 1000);
	RES(R_AY1_3, 1000);

	RES(R52, 2000)
	RES(R55, 2000)
	RES(R58, 2000)
	RES(R53, 2000)
	RES(R56, 2000)
	RES(R59, 2000)
	RES(R51, 20000)
	RES(R57, 30000)
	RES(R60, 30000)

	RES(R61, 120000)

	RES(ROUT, 5000)

	CAP(C4, 0.047e-6)
	CAP(C5, 330e-12)
	CAP(C6, 330e-12)
	CAP(C7, 3.3e-6)
	CAP(C40, 680e-12)

	NET_C(V5, R_AY1_1.1, R_AY1_2.1, R_AY1_3.1)

	NET_C(R_AY1_1.2, R52.1, R53.1)
	NET_C(R_AY1_2.2, R55.1, R56.1)
	NET_C(R_AY1_3.2, R58.1, R59.1)

	NET_C(R53.2, R51.1, C4.1)
	NET_C(R56.2, R57.1, C5.1)
	NET_C(R59.2, R60.1, C6.1)

	NET_C(R51.2, R57.2, R60.2, R61.1, C40.1, C7.1)

	NET_C(GND, R52.2, R55.2, R58.2, C4.2, C5.2, C6.2, R61.2, C40.2)

	INCLUDE(nl_popeye_imp_changer)

	/* output resistor (actually located in TV */

	NET_C(R67.2, ROUT.1)

	NET_C(GND, ROUT.2)

NETLIST_END()



INTERRUPT_GEN_MEMBER(popeye_state::popeye_interrupt)
{
	m_field ^= 1;
	/* NMIs are enabled by the I register?? How can that be? */
	if (device.state().state_int(Z80_I) & 1)    /* skyskipr: 0/1, popeye: 2/3 but also 0/1 */
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


/* the protection device simply returns the last two values written shifted left */
/* by a variable amount. */

READ8_MEMBER(popeye_state::protection_r)
{
	if (offset == 0)
	{
		return ((m_prot1 << m_prot_shift) | (m_prot0 >> (8-m_prot_shift))) & 0xff;
	}
	else    /* offset == 1 */
	{
		/* the game just checks if bit 2 is clear. Returning 0 seems to be enough. */
		return 0;
	}
}

WRITE8_MEMBER(popeye_state::protection_w)
{
	if (offset == 0)
	{
		/* this is the same as the level number (1-3) */
		m_prot_shift = data & 0x07;
	}
	else    /* offset == 1 */
	{
		m_prot0 = m_prot1;
		m_prot1 = data;
	}
}




static ADDRESS_MAP_START( skyskipr_map, AS_PROGRAM, 8, popeye_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8c00, 0x8c02) AM_RAM AM_SHARE("background_pos")
	AM_RANGE(0x8c03, 0x8c03) AM_RAM AM_SHARE("palettebank")
	AM_RANGE(0x8c04, 0x8e7f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x8e80, 0x8fff) AM_RAM
	AM_RANGE(0xa000, 0xa3ff) AM_WRITE(popeye_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xa400, 0xa7ff) AM_WRITE(popeye_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xc000, 0xcfff) AM_WRITE(skyskipr_bitmap_w)
	AM_RANGE(0xe000, 0xe001) AM_READWRITE(protection_r,protection_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( popeye_map, AS_PROGRAM, 8, popeye_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8bff) AM_RAM
	AM_RANGE(0x8c00, 0x8c02) AM_RAM AM_SHARE("background_pos")
	AM_RANGE(0x8c03, 0x8c03) AM_RAM AM_SHARE("palettebank")
	AM_RANGE(0x8c04, 0x8e7f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x8e80, 0x8fff) AM_RAM
	AM_RANGE(0xa000, 0xa3ff) AM_WRITE(popeye_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xa400, 0xa7ff) AM_WRITE(popeye_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xc000, 0xdfff) AM_WRITE(popeye_bitmap_w)
	AM_RANGE(0xe000, 0xe001) AM_READWRITE(protection_r,protection_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( popeyebl_map, AS_PROGRAM, 8, popeye_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8c00, 0x8c02) AM_RAM AM_SHARE("background_pos")
	AM_RANGE(0x8c03, 0x8c03) AM_RAM AM_SHARE("palettebank")
	AM_RANGE(0x8c04, 0x8e7f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x8e80, 0x8fff) AM_RAM
	AM_RANGE(0xa000, 0xa3ff) AM_WRITE(popeye_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xa400, 0xa7ff) AM_WRITE(popeye_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xc000, 0xcfff) AM_WRITE(skyskipr_bitmap_w)
	AM_RANGE(0xe000, 0xe01f) AM_ROM AM_REGION("proms", 0x0240)
ADDRESS_MAP_END

static ADDRESS_MAP_START( popeye_io_map, AS_IO, 8, popeye_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P2")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x03, 0x03) AM_DEVREAD("aysnd", ay8910_device, data_r)
ADDRESS_MAP_END


CUSTOM_INPUT_MEMBER(popeye_state::dsw1_read)
{
	return ioport("DSW1")->read() >> m_dswbit;
}


static INPUT_PORTS_START( skyskipr )
	PORT_START("P1")    /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 )

	PORT_START("P2")    /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START("SYSTEM")   /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("DSW0")  /* DSW0 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, "A 3/1 B 1/2" )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, "A 2/1 B 2/5" )
	PORT_DIPSETTING(    0x04, "A 2/1 B 1/3" )
	PORT_DIPSETTING(    0x07, "A 1/1 B 2/1" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, "A 1/1 B 1/2" )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, "A 1/2 B 1/4" )
	PORT_DIPSETTING(    0x0b, "A 1/2 B 1/5" )
	PORT_DIPSETTING(    0x02, "A 2/5 B 1/1" )
	PORT_DIPSETTING(    0x0a, "A 1/3 B 1/1" )
	PORT_DIPSETTING(    0x09, "A 1/4 B 1/1" )
	PORT_DIPSETTING(    0x05, "A 1/5 B 1/1" )
	PORT_DIPSETTING(    0x08, "A 1/6 B 1/1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, popeye_state, dsw1_read, NULL)

	PORT_START("DSW1")  /* DSW1 */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x20, "15000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

CUSTOM_INPUT_MEMBER( popeye_state::pop_field_r )
{
	return m_field ^ 1;
}

static INPUT_PORTS_START( popeye )
	PORT_START("P1")    /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */

	PORT_START("P2")    /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */

	PORT_START("SYSTEM")   /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, popeye_state,pop_field_r, NULL) /* inverted init e/o signal (even odd) */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("DSW0")  /* DSW0 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )    PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x40, "Copyright" )
	PORT_DIPSETTING(    0x40, "Nintendo" )
	PORT_DIPSETTING(    0x20, "Nintendo Co.,Ltd" )
	PORT_DIPSETTING(    0x60, "Nintendo of America" )
//  PORT_DIPSETTING(    0x00, "Nintendo of America" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, popeye_state, dsw1_read, NULL)

	PORT_START("DSW1")  /* DSW1 */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "40000" )
	PORT_DIPSETTING(    0x20, "60000" )
	PORT_DIPSETTING(    0x10, "80000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )     PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static INPUT_PORTS_START( popeyef )
	PORT_INCLUDE( popeye )

	PORT_MODIFY("DSW0")  /* DSW0 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )    PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x03, "A 3/1 B 1/2" )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, "A 2/1 B 2/5" )
	PORT_DIPSETTING(    0x04, "A 2/1 B 1/3" )
	PORT_DIPSETTING(    0x07, "A 1/1 B 2/1" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, "A 1/1 B 1/2" )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, "A 1/2 B 1/4" )
	PORT_DIPSETTING(    0x0b, "A 1/2 B 1/5" )
	PORT_DIPSETTING(    0x02, "A 2/5 B 1/1" )
	PORT_DIPSETTING(    0x0a, "A 1/3 B 1/1" )
	PORT_DIPSETTING(    0x09, "A 1/4 B 1/1" )
	PORT_DIPSETTING(    0x05, "A 1/5 B 1/1" )
	PORT_DIPSETTING(    0x08, "A 1/6 B 1/1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_MODIFY("DSW1")  /* DSW1 */
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "20000" )
	PORT_DIPSETTING(    0x20, "30000" )
	PORT_DIPSETTING(    0x10, "50000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	2,
	{ 0, RGN_FRAC(1,2) },
	{RGN_FRAC(1,4)+7,RGN_FRAC(1,4)+6,RGN_FRAC(1,4)+5,RGN_FRAC(1,4)+4,
		RGN_FRAC(1,4)+3,RGN_FRAC(1,4)+2,RGN_FRAC(1,4)+1,RGN_FRAC(1,4)+0,
		7,6,5,4,3,2,1,0 },
	{ 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8,
		7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	16*8
};

static GFXDECODE_START( popeye )
	GFXDECODE_SCALE( "gfx1", 0, charlayout,   16, 16, 2, 2 ) /* chars */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 16+16*2, 64 ) /* sprites */
GFXDECODE_END




WRITE8_MEMBER(popeye_state::popeye_portB_w)
{
	/* bit 0 flips screen */
	flip_screen_set(data & 1);

	/* bits 1-3 select DSW1 bit to read */
	m_dswbit = (data & 0x0e) >> 1;
}

static MACHINE_CONFIG_START( skyskipr, popeye_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_8MHz/2)   /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(skyskipr_map)
	MCFG_CPU_IO_MAP(popeye_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", popeye_state,  popeye_interrupt)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0*16, 32*16-1, 2*16, 30*16-1)
	MCFG_SCREEN_UPDATE_DRIVER(popeye_state, screen_update_popeye)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", popeye)
	MCFG_PALETTE_ADD("palette", 16+16*2+64*4)
	MCFG_PALETTE_INIT_OWNER(popeye_state, popeye)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_8MHz/4)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW0"))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(popeye_state, popeye_portB_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( popeye, skyskipr )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(popeye_map)

	MCFG_SOUND_MODIFY("aysnd")
	MCFG_SOUND_ROUTES_RESET()
	MCFG_AY8910_OUTPUT_TYPE(AY8910_RESISTOR_OUTPUT) /* Does Sky Skipper have the same filtering? */
	MCFG_AY8910_RES_LOADS(2000.0, 2000.0, 2000.0)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW0"))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(popeye_state, popeye_portB_w))
	MCFG_SOUND_ROUTE_EX(0, "snd_nl", 1.0, 0)
	MCFG_SOUND_ROUTE_EX(1, "snd_nl", 1.0, 1)
	MCFG_SOUND_ROUTE_EX(2, "snd_nl", 1.0, 2)

	/* NETLIST configuration using internal AY8910 resistor values */

	MCFG_SOUND_ADD("snd_nl", NETLIST_SOUND, 48000)
	MCFG_NETLIST_SETUP(nl_popeye)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_NETLIST_STREAM_INPUT("snd_nl", 0, "R_AY1_1.R")
	MCFG_NETLIST_STREAM_INPUT("snd_nl", 1, "R_AY1_2.R")
	MCFG_NETLIST_STREAM_INPUT("snd_nl", 2, "R_AY1_3.R")

	MCFG_NETLIST_STREAM_OUTPUT("snd_nl", 0, "ROUT.1")
	MCFG_NETLIST_ANALOG_MULT_OFFSET(30000.0, -65000.0)

	MCFG_VIDEO_START_OVERRIDE(popeye_state,popeye)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( popeyebl, popeye )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(popeyebl_map)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(popeye_state,popeyebl)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( skyskipr )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tnx1-c.2a",    0x0000, 0x1000, CRC(bdc7f218) SHA1(9a5f5959b9228912f810568ddad70daa81c4daac) )
	ROM_LOAD( "tnx1-c.2b",    0x1000, 0x1000, CRC(cbe601a8) SHA1(78edc384b75b7958906f887d11eb7cf235d6dc44) )
	ROM_LOAD( "tnx1-c.2c",    0x2000, 0x1000, CRC(5ca79abf) SHA1(0712364ad8785a146c4a146cc688c4892dd59c93) )
	ROM_LOAD( "tnx1-c.2d",    0x3000, 0x1000, CRC(6b7a7071) SHA1(949a8106a5b750bc15a5919786fb99f15cd9424e) )
	ROM_LOAD( "tnx1-c.2e",    0x4000, 0x1000, CRC(6b0c0525) SHA1(e4e12ea9e3140736d7543a274f3b266e58059356) )
	ROM_LOAD( "tnx1-c.2f",    0x5000, 0x1000, CRC(d1712424) SHA1(2de42c379f18bfbd68fc64db24c9b0d38de26c29) )
	ROM_LOAD( "tnx1-c.2g",    0x6000, 0x1000, CRC(8b33c4cf) SHA1(86d51b5098dffc69330b28662b04bd906d962792) )
	/* 7000-7fff empty */

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tnx1-v.3h",    0x0000, 0x0800, CRC(ecb6a046) SHA1(7fd2312d39fefe6237699e2916e0c313165755ad) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "tnx1-t.1e",    0x0000, 0x1000, CRC(01c1120e) SHA1(c1ab2af9c582e647e98836a87cb09fe4bcdaf19f) )
	ROM_LOAD( "tnx1-t.2e",    0x1000, 0x1000, CRC(70292a71) SHA1(51a5677351e438e7ef2b3812ef5e0e610da6ef4d) )
	ROM_LOAD( "tnx1-t.3e",    0x2000, 0x1000, CRC(92b6a0e8) SHA1(d11ff39932cec9b5408dc957bf825dcf1d00c027) )
	ROM_LOAD( "tnx1-t.5e",    0x3000, 0x1000, CRC(cc5f0ac3) SHA1(3374b1c387df3a3eba6c523f7c13711260074a89) )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "tnx1-t.4a",    0x0000, 0x0020, CRC(98846924) SHA1(89688d05c4c2d6f389da4e9c0c13e0f022f2d376) ) /* background palette */
	ROM_LOAD( "tnx1-t.1a",    0x0020, 0x0020, CRC(c2bca435) SHA1(e7d3e68d153d646eaf12b263d58b07da2615399c) ) /* char palette */
	ROM_LOAD( "tnx1-t.3a",    0x0040, 0x0100, CRC(8abf9de4) SHA1(6e5500639a2dca3c288619fb8bdd120eb49bf8e0) ) /* sprite palette - low 4 bits */
	ROM_LOAD( "tnx1-t.2a",    0x0140, 0x0100, CRC(aa7ff322) SHA1(522d21854aa11e24f3679163354ae4fb35619eff) ) /* sprite palette - high 4 bits */
	ROM_LOAD( "tnx1-t.3j",    0x0240, 0x0100, CRC(1c5c8dea) SHA1(5738303b2a9c79b7d06bcf20fdb4d9b29f6e2d96) ) /* timing for the protection ALU */
ROM_END

/*
    Popeye

    CPU/Sound Board: TPP2-01-CPU
    Video Board:     TPP2-01-VIDEO
*/

ROM_START( popeye )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tpp2-c.7a", 0x0000, 0x2000, CRC(9af7c821) SHA1(592acfe221b5c3bd9b920f639b141f37a56d6997) )
	ROM_LOAD( "tpp2-c.7b", 0x2000, 0x2000, CRC(c3704958) SHA1(af96d10fa9bdb86b00c89d10f67cb5ca5586f446) )
	ROM_LOAD( "tpp2-c.7c", 0x4000, 0x2000, CRC(5882ebf9) SHA1(5531229b37f9ba0ede7fdc24909e3c3efbc8ade4) )
	ROM_LOAD( "tpp2-c.7e", 0x6000, 0x2000, CRC(ef8649ca) SHA1(a0157f91600e56e2a953dadbd76da4330652e5c8) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tpp2-v.5n", 0x0000, 0x0800, CRC(cca61ddd) SHA1(239f87947c3cc8c6693c295ebf5ea0b7638b781c) )   /* first half is empty */
	ROM_CONTINUE(          0x0000, 0x0800 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "tpp2-v.1e", 0x0000, 0x2000, CRC(0f2cd853) SHA1(426c9b4f6579bfcebe72b976bfe4f05147d53f96) )
	ROM_LOAD( "tpp2-v.1f", 0x2000, 0x2000, CRC(888f3474) SHA1(ddee56b2b49bd50aaf9c98d8ef6e905e3f6ab859) )
	ROM_LOAD( "tpp2-v.1j", 0x4000, 0x2000, CRC(7e864668) SHA1(8e275dbb1c586f4ebca7548db05246ef0f56d7b1) )
	ROM_LOAD( "tpp2-v.1k", 0x6000, 0x2000, CRC(49e1d170) SHA1(bd51a4e34ce8109f26954760156e3cf05fb9db57) )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "tpp2-c.4a", 0x0000, 0x0020, CRC(375e1602) SHA1(d84159a0af5db577821c43712bc733329a43af80) ) /* background palette */
	ROM_LOAD( "tpp2-c.3a", 0x0020, 0x0020, CRC(e950bea1) SHA1(0b48082fe79d9fcdca7e80caff1725713d0c3163) ) /* char palette */
	ROM_LOAD( "tpp2-c.5b", 0x0040, 0x0100, CRC(c5826883) SHA1(f2c4d3473b3bfa55bffad003dc1fd79540e7e0d1) ) /* sprite palette - low 4 bits */
	ROM_LOAD( "tpp2-c.5a", 0x0140, 0x0100, CRC(c576afba) SHA1(013c8e8db08a03c7ba156cfefa671d26155fe835) ) /* sprite palette - high 4 bits */
	ROM_LOAD( "tpp2-v.7j", 0x0240, 0x0100, CRC(a4655e2e) SHA1(2a620932fccb763c6c667278c0914f31b9f00ddf) ) /* timing for the protection ALU */
ROM_END

ROM_START( popeyeu )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "7a",        0x0000, 0x2000, CRC(0bd04389) SHA1(3b08186c9b20dd4dfb92df98941b18999f23aece) )
	ROM_LOAD( "7b",        0x2000, 0x2000, CRC(efdf02c3) SHA1(4fa616bdb4e21f752e46890d007c911fff9ceadc) )
	ROM_LOAD( "7c",        0x4000, 0x2000, CRC(8eee859e) SHA1(a597d5655d06d565653c64b18ed8842625e15088) )
	ROM_LOAD( "7e",        0x6000, 0x2000, CRC(b64aa314) SHA1(b5367f518350223e191d94434dc535873efb4c74) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tpp2-v.5n", 0x0000, 0x0800, CRC(cca61ddd) SHA1(239f87947c3cc8c6693c295ebf5ea0b7638b781c) )   /* first half is empty */
	ROM_CONTINUE(          0x0000, 0x0800 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "tpp2-v.1e", 0x0000, 0x2000, CRC(0f2cd853) SHA1(426c9b4f6579bfcebe72b976bfe4f05147d53f96) )
	ROM_LOAD( "tpp2-v.1f", 0x2000, 0x2000, CRC(888f3474) SHA1(ddee56b2b49bd50aaf9c98d8ef6e905e3f6ab859) )
	ROM_LOAD( "tpp2-v.1j", 0x4000, 0x2000, CRC(7e864668) SHA1(8e275dbb1c586f4ebca7548db05246ef0f56d7b1) )
	ROM_LOAD( "tpp2-v.1k", 0x6000, 0x2000, CRC(49e1d170) SHA1(bd51a4e34ce8109f26954760156e3cf05fb9db57) )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "tpp2-c.4a", 0x0000, 0x0020, CRC(375e1602) SHA1(d84159a0af5db577821c43712bc733329a43af80) ) /* background palette */
	ROM_LOAD( "tpp2-c.3a", 0x0020, 0x0020, CRC(e950bea1) SHA1(0b48082fe79d9fcdca7e80caff1725713d0c3163) ) /* char palette */
	ROM_LOAD( "tpp2-c.5b", 0x0040, 0x0100, CRC(c5826883) SHA1(f2c4d3473b3bfa55bffad003dc1fd79540e7e0d1) ) /* sprite palette - low 4 bits */
	ROM_LOAD( "tpp2-c.5a", 0x0140, 0x0100, CRC(c576afba) SHA1(013c8e8db08a03c7ba156cfefa671d26155fe835) ) /* sprite palette - high 4 bits */
	ROM_LOAD( "tpp2-v.7j", 0x0240, 0x0100, CRC(a4655e2e) SHA1(2a620932fccb763c6c667278c0914f31b9f00ddf) ) /* timing for the protection ALU */
ROM_END

ROM_START( popeyef )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tpp2-c_f.7a", 0x0000, 0x2000, CRC(5fc5264d) SHA1(6c3d4df748c55293b6de58bd874a08f8164b878d) )
	ROM_LOAD( "tpp2-c_f.7b", 0x2000, 0x2000, CRC(51de48e8) SHA1(7573931c6fcb53ee5ab9408906cd8eb2ba271c64) )
	ROM_LOAD( "tpp2-c_f.7c", 0x4000, 0x2000, CRC(62df9647) SHA1(65d043b4142aa3ad2db7a1d4e1a2c22656ca7ade) )
	ROM_LOAD( "tpp2-c_f.7e", 0x6000, 0x2000, CRC(f31e7916) SHA1(0f54ea7b1691b7789067fe880ffc56fac1d9523a) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tpp2-v.5n",   0x0000, 0x0800, CRC(cca61ddd) SHA1(239f87947c3cc8c6693c295ebf5ea0b7638b781c) ) /* first half is empty */
	ROM_CONTINUE(            0x0000, 0x0800 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "tpp2-v.1e",   0x0000, 0x2000, CRC(0f2cd853) SHA1(426c9b4f6579bfcebe72b976bfe4f05147d53f96) )
	ROM_LOAD( "tpp2-v.1f",   0x2000, 0x2000, CRC(888f3474) SHA1(ddee56b2b49bd50aaf9c98d8ef6e905e3f6ab859) )
	ROM_LOAD( "tpp2-v.1j",   0x4000, 0x2000, CRC(7e864668) SHA1(8e275dbb1c586f4ebca7548db05246ef0f56d7b1) )
	ROM_LOAD( "tpp2-v.1k",   0x6000, 0x2000, CRC(49e1d170) SHA1(bd51a4e34ce8109f26954760156e3cf05fb9db57) )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "tpp2-c.4a",   0x0000, 0x0020, CRC(375e1602) SHA1(d84159a0af5db577821c43712bc733329a43af80) ) /* background palette */
	ROM_LOAD( "tpp2-c.3a",   0x0020, 0x0020, CRC(e950bea1) SHA1(0b48082fe79d9fcdca7e80caff1725713d0c3163) ) /* char palette */
	ROM_LOAD( "tpp2-c.5b",   0x0040, 0x0100, CRC(c5826883) SHA1(f2c4d3473b3bfa55bffad003dc1fd79540e7e0d1) ) /* sprite palette - low 4 bits */
	ROM_LOAD( "tpp2-c.5a",   0x0140, 0x0100, CRC(c576afba) SHA1(013c8e8db08a03c7ba156cfefa671d26155fe835) ) /* sprite palette - high 4 bits */
	ROM_LOAD( "tpp2-v.7j",   0x0240, 0x0100, CRC(a4655e2e) SHA1(2a620932fccb763c6c667278c0914f31b9f00ddf) ) /* timing for the protection ALU */
ROM_END

ROM_START( popeyebl )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "po1",          0x0000, 0x2000, CRC(b14a07ca) SHA1(b8666a4c6b833f60905692774e30e73f0795df11) )
	ROM_LOAD( "po2",          0x2000, 0x2000, CRC(995475ff) SHA1(5cd5ac23a73722e32c80cd6ffc435584750a46c9) )
	ROM_LOAD( "po3",          0x4000, 0x2000, CRC(99d6a04a) SHA1(b683a5bb1ac4f6bec7478760c8ad0ff7c00bc652) )
	ROM_LOAD( "po4",          0x6000, 0x2000, CRC(548a6514) SHA1(006e076781a3e5c3afa084c723247365358e3187) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "v-5n",         0x0000, 0x0800, CRC(cca61ddd) SHA1(239f87947c3cc8c6693c295ebf5ea0b7638b781c) )    /* first half is empty */
	ROM_CONTINUE(             0x0000, 0x0800 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "v-1e",         0x0000, 0x2000, CRC(0f2cd853) SHA1(426c9b4f6579bfcebe72b976bfe4f05147d53f96) )
	ROM_LOAD( "v-1f",         0x2000, 0x2000, CRC(888f3474) SHA1(ddee56b2b49bd50aaf9c98d8ef6e905e3f6ab859) )
	ROM_LOAD( "v-1j",         0x4000, 0x2000, CRC(7e864668) SHA1(8e275dbb1c586f4ebca7548db05246ef0f56d7b1) )
	ROM_LOAD( "v-1k",         0x6000, 0x2000, CRC(49e1d170) SHA1(bd51a4e34ce8109f26954760156e3cf05fb9db57) )

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "popeye.pr1",   0x0000, 0x0020, CRC(d138e8a4) SHA1(eba7f870ccab72105593007f5cd7e0b863912402) ) /* background palette */
	ROM_LOAD( "popeye.pr2",   0x0020, 0x0020, CRC(0f364007) SHA1(b77d71df391a9ac9e778e84475627e72de2b8507) ) /* char palette */
	ROM_LOAD( "popeye.pr3",   0x0040, 0x0100, CRC(ca4d7b6a) SHA1(ec979fffea9db5a327a5270241e376c21516e343) ) /* sprite palette - low 4 bits */
	ROM_LOAD( "popeye.pr4",   0x0140, 0x0100, CRC(cab9bc53) SHA1(e63ba8856190187996e405f6fcee254c8ca6e81f) ) /* sprite palette - high 4 bits */
	ROM_LOAD( "po_d1-e1.bin", 0x0240, 0x0020, CRC(8de22998) SHA1(e3a232ff85fb207afbe23049a65e828420589342) ) /* protection PROM */
ROM_END



DRIVER_INIT_MEMBER(popeye_state,skyskipr)
{
	UINT8 *rom = memregion("maincpu")->base();
	int len = memregion("maincpu")->bytes();

	/* decrypt the program ROMs */
	{
		dynamic_buffer buffer(len);
		int i;
		for (i = 0;i < len; i++)
			buffer[i] = BITSWAP8(rom[BITSWAP16(i,15,14,13,12,11,10,8,7,0,1,2,4,5,9,3,6) ^ 0xfc],3,4,2,5,1,6,0,7);
		memcpy(rom,&buffer[0],len);
	}

	save_item(NAME(m_prot0));
	save_item(NAME(m_prot1));
	save_item(NAME(m_prot_shift));
}

DRIVER_INIT_MEMBER(popeye_state,popeye)
{
	UINT8 *rom = memregion("maincpu")->base();
	int len = memregion("maincpu")->bytes();

	/* decrypt the program ROMs */
	{
		dynamic_buffer buffer(len);
		int i;
		for (i = 0;i < len; i++)
			buffer[i] = BITSWAP8(rom[BITSWAP16(i,15,14,13,12,11,10,8,7,6,3,9,5,4,2,1,0) ^ 0x3f],3,4,2,5,1,6,0,7);
		memcpy(rom,&buffer[0],len);
	}

	save_item(NAME(m_prot0));
	save_item(NAME(m_prot1));
	save_item(NAME(m_prot_shift));
}


GAME( 1981, skyskipr, 0,      skyskipr, skyskipr, popeye_state, skyskipr, ROT0, "Nintendo", "Sky Skipper", MACHINE_SUPPORTS_SAVE )
GAME( 1982, popeye,   0,      popeye,   popeye, popeye_state,   popeye,   ROT0, "Nintendo", "Popeye (revision D)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, popeyeu,  popeye, popeye,   popeye, popeye_state,   popeye,   ROT0, "Nintendo", "Popeye (revision D not protected)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, popeyef,  popeye, popeye,   popeyef, popeye_state,  popeye,   ROT0, "Nintendo", "Popeye (revision F)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, popeyebl, popeye, popeyebl, popeye, driver_device,  0,        ROT0, "bootleg",  "Popeye (bootleg)", MACHINE_SUPPORTS_SAVE )
