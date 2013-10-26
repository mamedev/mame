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

const device_type KC_STANDARD = &device_creator<kc_8k_device>;
const device_type KC_M006     = &device_creator<kc_m006_device>;
const device_type KC_M033     = &device_creator<kc_m033_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  kc_8k_device - constructor
//-------------------------------------------------

kc_8k_device::kc_8k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, KC_STANDARD, "Standard", tag, owner, clock, "kc_8k", __FILE__),
		device_kcexp_interface( mconfig, *this )
{
}

kc_8k_device::kc_8k_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
		: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_kcexp_interface( mconfig, *this )
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

const rom_entry *kc_8k_device::device_rom_region() const
{
	return ROM_NAME( kc_rom );
}

/*-------------------------------------------------
    get_cart_base
-------------------------------------------------*/

UINT8* kc_8k_device::get_cart_base()
{
	return m_rom;
}

/*-------------------------------------------------
    module control write
-------------------------------------------------*/

void kc_8k_device::control_w(UINT8 data)
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

void kc_8k_device::read(offs_t offset, UINT8 &data)
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

kc_m006_device::kc_m006_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: kc_8k_device(mconfig, KC_M006, "M006 BASIC", tag, owner, clock, "kc_m006", __FILE__)
{
}

/*-------------------------------------------------
    module control write
-------------------------------------------------*/

void kc_m006_device::control_w(UINT8 data)
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

void kc_m006_device::read(offs_t offset, UINT8 &data)
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

kc_m033_device::kc_m033_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: kc_8k_device(mconfig, KC_M033, "M033 TypeStar", tag, owner, clock, "kc_m033", __FILE__)
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

void kc_m033_device::control_w(UINT8 data)
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

void kc_m033_device::read(offs_t offset, UINT8 &data)
{
	if (offset >= m_base && offset < (m_base + 0x2000) && m_enabled && m_mei)
	{
		data = m_rom[(offset - m_base) | (m_bank<<13)];

		// clear the MEO line for disable other modules with less priority
		m_slot->meo_w(CLEAR_LINE);
	}
}
