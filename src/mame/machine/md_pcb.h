#ifndef __GENESIS_PCB_H
# define __GENESIS_PCB_H

/* PCB */
enum
{
    SEGA_STD = 0,

    SEGA_SRAM, SEGA_FRAM,

	CM_JCART, CM_JCART_SEPROM,   /* Codemasters PCB (J-Carts and SEPROM) */

	SSF2,                        /* Super Street Fighter 2 */
	GAME_KANDUME,                /* Game no Kandume Otokuyou */
	BEGGAR,                      /* Xin Qigai Wangzi uses different sram start address and has no valid header */

	// EEPROM
	SEGA_EEPROM,                 /* Wonder Boy V / Evander Holyfield's Boxing / Greatest Heavyweights of the Ring / Sports Talk Baseball / Megaman */
	NBA_JAM,                     /* NBA Jam */
	NBA_JAM_TE,                  /* NBA Jam TE / NFL Quarterback Club */
	NFL_QB_96,                   /* NFL Quarterback Club '96 */
	C_SLAM,                      /* College Slam / Frank Thomas Big Hurt Baseball */
	EA_NHLPA,                    /* NHLPA Hockey 93 / Rings of Power */
	CODE_MASTERS,                /* Micro Machines 2 / Military / 96 / Brian Lara Cricket 96 */

	MC_SUP19IN1,                 /* Super 19 in 1 */
	MC_SUP15IN1,                 /* Super 15 in 1 */
	MC_12IN1,                    /* 12 in 1 and a few more multicarts */
	BUGSLIFE,                    /* A Bug's Life */
	CHINFIGHT3,                  /* Chinese Fighters 3 */
	ELFWOR,                      /* Linghuan Daoshi Super Magician */
	KOF98,                       /* King of Fighters '98 */
	KOF99,                       /* King of Fighters '99 */
	LIONK2,                      /* Lion King 2 */
	LIONK3,                      /* Lion King 3 */
	SKINGKONG,                   /* Super King Kong 99 */
	SDK99,                       /* Super Donkey Kong 99 */
	MJLOVER,                     /* Mahjong Lover */
	MULAN,                       /* Hua Mu Lan - Mulan */
	POKEMON,                     /* Pocket Monster */
	POKEMON2,                    /* Pocket Monster 2 */
	KAIJU,                       /* Pokemon Stadium */
	RADICA,                      /* Radica TV games.. these probably should be a seperate driver since they are a seperate 'console' */
	REALTEC,                     /* Whac a Critter/Mallet legend, Defend the Earth, Funnyworld/Ballonboy */
	REDCLIFF,                    /* Romance of the Three Kingdoms - Battle of Red Cliffs, already decoded from .mdx format */
	REDCL_EN,                    /* The encoded version... */
	ROCKMANX3,                   /* Rockman X3 */
	SMB,                         /* Super Mario Bros. */
	SMB2,                        /* Super Mario Bros. 2 */
	SMOUSE,                      /* Smart Mouse */
	SOULBLAD,                    /* Soul Blade */
	SQUIRRELK,                   /* Squirrel King */
	SBUBBOB,                     /* Super Bubble Bobble */
	TOPFIGHTER                   /* Top Fighter 2000 MK VIII */
};

int md_get_pcb_id(const char *pcb);

#endif

