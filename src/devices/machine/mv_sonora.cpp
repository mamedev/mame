// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    Mac video support, "Sonora" edition
    Supports 5 different modelines at up to 16bpp

    The original Sonora ASIC and its follow-on Ardbeg require an
    external pixel clock source, while the version used in the PDM
    machines has an internal clock source that automatically is set
    appropriately for the video mode.

*********************************************************************/

#include "emu.h"
#include "mv_sonora.h"

DEFINE_DEVICE_TYPE(MAC_VIDEO_SONORA, mac_video_sonora_device, "mv_sonora", "Mac Sonora video support")

const mac_video_sonora_device::modeline mac_video_sonora_device::modelines[5] = {
	{ 0x02, "512x384 12\" RGB",      15667200,  640, 16, 32,  80,  407,  1, 3, 19,  true, false },
	{ 0x06, "640x480 13\" RGB",      31334400,  896, 80, 64, 112,  525,  3, 3, 39,  true, false },
	{ 0x01, "640x870 15\" Portrait", 57283200,  832, 32, 80,  80,  918,  3, 3, 42, false,  true },
	{ 0x09, "832x624 16\" RGB",      57283200, 1152, 32, 64, 224,  667,  1, 3, 39, false, false },
	{ 0x0b, "640x480 VGA",           25175000,  800, 16, 96,  48,  525, 10, 2, 33, false, false },
};

mac_video_sonora_device::mac_video_sonora_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MAC_VIDEO_SONORA, tag, owner, clock),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_monitor_config(*this, "monitor"),
	m_screen_vblank(*this),
	m_isPDM(false),
	m_is32bit(false)
{
}

void mac_video_sonora_device::device_start()
{
	m_vram = nullptr;

	save_item(NAME(m_vram_offset));
	save_item(NAME(m_mode));
	save_item(NAME(m_depth));
	save_item(NAME(m_monitor_id));
	save_item(NAME(m_vtest));
	save_item(NAME(m_modeline_id));
}

void mac_video_sonora_device::device_reset()
{
	m_modeline_id = -1;
	m_mode = 0x9f;
	m_depth = 0;
	m_monitor_id = 8;
	m_vtest = 0;
	m_vram_offset = 0;
}

void mac_video_sonora_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// dot clock, htotal, hstart, hend, vtotal, vstart, vend
	m_screen->set_raw(31334400, 896, 0, 640, 525, 0, 480);
	m_screen->set_screen_update(FUNC(mac_video_sonora_device::screen_update));
	m_screen->screen_vblank().set([this](int state) { m_screen_vblank(state && (m_modeline_id != -1)); });

	PALETTE(config, m_palette).set_entries(256);
}

// The monitor detection goes through 3 sense lines which are
// pulled-up but can be set to 0 or 1 by the system.

// The monitor itself either connects some lines to ground (old
// method) or connects some lines together, sometimes with a diode
// (extended monitor sense).

// The system starts by leaving everything undriven and reading the
// result.  If it's not 7, it's the monitor type.  If it's 7, then it
// needs the extended monitor sense.  It then drives a 0 on each of
// the 3 pins in sequence and records the result on the other two, and
// that gives the signature.

static constexpr uint8_t ext(uint8_t bc, uint8_t ac, uint8_t ab)
{
	return 0x40 | (bc << 4) | (ac << 2) | ab;
}

static INPUT_PORTS_START(monitor_config)
	PORT_START("monitor")
	PORT_CONFNAME(0x7f, 6, "Monitor type")
	PORT_CONFSETTING(2,            "512x384 12\" RGB")
	PORT_CONFSETTING(6,            "640x480 13\" RGB") // Biggest resolution with 16bpp support, hence default
	PORT_CONFSETTING(1,            "620x870 15\" Portrait")
	PORT_CONFSETTING(ext(2, 3, 1), "832x624 16\" RGB")
	PORT_CONFSETTING(ext(1, 1, 3), "640x480 VGA")
INPUT_PORTS_END


ioport_constructor mac_video_sonora_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(monitor_config);
}

