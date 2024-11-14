// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "mfi_dsk.h"

#include "ioprocs.h"

#include <zlib.h>

#include <cstring>
#include <functional>
#include <tuple>


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
  representing magnetic cells.  Bits 0-27 indicate the position,
  delta-packed (e.g. difference with the previous position, starts at
  0), and bits 28-31 the types.  Type can be:

  - 0, MG_F -> Flux orientation change
  - 1, MG_N -> Non-magnetized zone (neutral)
  - 2, MG_D -> Damaged zone, reads as neutral but cannot be changed by writing
  - 3, MG_E -> End of zone

  Tracks data is aligned so that the index pulse is at the start for soft-
  sectored disks. For hard-sectored disks, the sector hole for the first
  sector is at the start and the index hole is half a sector from the end
  of the track.

  The position is the angular position in units of 1/200,000,000th of
  a turn.  A size in such units, not coincidentally at all, is also
  the flyover time in nanoseconds for a perfectly stable 300rpm drive.
  That makes the standard cell size of a MFM 3.5" DD floppy at 2000
  exactly for instance (2us).  Smallest expected cell size is 500 (ED
  density drives).

  An unformatted track is equivalent to a pair (MG_N, 0), (MG_E,
  199999999) but is encoded as zero-size.

  The "track splice" information indicates where to start writing
  if you try to rewrite a physical disk with the data.  Some
  preservation formats encode that information, it is guessed for
  others.  The write track function of fdcs should set it.  The
  representation is the angular position relative to the index, for
  soft-sectored disks, and the first sector hole for hard-sectored
  disks.

  The media type is divided in two parts.  The first half
  indicate the physical form factor, i.e. all medias with that
  form factor can be physically inserted in a reader that handles
  it.  The second half indicates the variants which are usually
  detectable by the reader, such as density and number of sides.

  TODO: big-endian support
*/

const char mfi_format::sign_old[16] = "MESSFLOPPYIMAGE"; // Includes the final \0
const char mfi_format::sign[16]     = "MAMEFLOPPYIMAGE"; // Includes the final \0

mfi_format::mfi_format() : floppy_image_format_t()
{
}

const char *mfi_format::name() const noexcept
{
	return "mfi";
}

const char *mfi_format::description() const noexcept
{
	return "MAME floppy image";
}

const char *mfi_format::extensions() const noexcept
{
	return "mfi";
}

bool mfi_format::supports_save() const noexcept
{
	return true;
}

int mfi_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	header h;
	auto const [err, actual] = read_at(io, 0, &h, sizeof(header));
	if (err || (sizeof(header) != actual))
		return 0;

	if((!memcmp(h.sign, sign, 16) || !memcmp(h.sign, sign_old, 16)) &&
		(h.cyl_count & CYLINDER_MASK) <= 84 &&
		(h.cyl_count >> RESOLUTION_SHIFT) < 3 &&
		h.head_count <= 2 &&
		(!form_factor || !h.form_factor || h.form_factor == form_factor))
		return FIFID_SIGN|FIFID_STRUCT;

	return 0;
}

