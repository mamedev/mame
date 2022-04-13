// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
/*********************************************************************

    ap2_dsk.c

    Apple II disk images

*********************************************************************/

#include "ap2_dsk.h"
#include "basicdsk.h"

#include "ioprocs.h"

#include <cassert>
#include <cstdlib>
#include <cstring>


#define APPLE2_IMAGE_DO     0
#define APPLE2_IMAGE_PO     1
#define APPLE2_IMAGE_NIB    2


/* used in for all Apple II images */
static uint32_t apple2_get_track_size(floppy_image_legacy *floppy, int head, int track);
static int disk_decode_nib(uint8_t *data, const uint8_t *nibble, int *volume, int *track, int *sector);
static void disk_encode_nib(uint8_t *nibble, const uint8_t *data, int volume, int track, int sector);

/* used in DOS/ProDOS order images */
static int apple2_do_translate_sector(floppy_image_legacy *floppy, int sector);
static int apple2_po_translate_sector(floppy_image_legacy *floppy, int sector);
static floperr_t apple2_dsk_read_track(floppy_image_legacy *floppy, int head, int track, uint64_t offset, void *buffer, size_t buflen);
static floperr_t apple2_dsk_write_track(floppy_image_legacy *floppy, int head, int track, uint64_t offset, const void *buffer, size_t buflen);

/* used in nibble order images */
static floperr_t apple2_nib_read_track(floppy_image_legacy *floppy, int head, int track, uint64_t offset, void *buffer, size_t buflen);
static floperr_t apple2_nib_write_track(floppy_image_legacy *floppy, int head, int track, uint64_t offset, const void *buffer, size_t buflen);
static floperr_t apple2_nib_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen);
static floperr_t apple2_nib_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam);
static floperr_t apple2_nib_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, uint32_t *sector_length);


static const uint8_t translate6[0x40] =
{
	0x96, 0x97, 0x9a, 0x9b, 0x9d, 0x9e, 0x9f, 0xa6,
	0xa7, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb2, 0xb3,
	0xb4, 0xb5, 0xb6, 0xb7, 0xb9, 0xba, 0xbb, 0xbc,
	0xbd, 0xbe, 0xbf, 0xcb, 0xcd, 0xce, 0xcf, 0xd3,
	0xd6, 0xd7, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde,
	0xdf, 0xe5, 0xe6, 0xe7, 0xe9, 0xea, 0xeb, 0xec,
	0xed, 0xee, 0xef, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
	0xf7, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};



/* -----------------------------------------------------------------------
 * Utility code
 * ----------------------------------------------------------------------- */

static const uint8_t *get_untranslate6_map(void)
{
	static uint8_t map[256];
	static int map_inited = 0;
	uint8_t i;

	if (!map_inited)
	{
		memset(map, 0xff, sizeof(map));
		for (i = 0; i < std::size(translate6); i++)
			map[translate6[i]] = i;
		map_inited = 1;
	}
	return map;
}



/* -----------------------------------------------------------------------
 * Core constructor
 * ----------------------------------------------------------------------- */


static floperr_t apple2_general_construct(floppy_image_legacy *floppy, int floppy_type)
{
	floperr_t err;
	struct basicdsk_geometry geometry;
	struct FloppyCallbacks *format;

	format = floppy_callbacks(floppy);

	switch(floppy_type) {
	case APPLE2_IMAGE_DO:
	case APPLE2_IMAGE_PO:
		memset(&geometry, 0, sizeof(geometry));
		geometry.heads = 1;
		geometry.tracks = APPLE2_TRACK_COUNT;
		geometry.sectors = APPLE2_SECTOR_COUNT;
		geometry.sector_length = APPLE2_SECTOR_SIZE;
		geometry.translate_sector = (floppy_type == APPLE2_IMAGE_DO) ? apple2_do_translate_sector : apple2_po_translate_sector;

		err = basicdsk_construct(floppy, &geometry);
		if (err)
			return err;

		format->read_track = apple2_dsk_read_track;
		format->write_track = apple2_dsk_write_track;
		break;

	case APPLE2_IMAGE_NIB:
		format->read_track = apple2_nib_read_track;
		format->write_track = apple2_nib_write_track;
		format->read_sector = apple2_nib_read_sector;
		format->write_sector = apple2_nib_write_sector;
		format->get_sector_length = apple2_nib_get_sector_length;
		break;

	default:
		assert(0);
		return FLOPPY_ERROR_INTERNAL;
	}

	format->get_track_size = apple2_get_track_size;

	return FLOPPY_ERROR_SUCCESS;
}



/* -----------------------------------------------------------------------
 * DOS order and ProDOS order code
 * ----------------------------------------------------------------------- */

static FLOPPY_IDENTIFY(apple2_dsk_identify)
{
	uint64_t size;
	uint64_t expected_size;

	size = floppy_image_size(floppy);
	expected_size = APPLE2_TRACK_COUNT * APPLE2_SECTOR_COUNT * APPLE2_SECTOR_SIZE;

	if ((size == expected_size) || (size == APPLE2_STD_TRACK_COUNT * APPLE2_SECTOR_COUNT * APPLE2_SECTOR_SIZE))
		*vote = 100;
	else if ((size > expected_size) && ((size - expected_size) < 8))
		*vote = 90;     /* tolerate images with up to eight fewer/extra bytes (bug #638) */
	else if ((size < expected_size) && ((expected_size - size) < 8))
		*vote = 90;     /* tolerate images with up to eight fewer/extra bytes (bug #638) */
	else
		*vote = 0;
	return FLOPPY_ERROR_SUCCESS;
}



static int apple2_do_translate_sector(floppy_image_legacy *floppy, int sector)
{
	static const uint8_t skewing[] =
	{
		/* DOS order (*.do) */
		0x00, 0x07, 0x0E, 0x06, 0x0D, 0x05, 0x0C, 0x04,
		0x0B, 0x03, 0x0A, 0x02, 0x09, 0x01, 0x08, 0x0F
	};
	return skewing[sector];
}



static int apple2_po_translate_sector(floppy_image_legacy *floppy, int sector)
{
	static const uint8_t skewing[] =
	{
		/* ProDOS order (*.po) */
		0x00, 0x08, 0x01, 0x09, 0x02, 0x0A, 0x03, 0x0B,
		0x04, 0x0C, 0x05, 0x0D, 0x06, 0x0E, 0x07, 0x0F
	};
	return skewing[sector];
}



static FLOPPY_CONSTRUCT(apple2_do_construct)
{
	return apple2_general_construct(floppy, APPLE2_IMAGE_DO);
}



static FLOPPY_CONSTRUCT(apple2_po_construct)
{
	return apple2_general_construct(floppy, APPLE2_IMAGE_PO);
}



