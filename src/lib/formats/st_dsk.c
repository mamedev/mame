// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/st_dsk.c

    All usual Atari ST formats

*********************************************************************/

#include <assert.h>

#include "formats/st_dsk.h"

st_format::st_format()
{
}

const char *st_format::name() const
{
	return "st";
}

const char *st_format::description() const
{
	return "Atari ST floppy disk image";
}

const char *st_format::extensions() const
{
	return "st";
}

bool st_format::supports_save() const
{
	return true;
}

void st_format::find_size(io_generic *io, UINT8 &track_count, UINT8 &head_count, UINT8 &sector_count)
{
	UINT64 size = io_generic_size(io);
	for(track_count=80; track_count <= 82; track_count++)
		for(head_count=1; head_count <= 2; head_count++)
			for(sector_count=9; sector_count <= 11; sector_count++)
				if(size == (UINT32)512*track_count*head_count*sector_count)
					return;
	track_count = head_count = sector_count = 0;
}

int st_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT8 track_count, head_count, sector_count;
	find_size(io, track_count, head_count, sector_count);

	if(track_count)
		return 50;
	return 0;
}

bool st_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	UINT8 track_count, head_count, sector_count;
	find_size(io, track_count, head_count, sector_count);

	UINT8 sectdata[11*512];
	desc_s sectors[11];
	for(int i=0; i<sector_count; i++) {
		sectors[i].data = sectdata + 512*i;
		sectors[i].size = 512;
		sectors[i].sector_id = i + 1;
	}

	int track_size = sector_count*512;
	for(int track=0; track < track_count; track++) {
		for(int head=0; head < head_count; head++) {
			io_generic_read(io, sectdata, (track*head_count + head)*track_size, track_size);
			generate_track(atari_st_fcp_get_desc(track, head, head_count, sector_count),
							track, head, sectors, sector_count, 100000, image);
		}
	}

	image->set_variant(floppy_image::DSDD);

	return true;
}

bool st_format::save(io_generic *io, floppy_image *image)
{
	int track_count, head_count, sector_count;
	get_geometry_mfm_pc(image, 2000, track_count, head_count, sector_count);

	if(track_count < 80)
		track_count = 80;
	else if(track_count > 82)
		track_count = 82;

	// Happens for a fully unformatted floppy
	if(!head_count)
		head_count = 1;

	if(sector_count > 11)
		sector_count = 11;
	else if(sector_count < 9)
		sector_count = 9;

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


msa_format::msa_format()
{
}

const char *msa_format::name() const
{
	return "msa";
}

const char *msa_format::description() const
{
	return "Atari MSA floppy disk image";
}

const char *msa_format::extensions() const
{
	return "msa";
}

bool msa_format::supports_save() const
{
	return true;
}

void msa_format::read_header(io_generic *io, UINT16 &sign, UINT16 &sect, UINT16 &head, UINT16 &strack, UINT16 &etrack)
{
	UINT8 h[10];
	io_generic_read(io, h, 0, 10);
	sign = (h[0] << 8) | h[1];
	sect = (h[2] << 8) | h[3];
	head = (h[4] << 8) | h[5];
	strack = (h[6] << 8) | h[7];
	etrack = (h[8] << 8) | h[9];
}

bool msa_format::uncompress(UINT8 *buffer, int csize, int usize)
{
	UINT8 sectdata[11*512];
	int src=0, dst=0;
	while(src<csize && dst<usize) {
		unsigned char c = buffer[src++];
		if(c == 0xe5) {
			if(csize-src < 3)
				return false;
			c = buffer[src++];
			int count = (buffer[src] << 8) | buffer[src+1];
			src += 2;
			if(usize-dst < count)
				return false;
			for(int i=0; i<count; i++)
				sectdata[dst++] = c;
		} else
			sectdata[dst++] = c;
	}

	if(src != csize || dst != usize)
		return false;
	memcpy(buffer, sectdata, usize);
	return true;
}

bool msa_format::compress(const UINT8 *buffer, int usize, UINT8 *dest, int &csize)
{
	int src=0, dst=0;
	while(src<usize && dst<usize) {
		unsigned char c = buffer[src++];
		int ncopy = 1;
		while(src < usize && buffer[src] == c) {
			src++;
			ncopy++;
		}
		if(ncopy > 4 || c == 0xe5) {
			if(dst+3 > usize)
				return false;
			dest[dst++] = 0xe5;
			dest[dst++] = c;
			dest[dst++] = ncopy >> 8;
			dest[dst++] = ncopy;
		} else {
			src -= ncopy-1;
			dest[dst++] = c;
		}
	}

	csize = dst;

	return dst < usize;
}

int msa_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT16 sign, sect, head, strack, etrack;
	read_header(io, sign, sect, head, strack, etrack);

	if(sign == 0x0e0f &&
		(sect >= 9 && sect <= 11) &&
		(head == 0 || head == 1) &&
		strack <= etrack &&
		etrack < 82)
		return 100;
	return 0;
}

