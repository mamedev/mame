// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/pc1350.h
 *
 * Pocket Computer 1350
 *
 ****************************************************************************/

#ifndef MAME_SHARP_PC1350_H
#define MAME_SHARP_PC1350_H

#include "pocketc.h"
#include "machine/ram.h"

class pc1350_state : public pocketc_state
{
public:
	pc1350_state(const machine_config &mconfig, device_type type, const char *tag)
		: pocketc_state(mconfig, type, tag)
		, m_ram(*this, RAM_TAG)
		, m_keys(*this, "KEY%u", 0U)
	{
		std::fill(std::begin(m_reg), std::end(m_reg), 0);
	}

	void pc1350(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pc1350_mem(address_map &map) ATTR_COLD;

	void out_b_w(uint8_t data);
	void out_c_w(uint8_t data);

	uint8_t in_a_r();
	uint8_t in_b_r();
	uint8_t lcd_read(offs_t offset);
	void lcd_write(offs_t offset, uint8_t data);
	uint8_t keyboard_line_r();

private:
	required_device<ram_device> m_ram;
	required_ioport_array<12> m_keys;

	uint8_t m_reg[0x1000]{};

	static const char* const s_def[5];
	static const char* const s_shift[5];
	static const char* const s_run[5];
	static const char* const s_pro[5];
	static const char* const s_japan[5];
	static const char* const s_sml[5];
};

#endif // MAME_SHARP_PC1350_H
