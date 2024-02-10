// license:BSD-3-Clause
// copyright-holders:O. Galibert, R. Belmont
/*********************************************************************

    ap_dsk35.cpp

    Apple 3.5" disk images

    This code supports 3.5" 400k/800k disks used in early Macintoshes
    and the Apple IIgs, and 3.5" 1440k MFM disks used on most Macintoshes.

    These disks have the following properties:

        400k:   80 tracks, 1 head
        800k:   80 tracks, 2 heads

        Tracks  0-15 have 12 sectors each
        Tracks 16-31 have 11 sectors each
        Tracks 32-47 have 10 sectors each
        Tracks 48-63 have  9 sectors each
        Tracks 64-79 have  8 sectors each

        1440k disks are simply 80 tracks, 2 heads and 18 sectors per track.

    Each sector on 400/800k disks has 524 bytes, 512 of which are really used by the Macintosh

    (80 tracks) * (avg of 10 sectors) * (512 bytes) * (2 sides) = 800 kB

    Data is nibblized : 3 data bytes -> 4 bytes on disk.

    In addition to 512 logical bytes, each sector contains 800 physical
    bytes.  Here is the layout of the physical sector:

        Pos
        0       0xFF (pad byte where head is turned on ???)
        1-35    self synch 0xFFs (7*5) (42 bytes actually written to media)
        36      0xD5
        37      0xAA
        38      0x96
        39      diskbytes[(track number) & 0x3F]
        40      diskbytes[(sector number)]
        41      diskbytes[("side")]
        42      diskbytes[("format byte")]
        43      diskbytes[("sum")]
        44      0xDE
        45      0xAA
        46      pad byte where head is turned off/on (0xFF here)
        47-51   self synch 0xFFs (6 bytes actually written to media)
        52      0xD5
        53      0xAA
        54      0xAD
        55      spare byte, generally diskbytes[(sector number)]
        56-754  "nibblized" sector data ...
        755-758 checksum
        759     0xDE
        760     0xAA
        761     pad byte where head is turned off (0xFF here)

    MFM Track layout for 1440K disks:
        Pos
    --------- track ID
    0   0x4E (x80)
    80  00 (x12)
    92  C2 (x3) Mark byte
    93  FC Index mark
    94  4E (x50)
    --------- sector ID
    144 00 (x12)
    156 A1 (x3) Mark byte
    159 FE Address mark
    160 xx Track number
    161 xx Side number
    162 xx Sector number
    163 xx Sector length
    164/165 xx CRC
    166 4E (x22)
    --------- sector data
    188 00 (x12)
    200 A1 (x3) Mark byte
    203 FB Data address mark
    204 xx (x256) data
    460 4E (x54)
    (repeat from sector ID to fill track; end of track is padded with 4E)

    Note : "Self synch refers to a technique whereby two zeroes are inserted
    between each synch byte written to the disk.", i.e. "0xFF, 0xFF, 0xFF,
    0xFF, 0xFF" is actually "0xFF, 0x3F, 0xCF, 0xF3, 0xFC, 0xFF" on disk.
    Since the IWM assumes the data transfer is complete when the MSBit of its
    shift register is 1, we do read 4 0xFF, even though they are not
    contiguous on the disk media.  Some reflexion shows that 4 synch bytes
    allow the IWM to synchronize with the trailing data.

    Format byte codes:
        0x00    Apple II
        0x01    Lisa
        0x02    Mac MFS (single sided)?
        0x22    Mac MFS (double sided)?

*********************************************************************/

#include "ap_dsk35.h"

#include "ioprocs.h"
#include "multibyte.h"
#include "opresolv.h"

#include "eminline.h"
#include "osdcore.h" // osd_printf_error

#include <cassert>
#include <cstdio>
#include <cstring>

dc42_format::dc42_format() : floppy_image_format_t()
{
}

const char *dc42_format::name() const noexcept
{
	return "dc42";
}

const char *dc42_format::description() const noexcept
{
	return "DiskCopy 4.2 image";
}