bool msa_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	UINT16 sign, sect, heads, strack, etrack;
	read_header(io, sign, sect, heads, strack, etrack);

	UINT8 sectdata[11*512];
	desc_s sectors[11];
	for(int i=0; i<sect; i++) {
		sectors[i].data = sectdata + 512*1;
		sectors[i].size = 512;
		sectors[i].sector_id = i + 1;
	}

	int pos = 10;
	int track_size = sect*512;

	for(int track=strack; track <= etrack; track++) {
		for(int head=0; head <= heads; head++) {
			UINT8 th[2];
			io_generic_read(io, th, pos, 2);
			pos += 2;
			int tsize = (th[0] << 8) | th[1];
			io_generic_read(io, sectdata, pos, tsize);
			pos += tsize;
			if(tsize < track_size) {
				if(!uncompress(sectdata, tsize, track_size))
					return false;
			}
			generate_track(atari_st_fcp_get_desc(track, head, head+1, sect),
							track, head, sectors, sect, 100000, image);
		}
	}

	image->set_variant(floppy_image::DSDD);
	return true;
}


bool msa_format::save(io_generic *io, floppy_image *image)
{
	int track_count, head_count, sector_count;
	get_geometry_mfm_pc(image, 2000, track_count, head_count, sector_count);

	if(track_count < 80)
		track_count = 80;
	else if(track_count > 82)
		track_count = 82;

	// Happens for a fully unformatted floppy
	if(!head_count)
		head_count = 1;

	if(sector_count > 11)
		sector_count = 11;
	else if(sector_count < 9)
		sector_count = 9;

	UINT8 header[10];
	header[0] = 0x0e;
	header[1] = 0x0f;
	header[2] = 0;
	header[3] = sector_count;
	header[4] = 0;
	header[5] = head_count-1;
	header[6] = 0;
	header[7] = 0;
	header[8] = 0;
	header[9] = track_count-1;

	io_generic_write(io, header, 0, 10);

	int pos = 10;

	UINT8 sectdata[11*512];
	UINT8 compdata[11*512];
	int track_size = sector_count*512;

	for(int track=0; track < track_count; track++) {
		for(int head=0; head < head_count; head++) {
			get_track_data_mfm_pc(track, head, image, 2000, 512, sector_count, sectdata);
			int csize;
			if(compress(sectdata, track_size, compdata, csize)) {
				UINT8 th[2];
				th[0] = csize >> 8;
				th[1] = csize;
				io_generic_write(io, th, pos, 2);
				io_generic_write(io, compdata, pos+2, csize);
				pos += 2+csize;
			} else {
				UINT8 th[2];
				th[0] = track_size >> 8;
				th[1] = track_size;
				io_generic_write(io, th, pos, 2);
				io_generic_write(io, sectdata, pos+2, track_size);
				pos += 2+track_size;
			}
		}
	}

	return true;
}

const floppy_format_type FLOPPY_ST_FORMAT = &floppy_image_format_creator<st_format>;
const floppy_format_type FLOPPY_MSA_FORMAT = &floppy_image_format_creator<msa_format>;
