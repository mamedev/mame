// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Hudson/NEC HuC6260 Video Colour Encoder

    The HuC6260 takes a stream of pixel data, looks up the correct
    palette data and outputs a video stream.

    The HuC6260 generates the tv control signals. A full line lasts
    1365 "master" cycles (typically at 21.47727Mhz).

    HSync is low for 237 and high for 1128 master cycles.
    VSync is low for 4095 master cycles (3 lines).
    VSync changes 30 master cycles after HSync would go low.

**********************************************************************/

#include "emu.h"
#include "huc6260.h"

#include "screen.h"

#include <algorithm>

//#define VERBOSE 1
#include "logmacro.h"


#define HUC6260_HSYNC_LENGTH    237
#define HUC6260_HSYNC_START     (huc6260_device::WPF - HUC6260_HSYNC_LENGTH)


constexpr unsigned huc6260_device::PALETTE_SIZE;
constexpr unsigned huc6260_device::WPF;
constexpr unsigned huc6260_device::LPF;

void huc6260_device::palette_init()
{
	for (int i = 0; i < 512; i++)
	{
		int const r = pal3bit((i >> 3) & 7);
		int const g = pal3bit((i >> 6) & 7);
		int const b = pal3bit((i     ) & 7);
		int const y = ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;

		set_pen_color(i, r, g, b);
		set_pen_color(512 + i, y, y, y);
	}
}


DEFINE_DEVICE_TYPE(HUC6260, huc6260_device, "huc6260", "Hudson HuC6260 VCE")


huc6260_device::huc6260_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HUC6260, tag, owner, clock),
	device_palette_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_next_pixel_data_cb(*this, 0),
	m_time_til_next_event_cb(*this, 1),
	m_vsync_changed_cb(*this),
	m_hsync_changed_cb(*this)
{
}


TIMER_CALLBACK_MEMBER(huc6260_device::update_events)
{
	int const vpos = screen().vpos();
	int const hpos = screen().hpos();
	int h = m_last_h;
	int v = m_last_v;
	uint16_t *bitmap_line = &m_bmp.pix(v);

	while (h != hpos || v != vpos)
	{
		if (m_pixel_clock == 0)
		{
			auto profile = g_profiler.start(PROFILER_VIDEO);
			/* Get next pixel information */
			m_pixel_data = m_next_pixel_data_cb();
		}

		bitmap_line[h] = m_palette[m_pixel_data] | m_greyscales;
		m_pixel_clock = (m_pixel_clock + 1) % m_pixels_per_clock;
		h = (h + 1) % WPF;

		switch (h)
		{
		case HUC6260_HSYNC_START:       /* Start of HSync */
			m_hsync_changed_cb(0);
//          if (v == 0)
//          {
//              /* Check if the screen should be resized */
//              m_height = LPF - (m_blur ? 1 : 0);
//              if (m_height != video_screen_get_height(m_screen))
//              {
//                  rectangle visible_area;
//
//                  /* TODO: Set proper visible area parameters */
//                  visible_area.min_x = 64;
//                  visible_area.min_y = 18;
//                  visible_area.max_x = 64 + 1024 + 64 - 1;
//                  visible_area.max_y = 18 + 242 - 1;
//
//                  video_screen_configure(m_screen, WPF, m_height, &visible_area, HZ_TO_ATTOSECONDS(device->clock / (WPF * m_height)));
//              }
//          }
			break;

		case 0:     /* End of HSync */
			m_hsync_changed_cb(1);
			m_pixel_clock = 0;
			v = (v + 1) % m_height;
			bitmap_line = &m_bmp.pix(v);
			break;

		case HUC6260_HSYNC_START + 30:      /* End/Start of VSync */
			if (v>= m_height - 4)
			{
				m_vsync_changed_cb((v >= m_height - 4 && v < m_height - 1) ? 0 : 1);
			}
			break;
		}
	}

	m_last_h = h;
	m_last_v = v;

	/* Reschedule timer */
	if (m_last_h < HUC6260_HSYNC_START)
	{
		/* Next event is start of HSync signal */
		v = m_last_v;
		h = HUC6260_HSYNC_START;
	}
	else if ((m_last_v == m_height - 4 || m_last_v == m_height - 1) && m_last_h < HUC6260_HSYNC_START + 30)
	{
		/* Next event is start/end of VSync signal */
		v = m_last_v;
		h = HUC6260_HSYNC_START + 30;
	}
	else
	{
		/* Next event is end of HSync signal */
		v = (m_last_v + 1) % m_height;
		h = 0;
	}

	/* Ask our slave device for time until next possible event */
	{
		uint16_t next_event_clocks = m_time_til_next_event_cb();

		/* Adjust for pixel clocks per pixel */
		next_event_clocks *= m_pixels_per_clock;

		/* Adjust for clocks left to go for current pixel */
		next_event_clocks += (m_pixels_per_clock - (m_pixel_clock + 1));

		int event_hpos = hpos + next_event_clocks;
		int event_vpos = vpos;
		while (event_hpos > WPF)
		{
			event_vpos += 1;
			event_hpos -= WPF;
		}

		if (event_vpos < v || (event_vpos == v && event_hpos <= h))
		{
			if (event_vpos > vpos || (event_vpos == vpos && event_hpos > hpos))
			{
				v = event_vpos;
				h = event_hpos;
			}
		}
	}

	m_timer->adjust(screen().time_until_pos(v, h));
}


