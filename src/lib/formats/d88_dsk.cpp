// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
 *   D77 and D88 disk images
 *
 *
 *   Header (total size = 0x2b0 bytes):
 *     0x00 - Disk label
 *     0x1a - Write protect (bit 3)
 *     0x1b - 2D/2DD format (bit 3)
 *     0x1c-1f - image size (should match file size)
 *     0x20 - offsets for each track (max 164)
 *
 *   Sectors (0x110 bytes each, typically)
 *     0x00 - Sector info
 *          byte 0 - track number
 *          byte 1 - side (0 or 1)
 *          byte 2 - sector number
 *     0x10 - sector data
 *
 *   Images can be concatenated together.
 *   Sectors can be in any order.
 *   Tracks are in the order:
 *          Track 0 side 0
 *          Track 0 side 1
 *          Track 1 side 0
 *          ...
 *
 *
 */

#include "flopimg_legacy.h"
#include "imageutl.h"

#include "ioprocs.h"
#include "multibyte.h"

#include <tuple>


#define D88_HEADER_LEN 0x2b0

#define SPOT_DUPLICATES 0

struct d88_tag
{
	uint32_t image_size;
	uint32_t trackoffset[164];
	uint8_t write_protect;
	uint8_t disk_type;
	uint8_t heads;
};

static struct d88_tag *get_d88_tag(floppy_image_legacy *floppy)
{
	return (d88_tag *)floppy_tag(floppy);
}

static int d88_get_sector_id(floppy_image_legacy *floppy, int head, int track, int sector_index)
{
	struct d88_tag* tag = get_d88_tag(floppy);
	uint32_t offset;
	uint8_t sector_hdr[16];
	int x;

	offset = tag->trackoffset[(track*tag->heads)+head];

	if(offset == 0)
		return 0;

	floppy_image_read(floppy,sector_hdr,offset,16);

	// get to sector indexed
	x=0;
	while(x<sector_index)
	{
		offset += get_u16le(&sector_hdr[14]);
		offset += 16;
		floppy_image_read(floppy,sector_hdr,offset,16);
		x++;
	}

	return sector_hdr[2];
}

static int d88_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return 82;  // 82 tracks per side
}

static int d88_get_heads_per_disk(floppy_image_legacy *floppy)
{
	struct d88_tag* tag = get_d88_tag(floppy);
	return tag->heads;
}

static int d88_get_sectors_per_track(floppy_image_legacy *floppy, int head, int track)
{
	struct d88_tag* tag = get_d88_tag(floppy);
	uint32_t offset;
	uint8_t sector_hdr[16];

	offset = tag->trackoffset[(track*tag->heads)+head];

	floppy_image_read(floppy,sector_hdr,offset,16);

	return sector_hdr[4];
}

static floperr_t d88_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, uint32_t *sector_length)
{
	struct d88_tag* tag = get_d88_tag(floppy);
	uint32_t offset;
	uint8_t sector_hdr[16];
	uint32_t len;
	int count,secs;

	offset = tag->trackoffset[(track*tag->heads)+head];

	floppy_image_read(floppy,sector_hdr,offset,16);
	secs = sector_hdr[4];

	for(count=0;count<secs;count++)
	{
		floppy_image_read(floppy,sector_hdr,offset,16);
		if(sector == sector_hdr[2])
		{
			if(sector_length)
				*sector_length = get_u16le(&sector_hdr[14]);
			return FLOPPY_ERROR_SUCCESS;
		}
		len = get_u16le(&sector_hdr[14]);
		len += 16;
		offset += len;
	}

	return FLOPPY_ERROR_SEEKERROR;
}

static floperr_t d88_read_track(floppy_image_legacy *floppy, int head, int track, uint64_t offset, void *buffer, size_t buflen)
{
//  floperr_t err;

	return FLOPPY_ERROR_UNSUPPORTED;
}

