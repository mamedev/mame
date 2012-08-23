/***********************************************************************************

  Flicker Pinball

  Prototype create by Nutting Associates for Bally.

  Seems to be the first ever microprocessor-controlled pinball machine.

  2012-08-23 Made working [Robbbert]

  Inputs from US Patent 4093232
  Some clues from PinMAME

ToDo:
- Add remaining inputs


************************************************************************************/

#include "emu.h"
#include "cpu/i4004/i4004.h"
#include "flicker.lh"

class flicker_state : public driver_device
{
public:
	flicker_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }
	DECLARE_WRITE8_MEMBER(port00_w);
	DECLARE_WRITE8_MEMBER(port01_w);
	DECLARE_WRITE8_MEMBER(port10_w);
	DECLARE_READ8_MEMBER(port02_r);

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
public:
	DECLARE_DRIVER_INIT(flicker);
};


static ADDRESS_MAP_START( flicker_rom, AS_PROGRAM, 8, flicker_state )
	AM_RANGE(0x0000, 0x03FF) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(flicker_map, AS_DATA, 8, flicker_state )
	AM_RANGE(0x0000, 0x00FF) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( flicker_io , AS_IO, 8, flicker_state )
	AM_RANGE(0x0000, 0x0000) AM_WRITE(port00_w)
	AM_RANGE(0x0001, 0x0001) AM_WRITE(port01_w)
	AM_RANGE(0x0002, 0x0002) AM_READ(port02_r)
	AM_RANGE(0x0010, 0x0010) AM_WRITE(port10_w)
ADDRESS_MAP_END

// from us patent 4093232
static INPUT_PORTS_START( flicker )
	PORT_START("TEST")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Door Slam") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("4 coins")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("3 coins")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("2 coins")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("1 coin credit") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("2 credit") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("3 credit") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("4 credit") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("5 credit") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("6 credit") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Tilt") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Start") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test") PORT_CODE(KEYCODE_W)

	PORT_START("B1")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bumper") PORT_CODE(KEYCODE_M)
INPUT_PORTS_END

READ8_MEMBER( flicker_state::port02_r )
{
	offset = cpu_get_reg(m_maincpu, I4004_RAM) - 0x20; // we need the full address

	if (offset == 0)
		return ioport("B1")->read();
	else
		return 0;
}

WRITE8_MEMBER( flicker_state::port00_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0, 0, 0, 0, 0, 0 };
	offset = cpu_get_reg(m_maincpu, I4004_RAM); // we need the full address
	output_set_digit_value(offset, patterns[data]);
}

WRITE8_MEMBER( flicker_state::port01_w )
{
	offset = cpu_get_reg(m_maincpu, I4004_RAM) - 0x10; // we need the full address

	if (offset < 0x10)
		i4004_set_test(m_maincpu, BIT(ioport("TEST")->read(), offset));
}

WRITE8_MEMBER( flicker_state::port10_w )
{
	//offset = cpu_get_reg(m_maincpu, I4004_RAM) - 0x10; // we need the full address
}


void flicker_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(flicker_state,flicker)
{
}

static MACHINE_CONFIG_START( flicker, flicker_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I4004, XTAL_5MHz / 8)
	MCFG_CPU_PROGRAM_MAP(flicker_rom)
	MCFG_CPU_DATA_MAP(flicker_map)
	MCFG_CPU_IO_MAP(flicker_io)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_flicker)
MACHINE_CONFIG_END


ROM_START(flicker)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("flicker.rom", 0x0000, 0x0400, CRC(c692e586) SHA1(5cabb28a074d18b589b5b8f700c57e1610071c68))
ROM_END

//   YEAR    GAME     PARENT  MACHINE   INPUT    CLASS           INIT      ORIENTATION    COMPANY             DESCRIPTION             FLAGS
GAME(1974,  flicker,  0,      flicker,  flicker, flicker_state,  flicker,  ROT0,        "Nutting Associates", "Flicker (Prototype)", GAME_IS_SKELETON_MECHANICAL)
