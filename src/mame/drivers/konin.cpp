// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Mera-Elzab Konin

It's an industrial computer used in Poland

No information has been found. All code is guesswork.

2011-12-29 Skeleton driver.
2016-07-15 Added terminal and uart.

'maincpu' (0384): unmapped i/o memory write to 00F8 = 56 & FF
'maincpu' (0388): unmapped i/o memory write to 00F8 = B6 & FF
'maincpu' (038C): unmapped i/o memory write to 0024 = 00 & FF
'maincpu' (0A0B): unmapped i/o memory write to 0080 = BE & FF
'maincpu' (0A0F): unmapped i/o memory write to 0080 = 08 & FF
'maincpu' (0A13): unmapped i/o memory write to 0080 = 0C & FF
'maincpu' (0A15): unmapped i/o memory read from 0082 & FF
'maincpu' (0A19): unmapped i/o memory write to 0080 = 05 & FF
'maincpu' (04DE): unmapped i/o memory write to 00F6 = 27 & FF
'maincpu' (04E2): unmapped i/o memory write to 00F6 = 40 & FF
'maincpu' (04E6): unmapped i/o memory write to 00F6 = CE & FF
'maincpu' (04EA): unmapped i/o memory write to 00F6 = 27 & FF
'maincpu' (043B): unmapped i/o memory write to 00F8 = B6 & FF
'maincpu' (043F): unmapped i/o memory write to 00F6 = 27 & FF
'maincpu' (2AA3): unmapped i/o memory write to 00F8 = 14 & FF
'maincpu' (2AA7): unmapped i/o memory write to 00FB = C0 & FF
'maincpu' (2AC2): unmapped i/o memory write to 00F8 = 56 & FF
'maincpu' (2AC6): unmapped i/o memory write to 00FA = 03 & FF
'maincpu' (0082): unmapped i/o memory write to 0024 = 06 & FF

Debug stuff:
- Start it up
- Write FF to 7D57 to see some messages
- Write 00 to 7D57 to silence it

Even though it gives an input prompt, there's no code to accept anything

Terminal settings: 8 data bits, 2 stop bits, no parity @ 9600

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8212.h"
#include "machine/i8214.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "bus/rs232/rs232.h"

class konin_state : public driver_device
{
public:
	konin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_picu(*this, "picu")
		, m_ioppi(*this, "ioppi")
		, m_iopit(*this, "iopit")
	{ }

	DECLARE_WRITE8_MEMBER(picu_b_w);
	DECLARE_WRITE_LINE_MEMBER(picu_r3_w);

	void konin(machine_config &config);
	void konin_io(address_map &map);
	void konin_mem(address_map &map);
private:
	virtual void machine_start() override;
	required_device<cpu_device> m_maincpu;
	required_device<i8214_device> m_picu;
	required_device<i8255_device> m_ioppi;
	required_device<pit8253_device> m_iopit;
};

WRITE8_MEMBER(konin_state::picu_b_w)
{
	m_picu->b_w(data ^ 7);
}

WRITE_LINE_MEMBER(konin_state::picu_r3_w)
{
	m_picu->r_w(3, !state);
}

ADDRESS_MAP_START(konin_state::konin_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x4fff) AM_ROM
	AM_RANGE(0x5000, 0x7fff) AM_RAM
	AM_RANGE(0xf200, 0xf200) AM_WRITENOP // watchdog?
	AM_RANGE(0xf400, 0xfbff) AM_RAM
	AM_RANGE(0xfc80, 0xfc83) AM_DEVREADWRITE("mainppi", i8255_device, read, write)
	AM_RANGE(0xfc84, 0xfc87) AM_DEVREADWRITE("mainpit", pit8253_device, read, write)
	AM_RANGE(0xff00, 0xffff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(konin_state::konin_io)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x24, 0x24) AM_WRITE(picu_b_w)
	
	;map(0x80, 0x83).lrw8("ioppi_rw", [this](address_space &space, offs_t offset, u8 mem_mask) { return m_ioppi->read(space, offset^3, mem_mask); }, [this](address_space &space, offs_t offset, u8 data, u8 mem_mask) { m_ioppi->write(space, offset^3, data, mem_mask); });
	AM_RANGE(0xf6, 0xf6) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE(0xf7, 0xf7) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	;map(0xf8, 0xfb).lrw8("iopit_rw", [this](address_space &space, offs_t offset, u8 mem_mask) { return m_iopit->read(space, offset^3, mem_mask); }, [this](address_space &space, offs_t offset, u8 data, u8 mem_mask) { m_iopit->write(space, offset^3, data, mem_mask); });
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( konin )
INPUT_PORTS_END


