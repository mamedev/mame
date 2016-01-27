// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Kaneko GRAP2, RLE blitter / Framebuffer etc.? */


// todo: we're still far too heavily tied to galspanic3, which does the rendering by pulling a bunch
//       of our internals
//       lots of unknowns, both here and in rendering / mixing 3 chips in gp3





#include "emu.h"
#include "kaneko_grap2.h"

const device_type KANEKO_GRAP2 = &device_creator<kaneko_grap2_device>;

kaneko_grap2_device::kaneko_grap2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, KANEKO_GRAP2, "Kaneko GRAP2", tag, owner, clock, "kaneko_grap2", __FILE__),
	m_palette(*this)
{
	m_chipnum = 0;
}


void kaneko_grap2_device::set_chipnum(device_t &device, int chipnum)
{
	kaneko_grap2_device &dev = downcast<kaneko_grap2_device &>(device);
	dev.m_chipnum = chipnum;
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void kaneko_grap2_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<kaneko_grap2_device &>(device).m_palette.set_tag(tag);
}


void kaneko_grap2_device::device_start()
{
	m_framebuffer = make_unique_clear<UINT16[]>(0x80000/2);
	m_framebuffer_palette = make_unique_clear<UINT16[]>(0x200/2);
	m_framebuffer_unk1 = make_unique_clear<UINT16[]>(0x400/2);
	m_framebuffer_unk2 = make_unique_clear<UINT16[]>(0x400/2);

	save_pointer(NAME(m_framebuffer.get()), 0x80000/2);
	save_pointer(NAME(m_framebuffer_palette.get()), 0x200/2);
	save_pointer(NAME(m_framebuffer_unk1.get()), 0x400/2);
	save_pointer(NAME(m_framebuffer_unk2.get()), 0x400/2);

	save_item(NAME(m_framebuffer_bgcol));
	save_item(NAME(m_framebuffer_scrolly));
	save_item(NAME(m_framebuffer_scrollx));
	save_item(NAME(m_framebuffer_enable));
	save_item(NAME(m_regs1_i));
	save_item(NAME(m_framebuffer_bright1));
	save_item(NAME(m_framebuffer_bright2));
	save_item(NAME(m_regs1_address_regs[0x0]));
	save_item(NAME(m_regs1_address_regs[0x1]));

}

void kaneko_grap2_device::device_reset()
{
	m_framebuffer_bgcol = 0;
	m_framebuffer_scrolly = 0;
	m_framebuffer_scrollx = 0;
	m_framebuffer_enable = 0;
	m_regs1_i = 0x0;

	m_framebuffer_bright1 = 0;
	m_framebuffer_bright2 = 0;
}


READ16_MEMBER(kaneko_grap2_device::galpani3_regs1_r)
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
			logerror("cpu '%s' (PC=%06X): galpani3_regs1_r %02x %04x\n", space.device().tag(), space.device().safe_pcbase(), offset, mem_mask);
			break;

	}

	return 0x0000;
}



void kaneko_grap2_device::gp3_do_rle(UINT32 address, UINT16*framebuffer, UINT8* rledata)
{
	int rle_count = 0;
	int normal_count = 0;
	UINT32 dstaddress = 0;

	UINT8 thebyte;

	while (dstaddress<0x40000)
	{
		if (rle_count==0 && normal_count==0) // we need a new code byte
		{
			thebyte = rledata[address];

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
			thebyte = rledata[address];
			framebuffer[dstaddress] = thebyte;
			dstaddress++;
			rle_count--;

			if (rle_count==0)
			{
				address++;
			}
		}
		else if (normal_count)
		{
			thebyte = rledata[address];
			framebuffer[dstaddress] = thebyte;
			dstaddress++;
			normal_count--;
			address++;

		}
	}

}


WRITE16_MEMBER(kaneko_grap2_device::galpani3_regs1_go_w)
{
	UINT32 address = m_regs1_address_regs[1]| (m_regs1_address_regs[0]<<16);
	UINT8* rledata = memregion(":gfx2")->base();

//  printf("galpani3_regs1_go_w? %08x\n",address );
	if ((data==0x2000) || (data==0x3000)) gp3_do_rle(address, m_framebuffer.get(), rledata);
}


void kaneko_grap2_device::set_color_555_gp3(pen_t color, int rshift, int gshift, int bshift, UINT16 data)
{
	m_palette->set_pen_color(color, pal5bit(data >> rshift), pal5bit(data >> gshift), pal5bit(data >> bshift));
}

WRITE16_MEMBER(kaneko_grap2_device::galpani3_framebuffer1_palette_w)
{
	COMBINE_DATA(&m_framebuffer_palette[offset]);
	set_color_555_gp3(offset+0x4000 + (m_chipnum * 0x100), 5, 10, 0, m_framebuffer_palette[offset]);
}

/* definitely looks like a cycling bg colour used for the girls */
WRITE16_MEMBER(kaneko_grap2_device::galpani3_framebuffer1_bgcol_w)
{
	COMBINE_DATA(&m_framebuffer_bgcol);
	set_color_555_gp3(offset+0x4300 + (m_chipnum), 5, 10, 0, m_framebuffer_bgcol);
}
