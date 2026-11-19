// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Inder / Dinamic Video */

#include "emu.h"
#include "inder_vid.h"

#include "screen.h"


DEFINE_DEVICE_TYPE(INDER_VIDEO, inder_vid_device, "indervd", "Inder / Dinamic TMS Video")


inder_vid_device::inder_vid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INDER_VIDEO, tag, owner, clock),
		m_vram(*this, "vram"),
		m_ramdac(*this, "ramdac"),
		m_tms(*this, "tms")
{
}

void inder_vid_device::megaphx_tms_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram().share(m_vram); // vram?
	map(0x04000000, 0x0400007f).m(m_ramdac, FUNC(bt478_device::map)).umask16(0x00ff);
	map(0x04000080, 0x0400008f).w(FUNC(inder_vid_device::mode_w)).umask16(0x00ff);
	map(0x04000090, 0x0400009f).w(FUNC(inder_vid_device::hscroll_w)).umask16(0x00ff);
	map(0x7fc00000, 0x7fffffff).ram().mirror(0x80000000);
}


void inder_vid_device::mode_w(uint8_t data)
{
	// 7654----  not used?
	// ----3---  4/8 bpp?
	// -----21-  not used?
	// -------0  4/8 bpp?

	// games write 0x08 for 4 bpp and 0x01 for 8 bpp
	m_8bpp_mode = BIT(data, 0);
}


void inder_vid_device::hscroll_w(uint8_t data)
{
	m_hscroll = data;
}


TMS340X0_SCANLINE_RGB32_CB_MEMBER(inder_vid_device::scanline)
{
	uint16_t const *const vram = &m_vram[(params->rowaddr << 8) & 0x3ff00];
	uint32_t *const dest = &bitmap.pix(scanline);

	const pen_t *paldata = m_ramdac->pens();

	if (m_8bpp_mode)
	{
		int pixaddr = (params->coladdr << 1) + ((m_hscroll - 1) & 1);
		for (int x = params->heblnk; x < params->hsblnk; x++)
		{
			uint16_t pixels = vram[(pixaddr >> 1) & 0xff];
			dest[x] = paldata[(pixels >> ((pixaddr & 1) * 8)) & 0xff];
			pixaddr++;
		}
	}
	else
	{
		int pixaddr = (params->coladdr << 2) + ((m_hscroll - 2) & 3);
		for (int x = params->heblnk; x < params->hsblnk; x++)
		{
			uint16_t pixels = vram[(pixaddr >> 2) & 0xff];
			dest[x] = paldata[(pixels >> ((pixaddr & 3) * 4)) & 0x0f];
			pixaddr++;
		}
	}
}


TMS340X0_TO_SHIFTREG_CB_MEMBER(inder_vid_device::to_shiftreg)
{
	if (m_shiftfull == 0)
	{
		//printf("read to shift regs address %08x\n", address);

		memcpy(shiftreg, &m_vram[(address & ~0x1fff) >> 4], 0x400); // & ~0x1fff is needed for round 6
		m_shiftfull = 1;
	}
}

TMS340X0_FROM_SHIFTREG_CB_MEMBER(inder_vid_device::from_shiftreg)
{
//  printf("write from shift regs address %08x\n", address);

	memcpy(&m_vram[(address & ~0x1fff) >> 4], shiftreg, 0x400);

	m_shiftfull = 0;
}


void inder_vid_device::device_add_mconfig(machine_config &config)
{
	TMS34010(config, m_tms, XTAL(40'000'000));
	m_tms->set_addrmap(AS_PROGRAM, &inder_vid_device::megaphx_tms_map);
	m_tms->set_halt_on_reset(true);
	m_tms->set_pixel_clock(XTAL(40'000'000)/12);
	m_tms->set_pixels_per_clock(2);
	m_tms->set_scanline_rgb32_callback(FUNC(inder_vid_device::scanline));
	m_tms->output_int().set_inputline(":maincpu", 4);
	m_tms->set_shiftreg_in_callback(FUNC(inder_vid_device::to_shiftreg));
	m_tms->set_shiftreg_out_callback(FUNC(inder_vid_device::from_shiftreg));

	screen_device &screen(SCREEN(config, "inder_screen"));
	screen.set_raw(XTAL(40'000'000)/12, 424, 0, 338, 262, 0, 246);
	screen.set_screen_update("tms", FUNC(tms34010_device::tms340x0_rgb32));

	BT478(config, m_ramdac);
}


void inder_vid_device::device_start()
{
	save_item(NAME(m_shiftfull));
	save_item(NAME(m_8bpp_mode));
	save_item(NAME(m_hscroll));
}

void inder_vid_device::device_reset()
{
	m_shiftfull = 0;
}
