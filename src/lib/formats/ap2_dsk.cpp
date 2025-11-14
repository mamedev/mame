// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
/*********************************************************************

    ap2_dsk.cpp

    Apple II disk images

*********************************************************************/

#include "ap2_dsk.h"
#include "basicdsk.h"

#include "ioprocs.h"
#include "multibyte.h"

#include <cassert>
#include <cstdlib>
#include <cstring>


class a2_sect_format::byte_reader
{
public:
	byte_reader(const byte_reader &) = default;
	byte_reader& operator=(const byte_reader &) = default;

	byte_reader(const std::vector<bool> &b) : buf(&b) { }

	uint8_t operator()()
	{
		uint8_t v = 0;
		const int w1 = wrap;
		while((wrap != w1+2) && !(v & 0x80)) {
			v = (v << 1) | ((*buf)[pos] ? 1 : 0);
			pos++;
			if(pos == buf->size()) {
				pos = 0;
				wrap++;
			}
		}
		return v;
	}

	bool wrapped() const { return wrap != 0; }

private:
	const std::vector<bool> *buf;
	int pos = 0;
	int wrap = 0;
};


a2_sect_format::a2_sect_format(int nsect) : m_nsect(nsect)
{
	assert(nsect <= APPLE2_MAX_SECTOR_COUNT);
}

