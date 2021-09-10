// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/st_dsk.c

    All usual Atari ST formats

*********************************************************************/

#include "formats/st_dsk.h"

#include "ioprocs.h"


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

void st_format::find_size(util::random_read &io, uint8_t &track_count, uint8_t &head_count, uint8_t &sector_count)
{
	uint64_t size;
	if(!io.length(size)) {
		for(track_count=80; track_count <= 82; track_count++)
			for(head_count=1; head_count <= 2; head_count++)
				for(sector_count=9; sector_count <= 11; sector_count++)
					if(size == (uint32_t)512*track_count*head_count*sector_count)
						return;
	}
	track_count = head_count = sector_count = 0;
}

int st_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	uint8_t track_count, head_count, sector_count;
	find_size(io, track_count, head_count, sector_count);

	if(track_count)
		return 50;
	return 0;
}

bool st_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image)
{
	uint8_t track_count, head_count, sector_count;
	find_size(io, track_count, head_count, sector_count);

	uint8_t sectdata[11*512];
	desc_s sectors[11];
	for(int i=0; i<sector_count; i++) {
		sectors[i].data = sectdata + 512*i;
		sectors[i].size = 512;
		sectors[i].sector_id = i + 1;
	}

	int track_size = sector_count*512;
	for(int track=0; track < track_count; track++) {
		for(int head=0; head < head_count; head++) {
			size_t actual;
			io.read_at((track*head_count + head)*track_size, sectdata, track_size, actual);
			generate_track(atari_st_fcp_get_desc(track, head, head_count, sector_count),
							track, head, sectors, sector_count, 100000, image);
		}
	}

	image->set_variant(floppy_image::DSDD);

	return true;
}

bool st_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image)
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

	uint8_t sectdata[11*512];
	int track_size = sector_count*512;

	for(int track=0; track < track_count; track++) {
		for(int head=0; head < head_count; head++) {
			get_track_data_mfm_pc(track, head, image, 2000, 512, sector_count, sectdata);
			size_t actual;
			io.write_at((track*head_count + head)*track_size, sectdata, track_size, actual);
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

void msa_format::read_header(util::random_read &io, uint16_t &sign, uint16_t &sect, uint16_t &head, uint16_t &strack, uint16_t &etrack)
{
	uint8_t h[10];
	size_t actual;
	io.read_at(0, h, 10, actual);
	sign = (h[0] << 8) | h[1];
	sect = (h[2] << 8) | h[3];
	head = (h[4] << 8) | h[5];
	strack = (h[6] << 8) | h[7];
	etrack = (h[8] << 8) | h[9];
}

bool msa_format::uncompress(uint8_t *buffer, int csize, int usize)
{
	uint8_t sectdata[11*512];
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

bool msa_format::compress(const uint8_t *buffer, int usize, uint8_t *dest, int &csize)
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

int msa_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	uint16_t sign, sect, head, strack, etrack;
	read_header(io, sign, sect, head, strack, etrack);

	if(sign == 0x0e0f &&
		(sect >= 9 && sect <= 11) &&
		(head == 0 || head == 1) &&
		strack <= etrack &&
		etrack < 82)
		return 100;
	return 0;
}

bool msa_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image)
{
	uint16_t sign, sect, heads, strack, etrack;
	read_header(io, sign, sect, heads, strack, etrack);

	uint8_t sectdata[11*512];
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
			size_t actual;
			uint8_t th[2];
			io.read_at(pos, th, 2, actual);
			pos += 2;
			int tsize = (th[0] << 8) | th[1];
			io.read_at(pos, sectdata, tsize, actual);
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


bool msa_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image)
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

	uint8_t header[10];
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

	if(io.seek(0, SEEK_SET))
		return false;
	size_t actual;
	io.write(header, 10, actual);

	uint8_t sectdata[11*512];
	uint8_t compdata[11*512];
	int track_size = sector_count*512;

	for(int track=0; track < track_count; track++) {
		for(int head=0; head < head_count; head++) {
			get_track_data_mfm_pc(track, head, image, 2000, 512, sector_count, sectdata);
			int csize;
			if(compress(sectdata, track_size, compdata, csize)) {
				uint8_t th[2];
				th[0] = csize >> 8;
				th[1] = csize;
				io.write(th, 2, actual);
				io.write(compdata, csize, actual);
			} else {
				uint8_t th[2];
				th[0] = track_size >> 8;
				th[1] = track_size;
				io.write(th, 2, actual);
				io.write(sectdata, track_size, actual);
			}
		}
	}

	return true;
}

const floppy_format_type FLOPPY_ST_FORMAT = &floppy_image_format_creator<st_format>;
const floppy_format_type FLOPPY_MSA_FORMAT = &floppy_image_format_creator<msa_format>;
