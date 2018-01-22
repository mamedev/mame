// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    ram.c

    KC 85 RAM modules emulation

    Supported modules:
    - M011 64KB RAM
    - M022 16KB RAM
    - M032 256KB segmented RAM
    - M034 512KB segmented RAM
    - M035 1MB segmented RAM
    - M036 128KB segmented RAM

***************************************************************************/

#include "emu.h"
#include "ram.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(KC_M011, kc_m011_device, "kc_m011", "M011 64KB RAM")
DEFINE_DEVICE_TYPE(KC_M022, kc_m022_device, "kc_m022", "M022 16KB RAM")
DEFINE_DEVICE_TYPE(KC_M032, kc_m032_device, "kc_m032", "M032 256KB segmented RAM")
DEFINE_DEVICE_TYPE(KC_M034, kc_m034_device, "kc_m034", "M034 512KB segmented RAM")
DEFINE_DEVICE_TYPE(KC_M035, kc_m035_device, "kc_m035", "M035 1MB segmented RAM")
DEFINE_DEVICE_TYPE(KC_M036, kc_m036_device, "kc_m036", "M036 128KB segmented RAM")

//**************************************************************************
//  M011 64KB RAM
//**************************************************************************

//-------------------------------------------------
//  kc_m011_device - constructor
//-------------------------------------------------

kc_m011_device::kc_m011_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kc_m011_device(mconfig, KC_M011, tag, owner, clock)
{
}