static floperr_t apple2_dsk_read_track(floppy_image_legacy *floppy, int head, int track, uint64_t offset, void *buffer, size_t buflen)
{
	uint8_t sector_buffer[APPLE2_SECTOR_SIZE];
	int sector;
	uint8_t *nibble;

	if (buflen < APPLE2_NIBBLE_SIZE*APPLE2_SECTOR_COUNT)
		return FLOPPY_ERROR_INTERNAL;
	if (offset != 0)
		return FLOPPY_ERROR_UNSUPPORTED;

	memset(buffer, 0, buflen);

	for (sector = 0; sector < APPLE2_SECTOR_COUNT; sector++)
	{
		nibble = (uint8_t *)buffer;
		nibble += sector * APPLE2_SMALL_NIBBLE_SIZE;

		floppy_read_sector(floppy, head, track, sector, 0, sector_buffer, sizeof(sector_buffer));
		disk_encode_nib(nibble, sector_buffer, 254, track, sector);
	}
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t apple2_dsk_write_track(floppy_image_legacy *floppy, int head, int track, uint64_t offset, const void *buffer, size_t buflen)
{
	int sector;
	uint8_t sector_buffer[APPLE2_SECTOR_SIZE];
	const uint8_t *nibble;

	if (offset != 0)
		return FLOPPY_ERROR_UNSUPPORTED;

	for (sector = 0; sector < APPLE2_SECTOR_COUNT; sector++)
	{
		nibble = (uint8_t *)buffer;
		nibble += sector * APPLE2_SMALL_NIBBLE_SIZE;

		disk_decode_nib(sector_buffer, nibble, nullptr, nullptr, nullptr);
		floppy_write_sector(floppy, head, track, sector, 0, sector_buffer, sizeof(sector_buffer), 0);
	}

	return FLOPPY_ERROR_SUCCESS;
}



/* -----------------------------------------------------------------------
 * Nibble order code
 * ----------------------------------------------------------------------- */

static FLOPPY_IDENTIFY(apple2_nib_identify)
{
	uint64_t size;
	size = floppy_image_size(floppy);
	*vote = ((size == APPLE2_STD_TRACK_COUNT * APPLE2_SECTOR_COUNT * APPLE2_NIBBLE_SIZE) || (size == (APPLE2_STD_TRACK_COUNT + 1) * APPLE2_SECTOR_COUNT * APPLE2_NIBBLE_SIZE)) ? 100 : 0;
	return FLOPPY_ERROR_SUCCESS;
}



static FLOPPY_CONSTRUCT(apple2_nib_construct)
{
	return apple2_general_construct(floppy, APPLE2_IMAGE_NIB);
}



static floperr_t apple2_nib_read_track(floppy_image_legacy *floppy, int head, int track, uint64_t offset, void *buffer, size_t buflen)
{
	if ((head != 0) || (track < 0) || (track > APPLE2_TRACK_COUNT))
		return FLOPPY_ERROR_SEEKERROR;
	if (offset != 0)
		return FLOPPY_ERROR_UNSUPPORTED;
	floppy_image_read(floppy, buffer, track * APPLE2_NIBBLE_SIZE * APPLE2_SECTOR_COUNT, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t apple2_nib_write_track(floppy_image_legacy *floppy, int head, int track, uint64_t offset, const void *buffer, size_t buflen)
{
	if ((head != 0) || (track < 0) || (track > APPLE2_TRACK_COUNT))
		return FLOPPY_ERROR_SEEKERROR;
	if (offset != 0)
		return FLOPPY_ERROR_UNSUPPORTED;
	floppy_image_write(floppy, buffer, track * APPLE2_NIBBLE_SIZE * APPLE2_SECTOR_COUNT, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



/* -----------------------------------------------------------------------
 * Track conversion
 * ----------------------------------------------------------------------- */

static int decode_nibbyte(uint8_t *nibint, const uint8_t *nibdata)
{
	if ((nibdata[0] & 0xAA) != 0xAA)
		return 1;
	if ((nibdata[1] & 0xAA) != 0xAA)
		return 1;

	*nibint =  (nibdata[0] & ~0xAA) << 1;
	*nibint |= (nibdata[1] & ~0xAA) << 0;
	return 0;
}



static int disk_decode_nib(uint8_t *data, const uint8_t *nibble, int *volume, int *track, int *sector)
{
	uint8_t read_volume;
	uint8_t read_track;
	uint8_t read_sector;
	uint8_t read_checksum;
	int i;
	uint8_t b, xorvalue, newvalue;

	const uint8_t *untranslate6 = get_untranslate6_map();

	/* pick apart the volume/track/sector info and checksum */
	if (decode_nibbyte(&read_volume, &nibble[10]))
		return 1;
	if (decode_nibbyte(&read_track, &nibble[12]))
		return 1;
	if (decode_nibbyte(&read_sector, &nibble[14]))
		return 1;
	if (decode_nibbyte(&read_checksum, &nibble[16]))
		return 1;
	if (read_checksum != (read_volume ^ read_track ^ read_sector))
		return 1;

	/* decode the nibble core */
	xorvalue = 0;
	for (i = 0; i < 342; i++)
	{
		b = untranslate6[nibble[i+28]];
		if (b == 0xff)
			return 1;
		newvalue = b ^ xorvalue;

		if (i >= 0x56)
		{
			/* 6 bit */
			data[i - 0x56] |= (newvalue << 2);
		}
		else
		{
			/* 3 * 2 bit */
			data[i + 0x00] = ((newvalue >> 1) & 0x01) | ((newvalue << 1) & 0x02);
			data[i + 0x56] = ((newvalue >> 3) & 0x01) | ((newvalue >> 1) & 0x02);
			if (i + 0xAC < APPLE2_SECTOR_SIZE)
				data[i + 0xAC] = ((newvalue >> 5) & 0x01) | ((newvalue >> 3) & 0x02);
		}
		xorvalue = newvalue;
	}

	/* success; write out values if pointers not null */
	if (volume)
		*volume = read_volume;
	if (track)
		*track = read_track;
	if (sector)
		*sector = read_sector;
	return 0;
}



static floperr_t apple2_nib_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	floperr_t err;
	const uint8_t *nibble;
	uint8_t *track_data;
	void *track_data_v;

	if ((sector < 0) || (sector >= APPLE2_SECTOR_COUNT))
		return FLOPPY_ERROR_SEEKERROR;
	if (buflen != APPLE2_SECTOR_SIZE)
		return FLOPPY_ERROR_INTERNAL;

	err = floppy_load_track(floppy, head, track, false, &track_data_v, nullptr);
	if (err)
		return err;
	track_data = (uint8_t *) track_data_v;

	nibble = track_data + (sector * APPLE2_NIBBLE_SIZE);

	if (disk_decode_nib((uint8_t *)buffer, nibble, nullptr, nullptr, nullptr))
		return FLOPPY_ERROR_INVALIDIMAGE;

	return FLOPPY_ERROR_SUCCESS;
}



static void disk_encode_nib(uint8_t *nibble, const uint8_t *data, int volume, int track, int sector)
{
	int checksum, oldvalue, xorvalue, i;

	/* setup header values */
	checksum = volume ^ track ^ sector;

	memset(nibble, 0xFF, APPLE2_NIBBLE_SIZE);
	nibble[ 7]     = 0xD5;
	nibble[ 8]     = 0xAA;
	nibble[ 9]     = 0x96;
	nibble[10]     = (volume >> 1) | 0xAA;
	nibble[11]     = volume | 0xAA;
	nibble[12]     = (track >> 1) | 0xAA;
	nibble[13]     = track | 0xAA;
	nibble[14]     = (sector >> 1) | 0xAA;
	nibble[15]     = sector | 0xAA;
	nibble[16]     = (checksum >> 1) | 0xAA;
	nibble[17]     = (checksum) | 0xAA;
	nibble[18]     = 0xDE;
	nibble[19]     = 0xAA;
	nibble[20]     = 0xEB;
	nibble[25]     = 0xD5;
	nibble[26]     = 0xAA;
	nibble[27]     = 0xAD;
	nibble[27+344] = 0xDE;
	nibble[27+345] = 0xAA;
	nibble[27+346] = 0xEB;
	xorvalue = 0;

	for (i = 0; i < 342; i++)
	{
		if (i >= 0x56)
		{
			/* 6 bit */
			oldvalue = data[i - 0x56];
			oldvalue = oldvalue >> 2;
		}
		else
		{
			/* 3 * 2 bit */
			oldvalue = 0;
			oldvalue |= (data[i + 0x00] & 0x01) << 1;
			oldvalue |= (data[i + 0x00] & 0x02) >> 1;
			oldvalue |= (data[i + 0x56] & 0x01) << 3;
			oldvalue |= (data[i + 0x56] & 0x02) << 1;
			if (i + 0xAC < APPLE2_SECTOR_SIZE)
			{
				oldvalue |= (data[i + 0xAC] & 0x01) << 5;
				oldvalue |= (data[i + 0xAC] & 0x02) << 3;
			}

		}
		xorvalue ^= oldvalue;
		nibble[28+i] = translate6[xorvalue & 0x3F];
		xorvalue = oldvalue;
	}

	nibble[27+343] = translate6[xorvalue & 0x3F];
}



static floperr_t apple2_nib_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	floperr_t err;
	uint8_t *track_data;
	void *track_data_v;

	if ((sector < 0) || (sector >= APPLE2_SECTOR_COUNT))
		return FLOPPY_ERROR_SEEKERROR;
	if (buflen != APPLE2_SECTOR_SIZE)
		return FLOPPY_ERROR_INTERNAL;

	err = floppy_load_track(floppy, head, track, true, &track_data_v, nullptr);
	if (err)
		return err;
	track_data = (uint8_t *) track_data_v;

	disk_encode_nib(track_data + sector * APPLE2_NIBBLE_SIZE, (const uint8_t *)buffer, 254, track, sector);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t apple2_nib_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, uint32_t *sector_length)
{
	*sector_length = APPLE2_SECTOR_SIZE;
	return FLOPPY_ERROR_SUCCESS;
}



static uint32_t apple2_get_track_size(floppy_image_legacy *floppy, int head, int track)
{
	return APPLE2_NIBBLE_SIZE * APPLE2_SECTOR_COUNT;
}



/* ----------------------------------------------------------------------- */

LEGACY_FLOPPY_OPTIONS_START( apple2 )
	LEGACY_FLOPPY_OPTION( apple2_do, "do,dsk,bin",  "Apple ][ DOS order disk image",    apple2_dsk_identify,    apple2_do_construct, nullptr,
		HEADS([1])
		TRACKS([40]) // APPLE2_TRACK_COUNT
		SECTORS([16])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([0]))
	LEGACY_FLOPPY_OPTION( apple2_po, "po,dsk,bin",  "Apple ][ ProDOS order disk image", apple2_dsk_identify,    apple2_po_construct, nullptr,
		HEADS([1])
		TRACKS([40]) // APPLE2_TRACK_COUNT
		SECTORS([16])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([0]))
	LEGACY_FLOPPY_OPTION( apple2_nib, "dsk,nib",    "Apple ][ Nibble order disk image", apple2_nib_identify,    apple2_nib_construct, nullptr,
		HEADS([1])
		TRACKS([40]) // APPLE2_TRACK_COUNT
		SECTORS([16])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([0]))
LEGACY_FLOPPY_OPTIONS_END

// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    New implementation

****************************************************************************/

static const uint8_t dos_skewing[] =
{
	0x00, 0x07, 0x0E, 0x06, 0x0D, 0x05, 0x0C, 0x04,
	0x0B, 0x03, 0x0A, 0x02, 0x09, 0x01, 0x08, 0x0F
};

static const uint8_t prodos_skewing[] =
{
	0x00, 0x08, 0x01, 0x09, 0x02, 0x0A, 0x03, 0x0B,
	0x04, 0x0C, 0x05, 0x0D, 0x06, 0x0E, 0x07, 0x0F
};


a2_16sect_format::a2_16sect_format(bool prodos_order) : floppy_image_format_t(), m_prodos_order(prodos_order)
{
}

a2_16sect_dos_format::a2_16sect_dos_format() : a2_16sect_format(false)
{
}

const char *a2_16sect_dos_format::name() const
{
	return "a2_16sect_dos";
}

const char *a2_16sect_dos_format::description() const
{
	return "Apple II 16-sector dsk image (DOS sector order)";
}

const char *a2_16sect_dos_format::extensions() const
{
	return "dsk,do";
}

a2_16sect_prodos_format::a2_16sect_prodos_format() : a2_16sect_format(true)
{
}

const char *a2_16sect_prodos_format::name() const
{
	return "a2_16sect_prodos";
}

const char *a2_16sect_prodos_format::description() const
{
	return "Apple II 16-sector dsk image (ProDos sector order)";
}

const char *a2_16sect_prodos_format::extensions() const
{
	return "dsk,po";
}

bool a2_16sect_format::supports_save() const
{
	return true;
}

int a2_16sect_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return 0;

	//uint32_t expected_size = 35 * 16 * 256;
	uint32_t expected_size = APPLE2_TRACK_COUNT * 16 * 256;

	// check standard size plus some oddball sizes in our softlist
	if ((size != expected_size) && (size != 35 * 16 * 256) && (size != 143403) && (size != 143363) && (size != 143358))
	{
		return 0;
	}

	uint8_t sector_data[256*2];
	static const unsigned char pascal_block1[4] = { 0x08, 0xa5, 0x0f, 0x29 };
	static const unsigned char pascal2_block1[4] = { 0xff, 0xa2, 0x00, 0x8e };
	static const unsigned char dos33_block1[4] = { 0xa2, 0x02, 0x8e, 0x52 };
	static const unsigned char sos_block1[4] = { 0xc9, 0x20, 0xf0, 0x3e };
	static const unsigned char a3a2emul_block1[6] = { 0x8d, 0xd0, 0x03, 0x4c, 0xc7, 0xa4 };
	static const unsigned char cpm22_block1[8] = { 0xa2, 0x55, 0xa9, 0x00, 0x9d, 0x00, 0x0d, 0xca };
	static const unsigned char subnod_block1[8] = { 0x63, 0xaa, 0xf0, 0x76, 0x8d, 0x63, 0xaa, 0x8e };
 
	size_t actual;
	io.read_at(0, sector_data, 256*2, actual);

	bool prodos_order = false;
	// check ProDOS boot block
	if (!memcmp("PRODOS", &sector_data[0x103], 6))
	{
		prodos_order = true;
	}   // check for alternate version ProDOS boot block
	if (!memcmp("PRODOS", &sector_data[0x121], 6))
	{
		prodos_order = true;
	}   // check for ProDOS order SOS disk
	else if (!memcmp(sos_block1, &sector_data[0x100], 4))
	{
		prodos_order = true;
	}   // check for Apple III A2 emulator disk in ProDOS order
	else if (!memcmp(a3a2emul_block1, &sector_data[0x100], 6))
	{
		prodos_order = true;
	}   // check for PCPI Applicard software in ProDOS order
	else if (!memcmp("COPYRIGHT (C) 1979, DIGITAL RESEARCH", &sector_data[0x118], 36))
	{
		prodos_order = true;
	}   // check Apple II Pascal
	else if (!memcmp("SYSTEM.APPLE", &sector_data[0xd7], 12))
	{
		// Pascal discs can still be DOS order.
		// Check for the second half of the boot code at 0x100
		// (which means ProDOS order)
		if (!memcmp(pascal_block1, &sector_data[0x100], 4))
		{
			prodos_order = true;
		}
	}   // check for DOS 3.3 disks in ProDOS order
	else if (!memcmp(dos33_block1, &sector_data[0x100], 4))
	{
		prodos_order = true;
	}   // check for a later version of the Pascal boot block
	else if (!memcmp(pascal2_block1, &sector_data[0x100], 4))
	{
		prodos_order = true;
	}   // check for CP/M disks in ProDOS order
	else if (!memcmp(cpm22_block1, &sector_data[0x100], 8))
	{
		prodos_order = true;
	}   // check for subnodule disk
	else if (!memcmp(subnod_block1, &sector_data[0x100], 8))
	{
		prodos_order = true;
	}   // check for ProDOS 2.5's new boot block
	else if (!memcmp("PRODOS", &sector_data[0x3a], 6))
	{
		prodos_order = true;
	}
	else if (!memcmp("PRODOS", &sector_data[0x40], 6))
	{
		prodos_order = true;
	}

	return FIFID_SIZE | (m_prodos_order == prodos_order ? FIFID_HINT : 0);
}

bool a2_16sect_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	uint64_t size;
	if (io.length(size))
		return false;

	image->set_form_variant(floppy_image::FF_525, floppy_image::SSSD);

	int tracks = (size == (40 * 16 * 256)) ? 40 : 35;

	int fpos = 0;
	for(int track=0; track < tracks; track++) {
		std::vector<uint32_t> track_data;
		uint8_t sector_data[256*16];

		size_t actual;
		io.read_at(fpos, sector_data, 256*16, actual);

		fpos += 256*16;
		for(int i=0; i<49; i++)
			raw_w(track_data, 10, 0x3fc);
		for(int i=0; i<16; i++) {
			int sector;

			if (m_prodos_order)
			{
				sector = prodos_skewing[i];
			}
			else
			{
				sector = dos_skewing[i];
			}

			const uint8_t *sdata = sector_data + 256 * sector;
			for(int j=0; j<20; j++)
				raw_w(track_data, 10, 0x3fc);
			raw_w(track_data,  8, 0xff);
			raw_w(track_data, 24, 0xd5aa96);
			raw_w(track_data, 16, gcr4_encode(0xfe));
			raw_w(track_data, 16, gcr4_encode(track));
			raw_w(track_data, 16, gcr4_encode(i));
			raw_w(track_data, 16, gcr4_encode(0xfe ^ track ^ i));
			raw_w(track_data, 24, 0xdeaaeb);

			for(int j=0; j<4; j++)
				raw_w(track_data, 10, 0x3fc);

			raw_w(track_data,  9, 0x01fe);
			raw_w(track_data, 24, 0xd5aaad);
			raw_w(track_data,  1, 0);

			uint8_t pval = 0x00;
			for(int i=0; i<342; i++) {
				uint8_t nval;
				if(i >= 0x56)
					nval = sdata[i - 0x56] >> 2;
				else {
					nval =
						((sdata[i+0x00] & 0x01) << 1) |
						((sdata[i+0x00] & 0x02) >> 1) |
						((sdata[i+0x56] & 0x01) << 3) |
						((sdata[i+0x56] & 0x02) << 1);
					if(i < 256-0xac)
						nval |=
							((sdata[i+0xac] & 0x01) << 5) |
							((sdata[i+0xac] & 0x02) << 3);
				}
				raw_w(track_data, 8, translate6[nval ^ pval]);
				pval = nval;
			}
			raw_w(track_data, 8, translate6[pval]);
			raw_w(track_data, 24, 0xdeaaeb);
		}
		raw_w(track_data, 8, 0xff);
		assert(track_data.size() == 51090);

		generate_track_from_levels(track, 0, track_data, 0, image);
	}
	return true;
}

uint8_t a2_16sect_format::gb(const std::vector<bool> &buf, int &pos, int &wrap)
{
		uint8_t v = 0;
		int w1 = wrap;
		while(wrap != w1+2 && !(v & 0x80)) {
				v = (v << 1) | buf[pos];
				pos++;
				if(pos == buf.size()) {
						pos = 0;
						wrap++;
				}
		}
		return v;
}

//#define VERBOSE_SAVE

bool a2_16sect_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) const
{
		int g_tracks, g_heads;
		int visualgrid[16][APPLE2_TRACK_COUNT]; // visualizer grid, cleared/initialized below

// lenient addr check: if unset, only accept an addr mark if the checksum was good
// if set, accept an addr mark if the track and sector values are both sane
#undef LENIENT_ADDR_CHECK
// if set, use the old, not as robust logic for choosing which copy of a decoded sector to write
// to the resulting image if the sector has a bad checksum and/or postamble
#undef USE_OLD_BEST_SECTOR_PRIORITY
// nothing found
#define NOTFOUND 0
// address mark was found
#define ADDRFOUND 1
// address checksum is good
#define ADDRGOOD 2
// data mark was found (requires addrfound and sane values)
#define DATAFOUND 4
// data checksum is good
#define DATAGOOD 8
// data postamble is good
#define DATAPOST 16
		for (auto & elem : visualgrid) {
			for (int j = 0; j < APPLE2_TRACK_COUNT; j++) {
				elem[j] = 0;
			}
		}
		image->get_actual_geometry(g_tracks, g_heads);

		int head = 0;

		int pos_data = 0;

		for(int track=0; track < g_tracks; track++) {
				uint8_t sectdata[(256)*16];
				memset(sectdata, 0, sizeof(sectdata));
				int nsect = 16;
				#ifdef VERBOSE_SAVE
				fprintf(stderr,"DEBUG: a2_16sect_format::save() about to generate bitstream from track %d...", track);
				#endif
				auto buf = generate_bitstream_from_track(track, head, 3915, image);
				#ifdef VERBOSE_SAVE
				fprintf(stderr,"done.\n");
				#endif
				int pos = 0;
				int wrap = 0;
				int hb = 0;
				int dosver = 0; // apple dos version; 0 = >=3.3, 1 = <3.3
				for(;;) {
						uint8_t v = gb(buf, pos, wrap);
						if(v == 0xff)               {
								hb = 1;
							}
							else if(hb == 1 && v == 0xd5){
								hb = 2;
								}
							else if(hb == 2 && v == 0xaa) {
								hb = 3;
							}
							else if(hb == 3 && ((v == 0x96) || (v == 0xab))) { // 0x96 = dos 3.3/16sec, 0xab = dos 3.21 and below/13sec
								hb = 4;
								if (v == 0xab) dosver = 1;
								}
							else
								hb = 0;

						if(hb == 4) {
								uint8_t h[11];
								for(auto & elem : h)
										elem = gb(buf, pos, wrap);
								//uint8_t v2 = gcr6bw_tb[h[2]];
								uint8_t vl = gcr4_decode(h[0],h[1]);
								uint8_t tr = gcr4_decode(h[2],h[3]);
								uint8_t se = gcr4_decode(h[4],h[5]);
								uint8_t chk = gcr4_decode(h[6],h[7]);
								#ifdef VERBOSE_SAVE
								uint32_t post = (h[8]<<16)|(h[9]<<8)|h[10];
								printf("Address Mark:\tVolume %d, Track %d, Sector %2d, Checksum %02X: %s, Postamble %03X: %s\n", vl, tr, se, chk, (chk ^ vl ^ tr ^ se)==0?"OK":"BAD", post, (post&0xFFFF00)==0xDEAA00?"OK":"BAD");
								#endif
								// sanity check
								if (tr == track && se < nsect) {
								visualgrid[se][track] |= ADDRFOUND;
								visualgrid[se][track] |= ((chk ^ vl ^ tr ^ se)==0)?ADDRGOOD:0;
#ifdef LENIENT_ADDR_CHECK
									if ((visualgrid[se][track] & ADDRFOUND) == ADDRFOUND) {
#else
									if ((visualgrid[se][track] & ADDRGOOD) == ADDRGOOD) {
#endif
										int opos = pos;
										int owrap = wrap;
										hb = 0;
										for(int i=0; i<20 && hb != 4; i++) {
												v = gb(buf, pos, wrap);
												if(v == 0xff)
														hb = 1;
												else if(hb == 1 && v == 0xd5)
														hb = 2;
												else if(hb == 2 && v == 0xaa)
														hb = 3;
												else if(hb == 3 && v == 0xad)
														hb = 4;
												else
														hb = 0;
										}
										if((hb == 4)&&(dosver == 0)) {
												visualgrid[se][track] |= DATAFOUND;
												uint8_t *dest;
												uint8_t data[0x157];
												uint32_t dpost = 0;
												uint8_t c = 0;

												if (m_prodos_order)
												{
													dest = sectdata+(256)*prodos_skewing[se];
												}
												else
												{
													dest = sectdata+(256)*dos_skewing[se];
												}

												// first read in sector and decode to 6bit form
												for(int i=0; i<0x156; i++) {
														data[i] = gcr6bw_tb[gb(buf, pos, wrap)] ^ c;
														c = data[i];
												//  printf("%02x ", c);
												//  if (((i&0xf)+1)==0x10) printf("\n");
												}
												// read the checksum byte
												data[0x156] = gcr6bw_tb[gb(buf,pos,wrap)];
												// now read the postamble bytes
												for(int i=0; i<3; i++) {
														dpost <<= 8;
														dpost |= gb(buf, pos, wrap);
												}
												// next combine in the upper 2 bits of each byte
												uint8_t bit_swap[4] = { 0, 2, 1, 3 };
												for(int i=0; i<0x56; i++)
														data[i+0x056] = data[i+0x056]<<2 |  bit_swap[data[i]&3];
												for(int i=0; i<0x56; i++)
														data[i+0x0ac] = data[i+0x0ac]<<2 |  bit_swap[(data[i]>>2)&3];
												for(int i=0; i<0x54; i++)
														data[i+0x102] = data[i+0x102]<<2 |  bit_swap[(data[i]>>4)&3];
												// now decode it into 256 bytes
												// but only write it if the bitfield of the track shows datagood is NOT set.
												// if it is set we don't want to overwrite a guaranteed good read with a bad one
												// if past read had a bad checksum or bad postamble...
#ifndef USE_OLD_BEST_SECTOR_PRIORITY
												if (((visualgrid[se][track]&DATAGOOD)==0)||((visualgrid[se][track]&DATAPOST)==0)) {
													// if the current read is good, and postamble is good, write it in, no matter what.
													// if the current read is good and the current postamble is bad, write it in unless the postamble was good before
													// if the current read is bad and the current postamble is good and the previous read had neither good, write it in
													// if the current read isn't good and neither is the postamble but nothing better
													// has been written before, write it anyway.
													if ( ((data[0x156] == c) && (dpost&0xFFFF00)==0xDEAA00) ||
													(((data[0x156] == c) && (dpost&0xFFFF00)!=0xDEAA00) && ((visualgrid[se][track]&DATAPOST)==0)) ||
													(((data[0x156] != c) && (dpost&0xFFFF00)==0xDEAA00) && (((visualgrid[se][track]&DATAGOOD)==0)&&(visualgrid[se][track]&DATAPOST)==0)) ||
													(((data[0x156] != c) && (dpost&0xFFFF00)!=0xDEAA00) && (((visualgrid[se][track]&DATAGOOD)==0)&&(visualgrid[se][track]&DATAPOST)==0))
													) {
														for(int i=0x56; i<0x156; i++) {
															uint8_t dv = data[i];
															*dest++ = dv;
														}
													}
												}
#else
												if ((visualgrid[se][track]&DATAGOOD)==0) {
														for(int i=0x56; i<0x156; i++) {
															uint8_t dv = data[i];
															*dest++ = dv;
														}
												}
#endif
												// do some checking
												#ifdef VERBOSE_SAVE
												if ((data[0x156] != c) || (dpost&0xFFFF00)!=0xDEAA00)
													fprintf(stderr,"Data Mark:\tChecksum xpctd %d found %d: %s, Postamble %03X: %s\n", data[0x156], c, (data[0x156]==c)?"OK":"BAD", dpost, (dpost&0xFFFF00)==0xDEAA00?"OK":"BAD");
												#endif
												if (data[0x156] == c) visualgrid[se][track] |= DATAGOOD;
												if ((dpost&0xFFFF00)==0xDEAA00) visualgrid[se][track] |= DATAPOST;
										} else if ((hb == 4)&&(dosver == 1)) {
											fprintf(stderr,"ERROR: We don't handle dos sectors below 3.3 yet!\n");
										} else {
												pos = opos;
												wrap = owrap;
										}
									}
								}
								hb = 0;
						}
						if(wrap)
								break;
				}
				for(int i=0; i<nsect; i++) {
						//if(nsect>0) printf("t%d,", track);
						uint8_t const *const data = sectdata + (256)*i;
						size_t actual;
						io.write_at(pos_data, data, 256, actual);
						pos_data += 256;
				}
				//printf("\n");
		}
		// display a little table of which sectors decoded ok
		#ifdef VERBOSE_SAVE
		int total_good = 0;
		for (int j = 0; j < APPLE2_TRACK_COUNT; j++) {
			printf("T%2d: ",j);
			for (int i = 0; i < 16; i++) {
				if (visualgrid[i][j] == NOTFOUND) printf("-NF- ");
				else {
					if (visualgrid[i][j] & ADDRFOUND) printf("a"); else printf(" ");
					if (visualgrid[i][j] & ADDRGOOD) printf("A"); else printf(" ");
					if (visualgrid[i][j] & DATAFOUND) printf("d"); else printf(" ");
					if (visualgrid[i][j] & DATAGOOD) { printf("D"); total_good++; } else printf(" ");
					if (visualgrid[i][j] & DATAPOST) printf("."); else printf(" ");
				}
			}
			printf("\n");
		}
		printf("Total Good Sectors: %d\n", total_good);
		#endif

		return true;
}

const a2_16sect_dos_format FLOPPY_A216S_DOS_FORMAT;
const a2_16sect_prodos_format FLOPPY_A216S_PRODOS_FORMAT;
/* RWTS18 format
 * Developed by Roland Gustafsson (http://www.acts.org/roland/index.html)
   for Br0derbund Software around 1986
   This format works as follows:
 * Track 0, in its entirety, is a normal 16-sector track, nothing special.
   (some disks may lack a normal sector 0 on this track, more info needed)
 * Tracks 1 thru 34 are in the special "RWTS18" track format:
   The format consists of six "large" sectors with 768 bytes each.
   Each of those large sectors has a title-specific sync byte and contains
   three "virtual" small sectors of 256 bytes, in an order like follows:
   BigSector    Contains
   0:           0,  6, 12
   1:           1,  7, 13
   2:           2,  8, 14
   3:           3,  9, 15
   4:           4, 10, 16
   5:           5, 11, 17
   The sector format is: (all gcr6)
   D5 9D <track> <sector> <checksum> AA FF FF <titlespecific sync> <0x400 nybbles which represent 768 bytes> <data checksum> D6
   Title-specific sync bytes are:
    Airheart: D4
    Toy Shop: A5
    Carmen USA: unknown (not all released versions used RWTS18)
    Wings of Fury: 96
    Prince of Persia: A9
    And several others.
*/
a2_rwts18_format::a2_rwts18_format() : floppy_image_format_t()
{
}

const char *a2_rwts18_format::name() const
{
		return "a2_rwts18";
}

const char *a2_rwts18_format::description() const
{
		return "Apple II RWTS18-type Image";
}

const char *a2_rwts18_format::extensions() const
{
		return "rti";
}

bool a2_rwts18_format::supports_save() const
{
		return true;
}

int a2_rwts18_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
		uint64_t size;
		if(io.length(size))
			return 0;
		uint32_t const expected_size = APPLE2_TRACK_COUNT * 16 * 256;
		return size == expected_size ? FIFID_SIZE : 0;
}

