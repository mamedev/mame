// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Jack Rabbit memory map (preliminary)

driver by Nicola Salmoria
thanks to Andrea Babich for the manual.

TODO:

- implement discrete filters for analog signals 1 to 5 and attenuation control
  for signal 5 (74LS156)

- The 8910 outputs go through some analog circuitry to make them sound more like
  real intruments.
  #0 Ch. A = "rullante"/"cassa" (drum roll/bass drum) (selected by bits 3&4 of port A)
  #0 Ch. B = "basso" (bass)
  #0 Ch. C = straight out through an optional filter
  #1 Ch. A = "piano"
  #1 Ch. B = "tromba" (trumpet) (level selected by bit 0 of port A)
  #1 Ch. C = disabled (there's an open jumper, otherwise would go out through a filter)

- some minor color issues (see video)


Notes:
- There is a protection device which I haven't located on the schematics. It
  sits on bits 4-7 of the data bus, and is read from locations where only bits
  0-3 are connected to regular devices (6400-6407 has 4-bit RAM, while 6c00-6c07
  has a 4-bit input port).

- The 6802 driving the TMS5220 has a push button connected to the NMI line. On
  Zaccaria pinballs, when pressed, this causes the speech 6802 and the slave
  sound 6802 to play a few speech effects and sound effects;
  This currently doesn't work in MAME, and tracing the code it seems the code is
  possibly broken, made up of nonfunctional leftovers of some of the pinball test
  mode code.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6800/m6800.h"
#include "machine/i8255.h"
#include "sound/dac.h"
#include "includes/zaccaria.h"


void zaccaria_state::machine_start()
{
	save_item(NAME(m_dsw_sel));
	save_item(NAME(m_active_8910));
	save_item(NAME(m_port0a));
	save_item(NAME(m_acs));
	save_item(NAME(m_last_port0b));
	save_item(NAME(m_toggle));
	save_item(NAME(m_nmi_mask));
}

void zaccaria_state::machine_reset()
{
	m_dsw_sel = 0;
	m_active_8910 = 0;
	m_port0a = 0;
	m_acs = 0;
	m_last_port0b = 0;
	m_toggle = 0;
	m_nmi_mask = 0;
}

WRITE8_MEMBER(zaccaria_state::dsw_sel_w)
{
	switch (data & 0xf0)
	{
		case 0xe0:
			m_dsw_sel = 0;
			break;

		case 0xd0:
			m_dsw_sel = 1;
			break;

		case 0xb0:
			m_dsw_sel = 2;
			break;

		default:
			logerror("%s: portsel = %02x\n", machine().describe_context(), data);
			break;
	}
}

READ8_MEMBER(zaccaria_state::dsw_r)
{
	return m_dsw_port[m_dsw_sel]->read();
}


WRITE8_MEMBER(zaccaria_state::ay8910_port0a_w)
{
	/* bits 0-2 go to a 74LS156 with open collector outputs
	 * one out of 8 Resitors is than used to form a resistor
	 * divider with Analog input 5 (tromba)
	 */

	// bits 3-4 control the analog drum emulation on 8910 #0 ch. A

	static const int table[8] = { 8200, 5600, 3300, 1500, 820, 390, 150, 47 };
	int b0, b1, b2, ba, v;
	b0 = data & 0x01;
	b1 = (data & 0x02) >> 1;
	b2 = (data & 0x04) >> 2;
	ba = (b0<<2) | (b1<<1) | b2;
	/* 150 below to scale to volume 100 */
	v = (150 * table[ba]) / (4700 + table[ba]);
	//printf("dac1w %02d %04d\n", ba, v);
	m_ay2->set_volume(1, v);
}

READ8_MEMBER(zaccaria_state::port0a_r)
{
	return (m_active_8910 == 0) ? m_ay1->data_r(space, 0) : m_ay2->data_r(space, 0);
}

WRITE8_MEMBER(zaccaria_state::port0a_w)
{
	m_port0a = data;
}

