// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/imd_dsk.cpp

    IMD disk images

*********************************************************************/

#include "imd_dsk.h"
#include "flopimg_legacy.h"
#include "imageutl.h"

#include "ioprocs.h"

#include "osdcore.h" // osd_printf_*

#include <cstdio>
#include <cstring>
#include <ctime>



struct imddsk_tag
{
	int heads;
	int tracks;
	int track_sectors[84*2]; /* number of sectors for each track */
	int sector_size;
	uint64_t track_offsets[84*2]; /* offset within data for each track */
};


static struct imddsk_tag *get_tag(floppy_image_legacy *floppy)
{
	struct imddsk_tag *tag;
	tag = (imddsk_tag *)floppy_tag(floppy);
	return tag;
}



FLOPPY_IDENTIFY( imd_dsk_identify )
{
	uint8_t header[3];

	floppy_image_read(floppy, header, 0, 3);
	if (header[0]=='I' && header[1]=='M' && header[2]=='D') {
		*vote = 100;
	} else {
		*vote = 0;
	}
	return FLOPPY_ERROR_SUCCESS;
}

static int imd_get_heads_per_disk(floppy_image_legacy *floppy)
{
	return get_tag(floppy)->heads;
}

static int imd_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return get_tag(floppy)->tracks;
}

static int imd_get_sectors_per_track(floppy_image_legacy *floppy, int head, int track)
{
	return get_tag(floppy)->track_sectors[(track<<1) + head];
}

static uint64_t imd_get_track_offset(floppy_image_legacy *floppy, int head, int track)
{
	return get_tag(floppy)->track_offsets[(track<<1) + head];
}

static floperr_t get_offset(floppy_image_legacy *floppy, int head, int track, int sector, bool sector_is_index, uint64_t *offset)
{
	uint64_t offs = 0;
	uint8_t header[5];
	uint8_t sector_num;
	int i;


	if ((head < 0) || (head >= get_tag(floppy)->heads) || (track < 0) || (track >= get_tag(floppy)->tracks)
			|| (sector < 0) )
		return FLOPPY_ERROR_SEEKERROR;

	offs = imd_get_track_offset(floppy,head,track);
	floppy_image_read(floppy, header, offs, 5);

	sector_num = header[3];
	offs += 5 + sector_num; // skip header and sector numbering map
	if(header[2] & 0x80) offs += sector_num; // skip cylinder numbering map
	if(header[2] & 0x40) offs += sector_num; // skip head numbering map
	get_tag(floppy)->sector_size = 1 << (header[4] + 7);
	for(i=0;i<sector;i++) {
		floppy_image_read(floppy, header, offs, 1); // take sector data type
		switch(header[0]) {
			case 0: offs++; break;
			case 1:
			case 3:
			case 5:
			case 7: offs += get_tag(floppy)->sector_size + 1; break;
			default: offs += 2;
		}
	}
	if (offset)
		*offset = offs;
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_imd_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, bool sector_is_index, void *buffer, size_t buflen)
{
	uint64_t offset;
	floperr_t err;
	uint8_t header[1];

	// take sector offset
	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;

	floppy_image_read(floppy, header, offset, 1);
	switch(header[0]) {
		case 0: break;
		case 1:
		case 3:
		case 5:
		case 7:
				floppy_image_read(floppy, buffer, offset+1, buflen);
				break;

		default: // all data same
				floppy_image_read(floppy, header, offset+1, 1);
				memset(buffer,header[0],buflen);
				break;
	}

	return FLOPPY_ERROR_SUCCESS;
}


static floperr_t imd_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_imd_read_sector(floppy, head, track, sector, false, buffer, buflen);
}

static floperr_t imd_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_imd_read_sector(floppy, head, track, sector, true, buffer, buflen);
}

static floperr_t imd_expand_file(floppy_image_legacy *floppy , uint64_t offset , size_t amount)
{
		if (amount == 0) {
				return FLOPPY_ERROR_SUCCESS;
		}

		uint64_t file_size = floppy_image_size(floppy);

		if (offset > file_size) {
				return FLOPPY_ERROR_INTERNAL;
		}

	uint64_t size_after_off = file_size - offset;

	if (size_after_off == 0) {
		return FLOPPY_ERROR_SUCCESS;
	}

	auto buffer = std::make_unique<uint8_t []>(size_after_off);

	// Read the part of file after offset
	floppy_image_read(floppy, buffer.get(), offset, size_after_off);

	// Add zeroes
	floppy_image_write_filler(floppy, 0, offset, amount);

	// Write back the part of file after offset
	floppy_image_write(floppy, buffer.get(), offset + amount, size_after_off);

	buffer.reset();

	// Update track offsets
	struct imddsk_tag *tag = get_tag(floppy);
	for (int track = 0; track < tag->tracks; track++) {
		for (int head = 0; head < tag->heads; head++) {
			uint64_t *track_off = &(tag->track_offsets[ (track << 1) + head ]);
			if (*track_off >= offset) {
				*track_off += amount;
			}
		}
	}

	return FLOPPY_ERROR_SUCCESS;
}

