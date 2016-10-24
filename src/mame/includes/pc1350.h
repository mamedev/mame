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

	pc1350_state(const machine_config &mconfig, device_type type, const char *tag)
		: pocketc_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG) { }

	uint8_t m_outa;
	uint8_t m_outb;
	int m_power;
	uint8_t m_reg[0x1000];
	uint32_t screen_update_pc1350(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pc1350_outa(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pc1350_outb(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pc1350_outc(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	int pc1350_brk();
	uint8_t pc1350_ina(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pc1350_inb(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pc1350_lcd_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pc1350_lcd_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pc1350_keyboard_line_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	virtual void machine_start() override;
	required_device<sc61860_device> m_maincpu;
	required_device<ram_device> m_ram;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif /* PC1350_H_ */
