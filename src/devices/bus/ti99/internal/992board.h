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
#include "bus/ti99/ti99defs.h"
#include "bus/hexbus/hexbus.h"
#include "imagedev/cassette.h"

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

	// Callbacks
	auto readmem_cb() { return m_mem_read_cb.bind(); }
	auto hold_cb() { return m_hold_cb.bind(); }
	auto int_cb() { return m_int_cb.bind(); }

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

/*
    I/O custom chip
*/
class io992_device : public bus::hexbus::hexbus_chained_device
{
public:
	io992_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER( cruread );
	DECLARE_WRITE8_MEMBER( cruwrite );
	void device_start() override;
	ioport_constructor device_input_ports() const override;
	auto rombank_cb() { return m_set_rom_bank.bind(); }

protected:
	io992_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	bool m_have_banked_rom;

	void hexbus_value_changed(uint8_t data) override;

private:
	const uint8_t m_hexbval[6] = { 0x01, 0x02, 0x40, 0x80, 0x10, 0x04 };

	required_device<hexbus::hexbus_device> m_hexbus;
	required_device<cassette_image_device> m_cassette;
	required_device<video992_device> m_videoctrl;
	required_ioport_array<8> m_keyboard;

	// Set ROM bank
	devcb_write_line m_set_rom_bank;

	// Keyboard row
	int m_key_row;

	// Hexbus outgoing signal latch. Should be set to D7 when idle.
	// (see hexbus.cpp)
	uint8_t m_latch_out;

	// Hexbus incoming signal latch. Should be set to D7 when idle.
	uint8_t m_latch_in;

	// Hexbus inhibit. This prevents the incoming latches to store the data.
	bool m_communication_disable;

	// Bit 6. It is not documented, but likely to indicate the response phase.
	bool m_response_phase;
};


/* Variant for TI-99/2 24K */
class io992_24_device : public io992_device
{
public:
	io992_24_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

/* Variant for TI-99/2 32K */
class io992_32_device : public io992_device
{
public:
	io992_32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

} } } // end namespace bus::ti99::internal

DECLARE_DEVICE_TYPE_NS(VIDEO99224, bus::ti99::internal, video992_24_device)
DECLARE_DEVICE_TYPE_NS(VIDEO99232, bus::ti99::internal, video992_32_device)
DECLARE_DEVICE_TYPE_NS(IO99224, bus::ti99::internal, io992_24_device)
DECLARE_DEVICE_TYPE_NS(IO99232, bus::ti99::internal, io992_32_device)

#endif // MAME_BUS_TI99_INTERNAL_992BOARD_H