void konin_state::machine_start()
{
}

MACHINE_CONFIG_START(konin_state::konin)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, XTAL(4'000'000))
	MCFG_CPU_PROGRAM_MAP(konin_mem)
	MCFG_CPU_IO_MAP(konin_io)
	MCFG_I8085A_INTE(DEVWRITELINE("picu", i8214_device, inte_w))
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("intlatch", i8212_device, inta_cb)

	MCFG_DEVICE_ADD("intlatch", I8212, 0)
	MCFG_I8212_MD_CALLBACK(GND)
	MCFG_I8212_DI_CALLBACK(DEVREAD8("picu", i8214_device, vector_r))
	MCFG_I8212_INT_CALLBACK(INPUTLINE("maincpu", I8085_INTR_LINE))

	MCFG_DEVICE_ADD("picu", I8214, XTAL(4'000'000))
	MCFG_I8214_INT_CALLBACK(DEVWRITELINE("intlatch", i8212_device, stb_w))

	MCFG_DEVICE_ADD("mainpit", PIT8253, 0)
	// wild guess at UART clock and source
	MCFG_PIT8253_CLK0(1536000)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("uart", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart", i8251_device, write_rxc))

	MCFG_DEVICE_ADD("mainppi", I8255, 0)

	MCFG_DEVICE_ADD("iopit", PIT8253, 0)

	MCFG_DEVICE_ADD("ioppi", I8255, 0)

	MCFG_DEVICE_ADD("uart", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_I8251_RXRDY_HANDLER(WRITELINE(konin_state, picu_r3_w))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart", i8251_device, write_cts))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( konin )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "001.bin", 0x0000, 0x0800, CRC(0b13208a) SHA1(38ea17be591b729158d601c03bfd9954f32e0e67))
	ROM_LOAD( "008.bin", 0x0800, 0x0800, CRC(f003e407) SHA1(11f79ef3b90788cf627ee39705bbbd04dbf45f50))
	ROM_LOAD( "007.bin", 0x1000, 0x0800, CRC(3d390c03) SHA1(ac2fe31c065e8f630381d6cebd2eb58b403c1e02))
	ROM_LOAD( "006.bin", 0x1800, 0x0800, CRC(68c9732e) SHA1(f40a79719dca485a2db29be5c0c781f559c2551c))
	ROM_LOAD( "005.bin", 0x2000, 0x0800, CRC(14548ac4) SHA1(8987e528b3e479c4c5941366628f34f086d06838))
	ROM_LOAD( "004.bin", 0x2800, 0x0800, CRC(8a354cff) SHA1(24d9f1fb15458fc96f5265f79d54e030b68d9fc9))
	ROM_LOAD( "002.bin", 0x3000, 0x0800, CRC(791fb30d) SHA1(8dfbe0edb741e02cfdd138432999f89480b20471))
	ROM_LOAD( "003.bin", 0x3800, 0x0800, CRC(27dc9864) SHA1(0d3da7fd1db895883c106f5133f8c7228333ecc8))
	ROM_LOAD( "009.bin", 0x4000, 0x0800, CRC(80947d15) SHA1(0757fb191913d79f306874684f9fc082ce18a28e))
	ROM_LOAD( "010.bin", 0x4800, 0x0800, CRC(f0157e0c) SHA1(60ace1eaf0ba01a45987c2286e18f3d56441c994))
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT  STATE        INIT  COMPANY       FULLNAME  FLAGS
COMP( 198?, konin,  0,      0,       konin,     konin, konin_state, 0,    "Mera-Elzab", "Konin",  MACHINE_IS_SKELETON )