bool a2_rwts18_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const
{
/*      TODO: rewrite me properly
        uint8_t sector_data[(256)*16];
        memset(sector_data, 0, sizeof(sector_data));

        desc_s sectors[16];
        int format = 0;
        int pos_data = 0;

        int head_count = 1;

        for(int track=0; track < APPLE2_TRACK_COUNT; track++) {
                for(int head=0; head < head_count; head++) {
                        for(int si=0; si<16; si++) {
                                uint8_t *data = sector_data + (256)*si;
                                sectors[si].data = data;
                                sectors[si].size = 256;
                                sectors[si].sector_id = si;
                                sectors[si].sector_info = format;
                                size_t actual;
                                io.read_at(pos_data, data, 256, actual);
                                pos_data += 256;
                        }
                        generate_track(mac_gcr, track, head, sectors, 16, 3104*16, image);
                }
        }
        return true;*/
		return false; // I hope that throws an error...
}

uint8_t a2_rwts18_format::gb(const std::vector<bool> &buf, int &pos, int &wrap)
{
		uint8_t v = 0;
		int w1 = wrap;
		while(wrap != w1+2 && !(v & 0x80)) {
				v = (v << 1) | buf[pos];
				pos++;
				if(pos == buf.size()) {
						pos = 0;
						wrap++;
				}
		}
		return v;
}

