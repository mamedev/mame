// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        Brandt 8641

        Currency Counter

        2012-12-24 Skeleton driver. [Micko]
        2015-09-27 Added devices & inputs based entirely on guesswork [Robbbert]

        There seems to be 15 buttons (according to images, I just have board)
        also there are 8 dips currently set at 00011100 (1 is on)

        Looks like a LCD display? I think ports 8 & 9 write to it. Also looks
        like the display should say 8641 when machine is turned on.

ToDo:
- Need manuals, schematics, etc.
- Artwork
- Everything

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "sound/beep.h"

class brandt8641_state : public driver_device
{
public:
	brandt8641_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pio1(*this, "pio1")
		, m_pio2(*this, "pio2")
		, m_pio3(*this, "pio3")
		, m_ctc(*this, "ctc")
		, m_io_keyboard(*this, "KEY")
		, m_beep(*this, "beeper")
	{ }

	DECLARE_READ8_MEMBER(port08_r);
	DECLARE_WRITE8_MEMBER(port08_w);
	DECLARE_WRITE8_MEMBER(port09_w);
	DECLARE_MACHINE_RESET(br8641);

private:
	UINT8 m_port08;
	UINT8 m_port09;
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio1;
	required_device<z80pio_device> m_pio2;
	required_device<z80pio_device> m_pio3;
	required_device<z80ctc_device> m_ctc;
	required_ioport_array<8> m_io_keyboard;
	required_device<beep_device> m_beep;
};

static ADDRESS_MAP_START(brandt8641_mem, AS_PROGRAM, 8, brandt8641_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM // 27256 at U12
	AM_RANGE(0x8000, 0x9fff) AM_RAM // 8KB static ram 6264 at U12
ADDRESS_MAP_END

static ADDRESS_MAP_START(brandt8641_io, AS_IO, 8, brandt8641_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("pio1", z80pio_device, read, write)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("pio2", z80pio_device, read, write)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE("pio3", z80pio_device, read, write)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("ctc",  z80ctc_device, read, write)
	AM_RANGE(0x1E, 0x1F) AM_WRITENOP  // unknown device
ADDRESS_MAP_END

/* Input ports */
// No idea what each key does
static INPUT_PORTS_START( brandt8641 )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)

	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)

	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)

	PORT_START("KEY.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)

	PORT_START("KEY.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)

	PORT_START("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)

	PORT_START("KEY.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)
INPUT_PORTS_END

READ8_MEMBER( brandt8641_state::port08_r )
{
	UINT8 i, data = 7;

	for (i = 0; i < 8; i++)
		if BIT(m_port09, i)
			data &= m_io_keyboard[i]->read();

	return data | m_port08;
}

WRITE8_MEMBER( brandt8641_state::port08_w )
{
	m_port08 = data & 0xf8;
	m_beep->set_state(BIT(data, 4));
}

WRITE8_MEMBER( brandt8641_state::port09_w )
{
	m_port09 = data ^ 0xff;
}

MACHINE_RESET_MEMBER( brandt8641_state, br8641 )
{
	m_beep->set_frequency(2000);
}

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "pio1" },
	{ "pio2" },
	{ "pio3" },
	{ "ctc" },
	{ nullptr }
};



static MACHINE_CONFIG_START( brandt8641, brandt8641_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_4MHz) // U4 ,4MHz crystal on board
	MCFG_CPU_PROGRAM_MAP(brandt8641_mem)
	MCFG_CPU_IO_MAP(brandt8641_io)
	MCFG_CPU_CONFIG(daisy_chain_intf)

	MCFG_MACHINE_RESET_OVERRIDE(brandt8641_state, br8641)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	// Z80APIO U9
	// Z80APIO U14
	// Z80PIO U7 - unknown which is which
	MCFG_DEVICE_ADD("pio1", Z80PIO, XTAL_4MHz)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("pio2", Z80PIO, XTAL_4MHz)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(brandt8641_state, port08_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(brandt8641_state, port08_w))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(brandt8641_state, port09_w))

	MCFG_DEVICE_ADD("pio3", Z80PIO, XTAL_4MHz)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL_4MHz) // Z80CTC U8
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( br8641 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "v0911he.u11", 0x0000, 0x8000, CRC(59a16951) SHA1(893dba60ec8bfa391fb2d2a30db5d42d601f5eb9))
ROM_END

/* Driver */
COMP( 1986, br8641, 0, 0, brandt8641, brandt8641, driver_device, 0, "Brandt", "Brandt 8641", MACHINE_NOT_WORKING )
