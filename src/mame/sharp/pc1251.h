// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/pc1251.h
 *
 * Pocket Computer 1251
 *
 ****************************************************************************/

#ifndef MAME_SHARP_PC1251_H
#define MAME_SHARP_PC1251_H

#include "pocketc.h"

class pc1251_state : public pocketc_state
{
public:
	pc1251_state(const machine_config &mconfig, device_type type, const char *tag)
		: pocketc_state(mconfig, type, tag)
		, m_keys(*this, "KEY%u", 0U)
		, m_mode(*this, "MODE")
	{
		std::fill(std::begin(m_reg), std::end(m_reg), 0);
	}

	void init_pc1251();

	void pc1255(machine_config &config);
	void pc1251(machine_config &config);
	void pc1250(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pc1250_mem(address_map &map) ATTR_COLD;
	void pc1251_mem(address_map &map) ATTR_COLD;
	void pc1255_mem(address_map &map) ATTR_COLD;
	void pc1260_mem(address_map &map) ATTR_COLD;
	void pc1261_mem(address_map &map) ATTR_COLD;

	void out_b_w(uint8_t data);
	void out_c_w(uint8_t data);

	int reset_r();
	uint8_t in_a_r();
	uint8_t in_b_r();
	uint8_t lcd_read(offs_t offset);
	void lcd_write(offs_t offset, uint8_t data);

private:
	required_ioport_array<10> m_keys;
	required_ioport m_mode;

	uint8_t m_reg[0x100]{};

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
	virtual void machine_start() override ATTR_COLD;
};

#endif // MAME_SHARP_PC1251_H
