// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_MPU4_CHARACTERISER_PAL_BWB_H
#define MAME_MACHINE_MPU4_CHARACTERISER_PAL_BWB_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"

DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_PAL_BWB, mpu4_characteriser_pal_bwb)

class mpu4_characteriser_pal_bwb : public device_t
{
public:
	// construction/destruction
	mpu4_characteriser_pal_bwb(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu_tag(T &&tag)
	{
		m_cpu.set_tag(std::forward<T>(tag));
	}

	void set_allow_6809_cheat(bool allow)
	{
		m_allow_6809_cheat = allow;
	}

	void set_allow_68k_cheat(bool allow)
	{
		m_allow_68k_cheat = allow;
	}

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

	virtual void device_start() override;
	virtual void device_reset() override;

private:

	optional_device<cpu_device> m_cpu; // needed for some of the protection 'cheats'

	bool m_allow_6809_cheat;
	bool m_allow_68k_cheat;


	optional_region_ptr<uint8_t> m_protregion; // some of the simulations have a fake ROM to assist them

	int m_chr_counter = 0;
	int m_chr_value = 0;
	int m_bwb_return = 0;
	uint8_t m_call = 0;
	bool m_initval_ready = false;
	uint8_t m_commonkey = 0x00;
	uint32_t m_otherkey = 0x00000000;
};



#if 0
static const bwb_chr_table prizeinv_data1[5] = {
//This is all wrong, but without BWB Vid booting,
//I can't find the right values. These should be close though
	{0x67},{0x17},{0x0f},{0x24},{0x3c},
};
#endif

#if 0// TODOxx:
static mpu4_chr_table prizeinv_data[8] = {
{0xEF, 0x02},{0x81, 0x00},{0xCE, 0x00},{0x00, 0x2e},
{0x06, 0x20},{0xC6, 0x0f},{0xF8, 0x24},{0x8E, 0x3c},
};
#endif

#endif // MAME_MACHINE_MPU4_CHARACTERISER_PAL_BWB_H
