// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/dsk_dsk.c

    DSK disk images

*********************************************************************/

#include "dsk_dsk.h"
#include "imageutl.h"

#include "ioprocs.h"

#include <cstring>

#define MV_CPC      "MV - CPC"
#define EXTENDED    "EXTENDED"

struct dskdsk_tag
{
	int disk_image_type;  /* image type: standard or extended */
	int heads;
	int tracks;
	int sector_size;
	uint64_t track_offsets[84*2]; /* offset within data for each track */

};


static struct dskdsk_tag *get_tag(floppy_image_legacy *floppy)
{
	struct dskdsk_tag *tag;
	tag = (dskdsk_tag *)floppy_tag(floppy);
	return tag;
}



FLOPPY_IDENTIFY( dsk_dsk_identify )
{
	uint8_t header[8];

	floppy_image_read(floppy, header, 0, 8);
	if ( memcmp( header, MV_CPC, 8 ) ==0) {
		*vote = 100;
	} else
	if ( memcmp( header, EXTENDED, 8 ) ==0) {
		*vote = 100;
	} else {
		*vote = 0;
	}
	return FLOPPY_ERROR_SUCCESS;
}

static int dsk_get_heads_per_disk(floppy_image_legacy *floppy)
{
	return get_tag(floppy)->heads;
}

static int dsk_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return get_tag(floppy)->tracks;
}

static uint64_t dsk_get_track_offset(floppy_image_legacy *floppy, int head, int track)
{
	return get_tag(floppy)->track_offsets[(track<<1) + head];
}

static floperr_t get_offset(floppy_image_legacy *floppy, int head, int track, int sector, bool sector_is_index, uint64_t *offset)
{
	uint64_t offs;
	uint64_t track_offset;
	uint8_t track_info[0x100];
	uint8_t sectors_per_track;
	int i;
	/* translate the sector to a raw sector */

	/* check to see if we are out of range */
	if ((head < 0) || (head >= get_tag(floppy)->heads) || (track < 0) || (track >= get_tag(floppy)->tracks)
			|| (sector < 0) )
		return FLOPPY_ERROR_SEEKERROR;

	track_offset = dsk_get_track_offset(floppy, head, track);

	floppy_image_read(floppy, track_info, track_offset, 0x100);

	sectors_per_track = track_info[0x015];
	if (!sector_is_index) {
		if (sector >= sectors_per_track) {
			return FLOPPY_ERROR_SEEKERROR;
		}
	}

	if (get_tag(floppy)->disk_image_type==0) {
		get_tag(floppy)->sector_size = (1<<(track_info[0x014]+7));
		offs = track_offset + 0x100 +sector * get_tag(floppy)->sector_size;
	} else {
		get_tag(floppy)->sector_size = track_info[0x18 + (sector<<3) + 6] + (track_info[0x18+(sector<<3) + 7]<<8);
		offs = track_offset + 0x100;
		for (i=0;i<sector;i++) {
			offs += track_info[0x18 + (i<<3) + 6] + (track_info[0x18+(i<<3) + 7]<<8);
		}
	}

	if (offset)
		*offset = offs;
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_dsk_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, bool sector_is_index, void *buffer, size_t buflen)
{
	uint64_t offset;
	floperr_t err;
	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;
	floppy_image_read(floppy, buffer, offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_dsk_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, bool sector_is_index, const void *buffer, size_t buflen, int ddam)
{
	uint64_t offset;
	floperr_t err;

	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;

	floppy_image_write(floppy, buffer, offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t dsk_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_dsk_read_sector(floppy, head, track, sector, false, buffer, buflen);
}

static floperr_t dsk_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_dsk_write_sector(floppy, head, track, sector, false, buffer, buflen, ddam);
}

static floperr_t dsk_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_dsk_read_sector(floppy, head, track, sector, true, buffer, buflen);
}

static floperr_t dsk_write_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_dsk_write_sector(floppy, head, track, sector, true, buffer, buflen, ddam);
}

static floperr_t dsk_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, uint32_t *sector_length)
{
	floperr_t err;
	err = get_offset(floppy, head, track, sector, false, nullptr);
	if (err)
		return err;

	if (sector_length) {
		*sector_length = get_tag(floppy)->sector_size;
	}
	return FLOPPY_ERROR_SUCCESS;
}

static floperr_t dsk_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, uint32_t *sector_length, unsigned long *flags)
{
	floperr_t retVal;
	uint64_t offset;
	uint8_t sector_info[0x100];
	int pos;

	retVal = get_offset(floppy, head, track, sector_index, false, nullptr);
	offset = dsk_get_track_offset(floppy,head,track);
	pos = 0x18 + (sector_index << 3);
	floppy_image_read(floppy, sector_info, offset, 0x100);
	if (cylinder)
		*cylinder = sector_info[pos + 0];
	if (side)
		*side = sector_info[pos + 1];
	if (sector)
		*sector = sector_info[pos + 2];
	if (sector_length) {
		*sector_length = 1 << (sector_info[pos + 3] + 7);
	}
	if (flags) {
		*flags = 0;
		if (sector_info[pos + 4] & 0x20) *flags |= ID_FLAG_CRC_ERROR_IN_ID_FIELD;
		if (sector_info[pos + 5] & 0x20) *flags |= ID_FLAG_CRC_ERROR_IN_DATA_FIELD;
		if (sector_info[pos + 5] & 0x40) *flags |= ID_FLAG_DELETED_DATA;
	}
	return retVal;
}


