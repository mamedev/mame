#ifndef __GENESIS_PCB_H
# define __GENESIS_PCB_H

/* PCB */
enum
{
    SEGA_STD = 0,

    /* Sega PCB */
    SEGA_EEPROM, SEGA_SRAM, SEGA_FRAM,

    /* Codemasters PCB (J-Carts and SEPROM) */
	CM_JCART, CM_JCART_SEPROM,

	SSF2,					/* Super Street Fighter 2 */
	// EEPROM
	NBA_JAM,				/* NBA Jam */
	NBA_JAM_TE,				/* NBA Jam TE / NFL Quarterback Club */
	NFL_QB_96,				/* NFL Quarterback Club '96 */
	C_SLAM,					/* College Slam / Frank Thomas Big Hurt Baseball */
	EA_NHLPA,				/* NHLPA Hockey 93 / Rings of Power */
	WBOY_V,					/* Wonder Boy V / Evander Holyfield's Boxing / Greatest Heavyweights of the Ring / Sports Talk Baseball / Megaman */
	CODE_MASTERS,			/* Micro Machines 2 / Military / 96 / Brian Lara Cricket 96 */

	LIONK3,					/* Lion King 3 */
	SKINGKONG,				/* Super King Kong 99 */
	SDK99,					/* Super Donkey Kong 99 */
	REDCLIFF,				/* Romance of the Three Kingdoms - Battle of Red Cliffs, already decoded from .mdx format */
	REDCL_EN,				/* The encoded version... */
	RADICA,					/* Radica TV games.. these probably should be a seperate driver since they are a seperate 'console' */
	KOF99,					/* King of Fighters '99 */
	SOULBLAD,				/* Soul Blade */
	MJLOVER,				/* Mahjong Lover */
	SQUIRRELK,				/* Squirrel King */
	SMOUSE,					/* Smart Mouse */
	SMB,					/* Super Mario Bros. */
	SMB2,					/* Super Mario Bros. 2 */
	KAIJU,					/* Pokemon Stadium */
	CHINFIGHT3,				/* Chinese Fighters 3 */
	LIONK2,					/* Lion King 2 */
	BUGSLIFE,				/* A Bug's Life */
	ELFWOR,					/* Elf Wor */
	ROCKMANX3,				/* Rockman X3 */
	SBUBBOB,				/* Super Bubble Bobble */
	KOF98,					/* King of Fighters '98 */
	REALTEC,				/* Whac a Critter/Mallet legend, Defend the Earth, Funnyworld/Ballonboy */
	MC_SUP19IN1,			/* Super 19 in 1 */
	MC_SUP15IN1,			/* Super 15 in 1 */
	MC_12IN1,				/* 12 in 1 and a few more multicarts */
	TOPFIGHTER,				/* Top Fighter 2000 MK VIII */
	POKEMON,				/* Pocket Monster */
	POKEMON2,				/* Pocket Monster 2 */
	MULAN					/* Hua Mu Lan - Mulan */
};

int md_get_pcb_id(const char *pcb);

#endif

