/***************************************************************************

    Altos 5-15

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"
#include "machine/z80dma.h"
#include "machine/serial.h"
#include "machine/terminal.h"


class altos5_state : public driver_device
{
public:
	altos5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pio0(*this, "z80pio_0"),
		m_pio1(*this, "z80pio_1"),
		m_dart(*this, "z80dart"),
		m_sio (*this, "z80sio"),
		m_ctc (*this, "z80ctc"),
		m_terminal(*this, TERMINAL_TAG)
	{ }

	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
	DECLARE_READ8_MEMBER(port08_r);
	DECLARE_READ8_MEMBER(port09_r);
	DECLARE_WRITE8_MEMBER(port08_w);
	DECLARE_WRITE8_MEMBER(port09_w);
	DECLARE_DRIVER_INIT(altos5);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(port2e_r);
	DECLARE_READ8_MEMBER(port2f_r);
	UINT8 m_term_data;
	UINT8 m_port08;
	UINT8 m_port09;
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio0;
	required_device<z80pio_device> m_pio1;
	required_device<z80dart_device> m_dart;
	optional_device<z80dart_device> m_sio;
	required_device<z80ctc_device> m_ctc;
	required_device<generic_terminal_device> m_terminal;
};

static ADDRESS_MAP_START(altos5_mem, AS_PROGRAM, 8, altos5_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_RAMBANK("bank1")
	AM_RANGE(0x1000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(altos5_io, AS_IO, 8, altos5_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE_LEGACY("z80dma", z80dma_r, z80dma_w)
	//AM_RANGE(0x04, 0x07) // FD1797 fdc
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("z80pio_0", z80pio_device, read, write)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE("z80ctc", z80ctc_device, read, write)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("z80pio_1", z80pio_device, read, write)
	//AM_RANGE(0x14, 0x17) AM_WRITE(romoff_w)
	AM_RANGE(0x1c, 0x1f) AM_DEVREADWRITE("z80dart", z80dart_device, ba_cd_r, ba_cd_w)
	//AM_RANGE(0x20, 0x23) // Hard drive
	AM_RANGE(0x2c, 0x2d) AM_NOP
	AM_RANGE(0x2e, 0x2e) AM_READ(port2e_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x2f, 0x2f) AM_READ(port2f_r) AM_WRITENOP
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( altos5 )
INPUT_PORTS_END

void altos5_state::machine_reset()
{
	membank("bank1")->set_entry(1);
}

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "z80dma" },
	{ "z80pio_0" },
	{ "z80pio_1" },
	{ "z80ctc" },
	{ "z80dart" },
//	{ "z80sio" },
	{ NULL }
};

/*
d0: L = a HD is present
d1: L = a 2nd hard drive is present
d2: unused configuration input (must be H to skip HD boot)
d3: selected floppy is single(L) or double sided(H) 
d7: IRQ from FDC
*/
READ8_MEMBER( altos5_state::port08_r )
{
	return m_port08 | 0x87;
}

/*
d0: HD IRQ
*/
READ8_MEMBER( altos5_state::port09_r )
{
	return m_port09 | 0x01;
}

/*
d4: DDEN (H = double density
d5: DS (H = drive 2)
d6: SS (H = side 2)
*/
WRITE8_MEMBER( altos5_state::port08_w )
{
	m_port08 = data;
}

/*
d1, 2: Memory Map template selection (0 = diag; 1 = oasis; 2 = mp/m)
d3, 4: CPU bank select
d5:    H = Write protect of common area
d6, 7: DMA bank select
*/
WRITE8_MEMBER( altos5_state::port09_w )
{
	m_port09 = data;
}

READ8_MEMBER(altos5_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(altos5_state::memory_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}

READ8_MEMBER(altos5_state::io_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(altos5_state::io_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.write_byte(offset, data);
}

static Z80DMA_INTERFACE( dma_intf )
{
	DEVCB_NULL, //DEVCB_DRIVER_LINE_MEMBER(altos5_state, p8k_dma_irq_w),
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(altos5_state, memory_read_byte),
	DEVCB_DRIVER_MEMBER(altos5_state, memory_write_byte),
	DEVCB_DRIVER_MEMBER(altos5_state, io_read_byte),
	DEVCB_DRIVER_MEMBER(altos5_state, io_write_byte)
};

// baud rate generator and RTC. All inputs are 2MHz.
static Z80CTC_INTERFACE( ctc_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0), // interrupt callback
	DEVCB_NULL,         /* ZC/TO0 callback - SIO Ch B */
	DEVCB_NULL,         /* ZC/TO1 callback - Z80DART Ch A, SIO Ch A */
	DEVCB_NULL,         /* ZC/TO2 callback - Z80DART Ch B */
};

// system functions
static Z80PIO_INTERFACE( pio0_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0), // interrupt callback
	DEVCB_DRIVER_MEMBER(altos5_state, port08_r),         /* read port A */
	DEVCB_DRIVER_MEMBER(altos5_state, port08_w),         /* write port A */
	DEVCB_NULL,         /* portA ready active callback */
	DEVCB_DRIVER_MEMBER(altos5_state, port09_r),         /* read port B */
	DEVCB_DRIVER_MEMBER(altos5_state, port09_w),         /* write port B */
	DEVCB_NULL          /* portB ready active callback */
};

