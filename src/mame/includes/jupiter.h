// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Robbbert
#ifndef MAME_INCLUDES_JUPITER_H
#define MAME_INCLUDES_JUPITER_H

#pragma once

#define MCM6571AP_TAG   "vid125_6c"
#define S6820_TAG       "vid125_4a"
#define Z80_TAG         "cpu126_4c"
#define INS1771N1_TAG   "fdi027_4c"
#define MC6820P_TAG     "fdi027_4b"
#define MC6850P_TAG     "rsi068_6a"
#define MC6821P_TAG     "sdm058_4b"

class jupiter2_state : public driver_device
{
public:
	jupiter2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, MCM6571AP_TAG)
	{ }

	DECLARE_DRIVER_INIT(jupiter);

private:
	required_device<cpu_device> m_maincpu;

	virtual void machine_start() override;
};

class jupiter3_state : public driver_device
{
public:
	jupiter3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_p_videoram(*this, "videoram")
		, m_p_ram(*this, "ram")
		, m_p_chargen(*this, "chargen")
	{ }

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_DRIVER_INIT(jupiter3);
	void kbd_put(u8 data);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_READ8_MEMBER(key_r);
	DECLARE_READ8_MEMBER(ff_r);

private:
	virtual void machine_reset() override;

	uint8_t m_term_data;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_shared_ptr<uint8_t> m_p_ram;
	required_region_ptr<u8> m_p_chargen;
};

#endif // MAME_INCLUDES_JUPITER_H
