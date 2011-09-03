#include "emu.h"
#include "mfi_dsk.h"
#include <zlib.h>

/*
  Mess floppy image structure:

  - header with signature, number of cylinders, number of heads.  Min
    track and min head are considered to always be 0.

  - vector of track descriptions, looping on cylinders and sub-lopping
    on heads, each description composed of:
    - offset of the track data in bytes from the start of the file
    - size of the compressed track data in bytes (0 for unformatted)
    - size of the uncompressed track data in bytes (0 for unformatted)

  - track data

  All values are 32-bits lsb first.

  Track data is zlib-compressed independently for each track using the
  simple "compress" function.

  Track data consists of a series of 32-bits lsb-first values
  representing magnetic cells.  Bits 0-27 indicate the sizes, and bits
  28-31 the types.  Type can be:
  - 0, MG_A -> Magnetic orientation A
  - 1, MG_B -> Magnetic orientation B
  - 2, MG_N -> Non-magnetized zone (neutral)
  - 3, MG_D -> Damaged zone, reads as neutral but cannot be changed by writing

  Remember that the fdcs detect transitions, not absolute levels, so
  the actual physical significance of the orientation A and B is
  arbitrary.

  Tracks data is aligned so that the index pulse is at the start,
  whether the disk is hard-sectored or not.

  The size is the angular size in units of 1/200,000,000th of a turn.
  Such a size, not coincidentally at all, is also the flyover time in
  nanoseconds for a perfectly stable 300rpm drive.  That makes the
  standard cell size of a MFM 3.5" DD floppy at 2000 exactly for
  instance (2us).  Smallest expected cell size is 500 (ED density
  drives).

  The sum of all sizes must of course be 200,000,000.

  An unformatted track is equivalent to one big MG_N cell covering a
  whole turn, but is encoded as zero-size.

  TODO: big-endian support, cleanup pll, move it where it belongs.
*/

const char mfi_format::sign[16] = "MESSFLOPPYIMAGE"; // Includes the final \0

mfi_format::mfi_format() : floppy_image_format_t()
{
}

const char *mfi_format::name() const
{
	return "mfi";
}

const char *mfi_format::description() const
{
	return "MESS floppy image";
}

const char *mfi_format::extensions() const
{
	return "mfi";
}

bool mfi_format::supports_save() const
{
	return false;
}

int mfi_format::identify(floppy_image *image)
{
	header h;

	image->image_read(&h, 0, sizeof(header));
	if(memcmp( h.sign, sign, 16 ) == 0 &&
	   h.cyl_count > 0 && h.cyl_count <= 84 &&
	   h.head_count > 0 && h.head_count <= 2)
		return 100;
	return 0;
}

void mfi_format::advance(const UINT32 *trackbuf, UINT32 &cur_cell, UINT32 cell_count, UINT32 time)
{
	if(time >= 200000000) {
		cur_cell = cell_count;
		return;
	}

	while(cur_cell != cell_count-1 && (trackbuf[cur_cell+1] & TIME_MASK) < time)
		cur_cell++;
}

UINT32 mfi_format::get_next_edge(const UINT32 *trackbuf, UINT32 cur_cell, UINT32 cell_count)
{
	if(cur_cell == cell_count)
		return 200000000;
	UINT32 cur_bit = trackbuf[cur_cell] & MG_MASK;
	cur_cell++;
	while(cur_cell != cell_count) {
		UINT32 next_bit = trackbuf[cur_cell] & MG_MASK;
		if(next_bit != cur_bit)
			break;
	}
	return cur_cell == cell_count ? 200000000 : trackbuf[cur_cell] & TIME_MASK;
}

