// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    Kempston Disc Interface
    (c) Abbeydale Designers ltd 1985

    Known clones:
     Sandy Disco Vers. 3

    Few useful K-DOS commands (should be preceded with PRINT #4: ):
     CAT - disc file list
     LOAD "filename" - load and run program
     COPY - tape to disc transfer utility
     FORMAT "discname": PRINT drive#, tracks#, sides#, steprate - format disc

	Manual https://archive.org/download/World_of_Spectrum_June_2017_Mirror/World%20of%20Spectrum%20June%202017%20Mirror.zip/World%20of%20Spectrum%20June%202017%20Mirror/sinclair/hardware-info/k/KempstonDiscInterface_Manual.pdf

    Notes/TODO:
     - schematics is missing, actual I/O ports decode might be not right
     - find out port 0xDF 3 MSB bits wiring, probably FDC /DDEN, /MR
     - implement onboard Kempston Centronics E interace and joystick port
     - add more docs/information

*********************************************************************/

#include "emu.h"
#include "kempdisc.h"


/***************************************************************************
    DEVICE DEFINITIONS
***************************************************************************/

DEFINE_DEVICE_TYPE(SPECTRUM_KEMPDISC, spectrum_kempdisc_device, "spectrum_kempdisc", "Kempston Disc Interface")


//-------------------------------------------------
//  SLOT_INTERFACE( floppies )
//-------------------------------------------------

static void kempdisc_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("525qd", FLOPPY_525_QD);
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("3dsdd", FLOPPY_3_DSDD);
}

//-------------------------------------------------
//  floppy_format_type floppy_formats
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER(spectrum_kempdisc_device::floppy_formats)
	FLOPPY_DSK_FORMAT
FLOPPY_FORMATS_END

//-------------------------------------------------
//  ROM( kempdisc )
//-------------------------------------------------

ROM_START(kempdisc)
	ROM_REGION(0x2000, "rom", 0)
	ROM_DEFAULT_BIOS("kd21")

	// original
	ROM_SYSTEM_BIOS(0, "kd20", "K-DOS v2.0")
	ROMX_LOAD("kd20.rom", 0x0000, 0x2000, CRC(244816a7) SHA1(b08e0e30f1db4f57d38b112be0115256528c6621), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "kd21", "K-DOS v2.1")
	ROMX_LOAD("kd21.rom", 0x0000, 0x2000, CRC(d18fb812) SHA1(b3e00bc4111ef6311789159024fb4cd25c32a72f), ROM_BIOS(1))
	// TODO add Italian clone
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_kempdisc_device::device_add_mconfig(machine_config &config)
{
	WD1770(config, m_fdc, 16_MHz_XTAL / 2);

	FLOPPY_CONNECTOR(config, "fdc:0", kempdisc_floppies, "525qd", spectrum_kempdisc_device::floppy_formats).enable_sound(true);	
	FLOPPY_CONNECTOR(config, "fdc:1", kempdisc_floppies, "525qd", spectrum_kempdisc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", kempdisc_floppies, nullptr, spectrum_kempdisc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", kempdisc_floppies, nullptr, spectrum_kempdisc_device::floppy_formats).enable_sound(true);

	// passthru
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
}

const tiny_rom_entry *spectrum_kempdisc_device::device_rom_region() const
{
	return ROM_NAME(kempdisc);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_kempdisc_device - constructor
//-------------------------------------------------

spectrum_kempdisc_device::spectrum_kempdisc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_KEMPDISC, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0)
	, m_exp(*this, "exp")
//	, m_control(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_kempdisc_device::device_start()
{
	save_item(NAME(m_romcs));
//	save_item(NAME(m_control));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_kempdisc_device::device_reset()
{
	m_romcs = 1;
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ_LINE_MEMBER(spectrum_kempdisc_device::romcs)
{
	return m_romcs | m_exp->romcs();
}


uint8_t spectrum_kempdisc_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	switch (offset & 0xff)
	{
	case 0xe5: case 0xe7: case 0xed: case 0xef:
		data &= m_fdc->read(BIT(offset, 1) | (BIT(offset, 3) << 1));
		break;
	}

	return data;
}

void spectrum_kempdisc_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xff)
	{
	case 0xe5: case 0xe7: case 0xed: case 0xef:
		m_fdc->write(BIT(offset, 1) | (BIT(offset, 3) << 1), data);
		break;
	case 0xdf:
	{
		floppy_image_device* floppy = nullptr;
		for (int i = 1; i < 5; i++)
			if (BIT(data, i))
			{
				floppy = m_floppy[i - 1]->get_device();
				break;
			}

//		m_control = data;
		m_fdc->set_floppy(floppy);
		if (floppy) floppy->ss_w(BIT(data, 0));
		if (data & 0xe0)
			logerror("port DF unhandled %02X\n", data);
	}
	break;
	case 0xfd:
		m_romcs = 1;
		break;
	case 0xd7:
		m_romcs = 0;
		break;
	}

	m_exp->iorq_w(offset, data);
}

uint8_t spectrum_kempdisc_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
		data = m_rom->base()[offset & 0x1fff];

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}