void huc6260_device::video_update(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bmp, 0, 0, 0, 0, cliprect);
}


// the battlera arcade board reads/writes the palette directly
uint8_t huc6260_device::palette_direct_read(offs_t offset)
{
	if (BIT(~offset, 0)) return m_palette[offset >> 1];
	else return m_palette[offset >> 1] >> 8;
}

void huc6260_device::palette_direct_write(offs_t offset, uint8_t data)
{
	if (BIT(~offset, 0)) m_palette[offset >> 1] = (m_palette[offset >> 1] & 0xff00) | data;
	else m_palette[offset >> 1] = (m_palette[offset >> 1] & 0x00ff) | (data << 8);
}

uint8_t huc6260_device::read(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset & 7)
	{
		case 0x04:  /* Color table data LSB */
			data = m_palette[m_address] & 0xff;
			break;

		case 0x05:  /* Color table data MSB */
			data = 0xfe | (m_palette[m_address] >> 8);
			/* Increment internal address */
			if (!machine().side_effects_disabled())
				m_address = (m_address + 1) & 0x1ff;
			break;
	}

	return data;
}


void huc6260_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 7)
	{
		case 0x00:  /* Control register */
			m_greyscales = (data & 0x80) << 2; // setup the greyscale base
			m_blur = BIT(data, 2);
			m_pixels_per_clock = BIT(data, 1) ? 2 : (BIT(data, 0) ? 3 : 4);
			break;

		case 0x02:  /* Color table address LSB */
			m_address = ((m_address & 0xff00) | data) & 0x1ff;
			break;

		case 0x03:  /* Color table address MSB */
			m_address = ((m_address & 0x00ff) | (data << 8)) & 0x1ff;
			break;

		case 0x04:  /* Color table data LSB */
			m_palette[m_address] = ((m_palette[m_address] & 0xff00) | data) & 0x1ff;
			break;

		case 0x05:  /* Color table data MSB */
			m_palette[m_address] = ((m_palette[m_address] & 0x00ff) | (data << 8)) & 0x1ff;

			/* Increment internal address */
			m_address = (m_address + 1) & 0x1ff;
			break;
	}
}


void huc6260_device::device_start()
{
	m_timer = timer_alloc(FUNC(huc6260_device::update_events), this);
	m_bmp.allocate(WPF, LPF);

	/* We want to have a valid screen and valid callbacks */
	assert(!m_next_pixel_data_cb.isunset());
	assert(!m_time_til_next_event_cb.isunset());

	palette_init();

	save_item(NAME(m_last_h));
	save_item(NAME(m_last_v));
	save_item(NAME(m_height));
	save_item(NAME(m_palette));
	save_item(NAME(m_address));
	save_item(NAME(m_greyscales));
	save_item(NAME(m_blur));
	save_item(NAME(m_pixels_per_clock));
	save_item(NAME(m_pixel_data));
	save_item(NAME(m_pixel_clock));
	save_item(NAME(m_bmp));
}


void huc6260_device::device_reset()
{
	m_address = 0;
	m_greyscales = 0;
	m_blur = false;
	m_pixels_per_clock = 4;
	m_height = 263;
	m_pixel_clock = 0;
	std::fill(std::begin(m_palette), std::end(m_palette), 0);

	m_last_v = screen().vpos();
	m_last_h = screen().hpos();
	m_timer->adjust(screen().time_until_pos((screen().vpos() + 1) % 263, 0));
}
