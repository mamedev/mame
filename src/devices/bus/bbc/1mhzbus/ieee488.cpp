// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn IEEE-488 Interface

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANK01_IEEE488IF.html

    Aries-B488

    http://www.beebmaster.co.uk/CheeseWedges/AriesB488.html

    CST Procyon IEEE Interface

**********************************************************************/


#include "emu.h"
#include "ieee488.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_IEEE488, bbc_ieee488_device, "bbc_ieee488", "Acorn IEEE-488 Interface");
DEFINE_DEVICE_TYPE(BBC_B488,    bbc_b488_device,    "bbc_b488",    "Aries-B488");
//DEFINE_DEVICE_TYPE(BBC_PROCYON, bbc_procyon_device, "bbc_procyon", "CST Procyon IEEE Interface");


//-------------------------------------------------
//  ROM( ieee488 )
//-------------------------------------------------

ROM_START(ieee488)
	ROM_REGION(0x2000, "exp_rom", 0)
	ROM_DEFAULT_BIOS("ieee05")
	ROM_SYSTEM_BIOS(0, "ieee05", "IEEEFS 0.05")
	ROMX_LOAD("ieeefs-0.5.rom", 0x0000, 0x2000, CRC(4c2e80ee) SHA1(0bf33f0378657cf35a49f4f74be40c26ebcb9de5), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "ieee02", "IEEEFS 0.02")
	ROMX_LOAD("ieeefs-0.2.rom", 0x0000, 0x2000, CRC(73a7960c) SHA1(86828db58e6d1d0a9aade9e73fb1fafd5b6aedff), ROM_BIOS(1))
ROM_END

//ROM_START(procyon)
//  ROM_REGION(0x2000, "exp_rom", 0)
//  ROM_DEFAULT_BIOS("ieee198")
//  ROM_SYSTEM_BIOS(0, "ieee198", "IEEE 1.98")
//  ROMX_LOAD("ieee-1.98.rom", 0x0000, 0x2000, CRC(c2bbe17b) SHA1(96930b54d987dd1e4a87f546f7cd65fc1f0b9578), ROM_BIOS(0))
//  ROM_SYSTEM_BIOS(1, "ieee192", "IEEE 1.92")
//  ROMX_LOAD("ieee-1.92.rom", 0x0000, 0x2000, CRC(87e5f701) SHA1(673eab99031ca88aa90e6deade39b653f8c6b9da), ROM_BIOS(1))
//ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(bbc_ieee488_device::device_add_mconfig)
	TMS9914(config, m_tms9914, 5_MHz_XTAL);
	m_tms9914->int_write_cb().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_tms9914->dio_read_cb().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	m_tms9914->dio_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));
	m_tms9914->eoi_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	m_tms9914->dav_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	m_tms9914->nrfd_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	m_tms9914->ndac_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	m_tms9914->ifc_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	m_tms9914->srq_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));
	m_tms9914->atn_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	m_tms9914->ren_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));
	MCFG_IEEE488_BUS_ADD()
	MCFG_IEEE488_EOI_CALLBACK(WRITELINE(m_tms9914, tms9914_device, eoi_w))
	MCFG_IEEE488_DAV_CALLBACK(WRITELINE(m_tms9914, tms9914_device, dav_w))
	MCFG_IEEE488_NRFD_CALLBACK(WRITELINE(m_tms9914, tms9914_device, nrfd_w))
	MCFG_IEEE488_NDAC_CALLBACK(WRITELINE(m_tms9914, tms9914_device, ndac_w))
	MCFG_IEEE488_IFC_CALLBACK(WRITELINE(m_tms9914, tms9914_device, ifc_w))
	MCFG_IEEE488_SRQ_CALLBACK(WRITELINE(m_tms9914, tms9914_device, srq_w))
	MCFG_IEEE488_ATN_CALLBACK(WRITELINE(m_tms9914, tms9914_device, atn_w))
	MCFG_IEEE488_REN_CALLBACK(WRITELINE(m_tms9914, tms9914_device, ren_w))
	MCFG_IEEE488_SLOT_ADD("ieee_dev", 0, cbm_ieee488_devices, nullptr)

	BBC_1MHZBUS_SLOT(config, m_1mhzbus, bbc_1mhzbus_devices, nullptr);
	m_1mhzbus->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_1mhzbus->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::nmi_w));
MACHINE_CONFIG_END

MACHINE_CONFIG_START(bbc_b488_device::device_add_mconfig)
	TMS9914(config, m_tms9914, 5_MHz_XTAL); // TODO: verify clock
	m_tms9914->int_write_cb().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_tms9914->dio_read_cb().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	m_tms9914->dio_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));
	m_tms9914->eoi_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	m_tms9914->dav_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	m_tms9914->nrfd_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	m_tms9914->ndac_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	m_tms9914->ifc_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	m_tms9914->srq_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));
	m_tms9914->atn_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	m_tms9914->ren_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));
	MCFG_IEEE488_BUS_ADD()
	MCFG_IEEE488_EOI_CALLBACK(WRITELINE(m_tms9914, tms9914_device, eoi_w))
	MCFG_IEEE488_DAV_CALLBACK(WRITELINE(m_tms9914, tms9914_device, dav_w))
	MCFG_IEEE488_NRFD_CALLBACK(WRITELINE(m_tms9914, tms9914_device, nrfd_w))
	MCFG_IEEE488_NDAC_CALLBACK(WRITELINE(m_tms9914, tms9914_device, ndac_w))
	MCFG_IEEE488_IFC_CALLBACK(WRITELINE(m_tms9914, tms9914_device, ifc_w))
	MCFG_IEEE488_SRQ_CALLBACK(WRITELINE(m_tms9914, tms9914_device, srq_w))
	MCFG_IEEE488_ATN_CALLBACK(WRITELINE(m_tms9914, tms9914_device, atn_w))
	MCFG_IEEE488_REN_CALLBACK(WRITELINE(m_tms9914, tms9914_device, ren_w))
	MCFG_IEEE488_SLOT_ADD("ieee_dev", 0, cbm_ieee488_devices, nullptr)

	// TODO: LED's for ATN, TALK, and DATA
