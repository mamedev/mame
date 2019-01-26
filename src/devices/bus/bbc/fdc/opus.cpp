// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Opus Floppy Disc Controllers

    8272: https://www.youtube.com/watch?v=09alLIz16ck
    EDOS: http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Opus_DiscController.html
    2791: http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Opus_DDoss.html
    2793:
    1770: http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Opus_D-DOS.html

**********************************************************************/


#include "emu.h"
#include "opus.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_OPUS8272, bbc_opus8272_device, "bbc_opus8272", "Opus 8272 FDC")
DEFINE_DEVICE_TYPE(BBC_OPUS2791, bbc_opus2791_device, "bbc_opus2791", "Opus 2791 FDC")
DEFINE_DEVICE_TYPE(BBC_OPUS2793, bbc_opus2793_device, "bbc_opus2793", "Opus 2793 FDC")
DEFINE_DEVICE_TYPE(BBC_OPUS1770, bbc_opus1770_device, "bbc_opus1770", "Opus D-DOS(B) 1770 FDC")


//-------------------------------------------------
//  MACHINE_DRIVER( opus2791 )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( bbc_opusfdc_device::floppy_formats )
	FLOPPY_ACORN_SSD_FORMAT,
	FLOPPY_ACORN_DSD_FORMAT,
	FLOPPY_FSD_FORMAT,
	FLOPPY_OPUS_DDOS_FORMAT,
	FLOPPY_OPUS_DDCPM_FORMAT
FLOPPY_FORMATS_END0

static void bbc_floppies_525(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd",   FLOPPY_525_SD);
	device.option_add("525ssdd", FLOPPY_525_SSDD);
	device.option_add("525dd",   FLOPPY_525_DD);
	device.option_add("525qd",   FLOPPY_525_QD);
}

ROM_START( opus8272 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("ddos305")
	ROM_SYSTEM_BIOS(0, "ddos300", "Opus DDOS 3.00")
	ROMX_LOAD("opus-ddos300.rom", 0x0000, 0x4000, CRC(1b5fa131) SHA1(6b4e0363a9d39807973a2ef0871a78b287cea27e), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "ddos305", "Opus DDOS 3.05")
	ROMX_LOAD("opus-ddos305.rom", 0x0000, 0x4000, CRC(43c75fa4) SHA1(0b7194a234c2316ba825e878e3f69928bf3bb595), ROM_BIOS(1))
ROM_END

ROM_START( opus2791 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("ddos316")
	ROM_SYSTEM_BIOS(0, "ddos312", "Opus DDOS 3.12")
	ROMX_LOAD("opus-ddos312.rom", 0x0000, 0x4000, CRC(b2c393be) SHA1(6accaa0b13b0b939c86674cddd7bee1aea2f66cb), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "ddos315", "Opus DDOS 3.15")
	ROMX_LOAD("opus-ddos315.rom", 0x0000, 0x4000, CRC(5f06701c) SHA1(9e250dc7ddcde35b19e8f29f2cfe95a79f46d473), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "ddos316", "Opus DDOS 3.16")
	ROMX_LOAD("opus-ddos316.rom", 0x0000, 0x4000, CRC(268ebc0d) SHA1(e608f6e40a5579147cc631f351aae275fdabec5b), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "edos04", "Opus EDOS 0.4")
	ROMX_LOAD("opus-edos04.rom", 0x0000, 0x4000, CRC(1d8a3860) SHA1(05f461464707b4ca24636c9e726af561f227ccdb), ROM_BIOS(3))
ROM_END

ROM_START( opus2793 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("ddos336")
	ROM_SYSTEM_BIOS(0, "ddos335", "Opus DDOS 3.35")
	ROMX_LOAD("opus-ddos335.rom", 0x0000, 0x4000, CRC(e33167fb) SHA1(42fbc9932db2087708da41cb1ffa94358683cf7a), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "ddos336", "Opus DDOS 3.36")
	ROMX_LOAD("opus-ddos336.rom", 0x0000, 0x4000, CRC(2f400f69) SHA1(a6a57bb907d6b9bd351029fb0471a3a9c343da24), ROM_BIOS(1))
ROM_END

ROM_START( opus1770 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("ddos346")
	ROM_SYSTEM_BIOS(0, "ddos345", "Opus DDOS 3.45")
	ROMX_LOAD("opus-ddos345.rom", 0x0000, 0x4000, CRC(c0163b95) SHA1(1c5a68e08abbb7ffe663151c59088f750d2287a9), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "ddos346", "Opus DDOS 3.46")
	ROMX_LOAD("opus-ddos346.rom", 0x0000, 0x4000, CRC(bf9c35cf) SHA1(a1ad3e9acbd15400e7da1e50bc6673835cde1fe7), ROM_BIOS(1))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_opus8272_device::device_add_mconfig(machine_config &config)
{
	I8272A(config, m_fdc, 20_MHz_XTAL / 5, true);
	m_fdc->intrq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_fdc_slot_device::intrq_w));

	FLOPPY_CONNECTOR(config, m_floppy0, bbc_floppies_525, "525qd", bbc_opusfdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, bbc_floppies_525, "525qd", bbc_opusfdc_device::floppy_formats).enable_sound(true);
}

void bbc_opus2791_device::device_add_mconfig(machine_config &config)
{
	WD2791(config, m_fdc, DERIVED_CLOCK(1, 8));
	m_fdc->drq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_fdc_slot_device::drq_w));
	m_fdc->hld_wr_callback().set(FUNC(bbc_opusfdc_device::motor_w));

	FLOPPY_CONNECTOR(config, m_floppy0, bbc_floppies_525, "525qd", bbc_opusfdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, bbc_floppies_525, "525qd", bbc_opusfdc_device::floppy_formats).enable_sound(true);
}

