// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo

// MAME driver for Grant Searle's Simple CP/M Computer
// http://www.searle.wales/

// This driver uses a compact flash card as a hard disk device.
// To create a virtual disk file, use the following (for a 128MB card):
//     chdman createhd -s 134217728 -o filename.chd
// (or use -s 67108864 for 64MB card)
//
// Then, run MAME with -hard filename.chd, or mount it using the GUI
// and restart the driver
//

#include "emu.h"

#include "cpu/z80/z80.h"
#include "bus/ata/ataintf.h"
#include "machine/z80sio.h"
#include "machine/ram.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"


namespace {

class gscpm_state : public driver_device
{
public:
	gscpm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "ram")
		, m_ide(*this, "ide")
		, m_sio(*this, "sio")
	{ }

	void gscpm(machine_config &config);

protected:
	void machine_reset() override ATTR_COLD;

	void gscpm_mem(address_map &map) ATTR_COLD;
	void gscpm_io(address_map &map) ATTR_COLD;

	uint8_t cflash_r(offs_t offset);
	void cflash_w(offs_t offset, uint8_t data);
	uint8_t sio_r(offs_t offset);
	void sio_w(offs_t offset, uint8_t data);

	void switch_to_ram_w(uint8_t data);

	required_device<z80_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<ata_interface_device> m_ide;
	required_device<z80sio_device> m_sio;
};

void gscpm_state::gscpm_mem(address_map &map)
{
	//map(0x0000, 0x3fff).rom("maincpu");  // This is ROM after reset, and RAM is switched in when CP/M is booted
										   // (will install handlers dynamically)
	map(0x4000, 0xffff).ram();
}

void gscpm_state::gscpm_io(address_map &map)
{
	map.global_mask(0xff);  // use 8-bit ports
	map.unmap_value_high(); // unmapped addresses return 0xff
	map(0x00, 0x07).rw(FUNC(gscpm_state::sio_r), FUNC(gscpm_state::sio_w));
	map(0x10, 0x17).rw(FUNC(gscpm_state::cflash_r), FUNC(gscpm_state::cflash_w)); // compact flash
	map(0x38, 0x3f).w(FUNC(gscpm_state::switch_to_ram_w));
}

uint8_t gscpm_state::cflash_r(offs_t offset)
{
	return m_ide->cs0_r(offset, 0xff);
}

void gscpm_state::cflash_w(offs_t offset, uint8_t data)
{
	m_ide->cs0_w(offset, data, 0xff);
}

uint8_t gscpm_state::sio_r(offs_t offset)
{
	switch (offset & 3)
	{
	case 0x00:
		return m_sio->da_r();
	case 0x01:
		return m_sio->db_r();
	case 0x02:
		return m_sio->ca_r();
	case 0x03:
		return m_sio->cb_r();
	}
	return 0x00; // can't happen
}

void gscpm_state::sio_w(offs_t offset, uint8_t data)
{
	switch (offset & 3)
	{
	case 0x00:
		m_sio->da_w(data);
		break;
	case 0x01:
		m_sio->db_w(data);
		break;
	case 0x02:
		m_sio->ca_w(data);
		break;
	case 0x03:
		m_sio->cb_w(data);
		break;
	}
}

void gscpm_state::switch_to_ram_w(uint8_t data)
{
	// Install the RAM handler here
	m_maincpu->space(AS_PROGRAM).unmap_readwrite(0x0000, 0x3fff); // Unmap the ROM handler
	m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x3fff, m_ram->pointer());
}

void gscpm_state::machine_reset()
{
	// Install the ROM handler here
	m_maincpu->space(AS_PROGRAM).unmap_readwrite(0x0000, 0x3fff);  // Unmap RAM handler if being rebooted
	m_maincpu->space(AS_PROGRAM).install_rom(0x0000, 0x3fff, memregion("maincpu")->base());
}

// This is here only to configure our terminal for interactive use
static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

static const z80_daisy_config gscpm_daisy_chain[] =
{
	{ "sio" },
	{ nullptr }
};

void gscpm_state::gscpm(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(7'372'800));
	m_maincpu->set_addrmap(AS_PROGRAM, &gscpm_state::gscpm_mem);
	m_maincpu->set_addrmap(AS_IO, &gscpm_state::gscpm_io);
	m_maincpu->set_daisy_config(gscpm_daisy_chain);

	/* compact flash hard drive */
	ATA_INTERFACE(config, m_ide).options(ata_devices, "hdd", nullptr, false);

	RAM(config, m_ram).set_default_size("16K"); // This shadows the ROM

	Z80SIO(config, m_sio, 0);
	m_sio->out_txdb_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	m_sio->out_rtsb_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	m_sio->out_int_callback().set_inputline("maincpu", INPUT_LINE_IRQ0); // Connect interrupt pin to our Z80 INT line

	clock_device &sio_clock(CLOCK(config, "sio_clock", 7'372'800));
	sio_clock.signal_handler().set("sio", FUNC(z80sio_device::txca_w));
	sio_clock.signal_handler().append("sio", FUNC(z80sio_device::rxca_w));
	sio_clock.signal_handler().append("sio", FUNC(z80sio_device::txcb_w));
	sio_clock.signal_handler().append("sio", FUNC(z80sio_device::rxcb_w));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("sio", FUNC(z80sio_device::rxb_w));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal)); // must be below the DEVICE_INPUT_DEFAULTS_START block
}

// ROM mapping is trivial, this binary was created from the HEX file on Grant's website
ROM_START(gscpm)
	ROM_REGION(0x4000, "maincpu",0)
	ROM_LOAD("gscpm.bin",   0x0000, 0x4000, CRC(35ae0d43) SHA1(7fae4df419d38a1787a4a97cbef558f402109959))
ROM_END

} // anonymous namespace


//    YEAR  NAME         PARENT    COMPAT  MACHINE   INPUT    CLASS        INIT           COMPANY           FULLNAME                FLAGS
COMP( 201?, gscpm,       0,        0,      gscpm,    0,       gscpm_state, empty_init,    "Grant Searle",   "Simple CP/M Machine",  MACHINE_NO_SOUND_HW )
