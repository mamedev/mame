// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_MPU4_CHARACTERISER_PAL_H
#define MAME_MACHINE_MPU4_CHARACTERISER_PAL_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"

DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_PAL, mpu4_characteriser_pal)
DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_PAL_BWB, mpu4_characteriser_pal_bwb)
DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_BOOTLEG_PAL45, mpu4_characteriser_bootleg45)
DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_BOOTLEG_PAL51, mpu4_characteriser_bootleg51)
DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_BOOTLEG_PAL_BLASTBANK, mpu4_characteriser_bootleg_blastbank)
DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_BOOTLEG_PAL_COPCASH, mpu4_characteriser_bootleg_copcash)


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
	static constexpr uint8_t rhm_lamp_scramble[8] = { 0x00, 0x70, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00 }; // incomplete? doesn't seem correct for the lower rows of the roadhog set using it

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

	// games with sequence starting  00 84 C4 E4 4C 10 28 90 E8 78 34
	static constexpr uint8_t vivlv_lamp_scramble[8] = { 0x00, 0x28, 0x00, 0x28, 0x20, 0x08, 0x00, 0x00 };
	                                                //    00    28    00    28    24    14    00    00   m4jpjmp has this lamp scramble for it
													 
	// games with sequence starting  00 84 8c b8 74 80 1c b4 d8 74 00 d4 c8 78 a4 4c e0 dc f4 88
	static constexpr uint8_t celclb_lamp_scramble[8] = { 0x00, 0x50, 0x00, 0x50, 0x10, 0x40, 0x04, 0x00 };

	// games with sequence starting 00 14 10 C0 8C A8 68 30 D0 58 E4 DC F4
	static constexpr uint8_t cashmx_lamp_scramble[8] = { 0x04, 0x50, 0x10, 0x60, 0x60, 0x30, 0x30, 0x14 };

	// games with sequence 00 14 04 94 c8 68 a0 18 f4 8c e8 ec ac a8 6c 20 54 c4 dc
	static constexpr uint8_t viz_lamp_scramble[8] = { 0x00, 0x50, 0x10, 0x54, 0x14, 0x50, 0x10, 0x14 };

	// games with sequence 00 e4 ec f8 54 08 d0 80 44 2c 58 b4 e8 b0 80
	static constexpr uint8_t nifty_lamp_scramble[8] = { 0x03, 0xE7, 0xA7, 0x87, 0xE7, 0x07, 0xA7, 0xE7 };

	// games with sequence 00 84 a4 e4 b0 34 54 44 d4 64 80 f4 24 80 f4 20
	static constexpr uint8_t milclb_lamp_scramble[8] = { 0x00, 0x54, 0x00, 0x54, 0x40, 0x10, 0x00, 0x00 };

	// games with sequence 00 44 44 c4 58 60 c0 50 8c b8 e0 dc ec b0 1c e8 38
	static constexpr uint8_t fruitfall_lamp_scramble[8] = { 0x03, 0xCF, 0x47, 0xCB, 0xC3, 0x4F, 0x47, 0x43 };


	// games with sequence 00 60 68 bc d0 2c 94 20 e4 e8 bc f0 88 34 a0 c4 ec bc f4 
	static constexpr uint8_t m400_lamp_scramble[8] = { 0x03, 0xE7, 0x43 ,0xC3, 0xC3 ,0xE3, 0x43, 0xC3 }; // does anything using this have lamp scramble or was this extracted with tests?

	// games with sequence 00 bc b8 fc bc dc fc fc fc f8 d8 b8 f8 d8 fc bc fc 98 fc f8 f8
	static constexpr uint8_t intcep_lamp_scramble[8] = { 0x00, 0x1C, 0x38, 0x78, 0x7C, 0x78, 0x38, 0x7C };	

	// games with sequence 00 50 40 14 C4 B0 A4 30 C4 74 00 D4 E0 30 C0 34
	static constexpr uint8_t take2_lamp_scramble[8] = { 0x00, 0x50, 0x50, 0x10, 0x10, 0x50, 0x50, 0x00 };	

	// games with sequence 00 50 40 14 4C 80 34 44 5C 9C 9C 9C DC 9C DC 94
	static constexpr uint8_t m435_lamp_scramble[8] = { 0x03, 0x5F, 0x53, 0x1F, 0x17, 0x5B, 0x53, 0x13 };	

	// games with sequence 00 84 8C D8 74 80 4C 90 E8 78 54 60 84
	static constexpr uint8_t m578_lamp_scramble[8] = { 0x00, 0x60, 0x00, 0x60, 0x40, 0x20, 0x00, 0x00 };	

	// games with sequence 00 60 60 44 e0 e8 1c 74 a4 6c 14 84 e8 1c f4
	static constexpr uint8_t addr_lamp_scramble[8] = { 0x00, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00 }; // likely incomplete

	// games with sequence 00 c0 c8 1c f4 68 14 50 70 50 20 f0 48 34 60
	static constexpr uint8_t age_lamp_scramble[8] = { 0x00, 0x74, 0x44, 0x34, 0x14, 0x64, 0x44, 0x00 };

	// games with sequence starting 00 60 60 C0 58 44 E0 50 A8 9C CC BC E4 50 A0 58
	static constexpr uint8_t sunsetb_lamp_scramble[8] = { 0x03, 0xEB, 0x63, 0xCB, 0xC3, 0x6B, 0x63, 0x43 };
	
	// games with sequence starting 00 60 60 a0 38 64 e0 30 c8 9c ac dc ec 94 d8 a4 38 ec
	static constexpr uint8_t bjac_lamp_scramble[8] = { 0x00, 0x68, 0x60, 0x28, 0x20, 0x68, 0x60, 0x20 };

	// games with sequence starting 00 88 70 14 1c c0 a4 a0 bc d4 30 14 18 d4 2c 50 1c
	static constexpr uint8_t mag7s_lamp_scramble[8] = { 0x03, 0x9F, 0x0F, 0x17, 0x03, 0x1B, 0x8F, 0x87 };

	// games with sequence starting 00 44 44 54 d0 88 38 74 d0 58
	static constexpr uint8_t oad_lamp_scramble[8] = { 0x00, 0x50, 0x44, 0x14, 0x14, 0x44, 0x44, 0x00 };

	// games with sequence starting 00 18 70 24 38 58 74 0c 6c 64
	static constexpr uint8_t rhs_lamp_scramble[8] = { 0x00, 0x3C, 0x18, 0x30, 0x10, 0x3C, 0x18, 0x10 };

	// games with sequence starting 10 94 1c f4 b8 74 b4 98 f4 9c f0 b8 d4 38 74 10 (unusual sequence)
	static constexpr uint8_t rockmn_lamp_scramble[8] = { 0x10, 0x34, 0x14, 0x34, 0x30, 0x30, 0x14, 0x30 };

	// games with sequence starting 00 0c 50 90 b0 38 d4 a0 bc d4 30 90 38 c4 ac 70
	static constexpr uint8_t gambal_lamp_scramble[8] = { 0x00, 0x18, 0x08, 0x10, 0x00, 0x18, 0x08, 0x00 };


	/***************************************************************

	 Lamp data below is incorrect

	***************************************************************/


	// these lamp values were in the Twin Timer set, which is the only game using it, but they're not used, so probably incorrect
	static constexpr uint8_t m533_lamp_scramble[8] = { 0xFF, 0xFF, 0x10, 0x3F, 0x15, 0xFF, 0xFF, 0xFF };

	// this sequence is used for m4oldtmr and m4tic__l, but m4oldtmr does not appear to use the lamp scramble, and the data is not valid for m4tic__l
	// so is probably incorrect
	// games with sequence starting 00 90 C0 54 A4 F0 64 90 E4 D4 60 B4
	static constexpr uint8_t m470_lamp_scramble[8] = { 0xFF, 0xFF, 0x10, 0x3F, 0x15, 0xFF, 0xFF, 0xFF }; // one 'm470' set was set to all blank?

	// these lamp values were in the Golden Gate set, they do not appear to be valid as Golden Gate doesn't use them and other games don't work with them
	static constexpr uint8_t m450_lamp_scramble[8] = { 0xFF, 0xFF, 0x10, 0x3F, 0x15, 0xFF, 0xFF, 0xFF };


	// games with sequence starting 00 c0 e0 b0 38 c4 f0 30 58 9c 9c 9c dc 9c dc
	static constexpr uint8_t tentendia_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };   // INCORRECT

	// games with sequence starting 00 90 88 4c e0 b8 74 84 bc 74 00 b4
	static constexpr uint8_t jewelcrown_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  // INCORRECT

	// games with sequence starting 00 90 a0 34 8c 68 44 90 ac 6c 44 9c dc 5c d4 24 98 dc
	static constexpr uint8_t magicdragon_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 50 10 24 54 00 60 50 34 30 00 74 10 04 74
	static constexpr uint8_t premier_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 a0 88 38 94 2c 30 00 e4 c8 18 b4 4c 30
	static constexpr uint8_t squids_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 a0 88 38 94 2c 30 00 e4 c8 18 b4 4c 30
	static constexpr uint8_t graff_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 50 40 14 64 50 24 50 64 54 20 74 40 30 60 10 64
	static constexpr uint8_t buc_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 44 44 64 4c 80 70 24 6c a8 b0 38 e4
	static constexpr uint8_t pzmoney_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 14 10 60 54 00 24 14 70 30 00 74 10 40 34 40
	static constexpr uint8_t fortune_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 60 60 24 e0 e8 1c 74 c4 6c 14 84 e8 1c
	static constexpr uint8_t actionbank_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 90 88 2c e0 d8 74 84 dc 74 00 d4 c8 6c a0 58 f4 cc ec 68
	static constexpr uint8_t toplot_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 14 04 54 c4 98 f0 48 e4 5c f0 c8 ec 68 24
	static constexpr uint8_t luckystrike_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 18 70 44 58 30 44 18 7c 74 00 5c 7c 34 48 24 58
	static constexpr uint8_t tictak_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 24 24 64 2c 14 4c 14 4c 58 78 78 74
	static constexpr uint8_t actclba_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 14 10 c0 4c 20 84 0c f0 98 e4 dc f4 08 f0 08 70 d0
	static constexpr uint8_t phr_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 44 44 64 4c 10 28 50 68 38 34 28 70 00 6c 10 68
	static constexpr uint8_t cheryo_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 30 10 44 70 10 44 30 54 14 40 74 10 40 34
	static constexpr uint8_t cosmiccasino_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 84 8c 3c f4 4c 34 24 e4 ac 38 f0 0c 70 04 
	static constexpr uint8_t kingqn_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 c0 d0 38 ec 5c ec 14 68 2c 24 e8 74 00 e8 14
	static constexpr uint8_t turboplay_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 14 04 34 2c 44 34 24 3c 78 70 28 64
	static constexpr uint8_t sunsetclub_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 a0 b0 58 ec 3c ec 14 68 4c 4c 6c 64 80 f8 84 98
	static constexpr uint8_t topaction_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 18 c8 a4 0c 80 0c 90 34 30 00 58
	static constexpr uint8_t doublediamond_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 90 a0 70 c8 2c c4 30 c8 6c 44 d8 dc 5c
	static constexpr uint8_t ttt_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 44 44 c4 68 14 8c 30 8c b8 d0 a8
	static constexpr uint8_t cashencounters_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 8c 64 84 84 c4 84 84 9c f4 04 cc 24 84 c4 94 54 (NOTE, same sequence as v4addlad, which uses 4k table)
	static constexpr uint8_t bankrollerclub_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 60 60 44 68 a0 54 24 6c 8c 9c cc bc c4 74 00
	static constexpr uint8_t copycat_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 30 10 0c 58 60 24 30 1c 6c 44 3c 74 00
	static constexpr uint8_t thestreak_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 84 c4 d4 70 04 94 50 34 14 20 b4 44 a0 e4
	static constexpr uint8_t kqee2_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 18 b0 64 38 98 b4 44 3c b4 40 3c 9c b4
	static constexpr uint8_t kqee_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 14 10 48 38 34 58 74 58 6c 60 5c 7c 64 14
	static constexpr uint8_t pfloot_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting  00 50 40 30 68 44 70 60 78 3c 34 28 64 10
	static constexpr uint8_t berseralt_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 a0 e0 c4 c8 58 9c 94 6c 1c 9c 9c bc 94 6c 10
	static constexpr uint8_t berseralt2_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 24 24 a4 4c 10 c0 0c f0 a8 98
	static constexpr uint8_t bagtel_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 84 c4 d4 58 24 94 50 98 3c 34 18 bc 34 88 78 bc
	static constexpr uint8_t bucalt_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 84 a4 b4 38 c4 b4 30 1c d8 d8 d8 dc
	static constexpr uint8_t hotrodalt_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting  00 e0 88 18 b0 48 50 60 e4 c8 58 f0 08
	static constexpr uint8_t bdash_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 44 44 4c d0 30 18 cc f8 9c 9c 9c dc 9c
	static constexpr uint8_t saynomore_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 50 10 84 c8 a8 2c 30 94 1c e4 dc f4
	static constexpr uint8_t luckystrikealt_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting  00 e0 a8 38 90 68 30 60 e4 e8 18 d0 6c 10 60 c4
	static constexpr uint8_t hittop_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting   00 30 20 14 a4 b8 d4 0c e4 3c 54 84 b8 54 24 90
	static constexpr uint8_t vivaalt_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting    00 84 a4 e4 a8 3c dc d0 6c 58 d8 d8 dc d8 d4 60
	static constexpr uint8_t mintalt_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 24 24 34 b0 a8 58 74 b0 38 54 90
	static constexpr uint8_t hittopalt_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 40 50 40 54 64 50 64 50 64 54 60 74 40 70 (unusual sequence)
	static constexpr uint8_t hittopalt2_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 30 20 14 a4 f0 c4 50 a4 74 00 b4 60 10
	static constexpr uint8_t andyfloalt_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 14 04 54 4c 20 50 44 5c 78 70 48 6c 60 14 48 2c
	static constexpr uint8_t andycappalt_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 14 04 94 e0 74 a4 50 a4 d4 60 b4
	static constexpr uint8_t montealt_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting  00 50 10 a0 68 14 b0 88 bc e4 48
	static constexpr uint8_t przmontealt_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting  00 50 40 14 c4 98 b4 0c e4 5c b4 8c ec
	static constexpr uint8_t acechasealt_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 14 04 54 64 14 64 14 64 54 20 74 04
	static constexpr uint8_t hypvipalt_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 e0 ac 1c 90 2c 14 40 e4 ec 18 f4 68 10 40 c4
	static constexpr uint8_t toptake_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 44 44 54 1c 60 50 14 1c 78 70 18 7c 70 04 58 7c 7c
	static constexpr uint8_t sunsetbalt_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting  00 90 84 b4 2c c0 34 a0 bc 78 70 28 e0 14 a8 4c c8 ec
	static constexpr uint8_t eighth_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 14 10 a0 c4 c4 74 30 b0 70 00 b4 50 80 f4 40 94
	static constexpr uint8_t pontoon_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 30 20 50 68 24 70 60 78 5c 5c 5c 7c 54 60 10 68 6c
	static constexpr uint8_t blueflash_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	/***************************************************************

	 Types below have no games using lamp scramble, so data is unknown / blank

	***************************************************************/

	// games with sequence starting 00 e0 8c 58 b0 68 30 64 e4 cc 58 f0 2c 50 64 c4 88 5c f4 0c
	static constexpr uint8_t crkpot_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 60 68 38 d0 2c 90 24 e4 e8 3c f0 88 34 20 40 e8 bc f4 28
	static constexpr uint8_t wayin_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 c0 c8 38 f4 4c 70 60 e4 e8 38 b4 48 34 44 
	static constexpr uint8_t bluediamond_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 24 24 a4 68 14 c4 28 d4 8c d8 f0 0c d0 8c
	static constexpr uint8_t wildtime_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting  00 a0 a8 58 f4 8c d8 70 c4 e8 58 74 80 2c 94 4c
	static constexpr uint8_t redheat_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 60 60 a0 2c 50 84 28 d4 c8 9c b4 48 94
	static constexpr uint8_t blkcat_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 60 60 c0 4c 10 84 48 b4 a8 98 d4 2c 90
	static constexpr uint8_t salsa_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 24 24 64 2c 30 48 30 68 58 5c 5c
	static constexpr uint8_t goljok_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 a0 a8 1c f4 c8 1c b4 cc 5c 74 44 e0 28
	static constexpr uint8_t blackwhite_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting  00 c0 d0 1c ec 78 ac 30 4c 2c 24 cc 7c a4 d8
	static constexpr uint8_t tricolor_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 60 60 24 68 c0 34 44 6c 8c 9c ac d4 18 ec 90 1c
	static constexpr uint8_t tribank_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 84 8c 3c f4 4c 34 14 54 14 40 d4 4c 70 04 d0 58 f4
	static constexpr uint8_t grandclub_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 30 10 0c 98 a8 c4 60 3c ac c4 7c b4
	static constexpr uint8_t tajmahal_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 24 24 64 a4 ac 78 74 e0 6c 50 c0 ac 58 74 00 a4 ec f8 94 c8
	static constexpr uint8_t giant_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting  00 30 10 84 e0 f0 c4 60 b4 54 80 f4 50 80 f4 40 b0 d4
	static constexpr uint8_t randroul_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting  00 44 44 c4 1c 24 c4 14 c8 b8 a4 dc ec b0 58 a4
	static constexpr uint8_t starsbars_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 a0 88 18 b0 48 50 60 e4 c8 58 90
	static constexpr uint8_t topgear_lamp_scramble[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

protected:
	mpu4_characteriser_pal(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	uint8_t* m_current_chr_table;
	int m_prot_col;

private:

	void protection_w(uint8_t data);
	void lamp_scramble_w(uint8_t data);
	uint8_t protection_r();
	uint8_t lamp_scramble_r();


	optional_device<cpu_device> m_cpu; // needed for some of the protection 'cheats'

	bool m_allow_6809_cheat;
	bool m_allow_68k_cheat;

	const uint8_t* m_current_lamp_table;
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

class mpu4_characteriser_bootleg45 : public mpu4_characteriser_pal
{
public:
	// construction/destruction
	mpu4_characteriser_bootleg45(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
};

class mpu4_characteriser_bootleg51 : public mpu4_characteriser_pal
{
public:
	// construction/destruction
	mpu4_characteriser_bootleg51(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
};

class mpu4_characteriser_bootleg_blastbank : public mpu4_characteriser_pal
{
public:
	// construction/destruction
	mpu4_characteriser_bootleg_blastbank(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
};

class mpu4_characteriser_bootleg_copcash : public mpu4_characteriser_pal
{
public:
	// construction/destruction
	mpu4_characteriser_bootleg_copcash(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
};



#endif // MAME_MACHINE_MPU4_CHARACTERISER_PAL_H
