// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "emu.h"
#include "k001006.h"

/*****************************************************************************/
/* Konami K001006 Texel Unit (KS10081) */

/***************************************************************************/
/*                                                                         */
/*                                  001006                                 */
/*                                                                         */
/***************************************************************************/

DEFINE_DEVICE_TYPE(K001006, k001006_device, "k001006", "K001006 Texel Unit")

k001006_device::k001006_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K001006, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, m_pal_ram(nullptr)
	, m_unknown_ram(nullptr)
	, m_addr(0)
	, m_device_sel(0)
	, m_gfxrom(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k001006_device::device_start()
{
	if ((m_gfxrom.length() < 0x40000))
		fatalerror("K001006 %s: texture memory size is less than 0x40000 bytes.", tag());

	if ((m_gfxrom.length() & 0x3ffff))
		fatalerror("K001006 %s: texture memory size is must be a multiple of 0x40000 bytes.", tag());

	m_pal_ram = make_unique_clear<uint16_t []>(0x800);
	m_unknown_ram = make_unique_clear<uint16_t []>(0x1000);

	m_texrom = std::make_unique<uint8_t[]>(m_gfxrom.length());

	preprocess_texture_data(m_texrom.get(), m_gfxrom, m_gfxrom.length());

	save_pointer(NAME(m_pal_ram), 0x800);
	save_pointer(NAME(m_unknown_ram), 0x1000);
	save_item(NAME(m_device_sel));
	save_item(NAME(m_addr));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k001006_device::device_reset()
{
	m_addr = 0;
	m_device_sel = 0;
	for (int i = 0; i < entries(); i++)
		set_pen_color(i, 0);
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

uint32_t k001006_device::read(offs_t offset)
{
	if (offset == 1)
	{
		switch (m_device_sel)
		{
			case 0x0b:      // CG Board ROM read
			{
				uint16_t const *const rom = (uint16_t*)&m_gfxrom[0];
				return rom[m_addr / 2] << 16;
			}
			case 0x0d:      // Palette RAM read
			{
				uint32_t const addr = m_addr;
				if (!machine().side_effects_disabled())
					m_addr += 2;
				return m_pal_ram[addr >> 1];
			}
			case 0x0f:      // Unknown RAM read
			{
				uint32_t const addr = m_addr;
				if (!machine().side_effects_disabled())
					m_addr++;
				return m_unknown_ram[addr];
			}
			default:
			{
				fatalerror("%s:k001006_r, unknown device %02X\n", tag(), m_device_sel);
			}
		}
	}
	return 0;
}

void k001006_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == 0)
	{
		COMBINE_DATA(&m_addr);
	}
	else if (offset == 1)
	{
		switch (m_device_sel)
		{
			case 0xd:   // Palette RAM write
			{
				uint32_t const index = m_addr;

				m_pal_ram[index >> 1] = data & 0xffff;

				int const a = BIT(data, 15) ? 0x00 : 0xff;
				int b = ((data >> 10) & 0x1f) << 3;
				int g = ((data >>  5) & 0x1f) << 3;
				int r = ((data >>  0) & 0x1f) << 3;
				b |= (b >> 5);
				g |= (g >> 5);
				r |= (r >> 5);
				set_pen_color(index >> 1, rgb_t(a, r, g, b));

				m_addr += 2;
				break;
			}
			case 0xf:   // Unknown RAM write
			{
				//osd_printf_debug("Unknown RAM %08X = %04X\n", m_addr, data & 0xffff);
				m_unknown_ram[m_addr++] = data & 0xffff;
				break;
			}
			default:
			{
				osd_printf_debug("k001006_w: device %02X, write %04X to %08X\n", m_device_sel, data & 0xffff, m_addr++);
			}
		}
	}
	else if (offset == 2)
	{
		if (ACCESSING_BITS_16_31)
		{
			m_device_sel = (data >> 16) & 0xf;
			m_enable_bilinear = BIT(data, 20);
		}
	}
}

uint32_t k001006_device::fetch_texel(int page, int pal_index, int u, int v)
{
	uint8_t const *const tex = m_texrom.get() + page;
	int const texel = tex[((v & 0x1ff) << 9) | (u & 0x1ff)];
	return (texel == 0) ? 0 : (uint32_t)pen_color(pal_index + texel);
}


void k001006_device::preprocess_texture_data(uint8_t *dst, uint8_t *src, int length)
{
	static const int decode_x[8] = {  0, 16, 2, 18, 4, 20, 6, 22 };
	static const int decode_y[16] = {  0, 8, 32, 40, 1, 9, 33, 41, 64, 72, 96, 104, 65, 73, 97, 105 };

	uint8_t temp[0x40000];

	for (int index = 0; index < length; index += 0x40000)
	{
		int offset = index;

		memset(temp, 0, 0x40000);

		for (int i = 0; i < 0x800; i++)
		{
			int tx = bitswap<6>(i, 10, 8, 6, 4, 2, 0);
			int ty = bitswap<5>(i, 9, 7, 5, 3, 1);

			tx <<= 3;
			ty <<= 4;

			for (int y = 0; y < 16; y++)
			{
				for (int x = 0; x < 8; x++)
				{
					uint8_t const pixel = src[offset + decode_y[y] + decode_x[x]];

					temp[((ty + y) << 9) + (tx + x)] = pixel;
				}
			}
			offset += 128;
		}
		memcpy(&dst[index], temp, 0x40000);
	}
}
