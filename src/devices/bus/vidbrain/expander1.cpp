// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VideoBrain Expander 1 emulation

    3870 MCU with 2K mask ROM, two cassette decks (PLAYBACK and
    RECORD) and two RS-232 jacks (PRINTER, and MODEM for Expander 2).

    The port assignments below were recovered from the MCU firmware:

    Port 0:
        bit 0   PLAYBACK remote
        bit 1   RECORD remote
        bit 2   RECORD data, positive pulse
        bit 3   RECORD data, negative pulse
        bit 7   PLAYBACK data

    Port 1:
        bit 0   accessory jack I/O-15
        bit 1   accessory jack EXT INT
        bit 7   accessory jack I/O-16

    Port 4:
        bit 0   PRINTER TxD
        bit 7   PRINTER RxD

    Port 5:
        bit 0   MODEM TxD
        bit 7   MODEM RxD

    TODO:
    - port 5 bits 1 and 2 are pulsed during cassette transfers
    - data load LED

**********************************************************************/

#include "emu.h"
#include "expander1.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIDEOBRAIN_EXPANDER1, videobrain_expander1_device, "vidbrain_expander1", "VideoBrain Expander 1")


//-------------------------------------------------
//  ROM( videobrain_expander1 )
//-------------------------------------------------

ROM_START( videobrain_expander1 )
	ROM_REGION( 0x800, "mcu", 0 )
	ROM_LOAD( "expander1.bin", 0x000, 0x800, CRC(dac31abc) SHA1(e1ac7a9d654c2a70979effc744d98f21d13b4e05) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *videobrain_expander1_device::device_rom_region() const
{
	return ROM_NAME( videobrain_expander1 );
}


//-------------------------------------------------
//  ADDRESS_MAP( program_map )
//-------------------------------------------------

void videobrain_expander1_device::program_map(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x000, 0x7ff).rom();
}


//-------------------------------------------------
//  ADDRESS_MAP( io_map )
//-------------------------------------------------

void videobrain_expander1_device::io_map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(videobrain_expander1_device::p0_r), FUNC(videobrain_expander1_device::p0_w));
	map(0x01, 0x01).rw(FUNC(videobrain_expander1_device::p1_r), FUNC(videobrain_expander1_device::p1_w));
	map(0x04, 0x07).rw(m_psu, FUNC(f38t56_device::read), FUNC(f38t56_device::write));
}


//-------------------------------------------------
//  DEVICE_INPUT_DEFAULTS( modem )
//-------------------------------------------------

static DEVICE_INPUT_DEFAULTS_START( modem )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void videobrain_expander1_device::device_add_mconfig(machine_config &config)
{
	// the crystal is inferred from the RS-232 and cassette timer divisors in the firmware
	F8(config, m_mcu, 3.579545_MHz_XTAL / 2);
	m_mcu->set_addrmap(AS_PROGRAM, &videobrain_expander1_device::program_map);
	m_mcu->set_addrmap(AS_IO, &videobrain_expander1_device::io_map);
	m_mcu->int_cycle_callback().set(m_psu, FUNC(f38t56_device::int_acknowledge));

	F38T56(config, m_psu, 3.579545_MHz_XTAL / 2);
	m_psu->set_int_vector(0x0020);
	m_psu->int_req_callback().set_inputline(m_mcu, F8_INPUT_LINE_INT_REQ);
	m_psu->read_a().set(FUNC(videobrain_expander1_device::p4_r));
	m_psu->write_a().set(FUNC(videobrain_expander1_device::p4_w));
	m_psu->read_b().set(FUNC(videobrain_expander1_device::p5_r));
	m_psu->write_b().set(FUNC(videobrain_expander1_device::p5_w));

	CASSETTE(config, m_playback);
	m_playback->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);

	CASSETTE(config, m_record);
	m_record->set_default_state(CASSETTE_RECORD | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED);

	RS232_PORT(config, m_printer, default_rs232_devices, nullptr);
	m_printer->rxd_handler().set([this] (int state) { m_printer_rxd = state; });

	RS232_PORT(config, m_modem, default_rs232_devices, nullptr);
	m_modem->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(modem));
	m_modem->rxd_handler().set([this] (int state) { m_modem_rxd = state; });
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  videobrain_expander1_device - constructor
//-------------------------------------------------

