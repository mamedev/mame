// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Micromint SB180

****************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"
#include "bus/rs232/rs232.h"


namespace {

#define FDC9266_TAG "u24"

class sb180_state : public driver_device
{
public:
	sb180_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, FDC9266_TAG)
		, m_floppy(*this, FDC9266_TAG ":%u", 0)
	{ }

	void sb180(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;

	void sb180_io(address_map &map) ATTR_COLD;
	void sb180_mem(address_map &map) ATTR_COLD;

	required_device<z180_device> m_maincpu;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
};

void sb180_state::sb180_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x01fff).rom().mirror(0x3e000);
	map(0x40000, 0x7ffff).ram(); // 256KB RAM  8 * 41256 DRAM
}

void sb180_state::sb180_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x7f).ram(); /* Z180 internal registers */
	map(0x80, 0x81).m(m_fdc, FUNC(upd765a_device::map));
	map(0xa0, 0xa0).rw(m_fdc, FUNC(upd765a_device::dma_r), FUNC(upd765a_device::dma_w));
}

/* Input ports */
static INPUT_PORTS_START( sb180 )
INPUT_PORTS_END

void sb180_state::machine_reset()
{
	// motor is actually connected on TXS pin of CPU
	for (auto &drive : m_floppy)
	{
		if (drive->get_device())
			drive->get_device()->mon_w(0);
	}
}

static void sb180_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void sb180_state::sb180(machine_config &config)
{
	/* basic machine hardware */
	HD64180RP(config, m_maincpu, XTAL(12'288'000)); // location U17 HD64180
	m_maincpu->set_addrmap(AS_PROGRAM, &sb180_state::sb180_mem);
	m_maincpu->set_addrmap(AS_IO, &sb180_state::sb180_io);
	m_maincpu->tend1_wr_callback().set(m_fdc, FUNC(upd765a_device::tc_line_w));
	m_maincpu->txa1_wr_callback().set("rs232", FUNC(rs232_port_device::write_txd));

	// FDC9266 location U24
	UPD765A(config, m_fdc, XTAL(8'000'000));
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ2);
	m_fdc->drq_wr_callback().set_inputline(m_maincpu, Z180_INPUT_LINE_DREQ1);

	/* floppy drives */
	FLOPPY_CONNECTOR(config, m_floppy[0], sb180_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], sb180_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[2], sb180_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[3], sb180_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal)); // must be below the DEVICE_INPUT_DEFAULTS_START block
	rs232.rxd_handler().set(m_maincpu, FUNC(z180_device::rxa1_w));
}

/* ROM definition */
ROM_START( sb180 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "monitor.bin", 0x0000, 0x2000, CRC(49640012) SHA1(ea571dc7476430e31b74bd1ab7a577e9013ad0bd))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY      FULLNAME   FLAGS */
COMP( 1985, sb180,  0,      0,      sb180,   sb180,  sb180_state,  empty_init, "Micromint", "SB180",   MACHINE_NO_SOUND)
