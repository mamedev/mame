// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************************

Onyx C8002

2013-08-18 Skeleton Driver

Copied from p8k.c

The C8002 is one of the earliest minicomputers to use Unix as an operating system.

The system consists of a main CPU (Z8002), and a slave CPU for Mass Storage control (Z80)

The Z80 board contains a 19.6608 and 16 MHz crystals; 2x Z80CTC; 3x Z80SIO/0; Z80DMA; 3x Z80PIO;
2 eproms marked 459-3 and 460-3, plus 2 proms.

The Z8002 board contains a 16 MHz crystal; 3x Z80CTC; 5x Z80SIO/0; 3x Z80PIO; 2 eproms marked
466-E and 467E, plus the remaining 7 small proms.

The system can handle 8 RS232 terminals, 7 hard drives, a tape cartridge drive, parallel i/o,
and be connected to a RS422 network.

*************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z8000/z8000.h"
//#include "cpu/z80/z80daisy.h"
//#include "machine/z80ctc.h"
//#include "machine/z80pio.h"
//#include "machine/z80dart.h"
//#include "machine/z80dma.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class onyx_state : public driver_device
{
public:
	onyx_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	DECLARE_MACHINE_RESET(c8002);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(portff05_r);

private:
	UINT8 m_term_data;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};


READ8_MEMBER( onyx_state::portff05_r )
{
	//return m_term_data;

	return 4;
}

WRITE8_MEMBER( onyx_state::kbd_put )
{
	m_term_data = data;
}


/* Input ports */
static INPUT_PORTS_START( c8002 )
INPUT_PORTS_END


MACHINE_RESET_MEMBER(onyx_state, c8002)
{
}

static ADDRESS_MAP_START(c8002_mem, AS_PROGRAM, 16, onyx_state)
	AM_RANGE(0x00000, 0x00fff) AM_ROM AM_SHARE("share0")
	AM_RANGE(0x01000, 0x07fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x08000, 0xfffff) AM_RAM AM_SHARE("share2")
ADDRESS_MAP_END

//static ADDRESS_MAP_START(c8002_data, AS_DATA, 16, onyx_state)
//  AM_RANGE(0x00000, 0x00fff) AM_ROM AM_SHARE("share0")
//  AM_RANGE(0x01000, 0x07fff) AM_RAM AM_SHARE("share1")
//  AM_RANGE(0x08000, 0xfffff) AM_RAM AM_SHARE("share2")
//ADDRESS_MAP_END

static ADDRESS_MAP_START(c8002_io, AS_IO, 8, onyx_state)
	AM_RANGE(0xff00, 0xff01)  AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0xff04, 0xff05) AM_READ(portff05_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START(submem, AS_PROGRAM, 8, onyx_state)
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(subio, AS_IO, 8, onyx_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


/***************************************************************************

    Machine Drivers

****************************************************************************/

static MACHINE_CONFIG_START( c8002, onyx_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z8002, XTAL_4MHz )
	//MCFG_CPU_CONFIG(main_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(c8002_mem)
	//MCFG_CPU_DATA_MAP(c8002_data)
	MCFG_CPU_IO_MAP(c8002_io)

	MCFG_CPU_ADD("subcpu", Z80, XTAL_4MHz )
	//MCFG_CPU_CONFIG(sub_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(submem)
	MCFG_CPU_IO_MAP(subio)
	MCFG_MACHINE_RESET_OVERRIDE(onyx_state, c8002)

	/* peripheral hardware */
	//MCFG_DEVICE_ADD("z80ctc_0", Z80CTC, XTAL_4MHz)
	//MCFG_DEVICE_ADD("z80ctc_1", Z80CTC, XTAL_4MHz)
	//MCFG_Z80SIO0_ADD("z80sio_0", 9600, 0, 0, 0, 0)
	//MCFG_Z80SIO0_ADD("z80sio_1", 9600, 0, 0, 0, 0)
	//MCFG_DEVICE_ADD("z80pio_0", Z80CTC, XTAL_4MHz)
	//MCFG_DEVICE_ADD("z80pio_1", Z80CTC, XTAL_4MHz)
	//MCFG_DEVICE_ADD("z80pio_2", Z80CTC, XTAL_4MHz)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(onyx_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( c8002 )
	ROM_REGION16_BE( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("466-e", 0x0001, 0x0800, CRC(13534bcb) SHA1(976c76c69af40b0c0a5038e428a10b39a619a036))
	ROM_LOAD16_BYTE("467-e", 0x0000, 0x0800, CRC(0d5b557f) SHA1(0802bc6c2774f4e7de38a9c92e8558d710eed287))

	ROM_REGION( 0x10000, "subcpu", 0 )
	ROM_LOAD("459-3",   0x0000, 0x0800, CRC(c8906653) SHA1(7dea9fffa974479ef5926df567261f2aaa7a3283))
	ROM_LOAD("460-3",   0x0800, 0x0800, CRC(ce6c0214) SHA1(f69ee4c6c0d1e72574a9cf828dbb3e08f06d029a))

	ROM_REGION( 0x900, "proms", 0 )
	// for main cpu
	ROM_LOAD("468-a",  0x000, 0x100, CRC(89781491) SHA1(f874d0cf42a733eb2b92b15647aeac7178d7b9b1))
	ROM_LOAD("469-a",  0x100, 0x100, CRC(45e439de) SHA1(4f1af44332ae709d92e919c9e48433f29df5e632))
	ROM_LOAD("470a-3", 0x200, 0x100, CRC(c50622a9) SHA1(deda0df93fc4e4b5f4be313e4bfe0c5fc669a024))
	ROM_LOAD("471-a",  0x300, 0x100, CRC(c09ca06b) SHA1(cb99172f5342427c68a109ee108a0c49b44e7010))
	ROM_LOAD("472-a",  0x400, 0x100, CRC(e1316fed) SHA1(41ed2d822c74da4e1ce06eb229629576e7f5035f))
	ROM_LOAD("473-a",  0x500, 0x100, CRC(5e8efd7f) SHA1(647064e0c3b0d795a333febc57228472b1b32345))
	ROM_LOAD("474-a",  0x600, 0x100, CRC(0052edfd) SHA1(b5d18c9a6adce7a6d627ece40a60aab8c55a6597))
	// for sub cpu
	ROM_LOAD("453-a",  0x700, 0x100, CRC(7bc3871e) SHA1(6f75eb04911fa1ff66714276b8a88be62438a1b0))
	ROM_LOAD("454-a",  0x800, 0x100, CRC(aa2233cd) SHA1(4ec3a8c06cccda02f080e89831ecd8a9c96d3650))
ROM_END

/* Driver */

/*    YEAR  NAME   PARENT  COMPAT   MACHINE    INPUT  CLASS          INIT  COMPANY  FULLNAME       FLAGS */
COMP( 1982, c8002, 0,      0,       c8002,     c8002, driver_device, 0,     "Onyx", "C8002", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
