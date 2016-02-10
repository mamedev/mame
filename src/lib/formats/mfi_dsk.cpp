// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include <assert.h>

#include "mfi_dsk.h"
#include <zlib.h>

/*
  Mess floppy image structure:

  - header with signature, number of cylinders, number of heads.  Min
    track and min head are considered to always be 0.  The two top bits
    of the cylinder count is the resolution: 0=tracks, 1=half tracks,
    2=quarter tracks.

  - vector of track descriptions, looping on cylinders with the given
    resolution and sub-lopping on heads, each description composed of:
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

  The "track splice" information indicates where to start writing
  if you try to rewrite a physical disk with the data.  Some
  preservation formats encode that information, it is guessed for
  others.  The write track function of fdcs should set it.  The
  representation is the angular position relative to the index.

  The media type is divided in two parts.  The first half
  indicate the physical form factor, i.e. all medias with that
  form factor can be physically inserted in a reader that handles
  it.  The second half indicates the variants which are usually
  detectable by the reader, such as density and number of sides.

  TODO: big-endian support
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
	return true;
}

int mfi_format::identify(io_generic *io, UINT32 form_factor)
{
	header h;

	io_generic_read(io, &h, 0, sizeof(header));
	if(memcmp( h.sign, sign, 16 ) == 0 &&
		(h.cyl_count & CYLINDER_MASK) <= 84 &&
		(h.cyl_count >> RESOLUTION_SHIFT) < 3 &&
		h.head_count <= 2 &&
		(!form_factor || !h.form_factor || h.form_factor == form_factor))
		return 100;
	return 0;
}

bool mfi_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	header h;
	entry entries[84*2*4];
	io_generic_read(io, &h, 0, sizeof(header));
	int resolution = h.cyl_count >> RESOLUTION_SHIFT;
	h.cyl_count &= CYLINDER_MASK;
	io_generic_read(io, &entries, sizeof(header), (h.cyl_count << resolution)*h.head_count*sizeof(entry));

	image->set_variant(h.variant);

	dynamic_buffer compressed;

	entry *ent = entries;
	for(unsigned int cyl=0; cyl <= (h.cyl_count - 1) << 2; cyl += 4 >> resolution)
		for(unsigned int head=0; head != h.head_count; head++) {
			image->set_write_splice_position(cyl >> 2, head, ent->write_splice, cyl & 3);

			if(ent->uncompressed_size == 0) {
				// Unformatted track
				image->get_buffer(cyl >> 2, head, cyl & 3).clear();
				ent++;
				continue;
			}

			compressed.resize(ent->compressed_size);

			io_generic_read(io, &compressed[0], ent->offset, ent->compressed_size);

			unsigned int cell_count = ent->uncompressed_size/4;
			std::vector<UINT32> &trackbuf = image->get_buffer(cyl >> 2, head, cyl & 3);;
			trackbuf.resize(cell_count);

			uLongf size = ent->uncompressed_size;
			if(uncompress((Bytef *)&trackbuf[0], &size, &compressed[0], ent->compressed_size) != Z_OK)
				return false;

			UINT32 cur_time = 0;
			for(unsigned int i=0; i != cell_count; i++) {
				UINT32 next_cur_time = cur_time + (trackbuf[i] & TIME_MASK);
				trackbuf[i] = (trackbuf[i] & MG_MASK) | cur_time;
				cur_time = next_cur_time;
			}
			if(cur_time != 200000000)
				return false;

			ent++;
		}

	return true;
}

bool mfi_format::save(io_generic *io, floppy_image *image)
{
	int tracks, heads;
	image->get_actual_geometry(tracks, heads);
	int resolution = image->get_resolution();
	int max_track_size = 0;
	for(int track=0; track <= (tracks-1) << 2; track += 4 >> resolution)
		for(int head=0; head<heads; head++) {
			int tsize = image->get_buffer(track >> 2, head, track & 3).size();
			if(tsize > max_track_size)
					max_track_size = tsize;
		}

	header h;
	entry entries[84*2*4];
	memcpy(h.sign, sign, 16);
	h.cyl_count = tracks | (resolution << RESOLUTION_SHIFT);
	h.head_count = heads;
	h.form_factor = image->get_form_factor();
	h.variant = image->get_variant();

	io_generic_write(io, &h, 0, sizeof(header));

	memset(entries, 0, sizeof(entries));

	int pos = sizeof(header) + (tracks << resolution)*heads*sizeof(entry);
	int epos = 0;
	auto precomp = global_alloc_array(UINT32, max_track_size);
	auto postcomp = global_alloc_array(UINT8, max_track_size*4 + 1000);

	for(int track=0; track <= (tracks-1) << 2; track += 4 >> resolution)
		for(int head=0; head<heads; head++) {
			std::vector<UINT32> &buffer = image->get_buffer(track >> 2, head, track & 3);
			int tsize = buffer.size();
			if(!tsize) {
				epos++;
				continue;
			}

			memcpy(precomp, &buffer[0], tsize*4);
			for(int j=0; j<tsize-1; j++)
				precomp[j] = (precomp[j] & floppy_image::MG_MASK) |
					((precomp[j+1] & floppy_image::TIME_MASK) -
						(precomp[j] & floppy_image::TIME_MASK));
			precomp[tsize-1] = (precomp[tsize-1] & floppy_image::MG_MASK) |
				(200000000 - (precomp[tsize-1] & floppy_image::TIME_MASK));

			uLongf csize = max_track_size*4 + 1000;
			if(compress(postcomp, &csize, (const Bytef *)precomp, tsize*4) != Z_OK) {
				global_free_array(precomp);
				global_free_array(postcomp);
				return false;
			}

			entries[epos].offset = pos;
			entries[epos].uncompressed_size = tsize*4;
			entries[epos].compressed_size = csize;
			entries[epos].write_splice = image->get_write_splice_position(track >> 2, head, track & 3);
			epos++;

			io_generic_write(io, postcomp, pos, csize);
			pos += csize;
		}

	io_generic_write(io, entries, sizeof(header), (tracks << resolution)*heads*sizeof(entry));
	global_free_array(precomp);
	global_free_array(postcomp);
	return true;
}

const floppy_format_type FLOPPY_MFI_FORMAT = &floppy_image_format_creator<mfi_format>;
