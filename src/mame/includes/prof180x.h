// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __PROF180X__
#define __PROF180X__

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
		: driver_device(mconfig, type, tag),
		m_centronics(*this, CENTRONICS_TAG)
	{
	}

	required_device<centronics_device> m_centronics;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE8_MEMBER( flr_w );
	DECLARE_READ8_MEMBER( status0_r );
	DECLARE_READ8_MEMBER( status1_r );
	DECLARE_READ8_MEMBER( status_r );

	void ls259_w(int flag, int value);

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	int m_c0;
	int m_c1;
	int m_c2;
	int m_mm0;
	int m_mm1;
};

#endif
