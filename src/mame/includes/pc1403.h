// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/pc1403.h
 *
 * Pocket Computer 1403
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_PC1403_H
#define MAME_INCLUDES_PC1403_H

#include "pocketc.h"
#include "cpu/sc61860/sc61860.h"
#include "machine/nvram.h"

#define CONTRAST (ioport("DSW0")->read() & 0x07)


class pc1403_state : public pocketc_state
{
public:
	enum
	{
		TIMER_POWER_UP
	};

	pc1403_state(const machine_config &mconfig, device_type type, const char *tag)
		: pocketc_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	uint8_t m_portc;
	uint8_t m_outa;
	int m_power;
	uint8_t m_asic[4];
	int m_DOWN;
	int m_RIGHT;
	uint8_t m_reg[0x100];

	DECLARE_DRIVER_INIT(pc1403);
	uint32_t screen_update_pc1403(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_READ_LINE_MEMBER(pc1403_reset);
	DECLARE_READ_LINE_MEMBER(pc1403_brk);
	DECLARE_WRITE8_MEMBER(pc1403_outa);
	DECLARE_WRITE8_MEMBER(pc1403_outc);
	DECLARE_READ8_MEMBER(pc1403_ina);

	DECLARE_READ8_MEMBER(pc1403_asic_read);
	DECLARE_WRITE8_MEMBER(pc1403_asic_write);
	DECLARE_READ8_MEMBER(pc1403_lcd_read);
	DECLARE_WRITE8_MEMBER(pc1403_lcd_write);
	virtual void video_start() override;
	virtual void machine_start() override;
	required_device<sc61860_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void pc1403h(machine_config &config);
	void pc1403(machine_config &config);
	void pc1403_mem(address_map &map);
	void pc1403h_mem(address_map &map);
	void pc1421_readmem(address_map &map);
	void pc1421_writemem(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif // MAME_INCLUDES_PC1403_H