kc_m011_device::kc_m011_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_kcexp_interface(mconfig, *this)
	, m_slot(nullptr), m_mei(0), m_ram(nullptr), m_enabled(0), m_write_enabled(0), m_base(0), m_segment(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kc_m011_device::device_start()
{
	m_slot = dynamic_cast<kcexp_slot_device *>(owner());

	m_ram = machine().memory().region_alloc(tag(), get_ram_size(), 1, ENDIANNESS_LITTLE)->base();
	memset(m_ram, 0, get_ram_size());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kc_m011_device::device_reset()
{
	m_enabled = 0;
	m_write_enabled = 0;
	m_base = 0;
	m_segment = 0;
	m_mei = 0;
}

/*-------------------------------------------------
    module control write
-------------------------------------------------*/
void kc_m011_device::control_w(uint8_t data)
{
	if (m_mei)
	{
		m_enabled = BIT(data, 0);
		m_write_enabled = BIT(data, 1);
		m_base = (data & 0xc0) << 8;
	}
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void kc_m011_device::read(offs_t offset, uint8_t &data)
{
	uint32_t addr = offset ^ m_base;

	if (addr < 0x10000 && m_enabled && m_mei)
	{
		data = m_ram[addr];

		// clear the MEO line for disable other modules with less priority
		m_slot->meo_w(CLEAR_LINE);
	}
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

void kc_m011_device::write(offs_t offset, uint8_t data)
{
	uint32_t addr = offset ^ m_base;

	if (addr < 0x10000 && m_enabled && m_mei)
	{
		if (m_write_enabled)
			m_ram[addr] = data;

		// clear the MEO line for disable other modules with less priority
		m_slot->meo_w(CLEAR_LINE);
	}
}

/*-------------------------------------------------
   MEI line write
-------------------------------------------------*/

WRITE_LINE_MEMBER( kc_m011_device::mei_w )
{
	m_mei = state;

	// update MEO line
	m_slot->meo_w(state);
}

//**************************************************************************
//  M022 16KB RAM
//**************************************************************************

//-------------------------------------------------
//  kc_m022_device - constructor
//-------------------------------------------------

kc_m022_device::kc_m022_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kc_m011_device(mconfig, KC_M022, tag, owner, clock)
{
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void kc_m022_device::read(offs_t offset, uint8_t &data)
{
	if (offset >= m_base && offset < (m_base + 0x4000) && m_enabled && m_mei)
	{
		data = m_ram[offset - m_base];

		// clear the MEO line for disable other modules with less priority
		m_slot->meo_w(CLEAR_LINE);
	}
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

void kc_m022_device::write(offs_t offset, uint8_t data)
{
	if (offset >= m_base && offset < (m_base + 0x4000) && m_enabled && m_mei)
	{
		if (m_write_enabled)
			m_ram[offset - m_base] = data;

		// clear the MEO line for disable other modules with less priority
		m_slot->meo_w(CLEAR_LINE);
	}
}


//**************************************************************************
//  M032 256KB segmented RAM
//**************************************************************************

//-------------------------------------------------
//  kc_m032_device - constructor
//-------------------------------------------------

kc_m032_device::kc_m032_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kc_m011_device(mconfig, KC_M032, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kc_m032_device::device_reset()
{
	kc_m011_device::device_reset();
	m_base = 0x4000;
}

/*-------------------------------------------------
    module control write
-------------------------------------------------*/
void kc_m032_device::control_w(uint8_t data)
{
	if (m_mei)
	{
		m_enabled = BIT(data, 0);
		m_write_enabled = BIT(data, 1);
		m_segment = (data>>2) & 0x0f;
		m_base = (data & 0x80) ? 0x8000 : 0x4000;
	}
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void kc_m032_device::read(offs_t offset, uint8_t &data)
{
	if (offset >= m_base && offset < (m_base + 0x4000) && m_enabled && m_mei)
	{
		data = m_ram[(offset - m_base) | (m_segment << 14)];

		// clear the MEO line for disable other modules with less priority
		m_slot->meo_w(CLEAR_LINE);
	}
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

void kc_m032_device::write(offs_t offset, uint8_t data)
{
	if (offset >= m_base && offset < (m_base + 0x4000) && m_enabled && m_mei)
	{
		if (m_write_enabled)
			m_ram[(offset - m_base) | (m_segment << 14)] = data;

		// clear the MEO line for disable other modules with less priority
		m_slot->meo_w(CLEAR_LINE);
	}
}


//**************************************************************************
//  M034 512KB segmented RAM
//**************************************************************************

//-------------------------------------------------
//  kc_m034_device - constructor
//-------------------------------------------------

kc_m034_device::kc_m034_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kc_m011_device(mconfig, KC_M034, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kc_m034_device::device_reset()
{
	kc_m011_device::device_reset();
	m_base = 0x4000;
}

/*-------------------------------------------------
    module control write
-------------------------------------------------*/
void kc_m034_device::control_w(uint8_t data)
{
	if (m_mei)
	{
		m_enabled = BIT(data, 0);
		m_write_enabled = BIT(data, 1);
		m_segment = (data>>2) & 0x1f;
		m_base = (data & 0x80) ? 0x8000 : 0x4000;
	}
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void kc_m034_device::read(offs_t offset, uint8_t &data)
{
	if (offset >= m_base && offset < (m_base + 0x4000) && m_enabled && m_mei)
	{
		data = m_ram[(offset - m_base) | (m_segment << 14)];

		// clear the MEO line for disable other modules with less priority
		m_slot->meo_w(CLEAR_LINE);
	}
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

void kc_m034_device::write(offs_t offset, uint8_t data)
{
	if (offset >= m_base && offset < (m_base + 0x4000) && m_enabled && m_mei)
	{
		if (m_write_enabled)
			m_ram[(offset - m_base) | (m_segment << 14)] = data;

		// clear the MEO line for disable other modules with less priority
		m_slot->meo_w(CLEAR_LINE);
	}
}


//**************************************************************************
//  M035 1MB segmented RAM
//**************************************************************************

//-------------------------------------------------
//  kc_m035_device - constructor
//-------------------------------------------------

kc_m035_device::kc_m035_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kc_m011_device(mconfig, KC_M035, tag, owner, clock)
{
}

/*-------------------------------------------------
    module control write
-------------------------------------------------*/
void kc_m035_device::control_w(uint8_t data)
{
	if (m_mei)
	{
		m_enabled = BIT(data, 0);
		m_write_enabled = BIT(data, 1);
		m_segment = (data>>2) & 0x3f;
	}
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void kc_m035_device::read(offs_t offset, uint8_t &data)
{
	if (offset >= 0x8000 && offset < 0xc000 && m_enabled & m_mei)
	{
		data = m_ram[(offset - 0x8000) | (m_segment << 14)];

		// clear the MEO line for disable other modules with less priority
		m_slot->meo_w(CLEAR_LINE);
	}
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

void kc_m035_device::write(offs_t offset, uint8_t data)
{
	if (offset >= 0x8000 && offset < 0xc000 && m_enabled && m_mei)
	{
		if (m_write_enabled)
			m_ram[(offset-0x8000) | (m_segment << 14)] = data;

		// clear the MEO line for disable other modules with less priority
		m_slot->meo_w(CLEAR_LINE);
	}
}


//**************************************************************************
//  M036 128KB segmented RAM
//**************************************************************************

//-------------------------------------------------
//  kc_m036_device - constructor
//-------------------------------------------------

kc_m036_device::kc_m036_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kc_m011_device(mconfig, KC_M036, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kc_m036_device::device_reset()
{
	kc_m011_device::device_reset();
	m_base = 0x4000;
}

/*-------------------------------------------------
    module control write
-------------------------------------------------*/
void kc_m036_device::control_w(uint8_t data)
{
	if (m_mei)
	{
		m_enabled = BIT(data, 0);
		m_write_enabled = BIT(data, 1);
		m_segment = (data>>2) & 0x07;
		m_base = (data & 0x80) ? 0x8000 : 0x4000;
	}
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void kc_m036_device::read(offs_t offset, uint8_t &data)
{
	if (offset >= m_base && offset < (m_base + 0x4000) && m_enabled && m_mei)
	{
		data = m_ram[(offset - m_base) | (m_segment << 14)];

		// clear the MEO line for disable other modules with less priority
		m_slot->meo_w(CLEAR_LINE);
	}
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

void kc_m036_device::write(offs_t offset, uint8_t data)
{
	if (offset >= m_base && offset < (m_base + 0x4000) && m_enabled && m_mei)
	{
		if (m_write_enabled)
			m_ram[(offset - m_base) | (m_segment << 14)] = data;

		// clear the MEO line for disable other modules with less priority
		m_slot->meo_w(CLEAR_LINE);
	}
}
