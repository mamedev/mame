// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/pc1350.h
 *
 * Pocket Computer 1350
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_PC1350_H
#define MAME_INCLUDES_PC1350_H

#include "pocketc.h"
#include "machine/ram.h"

class pc1350_state : public pocketc_state
{
public:
	pc1350_state(const machine_config &mconfig, device_type type, const char *tag)
		: pocketc_state(mconfig, type, tag)
		, m_ram(*this, RAM_TAG)
		, m_keys(*this, "KEY%u", 0U)
	{ }

	void pc1350(machine_config &config);

protected:
	virtual void machine_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pc1350_mem(address_map &map);

	DECLARE_WRITE8_MEMBER(out_b_w);
	DECLARE_WRITE8_MEMBER(out_c_w);

	DECLARE_READ8_MEMBER(in_a_r);
	DECLARE_READ8_MEMBER(in_b_r);
	DECLARE_READ8_MEMBER(lcd_read);
	DECLARE_WRITE8_MEMBER(lcd_write);
	DECLARE_READ8_MEMBER(keyboard_line_r);

private:
	required_device<ram_device> m_ram;
	required_ioport_array<12> m_keys;

	uint8_t m_reg[0x1000];

	static const char* const s_def[5];
	static const char* const s_shift[5];
	static const char* const s_run[5];
	static const char* const s_pro[5];
	static const char* const s_japan[5];
	static const char* const s_sml[5];
};

#endif // MAME_INCLUDES_PC1350_H