videobrain_expander1_device::videobrain_expander1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VIDEOBRAIN_EXPANDER1, tag, owner, clock),
	device_videobrain_accessory_interface(mconfig, *this),
	m_mcu(*this, "mcu"),
	m_psu(*this, "psu"),
	m_playback(*this, "playback"),
	m_record(*this, "record"),
	m_printer(*this, "printer"),
	m_modem(*this, "modem"),
	m_p0(0),
	m_p1(0),
	m_p4(0),
	m_p5(0),
	m_host(0),
	m_printer_rxd(1),
	m_modem_rxd(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void videobrain_expander1_device::device_start()
{
	save_item(NAME(m_p0));
	save_item(NAME(m_p1));
	save_item(NAME(m_p4));
	save_item(NAME(m_p5));
	save_item(NAME(m_host));
	save_item(NAME(m_printer_rxd));
	save_item(NAME(m_modem_rxd));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void videobrain_expander1_device::device_reset()
{
	p0_w(0);
	p1_w(0);
}


//-------------------------------------------------
//  videobrain_io_w - accessory jack I/O lines
//-------------------------------------------------

void videobrain_expander1_device::videobrain_io_w(uint8_t data)
{
	m_host = data;
}


//-------------------------------------------------
//  videobrain_io_r - accessory jack I/O lines
//-------------------------------------------------

uint8_t videobrain_expander1_device::videobrain_io_r()
{
	return (BIT(m_p1, 7) << 1) | BIT(m_p1, 0);
}


//-------------------------------------------------
//  videobrain_reset_w - accessory jack reset
//-------------------------------------------------

void videobrain_expander1_device::videobrain_reset_w(int state)
{
	m_mcu->set_input_line(INPUT_LINE_RESET, state);

	if (state)
	{
		// the 3870 RESET pin also clears the I/O ports and the timer/interrupt registers
		for (int i = 0; i < 4; i++)
			m_psu->write(i, 0);

		p0_w(0);
		p1_w(0);
	}
}


//-------------------------------------------------
//  port 0 - cassette
//-------------------------------------------------

uint8_t videobrain_expander1_device::p0_r()
{
	return m_p0 | ((m_playback->input() > 0) ? 0x80 : 0);
}

void videobrain_expander1_device::p0_w(uint8_t data)
{
	m_p0 = data;

	m_playback->set_motor(BIT(data, 0));
	m_record->set_motor(BIT(data, 1));

	m_record->output(BIT(data, 2) ? 1.0 : (BIT(data, 3) ? -1.0 : 0.0));
}


//-------------------------------------------------
//  port 1 - accessory jack
//-------------------------------------------------

uint8_t videobrain_expander1_device::p1_r()
{
	return m_p1 | (BIT(m_host, 1) << 7) | BIT(m_host, 0);
}

void videobrain_expander1_device::p1_w(uint8_t data)
{
	m_p1 = data;

	m_port->ext_int_w(BIT(data, 1));
}


//-------------------------------------------------
//  port 4/5 - printer/modem
//-------------------------------------------------

uint8_t videobrain_expander1_device::p4_r()
{
	return m_p4 | (m_printer_rxd << 7);
}

void videobrain_expander1_device::p4_w(uint8_t data)
{
	m_p4 = data;

	m_printer->write_txd(BIT(data, 0));
}

uint8_t videobrain_expander1_device::p5_r()
{
	return m_p5 | (m_modem_rxd << 7);
}

void videobrain_expander1_device::p5_w(uint8_t data)
{
	m_p5 = data;

	m_modem->write_txd(BIT(data, 0));
}
