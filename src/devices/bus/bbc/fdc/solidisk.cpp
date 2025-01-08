// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Solidisk 1770 Issue 2 FDC

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Solidisk_1770DDFS.html

    Solidisk Dual FDC
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Solidisk_dfdc.html

**********************************************************************/

#include "emu.h"
#include "solidisk.h"

#include "formats/acorn_dsk.h"
#include "formats/fsd_dsk.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_STL1770_1, bbc_stl1770_1_device, "bbc_stl1770_1", "Solidisk 1770 DDFS Issue 1 FDC")
DEFINE_DEVICE_TYPE(BBC_STL1770_2, bbc_stl1770_2_device, "bbc_stl1770_2", "Solidisk 1770 DDFS Issue 2 FDC")
DEFINE_DEVICE_TYPE(BBC_STLDFDC_1, bbc_stldfdc_1_device, "bbc_stldfdc_1", "Solidisk 8271/1770 DFDC Issue 1 FDC")


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

void bbc_stlfdc_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ACORN_SSD_FORMAT);
	fr.add(FLOPPY_ACORN_DSD_FORMAT);
	fr.add(FLOPPY_ACORN_ADFS_OLD_FORMAT);
	fr.add(FLOPPY_FSD_FORMAT);
}

static void bbc_floppies_525(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd",   FLOPPY_525_SD);
	device.option_add("525ssdd", FLOPPY_525_SSDD);
	device.option_add("525dd",   FLOPPY_525_DD);
	device.option_add("525qd",   FLOPPY_525_QD);
}

//-------------------------------------------------
//  INPUT_PORTS( stldfdc )
//-------------------------------------------------

INPUT_PORTS_START( stldfdc )
	PORT_START("DFDC")
	PORT_CONFNAME(0x01, 0x00, "Dual FDC Select") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bbc_stlfdc_device::fdc_changed), 0)
	PORT_CONFSETTING(0x00, "8271")
	PORT_CONFSETTING(0x01, "1770")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_stldfdc_1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(stldfdc);
}

//-------------------------------------------------
//  ROM( solidisk )
//-------------------------------------------------

ROM_START( stl1770_1 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_SYSTEM_BIOS(0, "dfs21j", "DFS 2.1J (1770)")
	ROMX_LOAD("solidisk-dfs-2.1j.rom", 0x0000, 0x4000, CRC(b60296f1) SHA1(22e60d4dda37335f61c27a92ba76ccc3e2102c4e), ROM_BIOS(0))
ROM_END

ROM_START( stl1770_2 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_SYSTEM_BIOS(0, "stl22m", "DFS 2.2M Issue 2")
	ROMX_LOAD("solidisk-dfs-2.2m-iss2.rom", 0x0000, 0x4000, CRC(defe42ec) SHA1(78ee759c0f762bc2717d8454a88f117516d28325), ROM_BIOS(0))
ROM_END

ROM_START( stldfdc_1 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_SYSTEM_BIOS(0, "dfs21", "DFS 2.1 (Mar 85)")
	ROMX_LOAD("solidisk-dfs-8271-1770-2.1.rom", 0x0000, 0x4000, CRC(cc83d913) SHA1(c761102ac11be629676339e9e496fb78734e4399), ROM_BIOS(0))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_stl1770_1_device::device_add_mconfig(machine_config &config)
{
	WD1770(config, m_wd1770, DERIVED_CLOCK(1, 1));
	m_wd1770->drq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_fdc_slot_device::drq_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], bbc_floppies_525, "525qd", bbc_stlfdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], bbc_floppies_525, "525qd", bbc_stlfdc_device::floppy_formats).enable_sound(true);
}

void bbc_stl1770_2_device::device_add_mconfig(machine_config &config)
{
	WD1770(config, m_wd1770, DERIVED_CLOCK(1, 1));
	m_wd1770->intrq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_fdc_slot_device::intrq_w));
	m_wd1770->drq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_fdc_slot_device::drq_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], bbc_floppies_525, "525qd", bbc_stlfdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], bbc_floppies_525, "525qd", bbc_stlfdc_device::floppy_formats).enable_sound(true);
}

void bbc_stldfdc_1_device::device_add_mconfig(machine_config& config)
{
	bbc_stl1770_1_device::device_add_mconfig(config);

	I8271(config, m_i8271, DERIVED_CLOCK(1, 2));
	m_i8271->intrq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_fdc_slot_device::intrq_w));
	m_i8271->hdl_wr_callback().set(FUNC(bbc_stldfdc_1_device::motor_w));
	m_i8271->opt_wr_callback().set(FUNC(bbc_stldfdc_1_device::side_w));
}


const tiny_rom_entry *bbc_stl1770_1_device::device_rom_region() const
{
	return ROM_NAME( stl1770_1 );
}

const tiny_rom_entry *bbc_stl1770_2_device::device_rom_region() const
{
	return ROM_NAME( stl1770_2 );
}

