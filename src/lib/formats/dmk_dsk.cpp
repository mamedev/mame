// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,tim lindner
/*********************************************************************

    formats/dmk_dsk.cpp

    DMK disk images

Missing features and preservation limits:

- Header write protection (byte 0) is not enforced or retained on save.
  The format load API supplies a floppy_image, but no way to set the
  drive's write-protect state.  Enforcing this requires core support;
  merely preserving the flag would not prevent emulated writes.
- Density-independent access (option bit 7) is not emulated.  Loading
  honors its single-copy FM storage convention, but the resulting flux
  still has a definite encoding.  Making either controller density read
  the same sector requires support outside this format implementation.
- Compact FM output (option bit 6) and legacy single-copy FM output are
  not generated.  The writer normalizes to doubled FM bytes so ordinary
  FM/MFM tracks share one timing grid.  Compact input remains supported;
  retaining its storage policy would need additional writer logic.
- DEC RX02 (extension bit 5) is not implemented.  It needs FM headers,
  modified-MFM data decoding/encoding, different field lengths, and
  special handling of FD/F9 DAMs in both reader and writer.  Recognizing
  ordinary FM/MFM marks alone is insufficient; this is format work,
  not a requirement for core changes.
- The writer implements standard IBM/WD fields with 128-1024 byte data
  lengths (size code masked to two bits).  RX02, non-IBM WD1771 lengths,
  alternate premarks, extra fields, and damaged-clock phase repair need
  additional decoder logic.  CRC bytes are preserved, but CRC validity
  does not guide field state.
- Exact gap density boundaries, arbitrary missing clocks, weak bits,
  flux timing variations, and write-splice positions cannot be retained
  in ordinary DMK byte data and IDAM pointers.  The reader reconstructs
  clocks and estimates density changes; the writer uses recognizable
  marks/gaps and replaces ambiguous F8-FB bytes while awaiting a DAM.
  Saving is consequently not a bit-exact preservation operation.
- Half/quarter tracks and ED output are rejected: DMK has no subtrack
  addressing, and a full ED track exceeds the supported 14-bit offsets.
  More than 64 IDAMs or a track larger than 0x3fff bytes also fails
  rather than silently discarding sectors or wrapping pointers.
- REAL disk specification files are unsupported: they describe host
  drives rather than track contents.

*********************************************************************/

#include "dmk_dsk.h"

#include "coretmpl.h"
#include "ioprocs.h"
#include "multibyte.h"

#include <algorithm>