const char *dc42_format::extensions() const noexcept
{
	return "dc42";
}

bool dc42_format::supports_save() const noexcept
{
	return true;
}

int dc42_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if(io.length(size) || (size < 0x54))
		return 0;

	uint8_t h[0x54];
	size_t actual;
	io.read_at(0, h, 0x54, actual);
	uint32_t dsize = get_u32be(&h[0x40]);
	uint32_t tsize = get_u32be(&h[0x44]);

	uint8_t encoding = h[0x50];
	uint8_t format = h[0x51];
	// if it's a 1.44MB DC42 image, reject it so the generic PC MFM handler picks it up
	if ((encoding == 0x03) && (format == 0x02))
	{
		return 0;
	}

	return (size == 0x54+tsize+dsize && h[0] < 64 && h[0x52] == 1 && h[0x53] == 0) ? FIFID_STRUCT : 0;
}

bool dc42_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	size_t actual;
	uint8_t h[0x54];
	io.read_at(0, h, 0x54, actual);
	int dsize = get_u32be(&h[0x40]);
	int tsize = get_u32be(&h[0x44]);

	uint8_t encoding = h[0x50];
	uint8_t format = h[0x51];

	if((encoding != 0x00 || format != 0x02) && ((encoding != 0x01 && encoding != 0x03) || (format != 0x22 && format != 0x24))) {
		osd_printf_error("dc42: Unsupported encoding/format combination %02x/%02x\n", encoding, format);
		return false;
	}

	switch(dsize) {
		case 409600:    // Mac 400K
			image.set_form_variant(floppy_image::FF_35, floppy_image::SSDD);
			break;

		case 737280:    // PC 720K
		case 819200:    // Mac/A2 800K
			image.set_form_variant(floppy_image::FF_35, floppy_image::DSDD);
			break;

		case 1474560:   // PC or Mac 1.44M
			image.set_form_variant(floppy_image::FF_35, floppy_image::DSHD);
			break;

		case 871424:    // Apple Twiggy 851KiB
			image.set_form_variant(floppy_image::FF_525, floppy_image::DSHD);
			break;

		default:
			osd_printf_error("dc42: dsize is %d, unknown form factor\n");
			break;
	}

	uint8_t sector_data[(512+12)*12];
	memset(sector_data, 0, sizeof(sector_data));

	desc_gcr_sector sectors[12];

	int pos_data = 0x54;
	int pos_tag = 0x54+dsize;

	int head_count = encoding == 1 ? 2 : 1;

	for(int track=0; track < 80; track++) {
		for(int head=0; head < head_count; head++) {
			int ns = 12 - (track/16);
			int si = 0;
			for(int i=0; i<ns; i++) {
				uint8_t *data = sector_data + (512+12)*i;
				sectors[si].track = track;
				sectors[si].head = head;
				sectors[si].sector = i;
				sectors[si].info = format;
				if(tsize) {
					io.read_at(pos_tag, data, 12, actual);
					sectors[si].tag = data;
					pos_tag += 12;
				} else {
					sectors[si].tag = nullptr;
				}
				sectors[si].data = data+12;
				io.read_at(pos_data, data+12, 512, actual);
				pos_data += 512;
				si = (si + 2) % ns;
				if(si == 0)
					si++;
			}
			build_mac_track_gcr(track, head, image, sectors);
		}
	}
	return true;
}

void dc42_format::update_chk(const uint8_t *data, int size, uint32_t &chk)
{
	for(int i=0; i<size; i+=2) {
		chk += get_u16be(&data[i]);
		chk = rotr_32(chk, 1);
	}
}

