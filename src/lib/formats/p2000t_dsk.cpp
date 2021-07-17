// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/*********************************************************************

    formats/p2000t_dsk.c

    P2000T disk images

*********************************************************************/

#include "p2000t_dsk.h"

p2000t_format::p2000t_format() : upd765_format(formats)
{
}

const char *p2000t_format::name() const
{
    return "p2000t";
}

const char *p2000t_format::description() const
{
    return "P2000T disk image";
}

const char *p2000t_format::extensions() const
{
    return "dsk";
}

// Unverified gap sizes
const p2000t_format::format p2000t_format::formats[] = {
    /*
        uint32_t form_factor = 0U; // See floppy_image for possible values
        uint32_t variant = 0U;   // See floppy_image for possible values
        uint32_t encoding = 0U;  // See floppy_image for possible values
        int cell_size;           // See floppy_image_format_t for details
        int sector_count;
        int track_count;
        int head_count;
        int sector_base_size;
        int per_sector_size[40]; // if sector_base_size is 0
        int sector_base_id;      // 0 or 1 usually, -1 if there's interleave
        int per_sector_id[40];   // if sector_base_id is -1.  If both per are used, then sector per_sector_id[i] has size per_sector_size[i]
        int gap_4a;              // Usually 80 - number of 4e between index and IAM sync
        int gap_1;               // Usually 50 - number of 4e between IAM and first IDAM sync
        int gap_2;               // 22 for <=1.44Mb, 41 for 2.88Mb - number of 4e between sector header and data sync
        int gap_3;               // Usually 84 - number of 4e between sector crc and next IDAM

                                RPM    cell_size
        5.25" SD         4       300    4000
	    5.25" DD         2       300    2000
	    5.25" HD         1       360    1200
	    3.5" DD          2       300    2000
	    
    */
    {   /*  3 1/2 inch single sided double density */
        floppy_image::FF_35,  floppy_image::SSDD, floppy_image::MFM,
        2000,  16, 80, 1, 256, {}, 1, {}, 80, 50, 22, 50
    },
    {   /*  3 1/2 inch double density */
        floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM,
        2000,  16, 80, 2, 256, {}, 1, {}, 80, 50, 22, 50
    },
    {   /*  5 1/4 inch single sided single density 35 ttrk */
        floppy_image::FF_525,  floppy_image::SSSD, floppy_image::MFM,
        4000,  16, 35, 1, 256, {}, 1, {}, 80, 50, 22, 50
    },
    {   /*  5 1/4 inch single density 35 trk */
        floppy_image::FF_525,  floppy_image::DSSD, floppy_image::MFM,
        4000,  16, 35, 2, 256, {}, 1, {}, 80, 50, 22, 50
    },
    {   /*  5 1/4 inch single sided double density 40 trk */
        floppy_image::FF_525,  floppy_image::SSDD, floppy_image::MFM,
        2000,  16, 40, 1, 256, {}, 1, {}, 80, 50, 22, 50
    },
    {   /*  5 1/4 inch double density  40 trk */
        floppy_image::FF_525,  floppy_image::DSDD, floppy_image::MFM,
        2000,  16, 40, 2, 256, {}, 1, {}, 80, 50, 22, 50  // 2000,  16, 40, 2, 256, {}, 1, {}, 26, 24, 28, 50
    },
    {   /*  5 1/4 inch quad density  80 trk */
        floppy_image::FF_525,  floppy_image::DSDD, floppy_image::MFM,
        2000,  16, 80, 2, 256, {}, 1, {}, 80, 50, 22, 50
    },
    {}
};

const floppy_format_type FLOPPY_P2000T_FORMAT = &floppy_image_format_creator<p2000t_format>;

