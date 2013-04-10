/*****************************************************************************
 *
 * includes/pc1350.h
 *
 * Pocket Computer 1350
 *
 ****************************************************************************/

#ifndef PC1350_H_
#define PC1350_H_

#include "machine/nvram.h"

#define PC1350_CONTRAST (ioport("DSW0")->read() & 0x07)


class pc1350_state : public driver_device
{
public:
	pc1350_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	UINT8 m_outa;
	UINT8 m_outb;
	int m_power;
	UINT8 m_reg[0x1000];
	UINT32 screen_update_pc1350(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(pc1350_power_up);

	DECLARE_WRITE8_MEMBER(pc1350_outa);
	DECLARE_WRITE8_MEMBER(pc1350_outb);
	DECLARE_WRITE8_MEMBER(pc1350_outc);

	DECLARE_READ_LINE_MEMBER(pc1350_brk);
	DECLARE_READ8_MEMBER(pc1350_ina);
	DECLARE_READ8_MEMBER(pc1350_inb);
	DECLARE_READ8_MEMBER(pc1350_lcd_read);
	DECLARE_WRITE8_MEMBER(pc1350_lcd_write);
	DECLARE_READ8_MEMBER(pc1350_keyboard_line_r);

	virtual void machine_start();
	required_device<cpu_device> m_maincpu;
};

#endif /* PC1350_H_ */
