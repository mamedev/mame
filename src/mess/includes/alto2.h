/*****************************************************************************
 *
 * includes/alto2.h
 *
 ****************************************************************************/

#ifndef _INCLUDES_ALTO2_H_
#define _INCLUDES_ALTO2_H_

#include "emu.h"
#include "cpu/alto2/alto2.h"

class alto2_state : public driver_device
{
public:
	alto2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_io_row0(*this, "ROW0"),
		m_io_row1(*this, "ROW1"),
		m_io_row2(*this, "ROW2"),
		m_io_row3(*this, "ROW3"),
		m_io_row4(*this, "ROW4"),
		m_io_row5(*this, "ROW5"),
		m_io_row6(*this, "ROW6"),
		m_io_row7(*this, "ROW7"),
#if	0	// FIXME: write a harddisk_image_device like device_t for the DIABLO31
		m_disk0(*this, "disk0"),
		m_disk1(*this, "disk1"),
#endif
		m_io_config(*this, "CONFIG")
		{ }

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	bitmap_ind16 m_bitmap;

	DECLARE_READ16_MEMBER(alto2_ram_r);
	DECLARE_WRITE16_MEMBER(alto2_ram_w);
	DECLARE_DRIVER_INIT(alto2);

	virtual void palette_init();
	DECLARE_MACHINE_RESET(alto2);
	void screen_eof_alto2(screen_device &screen, bool state);

protected:
	required_device<cpu_device> m_maincpu;
	required_ioport m_io_row0;
	required_ioport m_io_row1;
	required_ioport m_io_row2;
	required_ioport m_io_row3;
	required_ioport m_io_row4;
	required_ioport m_io_row5;
	required_ioport m_io_row6;
	required_ioport m_io_row7;
#if	0	// FIXME: write a harddisk_image_device like device_t for the DIABLO31
	required_device<diablo_device> m_disk0;
	optional_device<diablo_device> m_disk1;
#endif
	optional_ioport m_io_config;

	// FIXME: use device timers instead of individual emu_timer* in alto2 code(?)
//	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};

#endif /* _INCLUDES_ALTO2_H_ */
