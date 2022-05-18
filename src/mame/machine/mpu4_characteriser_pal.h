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

	// while most games use unique keys and lamp scrambles, a lot use the same 'tri98' table, and this lamp scramble
	// these can be identified as games expecting a chr response starting with '00 84 94 3c ec 5c ec 50 2c 68 60 ac'
	static constexpr uint8_t m4dtri98_lamp_scramble[8] = { 0x03, 0xAF, 0x87, 0xAB, 0xA3, 0x8F, 0x87, 0x83 };


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
