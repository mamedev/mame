// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h" // logerror
#include "pasti_dsk.h"

// Pasti format supported using the documentation at
// http://www.sarnau.info/atari:pasti_file_format

// That format is an observational format, not a generative one.  In
// other terms, it encodes the raw responses of the WD1772 to the read
// track, read ids and read sectors commands.  So, in order to use it,
// we have to build a physical representation which gives similar
// enough results (read track varies even in the absence of fuzzy
// bits).

pasti_format::pasti_format()
{
}

const char *pasti_format::name() const
{
	return "pasti";
}

const char *pasti_format::description() const
{
	return "Atari PASTI floppy disk image";
}

const char *pasti_format::extensions() const
{
	return "stx";
}

bool pasti_format::supports_save() const
{
	return false;
}

int pasti_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT8 h[16];
	io_generic_read(io, h, 0, 16);

	if(!memcmp(h, "RSY\0\3\0", 6) &&
		(1 || (h[10] >= 80 && h[10] <= 82) || (h[10] >= 160 && h[10] <= 164)))
		return 100;

	return 0;
}

static void hexdump(const UINT8 *d, int s)
{
	for(int i=0; i<s; i+=32) {
		printf("%04x:", i);
		for(int j=i; j<s && j<i+32; j++)
			printf(" %02x", d[j]);
		printf("\n");
	}
}

bool pasti_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	UINT8 fh[16];
	io_generic_read(io, fh, 0, 16);

	dynamic_buffer raw_track;

	int tracks = fh[10];
	int heads = 1+(tracks >= 160);
	tracks /= heads;

	int pos = 16;

	wd_obs obs;

	for(int track=0; track < tracks; track++) {
		for(int head=0; head < heads; head++) {
			UINT8 th[16];
			io_generic_read(io, th, pos, 16);
			int entry_len = th[0] | (th[1] << 8) | (th[2] << 16) | (th[3] << 24);
			int fuzz_len  = th[4] | (th[5] << 8) | (th[6] << 16) | (th[7] << 24);
			int sect      = th[8] | (th[9] << 8);
			int flags     = th[10] | (th[11] << 8);
			int track_len = th[12] | (th[13] << 8);
			int track_num = th[14];
			int flags2    = th[15];

			raw_track.resize(entry_len-16);

			io_generic_read(io, &raw_track[0], pos+16, entry_len-16);

			UINT8 *fuzz = fuzz_len ? &raw_track[16*sect] : 0;
			UINT8 *bdata = fuzz ? fuzz+fuzz_len : &raw_track[16*sect];
			UINT8 *tdata = bdata;

			int syncpos = -1;
			if(flags & 0x0080) {
				syncpos = tdata[0] | (tdata[1] << 8);
				tdata += 2;
			}

			int tsize = 0;
			if(flags & 0x0040) {
				tsize = tdata[0] | (tdata[1] << 8);
				tdata += 2;
			} else
				tdata = 0;

			if(0) {
				printf("Track %2d.%d: el=%d fl=%d sect=%d flags=%04x tlen=%d/%d tnum=%d flags2=%02x sync=%x\n",
						track, head,
						entry_len, fuzz_len, sect, flags, track_len, tsize, track_num, flags2, syncpos);
				hexdump(&raw_track[16*sect], entry_len-16-16*sect);
			}

			if(0 && tdata) {
				hexdump(tdata, tsize);

				for(int i=0; i<tsize-8; i++) {
					if(tdata[i] == 0xa1 && tdata[i+1] == 0xa1 && tdata[i+2] != 0xa1)
						printf("  header %5d: %02x %02x %02x %02x %02x %02x %02x\n",
								i+2,
								tdata[i+2], tdata[i+3], tdata[i+4], tdata[i+5], tdata[i+6], tdata[i+7], tdata[i+8]);
				}
			}

			for(int s=0; s<sect; s++) {
				UINT8 *sh = &raw_track[16*s];
				int s_off   = sh[0] | (sh[1] << 8) | (sh[2] << 16) | (sh[3] << 24);
				int s_pos   = sh[4] | (sh[5] << 8);
				int s_time  = sh[6] | (sh[7] << 8);
				int s_flags = sh[14] | (sh[15] << 8);

				obs.sectors[s].data       = bdata + s_off;
				obs.sectors[s].fuzzy_mask = 0;
				memcpy(obs.sectors[s].id, sh+8, 6);
				obs.sectors[s].time_ratio = s_time ? s_time / 16384.0 : 1;
				obs.sectors[s].position   = s_pos/8-12;
				if(0) {
					printf("  sector %2d: off=%5d pos=%5d [%02x %02x] time=%04x flags=%04x id=%02x.%02x.%02x.%02x.%02x.%02x\n",
							s, s_off, s_pos, tdata ? tdata[s_pos/8] : 0x00, tdata ? tdata[s_pos/8+1] : 0x00, s_time, s_flags,
							sh[8], sh[9], sh[10], sh[11], sh[12], sh[13]);
					hexdump(bdata+s_off, 128<<(sh[11] & 3));
				}
			}

			obs.track_data = tdata;
			obs.track_size = tsize;
			obs.sector_count = sect;

			wd_generate_track_from_observations(track, head, image, obs);

			pos += entry_len;
		}
	}

	image->set_variant(floppy_image::DSDD);
	return true;
}