const tiny_rom_entry *bbc_stldfdc_1_device::device_rom_region() const
{
	return ROM_NAME( stldfdc_1 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_stlfdc_device - constructor
//-------------------------------------------------

bbc_stlfdc_device::bbc_stlfdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_fdc_interface(mconfig, *this)
	, m_wd1770(*this, "wd1770")
	, m_i8271(*this, "i8271")
	, m_floppy(*this, "%u", 0)
	, m_dfdc(*this, "DFDC")
{
}

bbc_stl1770_1_device::bbc_stl1770_1_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock)
	: bbc_stlfdc_device(mconfig, type, tag, owner, clock)
{
}

bbc_stl1770_1_device::bbc_stl1770_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_stlfdc_device(mconfig, BBC_STL1770_1, tag, owner, clock)
{
}

bbc_stl1770_2_device::bbc_stl1770_2_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock)
	: bbc_stlfdc_device(mconfig, type, tag, owner, clock)
{
}

bbc_stl1770_2_device::bbc_stl1770_2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_stlfdc_device(mconfig, BBC_STL1770_2, tag, owner, clock)
{
}

bbc_stldfdc_1_device::bbc_stldfdc_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_stl1770_1_device(mconfig, BBC_STLDFDC_1, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_stl1770_1_device::device_start()
{
}

void bbc_stl1770_2_device::device_start()
{
}

void bbc_stldfdc_1_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_stldfdc_1_device::device_reset()
{
	switch (m_dfdc->read())
	{
	case 0:
		m_i8271->set_floppies(m_floppy[0], m_floppy[1]);
		break;

	case 1:
		m_wd1770->set_floppy(m_floppy[0]->get_device());
		break;
	}
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

INPUT_CHANGED_MEMBER(bbc_stlfdc_device::fdc_changed)
{
	device_reset();
}

void bbc_stlfdc_device::motor_w(int state)
{
	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->mon_w(!state);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->mon_w(!state);
	m_i8271->ready_w(!state);
}

void bbc_stlfdc_device::side_w(int state)
{
	if (m_floppy[0]->get_device()) m_floppy[0]->get_device()->ss_w(state);
	if (m_floppy[1]->get_device()) m_floppy[1]->get_device()->ss_w(state);
}


uint8_t bbc_stl1770_1_device::read(offs_t offset)
{
	return m_wd1770->read(offset & 0x03);
}

void bbc_stl1770_1_device::write(offs_t offset, uint8_t data)
{
	if (offset & 0x04)
	{
		floppy_image_device *floppy = nullptr;

		// bit 0: drive select
		switch (BIT(data, 0))
		{
		case 0: floppy = m_floppy[0]->get_device(); break;
		case 1: floppy = m_floppy[1]->get_device(); break;
		}
		m_wd1770->set_floppy(floppy);

		// bit 1: side select
		if (floppy)
			floppy->ss_w(BIT(data, 1));

		// bit 2: density
		m_wd1770->dden_w(BIT(data, 2));
	}

	m_wd1770->write(offset & 0x03, data);
}


uint8_t bbc_stl1770_2_device::read(offs_t offset)
{
	uint8_t data;

	if (offset & 0x04)
	{
		data = m_wd1770->read(offset & 0x03);
	}
	else
	{
		data = 0xfe;
	}
	return data;
}

void bbc_stl1770_2_device::write(offs_t offset, uint8_t data)
{
	if (offset & 0x04)
	{
		m_wd1770->write(offset & 0x03, data);
	}
	else
	{
		floppy_image_device *floppy = nullptr;

		// bit 0, 1: drive select
		if (BIT(data, 0)) floppy = m_floppy[0]->get_device();
		if (BIT(data, 1)) floppy = m_floppy[1]->get_device();
		m_wd1770->set_floppy(floppy);

		// bit 2: side select
		if (floppy)
			floppy->ss_w(BIT(data, 2));

		// bit 3: density
		m_wd1770->dden_w(BIT(data, 3));

		// bit 5: reset
		m_wd1770->mr_w(BIT(data, 5));
	}
}


uint8_t bbc_stldfdc_1_device::read(offs_t offset)
{
	uint8_t data = 0xff;

	switch (m_dfdc->read())
	{
	case 0:
		if (offset & 0x04)
		{
			data = m_i8271->data_r();
		}
		else
		{
			data = m_i8271->read(offset & 0x03);
		}
		break;

	case 1:
		data = bbc_stl1770_1_device::read(offset);
		break;
	}

	return data;
}

void bbc_stldfdc_1_device::write(offs_t offset, uint8_t data)
{
	switch (m_dfdc->read())
	{
	case 0:
		if (offset & 0x04)
		{
			m_i8271->data_w(data);
		}
		else
		{
			m_i8271->write(offset & 0x03, data);
		}
		break;

	case 1:
		bbc_stl1770_1_device::write(offset, data);
		break;
	}
}
