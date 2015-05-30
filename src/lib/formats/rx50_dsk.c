// license:GPL-2.0+
// copyright-holders:Karl-Ludwig Deisenhofer
/**********************************************************************

    formats/rx50_dsk.c
    Floppies used by Rainbow 100 and 190.

    The RX50 drive: 5.25" format; 300 rpm; MFM 250 kbps; 96 - 100 tpi
    - single sided with two disk slots (1 drive motor served both).

    DEC used the RX50 in entirely different architectures (Pro / PDP-11).
    Native Rainbow 100 format:
    - SSQD - 80 tracks with 10 sectors per track (512 byte sectors)
    - first two tracks are reserved for loader code (DOS / CPM / custom)
    - FAT and root directory began immediately on track 2.
    - 2:1 sector interleave pattern - except in tracks 0 and 1.
    NOTE: PUTR tends to interleave loader tracks 0 + 1. Not recommended!

    Jeff's RBIMG is first choice for preservation, TeleDisk is second.
    Always check the track layout. Both have problems with weak disks.

    A container to handle copy protected RX 50 disks is needed. Some info
    can be derived from Mark Hittinger's RBACKUP (CP/M source from Nov-94).

    TODO: improve code to accept 40 T / single sided / 9 or 8 sector disks:
      a) disks from VT180 (9 sectors; READ ONLY - enforced by BIOS)

      b) 8 sector 160k MS-DOS disks (READ + WRITE support on DEC)
         FORMAT A: /F:160 on DOS; turn MEDIACHK ON
************************************************************************/

#include <assert.h>

#include "flopimg.h"
#include "formats/rx50_dsk.h"

// Controller: WD1793
// TRACK LAYOUT IS UNVERIFED. SEE SOURCES:
// - 'esq_dsk16' (uses WD 1772)
// - SDC-RX50 Floppy Controller Manual
// - 'PC 100 SYSTEM SPEC' 4.3 - page 42. (*)
const floppy_image_format_t::desc_e rx50img_format::rx50_10_desc[] = {
	{   MFM, 0x4e, 80 },          // (*)   GAP (1)
	{   MFM, 0x00, 12 },          // Value from (*). (?? = unverified)
	{   RAW, 0x5224, 3 },
	{   MFM, 0xfc, 1 },
	{   MFM, 0x4e, 50 },
	{ MFM, 0x00, 12 },
	{ SECTOR_LOOP_START, 0, 9 }, // 0 ... f.sector_count-1
	{   CRC_CCITT_START, 1 },
	{     RAW, 0x4489, 3 },
	{     MFM, 0xfe, 1 },
	{     TRACK_ID },
	{     HEAD_ID },
	{     SECTOR_ID },
	{     SIZE_ID },
	{   CRC_END, 1 },
	{   CRC, 1 },
	{   MFM, 0x4e, 22 },    // (*) POST-ID GAP (2)
	{   MFM, 0x00, 12 },    // (*)
	{   CRC_CCITT_START, 2 },
	{     RAW, 0x4489, 3 },
	{     MFM, 0xfb, 1 },
	{     SECTOR_DATA, -1 },
	{   CRC_END, 2 },
	{   CRC, 2 },
	{   MFM, 0x4e, 48 },    // GAP (3) - taken from RBACKUP source.
	{   MFM, 0x00, 12 },
	{ SECTOR_LOOP_END },
	{ MFM, 0x4e, 1 },    // UNVERIFIED - ('esq_16' has 170 x $4e)
	{ END }
};

rx50img_format::rx50img_format()
{
}

const char *rx50img_format::name() const
{
	return "img";
}

const char *rx50img_format::description() const
{
	return "DEC Rainbow 100 floppy image";
}

const char *rx50img_format::extensions() const
{
	return "img";
}

bool rx50img_format::supports_save() const
{
	return true;
}