WRITE8_MEMBER(zaccaria_state::port0b_w)
{
	/* bit 1 goes to 8910 #0 BDIR pin  */
	if ((m_last_port0b & 0x02) == 0x02 && (data & 0x02) == 0x00)
	{
		/* bit 0 goes to the 8910 #0 BC1 pin */
		m_ay1->data_address_w(space, m_last_port0b, m_port0a);
	}
	else if ((m_last_port0b & 0x02) == 0x00 && (data & 0x02) == 0x02)
	{
		/* bit 0 goes to the 8910 #0 BC1 pin */
		if (m_last_port0b & 0x01)
			m_active_8910 = 0;
	}
	/* bit 3 goes to 8910 #1 BDIR pin  */
	if ((m_last_port0b & 0x08) == 0x08 && (data & 0x08) == 0x00)
	{
		/* bit 2 goes to the 8910 #1 BC1 pin */
		m_ay2->data_address_w(space, m_last_port0b >> 2, m_port0a);
	}
	else if ((m_last_port0b & 0x08) == 0x00 && (data & 0x08) == 0x08)
	{
		/* bit 2 goes to the 8910 #1 BC1 pin */
		if (m_last_port0b & 0x04)
			m_active_8910 = 1;
	}

	m_last_port0b = data;
}

INTERRUPT_GEN_MEMBER(zaccaria_state::cb1_toggle)
{
	m_pia0->cb1_w(m_toggle & 1);
	m_toggle ^= 1;
}

WRITE8_MEMBER(zaccaria_state::port1b_w)
{
	// bit 0 = /RS
	m_tms->rsq_w((data >> 0) & 0x01);
	// bit 1 = /WS
	m_tms->wsq_w((data >> 1) & 0x01);

	// bit 3 = "ACS" (goes, inverted, to input port 6 bit 3)
	m_acs = ~data & 0x08;

	// bit 4 = led (for testing?)
	set_led_status(machine(), 0,~data & 0x10);
}


WRITE8_MEMBER(zaccaria_state::sound_command_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audio2->set_input_line(0, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}

WRITE8_MEMBER(zaccaria_state::sound1_command_w)
{
	m_pia0->ca1_w(data & 0x80);
	soundlatch2_byte_w(space, 0, data);
}

GAME_EXTERN(monymony);

READ8_MEMBER(zaccaria_state::prot1_r)
{
	switch (offset)
	{
		case 0:
			return 0x50;    /* Money Money */

		case 4:
			return 0x40;    /* Jack Rabbit */

		case 6:
			if (&machine().system() == &GAME_NAME(monymony))
				return 0x70;    /* Money Money */
			return 0xa0;    /* Jack Rabbit */

		default:
			return 0;
	}
}

READ8_MEMBER(zaccaria_state::prot2_r)
{
	switch (offset)
	{
		case 0:
			return ioport("COINS")->read();   /* bits 4 and 5 must be 0 in Jack Rabbit */

		case 2:
			return 0x10;    /* Jack Rabbit */

		case 4:
			return 0x80;    /* Money Money */

		case 6:
			return 0x00;    /* Money Money */

		default:
			return 0;
	}
}


WRITE8_MEMBER(zaccaria_state::coin_w)
{
	coin_counter_w(machine(), 0,data & 1);
}

WRITE8_MEMBER(zaccaria_state::nmi_mask_w)
{
	m_nmi_mask = data & 1;
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, zaccaria_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x63ff) AM_READONLY
	AM_RANGE(0x6400, 0x6407) AM_READ(prot1_r)
	AM_RANGE(0x6000, 0x67ff) AM_WRITE(videoram_w) AM_SHARE("videoram") /* 6400-67ff is 4 bits wide */
	AM_RANGE(0x6800, 0x683f) AM_WRITE(attributes_w) AM_SHARE("attributesram")
	AM_RANGE(0x6840, 0x685f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x6881, 0x68c0) AM_RAM AM_SHARE("spriteram2")
	AM_RANGE(0x6c00, 0x6c00) AM_WRITE(flip_screen_x_w)
	AM_RANGE(0x6c01, 0x6c01) AM_WRITE(flip_screen_y_w)
	AM_RANGE(0x6c02, 0x6c02) AM_WRITENOP    /* sound reset */
	AM_RANGE(0x6c06, 0x6c06) AM_WRITE(coin_w)
	AM_RANGE(0x6c07, 0x6c07) AM_WRITE(nmi_mask_w)
	AM_RANGE(0x6c00, 0x6c07) AM_READ(prot2_r)
	AM_RANGE(0x6e00, 0x6e00) AM_READWRITE(dsw_r, sound_command_w)
	AM_RANGE(0x7000, 0x77ff) AM_RAM
	AM_RANGE(0x7800, 0x7803) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0x7c00, 0x7c00) AM_READ(watchdog_reset_r)
	AM_RANGE(0x8000, 0xdfff) AM_ROM
