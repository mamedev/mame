// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Robbbert
#pragma once

#ifndef __JUPITER__
#define __JUPITER__

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
	jupiter2_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, MCM6571AP_TAG)
	{ }

	required_device<cpu_device> m_maincpu;

	virtual void machine_start() override;
	DECLARE_DRIVER_INIT(jupiter);
};

class jupiter3_state : public driver_device
{
public:
	jupiter3_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, Z80_TAG),
	m_p_videoram(*this, "p_videoram"),
	m_p_ram(*this, "p_ram")
	{ }

	required_device<cpu_device> m_maincpu;

	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_READ8_MEMBER(key_r);
	DECLARE_READ8_MEMBER(ff_r);
	UINT8 m_term_data;

	required_shared_ptr<UINT8> m_p_videoram;
	required_shared_ptr<UINT8> m_p_ram;
	const UINT8 *m_p_chargen;

	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_DRIVER_INIT(jupiter3);
};

#endif
