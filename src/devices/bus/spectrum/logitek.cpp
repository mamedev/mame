// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    Proceed 1 - Commodore IEC Bus and printer interface
    (c) 1984 Logitek (Germany)

    Allows to connect and use Commodore 1541 disk drive.
    Components: Z8420 PIO, 8K ROM, 256x4 PROM, NMI button.

    Photos and manual https://k1.spdns.de/Vintage/Sinclair/82/Peripherals/Disc%20Interfaces/Logitek%20Proceed%201%20C64%20Disc%20Interface/

    Notes / TODOs:
    - require "quantum perfect" host machine config, otherwise floppy drive access doesn't work with "Device timeout" error message.

    - ROM paging performed by undumped PROM, current logic might be innacurate.
      wiring: A0-A5 - Z80 A8-A13, A6 - PIO PB7, A7 - nc, /CE1 - A14, /CE2 - A15,
      Q1 - ROM A11, Q2 - ROM A10, Q3 - ROM /CE, Q4 - /ROMCS

*********************************************************************/

#include "emu.h"
#include "logitek.h"


/***************************************************************************
    DEVICE DEFINITIONS
***************************************************************************/

DEFINE_DEVICE_TYPE(SPECTRUM_PROCEED, spectrum_proceed_device, "spectrum_proceed", "Logitek Proceed 1 Interface")


//-------------------------------------------------
//  INPUT_PORTS
//-------------------------------------------------

INPUT_PORTS_START(proceed)
	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("NMI Button") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, spectrum_proceed_device, nmi_button, 0)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_proceed_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(proceed);
}

//-------------------------------------------------
//  ROM( proceed )
//-------------------------------------------------

ROM_START(proceed)
	ROM_REGION(0x2000, "rom", 0)
	ROM_DEFAULT_BIOS("ldos21")

	ROM_SYSTEM_BIOS(0, "ldos21", "LDOS v2.1")
	ROMX_LOAD("ldos_v21.bin", 0x0000, 0x2000, CRC(a7c47bac) SHA1(c999b0e4f5537405ed56c4c96beca5e7f5eb2b1e), ROM_BIOS(0))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_proceed_device::device_add_mconfig(machine_config &config)
{
	Z80PIO(config, m_z80pio, 3500000);
	m_z80pio->out_pa_callback().set(FUNC(spectrum_proceed_device::pioa_w));
	m_z80pio->in_pb_callback().set(FUNC(spectrum_proceed_device::piob_r));
	m_z80pio->out_pb_callback().set(FUNC(spectrum_proceed_device::piob_w));
	m_z80pio->out_int_callback().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));

	cbm_iec_slot_device::add(config, m_iec, "c1541");

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(m_z80pio, FUNC(z80pio_device::pb0_w));
}

const tiny_rom_entry *spectrum_proceed_device::device_rom_region() const
{
	return ROM_NAME(proceed);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_proceed_device - constructor
//-------------------------------------------------

spectrum_proceed_device::spectrum_proceed_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_PROCEED, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_z80pio(*this, "z80pio")
	, m_iec(*this, "iec_bus")
	, m_centronics(*this, "centronics")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_proceed_device::device_start()
{
	save_item(NAME(m_romcs));
	save_item(NAME(m_romen));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_proceed_device::device_reset()
{
	m_romcs = 0;
	m_romen = 0;
	m_iec->reset();
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

bool spectrum_proceed_device::romcs()
{
	return m_romcs;
}

void spectrum_proceed_device::fetch(offs_t offset)
{
	switch (offset >> 8)
	{
		// always override
	case 0x00:
	case 0x0e:
	case 0x39: case 0x3a: case 0x3b: case 0x3c:
		m_romcs = 1;
		break;
	case 0x1b:
		m_romcs = 1;
		offset |= 0x0400; // 1Bxx -> 1Fxx
		break;
		// override only if PIO PB7 is 0
	case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d:
	case 0x0f: case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: case 0x18:
	case 0x1d: case 0x1e:
		m_romcs = m_romen;
		break;

	default:
		m_romcs = 0;
	}
}

uint8_t spectrum_proceed_device::mreq_r(offs_t offset)
{
	u8 data = 0xff;

	if (m_romcs)
		data = m_rom->base()[offset & 0x1fff];

	return data;
}

uint8_t spectrum_proceed_device::iorq_r(offs_t offset)
{
	uint8_t data = offset & 1 ? m_slot->fb_r() : 0xff;

	if (!BIT(offset, 5))
		data = m_z80pio->read((offset >> 6) & 3);

	return data;
}

void spectrum_proceed_device::iorq_w(offs_t offset, uint8_t data)
{
	if (!BIT(offset, 5))
		m_z80pio->write((offset >> 6) & 3, data);
}

void spectrum_proceed_device::pioa_w(uint8_t data)
{
	m_centronics->write_data0(BIT(data, 0));
	m_centronics->write_data1(BIT(data, 1));
	m_centronics->write_data2(BIT(data, 2));
	m_centronics->write_data3(BIT(data, 3));
	m_centronics->write_data4(BIT(data, 4));
	m_centronics->write_data5(BIT(data, 5));
	m_centronics->write_data6(BIT(data, 6));
	m_centronics->write_data7(BIT(data, 7));
}

uint8_t spectrum_proceed_device::piob_r()
{
	u8 data = 0;

	data |= m_iec->clk_r() << 3;
	data |= m_iec->data_r() << 2;

	return data;
}

void spectrum_proceed_device::piob_w(uint8_t data)
{
	m_romen = !BIT(data, 7);
	m_iec->host_atn_w(BIT(data, 6));
	m_iec->host_clk_w(BIT(data, 5));
	m_iec->host_data_w(BIT(data, 4));
	m_centronics->write_strobe(BIT(data, 1));
}
