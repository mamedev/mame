// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * ddi1.cpp  --  Amstrad DDI-1 Floppy Disk Drive interface
 */

#include "emu.h"
#include "ddi1.h"
#include "softlist_dev.h"

void cpc_exp_cards(device_slot_interface &device);

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CPC_DDI1, cpc_ddi1_device, "cpc_ddi1", "Amstrad DDI-1")

static void ddi1_floppies(device_slot_interface &device)
{
	device.option_add("3ssdd", FLOPPY_3_SSDD);
}

//-------------------------------------------------
//  Device ROM definition
//-------------------------------------------------

ROM_START( cpc_ddi1 )
	ROM_REGION( 0x4000, "disc_rom", 0 )
	ROM_LOAD("cpcados.rom",  0x0000, 0x4000, CRC(1fe22ecd) SHA1(39102c8e9cb55fcc0b9b62098780ed4a3cb6a4bb))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *cpc_ddi1_device::device_rom_region() const
{
	return ROM_NAME( cpc_ddi1 );
}

// device machine config
void cpc_ddi1_device::device_add_mconfig(machine_config &config)
{
	UPD765A(config, m_fdc, DERIVED_CLOCK(1, 1), true, true); // pin 50 clock multiplied to 8 MHz, then divided back down through SMC FDC9229BT
	FLOPPY_CONNECTOR(config, m_connector, ddi1_floppies, "3ssdd", floppy_image_device::default_mfm_floppy_formats);
	SOFTWARE_LIST(config, "flop_list").set_original("cpc_flop");

	// pass-through
	cpc_expansion_slot_device &exp(CPC_EXPANSION_SLOT(config, "exp", DERIVED_CLOCK(1, 1), cpc_exp_cards, nullptr));
	exp.irq_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::irq_w));
	exp.nmi_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::nmi_w));
	exp.romdis_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::romdis_w));  // ROMDIS
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_ddi1_device::cpc_ddi1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CPC_DDI1, tag, owner, clock),
	device_cpc_expansion_card_interface(mconfig, *this), m_slot(nullptr),
	m_fdc(*this,"upd765"),
	m_connector(*this,"upd765:0"), m_rom_active(false), m_romen(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_ddi1_device::device_start()
{
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());
	address_space &space = m_slot->cpu().space(AS_IO);

	space.install_write_handler(0xfa7e,0xfa7f, write8sm_delegate(*this, FUNC(cpc_ddi1_device::motor_w)));
	space.install_readwrite_handler(0xfb7e,0xfb7f, read8sm_delegate(*this, FUNC(cpc_ddi1_device::fdc_r)), write8sm_delegate(*this, FUNC(cpc_ddi1_device::fdc_w)));
	space.install_write_handler(0xdf00,0xdfff, write8smo_delegate(*this, FUNC(cpc_ddi1_device::rombank_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cpc_ddi1_device::device_reset()
{
	m_rom_active = false;
}

void cpc_ddi1_device::motor_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 0x00:
	case 0x01:
		{
			/* FDC Motor Control - Bit 0 defines the state of the FDD motor:
			 * "1" the FDD motor will be active.
			 * "0" the FDD motor will be in-active.*/
			floppy_image_device *floppy;
			if(m_connector)
			{
				floppy = m_connector->get_device();
				if(floppy)
					floppy->mon_w(!BIT(data, 0));
			}
			break;
		}
	}
}

void cpc_ddi1_device::fdc_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 0x01:
		m_fdc->fifo_w(data);
		break;
	}
}

uint8_t cpc_ddi1_device::fdc_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch(offset)
	{
	case 0x00:
		data = m_fdc->msr_r();
		break;
	case 0x01:
		data = m_fdc->fifo_r();
		break;
	}
	return data;
}

void cpc_ddi1_device::rombank_w(uint8_t data)
{
	if(data == 0x07)
		m_rom_active = true;
	else
		m_rom_active = false;
	m_slot->rom_select(data);
}

void cpc_ddi1_device::set_mapping(uint8_t type)
{
	if(type != MAP_UPPER)
		return;
	if(m_rom_active)
	{
		uint8_t* ROM = memregion("disc_rom")->base();
		membank(":bank7")->set_base(ROM);
		membank(":bank8")->set_base(ROM+0x2000);
	}
}
