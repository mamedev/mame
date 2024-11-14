// license:BSD-3-Clause
// copyright-holders:David Haywood, James Wallace

#ifndef MAME_BARCREST_MPU4_CHARACTERISER_PAL_H
#define MAME_BARCREST_MPU4_CHARACTERISER_PAL_H

#pragma once

#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"

DECLARE_DEVICE_TYPE(MPU4_CHARACTERISER_PAL, mpu4_characteriser_pal)

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

	void set_allow_6800_cheat(bool allow)
	{
		m_allow_6800_cheat = allow;
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

	/* While some games use unique keys and lamp scrambles, several do write the same sequences and expect the
	   same responses.  It is possible PALs were reused.  Sometimes the lamp tables are masked subsets, as
	   they were handcrafted when the layouts were made, they could also be incorrect in places.

	   The code checking the responses always masks with 0xfc, so the real responses from the devices could
	   have the lowest 2 bits set depending on the device state, but this is ignored.

	   Likewise the code to read the lamps typically masks out bits, so presumably the lamp scrambles for
	   some PAL types have been worked out from tests on real hardware?

	   Some of the bits that aren't masked out on the lamp reads are not used for lamps, are these also used
	   for payout scrambling, as the later BWB games do in their own way?

	*/

	// these can be identified as games expecting a chr response starting with
	// 00 84 94 3c ec 5c ec 50 2c 68 60 ac'
	// Big Bandit (Nova)             "BIG 0.4"
	// Crown Jewels (German)         "GCN 0.9"
	//                               "GCN 1.1"
	// Action Note                   "AN 1.2"
	// Card Cash                     "CS 1.9"
	// Bank A Note                   "BN 1.0"
	// Money Maker                   "MMK 1.6"
	// Dutch Wild Mystery            "DWM 1.8"
	// Let The Good Times Roll       "GTR 2.0"
	//                               "GTS 1.0"
	//                               "GTS 0.2"
	//                               "GTR 1.1"
	// Jackpot Gems                  "RRH 0.1 / CG4 0.1"
	// Jackpot Gems Classic          "GTC 0.1 / CG4 0.1"
	//                               "HGE 0.1 / CG4 0.1"
	// Jolly Gems                    "GEM 0.7"
	//                               "GMS 0.5"
	// Hit The Top                   "HT2 0.1"
	//                               "HT5 0.1"
	// Nudge Nudge Wink Wink Classic "CN3 0.2 / NN4 0.2"
	//                               "CF3 0.2 / NN4 0.2"
	//                               "CH3 0.2 / NN4 0.2"
	//                               "CH3 0.1 / NN4 0.1"
	// Ready Steady Go               "RGO 0.8"
	//                               "CGO 1.1"
	//                               "DRR 0.2"
	//                               "HJJ 0.1"
	//                               "HJJ 0.2"
	//                               "PPL 0.2"
	//                               "RGT 1.0"
	// Magnificent 7s                "MA7 1.6"
	//                               "MAS 1.3"
	// Make A Mint                   "MAM 0.4"
	//                               "MMG 0.5"
	// Pot Black                     "PBS 0.6"
	//                               "PBG 1.6"
	// Place Your Bets               "PYB 0.7"
	//                               "PYH 0.6"
	// Cloud Nine                    "CT3 0.2"
	//                               "CT2 0.2"
	//                               "CT5 0.2"
	// Tutti Fruity                  "F1U 0.1"
	//                               "F2U 0.1"
	//                               "F3U 0.1"
	// Cash Attack                   "CSA 1.2"
	//                               "CAA 2.3"
	// Red Hot Roll                  "HHN 0.2 / CR4 0.2"
	//                               "CLD 0.3 / CRU 0.3"
	//                               "HHN 0.3 / CR4 0.3"
	//                               "RRD 0.3 / CR4 0.3"
	// Up Up and Away                "UPS 2.2"
	//                               "UUA 2.2"
	// Hi Jinx                       "JNX 1.0"
	//                               "JNS 0.3"
	// Cash Lines                    "CLS 0.4"
	//                               "CLI 1.2"
	// Lucky Strike                  "LSS 0.7"
	//                               "LST 1.0"
	// 10 X 10                       "T2T 0.1"
	//                               "T3T 0.1"
	//                               "TST 0.1"
	// Boulder Dash                  "BLD 1.0"
	//                               "BLS 0.2"
	// Hot Rod                       "ROD 0.4"
	// Buccaneer                     "BUG 0.5"
	//                               "BUS 0.2"
	// Bagatelle                     "EL1 0.1"
	//                               "EL2 0.1"
	//                               "EL3 1.0"
	// The Crystal Dome              "CD2 1.2"
	//                               "CD2 1.0"
	//                               "CD2 0.2"
	// Cash Machine                  "CMA 0.8"
	//                               "CMH 0.7"
	// Top Tenner                    "TTS 0.4"
	//                               "TTH 1.1"
	//                               "TTH 1.2"
	// Ten Out Of Ten                "TOC 0.4"
	//                               "TOT 0.6"
	// Everyone's A Winner           "ENN 0.1 / ER4 0.1"
	//                               "EON 0.1 / ER4 0.1"
	//                               "EUN 0.1 / ER4 0.1"
	// Let The Good Times Roll       "GTR 2.0"
	//                               "GTS 1.0"
	//                               "GTS 0.2"
	//                               "GTR 1.1"
	// Super Streak                  "CS4 0.2 / CSP 0.2"
	//                               "CS4 0.2 / EEH 0.2"
	//                               "CS4 0.2 / STC 0.2"
	//                               "STC 0.1"
	static constexpr uint8_t m4dtri98_characteriser_prot[8] = { 0x03, 0xAF, 0x87, 0xAB, 0xA3, 0x8F, 0x87, 0x83 };


	// games with sequence starting
	// 00 24 24 2C E0 B4 B8 4C E8 D8 (m4eaw__a9 etc.)
	// Over The Moon               "OT8 0.1"
	//                             "OTN 0.5"
	// Lucky Strike Club           "CLU 1.4"
	//                             "GLS 0.6"
	//                             "GS3 0.1"
	//                             "LS3 0.1"
	// Cash Machine                "CMA 0.7"
	//                             "CMH 0.6"
	//                             "CMH 0.2"
	// Everyone's A Winner         "ER8 0.1"
	//                             "ER4 0.2"
	//                             "CET 0.3 / ER4 0.3"
	//                             "CEU 0.2 / ER4 0.2"
	// Jokers Millenium 300        "DJO 0.1"
	// Dragon (Nova)               "DGL 0.1"
	// Red Alert                   "R2T 3.3"
	// Red Alert                   "RAH 3.3"
	static constexpr uint8_t m683_characteriser_prot[8] = { 0x03, 0xAF, 0x27, 0x8F, 0x0F, 0xA7, 0x27, 0x07 };


	// games with sequence starting 00 44 44 4c e0 d4 d8 2c e8 b8 (m4overmn)
	// Over The Moon   "OTT 0.2"
	//                 "OTU 0.1"
	static constexpr uint8_t otm_characteriser_prot[8] = { 0x00, 0x4C, 0x44, 0x04, 0x38, 0x74, 0x74, 0x20 };


	// games with sequence starting
	// 00 30 20 14 2c a0 54 24 3c 9c 9c 9c
	// Take Your Pick Club    "CTP 1.2"
	//                        "CTP 1.3"
	//                        "NTP 0.2"
	// Road Hog Club          "RHC 0.5"
	// Andy Capp              "C2T 0.2"
	//                        "C2T 0.1"
	//                        "C5T 0.1"
	// Make A Mint            "MAM 0.3"
	//                        "MMG 0.4"
	//                        "MMG 0.2"
	// Take Your Pick         "TAP 0.6"
	// Runaway Trail          "R4T 1.1"
	//                        "R4T 1.3"
	// Dutch Top Action       "TA 2.2" (only one set)
	// Dutch Hold On          "DHO 2.5"
	static constexpr uint8_t m441_characteriser_prot[8] = { 0x03, 0x3F, 0x33, 0x1F, 0x17, 0x3B, 0x33, 0x13 };
												   //    00    38    30    18    14    3C    34    14
												   //    00    38    30    18    10    38    30    14


	// games with sequence starting 00 c4 c4 44 c4 44 44 c4 cc 3c
	// this seems to be the same sequence as the MPU4 Video game 'Strike It Lucky' where we're having to use a 4k table for the question scramble
	// Road Hog 2 - I'm Back    "2RH 0.6"
	// Red Hot Roll Club        "RH2 1.1"
	//                          "RH2 1.0"
	// Dennis The Menace        "DM5 0.1"
	//                          "DMT 0.1"
	// Run For Your Money       "RUN 0.5"
	//                          "APR 0.1"
	//                          "RU8 0.1"
	// Prize Run For Your Money "PRU 0.2"
	//                          "RM8 0.1"
	// Dutch American Highway   "DAH 2.0"
	static constexpr uint8_t m462_characteriser_prot[8] = { 0x03, 0xC3, 0xC7, 0x4F, 0x47, 0xE7, 0xC7, 0x47 };
												   //    04    44    44    48    40    60    40    40   rhog2


	// games with sequence starting
	// 00 64 64 24 64 64 24 64 6C 9C BC
	// Lucky Strike Club          "LSC 1.0"
	// Winner Takes All           "WN5 0.1"
	//                            "WNT 0.1"
	// Jackpot Gems               "JGT 0.3"
	//                            "JGU 0.2"
	// Rich & Famous              "RAF 0.3"
	//                            "RF8 0.1"
	// Prize Rich & Famous        "PR8 0.1"
	// Lazy Bones                 "LBD 1.0"
	//                            "LBD 1.2"
	static constexpr uint8_t wta_characteriser_prot[8] = { 0x00, 0x64, 0x60, 0x28, 0x20, 0x70, 0x60, 0x20 };


	// games with sequence starting
	// 00 24 24 a4 4c 10 88 50 a8 d8 9c
	// Games Bond 006     "006 0.6"
	// Jackpot Gems       "CG4 0.7"
	//                    "CGT 0.3 / CG4 0.3"
	//                    "CGT 0.1 / CG4 0.1"
	//                    "CGU 0.2 / CG4 0.2"
	//                    "JG8 0.1"
	//                    "JAG 0.4"
	// Double 9's         "DU9 1.0"
	// Top Stop           "TSP 0.5"
	static constexpr uint8_t du91_characteriser_prot[8] = { 0x03, 0xAF, 0x27, 0x8F, 0x87, 0x2F, 0x27, 0x07 };
												   //    00    28    20    08    00    28    20    00    gb006


	// games with sequence starting 00 90 C0 54 8C 68 24 90 CC 6C 24 9C BC 34 88 6C
	// Supa Slot                "S4S 1.0"
	// Dutch Stardust           "DSD 1.3"
	// Road Hog                 "RR6 1.2"
	//                          "RR6 1.1"
	// Ace Chase                "AE5 3.0"
	//                          "AE10 3.0"
	//                          "AE  1.0"
	//                          "AE20 3.0"
	static constexpr uint8_t rr6_characteriser_prot[8] = { 0x00, 0x18, 0x10, 0x18, 0x10, 0x18, 0x10, 0x14 };


	// games with sequence starting 00 30 20 14 64 30 44 30 64 34 00 74 20
	// Dutch Turbo Reel Deluxe    "DTU 3.0"
	// Club Climber               "C1C 3.3"
	//                            "CC 4.5"
	// Reel 2 Reel                "RR  3.0"
	// Road Hog                   "RO_ 3.0"
	// Jackpot Gems               "JG3 0.1"
	static constexpr uint8_t rhm_characteriser_prot[8] = { 0x00, 0x70, 0x30, 0x50, 0x10, 0x70, 0x30, 0x10 };


	// games with sequence starting
	// 00 a0 a8 18 f4 2c 70 60 e4 e8
	// Dutch Samurai                 "DSM 1.0"
	// Jolly Taverner                "JT__ 2.0"
	// Ready Steady Go               "RSG 1.2"
	//                               "R4G 1.0"
	// Stop the Clock                "SC 2.5"
	// Viva Espana                   "EP8 0.1"
	//                               "ESP 0.3"
	//                               "ESP 0.2"
	// Viva Espana Showcase          "SE8 0.1"
	//                               "SES 0.2"
	// Prize Viva Espana             "PES 0.4"
	//                               "PE8 0.1"
	// Place Your Bets               "PYB 0.6"
	//                               "PYH 0.5"
	// Lucky Las Vegas               "LL3 0.1"
	//                               "LLT 0.3"
	//                               "LLU 0.1"
	// Fast Forward                  "SFF 3.0"
	// Ghost Buster                  "GB 5.0"
	//                               "GB 4.0"
	//                               "GB 3.0"
	//                               "GB 2.0"
	// Spotlight (Nova) (German)     "GSP 0.1"
	// Monaco Grand Prix (German)    "MGP 1.4"
	// Club X                        "CLX 1.2"
	// Dutch Magic Liner             "DMA 2.1"
	// Spend Spend Spend             "SP5 1.0" (uses different lamp scramble!)
	//                               "SP101.0" (uses different lamp scramble!)
	static constexpr uint8_t viva_characteriser_prot[8] = { 0x03, 0xE7, 0xA3, 0xC7, 0xC3, 0xA7, 0xA3, 0xC3 };
	// place your bets (same as above, but with unused lamp bits masked out, except for 0x24?)
	// static constexpr uint8_t viva_characteriser_prot[8] = { 0x00, 0x60, 0x20, 0x40, 0x40, 0x24, 0x20, 0x40 };
	// spend spend spend - This is unusual because the 2nd value DOES need to be different, bit 0x20 can't be set
	// or the 2nd reel will be in the wrong place.  Does this indicate the lamp reading is more complex than
	// we believe, or are there 2 parts with the same sequence and one value different?
	// clubx also needs this version instead
	static constexpr uint8_t viva_sss_characteriser_prot[8] = { 0x00, 0x40, 0x20, 0x40, 0x40, 0x20, 0x20, 0x40 };

	// games with sequence starting
	// 00 e0 a8 38 94 48 50 60 e4 e8 58 f0 68 14 60 c4 a8 5c f4 28 34 60 a0 e8 1c f0 68 30 04 c0 8c 1c b4 68 14 04 c0 a8 18 d0 0c 14 44 84 a8 5c d4 6c 74 04 c0 e8 38 d4 4c 14 44 e0 e8 3c b4 0c 74 00
	// Super Hyper Viper           "HVP 3.0"
	//                             "HVP 4.0"
	// Calamari Club               "CAC 0.3"
	//                             "CA3 0.1"
	// Hi Jinx                     "JNS 0.2"
	//                             "JNX 0.5"
	static constexpr uint8_t m407_characteriser_prot[8] = { 0x03, 0xC7, 0x83, 0xC3, 0xC3, 0xA3, 0x83, 0xC3 };


	// games with sequence starting
	// 00 24 24 2c 70 20 0c 60 3c 5c 5c 5c 7c 4c 68
	// Andy Loves Flo              "AL8 0.1"
	//                             "ALF 2.0"
	// Duty Free                   "DF5 0.3"
	//                             "DFT 0.1"
	//                             "XD5 0.2"
	//                             "XD5 0.1"
	//                             "XFT 0.1"
	// Give Us A Clue              "C20 0.2"
	//                             "C25 0.4"
	// Prize Spend Spend Spend     "PS3 0.2"
	// Crown Jewels Club           "CJC 1.5"
	//                             "CJN 0.2"
	// Crown Jewels Mk II Club     "CJ2 1.4"
	static constexpr uint8_t duty_characteriser_prot[8] = { 0x00, 0x38, 0x24, 0x18, 0x08, 0x34, 0x20, 0x00 };


	// games with sequence starting
	// 00 84 94 5c ec 3c ec 30 4c 68 60 cc
	// Andy's Big Time Club     "ABT 1.8"
	//                          "ABT 1.5"
	// Calamari Club            "BC3 0.2"
	//                          "BCA 0.4"
	//                          "BCA 0.2"
	// Duty Free                "DUT 0.4"
	//                          "DF8 0.1"
	// Showcase Duty Free       "SDF 0.2"
	//                          "SD8 0.1"
	// Prize Duty Free          "PDU 0.2"
	//                          "PD8 0.2"
	// Run For Your Money       "AP1 0.1"
	//                          "AP5 0.2"
	//                          "RU5 0.1"
	//                          "RUT 0.1"
	// Loads A Money            "LA 1.0"
	//                          "LA 1.1"
	// Dutch Show Timer         "DSH 1.3"
	// Dutch Megalink           "DML 2.0"
	// Club Connect             "CON 1.2"
	//                          "CON 1.1"
	//                          "CON 1.0"
	//                          "CON 1.5"
	static constexpr uint8_t andybt_characteriser_prot[8] = { 0x00, 0x48, 0x00, 0x48, 0x44, 0x08, 0x00, 0x00 };


	// games with sequence starting
	// 00 50 40 90 a8 6c c4 30 c8
	// Turbo Play                     "CTP 0.4"
	//                                "ZTP 0.7"
	// Golden Years (Nova) (German)   "TGY 0.1"
	// Chase Invaders                 "CI2 0.1"
	// Super Play (Czech)             "XSP 0.8"
	// Up Up and Away                 "UPS 2.1"
	//                                "UUA 2.1"
	// Andy Loves Flo                 "ALT 0.4"
	//                                "ALU 0.3"
	// Road Hog                       "RO_ 2.0"
	//                                "RO_ 1.0"
	// Dennis The Menace              "DEN 1.2"
	//                                "DM8 0.1"
	// Let The Good Times Roll        "GTA 0.1"
	//                                "GTR 1.0"
	//                                "GTS 0.1"
	//                                "GTK 0.2"
	// Eighth Wonder                  "WON 2.2"
	// Mirage                         "RAG 4.1"
	// Super Tubes                    "S4T 1.0"
	static constexpr uint8_t alf_characteriser_prot[8] = { 0x00, 0x58, 0x50, 0x1C, 0x10, 0x58, 0x50, 0x10 };
												  //    03    DB    53    9B    93    5B    53    13


	// games with sequence starting
	// 00 84 A4 AC 70 80 2C C0 BC 5C
	// California Club             "CA2 1.0"
	//                             "CAL 2.0"
	// Tropicana Club              "TRO 2.0"
	//                             "TR2 1.1"
	// Viva Espana                 "VE5 0.2"
	//                             "VET 0.2"
	// Berserk                     "BES 0.6"
	//                             "BE8 0.1"
	// Andy's Great Escape         "AN2 0.3"
	//                             "A28 0.1"
	// Winner Takes All            "WIN 0.6"
	//                             "WN8 0.1"
	// Prize Winner Takes All      "PWN 0.4"
	//                             "PW8 0.2"
	// Club Shuffle                "CSS 1.0"
	static constexpr uint8_t shuffle_characteriser_prot[8] = { 0x00, 0x18, 0x00, 0x18, 0x08, 0x10, 0x00, 0x00 };


	// games with sequence starting
	// 00 44 44 54 34 04 54 14 34 14 20 74 04 60
	// Andy's Great Escape   "AGC 2.0"
	// Everyone's A Winner   "ER2 0.1"
	//                       "ERT 0.2"
	// Nudge Banker          "SBN 2.0"
	//                       "NBN 1.0"
	//                       "SBN 1.1"
	// Club Vegas            "CLA 2.4"
	// Tic Tac Take          "TIC 2.0"
	// Dutch Nudge Up        "DNU 2.5"
	// Dutch Big Chief       "BCH 1.5"
	// Action Pack           "AP 0.4"
	//                       "AP 0.5"
	static constexpr uint8_t clbveg_characteriser_prot[8] = { 0x00, 0x70, 0x40, 0x70, 0x50, 0x60, 0x40, 0x40 };


	// games with sequence starting
	// 00 84 C4 E4 4C 10 28 90 E8 78 34
	// Viva Las Vegas          "VLV 1.1"
	//                         "VLV 1.0"
	// Cash Counter            "C3 2.4"
	//                         "C3 1.8"
	//                         "CO 0.5"
	//                         "C3 3.1"
	//                         "C3 2.0"
	// Dutch Black Jack Club   "DBC 1.1"
	// Jackpot Jump            "VJC 2.0"
	//                         "VJC 1.3"
	static constexpr uint8_t vivlv_characteriser_prot[8] = { 0x00, 0x28, 0x00, 0x28, 0x20, 0x08, 0x00, 0x00 };
													//    00    28    00    28    24    14    00    00   m4jpjmp has this lamp scramble for it


	// games with sequence starting  00 84 8c b8 74 80 1c b4 d8 74 00 d4 c8 78 a4 4c e0 dc f4 88
	// Celebration Club       "CEL 1.5"
	// Dutch Jolly Joker      "DJJ 1.5"
	// Night Spot Club        "NS2 2.2"
	//                        "NIT 1.1"
	static constexpr uint8_t celclb_characteriser_prot[8] = { 0x00, 0x50, 0x00, 0x50, 0x10, 0x40, 0x04, 0x00 };


	// games with sequence starting 00 14 10 C0 8C A8 68 30 D0 58 E4 DC F4
	// Nudge Nudge Wink Wink    "NNU 5.2"
	//                          "NNU 4.0"
	// Cash Matrix              "CM 1.7"
	static constexpr uint8_t cashmx_characteriser_prot[8] = { 0x04, 0x50, 0x10, 0x60, 0x60, 0x30, 0x30, 0x14 };


	// games with sequence 00 14 04 94 c8 68 a0 18 f4 8c e8 ec ac a8 6c 20 54 c4 dc
	// Nickelodeon        "NIL 4.1"
	// Dutch Black Jack   "BJ 1.6"
	// Viz                "VIZ 0.6"
	//                    "VIZ 0.3"
	//                    "VIZ 0.2"
	static constexpr uint8_t viz_characteriser_prot[8] = { 0x00, 0x50, 0x10, 0x54, 0x14, 0x50, 0x10, 0x14 };


	// games with sequence
	// 00 e4 ec f8 54 08 d0 80 44 2c 58 b4 e8 b0 80
	// Nifty Fifty                     "NF 2.0"
	//                                 "NF 2.1"
	// Supa Silva                      "SS2V 1.0"
	// Dutch Ambassador                "DAM 3.7"
	// Nudge Nudge Wink Wink Classic   "CNN 2.0"
	static constexpr uint8_t nifty_characteriser_prot[8] = { 0x03, 0xE7, 0xA7, 0x87, 0xE7, 0x07, 0xA7, 0xE7 };


	// games with sequence
	// 00 84 a4 e4 b0 34 54 44 d4 64 80 f4 24 80 f4 20
	// Millionaire's Club     "MI2 1.0"
	//                        "MIL 5.0"
	// Cash Attack            "CSA 1.1"
	//                        "CAA 2.2"
	// Ten Out Of Ten         "TOC 0.3"
	//                        "TOT 0.5"
	//                        "TOC 0.1"
	static constexpr uint8_t milclb_characteriser_prot[8] = { 0x00, 0x54, 0x00, 0x54, 0x40, 0x10, 0x00, 0x00 };


	// games with sequence 00 44 44 c4 58 60 c0 50 8c b8 e0 dc ec b0 1c e8 38
	// Nudge Quest             "NQ 2.0"
	// Fruit Full Club         "FFC 0.3"
	//                         "FFC 1.0"
	//                         "FFC 1.2"
	// Fruit Link Club         "FLC 1.8"
	//                         "FLC 1.6"
	// Octopus (Nova) (German) "OCT 0.3"
	// Jolly Gems              "GEM 0.5"
	//                         "GEM 0.6"
	//                         "GMS 0.4"
	//                         "GMS 0.3"
	static constexpr uint8_t fruitfall_characteriser_prot[8] = { 0x03, 0xCF, 0x47, 0xCB, 0xC3, 0x4F, 0x47, 0x43 };


	// games with sequence
	// 00 60 68 bc d0 2c 94 20 e4 e8 bc f0 88 34 a0 c4 ec bc f4
	// Multiplay Club          "MP 2.8"
	// Stake Up Club           "SU 4.4"
	//                         "SU 4.8"
	// Dutch Hold Timer        "DHT 1.0"
	// Extra Game              "CEG 2.0"
	// Dutch Tomahawk          "DTK 2.3"
	// Dutch Top Timer         "DTT 1.8"
	// Dutch Universe          "DUN 2.0"
	// Fruit Game              "FRU 2.0"
	static constexpr uint8_t m400_characteriser_prot[8] = { 0x03, 0xE7, 0x43 ,0xC3, 0xC3 ,0xE3, 0x43, 0xC3 }; // does anything using this have lamp scramble or was this extracted with tests?


	// games with sequence
	// 00 bc b8 fc bc dc fc fc fc f8 d8 b8 f8 d8 fc bc fc 98 fc f8 f8
	// Interceptor         "INT 3.0"
	// Omega               "DOM 2.3"
	// Grab The Bank       "G4B 2.0"
	//                     "G4B 2.1"
	// Smash 'n' Grab      "SAG 1.0"
	//                     "SAG 3.4"
	// Sun Club            "SUC 0.2"
	// Dutch Top Deck      "DT 2.6"
	static constexpr uint8_t intcep_characteriser_prot[8] = { 0x00, 0x1C, 0x38, 0x78, 0x7C, 0x78, 0x38, 0x7C };


	// games with sequence
	// 00 50 40 14 C4 B0 A4 30 C4 74 00 D4 E0 30 C0 34
	// Top Tenner            "TP 2.7"
	// Take Two              "TTO 1.2"
	// Dutch First Class     "DFC 2.0"
	// Red Hot Roll          "CR4 0.9"
	//                       "CRT 0.3 / CR4 0.3"
	//                       "CRU 0.1"
	//                       "RH8 0.1"
	//                       "RHR 0.3"
	//                       "RHT 0.3"
	//                       "RHU 0.2"
	//                       "RHR 5.0"
	//                       "RHR 2.0"
	// Rich & Famous         "RFT 0.2"
	//                       "RF5 0.2"
	// Prize High Roller     "PRL 0.3"
	static constexpr uint8_t take2_characteriser_prot[8] = { 0x00, 0x50, 0x50, 0x10, 0x10, 0x50, 0x50, 0x00 };


	// games with sequence
	// 00 50 40 14 4C 80 34 44 5C 9C 9C 9C DC 9C DC 94
	// Alphabet             "A4B 1.0"
	// Dutch Voodoo 1000    "DDO 3.2"
	// Monte Carlo          "NM8 0.1"
	//                      "NMN 0.1"
	// Prize Monte Carlo    "MSS 1.6"
	//                      "MC 53.0"
	//                      "MC103.0"
	static constexpr uint8_t m435_characteriser_prot[8] = { 0x03, 0x5F, 0x53, 0x1F, 0x17, 0x5B, 0x53, 0x13 };


	// games with sequence 00 84 8C D8 74 80 4C 90 E8 78 54 60 84
	// Adders & Ladders Classic Club  "ADC 1.1"
	//                                "ADC 0.5"
	// Cash Zone                      "CAZ 1.2"
	//                                "CAZ 1.5"
	// Escalera Tobogan               "ESC1"
	static constexpr uint8_t m578_characteriser_prot[8] = { 0x00, 0x60, 0x00, 0x60, 0x40, 0x20, 0x00, 0x00 };


	// games with sequence
	// 00 c0 c8 1c f4 68 14 50 70 50 20 f0 48 34 60
	// Hyper Viper Club             "HPC 0.5"
	// Mega Bucks                   "BUC 4.1"
	//                              "BUC 3.1"
	// Action Bank                  "AC3.0"
	//                              "ACT2.0"
	// Dutch Road Runner            "DRO 1.9)
	// Dutch Gun Smoke              "DGU 1.6"
	// Andy Capp                    "ACC52.0"
	// Andy's Great Escape          "A2T 0.1"
	//                              "A5T 0.1"
	static constexpr uint8_t age_characteriser_prot[8] = { 0x00, 0x74, 0x44, 0x34, 0x14, 0x64, 0x44, 0x00 };


	// games with sequence starting 00 60 60 C0 58 44 E0 50 A8 9C CC BC E4 50 A0 58
	// Sunset Boulevard            "SBU 2.0"
	// Blackjack Super Multi       "SM H1.6"
	static constexpr uint8_t sunsetb_characteriser_prot[8] = { 0x03, 0xEB, 0x63, 0xCB, 0xC3, 0x6B, 0x63, 0x43 };


	// games with sequence starting 00 60 60 a0 38 64 e0 30 c8 9c ac dc ec 94 d8 a4 38 ec
	// Blackjack Club   "C2J 1.8"
	//                  "C2J 2.1"
	static constexpr uint8_t bjac_characteriser_prot[8] = { 0x00, 0x68, 0x60, 0x28, 0x20, 0x68, 0x60, 0x20 };


	// games with sequence starting 00 88 70 14 1c c0 a4 a0 bc d4 30 14 18 d4 2c 50 1c
	// Magnificent 7s               "MAS 1.2"
	//                              "MA7 1.4"
	//                              "MA7 1.5"
	// Crazy Casino (Nova) (German) "CRZ 0.3"
	static constexpr uint8_t mag7s_characteriser_prot[8] = { 0x03, 0x9F, 0x0F, 0x17, 0x03, 0x1B, 0x8F, 0x87 };


	// games with sequence starting 00 44 44 54 d0 88 38 74 d0 58
	// Ooh Aah Dracula           "DR_ 2.0"
	//                           "DR_ 2.1"
	// Super Streak Classic      "CSS 6.0"
	//                           "CSS 5.0"
	//                           "CSS 2.0"
	static constexpr uint8_t oad_characteriser_prot[8] = { 0x00, 0x50, 0x44, 0x14, 0x14, 0x44, 0x44, 0x00 };


	// games with sequence starting 00 18 70 24 38 58 74 0c 6c 64
	// Rocky Horror Show   "RH__4.0"
	//                     "RH__6.0"
	static constexpr uint8_t rhs_characteriser_prot[8] = { 0x00, 0x3C, 0x18, 0x30, 0x10, 0x3C, 0x18, 0x10 };




	// games with sequence starting 00 0c 50 90 b0 38 d4 a0 bc d4 30 90 38 c4 ac 70
	// Gamball    "GBB 2.0"
	//            "GAB 2.0"
	static constexpr uint8_t gambal_characteriser_prot[8] = { 0x00, 0x18, 0x08, 0x10, 0x00, 0x18, 0x08, 0x00 };


	// Games with sequence starting
	// 00 60 60 44 e0 e8 1c 74 a4 6c 14 84 e8 1c f4 08 b0 ac bc d0 8c 9c f0 28 b0 8c 9c d0 08 14 00 44 e0 68 b0 08 94 88 9c 54 00 60 e4 ec 98 54 a4 e8 bc 54 a4 ec b8 d4 ac 98 d4 8c bc d0 8c 38 f4 00
	// used by
	// Classic Adders & Ladders  "A6L 0.1"
	// Luxor                     "LUX 0.6"
	// Prize Luxor (Barcrest)    "PLX 0.2"
	// Double Up                 " DU 1.5"
	static constexpr uint8_t addr_characteriser_prot[8] = { 0x00, 0x60, 0x60, 0x40, 0x40, 0x60, 0x60, 0x40 }; // match output of unprotected bootlegs


	// Games with sequence starting
	// 00 a0 88 38 94 2c 30 00 e4 c8 18 b4 4c 30 20 c0 ec 7c f4 48 50 24 c0 a8 58 f4 08 10 20 c0 e8 7c f0 08 34 64 80 ac 58 90 68 14 64 c0 ec 78 b4 28 74 40 c0 ac 3c d4 4c 74 40 e4 c8 7c b4 08 74 00
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
	// 00 90 88 4c e0 b8 74 84 bc 74 00 b4 88 6c c0 1c f4 ac ec 68 c4 1c f0 88 ec ec 6c c0 98 74 00 14 a0 b8 74 00 94 28 60 90 08 60 b4 ac 68 c0 1c f4 ac 4c c4 bc f0 8c cc 4c 44 b4 88 68 44 90 ac 00
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
	//                    "PO  1.1"
	// Fortune Club       "CFO 1.2"
	static constexpr uint8_t fortune_characteriser_prot[8] = { 0x00, 0x70, 0x10, 0x60, 0x20, 0x50, 0x10, 0x00 }; // guessed based on lamp positions in Pot Black Casino and Jewel in the Crown


	// Games with sequence starting
	// 00 14 04 54 c4 98 f0 48 e4 5c f0 c8 ec 68 24 90 c8 ec ec a8 ec 28 20 14 c4 dc f0 8c 2c 64 14 40 94 cc a8 68 e0 9c f0 48 24 14 c4 d8 f0 48 60 14 c4 98 f0 c8 a8 e8 68 20 54 c0 9c b0 c8 2c e4 00
	// used by
	// Kings & Queens    "EE  2.0"
	//                   "EE  1.0"
	// Lucky Strike      "LSS 0.6"
	//                   "LST 0.9"
	// Solid Silver Club "SOS 2.2"
	// Solid Silver Club "SOS 2.1"
	static constexpr uint8_t luckystrike_characteriser_prot[8] = { 0x00, 0x50, 0x10, 0x50, 0x50, 0x10, 0x10, 0x10 }; // Lucky Strike matches unprotected sets


	// Games with sequence starting
	// 00 14 04 94 a8 6c c4 30 8c e8 e0 bc d4 28 4c c0 38 dc dc 58 d0 a0 30 04 9c d8 d8 58 50 88 64 94 2c 4c 4c c8 e8 6c 40 14 00 14 8c e8 68 44 94 ac ec e0 b0 a8 6c c8 e0 30 84 bc d4 24 94 20 9c 00
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
	// 00 44 44 64 4c 80 70 24 6c a8 b0 38 e4 50 24 48 8c bc ec 98 e8 90 18 ec 94 38 e8 9c c0 70 00 60 44 64 4c a8 b8 cc b0 30 00 44 6c a4 58 e4 74 04 6c a0 70 28 84 74 2c 88 b8 e8 b4 14 2c 84 7c 00
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
	// 00 14 10 a0 8c c8 68 50 b0 38 64 b4 18 e4 1c e4 8c f8 f4 18 64 04 14 90 b8 e4 3c 54 00 24 14 20 14 b0 a8 68 e0 9c e4 0c 50 10 b0 b8 e4 0c 70 10 b0 08 60 b4 98 e4 3c c4 ac f8 74 10 a0 1c f4 00
	// used by
	// Club Classic      "CI  1.1"
	// Dutch Atlantis    "DAT 1.4"
	// Dutch Twin Timer  "D2T 1.1"
	static constexpr uint8_t m533_characteriser_prot[8] = { 0x00, 0x30, 0x10, 0x20, 0x20, 0x10, 0x10, 0x00 }; // based on clbcls lamp patterns


	// games with sequence starting
	// 00 60 60 24 e0 e8 1c 74 c4 6c 14 84 e8 1c f4 08 d0 cc dc b0 cc 1c f0 48 54 84 6c 50 00 24 60 64 60 60 e0 28 94 88 9c 34 00 60 e4 ec 98 34 c4 6c d4 88 1c b4 c8 9c f4 08 14 84 e8 98 b4 08 d4 00
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


	// games with sequence starting
	// 00 14 04 34 2c 44 34 24 3c 78 70 28 64 10 2c 48 4c 6c 6c 48 60 10 08 6c 44 3c 78 54 00 30 00 30 04 3c 7c 70 24 14 28 44 14 04 3c 74 2c 44 34 0c 6c 60 30 28 44 34 2c 48 60 3c 74 04 34 00 3c 00
	// used by
	// Sunset Club    "SSC 3.0"
	// Jolly Gems     "JGS 1.0"
	static constexpr uint8_t sunsetclub_characteriser_prot[8] = { 0x00, 0x38, 0x10, 0x38, 0x30, 0x18, 0x10, 0x10 }; // matches Jolly Gems with the unprotected sets


	// games with sequence starting
	// 00 18 c8 a4 0c 80 0c 90 34 30 00 58 c8 84 4c a0 4c c0 3c c8 a4 4c 80 0c 80 0c e0 1c 88 a4 0c a0 0c 80 4c a0 3c 98 ec 84 0c c0 1c a8 84 0c a0 5c e8 a4 0c d0 04 38 a8 c4 2c 90 44 18 e8 84 3c 00
	// used by
	// Double Diamond Club  "CDD 0.5"
	//                      "CDD 0.1"
	static constexpr uint8_t doublediamond_characteriser_prot[8] = { 0x00, 0x18, 0x18, 0x28, 0x08, 0x58, 0x18, 0x08 }; // based on Double Diamond Club lamp patterns


	// games with sequence starting
	// 00 30 10 0c 58 60 24 30 1c 6c 44 3c 74 00
	// The Streak "TS  3.0"
	//            "TST 3.0 / TS  3.0"
	//            "TST 2.0 / TS  2.0"
	//            "TS  1.4"
	//            "TS  1.3"
	static constexpr uint8_t thestreak_characteriser_prot[8] = { 0x00, 0x38, 0x30, 0x28, 0x20, 0x38, 0x30, 0x20 }; // some elements don't go through the scramble!


	// games with sequence starting
	// 00 90 a0 34 8c 68 44 90 ac 6c 44 9c dc 5c d4 24 98 dc dc 58 54 04 90 a8 e8 ec 6c c0 b0 2c c0 3c d4 a4 38 54 84 38 58 d4 20 90 ac ec 68 c0 34 88 ec 64 94 2c c0 b4 ac 68 c4 bc d4 a0 34 00 9c 00
	// used by
	// Viz          "VZ__1.0"
	// Gold Strike  "G4S 2.0" (doesn't use lamp scramble)
	// Magic Dragon "DMD 1.0" (doesn't use lamp scramble? doesn't boot)
	static constexpr uint8_t magicdragon_characteriser_prot[8] = { 0x00, 0x18, 0x10, 0x18, 0x10, 0x18, 0x10, 0x10 }; // verified against Viz


	// games with sequence starting
	// 00 90 88 2c e0 d8 74 84 dc 74 00 d4 c8 6c a0 58 f4 cc ec 68 a4 58 70 80 dc 74 84 58 30 44 90 4c e4 98 74 00 94 48 24 90 08 60 d4 4c e0 18 74 80 dc 74 c4 dc 70 84 9c 74 84 dc f4 88 2c a0 dc 00
	// Top The Lot  "T4L 1.0" (doesn't use lamp scramble)
	// Monte Carlo  "MX052.0"
	//              "MX102.0"
	static constexpr uint8_t toplot_characteriser_prot[8] = { 0x00, 0x50, 0x10, 0x50, 0x10, 0x50, 0x10, 0x10 }; // maybe


	// games with sequence starting
	// 00 14 04 94 e0 74 a4 50 a4 d4 60 b4
	// Monte Carlo   "MC 2.0"
	static constexpr uint8_t montealt_characteriser_prot[8] = { 0x00, 0x30, 0x10, 0x30, 0x10, 0x30, 0x10, 0x10 }; // maybe


	// games with sequence starting
	// 00 50 10 a0 68 14 b0 88 bc e4 48
	// Prize Monte Carlo "MT054.0"
	//                   "MT104.0"
	static constexpr uint8_t przmontealt_characteriser_prot[8] = { 0x00, 0x70, 0x50, 0x60, 0x40, 0x70, 0x50, 0x40 }; // maybe


	// games with sequence starting
	// 00 44 44 4c d0 30 18 cc f8 9c 9c 9c dc 9c
	// Andy's Full House    "AFU 0.2"
	// Say No More          "SNM 2.0"
	static constexpr uint8_t saynomore_characteriser_prot[8] = { 0x00, 0x58, 0x40, 0x18, 0x08, 0x50, 0x40, 0x00 }; // good?


	// games with sequence starting
	// 00 30 20 14 a4 f0 c4 50 a4 74 00 b4 60 10
	// Andy Loves Flo  "AL3 0.1"
	static constexpr uint8_t andyfloalt_characteriser_prot[8] = { 0x00, 0x30, 0x30, 0x10, 0x10, 0x30, 0x30, 0x10 }; // good?


	// games with sequence starting
	// 00 44 44 64 4c 10 28 50 68 38 34 28 70 00 6c 10 68
	// Andy Loves Flo   "AL4 2.1"
	// Andy Loves Flo   "AL_ 2.4"
	// Dutch Cherryo    "DCH 1.4"
	static constexpr uint8_t cheryo_characteriser_prot[8] = { 0x00, 0x68, 0x40, 0x28, 0x20, 0x48, 0x40, 0x00 }; // good?


	// games with sequence starting
	// 00 14 04 54 4c 20 50 44 5c 78 70 48 6c 60 14 48 2c
	// Andy Capp    "AC101.0"
	//              "AC5 1.0"
	static constexpr uint8_t andycappalt_characteriser_prot[8] = { 0x00, 0x58, 0x10, 0x58, 0x50, 0x18, 0x10, 0x10 }; // good?


	// games with sequence starting
	// 00 84 a4 b4 38 c4 b4 30 1c d8 d8 d8 dc
	// Luxor      "LX5 1.0"
	//            "LX101.0"
	// Hot Rod    "HR__1.0"
	static constexpr uint8_t hotrodalt_characteriser_prot[8] = { 0x00, 0x18, 0x00, 0x18, 0x10, 0x08, 0x00, 0x00 }; // good?


	// games with sequence starting
	// 00 30 20 14 a4 b8 d4 0c e4 3c 54 84 b8 54 24 90
	// Viva Espana  "VE105.0"
	//              "VE5 4.0"
	//              "VE104.0"
	static constexpr uint8_t vivaalt_characteriser_prot[8] = { 0x00, 0x30, 0x30, 0x10, 0x10, 0x30, 0x30, 0x10 }; // good?


	// games with sequence starting
	// 00 44 44 c4 68 14 8c 30 8c b8 d0 a8 b4 20 4c 90 a8 bc dc 58 d0 a0 60 44 c4 ec b8 5c 50 88 34 84 6c 14 8c b8 d8 58 d0 a0 64 44 cc b8 d8 d0 a8 3c dc d8 d8 d8 58 d8 d8 d0 a8 bc d4 24 c4 64 cc 00
	// Lucky Las Vegas Classic "LLU 0.1"
	//                         "LLU 3.0"
	// Cash Encounters         "CA_ 5.0"
	static constexpr uint8_t cashencounters_characteriser_prot[8] = { 0x00, 0x48, 0x40, 0x48, 0x40, 0x48, 0x40, 0x40 }; // maybe


	// games with sequence starting
	// 00 50 10 84 c8 a8 2c 30 94 1c e4 dc f4
	// Lucky Strike    "LSS 1.0"
	static constexpr uint8_t luckystrikealt_characteriser_prot[8] = { 0x00, 0x50, 0x50, 0x40, 0x40, 0x50, 0x50, 0x40 }; // matches unprotected set


	// games with sequence starting
	// 00 14 04 54 64 14 64 14 64 54 20 74 04
	// Super Hyper Viper   "HVC 1.0"
	static constexpr uint8_t hypvipalt_characteriser_prot[8] = { 0x00, 0x70, 0x10, 0x70, 0x50, 0x30, 0x10, 0x10 }; // good?


	// games with sequence starting
	// 00 e0 88 18 b0 48 50 60 e4 c8 58 f0 08
	// Boulder Dash        "BLS 0.1"
	//                     "BLD 0.6"
	//                     "BLD 0.7"
	// Crazy Cavern (Nova) "GCV 0.5"
	static constexpr uint8_t bdash_characteriser_prot[8] = { 0x00, 0x20, 0x00, 0x40, 0x40, 0x20, 0x00, 0x40 }; // good?


	// games with sequence starting
	// 00 e0 a8 38 90 68 30 60 e4 e8 18 d0 6c 10 60 c4
	// Cloud 999     "SC9 5.0"
	// Hit The Top    "H4T 2.0"
	static constexpr uint8_t hittop_characteriser_prot[8] = { 0x00, 0x60, 0x00, 0x40, 0x40, 0x20, 0x00, 0x40 }; // good?


	// games with sequence starting
	// 00 24 24 a4 1c 44 a4 14 a8 d8 cc f8 e4 14
	// Winner Takes All  "WN4 1.1"
	//                   "WN5 3.0"
	static constexpr uint8_t wtaalt_characteriser_prot[8] = { 0x00, 0x28, 0x20, 0x08, 0x00, 0x28, 0x20, 0x00 }; // good? matches unprotected set


	// games with sequence starting
	// 00 30 20 50 68 24 70 60 78 5c 5c 5c 7c 54 60 10 68 6c
	// Blue Flash  "TBF 0.3"
	//             "BFL 0.3"
	static constexpr uint8_t blueflash_characteriser_prot[8] = { 0x00, 0x78, 0x30, 0x58, 0x50, 0x38, 0x30, 0x10 }; // not much evidence


	// games with sequence starting 00 60 60 24 68 c0 34 44 6c 8c 9c ac d4 18 ec 90 1c
	// Dutch Triple Bank   "DTB 1.2"
	static constexpr uint8_t tribank_characteriser_prot[8] = { 0x00, 0x68, 0x60, 0x28, 0x20, 0x68, 0x60, 0x20 }; // many effects bypass the scramble, mostly correct?


	// games with sequence starting
	// 00 30 10 44 70 10 44 30 54 14 40 74 10 40 34
	// Tutti Fruity   "TFT 0.4 / TF4 0.4"
	//                "CTU 0.1"
	//                "TF4 0.2"
	// 10 X 10        "T20 0.2"
	//                "N25 0.3"
	//                "T25 0.4"
	// Cosmic Casinos "CC__3.0"
	//                "CC__7.0"
	// Dutch Express  "DXP 2.0"
	static constexpr uint8_t cosmiccasino_characteriser_prot[8] = { 0x00, 0x70, 0x30, 0x60, 0x20, 0x70, 0x30, 0x20 }; // good?


	// games with sequence starting
	// 00 14 10 c0 4c 20 84 0c f0 98 e4 dc f4 08 f0 08 70 d0 d8 64 94 c8 28 b0 58 e4 9c 64 14 d0 08 a0 1c f4 c8 e8 e8 78 24 14 00 14 d0 c8 68 30 c0 9c f4 08 a0 dc 64 94 d8 a4 dc e4 1c 64 94 08 f0 00
	// Black Jack        "B2J 2.2"
	//                   "BLA 2.0"
	// Prize High Roller "PR3 0.1"
	// Dutch Andy Capp   "DAC 1.3" (doesn't use lamp scramble? doesn't boot)
	// Dutch Step Timer  "DST 1.1" (doesn't use lamp scramble? doesn't boot)
	// Dutch Broadway    "DBR 1.1" (doesn't use lamp scramble? doesn't boot)
	static constexpr uint8_t phr_characteriser_prot[8] = { 0x00, 0x50, 0x10, 0x40, 0x00, 0x50, 0x10, 0x00 }; // good?


	// games with sequence starting
	// 00 14 10 a0 c4 c4 74 30 b0 70 00 b4 50 80 f4 40 94
	// Pontoon Club          "PON 3.0"
	//                       "PON 4.0"
	// Dutch Twilight        "DTL 2.2"
	static constexpr uint8_t pontoon_characteriser_prot[8] = { 0x00, 0x30, 0x10, 0x20, 0x20, 0x10, 0x10, 0x00 }; // maybe


	// games with sequence starting
	// 00 90 84 b4 2c c0 34 a0 bc 78 70 28 e0 14 a8 4c c8 ec
	// Eighth Wonder     "BEW 0.3"
	// Sunset Boulevard  "BSB 0.4"
	//                   "BSB 0.3"
	static constexpr uint8_t eighth_characteriser_prot[8] = { 0x00, 0x38, 0x10, 0x38, 0x30, 0x18, 0x10, 0x10 }; // good? matches other Eighth Wonder set


	// games with sequence starting
	// 00 44 44 54 1c 60 50 14 1c 78 70 18 7c 70 04 58 7c 7c
	// Sunset Boulevard    "BS__ 1.1"
	//                     "BS__ 1.0"
	static constexpr uint8_t sunsetbalt_characteriser_prot[8] = { 0x00, 0x58, 0x40, 0x58, 0x50, 0x48, 0x40, 0x40 }; // good? matches other Sunset sets


	// games with sequence starting
	// 00 e0 ac 1c 90 2c 14 40 e4 ec 18 f4 68 10 40 c4
	// Sunset Boulevard "SB__ 1.1"
	//                  "SB__ 1.0"
	// The Hit          "DTH 1.7"
	// Top Take         "TTK 1.1"
	static constexpr uint8_t toptake_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x40, 0x40, 0x20, 0x00, 0x40 }; // good? matches other sunset sets


	// games with sequence starting
	// 00 18 70 44 58 30 44 18 7c 74 00 5c 7c 34 48 24 58 7c 7c 70 0c 64 18 7c 38 74 0c 20 18 74 00 14 48 2c 68 24 1c 30 40 18 30 08 6c 2c 68 28 24 58 7c 34 4c 6c 28 64 1c 74 0c 6c 2c 60 14 00 5c 00
	// Tic Tak Cash " TC 1.1" (doesn't use lamp scramble)
	// Cloud 999    "CLN 4.0"
	// Boulder Dash "BO_ 2.0"
	// Boulder Dash "BO_ 2.1"
	static constexpr uint8_t tictak_characteriser_prot[8] = { 0x00, 0x58, 0x18, 0x50, 0x10, 0x58, 0x18, 0x10 }; // good?


	// games with sequence starting
	// 00 24 24 34 b0 a8 58 74 b0 38 54 90
	// Hit The Top  "HTT 0.5"
	static constexpr uint8_t hittopalt_characteriser_prot[8] = { 0x00, 0x30, 0x20, 0x10, 0x10, 0x20, 0x20, 0x00 }; // good?


	// games with sequence starting
	// 00 84 a4 e4 a8 3c dc d0 6c 58 d8 d8 dc d8 d4 60
	// Make A Mint      "MA_ 3.1"
	//                  "MA_ 3.0"
	static constexpr uint8_t mintalt_characteriser_prot[8] = { 0x00, 0x48, 0x00, 0x48, 0x40, 0x08, 0x00, 0x00 }; // lamp patterns quite different to other sets, but good?


	// games with sequence starting
	// 00 24 24 64 2c 14 4c 14 4c 58 78 78 74 00 6c 10 4c 5c 7c 38 74 00 20 24 2c 5c 78 34 00 60 24 64 24 2c 1c 78 78 34 08 14 00 24 6c 54 4c 14 4c 54 4c 58 78 78 34 44 64 20 64 6c 54 04 64 24 6c 00
	// Kings & Queens Classic     "CN4 / CN4 6.0"
	//                            "CN4 5.0"
	//                            "CN  / CN  3.0"
	//                            "CN  / CN  2.0"
	//                            "CN  1.4"
	//                            "CNT / CN4 2.0"
	//                            "CNT / CN4 1.0"
	// Andy Capp                  "AN8 0.1"
	//                            "AND 0.4"
	//                            "AND 0.2"
	// Pot Black                  "PBG 1.4"
	//                            "PBG 1.5"
	//                            "PBS 0.4"
	// Action Club                "A2C 1.1"
	// Nile Jewels (German)       "GJN 0.8"
	// Oriental Diamonds (German) "RAB 0.1"
	static constexpr uint8_t actclba_characteriser_prot[8] = { 0x00, 0x68, 0x20, 0x68, 0x60, 0x28, 0x20, 0x20 }; // good?


	// games with sequence starting
	// 00 60 60 44 68 a0 54 24 6c 8c 9c cc bc c4 74 00 68 ac bc c8 9c cc b0 30 28 ac 9c c8 b0 1c c0 5c ec b0 1c cc 9c c8 94 14 00 60 6c a4 58 e4 74 20 6c 84 74 0c 80 74 2c 88 9c cc bc c8 94 10 2c 00
	// Top Dog                   "TD4 7.1 / TD  7.1"
	//                           "TDT 7.1 / TD  7.1"
	//                           "TDT 8.3 / TD  8.3"
	//                           "TDP 2.0 / TD  2.0"
	//                           "TD  1.4"
	// Copy Cat                  "CO  1.1"
	//                           "CO  4.1"
	//                           "CO  1.3"
	//                           "CO  2.0"
	//                           "CO  3.0"
	//                           "CO  2.4"
	//                           "CO  4.0"