static uint32_t d88_get_sector_offset(floppy_image_legacy* floppy, int head, int track, int sector)
{
	struct d88_tag* tag = get_d88_tag(floppy);
	uint32_t offset = 0;
	uint8_t sector_hdr[16];
	uint32_t len;
	uint32_t secs;
	int count;

	// get offset of the beginning of the track
	offset = tag->trackoffset[(track*tag->heads)+head];

	floppy_image_read(floppy,sector_hdr,offset,16);
	secs = sector_hdr[4];

	for(count=0;count<secs;count++)
	{
		floppy_image_read(floppy,sector_hdr,offset,16);
		if(sector == sector_hdr[2])
		{
			LOG_FORMATS("d88_get_sector_offset - track %i, side %i, sector %02x, returns %08x\n",track,head,sector,offset+16);
			return offset + 16;
		}
		len = get_u16le(&sector_hdr[14]);
		len += 16;
		offset += len;
	}
	LOG_FORMATS("d88_get_sector_offset - track %i, side %i, sector %02x, not found\n",track,head,sector);
	return 0;
}

static floperr_t d88_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, uint32_t *sector_length, unsigned long *flags)
{
	struct d88_tag* tag = get_d88_tag(floppy);
	uint32_t offset;
	uint8_t sector_hdr[16];
	int x;

	offset = tag->trackoffset[(track*tag->heads)+head];

	if(offset == 0)
		return FLOPPY_ERROR_SEEKERROR;


	floppy_image_read(floppy,sector_hdr,offset,16);

	if(sector_index >= sector_hdr[4])
		return FLOPPY_ERROR_SEEKERROR;

	// get to sector indexed
	x=0;
	while(x<sector_index)
	{
		offset += get_u16le(&sector_hdr[14]);
		offset += 16;
		floppy_image_read(floppy,sector_hdr,offset,16);
		x++;
	}

	if(offset > tag->image_size || offset == 0)
		return FLOPPY_ERROR_SEEKERROR;

	if(sector_length)
		*sector_length = get_u16le(&sector_hdr[14]);
	if(cylinder)
		*cylinder = sector_hdr[0];
	if(side)
		*side = sector_hdr[1];
	if(sector)
		*sector = sector_hdr[2];
	if(flags)
		*flags = 0;

	return FLOPPY_ERROR_SUCCESS;
}

static floperr_t d88_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	uint64_t offset;
	uint32_t sector_length;

	offset = d88_get_sector_offset(floppy,head,track,sector);

	if(d88_get_sector_length(floppy,head,track,sector,&sector_length) != FLOPPY_ERROR_SUCCESS)
		return FLOPPY_ERROR_SEEKERROR;

	if(offset == 0)
		return FLOPPY_ERROR_SEEKERROR;

	if(buflen > sector_length)
		return FLOPPY_ERROR_INTERNAL;

	floppy_image_read(floppy,buffer,offset,sector_length);

	return FLOPPY_ERROR_SUCCESS;
}

static floperr_t d88_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buffer_len)
{
	int sec;

	sec = d88_get_sector_id(floppy,head,track,sector);
	return d88_read_sector(floppy,head,track,sec,buffer,buffer_len);
}

static floperr_t d88_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	uint64_t offset;
	uint32_t sector_length;

	offset = d88_get_sector_offset(floppy,head,track,sector);

	if(d88_get_sector_length(floppy,head,track,sector,&sector_length) != FLOPPY_ERROR_SUCCESS)
		return FLOPPY_ERROR_SEEKERROR;

	if(offset == 0)
		return FLOPPY_ERROR_SEEKERROR;

	if(buflen > sector_length)
		return FLOPPY_ERROR_INTERNAL;

	floppy_image_write(floppy,buffer,offset,sector_length);

	return FLOPPY_ERROR_SUCCESS;
}

static floperr_t d88_write_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	int sec;

	sec = d88_get_sector_id(floppy,head,track,sector);
	return d88_write_sector(floppy,head,track,sec,buffer,buflen,ddam);
}