bool a2_rwts18_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) const
{
		int g_tracks, g_heads;
		int visualgrid[18][APPLE2_TRACK_COUNT]; // visualizer grid, cleared/initialized below
// lenient addr check: if unset, only accept an addr mark if the checksum was good
// if set, accept an addr mark if the track and sector values are both sane
#undef LENIENT_ADDR_CHECK
// if set, use the old, not as robust logic for choosing which copy of a decoded sector to write
// to the resulting image if the sector has a bad checksum and/or postamble
#undef USE_OLD_BEST_SECTOR_PRIORITY
// select a sector order for resulting file: 0 = logical, 1 = dos3.3, 2 = prodos
#define SECTOR_ORDER 1
// nothing found
#define NOTFOUND 0
// address mark was found
#define ADDRFOUND 1
// address checksum is good
#define ADDRGOOD 2
// data mark was found (requires addrfound and sane values)
#define DATAFOUND 4
// data checksum is good
#define DATAGOOD 8
// data postamble is good
#define DATAPOST 16
		for (auto & elem : visualgrid) {
			for (int j = 0; j < APPLE2_TRACK_COUNT; j++) {
				elem[j] = 0;
			}
		}
		image->get_actual_geometry(g_tracks, g_heads);

		int head = 0;

		int pos_data = 0;

		// for track 0 ONLY:
		uint8_t sectdata[(768)*6];
		memset(sectdata, 0, sizeof(sectdata));
		int nsect = 18;
//fprintf(stderr,"DEBUG: a2_rwts18_format::save() about to generate bitstream from physical track %d (logical %d)...", track, track/2);
		//~332 samples per cell, times 3+8+3 (14) for address mark, 24 for sync, 3+343+3 (349) for data mark, 24 for sync is around 743, near 776 expected
		auto buf = generate_bitstream_from_track(0, head, 200000000/((3004*nsect*6)/2), image); // 3104 needs tweaking
//fprintf(stderr,"done.\n");
		int pos = 0;
		int wrap = 0;
		int hb = 0;
		int dosver = 0; // apple dos version; 0 = >=3.3, 1 = <3.3
		for(;;) {
				uint8_t v = gb(buf, pos, wrap);
				if(v == 0xff)
						hb = 1;
				else if(hb == 1 && v == 0xd5)
						hb = 2;
				else if(hb == 2 && v == 0xaa)
						hb = 3;
				else if(hb == 3 && ((v == 0x96) || (v == 0xab))) { // 0x96 = dos 3.3/16sec, 0xab = dos 3.21 and below/13sec
						hb = 4;
						if (v == 0xab) dosver = 1;
						}
				else
						hb = 0;

				if(hb == 4) {
						uint8_t h[11];
						for(auto & elem : h)
								elem = gb(buf, pos, wrap);
						//uint8_t v2 = gcr6bw_tb[h[2]];
						uint8_t vl = gcr4_decode(h[0],h[1]);
						uint8_t tr = gcr4_decode(h[2],h[3]);
						uint8_t se = gcr4_decode(h[4],h[5]);
						uint8_t chk = gcr4_decode(h[6],h[7]);
						uint32_t post = (h[8]<<16)|(h[9]<<8)|h[10];
						printf("Address Mark:\tVolume %d, Track %d, Sector %2d, Checksum %02X: %s, Postamble %03X: %s\n", vl, tr, se, chk, (chk ^ vl ^ tr ^ se)==0?"OK":"BAD", post, (post&0xFFFF00)==0xDEAA00?"OK":"BAD");
						// sanity check
						if (tr == 0 && se < nsect) {
						visualgrid[se][0] |= ADDRFOUND;
						visualgrid[se][0] |= ((chk ^ vl ^ tr ^ se)==0)?ADDRGOOD:0;
#ifdef LENIENT_ADDR_CHECK
//                          if ((visualgrid[se][0] & ADDRFOUND) == ADDRFOUND) {
#else
							if ((visualgrid[se][0] & ADDRGOOD) == ADDRGOOD) {
#endif
								int opos = pos;
								int owrap = wrap;
								hb = 0;
								for(int i=0; i<20 && hb != 4; i++) {
										v = gb(buf, pos, wrap);
										if(v == 0xff)
												hb = 1;
										else if(hb == 1 && v == 0xd5)
												hb = 2;
										else if(hb == 2 && v == 0xaa)
												hb = 3;
										else if(hb == 3 && v == 0xad)
												hb = 4;
										else
												hb = 0;
								}
								if((hb == 4)&&(dosver == 0)) {
										visualgrid[se][0] |= DATAFOUND;
										int sector_translate[16] = {
#if SECTOR_ORDER == 0
										// logical order (0-15)
										0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
										0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
#elif SECTOR_ORDER == 1
										// DOS order (*.do)
										0x00, 0x07, 0x0E, 0x06, 0x0D, 0x05, 0x0C, 0x04,
										0x0B, 0x03, 0x0A, 0x02, 0x09, 0x01, 0x08, 0x0F
#elif SECTOR_ORDER == 2
										// prodos order (*.po)
										0x00, 0x08, 0x01, 0x09, 0x02, 0x0A, 0x03, 0x0B,
										0x04, 0x0C, 0x05, 0x0D, 0x06, 0x0E, 0x07, 0x0F
#endif
										};
										uint8_t *dest = sectdata+(256)*sector_translate[se];
										uint8_t data[0x157];
										uint32_t dpost = 0;
										uint8_t c = 0;
										// first read in sector and decode to 6bit form
										for(int i=0; i<0x156; i++) {
												data[i] = gcr6bw_tb[gb(buf, pos, wrap)] ^ c;
												c = data[i];
										//  printf("%02x ", c);
										//  if (((i&0xf)+1)==0x10) printf("\n");
										}
										// read the checksum byte
										data[0x156] = gcr6bw_tb[gb(buf,pos,wrap)];
										// now read the postamble bytes
										for(int i=0; i<3; i++) {
												dpost <<= 8;
												dpost |= gb(buf, pos, wrap);
										}
										// next combine in the upper 2 bits of each byte
										uint8_t bit_swap[4] = { 0, 2, 1, 3 };
										for(int i=0; i<0x56; i++)
												data[i+0x056] = data[i+0x056]<<2 |  bit_swap[data[i]&3];
										for(int i=0; i<0x56; i++)
												data[i+0x0ac] = data[i+0x0ac]<<2 |  bit_swap[(data[i]>>2)&3];
										for(int i=0; i<0x54; i++)
												data[i+0x102] = data[i+0x102]<<2 |  bit_swap[(data[i]>>4)&3];
										// now decode it into 256 bytes
										// but only write it if the bitfield of the track shows datagood is NOT set.
										// if it is set we don't want to overwrite a guaranteed good read with a bad one
										// if past read had a bad checksum or bad postamble...
#ifndef USE_OLD_BEST_SECTOR_PRIORITY
										if (((visualgrid[se][0]&DATAGOOD)==0)||((visualgrid[se][0]&DATAPOST)==0)) {
											// if the current read is good, and postamble is good, write it in, no matter what.
											// if the current read is good and the current postamble is bad, write it in unless the postamble was good before
											// if the current read is bad and the current postamble is good and the previous read had neither good, write it in
											// if the current read isn't good and neither is the postamble but nothing better
											// has been written before, write it anyway.
											if ( ((data[0x156] == c) && (dpost&0xFFFF00)==0xDEAA00) ||
											(((data[0x156] == c) && (dpost&0xFFFF00)!=0xDEAA00) && ((visualgrid[se][0]&DATAPOST)==0)) ||
											(((data[0x156] != c) && (dpost&0xFFFF00)==0xDEAA00) && (((visualgrid[se][0]&DATAGOOD)==0)&&(visualgrid[se][0]&DATAPOST)==0)) ||
											(((data[0x156] != c) && (dpost&0xFFFF00)!=0xDEAA00) && (((visualgrid[se][0]&DATAGOOD)==0)&&(visualgrid[se][0]&DATAPOST)==0))
											) {
												for(int i=0x56; i<0x156; i++) {
													uint8_t dv = data[i];
													*dest++ = dv;
												}
											}
										}
#else
										if ((visualgrid[se][0]&DATAGOOD)==0) {
												for(int i=0x56; i<0x156; i++) {
													uint8_t dv = data[i];
													*dest++ = dv;
												}
										}
#endif
										// do some checking
										if ((data[0x156] != c) || (dpost&0xFFFF00)!=0xDEAA00)
											fprintf(stderr,"Data Mark:\tChecksum xpctd %d found %d: %s, Postamble %03X: %s\n", data[0x156], c, (data[0x156]==c)?"OK":"BAD", dpost, (dpost&0xFFFF00)==0xDEAA00?"OK":"BAD");
										if (data[0x156] == c) visualgrid[se][0] |= DATAGOOD;
										if ((dpost&0xFFFF00)==0xDEAA00) visualgrid[se][0] |= DATAPOST;
								} else if ((hb == 4)&&(dosver == 1)) {
									fprintf(stderr,"ERROR: We don't handle dos sectors below 3.3 yet!\n");
								} else {
										pos = opos;
										wrap = owrap;
								}
							}
						}
						hb = 0;
				}
				if(wrap)
						break;
		}
		for(int i=0; i<nsect; i++) {
				//if(nsect>0) printf("t%d,", track);
				uint8_t const *const data = sectdata + (256)*i;
				size_t actual;
				io.write_at(pos_data, data, 256, actual);
				pos_data += 256;
		}

		// for the rest of the tracks
		for(int track=2; track < 70; track+=2) {
				uint8_t sectdata[(768)*6];
				memset(sectdata, 0, sizeof(sectdata));
				int nsect = 18;
//fprintf(stderr,"DEBUG: a2_rwts18_format::save() about to generate bitstream from physical track %d (logical %d)...", track, track/2);
				//~332 samples per cell, times 3+8+3 (14) for address mark, 24 for sync, 3+343+3 (349) for data mark, 24 for sync is around 743, near 776 expected
				auto buf = generate_bitstream_from_track(track, head, 200000000/((3004*nsect*6)/2), image); // 3104 needs tweaking
//fprintf(stderr,"done.\n");
				int oldpos = 0; // DEBUG
				int pos = 0;
				int wrap = 0;
				int hb = 0;
				for(;;) {
						uint8_t v = gb(buf, pos, wrap);
						if((v == 0xff) || (v == 0x9a)) // note 0x9a varies per title! this is an LFSR? generated value intended to throw off copiers, and only appears after the track splice (before sector 5)
								hb = 1;
						else if(hb == 1 && v == 0xd5)
								hb = 2;
						else if(hb == 2 && v == 0x9d)
								hb = 3;
						else
								hb = 0;

						if(hb == 3) {
								printf("AM at offset: %d, relative: %d\n", pos, pos-oldpos);
								oldpos=pos;
								uint8_t h[7];
								// grab exactly 7 bytes: should be Track, Sector, Checksum, AA, FF and FF and the Br0derbund Title ID
								for(auto & elem : h)
										elem = gb(buf, pos, wrap);
								uint8_t tr = gcr6bw_tb[h[0]];
								uint8_t se = gcr6bw_tb[h[1]];
								uint8_t chk = gcr6bw_tb[h[2]];
								uint32_t post = (h[3]<<16)|(h[4]<<8)|h[5];
								uint8_t bbundid = h[6];
								printf("RWTS18 AM:\t Track %d, Sector %2d, Checksum %02X: %s, Postamble %03X: %s, BBUNDID %02x\n", tr, se, chk, (chk ^ tr ^ se)==0?"OK":"BAD", post, post==0xAAFFFF?"OK":"BAD", bbundid);
								// sanity check
								if (tr == track/2 && se < nsect) {
								visualgrid[se][track/2] |= ADDRFOUND;
								visualgrid[se][track/2] |= ((chk ^ tr ^ se)==0)?ADDRGOOD:0;
#ifdef LENIENT_ADDR_CHECK
//                                  if ((visualgrid[se][track/2] & ADDRFOUND) == ADDRFOUND) {
#else
									if ((visualgrid[se][track/2] & ADDRGOOD) == ADDRGOOD) {
#endif
										//int opos = pos;
										//int owrap = wrap;
										// RWTS18 doesn't have a true data mark, its part of the address header
										visualgrid[se][track/2] |= DATAFOUND;
										uint8_t *dest = sectdata+(256)*se;
										uint8_t data[0x401];
										uint32_t dpost = 0;
										uint8_t c = 0;
										//dest = sectdata+(768)*se;
										// now read in the sector and decode to 6bit form
										for(int i=0; i<0x400; i++) {
											data[i] = gcr6bw_tb[gb(buf, pos, wrap)] ;//^ c;
											c ^= data[i];
											/*if (((i&0x3)+1)==0x04) {
											    printf("%c", ((((data[i-3]&0x30)<<2)|((data[i-2]&0x3F)>>0))&0x3F)+0x40);
											    printf("%c", ((((data[i-3]&0x0C)<<4)|((data[i-1]&0x3F)>>0))&0x3F)+0x40);
											    printf("%c", ((((data[i-3]&0x03)<<6)|((data[i-0]&0x3F)>>0))&0x3F)+0x40);
											    *dest++ = ((data[i-3]&0x30)<<2)|((data[i-2]&0x3F)>>0);
											    *dest++ = ((data[i-3]&0x0C)<<4)|((data[i-1]&0x3F)>>0);
											    *dest++ = ((data[i-3]&0x03)<<6)|((data[i-0]&0x3F)>>0);
											}*/
										//  printf("%02x ", data[i]);
										//  if (((i&0xf)+1)==0x10) printf("\n");
										}
										// read the checksum byte (checksum is calced by xoring all data together)
										data[0x400] = gcr6bw_tb[gb(buf,pos,wrap)];
										// now read the postamble bytes
										for(int i=0; i<4; i++) {
												dpost <<= 8;
												dpost |= gb(buf, pos, wrap);
										}

										/*if (se == 0) // dump some debug data to help find the lfsr before sector 5
										{
										    printf("Data Postamble was 0x%08x\n", dpost);
										    for(int i=0; i<0x400; i++) {
										        data[i] = gcr6bw_tb[gb(buf, pos, wrap)] ;//^ c;
										        c ^= data[i];
										  printf("%02x ", data[i]);
										  if (((i&0xf)+1)==0x10) printf("\n");
										    }
										}*/

										// only write it if the bitfield of the track shows datagood is NOT set.
										// if it is set we don't want to overwrite a guaranteed good read with a bad one
										// if past read had a bad checksum or bad postamble...
#ifndef USE_OLD_BEST_SECTOR_PRIORITY
										if (((visualgrid[se][track/2]&DATAGOOD)==0)||((visualgrid[se][track/2]&DATAPOST)==0)) {
											// if the current read is good, and postamble is good, write it in, no matter what.
											// if the current read is good and the current postamble is bad, write it in unless the postamble was good before
											// if the current read is bad and the current postamble is good and the previous read had neither good, write it in
											// if the current read isn't good and neither is the postamble but nothing better
											// has been written before, write it anyway.
											if ( ((data[0x400] == c) && (dpost&0xFF000000)==0xD4000000) ||
											(((data[0x400] == c) && (dpost&0xFF000000)!=0xD4000000) && ((visualgrid[se][track/2]&DATAPOST)==0)) ||
											(((data[0x400] != c) && (dpost&0xFF000000)==0xD4000000) && (((visualgrid[se][track/2]&DATAGOOD)==0)&&(visualgrid[se][track/2]&DATAPOST)==0)) ||
											(((data[0x400] != c) && (dpost&0xFF000000)!=0xD4000000) && (((visualgrid[se][track/2]&DATAGOOD)==0)&&(visualgrid[se][track/2]&DATAPOST)==0))
											) {
												// next combine adjacent data bytes to form the 3 constituent sectors
												// format is 0x00AaBbCc 0x00aaaaaa 0x00bbbbbb 0x00cccccc 0x00AaBbCc ... etc
												// aa is sector 0, bb is sector 6, cc is sector 12
												// first sector:
												dest = sectdata+(256)*se;
												for(int i=0; i<0x100; i++) {
														data[(4*i)+1] |= (data[4*i]&0x30)<<2;
														//printf("%c", (data[4*i]&0x3F)+0x40);
														//if (((i&0xf)+1)==0x10) printf("\n");
														uint8_t dv = data[(4*i)+1];
														*dest++ = dv;
														}
												// second sector:
												dest = sectdata+(256)*(se+6);
												for(int i=0; i<0x100; i++) {
														data[(4*i)+2] |= (data[4*i]&0x0c)<<4;
														uint8_t dv = data[(4*i)+2];
														*dest++ = dv;
														}
												// third sector:
												dest = sectdata+(256)*(se+12);
												for(int i=0; i<0x100; i++) {
														data[(4*i)+3] |= (data[4*i]&0x03)<<6;
														uint8_t dv = data[(4*i)+3];
														*dest++ = dv;
														}
													}
										}
#else
										if ((visualgrid[se][track/2]&DATAGOOD)==0) {
											// next combine adjacent data bytes to form the 3 constituent sectors
											// format is 0x00AaBbCc 0x00aaaaaa 0x00bbbbbb 0x00cccccc 0x00AaBbCc ... etc
											// aa is sector 0, bb is sector 6, cc is sector 12
											// first sector:
											dest = sectdata+(256)*se;
											for(int i=0; i<0x100; i++) {
													data[(4*i)+1] |= (data[4*i]&0x30)<<2;
													//printf("%c", (data[4*i]&0x3F)+0x40);
													//if (((i&0xf)+1)==0x10) printf("\n");
													uint8_t dv = data[(4*i)+1];
													*dest++ = dv;
													}
											// second sector:
											dest = sectdata+(256)*(se+6);
											for(int i=0; i<0x100; i++) {
													data[(4*i)+2] |= (data[4*i]&0x0c)<<4;
													uint8_t dv = data[(4*i)+2];
													*dest++ = dv;
													}
											// third sector:
											dest = sectdata+(256)*(se+12);
											for(int i=0; i<0x100; i++) {
													data[(4*i)+3] |= (data[4*i]&0x03)<<6;
													uint8_t dv = data[(4*i)+3];
													*dest++ = dv;
													}
										}
#endif
										// do some checking
										if ((data[0x400] != c) || (dpost&0xFF000000)!=0xD4000000)
											fprintf(stderr,"Data Mark:\tChecksum xpctd %d found %d: %s, Postamble %03X: %s\n", data[0x400], c, (data[0x400]==c)?"OK":"BAD", dpost, (dpost&0xFF000000)==0xD4000000?"OK":"BAD");
										if (data[0x400] == c) visualgrid[se][track/2] |= DATAGOOD;
										if ((dpost&0xFF000000)==0xD4000000) visualgrid[se][track/2] |= DATAPOST;
									}
								}
								hb = 0;
						}
						if(wrap)
								break;
				}
				for(int i=0; i<nsect; i++) {
						//if(nsect>0) printf("t%d,", track);
						uint8_t const *const data = sectdata + (256)*i;
						size_t actual;
						io.write_at(pos_data, data, 256, actual);
						pos_data += 256;
				}
				//printf("\n");
		}
		// display a little table of which sectors decoded ok
		int total_good = 0;
		for (int j = 0; j < APPLE2_TRACK_COUNT; j++) {
			printf("T%2d: ",j);
			for (int i = 0; i < (j==0?16:6); i++) {
				if (visualgrid[i][j] == NOTFOUND) printf("-NF- ");
				else {
				if (visualgrid[i][j] & ADDRFOUND) printf("a"); else printf(" ");
				if (visualgrid[i][j] & ADDRGOOD) printf("A"); else printf(" ");
				if (visualgrid[i][j] & DATAFOUND) printf("d"); else printf(" ");
				if (visualgrid[i][j] & DATAGOOD) { printf("D"); total_good++; } else printf(" ");
				if (visualgrid[i][j] & DATAPOST) printf("."); else printf(" ");
				}
			}
			printf("\n");
		}
		printf("Total Good Sectors: %d\n", total_good);

		return true;
}

