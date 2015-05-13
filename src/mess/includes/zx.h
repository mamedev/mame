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
	enum
	{
		TIMER_TAPE_PULSE,
		TIMER_ULA_NMI,
		TIMER_ULA_IRQ
	};

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

	bitmap_ind16 m_bitmap;

	DECLARE_READ8_MEMBER(zx_ram_r);
	DECLARE_READ8_MEMBER(zx80_io_r);
	DECLARE_READ8_MEMBER(zx81_io_r);
	DECLARE_READ8_MEMBER(pc8300_io_r);
	DECLARE_READ8_MEMBER(pow3000_io_r);
	DECLARE_WRITE8_MEMBER(zx80_io_w);
	DECLARE_WRITE8_MEMBER(zx81_io_w);
	emu_timer *m_ula_nmi;
	int m_ula_irq_active;
	int m_ula_frame_vsync;
	int m_ula_scanline_count;
	UINT8 m_tape_bit;
	UINT8 m_speaker_state;
	int m_old_x;
	int m_old_y;
	UINT8 m_old_c;
	UINT8 m_charline[32];
	UINT8 m_charline_ptr;
	int m_offs1;
	void zx_ula_bkgnd(UINT8 color);
	DECLARE_WRITE8_MEMBER(zx_ram_w);
	DECLARE_DIRECT_UPDATE_MEMBER(zx_setdirect);
	DECLARE_DIRECT_UPDATE_MEMBER(pc8300_setdirect);
	DECLARE_DIRECT_UPDATE_MEMBER(pow3000_setdirect);
	DECLARE_DRIVER_INIT(zx);
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(zx);
	DECLARE_PALETTE_INIT(ts1000);
	DECLARE_MACHINE_RESET(pc8300);
	DECLARE_MACHINE_RESET(pow3000);
	void screen_eof_zx(screen_device &screen, bool state);
	TIMER_CALLBACK_MEMBER(zx_tape_pulse);
	TIMER_CALLBACK_MEMBER(zx_ula_nmi);
	TIMER_CALLBACK_MEMBER(zx_ula_irq);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
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

	void zx_ula_r(int offs, memory_region *region, const UINT8 param);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};

#endif /* ZX_H_ */
