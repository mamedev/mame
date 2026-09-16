// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Technology Research Beta 128 Disk interface

    This hardware type runs TR-DOS 5.xx (official) and newer
    unofficial updates.  It was designed to work properly with
    the 128k machines that had issues with the original Beta Disk
    due to changes in the 128k ROM structure etc. (enable address
    is moved from 3cxx to 3dxx for example)

    TODO:
    original ROMs should have bits 0 and 7 swapped

    there were many unofficial ROMs available for this, make them
    available for use.

*********************************************************************/

#include "emu.h"
#include "beta128.h"

#include "formats/trd_dsk.h"


/***************************************************************************
    DEVICE DEFINITIONS
***************************************************************************/

DEFINE_DEVICE_TYPE(SPECTRUM_BETA128, spectrum_beta128_device, "spectrum_beta128", "TR Beta 128 Disk Interface")


//-------------------------------------------------
//  INPUT_PORTS( beta128 )
//-------------------------------------------------

INPUT_PORTS_START(beta128)
	PORT_START("BUTTON") // don't use F12, it clashes with the 'exit from debugger' button
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Magic Button") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(spectrum_beta128_device::magic_button), 0)

	PORT_START("SWITCH")
	PORT_CONFNAME(0x03, 0x01, "System Switch") //PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(spectrum_beta128_device::switch_changed), 0)
	PORT_CONFSETTING(0x00, "Off (128)")
	PORT_CONFSETTING(0x01, "Normal (auto-boot)") // also enable Beta-disk V3/V4 compatibility, auto-boot feature does not work on Spectrum128.
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
//  floppy_formats
//-------------------------------------------------

void spectrum_beta128_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_TRD_FORMAT);
}

//-------------------------------------------------
//  ROM( beta )
//-------------------------------------------------

ROM_START(beta128)
	ROM_REGION(0x4000, "rom", 0)
	ROM_DEFAULT_BIOS("trd503")

	// original, but in plain form, should be replaced with "proper dumps" with data bits 0 and 7 swapped
	ROM_SYSTEM_BIOS(0, "trd501", "TR-DOS v5.01")
	ROMX_LOAD("trd501.rom", 0x0000, 0x4000, CRC(3e3cdd4c) SHA1(8303ba0cc79daa6c04cd1e6ce27e8b6886a3f0de), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "trd502", "TR-DOS v5.02")
	ROMX_LOAD("trd502.rom", 0x0000, 0x4000, CRC(64f0fcf8) SHA1(862e0af2245f68fca3d6a5b10186d09fd1faee55), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "trd503", "TR-DOS v5.03")
	ROMX_LOAD("trd503.rom", 0x0000, 0x4000, CRC(10751aba) SHA1(21695e3f2a8f796386ce66eea8a246b0ac44810c), ROM_BIOS(2))

	// clone/homebrew modifications based on original v5.03
	ROM_SYSTEM_BIOS(3, "trd504t", "TR-DOS v5.04T (hack)")
	// increased step rate (6ms), FORMAT command got interleave 1:1 option for faster read/write speed, this firmware was most common at post-soviet space in 90x.
	ROMX_LOAD("trd504t.rom", 0x0000, 0x4000, CRC(e212d1e0) SHA1(745e9caf576e64a5386ad845256d28593d34cc40), ROM_BIOS(3))
	// trd504.rom CRC ba310874 is bad dump of 5.03 with edited version text, no actual code changes.
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_beta128_device::device_add_mconfig(machine_config &config)
{
	FD1793(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->hld_wr_callback().set(FUNC(spectrum_beta128_device::fdc_hld_w));
	//KR1818VG93(config, m_fdc, 4_MHz_XTAL / 4);

	FLOPPY_CONNECTOR(config, "fdc:0", beta_floppies, "525qd", spectrum_beta128_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", beta_floppies, "525qd", spectrum_beta128_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", beta_floppies, nullptr, spectrum_beta128_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", beta_floppies, nullptr, spectrum_beta128_device::floppy_formats).enable_sound(true);

	// passthru
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
	m_exp->fb_r_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::fb_r));
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
	, m_control(0)
	, m_motor_active(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_beta128_device::device_start()
{
	save_item(NAME(m_romcs));
	save_item(NAME(m_control));
	save_item(NAME(m_motor_active));
	save_item(NAME(m_128rom_bit));
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

	m_128rom_bit = true;
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

bool spectrum_beta128_device::romcs()
{
	return m_romcs || m_exp->romcs();
}


void spectrum_beta128_device::pre_opcode_fetch(offs_t offset)
{
	m_exp->pre_opcode_fetch(offset);

	if (!machine().side_effects_disabled())
	{
		u8 offs = offset >> 8;

		if (offs == 0x3d && m_128rom_bit)
			m_romcs = 1;
		else if (offs == 0x3c && m_128rom_bit && m_switch->read() == 0x01)
			m_romcs = 1;
		else if (offs >= 0x40)
			m_romcs = 0;
	}
}

uint8_t spectrum_beta128_device::iorq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
	{
		switch (offset & 0x83)
		{
		case 0x03:
			data = m_fdc->read((offset >> 5) & 0x03);
			break;

		case 0x83:
			data &= 0x3f; // actually open bus
			data |= m_fdc->drq_r() ? 0x40 : 0;
			data |= m_fdc->intrq_r() ? 0x80 : 0;
			break;
		}
	} else
		data = m_exp->iorq_r(offset);

	return data;
}

void spectrum_beta128_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 0x8002) == 0)
		m_128rom_bit = bool(data & 0x10);

	if (m_romcs)
	{
		switch (offset & 0x83)
		{
		case 0x03:
			m_fdc->write((offset >> 5) & 0x03, data);
			break;

		case 0x83:
			floppy_image_device* floppy = m_floppy[data & 3]->get_device();

			m_control = data;
			m_fdc->set_floppy(floppy);
			if (floppy)
				floppy->ss_w(BIT(data, 4) ? 0 : 1);
			m_fdc->dden_w(BIT(data, 6));

			m_fdc->hlt_w(BIT(data, 3));
			// bit 3 also connected to FDC /IP pin via diode, AND logic: if this bit is 0 - /IP will be forcibly set to low.
			// used for bitbang index pulses generation to stop FDD drive motor with no disk inserted, currently not emulated.

			m_fdc->mr_w(BIT(data, 2));
			motors_control();
			break;
		}
	}
	else
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

INPUT_CHANGED_MEMBER(spectrum_beta128_device::magic_button)
{
	if (newval && !oldval)
	{
		m_romcs = 1;
		m_slot->nmi_w(ASSERT_LINE);
	}
	else
	{
		m_slot->nmi_w(CLEAR_LINE);
	}
}

void spectrum_beta128_device::fdc_hld_w(int state)
{
	m_fdc->set_force_ready(state); // HLD connected to RDY pin
	m_motor_active = state;
	motors_control();
}

void spectrum_beta128_device::motors_control()
{
	for (int i = 0; i < 4; i++)
	{
		floppy_image_device* floppy = m_floppy[i]->get_device();
		if (!floppy)
			continue;
		if (m_motor_active && (m_control & 3) == i)
			floppy->mon_w(CLEAR_LINE);
		else
			floppy->mon_w(ASSERT_LINE);
	}
}