const a2_rwts18_format FLOPPY_RWTS18_FORMAT;

a2_edd_format::a2_edd_format() : floppy_image_format_t()
{
}

const char *a2_edd_format::name() const
{
	return "a2_edd";
}

const char *a2_edd_format::description() const
{
	return "Apple II EDD Image";
}

const char *a2_edd_format::extensions() const
{
	return "edd";
}

bool a2_edd_format::supports_save() const
{
	return false;
}

int a2_edd_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return 0;
	return ((size == 2244608) || (size == 2310144)) ? FIFID_SIZE : 0;
}

uint8_t a2_edd_format::pick(const uint8_t *data, int pos)
{
	return ((data[pos>>3] << 8) | data[(pos>>3)+1]) >> (8-(pos & 7));
}

bool a2_edd_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	uint8_t nibble[16384], stream[16384];
	int npos[16384];

	std::unique_ptr<uint8_t []> img(new (std::nothrow) uint8_t[2244608]);
	if (!img)
		return false;

	size_t actual;
	io.read_at(0, img.get(), 2244608, actual);

	for(int i=0; i<137; i++) {
		uint8_t const *const trk = &img[16384*i];
		int pos = 0;
		int wpos = 0;
		while(pos < 16383*8) {
			uint8_t acc = pick(trk, pos);
			pos += 8;
			while(!(acc & 0x80) && pos < 16384*8) {
				acc <<= 1;
				if(trk[pos >> 3] & (0x80 >> (pos & 7)))
					acc |= 0x01;
				pos++;
			}
			if(acc & 0x80) {
				nibble[wpos] = acc;
				npos[wpos] = pos;
				wpos++;
			}
		}
		int nm = 0, nmj = 0, nmk = 0;
		for(int j=0; j<wpos-1; j++)
			for(int k=j+6200; k<wpos && k<j+6400; k++) {
				int m = 0;
				for(int l=0; k+l<wpos && nibble[j+l] == nibble[k+l]; l++)
					m++;
				if(m > nm) {
					nm = m;
					nmj = j;
					nmk = k;
				}
			}
		int delta = nmk - nmj;
		int spos = (wpos-delta)/2;
		int zpos = npos[spos];
		int epos = npos[spos+delta];
		int len = epos-zpos;
		int part1_size = zpos % len;
		int part1_bsize = part1_size >> 3;
		int part1_spos = epos-part1_size;
		int part2_offset = zpos - part1_size;
		int total_bsize = (len+7) >> 3;

		for(int j=0; j<part1_bsize; j++)
			stream[j] = pick(trk, part1_spos + 8*j);
		stream[part1_bsize] =
			(pick(trk, part1_spos + 8*part1_bsize) & (0xff00 >> (part1_size & 7))) |
			(pick(trk, part2_offset + 8*part1_bsize) & (0x00ff >> (part1_size & 7)));
		for(int j=part1_bsize+1; j<total_bsize; j++)
			stream[j] = pick(trk, part2_offset + 8*j);

		bool odd = false;
		for(int j=0; j<len; j++)
			if(stream[j>>3] & (0x80 >> (j & 7)))
				odd = !odd;

		int splice_byte = spos;
		while(splice_byte < spos+delta && (npos[splice_byte+1] - npos[splice_byte] != 8 || npos[splice_byte+2] - npos[splice_byte+1] == 8 || npos[splice_byte+3] - npos[splice_byte+2] == 8))
			splice_byte++;
		int splice = (npos[splice_byte+2]-1) % len;
		if(odd)
			stream[splice >> 3] ^= 0x80 >> (splice & 7);

		generate_track_from_bitstream(i >> 2, 0, stream, len, image, i & 3);
		image->set_write_splice_position(i >> 2, 0, uint32_t(uint64_t(200'000'000)*splice/len), i & 3);
	}
	img.reset();

	image->set_form_variant(floppy_image::FF_525, floppy_image::SSSD);

	return true;
}

