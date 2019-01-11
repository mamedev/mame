// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/pc1401.h
 *
 * Pocket Computer 1401
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_PC1401_H
#define MAME_INCLUDES_PC1401_H

#include "pocketc.h"

class pc1401_state : public pocketc_state
{
public:
	pc1401_state(const machine_config &mconfig, device_type type, const char *tag)
		: pocketc_state(mconfig, type, tag)
		, m_keys(*this, "KEY%u", 0U)
	{ }

	void init_pc1401();

	void pc1401(machine_config &config);
	void pc1402(machine_config &config);

protected:
	virtual void machine_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pc1401_mem(address_map &map);
	void pc1402_mem(address_map &map);

	DECLARE_READ_LINE_MEMBER(reset_r);
	DECLARE_WRITE8_MEMBER(out_b_w);
	DECLARE_WRITE8_MEMBER(out_c_w);
	DECLARE_READ8_MEMBER(in_a_r);
	DECLARE_READ8_MEMBER(in_b_r);
	DECLARE_READ8_MEMBER(lcd_read);
	DECLARE_WRITE8_MEMBER(lcd_write);

private:
	required_ioport_array<13> m_keys;

	uint8_t m_portc;
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

#endif // MAME_INCLUDES_PC1401_H