static void d88_get_header(floppy_image_legacy* floppy,uint32_t* size, uint8_t* prot, uint8_t* type, uint32_t* offsets)
{
	uint8_t header[D88_HEADER_LEN];

	floppy_image_read(floppy,header,0,D88_HEADER_LEN);

	if(SPOT_DUPLICATES)
	{
		// there exist many .d88 files with same data and different headers and
		// this allows to spot duplicates, making easier to debug softlists.
		uint32_t temp_size = floppy_image_size(floppy);
		auto tmp_copy = std::make_unique<uint8_t[]>(temp_size - D88_HEADER_LEN);
		floppy_image_read(floppy,tmp_copy.get(),D88_HEADER_LEN,temp_size - D88_HEADER_LEN);
		printf("CRC16: %d\n", ccitt_crc16(0xffff, tmp_copy.get(), temp_size - D88_HEADER_LEN));
	}

	if(prot)
		*prot = header[0x1a];
	if(type)
		*type = header[0x1b];
	if(size)
		*size = get_u32le(&header[0x1c]);
	if(offsets)
	{
		for(int x=0;x<164;x++)
			*(offsets+x) = get_u32le(&header[0x20 + (x*4)]);
	}
}

FLOPPY_IDENTIFY(d88_dsk_identify)
{
	uint32_t size;

	d88_get_header(floppy,&size,nullptr,nullptr,nullptr);

	if(floppy_image_size(floppy) == size)
	{
		*vote = 100;
	}
	else
	{
		*vote = 0;
	}
	return FLOPPY_ERROR_SUCCESS;
}

FLOPPY_CONSTRUCT(d88_dsk_construct)
{
	struct FloppyCallbacks *callbacks;
	struct d88_tag *tag;
	uint32_t size;
	uint8_t prot,type = 0;
	uint32_t offs[164];
	int x;

	if(params)
	{
		// create
		return FLOPPY_ERROR_UNSUPPORTED;
	}
	else
	{
		// load
		d88_get_header(floppy,&size,&prot,&type,offs);
	}

	tag = (d88_tag *)floppy_create_tag(floppy,sizeof(struct d88_tag));
	if (!tag)
		return FLOPPY_ERROR_OUTOFMEMORY;

	tag->write_protect = prot;
	tag->disk_type = type;
	tag->heads = 2;
	if (tag->disk_type==0x30 || tag->disk_type==0x40) tag->heads = 1;

	tag->image_size = size;
	for(x=0;x<164;x++)
		tag->trackoffset[x] = offs[x];

	callbacks = floppy_callbacks(floppy);
	callbacks->read_track = d88_read_track;
	callbacks->get_heads_per_disk = d88_get_heads_per_disk;
	callbacks->get_tracks_per_disk = d88_get_tracks_per_disk;
	callbacks->get_sector_length = d88_get_sector_length;
	callbacks->read_sector = d88_read_sector;
	callbacks->read_indexed_sector = d88_read_indexed_sector;
	callbacks->write_sector = d88_write_sector;
	callbacks->write_indexed_sector = d88_write_indexed_sector;
	callbacks->get_indexed_sector_info = d88_get_indexed_sector_info;
	callbacks->get_sectors_per_track = d88_get_sectors_per_track;


	return FLOPPY_ERROR_SUCCESS;
}



// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/d88_dsk.cpp

    D88 disk images

*********************************************************************/

#include "d88_dsk.h"

d88_format::d88_format()
{
}

const char *d88_format::name() const noexcept
{
	return "d88";
}

const char *d88_format::description() const noexcept
{
	return "D88 disk image";
}

const char *d88_format::extensions() const noexcept
{
	return "d77,d88,1dd";
}

int d88_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if(io.length(size))
		return 0;

	uint8_t h[32];
	auto const [err, actual] = read_at(io, 0, h, 32);
	if(err || (32 != actual))
		return 0;

	if(((get_u32le(h+0x1c) == size) || (get_u32le(h+0x1c) == (size >> 1))) &&
		(h[0x1b] == 0x00 || h[0x1b] == 0x10 || h[0x1b] == 0x20 || h[0x1b] == 0x30 || h[0x1b] == 0x40))
		return FIFID_SIZE|FIFID_STRUCT;

	return 0;
}