const a2_edd_format FLOPPY_EDD_FORMAT;


a2_woz_format::a2_woz_format() : floppy_image_format_t()
{
}

const char *a2_woz_format::name() const
{
	return "a2_woz";
}

const char *a2_woz_format::description() const
{
	return "Apple II WOZ Image";
}

const char *a2_woz_format::extensions() const
{
	return "woz";
}

bool a2_woz_format::supports_save() const
{
	return true;
}

const uint8_t a2_woz_format::signature[8] = { 0x57, 0x4f, 0x5a, 0x31, 0xff, 0x0a, 0x0d, 0x0a };
const uint8_t a2_woz_format::signature2[8] = { 0x57, 0x4f, 0x5a, 0x32, 0xff, 0x0a, 0x0d, 0x0a };

int a2_woz_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t header[8];
	size_t actual;
	io.read_at(0, header, 8, actual);
	if (!memcmp(header, signature, 8)) return FIFID_SIGN;
	if (!memcmp(header, signature2, 8)) return FIFID_SIGN;
	return 0;
}

bool a2_woz_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	uint64_t image_size;
	if(io.length(image_size))
		return false;
	std::vector<uint8_t> img(image_size);
	size_t actual;
	io.read_at(0, &img[0], img.size(), actual);

	// Check signature
	if((memcmp(&img[0], signature, 8)) && (memcmp(&img[0], signature2, 8)))
		return false;

	uint32_t woz_vers = 1;
	if(!memcmp(&img[0], signature2, 8)) woz_vers = 2;

	// Check integrity
	uint32_t crc = crc32r(&img[12], img.size() - 12);
	if(crc != r32(img, 8))
		return false;

	uint32_t off_info = find_tag(img, 0x4f464e49);
	uint32_t off_tmap = find_tag(img, 0x50414d54);
	uint32_t off_trks = find_tag(img, 0x534b5254);
