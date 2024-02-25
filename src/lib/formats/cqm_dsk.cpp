// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/cqm_dsk.cpp

    CopyQM disk images

*********************************************************************/

#include "cqm_dsk.h"
#include "flopimg_legacy.h"

#include "ioprocs.h"
#include "multibyte.h"

#include <cstring>


#define CQM_HEADER_SIZE 133

struct cqmdsk_tag
{
	int heads;
	int tracks;
	int sector_size;
	int sector_per_track;
	int sector_base;
	int interleave;
	int skew;

	uint8_t* buf;
	uint64_t track_offsets[84*2]; /* offset within data for each track */
};


static struct cqmdsk_tag *get_tag(floppy_image_legacy *floppy)
{
	struct cqmdsk_tag *tag;
	tag = (cqmdsk_tag *)floppy_tag(floppy );
	return tag;
}



FLOPPY_IDENTIFY( cqm_dsk_identify )
{
	uint8_t header[2];

	floppy_image_read(floppy, header, 0, 2);
	if (header[0]=='C' && header[1]=='Q') {
		*vote = 100;
	} else {
		*vote = 0;
	}
	return FLOPPY_ERROR_SUCCESS;
}

static int cqm_get_heads_per_disk(floppy_image_legacy *floppy)
{
	return get_tag(floppy)->heads;
}

static int cqm_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return get_tag(floppy)->tracks;
}

static uint64_t cqm_get_track_offset(floppy_image_legacy *floppy, int head, int track)
{
	return get_tag(floppy)->track_offsets[(track<<1) + head];
}

static floperr_t get_offset(floppy_image_legacy *floppy, int head, int track, int sector, bool sector_is_index, uint64_t *offset)
{
	uint64_t pos = 0;
	uint8_t data;
	int16_t len;
	int s;

	if ((head < 0) || (head >= get_tag(floppy)->heads) || (track < 0) || (track >= get_tag(floppy)->tracks)
			|| (sector < 0) )
		return FLOPPY_ERROR_SEEKERROR;

	pos = cqm_get_track_offset(floppy,head,track);
	s = 0;
	do {
		floppy_image_read(floppy, &len, pos, 2);
		pos+=2;
		if(len<0) {
			floppy_image_read(floppy, &data, pos, 1);
			memset(get_tag(floppy)->buf + s,data, -len);
			pos++;
			s += -len;
		} else {
			floppy_image_read(floppy, get_tag(floppy)->buf + s, pos, len);
			pos+=len;
			s += len;
		}
	} while(s<get_tag(floppy)->sector_size*get_tag(floppy)->sector_per_track);

	if (offset)
		*offset = sector * get_tag(floppy)->sector_size;
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_cqm_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, bool sector_is_index, void *buffer, size_t buflen)
{
	uint64_t offset;
	floperr_t err;

	// take sector offset
	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;
	memcpy(buffer,get_tag(floppy)->buf+offset,buflen);
	return FLOPPY_ERROR_SUCCESS;
}


static floperr_t cqm_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_cqm_read_sector(floppy, head, track, sector, false, buffer, buflen);
}

static floperr_t cqm_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_cqm_read_sector(floppy, head, track, sector, true, buffer, buflen);
}

static floperr_t cqm_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, uint32_t *sector_length)
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

static floperr_t cqm_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, uint32_t *sector_length, unsigned long *flags)
{
	if (sector_index >= get_tag(floppy)->sector_per_track) return FLOPPY_ERROR_SEEKERROR;

	if (cylinder) {
		*cylinder = track;
	}
	if (side) {
		*side = head;
	}
	if (sector) {
		*sector = get_tag(floppy)->sector_base + sector_index;
	}
	if (sector_length) {
		*sector_length = get_tag(floppy)->sector_size;
	}
	if (flags)
		*flags = 0;
	return FLOPPY_ERROR_SUCCESS;
}

FLOPPY_CONSTRUCT( cqm_dsk_construct )
{
	struct FloppyCallbacks *callbacks;
	struct cqmdsk_tag *tag;
	uint8_t header[CQM_HEADER_SIZE];
	uint64_t pos = 0;
	int16_t len;
	int head;
	int track;
	int s;
	if(params)
	{
		// create
		return FLOPPY_ERROR_UNSUPPORTED;
	}

	tag = (struct cqmdsk_tag *) floppy_create_tag(floppy, sizeof(struct cqmdsk_tag));
	if (!tag)
		return FLOPPY_ERROR_OUTOFMEMORY;

	floppy_image_read(floppy, header, 0, CQM_HEADER_SIZE);

	tag->sector_size      = get_u16le(&header[0x03]);
	tag->sector_per_track = header[0x10];
	tag->heads            = header[0x12];
	tag->tracks           = header[0x5b];
	tag->sector_base      = header[0x71] + 1;
	tag->interleave       = header[0x74];
	tag->skew             = header[0x75];

	// header + comment size to position on first data block

	pos = CQM_HEADER_SIZE + (header[0x70] << 8) + header[0x6f];
	track = 0;
	head = 0;
	tag->buf = (uint8_t*)malloc(tag->sector_size*tag->sector_per_track);
	do {
		tag->track_offsets[(track<<1) + head] = pos;
		s = 0;
		do {
			floppy_image_read(floppy, &len, pos, 2);
			pos+=2;
			if(len<0) {
				pos++;
				s += -len;
			} else {
				pos+=len;
				s += len;
			}
		} while(s<tag->sector_size*tag->sector_per_track);
		if(head ==0 && tag->heads > 1) {
			head = 1;
		} else {
			head = 0;
			track++;
		}
	} while(pos < floppy_image_size(floppy));


	callbacks = floppy_callbacks(floppy);
	callbacks->read_sector = cqm_read_sector;
	callbacks->read_indexed_sector = cqm_read_indexed_sector;
	callbacks->get_sector_length = cqm_get_sector_length;
	callbacks->get_heads_per_disk = cqm_get_heads_per_disk;
	callbacks->get_tracks_per_disk = cqm_get_tracks_per_disk;
	callbacks->get_indexed_sector_info = cqm_get_indexed_sector_info;

	return FLOPPY_ERROR_SUCCESS;
}



