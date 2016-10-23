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

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void flr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t status0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t status1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void ls259_w(int flag, int value);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	int m_c0;
	int m_c1;
	int m_c2;
	int m_mm0;
	int m_mm1;
};

#endif
