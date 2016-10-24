// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/pc1403.h
 *
 * Pocket Computer 1403
 *
 ****************************************************************************/

#ifndef PC1403_H_
#define PC1403_H_

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

	void init_pc1403();
	uint32_t screen_update_pc1403(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int pc1403_reset();
	int pc1403_brk();
	void pc1403_outa(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pc1403_outc(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pc1403_ina(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	uint8_t pc1403_asic_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pc1403_asic_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pc1403_lcd_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pc1403_lcd_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void video_start() override;
	virtual void machine_start() override;
	required_device<sc61860_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif /* PC1403_H_ */
