// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/*****************************************************************************************

    NES MMC Emulation

    Support for iNES Mappers

****************************************************************************************/


/* Set to generate prg & chr files when the cart is loaded */
#define SPLIT_PRG   0
#define SPLIT_CHR   0


/*************************************************************

 mmc_list

 Supported mappers and corresponding pcb id

 *************************************************************/

struct nes_mmc
{
	int    iNesMapper; /* iNES Mapper # */
	int    pcb_id;
};


static const nes_mmc mmc_list[] =
{
/*  INES   DESC                          LOW_W, LOW_R, MED_W, HIGH_W, PPU_latch, scanline CB, hblank CB */
	{  0, STD_NROM },
	{  1, STD_SXROM },
	{  2, STD_UXROM },
	{  3, STD_CNROM },
	{  4, STD_TXROM },
	{  5, STD_EXROM },
	{  6, FFE4_BOARD },
	{  7, STD_AXROM },
	{  8, FFE3_BOARD },
	{  9, STD_PXROM },
	{ 10, STD_FXROM },
	{ 11, DIS_74X377 },
	{ 12, REXSOFT_DBZ5 },
	{ 13, STD_CPROM },
	{ 14, REXSOFT_SL1632 },
	{ 15, BMC_K1029 },
	{ 16, BANDAI_LZ93EX2 },  // with 24c02
	{ 17, FFE8_BOARD },
	{ 18, JALECO_SS88006 },
	{ 19, NAMCOT_163 },
	// 20 Reserved for emulator use for FDS emulation.
	{ 21, KONAMI_VRC4 },
	{ 22, KONAMI_VRC2 },
	{ 23, KONAMI_VRC2 },
	{ 24, KONAMI_VRC6 },
	{ 25, KONAMI_VRC4 },
	{ 26, KONAMI_VRC6 },
	{ 27, UNL_CC21 },       // Mihunche, but previously used for World Hero
	{ 28, UNL_ACTION53 },   // Multi-discrete PCB designed by Tepples for Action 53
	{ 29, SEALIE_CUFROM },     // homebrew PCB used by Glider
	{ 30, SEALIE_UNROM512 },   // UNROM 512 + Flash
	{ 31, UNL_2A03PURITANS },   // PCB designed by infinitelives & rainwarrior for 2A03 Puritans Album
	{ 32, IREM_G101 },
	{ 33, TAITO_TC0190FMC },
	{ 34, STD_BXROM },
	{ 35, JYCOMPANY_C },    // NESdev: like 209 with explicit 8K of WRAM; Wario Land 2 (pirate)
	{ 36, TXC_STRIKEW },
	{ 37, PAL_ZZ },
	{ 38, DIS_74X161X138 },
	{ 39, UNL_STUDYNGAME },
	{ 40, BTL_SMB2JA },
	{ 41, CALTRON_6IN1 },
	{ 42, BTL_MARIOBABY },  // ai senshi nicole too, changed by its nes.hsi variant code
	{ 43, UNL_SMB2J },
	{ 44, BMC_SUPERBIG_7IN1 },
	{ 45, BMC_HIK8IN1 },
	{ 46, RUMBLESTATION_BOARD },
	{ 47, NES_QJ },
	{ 48, TAITO_TC0190FMCP },
	{ 49, BMC_SUPERHIK_4IN1 },
	{ 50, BTL_SMB2JB },
	{ 51, BMC_BALLGAMES_11IN1 },
	{ 52, BMC_GOLD_7IN1 },
	{ 53, SVISION16_BOARD },
	{ 54, BMC_21IN1 },   // duplicate of mapper 201, though possibly should be "Unused"
	{ 55, UNL_MMALEE },  // Genius SMB
	{ 56, KAISER_KS202 },
	{ 57, BMC_GKA },
	{ 58, BMC_GKB },
	{ 59, BMC_VT5201 },   // and BMC-T3H53, BMC-D1038
	{ 60, BMC_4IN1RESET },
	{ 61, RCM_TF9IN1 },
	{ 62, BMC_SUPER_700IN1 },
	{ 63, BMC_TH22913 },  // Powerful 250/255
	{ 64, TENGEN_800032 },
	{ 65, IREM_H3001 },
	{ 66, STD_GXROM },
	{ 67, SUNSOFT_3 },
	{ 68, SUNSOFT_DCS },
	{ 69, SUNSOFT_5 },    // FME-7/5A/5B family: the 5B board is a superset (audio at $C000/$E000, unused by FME-7 games)
	{ 70, DIS_74X161X161X32 },
	{ 71, CAMERICA_BF9093 },
	{ 72, JALECO_JF17 },
	{ 73, KONAMI_VRC3 },
	{ 74, WAIXING_TYPE_A },
	{ 75, KONAMI_VRC1 },
	{ 76, NAMCOT_3446 },
	{ 77, IREM_LROG017 },
	{ 78, IREM_HOLYDIVR },
	{ 79, AVE_NINA06 },
	{ 80, TAITO_X1_005 },
	{ 81, NTDEC_N715021 }, // 81 Super Gun
	{ 82, TAITO_X1_017 },          // Taito X1-017, older dumps with the pre-2020 PRG bank order (see mapper 552)
	{ 83, CONY_BOARD },
	// 84 Pasofami hacked images?
	{ 85, KONAMI_VRC7 },
	{ 86, JALECO_JF13 },
	{ 87, DIS_74X139X74 },
	{ 88, NAMCOT_34X3 },
	{ 89, SUNSOFT_2 },
	{ 90, JYCOMPANY_A },
	{ 91, UNL_JY830623C },
	{ 92, JALECO_JF19 },
	{ 93, SUNSOFT_2 },
	{ 94, STD_UN1ROM },
	{ 95, NAMCOT_3425 },
	{ 96, BANDAI_OEKAKIDS },
	{ 97, IREM_TAM_S1 },
	// 98 Unused
	// 99 VS. system - Not going to be implemented (use MAME instead)
	// 100 images hacked to work with nesticle?
	// 101 Unused (Urusei Yatsura had been assigned to this mapper, but it's Mapper 87)
	// 102 Unused
	{ 103, UNL_2708 },  // 103 Bootleg cart 2708 (Doki Doki Panic - FDS Conversion)
	{ 104, CAMERICA_GOLDENFIVE },
	{ 105, STD_EVENT },
	{ 106, BTL_SMB3 },
	{ 107, MAGICSERIES_MD },
	{ 108, UNL_LH28_LH54 }, // 108 has 4 variant boards
	// 109 Unused
	// 110 Unused
	{ 111, DIS_GTROM },        // GTROM (Cheapocabra) 512 + Flash
	// Old mapper 111: Ninja Ryukenden Chinese, an MMC1 variant with CHR-ROM (UNL_NINJARYU): images with CHR-ROM take that board, GTROM has none
	{ 112, NTDEC_ASDER },
	{ 113, HES_BOARD },
	{ 114, SUPERGAME_LIONKING },
	{ 115, KASING_BOARD },
	{ 116, SOMARI_SL12 },
	{ 117, FUTUREMEDIA_BOARD },
	{ 118, STD_TXSROM },
	{ 119, STD_TQROM },
	{ 120, BTL_TOBIDASE },
	{ 121, KAY_BOARD },
	{ 122, UNL_JY043 },   // JY043, a simpler bootleg variant of the Kaiser KS7058 (fwNES once used 122 for Sunsoft-1, now 184)
	{ 123, UNL_H2288 },
	// 124 Super Game Mega Type III bootleg arcade board. Emulated in MAME as supergm3.
	{ 125, UNL_LH32 },  // Monty no Doki Doki Daidassou - FDS Conversion
	{ 126, BMC_PJOY84 },
	{ 127, BTL_DDRAGON },   // Double Dragon II pirate
	// 128 1994 Super HiK 4-in-1 pirate. Dump available?
	// 129 duplicate assignment of the WQ1905-B 35-in-1 multicart (NESdev: use mapper 58); dumps carry it
	// 130 alternative assignment of the behavior later implemented by NES 2.0 mapper 331 (NESdev); dumps carry it
	// 131 Unused
	{ 132, TXC_22211 },
	{ 133, SACHEN_SA72008 },
	{ 134, BMC_FAMILY_4646 },
	// 135 Unused
	{ 136, SACHEN_TCU02 },
	{ 137, SACHEN_8259D },
	{ 138, SACHEN_8259B },
	{ 139, SACHEN_8259C },
	{ 140, JALECO_JF11 },
	{ 141, SACHEN_8259A },
	{ 142, KAISER_KS7032 },
	{ 143, SACHEN_TCA01 },
	{ 144, AGCI_50282 },
	{ 145, SACHEN_SA72007 },
	{ 146, AVE_NINA06 }, // basically same as Mapper 79 (Nina006)
	{ 147, SACHEN_TCU01 },
	{ 148, SACHEN_SA0037 },
	{ 149, SACHEN_SA0036 },
	{ 150, SACHEN_74LS374 },
	// 151 VS. system by Konami - Not going to be implemented (use MAME instead)
	{ 152, DIS_74X161X161X32 },
	{ 153, BANDAI_LZ93 },
	{ 154, NAMCOT_34X3 },
	{ 155, STD_SXROM }, // same as mapper 1 but forces the use of MMC1A
	{ 156, OPENCORP_DAOU306 },
	{ 157, BANDAI_DATACH }, // Datach Reader games -> must go in the Datach subslot
	{ 158, TENGEN_800037 },
	{ 159, BANDAI_LZ93EX1 }, // with 24c01
	{ 160, SACHEN_SA009 },
	// 161 Hanjuku Hero, assigned before MMC1 was understood (NESdev: use mapper 1); dumps carry it
	{ 162, WAIXING_FS304 },  // Waixing FS304 (NESdev), a few Nanjing games use a compatible board
	{ 163, NANJING_BOARD },
	{ 164, WAIXING_FFV },
	{ 165, WAIXING_SH2 },
	{ 166, SUBOR_TYPE1 },
	{ 167, SUBOR_TYPE0 },
	{ 168, UNL_RACERMATE },
	// 169 Unused
	{ 170, FUJIYA_BOARD },
	{ 171, KAISER_KS7058 },
	{ 172, TXC_DUMARACING },
	{ 173, TXC_MJBLOCK },
	{ 174, BMC_2751 },
	{ 175, KAISER_KS7022 },
	{ 176, UNL_XIAOZY },            // legacy iNES 1.0 Xiao Zhuan Yuan; NES 2.0 mapper 176 is routed by submapper (see below)
	{ 177, HENGG_SRICH },
	{ 178, WAIXING_SGZLZ },
	{ 179, HENGG_XHZS },
	{ 180, UXROM_CC },
	// 181 Unused
	{ 182, SUPERGAME_LIONKING },    // duplicate of mapper 114
	{ 183, BTL_SHUIGUAN },
	{ 184, SUNSOFT_1 },
	{ 185, STD_CNROM },
	{ 186, FUKUTAKE_BOARD },
	{ 187, UNL_KOF96 },
	{ 188, BANDAI_KARAOKE },
	{ 189, TXC_TW },
	{ 190, ZEMINA_BOARD },
	{ 191, WAIXING_TYPE_B },
	{ 192, WAIXING_TYPE_C },
	{ 193, NTDEC_FIGHTINGHERO },
	{ 194, WAIXING_TYPE_D },
	{ 195, WAIXING_TYPE_E },
	{ 196, BTL_SBROS11 },
	{ 197, UNL_SF3 },
	{ 198, WAIXING_TYPE_F },
	{ 199, WAIXING_TYPE_F1 },
	{ 200, BMC_36IN1 },
	{ 201, BMC_21IN1 },
	{ 202, BMC_150IN1 },
	{ 203, BMC_35IN1 },
	{ 204, BMC_64IN1 },
	{ 205, BMC_15IN1 },
	{ 206, NAMCOT_34X3 },
	{ 207, TAITO_X1_005 },
	{ 208, GOUDER_37017 },
	{ 209, JYCOMPANY_C },
	{ 210, NAMCOT_175 },
	{ 211, JYCOMPANY_B },
	{ 212, BMC_SUPERHIK_300IN1 },
	{ 213, BMC_GKB },           // duplicate of mapper 58
	{ 214, BMC_SUPERGUN_20IN1 },
	{ 215, UNL_8237 },          // and UNL_8237A
	{ 216, RCM_GS2015 },
	{ 217, BMC_500IN1 },
	{ 218, NOCASH_NOCHR },
	{ 219, UNL_A9746 },
	// 220 Unused - reserved for emulator debugging
	{ 221, UNL_N625092 },
	{ 222, BTL_DRAGONNINJA },
	{ 223, WAIXING_TYPE_I },    // (according to NEStopia source, it's MMC3 with more WRAM)
	{ 224, SMD133_BOARD },      // Jncota KT-008: the same chipset as the MINDKIDS multicarts, mapper 268 submapper 1 (NESdev)
	{ 225, BMC_72IN1 },
	{ 226, BMC_76IN1 },
	{ 227, BMC_1200IN1 },
	{ 228, ACTENT_ACT52 },
	{ 229, BMC_31IN1 },
	{ 230, BMC_22GAMES },
	{ 231, BMC_20IN1 },
	{ 232, CAMERICA_BF9096 },
	{ 233, BMC_42IN1RESET },
	{ 234, AVE_MAXI15 },
	{ 235, BMC_GOLD260 },   // 235 Golden Game x-in-1 games
	{ 236, BMC_70IN1 },
	{ 237, BMC_TELETUBBIES },
	{ 238, UNL_603_5052 },
	// 239 Unused
	{ 240, CNE_SHLZ },
	{ 241, TXC_MXMDHTWO },
	{ 242, WAIXING_WXZS },
	{ 243, SACHEN_74LS374_ALT },
	{ 244, CNE_DECATHLON },
	{ 245, WAIXING_TYPE_H },
	{ 246, CNE_FSB },
	// 247 Unused
	// 248 Unused
	{ 249, WAIXING_SECURITY },
	{ 250, NITRA_TDA },
	// 251 Unused
	{ 252, WAIXING_SGZ },
	{ 253, HENGG_SHJY3 },
	{ 254, BTL_PIKACHUY2K },
	{ 255, BMC_72IN1 },         // duplicate of mapper 225

	// NES 2.0
	// 256 OneBus Famiclones
	// 257 UNIF MAPR PEC-586?
	{ 258, UNL_158B },
	{ 259, BMC_F15 },
	{ 260, BMC_HPXX },             // HP10xx/HP20xx multicarts (BMC-HPxx, BMC-HP2018-A)
	{ 261, BMC_810544C },
	{ 262, SACHEN_SHERO },
	{ 263, UNL_KOF97 },
	{ 264, YOKO_BOARD },
	{ 265, BMC_T262 },
	{ 266, UNL_CITYFIGHT },
	{ 267, BMC_EL861121C },
	{ 268, SMD133_BOARD },
	{ 269, BMC_GX121 },            // Games Xplosion 121-in-1, 15000-in-1, 18000-in-1 (MMC3 clone, CHR in the PRG ROM)
	// 270 multicarts on OneBus Famiclones
	// 271 TXC 4 in 1 MGC-026, not in nes.xml?
	{ 272, UNL_AKUMAJO_272 },        // Akumajou Special - Boku Dracula-kun bootleg: VRC2/4 clone with a PAL chip for mirroring and a PPU A13 IRQ (FCEUmm 272.c; no NESdev page)
	// 273 Gremlins 2 bootleg, related to pirate gremlin2h or unk2?
	{ 274, BMC_80013B },
	// 275 Unused
	// 276 Unused
	// 277 Unused
	// 278 Unused
	// 279 Unused
	// 280 Unused
	{ 281, JYCOMPANY_281 },          // J.Y. Company multicart (ASIC, PCB-specific $D003)
	{ 282, JYCOMPANY_282 },          // J.Y. Company multicart (ASIC, PCB-specific $D003)
	{ 283, RCM_GS2004 },           // and RCM_GS2013
	{ 284, UNL_DRIPGAME },   // Quietust's FPGA mapper for the Drip port (UNIF UNL-DripGame)
	{ 285, BMC_A65AS },
	{ 286, BMC_BENSHIENG },
	{ 287, BMC_411120C },
	{ 288, BMC_GKCXIN1 },
	{ 289, BMC_60311C },
	{ 290, BMC_NTD_03 },
	{ 291, BMC_NT639 },
	{ 292, UNL_BMW8544 },          // Dragon Fighter by Flying Star
	// 293 NewStar multicarts, do we have these in nes.xml?
	{ 294, BMC_631601 },           // 63-1601 UNROM multicart; not the same board as mapper 134
	{ 295, JYCOMPANY_295 },          // J.Y. Company multicart (ASIC, PCB-specific $D003)
	// 296 VT3x handhelds
	{ 297, TXC_22110 },            // 2-in-1 Uzi Lightgun
	{ 298, UNL_TF1201 },           // Lethal Weapon (Enforcers) pirate
	{ 299, BMC_11160 },
	{ 300, BMC_190IN1 },
	{ 301, BMC_8157 },
	{ 302, KAISER_KS7057 },        // Gyruss FDS conversion
	{ 303, KAISER_KS7017 },        // Almana no Kiseki FDS conversion
	{ 304, BTL_09034A },           // various FDS conversions
	{ 305, KAISER_KS7031 },        // Dracula II FDS conversion
	{ 306, KAISER_KS7016 },        // Exciting Basket FDS conversion
	{ 307, KAISER_KS7037 },        // Metroid FDS conversion
	{ 308, UNL_TH21311 },          // Batman (Sunsoft) pirate on VRC2 clone hardware
	{ 309, UNL_LH51 },             // Ai Senshi Nicol alt FDS conversion
	// 310 K-1053 (1200-in-1 New World Multi), an extension of mapper 15 (NESdev NES 2.0 Mapper 310); not supported
	// 311 Unused (previously assigned in error to a bad SMB2 pirate dump)
	{ 312, KAISER_KS7013B },       // Highway Star Kaiser bootleg
	{ 313, BMC_RESETTXROM },
	{ 314, BMC_64IN1NR },
	// 315 820732C and 830134C multicarts, not in nes.xml?
	// 316 Unused
	// 317 Unused
	// 318 Unused
	{ 319, BMC_HP898F },
	{ 320, BMC_830425C },
	// 321 duplicate of 287?
	{ 322, BMC_K3033 },
	{ 323, FARID_SLROM8IN1 },      // homebrew 8-in-1
	{ 324, FARID_UNROM8IN1 },      // homebrew 8-in-1
	{ 325, UNL_MALISB },           // Super Mali Splash Bomb pirate hack
	{ 326, BTL_CONTRAJ },
	// 327 BMC-10-24-C-A1 6-in-1
	{ 328, UNL_RT01 },             // test cart (Russia)
	{ 329, UNL_EDU2K },
	{ 330, BTL_L001 },             // Sangokushi II bootleg (retitled part III)
	{ 331, BMC_12IN1 },
	{ 332, BMC_WS },
	{ 333, BMC_8IN1 },
	{ 334, BMC_5IN1_1993 },
	{ 335, BMC_CTC09 },
	{ 336, BMC_K3046 },
	{ 337, BMC_CTC_12IN1 },
	{ 338, BMC_SA005A },
	{ 339, BMC_K3006 },
	{ 340, BMC_K3036 },
	{ 341, BMC_TJ03 },
	// 342 COOLGIRL homebrew
	// 343 reset-based 4-in-1 pirate?
	// 344 3/6-in-1 GN-26 multicart, not in nes.xml
	{ 345, BMC_L6IN1 },
	{ 346, KAISER_KS7012 },        // Zanac alt FDS conversion
	{ 347, KAISER_KS7030 },        // Doki Doki Panic alt FDS conversion
	{ 348, BMC_830118C },
	{ 349, BMC_G146 },
	{ 350, BMC_891227 },
	{ 351, BMC_TECHLINE9IN1 },
	{ 352, KAISER_KS106C },        // 4-in-1
	{ 353, BMC_810305C },          // Super Mario Family multicart
	{ 354, BMC_FAM250 },
	{ 355, RCM_3DBLOCK },   // Hwang Shinwei 3-D Block and Block Force: NROM-256, CHR-RAM and a PIC16C54 that raises IRQs
	{ 356, BMC_JY208 },
	{ 357, BMC_BITCORP4602 },      // Bit Corp 4-in-1 (ID 4602), game chosen by DIP switches
	{ 358, JYCOMPANY_358 },          // J.Y. Company multicart (ASIC, PCB-specific $D003)
	{ 359, BMC_SB5013 },           // BMC-SB-5013 / GCL8050 / 841242C multicarts (Super Boy 6-in-1 and others)
	{ 360, BMC_BITCORP3150 },      // Bit Corp 31-in-1 (ID 3150), game chosen by five DIP switches
	{ 361, BMC_YY841101C },
	{ 362, BMC_830506C },
	// 363 variant of mapper 358?
	{ 364, BMC_830832C },
	// 365 is this asderp95 in nes.xml?
	{ 366, BMC_GN45 },
	// 367 7-in-1 cart that is a close variant of mapper 205
	{ 368, BTL_YUNG08 },            // SMB2 FDS conversion
	{ 369, BMC_N49C300 },          // N49C-300, Super Mario Bros. Party 6-in-1
	{ 370, BMC_F600 },              // Golden Mario Party II multicart
	// 371 Spanish PEC-586 computer main cart
	{ 372, BMC_SFC12 },
	// 373 Super 4-in-1, not in nes.xml?
	{ 374, BMC_RESETSXROM },
	// 375 135-in-1 2MB multicart
	{ 376, BMC_YY841155C },
	{ 377, BMC_EL860947C },
	// 378 8-in-1 multicart, which one?
	// 379 35-in-1 multicart, similar to mapper 38
	{ 380, BMC_970630C },
	{ 381, UNL_KN42 },             // 2-in-1 Big Nose games
	{ 382, BMC_830928C },
	// 383 JY-014 multicart
	// 384 4-in-1 VRC4 clone with Crisis Force
	// 385 NTDEC 2779 5-in-1, not in nes.xml?
	{ 386, JYCOMPANY_386 },          // J.Y. Company multicart (ASIC, PCB-specific $D003)
	{ 387, JYCOMPANY_387 },          // J.Y. Company multicart (ASIC, PCB-specific $D003)
	{ 388, JYCOMPANY_388 },          // J.Y. Company multicart (ASIC, PCB-specific $D003)
	{ 389, CALTRON_9IN1 },
	// 390 variant of mapper 236?
	// 391 BS-110 MMC3 clone
	{ 392, BMC_00202650 },
	{ 393, BMC_820720C },
	{ 394, BMC_HSK007 },   // Realtec HSK007 multicart: MMC3, BNROM or J.Y. ASIC, selected by a register
	// 395 Realtec 8210 multicarts
	{ 396, BMC_850437C },
	{ 397, JYCOMPANY_397 },          // J.Y. Company multicart (ASIC, PCB-specific $D003)
	{ 398, JY_YY840820C },          // 1995 Super HiK 5-in-1 (JY-048): VRC4 clone, outer register, GNROM-like mode
	{ 399, BATMAP_000 },           // homebrew game Star Versus
	{ 400, SEALIE_8BITXMAS17 },    // retroUSB 8-bit XMAS 2017
	{ 401, BMC_KC885 },
	// 402 22-in-1 Olympic Games, not in nes.xml?
	// 403 Tetris Family 19-in-1 that only works on Famiclones with 6502's BCD mode
	{ 404, BMC_JY012005 },
	// 405 UMC UM6578 NES-on-a-chip games...PnPs?
	// 406 homebrew game Haradius Zero
	// 407 VT03 PnP
	// 408 Konami PnP
	{ 409, SEALIE_DPCMCART },      // A Winner is You homebrew music cart
	{ 410, BMC_JY302 },
	{ 411, BMC_A88S1 },
	{ 412, HENGG_412 },            // Henggedianzi Super Game 4-in-1, INTV 10-in-1 PnP 2nd edition
	{ 413, BATMAP_SRRX },          // homebrew game Super Russian Roulette
	// 414 9999999-in-1 multicart
	{ 415, BTL_0353 },             // Lucky (Roger) Rabbit FDS conversion
	{ 416, BMC_N32_4IN1 },
	{ 417, BTL_BATMANFS },         // "Fine Studio" Batman bootleg
	{ 418, UNL_LH42 },             // Highway Star Whirlwind Manu bootleg
	// 419 VT03 PnPs
	// 420 Kasheng A971210 board
	// 421 JY SC871115C board
	{ 422, BMC_BS300 },             // BS-300 / ING003C family multicarts (same ASIC family as mappers 126 and 534)
	// 423 Lexibook PnP
	// 424 Lexibook PnP
	// 425 Cube Tech PnP
	// 426 PnP
	// 427 PnP
	{ 428, BMC_TF2740 },
	// 429 Milowork FCFC1 flash cartridge
	// 430 MMC3-based 831031C/T-308 multicart
	{ 431, BMC_GN91B },
	{ 432, BMC_REALTEC432 },       // Realtec 8023/8043/8085/8086/8090/8145, GN-30C, 940702 multicarts
	{ 433, BMC_NC20MB },
	{ 434, BMC_S009 },
	// 435 F-1002 multicart
	// 436 VT02 PnP
	{ 437, BMC_TH2348 },
	{ 438, BMC_K3071 },
	// 439 YS2309 multicart
	// 440 Dongda Sonic REC-9388 educational computer
	// 441 841026C and 850335C multicarts
	// 442 "Golden Key" educational computer
	// 443 NC3000M multicart
	// 444 NC7000M multicart
	// 445 DG574B MMC3-compatible multicart
	{ 446, SMD172B_BOARD },     // Mindkids SMD172B_FPGA
	{ 447, BMC_KL06 },
	// 448 VRC4-based 830768C multicart
	// 449 Super Games King multicart
	// 450 VRC2-based YY841157C multicart
	// 451 homebrew Haratyler HP/MP
	{ 452, BMC_DS927 },
	{ 470, UNL_INX007T },   // Retro-Bit Battletoads & Double Dragon
	// 453 Realtec 8042
	// 454...511 are unused here except:
	{ 487, AVE_NINA08 },           // Maxivision 30-in-1 (original), AVE NINA-08
	{ 493, AVE_MAXI30P },          // Maxivision 30 Super Games prototype
	{ 512, SACHEN_ZGDH },
	{ 513, SACHEN_SA9602B },
	{ 514, SUBOR_KARAOKE },          // Subor Karaoke
	{ 515, NORAEBANG_BOARD },      // Korean Family Noraebang karaoke cart with expansion cart, mic, and YM2413
	{ 516, COCOMA_BOARD },
	{ 517, KKACHI_BOARD },         // Kkachi-wa Norae Chingu, Korean karaoke cart: UNROM banking and a microphone ADC
	{ 518, SUBOR_TYPE2 },           // Subor 999, Subor V, Dance 2000 12-in-1 (UNL-DANCE2000)
	{ 519, UNL_EH8813A },          // Dr Mario II Chinese pirate
	{ 520, BTL_2YUDB },
	{ 521, DREAMTECH_BOARD },      // Korean Igo
	{ 522, UNL_LH10 },             // Fuuun Shaolin Kyo FDS conversion
	{ 523, JNCOTA_FS005 },         // Fengshenbang (Jncota re-release), FS005 with 2 KiB CHR banking
	{ 524, BTL_900218 },           // Lord of King pirate
	{ 525, KAISER_KS7021A },       // GetsuFumaDen pirate (and maybe a Contra?)
	{ 526, UNL_BJ56 },             // Sangokushi (Chuugen no Hasha) bootleg, UNL-BJ-56
	{ 527, UNL_AX40G },            // Fudou Myouou Den pirate
	{ 528, BMC_831128C },          // 1995 New Series Super 2-in-1
	{ 529, UNL_T230 },             // Datach Dragon Ball Z IV bootleg
	{ 530, UNL_AX5705 },           // Super Mario Bros Pocker Mali
	// 531 Used by Asder PC-95 Famicom clone built into a keyboard
	// 532 Emulator only mapper for Chinese version of sangoht2?
	{ 533, SACHEN_3014 },          // Dong Dong Nao II
	{ 534, BMC_ING022 },            // ING-022 multicarts (MMC3 IRQ latch is inverted)
	{ 535, UNL_LH53 },             // Nazo no Murasamejo FDS conversion
	// 536 and 537 Waixing FS303, mapper 195 variants?
	{ 538, BTL_60106416L },        // Super Soccer Champion, FDS conversion of Exciting Soccer (60-1064-16L)
	{ 539, BTL_PALTHENA },         // Hikari Shinwa (Kid Icarus) FDS conversion
	{ 540, UNL_82112C },            // UNL-82112C: the BMC-SB-5013 registers with 2 KiB CHR banks (Master Fighter VI' EJ-4003; FCEUmm 359.c; no NESdev page)
	{ 541, BMC_LITTLECOM160 },
	// 542 Chairman Mao's 100th anniversary cart? You've got to be kidding me.
	{ 543, BMC_SRPG_5IN1 },
	{ 544, WAIXING_FS306 },        // Bawang de Dalu, Waixing FS306 (VRC4 clone with CHR-RAM GAL)
	// 545 4 in 1 (ST-80) multicart, not in nes.xml?
	// 546 10 in 1 Tenchi wo Kurau multicart, not in nes.xml?
	{ 547, KONAMI_VRC5 },          // Konami QTa adapter games (NHK Gakuen Space School, Idemitsu Space College)
	{ 548, BTL_CTC15 },            // Almana no Kiseki, Co Tung CTC-15 FDS conversion
	{ 549, KAISER_KS7016B },       // Meikyuu Jiin Dababa alt FDS conversion
	{ 550, BMC_JY820845C },
	{ 551, JNCOTA_KT1001 },
	{ 552, TAITO_X1_017 },
	{ 553, SACHEN_3013 },          // Dong Dong Nao 1
	{ 554, KAISER_KS7010 },        // Akumajo Dracula FDS conversion
	{ 555, STD_EVENT2 },
	// 556 JY-215 multicart
	{ 557, UNL_LG25 },             // Moero TwinBee FDS conversion
	{ 558, WAIXING_YC030309 },     // Yancheng YC-03-09 and Waixing RPGs, a later version of the mapper 164 chipset
	// 559...681 are unused here except:
	{ 559, UNL_SANGO2_559 },        // Sangokushi II pirate: VRC4 clone, nibble data, external nametable/PRG register (FCEUmm 559.c; no NESdev page)
	{ 563, UNL_TMNT2_563 },         // TMNT II pirate: VRC2 clone, outer register, PA12-counting IRQ (FCEUmm 563.c; no NESdev page)
	{ 564, BMC_564 },               // NTDEC 22-in-1 data-latch multicart (FCEUmm 564.c; no NESdev page)
	{ 576, BMC_576 },               // 100-in-1 address-latch multicart (FCEUmm 576.c; no NESdev page)
	{ 582, BMC_582 },               // Super Micro Machines 1999, two 16 KiB banks (FCEUmm 582.c; no NESdev page)
	{ 585, BMC_585 },               // Super 72-in-1 address-latch multicart (FCEUmm 585.c; no NESdev page)
	{ 594, RINCO_FSG2 },           // Rinco Talkman 3-in-1 (FSG2), MMC3 clone with an ADPCM FIFO
	{ 620, BMC_620 },               // 1993 Super Fighter 22-in-1: MMC3 clone, PRG from an outer register (FCEUmm 620.c; no NESdev page)
	{ 621, BMC_621 },               // Predator pirate: MMC1A with a parallel register load at $6000 (FCEUmm 621.c; no NESdev page)
	{ 682, BROKE_RAINBOW },        // Broke Studio Rainbow (homebrew FPGA board)
	// 683...4095 Unused
};