const floppy_format_type FLOPPY_PASTI_FORMAT = &floppy_image_format_creator<pasti_format>;


void pasti_format::wd_generate_track_from_observations(int track, int head, floppy_image *image, wd_obs &obs)
{
	if(!obs.track_data)
		wd_generate_track_from_sectors_only(track, head, image, obs);
	else
		wd_generate_track_from_sectors_and_track(track, head, image, obs);
}

void pasti_format::wd_generate_unsynced_gap(std::vector<UINT32> &track, const wd_obs &obs, int tstart, int tend, UINT32 cell_size)
{
	for(int i=tstart; i != tend;) {
		unsigned char v = obs.track_data[i];
		int j;
		for(j=i+1; j != tend && obs.track_data[j] == v; j++);
		int size = j-i;
		if(size < 4) {
			mfm_w(track, 8, v, cell_size);
			i++;
			continue;
		}
		if(v == 0xff || v == 0x00)
			v = 0;
		else if(v == 0x4e || v == 0x21 || v == 0x9c || v == 0x42 || v == 0x39 || v == 0x84 || v == 0x72 || v == 0x09 ||
				v == 0xe4 || v == 0x12 || v == 0xc9 || v == 0x24 || v == 0x93 || v == 0x48 || v == 0x27 || v == 0x90)
			v = 0x4e;
		// Grab the transition byte too
		if((v == 0 || v == 0x4e) && j != tend)
			size++;
		for(j=0; j != size; j++)
			mfm_w(track, 8, v, cell_size);
		i += size;
	}
}

void pasti_format::wd_generate_synced_gap(std::vector<UINT32> &track, const wd_obs &obs, int tstart, int tend, UINT32 cell_size)
{
	for(int i = tstart; i != tend; i++) {
		unsigned char v = obs.track_data[i];
		if((v == 0x14 || v == 0xa1 || v == 0xc2) && i+2 < tend && obs.track_data[i+1] == 0xa1 && obs.track_data[i+2] == 0xa1) {
			raw_w(track, 16, 0x4489, cell_size);
			raw_w(track, 16, 0x4489, cell_size);
			raw_w(track, 16, 0x4489, cell_size);
			i += 2;
		} else if(!track.empty() && i != tend-1 && (((v == 0x14 || v == 0xc2) && (track.back() & 0x1f) == 10) || (v == 0xa1 && (track.back() & 0x1f) != 10)))
			raw_w(track, 16, 0x4489, cell_size);
		else if(i != tend-1 && (v == 0x14 || v == 0xc2))
			raw_w(track, 16, 0x5224, cell_size);
		else
			mfm_w(track, 8, v, cell_size);
	}
}

