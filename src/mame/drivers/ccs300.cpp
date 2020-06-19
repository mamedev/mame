// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

CCS Model 300 / 400

2009-12-11 Skeleton driver.

It requires a floppy disk to boot from.

The bankswitching appears to be the same as CCS's other systems.

Early on, it does a read from port F2. If bit 3 is low, the system becomes
a Model 400.

The CPU board appears to be similar to the 2820 System Processor, which has
Z80A CTC, Z80A PIO, Z80A SIO/0 and Z80A DMA peripherals on board. Several
features, including IEI/IEO daisy chain priority, are jumper-configurable.

However, the 2820 has the i/o ports rearranged slightly (even though the
manual says it should work!), and no fdc support.


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "bus/rs232/rs232.h"
//#include "bus/s100/s100.h"

class ccs300_state : public driver_device
{
public:
	ccs300_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
	{ }

	void ccs300(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void ccs300_io(address_map &map);
	void ccs300_mem(address_map &map);
	void port40_w(u8 data);
	bool m_rom_in_map;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
};

void ccs300_state::ccs300_mem(address_map &map)
{
	map(0x0000, 0xffff).ram().share("mainram");
	map(0x0000, 0x07ff).lr8(NAME([this] (offs_t offset) { if (m_rom_in_map) return m_rom[offset]; else return m_ram[offset]; } ));
}

void ccs300_state::ccs300_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x04, 0x04); // fdc related
	map(0x10, 0x13).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x14, 0x17).rw("pio", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x18, 0x1b).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x30, 0x33); // fdc?
	map(0x34, 0x34); // motor control?
	map(0x40, 0x40).w(FUNC(ccs300_state::port40_w));
	map(0xf0, 0xf0).rw("dma", FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0xf2, 0xf2); // dip or jumper? only used by CCS-400
}

/* Input ports */
static INPUT_PORTS_START( ccs300 )
INPUT_PORTS_END

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ "sio" },
	{ "pio" },
	{ "dma" },
	{ nullptr }
};

//*************************************
//
//  Machine
//
//*************************************
void ccs300_state::port40_w(u8 data)
{
	m_rom_in_map = !BIT(data, 0);
}

void ccs300_state::machine_reset()
{
	m_rom_in_map = true;
}

void ccs300_state::machine_start()
{
	save_item(NAME(m_rom_in_map));
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

void ccs300_state::ccs300(machine_config & config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &ccs300_state::ccs300_mem);
	m_maincpu->set_addrmap(AS_IO, &ccs300_state::ccs300_io);
	m_maincpu->set_daisy_config(daisy_chain);

	/* Devices */
	z80sio_device &sio(Z80SIO(config, "sio", 16_MHz_XTAL / 4));
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	sio.out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	sio.out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	sio.out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	sio.out_dtrb_callback().set("rs232b", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set("sio", FUNC(z80sio_device::rxa_w));
	rs232a.cts_handler().set("sio", FUNC(z80sio_device::ctsa_w));
	rs232a.dcd_handler().set("sio", FUNC(z80sio_device::dcda_w));
	rs232a.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal)); // must be exactly here

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("sio", FUNC(z80sio_device::rxb_w));
	rs232b.cts_handler().set("sio", FUNC(z80sio_device::ctsb_w));
	rs232b.dcd_handler().set("sio", FUNC(z80sio_device::dcdb_w));

	z80ctc_device &ctc(Z80CTC(config, "ctc", 16_MHz_XTAL / 4));
	ctc.set_clk<0>(16_MHz_XTAL / 8);     // 153'846
	//ctc.set_clk<1>(16_MHz_XTAL / 8);    // not used
	ctc.set_clk<2>(16_MHz_XTAL / 8);      // 9'615
	//ctc.set_clk<3>(16_MHz_XTAL / 8);   // 2'000'000 - this causes an IRQ storm, hanging the machine
	ctc.zc_callback<0>().set("sio", FUNC(z80sio_device::txca_w));
	ctc.zc_callback<0>().append("sio", FUNC(z80sio_device::rxca_w));
	ctc.zc_callback<2>().append("sio", FUNC(z80sio_device::rxtxcb_w));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80pio_device &pio(Z80PIO(config, "pio", 16_MHz_XTAL / 4));
	pio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80dma_device &dma(Z80DMA(config, "dma", 16_MHz_XTAL / 4));
	dma.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
}

/* ROM definition */
ROM_START( ccs300 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "ccs300.rom", 0x0000, 0x0800, CRC(6cf22e31) SHA1(9aa3327cd8c23d0eab82cb6519891aff13ebe1d0))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT   COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY                        FULLNAME         FLAGS */
COMP( 1981, ccs300, ccs2810, 0,      ccs300,  ccs300, ccs300_state, empty_init, "California Computer Systems", "CCS Model 300", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
