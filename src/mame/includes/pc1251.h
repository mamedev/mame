// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/pc1251.h
 *
 * Pocket Computer 1251
 *
 ****************************************************************************/

#ifndef PC1251_H_
#define PC1251_H_

#include "pocketc.h"
#include "cpu/sc61860/sc61860.h"
#include "machine/nvram.h"

#define PC1251_CONTRAST (ioport("DSW0")->read() & 0x07)


class pc1251_state : public pocketc_state
{
public:
	enum
	{
		TIMER_POWER_UP
	};

	pc1251_state(const machine_config &mconfig, device_type type, const char *tag)
		: pocketc_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	UINT8 m_outa;
	UINT8 m_outb;
	int m_power;
	UINT8 m_reg[0x100];

	DECLARE_DRIVER_INIT(pc1251);
	UINT32 screen_update_pc1251(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(pc1251_outa);
	DECLARE_WRITE8_MEMBER(pc1251_outb);
	DECLARE_WRITE8_MEMBER(pc1251_outc);

	DECLARE_READ_LINE_MEMBER(pc1251_reset);
	DECLARE_READ_LINE_MEMBER(pc1251_brk);
	DECLARE_READ8_MEMBER(pc1251_ina);
	DECLARE_READ8_MEMBER(pc1251_inb);
	DECLARE_READ8_MEMBER(pc1251_lcd_read);
	DECLARE_WRITE8_MEMBER(pc1251_lcd_write);
	virtual void machine_start() override;
	DECLARE_MACHINE_START(pc1260);
	required_device<sc61860_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif /* PC1251_H_ */