bool a2_sect_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	int g_tracks, g_heads;
	int visualgrid[APPLE2_MAX_SECTOR_COUNT][APPLE2_TRACK_COUNT]; // visualizer grid, cleared/initialized below

	constexpr bool VERBOSE_SAVE = false;

	// if false, only accept an addr mark if the checksum was good
	// if true, accept an addr mark if the track and sector values are both sane
	constexpr bool LENIENT_ADDR_CHECK = false;

	// if true, use the old, not as robust logic for choosing which copy of a decoded sector to write
	// to the resulting image if the sector has a bad checksum and/or postamble
	constexpr bool USE_OLD_BEST_SECTOR_PRIORITY = false;

	// nothing found
	constexpr int NOTFOUND = 0;
	// address mark was found
	constexpr int ADDRFOUND = 1;
	// address checksum is good
	constexpr int ADDRGOOD = 2;
	// data mark was found (requires addrfound and sane values)
	constexpr int DATAFOUND = 4;
	// data checksum is good
	constexpr int DATAGOOD = 8;
	// data postamble is good
	constexpr int DATAPOST = 16;

	for (auto & elem : visualgrid) {
		for (int j = 0; j < APPLE2_TRACK_COUNT; j++) {
			elem[j] = NOTFOUND;
		}
	}
	image.get_actual_geometry(g_tracks, g_heads);

	int head = 0;

	int pos_data = 0;

	for(int track=0; track < g_tracks; track++) {
		uint8_t sectdata[APPLE2_SECTOR_SIZE*APPLE2_MAX_SECTOR_COUNT] = {};

		if(VERBOSE_SAVE) {
			fprintf(stderr,"DEBUG: a2_sect_format::save() about to generate bitstream from track %d...", track);
		}
		auto buf = generate_bitstream_from_track(track, head, 3915, image);
		if(VERBOSE_SAVE) {
			fprintf(stderr,"done.\n");
		}
		byte_reader br(buf);
		int hb = 0;
		int dosver = 0; // apple dos version; 0 = >=3.3, 1 = <3.3
		for(;;) {
			uint8_t v = br();
			if(v == 0xff) {
				hb = 1;
			}
			else if(hb == 1 && v == 0xd5){
				hb = 2;
			}
			else if(hb == 2 && v == 0xaa) {
				hb = 3;
			}
			else if(hb == 3 && ((v == 0x96) || (v == 0xb5))) { // 0x96 = dos 3.3/16sec, 0xb5 = dos 3.21 and below/13sec
				hb = 4;
				if (v == 0xb5) dosver = 1;
			}
			else
				hb = 0;

			if(hb == 4) {
				uint8_t h[11];
				for(auto & elem : h)
					elem = br();
				//uint8_t v2 = gcr6bw_tb[h[2]];
				uint8_t vl = gcr4_decode(h[0],h[1]);
				uint8_t tr = gcr4_decode(h[2],h[3]);
				uint8_t se = gcr4_decode(h[4],h[5]);
				uint8_t chk = gcr4_decode(h[6],h[7]);
				if(VERBOSE_SAVE) {
					uint32_t post = get_u24be(&h[8]);
					printf("Address Mark:\tVolume %d, Track %d, Sector %2d, Checksum %02X: %s, Postamble %03X: %s\n",
							vl, tr, se, chk, (chk ^ vl ^ tr ^ se)==0?"OK":"BAD", post, (post&0xffff00)==0xdeaa00?"OK":"BAD");
				}
				// sanity check
				if (tr == track && se < m_nsect) {
					int &gridcell = visualgrid[se][track];
					gridcell |= ADDRFOUND;
					gridcell |= ((chk ^ vl ^ tr ^ se)==0)?ADDRGOOD:0;

					if (gridcell & (LENIENT_ADDR_CHECK ? ADDRFOUND : ADDRGOOD)) {
						byte_reader orig_br(br);

						hb = 0;
						for(int i=0; i<20 && hb != 4; i++) {
							v = br();
							if(v == 0xff)
								hb = 1;
							else if(hb == 1 && v == 0xd5)
								hb = 2;
							else if(hb == 2 && v == 0xaa)
								hb = 3;
							else if(hb == 3 && v == 0xad)
								hb = 4;
							else
								hb = 0;
						}
						if(hb == 4 && check_dosver(dosver)) {
							gridcell |= DATAFOUND;

							uint8_t decoded_buf[APPLE2_SECTOR_SIZE];
							uint8_t dchk_expected, dchk_actual;
							decode_sector_data(br, decoded_buf, dchk_expected, dchk_actual);

							bool dchk_good = dchk_expected == dchk_actual;

							// now read the postamble bytes
							uint32_t dpost = 0;
							for(int i=0; i<3; i++) {
								dpost <<= 8;
								dpost |= br();
							}
							bool dpost_good = (dpost & 0xffff00) == 0xdeaa00;

							uint8_t *dest = sectdata + APPLE2_SECTOR_SIZE * logical_sector_index(se);

							// only write it if the bitfield of the track shows datagood is NOT set.
							// if it is set we don't want to overwrite a guaranteed good read with a bad one
							// if past read had a bad checksum or bad postamble...
							if(USE_OLD_BEST_SECTOR_PRIORITY) {
								if (!(gridcell & DATAGOOD)) {
									std::copy_n(decoded_buf, sizeof decoded_buf, dest);
								}
							} else {
								bool was_datagood = gridcell & DATAGOOD;
								bool was_datapost = gridcell & DATAPOST;

								if (!was_datagood || !was_datapost) {
									// if the current read is good, and postamble is good, write it in, no matter what.
									// if the current read is good and the current postamble is bad, write it in unless the postamble was good before
									// if the current read is bad and the current postamble is good and the previous read had neither good, write it in
									// if the current read isn't good and neither is the postamble but nothing better
									// has been written before, write it anyway.
									if ((dchk_good && dpost_good) ||
										(dchk_good && !dpost_good && !was_datapost) ||
										(!dchk_good && dpost_good && !was_datagood && !was_datapost) ||
										(!dchk_good && !dpost_good && !was_datagood && !was_datapost)
									) {
										std::copy_n(decoded_buf, sizeof decoded_buf, dest);
									}
								}
							}
							// do some checking
							if(VERBOSE_SAVE && (!dchk_good || !dpost_good)) {
								fprintf(stderr,"Data Mark:\tChecksum xpctd %d found %d: %s, Postamble %03X: %s\n",
										dchk_expected, dchk_actual, dchk_good?"OK":"BAD", dpost, dpost_good?"OK":"BAD");
							}
							if (dchk_good) gridcell |= DATAGOOD;
							if (dpost_good) gridcell |= DATAPOST;
						} else {
							br = orig_br;
						}
					}
				}
				hb = 0;
			}
			if(br.wrapped())
				break;
		}
		for(int i = 0; i < m_nsect; i++) {
			uint8_t const *const data = sectdata + APPLE2_SECTOR_SIZE*i;
			auto const [err, actual] = write_at(io, pos_data, data, APPLE2_SECTOR_SIZE);
			if (err || actual != APPLE2_SECTOR_SIZE)
				return false;
			pos_data += APPLE2_SECTOR_SIZE;
		}
	}
	// display a little table of which sectors decoded ok
	if(VERBOSE_SAVE) {
		int total_good = 0;
		for (int j = 0; j < APPLE2_TRACK_COUNT; j++) {
			printf("T%2d: ",j);
			for (int i = 0; i < m_nsect; i++) {
				if (visualgrid[i][j] == NOTFOUND) printf("-NF- ");
				else {
					if (visualgrid[i][j] & ADDRFOUND) printf("a"); else printf(" ");
					if (visualgrid[i][j] & ADDRGOOD) printf("A"); else printf(" ");
					if (visualgrid[i][j] & DATAFOUND) printf("d"); else printf(" ");
					if (visualgrid[i][j] & DATAGOOD) { printf("D"); total_good++; } else printf(" ");
					if (visualgrid[i][j] & DATAPOST) printf("."); else printf(" ");
				}
			}
			printf("\n");
		}
		printf("Total Good Sectors: %d\n", total_good);
	}

	return true;
}

