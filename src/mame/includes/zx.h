// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Krzysztof Strzecha, Robbbert
/*****************************************************************************
 *
 * includes/zx.h
 *
 ****************************************************************************/

#ifndef ZX_H_
#define ZX_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/speaker.h"
#include "sound/wave.h"
#include "imagedev/cassette.h"
#include "formats/zx81_p.h"
#include "machine/ram.h"


class zx_state : public driver_device
{
public:
	zx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_cassette(*this, "cassette"),
		m_speaker(*this, "speaker"),
		m_region_maincpu(*this, "maincpu"),
		m_region_gfx1(*this, "gfx1"),
		m_io_row0(*this, "ROW0"),
		m_io_row1(*this, "ROW1"),
		m_io_row2(*this, "ROW2"),
		m_io_row3(*this, "ROW3"),
		m_io_row4(*this, "ROW4"),
		m_io_row5(*this, "ROW5"),
		m_io_row6(*this, "ROW6"),
		m_io_row7(*this, "ROW7"),
		m_io_config(*this, "CONFIG"),
		m_screen(*this, "screen") { }

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(ula_high_r);
	DECLARE_READ8_MEMBER(ula_low_r);
	DECLARE_WRITE16_MEMBER(refresh_w);
	DECLARE_READ8_MEMBER(zx80_io_r);
	DECLARE_READ8_MEMBER(zx81_io_r);
	DECLARE_READ8_MEMBER(pc8300_io_r);
	DECLARE_READ8_MEMBER(pow3000_io_r);
	DECLARE_WRITE8_MEMBER(zx80_io_w);
	DECLARE_WRITE8_MEMBER(zx81_io_w);
	DECLARE_DRIVER_INIT(zx);
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(zx);
	DECLARE_PALETTE_INIT(ts1000);
	void zx_tape_input();
	void zx_ula_hsync();

	UINT32 get_ram_size();

protected:
	enum
	{
		TIMER_TAPE_INPUT,
		TIMER_ULA_HSYNC
	};

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<cassette_image_device> m_cassette;
	optional_device<speaker_sound_device> m_speaker;
	required_memory_region m_region_maincpu;
	optional_memory_region m_region_gfx1;
	required_ioport m_io_row0;
	required_ioport m_io_row1;
	required_ioport m_io_row2;
	required_ioport m_io_row3;
	required_ioport m_io_row4;
	required_ioport m_io_row5;
	required_ioport m_io_row6;
	required_ioport m_io_row7;
	optional_ioport m_io_config;
	required_device<screen_device> m_screen;

	address_space *m_program;
	emu_timer *m_tape_input, *m_ula_hsync;

	bool m_vsync_active, m_hsync_active, m_nmi_on, m_nmi_generator_active;
	UINT64 m_base_vsync_clock, m_vsync_start_time;
	UINT32 m_ypos;

	UINT8 m_prev_refresh;
	UINT8 m_speaker_state;

	bitmap_ind16 *m_bitmap_render, *m_bitmap_buffer;

	UINT16 m_ula_char_buffer;
	double m_cassette_cur_level;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void drop_sync();
	void recalc_hsync();
};

#endif /* ZX_H_ */