namespace {

constexpr int HEADER_SIZE = 16;
constexpr int TRACK_HEADER_SIZE = 0x80;
constexpr int MAX_IDAMS = TRACK_HEADER_SIZE / 2;

bool valid_track(const std::vector<uint8_t> &data)
{
	int previous_offset = TRACK_HEADER_SIZE - 1;
	bool terminated = false;
	for (int i = 0; i < MAX_IDAMS; i++)
	{
		uint16_t const entry = get_u16le(&data[i * 2]);
		if (!entry)
		{
			terminated = true;
			continue;
		}
		int const offset = entry & 0x3fff;
		if (terminated || offset <= previous_offset || offset >= data.size() || data[offset] != 0xfe)
			return false;
		previous_offset = offset;
	}
	return true;
}

// Wrap within the track data, never into the IDAM pointer table.
int track_offset(const std::vector<uint8_t> &data, int offset)
{
	int const length = int(data.size()) - TRACK_HEADER_SIZE;
	int const relative = (offset - TRACK_HEADER_SIZE) % length;
	return TRACK_HEADER_SIZE + (relative < 0 ? (relative + length) : relative);
}

// DMK retains data bytes, not individual clock bits.  Only change the clocks
// of sync bytes actually present, including preambles crossing index.
int sync_count(const std::vector<uint8_t> &data, int offset, uint8_t value)
{
	int count = 0;
	int const limit = std::min(3, int(data.size()) - TRACK_HEADER_SIZE - 1);
	while (count < limit && data[track_offset(data, offset - count - 1)] == value)
		count++;
	return count;
}

// Recover the standard six-byte FM zero sync, including both stored copies.
// A longer zero run may belong to a damaged preceding MFM data field; its
// bytes must not be reclassified as FM merely because they decode as zero.
int fm_sync_count(const std::vector<uint8_t> &data, int offset, int stride, int begin)
{
	int count = 0;
	while (count < 6 * stride && offset - count - stride >= begin)
	{
		bool zero = true;
		for (int i = 1; i <= stride; i++)
			zero &= data[track_offset(data, offset - count - i)] == 0;
		if (!zero)
			break;
		count += stride;
	}
	return count;
}

uint32_t wide_fm(uint16_t val)
{
	uint32_t res = 0;
	for (int i = 15; i >= 0; i--) {
		res |= (util::BIT(val, i) << (i * 2 + 1));
	}
	return res;
}

uint32_t data_to_wide_fm(uint8_t val)
{
	uint16_t res = 0xaaaa;  // clock
	for (int i = 7; i >= 0; i--) {
		res |= (util::BIT(val, i) << i * 2);      // data
	}
	return wide_fm(res);
}

// Decode on the common MFM timing grid.  Looking ahead for
// missing-clock marks lets us select density and byte phase before emitting
// the mark.  Never infer FM from transition spacing alone.
bool decode_track(const std::vector<bool> &bits, bool initial_mfm, std::vector<uint8_t> &data, std::vector<int> &dams)
{
	int const length = bits.size();
	if (length < 64)
		return false;
	auto raw = [&bits, length](int position, int count) -> uint64_t
	{
		uint64_t value = 0;
		for (int i = 0; i < count; i++)
			value = (value << 1) | bits[((position + i) % length + length) % length];
		return value;
	};

	data.assign(TRACK_HEADER_SIZE, 0);
	dams.clear();
	int leading_dam = 0;
	std::vector<uint16_t> idams;
	bool mfm = initial_mfm;
	int next = -length;
	int protected_until = -length;
	int id_byte = -1;
	int size_code = 0;
	bool awaiting_dam = false;
	int splice_until = -length;
	std::vector<bool> fm_sync(length, false);

	// One warm-up revolution establishes density and field context across
	// index, and locates FM zero preambles before any bytes are saved.  CRC
	// bytes are copied verbatim, including deliberately bad CRCs.
	uint32_t pattern = raw(-length, 32);
	for (int position = -length; position < length; pattern = (pattern << 1) | raw(position + 32, 1), position++)
	{
		bool mark = false;
		// A mark establishes the phase of the preceding zero bytes as well
		// as the mark itself.  Select FM at their start on the output pass,
		// before a previous MFM phase can turn them into AA bytes and emit
		// part of the address mark as an extra byte.
		if (fm_sync[(position < 0) ? (position + length) : position]
			&& position >= protected_until && position >= splice_until)
		{
			mfm = false;
			next = position;
		}
		if (position >= splice_until)
		{
			if (pattern == 0x44894489 || pattern == 0x52245224 ||
				(position >= protected_until && pattern == 0x92549254))
			{
				mfm = true;
				next = position;
			}
			if ((!mfm || position >= protected_until) && raw(position - 4, 4) == 8)
			{
				switch (pattern)
				{
				case 0xaa2a2a88: // FC / D7
				case 0xaa222aa8: // FE / C7
				case 0xaa222888: // F8 / C7
				case 0xaa22288a: // F9 / C7
				case 0xaa2228a8: // FA / C7
				case 0xaa2228aa: // FB / C7
					if (position < 0)
					{
						int sync = position;
						int const begin = std::max(protected_until, splice_until);
						while (sync - 32 >= begin && raw(sync - 32, 32) == 0x88888888)
							sync -= 32;
						if (sync < position)
							fm_sync[sync + length] = true;
					}
					mfm = false;
					next = position;
					mark = true;
					break;
				}
			}
		}
		if (position != next)
			continue;

		int const width = mfm ? 16 : 32;
		uint8_t value = 0;
		for (int bit = 0; bit < 8; bit++)
			value = (value << 1) | raw(position + (mfm ? (2 * bit + 1) : (4 * bit + 2)), 1);
		if (mfm)
		{
			uint64_t const preamble = raw(position - 32, 32);
			mark = (preamble == 0x44894489 && (value == 0xfe || (value >= 0xf8 && value <= 0xfb))) ||
				(preamble == 0x52245224 && value == 0xfc);
		}

		bool const in_id = id_byte >= 0;
		bool const idam = mark && value == 0xfe;
		bool const dam = mark && awaiting_dam && value >= 0xf8 && value <= 0xfb;
		if (idam)
		{
			id_byte = 0;
			awaiting_dam = true;
			protected_until = position + 19 * width;
		}
		else if (dam)
		{
			id_byte = -1;
			awaiting_dam = false;
			protected_until = position + ((128 << (size_code & 3)) + 3) * width;
			splice_until = protected_until + 32;
		}
		else if (id_byte >= 0)
		{
			if (++id_byte == 4)
				size_code = value;
			if (id_byte == 6)
				id_byte = -1;
		}

		// DMK readers locate DAMs implicitly.  Hide ordinary gap bytes which
		// could otherwise be mistaken for a missing-clock DAM.
		if (awaiting_dam && !in_id && !mark && value >= 0xf8 && value <= 0xfb)
			value = 0xf0;

		// Double FM bytes, including a byte straddling index.  Only its
		// second half belongs at the start of the saved revolution.
		if (position >= 0 || (!mfm && position < 0 && position + 16 >= 0))
		{
			if (idam && position >= 0)
			{
				if (idams.size() == MAX_IDAMS || data.size() > 0x3ffe)
					return false;
				idams.emplace_back(uint16_t(data.size()) | (mfm ? 0x8000 : 0));
				dams.push_back(0);
			}
			if (dam && position >= 0)
			{
				if (dams.empty())
					leading_dam = data.size();
				else
					dams.back() = data.size();
			}
			data.emplace_back(value);
			if (!mfm && position >= 0 && position + 16 < length)
				data.emplace_back(value);
		}
		next = position + width;
	}
	if (data.size() > 0x3fff)
		return false;
	if (leading_dam && !dams.empty())
		dams.back() = leading_dam;
	for (unsigned i = 0; i < idams.size(); i++)
		put_u16le(&data[2 * i], idams[i]);
	return true;
}

} // anonymous namespace



