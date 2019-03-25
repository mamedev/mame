// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * cpc_rom.cpp
 * Amstrad CPC mountable ROM image device
 *
 */

#include "emu.h"
#include "cpc_rom.h"

DEFINE_DEVICE_TYPE(CPC_ROM, cpc_rom_device, "cpc_rom", "CPC ROM Box")

void cpc_exp_cards(device_slot_interface &device);

//**************************************************************************
//  DEVICE CONFIG INTERFACE
//**************************************************************************

// device machine config
MACHINE_CONFIG_START(cpc_rom_device::device_add_mconfig)
	MCFG_CPC_ROMSLOT_ADD("rom1")
	MCFG_CPC_ROMSLOT_ADD("rom2")
	MCFG_CPC_ROMSLOT_ADD("rom3")
	MCFG_CPC_ROMSLOT_ADD("rom4")
	MCFG_CPC_ROMSLOT_ADD("rom5")
	MCFG_CPC_ROMSLOT_ADD("rom6")
	MCFG_CPC_ROMSLOT_ADD("rom7")
	MCFG_CPC_ROMSLOT_ADD("rom8")

	// pass-through
	cpc_expansion_slot_device &exp(CPC_EXPANSION_SLOT(config, "exp", DERIVED_CLOCK(1, 1), cpc_exp_cards, nullptr));
	exp.irq_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::irq_w));
	exp.nmi_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::nmi_w));
	exp.romdis_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::romdis_w));  // ROMDIS
MACHINE_CONFIG_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_rom_device::cpc_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CPC_ROM, tag, owner, clock),
	device_cpc_expansion_card_interface(mconfig, *this),
	m_rom(*this, "rom%u", 1)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_rom_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cpc_rom_device::device_reset()
{
}


/*** ROM image device ***/

// device type definition
DEFINE_DEVICE_TYPE(CPC_ROMSLOT, cpc_rom_image_device, "cpc_rom_image", "CPC ROM image")

//-------------------------------------------------
//  cpc_rom_image_device - constructor
//-------------------------------------------------

cpc_rom_image_device::cpc_rom_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CPC_ROMSLOT, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, m_base(nullptr)
{
}

//-------------------------------------------------
//  cpc_rom_image_device - destructor
//-------------------------------------------------

cpc_rom_image_device::~cpc_rom_image_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_rom_image_device::device_start()
{
	m_base = nullptr;
}

/*-------------------------------------------------
    DEVICE_IMAGE_LOAD( rom )
-------------------------------------------------*/
image_init_result cpc_rom_image_device::call_load()
{
	device_image_interface* image = this;
	uint64_t size = image->length();

	m_base = std::make_unique<uint8_t[]>(16384);
	if(size <= 16384)
	{
		image->fread(m_base.get(),size);
	}
	else
	{
		image->fseek(size-16384,SEEK_SET);
		image->fread(m_base.get(),16384);
	}

	return image_init_result::PASS;
}


/*-------------------------------------------------
    DEVICE_IMAGE_UNLOAD( rom )
-------------------------------------------------*/
void cpc_rom_image_device::call_unload()
{
	m_base = nullptr;
}