bool d88_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	std::error_condition err;
	size_t actual;

	uint8_t h[32];
	std::tie(err, actual) = read_at(io, 0, h, 32);
	if(err || (32 != actual))
		return false;

	int cell_count = 0;
	int track_count = 0;
	int head_count = 0;
	switch(h[0x1b]) {
	case 0x00:
		cell_count = 100000;
		track_count = 42;
		head_count = 2;
		image.set_variant(floppy_image::DSDD);
		break;

	case 0x10:
		cell_count = 100000;
		track_count = 82;
		head_count = 2;
		image.set_variant(floppy_image::DSQD);
		break;

	case 0x20:
		cell_count = form_factor == floppy_image::FF_35 ? 200000 : 166666;
		track_count = 82;
		head_count = 2;
		image.set_variant(floppy_image::DSHD);
		break;

	case 0x30:
		cell_count = 100000;
		track_count = 42;
		head_count = 1;
		image.set_variant(floppy_image::SSDD);
		break;

	case 0x40:
		cell_count = 100000;
		track_count = 82;
		head_count = 1;
		image.set_variant(floppy_image::SSQD);
		break;
	}

	if(!head_count)
		return false;

	int img_tracks, img_heads;
	image.get_maximal_geometry(img_tracks, img_heads);
	if (track_count > img_tracks)
		osd_printf_warning("d88: Floppy disk has too many tracks for this drive (floppy tracks=%d, drive tracks=%d).\n", track_count, img_tracks);

	if (head_count > img_heads)
		osd_printf_warning("d88: Floppy disk has excess of heads for this drive that will be discarded (floppy heads=%d, drive heads=%d).\n", head_count, img_heads);

	uint32_t track_pos[164];
	std::tie(err, actual) = read_at(io, 32, track_pos, 164*4); // FIXME: check for errors and premature EOF

	uint64_t file_size;
	if(io.length(file_size))
		return false;

	for(int track=0; track < track_count; track++)
		for(int head=0; head < head_count; head++) {
			int pos = little_endianize_int32(track_pos[track * head_count + head]);
			if(!pos)
				continue;
			desc_pc_sector sects[256];
			uint8_t sect_data[65536];
			int sdatapos = 0;
			int sector_count = 1;
			uint8_t density = 0;
			for(int i=0; i<sector_count; i++) {

				if (pos + 16 > file_size)
					return true;

				uint8_t hs[16];
				std::tie(err, actual) = read_at(io, pos, hs, 16); // FIXME: check for errors and premature EOF
				pos += 16;

				uint16_t size = little_endianize_int16(*(uint16_t *)(hs+14));

				if(pos + size > file_size)
					return true;

				if(i == 0) {
					sector_count = little_endianize_int16(*(uint16_t *)(hs+4));
					// Support broken vfman converter
					if(sector_count == 0x1000)
						sector_count = 0x10;

					density = hs[6];
				}

				sects[i].track       = hs[0];
				sects[i].head        = hs[1];
				sects[i].sector      = hs[2];
				sects[i].size        = hs[3];
				sects[i].actual_size = size;
				sects[i].deleted     = hs[7] != 0;
				sects[i].bad_crc     = hs[8] == 0xb0;  // according to hxc

				if(size) {
					sects[i].data    = sect_data + sdatapos;
					std::tie(err, actual) = read_at(io, pos, sects[i].data, size); // FIXME: check for errors and premature EOF
					pos += size;
					sdatapos += size;

				} else
					sects[i].data    = nullptr;
			}

			if(head < img_heads) {
				if(density == 0x40)
					build_pc_track_fm(track, head, image, cell_count / 2, sector_count, sects, calc_default_pc_gap3_size(form_factor, sects[0].actual_size));
				else
					build_pc_track_mfm(track, head, image, cell_count, sector_count, sects, calc_default_pc_gap3_size(form_factor, sects[0].actual_size));
			}
		}

	return true;
}

bool d88_format::supports_save() const noexcept
{
	return false;
}

const d88_format FLOPPY_D88_FORMAT;
