// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/*****************************************************************************
 *
 * machine/spec_snqk.h
 *
 ****************************************************************************/

#ifndef __SPEC_SNQK_H__
#define __SPEC_SNQK_H__

#include "imagedev/snapquik.h"

void spectrum_setup_sna(running_machine &machine, UINT8 *snapdata, UINT32 snapsize);
void spectrum_setup_z80(running_machine &machine, UINT8 *snapdata, UINT32 snapsize);
void spectrum_setup_sp(running_machine &machine, UINT8 *snapdata, UINT32 snapsize);
void spectrum_setup_ach(running_machine &machine, UINT8 *snapdata, UINT32 snapsize);
void spectrum_setup_prg(running_machine &machine, UINT8 *snapdata, UINT32 snapsize);
void spectrum_setup_plusd(running_machine &machine, UINT8 *snapdata, UINT32 snapsize);
void spectrum_setup_sem(running_machine &machine, UINT8 *snapdata, UINT32 snapsize);
void spectrum_setup_sit(running_machine &machine, UINT8 *snapdata, UINT32 snapsize);
void spectrum_setup_zx(running_machine &machine, UINT8 *snapdata, UINT32 snapsize);
void spectrum_setup_snp(running_machine &machine, UINT8 *snapdata, UINT32 snapsize);
void spectrum_setup_snx(running_machine &machine, UINT8 *snapdata, UINT32 snapsize);
void spectrum_setup_frz(running_machine &machine, UINT8 *snapdata, UINT32 snapsize);

void spectrum_setup_scr(running_machine &machine, UINT8 *quickdata, UINT32 quicksize);
void spectrum_setup_raw(running_machine &machine, UINT8 *quickdata, UINT32 quicksize);

#define BASE_RAM      0x4000
#define SPECTRUM_BANK 0x4000

/*****************************************************************************
 *
 * .SNA format (used by JPP)
 *
 ****************************************************************************/
#define SNA48_OFFSET  0
#define SNA48_HDR     27
#define SNA48_SIZE    (SNA48_HDR + 3*SPECTRUM_BANK)

#define SNA128_OFFSET (SNA48_HDR + 3*SPECTRUM_BANK)
#define SNA128_HDR    4
#define SNA128_SIZE_1 (SNA48_HDR + 3*SPECTRUM_BANK + SNA128_HDR + 5*SPECTRUM_BANK)
#define SNA128_SIZE_2 (SNA48_HDR + 3*SPECTRUM_BANK + SNA128_HDR + 6*SPECTRUM_BANK)

/*****************************************************************************
 *
 * .SP format (used by VGASPEC and Spanish Spectrum emulator)
 *
 ****************************************************************************/
#define SP_OLD_HDR      32
#define SP_NEW_HDR      38
#define SP_OLD_SIZE     (SP_OLD_HDR + 3*SPECTRUM_BANK)
#define SP_NEW_SIZE_16K (SP_NEW_HDR + 1*SPECTRUM_BANK)
#define SP_NEW_SIZE_48K (SP_NEW_HDR + 3*SPECTRUM_BANK)

/*****************************************************************************
 *
 * .ACH format (used by !Speccy)
 *
 ****************************************************************************/
#define ACH_OFFSET 0
#define ACH_HDR    256
#define ACH_SIZE   (ACH_HDR + BASE_RAM + 3*SPECTRUM_BANK)

/*****************************************************************************
 *
 * .PRG format (used by SpecEm)
 *
 ****************************************************************************/
#define PRG_OFFSET 0
#define PRG_HDR    256
#define PRG_SIZE   (PRG_HDR + 3*SPECTRUM_BANK)

/*****************************************************************************
 *
 * .PLUSD format (used by FUSE)
 *
 ****************************************************************************/
#define PLUSD_OFFSET 0
#define PLUSD48_HDR  22
#define PLUSD48_SIZE (PLUSD48_HDR + 3*SPECTRUM_BANK)

#define PLUSD128_HDR  23
#define PLUSD128_SIZE (PLUSD128_HDR + 8*SPECTRUM_BANK)

/*****************************************************************************
 *
 * .SEM format (used by SpecEmu)
 *
 ****************************************************************************/
#define SEM_SIGNATURE 6
#define SEM_OFFSET    49158
#define SEM_HDR       34
#define SEM_SIZE      (SEM_SIGNATURE + 3*SPECTRUM_BANK + SEM_HDR)

/*****************************************************************************
 *
 * .SIT format (used by Sinclair)
 *
 ****************************************************************************/
#define SIT_OFFSET 0
#define SIT_HDR    28
#define SIT_SIZE   (SIT_HDR + BASE_RAM + 3*SPECTRUM_BANK)

/*****************************************************************************
 *
 * .ZX format (used by KGB)
 *
 ****************************************************************************/
#define ZX_OFFSET 49284
#define ZX_HDR    202
#define ZX_SIZE   (132 + 3*SPECTRUM_BANK + ZX_HDR)

/*****************************************************************************
 *
 * .SNP format (used by Nuclear ZX)
 *
 ****************************************************************************/
#define SNP_OFFSET 49152
#define SNP_HDR    31
#define SNP_SIZE   (3*SPECTRUM_BANK + SNP_HDR)

/*****************************************************************************
 *
 * .SNX format (used by Specci)
 *
 ****************************************************************************/
#define SNX_OFFSET 0
#define SNX_HDR    43

#define SNX_COMPRESSED   0xff
#define SNX_UNCOMPRESSED 0x00

/*****************************************************************************
 *
 * .FRZ format (used by CBSpeccy, ZX-Live and ASp)
 *
 ****************************************************************************/
#define FRZ_OFFSET 0
#define FRZ_HDR    42
#define FRZ_SIZE   (FRZ_HDR + 8*SPECTRUM_BANK)

enum SPECTRUM_SNAPSHOT_TYPE
{
	SPECTRUM_SNAPSHOT_NONE,
	SPECTRUM_SNAPSHOT_SNA,
	SPECTRUM_SNAPSHOT_Z80,
	SPECTRUM_SNAPSHOT_SP,
	SPECTRUM_TAPEFILE_TAP
};

enum SPECTRUM_Z80_SNAPSHOT_TYPE {
	SPECTRUM_Z80_SNAPSHOT_INVALID,
	SPECTRUM_Z80_SNAPSHOT_48K_OLD,
	SPECTRUM_Z80_SNAPSHOT_48K,
	SPECTRUM_Z80_SNAPSHOT_SAMRAM,
	SPECTRUM_Z80_SNAPSHOT_128K,
	SPECTRUM_Z80_SNAPSHOT_TS2068
};

/*****************************************************************************
 *
 * .SCR format (used by many emulators)
 *
 ****************************************************************************/
#define SCR_BITMAP 6144
#define SCR_ATTR   768
#define SCR_SIZE   (SCR_BITMAP + SCR_ATTR)

/*****************************************************************************
 *
 * .RAW format (used by many emulators)
 *
 ****************************************************************************/
#define RAW_OFFSET 0
#define RAW_HDR    9
#define RAW_SIZE   (RAW_HDR + 3*SPECTRUM_BANK)

#endif  /* __SPEC_SNQK_H__ */