//  uint32_t off_writ = find_tag(img, 0x54495257);

	if(!off_info || !off_tmap || !off_trks)
		return false;

	uint32_t info_vers = r8(img, off_info + 0);

	if((info_vers != 1) && (info_vers != 2))
		return false;

	bool is_35 = r8(img, off_info + 1) == 2;
	if((form_factor == floppy_image::FF_35 && !is_35) || (form_factor == floppy_image::FF_525 && is_35))
		return false;

	unsigned int limit = is_35 ? 160 : 141;

	if(is_35)
		image->set_form_variant(floppy_image::FF_35, floppy_image::SSDD);
	else
		image->set_form_variant(floppy_image::FF_525, floppy_image::SSSD);

	if (woz_vers == 1) {

		for (unsigned int trkid = 0; trkid != limit; trkid++) {
			int head = is_35 && trkid >= 80 ? 1 : 0;
			int track = is_35 ? trkid % 80 : trkid / 4;
			int subtrack = is_35 ? 0 : trkid & 3;

			uint8_t idx = r8(img, off_tmap + trkid);
			if(idx != 0xff) {
				uint32_t boff = off_trks + 6656*idx;
				if (r16(img, boff + 6648) == 0)
					return false;
				generate_track_from_bitstream(track, head, &img[boff], r16(img, boff + 6648), image, subtrack, r16(img, boff + 6650));
				if(is_35 && !track && head)
					image->set_variant(floppy_image::DSDD);
			}
		}
	} else if (woz_vers == 2) {
		for (unsigned int trkid = 0; trkid != limit; trkid++) {
			int head = is_35 && trkid & 1 ? 1 : 0;
			int track = is_35 ? trkid >> 1 : trkid / 4;
			int subtrack = is_35 ? 0 : trkid & 3;

			uint8_t idx = r8(img, off_tmap + trkid);
			if(idx != 0xff) {
				uint32_t trks_off = off_trks + (idx * 8);

				uint32_t boff = (uint32_t)r16(img, trks_off + 0) * 512;

				if (r32(img, trks_off + 4) == 0)
					return false;

				uint32_t track_size = r32(img, trks_off + 4);

				// With 5.25 floppies the end-of-track may be missing
				// if unformatted.  Accept track length down to 95% of
				// 51090, otherwise pad it

				bool short_track = !is_35 && track_size < 48535;

				if(short_track) {
					std::vector<uint8_t> buffer(6387, 0);
					memcpy(buffer.data(), &img[boff], (track_size + 7) / 8);
					generate_track_from_bitstream(track, head, buffer.data(), 51090, image, subtrack, 0xffff);

				} else
					generate_track_from_bitstream(track, head, &img[boff], track_size, image, subtrack, 0xffff);

				if(is_35 && !track && head)
					image->set_variant(r32(img, trks_off + 4) >= 90000 ? floppy_image::DSHD : floppy_image::DSDD);
			}
		}
	}
	else return false;

	return true;
}

