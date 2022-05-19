// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_MPU4_CHARACTERISER_PAL_H
#define MAME_MACHINE_MPU4_CHARACTERISER_PAL_H

#pragma once

#include "cpu/m6809/m6809.h"

DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_PAL, mpu4_characteriser_pal)

DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_PAL_4KSIM, mpu4_characteriser_pal_4ksim)

struct bwb_chr_table//dynamically populated table for BwB protection
{
	uint8_t response = 0;
};

class mpu4_characteriser_pal : public device_t
{
public:
	// construction/destruction
	mpu4_characteriser_pal(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	mpu4_characteriser_pal(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu_tag(T &&tag)
	{
		m_cpu.set_tag(std::forward<T>(tag));
	}

	void set_lamp_table(const uint8_t* table)
	{
		m_current_lamp_table = table;
	}

	void set_character_table(uint8_t* table)
	{
		m_current_chr_table = table;
	}

	void set_allow_6809_cheat(bool allow)
	{
		m_allow_6809_cheat = allow;
	}

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	// While most games use unique keys and lamp scrambles, several do write the same sequences and expect the
	// same responses.  It is possible PALs were reused.  Sometimes the lamp tables are masked subsets, as
	// they were handcrafted when the layouts were made, they could also be incorrect in places.
	
	// these can be identified as games expecting a chr response starting with '00 84 94 3c ec 5c ec 50 2c 68 60 ac'
	static constexpr uint8_t m4dtri98_lamp_scramble[8] = { 0x03, 0xAF, 0x87, 0xAB, 0xA3, 0x8F, 0x87, 0x83 };

	// m4supst__bi etc.
	static constexpr uint8_t m4lv_lamp_scramble[9] = { 0x00, 0x18, 0x10, 0x08 ,0x00, 0x08, 0x00, 0x00 };

	// games with sequence starting 00 24 24 2C E0 B4 B8 4C E8 D8 (m4eaw__a9 etc.)
	static constexpr uint8_t m683_lamp_scramble[8] = { 0x03, 0xAF, 0x27, 0x8F, 0x0F, 0xA7, 0x27, 0x07 };

	// games with sequence starting 00 44 44 4c e0 d4 d8 2c e8 b8 (m4overmn)
	static constexpr uint8_t otm_lamp_scramble[8] = { 0x00, 0x4C, 0x44, 0x04, 0x38, 0x74, 0x74, 0x20 };

	// games with sequence starting 00 30 20 14 2c a0 54 24 3c 9c 9c 9c
	static constexpr uint8_t m441_lamp_scramble[8] = { 0x03, 0x3F, 0x33, 0x1F, 0x17, 0x3B, 0x33, 0x13 };
	                                               //    00    38    30    18    14    3C    34    14
                                                   //    00    38    30    18    10    38    30    14

   // games with sequence starting 00 c4 c4 44 c4 44 44 c4 cc 3c
	static constexpr uint8_t m462_lamp_scramble[8] = { 0x03, 0xC3, 0xC7, 0x4F, 0x47, 0xE7, 0xC7, 0x47 };
	                                               //    04    44    44    48    40    60    40    40   rhog2

	// games with sequence starting 00 64 64 24 64 64 24 64 6C 9C BC
	static constexpr uint8_t wta_lamp_scramble[8] = { 0x00, 0x64, 0x60, 0x28, 0x20, 0x70, 0x60, 0x20 };


	// games with sequence starting 00 24 24 a4 4c 10 88 50 a8 d8 9c
	static constexpr uint8_t du91_lamp_scramble[8] = { 0x03, 0xAF, 0x27, 0x8F, 0x87, 0x2F, 0x27, 0x07 };
	                                               //    00    28    20    08    00    28    20    00    gb006


	// games with sequence starting 00 90 C0 54 8C 68 24 90 CC 6C 24 9C BC 34 88 6C
	static constexpr uint8_t rr6_lamp_scramble[8] = { 0x00, 0x18, 0x10, 0x18, 0x10, 0x18, 0x10, 0x14 };

	// games with sequence starting 00 30 20 14 64 30 44 30 64 34 00 74 20
	static constexpr uint8_t rhm_lamp_scramble[8] = { 0x00, 0x70, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 14 04 94 A8 6C C4 30 8C E8 E0 BC D4
	static constexpr uint8_t m574_lamp_scramble[8] = { 0x03, 0x9F, 0x17, 0x9B, 0x93, 0x1F, 0x17, 0x13 };

	// games with sequence starting 00 a0 a8 18 f4 2c 70 60 e4 e8
	static constexpr uint8_t viva_lamp_scramble[8] = { 0x03, 0xE7, 0xA3, 0xC7, 0xC3, 0xA7, 0xA3, 0xC3 };
	                                                //   00    60    20    40    40    24    20    40   place your bets
	
	static constexpr uint8_t m407_lamp_scramble[8] = { 0x03, 0xC7, 0x83, 0xC3, 0xC3, 0xA3, 0x83, 0xC3 };

	

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:

	void protection_w(uint8_t data);
	void lamp_scramble_w(uint8_t data);
	uint8_t protection_r();
	uint8_t lamp_scramble_r();


	optional_device<cpu_device> m_cpu; // needed for some of the protection 'cheats'

	bool m_allow_6809_cheat = false;
	uint8_t* m_current_chr_table = nullptr;
	const uint8_t* m_current_lamp_table = nullptr;
	int m_prot_col = 0;
	int m_lamp_col = 0;

	optional_region_ptr<uint8_t> m_protregion; // some of the simulations have a fake ROM to assist them

	static constexpr bool IDENTIFICATION_HELPER = true;
	int m_temp_debug_write_count;
	uint8_t m_temp_debug_table[64];
};


class mpu4_characteriser_pal_4ksim : public mpu4_characteriser_pal
{
public:
	// construction/destruction
	mpu4_characteriser_pal_4ksim(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);
};

#endif // MAME_MACHINE_MPU4_CHARACTERISER_PAL_H
