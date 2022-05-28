// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_MPU4_CHARACTERISER_PAL_H
#define MAME_MACHINE_MPU4_CHARACTERISER_PAL_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"

DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_PAL, mpu4_characteriser_pal)

DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_PAL_BWB, mpu4_characteriser_pal_bwb)



class mpu4_characteriser_pal : public device_t
{
public:
	// construction/destruction
	mpu4_characteriser_pal(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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

	void set_allow_68k_cheat(bool allow)
	{
		m_allow_68k_cheat = allow;
	}

	void set_use_4k_table_sim(bool largetable)
	{
		m_is_4ksim = largetable;
	}

	virtual uint8_t read(offs_t offset);
	virtual void write(offs_t offset, uint8_t data);

	/* While some games use unique keys and lamp scrambles, several do write the same sequencesand expect the
	   same responses.  It is possible PALs were reused.  Sometimes the lamp tables are masked subsets, as
	   they were handcrafted when the layouts were made, they could also be incorrect in places.

	   The code checking the responses always masks with 0xfc, so the real responses from the devices could
	   have the lowest 2 bits set depending on the device state, but this is ignored.

	   Likewise the code to read the lamps typically masks out bits, so presumably the lamp scrambles for
	   some PAL types have been worked out from tests on real hardware?

	*/

	// these can be identified as games expecting a chr response starting with '00 84 94 3c ec 5c ec 50 2c 68 60 ac'
	static constexpr uint8_t m4dtri98_lamp_scramble[8] = { 0x03, 0xAF, 0x87, 0xAB, 0xA3, 0x8F, 0x87, 0x83 };

	// m4supst__bi etc.
	static constexpr uint8_t m4lv_lamp_scramble[9] = { 0x00, 0x18, 0x10, 0x08 ,0x00, 0x08, 0x00, 0x00 };
	                                              //     03    EB    63    CB    C3    6B    63    43    in sunsetb (complete table?)
    
	// games with sequence starting 00 24 24 2C E0 B4 B8 4C E8 D8 (m4eaw__a9 etc.)
	static constexpr uint8_t m683_lamp_scramble[8] = { 0x03, 0xAF, 0x27, 0x8F, 0x0F, 0xA7, 0x27, 0x07 };

	// games with sequence starting 00 44 44 4c e0 d4 d8 2c e8 b8 (m4overmn)
	static constexpr uint8_t otm_lamp_scramble[8] = { 0x00, 0x4C, 0x44, 0x04, 0x38, 0x74, 0x74, 0x20 };

	// games with sequence starting 00 30 20 14 2c a0 54 24 3c 9c 9c 9c
	static constexpr uint8_t m441_lamp_scramble[8] = { 0x03, 0x3F, 0x33, 0x1F, 0x17, 0x3B, 0x33, 0x13 };
	                                               //    00    38    30    18    14    3C    34    14
                                                   //    00    38    30    18    10    38    30    14

	// games with sequence starting 00 c4 c4 44 c4 44 44 c4 cc 3c
	// this seems to be the same sequence as the MPU4 Video game 'Strike It Lucky' where we're having to use a 4k table for the question scramble
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


	// games with sequence starting 00 24 24 2c 70 20 0c 60 3c 5c 5c 5c 7c 4c 68
	static constexpr uint8_t duty_lamp_scramble[8] = { 0x00, 0x38, 0x24, 0x18, 0x08, 0x34, 0x20, 0x00 };

	// games with sequence starting 00 84 94 5c ec 3c ec 30 4c 68 60 cc
	static constexpr uint8_t andybt_lamp_scramble[8] = { 0x00, 0x48, 0x00, 0x48, 0x44, 0x08, 0x00, 0x00 };

	// games with sequence starting 00 50 40 90 a8 6c c4 30 c8
	static constexpr uint8_t alf_lamp_scramble[8] = { 0x00, 0x58, 0x50, 0x1C, 0x10, 0x58, 0x50, 0x10 };
	                                              //    03    DB    53    9B    93    5B    53    13

	// games with sequence starting 00 84 A4 AC 70 80 2C C0 BC 5C
	static constexpr uint8_t shuffle_lamp_scramble[8] = { 0x00, 0x18, 0x00, 0x18, 0x08, 0x10, 0x00, 0x00 };

	// games with sequence starting 00 44 44 54 34 04 54 14 34 14 20 74 04 60
	static constexpr uint8_t clbveg_lamp_scramble[8] = { 0x00, 0x70, 0x40, 0x70, 0x50, 0x60, 0x40, 0x40 };

	// this sequence is used for m4oldtmr and m4tic__l, but m4oldtmr does not appear to use the lamp scramble, and the data is not valid for m4tic__l
	// so is probably incorrect
	// games with sequence starting 00 90 C0 54 A4 F0 64 90 E4 D4 60 B4
	static constexpr uint8_t m470_lamp_scramble[8] = { 0xFF, 0xFF, 0x10, 0x3F, 0x15, 0xFF, 0xFF, 0xFF }; // one 'm470' set was set to all blank?

	// games with sequence starting  00 84 C4 E4 4C 10 28 90 E8 78 34
	static constexpr uint8_t vivlv_lamp_scramble[8] = { 0x00, 0x28, 0x00, 0x28, 0x20, 0x08, 0x00, 0x00 };

	// games with sequence starting 00 c0 e0 b0 38 c4 f0 30 58 9c 9c 9c dc 9c dc
	static constexpr uint8_t tentendia_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; 	// INCORRECT 

	// games with sequence starting 00 90 88 4c e0 b8 74 84 bc 74 00 b4
	static constexpr uint8_t jewelcrown_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; 	// INCORRECT 

	// games with sequence starting 00 90 a0 34 8c 68 44 90 ac 6c 44 9c dc 5c d4 24 98 dc
	static constexpr uint8_t magicdragon_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; 	// INCORRECT 

protected:
	mpu4_characteriser_pal(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	uint8_t* m_current_chr_table;

private:

	void protection_w(uint8_t data);
	void lamp_scramble_w(uint8_t data);
	uint8_t protection_r();
	uint8_t lamp_scramble_r();


	optional_device<cpu_device> m_cpu; // needed for some of the protection 'cheats'

	bool m_allow_6809_cheat;
	bool m_allow_68k_cheat;

	const uint8_t* m_current_lamp_table;
	int m_prot_col;
	int m_lamp_col;
	int m_4krow;
	bool m_is_4ksim;

	optional_region_ptr<uint8_t> m_protregion; // some of the simulations have a fake ROM to assist them

	static constexpr bool IDENTIFICATION_HELPER = true;
	int m_temp_debug_write_count;
	uint8_t m_temp_debug_table[64];
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

class mpu4_characteriser_pal_bwb : public mpu4_characteriser_pal
{
public:
	// construction/destruction
	mpu4_characteriser_pal_bwb(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

	constexpr static uint8_t bwb_chr_table_common[10] = {0x00,0x04,0x04,0x0c,0x0c,0x1c,0x14,0x2c,0x5c,0x2c};

	int m_chr_state = 0;
	int m_chr_counter = 0;
	int m_chr_value = 0;
	int m_bwb_return = 0;
	int m_init_col = 0;

	uint8_t* m_bwb_chr_table1;

};

#endif // MAME_MACHINE_MPU4_CHARACTERISER_PAL_H