bool mfi_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	std::error_condition err;
	size_t actual;

	header h;
	std::tie(err, actual) = read_at(io, 0, &h, sizeof(header));
	if(err || (sizeof(header) != actual))
		return false;
	int const resolution = h.cyl_count >> RESOLUTION_SHIFT;
	h.cyl_count &= CYLINDER_MASK;

	image.set_form_variant(h.form_factor, h.variant);

	if(!h.cyl_count)
		return true;

	entry entries[84*2*4];
	std::tie(err, actual) = read_at(io, sizeof(header), &entries, (h.cyl_count << resolution)*h.head_count*sizeof(entry));
	if(err || (((h.cyl_count << resolution)*h.head_count*sizeof(entry)) != actual))
		return false;

	std::function<void (const std::vector<uint32_t> &src, std::vector<uint32_t> &track)> converter;

	if(!memcmp(h.sign, sign, 16)) {
		converter = [] (const std::vector<uint32_t> &src, std::vector<uint32_t> &track) {
			uint32_t ctime = 0;
			for(uint32_t mg : src) {
				ctime += mg & TIME_MASK;
				track.push_back((mg & MG_MASK) | ctime);
			}
		};

	} else {
		converter = [] (const std::vector<uint32_t> &src, std::vector<uint32_t> &track) {
			unsigned int cell_count = src.size();
			uint32_t mg = src[0] & MG_MASK;
			uint32_t wmg = src[cell_count - 1] & MG_MASK;
			if(mg != wmg && (mg == OLD_MG_A || mg == OLD_MG_B) && (wmg == OLD_MG_A || wmg == OLD_MG_B))
				// Flux change at 0, add it
				track.push_back(MG_F | 0);

			uint32_t ctime = 0;
			for(unsigned int i=0; i != cell_count; i++) {
				uint32_t nmg = src[i] & MG_MASK;
				if(nmg == OLD_MG_N || nmg == OLD_MG_D) {
					track.push_back((nmg == OLD_MG_N ? MG_N : MG_D) | ctime);
					ctime += src[i] & TIME_MASK;
					track.push_back(MG_E | (ctime-1));
					nmg = 0xffffffff;
				} else {
					if(mg != 0xffffffff && mg != nmg)
						track.push_back(MG_F | ctime);
					ctime += src[i] & TIME_MASK;
				}
				mg = nmg;
			}
		};
	}

	std::vector<uint8_t> compressed;
	std::vector<uint32_t> uncompressed;

	entry *ent = entries;
	for(unsigned int cyl=0; cyl <= (h.cyl_count - 1) << 2; cyl += 4 >> resolution)
		for(unsigned int head=0; head != h.head_count; head++) {
			image.set_write_splice_position(cyl >> 2, head, ent->write_splice, cyl & 3);

			if(ent->uncompressed_size == 0) {
				// Unformatted track
				image.get_buffer(cyl >> 2, head, cyl & 3).clear();
				ent++;
				continue;
			}

			unsigned int cell_count = ent->uncompressed_size/4;
			compressed.resize(ent->compressed_size);
			uncompressed.resize(cell_count);

			std::tie(err, actual) = read_at(io, ent->offset, &compressed[0], ent->compressed_size); // FIXME: check for errors and premature EOF

			uLongf size = ent->uncompressed_size;
			if(uncompress((Bytef *)uncompressed.data(), &size, &compressed[0], ent->compressed_size) != Z_OK) {
				fprintf(stderr, "fail1\n");
				return false;
			}

			std::vector<uint32_t> &trackbuf = image.get_buffer(cyl >> 2, head, cyl & 3);
			trackbuf.clear();

			converter(uncompressed, trackbuf);
			ent++;
		}

	return true;
}

bool mfi_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	int tracks, heads;
	image.get_actual_geometry(tracks, heads);
	int resolution = image.get_resolution();
	int max_track_size = 0;
	for(int track=0; track <= (tracks-1) << 2; track += 4 >> resolution)
		for(int head=0; head<heads; head++) {
			int tsize = image.get_buffer(track >> 2, head, track & 3).size();
			if(tsize > max_track_size)
					max_track_size = tsize;
		}

	header h;
	entry entries[84*2*4];
	memcpy(h.sign, sign, 16);
	h.cyl_count = tracks | (resolution << RESOLUTION_SHIFT);
	h.head_count = heads;
	h.form_factor = image.get_form_factor();
	h.variant = image.get_variant();

	write_at(io, 0, &h, sizeof(header)); // FIXME: check for errors

	memset(entries, 0, sizeof(entries));

	int pos = sizeof(header) + (tracks << resolution)*heads*sizeof(entry);
	int epos = 0;
	auto precomp = std::make_unique<uint32_t []>(max_track_size);
	auto postcomp = std::make_unique<uint8_t []>(max_track_size*4 + 1000);

	for(int track=0; track <= (tracks-1) << 2; track += 4 >> resolution)
		for(int head=0; head<heads; head++) {
			const std::vector<uint32_t> &buffer = image.get_buffer(track >> 2, head, track & 3);
			int tsize = buffer.size();
			if(!tsize) {
				epos++;
				continue;
			}

			uint32_t ctime = 0;
			for(int i=0; i != tsize; i++) {
				precomp[i] = (buffer[i] & MG_MASK) | ((buffer[i] & TIME_MASK) - ctime);
				ctime = buffer[i] & TIME_MASK;
			}

			uLongf csize = max_track_size*4 + 1000;
			if(compress(postcomp.get(), &csize, (const Bytef *)precomp.get(), tsize*4) != Z_OK)
				return false;

			entries[epos].offset = pos;
			entries[epos].uncompressed_size = tsize*4;
			entries[epos].compressed_size = csize;
			entries[epos].write_splice = image.get_write_splice_position(track >> 2, head, track & 3);
			epos++;

			write_at(io, pos, postcomp.get(), csize); // FIXME: check for errors
			pos += csize;
		}

	write_at(io, sizeof(header), entries, (tracks << resolution)*heads*sizeof(entry)); // FIXME: check for errors
	return true;
}

const mfi_format FLOPPY_MFI_FORMAT;