static floperr_t imd_write_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector_index, const void *buffer, size_t buflen, int ddam)
{
	uint64_t offset;
	floperr_t err;
	uint8_t header[1];

	// take sector offset
	err = get_offset(floppy, head, track, sector_index, true, &offset);
	if (err)
		return err;

	floppy_image_read(floppy, header, offset, 1);

	switch (header[ 0 ]) {
	case 0:
		return FLOPPY_ERROR_SEEKERROR;

	default:
		// Expand image file (from 1 byte to a whole sector)
		err = imd_expand_file(floppy , offset , buflen - 1);
		if (err) {
			return err;
		}
		[[fallthrough]];

	case 1:
	case 3:
	case 5:
	case 7:
		// Turn every kind of sector into type 1 (normal data)
		header[ 0 ] = 1;
		floppy_image_write(floppy, header, offset, 1);
		// Write sector
		floppy_image_write(floppy, buffer, offset + 1, buflen);
		break;
	}

	return FLOPPY_ERROR_SUCCESS;
}

static floperr_t imd_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, uint32_t *sector_length)
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

static floperr_t imd_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, uint32_t *sector_length, unsigned long *flags)
{
	uint64_t offset;
	uint8_t header[5];
	uint8_t hd;
	uint8_t tr;
	uint32_t sector_size;
	uint8_t sector_num;

	offset = imd_get_track_offset(floppy,head,track);
	floppy_image_read(floppy, header, offset, 5);
	tr = header[1];
	hd = header[2];
	sector_num = header[3];
	sector_size = 1 << (header[4] + 7);
	if (sector_index >= sector_num) return FLOPPY_ERROR_SEEKERROR;
	if (cylinder) {
		if (head & 0x80) {
			floppy_image_read(floppy, header, offset + 5 + sector_num+ sector_index, 1);
			*cylinder = header[0];
		} else {
			*cylinder = tr;
		}
	}
	if (side) {
		if (head & 0x40) {
			if (head & 0x80) {
				floppy_image_read(floppy, header, offset + 5 + 2 * sector_num+sector_index, 1);
			} else {
				floppy_image_read(floppy, header, offset + 5 + sector_num+sector_index, 1);
			}
			*side = header[0];
		} else {
			*side = hd & 1;
		}
	}
	if (sector) {
		floppy_image_read(floppy, header, offset + 5 + sector_index, 1);
		*sector = header[0];
	}
	if (sector_length) {
		*sector_length = sector_size;
	}
	if (flags) {
		uint8_t skip;
		if (head & 0x40) {
			if (head & 0x80) {
				skip = 3;
			} else {
				skip = 2;
			}
		} else {
			skip = 1;
		}
		floppy_image_read(floppy, header, offset + 5 + skip * sector_num, 1);
		*flags = 0;
		if ((header[0]-1) & 0x02) *flags |= ID_FLAG_DELETED_DATA;
		if ((header[0]-1) & 0x04) *flags |= ID_FLAG_CRC_ERROR_IN_DATA_FIELD;
	}
	return FLOPPY_ERROR_SUCCESS;
}


FLOPPY_CONSTRUCT( imd_dsk_construct )
{
	struct FloppyCallbacks *callbacks;
	struct imddsk_tag *tag;
	uint8_t header[0x100];
	uint64_t pos = 0;
	int sector_size = 0;
	int sector_num;
	int i;
	if(params)
	{
		// create
		return FLOPPY_ERROR_UNSUPPORTED;
	}

	tag = (struct imddsk_tag *) floppy_create_tag(floppy, sizeof(struct imddsk_tag));
	if (!tag)
		return FLOPPY_ERROR_OUTOFMEMORY;

	floppy_image_read(floppy, header, pos, 1);
	while(header[0]!=0x1a) {
		pos++;
		floppy_image_read(floppy, header, pos, 1);
	}
	pos++;
	tag->tracks = 0;
	tag->heads = 1;
	do {
		floppy_image_read(floppy, header, pos, 5);
		sector_num = header[3];
		int track = (header[1]<<1) + (header[2] & 1);
		if ((header[2] & 1)==1) tag->heads = 2;
		tag->track_offsets[track] = pos;
		tag->track_sectors[track] = sector_num;
		pos += 5 + sector_num; // skip header and sector numbering map
		if(header[2] & 0x80) pos += sector_num; // skip cylinder numbering map
		if(header[2] & 0x40) pos += sector_num; // skip head numbering map
		sector_size = 1 << (header[4] + 7);
		for(i=0;i<sector_num;i++) {
			floppy_image_read(floppy, header, pos, 1); // take sector data type
			switch(header[0]) {
				case 0: pos++; break;
				case 1:
				case 3:
				case 5:
				case 7: pos += sector_size + 1; break;
				default: pos += 2;break;
			}
		}
		tag->tracks += 1;
	} while(pos < floppy_image_size(floppy));
	if (tag->heads==2) {
		tag->tracks = tag->tracks / 2;
	}
	callbacks = floppy_callbacks(floppy);
	callbacks->read_sector = imd_read_sector;
	callbacks->read_indexed_sector = imd_read_indexed_sector;
	callbacks->write_indexed_sector = imd_write_indexed_sector;
	callbacks->get_sector_length = imd_get_sector_length;
	callbacks->get_heads_per_disk = imd_get_heads_per_disk;
	callbacks->get_tracks_per_disk = imd_get_tracks_per_disk;
	callbacks->get_indexed_sector_info = imd_get_indexed_sector_info;
	callbacks->get_sectors_per_track = imd_get_sectors_per_track;

	return FLOPPY_ERROR_SUCCESS;
}


// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/imd_dsk.cpp

    IMD disk images

*********************************************************************/

imd_format::imd_format()
{
}

const char *imd_format::name() const noexcept
{
	return "imd";
}

const char *imd_format::description() const noexcept
{
	return "IMD disk image";
}

const char *imd_format::extensions() const noexcept
{
	return "imd";
}

int imd_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	char h[4];
	auto const [err, actual] = read_at(io, 0, h, 4);
	if(err || (4 != actual))
		return 0;

	if(!memcmp(h, "IMD ", 4))
		return FIFID_SIGN;

	return 0;
}

bool imd_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	std::vector<uint8_t> comment;
	std::vector<std::vector<uint8_t> > snum;
	std::vector<std::vector<uint8_t> > tnum;
	std::vector<std::vector<uint8_t> > hnum;

	std::vector<uint8_t> mode;
	std::vector<uint8_t> track;
	std::vector<uint8_t> head;
	std::vector<uint8_t> sector_count;
	std::vector<uint8_t> ssize;

	int trackmult;
	uint64_t size;
	if(io.length(size))
		return false;

	auto const [err, img, actual] = read_at(io, 0, size);
	if(err || (actual != size))
		return false;

	uint64_t pos, savepos;
	for(pos=0; pos < size && img[pos] != 0x1a; pos++) { }
	pos++;

	comment.resize(pos);
	memcpy(&comment[0], &img[0], pos);

	if(pos >= size)
		return false;

	int tracks, heads;
	image.get_maximal_geometry(tracks, heads);

	mode.clear();
	track.clear();
	head.clear();
	sector_count.clear();
	ssize.clear();
	trackmult = 1;

	// we have to walk the whole file to find out the number of tracks
	savepos = pos;
	uint8_t maxtrack = 0;
	while(pos < size)
	{
		pos++;   // skip mode
		uint8_t track = img[pos++];
		uint8_t head = img[pos++];
		uint8_t sector_count = img[pos++];
		uint8_t sector_size = img[pos++];
		int actual_size = sector_size < 7 ? 128 << sector_size : 8192;

		if (track > maxtrack)
		{
			maxtrack = track;
		}

		pos += sector_count;
		if (head & 0x80)
		{
			pos += sector_count;
		}
		if (head & 0x40)
		{
			pos += sector_count;
		}

		for (int i = 0; i < sector_count; i++)
		{
			uint8_t stype = img[pos++];
			if (stype == 0 || stype > 8)
			{
			}
			else
			{
				if (stype == 2 || stype == 4 || stype == 6 || stype == 8)
				{
					pos++;
				}
				else
				{
					pos += actual_size;
				}
			}
		}
	}

	if(form_factor == floppy_image::FF_525)
	{
		// On 5.25, check if the drive is QD or HD but we're a 40 track
		// image.  If so, put the image on even tracks.
		if ((has_variant(variants, floppy_image::DSQD)) ||
			(has_variant(variants, floppy_image::DSHD)))
		{
			if (maxtrack <= 39)
				trackmult = 2;
		}
		else
		{
			if (maxtrack > 42)
				return false;
		}
	}

	pos = savepos;
	while(pos < size) {
		mode.push_back(img[pos++]);
		track.push_back(img[pos++]);
		head.push_back(img[pos++]);
		sector_count.push_back(img[pos++]);
		ssize.push_back(img[pos++]);

		if(track.back() >= tracks)
		{
			osd_printf_error("imd_format: Track %d exceeds maximum of %d\n", track.back(), tracks);
			return false;
		}

		if((head.back() & 0x3f) >= heads)
		{
			osd_printf_error("imd_format: Head %d exceeds maximum of %d\n", head.back() & 0x3f, heads);
			return false;
		}

		if(ssize.back() == 0xff)
		{
			osd_printf_error("imd_format: Unsupported variable sector size on track %d head %d", track.back(), head.back() & 0x3f);
			return false;
		}

		uint32_t actual_size = ssize.back() < 7 ? 128 << ssize.back() : 8192;

		static const int rates[3] = { 500000, 300000, 250000 };
		bool fm = mode.back() < 3;
		int rate = rates[mode.back() % 3];
		int rpm = form_factor == floppy_image::FF_8 || (form_factor == floppy_image::FF_525 && rate >= 300000) ? 360 : 300;
		int cell_count = (fm ? 1 : 2)*rate*60/rpm;

		//const uint8_t *snum = &img[pos];
		snum.push_back(std::vector<uint8_t>(sector_count.back()));
		memcpy(&snum.back()[0], &img[pos], sector_count.back());
		pos += sector_count.back();

		//const uint8_t *tnum = head & 0x80 ? &img[pos] : nullptr;
		if (head.back() & 0x80)
		{
			tnum.push_back(std::vector<uint8_t>(sector_count.back()));
			memcpy(&tnum.back()[0], &img[pos], sector_count.back());
			pos += sector_count.back();
		}
		else
		{
			tnum.push_back(std::vector<uint8_t>(0));
		}

		//const uint8_t *hnum = head & 0x40 ? &img[pos] : nullptr;
		if (head.back() & 0x40)
		{
			hnum.push_back(std::vector<uint8_t>(sector_count.back()));
			memcpy(&hnum.back()[0], &img[pos], sector_count.back());
			pos += sector_count.back();
		}
		else
		{
			hnum.push_back(std::vector<uint8_t>(0));
		}

		uint8_t chead = head.back() & 0x3f;

		int gap_3 = calc_default_pc_gap3_size(form_factor, actual_size);

		desc_pc_sector sects[256];

		for(int i=0; i<sector_count.back(); i++) {
			uint8_t stype        = img[pos++];
			sects[i].track       = tnum.back().size() ? tnum.back()[i] : track.back();
			sects[i].head        = hnum.back().size() ? hnum.back()[i] : head.back();
			sects[i].sector      = snum.back()[i];
			sects[i].size        = ssize.back();
			sects[i].actual_size = actual_size;

			if(stype == 0 || stype > 8) {
				sects[i].data = nullptr;
			} else {
				sects[i].deleted = stype == 3 || stype == 4 || stype == 7 || stype == 8;
				sects[i].bad_data_crc = stype == 5 || stype == 6 || stype == 7 || stype == 8;
				sects[i].bad_addr_crc = false;

				if(stype == 2 || stype == 4 || stype == 6 || stype == 8) {
					sects[i].data = new uint8_t [actual_size];
					memset(sects[i].data, img[pos++], actual_size);
				} else {
					sects[i].data = &img[pos];
					pos += actual_size;
				}
			}
		}

		if(sector_count.back()) {
			if(fm) {
				build_pc_track_fm(track.back()*trackmult, chead, image, cell_count, sector_count.back(), sects, gap_3);
			} else {
				build_pc_track_mfm(track.back()*trackmult, chead, image, cell_count, sector_count.back(), sects, gap_3);
			}
		}

		for(int i=0; i< sector_count.back(); i++)
			if(sects[i].data && (sects[i].data < &img[0] || sects[i].data >= (&img[0] + size)))
				delete [] sects[i].data;
	}

	// Tag the image with form-factor + variant.  Without this the floppy_image
	// keeps FF_UNKNOWN/variant=0, which propagates to any subsequent save
	// (e.g. MFI) and to MAME runtime code that uses the form factor to pick
	// rotation rate / cell timing.  When the caller passed FF_UNKNOWN (e.g.
	// floptool's auto-detect) we infer everything from the parsed geometry;
	// when the caller passed a real form factor we keep theirs and only deduce
	// the variant.
	uint32_t img_form = form_factor;
	if (img_form == floppy_image::FF_UNKNOWN) {
		if (maxtrack >= 75 && maxtrack <= 78)
			img_form = floppy_image::FF_8;       // 77-track 8"
		else if (maxtrack <= 42)
			img_form = floppy_image::FF_525;     // 40-track 5.25"
		else
			img_form = floppy_image::FF_525;     // 80-track: ambiguous 5.25" vs 3.5"; prefer 525 (IMD-era PC default)
	}

	bool any_mfm = false;
	bool any_500kbps = false;
	for (uint8_t m : mode) {
		if (m >= 3) any_mfm = true;
		if (m == 0 || m == 3) any_500kbps = true;
	}
	uint8_t maxhead = 0;
	for (uint8_t h : head)
		if ((h & 0x3f) > maxhead)
			maxhead = h & 0x3f;
	const bool ds = (maxhead >= 1);

	uint32_t img_variant;
	if (img_form == floppy_image::FF_8) {
		img_variant = any_mfm ? (ds ? floppy_image::DSDD : floppy_image::SSDD)
		                      : (ds ? floppy_image::DSSD : floppy_image::SSSD);
	} else if (img_form == floppy_image::FF_525 || img_form == floppy_image::FF_35) {
		if (any_500kbps && any_mfm)
			img_variant = floppy_image::DSHD;
		else if (any_mfm)
			img_variant = ds ? (maxtrack > 42 ? floppy_image::DSQD : floppy_image::DSDD)
			                 : floppy_image::SSDD;
		else
			img_variant = ds ? floppy_image::DSSD : floppy_image::SSSD;
	} else {
		img_variant = any_mfm ? (ds ? floppy_image::DSDD : floppy_image::SSDD)
		                      : (ds ? floppy_image::DSSD : floppy_image::SSSD);
	}
	image.set_form_variant(img_form, img_variant);

	return true;
}


