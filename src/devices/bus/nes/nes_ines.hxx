// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
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
	{ 35, UNL_SC127 },
	{ 36, TXC_STRIKEW },
	{ 37, PAL_ZZ },
	{ 38, DIS_74X161X138 },
	{ 39, UNL_STUDYNGAME },
	{ 40, BTL_SMB2JA },
	{ 41, CALTRON_6IN1 },
	{ 42, BTL_MARIOBABY },  // ai senshi nicole too, changed by crc_hack
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
	{ 69, SUNSOFT_FME7 },
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
	{ 82, TAITO_X1_017 },
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
	// 111 GTROM 512 + Flash, currently unsupported
	// Old mapper 111: Ninja Ryukenden Chinese - currently supported in software list only
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
	// 122 Unused
	{ 123, UNL_H2288 },
	// 124 Super Game Mega Type III bootleg arcade board. Emulated in MAME as supergm3.
	{ 125, UNL_LH32 },  // Monty no Doki Doki Daidassou - FDS Conversion
	{ 126, BMC_PJOY84 },
	// 127 Double Dragon II Japan pirate. Dump available?
	// 128 1994 Super HiK 4-in-1 pirate. Dump available?
	// 129 Unused
	// 130 Unused
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
	// 161 Unused
	{ 162, WAIXING_FS304 },  // not confirmed, but a lot of chinese releases use it like this...
	{ 163, NANJING_BOARD },
	{ 164, WAIXING_FFV },
	{ 165, WAIXING_SH2 },
	{ 166, SUBOR_TYPE1 },
	{ 167, SUBOR_TYPE0 },
	{ 168, UNL_RACERMATE },
	// 169 Unused
	// 170 Fujiya
	{ 171, KAISER_KS7058 },
	{ 172, TXC_DUMARACING },
	{ 173, TXC_MJBLOCK },
	{ 174, BMC_2751 },
	{ 175, KAISER_KS7022 },
	{ 176, UNL_XIAOZY },
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
	{ 199, WAIXING_TYPE_G },
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
	{ 224, WAIXING_TYPE_J },    // (according to NEStopia source, it's MMC3 with more WRAM)
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
	{ 235, BMC_GOLD150 },   // 235 Golden Game x-in-1 - Unsupported
	{ 236, BMC_70IN1 },
	{ 237, BMC_TELETUBBIES },
	{ 238, UNL_603_5052 },
	// 239 Unused
	{ 240, CNE_SHLZ },
	{ 241, TXC_COMMANDOS },
	{ 242, WAIXING_WXZS },
	{ 243, SACHEN_74LS374_ALT },
	{ 244, CNE_DECATHLON },
	{ 245, WAIXING_TYPE_H },
	{ 246, CNE_FSB },
	// 247 Unused
	// 248 Unused
	{ 249, WAIXING_SECURITY },
	{ 250, NITRA_TDA },
	// 251 Shen Hua Jian Yun III?? - Unsupported
	{ 252, WAIXING_SGZ },
	// 253 Super 8-in-1 99 King Fighter?? - Unsupported
	{ 254, BTL_PIKACHUY2K },
	{ 255, BMC_72IN1 },         // duplicate of mapper 225

	// NES 2.0
	// 256 OneBus Famiclones
	// 257 UNIF MAPR PEC-586?
	{ 258, UNL_158B },
	{ 259, BMC_F15 },
	// 260 HP10xx/HP20xx multicarts?
	{ 261, BMC_810544C },
	{ 262, SACHEN_SHERO },
	{ 263, UNL_KOF97 },
	{ 264, YOKO_BOARD },
	{ 265, BMC_T262 },
	{ 266, UNL_CITYFIGHT },
	{ 267, BMC_EL861121C },
	{ 268, SMD133_BOARD },
	// 269 mc_gx121 seems to be a PnP, but there are two actual multicarts for this mapper?
	// 270 multicarts on OneBus Famiclones
	// 271 TXC 4 in 1 MGC-026, not in nes.xml?
	// 272 Akumajo Special bootleg not in nes.xml
	// 273 Gremlins 2 bootleg, related to pirate gremlin2h or unk2?
	{ 274, BMC_80013B },
	// 275 Unused
	// 276 Unused
	// 277 Unused
	// 278 Unused
	// 279 Unused
	// 280 Unused
	// 281 seems to be mc_sh4b and many other JY multicarts not in nes.xml?
	// 282 more JY multicarts not in nes.xml?
	{ 283, RCM_GS2004 },           // and RCM_GS2013
	// 284 UNL_DRIPGAME, not in nes.xml
	{ 285, BMC_A65AS },
	{ 286, BMC_BENSHIENG },
	{ 287, BMC_411120C },
	{ 288, BMC_GKCXIN1 },
	{ 289, BMC_60311C },
	{ 290, BMC_NTD_03 },
	{ 291, BMC_NT639 },
	{ 292, UNL_BMW8544 },          // Dragon Fighter by Flying Star
	// 293 NewStar multicarts, do we have these in nes.xml?
	{ 294, BMC_FAMILY_4646 },      // FIXME: is this really exactly the same as mapper 134?
	// 295 JY multicarts not yet in nes.xml
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
	// 310 variant of mapper 125?
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
	// 355 Hwang Shinwei 3-D Block etc, currently has unemulated PIC16C54
	{ 356, BMC_JY208 },
	// 357 Bit Corp 4-in-1 (ID 4602)
	// 358 JY multicarts, variant of mapper 282
	// 359 BMC-SB-5013 multicarts
	// 360 Bit Corp 31-in-1 (ID 3150) (has five accessible DIP switches!)
	{ 361, BMC_YY841101C },
	{ 362, BMC_830506C },
	// 363 variant of mapper 358?
	{ 364, BMC_830832C },
	// 365 is this asderp95 in nes.xml?
	{ 366, BMC_GN45 },
	// 367 7-in-1 cart that is a close variant of mapper 205
	{ 368, BTL_YUNG08 },            // SMB2 FDS conversion
	// 369 Super Mario Bros Party multicart
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
	// 386 JY-090 multicart
	// 387 various JY multicarts
	// 388 various JY multicarts
	{ 389, CALTRON_9IN1 },
	// 390 variant of mapper 236?
	// 391 BS-110 MMC3 clone
	{ 392, BMC_00202650 },
	{ 393, BMC_820720C },
	// 394 Realtec HSK007 multicart
	// 395 Realtec 8210 multicarts
	{ 396, BMC_850437C },
	// 397 JY-082 multicart, not in nes.xml?
	// 398 JY-048 multicart, not in nes.xml?
	{ 399, BATMAP_000 },           // homebrew game Star Versus
	// 400 retroUSB (Sealie?) 8-bit XMAS 2017
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
	// 412 INTV 10-in-1 PnP 2nd edition
	{ 413, BATMAP_SRRX },          // homebrew game Super Russian Roulette
	// 414 9999999-in-1 multicart
	{ 415, BTL_0353 },             // Lucky (Roger) Rabbit FDS conversion
	{ 416, BMC_N32_4IN1 },
	{ 417, BTL_BATMANFS },         // "Fine Studio" Batman bootleg
	{ 418, UNL_LH42 },             // Highway Star Whirlwind Manu bootleg
	// 419 VT03 PnPs
	// 420 Kasheng A971210 board
	// 421 JY SC871115C board
	// 422 BS-300 etc multicarts related to mappers 126 and 534
	// 423 Lexibook PnP
	// 424 Lexibook PnP
	// 425 Cube Tech PnP
	// 426 PnP
	// 427 PnP
	{ 428, BMC_TF2740 },
	// 429 Unused
	// 430 Unused
	{ 431, BMC_GN91B },
	// 432 Realtec 8090
	{ 433, BMC_NC20MB },
	// 434 S-009
	// 435...442 Unused
	// 443 NC3000M multicart
	// 444 NC7000M multicart
	// 445...511 Unused
	// 512 probably the correct MMC3 clone for chuugokt in nes.xml
	{ 513, SACHEN_SA9602B },
	// 514 seems to be for skaraok, currently set to UNKNOWN in nes.xml
	// 515 Korean Family Noraebang karaoke cart with expansion cart, mic, and YM2413!
	{ 516, COCOMA_BOARD },
	// 517 another Korean karaoke cart with mic
	// 518 Subor UNL-DANCE2000 and a few others
	{ 519, UNL_EH8813A },          // Dr Mario II Chinese pirate
	{ 520, BTL_2YUDB },
	{ 521, DREAMTECH_BOARD },      // Korean Igo
	{ 522, UNL_LH10 },             // Fuuun Shaolin Kyo FDS conversion
	// { 523, UNKNOWN }, likely fengshnb or a clone not yet in nes.xml
	{ 524, BTL_900218 },           // Lord of King pirate
	{ 525, KAISER_KS7021A },       // GetsuFumaDen pirate (and maybe a Contra?)
	// 526 sangochu clone not yet in nes.xml?
	{ 527, UNL_AX40G },            // Fudou Myouou Den pirate
	// 528 1995 New Series Super 2-in-1 multicart not in nes.xml
	{ 529, UNL_T230 },             // Datach Dragon Ball Z IV bootleg
	{ 530, UNL_AX5705 },           // Super Mario Bros Pocker Mali
	// 531 Used by Asder PC-95 Famicom clone built into a keyboard
	// 532 Emulator only mapper for Chinese version of sangoht2?
	{ 533, SACHEN_3014 },          // Dong Dong Nao II
	// 534 Are these all PnPs? Is one mc_101 or a clone not in nes.xml?
	{ 535, UNL_LH53 },             // Nazo no Murasamejo FDS conversion
	// 536 and 537 Waixing FS303, mapper 195 variants?
	// { 538, BTL_60106416L }, Exciting Soccer bootleg, not in nes.xml (available baddump needs banks rearranged?)
	{ 539, BTL_PALTHENA },         // Hikari Shinwa (Kid Icarus) FDS conversion
	// 540 for mstrfgt6 in nes.xml or a variant of it not in nes.xml?
	{ 541, BMC_LITTLECOM160 },
	// 542 Chairman Mao's 100th anniversary cart? You've got to be kidding me.
	{ 543, BMC_SRPG_5IN1 },
	// 544 another alt of sango2ht/sanguo2a?
	// 545 4 in 1 (ST-80) multicart, not in nes.xml?
	// 546 10 in 1 Tenchi wo Kurau multicart, not in nes.xml?
	// 547 Konami QTa adapter games
	// { 548, BTL_CTC15 },            // Almana no Kiseki alt FDS conversion (dump available?)
	{ 549, KAISER_KS7016B },       // Meikyuu Jiin Dababa alt FDS conversion
	{ 550, BMC_JY820845C },
	{ 551, JNCOTA_KT1001 },
	// 552 TAITO_X1_017, this is a correction of mapper 82. We should drop 82 and only support the accurate dumps of 552?
	{ 553, SACHEN_3013 },          // Dong Dong Nao 1
	{ 554, KAISER_KS7010 },        // Akumajo Dracula FDS conversion
	{ 555, STD_EVENT2 },
	// 556 JY-215 multicart
	{ 557, UNL_LG25 },             // Moero TwinBee FDS conversion
	// 558 some games on YC-03-09 board (related to mappers 162-164)
	// 559...4095 Unused
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
	int crc_hack = 0;
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
			/* image is present in nes.hsi: overwrite the header settings with these */
			mapper = mapint1;
			local_options = mapint2 & 0x0f;
			crc_hack = (mapint2 & 0xf0) >> 4; // this is used to differentiate among variants of the same Mapper (see below)
			prg16k = (mapint3 == 1);
			prg_size = prg16k ? 2 * 0x4000 : mapint3 * 0x4000;
			vrom_size = mapint4 * 0x2000;
			logerror("NES.HSI info: %d %d %d %d\n", mapint1, mapint2, mapint3, mapint4);
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

	// handle submappers
	if (submapper)
	{
		// 001: MMC1 (other submappers are deprecated)
		if (mapper == 1 && submapper == 5)
			logerror("Unimplemented NES 2.0 submapper: SEROM/SHROM/SH1ROM.\n");
		// 002, 003, 007: UxROM, CNROM, AxROM
		else if (mapper == 2 && submapper == 2)
			bus_conflict = true;
		else if (mapper == 3 && submapper == 2)
			bus_conflict = true;
		else if (mapper == 7 && submapper == 2)
			bus_conflict = true;
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
		// 021, 023, 025: VRC4 / VRC2
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
		// iNES Mapper 034
		else if (mapper == 34 && submapper == 1)
		{
			pcb_id = AVE_NINA01; // Mapper 34 is used for 2 diff boards
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
		// iNES Mapper 185
		else if (mapper == 185)
		{
			int ce_state = (submapper & 0x0c) >> 2;
			m_cart->set_ce(0x03, ce_state);
		}
		// iNES Mapper 232
		else if (mapper == 210 && submapper == 1)
		{
			submapper = 0;
			logerror("Unimplemented NES 2.0 submapper: CAMERICA-BF9096.\n");
		}
		// 268: SMD133 boards
		else if (mapper == 268)
		{
			if (submapper == 0)
				m_cart->set_smd133_addr(0x6000);
			else if (submapper == 1)
				m_cart->set_smd133_addr(0x5000);
			else
				logerror("Unimplemented NES 2.0 submapper: %d\n", submapper);
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
			else
				logerror("Unimplemented NES 2.0 submapper: %d\n", submapper);
		}
		else if (submapper)
		{
			submapper = 0;
			logerror("Undocumented NES 2.0 submapper, please report it to the MAME boards!\n");
		}
	}

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
			vram_size |= 0x80 << (((header[11] & 0xf0) >> 4) - 1);
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
				switch (crc_hack)
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
			if (mapper == 23 && !crc_hack && !submapper)
				m_cart->set_vrc_lines(1, 0, 0);
			if (mapper == 23 && crc_hack && !submapper)
			{
				// here there are also Akumajou Special, Crisis Force, Parodius da!, Tiny Toons which are VRC-4
				m_cart->set_vrc_lines(3, 2, 0);
				m_pcb_id = KONAMI_VRC4; // this allows for konami_irq to be installed at reset
			}
			break;

		case KONAMI_VRC4:
			if (mapper == 21 && !submapper)   // Wai Wai World 2 & Ganbare Goemon Gaiden 2 (the latter with crc_hack)
				m_cart->set_vrc_lines(crc_hack ? 7 : 2, crc_hack ? 6 : 1, 0);
			if (mapper == 25 && !submapper)   // here there is also Ganbare Goemon Gaiden which is VRC-2
				m_cart->set_vrc_lines(crc_hack ? 2 : 0, crc_hack ? 3 : 1, 0);
			break;

		case KONAMI_VRC6:
			if (mapper == 24)
				m_cart->set_vrc_lines(1, 0, 0);
			if (mapper == 26)
				m_cart->set_vrc_lines(0, 1, 0);
			break;

		case KONAMI_VRC7:
			m_cart->set_vrc_lines((crc_hack || submapper == 2) ? 4 : 3, 0, 0);
			break;

		case IREM_G101:
			if (crc_hack && !submapper)
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
			if (mapper == 93)
				m_cart->set_mirroring(PPU_MIRROR_VERT); // only hardwired mirroring makes different mappers 89 & 93
			else
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
			if (crc_hack)
				m_cart->set_pcb_ctrl_mirror(true);    // Mapper 113 is used for 2 diff boards
			break;

		case CAMERICA_BF9093:
			if (crc_hack && !submapper)
				m_cart->set_pcb_ctrl_mirror(true);    // Mapper 71 is used for 2 diff boards
			break;

		case STD_BXROM:
			if (crc_hack && !submapper)
				m_pcb_id = AVE_NINA01; // Mapper 34 is used for 2 diff boards
			break;

		case BANDAI_LZ93:
			if (crc_hack)
				m_pcb_id = BANDAI_FJUMP2;   // Mapper 153 is used for 2 diff boards
			break;

		case IREM_HOLYDIVR:
			if (crc_hack && !submapper)
				m_pcb_id = JALECO_JF16;    // Mapper 78 is used for 2 diff boards
			break;

		case WAIXING_WXZS:
			if (crc_hack)
				m_pcb_id = WAIXING_DQ8;    // Mapper 242 is used for 2 diff boards
			break;

		case BMC_GOLD_7IN1:
			if (crc_hack)
				m_pcb_id = BMC_MARIOPARTY_7IN1;    // Mapper 52 is used for 2 diff boards
			break;

		case BTL_MARIOBABY:
			if (crc_hack)
				m_pcb_id = BTL_AISENSHINICOL;    // Mapper 42 is used for 2 diff boards
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
			mapper_sram_size = m_cart->get_mapper_sram_size();
			break;

		case BMC_EL860947C:
			m_cart->set_outer_prg_size(128);
			break;

		case BMC_EL861121C:
			m_cart->set_outer_prg_size(256);
			break;

		//FIXME: we also have to fix Action 52 PRG loading somewhere...

		case BANDAI_DATACH:
			fatalerror("Bandai Datach games have to be mounted in the Datach subslot!\n");
			break;
	}

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

	// SETUP step 5: allocate pointers for PRG/VROM
	if (prg_size)
		m_cart->prg_alloc(prg_size, tag());
	if (vrom_size)
		m_cart->vrom_alloc(vrom_size, tag());

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
	int crc_hack = 0;

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
			/* image is present in nes.hsi: overwrite the header settings with these */
			mapper = mapint1;
			crc_hack = (mapint2 & 0xf0) >> 4; // this is used to differentiate among variants of the same Mapper (see below)
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

	// handle submappers
	if (submapper)
	{
		// 001: MMC1 (other submappers are deprecated)
		if (mapper == 1 && submapper == 5)
			logerror("Unimplemented NES 2.0 submapper: SEROM/SHROM/SH1ROM.\n");
		// 021, 023, 025: VRC4 / VRC2
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
		// iNES Mapper 034
		else if (mapper == 34 && submapper == 1)
		{
			pcb_id = AVE_NINA01; // Mapper 34 is used for 2 diff boards
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
	}

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
			if (mapper == 23 && crc_hack && !submapper)
				pcb_id = KONAMI_VRC4; // this allows for konami_irq to be installed at reset
			break;

		case STD_BXROM:
			if (crc_hack && !submapper)
				pcb_id = AVE_NINA01; // Mapper 34 is used for 2 diff boards
			break;

		case BANDAI_LZ93:
			if (crc_hack)
				pcb_id = BANDAI_FJUMP2;   // Mapper 153 is used for 2 diff boards
			break;

		case IREM_HOLYDIVR:
			if (crc_hack && !submapper)
				pcb_id = JALECO_JF16;    // Mapper 78 is used for 2 diff boards
			break;

		case WAIXING_WXZS:
			if (crc_hack)
				pcb_id = WAIXING_DQ8;    // Mapper 242 is used for 2 diff boards
			break;

		case BMC_GOLD_7IN1:
			if (crc_hack)
				pcb_id = BMC_MARIOPARTY_7IN1;    // Mapper 52 is used for 2 diff boards
			break;

		case BTL_MARIOBABY:
			if (crc_hack)
				pcb_id = BTL_AISENSHINICOL;    // Mapper 42 is used for 2 diff boards
			break;

		case CONY_BOARD:
			if (submapper == 0 || submapper == 2)
				pcb_id = CONY1K_BOARD;         // Mapper 83 is used for 3 diff boards
			break;

		case UNL_LH28_LH54:                            // Mapper 108 is used for 4 diff boards
			if (ROM[5])
				pcb_id = (ROM[5] == 2) ? UNL_LE05 : UNL_LH31;
			else if (!BIT(ROM[6], 0))
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

	return nes_get_slot(pcb_id);
}
