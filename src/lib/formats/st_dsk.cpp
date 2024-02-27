// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/st_dsk.cpp

    All usual Atari ST formats

*********************************************************************/

#include "formats/st_dsk.h"

#include "ioprocs.h"
#include "multibyte.h"

#include <cstring>


st_format::st_format()
{
}

const char *st_format::name() const noexcept
{
	return "st";
}

const char *st_format::description() const noexcept
{
	return "Atari ST floppy disk image";
}

const char *st_format::extensions() const noexcept
{
	return "st";
}

bool st_format::supports_save() const noexcept
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

int st_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t track_count, head_count, sector_count;
	find_size(io, track_count, head_count, sector_count);

	if(track_count)
		return FIFID_SIZE;
	return 0;
}

bool st_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
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
			/*auto const [err, actual] =*/ read_at(io, (track*head_count + head)*track_size, sectdata, track_size); // FIXME: check for errors and premature EOF
			generate_track(atari_st_fcp_get_desc(track, head, head_count, sector_count),
							track, head, sectors, sector_count, 100000, image);
		}
	}

	image.set_variant(floppy_image::DSDD);

	return true;
}

bool st_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
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
			/*auto const [err, actual] =*/ write_at(io, (track*head_count + head)*track_size, sectdata, track_size); // FIXME: check for errors
		}
	}

	return true;
}


msa_format::msa_format()
{
}

const char *msa_format::name() const noexcept
{
	return "msa";
}

const char *msa_format::description() const noexcept
{
	return "Atari MSA floppy disk image";
}

const char *msa_format::extensions() const noexcept
{
	return "msa";
}

bool msa_format::supports_save() const noexcept
{
	return true;
}

void msa_format::read_header(util::random_read &io, uint16_t &sign, uint16_t &sect, uint16_t &head, uint16_t &strack, uint16_t &etrack)
{
	uint8_t h[10];
	/*auto const [err, actual] =*/ read_at(io, 0, h, 10); // FIXME: check for errors and premature EOF
	sign = get_u16be(&h[0]);
	sect = get_u16be(&h[2]);
	head = get_u16be(&h[4]);
	strack = get_u16be(&h[6]);
	etrack = get_u16be(&h[8]);
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
			int count = get_u16be(&buffer[src]);
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
			put_u16be(&dest[dst], ncopy);
			dst += 2;
		} else {
			src -= ncopy-1;
			dest[dst++] = c;
		}
	}

	csize = dst;

	return dst < usize;
}

int msa_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint16_t sign, sect, head, strack, etrack;
	read_header(io, sign, sect, head, strack, etrack);

	if(sign == 0x0e0f &&
		(sect >= 9 && sect <= 11) &&
		(head == 0 || head == 1) &&
		strack <= etrack &&
		etrack < 82)
		return FIFID_SIGN | FIFID_STRUCT;
	return 0;
}

bool msa_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint16_t sign, sect, heads, strack, etrack;
	read_header(io, sign, sect, heads, strack, etrack);

	uint8_t sectdata[11*512];
	desc_s sectors[11];
	for(int i=0; i<sect; i++) {
		sectors[i].data = sectdata + 512*i;
		sectors[i].size = 512;
		sectors[i].sector_id = i + 1;
	}

	int pos = 10;
	int track_size = sect*512;

	for(int track=strack; track <= etrack; track++) {
		for(int head=0; head <= heads; head++) {
			uint8_t th[2];
			read_at(io, pos, th, 2); // FIXME: check for errors and premature EOF
			pos += 2;
			int tsize = get_u16be(th);
			read_at(io, pos, sectdata, tsize); // FIXME: check for errors and premature EOF
			pos += tsize;
			if(tsize < track_size) {
				if(!uncompress(sectdata, tsize, track_size))
					return false;
			}
			generate_track(atari_st_fcp_get_desc(track, head, head+1, sect),
							track, head, sectors, sect, 100000, image);
		}
	}

	image.set_variant(floppy_image::DSDD);
	return true;
}


bool msa_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
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
	write(io, header, 10); // FIXME: check for errors

	uint8_t sectdata[11*512];
	uint8_t compdata[11*512];
	int track_size = sector_count*512;

	for(int track=0; track < track_count; track++) {
		for(int head=0; head < head_count; head++) {
			get_track_data_mfm_pc(track, head, image, 2000, 512, sector_count, sectdata);
			int csize;
			if(compress(sectdata, track_size, compdata, csize)) {
				uint8_t th[2];
				put_u16be(th, csize);
				write(io, th, 2); // FIXME: check for errors
				write(io, compdata, csize); // FIXME: check for errors
			} else {
				uint8_t th[2];
				put_u16be(th, track_size);
				write(io, th, 2); // FIXME: check for errors
				write(io, sectdata, track_size); // FIXME: check for errors
			}
		}
	}

	return true;
}

const st_format FLOPPY_ST_FORMAT;
const msa_format FLOPPY_MSA_FORMAT;