/*********************************************************************

    Save side: flux-level floppy_image -> IMD container.

    Approach:
      1. For each (cyl, head), probe four (encoding, cell_size)
         combinations covering 250/500 kbps MFM/FM.  The combination
         that yields the most cleanly-decoded sectors wins.
      2. Walk the resulting per-track sector list (in physical order)
         and emit IMD's per-track record: 5-byte header, sector
         numbering map, optional cyl/head maps, then per-sector type
         byte + data (compressed when the sector is all-fill).
      3. A fresh "Created by MAME flopconvert" header with localtime is
         emitted at the start of every saved IMD.

*********************************************************************/

bool imd_format::supports_save() const noexcept
{
	return true;
}

// Single-bit reader with wrap-around.  The private sbit_rp method in
// floppy_image_format_t is not visible from this translation unit, so
// keep an identical local copy.
static inline bool sbit_local(const std::vector<bool> &bs, uint32_t &pos)
{
	bool b = bs[pos++];
	if (pos == bs.size()) pos = 0;
	return b;
}

void imd_format::extract_track_rich(const std::vector<bool> &bs, bool is_mfm,
									std::vector<extracted_sector> &out) const
{
	out.clear();
	if (bs.size() < 100)
		return;

	// Pass 1: locate every IDAM and every DAM in the bitstream, recording
	// both the position immediately after the type byte and the actual
	// type byte (needed later for CRC verification).
	struct mark { uint32_t pos; uint8_t type_byte; };
	std::vector<mark> idam_marks, dam_marks;

	uint16_t shift_reg = 0;
	// Precharge for wrap-around-the-index detection.
	for (uint32_t i = 0; i < 16; i++)
		if (bs[bs.size() - 16 + i])
			shift_reg |= 0x8000 >> i;

	if (is_mfm) {
		for (uint32_t i = 0; i < bs.size(); i++) {
			shift_reg = uint16_t((shift_reg << 1) | bs[i]);
			if (shift_reg == 0x4489) {        // MFM 0xA1 sync (missing clock)
				uint16_t header_word;
				uint32_t pos = i + 1;
				do {
					header_word = 0;
					for (int j = 0; j < 16; j++)
						if (sbit_local(bs, pos))
							header_word |= 0x8000 >> j;
				} while (header_word == 0x4489 && pos > i);

				// 0x5554 = MFM(0xFE), 0x5555 = MFM(0xFF)
				if (header_word == 0x5554 || header_word == 0x5555) {
					idam_marks.push_back({pos, uint8_t(header_word == 0x5554 ? 0xfe : 0xff)});
					i = pos - 1;
				}
				// 0x554a = MFM(0xF8) deleted DAM
				else if (header_word == 0x554a) {
					dam_marks.push_back({pos, 0xf8});
					i = pos - 1;
				}
				// 0x5549 = MFM(0xF9), 0x5544 = MFM(0xFA), 0x5545 = MFM(0xFB)
				else if (header_word == 0x5549) {
					dam_marks.push_back({pos, 0xf9});
					i = pos - 1;
				} else if (header_word == 0x5544) {
					dam_marks.push_back({pos, 0xfa});
					i = pos - 1;
				} else if (header_word == 0x5545) {
					dam_marks.push_back({pos, 0xfb});
					i = pos - 1;
				}
			}
		}
	} else {
		// FM: address marks have unique clock patterns; no separate sync
		// byte.  See IBM 3740 SD format.
		for (uint32_t i = 0; i < bs.size(); i++) {
			shift_reg = uint16_t((shift_reg << 1) | bs[i]);
			if (shift_reg == 0xf57e) {                                   // 0xFE
				idam_marks.push_back({i + 1, 0xfe});
			} else if (shift_reg == 0xf56a) {                            // 0xF8 deleted
				dam_marks.push_back({i + 1, 0xf8});
			} else if (shift_reg == 0xf56b) {                            // 0xF9
				dam_marks.push_back({i + 1, 0xf9});
			} else if (shift_reg == 0xf56e) {                            // 0xFA
				dam_marks.push_back({i + 1, 0xfa});
			} else if (shift_reg == 0xf56f) {                            // 0xFB
				dam_marks.push_back({i + 1, 0xfb});
			}
		}
	}

	// Pass 2: walk IDAMs in physical order, read the 4-byte sector header
	// plus CRC, find the matching DAM, read data + CRC, validate both.
	for (size_t i = 0; i < idam_marks.size(); i++) {
		uint32_t ipos = idam_marks[i].pos;

		extracted_sector sec;
		sec.idam_track  = sbyte_mfm_r(bs, ipos);
		sec.idam_head   = sbyte_mfm_r(bs, ipos);
		sec.idam_sector = sbyte_mfm_r(bs, ipos);
		sec.idam_size   = sbyte_mfm_r(bs, ipos);
		uint8_t crc_hi  = sbyte_mfm_r(bs, ipos);
		uint8_t crc_lo  = sbyte_mfm_r(bs, ipos);
		uint16_t idam_stored_crc = uint16_t(uint16_t(crc_hi) << 8 | crc_lo);

		uint8_t idam_buf[8];
		size_t idam_buf_len;
		if (is_mfm) {
			idam_buf[0] = 0xa1; idam_buf[1] = 0xa1; idam_buf[2] = 0xa1;
			idam_buf[3] = idam_marks[i].type_byte;
			idam_buf[4] = sec.idam_track;
			idam_buf[5] = sec.idam_head;
			idam_buf[6] = sec.idam_sector;
			idam_buf[7] = sec.idam_size;
			idam_buf_len = 8;
		} else {
			idam_buf[0] = idam_marks[i].type_byte;
			idam_buf[1] = sec.idam_track;
			idam_buf[2] = sec.idam_head;
			idam_buf[3] = sec.idam_sector;
			idam_buf[4] = sec.idam_size;
			idam_buf_len = 5;
		}
		sec.addr_crc_ok = (ccitt_crc16(0xffff, idam_buf, idam_buf_len) == idam_stored_crc);

		if (sec.idam_size >= 8) {
			// Idam size >= 8 means the size code byte fell outside the
			// spec range 0..6.  Almost always flux noise dressed up as a
			// false sync.  Drop entirely.
			continue;
		}
		int ssize = 128 << sec.idam_size;

		// Find first DAM after this IDAM within the IBM-spec tolerance.
		// MFM: 704 cells nominal, allow 704-128 .. 1008+128.
		// FM:  384 cells nominal, allow 384-128 .. 384+128.
		int dam_idx = -1;
		for (size_t d = 0; d < dam_marks.size(); d++) {
			int delta = int(dam_marks[d].pos) - int(idam_marks[i].pos);
			if (is_mfm) {
				if (delta >= 704 - 128 && delta <= 1008 + 128) { dam_idx = int(d); break; }
			} else {
				if (delta >= 384 - 128 && delta <= 384 + 128) { dam_idx = int(d); break; }
			}
		}
		if (dam_idx < 0) {
			// No matching DAM after this IDAM.  Real sectors usually
			// have a DAM; a missing-data sector is rare but legal in
			// IMD (sector type 0).  We only emit one if the IDAM CRC
			// validated — otherwise it's more likely a phantom sync.
			if (sec.addr_crc_ok) out.push_back(std::move(sec));
			continue;
		}

		sec.deleted_dam = (dam_marks[dam_idx].type_byte == 0xf8);

		uint32_t dpos = dam_marks[dam_idx].pos;
		sec.data.resize(ssize);
		for (int j = 0; j < ssize; j++)
			sec.data[j] = sbyte_mfm_r(bs, dpos);
		uint8_t dcrc_hi = sbyte_mfm_r(bs, dpos);
		uint8_t dcrc_lo = sbyte_mfm_r(bs, dpos);
		uint16_t dam_stored_crc = uint16_t(uint16_t(dcrc_hi) << 8 | dcrc_lo);

		std::vector<uint8_t> dam_buf;
		if (is_mfm) {
			dam_buf = { 0xa1, 0xa1, 0xa1, dam_marks[dam_idx].type_byte };
		} else {
			dam_buf = { dam_marks[dam_idx].type_byte };
		}
		dam_buf.insert(dam_buf.end(), sec.data.begin(), sec.data.end());
		sec.data_crc_ok = (ccitt_crc16(0xffff, dam_buf.data(), size_t(dam_buf.size())) == dam_stored_crc);
		sec.has_data    = true;

		if (sec.addr_crc_ok || sec.data_crc_ok)
			out.push_back(std::move(sec));
	}

	// Deduplicate by sector ID.  Phantom IDAMs (random sync-pattern hits in
	// noisy flux) can produce extra entries with the same idam_sector as a
	// real sector but no matching DAM.  When two candidates share the same
	// sector ID, prefer the one with valid data over the one without; if
	// both have data, prefer good data CRC; if both lack data, prefer the
	// good IDAM CRC.  Stable: preserves the physical order of the kept
	// entries.
	auto rank = [](const extracted_sector &s) -> int {
		// Higher rank = more trustworthy
		if (s.has_data && s.data_crc_ok && s.addr_crc_ok) return 4;
		if (s.has_data && s.addr_crc_ok)                   return 3;
		if (s.has_data)                                    return 2;
		if (s.addr_crc_ok)                                 return 1;
		return 0;
	};
	std::vector<extracted_sector> dedup;
	dedup.reserve(out.size());
	for (auto &candidate : out) {
		bool replaced = false;
		for (auto &kept : dedup) {
			if (kept.idam_sector == candidate.idam_sector) {
				if (rank(candidate) > rank(kept))
					kept = std::move(candidate);
				replaced = true;
				break;
			}
		}
		if (!replaced)
			dedup.push_back(std::move(candidate));
	}
	out = std::move(dedup);
}

