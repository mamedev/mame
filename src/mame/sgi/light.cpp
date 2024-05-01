// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Tyson Smith
/*
        Silicon Graphics LG1 "Light" graphics board used as
        entry level graphics in the Indigo and IRIS Crimson.
*/

/*
 * TODO:
 *  - devicify rex
 *  - slotify
 *  - later board revisions and LG2X2 dual-head variant
 *
 * WIP:
 *  - passes power-on diagnostic tests
 *  - usable for firmware graphic output
 *  - fails badly for IRIX graphic output
 */

#include "emu.h"
#include "light.h"

#define LOG_REX (1U << 1)

//#define VERBOSE (LOG_GENERAL|LOG_REX)
#include "logmacro.h"

static constexpr uint32_t x_res = 1024;
static constexpr uint32_t y_res = 768;

enum
{
	REX15_OP_NOP                = 0x00000000,
	REX15_OP_DRAW               = 0x00000001,
	REX15_OP_FLAG_BLOCK         = 0x00000008,
	REX15_OP_FLAG_LENGTH32      = 0x00000010,
	REX15_OP_FLAG_QUADMODE      = 0x00000020,
	REX15_OP_FLAG_XYCONTINUE    = 0x00000080,
	REX15_OP_FLAG_STOPONX       = 0x00000100,
	REX15_OP_FLAG_STOPONY       = 0x00000200,
	REX15_OP_FLAG_ENZPATTERN    = 0x00000400,
	REX15_OP_FLAG_LOGICSRC      = 0x00080000,
	REX15_OP_FLAG_ZOPAQUE       = 0x00800000,
	REX15_OP_FLAG_ZCONTINUE     = 0x01000000,
};

DEFINE_DEVICE_TYPE(SGI_LG1, sgi_lg1_device, "sgi_lg1", "SGI LG1")

sgi_lg1_device::sgi_lg1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SGI_LG1, tag, owner, clock)
	, m_vc1(*this, "vc1")
	, m_lut1(*this, "lut1")
	, m_screen(*this, "screen")
{
}

void sgi_lg1_device::device_add_mconfig(machine_config &config)
{
	SGI_VC1(config, m_vc1, 64_MHz_XTAL);

	// TODO: does LUT1 differ from Bt479?
	BT479(config, m_lut1, 64_MHz_XTAL);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// TODO: video timing from VC1
	m_screen->set_raw(1024 * 768 * 60, 1024, 0, 1024, 768, 0, 768);
	m_screen->set_screen_update(FUNC(sgi_lg1_device::screen_update));
}

