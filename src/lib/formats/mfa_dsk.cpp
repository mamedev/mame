// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/mfa_dsk.cpp

    MFA Mikrocomputer disk formats
	
	Three formats occur, with identical track layout (the largest format squeezes in one mor sector per track)
	
	  Pre-gap: 80*0x4e, 12*0x00, 3*F6 (writes C2, missing clock between bits 3 and 4), 1*0xfc (index mark), 50x4e
	  8/9 Sectors:
	  Gap: 12*0x00, 3x0xf5 (writes A1, missing clock between bits 4 and 5)
	  Mark: 1*0xfe (ID address mark)
	  Sector-ID: Track Nr., Side Nr., Sector Nr., Code for: 512 bytes/sector, 1*0xf7 (two CRC bytes)
	  Gap: 22*0x4e, 12*0x00, 3*0xf5 (writes A1, missing clock between bits 4 and 5)
	  Mark: 1*0xfb (Data address mark)
	  Data: 512*0xe5
	  Test bytes: 1*0xf7 (creates two CRC bytes)
	  Gap: 54*0x4e
	    => for 8/9 sectors, then
	  Post-Gap: Fill buffer with 0x4e
	
	BFZ Mini-DOS: module included in the BFZ MAT monitor; as original CP/M below
	MFA original CP/M: 40 tracks, 8 sectors per track, 512 bytes per sector on 3.5" DD or 5.25" DD or QD drives (only the first 40 tracks are used on QD)
	MFA open CP/M: 80 tracks, 9 sectors per track, 512 bytes per sector on 3.5" DD or 5.25" QD drives
*********************************************************************/

#include "formats/mfa_dsk.h"

mfa_format::mfa_format() : upd765_format(formats) // the format is a upd765 type, the MFA's WD1793 can handle it
{
}

const char *mfa_format::name() const noexcept
{
	return "mfa";
}

const char *mfa_format::description() const noexcept
{
	return "MFA Mikrocomputer disk image";
}

const char *mfa_format::extensions() const noexcept
{
	return "img,dsk";
}

// Unverified gap sizes
const mfa_format::format mfa_format::formats[] =
{
	{   //  720K 5.25 inch
		floppy_image::FF_525,  floppy_image::DSQD, floppy_image::MFM,
		2000, 9, 80, 2, 512, {}, 1, {}, 80, 50, 22, 54
	},
	{   //  320K 5.25 inch
		floppy_image::FF_525,  floppy_image::DSDD, floppy_image::MFM,
		2000, 8, 40, 2, 512, {}, 1, {}, 80, 50, 22, 54
	},
	{}
};

const mfa_format FLOPPY_MFA_FORMAT;