void pasti_format::wd_generate_gap(std::vector<UINT32> &track, const wd_obs &obs, int tstart, int tend, bool synced, UINT32 cell_size_start, UINT32 cell_size_end)
{
	unsigned int spos = track.size();
	if(!synced) {
		int sync = -1;
		for(int i = tstart; sync == -1 && i != tend; i++)
			if(obs.track_data[i] == 0x14 || obs.track_data[i] == 0xa1 || obs.track_data[i] == 0xc2)
				sync = i;
		if(sync == -1)
			sync = tend;
		wd_generate_unsynced_gap(track, obs, tstart, sync, cell_size_start);
		tstart = sync;
	}
	if(tstart != tend)
		wd_generate_synced_gap(track, obs, tstart, tend, cell_size_start);

	if(cell_size_end != cell_size_start) {
		INT32 total_size = 0;
		for(unsigned int i=spos; i != track.size(); i++)
			total_size += track[i] & floppy_image::TIME_MASK;
		INT64 cur_size = 0;
		for(unsigned int i=spos; i != track.size(); i++) {
			cur_size += track[i] & floppy_image::TIME_MASK;
			track[i] = (track[i] & floppy_image::MG_MASK) |
				(cur_size*int(cell_size_end-cell_size_start)/total_size + cell_size_start);
		}
	}
}

void pasti_format::wd_generate_sector_header(std::vector<UINT32> &track, const wd_obs &obs, int sector, int tstart, UINT32 cell_size)
{
	raw_w(track, 16, 0x4489, cell_size);
	raw_w(track, 16, 0x4489, cell_size);
	raw_w(track, 16, 0x4489, cell_size);
	mfm_w(track, 8, obs.track_data[tstart+3], cell_size);
	for(int i=0; i != 6; i++)
		mfm_w(track, 8, obs.sectors[sector].id[i], cell_size);
}

void pasti_format::wd_generate_sector_data(std::vector<UINT32> &track, const wd_obs &obs, int sector, int tstart, UINT32 cell_size)
{
	const wd_sect &s = obs.sectors[sector];
	raw_w(track, 16, 0x4489, cell_size);
	raw_w(track, 16, 0x4489, cell_size);
	raw_w(track, 16, 0x4489, cell_size);
	mfm_w(track, 8, obs.track_data[tstart+3], cell_size);
	for(int i=0; i<128 << (s.id[3] & 3); i++)
		mfm_w(track, 8, s.data[i], cell_size);
	UINT16 crc = calc_crc_ccitt(track, track.size() - (2048 << (s.id[3] & 3)) - 16*4, track.size());
	mfm_w(track, 8, crc >> 8, cell_size);
	mfm_w(track, 8, crc, cell_size);
}

void pasti_format::wd_generate_track_from_sectors_and_track(int track, int head, floppy_image *image, wd_obs &obs)
{
	if(0)
		printf("Track %d head %d sectors %d\n", track, head, obs.sector_count);
	std::vector<UINT32> trackbuf;

	wd_sect_info sect_infos[256];

	if(0)
		hexdump(obs.track_data, obs.track_size);
	map_sectors_in_track(obs, sect_infos);

	if(0)
		for(int i=0; i != obs.sector_count; i++) {
			wd_sect_info *s = sect_infos + i;
			printf("%2d: %5d-%5d %c %02x %02x|%02x %02x %5d-%5d %c %02x %02x|%02x %02x %f\n",
					i,
					s->hstart, s->hend, s->hsynced ? 'S' : '-',
					s->hstart == -1 ? 0 : obs.track_data[s->hstart],
					s->hstart == -1 ? 0 : obs.track_data[s->hstart+1],
					s->hend == -1 ? 0 : obs.track_data[s->hend],
					s->hend == -1 ? 0 : obs.track_data[s->hend+1],

					s->dstart, s->dend, s->dsynced ? 'S' : '-',
					s->dstart == -1 ? 0 : obs.track_data[s->dstart],
					s->dstart == -1 ? 0 : obs.track_data[s->dstart+1],
					s->dend == -1 ? 0 : obs.track_data[s->dend],
					s->dend == -1 ? 0 : obs.track_data[s->dend+1],

					obs.sectors[i].time_ratio);
		}

	if(obs.sector_count) {
		wd_sect_info *last = sect_infos + obs.sector_count-1;
		if(last->dend != -1 && last->dend < last->hstart) {
			osd_printf_error("pasti: Unsupported sector header/data over index, track %d head %d\n", track, head);
			return;
		}

		UINT32 cell_size = UINT32(obs.sectors[0].time_ratio * 1000+0.5);
		wd_generate_gap(trackbuf, obs, 0, sect_infos[0].hstart, false, cell_size, cell_size);

		for(int i=0; i != obs.sector_count; i++) {
			wd_sect_info *s = sect_infos + i;
			if(i+1 != obs.sector_count) {
				if(s->dstart != -1 && s[1].hstart < s->dend) {
					osd_printf_error("pasti: Unsupported sector overlap, track %d head %d\n", track, head);
					return;
				}
			}

			UINT32 ncell_size =  UINT32(obs.sectors[i+1 != obs.sector_count ? i+1 : 0].time_ratio * 1000+0.5);

			wd_generate_sector_header(trackbuf, obs, i, s->hstart, cell_size);

			if(s->dstart == -1) {
				if(i == obs.sector_count-1)
					wd_generate_gap(trackbuf, obs, s->hend, obs.track_size, s->hsynced, cell_size, ncell_size);
				else
					wd_generate_gap(trackbuf, obs, s->hend, s[1].hstart, s->hsynced, cell_size, ncell_size);
			} else {
				wd_generate_gap(trackbuf, obs, s->hend, s->dstart, s->hsynced, cell_size, cell_size);
				wd_generate_sector_data(trackbuf, obs, i, s->dstart, cell_size);
				if(i == obs.sector_count-1)
					wd_generate_gap(trackbuf, obs, s->dend, obs.track_size, s->dsynced, cell_size, ncell_size);
				else
					wd_generate_gap(trackbuf, obs, s->dend, s[1].hstart, s->dsynced, cell_size, ncell_size);
			}
			cell_size = ncell_size;
		}

	} else
		wd_generate_gap(trackbuf, obs, 0, obs.track_size, false, 1000, 1000);

	generate_track_from_levels(track, head, trackbuf, 0, image);
}