bool dc42_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	int g_tracks, g_heads;
	image.get_actual_geometry(g_tracks, g_heads);

	if(g_heads == 0)
		g_heads = 1;

	uint8_t h[0x54];
	memset(h, 0, 0x54);
	strcpy((char *)h+1, "Unnamed");
	h[0] = 7;
	int nsect = 16*(12+11+10+9+8)*g_heads;
	uint32_t dsize = nsect*512;
	uint32_t tsize = nsect*12;
	put_u32be(&h[0x40], dsize);
	put_u32be(&h[0x44], tsize);
	h[0x50] = g_heads == 2 ? 0x01 : 0x00;
	h[0x51] = g_heads == 2 ? 0x22 : 0x02;
	h[0x52] = 0x01;
	h[0x53] = 0x00;

	uint32_t dchk = 0;
	uint32_t tchk = 0;

	int pos_data = 0x54;
	int pos_tag = 0x54+dsize;

	for(int track=0; track < 80; track++) {
		for(int head=0; head < g_heads; head++) {
			auto sectors = extract_sectors_from_track_mac_gcr6(head, track, image);
			for(unsigned int i=0; i < sectors.size(); i++) {
				auto &sdata = sectors[i];
				sdata.resize(512+12);
				size_t actual;
				io.write_at(pos_tag, &sdata[0], 12, actual);
				io.write_at(pos_data, &sdata[12], 512, actual);
				pos_tag += 12;
				pos_data += 512;
				if(track || head || i)
					update_chk(&sdata[0], 12, tchk);
				update_chk(&sdata[12], 512, dchk);
			}
		}
	}

	put_u32be(&h[0x48], dchk);
	put_u32be(&h[0x4c], tchk);

	size_t actual;
	io.write_at(0, h, 0x54, actual);
	return true;
}

const dc42_format FLOPPY_DC42_FORMAT;


apple_gcr_format::apple_gcr_format() : floppy_image_format_t()
{
}

const char *apple_gcr_format::name() const noexcept
{
	return "apple_gcr";
}

const char *apple_gcr_format::description() const noexcept
{
	return "Apple GCR 400/800K raw sector image";
}

const char *apple_gcr_format::extensions() const noexcept
{
	return "img";
}

bool apple_gcr_format::supports_save() const noexcept
{
	return true;
}

int apple_gcr_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if(io.length(size))
		return 0;

	if(size == 409600 || (size == 819200 && (variants.empty() || has_variant(variants, floppy_image::DSDD))))
		return FIFID_SIZE;

	return 0;
}

bool apple_gcr_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	size_t actual;
	desc_gcr_sector sectors[12];
	uint8_t sdata[512*12];

	int pos_data = 0;

	uint8_t header[64];
	io.read_at(0, header, 64, actual);

	uint64_t size;
	if(io.length(size))
		return false;
	int head_count = size == 409600 ? 1 : size == 819200 ? 2 : 0;

	image.set_form_variant(floppy_image::FF_35, head_count == 2 ? floppy_image::DSDD : floppy_image::SSDD);

	if(!head_count)
		return false;
	for(int track=0; track < 80; track++) {
		for(int head=0; head < head_count; head++) {
			int ns = 12 - (track/16);
			io.read_at(pos_data, sdata, 512*ns, actual);
			pos_data += 512*ns;

			int si = 0;
			for(int i=0; i<ns; i++) {
				sectors[si].track = track;
				sectors[si].head = head;
				sectors[si].sector = i;
				sectors[si].info = head_count == 2 ? 0x22 : 0x02;
				sectors[si].tag = nullptr;
				sectors[si].data = sdata + 512*i;
				si = (si + 2) % ns;
				if(si == 0)
					si++;
			}
			build_mac_track_gcr(track, head, image, sectors);
		}
	}
	return true;
}

bool apple_gcr_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	// HACK: don't let floptool force a square peg into a round hole
	if(image.get_form_factor() != floppy_image::FF_35)
		return false;

	int g_tracks, g_heads;
	image.get_actual_geometry(g_tracks, g_heads);

	if(g_heads == 0)
		g_heads = 1;

	int pos_data = 0;

	for(int track=0; track < 80; track++) {
		for(int head=0; head < g_heads; head++) {
			auto sectors = extract_sectors_from_track_mac_gcr6(head, track, image);
			for(unsigned int i=0; i < sectors.size(); i++) {
				auto &sdata = sectors[i];
				sdata.resize(512+12);
				size_t actual;
				io.write_at(pos_data, &sdata[12], 512, actual);
				pos_data += 512;
			}
		}
	}

	return true;
}

