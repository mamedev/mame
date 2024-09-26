// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/pc1401.h
 *
 * Pocket Computer 1401
 *
 ****************************************************************************/

#ifndef MAME_SHARP_PC1401_H
#define MAME_SHARP_PC1401_H

#include "pocketc.h"

class pc1401_state : public pocketc_state
{
public:
	pc1401_state(const machine_config &mconfig, device_type type, const char *tag)
		: pocketc_state(mconfig, type, tag)
		, m_keys(*this, "KEY%u", 0U)
	{
		std::fill(std::begin(m_reg), std::end(m_reg), 0);
	}

	void init_pc1401();

	void pc1401(machine_config &config);
	void pc1402(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pc1401_mem(address_map &map) ATTR_COLD;
	void pc1402_mem(address_map &map) ATTR_COLD;

	int reset_r();
	void out_b_w(uint8_t data);
	void out_c_w(uint8_t data);
	uint8_t in_a_r();
	uint8_t in_b_r();
	uint8_t lcd_read(offs_t offset);
	void lcd_write(offs_t offset, uint8_t data);

private:
	required_ioport_array<13> m_keys;

	uint8_t m_portc = 0;
	uint8_t m_reg[0x100];

	static const char* const s_line[5];
	static const char* const s_busy[5];
	static const char* const s_def[5];
	static const char* const s_shift[5];
	static const char* const s_hyp[5];
	static const char* const s_de[5];
	static const char* const s_g[5];
	static const char* const s_rad[5];
	static const char* const s_braces[5];
	static const char* const s_m[5];
	static const char* const s_e[5];
};

#endif // MAME_SHARP_PC1401_H
