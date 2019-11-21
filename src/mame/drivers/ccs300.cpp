// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        CCS Model 300

        2009-12-11 Skeleton driver.

It requires a floppy disk to boot from.

The bankswitching appears to be the same as CCS's other systems.

Early on, it does a read from port F2. If bit 3 is low, the system becomes
a Model 400.

The CPU board appears to be the 2820 System Processor, which has Z80A CTC,
Z80A PIO, Z80A SIO/0 and Z80A DMA peripherals on board. Several features,
including IEI/IEO daisy chain priority, are jumper-configurable.

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
	{ }

	void ccs300(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<z80_device> m_maincpu;

	void ccs300_io(address_map &map);
	void ccs300_mem(address_map &map);

	DECLARE_WRITE8_MEMBER(port40_w);
};

void ccs300_state::ccs300_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).bankr("bankr0").bankw("bankw0");
	map(0x0800, 0xffff).ram();
}

void ccs300_state::ccs300_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x10, 0x13).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x14, 0x17).rw("pio", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));  // init bytes seem to be for a PIO
	map(0x18, 0x1b).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));  // init bytes seem to be for a CTC
	map(0x30, 0x33); // fdc?
	map(0x34, 0x34); // motor control?
	map(0x40, 0x40).w(FUNC(ccs300_state::port40_w));
	map(0xf0, 0xf0).rw("dma", FUNC(z80dma_device::read), FUNC(z80dma_device::write)); // long sequence of init bytes
	map(0xf2, 0xf2); // dip or jumper?
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
WRITE8_MEMBER( ccs300_state::port40_w )
{
	membank("bankr0")->set_entry( (data) ? 1 : 0);
}

void ccs300_state::machine_reset()
{
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
}

void ccs300_state::machine_start()
{
	uint8_t *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);
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
	sio.out_txda_callback().set("sioa", FUNC(rs232_port_device::write_txd));
	sio.out_dtra_callback().set("sioa", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsa_callback().set("sioa", FUNC(rs232_port_device::write_rts));
	sio.out_txdb_callback().set("siob", FUNC(rs232_port_device::write_txd));
	sio.out_dtrb_callback().set("siob", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsb_callback().set("siob", FUNC(rs232_port_device::write_rts));

	rs232_port_device &sioa(RS232_PORT(config, "sioa", default_rs232_devices, "terminal"));
	sioa.rxd_handler().set("sio", FUNC(z80sio_device::rxa_w));
	sioa.cts_handler().set("sio", FUNC(z80sio_device::ctsa_w));
	sioa.dcd_handler().set("sio", FUNC(z80sio_device::dcda_w));
	sioa.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal)); // must be exactly here

	rs232_port_device &siob(RS232_PORT(config, "siob", default_rs232_devices, nullptr));
	siob.rxd_handler().set("sio", FUNC(z80sio_device::rxa_w));
	siob.cts_handler().set("sio", FUNC(z80sio_device::ctsa_w));
	siob.dcd_handler().set("sio", FUNC(z80sio_device::dcda_w));

	z80ctc_device &ctc(Z80CTC(config, "ctc", 16_MHz_XTAL / 4));
	ctc.set_clk<0>(16_MHz_XTAL / 8);
	ctc.set_clk<1>(16_MHz_XTAL / 8);
	ctc.set_clk<2>(16_MHz_XTAL / 8);
	//ctc.set_clk<3>(16_MHz_XTAL / 8);   // this causes an IRQ storm, hanging the machine
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
	ROM_REGION( 0x10800, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ccs300.rom", 0x10000, 0x0800, CRC(6cf22e31) SHA1(9aa3327cd8c23d0eab82cb6519891aff13ebe1d0))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT   COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY                        FULLNAME         FLAGS */
COMP( 19??, ccs300, ccs2810, 0,      ccs300,  ccs300, ccs300_state, empty_init, "California Computer Systems", "CCS Model 300", MACHINE_IS_SKELETON )