dmk_format::dmk_format()
{
}


bool dmk_format::read_header(util::random_read &io, disk_header &disk) const
{
	uint64_t size;
	if (io.length(size))
		return false;

	uint8_t header[HEADER_SIZE];
	auto const [err, actual] = read_at(io, 0, header, HEADER_SIZE);
	if (err || actual != HEADER_SIZE)
		return false;

	if (header[0] != 0x00 && header[0] != 0xff)
		return false;

	// Native images only; REAL disk specification files are not track dumps.
	for (int i = 5; i < HEADER_SIZE; i++)
		if (header[i])
			return false;

	disk.tracks = header[1];
	disk.heads = util::BIT(header[4], 4) ? 1 : 2;
	disk.track_size = get_u16le(&header[2]);
	disk.flags = header[4];
	// Retain support for extended track lengths fitting the 14-bit offsets.
	if (disk.track_size <= TRACK_HEADER_SIZE || disk.track_size > 0x3fff)
		return false;

	return size == HEADER_SIZE + uint64_t(disk.tracks) * disk.heads * disk.track_size;
}

bool dmk_format::read_track(util::random_read &io, const disk_header &disk, int track, int head,
	std::vector<uint8_t> &data, std::vector<int> &dams) const
{
	data.resize(disk.track_size);
	dams.clear();
	auto const [err, actual] = read_at(io, HEADER_SIZE + (disk.heads * track + head) * disk.track_size, data.data(), disk.track_size);
	return !err && actual == disk.track_size;
}


const char *dmk_format::name() const noexcept
{
	return "dmk";
}


const char *dmk_format::description() const noexcept
{
	return "DMK disk image";
}


const char *dmk_format::extensions() const noexcept
{
	return "dmk";
}


int dmk_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	disk_header disk;
	if (!read_header(io, disk))
		return 0;

	std::vector<uint8_t> track_data;
	std::vector<int> dams;
	for (int track = 0; track < disk.tracks; track++)
	{
		for (int head = 0; head < disk.heads; head++)
		{
			if (!read_track(io, disk, track, head, track_data, dams) || !valid_track(track_data))
				return 0;
		}
	}
	return FIFID_HINT | FIFID_SIZE;
}