const nes_mmc *nes_mapper_lookup( int mapper )
{
	for (int i = 0; i < std::size(mmc_list); i++)
	{
		if (mmc_list[i].iNesMapper == mapper)
			return &mmc_list[i];
	}

	return nullptr;
}

#if 0
int nes_get_mmc_id( running_machine &machine, int mapper )
{
	const nes_mmc *mmc = nes_mapper_lookup(mapper);

	if (mmc == nullptr)
		fatalerror("Unimplemented Mapper %d\n", mapper);

	return mmc->pcb_id;
}
#endif

/*************************************************************

 ines_mapr_setup

 setup the board specific pcb_id for a given mapper

 *************************************************************/

void ines_mapr_setup( int mapper, int *pcb_id )
{
	const nes_mmc *mmc = nes_mapper_lookup(mapper);
	if (mmc == nullptr)
		fatalerror("Unimplemented Mapper %d\n", mapper);

	*pcb_id = mmc->pcb_id;
}

/*************************************************************

 call_load_ines

 *************************************************************/

void nes_cart_slot_device::call_load_ines()
{
	uint32_t vram_size = 0, prgram_size = 0, battery_size = 0, mapper_sram_size = 0;
	uint32_t prg_size, vrom_size;
	uint8_t header[0x10];
	uint16_t mapper;
	uint8_t submapper = 0, local_options;
	bool ines20 = false, prg16k;
	std::string mapinfo;
	int pcb_id = 0, mapint1 = 0, mapint2 = 0, mapint3 = 0, mapint4 = 0;
	int hsi_variant = 0;
	bool bus_conflict = false;

	// read out the header
	fseek(0, SEEK_SET);
	fread(&header, 0x10);

	// SETUP step 1: getting PRG, VROM, VRAM sizes
	prg16k = (header[4] == 1);
	prg_size = prg16k ? 2 * 0x4000 : header[4] * 0x4000;
	vrom_size = header[5] * 0x2000;
	vram_size = 0x4000;

	// SETUP step 2: getting PCB and other settings
	mapper = (header[6] & 0xf0) >> 4;
	local_options = header[6] & 0x0f;

	switch (header[7] & 0xc)
	{
		case 0x4:
		case 0xc:
			// probably the header got corrupted: don't trust upper bits for mapper
			break;

		case 0x8:   // it's NES 2.0 format
			ines20 = true;
			[[fallthrough]];
		case 0x0:
		default:
			mapper |= header[7] & 0xf0;
			break;
	}

	// use info from nes.hsi if available!
	if (hashfile_extrainfo(*this, mapinfo))
	{
		if (4 == sscanf(mapinfo.c_str(),"%d %d %d %d", &mapint1, &mapint2, &mapint3, &mapint4))
		{
			if (!ines20)
			{
				/* image is present in nes.hsi: overwrite the header settings with these */
				mapper = mapint1;
				local_options = mapint2 & 0x0f;
				hsi_variant = (mapint2 & 0xf0) >> 4; // this is used to differentiate among variants of the same Mapper (see below)
				prg16k = (mapint3 == 1);
				prg_size = prg16k ? 2 * 0x4000 : mapint3 * 0x4000;
				vrom_size = mapint4 * 0x2000;
				logerror("NES.HSI info: %d %d %d %d\n", mapint1, mapint2, mapint3, mapint4);
			}
			else if (mapint1 == (mapper | ((header[8] & 0x0f) << 8)))
			{
				// NES 2.0 header: it is authoritative for mapper, mirroring/battery/trainer/four-screen and sizes; nes.hsi
				// only supplies MAME's variant code (which no header can carry), and only when it describes the same mapper
				hsi_variant = (mapint2 & 0xf0) >> 4;
				logerror("NES.HSI info: %d %d %d %d (NES 2.0 header kept, variant code %d taken)\n", mapint1, mapint2, mapint3, mapint4, hsi_variant);
			}
			else
				logerror("NES.HSI info: %d %d %d %d ignored (NES 2.0 header says mapper %d)\n", mapint1, mapint2, mapint3, mapint4, mapper | ((header[8] & 0x0f) << 8));
		}
		else
		{
			logerror("NES: [%s], Invalid mapinfo found\n", mapinfo.c_str());
		}
	}
	else
	{
		logerror("NES: No extrainfo found\n");
	}

	// use extended NES 2.0 info if available!
	if (ines20)
	{
		mapper |= (header[8] & 0x0f) << 8;
		// read submappers (based on 20140116 specs)
		submapper = (header[8] & 0xf0) >> 4;

		// NES 2.0's extended exponential sizes, needed for loading PRG >= 64MB, CHR >= 32MB. These bizarrely go up to 7 * 2^63!
		auto expsize = [] (u8 byte) { return (2*(byte & 0x03) + 1) << (byte >> 2); };

		if ((header[9] & 0x0f) == 0x0f)
		{
			prg_size = expsize(header[4]);
			if (prg_size == 0)    // 0 only on overflow
				fatalerror("NES 2.0 PRG size >= 4GB is unsupported.\n");
		}
		else
			prg_size += ((header[9] & 0x0f) << 8) * 0x4000;

		if ((header[9] & 0xf0) == 0xf0)
		{
			vrom_size = expsize(header[5]);
			if (vrom_size == 0)    // 0 only on overflow
				fatalerror("NES 2.0 CHR size >= 4GB is unsupported.\n");
		}
		else
			vrom_size += ((header[9] & 0xf0) << 4) * 0x2000;
	}
	ines_mapr_setup(mapper, &pcb_id);
	// NES 2.0 mapper 176 is the 8025 enhanced MMC3 family, selected by submapper:
	// 0 = LP-8002KB/SFC-12B (Super24in1SC03), 1 = FK-xxx/BS-xxx (FK23C/FK23CA), 2 = FS005, 3 = JX9003B, 4 = GameStar, 5 = HST-162:
	// all of them are emulated by nes_8025_device, which follows the NESdev register description.
	// The older BMC-SUPER24IN1SC03 and BMC-FK23C/FK23CA boards remain for UNIF files and software list entries.
	// iNES 1.0 images have no submapper: the legacy Xiao Zhuan Yuan board (pirate.cpp) is kept for the 128 KiB PRG / 64 KiB CHR size only (see below).
	if (mapper == 176 && ines20)
	{
		if (submapper <= 5)
			pcb_id = BMC_8025;
		else
			logerror("Unimplemented NES 2.0 mapper 176 submapper: %d\n", submapper);
	}

	// iNES 1.0 has no submapper: only the 128 KiB PRG / 64 KiB CHR Xiao Zhuan Yuan carts keep the legacy board,
	// any other mapper 176 image is an FK23C-type multicart (submapper 1)
	if (mapper == 176 && !ines20 && !(prg_size == 0x20000 && vrom_size == 0x10000))
	{
		pcb_id = BMC_8025;
		submapper = 1;
	}

	// Mapper 294 is the CHR-RAM UNROM 63-1601 board of NESdev; images with CHR-ROM filed under it (the CB-4035
	// 4-in-1, 512 KiB CHR-ROM) are MMC3-type multicarts and keep the mapper 134 board they always loaded as
	if (mapper == 294 && vrom_size)
		pcb_id = BMC_FAMILY_4646;

	// NES 2.0 submappers of boards that implement several incompatible hardware variants
	if ((mapper == 126 || mapper == 422 || mapper == 534) && submapper <= 4)
		m_cart->set_pjoy_submapper(submapper);
	if (mapper == 208 && submapper <= 1)
		m_cart->set_gouder_submapper(submapper);
	if (mapper == 394 && submapper <= 1)
		m_cart->set_hsk007_submapper(submapper);
	if (mapper == 268 && submapper <= 11)
		m_cart->set_smd133_submapper(submapper);
	if (mapper == 446 && submapper <= 4)
		m_cart->set_smd172b_submapper(submapper);
	if (mapper == 91 && submapper <= 1)
		m_cart->set_m91_submapper(submapper);
	if (mapper == 224)
		m_cart->set_smd133_submapper(1);
	if (mapper == 197 && submapper <= 3)
		m_cart->set_sf3_submapper(submapper);
	if (mapper == 313 && submapper <= 4)
		m_cart->set_resettxrom_submapper(submapper);
	if (mapper == 411 && submapper <= 2)
		m_cart->set_a88s1_submapper(submapper);
	if (mapper == 176 && submapper <= 5)
		m_cart->set_8025_submapper(submapper);
	if (mapper == 196 && (submapper == 1 || submapper == 3))
		m_cart->set_fs6_submapper(submapper);
	if (mapper == 354 && submapper <= 1)
		m_cart->set_fam250_submapper(submapper);
	if (mapper == 432 && submapper <= 4)
		m_cart->set_realtec432_submapper(submapper);
	if (mapper == 178 && submapper <= 3)
		m_cart->set_sgzlz_submapper(submapper);
	if (mapper == 227 && submapper <= 2)
		m_cart->set_1200in1_submapper(submapper);
	if (mapper == 82)
		m_cart->set_x1_017_old_order(true);
	// 191: submapper 1 has the outer bank register at $90aa; iNES 1.0 images are told apart by their CHR-ROM size
	if (mapper == 191)
		m_cart->set_waixing_b_submapper(ines20 ? (submapper == 1 ? 1 : 0) : (vrom_size == 0x40000 ? 1 : 0));

	// MMC3 IRQ variants selected by NES 2.0 submapper (3 = MC-ACC, 4 = NEC MMC3)
	if (mapper == 4 && (submapper == 3 || submapper == 4))
		m_cart->set_mmc3_irq_variant(submapper);
	// iNES 1.0 mapper 4 images flagged in nes.hsi with hsi_variant 4 use the old-style (MMC3A/NEC) IRQ: blargg's
	// mmc3_irq_tests 5.MMC3_rev_A and mmc3_test 6-MMC3_alt need it, while the default stays the new-style Sharp IRQ
	else if (mapper == 4 && !ines20 && hsi_variant == 4)
		m_cart->set_mmc3_irq_variant(4);
	// iNES 1.0 mapper 4 images flagged in nes.hsi with hsi_variant 1 are MMC6 (HKROM), as NES 2.0 submapper 1 would say:
	// blargg's mmc3_test 6-MMC6
	if (mapper == 4 && !ines20 && hsi_variant == 1)
		pcb_id = STD_HKROM;

	// Waixing T9552: mapper 4 submapper 5 images store the banks in the $5000=$02 order (mapper 249 uses $5000=$00)
	if (mapper == 4 && submapper == 5)
		m_cart->set_t9552_file_order(2);

	// handle submappers
	if (submapper)
	{
		// 001: MMC1 (other submappers are deprecated)
		// 001: submapper 5 (SEROM/SHROM/SH1ROM, fixed 32K PRG) is handled by the SxROM board; it must be claimed here,
		// or the catch-all at the end of this chain resets the submapper to 0
		if (mapper == 1 && submapper == 5)
		{
		}
		// 001: submapper 7 (Kaiser KS-7058 with the KS 203 MMC1 clone, hard-wired nametable arrangement) is handled by the
		// SxROM board as well
		else if (mapper == 1 && submapper == 7)
		{
		}
		// 002, 003, 007: UxROM, CNROM, AxROM
		else if (mapper == 2 && submapper == 2)
			bus_conflict = true;
		else if (mapper == 2 && submapper == 1)
			bus_conflict = false;    // submapper 1: no bus conflicts (this is also the iNES default here)
		else if (mapper == 3 && submapper == 2)
			bus_conflict = true;
		else if (mapper == 3 && submapper == 1)
			bus_conflict = false;    // submapper 1: no bus conflicts (this is also the iNES default here)
		else if (mapper == 7 && submapper == 2)
			bus_conflict = true;
		else if (mapper == 7 && submapper == 1)
			bus_conflict = false;    // submapper 1: no bus conflicts (this is also the iNES default here)
		// 011: Color Dreams
		else if (mapper == 11 && submapper == 2)
			bus_conflict = true;
		else if (mapper == 11 && submapper == 1)
			bus_conflict = false;    // submapper 1: no bus conflicts (this is also the iNES default here)
		// 004: NES 2.0 submappers (NESdev): 1 = MMC6 (HKROM), 3 = Acclaim MC-ACC, 4 = NEC MMC3 ("old" IRQ), 5 = Waixing T9552
		else if (mapper == 4 && submapper == 1)
			pcb_id = STD_HKROM;
		else if (mapper == 4 && submapper == 5)
			pcb_id = WAIXING_SECURITY;
		// 019: Namcot N163
		else if (mapper == 19)
		{
			int vol = submapper & 0x07;
			if (vol >= 0 && vol <= 5)
			{
				pcb_id = NAMCOT_163;
				m_cart->set_n163_vol(vol);
			}
		}
		// 021, 023, 025: VRC2 / VRC4.  NESdev NES 2.0 submappers 1-3 (lines given as chip A1, chip A0):
		// 21/1 VRC4a (A2,A1), 21/2 VRC4c (A7,A6), 23/1 VRC4f (A1,A0), 23/2 VRC4e (A3,A2), 23/3 VRC2b (A1,A0),
		// 25/1 VRC4b (A0,A1), 25/2 VRC4d (A2,A3), 25/3 VRC2c (A0,A1).  Other submappers keep the older line encoding.
		else if ((mapper == 21 || mapper == 23 || mapper == 25) && submapper >= 1 && submapper <= 3)
		{
			static const struct { int mapper, sub, a1, a0; bool vrc2; } vrc_subs[] = {
				{ 21, 1, 2, 1, false }, { 21, 2, 7, 6, false },
				{ 23, 1, 1, 0, false }, { 23, 2, 3, 2, false }, { 23, 3, 1, 0, true },
				{ 25, 1, 0, 1, false }, { 25, 2, 2, 3, false }, { 25, 3, 0, 1, true } };
			for (const auto &v : vrc_subs)
				if (v.mapper == mapper && v.sub == submapper)
				{
					pcb_id = v.vrc2 ? KONAMI_VRC2 : KONAMI_VRC4;
					m_cart->set_vrc_lines(v.a1, v.a0, 0);
				}
		}
		// 021, 023, 025: VRC4 / VRC2 (submappers other than 1-3: older line encoding)
		else if (mapper == 21 || mapper == 23 || mapper == 25)
		{
			// 021, 023, 025: VRC4
			int line_1 = submapper & 0x07;
			int line_2 = (submapper & 0x08) ? line_1 + 1 : line_1 - 1;
			if (line_2 >= 0 && line_2 <= 7)
			{
				pcb_id = KONAMI_VRC4;
				m_cart->set_vrc_lines(line_1, line_2, 0);
			}
			else if (submapper == 15)
			{
				pcb_id = KONAMI_VRC2;
				m_cart->set_vrc_lines(1, 0, 0);
			}
		}
		// 032: Irem G101
		else if (mapper == 32 && submapper == 1)
		{
			m_cart->set_mirroring(PPU_MIRROR_HIGH); // Major League has hardwired mirroring
		}
		// iNES Mapper 016 (NESdev): submapper 4 = Bandai FCG-1/2 (registers at $6000-$7FFF only, no EEPROM),
		// submapper 5 = LZ93D50 with 24C02 (registers at $8000-$FFFF only)
		else if (mapper == 16 && submapper == 4)
		{
			pcb_id = BANDAI_FCG;
		}
		else if (mapper == 16 && submapper == 5)
		{
			m_cart->set_bandai_submapper(5);
		}
		// iNES Mapper 034
		else if (mapper == 34 && submapper == 1)
		{
			pcb_id = AVE_NINA01; // Mapper 34 is used for 2 diff boards
		}
		// iNES Mapper 114: submapper 1 (Boogerman) uses a different register scrambling pattern
		else if (mapper == 114 && submapper == 1)
		{
			pcb_id = SUPERGAME_BOOGERMAN;
		}
		// iNES Mapper 196: submapper 1 has the MMC3's A0 input on CPU A1 (Fight Street VI), submapper 2 on CPU A2, submapper 3
		// has it on CPU A1 with mapper 189-style PRG banking (the same board with the PRG register of mapper 189)
		else if (mapper == 196 && (submapper == 1 || submapper == 3))
		{
			pcb_id = UNL_FS6;
		}
		else if (mapper == 34 && submapper == 2)
		{
			bus_conflict = true;    // submapper 2: BNROM, which is always subject to bus conflicts
		}
		// iNES Mapper 068 / Sunsoft 4
		else if (mapper == 68 && submapper == 1)
		{
			submapper = 0;
			logerror("Unimplemented NES 2.0 submapper: SUNSOFT-DCS.\n");
		}
		// iNES Mapper 071
		else if (mapper == 71 && submapper == 1)
		{
			m_cart->set_pcb_ctrl_mirror(true);    // Mapper 71 is used for 2 diff boards
		}
		// iNES Mapper 078
		else if (mapper == 78)
		{
			if (submapper == 1)
				pcb_id = JALECO_JF16;    // Mapper 78 is used for 2 diff boards
			else if (submapper == 3)
				pcb_id = IREM_HOLYDIVR;
		}
		// iNES Mapper 116
		else if (mapper == 116 && submapper == 2)
		{
			pcb_id = SOMARI_HUANG2; // Mapper 116 is used for 2 diff boards
		}
		// NES 2.0 submapper 3: the reset-based SL-FC5-1 Mario 5-in-1 (NESdev INES Mapper 116)
		else if (mapper == 116 && submapper == 3)
		{
			pcb_id = SOMARI_SL12_MC;
		}
		// iNES Mapper 185: submappers 4-7 enable CHR-ROM when bits 0-1 of the latch hold 0-3 (other values use the nes.hsi variant code below)
		else if (mapper == 185 && submapper >= 4 && submapper <= 7)
		{
			m_cart->set_ce(0x03, submapper & 0x03);
			// NESdev lists these CNROM copy-protection boards as subject to bus conflicts; a submapper identifies the
			// board exactly (images without one keep the default, as their board is not known)
			bus_conflict = true;
		}
		// iNES Mapper 232
		else if (mapper == 232 && submapper == 1)
		{
			pcb_id = CAMERICA_BF9096_ALT;
		}
		// 126, 422, 534: TEC9719 / BS-300 family / ING-022, PRG/CHR wiring selected by submapper 0-4
		else if ((mapper == 126 || mapper == 422 || mapper == 534) && submapper <= 4)
		{
		}
		// 176: 8025 family, boards selected by submapper 0-5 above
		else if (mapper == 176 && submapper <= 5)
		{
		}
		// 191: Waixing Sugoro Quest board, submappers 0-1 handled by the board
		else if (mapper == 191 && submapper <= 1)
		{
		}
		// 208: Gouder (0) and MMC3-register-6 PRG variant (1); 215: UNL-8237A; 411: A88S-1 variants 0-2
		else if ((mapper == 208 && submapper <= 1) || (mapper == 215 && submapper == 1) || (mapper == 411 && submapper <= 2))
		{
		}
		// 210: Namco 175 (1) / Namco 340 (2), routed after this chain
		else if (mapper == 210 && submapper <= 2)
		{
		}
		// 268: AA6023 / SMD132/133 boards, register address and wiring selected by submapper 0-11
		else if (mapper == 268 && submapper <= 11)
		{
		}
		// 91: JY830623C/YY840238C (0) and EJ-006-1 (1, M2 IRQ counter and selectable mirroring)
		else if (mapper == 91 && submapper <= 1)
		{
		}
		// 446: SMD172B_FPGA, the submapper selects which set of mappers the FPGA offers (0-4)
		else if (mapper == 446 && submapper <= 4)
		{
		}
		// 313: BMC RESET-TXROM
		else if (mapper == 313)
		{
			if (submapper == 0)
			{
				m_cart->set_outer_prg_size(128);
				m_cart->set_outer_chr_size(128);
			}
			else if (submapper == 1)
			{
				m_cart->set_outer_prg_size(256);
				m_cart->set_outer_chr_size(128);
			}
			else if (submapper == 2)
			{
				m_cart->set_outer_prg_size(128);
				m_cart->set_outer_chr_size(256);
			}
			else if (submapper == 3)
			{
				m_cart->set_outer_prg_size(256);
				m_cart->set_outer_chr_size(256);
			}
			else if (submapper == 4)    // 256 KiB of PRG-ROM for the first game, 128 KiB for the others (handled by the board)
			{
				m_cart->set_outer_prg_size(128);
				m_cart->set_outer_chr_size(128);
			}
			else
				logerror("Unimplemented NES 2.0 submapper: %d\n", submapper);
		}
		// UNROM 512 (mapper 30): submappers 1 (no bus conflicts) and 2 (bus conflicts) are kept for the bus conflict setup below
		else if (mapper == 30 && submapper >= 1 && submapper <= 4)
		{
		}
		// mapper 42 submapper 2: the 160K Green Beret conversions are the UNL-AC08 board (selected below); NESdev defines no
		// submappers for mapper 42, so this one is kept instead of being reset as undocumented
		else if (mapper == 42 && submapper == 2)
		{
		}
		// mapper 417: NESdev's NES 2.0 Mapper 417 defines submapper 1 (RoboCop)
		else if (mapper == 417 && submapper == 1)
		{
		}
		// mapper 108: NESdev's INES Mapper 108 defines submappers 1 to 4 (DH-08, UNL-BB with CHR-ROM, LH28/LH54, LE05)
		else if (mapper == 108 && submapper >= 1 && submapper <= 4)
		{
		}
		// mapper 354 submapper 1: the 810331C/SCHI-24 board (NESdev NES 2.0 Mapper 354), passed to the board above
		else if (mapper == 354 && submapper == 1)
		{
		}
		// mapper 432 submappers 1-4: the Realtec 8023, 8085, 8145 and 940702 boards (NESdev NES 2.0 Mapper 432)
		else if (mapper == 432 && submapper <= 4)
		{
		}
		// mapper 178 submappers 1-3: infrared sensor, LPC speech chip, solder pad read-out (NESdev INES Mapper 178)
		else if (mapper == 178 && submapper <= 3)
		{
		}
		// mapper 227 submappers 1-2: solder pad read mode, and the outer bank clearing with inner bank 0 (NESdev INES Mapper 227)
		else if (mapper == 227 && submapper <= 2)
		{
		}
		else if (submapper)
		{
			submapper = 0;
			logerror("Undocumented NES 2.0 submapper, please report it to the MAME boards!\n");
		}
	}

	// 210: Namco 175 (submapper 1) or Namco 340 (submapper 2); without a submapper, NESdev's rule is a 175 when the battery
	// bit is set and a 340 otherwise, which fits every commercial game (INES Mapper 210)
	if (mapper == 210)
		pcb_id = (submapper == 1 || (submapper != 2 && BIT(local_options, 1))) ? NAMCOT_175 : NAMCOT_340;

	// 034: NINA-001/002 (submapper 1) or BNROM (submapper 2); without a submapper, NESdev's rule is BNROM for 0-8 KiB of CHR
	// and NINA-001/002 above that (INES Mapper 034)
	if (mapper == 34 && submapper == 0 && vrom_size > 0x2000)
		pcb_id = AVE_NINA01;

	// SETUP step 3: storing the info needed for emulation
	m_pcb_id = pcb_id;
	m_cart->set_mirroring(BIT(local_options, 0) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
	if (BIT(local_options, 1))
		battery_size = NES_BATTERY_SIZE; // with original iNES format we can only support 8K WRAM battery
	m_cart->set_trainer(BIT(local_options, 2) ? true : false);

	// A select few boards or their variants have on-cart RAM to support 4-screen mirroring
	if ((BIT(local_options, 3) && (m_pcb_id == STD_TXROM || m_pcb_id == NAMCOT_34X3)) || m_pcb_id == IREM_LROG017 || m_pcb_id == SACHEN_SHERO)
	{
		m_cart->set_four_screen_vram(true);
		m_cart->set_mirroring(PPU_MIRROR_4SCREEN);
	}

	// The JY-208 board can switch to four nametables of cartridge VRAM (NESdev NES 2.0 mapper 356), whatever the header says
	if (m_pcb_id == BMC_JY208)
		m_cart->set_four_screen_vram(true);

	if (ines20)
	{
		// PRGRAM/BWRAM (not fully supported, also due to lack of 2.0 files)
		if ((header[10] & 0x0f) > 0)
			prgram_size = 0x80 << ((header[10] & 0x0f) - 1);
		if ((header[10] & 0xf0) > 0)
			battery_size = 0x80 << (((header[10] & 0xf0) >> 4) - 1);
		// VRAM
		vram_size = 0;
		if ((header[11] & 0x0f) > 0)
			vram_size = 0x80 << ((header[11] & 0x0f) - 1);
		if ((header[11] & 0xf0) > 0)
			vram_size += 0x80 << (((header[11] & 0xf0) >> 4) - 1);
		// header[11] & 0xf0 is the size of battery backed VRAM, found so far in Racermate II only and not supported yet
	}
	else
	{
		// PRGRAM size is 8k for most games, but pirate carts often use different sizes,
		// so its size has been added recently to the iNES format spec, but almost no image uses it
		prgram_size = header[8] ? header[8] * 0x2000 : 0x2000;
	}

	// a few mappers correspond to multiple PCBs, so we need a few additional checks and tweaks
	switch (m_pcb_id)
	{
		case STD_NROM:
			if (prg_size == 3 * 0x4000) // NROM368 are padded with 2k empty data at start to accomplish with iNES standard
			{
				m_pcb_id = STD_NROM368;
				fseek(0x810, SEEK_SET);
				prg_size = 0xb800;
			}
			break;

		case STD_SXROM:
			if (mapper == 1 && ines20 && prgram_size == 0x2000 && battery_size == 0x2000 && vrom_size == 0x4000)
				m_pcb_id = STD_SZROM;
			if (mapper == 155)
				m_cart->set_mmc1_type(device_nes_cart_interface::mmc1_type::MMC1A);
			if (mapper == 1 && submapper == 5)
				m_cart->set_mmc1_fixed_prg(true);
			if (mapper == 1 && submapper == 7)
				m_cart->set_mmc1_ks203(true);
			break;

		case NOCASH_NOCHR:
			// this mapper uses mirroring flags differently
			m_cart->set_four_screen_vram(false);
			switch (local_options & 0x09)
			{
				case 0x00:
					m_cart->set_mirroring(PPU_MIRROR_HORZ);
					break;
				case 0x01:
					m_cart->set_mirroring(PPU_MIRROR_VERT);
					break;
				case 0x08:
					m_cart->set_mirroring(PPU_MIRROR_LOW);
					break;
				case 0x09:
					m_cart->set_mirroring(PPU_MIRROR_HIGH);
					break;
			}
			break;

		case DIS_GTROM:
			// mapper 111 was first assigned to a Chinese translation of Ninja Ryukenden, an MMC1 variant that has CHR-ROM (GTROM has none)
			if (vrom_size)
				m_pcb_id = UNL_NINJARYU;
			else
			{
				// the board has 32 KiB of PPU RAM (pattern tables and nametables) and no PRG-RAM: the battery flag means the PRG-ROM
				// is self-flashable, so the battery and PRG-RAM sizes the header implies are dropped
				m_cart->set_flashable(BIT(local_options, 1));
				vram_size = 0x8000;
				prgram_size = 0;
				battery_size = 0;
			}
			break;

		case SEALIE_UNROM512:
			// this mapper also uses mirroring flags differently
			m_cart->set_four_screen_vram(false);
			switch (local_options & 0x09)
			{
				case 0x00:
					m_cart->set_mirroring(PPU_MIRROR_HORZ);
					break;
				case 0x01:
					m_cart->set_mirroring(PPU_MIRROR_VERT);
					break;
				case 0x08:
					m_cart->set_mirroring(PPU_MIRROR_LOW);
					m_cart->set_pcb_ctrl_mirror(true);
					break;
				case 0x09:
					m_cart->set_mirroring(PPU_MIRROR_4SCREEN);
					m_cart->set_four_screen_vram(true);
					break;
			}
			break;

		case STD_CNROM:
			if (mapper == 185 && !submapper)
			{
				switch (hsi_variant)
				{
					case 0x0: // pin26: CE, pin27: CE (B-Wings, Bird Week)
						m_cart->set_ce(0x03, 0x03);
						break;
					case 0x4: // pin26: CE, pin27: /CE (Mighty Bomb Jack, Spy Vs. Spy)
						m_cart->set_ce(0x03, 0x01);
						break;
					case 0x8: // pin26: /CE, pin27: CE (Sansu 1, 2, 3 Nen, Othello)
						m_cart->set_ce(0x03, 0x02);
						break;
					case 0xc: // pin26: /CE, pin27: /CE (Seicross v2.0)
						m_cart->set_ce(0x03, 0x00);
						break;
				}
			}
			break;

		case KONAMI_VRC2:
			if (mapper == 22)
				m_cart->set_vrc_lines(0, 1, 1);
			if (mapper == 23 && !hsi_variant && !submapper)
				m_cart->set_vrc_lines(1, 0, 0);
			if (mapper == 23 && hsi_variant && !submapper)
			{
				// here there are also Akumajou Special, Crisis Force, Parodius da!, Tiny Toons which are VRC-4
				m_cart->set_vrc_lines(3, 2, 0);
				m_pcb_id = KONAMI_VRC4; // this allows for konami_irq to be installed at reset
			}
			break;

		case KONAMI_VRC4:
			if (mapper == 21 && !submapper)   // Wai Wai World 2 & Ganbare Goemon Gaiden 2 (the latter with hsi_variant)
				m_cart->set_vrc_lines(hsi_variant ? 7 : 2, hsi_variant ? 6 : 1, 0);
			if (mapper == 25 && !submapper)   // here there is also Ganbare Goemon Gaiden which is VRC-2
				m_cart->set_vrc_lines(hsi_variant ? 2 : 0, hsi_variant ? 3 : 1, 0);
			break;

		case KONAMI_VRC6:
			if (mapper == 24)
				m_cart->set_vrc_lines(1, 0, 0);
			if (mapper == 26)
				m_cart->set_vrc_lines(0, 1, 0);
			break;

		case KONAMI_VRC7:
			m_cart->set_vrc_lines((hsi_variant || submapper == 2) ? 4 : 3, 0, 0);
			break;

		case IREM_G101:
			if (hsi_variant && !submapper)
				m_cart->set_mirroring(PPU_MIRROR_HIGH); // Major League has hardwired mirroring
			else if (!submapper)
				m_cart->set_pcb_ctrl_mirror(true);
			break;

		case DIS_74X161X161X32:
			if (mapper == 70)
				m_cart->set_mirroring(PPU_MIRROR_VERT); // only hardwired mirroring makes different mappers 70 & 152
			else
				m_cart->set_pcb_ctrl_mirror(true);
			break;

		case SUNSOFT_2:
			// mapper 89 (Sunsoft-3 board) has mapper-controlled one-screen mirroring; mapper 93 (Sunsoft-3R) is hardwired and
			// keeps the header's setting (Fantasy Zone is horizontal, Shanghai vertical)
			if (mapper != 93)
				m_cart->set_pcb_ctrl_mirror(true);
			break;

		case CONY_BOARD:
			if (submapper == 0 || submapper == 2)
				pcb_id = CONY1K_BOARD;
			break;

		case UNL_LH28_LH54:
			if (vrom_size)
				m_pcb_id = (vrom_size == 0x4000) ? UNL_LE05 : UNL_LH31;
			else if (!BIT(local_options, 0))
				m_pcb_id = UNL_DH08;
			break;

		case UNL_8237:
			if (submapper == 1)
				m_pcb_id = UNL_8237A;
			break;

		case BMC_70IN1:
			if (vrom_size == 0)
				m_pcb_id = BMC_800IN1;
			break;

		case RCM_GS2004:
			if (prg_size >= 0x50000)
				m_pcb_id = RCM_GS2013;
			break;

		case HES_BOARD:
			if (hsi_variant)
				m_cart->set_pcb_ctrl_mirror(true);    // Mapper 113 is used for 2 diff boards
			break;

		case CAMERICA_BF9093:
			if (hsi_variant && !submapper)
				m_cart->set_pcb_ctrl_mirror(true);    // Mapper 71 is used for 2 diff boards
			break;

		case STD_BXROM:
			if (hsi_variant && !submapper)
				m_pcb_id = AVE_NINA01; // Mapper 34 is used for 2 diff boards
			break;

		case BANDAI_LZ93:
			// NESdev INES Mapper 153: the LZ93D50 with 8 KiB of SRAM and a 512 KiB outer PRG bank, used only by Famicom
			// Jump II; older iNES 1.0 dumps of other LZ93D50 games were sometimes given 153 too. A NES 2.0 header or
			// 512 KiB of PRG is that board (Famicom Jump II's NES 2.0 dump, not flagged in nes.hsi, otherwise got the
			// plain LZ93D50: no outer bank, no SRAM, and jumped through a pointer read from the wrong bank)
			if (hsi_variant || ines20 || (prg_size == 0x80000))
				m_pcb_id = BANDAI_FJUMP2;
			break;

		case IREM_HOLYDIVR:
			if (hsi_variant && !submapper)
				m_pcb_id = JALECO_JF16;    // Mapper 78 is used for 2 diff boards
			break;

		case WAIXING_WXZS:
			// NESdev INES Mapper 242: the Waixing RPGs have 8 KiB of battery-backed WRAM and only the 32 KiB latch; the multicarts
			// (512 KiB, or 640 KiB with the ET-113's extra chip) have no RAM and the full address latch of the ET-113 board
			if (hsi_variant)
				m_pcb_id = WAIXING_DQ8;    // Mapper 242 is used for 2 diff boards
			else if (!battery_size && prg_size >= 0x80000)
				m_pcb_id = BMC_ET113;
			break;

		case UNL_2708:
			// the board has 16K of PRG-RAM of its own (8K at $6000 and 8K at $B800, NESdev's INES Mapper 103 and krzysiobal's
			// analysis it links), whatever the header says: iNES 1.0 images default to 8K and the board reads past that
			if (prgram_size < 0x4000)
				prgram_size = 0x4000;
			break;

		case UNL_MMALEE:
			// 2K of WRAM of its own
			if (prgram_size < 0x800)
				prgram_size = 0x800;
			break;

		case UNL_LH32:
		case UNL_LH10:
		case UNL_LH51:
		case UNL_LG25:
		case BTL_PALTHENA:
		case BTL_CTC15:
		case BMC_HPXX:      // NESdev: WRAM at $6000-$7FFF is supported
		case BMC_GX121:     // NESdev: WRAM at $6000-$7FFF (18000-in-1's Super Mario Bros. 3 needs it)
			// these boards have 8K of WRAM of their own
			if (prgram_size < 0x2000)
				prgram_size = 0x2000;
			break;

		case BTL_MARIOBABY:
			// Mapper 42 is used for 3 diff boards: Bio Miracle Bokutte Upa (Mario Baby) has 8K of CHR-RAM and ignores the CHR
			// select, the Ai Senshi Nicol conversions have CHR-ROM (NESdev, INES Mapper 042), and the Green Beret conversions
			// (160K of PRG, NES 2.0 submapper 2) are the UNL-AC08 board, whose registers are at $8000/$8001
			if (ines20 && submapper == 2)
				m_pcb_id = UNL_AC08;
			else if (hsi_variant || vrom_size)
				m_pcb_id = BTL_AISENSHINICOL;
			break;

		case TAITO_X1_017:
			mapper_sram_size = m_cart->get_mapper_sram_size();
			break;

		case TAITO_X1_005:
			if (mapper == 207)
				m_cart->set_x1_005_alt(true);
			mapper_sram_size = m_cart->get_mapper_sram_size();
			break;

		case NAMCOT_163:
			// the chip's 128 bytes of RAM are save RAM only on boards with a battery (NESdev INES Mapper 019)
			if (BIT(local_options, 1))
				mapper_sram_size = m_cart->get_mapper_sram_size();
			else
				m_cart->set_mapper_sram_size(0);   // no battery: nothing is saved
			break;

		case BMC_EL860947C:
			m_cart->set_outer_prg_size(128);
			break;

		case BMC_EL861121C:
			m_cart->set_outer_prg_size(256);
			break;

		//FIXME: we also have to fix Action 52 PRG loading somewhere...

		case BANDAI_DATACH:
			// a mapper 157 image is the game ROM itself: it is mapped by the Datach base board, which reads its own PRG
			// when no Datach ROM is mounted in the subslot (no barcode cards and no second EEPROM in this case)
			break;
	}

	// iNES mapper 180 (Crazy Climber) is the UNROM PCB with an AND gate instead of the OR gate, so bus conflicts occur
	if (mapper == 180)
		bus_conflict = true;

	// iNES mapper 13 is only Nintendo's NES-CPROM (Videomation), which NESdev lists as subject to bus conflicts
	if (mapper == 13)
		bus_conflict = true;

	// iNES mapper 144 is the AGCI 50282 board (Death Race): a Color Dreams board where only D0 comes from the ROM, so the other
	// data bits still lose to the ROM in a bus conflict (NESdev: EffectiveData = ROM[address] & (WrittenData | 1))
	if (mapper == 144)
		bus_conflict = true;

	// iNES mapper 72 is Jaleco's JF-17, which NESdev lists as subject to bus conflicts (a 74161 latch on the data bus)
	if (mapper == 72)
		bus_conflict = true;

	// UNROM 512 (mapper 30): NES 2.0 submapper 1 = no bus conflicts, 2 = bus conflicts (as used by NES 2.0 headers, e.g.
	// No-Intro's); otherwise NESdev's rule - the battery flag marks the self-flashable board, which has no bus conflicts.
	// Whether the board is flashable always follows the battery flag (submapper 1 boards without it are not flashable)
	if (mapper == 30)
	{
		// NESdev UNROM 512 submappers: 0 = bus conflicts unless the battery bit is set, 1/3/4 = none, 2 = present
		bus_conflict = (submapper == 2) ? true : (submapper == 0) ? !BIT(local_options, 1) : false;
		m_cart->set_unrom512_flashable(BIT(local_options, 1));
		m_cart->set_unrom512_submapper(submapper);
	}

	// iNES Mapper 159 (NESdev): LZ93D50 with 24C01 - its registers are at $8000-$FFFF only
	if (mapper == 159)
		m_cart->set_bandai_submapper(5);

	// Finally turn off bus conflict emulation, because the pirate variants of the boards are bus conflict free and games would glitch
	m_cart->set_bus_conflict(bus_conflict);

	// SETUP step 4: logging what we have found
	logerror("Loaded game in %s format:\n", ines20 ? "NES 2.0" : "iNES");
	logerror("-- Mapper: %u\n", mapper);
	if (ines20)
		logerror("-- Submapper: %u\n", header[8] >> 4);
	logerror("-- PRG 0x%x (%d x 16k chunks)\n", prg_size, prg_size / 0x4000);
	logerror("-- VROM 0x%x (%d x 8k chunks)\n", vrom_size, vrom_size / 0x2000);
	logerror("-- VRAM 0x%x (%d x 8k chunks)\n", vram_size, vram_size / 0x2000);
	logerror("-- Mirroring: %s\n", BIT(header[6], 0) ? "Vertical" : "Horizontal");
	if (battery_size)
		logerror("-- Battery found\n");
	if (m_cart->get_trainer())
		logerror("-- Trainer found\n");
	if (m_cart->get_four_screen_vram())
		logerror("-- 4-screen VRAM\n");
	if (ines20)
	{
		logerror("-- PRG NVWRAM: %d\n", header[10] >> 4);
		logerror("-- PRG WRAM: %d\n", header[10] & 0x0f);
		logerror("-- CHR NVWRAM: %d\n", header[11] >> 4);
		logerror("-- CHR WRAM: %d\n", header[11] & 0x0f);

		static const char *timing[] = { "NTSC", "PAL", "Multi-region", "Dendy" };
		logerror("-- CPU/PPU Timing: %s\n", timing[header[12] & 3]);
	}
	else
		logerror("-- TV System: %s\n", ((header[10] & 3) == 0) ? "NTSC" : (header[10] & 1) ? "Both NTSC and PAL" : "PAL");

	// BMC-GOLDEN260IN1 (mapper 235) has four 1 MiB ROM sockets chosen by A9-A8 and its menu checks each socket's checksum, so a
	// dump that leaves the empty sockets out has to be put back where the chips sit: NESdev says only sockets 0 and 2 are
	// filled on the 2 MiB cart, and the software list puts the 260 in 1's chips in sockets 0, 2 and 3 with $FF in the rest
	static const uint8_t golden260_sockets[3] = { 0, 2, 3 };
	const bool golden260_dump = (mapper == 235) && (prg_size >= 0x100000) && (prg_size < 0x400000) && !(prg_size & 0xfffff);
	const uint32_t prg_file_size = prg_size;
	if (golden260_dump)
		prg_size = 0x400000;

	// SETUP step 5: allocate pointers for PRG/VROM
	if (prg_size)
		m_cart->prg_alloc(prg_size);
	if (vrom_size)
		m_cart->vrom_alloc(vrom_size);

	// if there is a trainer, skip it for the moment
	if (m_cart->get_trainer())
		fseek(0x210, SEEK_SET);

	// SETUP step 6: at last load the data!
	// Read in the program chunks
	if (prg16k)
	{
		fread(m_cart->get_prg_base(), 0x4000);
		memcpy(m_cart->get_prg_base() + 0x4000, m_cart->get_prg_base(), 0x4000);
	}
	else if (golden260_dump)
	{
		memset(m_cart->get_prg_base(), 0xff, m_cart->get_prg_size());
		for (int chip = 0; chip < prg_file_size / 0x100000; chip++)
			fread(m_cart->get_prg_base() + golden260_sockets[chip] * 0x100000, 0x100000);
	}
	else
		fread(m_cart->get_prg_base(), m_cart->get_prg_size());
#if SPLIT_PRG
	{
		FILE *prgout;
		char outname[255];

		sprintf(outname, "%s.prg", filename());
		prgout = fopen(outname, "wb");
		if (prgout)
		{
			fwrite(m_cart->get_prg_base(), 1, 0x4000 * m_cart->get_prg_size(), prgout);
			osd_printf_error("Created PRG chunk\n");
		}

		fclose(prgout);
	}
#endif

	// Read in any chr chunks
	if (m_cart->get_vrom_size())
		fread(m_cart->get_vrom_base(), m_cart->get_vrom_size());

#if SPLIT_CHR
	if (state->m_chr_chunks > 0)
	{
		FILE *chrout;
		char outname[255];

		sprintf(outname, "%s.chr", filename());
		chrout= fopen(outname, "wb");
		if (chrout)
		{
			fwrite(m_cart->get_vrom_base(), 1, m_cart->get_vrom_size(), chrout);
			osd_printf_error("Created CHR chunk\n");
		}
		fclose(chrout);
	}
#endif

	// NES 2.0 miscellaneous ROM data follows the CHR-ROM; mapper 355 keeps the program of its PIC16C54 there, and mapper 413
	// its DPCM sample data
	if (ines20 && (header[14] & 3) && ((m_pcb_id == RCM_3DBLOCK) || (m_pcb_id == BATMAP_SRRX)))
	{
		const uint32_t misc_size = length() - ftell();
		if (misc_size)
		{
			m_cart->misc_rom_alloc(misc_size);
			fread(m_cart->get_misc_rom_base(), misc_size);
		}
	}

	// SETUP steps 7: allocate the remaining pointer, when needed
	if (vram_size)
		m_cart->vram_alloc(vram_size);
	if (prgram_size || m_cart->get_trainer())
	{
		if (prgram_size)
			m_cart->prgram_alloc(prgram_size);
		else
			m_cart->prgram_alloc(0x2000);
		if (m_cart->get_trainer())
		{
			fseek(0x10, SEEK_SET);
			fread(m_cart->get_prgram_base() + 0x1000, 0x200);
		}
	}


	// Attempt to load a battery file for this ROM
	// A few boards have internal RAM with a battery (MMC6, Taito X1-005 & X1-017, etc.)
	if (battery_size || mapper_sram_size)
	{
		uint32_t tot_size = battery_size + mapper_sram_size;
		std::vector<uint8_t> temp_nvram(tot_size);
		battery_load(&temp_nvram[0], tot_size, 0x00);
		if (battery_size)
		{
			//printf("here %d\n", battery_size);
			m_cart->battery_alloc(battery_size);
			memcpy(m_cart->get_battery_base(), &temp_nvram[0], battery_size);
		}
		if (mapper_sram_size)
			memcpy(m_cart->get_mapper_sram_base(), &temp_nvram[battery_size], m_cart->get_mapper_sram_size());
	}
}

const char * nes_cart_slot_device::get_default_card_ines(get_default_card_software_hook &hook, const uint8_t *ROM, uint32_t len) const
{
	uint16_t mapper;
	uint8_t submapper = 0;
	bool ines20 = false;
	std::string mapinfo;
	int pcb_id = 0, mapint1 = 0, mapint2 = 0, mapint3 = 0, mapint4 = 0;
	int hsi_variant = 0;

	mapper = (ROM[6] & 0xf0) >> 4;

	switch (ROM[7] & 0xc)
	{
		case 0x4:
		case 0xc:
			// probably the header got corrupted: don't trust upper bits for mapper
			break;

		case 0x8:   // it's NES 2.0 format
			ines20 = true;
			[[fallthrough]];
		case 0x0:
		default:
			mapper |= ROM[7] & 0xf0;
			break;
	}

	// use info from nes.hsi if available!
	if (hook.hashfile_extrainfo(mapinfo))
	{
		if (4 == sscanf(mapinfo.c_str(),"%d %d %d %d", &mapint1, &mapint2, &mapint3, &mapint4))
		{
			if (!ines20)
			{
				/* image is present in nes.hsi: overwrite the header settings with these */
				mapper = mapint1;
				hsi_variant = (mapint2 & 0xf0) >> 4; // this is used to differentiate among variants of the same Mapper (see below)
			}
			else if (mapint1 == (mapper | ((ROM[8] & 0x0f) << 8)))
				hsi_variant = (mapint2 & 0xf0) >> 4;   // NES 2.0 header kept; only MAME's variant code is taken from nes.hsi
		}
	}

	// use extended NES 2.0 info if available!
	if (ines20)
	{
		mapper |= (ROM[8] & 0x0f) << 8;
		// read submappers (based on 20140116 specs)
		submapper = (ROM[8] & 0xf0) >> 4;
	}

	ines_mapr_setup(mapper, &pcb_id);
	// NES 2.0 mapper 176 is the 8025 enhanced MMC3 family, selected by submapper:
	// 0 = LP-8002KB/SFC-12B (Super24in1SC03), 1 = FK-xxx/BS-xxx (FK23C/FK23CA), 2 = FS005, 3 = JX9003B, 4 = GameStar, 5 = HST-162:
	// all of them are emulated by nes_8025_device, which follows the NESdev register description.
	// The older BMC-SUPER24IN1SC03 and BMC-FK23C/FK23CA boards remain for UNIF files and software list entries.
	// iNES 1.0 images have no submapper: the legacy Xiao Zhuan Yuan board (pirate.cpp) is kept for the 128 KiB PRG / 64 KiB CHR size only (see below).
	if (mapper == 176 && ines20)
	{
		if (submapper <= 5)
			pcb_id = BMC_8025;
		else
			logerror("Unimplemented NES 2.0 mapper 176 submapper: %d\n", submapper);
	}

	// iNES 1.0 has no submapper: only the 128 KiB PRG / 64 KiB CHR Xiao Zhuan Yuan carts keep the legacy board,
	// any other mapper 176 image is an FK23C-type multicart
	if (mapper == 176 && !ines20 && !(ROM[4] == 8 && ROM[5] == 8))
		pcb_id = BMC_8025;

	// mapper 294 images with CHR-ROM are MMC3-type multicarts, see call_load_ines
	if (mapper == 294 && ROM[5])
		pcb_id = BMC_FAMILY_4646;

	// iNES 1.0 mapper 4 images flagged in nes.hsi with hsi_variant 1 are MMC6 (HKROM), as NES 2.0 submapper 1 would say
	if (mapper == 4 && !ines20 && hsi_variant == 1)
		pcb_id = STD_HKROM;

	// mapper 111 is GTROM, which has no CHR-ROM; with CHR-ROM it is the old assignment, a Chinese Ninja Ryukenden (MMC1 variant)
	if (mapper == 111 && ROM[5])
		pcb_id = UNL_NINJARYU;

	// handle submappers
	if (submapper)
	{
		// 004: submapper 1 = MMC6 (HKROM), submapper 5 = Waixing T9552
		if (mapper == 4 && submapper == 1)
			pcb_id = STD_HKROM;
		else if (mapper == 4 && submapper == 5)
			pcb_id = WAIXING_SECURITY;
		// 001: MMC1 (other submappers are deprecated)
		// 001: submapper 5 = SEROM/SHROM/SH1ROM (fixed 32K PRG), handled by the SxROM board
		// 021, 023, 025: VRC4 / VRC2 (NESdev NES 2.0 submappers 1-3: 23/3 and 25/3 are VRC2, the rest VRC4)
		if ((mapper == 21 || mapper == 23 || mapper == 25) && submapper >= 1 && submapper <= 3)
			pcb_id = ((mapper == 23 || mapper == 25) && submapper == 3) ? KONAMI_VRC2 : KONAMI_VRC4;
		else if (mapper == 21 || mapper == 23 || mapper == 25)
		{
			// 021, 023, 025: VRC4
			int line_1 = submapper & 0x07;
			int line_2 = (submapper & 0x08) ? line_1 + 1 : line_1 - 1;
			if (line_2 >= 0 && line_2 <= 7)
				pcb_id = KONAMI_VRC4;
			else if (submapper == 15)
				pcb_id = KONAMI_VRC2;
		}
		// iNES Mapper 016 submapper 4: Bandai FCG-1/2
		else if (mapper == 16 && submapper == 4)
		{
			pcb_id = BANDAI_FCG;
		}
		// iNES Mapper 034
		else if (mapper == 34 && submapper == 1)
		{
			pcb_id = AVE_NINA01; // Mapper 34 is used for 2 diff boards
		}
		// iNES Mapper 114: submapper 1 (Boogerman) uses a different register scrambling pattern
		else if (mapper == 114 && submapper == 1)
		{
			pcb_id = SUPERGAME_BOOGERMAN;
		}
		// iNES Mapper 196: submapper 1 has the MMC3's A0 input on CPU A1 (Fight Street VI), submapper 2 on CPU A2, submapper 3
		// has it on CPU A1 with mapper 189-style PRG banking (the same board with the PRG register of mapper 189)
		else if (mapper == 196 && (submapper == 1 || submapper == 3))
		{
			pcb_id = UNL_FS6;
		}
		// iNES Mapper 078
		else if (mapper == 78)
		{
			if (submapper == 1)
				pcb_id = JALECO_JF16;    // Mapper 78 is used for 2 diff boards
			else if (submapper == 3)
				pcb_id = IREM_HOLYDIVR;
		}
		// iNES Mapper 116
		else if (mapper == 116 && submapper == 2)
		{
			pcb_id = SOMARI_HUANG2; // Mapper 116 is used for 2 diff boards
		}
		// NES 2.0 submapper 3: the reset-based SL-FC5-1 Mario 5-in-1 (NESdev INES Mapper 116)
		else if (mapper == 116 && submapper == 3)
		{
			pcb_id = SOMARI_SL12_MC;
		}
		// iNES Mapper 232
		else if (mapper == 232 && submapper == 1)
		{
			pcb_id = CAMERICA_BF9096_ALT;
		}
	}

	// 210: Namco 175 (submapper 1) or Namco 340 (submapper 2); without a submapper, NESdev's rule is a 175 when the battery
	// bit is set and a 340 otherwise, which fits every commercial game (INES Mapper 210)
	if (mapper == 210)
		pcb_id = (submapper == 1 || (submapper != 2 && BIT(ROM[6], 1))) ? NAMCOT_175 : NAMCOT_340;

	// solve mapper conflicts
	switch (pcb_id)
	{
		case STD_NROM:
			if (ROM[4] == 3)
				pcb_id = STD_NROM368;
			break;

		case STD_SXROM:
			// only A Ressha de Ikou uses SZROM and it can be detected by its profile: 8K WRAM, 8K BWRAM, 16K CHR ROM
			if (mapper == 1 && ines20 && ROM[10] == 0x77 && ROM[5] == 2)
				pcb_id = STD_SZROM;
			break;

		case KONAMI_VRC2:
			if (mapper == 23 && hsi_variant && !submapper)
				pcb_id = KONAMI_VRC4; // this allows for konami_irq to be installed at reset
			break;

		case STD_BXROM:
			if (hsi_variant && !submapper)
				pcb_id = AVE_NINA01; // Mapper 34 is used for 2 diff boards
			break;

		case BANDAI_LZ93:
			// as in call_load_ines: a NES 2.0 header or 512 KiB of PRG (32 * 16 KiB) is the Famicom Jump II board
			if (hsi_variant || ines20 || (ROM[4] == 32))
				pcb_id = BANDAI_FJUMP2;
			break;

		case IREM_HOLYDIVR:
			if (hsi_variant && !submapper)
				pcb_id = JALECO_JF16;    // Mapper 78 is used for 2 diff boards
			break;

		case WAIXING_WXZS:
			// as in call_load_ines: a battery-less image of 512 KiB or more is a multicart (ET-113 board)
			if (hsi_variant)
				pcb_id = WAIXING_DQ8;    // Mapper 242 is used for 2 diff boards
			else if (!(ROM[6] & 0x02) && ROM[4] >= 32)
				pcb_id = BMC_ET113;
			break;

		case BTL_MARIOBABY:
			// Mapper 42 is used for 3 diff boards, told apart by the submapper and the CHR-ROM (see call_load_ines)
			if (ines20 && submapper == 2)
				pcb_id = UNL_AC08;
			else if (hsi_variant || ROM[5] || (ines20 && (ROM[9] & 0xf0)))
				pcb_id = BTL_AISENSHINICOL;
			break;

		case CONY_BOARD:
			if (submapper == 0 || submapper == 2)
				pcb_id = CONY1K_BOARD;         // Mapper 83 is used for 3 diff boards
			break;

		case BTL_BATMANFS:                             // Mapper 417 is used for 2 diff boards
			if (submapper == 1)
				pcb_id = BTL_ROBOCOPFS;
			break;

		case UNL_LH28_LH54:                            // Mapper 108 is used for 4 diff boards
			if (submapper >= 1 && submapper <= 4)      // NES 2.0 submapper
				pcb_id = (submapper == 1) ? UNL_DH08 : (submapper == 2) ? UNL_LH31 : (submapper == 3) ? UNL_LH28_LH54 : UNL_LE05;
			else if (ROM[5])                           // NESdev's rule without a submapper: more than 16 KiB of CHR-ROM is LH31, otherwise LE05
				pcb_id = (ROM[5] <= 2) ? UNL_LE05 : UNL_LH31;
			else if (!BIT(ROM[6], 0))                  // CHR-RAM: horizontal mirroring is DH-08, otherwise LH28/LH54
				pcb_id = UNL_DH08;
			break;

		case UNL_8237:                                 // Mapper 215 is used for 2 diff boards
			if (submapper == 1)
				pcb_id = UNL_8237A;
			break;

		case BMC_70IN1:                                // Mapper 236 is used for 2 diff boards
			if (ROM[5] == 0)
				pcb_id = BMC_800IN1;
			break;

		case RCM_GS2004:                               // Mapper 283 is used for 2 diff boards
			if (ROM[4] >= 20)
				pcb_id = RCM_GS2013;
			break;
	}

	// 034: without a submapper, NESdev's rule is BNROM for 0-8 KiB of CHR and NINA-001/002 above that (INES Mapper 034)
	if (mapper == 34 && submapper == 0 && (ROM[5] > 1 || (ines20 && (ROM[9] & 0xf0))))
		pcb_id = AVE_NINA01;

	return nes_get_slot(pcb_id);
}
