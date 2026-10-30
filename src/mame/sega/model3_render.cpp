// license:BSD-3-Clause
// copyright-holders:Ville Linde, R. Belmont
/*
    Real3D renderer
    Emulation by Ville Linde, repackaged for PCI by R. Belmont
 */

#include "emu.h"
#include "model3_render.h"

DEFINE_DEVICE_TYPE(MODEL3_REAL3D_RENDERER, model3_real3d_renderer_device, "model3_real3d_renderer", "Model 3 Real3D Renderer Adapter")

namespace {

constexpr u8 DECODE16[64] = {
	 0,  1,  4,  5,  8,  9, 12, 13,
	 2,  3,  6,  7, 10, 11, 14, 15,
	16, 17, 20, 21, 24, 25, 28, 29,
	18, 19, 22, 23, 26, 27, 30, 31,
	32, 33, 36, 37, 40, 41, 44, 45,
	34, 35, 38, 39, 42, 43, 46, 47,
	48, 49, 52, 53, 56, 57, 60, 61,
	50, 51, 54, 55, 58, 59, 62, 63
};

constexpr u8 DECODE8[32] = {
	 1,  3,  5,  7,
	 0,  2,  4,  6,
	 9, 11, 13, 15,
	 8, 10, 12, 14,
	17, 19, 21, 23,
	16, 18, 20, 22,
	25, 27, 29, 31,
	24, 26, 28, 30
};

constexpr u16 MIP_POSITION[9][2] = {
	{ 1024,  512 }, { 1536,  768 }, { 1792,  896 },
	{ 1920,  960 }, { 1984,  992 }, { 2016, 1008 },
	{ 2032, 1016 }, { 2040, 1020 }, { 2044, 1022 }
};

} // anonymous namespace

model3_real3d_renderer_device::model3_real3d_renderer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MODEL3_REAL3D_RENDERER, tag, owner, clock)
	, m_graphics(*this, finder_base::DUMMY_TAG)
	, m_texture_ram(*this, "texture_ram%u", 0U, WIDTH * HEIGHT * 2, ENDIANNESS_LITTLE)
	, m_sync_callback(*this)
	, m_invalidate_callback(*this)
	, m_submit_callback(*this)
{
}

void model3_real3d_renderer_device::set_graphics(model3_real3d_pci_device &graphics)
{
	m_graphics.set_tag(graphics);
	graphics.set_texture_callback(*this, FUNC(model3_real3d_renderer_device::texture));
	graphics.set_vrom_callback(*this, FUNC(model3_real3d_renderer_device::vrom));
	graphics.set_submit_callback(*this, FUNC(model3_real3d_renderer_device::submit));
}

void model3_real3d_renderer_device::device_start()
{
	m_sync_callback.resolve_safe();
	m_invalidate_callback.resolve_safe();
	m_submit_callback.resolve_safe();
}

void model3_real3d_renderer_device::device_reset()
{
	invalidate_all();
}

void model3_real3d_renderer_device::device_pre_save()
{
	synchronize();
}

void model3_real3d_renderer_device::device_post_load()
{
	invalidate_all();
}

std::span<const u16> model3_real3d_renderer_device::texture_ram(u32 page) const
{
	assert(page < 2);
	return { m_texture_ram[page].target(), WIDTH * HEIGHT };
}

void model3_real3d_renderer_device::synchronize()
{
	m_sync_callback();
}

void model3_real3d_renderer_device::invalidate_all()
{
	synchronize();
	m_invalidate_callback(0);
	m_invalidate_callback(1);
}

void model3_real3d_renderer_device::texture(u32 header, u32 length, std::span<const u32> source)
{
	if (!upload(header, source))
	{
		logerror("Rejected Real3D texture upload: header %08x length %08x, %u available words\n", header, length, source.size());
	}
}

void model3_real3d_renderer_device::vrom(u32 address, u32 header, u32 third, std::span<const u32> source)
{
	// the third word's meaning is unknown
	if (!upload(header, source))
	{
		logerror("Rejected Real3D VROM upload: address %08x header %08x third %08x, %u available words\n",
			address, header, third, source.size());
	}
}

bool model3_real3d_renderer_device::upload(u32 header, std::span<const u32> source)
{
	const u32 type = header >> 24;
	if (type == 0x80)
	{
		return true; // gamma-table records are not decoded by the current renderer
	}
	if (type > 2)
	{
		return false;
	}
	const bool sixteen = BIT(header, 23);
	if (!sixteen && !(header & 0x00600000))
	{
		return true; // neither byte lane selected
	}

	const u32 xpos = (header & 0x3f) * 32;
	const u32 ypos = ((header >> 7) & 0x1f) * 32;
	u32 width = 32U << ((header >> 14) & 7);
	u32 height = 32U << ((header >> 17) & 7);
	u32 mip = 0;
	if (type == 2)
	{
		width /= 2;
		height /= 2;
		mip = 1;
	}

	std::array<level, 10> levels;
	u32 count = 0;
	std::size_t words = 0;
	while (width >= 8 && height >= 8)
	{
		const u32 x = mip ? MIP_POSITION[mip - 1][0] + (xpos >> mip) : xpos;
		const u32 y = mip ? MIP_POSITION[mip - 1][1] + (ypos >> mip) : ypos;
		if (width > WIDTH || height > HEIGHT || x > WIDTH - width || y > HEIGHT - height)
		{
			return false;
		}
		levels[count++] = { x, y, width, height, words };
		words += width * height / (sixteen ? 2 : 4);
		if (type == 1)
		{
			break;
		}
		width /= 2;
		height /= 2;
		++mip;
	}
	if (words > source.size())
	{
		return false;
	}

	synchronize();
	for (u32 i = 0; i != count; ++i)
	{
		write_level(levels[i], header, source.subspan(levels[i].offset, levels[i].width * levels[i].height / (sixteen ? 2 : 4)));
	}
	m_invalidate_callback(BIT(header, 20));
	return true;
}

void model3_real3d_renderer_device::write_level(const level &mip, u32 header, std::span<const u32> source)
{
	const bool sixteen = BIT(header, 23);
	const u16 mask = (BIT(header, 22) ? 0xff00 : 0) | (BIT(header, 21) ? 0x00ff : 0);
	u16 *const sheet = m_texture_ram[BIT(header, 20)].target();
	std::size_t tile = 0;
	for (u32 y = 0; y != mip.height; y += 8)
	{
		for (u32 x = 0; x != mip.width; x += 8)
		{
			for (u32 row = 0; row != 8; ++row)
			{
				for (u32 col = 0; col != 8; ++col)
				{
					const u32 index = sixteen ? DECODE16[(row * 8 + col) ^ 1] : DECODE8[row * 4 + col / 2];
					const u16 half = source[tile + index / 2] >> ((index & 1) * 16);
					u16 &texel = sheet[(mip.y + y + row) * WIDTH + mip.x + x + col];
					if (sixteen)
					{
						texel = half;
					}
					else
					{
						const u8 value = half >> ((col & 1) ? 0 : 8);
						texel = (texel & ~mask) | ((u16(value) * 0x101) & mask);
					}
				}
			}
			tile += sixteen ? 32 : 16;
		}
	}
}

void model3_real3d_renderer_device::submit()
{
	synchronize();
	const memory_view memory = {
		m_graphics->display_list_ram(), m_graphics->culling_ram(), m_graphics->polygon_ram(), m_graphics->vrom(),
		m_graphics->buffer_config(), { texture_ram(0), texture_ram(1) }
	};
	m_submit_callback(memory);
}
