// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_INCLUDES_PROF180X_H
#define MAME_INCLUDES_PROF180X_H

#pragma once

#include "bus/centronics/ctronics.h"

#define HD64180_TAG             "hd64180"
#define FDC9268_TAG             "fdc9268"
#define FDC9229_TAG             "fdc9229"
#define MK3835_TAG              "mk3835"
#define SCREEN_TAG              "screen"
#define CENTRONICS_TAG          "centronics"

class prof180x_state : public driver_device
{
public:
	prof180x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_centronics(*this, CENTRONICS_TAG)
	{
	}

	void prof180x(machine_config &config);

private:
	required_device<centronics_device> m_centronics;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t status0_r();
	uint8_t status1_r();
	uint8_t status_r(offs_t offset);

	DECLARE_WRITE_LINE_MEMBER(c0_flag_w);
	DECLARE_WRITE_LINE_MEMBER(c1_flag_w);
	DECLARE_WRITE_LINE_MEMBER(c2_flag_w);
	DECLARE_WRITE_LINE_MEMBER(mini_flag_w);
	DECLARE_WRITE_LINE_MEMBER(mm0_flag_w);
	DECLARE_WRITE_LINE_MEMBER(rtc_ce_w);
	DECLARE_WRITE_LINE_MEMBER(peps_flag_w);
	DECLARE_WRITE_LINE_MEMBER(mm1_flag_w);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	int m_c0 = 0;
	int m_c1 = 0;
	int m_c2 = 0;
	int m_mm0 = 0;
	int m_mm1 = 0;
	void prof180x_io(address_map &map);
	void prof180x_mem(address_map &map);
};

#endif // MAME_INCLUDES_PROF180X_H