bool a2_sect_format::supports_save() const noexcept
{
	return true;
}

static const uint8_t translate5[] = {
	0xab, 0xad, 0xae, 0xaf, 0xb5, 0xb6, 0xb7, 0xba,
	0xbb, 0xbd, 0xbe, 0xbf, 0xd6, 0xd7, 0xda, 0xdb,
	0xdd, 0xde, 0xdf, 0xea, 0xeb, 0xed, 0xee, 0xef,
	0xf5, 0xf6, 0xf7, 0xfa, 0xfb, 0xfd, 0xfe, 0xff,
};

static const uint8_t untranslate5[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x05, 0x06, 0x00, 0x00, 0x07, 0x08, 0x00, 0x09, 0x0a, 0x0b,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0d, 0x00, 0x00, 0x0e, 0x0f, 0x00, 0x10, 0x11, 0x12,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x00, 0x15, 0x16, 0x17,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x19, 0x1a, 0x00, 0x00, 0x1b, 0x1c, 0x00, 0x1d, 0x1e, 0x1f,
};

a2_13sect_format::a2_13sect_format() : a2_sect_format(SECTOR_COUNT)
{
}

int a2_13sect_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return 0;

	if (size != APPLE2_STD_TRACK_COUNT * SECTOR_COUNT * APPLE2_SECTOR_SIZE)
		return 0;

	return FIFID_SIZE;
}

bool a2_13sect_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint64_t size;
	if (io.length(size))
		return false;

	image.set_form_variant(floppy_image::FF_525, floppy_image::SSSD);

	int tracks = size / SECTOR_COUNT / APPLE2_SECTOR_SIZE;

	for(int track = 0; track < tracks; track++) {
		std::vector<uint32_t> track_data;
		uint8_t sector_data[APPLE2_SECTOR_SIZE * SECTOR_COUNT];

		auto const [err, actual] = read_at(
			io, track * sizeof sector_data, sector_data, sizeof sector_data);
		if (err || actual != sizeof sector_data)
			return false;

		for(int i=0; i<SECTOR_COUNT; i++) {
			int sector = (i * 10) % SECTOR_COUNT;

			// write inter-sector padding
			for(int j=0; j<40; j++)
				raw_w(track_data, 9, 0x1fe);
			raw_w(track_data,  8, 0xff);

			// write sector address
			raw_w(track_data, 24, 0xd5aab5);
			raw_w(track_data, 16, gcr4_encode(0xfe));
			raw_w(track_data, 16, gcr4_encode(track));
			raw_w(track_data, 16, gcr4_encode(sector));
			raw_w(track_data, 16, gcr4_encode(0xfe ^ track ^ sector));
			raw_w(track_data, 24, 0xdeaaeb);

			// write intra-sector padding
			for(int j=0; j<11; j++)
				raw_w(track_data, 9, 0x1fe);

			// write sector data
			raw_w(track_data, 24, 0xd5aaad);

			uint8_t pval = 0x00;
			auto write_data_byte =
					[&track_data, &pval] (uint8_t nval)
					{
						raw_w(track_data, 8, translate5[nval ^ pval]);
						pval = nval;
					};

			const uint8_t *sdata = sector_data + APPLE2_SECTOR_SIZE * sector;

			// write 154 bytes encoding bits 2-0
			write_data_byte(sdata[255] & 7);
			for (int k=2; k>-1; k--)
				for (int j=0; j<51; j++)
					write_data_byte(
						(sdata[j*5+k] & 7) << 2
						| ((sdata[j*5+3] >> (2-k)) & 1) << 1
						| ((sdata[j*5+4] >> (2-k)) & 1));

			// write 256 bytes encoding bits 7-3
			for (int k=0; k<5; k++)
				for (int j=50; j>-1; j--)
					write_data_byte(sdata[j*5+k] >> 3);
			write_data_byte(sdata[255] >> 3);

			raw_w(track_data, 8, translate5[pval]);
			raw_w(track_data, 24, 0xdeaaeb);
			raw_w(track_data, 8, 0xff);
		}

		generate_track_from_levels(track, 0, track_data, 0, image);
	}

	return true;
}


