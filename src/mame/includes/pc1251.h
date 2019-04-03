// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/pc1251.h
 *
 * Pocket Computer 1251
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_PC1251_H
#define MAME_INCLUDES_PC1251_H

#include "pocketc.h"

class pc1251_state : public pocketc_state
{
public:
	pc1251_state(const machine_config &mconfig, device_type type, const char *tag)
		: pocketc_state(mconfig, type, tag)
		, m_keys(*this, "KEY%u", 0U)
		, m_mode(*this, "MODE")
	{ }

	void init_pc1251();

	void pc1255(machine_config &config);
	void pc1251(machine_config &config);
	void pc1250(machine_config &config);

protected:
	virtual void machine_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pc1250_mem(address_map &map);
	void pc1251_mem(address_map &map);
	void pc1255_mem(address_map &map);
	void pc1260_mem(address_map &map);
	void pc1261_mem(address_map &map);

	DECLARE_WRITE8_MEMBER(out_b_w);
	DECLARE_WRITE8_MEMBER(out_c_w);

	DECLARE_READ_LINE_MEMBER(reset_r);
	DECLARE_READ8_MEMBER(in_a_r);
	DECLARE_READ8_MEMBER(in_b_r);
	DECLARE_READ8_MEMBER(lcd_read);
	DECLARE_WRITE8_MEMBER(lcd_write);

private:
	required_ioport_array<10> m_keys;
	required_ioport m_mode;

	uint8_t m_reg[0x100];

	static const char *const s_def[5];
	static const char *const s_shift[5];
	static const char *const s_de[5];
	static const char *const s_g[5];
	static const char *const s_rad[5];
	static const char *const s_run[5];
	static const char *const s_pro[5];
	static const char *const s_rsv[5];
};

class pc1260_state : public pc1251_state
{
public:
	pc1260_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc1251_state(mconfig, type, tag)
	{ }

	void pc1260(machine_config &config);
	void pc1261(machine_config &config);

protected:
	virtual void machine_start() override;
};

#endif // MAME_INCLUDES_PC1251_H
