// license:BSD-3-Clause
// copyright-holders:MetalliC
/**********************************************************************

	Didaktik D40/D80 disk interface
	(C) 1991 Didaktik Scalica

**********************************************************************/


#include "emu.h"
#include "d40.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_D40, spectrum_d40_device, "spectrum_d40", "Didaktik D40 disk interface")
DEFINE_DEVICE_TYPE(SPECTRUM_D80, spectrum_d80_device, "spectrum_d80", "Didaktik D80 disk interface")

//-------------------------------------------------
//  INPUT_PORTS
//-------------------------------------------------

INPUT_PORTS_START(d40)
	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Snapshot Button") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, spectrum_d40_device, snapshot_button, 0)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_d40_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(d40);
}

//-------------------------------------------------
//  SLOT_INTERFACE( floppies )
//-------------------------------------------------

static void didaktik_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("525dd", FLOPPY_525_DD);
}

//-------------------------------------------------
//  floppy_format_type floppy_formats
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER(spectrum_d40_device::floppy_formats)
	FLOPPY_D40D80_FORMAT
FLOPPY_FORMATS_END

//-------------------------------------------------
//  ROM
//-------------------------------------------------

ROM_START(d40)
	ROM_REGION(0x4000, "rom", 0)
	ROM_DEFAULT_BIOS("mdos11")
	ROM_SYSTEM_BIOS(0, "mdos11", "MDOS 1.0 (1991)")
	ROMX_LOAD("mdos10.bin", 0x0000, 0x4000, CRC(e6b70939) SHA1(308c4b5daf6bb1f05c68a447129d723da423326e), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "mdos12", "MDOS 1.0+ (1992)")
	ROMX_LOAD("mdos10p.bin", 0x0000, 0x4000, CRC(92c45741) SHA1(b8473235feecff4eccbace56a90cf1d8c79506eb), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "mdos2", "MDOS 2.0 (1993)") // doesn't work, from different board type with GM82C765B FDC
	ROMX_LOAD("mdos20.bin", 0x0000, 0x4000, CRC(9e79d022) SHA1(e8d3355051fb287dd0dda34ba8824442130c8254), ROM_BIOS(2))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_d40_device::device_add_mconfig(machine_config &config)
{
	WD2797(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(spectrum_d40_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(spectrum_d40_device::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", didaktik_floppies, "525dd", spectrum_d40_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", didaktik_floppies, "525dd", spectrum_d40_device::floppy_formats).enable_sound(true);

	/* software list */
	//SOFTWARE_LIST(config, "flop_list").set_original("spectrum_d40_flop");
}

void spectrum_d80_device::device_add_mconfig(machine_config &config)
{
	WD2797(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(spectrum_d80_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(spectrum_d80_device::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", didaktik_floppies, "35dd", spectrum_d80_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", didaktik_floppies, "35dd", spectrum_d80_device::floppy_formats).enable_sound(true);

	/* software list */
	//SOFTWARE_LIST(config, "flop_list").set_original("spectrum_d40_flop");
}

const tiny_rom_entry *spectrum_d40_device::device_rom_region() const
{
	return ROM_NAME(d40);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_d40_device - constructor
//-------------------------------------------------

spectrum_d40_device::spectrum_d40_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0)
{
}

spectrum_d40_device::spectrum_d40_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_d40_device(mconfig, SPECTRUM_D40, tag, owner, clock)
{
}

spectrum_d80_device::spectrum_d80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_d40_device(mconfig, SPECTRUM_D80, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_d40_device::device_start()
{
	std::fill(std::begin(m_ram), std::end(m_ram), 0);
	save_item(NAME(m_romcs));
	save_item(NAME(m_ram));
	save_item(NAME(m_control));
	save_item(NAME(m_snap_flag));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_d40_device::device_reset()
{
	m_romcs = 0;
	m_control = 0;
	m_snap_flag = 0;
}


//**************************************************************************
//  IMPLEMENTATION  spectrum_d40_device
//**************************************************************************

READ_LINE_MEMBER(spectrum_d40_device::romcs)
{
	return m_romcs;
}

void spectrum_d40_device::pre_opcode_fetch(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		switch (offset)
		{
		case 0x0000:
		case 0x0008:
			m_romcs = 1;
			break;
		case 0x0066:
			if (!m_romcs)
			{
				m_romcs = 1;
				m_snap_flag = 1;
			}
			break;
		case 0x1700:
			m_romcs = 0;
			break;
		}
	}
}

uint8_t spectrum_d40_device::iorq_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset & 0xf9)
	{
	case 0x81: // FDC
		data = m_fdc->read((offset >> 1) & 0x03);
		break;
	}

	return data;
}

void spectrum_d40_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xf9)
	{
	case 0x81: // FDC
		m_fdc->write((offset >> 1) & 0x03, data);
		break;
	case 0x89: // control port
	{
		m_control = data;

		floppy_image_device* floppy = nullptr;
		if (BIT(data, 0))
			floppy = m_floppy[0]->get_device();
		else if (BIT(data, 1))
			floppy = m_floppy[1]->get_device();
		m_fdc->set_floppy(floppy);

		m_floppy[0]->get_device()->mon_w(BIT(data, 2) ? 0 : 1);
		m_floppy[1]->get_device()->mon_w(BIT(data, 3) ? 0 : 1);
	}
		break;
	case 0x91: // PPI reset
		// TODO PPI
		break;
	case 0x99: // PPI enable
		// TODO PPI
		break;
	}
}

uint8_t spectrum_d40_device::mreq_r(offs_t offset)
{
	if (m_snap_flag && offset == 0x66)
	{
		m_snap_flag = 0;
		return 0xc7;
	}

	uint8_t data = 0xff;

	if (m_romcs)
	{
		if (offset < 0x3800)
			data = m_rom->base()[offset];
		else
			data = m_ram[offset & 0x7ff];
	}

	return data;
}

void spectrum_d40_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_romcs)
	{
		if ((offset & 0xf800) == 0x3800)
			m_ram[offset & 0x7ff] = data;
	}
}

INPUT_CHANGED_MEMBER(spectrum_d40_device::snapshot_button)
{
	m_slot->nmi_w((!newval && oldval && !m_romcs) ? ASSERT_LINE : CLEAR_LINE);
}

void spectrum_d40_device::fdc_intrq_w(int state)
{
	m_slot->nmi_w((state && BIT(m_control, 7)) ? ASSERT_LINE : CLEAR_LINE);
}

void spectrum_d40_device::fdc_drq_w(int state)
{
	m_slot->nmi_w((state && BIT(m_control, 6)) ? ASSERT_LINE : CLEAR_LINE);
}