const char *a2_13sect_format::name() const noexcept
{
	return "a2_13sect";
}

const char *a2_13sect_format::description() const noexcept
{
	return "Apple II 13-sector d13 image";
}

const char *a2_13sect_format::extensions() const noexcept
{
	return "d13";
}

bool a2_13sect_format::check_dosver(int dosver) const
{
	if (dosver != 1) {
		fprintf(stderr, "ERROR: DOS 3.3 sector found while saving to 13-sector image format\n");
		return false;
	}

	return true;
}

int a2_13sect_format::logical_sector_index(int physical) const {
	return physical;
}

void a2_13sect_format::decode_sector_data(
		byte_reader &br, uint8_t (&decoded_buf)[APPLE2_SECTOR_SIZE],
		uint8_t &dchk_expected, uint8_t &dchk_actual) const
{
	uint8_t low_bits[154];

	dchk_expected = 0;

	// read the block of bits 2-0
	for(auto &b : low_bits)
		dchk_expected = b = untranslate5[br()] ^ dchk_expected;

	// read the block of bits 7-3
	for (int k=0; k<5; k++)
		for (int j=50; j>-1; j--)
			dchk_expected = decoded_buf[j*5+k] = untranslate5[br()] ^ dchk_expected;
	dchk_expected = decoded_buf[255] = untranslate5[br()] ^ dchk_expected;

	// read the checksum byte
	dchk_actual = untranslate5[br()];

	// combine the lower and upper bits
	for (int j=0; j<51; j++) {
		for (int k=0; k<3; k++) {
			uint8_t lb = low_bits[(2-k)*51+j+1];
			decoded_buf[j*5+k] = (decoded_buf[j*5+k] << 3) | (lb >> 2);
			decoded_buf[j*5+3] = (decoded_buf[j*5+3] << 1) | ((lb >> 1) & 1);
			decoded_buf[j*5+4] = (decoded_buf[j*5+4] << 1) | (lb & 1);
		}
	}
	decoded_buf[255] = (decoded_buf[255] << 3) | (low_bits[0] & 7);
}

const a2_13sect_format FLOPPY_A213S_FORMAT;

static const uint8_t dos_skewing[] =
{
	0x00, 0x07, 0x0e, 0x06, 0x0d, 0x05, 0x0c, 0x04,
	0x0b, 0x03, 0x0a, 0x02, 0x09, 0x01, 0x08, 0x0f
};

static const uint8_t prodos_skewing[] =
{
	0x00, 0x08, 0x01, 0x09, 0x02, 0x0a, 0x03, 0x0b,
	0x04, 0x0c, 0x05, 0x0d, 0x06, 0x0e, 0x07, 0x0f
};


a2_16sect_format::a2_16sect_format(bool prodos_order) : a2_sect_format(SECTOR_COUNT), m_prodos_order(prodos_order)
{
}

a2_16sect_dos_format::a2_16sect_dos_format() : a2_16sect_format(false)
{
}

const char *a2_16sect_dos_format::name() const noexcept
{
	return "a2_16sect_dos";
}

const char *a2_16sect_dos_format::description() const noexcept
{
	return "Apple II 16-sector dsk image (DOS sector order)";
}

const char *a2_16sect_dos_format::extensions() const noexcept
{
	return "dsk,do";
}

a2_16sect_prodos_format::a2_16sect_prodos_format() : a2_16sect_format(true)
{
}

const char *a2_16sect_prodos_format::name() const noexcept
{
	return "a2_16sect_prodos";
}