void pasti_format::wd_generate_track_from_sectors_only(int track, int head, floppy_image *image, wd_obs &obs)
{
	if(0) {
		printf("Track %d head %d sectors %d\n", track, head, obs.sector_count);
		for(int i=0; i != obs.sector_count; i++) {
			const wd_sect &s = obs.sectors[i];

			printf("%2d: %02x.%02x.%02x.%02x.%02x.%02x %d %f\n",
					i,
					s.id[0], s.id[1], s.id[2], s.id[3], s.id[4], s.id[5],
					s.position,
					s.time_ratio);
			if(track==10 && i==0)
				hexdump(s.data, 512);
		}
	}

	std::vector<UINT32> tdata;
	for(int i=0; i != obs.sector_count; i++) {
		const wd_sect &s = obs.sectors[i];
		if(i+1 != obs.sector_count && obs.sectors[i+1].position < s.position+10+44+4+(128 << (s.id[3] & 3))) {
			osd_printf_error("pasti: Unsupported sector data sharing, track %d head %d\n", track, head);
			return;
		}
		if(tdata.size() >> 4 < s.position - 12) {
			int count = s.position - 12 - (tdata.size() >> 4);
			if(count & 1) {
				mfm_w(tdata, 8, 0x4e);
				count--;
			}
			for(int j=0; j<count; j+=2)
				mfm_w(tdata, 8, 0x4e);
		}
		if(tdata.size() < s.position*16) {
			int count = s.position - (tdata.size() >> 4);
			if(count & 1) {
				mfm_w(tdata, 8, 0x00);
				count--;
			}
			for(int j=0; j<count; j+=2)
				mfm_w(tdata, 8, 0x00);
		}
		raw_w(tdata, 16, 0x4489);
		raw_w(tdata, 16, 0x4489);
		raw_w(tdata, 16, 0x4489);
		mfm_w(tdata, 8, 0xfe);
		for(int j=0; j<6; j++)
			mfm_w(tdata, 8, s.id[j]);

		if(!s.data)
			continue;

		for(int j=0; j<22; j++)
			mfm_w(tdata, 8, 0x4e);
		for(int j=0; j<12; j++)
			mfm_w(tdata, 8, 0x00);

		raw_w(tdata, 16, 0x4489);
		raw_w(tdata, 16, 0x4489);
		raw_w(tdata, 16, 0x4489);
		mfm_w(tdata, 8, 0xfb);
		for(int j=0; j<128 << (s.id[3] & 3); j++)
			mfm_w(tdata, 8, s.data[j]);
		UINT16 crc = calc_crc_ccitt(tdata, tdata.size() - (2048 << (s.id[3] & 3)) - 16*4, tdata.size());
		mfm_w(tdata, 8, crc >> 8);
		mfm_w(tdata, 8, crc);
	}

	int count = (100015 - tdata.size()) >> 16;
	for(int i=0; i<count; i++)
		mfm_w(tdata, 8, 0x4e);

	generate_track_from_levels(track, head, tdata, 0, image);
}