bool dmk_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	disk_header disk;
	if (!read_header(io, disk))
		return false;

	int max_tracks, max_heads;
	image.get_maximal_geometry(max_tracks, max_heads);
	if (disk.tracks > max_tracks || disk.heads > max_heads)
		return false;

	int const fm_stride = disk.fm_stride();
	bool has_idam = false;
	bool has_mfm = false;

	for (int track = 0; track < disk.tracks; track++)
	{
		for (int head = 0; head < disk.heads; head++)
		{
			std::vector<uint8_t> track_data;
			std::vector<int> dams;
			std::vector<uint32_t> raw_track_data;

			if (!read_track(io, disk, track, head, track_data, dams) || !valid_track(track_data))
				return false;

			std::vector<int> idams;
			for (int i = 0; i < MAX_IDAMS && get_u16le(&track_data[i * 2]); i++)
				idams.emplace_back(get_u16le(&track_data[i * 2]) & 0x3fff);

			bool const initial_mfm = idams.empty() ? !util::BIT(disk.flags, 6) : util::BIT(track_data[1], 7);
			std::vector<bool> encoding(disk.track_size, initial_mfm);
			std::vector<uint16_t> special(disk.track_size, 0);
			int const length = disk.track_size - TRACK_HEADER_SIZE;
			int first_sync = idams.empty() ? TRACK_HEADER_SIZE : idams.front();
			std::vector<int> encoding_start;
			int const last_stride = idams.empty() || util::BIT(track_data[(idams.size() - 1) * 2 + 1], 7) ? 1 : fm_stride;
			int iam_begin = idams.empty() ? TRACK_HEADER_SIZE : (idams.back() - length + 7 * last_stride);

			// Use unwrapped offsets to describe density regions through index.
			for (unsigned index = 0; index < idams.size(); index++)
			{
				int const offset = idams[index];
				bool const mfm = util::BIT(track_data[index * 2 + 1], 7);
				int const previous = index ? idams[index - 1] : (idams.back() - length);
				bool const previous_mfm = util::BIT(track_data[((index + idams.size() - 1) % idams.size()) * 2 + 1], 7);

				// Early versions of this writer emitted a phase-shifted first
				// FM sync as twelve AA bytes and a stray F5 before the doubled
				// FE.  Recover only this specific leading-gap signature at an
				// MFM-to-FM boundary; do not normalize arbitrary nonzero syncs.
				if (!index && !mfm && previous_mfm && fm_stride == 2 && offset - 14 > previous + 7)
				{
					bool damaged_sync = track_data[track_offset(track_data, offset - 14)] == 0xff
						&& track_data[track_offset(track_data, offset - 1)] == 0xf5
						&& track_data[track_offset(track_data, offset + 1)] == 0xfe;
					for (int i = 2; i <= 13; i++)
						damaged_sync &= track_data[track_offset(track_data, offset - i)] == 0xaa;
					if (damaged_sync)
						for (int i = 1; i <= 12; i++)
							track_data[track_offset(track_data, offset - i)] = 0;
				}

				int const sync = mfm ? sync_count(track_data, offset, 0xa1)
					: fm_sync_count(track_data, offset, fm_stride, previous + 7 * (previous_mfm ? 1 : fm_stride));
				// Exact density-change positions are not recorded.  Switch at
				// the recognizable sync, where the writer can recover the same
				// density and phase.  An arbitrary margin before it makes gap
				// bytes alternate between FM and MFM on successive saves.
				encoding_start.emplace_back(std::max(previous + 1, offset - sync));
				if (!index)
					first_sync = offset - sync;
			}

			for (unsigned index = 0; index < idams.size(); index++)
			{
				int const offset = idams[index];
				bool const mfm = util::BIT(track_data[index * 2 + 1], 7);
				int const sync = mfm ? sync_count(track_data, offset, 0xa1) : 0;
				int const next_idam = (index + 1 < idams.size()) ? idams[index + 1] : (idams.front() + length);
				int const stride = mfm ? 1 : fm_stride;
				has_idam = true;
				has_mfm |= mfm;

				int const end = (index + 1 < idams.size()) ? encoding_start[index + 1] : (encoding_start.front() + length);
				for (int i = encoding_start[index]; i < end; i++)
					encoding[track_offset(track_data, i)] = mfm;

				if (mfm)
				{
					for (int i = offset - sync; i < offset; i++)
						special[track_offset(track_data, i)] = 0x4489;
				}
				else
					special[offset] = 0xf57e; // FE clocked with C7

				// DMK leaves DAM locations implicit: search from the end of the
				// seven-byte ID through the next ID, stepping over FM copies.
				// Derived formats can supply exact locations, including absent
				// fields, nested sectors, and fields crossing index.
				int dam_begin = offset + 7 * stride;
				if (!dams.empty())
				{
					dam_begin = dams[index];
					if (dam_begin && dam_begin < offset)
						dam_begin += length;
				}
				int const dam_end = dams.empty() ? next_idam : (dam_begin ? dam_begin + 1 : 0);
				for (int i = dam_begin; i < dam_end; i += stride)
				{
					int const dam = track_offset(track_data, i);
					if (track_data[dam] < 0xf8 || track_data[dam] > 0xfb)
						continue;
					int const dam_sync = mfm ? sync_count(track_data, dam, 0xa1) : 0;
					if (mfm && dam_sync < 2)
						continue;
					if (mfm)
					{
						for (int j = i - dam_sync; j < i; j++)
							special[track_offset(track_data, j)] = 0x4489;
					}
					else
					{
						static constexpr uint16_t marks[] = { 0xf56a, 0xf56b, 0xf56e, 0xf56f };
						special[dam] = marks[track_data[dam] - 0xf8];
					}
					if (index + 1 == idams.size())
					{
						// The leading gap may follow a data field crossing index.
						// Keep the IAM search past its standard WD data field/CRC.
						int const size_code = track_data[track_offset(track_data, offset + 4 * stride)];
						int const sector_size = 128 << (size_code & 3);
						iam_begin = i + (sector_size + 3) * stride - length;
					}
					break;
				}
			}

			// IAM encoding follows the first sector, not the FM storage flags.
			// Step backwards in logical bytes so doubled FM selects the first FC.
			int const iam_stride = initial_mfm ? 1 : fm_stride;
			int const iam_end = idams.empty() ? (TRACK_HEADER_SIZE + length - (length % iam_stride)) : first_sync;
			for (int i = iam_end - iam_stride; i >= iam_begin; i -= iam_stride)
			{
				int const iam = track_offset(track_data, i);
				if (track_data[iam] != 0xfc)
					continue;
				int iam_sync = 0;
				if (initial_mfm)
				{
					iam_sync = sync_count(track_data, iam, 0xc2);
					if (iam_sync < 2)
						continue;
					for (int j = i - iam_sync; j < i; j++)
						special[track_offset(track_data, j)] = 0x5224;
				}
				else
					special[iam] = 0xf77a; // FC clocked with D7
				// The leading gap can belong to the last sector's density until
				// the IAM establishes the first sector's encoding across index.
				for (int j = i - iam_sync; j < iam_end; j++)
					encoding[track_offset(track_data, j)] = initial_mfm;
				break;
			}

			// Start at a known byte boundary.  An FM byte split by index must be
			// encoded once in full, then split into its two halves when rotating.
			int const origin = track_offset(track_data, first_sync);
			int index_cell = 0;
			for (int position = 0; position < length; )
			{
				int const offset = track_offset(track_data, origin + position);
				if (offset == TRACK_HEADER_SIZE)
					index_cell = int(raw_track_data.size());
				if (encoding[offset])
				{
					if (special[offset])
						raw_w(raw_track_data, 16, special[offset]);
					else
						mfm_w(raw_track_data, 8, track_data[offset]);
					position++;
				}
				else
				{
					int count = std::min(fm_stride, length - position);
					int const next = track_offset(track_data, offset + 1);
					// A mark or density change can realign doubled FM bytes.
					if (count == 2 && (encoding[next] || special[next]))
						count = 1;
					if (count == 2 && next == TRACK_HEADER_SIZE)
						index_cell = int(raw_track_data.size()) + 16;
					uint32_t const word = special[offset] ? wide_fm(special[offset]) : data_to_wide_fm(track_data[offset]);
					int const bits = count == fm_stride ? 32 : 16;
					raw_w(raw_track_data, bits, word >> (32 - bits));
					position += count;
				}
			}

			// Only bit 6 selects compact-SD track timing.  Bit 7 also changes FM
			// storage stride, but must not stretch a normal MFM track to SD size.
			// Put compact storage's unused time in the gap before the first ID,
			// rather than inserting it into a field which may cross index.
			if (util::BIT(disk.flags, 6))
			{
				int remaining = (disk.track_size - TRACK_HEADER_SIZE) * 32 - int(raw_track_data.size());
				while (remaining > 0)
				{
					if (encoding[track_offset(track_data, origin - 1)])
					{
						mfm_w(raw_track_data, 8, 0x4e);
						remaining -= 16;
					}
					else
					{
						int const bits = std::min(remaining, 32);
						raw_w(raw_track_data, bits, data_to_wide_fm(0xff) >> (32 - bits));
						remaining -= bits;
					}
				}
			}

			// Ordinary MFM clocks at the encoding origin depend on the final
			// data bit of the revolution, not an implicit preceding zero.
			if (encoding[origin] && !special[origin])
			{
				bool const clock = !(util::BIT(track_data[origin], 7) || (raw_track_data.back() & floppy_image::MG_MASK) == MG_1);
				raw_track_data[0] = (clock ? MG_1 : MG_0) | (raw_track_data[0] & floppy_image::TIME_MASK);
			}
			std::rotate(raw_track_data.begin(), raw_track_data.begin() + index_cell, raw_track_data.end());
			generate_track_from_levels(track, head, raw_track_data, 0, image);
		}
	}

	// Storage size is independent of the encoding actually present on disk.
	bool const single_density = has_idam ? !has_mfm : util::BIT(disk.flags, 6);
	image.set_variant(single_density ? ((disk.heads == 2) ? floppy_image::DSSD : floppy_image::SSSD)
		: ((disk.heads == 2) ? floppy_image::DSDD : floppy_image::SSDD));

	return true;
}

