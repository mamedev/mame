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

class pc1403_state : public pocketc_state
{
public:
	pc1403_state(const machine_config &mconfig, device_type type, const char *tag)
		: pocketc_state(mconfig, type, tag)
		, m_keys(*this, "KEY%u", 0U)
	{ }

	void pc1403(machine_config &config);
	void pc1403h(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pc1403_mem(address_map &map);
	void pc1403h_mem(address_map &map);
	void pc1421_readmem(address_map &map);
	void pc1421_writemem(address_map &map);

	DECLARE_READ_LINE_MEMBER(reset_r);
	DECLARE_WRITE8_MEMBER(out_c_w);
	DECLARE_READ8_MEMBER(in_a_r);

	DECLARE_READ8_MEMBER(asic_read);
	DECLARE_WRITE8_MEMBER(asic_write);
	DECLARE_READ8_MEMBER(lcd_read);
	DECLARE_WRITE8_MEMBER(lcd_write);

	int m_down;
	int m_right;

private:
	required_ioport_array<14> m_keys;

	uint8_t m_portc;
	uint8_t m_asic[4];
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
	static const char* const s_kana[5];
	static const char* const s_shoo[5];
	static const char* const s_sml[5];
};

class pc1403h_state : public pc1403_state
{
public:
	pc1403h_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc1403_state(mconfig, type, tag)
	{ }

protected:
	virtual void video_start() override;
};

#endif // MAME_INCLUDES_PC1403_H
