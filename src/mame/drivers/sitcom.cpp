// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    SITCOM (known as Sitcom, Sitcom85, Sitcom8085)

    25/09/2011 Driver [Robbbert]

    http://www.izabella.me.uk/html/sitcom_.html
    http://www.sbprojects.com/sitcom/sitcom.htm

    The display consists of a LED connected to SOD, and a pair of
    DL1414 intelligent alphanumeric displays.

    The idea of this device is that you write a 8085 program with an
    assembler on your PC. You then compile it, and then send it to
    the SITCOM via a serial cable. The program then (hopefully) runs
    on the SITCOM. With the 8255 expansion, you could wire up input
    devices or other hardware for your program to use.

    The SOD LED blinks slowly while waiting; stays on while downloading;
    and blinks quickly if an error occurs.

    After a successful download, the ROM is switched out and the RAM
    mirrored to the lower 32k. The downloaded program is then executed.
    This part is not emulated.

    In MESS, start emulation. After about 10 seconds the display will
    scroll sideways with a message and a weblink. There are no input keys.

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "video/dl1416.h"
#include "sitcom.lh"



class sitcom_state : public driver_device
{
public:
	sitcom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_ds0(*this, "ds0"),
	m_ds1(*this, "ds1")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<dl1416_device> m_ds0;
	required_device<dl1416_device> m_ds1;
	DECLARE_WRITE_LINE_MEMBER(sod_led);
	DECLARE_READ_LINE_MEMBER(sid_line);
	virtual void machine_reset() override;
	DECLARE_WRITE16_MEMBER(sitcom_update_ds0);
	DECLARE_WRITE16_MEMBER(sitcom_update_ds1);
};

static ADDRESS_MAP_START( sitcom_mem, AS_PROGRAM, 8, sitcom_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sitcom_io, AS_IO, 8, sitcom_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	// AM_RANGE(0x00, 0x1f) 8255 for expansion only
	AM_RANGE(0xc0, 0xc3) AM_DEVWRITE("ds0", dl1416_device, data_w) //left display
	AM_RANGE(0xe0, 0xe3) AM_DEVWRITE("ds1", dl1416_device, data_w) //right display
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( sitcom )
INPUT_PORTS_END

void sitcom_state::machine_reset()
{
	m_ds0->ce_w(0); // enable
	m_ds0->wr_w(0);
	m_ds0->cu_w(1); // no cursor
	m_ds1->ce_w(0);
	m_ds1->wr_w(0);
	m_ds1->cu_w(1);
}

WRITE16_MEMBER(sitcom_state::sitcom_update_ds0)
{
	output_set_digit_value(offset, data);
}

WRITE16_MEMBER(sitcom_state::sitcom_update_ds1)
{
	output_set_digit_value(4 + offset, data);
}

// SID line used as serial input from a pc
READ_LINE_MEMBER( sitcom_state::sid_line )
{
	return 1; //idle - changing to 0 gives a FR ERROR
}

WRITE_LINE_MEMBER( sitcom_state::sod_led )
{
	output_set_value("sod_led", state);
}

static MACHINE_CONFIG_START( sitcom, sitcom_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8085A, XTAL_6_144MHz) // 3.072MHz can be used for an old slow 8085
	MCFG_CPU_PROGRAM_MAP(sitcom_mem)
	MCFG_CPU_IO_MAP(sitcom_io)
	MCFG_I8085A_SID(READLINE(sitcom_state, sid_line))
	MCFG_I8085A_SOD(WRITELINE(sitcom_state, sod_led))

	MCFG_DEFAULT_LAYOUT(layout_sitcom)

	/* video hardware */
	MCFG_DEVICE_ADD("ds0", DL1416B, 0)
	MCFG_DL1416_UPDATE_HANDLER(WRITE16(sitcom_state, sitcom_update_ds0))
	MCFG_DEVICE_ADD("ds1", DL1416B, 0)
	MCFG_DL1416_UPDATE_HANDLER(WRITE16(sitcom_state, sitcom_update_ds1))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( sitcom )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "boot8085.bin", 0x0000, 0x06b8, CRC(1b5e3310) SHA1(3323b65f0c10b7ab6bb75ec824e6d5fb643693a8))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT               COMPANY              FULLNAME       FLAGS */
COMP( 2002, sitcom, 0,      0,       sitcom,    sitcom, driver_device,  0,   "San Bergmans & Izabella Malcolm", "Sitcom", MACHINE_NO_SOUND_HW)