bool dmk_format::supports_save() const noexcept
{
	return true;
}

bool dmk_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	int tracks, heads;
	image.get_actual_geometry(tracks, heads);
	if (!tracks || tracks > 255 || !heads || heads > 2 || image.get_resolution())
		return false;

	bool const initial_mfm = image.get_variant() != floppy_image::SSSD && image.get_variant() != floppy_image::DSSD;
	int cell_size = (image.get_form_factor() == floppy_image::FF_8) ? 1200 : 2000;
	if (image.get_variant() == floppy_image::DSHD)
		cell_size = (image.get_form_factor() == floppy_image::FF_35) ? 1000 : 1200;
	if (image.get_variant() == floppy_image::DSED)
		return false; // A full ED revolution cannot fit DMK's 14-bit offsets.

	std::vector<decoded_track> data(tracks * heads);
	for (int track = 0; track < tracks; track++)
		for (int head = 0; head < heads; head++)
		{
			auto &buffer = data[track * heads + head];
			if (!decode_track(generate_bitstream_from_track(track, head, cell_size, image), initial_mfm, buffer.data, buffer.dams))
				return false;
		}

	return write_image(io, tracks, heads, initial_mfm, data);
}


bool dmk_format::write_image(util::random_read_write &io, int tracks, int heads, bool initial_mfm,
	std::vector<decoded_track> &data) const
{
	size_t track_size = TRACK_HEADER_SIZE;
	for (auto const &track : data)
		track_size = std::max(track_size, track.data.size());

	uint8_t header[HEADER_SIZE] = { 0 };
	header[1] = tracks;
	put_u16le(&header[2], track_size);
	header[4] = (heads == 1) ? 0x10 : 0;
	auto const [header_error, header_actual] = write_at(io, 0, header, sizeof(header));
	if (header_error || header_actual != sizeof(header))
		return false;
	for (unsigned i = 0; i < data.size(); i++)
	{
		data[i].data.resize(track_size, initial_mfm ? 0x4e : 0xff);
		auto const [err, actual] = write_at(io, HEADER_SIZE + uint64_t(i) * track_size, data[i].data.data(), track_size);
		if (err || actual != track_size)
			return false;
	}
	return true;
}

const dmk_format FLOPPY_DMK_FORMAT;