UINT16 pasti_format::byte_to_mfm(UINT8 data, bool context)
{
	static const UINT8 expand[32] = {
		0xaa, 0xa9, 0xa4, 0xa5, 0x92, 0x91, 0x94, 0x95, 0x4a, 0x49, 0x44, 0x45, 0x52, 0x51, 0x54, 0x55,
		0x2a, 0x29, 0x24, 0x25, 0x12, 0x11, 0x14, 0x15, 0x4a, 0x49, 0x44, 0x45, 0x52, 0x51, 0x54, 0x55,
	};

	return (expand[(data >> 4) | (context ? 16 : 0)] << 8) | expand[data & 0x1f];
}

void pasti_format::match_mfm_data(wd_obs &obs, int tpos, const UINT8 *data, int size, UINT8 context, int &bcount, int &tend, bool &synced)
{
	UINT16 shift = byte_to_mfm(context, true);
	int bc = 0;
	int bc_phase = 0;
	int bi = 0;
	UINT8 dbyte = 0;
	bool ds_phase = false;
	UINT16 inshift = byte_to_mfm(data[bi++], shift & 1);
	synced = false;
	for(;;) {
		int bit = (inshift >> (15-bc)) & 1;
		shift = (shift << 1) | bit;
		if(ds_phase)
			dbyte = (dbyte << 1) | bit;
		//      printf("  %04x %02x (%04x %02x)\n", shift, dbyte, inshift, data[bi-1]);
		ds_phase = !ds_phase;
		bc++;
		if(shift == 0x4489 || shift == 0x5224) {
			bc_phase = 16-bc;
			ds_phase = false;
		}
		if(!((bc_phase + bc) & 15)) {
			//          printf("dbyte=%02x data=%02x in=%02x bc=%d bc_shift=%d ds_phase=%s\n", dbyte, obs.track_data[tpos], data[bi-1], bc, bc_phase, ds_phase ? "on" : "off");
			if(dbyte != obs.track_data[tpos++]) {
				bcount = bi-1;
				tend = tpos-1;
				return;
			}
			if(tpos == obs.track_size)
				tpos = 0;
			if(bi == size) {
				bcount = bi;
				tend = tpos;
				synced = bc_phase == 0;
				return;
			}
		}
		if(bc == 16) {
			inshift = byte_to_mfm(data[bi++], shift & 1);
			bc = 0;
		}
	}
}

void pasti_format::match_raw_data(wd_obs &obs, int tpos, const UINT8 *data, int size, UINT8 context, int &bcount, int &tend)
{
	tend = tpos;
	for(bcount=0; bcount != size; bcount++) {
		if(data[bcount] != obs.track_data[tend])
			return;
		tend++;
		if(tend == obs.track_size)
			tend = 0;
	}
}

UINT16 pasti_format::calc_crc(const UINT8 *data, int size, UINT16 crc1)
{
	UINT32 crc = crc1;
	for(int i=0; i<size; i++) {
		crc = (crc << 8) ^ (*data++ << 16);
		if(crc & 0x800000) crc ^= 0x881080;
		if(crc & 0x400000) crc ^= 0x440840;
		if(crc & 0x200000) crc ^= 0x220420;
		if(crc & 0x100000) crc ^= 0x110210;
		if(crc & 0x080000) crc ^= 0x088108;
		if(crc & 0x040000) crc ^= 0x044084;
		if(crc & 0x020000) crc ^= 0x022042;
		if(crc & 0x010000) crc ^= 0x011021;
	}
	return crc;
}

