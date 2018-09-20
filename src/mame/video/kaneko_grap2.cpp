// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Kaneko GRAP2, RLE blitter / Framebuffer etc.? */


// todo: we're still far too heavily tied to galspanic3, which does the rendering by pulling a bunch
//       of our internals
//       lots of unknowns, both here and in rendering / mixing 3 chips in gp3





#include "emu.h"
#include "kaneko_grap2.h"

void kaneko_grap2_device::grap2_map(address_map &map)
{
	map(0x000000, 0x0003ff).rw(FUNC(kaneko_grap2_device::unk1_r), FUNC(kaneko_grap2_device::unk1_w));
	map(0x000400, 0x000401).w(FUNC(kaneko_grap2_device::framebuffer1_scrollx_w));
	map(0x000800, 0x000bff).rw(FUNC(kaneko_grap2_device::unk2_r), FUNC(kaneko_grap2_device::unk2_w));
	map(0x000c00, 0x000c01).w(FUNC(kaneko_grap2_device::framebuffer1_scrolly_w));
	map(0x000c02, 0x000c03).w(FUNC(kaneko_grap2_device::framebuffer1_enable_w));
	map(0x000c06, 0x000c07).w(FUNC(kaneko_grap2_device::framebuffer1_bgcol_w));
	map(0x000c10, 0x000c11).rw(FUNC(kaneko_grap2_device::framebuffer1_fbbright1_r), FUNC(kaneko_grap2_device::framebuffer1_fbbright1_w));
	map(0x000c12, 0x000c13).rw(FUNC(kaneko_grap2_device::framebuffer1_fbbright2_r), FUNC(kaneko_grap2_device::framebuffer1_fbbright2_w));
	map(0x000c18, 0x000c1b).w(FUNC(kaneko_grap2_device::regs1_address_w));
	map(0x000c1c, 0x000c1d).w(FUNC(kaneko_grap2_device::regs2_w));
	map(0x000c1e, 0x000c1f).w(FUNC(kaneko_grap2_device::regs1_go_w));
	map(0x000c00, 0x000c1f).r(FUNC(kaneko_grap2_device::regs1_r));
	map(0x080000, 0x0801ff).rw(FUNC(kaneko_grap2_device::pal_r), FUNC(kaneko_grap2_device::framebuffer1_palette_w));
	map(0x100000, 0x17ffff).rw(FUNC(kaneko_grap2_device::framebuffer_r), FUNC(kaneko_grap2_device::framebuffer_w));
}

DEFINE_DEVICE_TYPE(KANEKO_GRAP2, kaneko_grap2_device, "kaneko_grap2", "Kaneko GRAP2")

kaneko_grap2_device::kaneko_grap2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KANEKO_GRAP2, tag, owner, clock)
	, device_rom_interface(mconfig, *this, 32) // TODO : Unknown Address Bits
	, device_palette_interface(mconfig, *this)
{
}

void kaneko_grap2_device::device_start()
{
	m_framebuffer = make_unique_clear<uint16_t[]>(0x80000/2);
	m_framebuffer_palette = make_unique_clear<uint16_t[]>(PALETTE_SIZE);
	m_framebuffer_unk1 = make_unique_clear<uint16_t[]>(0x400/2);
	m_framebuffer_unk2 = make_unique_clear<uint16_t[]>(0x400/2);

	save_pointer(NAME(m_framebuffer), 0x80000/2);
	save_pointer(NAME(m_framebuffer_palette), PALETTE_SIZE);
	save_pointer(NAME(m_framebuffer_unk1), 0x400/2);
	save_pointer(NAME(m_framebuffer_unk2), 0x400/2);

	save_item(NAME(m_framebuffer_scrolly));
	save_item(NAME(m_framebuffer_scrollx));
	save_item(NAME(m_framebuffer_enable));
	save_item(NAME(m_regs1_i));
	save_item(NAME(m_regs2));
	save_item(NAME(m_framebuffer_bright1));
	save_item(NAME(m_framebuffer_bright2));
	save_item(NAME(m_regs1_address_regs));
}

void kaneko_grap2_device::device_reset()
{
	m_framebuffer_scrolly = 0;
	m_framebuffer_scrollx = 0;
	m_framebuffer_enable = 0;
	m_regs1_i = 0x0;

	m_framebuffer_bright1 = 0;
	m_framebuffer_bright2 = 0;
}

void kaneko_grap2_device::rom_bank_updated()
{
}

READ16_MEMBER(kaneko_grap2_device::regs1_r)
{
	switch (offset)
	{
		case 0x2:
			return m_framebuffer_enable;

		case 0xb:
		{
			m_regs1_i^=1;
			if (m_regs1_i) return 0xfffe;
			else return 0xffff;
		}

		default:
			logerror("%s: regs1_r %02x %04x\n", machine().describe_context(), offset, mem_mask);
			break;

	}

	return 0x0000;
}



void kaneko_grap2_device::do_rle(uint32_t address)
{
	int rle_count = 0;
	int normal_count = 0;
	uint32_t dstaddress = 0;

	uint8_t thebyte;

	while (dstaddress<0x40000)
	{
		if (rle_count==0 && normal_count==0) // we need a new code byte
		{
			thebyte = read_byte(address);

			if ((thebyte & 0x80)) // stream of normal bytes follows
			{
				normal_count = (thebyte & 0x7f)+1;
				address++;
			}
			else // rle block
			{
				rle_count = (thebyte & 0x7f)+1;
				address++;
			}
		}
		else if (rle_count)
		{
			thebyte = read_byte(address);
			m_framebuffer[dstaddress] = thebyte;
			dstaddress++;
			rle_count--;

			if (rle_count==0)
			{
				address++;
			}
		}
		else if (normal_count)
		{
			thebyte = read_byte(address);
			m_framebuffer[dstaddress] = thebyte;
			dstaddress++;
			normal_count--;
			address++;

		}
	}

}


WRITE16_MEMBER(kaneko_grap2_device::regs1_go_w)
{
	uint32_t address = m_regs1_address_regs[1]| (m_regs1_address_regs[0]<<16);

//  printf("regs1_go_w? %08x\n",address );
	if ((data==0x2000) || (data==0x3000)) do_rle(address);
}


void kaneko_grap2_device::set_color_555(pen_t color, int rshift, int gshift, int bshift, uint16_t data)
{
	set_pen_color(color, pal5bit(data >> rshift), pal5bit(data >> gshift), pal5bit(data >> bshift));
}

WRITE16_MEMBER(kaneko_grap2_device::framebuffer1_palette_w)
{
	COMBINE_DATA(&m_framebuffer_palette[offset]);
	set_color_555(offset, 5, 10, 0, m_framebuffer_palette[offset]);
}

/* definitely looks like a cycling bg colour used for the girls */
WRITE16_MEMBER(kaneko_grap2_device::framebuffer1_bgcol_w)
{
	COMBINE_DATA(&m_framebuffer_palette[0x100]);
	set_color_555(0x100, 5, 10, 0, m_framebuffer_palette[0x100]);
}