ADDRESS_MAP_END

/* slave sound cpu, produces music and sound effects */
/* mapping:
   A15 A14 A13 A12 A11 A10 A09 A08 A07 A06 A05 A04 A03 A02 A01 A00
     0   0   0   0   0   0   0   0   0   *   *   *   *   *   *   *  RW 6802 internal ram
     0   0   0   x   x   x   x   x   x   x   x   x   x   x   x   x  Open bus (for area that doesn't overlap ram)
     0   0   1   x   x   x   x   x   x   x   x   x   x   x   x   x  Open bus
     0   1   0   x   x   x   x   x   x   x   x   x   0   0   x   x  Open bus
     0   1   0   x   x   x   x   x   x   x   x   x   0   1   x   x  Open bus
     0   1   0   x   x   x   x   x   x   x   x   x   1   0   x   x  Open bus
     0   1   0   x   x   x   x   x   x   x   x   x   1   1   *   *  RW 6821 PIA @ 4I
     0   1   1   x   x   x   x   x   x   x   x   x   x   x   x   x  Open bus
     1   0   %   %   *   *   *   *   *   *   *   *   *   *   *   *  R /CS4A: Enable Rom 13
     1   1   %   %   *   *   *   *   *   *   *   *   *   *   *   *  R /CS5A: Enable Rom 9
     note that the % bits go to pins 2 (6802 A12) and 26 (6802 A13) of the roms
     monymony and jackrabt both use 2764 roms, which use pin 2 as A12 and pin 26 as N/C don't care
     hence for actual chips used, the mem map is:
     1   0   x   *   *   *   *   *   *   *   *   *   *   *   *   *  R /CS4A: Enable Rom 13
     1   1   x   *   *   *   *   *   *   *   *   *   *   *   *   *  R /CS5A: Enable Rom 9

     6821 PIA: CA1 comes from the master sound cpu's latch bit 7 (which is also connected to the AY chip at 4G's IOB1); CB1 comes from a periodic counter clocked by the 6802's clock, divided by 4096. CA2 and CB2 are disconnected. PA0-7 connect to the data busses of the AY-3-8910 chips; PB0 and PB1 connect to the BC1 and BDIR pins of the AY chip at 4G; PB2 and PB3 connect to the BC1 and BDIR pins of the AY chip at 4H.
*/
static ADDRESS_MAP_START( sound_map_1, AS_PROGRAM, 8, zaccaria_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x500c, 0x500f) AM_DEVREADWRITE("pia0", pia6821_device, read, write) AM_MIRROR(0x1ff0)
	AM_RANGE(0x8000, 0x9fff) AM_ROM AM_MIRROR(0x2000) // rom 13
	AM_RANGE(0xc000, 0xdfff) AM_ROM AM_MIRROR(0x2000) // rom 9
ADDRESS_MAP_END

