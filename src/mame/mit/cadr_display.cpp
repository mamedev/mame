// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************************

CADR display emulation

**********************************************************************************/
#include "emu.h"
#include "cadr_display.h"

#include "emupal.h"
#include "screen.h"


//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(CADR_DISPLAY, cadr_display_device, "cadr_display", "CADR display")


cadr_display_device::cadr_display_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CADR_DISPLAY, tag, owner, clock)
	, m_palette(*this, "palette")
	, m_irq_cb(*this)
{
}

void cadr_display_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(0);
	screen.set_screen_update(FUNC(cadr_display_device::screen_update));
	screen.set_size(SCREEN_WIDTH, SCREEN_HEIGHT);
	screen.set_visarea_full();
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::MONOCHROME);
}


uint32_t cadr_display_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < SCREEN_HEIGHT; y++)
	{
		const u16 line_start = y * (SCREEN_WIDTH / 32);
		for (int x = 0; x < (SCREEN_WIDTH / 32); x++)
		{
			const u32 d = m_video_ram[line_start + x];
			const u32 xs = x * 32;
			for (int i = 0; i < 32; i++)
			{
				bitmap.pix(y, xs + i) = BIT(d, i);
			}
		}
	}
	return 0;
}


void cadr_display_device::device_start()
{
	m_video_ram = std::make_unique<u32[]>(VIDEO_RAM_SIZE);

	save_pointer(NAME(m_video_ram), VIDEO_RAM_SIZE);
	save_item(NAME(m_status));

	m_bitmap.allocate(SCREEN_WIDTH, SCREEN_HEIGHT, BITMAP_FORMAT_IND16);

	m_60hz_timer = timer_alloc(FUNC(cadr_display_device::tv_60hz_callback), this);
	m_60hz_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));
}


void cadr_display_device::device_reset()
{
	m_status = 0;
}


TIMER_CALLBACK_MEMBER(cadr_display_device::tv_60hz_callback)
{
	m_status |= 0x10;
	m_irq_cb(ASSERT_LINE);
}


u32 cadr_display_device::video_ram_read(offs_t offset)
{
	return m_video_ram[offset & (VIDEO_RAM_SIZE - 1)];
}


void cadr_display_device::video_ram_write(offs_t offset, u32 data)
{
	m_video_ram[offset & (VIDEO_RAM_SIZE - 1)] = data;
}


u32 cadr_display_device::status_r()
{
	return m_status;
}


void cadr_display_device::status_w(u32 data)
{
	m_status = data;
	m_status &= ~0x10;
	m_irq_cb(CLEAR_LINE);
}