uint32_t mac_video_sonora_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if((m_mode & 0x80) || m_modeline_id == -1 || m_depth > 4 || (m_depth == 4 && !modelines[m_modeline_id].supports_16bpp) || !m_vram) {
		bitmap.fill(0);
		return 0;
	}

	const auto &m = modelines[m_modeline_id];
	uint32_t hres = m.htot - m.hfp - m.hs - m.hbp;
	uint32_t vres = m.vtot - m.vfp - m.vs - m.vbp;

	const pen_t *pens = m_palette->pens();

	if (m_is32bit) {
		const uint32_t *vram = (uint32_t *)(m_vram + (m_vram_offset / 8));
		switch (m_depth)
		{
		case 0: // 1bpp
			for (uint32_t y = 0; y != vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (uint32_t x = 0; x != hres; x += 32)
				{
					uint32_t pixels = *vram++;
					for (int32_t bit = 31; bit >= 0; bit--)
						*scanline++ = pens[(((pixels >> bit) & 1) << 7) | 0x7f];
				}
			}
			break;

		case 1: // 2bpp
			for (uint32_t y = 0; y != vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (uint32_t x = 0; x != hres; x += 16)
				{
					uint32_t pixels = *vram++;
					for (int32_t bit = 30; bit >= 0; bit -= 2)
						*scanline++ = pens[(((pixels >> bit) & 0x3)<<6) | 0x3f];
				}
			}
			break;

		case 2: // 4bpp
			for (uint32_t y = 0; y != vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (uint32_t x = 0; x != hres; x += 8)
				{
					uint32_t pixels = *vram++;
					for (int32_t bit = 28; bit >= 0; bit -= 4)
						*scanline++ = pens[(((pixels >> bit) & 0x0f)<<4) | 0x0f];
				}
			}
			break;

		case 3: // 8bpp
			for (uint32_t y = 0; y != vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (uint32_t x = 0; x != hres; x += 4)
				{
					uint32_t pixels = *vram++;
					for (int32_t bit = 24; bit >= 0; bit -= 8)
						*scanline++ = pens[((pixels >> bit) & 0xff)];
				}
			}
			break;

		case 4: // 16bpp
			for (uint32_t y = 0; y != vres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (uint32_t x = 0; x != hres; x += 2)
				{
					const uint32_t pixels = *vram++;
					*scanline++ = rgb_t(((pixels >> 26) & 0x1f) << 3, ((pixels >> 21) & 0x1f) << 3, ((pixels >> 16) & 0x1f) << 3);
					*scanline++ = rgb_t(((pixels >> 10) & 0x1f) << 3, ((pixels >> 5) & 0x1f) << 3, (pixels & 0x1f) << 3);
				}
			}
			break;

		default:
			bitmap.fill(0xff0000);
			break;
		}
	} else {
		const uint64_t *vram = m_vram + (m_vram_offset / 8);
		switch(m_depth) {
		case 0: // 1bpp
			for(uint32_t y = 0; y != vres; y++) {
				uint32_t *scanline = &bitmap.pix(y);
				for(uint32_t x = 0; x != hres; x += 64) {
					uint64_t pixels = *vram ++;
					for(int32_t bit = 63; bit >= 0; bit --)
						*scanline ++ = pens[(((pixels >> bit) & 1) << 7) | 0x7f];
				}
			}
			break;

		case 3: // 8bpp
			for(uint32_t y = 0; y != vres; y++) {
				uint32_t *scanline = &bitmap.pix(y);
				for(uint32_t x = 0; x != hres; x += 8) {
					uint64_t pixels = *vram ++;
					for(int32_t bit = 56; bit >= 0; bit -= 8)
						*scanline ++ = pens[((pixels >> bit) & 0xff)];
				}
			}
			break;

		default:
			bitmap.fill(0xff0000);
			break;
		}
	}

	return 0;
}

