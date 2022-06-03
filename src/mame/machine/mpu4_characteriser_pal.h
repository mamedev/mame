// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_MPU4_CHARACTERISER_PAL_H
#define MAME_MACHINE_MPU4_CHARACTERISER_PAL_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"

DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_PAL, mpu4_characteriser_pal)
DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_PAL_BWB, mpu4_characteriser_pal_bwb)

// bootleg protections
DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_BOOTLEG_PAL, mpu4_characteriser_bootleg)

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

	void set_bootleg_fixed_return(uint8_t ret)
	{
		m_bootlegfixedreturn = ret;
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
	static constexpr uint8_t m4dtri98_characteriser_prot[8] = { 0x03, 0xAF, 0x87, 0xAB, 0xA3, 0x8F, 0x87, 0x83 };


	// games with sequence starting 00 24 24 2C E0 B4 B8 4C E8 D8 (m4eaw__a9 etc.)
	static constexpr uint8_t m683_characteriser_prot[8] = { 0x03, 0xAF, 0x27, 0x8F, 0x0F, 0xA7, 0x27, 0x07 };

	// games with sequence starting 00 44 44 4c e0 d4 d8 2c e8 b8 (m4overmn)
	static constexpr uint8_t otm_characteriser_prot[8] = { 0x00, 0x4C, 0x44, 0x04, 0x38, 0x74, 0x74, 0x20 };

	// games with sequence starting 00 30 20 14 2c a0 54 24 3c 9c 9c 9c
	static constexpr uint8_t m441_characteriser_prot[8] = { 0x03, 0x3F, 0x33, 0x1F, 0x17, 0x3B, 0x33, 0x13 };
												   //    00    38    30    18    14    3C    34    14
												   //    00    38    30    18    10    38    30    14

	// games with sequence starting 00 c4 c4 44 c4 44 44 c4 cc 3c
	// this seems to be the same sequence as the MPU4 Video game 'Strike It Lucky' where we're having to use a 4k table for the question scramble
	static constexpr uint8_t m462_characteriser_prot[8] = { 0x03, 0xC3, 0xC7, 0x4F, 0x47, 0xE7, 0xC7, 0x47 };
												   //    04    44    44    48    40    60    40    40   rhog2

	// games with sequence starting 00 64 64 24 64 64 24 64 6C 9C BC
	static constexpr uint8_t wta_characteriser_prot[8] = { 0x00, 0x64, 0x60, 0x28, 0x20, 0x70, 0x60, 0x20 };


	// games with sequence starting 00 24 24 a4 4c 10 88 50 a8 d8 9c
	static constexpr uint8_t du91_characteriser_prot[8] = { 0x03, 0xAF, 0x27, 0x8F, 0x87, 0x2F, 0x27, 0x07 };
												   //    00    28    20    08    00    28    20    00    gb006


	// games with sequence starting 00 90 C0 54 8C 68 24 90 CC 6C 24 9C BC 34 88 6C
	static constexpr uint8_t rr6_characteriser_prot[8] = { 0x00, 0x18, 0x10, 0x18, 0x10, 0x18, 0x10, 0x14 };

	// games with sequence starting 00 30 20 14 64 30 44 30 64 34 00 74 20
	static constexpr uint8_t rhm_characteriser_prot[8] = { 0x00, 0x70, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00 }; // incomplete? doesn't seem correct for the lower rows of the roadhog set using it


	// games with sequence starting 00 a0 a8 18 f4 2c 70 60 e4 e8
	static constexpr uint8_t viva_characteriser_prot[8] = { 0x03, 0xE7, 0xA3, 0xC7, 0xC3, 0xA7, 0xA3, 0xC3 };
	// place your bets (same as above, but with unused lamp bits masked out, except for 0x24?)
	// static constexpr uint8_t viva_characteriser_prot[8] = { 0x00, 0x60, 0x20, 0x40, 0x40, 0x24, 0x20, 0x40 };
	// spend spend spend - This is unusual because the 2nd value DOES need to be different, bit 0x20 can't be set
	// or the 2nd reel will be in thew wrong place.  Does this indicate the lamp reading is more complex than
	// we believe, or are there 2 parts with the same sequence and one value different?
	static constexpr uint8_t viva_sss_characteriser_prot[8] = { 0x00, 0x40, 0x20, 0x40, 0x40, 0x20, 0x20, 0x40 };


	static constexpr uint8_t m407_characteriser_prot[8] = { 0x03, 0xC7, 0x83, 0xC3, 0xC3, 0xA3, 0x83, 0xC3 };


	// games with sequence starting 00 24 24 2c 70 20 0c 60 3c 5c 5c 5c 7c 4c 68
	static constexpr uint8_t duty_characteriser_prot[8] = { 0x00, 0x38, 0x24, 0x18, 0x08, 0x34, 0x20, 0x00 };

	// games with sequence starting 00 84 94 5c ec 3c ec 30 4c 68 60 cc
	static constexpr uint8_t andybt_characteriser_prot[8] = { 0x00, 0x48, 0x00, 0x48, 0x44, 0x08, 0x00, 0x00 };

	// games with sequence starting 00 50 40 90 a8 6c c4 30 c8
	static constexpr uint8_t alf_characteriser_prot[8] = { 0x00, 0x58, 0x50, 0x1C, 0x10, 0x58, 0x50, 0x10 };
												  //    03    DB    53    9B    93    5B    53    13

	// games with sequence starting 00 84 A4 AC 70 80 2C C0 BC 5C
	static constexpr uint8_t shuffle_characteriser_prot[8] = { 0x00, 0x18, 0x00, 0x18, 0x08, 0x10, 0x00, 0x00 };

	// games with sequence starting 00 44 44 54 34 04 54 14 34 14 20 74 04 60
	static constexpr uint8_t clbveg_characteriser_prot[8] = { 0x00, 0x70, 0x40, 0x70, 0x50, 0x60, 0x40, 0x40 };

	// games with sequence starting  00 84 C4 E4 4C 10 28 90 E8 78 34
	static constexpr uint8_t vivlv_characteriser_prot[8] = { 0x00, 0x28, 0x00, 0x28, 0x20, 0x08, 0x00, 0x00 };
	                                                //    00    28    00    28    24    14    00    00   m4jpjmp has this lamp scramble for it
													 
	// games with sequence starting  00 84 8c b8 74 80 1c b4 d8 74 00 d4 c8 78 a4 4c e0 dc f4 88
	static constexpr uint8_t celclb_characteriser_prot[8] = { 0x00, 0x50, 0x00, 0x50, 0x10, 0x40, 0x04, 0x00 };

	// games with sequence starting 00 14 10 C0 8C A8 68 30 D0 58 E4 DC F4
	static constexpr uint8_t cashmx_characteriser_prot[8] = { 0x04, 0x50, 0x10, 0x60, 0x60, 0x30, 0x30, 0x14 };

	// games with sequence 00 14 04 94 c8 68 a0 18 f4 8c e8 ec ac a8 6c 20 54 c4 dc
	static constexpr uint8_t viz_characteriser_prot[8] = { 0x00, 0x50, 0x10, 0x54, 0x14, 0x50, 0x10, 0x14 };

	// games with sequence 00 e4 ec f8 54 08 d0 80 44 2c 58 b4 e8 b0 80
	static constexpr uint8_t nifty_characteriser_prot[8] = { 0x03, 0xE7, 0xA7, 0x87, 0xE7, 0x07, 0xA7, 0xE7 };

	// games with sequence 00 84 a4 e4 b0 34 54 44 d4 64 80 f4 24 80 f4 20
	static constexpr uint8_t milclb_characteriser_prot[8] = { 0x00, 0x54, 0x00, 0x54, 0x40, 0x10, 0x00, 0x00 };

	// games with sequence 00 44 44 c4 58 60 c0 50 8c b8 e0 dc ec b0 1c e8 38
	static constexpr uint8_t fruitfall_characteriser_prot[8] = { 0x03, 0xCF, 0x47, 0xCB, 0xC3, 0x4F, 0x47, 0x43 };


	// games with sequence 00 60 68 bc d0 2c 94 20 e4 e8 bc f0 88 34 a0 c4 ec bc f4 
	static constexpr uint8_t m400_characteriser_prot[8] = { 0x03, 0xE7, 0x43 ,0xC3, 0xC3 ,0xE3, 0x43, 0xC3 }; // does anything using this have lamp scramble or was this extracted with tests?

	// games with sequence 00 bc b8 fc bc dc fc fc fc f8 d8 b8 f8 d8 fc bc fc 98 fc f8 f8
	static constexpr uint8_t intcep_characteriser_prot[8] = { 0x00, 0x1C, 0x38, 0x78, 0x7C, 0x78, 0x38, 0x7C };	

	// games with sequence 00 50 40 14 C4 B0 A4 30 C4 74 00 D4 E0 30 C0 34
	static constexpr uint8_t take2_characteriser_prot[8] = { 0x00, 0x50, 0x50, 0x10, 0x10, 0x50, 0x50, 0x00 };	

	// games with sequence 00 50 40 14 4C 80 34 44 5C 9C 9C 9C DC 9C DC 94
	static constexpr uint8_t m435_characteriser_prot[8] = { 0x03, 0x5F, 0x53, 0x1F, 0x17, 0x5B, 0x53, 0x13 };	

	// games with sequence 00 84 8C D8 74 80 4C 90 E8 78 54 60 84
	static constexpr uint8_t m578_characteriser_prot[8] = { 0x00, 0x60, 0x00, 0x60, 0x40, 0x20, 0x00, 0x00 };	

	// games with sequence 00 c0 c8 1c f4 68 14 50 70 50 20 f0 48 34 60
	static constexpr uint8_t age_characteriser_prot[8] = { 0x00, 0x74, 0x44, 0x34, 0x14, 0x64, 0x44, 0x00 };

	// games with sequence starting 00 60 60 C0 58 44 E0 50 A8 9C CC BC E4 50 A0 58
	static constexpr uint8_t sunsetb_characteriser_prot[8] = { 0x03, 0xEB, 0x63, 0xCB, 0xC3, 0x6B, 0x63, 0x43 };
	
	// games with sequence starting 00 60 60 a0 38 64 e0 30 c8 9c ac dc ec 94 d8 a4 38 ec
	static constexpr uint8_t bjac_characteriser_prot[8] = { 0x00, 0x68, 0x60, 0x28, 0x20, 0x68, 0x60, 0x20 };

	// games with sequence starting 00 88 70 14 1c c0 a4 a0 bc d4 30 14 18 d4 2c 50 1c
	static constexpr uint8_t mag7s_characteriser_prot[8] = { 0x03, 0x9F, 0x0F, 0x17, 0x03, 0x1B, 0x8F, 0x87 };

	// games with sequence starting 00 44 44 54 d0 88 38 74 d0 58
	static constexpr uint8_t oad_characteriser_prot[8] = { 0x00, 0x50, 0x44, 0x14, 0x14, 0x44, 0x44, 0x00 };

	// games with sequence starting 00 18 70 24 38 58 74 0c 6c 64
	static constexpr uint8_t rhs_characteriser_prot[8] = { 0x00, 0x3C, 0x18, 0x30, 0x10, 0x3C, 0x18, 0x10 };

	// games with sequence starting 10 94 1c f4 b8 74 b4 98 f4 9c f0 b8 d4 38 74 10 (unusual sequence)
	static constexpr uint8_t rockmn_characteriser_prot[8] = { 0x10, 0x34, 0x14, 0x34, 0x30, 0x30, 0x14, 0x30 };

	// games with sequence starting 00 0c 50 90 b0 38 d4 a0 bc d4 30 90 38 c4 ac 70
	static constexpr uint8_t gambal_characteriser_prot[8] = { 0x00, 0x18, 0x08, 0x10, 0x00, 0x18, 0x08, 0x00 };


	// Games with sequence starting
	// 00 60 60 44 e0 e8 1c 74 a4 6c 14 84 e8 1c f4
	// used by
	// Classic Adders & Ladders  "A6L 0.1"
	// Luxor                     "LUX 0.6"
	// Prize Luxor (Barcrest)    "PLX 0.2"
	// Double Up                 " DU 1.5"
	static constexpr uint8_t addr_characteriser_prot[8] = { 0x00, 0x60, 0x60, 0x40, 0x40, 0x60, 0x60, 0x40 }; // match output of unprotected bootlegs


	// Games with sequence starting
	// 00 a0 88 38 94 2c 30 00 e4 c8 18 b4 4c 30
	// used by
	// Classic Adders & Ladders  "ADD 1.0"
	//                           "ADD 3.0"
	//                           "ADD 4.0"
	//                           "ADD 5.0"
	// Squids In                 "SQ_ 2.0"
	// Reel Poker                "R2P 3.0" (lamp scramble not used)
	static constexpr uint8_t squids_characteriser_prot[8] = { 0x00, 0x60, 0x20, 0x60, 0x40, 0x20, 0x20, 0x40 }; // match m4addr decodes for sets using this table


	// Games with sequence starting
	// 00 90 18 e4 a8 3c f4 48 74 50 20 f0 18 e4 98 e4 a8 7c f4 18 c4 c8 0c 74 10 60 d0 28 14 70 00 c0 b8 b4 68 44 d0 28 24 90 08 24 f0 78 f4 48 44 d0 78 c4 d8 e4 b8 e4 d8 c4 e8 7c d4 18 e4 98 f4 00
	// used by
	// Spend Spend Spend "SX5 2.0"
	//                   "SX102.0"
	// Super Hyper Viper "H6Y 0.3"
	//                   "H6Y 0.2"
	// Golden Gate       "DGG 2.2" (lamp scramble not used? currently not booting)	
	static constexpr uint8_t m450_characteriser_prot[8] = { 0x00, 0x70, 0x10, 0x60, 0x40, 0x30, 0x10, 0x00 };


	// Games with sequence starting
	// 00 c0 e0 b0 38 c4 f0 30 58 9c 9c 9c dc 9c dc 94 38 dc dc 8c 3c 8c 64 c0 f0 38 9c 8c 64 d0 20 d0 68 44 c8 3c 9c 8c 3c d4 20 c0 f8 dc 9c 94 78 c4 f8 94 78 9c 8c 3c dc 94 38 9c dc 8c 74 00 d8 00
	// used by
	// Viva Las Vegas        "VL_ 2.0"
	// Ten Ten Do It Again   "TDA 0.4"
	// Cloud Nine Club       "CNC 2.1"
	// Nudge Nudge Wink Wink "NN3 0.1"
	// Cash Connect          "CCO 3.2"
	// Ring Of Fire          "ROF 0.3" (lamp scramble not used)
	// Twenty One            "DTO 2.0" (lamp scramble not used)
	static constexpr uint8_t tentendia_characteriser_prot[8] = { 0x00, 0x58, 0x40, 0x18, 0x10, 0x48, 0x40, 0x00 };


	// Games with sequence starting
	// 00 90 a0 70 c8 2c c4 30 c8 6c 44 d8 dc 5c d4 60 98 dc dc 1c 54 40 10 88 ec ec 6c 84 b0 68 84 78 d4 e0 38 54 c0 38 1c d4 20 90 e8 ec 2c 84 f0 a0 f8 54 c8 ec ac 6c c4 70 c0 f8 d4 a0 70 00 d8 00
	// used by
	// Tic Tac Toe   "TT_ 2.0"
	//               "TT  1.0"
	static constexpr uint8_t ttt_characteriser_prot[8] = { 0x00, 0x58, 0x10, 0x58, 0x50, 0x18, 0x10, 0x10 }; // lack of evidence, guessed based on logical lamp pattersn


	// Games with sequence starting
	// 00 90 c0 54 a4 f0 64 90 e4 d4 60 b4 c0 70 80 74 a4 f4 e4 d0 64 10 20 90 e4 f4 c4 70 00 14 00 14 a0 f0 64 10 84 70 00 90 40 90 e4 f4 64 90 64 90 e4 50 24 b4 e0 d4 e4 50 04 b4 c0 d0 64 90 e4 00
	// used by
	// Tic Tac Toe            "TT_ 3.0"
	// Dutch Adders & Ladders "DAL 1.2" (lamp scramble not used? currently not booting)
	// Dutch Old Timer        "DOT 1.1" (lamp scramble not used? currently not booting)
	static constexpr uint8_t m470_characteriser_prot[8] = { 0x00, 0x30, 0x10, 0x30, 0x10, 0x30, 0x10, 0x10 }; // lack of evidence, crafted to match ttt_characteriser_prot output


	// Games with sequence starting
	// 00 a0 b0 58 ec 3c ec 14 68 4c 4c 6c 64 80 f8 84 98 ec 7c 8c 5c c4 b0 30 28 6c 4c 04 a0 d0 10 40 a8 3c ec 54 60 a0 98 c4 b0 30 68 64 a8 14 68 24 e8 54 68 6c 24 e0 d0 50 40 e8 74 20 c0 b0 78 00
	// used by
	// Tic Tac Toe Gold    "TG  3.3"
	//                     "TG  4.4"
	// Tic Tac Toe Classic "CT4 7.0"
	//                     "CT  4.0"
	//                     "CTT 3.0" / "CT4 3.0"
	//                     "CT  2.3"
	//                     "CT  2.4"		 
	// Top Action          " TA 2.2" (one set) (lamp scramble not used)
	static constexpr uint8_t topaction_characteriser_prot[8] = { 0x00, 0x68, 0x20, 0x48, 0x40, 0x28, 0x20, 0x00 }; // lack of evidence, guessed based on logical lamp patterns


	// Games with sequence starting
	// 00 c4 e8 58 b4 4c 30 40 e4 a8 18 94 48 34 64 c4 c8 7c f4 28 30 64 c0 ac 1c d0 68 70 04 a0 e8 3c f0 0c 30 60 c0 ec 1c b0 48 54 64 80 cc 3c d4 28 74 44 a0 ac 5c 94 2c 74 00 e0 8c 3c d4 0c 74 00
	// used by
	// Graffiti          "GRA 2.0"
	//                   "GRA 2.1"
	// Red Alert         "RA3 0.2"
	// Pot Luck 100 Club "P1L 2.2"
    //                   " PL 2.7"
	// Flashlite         "FLT 1.0"
	static constexpr uint8_t graff_characteriser_prot[8] = { 0x00, 0x60, 0x40, 0x60, 0x20, 0x40, 0x40, 0x20 }; // based on logical arrangements for m4ra__g set


	// Games with sequence starting
	// 00 50 40 14 64 50 24 50 64 54 20 74 40 30 60 10 64 74 64 50 04 34 60 50 44 74 44 10 00 14 00 14 60 50 64 10 44 10 24 50 00 50 64 54 64 50 24 70 64 10 24 74 40 54 64 10 04 74 40 50 24 50 64 00
	// used by
	// Hot Rod                      "HRC_1.0"
	// Buccaneer                    "BUG 0.4"
	//                              "BUS 0.1"
    // All Cash Advance             "C2B 6.0"
	static constexpr uint8_t buc_characteriser_prot[8] = { 0x00, 0x70, 0x50, 0x30, 0x10, 0x70, 0x50, 0x10 }; // based on matching m4hotrod__a and m4buc__2 with unprotected sets


	// Games with sequence starting
	// 00 90 88 4c e0 b8 74 84 bc 74 00 b4
	// used by
	// Jewel In the Crown "CJE 1.0"
	//                    "CJE 0.8"
	//                    "CJH 1.0"
	//                    "CJH 0.8"
	// Las Vegas Strip    "VSG 0.4"
	//                    "VSG 0.3"
	// Royal Jewels       "GRJ 1.4"
	static constexpr uint8_t jewelcrown_characteriser_prot[8] = { 0x00, 0x30, 0x10, 0x30, 0x10, 0x30, 0x10, 0x10 };  // matches unprotected Las Vegas Strip sets


	// Games with sequence starting
	// 00 14 04 94 c8 68 a0 50 8c e8 e0 dc bc b0 4c a0 58 bc bc 38 b4 48 20 14 8c ec e8 6c 24 94 40 90 4c a4 58 b8 b8 3c 38 b4 40 14 8c e8 68 24 94 cc ec e0 d0 c8 68 a8 e0 50 80 dc bc 38 b8 30 8c 00
	// used by
	// The Crystal Maze      "CRM 3.0"
	//                       "CRM 2.3"
	// Showcase Crystal Maze "SCM 0.1"
	// Cloud Nine            "C92 1.1"
	//                       "C92 1.0"
	//                       "C95 1.0"
	// Las Vegas Strip       "UVS 0.3"
	// Prize What's On       "PWO 0.5"
	// Carry On Joker        "COJ 2.1"
	// Super Streak          "STT 0.3"
	// Crown Jewels (German) "CJG 0.4"
	// Sunset Boulevard      "B25 1.2"
	// Dutch Big Ben         "DBB 1.2"
	// Fruit Preserve        "F4P 1.1"
	// Blue Moon             "BLU 2.3"
	//                       "BLU 2.1"
	static constexpr uint8_t m4lv_characteriser_prot[9] = { 0x00, 0x18, 0x10, 0x18 ,0x10, 0x18, 0x10, 0x10 }; // games match unprotected versions
	//static constexpr uint8_t m4lv_characteriser_prot[9] = { 0x03, 0xEB, 0x63, 0xCB, 0xC3, 0x6B, 0x63, 0x43 };//   in sunsetb (wrong?)


	// Games with sequence starting
	// 00 14 10 60 54 00 24 14 70 30 00 74 10 40 34 40 54 70 70 10 20 44 14 50 30 60 34 10 00 24 14 20 14 70 70 00 34 10 60 14 00 14 70 30 70 30 60 14 70 00 24 74 10 30 60 04 24 74 10 10 60 14 70 00
	// used by
	// Jewel In the Crown "JCC 3.7"
	//                    "JCC 3.3"
	//                    "JC4 3.1"
	//                    "JC5 1.9"
	//                    "JC8 4.4"
	//                    "JC8 4.2"
	// Pot Black Casino   "PO  1.2"
	// Fortune Club       "CFO 1.2"
	static constexpr uint8_t fortune_characteriser_prot[8] = { 0x00, 0x70, 0x10, 0x60, 0x20, 0x50, 0x10, 0x00 }; // guessed based on lamp positions in Pot Black Casino and Jewel in the Crown


	// Games with sequence starting
	// 00 14 04 54 c4 98 f0 48 e4 5c f0 c8 ec 68 24
	// used by
	// Kings & Queens    "EE  2.0"
    //                   "EE  1.0"
	// Lucky Strike      "LSS 0.6"
    //                   "LST 0.9"
	// Solid Silver Club "SOS 2.2"
    // Solid Silver Club "SOS 2.1"
	static constexpr uint8_t luckystrike_characteriser_prot[8] = { 0x00, 0x50, 0x10, 0x50, 0x50, 0x10, 0x10, 0x10 }; // Lucky Strike matches unprotected sets


	// Games with sequence starting
	// 00 14 04 94 A8 6C C4 30 8C E8 E0 BC D4
	// used by
	// Andy's Great Escape           "AG5 3.0"
	//                               "AG__2.0"
	// Nudge Nudge Wink Wink Classic "NN5 0.2 / NN4 0.2"
	//                               "CN1 0.1 / NN4 0.1"
	//                               "CNU 0.2 / NN4 0.2"
	//                               "NN4 0.2"
	// Viva Espana                   "VE5 3.0"
	// Lucky Las Vegas               "LLV 0.2"
	//                               "LL8 0.1"
	// Andy's Full House             "AFH 0.1"
	//                               "AF3 0.1"
	//                               "AF8 0.1"
	//                               "AFT 0.3"
	//                               "CA4 0.8"
	//                               "CAT 0.2"
	//                               "CAU 0.1 / CA4 0.1"
	// Mad House                     "MH5 0.2"
	//                               "MD8 0.1"
	//                               "MAD 0.5"
	//                               "MHT 0.2"
	// Super Blackjack Club          "SBJ 3.1"
	//                               "SBJ 2.0"
	// Prize Spend Spend Spend       "PS8 0.1"
	//                               "SSP 0.5"
	// Super Streak                  "SP8 0.1"
	//                               "SPS 0.8"
	//                               "CS4 0.7"
	//                               "CS4 0.4 / CST 0.4"
	//                               "CS4 0.3 / CSU 0.3"
	// Jolly Joker                   "JOJ 1.6"
	// Jolly Taverner                "TAV 1.3"
	// Club Double                   " CD 1.6"
	static constexpr uint8_t m574_characteriser_prot[8] = { 0x03, 0x9F, 0x17, 0x9B, 0x93, 0x1F, 0x17, 0x13 };

	// games with sequence starting
	// 00 44 44 64 4c 80 70 24 6c a8 b0 38 e4
	// used by
	// Prize Money          "FP8 0.1"
	//                      "FPM 0.3"
	// Prize Money Showcase "SPM 0.2"
	//                      "SM8 0.1"
	// Brooklyn             "PFT 1.8" (doesn't use lamp scramble)
	// Flash Cash           " FC 1.0" (doesn't use lamp scramble)
	// Dutch Number One     "DNO 1.7" (doesn't use lamp scramble? doesn't boot)
	static constexpr uint8_t pzmoney_characteriser_prot[8] = { 0x00, 0x68, 0x40, 0x68, 0x60, 0x48, 0x40, 0x40 }; // based on przmoney lamp patterns

	// games with sequence starting
	// 00 14 10 a0 8c c8 68 50 b0 38 64 b4 18
	// used by
	// Club Classic      "CI  1.1"
	// Dutch Atlantis    "DAT 1.4"
	// Dutch Twin Timer  "D2T 1.1"
	static constexpr uint8_t m533_characteriser_prot[8] = { 0x00, 0x30, 0x10, 0x20, 0x20, 0x10, 0x10, 0x00 }; // based on clbcls lamp patterns


	// games with sequence starting
	// 00 60 60 24 e0 e8 1c 74 c4 6c 14 84 e8 1c
	// used by
	// Cash Lines (Barcrest) (MPU4) "CLS 0.3"
	//                              "CLI 1.1"
	//                              "CLI 1.0"
	//                              "NCL 1.1"
	//                              "NCC 1.0"
	// Action Bank                  "ACT 0.7"
	// Top Tenner                   "TTS 0.2"
	// Top Tenner                   "TTH 1.0"
	// Super Two                    "SUT 1.2"            
	// Centrepoint                  " DU 1.3"
	// Supatron                     "DSU 2.1"
	// Swap-A-Note                  " SN 3.3"
	//                              " SN 3.2"
	//                              " SN 3.5"
	static constexpr uint8_t actionbank_characteriser_prot[8] = { 0x00, 0x60, 0x60, 0x20, 0x20, 0x60, 0x60, 0x20 }; // matches unprotected sets for various games

	/***************************************************************

	 Lamp data below is definitely incorrect

	***************************************************************/




	// games with sequence starting
	// 00 90 a0 34 8c 68 44 90 ac 6c 44 9c dc 5c d4 24 98 dc
	// used by
	// Viz          "VZ__1.0"
	// Gold Strike  "G4S 2.0" (doesn't use lamp scramble)
	// Magic Dragon "DMD 1.0" (doesn't use lamp scramble? doesn't boot)
	static constexpr uint8_t magicdragon_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT





	// games with sequence starting 00 90 88 2c e0 d8 74 84 dc 74 00 d4 c8 6c a0 58 f4 cc ec 68
	static constexpr uint8_t toplot_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 18 70 44 58 30 44 18 7c 74 00 5c 7c 34 48 24 58
	static constexpr uint8_t tictak_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 24 24 64 2c 14 4c 14 4c 58 78 78 74
	static constexpr uint8_t actclba_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 14 10 c0 4c 20 84 0c f0 98 e4 dc f4 08 f0 08 70 d0
	static constexpr uint8_t phr_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 44 44 64 4c 10 28 50 68 38 34 28 70 00 6c 10 68
	static constexpr uint8_t cheryo_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 30 10 44 70 10 44 30 54 14 40 74 10 40 34
	static constexpr uint8_t cosmiccasino_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 84 8c 3c f4 4c 34 24 e4 ac 38 f0 0c 70 04 
	static constexpr uint8_t kingqn_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 c0 d0 38 ec 5c ec 14 68 2c 24 e8 74 00 e8 14
	static constexpr uint8_t turboplay_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 14 04 34 2c 44 34 24 3c 78 70 28 64
	static constexpr uint8_t sunsetclub_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 18 c8 a4 0c 80 0c 90 34 30 00 58
	static constexpr uint8_t doublediamond_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 44 44 c4 68 14 8c 30 8c b8 d0 a8
	static constexpr uint8_t cashencounters_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 8c 64 84 84 c4 84 84 9c f4 04 cc 24 84 c4 94 54 (NOTE, same sequence as v4addlad, which uses 4k table)
	static constexpr uint8_t bankrollerclub_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 60 60 44 68 a0 54 24 6c 8c 9c cc bc c4 74 00
	static constexpr uint8_t copycat_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 30 10 0c 58 60 24 30 1c 6c 44 3c 74 00
	static constexpr uint8_t thestreak_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 84 c4 d4 70 04 94 50 34 14 20 b4 44 a0 e4
	static constexpr uint8_t kqee2_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 18 b0 64 38 98 b4 44 3c b4 40 3c 9c b4
	static constexpr uint8_t kqee_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 14 10 48 38 34 58 74 58 6c 60 5c 7c 64 14
	static constexpr uint8_t pfloot_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting  00 50 40 30 68 44 70 60 78 3c 34 28 64 10
	static constexpr uint8_t berseralt_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 a0 e0 c4 c8 58 9c 94 6c 1c 9c 9c bc 94 6c 10
	static constexpr uint8_t berseralt2_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 24 24 a4 4c 10 c0 0c f0 a8 98
	static constexpr uint8_t bagtel_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 84 c4 d4 58 24 94 50 98 3c 34 18 bc 34 88 78 bc
	static constexpr uint8_t bucalt_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 84 a4 b4 38 c4 b4 30 1c d8 d8 d8 dc
	static constexpr uint8_t hotrodalt_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting  00 e0 88 18 b0 48 50 60 e4 c8 58 f0 08
	static constexpr uint8_t bdash_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 44 44 4c d0 30 18 cc f8 9c 9c 9c dc 9c
	static constexpr uint8_t saynomore_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 50 10 84 c8 a8 2c 30 94 1c e4 dc f4
	static constexpr uint8_t luckystrikealt_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting  00 e0 a8 38 90 68 30 60 e4 e8 18 d0 6c 10 60 c4
	static constexpr uint8_t hittop_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting   00 30 20 14 a4 b8 d4 0c e4 3c 54 84 b8 54 24 90
	static constexpr uint8_t vivaalt_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting    00 84 a4 e4 a8 3c dc d0 6c 58 d8 d8 dc d8 d4 60
	static constexpr uint8_t mintalt_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 24 24 34 b0 a8 58 74 b0 38 54 90
	static constexpr uint8_t hittopalt_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 40 50 40 54 64 50 64 50 64 54 60 74 40 70 (unusual sequence)
	static constexpr uint8_t hittopalt2_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 30 20 14 a4 f0 c4 50 a4 74 00 b4 60 10
	static constexpr uint8_t andyfloalt_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 14 04 54 4c 20 50 44 5c 78 70 48 6c 60 14 48 2c
	static constexpr uint8_t andycappalt_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 14 04 94 e0 74 a4 50 a4 d4 60 b4
	static constexpr uint8_t montealt_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting  00 50 10 a0 68 14 b0 88 bc e4 48
	static constexpr uint8_t przmontealt_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting  00 50 40 14 c4 98 b4 0c e4 5c b4 8c ec
	static constexpr uint8_t acechasealt_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 14 04 54 64 14 64 14 64 54 20 74 04
	static constexpr uint8_t hypvipalt_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 e0 ac 1c 90 2c 14 40 e4 ec 18 f4 68 10 40 c4
	static constexpr uint8_t toptake_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 44 44 54 1c 60 50 14 1c 78 70 18 7c 70 04 58 7c 7c
	static constexpr uint8_t sunsetbalt_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting  00 90 84 b4 2c c0 34 a0 bc 78 70 28 e0 14 a8 4c c8 ec
	static constexpr uint8_t eighth_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 14 10 a0 c4 c4 74 30 b0 70 00 b4 50 80 f4 40 94
	static constexpr uint8_t pontoon_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 30 20 50 68 24 70 60 78 5c 5c 5c 7c 54 60 10 68 6c
	static constexpr uint8_t blueflash_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 24 24 a4 1c 44 a4 14 a8 d8 cc f8 e4 14
	static constexpr uint8_t wtaalt_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	/***************************************************************

	 Types below have no games using lamp scramble, so data is unknown / blank

	***************************************************************/

	// games with sequence starting 00 50 10 24 54 00 60 50 34 30 00 74 10 04 74
	static constexpr uint8_t premier_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT

	// games with sequence starting 00 e0 8c 58 b0 68 30 64 e4 cc 58 f0 2c 50 64 c4 88 5c f4 0c
	static constexpr uint8_t crkpot_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 60 68 38 d0 2c 90 24 e4 e8 3c f0 88 34 20 40 e8 bc f4 28
	static constexpr uint8_t wayin_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 c0 c8 38 f4 4c 70 60 e4 e8 38 b4 48 34 44 
	static constexpr uint8_t bluediamond_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 24 24 a4 68 14 c4 28 d4 8c d8 f0 0c d0 8c
	static constexpr uint8_t wildtime_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting  00 a0 a8 58 f4 8c d8 70 c4 e8 58 74 80 2c 94 4c
	static constexpr uint8_t redheat_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 60 60 a0 2c 50 84 28 d4 c8 9c b4 48 94
	static constexpr uint8_t blkcat_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 60 60 c0 4c 10 84 48 b4 a8 98 d4 2c 90
	static constexpr uint8_t salsa_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 24 24 64 2c 30 48 30 68 58 5c 5c
	static constexpr uint8_t goljok_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 a0 a8 1c f4 c8 1c b4 cc 5c 74 44 e0 28
	static constexpr uint8_t blackwhite_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting  00 c0 d0 1c ec 78 ac 30 4c 2c 24 cc 7c a4 d8
	static constexpr uint8_t tricolor_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 60 60 24 68 c0 34 44 6c 8c 9c ac d4 18 ec 90 1c
	static constexpr uint8_t tribank_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 84 8c 3c f4 4c 34 14 54 14 40 d4 4c 70 04 d0 58 f4
	static constexpr uint8_t grandclub_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 30 10 0c 98 a8 c4 60 3c ac c4 7c b4
	static constexpr uint8_t tajmahal_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 24 24 64 a4 ac 78 74 e0 6c 50 c0 ac 58 74 00 a4 ec f8 94 c8
	static constexpr uint8_t giant_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting  00 30 10 84 e0 f0 c4 60 b4 54 80 f4 50 80 f4 40 b0 d4
	static constexpr uint8_t randroul_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting  00 44 44 c4 1c 24 c4 14 c8 b8 a4 dc ec b0 58 a4
	static constexpr uint8_t starsbars_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 a0 88 18 b0 48 50 60 e4 c8 58 90
	static constexpr uint8_t topgear_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 c0 c8 38 f4 8c b8 70 a4 e8 38 74 80 4c b0 0c 94
	static constexpr uint8_t redwhite_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting  00 50 10 a0 c4 c4 74 30 b0 34 00 f0 14 80 f4 04
	static constexpr uint8_t techno_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting   00 48 a0 54 2c 88 94 14 2c a4 50 24 48 a4 78 c0 70
	static constexpr uint8_t bucksfizz_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 0c 50 60 4c 10 60 0c 78 74 00 6c 38 34 48 
	static constexpr uint8_t hirise_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 24 24 2c b0 e0 4c 30 a8 d8 9c 9c bc 1c bc 94
	static constexpr uint8_t nudshf_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


protected:
	mpu4_characteriser_pal(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	uint8_t* m_current_chr_table;
	int m_prot_col;
	uint8_t m_bootlegfixedreturn = 0;

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

class mpu4_characteriser_bootleg : public mpu4_characteriser_pal
{
public:
	// construction/destruction
	mpu4_characteriser_bootleg(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: mpu4_characteriser_pal(mconfig, MPU4_CHARACTERISER_BOOTLEG_PAL, tag, owner, clock)
	{
	}

	virtual uint8_t read(offs_t offset) override
	{
		logerror("%s: Characteriser read offset %02x\n", machine().describe_context(), offset);
		return m_bootlegfixedreturn;
	}

	virtual void write(offs_t offset, uint8_t data) override
	{
		logerror("%s: Characteriser write offset %02x data %02x\n", machine().describe_context(), offset, data);
	}
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
