// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99/2 main board logic

    This component implements the custom video controller and interface chip
    from the TI-99/2 console.

    See 992board.cpp for documentation

    Michael Zapf

*****************************************************************************/

#ifndef MAME_BUS_TI99_INTERNAL_992BOARD_H
#define MAME_BUS_TI99_INTERNAL_992BOARD_H

#pragma once

#include "screen.h"

namespace bus { namespace ti99 { namespace internal {

class video992_device : public device_t,
							public device_video_interface
{
public:
	// Complete line (31.848 us)
	static constexpr unsigned TOTAL_HORZ                 = 342;
	static constexpr unsigned TOTAL_VERT_NTSC            = 262;

	template <class Object> devcb_base &set_readmem_callback(Object &&cb) { return m_mem_read_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_hold_callback(Object &&cb) { return m_hold_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_int_callback(Object &&cb) { return m_int_cb.set_callback(std::forward<Object>(cb)); }

	// Delay(2) + ColorBurst(14) + Delay(8) + LeftBorder(13)
	static constexpr unsigned HORZ_DISPLAY_START         = 2 + 14 + 8 + 13;
	// RightBorder(15) + Delay(8) + HorizSync(26)
	static constexpr unsigned VERT_DISPLAY_START_NTSC    = 13 + 27;

	// update the screen
	uint32_t screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );

	// Video enable
	DECLARE_WRITE_LINE_MEMBER( videna );

protected:
	video992_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	int     m_beol;

private:
	static const device_timer_id HOLD_TIME = 0;
	static const device_timer_id FREE_TIME = 1;

	void device_start() override;
	void device_reset() override;
	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	std::string tts(attotime t);
	devcb_read8   m_mem_read_cb; // Callback to read memory
	devcb_write_line m_hold_cb;
	devcb_write_line m_int_cb;

	bitmap_rgb32 m_tmpbmp;
	emu_timer   *m_free_timer;
	emu_timer   *m_hold_timer;

	int         m_top_border;
	int         m_vertical_size;

	rgb_t       m_border_color;
	rgb_t       m_background_color;
	rgb_t       m_text_color;

	bool        m_videna;

	attotime    m_hold_time;
};

/* Variant for TI-99/2 24K */
class video992_24_device : public video992_device
{
public:
	video992_24_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

/* Variant for TI-99/2 32K */
class video992_32_device : public video992_device
{
public:
	video992_32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

} } } // end namespace bus::ti99::internal

#define MCFG_VIDEO992_SCREEN_ADD(_screen_tag) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag) \
	MCFG_SCREEN_ADD( _screen_tag, RASTER ) \
	MCFG_SCREEN_RAW_PARAMS( XTAL(10'738'635) / 2, bus::ti99::internal::video992_device::TOTAL_HORZ, bus::ti99::internal::video992_device::HORZ_DISPLAY_START-12, bus::ti99::internal::video992_device::HORZ_DISPLAY_START + 256 + 12, \
			bus::ti99::internal::video992_device::TOTAL_VERT_NTSC, bus::ti99::internal::video992_device::VERT_DISPLAY_START_NTSC - 12, bus::ti99::internal::video992_device::VERT_DISPLAY_START_NTSC + 192 + 12 )

#define MCFG_VIDEO992_MEM_ACCESS_CB(_devcb) \
	devcb = &downcast<bus::ti99::internal::video992_device &>(*device).set_readmem_callback(DEVCB_##_devcb);

#define MCFG_VIDEO992_HOLD_CB(_devcb) \
	devcb = &downcast<bus::ti99::internal::video992_device &>(*device).set_hold_callback(DEVCB_##_devcb);

#define MCFG_VIDEO992_INT_CB(_devcb) \
	devcb = &downcast<bus::ti99::internal::video992_device &>(*device).set_int_callback(DEVCB_##_devcb);

DECLARE_DEVICE_TYPE_NS(VIDEO99224, bus::ti99::internal, video992_24_device)
DECLARE_DEVICE_TYPE_NS(VIDEO99232, bus::ti99::internal, video992_32_device)

#endif // MAME_BUS_TI99_INTERNAL_992BOARD_H