MACHINE_CONFIG_END

//MACHINE_CONFIG_START(bbc_procyon_device::device_add_mconfig)
	// TODO: Implement MC68488
	//MCFG_IEEE488_BUS_ADD()
	//MCFG_IEEE488_EOI_CALLBACK(WRITELINE(m_mc68488, mc68488_device, eoi_w))
	//MCFG_IEEE488_DAV_CALLBACK(WRITELINE(m_mc68488, mc68488_device, dav_w))
	//MCFG_IEEE488_NRFD_CALLBACK(WRITELINE(m_mc68488, mc68488_device, nrfd_w))
	//MCFG_IEEE488_NDAC_CALLBACK(WRITELINE(m_mc68488, mc68488_device, ndac_w))
	//MCFG_IEEE488_IFC_CALLBACK(WRITELINE(m_mc68488, mc68488_device, ifc_w))
	//MCFG_IEEE488_SRQ_CALLBACK(WRITELINE(m_mc68488, mc68488_device, srq_w))
	//MCFG_IEEE488_ATN_CALLBACK(WRITELINE(m_mc68488, mc68488_device, atn_w))
	//MCFG_IEEE488_REN_CALLBACK(WRITELINE(m_mc68488, mc68488_device, ren_w))
	//MCFG_IEEE488_SLOT_ADD("ieee_dev", 0, cbm_ieee488_devices, nullptr)

	// TODO: LED's for Bus Active, Byte Out, and Byte In
//MACHINE_CONFIG_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_ieee488_device::device_rom_region() const
{
	return ROM_NAME(ieee488);
}

//const tiny_rom_entry *bbc_procyon_device::device_rom_region() const
//{
//  return ROM_NAME(procyon);
//}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_ieee488_device - constructor
//-------------------------------------------------

bbc_ieee488_device::bbc_ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_IEEE488, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_ieee(*this, IEEE488_TAG)
	, m_tms9914(*this, "hpib")
	, m_1mhzbus(*this, "1mhzbus")
{
}

bbc_b488_device::bbc_b488_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_B488, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_ieee(*this, IEEE488_TAG)
	, m_tms9914(*this, "hpib")
{
}

//bbc_procyon_device::bbc_procyon_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
//  : device_t(mconfig, BBC_PROCYON, tag, owner, clock)
//  , device_bbc_1mhzbus_interface(mconfig, *this)
//  , m_ieee(*this, IEEE488_TAG)
//{
//}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_ieee488_device::device_start()
{
}

void bbc_b488_device::device_start()
{
}

//void bbc_procyon_device::device_start()
//{
//}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_ieee488_device::fred_r)
{
	uint8_t data = 0xff;

	if (offset >= 0x20 && offset < 0x28)
	{
		data = m_tms9914->reg8_r(space, offset & 0x07);
	}

	data &= m_1mhzbus->fred_r(space, offset);

	return data;
}

WRITE8_MEMBER(bbc_ieee488_device::fred_w)
{
	if (offset >= 0x20 && offset < 0x28)
	{
		m_tms9914->reg8_w(space, offset & 0x07, data);
	}

	m_1mhzbus->fred_w(space, offset, data);
}

READ8_MEMBER(bbc_ieee488_device::jim_r)
{
	uint8_t data = 0xff;

	data &= m_1mhzbus->jim_r(space, offset);

	return data;
}

WRITE8_MEMBER(bbc_ieee488_device::jim_w)
{
	m_1mhzbus->jim_w(space, offset, data);
}


READ8_MEMBER(bbc_b488_device::fred_r)
{
	uint8_t data = 0xff;

	if (offset >= 0x20 && offset < 0x28)
	{
		data = m_tms9914->reg8_r(space, offset & 0x07);
	}

	return data;
}

WRITE8_MEMBER(bbc_b488_device::fred_w)
{
	if (offset >= 0x20 && offset < 0x28)
	{
		m_tms9914->reg8_w(space, offset & 0x07, data);
	}
}


//READ8_MEMBER(bbc_procyon_device::fred_r)
//{
	//uint8_t data = 0xff;

	//if (offset >= 0x20 && offset < 0x28)
	//{
	//  data = mc68488_device->reg8_r(space, offset & 0x07);
	//}

	//return data;
//}

//WRITE8_MEMBER(bbc_procyon_device::fred_w)
//{
	//if (offset >= 0x20 && offset < 0x28)
	//{
	//  mc68488_device->reg8_w(space, offset & 0x07, data);
	//}
//}
