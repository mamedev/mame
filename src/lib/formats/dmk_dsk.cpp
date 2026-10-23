// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*********************************************************************

    formats/dmk_dsk.cpp

    DMK disk images

TODO:
- Add write/format support.
- Honor header write protection and density-independent access (needs core support).
- Check support on other drivers besides MSX.

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

struct disk_header
{
	int tracks;
	int heads;
	int track_size;
	uint8_t flags;

	// Both compact SD images and legacy ignore-density images store FM once.
	int fm_stride() const { return (flags & 0xc0) ? 1 : 2; }
};

bool read_header(util::random_read &io, disk_header &disk)
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
	return TRACK_HEADER_SIZE + (relative < 0 ? relative + length : relative);
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

} // anonymous namespace



dmk_format::dmk_format()
{
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

	std::vector<uint8_t> track_data(disk.track_size);
	for (int track = 0; track < disk.tracks; track++)
	{
		for (int head = 0; head < disk.heads; head++)
		{
			auto const [err, actual] = read_at(io, HEADER_SIZE + (disk.heads * track + head) * disk.track_size, track_data.data(), disk.track_size);
			if (err || actual != disk.track_size || !valid_track(track_data))
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
			std::vector<uint8_t> track_data(disk.track_size);
			std::vector<uint32_t> raw_track_data;

			auto const [err, actual] = read_at(io, HEADER_SIZE + (disk.heads * track + head) * disk.track_size, track_data.data(), disk.track_size);
			if (err || actual != disk.track_size || !valid_track(track_data))
				return false;

			std::vector<int> idams;
			for (int i = 0; i < MAX_IDAMS && get_u16le(&track_data[i * 2]); i++)
				idams.push_back(get_u16le(&track_data[i * 2]) & 0x3fff);

			bool const initial_mfm = idams.empty() ? !util::BIT(disk.flags, 6) : util::BIT(track_data[1], 7);
			std::vector<bool> encoding(disk.track_size, initial_mfm);
			std::vector<uint16_t> special(disk.track_size, 0);
			int const length = disk.track_size - TRACK_HEADER_SIZE;
			int first_sync = idams.empty() ? TRACK_HEADER_SIZE : idams.front();
			std::vector<int> encoding_start;
			int const last_stride = idams.empty() || util::BIT(track_data[(idams.size() - 1) * 2 + 1], 7) ? 1 : fm_stride;
			int iam_begin = idams.empty() ? TRACK_HEADER_SIZE : idams.back() - length + 7 * last_stride;

			// Use unwrapped offsets to describe density regions through index.
			for (unsigned index = 0; index < idams.size(); index++)
			{
				int const offset = idams[index];
				bool const mfm = util::BIT(track_data[index * 2 + 1], 7);
				int const sync = mfm ? sync_count(track_data, offset, 0xa1) : 0;
				int const previous = index ? idams[index - 1] : idams.back() - length;
				// Exact density-change positions are not recorded in DMK.  Switch
				// in the gap before the next ID's sync, without crossing the last ID.
				encoding_start.push_back(std::max(previous + 1, offset - sync - 8));
				if (!index)
					first_sync = offset - sync;
			}

			for (unsigned index = 0; index < idams.size(); index++)
			{
				int const offset = idams[index];
				bool const mfm = util::BIT(track_data[index * 2 + 1], 7);
				int const sync = mfm ? sync_count(track_data, offset, 0xa1) : 0;
				int const next_idam = index + 1 < idams.size() ? idams[index + 1] : idams.front() + length;
				int const stride = mfm ? 1 : fm_stride;
				has_idam = true;
				has_mfm |= mfm;

				int const end = index + 1 < idams.size() ? encoding_start[index + 1] : encoding_start.front() + length;
				for (int i = encoding_start[index]; i < end; i++)
					encoding[track_offset(track_data, i)] = mfm;

				if (mfm)
				{
					for (int i = offset - sync; i < offset; i++)
						special[track_offset(track_data, i)] = 0x4489;
				}
				else
					special[offset] = 0xf57e; // FE clocked with C7

				// The ID occupies seven bytes including CRC.  DAM locations are
				// implicit, so search the following gap up to the next ID, not a
				// fixed gap length.  Do not test the second copy of an FM byte.
				for (int i = offset + 7 * stride; i < next_idam; i += stride)
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
			int const iam_end = idams.empty() ? TRACK_HEADER_SIZE + length - (length % iam_stride) : first_sync;
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
	image.set_variant(single_density ? (disk.heads == 2 ? floppy_image::DSSD : floppy_image::SSSD)
		: (disk.heads == 2 ? floppy_image::DSDD : floppy_image::SSDD));

	return true;
}

const dmk_format FLOPPY_DMK_FORMAT;
