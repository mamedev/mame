// license:BSD-3-Clause
// copyright-holders:hap
/*

Konami 005849
-------------

Video controller, it does sprites, a 512*256 tilemap, and interrupts.
It has similarities with the newer Konami 007121.

control registers:

000:          scroll x? (always 0)
001:          scroll y? (always 0)

002: -------x scroll x high bit? (always 0)
     ------x- enable row/colscroll instead of normal scroll
     -----x-- if above is enabled: 0 = rowscroll, 1 = colscroll

003: ----x--- selects sprite ram bank/offset (0 = 0x0, 1 = 0x100)

004: -------x nmi enable
     ------x- irq enable
     -----x-- firq enable
     ----x--- flip screen

Above undocumented bits/regs = unknown

TODO:
- Move sprites and tilemap emulation from drivers to this device.

*/

#include "emu.h"
#include "k005849.h"
#include "konami_helper.h"

#include "screen.h"


DEFINE_DEVICE_TYPE(K005849, k005849_device, "k005849", "Konami 005849 Video Controller")

k005849_device::k005849_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, K005849, tag, owner, clock),
	device_video_interface(mconfig, *this),
	m_flipscreen(false),
	m_flipscreen_cb(*this),
	m_irq_cb(*this),
	m_firq_cb(*this),
	m_nmi_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k005849_device::device_start()
{
	save_item(NAME(m_ctrlram));
	save_item(NAME(m_scrollram));
	save_item(NAME(m_flipscreen));

	memset(m_ctrlram, 0, sizeof(m_ctrlram));
	memset(m_scrollram, 0, sizeof(m_scrollram));

	m_scanline_timer = timer_alloc(FUNC(k005849_device::scanline), this);
	m_scanline_timer->adjust(screen().time_until_pos(0), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k005849_device::device_reset()
{
	for (int i = 0; i < 8; i++)
		ctrl_w(i, 0);
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void k005849_device::ctrl_w(offs_t offset, uint8_t data)
{
	offset &= 7;

	if (offset == 4)
	{
		// clear interrupts
		if (BIT(~data & m_ctrlram[4], 1))
			m_irq_cb(CLEAR_LINE);

		if (BIT(~data & m_ctrlram[4], 2))
			m_firq_cb(CLEAR_LINE);

		if (BIT(~data & m_ctrlram[4], 0))
			m_nmi_cb(CLEAR_LINE);

		// flipscreen
		if (BIT(data ^ m_ctrlram[4], 3))
		{
			m_flipscreen = BIT(data, 3);
			m_flipscreen_cb(BIT(data, 3));
		}
	}

	m_ctrlram[offset] = data;
}


/*****************************************************************************
    INTERRUPTS
*****************************************************************************/

TIMER_CALLBACK_MEMBER(k005849_device::scanline)
{
	int scanline = param;

	// NMI 8 times per frame
	if (BIT(m_ctrlram[4], 0) && (scanline & 0x1f) == 0x10)
		m_nmi_cb(ASSERT_LINE);

	// vblank
	if (scanline == 240)
	{
		// FIRQ once every other frame
		if (BIT(m_ctrlram[4], 2) && screen().frame_number() & 1)
			m_firq_cb(ASSERT_LINE);

		if (BIT(m_ctrlram[4], 1))
			m_irq_cb(ASSERT_LINE);
	}

	// wait for next line
	scanline += 16;
	if (scanline >= screen().height())
		scanline = 0;

	m_scanline_timer->adjust(screen().time_until_pos(scanline), scanline);
}
