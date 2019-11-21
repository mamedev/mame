// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Technology Research Beta 128 Disk interface

    This hardware type runs TR-DOS 5.xx (official) and newer
    unofficial updates.  It was designed to work properly with
    the 128k machines that had issues with the original Beta Disk
    due to changes in the 128k ROM structure etc. (enable address
    is moved from 3cxx to 3dxx for example)

    Issues:

    Using the FD1793 device a 'CAT' operation in the 'spectrum' driver
    will always report 'No Disk' but using the Soviet clone KR1818VG93
    it properly gives the disk catalogue.  Despite this files can still
    be loaded from disk.

    The 128k Spectrum drivers have a similar issues, although even if
    you replace the controller doing a 'CAT' operation seems to have
    an adverse effect on the system memory setup as things become
    corrupt (LOADing or MERGEing a program afterwards can cause a reset)

    Neither of these issues occur in other Spectrum emulators using
    the same ROMs and floppy images.

    TODO:

    there were many unofficial ROMs available for this, make them
    available for use.

*********************************************************************/

#include "emu.h"
#include "beta128.h"


/***************************************************************************
    DEVICE DEFINITIONS
***************************************************************************/

DEFINE_DEVICE_TYPE(SPECTRUM_BETA128, spectrum_beta128_device, "spectrum_beta128", "TR Beta 128 Disk Interface")


//-------------------------------------------------
//  INPUT_PORTS( beta128 )
//-------------------------------------------------

INPUT_PORTS_START(beta128)
	PORT_START("BUTTON") // don't use F12, it clashes with the 'exit from debugger' button
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Magic Button") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, spectrum_beta128_device, magic_button, 0)

	PORT_START("SWITCH")
	PORT_CONFNAME(0x03, 0x01, "System Switch") //PORT_CHANGED_MEMBER(DEVICE_SELF, spectrum_beta128_device, switch_changed, 0)
	PORT_CONFSETTING(0x00, "Off (128)")
	PORT_CONFSETTING(0x01, "Normal (auto-boot)")
	//PORT_CONFSETTING(0x02, "Reset") // TODO: implement RESET callback
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_beta128_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(beta128);
}

//-------------------------------------------------
//  SLOT_INTERFACE( beta_floppies )
//-------------------------------------------------

static void beta_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

//-------------------------------------------------
//  floppy_format_type floppy_formats
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER(spectrum_beta128_device::floppy_formats)
	FLOPPY_TRD_FORMAT
FLOPPY_FORMATS_END

//-------------------------------------------------
//  ROM( beta )
//-------------------------------------------------

ROM_START(beta128)
	ROM_REGION(0x4000, "rom", 0)
	ROM_DEFAULT_BIOS("trd504")
	ROM_SYSTEM_BIOS(0, "trd501", "TR-DOS v5.01")
	ROMX_LOAD("trd501.rom", 0x0000, 0x4000, CRC(3e3cdd4c) SHA1(8303ba0cc79daa6c04cd1e6ce27e8b6886a3f0de), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "trd503", "TR-DOS v5.03")
	ROMX_LOAD("trd503.rom", 0x0000, 0x4000, CRC(10751aba) SHA1(21695e3f2a8f796386ce66eea8a246b0ac44810c), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "trd504", "TR-DOS v5.04")
	ROMX_LOAD("trd504.rom", 0x0000, 0x4000, CRC(ba310874) SHA1(05e55e37df8eee6c68601ba9cf6c92195852ce3f), ROM_BIOS(2))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_beta128_device::device_add_mconfig(machine_config &config)
{
	FD1793(config, m_fdc, 4_MHz_XTAL / 4);
	//KR1818VG93(config, m_fdc, 4_MHz_XTAL / 4);

	FLOPPY_CONNECTOR(config, "fdc:0", beta_floppies, "525qd", spectrum_beta128_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", beta_floppies, "525qd", spectrum_beta128_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", beta_floppies, nullptr, spectrum_beta128_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", beta_floppies, nullptr, spectrum_beta128_device::floppy_formats).enable_sound(true);

	// passthru
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
}

const tiny_rom_entry *spectrum_beta128_device::device_rom_region() const
{
	return ROM_NAME(beta128);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_beta128_device - constructor
//-------------------------------------------------

spectrum_beta128_device::spectrum_beta128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_BETA128, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0)
	, m_exp(*this, "exp")
	, m_switch(*this, "SWITCH")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_beta128_device::device_start()
{
	save_item(NAME(m_romcs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_beta128_device::device_reset()
{
	// Page in the ROM if auto-boot is selected
	if (m_switch->read() == 0x01)
		m_romcs = 1;
	else
		m_romcs = 0;
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ_LINE_MEMBER(spectrum_beta128_device::romcs)
{
	return m_romcs | m_exp->romcs();
}


void spectrum_beta128_device::pre_opcode_fetch(offs_t offset)
{
	m_exp->pre_opcode_fetch(offset);

	if (!machine().side_effects_disabled())
	{
		if ((offset == 0x0066) || (offset & 0xff00) == 0x3d00)
			m_romcs = 1;
		else if (offset >= 0x4000)
			m_romcs = 0;
	}
}

uint8_t spectrum_beta128_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	if (m_romcs)
	{
		switch (offset & 0xff)
		{
		case 0x1f: case 0x3f: case 0x5f: case 0x7f:
			data = m_fdc->read((offset >> 5) & 0x03);
			break;

		case 0xff:
			data &= 0x3f; // actually open bus
			data |= m_fdc->drq_r() ? 0x40 : 0;
			data |= m_fdc->intrq_r() ? 0x80 : 0;
			break;
		}
	}
	return data;
}

void spectrum_beta128_device::iorq_w(offs_t offset, uint8_t data)
{
	if (m_romcs)
	{
		switch (offset & 0xff)
		{
		case 0x1f: case 0x3f: case 0x5f: case 0x7f:
			m_fdc->write((offset >> 5) & 0x03, data);
			break;

		case 0xff:
			floppy_image_device* floppy = m_floppy[data & 3]->get_device();

			m_fdc->set_floppy(floppy);
			if (floppy)
				floppy->ss_w(BIT(data, 4) ? 0 : 1);
			m_fdc->dden_w(BIT(data, 6));

			// bit 3 connected to pin 23 "HLT" of FDC and via diode to INDEX
			//m_fdc->hlt_w(BIT(data, 3)); // not handled in current wd_fdc

			if (BIT(data, 2) == 0) // reset
			{
				m_fdc->reset();
				if (floppy)
					floppy->mon_w(ASSERT_LINE);
			}
			else
			{
				// TODO: implement correct motor control, FDD motor and RDY FDC pin controlled by HLD pin of FDC
				if (floppy)
				 floppy->mon_w(CLEAR_LINE);
			}
			break;
		}
	}
	m_exp->iorq_w(offset, data);
}

uint8_t spectrum_beta128_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
	{
		data = m_rom->base()[offset & 0x3fff];
	}

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}

void spectrum_beta128_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_exp->romcs())
		m_exp->mreq_w(offset, data);
}

INPUT_CHANGED_MEMBER(spectrum_beta128_device::magic_button)
{
	if (newval && !oldval)
	{
		m_slot->nmi_w(ASSERT_LINE);
	}
	else
	{
		m_slot->nmi_w(CLEAR_LINE);
	}
}