//  static constexpr uint8_t copycat_characteriser_prot[8] = { 0x00, 0x68, 0x60, 0x30, 0x38, 0x10, 0x18, 0x38 }; // rows 4-8 could be reversed, little evidence
	static constexpr uint8_t copycat_characteriser_prot[8] = { 0x00, 0x68, 0x60, 0x48, 0x40, 0x68, 0x60, 0x40 }; //


	// games with sequence starting
	// 00 50 40 14 c4 98 b4 0c e4 5c b4 8c ec
	// Ace Chase     "AE5 2.0"
	//               "AE10 2.0"
	static constexpr uint8_t acechasealt_characteriser_prot[8] = { 0x00, 0x50, 0x50, 0x10, 0x10, 0x50, 0x50, 0x10 }; // good?


	// games with sequence starting
	// 00 84 c4 d4 58 24 94 50 98 3c 34 18 bc 34 88 78 bc
	// Buccaneer    "BR_ 1.0"
	static constexpr uint8_t bucalt_characteriser_prot[8] = { 0x00, 0x18, 0x00, 0x18, 0x10, 0x08, 0x00, 0x00 }; // good?


	// games with sequence starting
	// 00 24 24 a4 4c 10 c0 0c f0 a8 98
	// Bagatelle           "BGT 0.5"
	//                     "BG2 0.1"
	// Hi Lo Casino (Nova) "HNC 0.2"
	static constexpr uint8_t bagtel_characteriser_prot[8] = { 0x00, 0x60, 0x20, 0x40, 0x00, 0x60, 0x20, 0x00 }; // good?


	// games with sequence starting
	// 00 50 40 30 68 44 70 60 78 3c 34 28 64 10
	// Berserk        "BE3 0.1"
	// Flashlite      "BFL 0.5"
	static constexpr uint8_t berseralt_characteriser_prot[8] = { 0x00, 0x78, 0x50, 0x38, 0x30, 0x58, 0x50, 0x10 }; // good?


	// games with sequence starting
	// 00 a0 e0 c4 c8 58 9c 94 6c 1c 9c 9c bc 94 6c 10
	// Berserk  "BE4 1.1"
	static constexpr uint8_t berseralt2_characteriser_prot[8] = { 0x00, 0x28, 0x20, 0x08, 0x00, 0x28, 0x20, 0x00 }; // good?


	// games with sequence starting
	// 00 14 10 48 38 34 58 74 58 6c 60 5c 7c 64 14
	// Magnificent 7s       "M7  2.0"
	// Prize Fruit & Loot   "PFR 0.3"
	static constexpr uint8_t pfloot_characteriser_prot[8] = { 0x00, 0x58, 0x10, 0x48, 0x40, 0x18, 0x10, 0x00 }; // good?


	// games with sequence starting
	// 00 84 8c 3c f4 4c 34 24 e4 ac 38 f0 0c 70 04
	// Kings & Queens           "EE4 2.1/ EE  2.1"
	//                          "EE8 2.2/ EE  2.2"
	// Dutch Multiway           "DMU 1.7"
	static constexpr uint8_t kingqn_characteriser_prot[8] = { 0x00, 0x60, 0x00, 0x60, 0x20, 0x40, 0x00, 0x00 }; // good?


	// games with sequence starting
	// 00 18 b0 64 38 98 b4 44 3c b4 40 3c 9c b4
	// Kings & Queens  "EE' 2.0 / EE_ 2.0"
	static constexpr uint8_t kqee_characteriser_prot[8] = { 0x00, 0x38, 0x18, 0x30, 0x10, 0x38, 0x18, 0x10 }; // good?


	// games with sequence starting
	// 00 84 c4 d4 70 04 94 50 34 14 20 b4 44 a0 e4
	// Action Club      "ABV 1.9"
	// Kings & Queens   "EE2 1.0 / EE2 1.0"
	static constexpr uint8_t kqee2_characteriser_prot[8] = { 0x00, 0x30, 0x00, 0x30, 0x10, 0x20, 0x00, 0x00 }; // good?


	// games with sequence starting
	// 00 8c 64 84 84 c4 84 84 9c f4 04 cc 24 84 c4 94 54 0c 74 0c 34 04 84 84 c4 84 9c e4 84 84 84 d4 44 84 c4 84 9c e4 84 84 84 8c 60 84 84 84 84 c4 9c f4 04 cc 24 9c f4 04 94 14 44 8c 34 04 9c 00
	// Bank Roller Club         "CBR 0.5"
	//                          "BR3 0.1"
	// Dracula (Nova, German)   "DRA 2.1"
	//                          "DRA 2.4"
	//                          "DRA 2.7"
	// Adders and Ladders (Vid) "v2.1"    (MPU4 Video quiz, using 4k table implementation instead)
	//                          "v2.0"    (MPU4 Video quiz, using 4k table implementation instead)
	static constexpr uint8_t bankrollerclub_characteriser_prot[8] = { 0x00, 0x08, 0x08, 0x10, 0x00, 0x48, 0x08, 0x00 }; // good?


	// games with sequence starting 00 0c 50 60 4c 10 60 0c 78 74 00 6c 38 34 48
	// High Rise  "HII 0.3"
	//            "HIR 3.1"
	//            "HIR 3.0"
	//static constexpr uint8_t hirise_characteriser_prot[8] = { 0x00, 0x68, 0x08, 0x60, 0x40, 0x28, 0x68, 0x60 };
	static constexpr uint8_t hirise_characteriser_prot[8] = { 0x00, 0x68, 0x08, 0x60, 0x20, 0x48, 0x08, 0x00 }; // matches bootleg


	// games with sequence starting   00 48 a0 54 2c 88 94 14 2c a4 50 24 48 a4 78 c0 70
	// Bucks Fizz Club        "BUF 1.2"
	// Super Bucks Fizz Club  "SBF 2.0"
	static constexpr uint8_t bucksfizz_characteriser_prot[8] = { 0x00, 0x68, 0x48, 0x60, 0x40, 0x68, 0x48, 0x40 };


	// games with sequence starting 00 24 24 2c b0 e0 4c 30 a8 d8 9c 9c bc 1c bc 94
	// Nudge Shuffle   "NUS 3.1"
	//                 "NUS 3.0"
	static constexpr uint8_t nudshf_characteriser_prot[8] = { 0x00, 0x28, 0x20, 0x08, 0x08, 0x20, 0x20, 0x00 };


	/***************************************************************

	 Unusual sequences (but correct?)

	***************************************************************/

	// games with sequence starting
	// 40 50 40 54 64 50 64 50 64 54 60 74 40 70 40 54 64 74 64 50 44 54 60 50 44 74 44 50 40 54 40 54 60 50 64 50 44 50 44 50 40 50 64 54 64 50 64 50 64 50 64 74 40 54 64 50 44 74 40 50 64 50 64 00
	// ** This is an unusual sequence, bit 0x40 always seems to be set, both here and in the lamp results
	//    Check if it isn't just one of the other ones but with an output line tied high?
	// Hit The Top    "HI4 0.3"
	//                "CHU 0.1"
	static constexpr uint8_t hittopalt2_characteriser_prot[8] = { 0x40, 0x70, 0x50, 0x50, 0x50, 0x70, 0x50, 0x50 }; // good?


	// games with sequence starting
	// 10 94 1c f4 b8 74 b4 98 f4 9c f0 b8 d4 38 74 10 b4 bc f4 1c d0 98 70 14 b4 b8 f0 3c 50 b0 18 d0 3c 74 b4 18 f0 3c 70 94 18 70 b4 bc f0 98 f0 1c f4 18 f0 b8 74 94 bc 70 94 b8 d4 1c f4 18 f4 00
	// ** This is an unusual sequence, bit 0x10 always seems to be set, both here and in the lamp results
	// ** Check if it isn't just one of the other ones but with an output line tied high?
	// Rocket Money      "ROK 0.6"
	static constexpr uint8_t rockmn_characteriser_prot[8] = { 0x10, 0x34, 0x14, 0x34, 0x30, 0x30, 0x14, 0x30 };


	/***************************************************************

	 Games below don't boot far enough to draw conclusions
	 or ones that do boot aren't using lamp scramble

	***************************************************************/


	// games with sequence starting
	// 00 c0 d0 38 ec 5c ec 14 68 2c 24 e8 74 00 e8 14
	// Dutch Magic Replay "DMR 1.3"
	// Dutch Turbo Play   "DTP 1.3"
	// Dutch Grafitti     "DGR 1.3"
	static constexpr uint8_t turboplay_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // INCORRECT


	// games with sequence starting
	// 00 50 10 24 54 00 60 50 34 30 00 74 10 04 74
	// German Viva Las Vegas   "GLV 1.2"
	// Dutch Viva Las Vegas    "DLV 1.1"
	// Dutch Premier           "DPM 1.4"
	static constexpr uint8_t premier_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


	/***************************************************************

	 No sets using lamp scramble

	***************************************************************/

	// games with sequence starting
	// 00 e0 8c 58 b0 68 30 64 e4 cc 58 f0 2c 50 64 c4 88 5c f4 0c
	// Dutch Liberty        "DLI 1.0"
	// Crackpot 100 Club    "C1P 1.2"
	//                      " CP 3.8"
	//                      " CP 3.1"
	// Dutch High Roller    " HR 3.0"
	static constexpr uint8_t crkpot_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


	// games with sequence starting 00 c0 c8 38 f4 4c 70 60 e4 e8 38 b4 48 34 44
	// Dutch Blue Diamond  "DBD 1.0"
	// Czech Lucky Devil   "CLD 3.0"
	// German Secret Agent "GSE 3.0"
	static constexpr uint8_t bluediamond_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 24 24 a4 68 14 c4 28 d4 8c d8 f0 0c d0 8c
	// Dutch Wild Timer "DWT 1.3"
	static constexpr uint8_t wildtime_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting  00 a0 a8 58 f4 8c d8 70 c4 e8 58 74 80 2c 94 4c
	// Dutch Happy Joker   "DHJ 1.2"
	// Dutch Red Heat      "DRH 1.2"
	static constexpr uint8_t redheat_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


	// games with sequence starting 00 60 60 c0 4c 10 84 48 b4 a8 98 d4 2c 90
	// Dutch Lucky Devil     "DLD 1.3"
	// Hungarian Jolly Joker "HJJ 1.4"
	// Dutch Ceptor          "DCE 1.0"
	// Dutch Salsa           "DSA 1.5"
	static constexpr uint8_t salsa_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


	// games with sequence starting 00 a0 a8 1c f4 c8 1c b4 cc 5c 74 44 e0 28
	// Dutch 21 Club          "DTW 2.7"
	// Dutch Black & White    "DBW 1.1"
	static constexpr uint8_t blackwhite_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting  00 c0 d0 1c ec 78 ac 30 4c 2c 24 cc 7c a4 d8
	// Magic Turbo        "XST 0.4"
	// Dutch Tricolor     "DTC 2.5"
	static constexpr uint8_t tricolor_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


	// games with sequence starting 00 84 8c 3c f4 4c 34 14 54 14 40 d4 4c 70 04 d0 58 f4
	// Dutch Turbo Reel   "DTR 3.1"
	// Grandstand Club    "G2D 4.0"
	// Grandstand Club    " GD 1.1"
	static constexpr uint8_t grandclub_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 30 10 0c 98 a8 c4 60 3c ac c4 7c b4
	// Dutch Taj Mahal   "DTM 1.0"
	static constexpr uint8_t tajmahal_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 24 24 64 a4 ac 78 74 e0 6c 50 c0 ac 58 74 00 a4 ec f8 94 c8
	// Dutch Giant   "DGI 2.1"
	static constexpr uint8_t giant_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting  00 30 10 84 e0 f0 c4 60 b4 54 80 f4 50 80 f4 40 b0 d4
	// Dutch Random Roulette   "DRR 2.2"
	static constexpr uint8_t randroul_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


	// games with sequence starting 00 c0 c8 38 f4 8c b8 70 a4 e8 38 74 80 4c b0 0c 94
	// Dutch Red White & Blue  "DRW 1.4"
	static constexpr uint8_t redwhite_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting  00 50 10 a0 c4 c4 74 30 b0 34 00 f0 14 80 f4 04
	// Dutch Techno Reel  "DTE 1.3"
	static constexpr uint8_t techno_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 60 68 38 d0 2c 90 24 e4 e8 3c f0 88 34 20 40 e8 bc f4 28
	// Super Way In   "WS 1.0"
	static constexpr uint8_t wayin_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 60 60 a0 2c 50 84 28 d4 c8 9c b4 48 94
	// Dutch Black Cat     "DBL 1.4"
	// Dutch Rio Tropico   "DRT 1.0"
	static constexpr uint8_t blkcat_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 24 24 64 2c 30 48 30 68 58 5c 5c
	// Dutch Golden Joker    "DGJ 1.2"
	static constexpr uint8_t goljok_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting  00 44 44 c4 1c 24 c4 14 c8 b8 a4 dc ec b0 58 a4
	// Dutch Stars And Bars  "DSB 2.8"
	static constexpr uint8_t starsbars_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// games with sequence starting 00 a0 88 18 b0 48 50 60 e4 c8 58 90
	// Top Gear (Barcrest) (MPU4) (TG4 1.1)
	static constexpr uint8_t topgear_characteriser_prot[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

protected:
	mpu4_characteriser_pal(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t* m_current_chr_table;
	int m_prot_col;

private:

	void protection_w(uint8_t data);
	void lamp_scramble_w(uint8_t data);
	uint8_t protection_r();
	uint8_t lamp_scramble_r();

	optional_device<cpu_device> m_cpu; // needed for some of the protection 'cheats'

	bool m_allow_6800_cheat;
	bool m_allow_6809_cheat;
	bool m_allow_68k_cheat;

	const uint8_t* m_current_lamp_table;
	int m_lamp_col;
	int m_4krow;
	bool m_is_4ksim;

	optional_region_ptr<uint8_t> m_protregion; // some of the simulations have a fake ROM to assist them

	// debugging only!
	static constexpr bool IDENTIFICATION_HELPER = false;
	int m_temp_debug_write_count;
	uint8_t m_temp_debug_table[64];
};

#endif // MAME_BARCREST_MPU4_CHARACTERISER_PAL_H