/* master sound cpu, controls speech directly */
/* mapping:
   A15 A14 A13 A12 A11 A10 A09 A08 A07 A06 A05 A04 A03 A02 A01 A00
     0   0   0   0   0   0   0   0   0   *   *   *   *   *   *   *  RW 6802 internal ram
**** x   0   0   0   x   x   x   x   1   x   x   0   x   x   *   *  Open bus (test mode writes as if there was another PIA here)
     x   0   0   0   x   x   x   x   1   x   x   1   x   x   *   *  RW 6821 PIA @ 1I
     x   0   0   1   0   0   x   x   x   x   x   x   x   x   x   x  W  MC1408 DAC
     x   x   0   1   0   1   x   x   x   x   x   x   x   x   x   x  W  Command to slave sound1 cpu
     x   x   0   1   1   0   x   x   x   x   x   x   x   x   x   x  R  Command read latch from z80
     x   x   0   1   1   1   x   x   x   x   x   x   x   x   x   x  Open bus
     %   %   1   0   *   *   *   *   *   *   *   *   *   *   *   *  R /CS1A: Enable Rom 8
     %   %   1   1   *   *   *   *   *   *   *   *   *   *   *   *  R /CS0A: Enable Rom 7
     note that the % bits go to pins 2 (6802 A14) and 26 (6802 A15) of the roms
     monymony and jackrabt both use 2764 roms, which use pin 2 as A12 and pin 26 as N/C don't care
     hence for actual chips used, the mem map is:
     x   *   1   0   *   *   *   *   *   *   *   *   *   *   *   *  R /CS1A: Enable Rom 8
     x   *   1   1   *   *   *   *   *   *   *   *   *   *   *   *  R /CS0A: Enable Rom 7

     6821 PIA: PA0-7, CA2 and CB1 connect to the TMS5200; CA1 and CB2 are disconnected, though the test mode assumes there's something connected to CB2 (possibly another LED like the one connected to PB4); PB3 connects to 'ACS' which goes to the z80.
*/
static ADDRESS_MAP_START( sound_map_2, AS_PROGRAM, 8, zaccaria_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM /* 6802 internal ram */
	AM_RANGE(0x0090, 0x0093) AM_DEVREADWRITE("pia1", pia6821_device, read, write) AM_MIRROR(0x8F6C)
	AM_RANGE(0x1000, 0x1000) AM_DEVWRITE("mc1408", dac_device, write_unsigned8) AM_MIRROR(0x83FF) /* MC1408 */
	AM_RANGE(0x1400, 0x1400) AM_WRITE(sound1_command_w) AM_MIRROR(0xC3FF)
	AM_RANGE(0x1800, 0x1800) AM_READ(soundlatch_byte_r) AM_MIRROR(0xC3FF)
	AM_RANGE(0x2000, 0x2fff) AM_ROM AM_MIRROR(0x8000) // rom 8 with A12 low
	AM_RANGE(0x3000, 0x3fff) AM_ROM AM_MIRROR(0x8000) // rom 7 with A12 low
	AM_RANGE(0x6000, 0x6fff) AM_ROM AM_MIRROR(0x8000) // rom 8 with A12 high
	AM_RANGE(0x7000, 0x7fff) AM_ROM AM_MIRROR(0x8000) // rom 7 with A12 high
ADDRESS_MAP_END


CUSTOM_INPUT_MEMBER(zaccaria_state::acs_r)
{
	return (m_acs & 0x08) ? 1 : 0;
}

static INPUT_PORTS_START( monymony )
	PORT_START("DSW.0")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW 5I:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x04, 0x00, "Infinite Lives (Cheat)")     PORT_DIPLOCATION("SW 5I:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW 5I:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW 5I:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x00, "Freeze" )                    PORT_DIPLOCATION("SW 5I:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Cross Hatch Pattern" )       PORT_DIPLOCATION("SW 5I:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 5I:8") /* random high scores? */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW.1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW 4I:1,2")
	PORT_DIPSETTING(    0x01, "200000" )
	PORT_DIPSETTING(    0x02, "300000" )
	PORT_DIPSETTING(    0x03, "400000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, "Table Title" )               PORT_DIPLOCATION("SW 4I:3")
	PORT_DIPSETTING(    0x00, "Todays High Scores" )
	PORT_DIPSETTING(    0x04, "High Scores" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW 4I:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW 4I:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW 4I:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW 4I:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )                    PORT_DIPLOCATION("SW 4I:8")

	PORT_START("DSW.2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW 3I:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x8c, 0x84, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW 3I:3,4,5")
	PORT_DIPSETTING(    0x8c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x88, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x84, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x70, 0x50, "Coin C" )                    PORT_DIPLOCATION("SW 3I:6,7,8")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_7C ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* other bits are outputs */

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, zaccaria_state,acs_r, NULL)   /* "ACS" - from pin 13 of a PIA on the sound board */
	/* other bits come from a protection device */
INPUT_PORTS_END

static INPUT_PORTS_START( jackrabt )
	PORT_INCLUDE( monymony )

	PORT_MODIFY("DSW.0")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 5I:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW.1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Table Title" )               PORT_DIPLOCATION("SW 4I:3")
	PORT_DIPSETTING(    0x00, "Todays High Scores" )
	PORT_DIPSETTING(    0x04, "High Scores" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static GFXDECODE_START( zaccaria )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x3_planar, 0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 32*8, 32 )
GFXDECODE_END