bool mfi_format::load(floppy_image *image)
{
	header h;
	entry entries[84*2];
	image->image_read(&h, 0, sizeof(header));
	image->image_read(&entries, sizeof(header), h.cyl_count*h.head_count*sizeof(entry));
	image->set_meta_data(h.cyl_count, h.head_count, 300, (UINT16)253360);

	UINT32 *trackbuf = 0;
	int trackbuf_size = 0;
	UINT8 *compressed = 0;
	int compressed_size = 0;

	entry *ent = entries;
	for(unsigned int cyl=0; cyl != h.cyl_count; cyl++)
		for(unsigned int head=0; head != h.head_count; head++) {
			if(ent->uncompressed_size == 0) {
				// Unformatted track
				image->set_track_size(cyl, head, 0);
				continue;
			}

			if(ent->compressed_size > compressed_size) {
				if(compressed)
					global_free(compressed);
				compressed_size = ent->compressed_size;
				compressed = global_alloc_array(UINT8, compressed_size);
			}

			if(ent->uncompressed_size > trackbuf_size) {
				if(trackbuf)
					global_free(trackbuf);
				trackbuf_size = ent->uncompressed_size;
				trackbuf = global_alloc_array(UINT32, trackbuf_size/4);
			}

			image->image_read(compressed, ent->offset, ent->compressed_size);

			uLongf size = ent->uncompressed_size;
			if(uncompress((Bytef *)trackbuf, &size, compressed, ent->compressed_size) != Z_OK)
				return true;

			UINT8 *mfm = image->get_buffer(cyl, head);
			image->set_track_size(cyl, head, 16384);
			memset(mfm, 0, 16384);
			int bit = 0;

			// Extract the bits using a quick-n-dirty software pll
			// expecting mfm 2us data.  Eventually the plls will end
			// up in the fdc simulations themselves.

			// Neutral/damaged bits are not really taken into account

			//  Start by turning the cell times into absolute
			//  positions, it's easier to use that way.

			unsigned int cell_count = ent->uncompressed_size/4;
			UINT32 cur_time = 0;
			for(unsigned int i=0; i != cell_count; i++) {
				UINT32 next_cur_time = cur_time + (trackbuf[i] & TIME_MASK);
				trackbuf[i] = (trackbuf[i] & MG_MASK) | cur_time;
				cur_time = next_cur_time;
			}
			if(cur_time != 200000000)
				return true;

			//  Then pll the hell out of the bits

			UINT32 cur_cell = 0;
			UINT32 pll_period = 2000;
			UINT32 pll_phase = 0;
			for(;;) {
				advance(trackbuf, cur_cell, cell_count, pll_phase);
				if(cur_cell == cell_count)
					break;

#if 0
				printf("%09d: (%d, %09d) - (%d, %09d) - (%d, %09d)\n",
					   pll_phase,
					   trackbuf[cur_cell] >> MG_SHIFT, trackbuf[cur_cell] & TIME_MASK,
					   trackbuf[cur_cell+1] >> MG_SHIFT, trackbuf[cur_cell+1] & TIME_MASK,
					   trackbuf[cur_cell+2] >> MG_SHIFT, trackbuf[cur_cell+2] & TIME_MASK);
#endif

				UINT32 next_edge = get_next_edge(trackbuf, cur_cell, cell_count);

				if(next_edge > pll_phase + pll_period) {
					// free run, zero bit
					//					printf("%09d: %4d - Free run\n", pll_phase, pll_period);
					pll_phase += pll_period;
				} else {
					// Transition in the window, one bit, adjust the period

					mfm[bit >> 3] |= 0x80 >> (bit & 7);

					INT32 delta = next_edge - (pll_phase + pll_period/2);
					//					printf("%09d: %4d - Delta = %d\n", pll_phase, pll_period, delta);

					// The deltas should be lowpassed, the amplification factor tuned...
					pll_period += delta/2;
					pll_phase += pll_period;
				}

				bit++;
			}
			image->set_track_size(cyl, head, (bit+7)/8);

			ent++;
		}
			
	return false;
}

const floppy_format_type FLOPPY_MFI_FORMAT = &floppy_image_format_creator<mfi_format>;
