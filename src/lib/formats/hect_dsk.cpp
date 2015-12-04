// license:BSD-3-Clause
// copyright-holders:JJ Stacino
/*********************************************************************

    formats/hect_dsk.c

    Hector disk images

*********************************************************************/

#include <assert.h>

#include "formats/hect_dsk.h"
#include "formats/basicdsk.h"

/*****************************************************************************/
/******  Management of the floppy images 200Ko and 800Ko *********************/
/*****************************************************************************/
/* For the 200Ko disk :
        512 bytes per sectors,
        10  sector per track,
        From sector =0 to sector 9,
        40  tracks,
        1 Head
    This format can be extract from a real disc with anadisk (*.IMG format rename in *.HE2).
*/
static FLOPPY_IDENTIFY(hector_disc2_dsk200_identify)
{
	*vote = (floppy_image_size(floppy) == (1*40*10*512)) ? 100 : 0;
	return FLOPPY_ERROR_SUCCESS;
}

static FLOPPY_CONSTRUCT(hector_disc2_dsk200_construct)
{
	struct basicdsk_geometry geometry;
	memset(&geometry, 0, sizeof(geometry));
	geometry.heads = 1;
	geometry.first_sector_id = 0;
	geometry.sector_length = 512;
	geometry.tracks = 40;
	geometry.sectors = 10;
	return basicdsk_construct(floppy, &geometry);
}
/* For the 720Ko disk :
        512 bytes per sectors,
        9  sector per track,
        From sector =0 to sector 8,
        80  tracks,
        2 Head
    This format can be extract from a real disc with anadisk (*.IMG format rename in *.HE7).
*/
static FLOPPY_IDENTIFY(hector_disc2_dsk720_identify)
{
	*vote = (floppy_image_size(floppy) == (2*80*9*512)) ? 100 : 0;
	return FLOPPY_ERROR_SUCCESS;
}

static FLOPPY_CONSTRUCT(hector_disc2_dsk720_construct)
{
	struct basicdsk_geometry geometry;
	memset(&geometry, 0, sizeof(geometry));
	geometry.heads = 2;
	geometry.first_sector_id = 0;
	geometry.sector_length = 512;
	geometry.tracks = 80;
	geometry.sectors = 9;
	return basicdsk_construct(floppy, &geometry);

}/* For the 800Ko disk :
        512 bytes per sectors,
        10  sector per track,
        From sector =0 to sector 9,
        80  tracks
        2 Heads
    This format can be extract from a real disk with anadisk (*.IMG format rename in *.HE2).
*/

static FLOPPY_IDENTIFY(hector_disc2_dsk800_identify)
{
	*vote = (floppy_image_size(floppy) == (2*80*10*512)) ? 100 : 0;
	return FLOPPY_ERROR_SUCCESS;
}

static FLOPPY_CONSTRUCT(hector_disc2_dsk800_construct)
{
	struct basicdsk_geometry geometry;
	memset(&geometry, 0, sizeof(geometry));
	geometry.heads = 2;
	geometry.first_sector_id = 0;
	geometry.sector_length = 512;
	geometry.tracks = 80;
	geometry.sectors = 10;
	return basicdsk_construct(floppy, &geometry);
}

LEGACY_FLOPPY_OPTIONS_START( hector_disc2 )
	LEGACY_FLOPPY_OPTION( hector_dsk, "HE2", "hector disc2 floppy disk image 200K", hector_disc2_dsk200_identify, hector_disc2_dsk200_construct, nullptr, nullptr)
	LEGACY_FLOPPY_OPTION( hector_dsk, "HE7", "hector disc2 floppy disk image 720K", hector_disc2_dsk720_identify, hector_disc2_dsk720_construct, nullptr, nullptr)
	LEGACY_FLOPPY_OPTION( hector_dsk, "HE8", "hector disc2 floppy disk image 800K", hector_disc2_dsk800_identify, hector_disc2_dsk800_construct, nullptr, nullptr)
LEGACY_FLOPPY_OPTIONS_END