void bbc_opus2793_device::device_add_mconfig(machine_config &config)
{
	WD2793(config, m_fdc, DERIVED_CLOCK(1, 8));
	m_fdc->drq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_fdc_slot_device::drq_w));
	m_fdc->hld_wr_callback().set(FUNC(bbc_opusfdc_device::motor_w));

	FLOPPY_CONNECTOR(config, m_floppy0, bbc_floppies_525, "525qd", bbc_opusfdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, bbc_floppies_525, "525qd", bbc_opusfdc_device::floppy_formats).enable_sound(true);
}

void bbc_opus1770_device::device_add_mconfig(machine_config &config)
{
	WD1770(config, m_fdc, DERIVED_CLOCK(1, 1));
	m_fdc->drq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_fdc_slot_device::drq_w));

	FLOPPY_CONNECTOR(config, m_floppy0, bbc_floppies_525, "525qd", bbc_opusfdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, bbc_floppies_525, "525qd", bbc_opusfdc_device::floppy_formats).enable_sound(true);
}

const tiny_rom_entry *bbc_opus8272_device::device_rom_region() const
{
	return ROM_NAME( opus8272 );
}

const tiny_rom_entry *bbc_opus2791_device::device_rom_region() const
{
	return ROM_NAME( opus2791 );
}

const tiny_rom_entry *bbc_opus2793_device::device_rom_region() const
{
	return ROM_NAME( opus2793 );
}

const tiny_rom_entry *bbc_opus1770_device::device_rom_region() const
{
	return ROM_NAME( opus1770 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_opusfdc_device - constructor
//-------------------------------------------------

bbc_opus8272_device::bbc_opus8272_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BBC_OPUS8272, tag, owner, clock),
	device_bbc_fdc_interface(mconfig, *this),
	m_fdc(*this, "i8272"),
	m_floppy0(*this, "i8272:0"),
	m_floppy1(*this, "i8272:1")
{
}

bbc_opusfdc_device::bbc_opusfdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_bbc_fdc_interface(mconfig, *this),
	m_fdc(*this, "fdc"),
	m_floppy0(*this, "fdc:0"),
	m_floppy1(*this, "fdc:1"),
	m_drive_control(0)
{
}

bbc_opus2791_device::bbc_opus2791_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_opusfdc_device(mconfig, BBC_OPUS2791, tag, owner, clock)
{
}

bbc_opus2793_device::bbc_opus2793_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_opusfdc_device(mconfig, BBC_OPUS2793, tag, owner, clock)
{
}

bbc_opus1770_device::bbc_opus1770_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_opusfdc_device(mconfig, BBC_OPUS1770, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_opus8272_device::device_start()
{
}

void bbc_opusfdc_device::device_start()
{
	save_item(NAME(m_drive_control));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_opus8272_device::read)
{
	uint8_t data = 0xff;

	switch (offset & 0x07)
	{
	case 0x02:
		m_fdc->tc_w(true);
		m_fdc->tc_w(false);
		break;

	case 0x06:
		if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(1);
		if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(1);
	case 0x04:
		data = m_fdc->msr_r(space, 0);
		break;

	case 0x05:
		if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(0);
		if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(0);
	case 0x07:
		data = m_fdc->fifo_r(space, 0);
		break;
	}
	return data;
}

WRITE8_MEMBER(bbc_opus8272_device::write)
{
	floppy_image_device *floppy = nullptr;

	switch (offset & 0x07)
	{
	case 0x01:
		switch (data & 0x01)
		{
		case 0: floppy = m_floppy1->get_device(); break;
		case 1: floppy = m_floppy0->get_device(); break;
		}
		m_fdc->set_floppy(floppy);
		break;

	case 0x05:
		if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(0);
		if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(0);
	case 0x07:
		m_fdc->fifo_w(space, 0, data);
		break;
	}
}


READ8_MEMBER(bbc_opusfdc_device::read)
{
	uint8_t data;

	if (offset & 0x04)
	{
		data = m_drive_control;
	}
	else
	{
		data = m_fdc->read(offset & 0x03);
	}
	return data;
}

WRITE8_MEMBER(bbc_opusfdc_device::write)
{
	if (offset & 0x04)
	{
		floppy_image_device *floppy = nullptr;

		m_drive_control = data;

		// bit 0: drive select
		switch (BIT(data, 0))
		{
		case 0: floppy = m_floppy0->get_device(); break;
		case 1: floppy = m_floppy1->get_device(); break;
		}
		m_fdc->set_floppy(floppy);

		// bit 1: side select
		if (floppy)
			floppy->ss_w(BIT(data, 1));

		// bit 6: density
		m_fdc->dden_w(!BIT(data, 6));
	}
	else
	{
		m_fdc->write(offset & 0x03, data);
	}
}

WRITE_LINE_MEMBER(bbc_opusfdc_device::motor_w)
{
	if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(!state);
	if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(!state);
}
