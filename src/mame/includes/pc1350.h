// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/pc1350.h
 *
 * Pocket Computer 1350
 *
 ****************************************************************************/

#ifndef PC1350_H_
#define PC1350_H_

#include "pocketc.h"
#include "cpu/sc61860/sc61860.h"
#include "machine/nvram.h"
#include "machine/ram.h"

#define PC1350_CONTRAST (ioport("DSW0")->read() & 0x07)


class pc1350_state : public pocketc_state
{
public:
	enum
	{
		TIMER_POWER_UP
	};

	pc1350_state(const machine_config &mconfig, device_type type, std::string tag)
		: pocketc_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG) { }

	UINT8 m_outa;
	UINT8 m_outb;
	int m_power;
	UINT8 m_reg[0x1000];
	UINT32 screen_update_pc1350(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER(pc1350_outa);
	DECLARE_WRITE8_MEMBER(pc1350_outb);
	DECLARE_WRITE8_MEMBER(pc1350_outc);

	DECLARE_READ_LINE_MEMBER(pc1350_brk);
	DECLARE_READ8_MEMBER(pc1350_ina);
	DECLARE_READ8_MEMBER(pc1350_inb);
	DECLARE_READ8_MEMBER(pc1350_lcd_read);
	DECLARE_WRITE8_MEMBER(pc1350_lcd_write);
	DECLARE_READ8_MEMBER(pc1350_keyboard_line_r);

	virtual void machine_start() override;
	required_device<sc61860_device> m_maincpu;
	required_device<ram_device> m_ram;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif /* PC1350_H_ */
