// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Generic Palette RAMDAC device

    Written by Angelo Salese

    TODO:
    - masking register, almost likely it controls rollback on incrementing
      r/w palette access;
    - needs information about different models and what exactly they does

***************************************************************************/

#include "emu.h"
#include "video/ramdac.h"

// default address map
void ramdac_device::ramdac_palram(address_map &map)
{
	if (!has_configured_map(0))
	{
		map(0x000, 0x0ff).ram(); // R bank
		map(0x100, 0x1ff).ram(); // G bank
		map(0x200, 0x2ff).ram(); // B bank
		map(0x300, 0x3ff).noprw();
	}
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(RAMDAC, ramdac_device, "ramdac", "RAMDAC")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ramdac_device - constructor
//-------------------------------------------------

ramdac_device::ramdac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, RAMDAC, tag, owner, clock),
		device_memory_interface(mconfig, *this),
		m_space_config("videoram", ENDIANNESS_LITTLE, 8, 10, 0, address_map_constructor(FUNC(ramdac_device::ramdac_palram), this)),
		m_palette(*this, finder_base::DUMMY_TAG),
		m_color_base(0),
		m_split_read_reg(0)
{
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector ramdac_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

inline uint8_t ramdac_device::readbyte(offs_t address)
{
	return space().read_byte(address);
}


//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void ramdac_device::writebyte(offs_t address, uint8_t data)
{
	space().write_byte(address, data);
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void ramdac_device::device_validity_check(validity_checker &valid) const
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ramdac_device::device_start()
{
	m_palram = make_unique_clear<uint8_t[]>(1 << 10);

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ramdac_device::device_reset()
{
	m_pal_index[0] = 0;
	m_int_index[0] = 0;
	m_pal_index[1] = 0;
	m_int_index[1] = 0;
	m_pal_mask = 0xff;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//  [0] = W register, [1] = R register
//**************************************************************************

inline void ramdac_device::reg_increment(uint8_t inc_type)
{
	m_int_index[inc_type]++;
	if(m_int_index[inc_type] == 3)
	{
		m_int_index[inc_type] = 0;
		m_pal_index[inc_type]++;
	}
}

READ8_MEMBER( ramdac_device::index_r )
{
	return m_pal_index[0];
}

WRITE8_MEMBER( ramdac_device::index_w )
{
	m_pal_index[0] = data;
	m_int_index[0] = 0;
}

WRITE8_MEMBER( ramdac_device::index_r_w )
{
	m_pal_index[1] = data;
	m_int_index[1] = 0;
}

READ8_MEMBER( ramdac_device::pal_r )
{
	uint8_t res;
	res = readbyte(m_pal_index[m_split_read_reg] | (m_int_index[m_split_read_reg] << 8));
	reg_increment(m_split_read_reg);
	return res;
}

WRITE8_MEMBER( ramdac_device::pal_w )
{
	writebyte(m_pal_index[0] | (m_int_index[0] << 8),data);
	reg_increment(0);
}

READ8_MEMBER( ramdac_device::mask_r )
{
	return m_pal_mask;
}

WRITE8_MEMBER( ramdac_device::mask_w )
{
	m_pal_mask = data;
}


//**************************************************************************
//  Generic bank read/write handlers
//**************************************************************************

READ8_MEMBER( ramdac_device::ramdac_pal_r )
{
	return m_palram[offset];
}

WRITE8_MEMBER( ramdac_device::ramdac_rgb666_w )
{
	uint16_t pal_offs;

	m_palram[offset] = data & 0x3f;
	pal_offs = (offset & 0xff);

	pen_t pen = m_color_base + (offset & 0xff);
	m_palette->set_pen_color(pen, pal6bit(m_palram[pal_offs|0x000]), pal6bit(m_palram[pal_offs|0x100]), pal6bit(m_palram[pal_offs|0x200]));
}

WRITE8_MEMBER( ramdac_device::ramdac_rgb888_w )
{
	uint16_t pal_offs;

	m_palram[offset] = data;
	pal_offs = (offset & 0xff);

	pen_t pen = m_color_base + (offset & 0xff);
	m_palette->set_pen_color(pen, m_palram[pal_offs|0x000], m_palram[pal_offs|0x100], m_palram[pal_offs|0x200]);
}