// parallel port
static Z80PIO_INTERFACE( pio1_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0), // interrupt callback
	DEVCB_NULL,         /* read port A */
	DEVCB_NULL,         /* write port A */
	DEVCB_NULL,         /* portA ready active callback */
	DEVCB_NULL,         /* read port B */
	DEVCB_NULL,         /* write port B */
	DEVCB_NULL          /* portB ready active callback */
};

// serial printer and console#3
static Z80DART_INTERFACE( dart_intf )
{
	0, 0, 0, 0,

	// console#3
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	// printer
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


DRIVER_INIT_MEMBER( altos5_state, altos5 )
{
	UINT8 *RAM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 2, &RAM[0], 0x10000);
}

READ8_MEMBER( altos5_state::port2e_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( altos5_state::port2f_r )
{
	return (m_term_data) ? 13 : 12;
}

WRITE8_MEMBER( altos5_state::kbd_put )
{
	m_term_data = data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(altos5_state, kbd_put)
};

static MACHINE_CONFIG_START( altos5, altos5_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_8MHz / 2)
	MCFG_CPU_PROGRAM_MAP(altos5_mem)
	MCFG_CPU_IO_MAP(altos5_io)
	MCFG_CPU_CONFIG(daisy_chain_intf)

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)

	/* Devices */
	MCFG_Z80DMA_ADD( "z80dma",   XTAL_8MHz / 2, dma_intf)
	MCFG_Z80PIO_ADD( "z80pio_0", XTAL_8MHz / 2, pio0_intf )
	MCFG_Z80PIO_ADD( "z80pio_1", XTAL_8MHz / 2, pio1_intf )
	MCFG_Z80CTC_ADD( "z80ctc",   XTAL_8MHz / 2, ctc_intf )
	MCFG_Z80DART_ADD("z80dart",  XTAL_8MHz / 2, dart_intf )
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( altos5 )
	ROM_REGION( 0x11000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("2732.bin",   0x10000, 0x1000, CRC(15fdc7eb) SHA1(e15bdf5d5414ad56f8c4bb84edc6f967a5f01ba9)) // bios
	ROM_FILL(0x10054, 2, 0) // temp until banking sorted out
	ROM_FILL(0x10344, 3, 0) // kill self test

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD("82s141.bin", 0x0000, 0x0200, CRC(35c8078c) SHA1(dce24374bfcc5d23959e2c03485d82a119c0c3c9)) // banking control
ROM_END

/* Driver */

/*   YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT   CLASS           INIT    COMPANY    FULLNAME       FLAGS */
COMP(1982, altos5, 0,      0,       altos5,  altos5, altos5_state,  altos5, "Altos", "Altos 5-15", GAME_NOT_WORKING | GAME_NO_SOUND)




#if 0
/****************** UNUSED SERIAL CODE ************************/

	//AM_RANGE(0x2c, 0x2f) AM_DEVREADWRITE("z80sio", z80dart_device, ba_cd_r, ba_cd_w)

static Z80SIO_INTERFACE( sio_intf )
{
	9600, 9600, 153600, 153600, // rxa, txa, rxb, txb clocks (from CTC)

	// console#2
	DEVCB_NULL, // ChA in data
	DEVCB_NULL, // out data
	DEVCB_NULL, // DTR
	DEVCB_NULL, // RTS
	DEVCB_NULL, // WRDY
	DEVCB_NULL, // SYNC

	// console#1
	DEVCB_DEVICE_LINE_MEMBER("rs232", serial_port_device, rx),
	DEVCB_DEVICE_LINE_MEMBER("rs232", serial_port_device, tx),
	DEVCB_DEVICE_LINE_MEMBER("rs232", rs232_port_device, dtr_w),
	DEVCB_DEVICE_LINE_MEMBER("rs232", rs232_port_device, rts_w),
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_NULL, // unused DRQ pins
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static const rs232_port_interface rs232_intf =
{
	// all outputs
	DEVCB_NULL,//DEVCB_DEVICE_LINE_MEMBER("z80sio", z80dart_device, rxcb_w), // data
	DEVCB_NULL, // DCD
	DEVCB_NULL, // DSR
	DEVCB_NULL, // RI
	DEVCB_NULL //DEVCB_DEVICE_LINE_MEMBER("z80sio", z80dart_device, ctsb_w) // CTS
};


	//MCFG_Z80SIO0_ADD("z80sio",   XTAL_8MHz / 2, sio_intf )
	//MCFG_RS232_PORT_ADD("rs232", rs232_intf, default_rs232_devices, "serial_terminal")


#endif