const apple_gcr_format FLOPPY_APPLE_GCR_FORMAT;

// .2MG format
apple_2mg_format::apple_2mg_format() : floppy_image_format_t()
{
}

const char *apple_2mg_format::name() const noexcept
{
	return "apple_2mg";
}

const char *apple_2mg_format::description() const noexcept
{
	return "Apple II .2MG image";
}

const char *apple_2mg_format::extensions() const noexcept
{
	return "2mg";
}

bool apple_2mg_format::supports_save() const noexcept
{
	return true;
}

int apple_2mg_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t signature[4];
	size_t actual;
	io.read_at(0, signature, 4, actual);
	if (!strncmp(reinterpret_cast<char *>(signature), "2IMG", 4))
	{
		return FIFID_SIGN;
	}

	// Bernie ][ The Rescue wrote 2MGs with the signature byte-flipped, other fields are valid
	if (!strncmp(reinterpret_cast<char *>(signature), "GMI2", 4))
	{
		return FIFID_SIGN;
	}

	return 0;
}

bool apple_2mg_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	size_t actual;
	desc_gcr_sector sectors[12];
	uint8_t sdata[512*12], header[64];
	io.read_at(0, header, 64, actual);
	uint32_t blocks = get_u32le(&header[0x14]);
	uint32_t pos_data = get_u32le(&header[0x18]);

	if(blocks != 1600 && blocks != 16390)
		return false;

	image.set_form_variant(floppy_image::FF_35, (blocks > 800) ? floppy_image::DSDD : floppy_image::SSDD);

	for(int track=0; track < 80; track++) {
		for(int head=0; head < 2; head++) {
			int ns = 12 - (track/16);
			io.read_at(pos_data, sdata, 512*ns, actual);
			pos_data += 512*ns;

			int si = 0;
			for(int i=0; i<ns; i++) {
				sectors[si].track = track;
				sectors[si].head = head;
				sectors[si].sector = i;
				sectors[si].info = 0x22;
				sectors[si].tag = nullptr;
				sectors[si].data = sdata + 512*i;
				si = (si + 2) % ns;
				if(si == 0)
					si++;
			}
			build_mac_track_gcr(track, head, image, sectors);
		}
	}
	return true;
}

bool apple_2mg_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	size_t actual;

	uint8_t header[0x40];
	int pos_data = 0x40;

	memset(header, 0, sizeof(header));
	// file ID
	header[0] = '2'; header[1] = 'I'; header[2] = 'M'; header[3] = 'G';
	// creator program
	header[4] = 'M'; header[5] = 'A'; header[6] = 'M'; header[7] = 'E';
	// header size
	header[8] = 0x40;
	// version
	header[0xa] = 1;
	// flags
	header[0xc] = 1;    // ProDOS sector order
	// number of ProDOS blocks
	header[0x14] = 0x40; header[0x15] = 0x06;   // 0x640 (1600)
	// offset to sector data
	header[0x18] = 0x40;
	// bytes of disk data
	header[0x1c] = 0x00; header[0x1d] = 0x80; header[0x1e] = 0x0c;  // 0xC8000 (819200)
	io.write_at(0, header, 0x40, actual);

	for(int track=0; track < 80; track++) {
		for(int head=0; head < 2; head++) {
			auto sectors = extract_sectors_from_track_mac_gcr6(head, track, image);
			for(unsigned int i=0; i < sectors.size(); i++) {
				auto &sdata = sectors[i];
				sdata.resize(512+12);
				io.write_at(pos_data, &sdata[12], 512, actual);
				pos_data += 512;
			}
		}
	}

	return true;
}

const apple_2mg_format FLOPPY_APPLE_2MG_FORMAT;