bool imd_format::detect_track(const floppy_image &image, int cyl, int head,
							  track_info &out) const
{
	// (encoding, cell_size_ns, IMD mode byte)
	// cell_size is in MAME's internal flux representation: 200,000,000 ns
	// canonical per revolution / cells_per_revolution.  So a given media rate
	// needs *different* probe cell_sizes for 300 RPM vs 360 RPM disks:
	//   300 RPM, MFM 500 kbps: 200M / 200000 = 1000 ns/cell  (5.25" HD, 3.5" HD)
	//   360 RPM, MFM 500 kbps: 200M / 166666 = 1200 ns/cell  (8" DSDD/SSDD)
	//   300 RPM, MFM 250 kbps: 200M / 100000 = 2000 ns/cell  (5.25" DD, 3.5" DD)
	//   360 RPM, MFM 250 kbps: 200M /  83333 = 2400 ns/cell  (8" rare)
	//   300 RPM, FM  500 kbps: 200M / 100000 = 2000 ns/cell
	//   360 RPM, FM  500 kbps: 200M /  83333 = 2400 ns/cell  (8" SSSD)
	//   300 RPM, FM  250 kbps: 200M /  50000 = 4000 ns/cell  (5.25" SD; T-200/250)
	//   360 RPM, FM  250 kbps: 200M /  41666 = 4800 ns/cell  (8" rare)
	// Without the 360 RPM variants, MAME-loaded 8" MFM disks save as empty IMD
	// records because the PLL's ~25% lock tolerance is overrun by the 20%
	// cell-size mismatch (1200 vs 1000) -- the 5.25" probe scores zero sectors
	// and detect_track returns "mode 3 with no sectors".
	struct probe { bool is_mfm; int cell_size; uint8_t mode; };
	static const probe probes[] = {
		{ true,  1000, 3 },   // MFM 500 kbps @ 300 RPM (5.25" HD, 3.5" HD)
		{ true,  1200, 3 },   // MFM 500 kbps @ 360 RPM (8" DSDD/SSDD)
		{ true,  2000, 5 },   // MFM 250 kbps @ 300 RPM (5.25" DD, 3.5" DD)
		{ true,  2400, 5 },   // MFM 250 kbps @ 360 RPM (8")
		{ false, 2000, 0 },   // FM  500 kbps @ 300 RPM
		{ false, 2400, 0 },   // FM  500 kbps @ 360 RPM (8" SSSD)
		{ false, 4000, 2 },   // FM  250 kbps @ 300 RPM (5.25" SD; T-200/250 boot)
		{ false, 4800, 2 },   // FM  250 kbps @ 360 RPM (8")
	};

	int best_score = -1;
	track_info best;

	for (const auto &p : probes) {
		auto bs = generate_bitstream_from_track(cyl, head, p.cell_size, image);
		if (bs.empty())
			continue;

		std::vector<extracted_sector> secs;
		extract_track_rich(bs, p.is_mfm, secs);

		// Score by sectors with a good IDAM CRC.  Bad data CRC is not
		// penalised — the saved IMD will honestly mark such sectors
		// type 5/6/7/8 — but a bad IDAM CRC means we likely guessed
		// the wrong encoding.
		int score = 0;
		for (const auto &s : secs)
			if (s.addr_crc_ok) score++;

		if (score > best_score) {
			best_score      = score;
			best.is_mfm     = p.is_mfm;
			best.cell_size  = p.cell_size;
			best.mode_byte  = p.mode;
			best.sectors    = std::move(secs);
		}
	}

	out = std::move(best);
	return best_score > 0;
}