const char *a2_16sect_prodos_format::description() const noexcept
{
	return "Apple II 16-sector dsk image (ProDos sector order)";
}

const char *a2_16sect_prodos_format::extensions() const noexcept
{
	return "dsk,po";
}

int a2_16sect_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return 0;

	// check standard size plus some oddball sizes in our softlist
	if (size != APPLE2_TRACK_COUNT * SECTOR_COUNT * APPLE2_SECTOR_SIZE
		&& size != APPLE2_STD_TRACK_COUNT * SECTOR_COUNT * APPLE2_SECTOR_SIZE
		&& size != 143403 && size != 143363 && size != 143358 && size != 143195) {
		return 0;
	}

	uint8_t sector_data[APPLE2_SECTOR_SIZE*2];
	static const unsigned char pascal_block1[4] = { 0x08, 0xa5, 0x0f, 0x29 };
	static const unsigned char pascal2_block1[4] = { 0xff, 0xa2, 0x00, 0x8e };
	static const unsigned char dos33_block1[4] = { 0xa2, 0x02, 0x8e, 0x52 };
	static const unsigned char sos_block1[4] = { 0xc9, 0x20, 0xf0, 0x3e };
	static const unsigned char a3a2emul_block1[6] = { 0x8d, 0xd0, 0x03, 0x4c, 0xc7, 0xa4 };
	static const unsigned char cpm22_block1[8] = { 0xa2, 0x55, 0xa9, 0x00, 0x9d, 0x00, 0x0d, 0xca };
	static const unsigned char subnod_block1[8] = { 0x63, 0xaa, 0xf0, 0x76, 0x8d, 0x63, 0xaa, 0x8e };

	auto const [err, actual] = read_at(io, 0, sector_data, sizeof sector_data);
	if (err || actual != sizeof sector_data)
		return 0;

	bool prodos_order = false;
	if (!memcmp("PRODOS", &sector_data[0x103], 6)) {
		// ProDOS boot block
		prodos_order = true;
	} else if (!memcmp("PRODOS", &sector_data[0x121], 6)) {
		// alternate version ProDOS boot block
		prodos_order = true;
	} else if (!memcmp(sos_block1, &sector_data[0x100], 4)) {
		// ProDOS order SOS disk
		prodos_order = true;
	} else if (!memcmp(a3a2emul_block1, &sector_data[0x100], 6)) {
		// Apple III A2 emulator disk in ProDOS order
		prodos_order = true;
	} else if (!memcmp("COPYRIGHT (C) 1979, DIGITAL RESEARCH", &sector_data[0x118], 36)) {
		// PCPI Applicard software in ProDOS order
		prodos_order = true;
	} else if (!memcmp("SYSTEM.APPLE", &sector_data[0xd7], 12)) {
		// Apple II Pascal
		// Pascal discs can still be DOS order.
		// Check for the second half of the boot code at 0x100
		// (which means ProDOS order)
		if (!memcmp(pascal_block1, &sector_data[0x100], 4)) {
			prodos_order = true;
		}
	} else if (!memcmp(dos33_block1, &sector_data[0x100], 4)) {
		// DOS 3.3 disks in ProDOS order
		prodos_order = true;
	} else if (!memcmp(pascal2_block1, &sector_data[0x100], 4)) {
		// a later version of the Pascal boot block
		prodos_order = true;
	} else if (!memcmp(cpm22_block1, &sector_data[0x100], 8)) {
		// CP/M disks in ProDOS order
		prodos_order = true;
	} else if (!memcmp(subnod_block1, &sector_data[0x100], 8)) {
		// subnodule disk
		prodos_order = true;
	} else if (!memcmp("PRODOS", &sector_data[0x3a], 6)) {
		// ProDOS 2.5's new boot block
		prodos_order = true;
	}

	return FIFID_SIZE | (m_prodos_order == prodos_order ? FIFID_HINT : 0);
}

