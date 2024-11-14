// license:BSD-3-Clause
// copyright-holders:David Haywood, James Wallace

#ifndef MAME_BARCREST_MPU4_CHARACTERISER_PAL_BWB_H
#define MAME_BARCREST_MPU4_CHARACTERISER_PAL_BWB_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"

DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_PAL_BWB, mpu4_characteriser_pal_bwb)

class mpu4_characteriser_pal_bwb : public device_t
{
public:
	// construction/destruction
	mpu4_characteriser_pal_bwb(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_common_key(uint8_t commonkey)
	{
		m_commonkey = commonkey;
	}

	void set_other_key(uint32_t otherkey)
	{
		m_otherkey = otherkey;
	}

	virtual uint8_t read(offs_t offset);
	virtual void write(offs_t offset, uint8_t data);

	constexpr static uint8_t bwb_chr_table_common[16] = {0x00,0x04,0x04,0x0c,0x0c,0x1c,0x14,0x2c,0x5c,0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

protected:
	mpu4_characteriser_pal_bwb(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	int m_chr_counter = 0;
	int m_chr_value = 0;
	int m_bwb_return = 0;
	uint8_t m_call = 0;
	bool m_initval_ready = false;
	uint8_t m_commonkey = 0x00;
	uint32_t m_otherkey = 0x00000000;
};

#endif // MAME_BARCREST_MPU4_CHARACTERISER_PAL_BWB_H
