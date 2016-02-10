// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/mikro80.h
 *
 ****************************************************************************/

#ifndef MIKRO80_H_
#define MIKRO80_H_

#include "machine/i8255.h"
#include "imagedev/cassette.h"

class mikro80_state : public driver_device
{
public:
	enum
	{
		TIMER_RESET
	};

	mikro80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cursor_ram(*this, "cursor_ram"),
		m_video_ram(*this, "video_ram"),
		m_ppi8255(*this, "ppi8255"),
		m_cassette(*this, "cassette"),
		m_region_maincpu(*this, "maincpu"),
		m_region_gfx1(*this, "gfx1"),
		m_bank1(*this, "bank1"),
		m_io_line0(*this, "LINE0"),
		m_io_line1(*this, "LINE1"),
		m_io_line2(*this, "LINE2"),
		m_io_line3(*this, "LINE3"),
		m_io_line4(*this, "LINE4"),
		m_io_line5(*this, "LINE5"),
		m_io_line6(*this, "LINE6"),
		m_io_line7(*this, "LINE7"),
		m_io_line8(*this, "LINE8") ,
		m_maincpu(*this, "maincpu") { }

	required_shared_ptr<UINT8> m_cursor_ram;
	required_shared_ptr<UINT8> m_video_ram;
	int m_keyboard_mask;
	int m_key_mask;
	DECLARE_READ8_MEMBER(mikro80_8255_portb_r);
	DECLARE_READ8_MEMBER(mikro80_8255_portc_r);
	DECLARE_WRITE8_MEMBER(mikro80_8255_porta_w);
	DECLARE_WRITE8_MEMBER(mikro80_8255_portc_w);
	DECLARE_READ8_MEMBER(mikro80_keyboard_r);
	DECLARE_WRITE8_MEMBER(mikro80_keyboard_w);
	DECLARE_WRITE8_MEMBER(mikro80_tape_w);
	DECLARE_READ8_MEMBER(mikro80_tape_r);
	DECLARE_DRIVER_INIT(radio99);
	DECLARE_DRIVER_INIT(mikro80);
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_mikro80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	required_device<i8255_device> m_ppi8255;
	required_device<cassette_image_device> m_cassette;
	required_memory_region m_region_maincpu;
	required_memory_region m_region_gfx1;
	required_memory_bank m_bank1;
	required_ioport m_io_line0;
	required_ioport m_io_line1;
	required_ioport m_io_line2;
	required_ioport m_io_line3;
	required_ioport m_io_line4;
	required_ioport m_io_line5;
	required_ioport m_io_line6;
	required_ioport m_io_line7;
	required_ioport m_io_line8;
	required_device<cpu_device> m_maincpu;
};

#endif /* UT88_H_ */