bool a2_woz_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	std::vector<std::vector<bool>> tracks(160);
	bool twosided = false;

	if(image->get_form_factor() == floppy_image::FF_525) {
		for(unsigned int i=0; i != 160; i++)
			if(image->track_is_formatted(i >> 2, 0, i & 3))
				tracks[i] = generate_bitstream_from_track(i >> 2, 0, 3915, image, i & 3);

	} else if(image->get_variant() == floppy_image::DSHD) {
		for(unsigned int i=0; i != 160; i++)
			if(image->track_is_formatted(i >> 1, i & 1)) {
				tracks[i] = generate_bitstream_from_track(i >> 1, i & 1, 1000, image);
				if(i & 1)
					twosided = true;
			}

	} else {
		// 200000000 / 60.0 * 1.979e-6 ~= 6.5967
		static const int cell_size_per_speed_zone[5] = {
			394 * 65967 / 10000,
			429 * 65967 / 10000,
			472 * 65967 / 10000,
			525 * 65967 / 10000,
			590 * 65967 / 10000
		};

		for(unsigned int i=0; i != 160; i++)
			if(image->track_is_formatted(i >> 1, i & 1)) {
				tracks[i] = generate_bitstream_from_track(i >> 1, i & 1, cell_size_per_speed_zone[i / (2*16)], image);
				if(i & 1)
					twosided = true;
			}
	}

	int max_blocks = 0;
	int total_blocks = 0;
	for(const auto &t : tracks) {
		int blocks = (t.size() + 4095) / 4096;
		total_blocks += blocks;
		if(max_blocks < blocks)
			max_blocks = blocks;
	}

	std::vector<uint8_t> data(1536 + total_blocks*512, 0);

	memcpy(&data[0], signature2, 8);

	w32(data, 12, 0x4F464E49);  // INFO
	w32(data, 16, 60);          // size
	data[20] = 2;               // chunk version
	data[21] = image->get_form_factor() == floppy_image::FF_525 ? 1 : 2;
	data[22] = 0;               // not write protected
	data[23] = 1;               // synchronized, since our internal format is
	data[24] = 1;               // weak bits are generated, not stored
	data[25] = 'M';
	data[26] = 'A';
	data[27] = 'M';
	data[28] = 'E';
	memset(&data[29], ' ', 32-4);
	data[57] = twosided ? 2 : 1;
	data[58] = 0;               // boot sector unknown
	data[59] = image->get_form_factor() == floppy_image::FF_525 ? 32 : image->get_variant() == floppy_image::DSHD ? 8 : 16;
	w16(data, 60, 0);           // compatibility unknown
	w16(data, 62, 0);           // needed ram unknown
	w16(data, 64, max_blocks);
	w32(data, 80, 0x50414D54);  // TMAP
	w32(data, 84, 160);         // size

	uint8_t tcount = 0;
	for(int i=0; i != 160 ; i++)
		data[88 + i] = tracks[i].empty() ? 0xff : tcount++;

	w32(data, 248, 0x534B5254); // TRKS
	w32(data, 252, 1280 + total_blocks*512);   // size

	uint8_t tid = 0;
	uint16_t tb = 3;
	for(int i=0; i != 160 ; i++)
		if(!tracks[i].empty()) {
			int blocks = (tracks[i].size() + 4095) / 4096;
			w16(data, 256 + tid*8, tb);
			w16(data, 256 + tid*8 + 2, blocks);
			w32(data, 256 + tid*8 + 4, tracks[i].size());
			tb += blocks;
			tid ++;
		}

	tb = 3;
	for(int i=0; i != 160 ; i++)
		if(!tracks[i].empty()) {
			int off = tb * 512;
			int size = tracks[i].size();
			for(int j=0; j != size; j++)
				if(tracks[i][j])
					data[off + (j >> 3)] |= 0x80 >> (j & 7);
			tb += (size + 4095) / 4096;
		}

	w32(data, 8, crc32r(&data[12], data.size() - 12));

	size_t actual;
	io.write_at(0, data.data(), data.size(), actual);
	return true;
}


uint32_t a2_woz_format::find_tag(const std::vector<uint8_t> &data, uint32_t tag)
{
	uint32_t offset = 12;
	do {
		if(r32(data, offset) == tag)
			return offset + 8;
		offset += r32(data, offset+4) + 8;
	} while(offset < data.size() - 8);
	return 0;
}

uint32_t a2_woz_format::r32(const std::vector<uint8_t> &data, uint32_t offset)
{
	return data[offset] | (data[offset+1] << 8) | (data[offset+2] << 16) | (data[offset+3] << 24);
}

uint16_t a2_woz_format::r16(const std::vector<uint8_t> &data, uint32_t offset)
{
	return data[offset] | (data[offset+1] << 8);
}

uint8_t a2_woz_format::r8(const std::vector<uint8_t> &data, uint32_t offset)
{
	return data[offset];
}

void a2_woz_format::w32(std::vector<uint8_t> &data, int offset, uint32_t value)
{
	data[offset] = value;
	data[offset+1] = value >> 8;
	data[offset+2] = value >> 16;
	data[offset+3] = value >> 24;
}

void a2_woz_format::w16(std::vector<uint8_t> &data, int offset, uint16_t value)
{
	data[offset] = value;
	data[offset+1] = value >> 8;
}

uint32_t a2_woz_format::crc32r(const uint8_t *data, uint32_t size)
{
	// Reversed crc32
	uint32_t crc = 0xffffffff;
	for(uint32_t i=0; i != size; i++) {
		crc = crc ^ data[i];
		for(int j=0; j<8; j++)
			if(crc & 1)
				crc = (crc >> 1) ^ 0xedb88320;
			else
				crc = crc >> 1;
	}
	return ~crc;
}


const a2_woz_format FLOPPY_WOZ_FORMAT;


a2_nib_format::a2_nib_format() : floppy_image_format_t()
{
}

const char *a2_nib_format::name() const
{
	return "a2_nib";
}

const char *a2_nib_format::description() const
{
	return "Apple II NIB Image";
}

const char *a2_nib_format::extensions() const
{
	return "nib";
}

bool a2_nib_format::supports_save() const
{
	return false;
}

int a2_nib_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return 0;

	if (size == expected_size_35t || size == expected_size_40t)
		return FIFID_SIZE;

	return 0;
}


template<class It>
static
size_t count_leading_FFs(const It first, const It last)
{
	auto curr = first;
	for (; curr != last; ++curr) {
		if (*curr != 0xFF) {
			break;
		}
	}
	return curr - first;
}

static
size_t count_trailing_padding(const std::vector<uint8_t>& nibbles) {
	const auto b = nibbles.rbegin();
	const auto e = nibbles.rend();
	auto i = b;

	// skip until the first valid nibble...
	for (; i != e; ++i) {
		if ((*i & 0x80) != 0) { // valid nibble
			break;
		}
	}
	return i - b;
}

std::vector<uint32_t> a2_nib_format::generate_levels_from_nibbles(const std::vector<uint8_t>& nibbles)
{
	std::vector<uint32_t> levels;
	const auto append_FFs = [&] (size_t count) {
		while (count-- > 0) {
			raw_w(levels, 8, 0xFF);
		}
	};
	const auto append_syncs = [&] (size_t count) {
		while (count-- > 0) {
			raw_w(levels, 10, 0x00FF << 2);
		}
	};
	const auto append_byte = [&] (uint8_t byte) {
		raw_w(levels, 8, byte);
	};


	const auto leading_FF_count =
		count_leading_FFs(nibbles.begin(), nibbles.end());

	if (leading_FF_count >= nibbles.size()) { // all are 0xFF !?!?
		assert(leading_FF_count >= min_sync_bytes);
		append_syncs(leading_FF_count);
		return levels;
	}

	const auto trailing_padding_size = count_trailing_padding(nibbles);
	const auto trailing_FF_count =
		count_leading_FFs(nibbles.rbegin() + trailing_padding_size,
						  nibbles.rend());
	const auto wrapped_FF_count = leading_FF_count + trailing_FF_count;
	const bool wrapped_FF_are_syncs = wrapped_FF_count >= min_sync_bytes;

	if (wrapped_FF_are_syncs) {
		append_syncs(leading_FF_count);
	} else {
		append_FFs(leading_FF_count);
	}

	{
		size_t FF_count = 0;
		const auto flush_FFs = [&] {
			if (FF_count == 0) {
				return;
			}

			if (FF_count >= a2_nib_format::min_sync_bytes) {
				append_syncs(FF_count);
			} else {
				append_FFs(FF_count);
			}
			FF_count = 0;
		};

		const auto end = nibbles.end() - trailing_padding_size - trailing_FF_count;
		for (auto i = nibbles.begin() + leading_FF_count; i != end; ++i) {
			const auto nibble = *i;
			if ((nibble & 0x80) == 0) {
				continue;
			}

			if (nibble == 0xFF) {
				++FF_count;
				continue;
			}

			flush_FFs();
			append_byte(nibble);
		}
		flush_FFs();
	}

	if (wrapped_FF_are_syncs) {
		append_syncs(trailing_FF_count);
	} else {
		append_FFs(trailing_FF_count);
	}

	return levels;
}

bool a2_nib_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const
{
	uint64_t size;
	if (io.length(size))
		return false;
	if (size != expected_size_35t && size != expected_size_40t)
		return false;

	const auto nr_tracks = size == expected_size_35t? 35 : 40;

	std::vector<uint8_t> nibbles(nibbles_per_track);
	for (unsigned track = 0; track < nr_tracks; ++track) {
		size_t actual;
		io.read_at(track * nibbles_per_track, &nibbles[0], nibbles_per_track, actual);
		auto levels = generate_levels_from_nibbles(nibbles);
		generate_track_from_levels(track, 0,
								   levels,
								   0, image);
	}

	image->set_form_variant(floppy_image::FF_525, floppy_image::SSSD);

	return true;
}


const a2_nib_format FLOPPY_NIB_FORMAT;
