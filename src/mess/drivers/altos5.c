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


class altos5_state : public driver_device
{
public:
	altos5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pio0(*this, "z80pio_0"),
		m_pio1(*this, "z80pio_1"),
		m_sio1(*this, "z80sio_1"),
		m_sio2(*this, "z80sio_2"),
		m_ctc(*this, "z80ctc")
	{ }

	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
	DECLARE_DRIVER_INIT(altos5);
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio0;
	required_device<z80pio_device> m_pio1;
	required_device<z80sio0_device> m_sio1;
	required_device<z80sio0_device> m_sio2;
	required_device<z80ctc_device> m_ctc;
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
	AM_RANGE(0x1c, 0x1f) AM_DEVREADWRITE("z80sio_1", z80sio0_device, cd_ba_r, cd_ba_w)
	AM_RANGE(0x2c, 0x2f) AM_DEVREADWRITE("z80sio_2", z80sio0_device, cd_ba_r, cd_ba_w)
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
	{ "z80sio_1" },
	{ "z80sio_2" },
	{ NULL }
};

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

static Z80CTC_INTERFACE( ctc_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0), // interrupt callback
	DEVCB_NULL,         /* ZC/TO0 callback - SIO channel A clock */
	DEVCB_NULL,         /* ZC/TO1 callback - SIO channel B clock */
	DEVCB_NULL,//DRIVER_LINE_MEMBER(altos5_state, pcm_82_w) /* ZC/TO2 callback - speaker */
};

static Z80PIO_INTERFACE( pio0_intf ) // all pins go to expansion socket
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0), // interrupt callback
	DEVCB_NULL,         /* read port A */
	DEVCB_NULL,         /* write port A */
	DEVCB_NULL,         /* portA ready active callback */
	DEVCB_NULL,         /* read port B */
	DEVCB_NULL,         /* write port B */
	DEVCB_NULL          /* portB ready active callback */
};

static Z80PIO_INTERFACE( pio1_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0), // interrupt callback
	DEVCB_NULL,//DEVICE_MEMBER(K7659_KEYBOARD_TAG, k7659_keyboard_device, read),           /* read port A */
	DEVCB_NULL,         /* write port A */
	DEVCB_NULL,         /* portA ready active callback */
	DEVCB_NULL,//DRIVER_MEMBER(altos5_state, pcm_85_r),           /* read port B */
	DEVCB_NULL,//DRIVER_MEMBER(altos5_state, pcm_85_w),           /* write port B */
	DEVCB_NULL          /* portB ready active callback */
};

static Z80SIO_INTERFACE( sio1_intf )
{
	0, 0, 0, 0,

	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL
};

static Z80SIO_INTERFACE( sio2_intf )
{
	0, 0, 0, 0,

	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
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

static MACHINE_CONFIG_START( altos5, altos5_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_8MHz / 2)
	MCFG_CPU_PROGRAM_MAP(altos5_mem)
	MCFG_CPU_IO_MAP(altos5_io)
	MCFG_CPU_CONFIG(daisy_chain_intf)

	/* video hardware */

	/* Devices */
	MCFG_Z80DMA_ADD( "z80dma", XTAL_8MHz / 2, dma_intf)
	MCFG_Z80PIO_ADD( "z80pio_0", XTAL_8MHz / 2, pio0_intf )
	MCFG_Z80PIO_ADD( "z80pio_1", XTAL_8MHz / 2, pio1_intf )
	MCFG_Z80SIO0_ADD("z80sio_1", XTAL_8MHz / 2, sio1_intf )
	MCFG_Z80SIO0_ADD("z80sio_2", XTAL_8MHz / 2, sio2_intf )
	MCFG_Z80CTC_ADD( "z80ctc", XTAL_8MHz / 2, ctc_intf )
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( altos5 )
	ROM_REGION( 0x11000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("2732.bin",   0x10000, 0x1000, CRC(15fdc7eb) SHA1(e15bdf5d5414ad56f8c4bb84edc6f967a5f01ba9)) // bios
	ROM_FILL(0x10054, 2, 0) // temp until banking sorted out
	ROM_FILL(0x10344, 3, 0) // kill self test

	ROM_REGION( 0x200, "user1", 0 )
	ROM_LOAD("82s141.bin", 0x0000, 0x0200, CRC(35c8078c) SHA1(dce24374bfcc5d23959e2c03485d82a119c0c3c9)) // looks like a keyboard matrix
ROM_END

/* Driver */

/*   YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT   CLASS           INIT    COMPANY    FULLNAME       FLAGS */
COMP(1982, altos5, 0,      0,       altos5,  altos5, altos5_state,  altos5, "Altos", "Altos 5-15", GAME_NOT_WORKING | GAME_NO_SOUND)