void pasti_format::map_sectors_in_track(wd_obs &obs, wd_sect_info *sect_infos)
{
	for(int i=0; i != obs.sector_count; i++) {
		sect_infos[i].hstart = -1;
		sect_infos[i].hend = -1;
		sect_infos[i].dstart = -1;
		sect_infos[i].dend = -1;
		sect_infos[i].hsynced = false;
		sect_infos[i].dsynced = false;
	}

	const UINT8 *tdata = obs.track_data;
	int tsize = obs.track_size;

	for(int i=0; i != tsize; i++)
		if(tdata[i] == 0xa1 &&
			tdata[(i+1) % tsize] == 0xa1 &&
			(tdata[(i+2) % tsize] == 0xfe ||
			tdata[(i+2) % tsize] == 0xff)) {
			UINT8 hbyte = tdata[(i+2) % tsize];
			int hpos = (i+3) % tsize;
			int j;
			bool synced = false;
			int bcount=0, tend=0;
			int best_bcount=0, best_j=0;
			for(j=0; j != obs.sector_count; j++) {
				match_mfm_data(obs, hpos, obs.sectors[j].id, 6, hbyte, bcount, tend, synced);
				if(bcount > best_bcount) {
					best_bcount = bcount;
					best_j = j;
				}
				if(bcount == 6)
					break;
				match_raw_data(obs, hpos, obs.sectors[j].id, 6, hbyte, bcount, tend);
				if(bcount > best_bcount) {
					best_bcount = bcount;
					best_j = j;
				}
				if(bcount == 6) {
					synced = true;
					break;
				}
			}

			j = best_j;
			if(best_bcount < 4) {
				if(0)
					printf("sector header at %x no match [%02x %02x %02x %02x %02x %02x]\n", i,
							tdata[hpos], tdata[hpos+1], tdata[hpos+2], tdata[hpos+3], tdata[hpos+4], tdata[hpos+5]);
			} else {
				if(0)
					printf("sector header at %x matches %d [%02x %02x %02x %02x %02x %02x] [%02x] - %d %s\n", i, j,
							tdata[hpos], tdata[hpos+1], tdata[hpos+2], tdata[hpos+3], tdata[hpos+4], tdata[hpos+5], tdata[tend],
							obs.sectors[j].position, synced ? "synced" : "unsynced");

				sect_infos[j].hstart  = hpos - 4;
				sect_infos[j].hend    = tend + (6-bcount);
				sect_infos[j].hsynced = synced;

				int dpos = -1;
				if(obs.sectors[j].data)
					for(int j=35; j<45; j++) {
						if(tdata[(hpos+j) % tsize] == 0xa1 &&
							tdata[(hpos+j+1) % tsize] == 0xa1 &&
							tdata[(hpos+j+2) % tsize] >= 0xfa &&
							tdata[(hpos+j+2) % tsize] <= 0xfd) {
							dpos = (hpos+j+3) % tsize;
							break;
						}
					}
				if(dpos != -1) {
					int bcount2, tend2;
					UINT8 dhbyte = tdata[(dpos+tsize-1) % tsize];
					int ssize = 128 << (obs.sectors[j].id[3] & 3);
					match_mfm_data(obs, dpos, obs.sectors[j].data, ssize, dhbyte, bcount, tend, synced);
					if(bcount < ssize) {
						match_raw_data(obs, dpos, obs.sectors[j].data, ssize, dhbyte, bcount2, tend2);
						if(bcount2 > bcount) {
							bcount = bcount2;
							tend = tend2;
							if(bcount == ssize)
								synced = true;
						}
					}
					UINT16 crc = calc_crc(obs.sectors[j].data, ssize, calc_crc(tdata+((dpos+tsize-1) % tsize), 1, 0xcdb4));
					if(synced && tdata[tend] == (crc >> 8) && tdata[(tend+1) % tsize] == (crc & 0xff)) {
						tend = (tend+2) % tsize;
						bcount += 2;
					}

					if(0)
						printf("  associated data at %d, match %d [%02x %02x %02x] %04x, %s\n", dpos, bcount, tdata[tend], tdata[tend+1], tdata[tend+2], crc, synced ? "synced" : "unsynced");

					sect_infos[j].dstart  = dpos - 4;
					sect_infos[j].dend    = tend + (ssize+2-bcount);
					sect_infos[j].dsynced = synced;

				}
			}
		}

}
