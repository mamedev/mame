// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************************

Onyx C8002

2013-08-18 Skeleton Driver

The C8002 is one of the earliest minicomputers to use Unix as an operating system.

The system consists of a main CPU (Z8002), and a slave CPU for Mass Storage control (Z80)

The Z80 board contains a 19.6608 and 16 MHz crystals; 2x Z80CTC; 3x Z80SIO/0; Z80DMA; 3x Z80PIO;
2 eproms marked 459-3 and 460-3, plus 2 proms.

The Z8002 board contains a 16 MHz crystal; 3x Z80CTC; 5x Z80SIO/0; 3x Z80PIO; 2 eproms marked
466-E and 467E, plus the remaining 7 small proms.

The system can handle 8 RS232 terminals, 7 hard drives, a tape cartridge drive, parallel i/o,
and be connected to a RS422 network.

Status:
- Main screen prints an error with CTC (because there's no clock into it atm)
- Subcpu screen (after a while) prints various statuses then waits for the fdc to respond

To Do:
- Hook up daisy chains (see p8k.cpp for how to hook up a 16-bit chain)
  (keyboard input depends on interrupts)
- Remaining devices
- Whatever hooks up to the devices
- Eventually we'll need software
- Manuals / schematics would be nice

*************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z8000/z8000.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"
//#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
//#include "machine/z80dma.h"
#include "machine/terminal.h"

class onyx_state : public driver_device
{
public:
	onyx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sio1(*this, "sio1")
		, m_sio2(*this, "sio2")
		, m_sio3(*this, "sio3")
		, m_sio4(*this, "sio4")
		, m_sio5(*this, "sio5")
		, m_ctc1(*this, "ctc1")
		, m_ctc2(*this, "ctc2")
		, m_ctc3(*this, "ctc3")
		, m_pio1(*this, "pio1")
		, m_pio2(*this, "pio2")
	{ }

	DECLARE_MACHINE_RESET(c8002);

	void c8002(machine_config &config);
	void c5000(machine_config &config);
	void c5000_io(address_map &map);
	void c5000_mem(address_map &map);
	void c8002_io(address_map &map);
	void c8002_mem(address_map &map);
	void subio(address_map &map);
	void submem(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
	optional_device<z80sio_device> m_sio1, m_sio2, m_sio3, m_sio4, m_sio5;
	optional_device<z80ctc_device> m_ctc1, m_ctc2, m_ctc3;
	optional_device<z80pio_device> m_pio1, m_pio2;
};


/* Input ports */
static INPUT_PORTS_START( c8002 )
INPUT_PORTS_END


MACHINE_RESET_MEMBER(onyx_state, c8002)
{
}

ADDRESS_MAP_START(onyx_state::c8002_mem)
	AM_RANGE(0x00000, 0x00fff) AM_ROM AM_SHARE("share0")
	AM_RANGE(0x01000, 0x07fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x08000, 0x0ffff) AM_RAM AM_SHARE("share2") // Z8002 has 64k memory
ADDRESS_MAP_END

//static ADDRESS_MAP_START(c8002_data, AS_DATA, 16, onyx_state)
//  AM_RANGE(0x00000, 0x00fff) AM_ROM AM_SHARE("share0")
//  AM_RANGE(0x01000, 0x07fff) AM_RAM AM_SHARE("share1")
//  AM_RANGE(0x08000, 0xfffff) AM_RAM AM_SHARE("share2")
//ADDRESS_MAP_END

ADDRESS_MAP_START(onyx_state::c8002_io)
	map(0xff00, 0xff07).lrw8("sio1_rw", [this](address_space &space, offs_t offset, u8 mem_mask) { return m_sio1->cd_ba_r(space, offset >> 1, mem_mask); }, [this](address_space &space, offs_t offset, u8 data, u8 mem_mask) { m_sio1->cd_ba_w(space, offset >> 1, data, mem_mask); });
	map(0xff08, 0xff0f).lrw8("sio2_rw", [this](address_space &space, offs_t offset, u8 mem_mask) { return m_sio1->cd_ba_r(space, offset >> 1, mem_mask); }, [this](address_space &space, offs_t offset, u8 data, u8 mem_mask) { m_sio1->cd_ba_w(space, offset >> 1, data, mem_mask); });
	map(0xff10, 0xff17).lrw8("sio3_rw", [this](address_space &space, offs_t offset, u8 mem_mask) { return m_sio1->cd_ba_r(space, offset >> 1, mem_mask); }, [this](address_space &space, offs_t offset, u8 data, u8 mem_mask) { m_sio1->cd_ba_w(space, offset >> 1, data, mem_mask); });
	map(0xff18, 0xff1f).lrw8("sio4_rw", [this](address_space &space, offs_t offset, u8 mem_mask) { return m_sio1->cd_ba_r(space, offset >> 1, mem_mask); }, [this](address_space &space, offs_t offset, u8 data, u8 mem_mask) { m_sio1->cd_ba_w(space, offset >> 1, data, mem_mask); });
	map(0xff20, 0xff27).lrw8("sio5_rw", [this](address_space &space, offs_t offset, u8 mem_mask) { return m_sio1->cd_ba_r(space, offset >> 1, mem_mask); }, [this](address_space &space, offs_t offset, u8 data, u8 mem_mask) { m_sio1->cd_ba_w(space, offset >> 1, data, mem_mask); });
	map(0xff30, 0xff37).lrw8("ctc1_rw", [this](address_space &space, offs_t offset, u8 mem_mask) { return m_ctc1->read(space, offset >> 1, mem_mask); }, [this](address_space &space, offs_t offset, u8 data, u8 mem_mask) { m_ctc1->write(space, offset >> 1, data, mem_mask); });
	map(0xff38, 0xff3f).lrw8("ctc2_rw", [this](address_space &space, offs_t offset, u8 mem_mask) { return m_ctc2->read(space, offset >> 1, mem_mask); }, [this](address_space &space, offs_t offset, u8 data, u8 mem_mask) { m_ctc2->write(space, offset >> 1, data, mem_mask); });
	map(0xff40, 0xff47).lrw8("ctc3_rw", [this](address_space &space, offs_t offset, u8 mem_mask) { return m_ctc3->read(space, offset >> 1, mem_mask); }, [this](address_space &space, offs_t offset, u8 data, u8 mem_mask) { m_ctc3->write(space, offset >> 1, data, mem_mask); });
	map(0xff50, 0xff57).lrw8("pio1_rw", [this](address_space &space, offs_t offset, u8 mem_mask) { return m_pio1->read(space, offset >> 1, mem_mask); }, [this](address_space &space, offs_t offset, u8 data, u8 mem_mask) { m_pio1->write(space, offset >> 1, data, mem_mask); });
	map(0xff58, 0xff5f).lrw8("pio2_rw", [this](address_space &space, offs_t offset, u8 mem_mask) { return m_pio2->read(space, offset >> 1, mem_mask); }, [this](address_space &space, offs_t offset, u8 data, u8 mem_mask) { m_pio2->write(space, offset >> 1, data, mem_mask); });
ADDRESS_MAP_END

ADDRESS_MAP_START(onyx_state::submem)
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0xffff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(onyx_state::subio)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("pio1s", z80pio_device, read, write)
	AM_RANGE(0x04, 0x04) AM_READNOP   // disk status?
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE("sio1s", z80sio_device, cd_ba_r, cd_ba_w )
ADDRESS_MAP_END

/***************************************************************************

    Machine Drivers

****************************************************************************/

MACHINE_CONFIG_START(onyx_state::c8002)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z8002, XTAL(4'000'000) )
	//MCFG_Z80_DAISY_CHAIN(main_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(c8002_mem)
	//MCFG_CPU_DATA_MAP(c8002_data)
	MCFG_CPU_IO_MAP(c8002_io)

	MCFG_CPU_ADD("subcpu", Z80, XTAL(4'000'000) )
	//MCFG_Z80_DAISY_CHAIN(sub_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(submem)
	MCFG_CPU_IO_MAP(subio)
	MCFG_MACHINE_RESET_OVERRIDE(onyx_state, c8002)

	MCFG_DEVICE_ADD("sio1_clock", CLOCK, 307200)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("sio1", z80sio_device, rxca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("sio1" ,z80sio_device, txca_w))

	/* peripheral hardware */
	MCFG_DEVICE_ADD("pio1", Z80PIO, XTAL(16'000'000)/4)
	//MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_DEVICE_ADD("pio2", Z80PIO, XTAL(16'000'000)/4)
	//MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_DEVICE_ADD("ctc1", Z80CTC, XTAL(16'000'000) /4)
	MCFG_DEVICE_ADD("ctc2", Z80CTC, XTAL(16'000'000) /4)
	MCFG_DEVICE_ADD("ctc3", Z80CTC, XTAL(16'000'000) /4)
	MCFG_DEVICE_ADD("sio1", Z80SIO, XTAL(16'000'000) /4)
	MCFG_Z80SIO_OUT_TXDA_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRA_CB(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSA_CB(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_DEVICE_ADD("sio2", Z80SIO, XTAL(16'000'000) /4)
	MCFG_DEVICE_ADD("sio3", Z80SIO, XTAL(16'000'000) /4)
	MCFG_DEVICE_ADD("sio4", Z80SIO, XTAL(16'000'000) /4)
	MCFG_DEVICE_ADD("sio5", Z80SIO, XTAL(16'000'000) /4)

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sio1", z80sio_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("sio1", z80sio_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("sio1", z80sio_device, ctsa_w))

	MCFG_DEVICE_ADD("pio1s", Z80PIO, XTAL(16'000'000)/4)
	//MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("subcpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("sio1s_clock", CLOCK, 614400)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("sio1s", z80sio_device, rxtxcb_w))
	//MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("sio1s" ,z80sio_device, txca_w))

	MCFG_DEVICE_ADD("sio1s", Z80SIO, XTAL(16'000'000) /4)
	MCFG_Z80SIO_OUT_TXDB_CB(DEVWRITELINE("rs232s", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRB_CB(DEVWRITELINE("rs232s", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSB_CB(DEVWRITELINE("rs232s", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232s", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sio1s", z80sio_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("sio1s", z80sio_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("sio1s", z80sio_device, ctsb_w))
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

//    YEAR  NAME   PARENT  COMPAT   MACHINE    INPUT  CLASS       INIT  COMPANY          FULLNAME  FLAGS
COMP( 1982, c8002, 0,      0,       c8002,     c8002, onyx_state, 0,    "Onyx Systems",  "C8002",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )



/********************************************************************************************************************************

Onyx Systems C5000.
(says C8000 on the backplate)

Chips: 256k dynamic RAM, Z80A, Z80DMA, 5x Z80PIO, 2x Z80SIO/0, 2x Z80CTC, 5? undumped proms, 3 red leds, 1x 4-sw DIP
Crystals: 16.000000, 19.660800
Labels of proms: 339, 153, XMN4, 2_1, 1_2

*********************************************************************************************************************************/

ADDRESS_MAP_START(onyx_state::c5000_mem)
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0xffff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(onyx_state::c5000_io)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("sio1", z80sio_device, cd_ba_r, cd_ba_w )
ADDRESS_MAP_END

MACHINE_CONFIG_START(onyx_state::c5000)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL(16'000'000) / 4 )
	//MCFG_Z80_DAISY_CHAIN(sub_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(c5000_mem)
	MCFG_CPU_IO_MAP(c5000_io)
	//MCFG_MACHINE_RESET_OVERRIDE(onyx_state, c8002)

	MCFG_DEVICE_ADD("sio1_clock", CLOCK, 614400)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("sio1", z80sio_device, rxtxcb_w))
	//MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("sio1" ,z80sio_device, txca_w))

	/* peripheral hardware */
	//MCFG_DEVICE_ADD("pio1", Z80PIO, XTAL(16'000'000)/4)
	//MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	//MCFG_DEVICE_ADD("pio2", Z80PIO, XTAL(16'000'000)/4)
	//MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	//MCFG_DEVICE_ADD("ctc1", Z80CTC, XTAL(16'000'000) /4)
	//MCFG_DEVICE_ADD("ctc2", Z80CTC, XTAL(16'000'000) /4)
	//MCFG_DEVICE_ADD("ctc3", Z80CTC, XTAL(16'000'000) /4)
	MCFG_DEVICE_ADD("sio1", Z80SIO, XTAL(16'000'000) /4)
	MCFG_Z80SIO_OUT_TXDB_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRB_CB(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSB_CB(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sio1", z80sio_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("sio1", z80sio_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("sio1", z80sio_device, ctsb_w))

	//MCFG_DEVICE_ADD("sio2", Z80SIO, XTAL(16'000'000) /4)
MACHINE_CONFIG_END


ROM_START( c5000 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "860-3.prom1", 0x0000, 0x1000, CRC(31b52df3) SHA1(e221c7829b4805805cde1bde763bd5a936e7db1a) )
	ROM_LOAD( "861-3.prom2", 0x1000, 0x1000, CRC(d1eba182) SHA1(850035497975b821fc1e51fbb73642cba3ff9784) )
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT   MACHINE    INPUT  CLASS       INIT  COMPANY          FULLNAME  FLAGS
COMP( 1981, c5000, 0,      0,       c5000,     c8002, onyx_state, 0,    "Onyx Systems",  "C5000",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