bool a2_16sect_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint64_t size;
	if (io.length(size))
		return false;

	image.set_form_variant(floppy_image::FF_525, floppy_image::SSSD);

	int tracks = (size == (APPLE2_TRACK_COUNT * SECTOR_COUNT * APPLE2_SECTOR_SIZE))
			? APPLE2_TRACK_COUNT : APPLE2_STD_TRACK_COUNT;

	int fpos = 0;
	for(int track=0; track < tracks; track++) {
		std::vector<uint32_t> track_data;
		uint8_t sector_data[APPLE2_SECTOR_SIZE*SECTOR_COUNT];

		auto const [err, actual] = read_at(io, fpos, sector_data, sizeof sector_data);
		// Some supported images have oddball sizes, where the last track is incomplete.
		// Skip the `actual` check to avoid rejecting them.
		if (err /* || actual != sizeof sector_data */)
			return false;

		fpos += APPLE2_SECTOR_SIZE*SECTOR_COUNT;
		for(int i=0; i<49; i++)
			raw_w(track_data, 10, 0x3fc);
		for(int i=0; i<SECTOR_COUNT; i++) {
			int sector;

			if (m_prodos_order) {
				sector = prodos_skewing[i];
			} else {
				sector = dos_skewing[i];
			}

			const uint8_t *sdata = sector_data + APPLE2_SECTOR_SIZE * sector;
			for(int j=0; j<20; j++)
				raw_w(track_data, 10, 0x3fc);
			raw_w(track_data,  8, 0xff);
			raw_w(track_data, 24, 0xd5aa96);
			raw_w(track_data, 16, gcr4_encode(0xfe));
			raw_w(track_data, 16, gcr4_encode(track));
			raw_w(track_data, 16, gcr4_encode(i));
			raw_w(track_data, 16, gcr4_encode(0xfe ^ track ^ i));
			raw_w(track_data, 24, 0xdeaaeb);

			for(int j=0; j<4; j++)
				raw_w(track_data, 10, 0x3fc);

			raw_w(track_data,  9, 0x01fe);
			raw_w(track_data, 24, 0xd5aaad);
			raw_w(track_data,  1, 0);

			uint8_t pval = 0x00;
			for(int i=0; i<342; i++) {
				uint8_t nval;
				if(i >= 0x56)
					nval = sdata[i - 0x56] >> 2;
				else {
					nval =
						((sdata[i+0x00] & 0x01) << 1) |
						((sdata[i+0x00] & 0x02) >> 1) |
						((sdata[i+0x56] & 0x01) << 3) |
						((sdata[i+0x56] & 0x02) << 1);
					if(i < 256-0xac)
						nval |=
							((sdata[i+0xac] & 0x01) << 5) |
							((sdata[i+0xac] & 0x02) << 3);
				}
				raw_w(track_data, 8, gcr6fw_tb[nval ^ pval]);
				pval = nval;
			}
			raw_w(track_data, 8, gcr6fw_tb[pval]);
			raw_w(track_data, 24, 0xdeaaeb);
		}
		raw_w(track_data, 8, 0xff);
		assert(track_data.size() == 51090);

		generate_track_from_levels(track, 0, track_data, 0, image);
	}
	return true;
}

bool a2_16sect_format::check_dosver(int dosver) const
{
	if (dosver != 0) {
		fprintf(stderr, "ERROR: DOS 3.2 sector found while saving to 16-sector image format\n");
		return false;
	}

	return true;
}

int a2_16sect_format::logical_sector_index(int physical) const {
	if (m_prodos_order) {
		return prodos_skewing[physical];
	} else {
		return dos_skewing[physical];
	}
}

void a2_16sect_format::decode_sector_data(
		byte_reader &br, uint8_t (&decoded_buf)[APPLE2_SECTOR_SIZE],
		uint8_t &dchk_expected, uint8_t &dchk_actual) const
{
	uint8_t low_bits[0x56];

	dchk_expected = 0;

	// first read in sector and decode to 6bit form
	for(auto &b : low_bits)
		dchk_expected = b = gcr6bw_tb[br()] ^ dchk_expected;
	for(auto &b : decoded_buf)
		dchk_expected = b = gcr6bw_tb[br()] ^ dchk_expected;

	// read the checksum byte
	dchk_actual = gcr6bw_tb[br()];

	// next combine in the lower 2 bits of each byte
	static const uint8_t bit_swap[4] = { 0, 2, 1, 3 };
	for(int i=0; i<0x56; i++)
		decoded_buf[i] = decoded_buf[i]<<2 | bit_swap[low_bits[i]&3];
	for(int i=0; i<0x56; i++)
		decoded_buf[i+0x56] = decoded_buf[i+0x56]<<2 | bit_swap[(low_bits[i]>>2)&3];
	for(int i=0; i<0x54; i++)
		decoded_buf[i+0xac] = decoded_buf[i+0xac]<<2 | bit_swap[(low_bits[i]>>4)&3];
}

