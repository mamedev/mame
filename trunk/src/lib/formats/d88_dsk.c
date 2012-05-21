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

#include "flopimg.h"
#include "imageutl.h"

#define D88_HEADER_LEN 0x2b0

struct d88_tag
{
	UINT32 image_size;
	UINT32 trackoffset[164];
	UINT8 write_protect;
	UINT8 disk_type;
	UINT8 heads;
};

static struct d88_tag *get_d88_tag(floppy_image_legacy *floppy)
{
	return (d88_tag *)floppy_tag(floppy);
}

static int d88_get_sector_id(floppy_image_legacy *floppy, int head, int track, int sector_index)
{
	struct d88_tag* tag = get_d88_tag(floppy);
	UINT32 offset;
	UINT8 sector_hdr[16];
	int x;

	offset = tag->trackoffset[(track*tag->heads)+head];

	if(offset == 0)
		return 0;

	floppy_image_read(floppy,sector_hdr,offset,16);

	// get to sector indexed
	x=0;
	while(x<sector_index)
	{
		offset += ((sector_hdr[15] << 8) | sector_hdr[14]);
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
	UINT32 offset;
	UINT8 sector_hdr[16];

	offset = tag->trackoffset[(track*tag->heads)+head];

	floppy_image_read(floppy,sector_hdr,offset,16);

	return sector_hdr[4];
}

static floperr_t d88_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *sector_length)
{
	struct d88_tag* tag = get_d88_tag(floppy);
	UINT32 offset;
	UINT8 sector_hdr[16];
	UINT32 len;
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
				*sector_length = (sector_hdr[15] << 8) | sector_hdr[14];
			return FLOPPY_ERROR_SUCCESS;
		}
		len = (sector_hdr[15] << 8) | sector_hdr[14];
		len += 16;
		offset += len;
	}

	return FLOPPY_ERROR_SEEKERROR;
}

static floperr_t d88_read_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, void *buffer, size_t buflen)
{
//  floperr_t err;

	return FLOPPY_ERROR_UNSUPPORTED;
}

static UINT32 d88_get_sector_offset(floppy_image_legacy* floppy, int head, int track, int sector)
{
	struct d88_tag* tag = get_d88_tag(floppy);
	UINT32 offset = 0;
	UINT8 sector_hdr[16];
	UINT32 len;
	UINT32 secs;
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
		len = (sector_hdr[15] << 8) | sector_hdr[14];
		len += 16;
		offset += len;
	}
	LOG_FORMATS("d88_get_sector_offset - track %i, side %i, sector %02x, not found\n",track,head,sector);
	return 0;
}

static floperr_t d88_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, UINT32 *sector_length, unsigned long *flags)
{
	struct d88_tag* tag = get_d88_tag(floppy);
	UINT32 offset;
	UINT8 sector_hdr[16];
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
		offset += ((sector_hdr[15] << 8) | sector_hdr[14]);
		offset += 16;
		floppy_image_read(floppy,sector_hdr,offset,16);
		x++;
	}

	if(offset > tag->image_size || offset == 0)
		return FLOPPY_ERROR_SEEKERROR;

	if(sector_length)
		*sector_length = (sector_hdr[15] << 8) | sector_hdr[14];
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
	UINT64 offset;
	UINT32 sector_length;

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
	UINT64 offset;
	UINT32 sector_length;

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

static void d88_get_header(floppy_image_legacy* floppy,UINT32* size, UINT8* prot, UINT8* type, UINT32* offsets)
{
	UINT8 header[D88_HEADER_LEN];
	int x,s;

	floppy_image_read(floppy,header,0,D88_HEADER_LEN);

#ifdef SPOT_DUPLICATES
		// there exist many .d88 files with same data and different headers and
		// this allows to spot duplicates, making easier to debug softlists.
		UINT32 temp_size = floppy_image_size(floppy);
		UINT8 tmp_copy[temp_size - D88_HEADER_LEN];
		floppy_image_read(floppy,tmp_copy,D88_HEADER_LEN,temp_size - D88_HEADER_LEN);
		printf("CRC16: %d\n", ccitt_crc16(0xffff, tmp_copy, temp_size - D88_HEADER_LEN));
#endif

	if(prot)
		*prot = header[0x1a];
	if(type)
		*type = header[0x1b];
	if(size)
	{
		s = 0;
		s |= header[0x1f] << 24;
		s |= header[0x1e] << 16;
		s |= header[0x1d] << 8;
		s |= header[0x1c];
		*size = s;
	}
	if(offsets)
	{
		for(x=0;x<164;x++)
		{
			s = 0;
			s |= header[0x23 + (x*4)] << 24;
			s |= header[0x22 + (x*4)] << 16;
			s |= header[0x21 + (x*4)] << 8;
			s |= header[0x20 + (x*4)];
			*(offsets+x) = s;
		}
	}
}

FLOPPY_IDENTIFY(d88_dsk_identify)
{
	UINT32 size;

	d88_get_header(floppy,&size,NULL,NULL,NULL);

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
	UINT32 size;
	UINT8 prot,type = 0;
	UINT32 offs[164];
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
