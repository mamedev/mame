// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Kenda Professional DMFS

    The Kenda Professional is packaged with the board in an epoxy blob, and is
    quite rare. The board contents are not known as no-one has attempted to
    remove the epoxy.
    What we do know is that it contains:
    - 8K ROM
    - 2K RAM
    - FDC (definitely WD compatible, and likely a WD2793)

    The board plugs into the usual 8271 socket and has a ribbon cable with 24 pin
    header that plugs into a ROM socket.

    The 8K ROM is mirrored to fill the 16K ROM space, and has the 2K RAM overlayed
    from offset &3000, and also mirrored upto &4000.

**********************************************************************/


#include "emu.h"
#include "kenda.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_KENDA, bbc_kenda_device,  "bbc_kenda", "Kenda Professional DMFS")


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( bbc_kenda_device::floppy_formats )
	FLOPPY_ACORN_SSD_FORMAT,
	FLOPPY_ACORN_DSD_FORMAT
FLOPPY_FORMATS_END

//-------------------------------------------------
//  SLOT_INTERFACE( bbc_floppies_525 )
//-------------------------------------------------

static void bbc_floppies_525(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd",   FLOPPY_525_SD);
	device.option_add("525ssdd", FLOPPY_525_SSDD);
	device.option_add("525dd",   FLOPPY_525_DD);
	device.option_add("525qd",   FLOPPY_525_QD);
}

//-------------------------------------------------
//  ROM( kenda )
//-------------------------------------------------

ROM_START( kenda )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_LOAD("kenda102.rom", 0x0000, 0x2000, CRC(430b911c) SHA1(594ae1d1aeaa20a1d5d1c64cd94d43926dda4029))
	ROM_RELOAD(              0x2000, 0x2000)
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_kenda_device::device_add_mconfig(machine_config &config)
{
	WD2793(config, m_fdc, DERIVED_CLOCK(1, 8)); // TODO: unconfirmed FDC
	m_fdc->intrq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_fdc_slot_device::intrq_w));
	m_fdc->drq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_fdc_slot_device::drq_w));
	m_fdc->hld_wr_callback().set(FUNC(bbc_kenda_device::motor_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], bbc_floppies_525, "525qd", floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], bbc_floppies_525, "525qd", floppy_formats).enable_sound(true);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_kenda_device::device_rom_region() const
{
	return ROM_NAME( kenda );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_kenda_device - constructor
//-------------------------------------------------

bbc_kenda_device::bbc_kenda_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_KENDA, tag, owner, clock)
	, device_bbc_fdc_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_kenda_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_kenda_device::read(offs_t offset)
{
	uint8_t data;

	if (offset & 0x04)
	{
		data = 0xfe;
	}
	else
	{
		data = m_fdc->read(offset & 0x03);
	}

	return data;
}

void bbc_kenda_device::write(offs_t offset, uint8_t data)
{
	if (offset & 0x04)
	{
		floppy_image_device *floppy = nullptr;

		// bit 0: drive select
		floppy = m_floppy[BIT(data, 0)]->get_device();
		m_fdc->set_floppy(floppy);

		// bit 1: side select
		if (floppy)
			floppy->ss_w(BIT(data, 1));

		// other bits unknown, or unused

		// bit 7: density
		m_fdc->dden_w(!BIT(data, 7));
	}
	else
	{
		m_fdc->write(offset & 0x03, data);
	}
}

WRITE_LINE_MEMBER(bbc_kenda_device::motor_w)
{
	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->mon_w(!state);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->mon_w(!state);
}
