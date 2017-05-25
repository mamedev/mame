// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    rom.c

    KC 85 ROM modules emulation

    Supported modules:
    - Generic 8KB ROM module
    - M006 BASIC
    - M033 TypeStar

***************************************************************************/

#include "emu.h"
#include "rom.h"

#define REGION_TAG          "rom"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

ROM_START( kc_rom )
	ROM_REGION(0x4000, REGION_TAG, ROMREGION_ERASEFF)
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(KC_STANDARD, kc_8k_device,   "kc_8k",   "Standard 8K ROM module")
DEFINE_DEVICE_TYPE(KC_M006,     kc_m006_device, "kc_m006", "M006 BASIC")
DEFINE_DEVICE_TYPE(KC_M033,     kc_m033_device, "kc_m033", "M033 TypeStar")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  kc_8k_device - constructor
//-------------------------------------------------

kc_8k_device::kc_8k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kc_8k_device(mconfig, KC_STANDARD, tag, owner, clock)
{
}

kc_8k_device::kc_8k_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_kcexp_interface(mconfig, *this)
	, m_slot(nullptr), m_mei(0), m_rom(nullptr), m_enabled(0), m_base(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kc_8k_device::device_start()
{
	m_slot = dynamic_cast<kcexp_slot_device *>(owner());
	m_rom = memregion(REGION_TAG)->base();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kc_8k_device::device_reset()
{
	m_enabled = 0;
	m_base = 0;
	m_mei = 0;
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *kc_8k_device::device_rom_region() const
{
	return ROM_NAME( kc_rom );
}

/*-------------------------------------------------
    get_cart_base
-------------------------------------------------*/

uint8_t* kc_8k_device::get_cart_base()
{
	return m_rom;
}

/*-------------------------------------------------
    module control write
-------------------------------------------------*/

void kc_8k_device::control_w(uint8_t data)
{
	if (m_mei)
	{
		m_enabled = BIT(data, 0);
		m_base = (data & 0xe0) << 8;
	}
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void kc_8k_device::read(offs_t offset, uint8_t &data)
{
	if (offset >= m_base && offset < (m_base + 0x2000) && m_enabled && m_mei)
	{
		data = m_rom[offset - m_base];

		// clear the MEO line for disable other modules with less priority
		m_slot->meo_w(CLEAR_LINE);
	}
}

/*-------------------------------------------------
   MEI line write
-------------------------------------------------*/

WRITE_LINE_MEMBER( kc_8k_device::mei_w )
{
	m_mei = state;

	// update MEO line
	m_slot->meo_w(state);
}

//**************************************************************************
//  M006 BASIC
//**************************************************************************

//-------------------------------------------------
//  kc_m006_device - constructor
//-------------------------------------------------

kc_m006_device::kc_m006_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kc_8k_device(mconfig, KC_M006, tag, owner, clock)
{
}

/*-------------------------------------------------
    module control write
-------------------------------------------------*/

void kc_m006_device::control_w(uint8_t data)
{
	if (m_mei)
	{
		m_enabled = BIT(data, 0);
		m_base = (data & 0xc0) << 8;
	}
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void kc_m006_device::read(offs_t offset, uint8_t &data)
{
	if (offset >= m_base && offset < (m_base + 0x4000) && m_enabled)
	{
		data = m_rom[offset - m_base];

		// clear the MEO line for disable other modules with less priority
		m_slot->meo_w(CLEAR_LINE);
	}
}


//**************************************************************************
//  M033 TypeStar module
//**************************************************************************

//-------------------------------------------------
//  kc_m033_device - constructor
//-------------------------------------------------

kc_m033_device::kc_m033_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kc_8k_device(mconfig, KC_M033, tag, owner, clock)
	, m_bank(0)
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kc_m033_device::device_reset()
{
	kc_8k_device::device_reset();

	m_bank = 0;
}

/*-------------------------------------------------
    module control write
-------------------------------------------------*/

void kc_m033_device::control_w(uint8_t data)
{
	if (m_mei)
	{
		m_enabled = BIT(data, 0);
		m_base = (data & 0xc0) << 8;
		m_bank = BIT(data, 4);
	}
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void kc_m033_device::read(offs_t offset, uint8_t &data)
{
	if (offset >= m_base && offset < (m_base + 0x2000) && m_enabled && m_mei)
	{
		data = m_rom[(offset - m_base) | (m_bank<<13)];

		// clear the MEO line for disable other modules with less priority
		m_slot->meo_w(CLEAR_LINE);
	}
}