FLOPPY_CONSTRUCT( dsk_dsk_construct )
{
	struct FloppyCallbacks *callbacks;
	struct dskdsk_tag *tag;
	uint8_t header[0x100];
	uint64_t tmp = 0;
	int i;
	int skip,cnt;

	if(params)
	{
		// create
		return FLOPPY_ERROR_UNSUPPORTED;
	}

	floppy_image_read(floppy, header, 0, 0x100);
#ifdef SPOT_DUPLICATES
	// this allow to spot .dsk files with same data and different headers, making easier to debug softlists.
	uint32_t temp_size = floppy_image_size(floppy);
	uint8_t tmp_copy[temp_size - 0x100];
	floppy_image_read(floppy,tmp_copy,0x100,temp_size - 0x100);
	printf("CRC16: %d\n", ccitt_crc16(0xffff, tmp_copy, temp_size - 0x100));
#endif

	tag = (struct dskdsk_tag *) floppy_create_tag(floppy, sizeof(struct dskdsk_tag));
	if (!tag)
		return FLOPPY_ERROR_OUTOFMEMORY;

	tag->heads   = header[0x31];
	if (tag->heads==1) {
		skip = 2;
	} else {
		skip = 1;
	}
	tag->tracks  = header[0x30];
	cnt =0;
	if ( memcmp( header, MV_CPC, 8 ) ==0) {
		tag->disk_image_type = 0;
		tmp = 0x100;
		for (i=0; i<tag->tracks * tag->heads; i++)
		{
			tag->track_offsets[cnt] = tmp;
			tmp += pick_integer_le(header, 0x32, 2);
			cnt += skip;
		}
	} else  {
		tag->disk_image_type = 1;
		tmp = 0x100;
		for (i=0; i<tag->tracks * tag->heads; i++)
		{
			tag->track_offsets[cnt] = tmp;
			tmp += header[0x34 + i] << 8;
			cnt += skip;
		}
	}

	callbacks = floppy_callbacks(floppy);
	callbacks->read_sector = dsk_read_sector;
	callbacks->write_sector = dsk_write_sector;
	callbacks->read_indexed_sector = dsk_read_indexed_sector;
	callbacks->write_indexed_sector = dsk_write_indexed_sector;
	callbacks->get_sector_length = dsk_get_sector_length;
	callbacks->get_heads_per_disk = dsk_get_heads_per_disk;
	callbacks->get_tracks_per_disk = dsk_get_tracks_per_disk;
	callbacks->get_indexed_sector_info = dsk_get_indexed_sector_info;
	return FLOPPY_ERROR_SUCCESS;
}




#define DSK_FORMAT_HEADER   "MV - CPC"
#define EXT_FORMAT_HEADER   "EXTENDED CPC DSK"

dsk_format::dsk_format() : floppy_image_format_t()
{
}

const char *dsk_format::name() const
{
	return "dsk";
}

const char *dsk_format::description() const
{
	return "CPC DSK Format";
}

const char *dsk_format::extensions() const
{
	return "dsk";
}

bool dsk_format::supports_save() const
{
	return false;
}

int dsk_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	uint8_t header[16];

	size_t actual;
	io.read_at(0, &header, sizeof(header), actual);
	if ( memcmp( header, DSK_FORMAT_HEADER, 8 ) ==0) {
		return 100;
	}
	if ( memcmp( header, EXT_FORMAT_HEADER, 16 ) ==0) {
		return 100;
	}
	return 0;
}


#pragma pack(1)

struct track_header
{
	uint8_t headertag[13];
	uint16_t unused1;
	uint8_t unused1b;
	uint8_t track_number;
	uint8_t side_number;
	uint8_t datarate;
	uint8_t rec_mode;
	uint8_t sector_size_code;
	uint8_t number_of_sector;
	uint8_t gap3_length;
	uint8_t filler_byte;
};

struct sector_header
{
	uint8_t   track;
	uint8_t   side;
	uint8_t   sector_id;
	uint8_t   sector_size_code;
	uint8_t   fdc_status_reg1;
	uint8_t   fdc_status_reg2;
	uint16_t  data_length;
};

#pragma pack()