void rx50img_format::find_size(io_generic *io, UINT8 &track_count, UINT8 &head_count, UINT8 &sector_count)
{
	head_count = 1;

	UINT32 expected_size = 0;
	UINT64 size = io_generic_size(io);

	track_count = 80;
	sector_count = 10;
	expected_size = 512 * track_count * head_count * sector_count;

	if (size == expected_size) // standard format has 409600 byte
		return;
/*
    track_count = 40;
    sector_count = 9; // [VT 180]
    expected_size = 512 * track_count * head_count * sector_count;
    if (size == expected_size)
        return;

    track_count = 40;
    sector_count = 8; // [DOS]
    expected_size = 512 * track_count * head_count * sector_count;
    if (size == expected_size)
        return;
*/
	track_count = head_count = sector_count = 0;
}

int rx50img_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT8 track_count, head_count, sector_count;
	find_size(io, track_count, head_count, sector_count);

	if(track_count)
		return 50;
	return 0;
}

	//  /* Sectors are numbered 1 to 10 */
bool rx50img_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	UINT8 track_count, head_count, sector_count;
	find_size(io, track_count, head_count, sector_count);
	if(track_count == 0)
		return false;

	UINT8 sectdata[10*512];
	desc_s sectors[10];
	for(int i=0; i<sector_count; i++) {
		sectors[i].data = sectdata + 512*i;
		sectors[i].size = 512;
		sectors[i].sector_id = i + 1; // SECTOR_ID +1  <===
	}

	int track_size = sector_count*512;
	for(int track=0; track < track_count; track++) {
		for(int head=0; head < head_count; head++) {
			io_generic_read(io, sectdata, (track*head_count + head)*track_size, track_size);
			generate_track(rx50_10_desc, track, head, sectors, sector_count, 102064, image);  // 98480
		}
	}

	image->set_variant(floppy_image::SSQD);

	return true;
}

bool rx50img_format::save(io_generic *io, floppy_image *image)
{
	int track_count, head_count, sector_count;
	get_geometry_mfm_pc(image, 2000, track_count, head_count, sector_count);

	if(track_count != 80)
		track_count = 80;

	// Happens for a fully unformatted floppy
	if(!head_count)
		head_count = 1;

	if(sector_count == 9) // [VT180] 9 sector format : no save!
		return false;

	if(sector_count != 10) // either 8 or 10 sectors
		sector_count = 10; // [STANDARD]

	/*
	if(sector_count != 10) // either 8 or 10 sectors
	{
	  if(sector_count == 8)
	  {
	      track_count = 40;  // [DOS]
	  } else
	  {
	      sector_count = 10; // [STANDARD]
	  }
	}
*/
	UINT8 sectdata[11*512];
	int track_size = sector_count*512;

	for(int track=0; track < track_count; track++) {
		for(int head=0; head < head_count; head++) {
			get_track_data_mfm_pc(track, head, image, 2000, 512, sector_count, sectdata);
			io_generic_write(io, sectdata, (track*head_count + head)*track_size, track_size);
		}
	}
	return true;
}

const floppy_format_type FLOPPY_RX50IMG_FORMAT = &floppy_image_format_creator<rx50img_format>;


/*

// Native 400K format (80 T * 10 S * 512 bytes) on 'quad density' RX50 drives
// ( 5.25" single sided; 300 rpm; MFM 250 kbps; 96 - 100 tpi ).
//
// The BIOS can also * read * VT-180 disks and access MS-DOS 160 k disks (R + W)
// ( 40 tracks; single sided with 9 or 8 sectors per track )
static LEGACY_FLOPPY_OPTIONS_START( dec100_floppy )
    LEGACY_FLOPPY_OPTION( dec100_floppy, "td0", "Teledisk floppy disk image", td0_dsk_identify, td0_dsk_construct, td0_dsk_destruct, NULL )
    LEGACY_FLOPPY_OPTION( dec100_floppy, "img", "DEC Rainbow 100", basicdsk_identify_default, basicdsk_construct_default,    NULL,
        HEADS([1])
        TRACKS(40/[80])
        SECTORS(8/9/[10])
        SECTOR_LENGTH([512])
        INTERLEAVE([0])
        FIRST_SECTOR_ID([1])
                        )
LEGACY_FLOPPY_OPTIONS_END

*/