void sgi_lg1_device::device_start()
{
	m_vram = std::make_unique<u8[]>(0x10'0000); // 8xTC524256BJ-10 262,144x4 multiport dram

	save_pointer(NAME(m_vram), 0x10'0000);
}

void sgi_lg1_device::device_reset()
{
}

void sgi_lg1_device::map(address_map &map)
{
	map(0x0000, 0x0003).select(0x0800).rw(FUNC(sgi_lg1_device::command_r), FUNC(sgi_lg1_device::command_w));
	map(0x0004, 0x0007).select(0x0800).rw(FUNC(sgi_lg1_device::aux1_r), FUNC(sgi_lg1_device::aux1_w));
	map(0x0008, 0x000b).select(0x0800).rw(FUNC(sgi_lg1_device::xstate_r), FUNC(sgi_lg1_device::xstate_w));
	map(0x000c, 0x000f).select(0x0800).rw(FUNC(sgi_lg1_device::xstarti_r), FUNC(sgi_lg1_device::xstarti_w));
	map(0x0010, 0x0013).select(0x0800).rw(FUNC(sgi_lg1_device::xstartf_r), FUNC(sgi_lg1_device::xstartf_w));
	map(0x0014, 0x0017).select(0x0800).rw(FUNC(sgi_lg1_device::xstart_r), FUNC(sgi_lg1_device::xstart_w));
	map(0x0018, 0x001b).select(0x0800).rw(FUNC(sgi_lg1_device::xendf_r), FUNC(sgi_lg1_device::xendf_w));
	map(0x001c, 0x001f).select(0x0800).rw(FUNC(sgi_lg1_device::ystarti_r), FUNC(sgi_lg1_device::ystarti_w));
	map(0x0020, 0x0023).select(0x0800).rw(FUNC(sgi_lg1_device::ystartf_r), FUNC(sgi_lg1_device::ystartf_w));
	map(0x0024, 0x0027).select(0x0800).rw(FUNC(sgi_lg1_device::ystart_r), FUNC(sgi_lg1_device::ystart_w));
	map(0x0028, 0x002b).select(0x0800).rw(FUNC(sgi_lg1_device::yendf_r), FUNC(sgi_lg1_device::yendf_w));
	map(0x002c, 0x002f).select(0x0800).rw(FUNC(sgi_lg1_device::xsave_r), FUNC(sgi_lg1_device::xsave_w));
	map(0x0030, 0x0033).select(0x0800).rw(FUNC(sgi_lg1_device::minorslope_r), FUNC(sgi_lg1_device::minorslope_w));
	map(0x0034, 0x0037).select(0x0800).rw(FUNC(sgi_lg1_device::xymove_r), FUNC(sgi_lg1_device::xymove_w));
	map(0x0038, 0x003b).select(0x0800).rw(FUNC(sgi_lg1_device::colori_r<0>), FUNC(sgi_lg1_device::colori_w<0>));
	map(0x003c, 0x003f).select(0x0800).rw(FUNC(sgi_lg1_device::colorf_r<0>), FUNC(sgi_lg1_device::colorf_w<0>));
	map(0x0040, 0x0043).select(0x0800).rw(FUNC(sgi_lg1_device::colori_r<1>), FUNC(sgi_lg1_device::colori_w<1>));
	map(0x0044, 0x0047).select(0x0800).rw(FUNC(sgi_lg1_device::colorf_r<1>), FUNC(sgi_lg1_device::colorf_w<1>));
	map(0x0048, 0x004b).select(0x0800).rw(FUNC(sgi_lg1_device::colori_r<2>), FUNC(sgi_lg1_device::colori_w<2>));
	map(0x004c, 0x004f).select(0x0800).rw(FUNC(sgi_lg1_device::colorf_r<2>), FUNC(sgi_lg1_device::colorf_w<2>));
	map(0x0050, 0x0053).select(0x0800).rw(FUNC(sgi_lg1_device::slopecolor_r<0>), FUNC(sgi_lg1_device::slopecolor_w<0>));
	map(0x0054, 0x0057).select(0x0800).rw(FUNC(sgi_lg1_device::slopecolor_r<1>), FUNC(sgi_lg1_device::slopecolor_w<1>));
	map(0x0058, 0x005b).select(0x0800).rw(FUNC(sgi_lg1_device::slopecolor_r<2>), FUNC(sgi_lg1_device::slopecolor_w<2>));
	map(0x005c, 0x005f).select(0x0800).rw(FUNC(sgi_lg1_device::colorback_r), FUNC(sgi_lg1_device::colorback_w));
	map(0x0060, 0x0063).select(0x0800).rw(FUNC(sgi_lg1_device::zpattern_r), FUNC(sgi_lg1_device::zpattern_w));
	map(0x0064, 0x0067).select(0x0800).rw(FUNC(sgi_lg1_device::lspattern_r), FUNC(sgi_lg1_device::lspattern_w));
	map(0x0068, 0x006b).select(0x0800).rw(FUNC(sgi_lg1_device::lsmode_r), FUNC(sgi_lg1_device::lsmode_w));
	map(0x006c, 0x006f).select(0x0800).rw(FUNC(sgi_lg1_device::aweight_r), FUNC(sgi_lg1_device::aweight_w));
	map(0x0070, 0x0073).select(0x0800).rw(FUNC(sgi_lg1_device::rwaux1_r), FUNC(sgi_lg1_device::rwaux1_w));
	map(0x0074, 0x0077).select(0x0800).rw(FUNC(sgi_lg1_device::rwaux2_r), FUNC(sgi_lg1_device::rwaux2_w));
	map(0x0078, 0x007b).select(0x0800).rw(FUNC(sgi_lg1_device::rwmask_r), FUNC(sgi_lg1_device::rwmask_w));
	map(0x007c, 0x007f).select(0x0800).rw(FUNC(sgi_lg1_device::smask_r<0>), FUNC(sgi_lg1_device::smask_w<0>));
	map(0x0080, 0x0083).select(0x0800).rw(FUNC(sgi_lg1_device::smask_r<1>), FUNC(sgi_lg1_device::smask_w<1>));
	map(0x0084, 0x0087).select(0x0800).rw(FUNC(sgi_lg1_device::xendi_r), FUNC(sgi_lg1_device::xendi_w));
	map(0x0088, 0x008b).select(0x0800).rw(FUNC(sgi_lg1_device::yendi_r), FUNC(sgi_lg1_device::yendi_w));

	// configuration registers
	map(0x4790, 0x4793).select(0x0800).rw(FUNC(sgi_lg1_device::smask_r<2>), FUNC(sgi_lg1_device::smask_w<2>));
	map(0x4794, 0x4797).select(0x0800).rw(FUNC(sgi_lg1_device::smask_r<3>), FUNC(sgi_lg1_device::smask_w<3>));
	map(0x4798, 0x479b).select(0x0800).rw(FUNC(sgi_lg1_device::smask_r<4>), FUNC(sgi_lg1_device::smask_w<4>));
	map(0x479c, 0x479f).select(0x0800).rw(FUNC(sgi_lg1_device::smask_r<5>), FUNC(sgi_lg1_device::smask_w<5>));
	map(0x47a0, 0x47a3).select(0x0800).rw(FUNC(sgi_lg1_device::smask_r<6>), FUNC(sgi_lg1_device::smask_w<6>));
	map(0x47a4, 0x47a7).select(0x0800).rw(FUNC(sgi_lg1_device::smask_r<7>), FUNC(sgi_lg1_device::smask_w<7>));
	map(0x47a8, 0x47ab).select(0x0800).rw(FUNC(sgi_lg1_device::aux2_r), FUNC(sgi_lg1_device::aux2_w));

	map(0x47d8, 0x47db).select(0x0800).rw(FUNC(sgi_lg1_device::diagvram_r), FUNC(sgi_lg1_device::diagvram_w));
	map(0x47dc, 0x47df).select(0x0800).rw(FUNC(sgi_lg1_device::diagcid_r), FUNC(sgi_lg1_device::diagcid_w));

	map(0x47e4, 0x47e7).select(0x0800).rw(FUNC(sgi_lg1_device::wclock_r), FUNC(sgi_lg1_device::wclock_w));
	map(0x47e8, 0x47eb).select(0x0800).rw(FUNC(sgi_lg1_device::dac_r), FUNC(sgi_lg1_device::dac_w));
	map(0x47ec, 0x47ef).select(0x0800).rw(FUNC(sgi_lg1_device::configsel_r), FUNC(sgi_lg1_device::configsel_w));
	map(0x47f0, 0x47f3).select(0x0800).rw(FUNC(sgi_lg1_device::vc1_r), FUNC(sgi_lg1_device::vc1_w));
	map(0x47f4, 0x47f7).select(0x0800).rw(FUNC(sgi_lg1_device::togglectxt_r), FUNC(sgi_lg1_device::togglectxt_w));
	map(0x47f8, 0x47fb).select(0x0800).rw(FUNC(sgi_lg1_device::configmode_r), FUNC(sgi_lg1_device::configmode_w));
	map(0x47fc, 0x47ff).select(0x0800).rw(FUNC(sgi_lg1_device::xywin_r), FUNC(sgi_lg1_device::xywin_w));
}

u32 sgi_lg1_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	u8 const *src = &m_vram[0];

	for (unsigned y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
		for (unsigned x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x++)
			bitmap.pix(y, x) = m_lut1->palette_lookup(*src++);

	return 0;
}

void sgi_lg1_device::minorslope_w(u32 data)
{
	if (BIT(data, 31))
		data = ~(data & 0x8000'ffffU) + 1;

	m_minorslope = data & 0x1'ffffU;
}

template <unsigned Color> void sgi_lg1_device::slopecolor_w(u32 data)
{
	if (BIT(data, 31))
		data = ~(data & 0x800f'ffffU) + 1;

	m_slopecolor[Color] = ((data & 0x0008'0000U) << 1) | (data & 0x001f'ffffU);
}

/*
 * This logic assumes that there is an 8 bit latch used to interface to the
 * LUT1 and VC1. The GO signal controls whether the latch is read or written
 * by the host or by the addressed device.
 */

u32 sgi_lg1_device::dac_r(offs_t offset)
{
	if (offset & 0x200)
		m_data = m_lut1->read(m_configsel);

	return m_data;
}

void sgi_lg1_device::dac_w(offs_t offset, u32 data)
{
	if (offset & 0x200)
		m_lut1->write(m_configsel, m_data);
	else
		m_data = data;
}

u32 sgi_lg1_device::vc1_r(offs_t offset)
{
	if (offset & 0x200)
		m_data = m_vc1->read(m_configsel);

	return m_data;
}

void sgi_lg1_device::vc1_w(offs_t offset, u32 data)
{
	if (offset & 0x200)
		m_vc1->write(m_configsel, m_data);
	else
		m_data = data;
}

void sgi_lg1_device::do_rex_command()
{
	if (m_command == 0)
	{
		return;
	}
	if (m_command == 0x30080329)
	{
		bool xycontinue = (m_command & REX15_OP_FLAG_XYCONTINUE);
		bool copy = (m_command & REX15_OP_FLAG_LOGICSRC);
		const uint32_t start_x = xycontinue ? m_xcurri : xstarti_r();
		const uint32_t start_y = xycontinue ? m_ycurri : ystarti_r();

		const uint32_t src_start_x = start_x + (m_xymove >> 16);
		const uint32_t src_start_y = start_y + u16(m_xymove);

		LOGMASKED(LOG_REX, "command %08x: Block copy from %d,%d-%d,%d inclusive.\n", m_command, start_x, start_y, xendi_r(), yendi_r());
		if (copy)
		{
			for (uint32_t y = start_y, src_y = src_start_y; y <= yendi_r(); y++, src_y++)
				for (uint32_t x = start_x, src_x = src_start_x; x <= xendi_r(); x++, src_x++)
					m_vram[y*x_res + x] = m_vram[src_y*x_res + src_x];
		}
		else
		{
			for (uint32_t y = start_y; y <= yendi_r(); y++)
				for (uint32_t x = start_x; x <= xendi_r(); x++)
					m_vram[y*x_res + x] = colori_r<0>();
		}
	}
	else if (m_command == 0x30000329)
	{
		bool xycontinue = (m_command & REX15_OP_FLAG_XYCONTINUE);
		uint32_t start_x = xycontinue ? m_xcurri : xstarti_r();
		uint32_t start_y = xycontinue ? m_ycurri : ystarti_r();

		LOGMASKED(LOG_REX, "command %08x: Block draw from %d,%d-%d,%d inclusive.\n", m_command, start_x, start_y, xendi_r(), yendi_r());
		for (uint32_t y = start_y; y <= yendi_r(); y++)
		{
			for (uint32_t x = start_x; x <= xendi_r(); x++)
			{
				m_vram[y*x_res + x] = colori_r<0>();
			}
		}
	}
	else if (m_command == 0x300005a1 ||
		m_command == 0x300005a9 ||
		m_command == 0x300005b9)
	{
		bool xycontinue = (m_command & REX15_OP_FLAG_XYCONTINUE);
		uint32_t start_x = xycontinue ? m_xcurri : xstarti_r();
		uint32_t start_y = xycontinue ? m_ycurri : ystarti_r();

		LOGMASKED(LOG_REX, "command %08x: Pattern draw from %d-%d at %d\n", m_command, start_x, xendi_r(), start_y);
		for (uint32_t x = start_x; x <= xendi_r() && x < (start_x + 32); x++)
		{
			if (BIT(m_zpattern, 31 - (x - start_x)))
			{
				m_vram[start_y*x_res + x] = colori_r<0>();
			}
			m_xcurri++;
		}

		if (m_command & REX15_OP_FLAG_BLOCK)
		{
			if (m_xcurri > xendi_r())
			{
				m_ycurri--;
				m_xcurri = xstarti_r();
			}
		}
	}
	else
	{
		LOGMASKED(LOG_REX, "%s: unknown command: %08x\n", machine().describe_context(), m_command);
	}
}
