/*********************************************************************

    formats/cqm_dsk.c

    CopyQM disk images

*********************************************************************/

#include <string.h>
#include "emu.h"
#include "imagedev/flopimg.h"
#include "imagedev/flopdrv.h"

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

	UINT8* buf;
	UINT64 track_offsets[84*2]; /* offset within data for each track */
};


static struct cqmdsk_tag *get_tag(floppy_image *floppy)
{
	struct cqmdsk_tag *tag;
	tag = (cqmdsk_tag *)floppy_tag(floppy );
	return tag;
}



FLOPPY_IDENTIFY( cqm_dsk_identify )
{
	UINT8 header[2];

	floppy_image_read(floppy, header, 0, 2);
	if (header[0]=='C' && header[1]=='Q') {
		*vote = 100;
	} else {
		*vote = 0;
	}
	return FLOPPY_ERROR_SUCCESS;
}

static int cqm_get_heads_per_disk(floppy_image *floppy)
{
	return get_tag(floppy)->heads;
}

static int cqm_get_tracks_per_disk(floppy_image *floppy)
{
	return get_tag(floppy)->tracks;
}

static UINT64 cqm_get_track_offset(floppy_image *floppy, int head, int track)
{
	return get_tag(floppy)->track_offsets[(track<<1) + head];
}

static floperr_t get_offset(floppy_image *floppy, int head, int track, int sector, int sector_is_index, UINT64 *offset)
{
	UINT64 pos = 0;
	UINT8 data;
	INT16 len;
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



static floperr_t internal_cqm_read_sector(floppy_image *floppy, int head, int track, int sector, int sector_is_index, void *buffer, size_t buflen)
{
	UINT64 offset;
	floperr_t err;

	// take sector offset
	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;
	memcpy(buffer,get_tag(floppy)->buf+offset,buflen);
	return FLOPPY_ERROR_SUCCESS;
}


static floperr_t cqm_read_sector(floppy_image *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_cqm_read_sector(floppy, head, track, sector, FALSE, buffer, buflen);
}

static floperr_t cqm_read_indexed_sector(floppy_image *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_cqm_read_sector(floppy, head, track, sector, TRUE, buffer, buflen);
}

static floperr_t cqm_get_sector_length(floppy_image *floppy, int head, int track, int sector, UINT32 *sector_length)
{
	floperr_t err;
	err = get_offset(floppy, head, track, sector, FALSE, NULL);
	if (err)
		return err;

	if (sector_length) {
		*sector_length = get_tag(floppy)->sector_size;
	}
	return FLOPPY_ERROR_SUCCESS;
}

static floperr_t cqm_get_indexed_sector_info(floppy_image *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, UINT32 *sector_length, unsigned long *flags)
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
	UINT8 header[CQM_HEADER_SIZE];
	UINT64 pos = 0;
	INT16 len;
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

	tag->sector_size      = (header[0x04] << 8) + header[0x03];
	tag->sector_per_track = header[0x10];
	tag->heads			  = header[0x12];
	tag->tracks			  = header[0x5b];
	tag->sector_base	  = header[0x71] + 1;
	tag->interleave		  = header[0x74];
	tag->skew			  = header[0x75];

	// header + comment size to position on first data block

	pos = CQM_HEADER_SIZE + (header[0x70] << 8) + header[0x6f];
	track = 0;
	head = 0;
	tag->buf = (UINT8*)malloc(tag->sector_size*tag->sector_per_track);
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

