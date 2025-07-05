// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************************

CADR TV Control emulation

The TV control contains sync ram which looks like it can be configured to
generate output to different types of TV displays.

This currently emulates a 768x939 monochrome display.

Other information about displays:

Color screen: 454x576, 16 colors, RGB.
Grayscale monitor: 454x576, 16 scales of gray.
'CPT' monitor: 768x896, monochrome?

60MHz and 64Mhz clocks are mentioned in the docs, as well as 64.69Hz default refresh rate.

Implementation based on description of the operation, not schematics.


**********************************************************************************/
#include "emu.h"
#include "cadr_tv_control.h"


//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


namespace {

// Docs mention a 768x900 display, but this displays the entire screen.
// CPT monitor was 768x896.
static constexpr u16 SCREEN_WIDTH = 768;
static constexpr u16 SCREEN_HEIGHT = 939;
static constexpr u16 VIDEO_RAM_SIZE = 32 * 1024;
static constexpr u16 VIDEO_RAM_MASK = VIDEO_RAM_SIZE - 1;
static constexpr u16 SYNC_RAM_SIZE = 0x200; // Guess, noticed writes up to the 0x01xx range.
static constexpr u16 SYNC_RAM_MASK = SYNC_RAM_SIZE - 1;

} // anonymous namespace


DEFINE_DEVICE_TYPE(CADR_TV_CONTROL, cadr_tv_control_device, "cadr_tv_control", "CADR TV Control")


cadr_tv_control_device::cadr_tv_control_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CADR_TV_CONTROL, tag, owner, clock)
	, m_screen(*this, "screen")
	, m_irq_cb(*this)
{
}

void cadr_tv_control_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(0);
	m_screen->set_screen_update(FUNC(cadr_tv_control_device::screen_update));
	m_screen->set_size(SCREEN_WIDTH, SCREEN_HEIGHT);
	m_screen->set_visarea_full();
}


uint32_t cadr_tv_control_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const u32 black = 0x000000;
	const u32 white = 0xffffff;

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		const u16 line_start = y * (SCREEN_WIDTH / 32);

		for (int x = 0; x < (SCREEN_WIDTH / 32); x++)
		{
			const u32 d = m_video_ram[line_start + x];
			const u32 xs = x * 32;

			for (int i = 0; i < 32; i++)
			{
				bitmap.pix(y, xs + i) = BIT(d, i) ? white : black;
			}
		}
	}
	return 0;
}


void cadr_tv_control_device::device_start()
{
	m_video_ram = std::make_unique<u32[]>(VIDEO_RAM_SIZE);
	m_sync_ram = std::make_unique<u8[]>(SYNC_RAM_SIZE);

	save_pointer(NAME(m_video_ram), VIDEO_RAM_SIZE);
	save_pointer(NAME(m_sync_ram), SYNC_RAM_SIZE);
	save_item(NAME(m_status));
	save_item(NAME(m_sync_csr));
	save_item(NAME(m_sync_address));

	m_60hz_timer = timer_alloc(FUNC(cadr_tv_control_device::tv_60hz_callback), this);
	m_60hz_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));
}


void cadr_tv_control_device::device_reset()
{
	m_status = 0;
	m_sync_csr = 0;
	m_sync_address = 0;
}


TIMER_CALLBACK_MEMBER(cadr_tv_control_device::tv_60hz_callback)
{
	m_status |= 0x10;
	m_irq_cb(ASSERT_LINE);
}


u32 cadr_tv_control_device::video_ram_read(offs_t offset)
{
	return m_video_ram[offset & VIDEO_RAM_MASK];
}


void cadr_tv_control_device::video_ram_write(offs_t offset, u32 data)
{
	m_video_ram[offset & VIDEO_RAM_MASK] = data;
}


u32 cadr_tv_control_device::tv_control_r(offs_t offset)
{
	switch (offset)
	{
	case 0x00:
		// -------- --x----- 0 = vertical retrace
		return (m_status & ~0x20) | (m_screen->vblank() ? 0 : 0x20);
	case 0x01: // DATA
		return m_sync_ram[m_sync_address & SYNC_RAM_MASK];
	}
	return 0xffffffff;
}



void cadr_tv_control_device::tv_control_w(offs_t offset, u32 data)
{
	switch (offset)
	{
	case 0x00:
		m_status = data;
		m_status &= ~0x10;
		m_irq_cb(CLEAR_LINE);
		break;
	case 0x01: // DATA
		m_sync_ram[m_sync_address & SYNC_RAM_MASK] = data;
		break;
	case 0x02: // ADR
		m_sync_address = data;
		break;
	case 0x03: // Sync control/status
		// 200 is written to enable Sync RAM, disable SYNC, and/or enable SYNC.
		m_sync_csr = data;
		break;

	// unknown
	case 0x04: // Related to writing the color map
		break;
	}
}