bool imd_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants,
					  const floppy_image &image) const
{
	int tracks = 0, heads = 0;
	image.get_actual_geometry(tracks, heads);
	if (tracks <= 0 || heads <= 0) {
		osd_printf_error("imd_format: image has no formatted tracks; refusing to save.\n");
		return false;
	}

	// -- ASCII header.  Always fresh; the format manager is const, so
	//    we deliberately do not cache anything from a prior load().
	std::string header;
	{
		std::time_t const t = std::time(nullptr);
		std::tm const lt = *std::localtime(&t);
		char buf[64];
		std::snprintf(buf, sizeof(buf), "IMD 1.20: %02d/%02d/%04d %02d:%02d:%02d\r\n",
					  lt.tm_mday, lt.tm_mon + 1, lt.tm_year + 1900,
					  lt.tm_hour, lt.tm_min, lt.tm_sec);
		header  = buf;
		header += "Created by MAME flopconvert\r\n";
		header += '\x1a';
	}

	if (auto pr = write_at(io, 0, header.data(), header.size()); pr.first || pr.second != header.size())
		return false;
	uint64_t out_pos = header.size();

	// -- Walk tracks in IMD's canonical order: cyl outer, head inner.
	for (int cyl = 0; cyl < tracks; cyl++) {
		for (int hd = 0; hd < heads; hd++) {
			track_info ti;
			bool ok = detect_track(image, cyl, hd, ti);
			(void)ok;  // ok==false just means we emit an empty record

			uint8_t mode      = ti.mode_byte;
			uint8_t sec_count = uint8_t(ti.sectors.size());
			uint8_t size_code = 0;

			if (sec_count > 0) {
				size_code = ti.sectors[0].idam_size;
				for (size_t i = 1; i < ti.sectors.size(); i++) {
					if (ti.sectors[i].idam_size != size_code) {
						osd_printf_error("imd_format: cyl %d head %d has mixed sector "
										 "sizes (code %d vs %d); IMD cannot represent this.\n",
										 cyl, hd, size_code, ti.sectors[i].idam_size);
						return false;
					}
				}
				if (size_code >= 8) {
					osd_printf_error("imd_format: cyl %d head %d has out-of-spec sector "
									 "size code %d.\n", cyl, hd, size_code);
				return false;
				}
			}
			int sec_size = 128 << size_code;

			bool need_cmap = false, need_hmap = false;
			for (const auto &s : ti.sectors) {
				if (s.idam_track != uint8_t(cyl)) need_cmap = true;
				if (s.idam_head  != uint8_t(hd))  need_hmap = true;
			}
			uint8_t head_byte = uint8_t(uint8_t(hd) & 0x3f)
							  | uint8_t(need_cmap ? 0x80 : 0)
							  | uint8_t(need_hmap ? 0x40 : 0);

			uint8_t recbuf[5] = { mode, uint8_t(cyl), head_byte, sec_count, size_code };
			if (auto pr = write_at(io, out_pos, recbuf, 5); pr.first || pr.second != 5) return false;
			out_pos += 5;

			if (sec_count == 0)
				continue;

			std::vector<uint8_t> snum(sec_count);
			for (int i = 0; i < sec_count; i++) snum[i] = ti.sectors[i].idam_sector;
			if (auto pr = write_at(io, out_pos, snum.data(), sec_count); pr.first || pr.second != sec_count) return false;
			out_pos += sec_count;

			if (need_cmap) {
				std::vector<uint8_t> cmap(sec_count);
				for (int i = 0; i < sec_count; i++) cmap[i] = ti.sectors[i].idam_track;
				if (auto pr = write_at(io, out_pos, cmap.data(), sec_count); pr.first || pr.second != sec_count) return false;
				out_pos += sec_count;
			}
			if (need_hmap) {
				std::vector<uint8_t> hmap(sec_count);
				for (int i = 0; i < sec_count; i++) hmap[i] = ti.sectors[i].idam_head;
				if (auto pr = write_at(io, out_pos, hmap.data(), sec_count); pr.first || pr.second != sec_count) return false;
				out_pos += sec_count;
			}

			for (const auto &s : ti.sectors) {
				if (!s.has_data) {
					uint8_t zero = 0;
					if (auto pr = write_at(io, out_pos, &zero, 1); pr.first || pr.second != 1) return false;
					out_pos += 1;
					continue;
				}

				// Compress if every byte is the same value.
				bool compressed = true;
				uint8_t fill = s.data[0];
				for (int i = 1; i < sec_size; i++) {
					if (s.data[i] != fill) { compressed = false; break; }
				}

				// Type byte: 1=normal/good/normal-DAM, +1 if compressed,
				// +2 if deleted DAM, +4 if bad data CRC OR bad addr CRC.
				bool bad = !s.data_crc_ok || !s.addr_crc_ok;
				int t = 1;
				if (compressed)    t += 1;
				if (s.deleted_dam) t += 2;
				if (bad)           t += 4;
				uint8_t type = uint8_t(t);
				if (auto pr = write_at(io, out_pos, &type, 1); pr.first || pr.second != 1) return false;
				out_pos += 1;

				if (compressed) {
					if (auto pr = write_at(io, out_pos, &fill, 1); pr.first || pr.second != 1) return false;
					out_pos += 1;
				} else {
					if (auto pr = write_at(io, out_pos, s.data.data(), sec_size); pr.first || pr.second != size_t(sec_size)) return false;
					out_pos += sec_size;
				}
			}
		}
	}

	return true;
}


const imd_format FLOPPY_IMD_FORMAT;