INTERRUPT_GEN_MEMBER(zaccaria_state::vblank_irq)
{
	if(m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


static MACHINE_CONFIG_START( zaccaria, zaccaria_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,XTAL_18_432MHz/6)   /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", zaccaria_state,  vblank_irq)
//  MCFG_QUANTUM_TIME(attotime::from_hz(1000000))

	MCFG_CPU_ADD("audiocpu", M6802,XTAL_3_579545MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(sound_map_1)
	MCFG_CPU_PERIODIC_INT_DRIVER(zaccaria_state, cb1_toggle, (double)XTAL_3_579545MHz/4096)
//  MCFG_QUANTUM_TIME(attotime::from_hz(1000000))

	MCFG_CPU_ADD("audio2", M6802,XTAL_3_579545MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(sound_map_2)
//  MCFG_QUANTUM_TIME(attotime::from_hz(1000000))

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("P1"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("P2"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("SYSTEM"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(zaccaria_state, dsw_sel_w))

	MCFG_DEVICE_ADD( "pia0", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(zaccaria_state, port0a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(zaccaria_state, port0a_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(zaccaria_state, port0b_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("audiocpu", m6802_cpu_device, nmi_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("audiocpu", m6802_cpu_device, irq_line))

	MCFG_DEVICE_ADD( "pia1", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(DEVREAD8("tms", tms5220_device, status_r))
	MCFG_PIA_WRITEPA_HANDLER(DEVWRITE8("tms", tms5220_device, data_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(zaccaria_state,port1b_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.57) /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(zaccaria_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", zaccaria)
	MCFG_PALETTE_ADD("palette", 32*8+32*8)
	MCFG_PALETTE_INDIRECT_ENTRIES(512)
	MCFG_PALETTE_INIT_OWNER(zaccaria_state, zaccaria)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_3_579545MHz/2) /* verified on pcb */
	MCFG_AY8910_PORT_B_READ_CB(READ8(driver_device, soundlatch2_byte_r))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(zaccaria_state, ay8910_port0a_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_3_579545MHz/2) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_DAC_ADD("mc1408")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* There is no xtal, the clock is obtained from a RC oscillator as shown in the TMS5220 datasheet (R=100kOhm C=22pF) */
	/* 162kHz measured on pin 3 20 minutesa fter power on. Clock would then be 162*4=648kHz. */
	MCFG_SOUND_ADD("tms", TMS5200, 649200) /* ROMCLK pin measured at 162.3Khz, OSC is exactly *4 of that) */
	MCFG_TMS52XX_IRQ_HANDLER(DEVWRITELINE("pia1", pia6821_device, cb1_w))
	MCFG_TMS52XX_READYQ_HANDLER(DEVWRITELINE("pia1", pia6821_device, ca2_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( monymony )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cpu1.1a",           0x0000, 0x1000, CRC(13c227ca) SHA1(be305d112917904dd130b08f6b5186e3fbcb858a) )
	ROM_CONTINUE(             0x8000, 0x1000 )
	ROM_LOAD( "cpu2.1b",           0x1000, 0x1000, CRC(87372545) SHA1(04618d007a93b3f6706f56b10bdf39727d7d748d) )
	ROM_CONTINUE(             0x9000, 0x1000 )
	ROM_LOAD( "cpu3.1c",           0x2000, 0x1000, CRC(6aea9c01) SHA1(36a57f4dfae52d674dcf55d2b93dbacf734866b1) )
	ROM_CONTINUE(             0xa000, 0x1000 )
	ROM_LOAD( "cpu4.1d",           0x3000, 0x1000, CRC(5fdec451) SHA1(0f955c907e0a61a725a951018fdf5cc321139863) )
	ROM_CONTINUE(             0xb000, 0x1000 )
	ROM_LOAD( "cpu5.2a",           0x4000, 0x1000, CRC(af830e3c) SHA1(bed57c341ae3500f147efe31bcf01f81466ec1c0) )
	ROM_CONTINUE(             0xc000, 0x1000 )
	ROM_LOAD( "cpu6.2c",           0x5000, 0x1000, CRC(31da62b1) SHA1(486f07087244f8537510afacb64ddd59eb512a4d) )
	ROM_CONTINUE(             0xd000, 0x1000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for first 6802 */
	ROM_LOAD( "snd13.2g",           0x8000, 0x2000, CRC(78b01b98) SHA1(2aabed56cdae9463deb513c0c5021f6c8dfd271e) )
	ROM_LOAD( "snd9.1i",           0xc000, 0x2000, CRC(94e3858b) SHA1(04961f67b95798b530bd83355dec612389f22255) )

	ROM_REGION( 0x10000, "audio2", 0 ) /* 64k for second 6802 */
	ROM_LOAD( "snd8.1h",           0x2000, 0x1000, CRC(aad76193) SHA1(e08fc184efced392ee902c4cc9daaaf3310cdfe2) )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_LOAD( "snd7.1g",           0x3000, 0x1000, CRC(1e8ffe3e) SHA1(858ee7abe88d5801237e519cae2b50ae4bf33a58) )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "bg1.2d",           0x0000, 0x2000, CRC(82ab4d1a) SHA1(5aaf42a508df236f2e7c844d377132d73053907b) )
	ROM_LOAD( "bg2.1f",           0x2000, 0x2000, CRC(40d4e4d1) SHA1(79cbade30f1c9269e70ddb9c4332cfe1e8dc50a9) )
	ROM_LOAD( "bg3.1e",           0x4000, 0x2000, CRC(36980455) SHA1(4140b0cd4137c8f209124b12d9c0eb3b04f91991) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "9g",  0x0000, 0x0200, CRC(fc9a0f21) SHA1(2a93d684645ee1b70315386127223151582ab370) )
	ROM_LOAD( "9f",  0x0200, 0x0200, CRC(93106704) SHA1(d3b8281c87d253a2ed40ff400438e879ca40c2b7) )
ROM_END

ROM_START( jackrabt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cpu-01.1a",    0x0000, 0x1000, CRC(499efe97) SHA1(f0efc910a5343001b27637779e1d4de218d44a4e) )
	ROM_CONTINUE(             0x8000, 0x1000 )
	ROM_LOAD( "cpu-01.2l",    0x1000, 0x1000, CRC(4772e557) SHA1(71c1eb49c978799294e732e65a77eba330d8da9b) )
	ROM_LOAD( "cpu-01.3l",    0x2000, 0x1000, CRC(1e844228) SHA1(0525fe95a0f90c50b54c0bf618eb083ccf20e6c4) )
	ROM_LOAD( "cpu-01.4l",    0x3000, 0x1000, CRC(ebffcc38) SHA1(abaf0e96d92f9c828a95446af6d5301053416f3d) )
	ROM_LOAD( "cpu-01.5l",    0x4000, 0x1000, CRC(275e0ed6) SHA1(c0789007a4de1aa848b7e5d26cf9fe847cc5d8a4) )
	ROM_LOAD( "cpu-01.6l",    0x5000, 0x1000, CRC(8a20977a) SHA1(ba15f4c62f600372390e56c2067b4a8ab1f2dba9) )
	ROM_LOAD( "cpu-01.2h",    0x9000, 0x1000, CRC(21f2be2a) SHA1(7d10489ca7325eebfa309ae4ffd4962a4310c403) )
	ROM_LOAD( "cpu-01.3h",    0xa000, 0x1000, CRC(59077027) SHA1(d6c2e68b4b2f1dce8a2141ec259812e732c1c69c) )
	ROM_LOAD( "cpu-01.4h",    0xb000, 0x1000, CRC(0b9db007) SHA1(836f8cacf2a097fd80d5c045bdc49b3a3174b89e) )
	ROM_LOAD( "cpu-01.5h",    0xc000, 0x1000, CRC(785e1a01) SHA1(a748d300be9455cad4f912e01c2279bb8465edfe) )
	ROM_LOAD( "cpu-01.6h",    0xd000, 0x1000, CRC(dd5979cf) SHA1(e9afe7002b2258a1c3132bdd951c6e20d473fb6a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for first 6802 */
	ROM_LOAD( "13snd.2g",     0x8000, 0x2000, CRC(fc05654e) SHA1(ed9c66672fe89c41e320e1d27b53f5efa92dce9c) )
	ROM_LOAD( "9snd.1i",      0xc000, 0x2000, CRC(3dab977f) SHA1(3e79c06d2e70b050f01b7ac58be5127ba87904b0) )

	ROM_REGION( 0x10000, "audio2", 0 ) /* 64k for second 6802 */
	ROM_LOAD( "8snd.1h",      0x2000, 0x1000, CRC(f4507111) SHA1(0513f0831b94aeda84aa4f3b4a7c60dfc5113b2d) )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_LOAD( "7snd.1g",      0x3000, 0x1000, CRC(c722eff8) SHA1(d8d1c091ab80ea2d6616e4dc030adc9905c0a496) )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "1bg.2d",       0x0000, 0x2000, CRC(9f880ef5) SHA1(0ee20fb7c794f6dafdaf2c9ee8456221c9d668c5) )
	ROM_LOAD( "2bg.1f",       0x2000, 0x2000, CRC(afc04cd7) SHA1(f4349e86b9caee71c9bf9faf68b86603417d9a2b) )
	ROM_LOAD( "3bg.1e",       0x4000, 0x2000, CRC(14f23cdd) SHA1(e5f3dac52288c56f2fd2940b397bb6c896131a26) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "jr-ic9g",      0x0000, 0x0200, CRC(85577107) SHA1(76575fa68b66130b18dfe7374d1a03740963cc73) )
	ROM_LOAD( "jr-ic9f",      0x0200, 0x0200, CRC(085914d1) SHA1(3d6f9318f5a9f08ce89e4184e3efb9881f671fa7) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "jr-pal16l8.6j",   0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "jr-pal16l8.6k",   0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( jackrabt2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1cpu2.1a",     0x0000, 0x1000, CRC(f9374113) SHA1(521f293f1894bcaf21e44bc7841a20ae29232da3) )
	ROM_CONTINUE(             0x8000, 0x1000 )
	ROM_LOAD( "2cpu2.1b",     0x1000, 0x1000, CRC(0a0eea4a) SHA1(4dfd9b2511d480bb5cc918f7d91013205911d377) )
	ROM_CONTINUE(             0x9000, 0x1000 )
	ROM_LOAD( "3cpu2.1c",     0x2000, 0x1000, CRC(291f5772) SHA1(958c2601d43de3c95ed5e3d79737199703263a6a) )
	ROM_CONTINUE(             0xa000, 0x1000 )
	ROM_LOAD( "4cpu2.1d",     0x3000, 0x1000, CRC(10972cfb) SHA1(30dd473b3416ee37f887d930ba0017b5b694398e) )
	ROM_CONTINUE(             0xb000, 0x1000 )
	ROM_LOAD( "5cpu2.2a",     0x4000, 0x1000, CRC(aa95d06d) SHA1(2216effe6cacd02a5320e71a85842087dda5f85a) )
	ROM_CONTINUE(             0xc000, 0x1000 )
	ROM_LOAD( "6cpu2.2c",     0x5000, 0x1000, CRC(404496eb) SHA1(44381e27e540fe9d8cacab4c3b1fe9a4f20d26a8) )
	ROM_CONTINUE(             0xd000, 0x1000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for first 6802 */
	ROM_LOAD( "13snd.2g",     0x8000, 0x2000, CRC(fc05654e) SHA1(ed9c66672fe89c41e320e1d27b53f5efa92dce9c) )
	ROM_LOAD( "9snd.1i",      0xc000, 0x2000, CRC(3dab977f) SHA1(3e79c06d2e70b050f01b7ac58be5127ba87904b0) )

	ROM_REGION( 0x10000, "audio2", 0 ) /* 64k for second 6802 */
	ROM_LOAD( "8snd.1h",      0x2000, 0x1000, CRC(f4507111) SHA1(0513f0831b94aeda84aa4f3b4a7c60dfc5113b2d) )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_LOAD( "7snd.1g",      0x3000, 0x1000, CRC(c722eff8) SHA1(d8d1c091ab80ea2d6616e4dc030adc9905c0a496) )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "1bg.2d",       0x0000, 0x2000, CRC(9f880ef5) SHA1(0ee20fb7c794f6dafdaf2c9ee8456221c9d668c5) )
	ROM_LOAD( "2bg.1f",       0x2000, 0x2000, CRC(afc04cd7) SHA1(f4349e86b9caee71c9bf9faf68b86603417d9a2b) )
	ROM_LOAD( "3bg.1e",       0x4000, 0x2000, CRC(14f23cdd) SHA1(e5f3dac52288c56f2fd2940b397bb6c896131a26) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "jr-ic9g",      0x0000, 0x0200, CRC(85577107) SHA1(76575fa68b66130b18dfe7374d1a03740963cc73) )
	ROM_LOAD( "jr-ic9f",      0x0200, 0x0200, CRC(085914d1) SHA1(3d6f9318f5a9f08ce89e4184e3efb9881f671fa7) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8.6j",   0x0000, 0x0104, CRC(a88e52d6) SHA1(32efecb91843d5d1bdace86cbcc94ebacf1b9389) )
	ROM_LOAD( "pal16l8.6k",   0x0200, 0x0104, NO_DUMP )
	ROM_LOAD( "82s100.8c",    0x0400, 0x00f5, CRC(70ddfa6d) SHA1(904347cc63e88413c393f14b5f1260a57ab72677) )
	ROM_LOAD( "82s100.8n",    0x0500, 0x00f5, CRC(e00625ee) SHA1(88bbd020be67355dc0eb58b79f7deb77cbe505bb) )
ROM_END

ROM_START( jackrabts )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1cpu.1a",      0x0000, 0x1000, CRC(6698dc65) SHA1(33e3518846e88dc34f4b6c4e9ca9f8999c0460c8) )
	ROM_CONTINUE(             0x8000, 0x1000 )
	ROM_LOAD( "2cpu.1b",      0x1000, 0x1000, CRC(42b32929) SHA1(5b400d434ce903c74f58780a422a8c2594af90be) )
	ROM_CONTINUE(             0x9000, 0x1000 )
	ROM_LOAD( "3cpu.1c",      0x2000, 0x1000, CRC(89b50c9a) SHA1(5ab56247de013b5196c1c5765ead4361a5df53e0) )
	ROM_CONTINUE(             0xa000, 0x1000 )
	ROM_LOAD( "4cpu.1d",      0x3000, 0x1000, CRC(d5520665) SHA1(69b34d87d50e6d6e8d365ba0479405380ba3cf11) )
	ROM_CONTINUE(             0xb000, 0x1000 )
	ROM_LOAD( "5cpu.2a",      0x4000, 0x1000, CRC(0f9a093c) SHA1(7fba0d2b8d5d4d1597decec96ed93b997c721d99) )
	ROM_CONTINUE(             0xc000, 0x1000 )
	ROM_LOAD( "6cpu.2c",      0x5000, 0x1000, CRC(f53d6356) SHA1(9b167edca59cf81a2468368a372bab132f15e2ea) )
	ROM_CONTINUE(             0xd000, 0x1000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for first 6802 */
	ROM_LOAD( "13snd.2g",     0x8000, 0x2000, CRC(fc05654e) SHA1(ed9c66672fe89c41e320e1d27b53f5efa92dce9c) )
	ROM_LOAD( "9snd.1i",      0xc000, 0x2000, CRC(3dab977f) SHA1(3e79c06d2e70b050f01b7ac58be5127ba87904b0) )

	ROM_REGION( 0x10000, "audio2", 0 ) /* 64k for second 6802 */
	ROM_LOAD( "8snd.1h",      0x2000, 0x1000, CRC(f4507111) SHA1(0513f0831b94aeda84aa4f3b4a7c60dfc5113b2d) )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_LOAD( "7snd.1g",      0x3000, 0x1000, CRC(c722eff8) SHA1(d8d1c091ab80ea2d6616e4dc030adc9905c0a496) )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "1bg.2d",       0x0000, 0x2000, CRC(9f880ef5) SHA1(0ee20fb7c794f6dafdaf2c9ee8456221c9d668c5) )
	ROM_LOAD( "2bg.1f",       0x2000, 0x2000, CRC(afc04cd7) SHA1(f4349e86b9caee71c9bf9faf68b86603417d9a2b) )
	ROM_LOAD( "3bg.1e",       0x4000, 0x2000, CRC(14f23cdd) SHA1(e5f3dac52288c56f2fd2940b397bb6c896131a26) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "jr-ic9g",      0x0000, 0x0200, CRC(85577107) SHA1(76575fa68b66130b18dfe7374d1a03740963cc73) )
	ROM_LOAD( "jr-ic9f",      0x0200, 0x0200, CRC(085914d1) SHA1(3d6f9318f5a9f08ce89e4184e3efb9881f671fa7) )
ROM_END



GAME( 1983, monymony,  0,        zaccaria, monymony, driver_device, 0, ROT90, "Zaccaria", "Money Money", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, jackrabt,  0,        zaccaria, jackrabt, driver_device, 0, ROT90, "Zaccaria", "Jack Rabbit (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, jackrabt2, jackrabt, zaccaria, jackrabt, driver_device, 0, ROT90, "Zaccaria", "Jack Rabbit (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, jackrabts, jackrabt, zaccaria, jackrabt, driver_device, 0, ROT90, "Zaccaria", "Jack Rabbit (special)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
