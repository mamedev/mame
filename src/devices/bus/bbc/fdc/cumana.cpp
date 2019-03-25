// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Cumana QFS 8877A FDC

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Cumana_QFS.html

**********************************************************************/


#include "emu.h"
#include "cumana.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_CUMANA1, bbc_cumana1_device, "bbc_cumana1", "Cumana QFS 8877A FDC")
DEFINE_DEVICE_TYPE(BBC_CUMANA2, bbc_cumana2_device, "bbc_cumana2", "Cumana QFS Issue 2 8877A FDC")


//-------------------------------------------------
//  MACHINE_DRIVER( cumana )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( bbc_cumanafdc_device::floppy_formats )
	FLOPPY_ACORN_SSD_FORMAT,
	FLOPPY_ACORN_DSD_FORMAT,
	FLOPPY_FSD_FORMAT
FLOPPY_FORMATS_END0

static void bbc_floppies_525(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd",   FLOPPY_525_SD);
	device.option_add("525ssdd", FLOPPY_525_SSDD);
	device.option_add("525dd",   FLOPPY_525_DD);
	device.option_add("525qd",   FLOPPY_525_QD);
}

ROM_START( cumana1 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("qfs102")
	ROM_SYSTEM_BIOS(0, "qfs102", "QFS 1.02")
	ROMX_LOAD("qfs102.rom", 0x0000, 0x4000, CRC(083a9285) SHA1(4bf31a420a9d752285b088ee4f08a64563527662), ROM_BIOS(0))
ROM_END

ROM_START( cumana2 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("qfs200")
	ROM_SYSTEM_BIOS(0, "qfs200", "QFS 2.00")
	ROMX_LOAD("qfs200.rom", 0x0000, 0x4000, CRC(5863962a) SHA1(0dced741482321938842746ee47090ae443d7ad5), ROM_BIOS(0))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_cumana1_device::device_add_mconfig(machine_config &config)
{
	MB8877(config, m_fdc, DERIVED_CLOCK(1, 8));
	m_fdc->intrq_wr_callback().set(FUNC(bbc_cumanafdc_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(bbc_cumanafdc_device::fdc_drq_w));
	m_fdc->hld_wr_callback().set(FUNC(bbc_cumanafdc_device::motor_w));

	FLOPPY_CONNECTOR(config, m_floppy0, bbc_floppies_525, "525qd", floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, bbc_floppies_525, "525qd", floppy_formats).enable_sound(true);
}

void bbc_cumana2_device::device_add_mconfig(machine_config &config)
{
	MB8877(config, m_fdc, DERIVED_CLOCK(1, 8));
	m_fdc->intrq_wr_callback().set(FUNC(bbc_cumanafdc_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(bbc_cumanafdc_device::fdc_drq_w));
	m_fdc->hld_wr_callback().set(FUNC(bbc_cumanafdc_device::motor_w));

	FLOPPY_CONNECTOR(config, m_floppy0, bbc_floppies_525, "525qd", floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, bbc_floppies_525, "525qd", floppy_formats).enable_sound(true);
}

const tiny_rom_entry *bbc_cumana1_device::device_rom_region() const
{
	return ROM_NAME( cumana1 );
}

const tiny_rom_entry *bbc_cumana2_device::device_rom_region() const
{
	return ROM_NAME( cumana2 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_cumanafdc_device - constructor
//-------------------------------------------------

bbc_cumanafdc_device::bbc_cumanafdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_bbc_fdc_interface(mconfig, *this),
	m_fdc(*this, "mb8877a"),
	m_floppy0(*this, "mb8877a:0"),
	m_floppy1(*this, "mb8877a:1"),
	m_drive_control(0)
{
}

bbc_cumana1_device::bbc_cumana1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbc_cumanafdc_device(mconfig, BBC_CUMANA1, tag, owner, clock)
{
	m_invert = true;
}

bbc_cumana2_device::bbc_cumana2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbc_cumanafdc_device(mconfig, BBC_CUMANA2, tag, owner, clock)
{
	m_invert = false;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_cumanafdc_device::device_start()
{
	save_item(NAME(m_drive_control));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_cumanafdc_device::read)
{
	uint8_t data;

	if (offset & 0x04)
	{
		data = m_fdc->read(offset & 0x03);
	}
	else
	{
		data = m_drive_control;
	}
	return data;
}

WRITE8_MEMBER(bbc_cumanafdc_device::write)
{
	if (offset & 0x04)
	{
		m_fdc->write(offset & 0x03, data);
	}
	else
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

		// Not sure why QFS 1.02 inverts these two bits, need to see an original board to verify
		if (m_invert)
		{
			// bit 1: side select
			if (floppy)
				floppy->ss_w(!BIT(data, 1));

			// bit 2: density
			m_fdc->dden_w(!BIT(data, 2));
		}
		else
		{
			// bit 1: side select
			if (floppy)
				floppy->ss_w(BIT(data, 1));

			// bit 2: density
			m_fdc->dden_w(BIT(data, 2));
		}
		// bit 3: reset
		if (BIT(data, 3)) m_fdc->soft_reset();

		// bit 4: interrupt enable
		m_fdc_ie = BIT(data, 4);
	}
}

WRITE_LINE_MEMBER(bbc_cumanafdc_device::fdc_intrq_w)
{
	if (m_fdc_ie)
		m_slot->intrq_w(state);
}

WRITE_LINE_MEMBER(bbc_cumanafdc_device::fdc_drq_w)
{
	if (m_fdc_ie)
		m_slot->drq_w(state);
}

WRITE_LINE_MEMBER(bbc_cumanafdc_device::motor_w)
{
	if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(!state);
	if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(!state);
}
