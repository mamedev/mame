// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_OPTIONS_H
#define MAME_BUS_MEGADRIVE_CART_OPTIONS_H

#pragma once

void megadrive_cart_options(device_slot_interface &device);

namespace bus::megadrive::slotoptions {

	extern char const *const MD_STD;
	extern char const *const MD_SSF2;
	extern char const *const MD_SRAM;
	extern char const *const MD_SONIC3;
	extern char const *const MD_TPLAY96;
	extern char const *const MD_HARDBALL95;

	extern char const *const MD_CM2IN1;
	extern char const *const MD_TECTOY_SPORTS;
	extern char const *const MD_GAME_KANZUME;

	extern char const *const MC_PIRATE;
	extern char const *const UNL_XINQIG;
	extern char const *const UNL_TILESMJ2;
	extern char const *const UNL_ELFWOR;
	extern char const *const UNL_SMOUSE;
	extern char const *const UNL_YASECH;
	extern char const *const UNL_777CASINO;
	extern char const *const UNL_SOULBLADE;
	extern char const *const UNL_SUPRBUBL;
	extern char const *const UNL_CJMJCLUB;
	extern char const *const UNL_MJLOV;
	extern char const *const UNL_REDCLIFF;
	extern char const *const UNL_LIONKING2;
	extern char const *const UNL_KOF98;
	extern char const *const UNL_BUGSLIFE;
	extern char const *const UNL_POKEMONA;
	extern char const *const UNL_KOF99;
	extern char const *const UNL_SMB;
	extern char const *const UNL_SMB2;
	extern char const *const UNL_ROCKMANX3;

	extern char const *const UNL_SANGUO5;

//	BEGGARP;                     /* Beggar Prince uses different sram start address + bankswitch tricks */
//	WUKONG;                      /* Legend of Wukong uses different sram start address + bankswitch trick for last 128K of ROM */
//	STARODYS;                    /* Star Odyssey */

	// Cart + Slot Expansion
//	SEGA_SK;                     /* Sonic & Knuckles pass-through cart */
//
//	// Cart + SVP
//	SEGA_SVP;                    /* Virtua Racing */
//
//	// Cart + NVRAM
//
//	// EEPROM
//	SEGA_EEPROM;                 /* Wonder Boy V / Evander Holyfield's Boxing / Greatest Heavyweights of the Ring / Sports Talk Baseball / Megaman */
//	NBA_JAM;                     /* NBA Jam */
//	NBA_JAM_ALT;                 /* NBA Jam */
//	NBA_JAM_TE;                  /* NBA Jam TE / NFL Quarterback Club */
//	NFL_QB_96;                   /* NFL Quarterback Club '96 */
//	C_SLAM;                      /* College Slam / Frank Thomas Big Hurt Baseball */
//	EA_NHLPA;                    /* NHLPA Hockey 93 / Rings of Power */
//	BRIAN_LARA;                  /* Brian Lara Cricket 96 */
//	PSOLAR;                      /* Pier Solar (STM95 EEPROM) */
//
//	// J-Cart
//	CM_JCART;                    /* Pete Sampras Tennis */
//	CODE_MASTERS;                /* Micro Machines 2 / Military (J-Cart + SEPROM)  */
//	CM_MM96;                     /* Micro Machines 96 (J-Cart + SEPROM; diff I2C model)  */
//	CHINFIGHT3;                  /* Chinese Fighters 3 */
//	KAIJU;                       /* Pokemon Stadium */
//	KOF99;                       /* King of Fighters '99 */
//	LIONK2;                      /* Lion King 2 */
//	LIONK3;                      /* Lion King 3; Super Donkey Kong 99; Super King Kong 99 */
//	MJLOVER;                     /* Mahjong Lover */
//	REALTEC;                     /* Whac a Critter/Mallet legend; Defend the Earth; Funnyworld/Ballonboy */
//	ROCKMANX3;                   /* Rockman X3 */
//	SMB;                         /* Super Mario Bros. */
//	SMB2;                        /* Super Mario Bros. 2 */
//	SMW64;                       /* Super Mario World 64 */
//	SQUIRRELK;                   /* Squirrel King */
//	SRAM_ARG96;                  /* Futbol Argentino 96 (Argentina) (hack of J. League Pro Striker 2) */
//	TC2000;                      /* TC 2000 (Argentina; protected) */
//	TEKKENSP;                    /* Tekken Special */
//	TOPFIGHTER;                  /* Top Fighter 2000 MK VIII */
//
//	TITAN;

} // namespace bus::megadrive::slotoptions


#endif // MAME_BUS_MEGADRIVE_CART_OPTIONS_H
