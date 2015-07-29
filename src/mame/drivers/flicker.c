// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************

  PINBALL
  Flicker was originally an EM machine, and Bally asked Nutting Associates
  to create a solid-state prototype.

  Seems to be the first ever microprocessor-controlled pinball machine.

  2012-08-23 Made working [Robbbert]

  Inputs from US Patent 4093232
  Some clues from PinMAME

  Note: If F3 pressed, or you start the system, it will remember any credits from
        last time. However, you still need to insert a coin before the start button
        will work.

************************************************************************************/

#include "machine/genpin.h"
#include "cpu/i4004/i4004.h"
#include "flicker.lh"

class flicker_state : public genpin_class
{
public:
	flicker_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_testport(*this, "TEST")
		, m_coinport(*this, "COIN")
		, m_switch(*this, "SWITCH")
	{ }

	DECLARE_WRITE8_MEMBER(port00_w);
	DECLARE_WRITE8_MEMBER(port01_w);
	DECLARE_WRITE8_MEMBER(port10_w);
	DECLARE_READ8_MEMBER(port02_r);
private:
	UINT8 m_out_data;
	required_device<i4004_cpu_device> m_maincpu;
	required_ioport m_testport;
	required_ioport m_coinport;
	required_ioport_array<7> m_switch;
};


static ADDRESS_MAP_START( flicker_rom, AS_PROGRAM, 8, flicker_state )
	AM_RANGE(0x0000, 0x03FF) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(flicker_map, AS_DATA, 8, flicker_state )
	AM_RANGE(0x0000, 0x00FF) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( flicker_io, AS_IO, 8, flicker_state )
	AM_RANGE(0x0000, 0x0000) AM_WRITE(port00_w)
	AM_RANGE(0x0001, 0x0001) AM_WRITE(port01_w)
	AM_RANGE(0x0002, 0x0002) AM_READ(port02_r)
	AM_RANGE(0x0010, 0x0010) AM_WRITE(port10_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( flicker )
	PORT_START("TEST")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Door Slam") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_TILT)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_START)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test")

	PORT_START("COIN")
	// The coin slot would be connected to one of six lines via a wire jumper on a terminal strip
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_DIPNAME( 0x07e0, 0x0020, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_6C ) )

	PORT_START("SWITCH.0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left Lane Target") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("/B Target") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left Lane 1000") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("/A Target") PORT_CODE(KEYCODE_Y)
	PORT_START("SWITCH.1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right Lane Target") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("/C Target") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right Lane 1000") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("/D Target") PORT_CODE(KEYCODE_A)
	PORT_START("SWITCH.2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Spinner") PORT_CODE(KEYCODE_S)
	PORT_START("SWITCH.3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("10's Target") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("100's Target") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Pot Bumper") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("3000 Hole") PORT_CODE(KEYCODE_H)
	PORT_START("SWITCH.4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("1000 Bonus") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("500 Target") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Out Hole") PORT_CODE(KEYCODE_X)
	PORT_START("SWITCH.5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left 500 Out") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left Bumper") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right 500 Out") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right Bumper") PORT_CODE(KEYCODE_V)
	PORT_START("SWITCH.6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A Target") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("B target") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("C target") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("D Target") PORT_CODE(KEYCODE_COMMA)
INPUT_PORTS_END

READ8_MEMBER( flicker_state::port02_r )
{
	offset = m_maincpu->state_int(I4004_RAM) & 0x0f; // we need the full address

	if (offset < 7)
		return m_switch[offset]->read();

	return 0;
}

WRITE8_MEMBER( flicker_state::port00_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0, 0, 0, 0, 0, 0 };
	offset = m_maincpu->state_int(I4004_RAM); // we need the full address
	output_set_digit_value(offset, patterns[data]);
}

WRITE8_MEMBER( flicker_state::port01_w )
{
// The output lines operate the various lamps (44 of them)
	offset = m_maincpu->state_int(I4004_RAM) & 0x0f; // we need the full address

	UINT16 test_port = m_testport->read() & 0xf81e;
	UINT16 coin_port = m_coinport->read() & 0x07e0;

	if (BIT(m_coinport->read(), 0) )
		test_port |= coin_port;

	m_maincpu->set_test(BIT(test_port, offset));
}

WRITE8_MEMBER( flicker_state::port10_w )
{
/* Outputs depend on data:
    1 = tens chime
    2 = hundreds chime
    3 = thousands chime
    4 = left bumper
    5 = right bumper
    6 = pot bumper
    7 = out hole
    8 = 3000 hole
    9 = knocker
    A = coin counter
    B = coin acceptor

The coin outputs (A and B) don't activate

A large amount of data is continuously flowing through here, even when there is no
sound to produce. We need to change this to just one pulse per actual sound. */

	if (!data && offset == m_out_data)
		m_out_data = 0;
	else
	{
		offset = m_maincpu->state_int(I4004_RAM) & 0x0f; // we need the full address
		if (data != offset)
		{
			if (data != m_out_data)
			{
				m_out_data = data;
				switch (data)
				{
					case 0x01:
						m_samples->start(1, 1);
						break;
					case 0x02:
						m_samples->start(2, 2);
						break;
					case 0x03:
						m_samples->start(3, 3);
						break;
					case 0x04:
					case 0x05:
					case 0x06:
						m_samples->start(0, 0);
						break;
					case 0x07:
					case 0x08:
						m_samples->start(5, 5);
						break;
					case 0x09:
						m_samples->start(0, 6);
						break;
					default:
						break;
				}
			}
		}
	}
}


static MACHINE_CONFIG_START( flicker, flicker_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I4004, XTAL_5MHz / 8)
	MCFG_CPU_PROGRAM_MAP(flicker_rom)
	MCFG_CPU_DATA_MAP(flicker_map)
	MCFG_CPU_IO_MAP(flicker_io)
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_flicker)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
MACHINE_CONFIG_END


ROM_START(flicker)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("flicker.rom", 0x0000, 0x0400, CRC(c692e586) SHA1(5cabb28a074d18b589b5b8f700c57e1610071c68))
ROM_END

//   YEAR    GAME     PARENT  MACHINE   INPUT    CLASS           INIT      ORIENTATION   COMPANY                            DESCRIPTION           FLAGS
GAME(1974,  flicker,  0,      flicker,  flicker, driver_device,  0,        ROT0,        "Dave Nutting Associates / Bally", "Flicker (prototype)", MACHINE_MECHANICAL )