cqm_format::cqm_format()
{
}

const char *cqm_format::name() const noexcept
{
	return "cqm";
}

const char *cqm_format::description() const noexcept
{
	return "CopyQM disk image";
}

const char *cqm_format::extensions() const noexcept
{
	return "cqm,cqi,dsk";
}

int cqm_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t h[3];
	auto const [err, actual] = read_at(io, 0, h, 3);
	if (err || (3 != actual))
		return 0;

	if (h[0] == 'C' && h[1] == 'Q' && h[2] == 0x14)
		return FIFID_SIGN;

	return 0;
}

bool cqm_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	const int max_size = 4*1024*1024; // 4MB ought to be large enough for any floppy
	std::vector<uint8_t> imagebuf(max_size);
	uint8_t header[CQM_HEADER_SIZE];
	read_at(io, 0, header, CQM_HEADER_SIZE); // FIXME: check for errors and premature EOF

	int sector_size      = get_u16le(&header[0x03]);
	int sector_per_track = get_u16le(&header[0x10]);
	int heads            = get_u16le(&header[0x12]);
	int tracks           = header[0x5b];
//  int blind            = header[0x58];    // 0=DOS, 1=blind, 2=HFS
	int density          = header[0x59];    // 0=DD, 1=HD, 2=ED
	int comment_size     = get_u16le(&header[0x6f]);
	int sector_base      = header[0x71] + 1;
//  int interleave       = header[0x74];    // TODO
//  int skew             = header[0x75];    // TODO
//  int drive            = header[0x76];    // source drive type: 1=5.25" 360KB, 2=5.25" 1.2MB, 3=3.5" 720KB, 4=3.5" 1.44MB, 6=3.5" 2.88MB, 8" is unknown (0 or 5?)

	switch(density)
	{
		case 0:
			if (form_factor == floppy_image::FF_525 && tracks > 50)
				image.set_variant(heads == 1 ? floppy_image::SSQD : floppy_image::DSQD);
			else
				image.set_variant(heads == 1 ? floppy_image::SSDD : floppy_image::DSDD);
			break;
		case 1:
			if (heads == 1)
				return false; // single side HD ?
			image.set_variant(floppy_image::DSHD);
			break;
		case 2:
			if (heads == 1)
				return false; // single side ED ?
			image.set_variant(floppy_image::DSED);
			break;
		default:
			return false;
	}

	static const int rates[3] = { 250000, 300000, 500000 };
	int rate = density >= 3 ? 500000 : rates[density];
	int rpm = form_factor == floppy_image::FF_8 || (form_factor == floppy_image::FF_525 && rate >= 300000) ? 360 : 300;
	int base_cell_count = rate*60/rpm;

	uint64_t cqm_size;
	if (io.length(cqm_size))
		return false;
	auto const [err, cqmbuf, actual] = read_at(io, 0, cqm_size);
	if (err || (actual != cqm_size))
		return false;

	// decode the RLE data
	for (int s = 0, pos = CQM_HEADER_SIZE + comment_size; pos < cqm_size; )
	{
		int16_t len = get_s16le(&cqmbuf[pos]);
		pos += 2;
		if(len < 0)
		{
			len = -len;
			memset(&imagebuf[s], cqmbuf[pos], len);
			pos++;
		}
		else
		{
			memcpy(&imagebuf[s], &cqmbuf[pos], len);
			pos += len;
		}

		s += len;
	}

	int ssize;
	for(ssize=0; (128 << ssize) < sector_size; ssize++)
		;

	desc_pc_sector sects[256];
	for(int track = 0, pos = 0; track < tracks; track++)
		for(int head = 0; head < heads; head++)
		{
			for(int sector = 0; sector < sector_per_track; sector++)
			{
				sects[sector].track       = track;
				sects[sector].head        = head;
				sects[sector].sector      = sector_base + sector;
				sects[sector].size        = ssize;
				sects[sector].deleted     = false;
				sects[sector].bad_crc     = false;
				sects[sector].actual_size = sector_size;
				sects[sector].data        = &imagebuf[pos];
				pos += sector_size;
			}

			build_pc_track_mfm(track, head, image, base_cell_count*2, sector_per_track, sects, calc_default_pc_gap3_size(form_factor, sector_size));
		}

	return true;
}

bool cqm_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	return false;
}

bool cqm_format::supports_save() const noexcept
{
	return false;
}

const cqm_format FLOPPY_CQM_FORMAT;