const a2_16sect_dos_format FLOPPY_A216S_DOS_FORMAT;
const a2_16sect_prodos_format FLOPPY_A216S_PRODOS_FORMAT;

a2_edd_format::a2_edd_format() : floppy_image_format_t()
{
}

const char *a2_edd_format::name() const noexcept
{
	return "a2_edd";
}

const char *a2_edd_format::description() const noexcept
{
	return "Apple II EDD Image";
}

const char *a2_edd_format::extensions() const noexcept
{
	return "edd";
}

int a2_edd_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return 0;
	return ((size == 2244608) || (size == 2310144)) ? FIFID_SIZE : 0;
}

uint8_t a2_edd_format::pick(const uint8_t *data, int pos)
{
	return get_u16be(&data[pos>>3]) >> (8-(pos & 7));
}

bool a2_edd_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint8_t nibble[16384], stream[16384];
	int npos[16384];
	static const size_t img_size = 2'244'608;

	auto [err, img, actual] = read_at(io, 0, img_size);
	if(err || actual != img_size)
		return false;

	for(int i=0; i<137; i++) {
		uint8_t const *const trk = &img[16384*i];
		int pos = 0;
		int wpos = 0;
		while(pos < 16383*8) {
			uint8_t acc = pick(trk, pos);
			pos += 8;
			while(!(acc & 0x80) && pos < 16384*8) {
				acc <<= 1;
				if(trk[pos >> 3] & (0x80 >> (pos & 7)))
					acc |= 0x01;
				pos++;
			}
			if(acc & 0x80) {
				nibble[wpos] = acc;
				npos[wpos] = pos;
				wpos++;
			}
		}
		int nm = 0, nmj = 0, nmk = 0;
		for(int j=0; j<wpos-1; j++)
			for(int k=j+6200; k<wpos && k<j+6400; k++) {
				int m = 0;
				for(int l=0; k+l<wpos && nibble[j+l] == nibble[k+l]; l++)
					m++;
				if(m > nm) {
					nm = m;
					nmj = j;
					nmk = k;
				}
			}
		int delta = nmk - nmj;
		int spos = (wpos-delta)/2;
		int zpos = npos[spos];
		int epos = npos[spos+delta];
		int len = epos-zpos;
		int part1_size = zpos % len;
		int part1_bsize = part1_size >> 3;
		int part1_spos = epos-part1_size;
		int part2_offset = zpos - part1_size;
		int total_bsize = (len+7) >> 3;

		for(int j=0; j<part1_bsize; j++)
			stream[j] = pick(trk, part1_spos + 8*j);
		stream[part1_bsize] =
			(pick(trk, part1_spos + 8*part1_bsize) & (0xff00 >> (part1_size & 7))) |
			(pick(trk, part2_offset + 8*part1_bsize) & (0x00ff >> (part1_size & 7)));
		for(int j=part1_bsize+1; j<total_bsize; j++)
			stream[j] = pick(trk, part2_offset + 8*j);

		bool odd = false;
		for(int j=0; j<len; j++)
			if(stream[j>>3] & (0x80 >> (j & 7)))
				odd = !odd;

		int splice_byte = spos;
		while(splice_byte < spos+delta && (npos[splice_byte+1] - npos[splice_byte] != 8 || npos[splice_byte+2] - npos[splice_byte+1] == 8 || npos[splice_byte+3] - npos[splice_byte+2] == 8))
			splice_byte++;
		int splice = (npos[splice_byte+2]-1) % len;
		if(odd)
			stream[splice >> 3] ^= 0x80 >> (splice & 7);

		generate_track_from_bitstream(i >> 2, 0, stream, len, image, i & 3);
		image.set_write_splice_position(i >> 2, 0, uint32_t(uint64_t(200'000'000)*splice/len), i & 3);
	}
	img.reset();

	image.set_form_variant(floppy_image::FF_525, floppy_image::SSSD);

	return true;
}

const a2_edd_format FLOPPY_EDD_FORMAT;


a2_nib_format::a2_nib_format() : floppy_image_format_t()
{
}