bool dsk_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image)
{
	size_t actual;

	uint8_t header[0x100];
	bool extendformat = false;

	uint64_t image_size;
	if (io.length(image_size))
		return false;

	io.read_at(0, &header, sizeof(header), actual);
	if ( memcmp( header, EXT_FORMAT_HEADER, 16 ) ==0) {
		extendformat = true;
	}

	int heads = header[0x31];
	int skip = 1;
	if (heads==1) {
		skip = 2;
	}
	int tracks  = header[0x30];

	int img_tracks, img_heads;
	image->get_maximal_geometry(img_tracks, img_heads);
	if (tracks > img_tracks)
	{
		if (tracks - img_tracks > DUMP_THRESHOLD)
		{
			osd_printf_error("dsk: Floppy disk has too many tracks for this drive (floppy tracks=%d, drive tracks=%d).\n", tracks, img_tracks);
			return false;
		}
		else
		{
			// Some dumps has a few excess tracks to be safe,
			// lets be nice and just skip those tracks
			osd_printf_warning("dsk: Floppy disk has a slight excess of tracks for this drive that will be discarded (floppy tracks=%d, drive tracks=%d).\n", tracks, img_tracks);
			tracks = img_tracks;
		}
	}
	if (heads > img_heads)
		osd_printf_warning("dsk: Floppy disk has excess of heads for this drive that will be discarded (floppy heads=%d, drive heads=%d).\n", heads, img_heads);

	uint64_t track_offsets[84*2];
	int cnt =0;
	if (!extendformat) {
		int tmp = 0x100;
		for (int i=0; i<tracks * heads; i++)
		{
			track_offsets[cnt] = tmp;
			tmp += pick_integer_le(header, 0x32, 2);
			cnt += skip;
		}
	} else  {
		int tmp = 0x100;
		for (int i=0; i<tracks * heads; i++)
		{
			int length = header[0x34 + i] << 8;
			if (length != 0)
			{
				track_offsets[cnt] = tmp;
				tmp += length;
			}
			else
			{
				track_offsets[cnt] = image_size;
			}

			cnt += skip;
		}
	}

	int counter = 0;
	for(int track=0; track < tracks; track++) {
		for(int side=0; side < std::min(heads, img_heads); side++) {
			if(track_offsets[(track<<1)+side] >= image_size)
				continue;
			track_header tr;
			io.read_at(track_offsets[(track<<1)+side], &tr, sizeof(tr), actual);

			// skip if there are no sectors in this track
			if (tr.number_of_sector == 0)
				continue;

			int protection = 0;
			int first_sector_code = -1;
			for(int j=0;j<tr.number_of_sector;j++) {
				sector_header sector;
				io.read_at(track_offsets[(track<<1)+side]+sizeof(tr)+(sizeof(sector)*j), &sector, sizeof(sector), actual);

				if (j == 0)
					first_sector_code = sector.sector_size_code;

				if ((j > 0) && (sector.sector_size_code == 2) && (first_sector_code == 6))
					protection = 1;  //  first: 6144 rest: 512

				if ((j > 0) && (sector.sector_size_code == j) && (first_sector_code == 0))
					protection = 2;  // first: 128 rest: N*128
			}

			desc_pc_sector sects[256];
			uint8_t sect_data[65536];
			int sdatapos = 0;
			int pos = track_offsets[(track<<1)+side] + 0x100;

			for(int j=0;j<tr.number_of_sector;j++) {
				sector_header sector;
				io.read_at(track_offsets[(track<<1)+side]+sizeof(tr)+(sizeof(sector)*j), &sector, sizeof(sector), actual);

				sects[j].track       = sector.track;
				sects[j].head        = sector.side;
				sects[j].sector      = sector.sector_id;
				sects[j].size        = sector.sector_size_code;
				if(extendformat)
				{
					if (protection == 1)
						sects[j].actual_size = 512;
					else if (protection == 2)
						sects[j].actual_size = 128;
					else
						sects[j].actual_size = sector.data_length;
				}
				else
					sects[j].actual_size = 128 << tr.sector_size_code;

				sects[j].deleted = (sector.fdc_status_reg2 & 0x40);
				sects[j].bad_crc = ((sector.fdc_status_reg1 & 0x20) || (sector.fdc_status_reg2 & 0x20));

				if(!(sector.fdc_status_reg1 & 0x04)) {
					sects[j].data = sect_data + sdatapos;
					io.read_at(pos, sects[j].data, sects[j].actual_size, actual);
					sdatapos += sects[j].actual_size;

				} else
					sects[j].data = nullptr;

				if(extendformat)
					pos += sector.data_length;
				else
					pos += 128 << tr.sector_size_code;
			}
			// larger cell count (was 100000) to allow for slightly out of spec images (theatre europe on einstein)
			build_pc_track_mfm(track, side, image, 105000, tr.number_of_sector, sects, tr.gap3_length);
			counter++;
		}
	}
	return true;
}

const floppy_format_type FLOPPY_DSK_FORMAT = &floppy_image_format_creator<dsk_format>;