uint8_t mac_video_sonora_device::vctrl_r(offs_t offset)
{
	switch(offset) {
	case 0: return m_mode;
	case 1: return m_depth;

	case 2: {
		uint8_t mon = m_monitor_config->read();
		uint8_t res;
		if(mon & 0x40) {
			res = 7;
			if(!(m_monitor_id & 0xc))
				res &= 4 | (BIT(mon, 5) << 1) | BIT(mon, 4);
			if(!(m_monitor_id & 0xa))
				res &= (BIT(mon, 3) << 2) | 2 | BIT(mon, 2);
			if(!(m_monitor_id & 0x9))
				res &= (BIT(mon, 1) << 2) | (BIT(mon, 0) << 1) | 1;

		} else {
			res = mon;
			if(!(m_monitor_id & 8))
				res &= m_monitor_id & 7;
		}

		return m_monitor_id | (res << 4);
	}

	case 3: return m_vtest;
	case 4: return (m_screen->hpos() >> 8) & 7;
	case 5: return m_screen->hpos() & 0xff;
	case 6: return (m_screen->vpos() >> 8) & 3;
	case 7: return m_screen->vpos() & 0xff;
	}

	return 0;
}

void mac_video_sonora_device::vctrl_w(offs_t offset, uint8_t data)
{
	switch(offset) {
	case 0: {
		int prev_modeline = m_modeline_id;
		m_mode = data & 0x9f;
		m_modeline_id = -1;
		for(unsigned i=0; i != std::size(modelines) && m_modeline_id == -1; i++)
			if((m_mode & 0x1f) == modelines[i].mode_id)
				m_modeline_id = i;

		logerror("Mode switch %02x %s%s\n", data,
				 m_mode & 0x80 ? "blanked " : "",
				 m_modeline_id == -1 ? "disabled" : modelines[m_modeline_id].name);

		if(m_modeline_id != -1 && m_modeline_id != prev_modeline) {
			const modeline &m = modelines[m_modeline_id];
			rectangle visarea(0, m.htot - m.hfp - m.hs - m.hbp - 1, 0, m.vtot - m.vfp - m.vs - m.vbp - 1);
			if (m_isPDM) {
				m_screen->configure(m.htot, m.vtot, visarea, attotime::from_ticks(m.htot*m.vtot, m.dotclock).as_attoseconds());
			} else {
				m_screen->configure(m.htot, m.vtot, visarea, attotime::from_ticks(m.htot * m.vtot, m_extPixelClock).as_attoseconds());
			}
		}
		break;
	}

	case 1:
		m_depth = data & 7;
		if(m_depth <= 4)
			logerror("Pixel depth %dbpp\n", 1 << m_depth);
		else
			logerror("Pixel depth invalid (%d)\n", m_depth);
		break;

	case 2:
		m_monitor_id = data & 0xf;
		break;

	case 3:
		m_vtest = data & 1;
		break;

	default:
		logerror("vctrl_w %x, %02x\n", offset, data);
	}
}

uint8_t mac_video_sonora_device::dac_r(offs_t offset)
{
	switch(offset) {
	case 2:
		return m_pal_control;

	default:
//      logerror("dac_r %x\n", offset);
		return 0;
	}
}

void mac_video_sonora_device::dac_w(offs_t offset, uint8_t data)
{
	switch(offset) {
	case 0:
		m_pal_address = data;
		m_pal_idx = 0;
		break;

	case 1:
		switch(m_pal_idx) {
		case 0: m_palette->set_pen_red_level(m_pal_address, data); break;
		case 1: m_palette->set_pen_green_level(m_pal_address, data); break;
		case 2:
			// monochrome monitors use the blue line as the video, so duplicate blue to all 3 primaries
			if (modelines[m_modeline_id].monochrome) {
				m_palette->set_pen_red_level(m_pal_address, data);
				m_palette->set_pen_green_level(m_pal_address, data);
				m_palette->set_pen_blue_level(m_pal_address, data);
			}
			else
			{
				m_palette->set_pen_blue_level(m_pal_address, data);
			}
			break;
		}
		m_pal_idx ++;
		if(m_pal_idx == 3) {
			m_pal_idx = 0;
			m_pal_address ++;
		}
		break;

	case 2:
		logerror("control = %02x\n", data);
		m_pal_control = data;
		break;

	case 3:
		m_pal_colkey = data;
		break;
	}
}
