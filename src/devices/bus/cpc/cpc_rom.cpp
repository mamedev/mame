// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * cpc_rom.cpp
 * Amstrad CPC mountable ROM image device
 *
 */

#include "emu.h"
#include "cpc_rom.h"

#include <tuple>


DEFINE_DEVICE_TYPE(CPC_ROM, cpc_rom_device, "cpc_rom", "CPC ROM Box")

void cpc_exp_cards(device_slot_interface &device);

//**************************************************************************
//  DEVICE CONFIG INTERFACE
//**************************************************************************

// device machine config
void cpc_rom_device::device_add_mconfig(machine_config &config)
{
	CPC_ROMSLOT(config, m_rom[0], 0);
	CPC_ROMSLOT(config, m_rom[1], 0);
	CPC_ROMSLOT(config, m_rom[2], 0);
	CPC_ROMSLOT(config, m_rom[3], 0);
	CPC_ROMSLOT(config, m_rom[4], 0);
	CPC_ROMSLOT(config, m_rom[5], 0);
	CPC_ROMSLOT(config, m_rom[6], 0);
	CPC_ROMSLOT(config, m_rom[7], 0);

	// pass-through
	cpc_expansion_slot_device &exp(CPC_EXPANSION_SLOT(config, "exp", DERIVED_CLOCK(1, 1), cpc_exp_cards, nullptr));
	exp.irq_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::irq_w));
	exp.nmi_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::nmi_w));
	exp.romdis_callback().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::romdis_w));  // ROMDIS
}


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
	, device_rom_image_interface(mconfig, *this)
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
std::pair<std::error_condition, std::string> cpc_rom_image_device::call_load()
{
	uint64_t const total = length();
	size_t const size = std::min<uint64_t>(total, 16384);

	std::error_condition err;
	size_t actual;
	std::tie(err, m_base, actual) = read_at(image_core_file(), total - size, size);
	if (!err && (actual != size))
		err = std::errc::io_error;

	return std::make_pair(err, std::string());
}


/*-------------------------------------------------
    DEVICE_IMAGE_UNLOAD( rom )
-------------------------------------------------*/
void cpc_rom_image_device::call_unload()
{
	m_base = nullptr;
}