const char *a2_nib_format::name() const noexcept
{
	return "a2_nib";
}

const char *a2_nib_format::description() const noexcept
{
	return "Apple II NIB Image";
}

const char *a2_nib_format::extensions() const noexcept
{
	return "nib";
}

int a2_nib_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint64_t size;
	if (io.length(size))
		return 0;

	if (size == expected_size_35t || size == expected_size_40t)
		return FIFID_SIZE;

	return 0;
}


template <typename It>
static size_t count_leading_FFs(const It first, const It last)
{
	auto curr = first;
	for (; curr != last; ++curr) {
		if (*curr != 0xff) {
			break;
		}
	}
	return curr - first;
}

static size_t count_trailing_padding(const std::vector<uint8_t>& nibbles)
{
	const auto b = nibbles.rbegin();
	const auto e = nibbles.rend();
	auto i = b;

	// skip until the first valid nibble...
	for (; i != e; ++i) {
		if ((*i & 0x80) != 0) { // valid nibble
			break;
		}
	}
	return i - b;
}

std::vector<uint32_t> a2_nib_format::generate_levels_from_nibbles(const std::vector<uint8_t>& nibbles)
{
	std::vector<uint32_t> levels;
	const auto append_FFs =
			[&levels] (size_t count)
			{
				while (count-- > 0) {
					raw_w(levels, 8, 0xff);
				}
			};
	const auto append_syncs =
			[&levels] (size_t count)
			{
				while (count-- > 0) {
					raw_w(levels, 10, 0x00ff << 2);
				}
			};
	const auto append_byte = [&levels] (uint8_t byte) { raw_w(levels, 8, byte); };


	const auto leading_FF_count = count_leading_FFs(nibbles.begin(), nibbles.end());

	if (leading_FF_count >= nibbles.size()) { // all are 0xff !?!?
		assert(leading_FF_count >= min_sync_bytes);
		append_syncs(leading_FF_count);
		return levels;
	}

	const auto trailing_padding_size = count_trailing_padding(nibbles);
	const auto trailing_FF_count = count_leading_FFs(nibbles.rbegin() + trailing_padding_size, nibbles.rend());
	const auto wrapped_FF_count = leading_FF_count + trailing_FF_count;
	const bool wrapped_FF_are_syncs = wrapped_FF_count >= min_sync_bytes;

	if (wrapped_FF_are_syncs) {
		append_syncs(leading_FF_count);
	} else {
		append_FFs(leading_FF_count);
	}

	{
		size_t FF_count = 0;
		const auto flush_FFs =
				[&append_syncs, &append_FFs, &FF_count]
				{
					if (FF_count == 0) {
						return;
					}

					if (FF_count >= a2_nib_format::min_sync_bytes) {
						append_syncs(FF_count);
					} else {
						append_FFs(FF_count);
					}
					FF_count = 0;
				};

		const auto end = nibbles.end() - trailing_padding_size - trailing_FF_count;
		for (auto i = nibbles.begin() + leading_FF_count; i != end; ++i) {
			const auto nibble = *i;
			if ((nibble & 0x80) == 0) {
				continue;
			}

			if (nibble == 0xff) {
				++FF_count;
				continue;
			}

			flush_FFs();
			append_byte(nibble);
		}
		flush_FFs();
	}

	if (wrapped_FF_are_syncs) {
		append_syncs(trailing_FF_count);
	} else {
		append_FFs(trailing_FF_count);
	}

	return levels;
}

bool a2_nib_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint64_t size;
	if (io.length(size))
		return false;
	if (size != expected_size_35t && size != expected_size_40t)
		return false;

	const auto nr_tracks = size / nibbles_per_track;

	std::vector<uint8_t> nibbles(nibbles_per_track);
	for (unsigned track = 0; track < nr_tracks; ++track) {
		auto const [err, actual] = read_at(io, track * nibbles_per_track, &nibbles[0], nibbles_per_track);
		if (err || actual != nibbles_per_track)
			return false;

		auto levels = generate_levels_from_nibbles(nibbles);
		if (!levels.empty()) {
			generate_track_from_levels(track, 0, levels, 0, image);
		}
	}

	image.set_form_variant(floppy_image::FF_525, floppy_image::SSSD);

	return true;
}


const a2_nib_format FLOPPY_NIB_FORMAT;
