// license:BSD-3-Clause
// copyright-holders:Robbbert,Vas Crabb
/***************************************************************************

    SITCOM (known as Sitcom, Sitcom85, Sitcom8085)

    25/09/2011 Driver [Robbbert]

    http://www.sitcom.tk/
    http://www.sbprojects.net/projects/izabella/html/sitcom_.html

    The display consists of a LED connected to SOD, and a pair of DL1414
    intelligent alphanumeric displays.

    The idea of this device is that you write a 8085 program with an
    assembler on your PC.  You then compile it, and then send it to the
    SITCOM via a serial cable. The program then (hopefully) runs on the
    SITCOM.  With the 8255 expansion, you could wire up input devices or
    other hardware for your program to use.

    The SOD LED blinks slowly while waiting; stays on while downloading;
    and blinks quickly if an error occurs.

    After a successful upload, hit the Reset button to mirror RAM into
    the low 32kB of the address space in place of ROM and run the
    program.

    The null modem bitbanger should be configured for 9600 8N1 to upload
    a program.  The data should be in Intel HEX format.

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"

#include "cpu/i8085/i8085.h"

#include "machine/clock.h"
#include "machine/bankdev.h"
#include "machine/i8255.h"

#include "video/dl1416.h"

#include "softlist_dev.h"

#include "sitcom.lh"


namespace {

class sitcom_state : public driver_device
{
public:
	sitcom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_buttons(*this, "BUTTONS")
		, m_maincpu(*this, "maincpu")
		, m_bank(*this, "bank")
		, m_rxd(true)
	{
	}

	template <unsigned D> DECLARE_WRITE16_MEMBER(update_ds) { output().set_digit_value((D << 2) | offset, data); }
	DECLARE_WRITE_LINE_MEMBER(update_rxd)                   { m_rxd = bool(state); }
	DECLARE_WRITE_LINE_MEMBER(sod_led)                      { output().set_value("sod_led", state); }
	DECLARE_READ_LINE_MEMBER(sid_line)                      { return m_rxd ? 1 : 0; }

	DECLARE_WRITE8_MEMBER(update_pia_pa);
	DECLARE_WRITE8_MEMBER(update_pia_pb);

	DECLARE_INPUT_CHANGED_MEMBER(buttons);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_ioport                          m_buttons;
	required_device<cpu_device>              m_maincpu;
	required_device<address_map_bank_device> m_bank;

	bool m_rxd;
};

ADDRESS_MAP_START( sitcom_bank, AS_PROGRAM, 8, sitcom_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x8000, 0xffff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END

ADDRESS_MAP_START( sitcom_mem, AS_PROGRAM, 8, sitcom_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_DEVICE("bank", address_map_bank_device, amap8)
	AM_RANGE(0x8000, 0xffff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END

ADDRESS_MAP_START( sitcom_io, AS_IO, 8, sitcom_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x1c) AM_DEVREADWRITE("pia", i8255_device, read, write)
	AM_RANGE(0xc0, 0xc3) AM_MIRROR(0x1c) AM_DEVWRITE("ds0", dl1414_device, bus_w)
	AM_RANGE(0xe0, 0xe3) AM_MIRROR(0x1c) AM_DEVWRITE("ds1", dl1414_device, bus_w)
ADDRESS_MAP_END


INPUT_PORTS_START( sitcom )
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Boot")  PORT_CHANGED_MEMBER(DEVICE_SELF, sitcom_state, buttons, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, sitcom_state, buttons, 0)
INPUT_PORTS_END


void sitcom_state::machine_start()
{
	save_item(NAME(m_rxd));

	m_rxd = true;
}

void sitcom_state::machine_reset()
{
	m_bank->set_bank(0);
}

WRITE8_MEMBER( sitcom_state::update_pia_pa )
{
	for (int i = 0; 8 > i; ++i)
		output().set_indexed_value("pa", i, BIT(data, i));
}

WRITE8_MEMBER( sitcom_state::update_pia_pb )
{
	for (int i = 0; 8 > i; ++i)
		output().set_indexed_value("pb", i, BIT(data, i));
}

INPUT_CHANGED_MEMBER( sitcom_state::buttons )
{
	bool const boot(BIT(m_buttons->read(), 0));
	bool const reset(BIT(m_buttons->read(), 1));

	m_maincpu->set_input_line(INPUT_LINE_RESET, (boot || reset) ? ASSERT_LINE : CLEAR_LINE);

	if (boot)
		m_bank->set_bank(0);
	else if (reset)
		m_bank->set_bank(1);
}


MACHINE_CONFIG_START( sitcom, sitcom_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", I8085A, XTAL_6_144MHz) // 3.072MHz can be used for an old slow 8085
	MCFG_CPU_PROGRAM_MAP(sitcom_mem)
	MCFG_CPU_IO_MAP(sitcom_io)
	MCFG_I8085A_SID(READLINE(sitcom_state, sid_line))
	MCFG_I8085A_SOD(WRITELINE(sitcom_state, sod_led))

	MCFG_DEVICE_ADD("bank", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(sitcom_bank)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x8000)

	MCFG_CLOCK_ADD("100hz", 100)
	MCFG_CLOCK_SIGNAL_HANDLER(INPUTLINE("maincpu", I8085_RST75_LINE))

	MCFG_DEVICE_ADD("pia", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(sitcom_state, update_pia_pa))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(sitcom_state, update_pia_pb))

	// video hardware
	MCFG_DEVICE_ADD("ds0", DL1414T, 0) // left display
	MCFG_DL1414_UPDATE_HANDLER(WRITE16(sitcom_state, update_ds<0>))
	MCFG_DEVICE_ADD("ds1", DL1414T, 0) // right display
	MCFG_DL1414_UPDATE_HANDLER(WRITE16(sitcom_state, update_ds<1>))

	// host interface
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "null_modem")
	MCFG_RS232_RXD_HANDLER(WRITELINE(sitcom_state, update_rxd))

	MCFG_SOFTWARE_LIST_ADD("bitb_list", "sitcom")
	MCFG_DEFAULT_LAYOUT(layout_sitcom)
MACHINE_CONFIG_END


ROM_START( sitcom )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "boot8085.bin", 0x0000, 0x06b8, CRC(1b5e3310) SHA1(3323b65f0c10b7ab6bb75ec824e6d5fb643693a8) )
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   STATE          INIT  COMPANY                            FULLNAME  FLAGS */
COMP( 2002, sitcom, 0,      0,      sitcom,  sitcom, driver_device, 0,    "San Bergmans & Izabella Malcolm", "Sitcom", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW)
