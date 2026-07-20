// license:BSD-3-Clause
// copyright-holders:Curt Coder, Sylvain Glaize
/*********************************************************************

    microdrv.cpp

    MAME interface to the Sinclair Microdrive image abstraction code
    (this seems to be QL format specific and hardcoded to a specific
     size, Spectrum might need a different implementation?)

*********************************************************************/

#include "emu.h"
#include "microdrv.h"

#include <vector>

#define LOG_COMMS (1U << 1)   // per-line chatter: CLK, COMMS IN, ERASE, READ/WRITE

// #define VERBOSE (LOG_GENERAL | LOG_COMMS)
#include "logmacro.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

namespace {
	constexpr int MDV_SECTOR_COUNT           = 255;
	constexpr int MDV_SECTOR_LENGTH          = 686;
	constexpr int MDV_IMAGE_LENGTH           = MDV_SECTOR_COUNT * MDV_SECTOR_LENGTH;

	// QLAY image format: sector byte layout
	[[maybe_unused]]
	constexpr int MDV_OFFSET_HEADER_PREAMBLE = 0;   // 10x00 + 2xFF (unused, only for reference)
	constexpr int MDV_OFFSET_HEADER          = 12;  // sflag, sno, name[10], rand[2], csum[2]
	constexpr int MDV_OFFSET_BLOCK_PREAMBLE  = 28;  // 10x00 + 2xFF
	constexpr int MDV_OFFSET_BLOCK_HEADER    = 40;  // fno, bno, csum[2]
	constexpr int MDV_OFFSET_DATA_PREAMBLE   = 44;  // 6x00 + 2xFF
	constexpr int MDV_OFFSET_DATA            = 52;  // 512 data + csum[2]
	[[maybe_unused]]
	constexpr int MDV_OFFSET_FILLER          = 566; // (unused, only for reference)

	// record sizes on tape (after the preamble sync)
	constexpr int MDV_HEADER_LENGTH          = 16;  // sflag, sno, name[10], rand[2], csum[2]
	constexpr int MDV_BLOCK_HEADER_LENGTH    = 4;   // fno, bno, csum[2]
	constexpr int MDV_DATA_LENGTH            = 514; // 512 data + csum[2]

	// 40us/byte deduced from Minerva ROM sources
	// Both tracks shift one bit per tick, so a byte pair takes 8 ticks
	// 80us -> 100kHz per track
	constexpr int MDV_BITRATE                = 100000;

	// Tape timeline in byte-pair positions.
	// QLAY images store no erased regions, they have to be reconstructed
	// One byte pair is 80us
	constexpr int MDV_PAIRS_HEADER           = 14;  // QLAY bytes 0-27
	constexpr int MDV_PAIRS_BLOCK_GAP        = 35;  // 2.8ms erased gap between header and block
	constexpr int MDV_PAIRS_BLOCK            = 269; // QLAY bytes 28-565
	constexpr int MDV_PAIRS_SECTOR_GAP       = 60;  // 4.8ms erased gap (filler)
	constexpr int MDV_PAIRS_PER_SECTOR       = MDV_PAIRS_HEADER + MDV_PAIRS_BLOCK_GAP + MDV_PAIRS_BLOCK + MDV_PAIRS_SECTOR_GAP;
	constexpr int MDV_TAPE_PAIRS             = MDV_SECTOR_COUNT * MDV_PAIRS_PER_SECTOR;
} // anonymous namespace

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// device type definition
DEFINE_DEVICE_TYPE(MICRODRIVE, microdrive_image_device, "microdrive_image", "Sinclair Microdrive")

microdrive_image_device::microdrive_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	microtape_image_device(mconfig, MICRODRIVE, tag, owner, clock),
	m_write_comms_out(*this),
	m_write_data1_out(*this),
	m_write_data2_out(*this),
	m_write_gap_out(*this),
	m_read_tx_pair(*this, 0)
{
}

void microdrive_image_device::device_start()
{
	// allocate the tape timeline
	m_left = std::make_unique<uint8_t[]>(MDV_TAPE_PAIRS);
	m_right = std::make_unique<uint8_t[]>(MDV_TAPE_PAIRS);
	m_erased = std::make_unique<uint8_t[]>(MDV_TAPE_PAIRS);
	memset(m_erased.get(), 1, MDV_TAPE_PAIRS); // no tape == all erased

	// allocate timers
	m_bit_timer = timer_alloc(FUNC(microdrive_image_device::bit_timer), this);
	m_bit_timer->adjust(attotime::zero, 0, attotime::from_hz(MDV_BITRATE));
	m_bit_timer->enable(false);

	m_clk = 0;
	m_comms_in = 0;
	m_comms_out = 0;
	m_erase = 0;
	m_read_write = 0;
	m_bit_offset = 0;
	m_byte_pair_offset = 0;
	m_gap_level = -1;
	m_dirty = false;

	// register for state saving
	save_item(NAME(m_clk));
	save_item(NAME(m_comms_in));
	save_item(NAME(m_comms_out));
	save_item(NAME(m_erase));
	save_item(NAME(m_read_write));
	save_item(NAME(m_bit_offset));
	save_item(NAME(m_byte_pair_offset));
	save_item(NAME(m_gap_level));
	save_item(NAME(m_dirty));
	save_pointer(NAME(m_left), MDV_TAPE_PAIRS);
	save_pointer(NAME(m_right), MDV_TAPE_PAIRS);
	save_pointer(NAME(m_erased), MDV_TAPE_PAIRS);
}

//-------------------------------------------------
//  tape_from_image
//  Loads a QLAY format image on a internal tape timeline
//  Reconstructs the GAPs
//-------------------------------------------------

void microdrive_image_device::tape_from_image(const uint8_t *image)
{
	for (int sector = 0; sector < MDV_SECTOR_COUNT; sector++)
	{
		const uint8_t *sec = image + sector * MDV_SECTOR_LENGTH;
		const int base = sector * MDV_PAIRS_PER_SECTOR;

		// sector is unformatted/no data by default
		memset(m_left.get() + base, 0, MDV_PAIRS_PER_SECTOR);
		memset(m_right.get() + base, 0, MDV_PAIRS_PER_SECTOR);
		memset(m_erased.get() + base, 1, MDV_PAIRS_PER_SECTOR);

		// if no preamble on the header, it's considered unformatted
		if (sec[10] != 0xff || sec[11] != 0xff)
			continue;

		// copy header
		int pos = base;
		for (int i = 0; i < MDV_PAIRS_HEADER; i++, pos++)
		{
			m_left[pos] = sec[2 * i];
			m_right[pos] = sec[2 * i + 1];
			m_erased[pos] = 0;
		}

		// copy data
		pos += MDV_PAIRS_BLOCK_GAP;

		// a slot with a header but no block sync is header-only tape:
		// keep the block region erased rather than loading dead zeros
		if (sec[MDV_OFFSET_BLOCK_PREAMBLE + 10] != 0xff || sec[MDV_OFFSET_BLOCK_PREAMBLE + 11] != 0xff)
			continue;

		for (int i = 0; i < MDV_PAIRS_BLOCK; i++, pos++)
		{
			m_left[pos] = sec[MDV_OFFSET_BLOCK_PREAMBLE + 2 * i];
			m_right[pos] = sec[MDV_OFFSET_BLOCK_PREAMBLE + 2 * i + 1];
			m_erased[pos] = 0;
		}
	}
}

// Helper functions for tape_to_image

bool read_tape_record_header(uint8_t *image, const std::vector<uint8_t> &tape_data,
	int &slot, int &next_slot, size_t &p)
{
	if (p + MDV_HEADER_LENGTH > tape_data.size() || next_slot >= MDV_SECTOR_COUNT) {
	    return false;
	}

	slot = next_slot;
	next_slot += 1;

	uint8_t *sector = image + slot * MDV_SECTOR_LENGTH;
	sector[10] = 0xff;
	sector[11] = 0xff;
	memcpy(sector + MDV_OFFSET_HEADER, tape_data.data() + p, MDV_HEADER_LENGTH);
	p += MDV_HEADER_LENGTH;

	return true;
}

void fill_image_data_record(uint8_t *image, const uint8_t *data_header, const uint8_t *data, int slot)
{
	uint8_t *sector = image + slot * MDV_SECTOR_LENGTH;
	sector[MDV_OFFSET_BLOCK_PREAMBLE + 10] = 0xff;
	sector[MDV_OFFSET_BLOCK_PREAMBLE + 11] = 0xff;
	memcpy(sector + MDV_OFFSET_BLOCK_HEADER, data_header, MDV_BLOCK_HEADER_LENGTH);
	sector[MDV_OFFSET_DATA_PREAMBLE + 6] = 0xff;
	sector[MDV_OFFSET_DATA_PREAMBLE + 7] = 0xff;
	memcpy(sector + MDV_OFFSET_DATA, data, MDV_DATA_LENGTH);
};


bool read_tape_record_data(uint8_t *image, const std::vector<uint8_t> &tape_data,
                           const int &slot, size_t &p, std::vector<uint8_t> &orphan)
{
	if (p + MDV_BLOCK_HEADER_LENGTH > tape_data.size()) {
		return false;
	}

	// keep data header pointer, it's copied only if a valid data is found
	const uint8_t *data_header = tape_data.data() + p;
	p += MDV_BLOCK_HEADER_LENGTH;

	// sync on the inner (smaller) preamble
	size_t d = p;
	while (d + 1 < tape_data.size() && tape_data[d] == 0x00) {
		d++;
	}

	if (d + 1 < tape_data.size() && tape_data[d] == 0xff && tape_data[d + 1] == 0xff)
	{
		d += 2;
		if (d + MDV_DATA_LENGTH > tape_data.size()) {
			return true; // it seems ROM can produce header only sectors (orphans?)
		}
		if (slot >= 0)
		{
			fill_image_data_record(image, data_header, tape_data.data() + d, slot);
		}
		else if (orphan.empty())
		{
			// a data record arrived before any header at start of tape
			// keep it aside to pair it later with the last header on tape
			orphan.assign(data_header, data_header + MDV_BLOCK_HEADER_LENGTH);
			orphan.insert(orphan.end(), tape_data.data() + d, tape_data.data() + d + MDV_DATA_LENGTH);
		}

		p = d + MDV_DATA_LENGTH;
	}
	// no data record found: leave p after the block header
	// and resync on the next preamble
	return true;
}

bool read_tape_record(uint8_t *image, const std::vector<uint8_t> &tape_data,
	int &slot, int &next_slot, size_t &p, std::vector<uint8_t> &orphan)
{
	if (p >= tape_data.size())
	{
		return false;
	}

	if (tape_data[p] == 0xff)
	{
		if (!read_tape_record_header(image, tape_data, slot, next_slot, p))
		{
			return false;
		}
	}
	else if (!read_tape_record_data(image, tape_data, slot, p, orphan))
	{
		return false;
	}
	return true;
}

//-------------------------------------------------
//  tape_to_image
//  Reconstruct a QLAY format image from the internal tape timeline
//  Data written by the ROM are not at specific positions on the tape
//  So the tape is scanned, mimicking the way the ROM finds the data
//  (using the erased parts (GAPs) and preambles.
//-------------------------------------------------

void microdrive_image_device::tape_to_image(uint8_t *image) const
{
	memset(image, 0, MDV_IMAGE_LENGTH);

	// synchronize on the first not erased part and store the origin
	// (because it's an infinite tape)
	int origin_on_tape = 0;
	for (int i = 0; i < MDV_TAPE_PAIRS; i++)
	{
		if (m_erased[i])
		{
			origin_on_tape = i;
			break;
		}
	}

	int slot = -1; // QLAY slot of the last sector header found (tape order)
	int next_slot = 0;

	// As the tape is circular, a data block might arrive before the first
	// detected header. This data will be set into orphan and associated to
	// the last header read.
	std::vector<uint8_t> orphan;

	int tape_index = 0;
	while (tape_index < MDV_TAPE_PAIRS)
	{
		// forward to the next non-erased run
		while (tape_index < MDV_TAPE_PAIRS && m_erased[(origin_on_tape + tape_index) % MDV_TAPE_PAIRS])
		{
			tape_index++;
		}

		// read the bytes from the two tracks into a flat buffer
		std::vector<uint8_t> flat_data;
		flat_data.reserve(MDV_PAIRS_PER_SECTOR * 2); // most probable size of the sector
		while (tape_index < MDV_TAPE_PAIRS && !m_erased[(origin_on_tape + tape_index) % MDV_TAPE_PAIRS])
		{
			const int pos = (origin_on_tape + tape_index) % MDV_TAPE_PAIRS;
			flat_data.push_back(m_left[pos]);
			flat_data.push_back(m_right[pos]);
			tape_index++;
		}

		// parse the records from the flat data buffer
		size_t p = 0;
		while (p + 2 < flat_data.size())
		{
			// search preamble: zeros then the FF/FF sync pair
			if (flat_data[p] == 0x00 && flat_data[p + 1] == 0xff && flat_data[p + 2] == 0xff)
			{
				p += 3; // point to the record
				if (read_tape_record(image, flat_data, slot, next_slot, p, orphan))
					break;
			}
			else
			{
				p++;
			}
		}
	}

	if (!orphan.empty() && next_slot > 0)
	{
		fill_image_data_record(image, orphan.data(), orphan.data() + MDV_BLOCK_HEADER_LENGTH, next_slot - 1);
	}
}

//-------------------------------------------------
//  save_image - write the tape back to the file
//-------------------------------------------------

void microdrive_image_device::save_image()
{
	std::vector<uint8_t> image(MDV_IMAGE_LENGTH);
	tape_to_image(image.data());
	fseek(0, SEEK_SET);
	fwrite(image.data(), MDV_IMAGE_LENGTH);
	LOG("image saved\n");
}

std::pair<std::error_condition, std::string> microdrive_image_device::call_load()
{
	if (length() != MDV_IMAGE_LENGTH)
	{
		return std::make_pair(image_error::INVALIDLENGTH, std::string());
	}

	std::vector<uint8_t> image(MDV_IMAGE_LENGTH);
	if (fread(image.data(), MDV_IMAGE_LENGTH) != MDV_IMAGE_LENGTH)
	{
		return std::make_pair(std::errc::io_error, std::string());
	}

	tape_from_image(image.data());

	m_bit_offset = 0;
	m_byte_pair_offset = 0;
	m_clk = 0;
	m_gap_level = -1;
	m_dirty = false;

	return std::make_pair(std::error_condition(), std::string());
}

std::pair<std::error_condition, std::string> microdrive_image_device::call_create(int format_type, util::option_resolution *format_options)
{
	const std::vector<uint8_t> image(MDV_IMAGE_LENGTH, 0);
	if (fwrite(image.data(), MDV_IMAGE_LENGTH) != MDV_IMAGE_LENGTH)
	{
		return std::make_pair(std::errc::io_error, std::string());
	}

	// a blank image loads as a fully unformatted tape
	tape_from_image(image.data());

	m_bit_offset = 0;
	m_byte_pair_offset = 0;
	m_clk = 0;
	m_gap_level = -1;
	m_dirty = false;

	return std::make_pair(std::error_condition(), std::string());
}

void microdrive_image_device::call_unload()
{
	m_bit_timer->enable(false);

	if (m_dirty && !is_readonly())
	{
		save_image();
	}
	m_dirty = false;

	// ejecting while the drive is selected leaves the head over nothing
	if (m_comms_out)
	{
		m_write_gap_out(1);
	}

	if (m_erased)
	{
		memset(m_erased.get(), 1, MDV_TAPE_PAIRS);
	}
	if (m_left)
	{
		memset(m_left.get(), 0, MDV_TAPE_PAIRS);
	}
	if (m_right)
	{
		memset(m_right.get(), 0, MDV_TAPE_PAIRS);
	}
}

TIMER_CALLBACK_MEMBER(microdrive_image_device::bit_timer)
{
	if (m_read_write)
	{
		// write (technically, it's erase + write because the erase
		// head is ahead of writing head, here we don't care)
		// every 8 bits, gets a byte paire from zx8302 and write
		// it on tape.
		if (m_bit_offset == 0)
		{
			const uint16_t pair = m_read_tx_pair();
			m_left[m_byte_pair_offset] = pair >> 8;
			m_right[m_byte_pair_offset] = pair & 0xff;
			m_erased[m_byte_pair_offset] = 0;
			m_dirty = true;
		}
	}
	else if (m_erase)
	{
		// erase-only
		m_left[m_byte_pair_offset] = 0;
		m_right[m_byte_pair_offset] = 0;
		m_erased[m_byte_pair_offset] = 1;
		m_dirty = true;
	}
	else
	{
		// read mode
		const int gap = m_erased[m_byte_pair_offset] ? 1 : 0;
		if (gap != m_gap_level)
		{
			m_gap_level = gap;
			m_write_gap_out(gap);
		}
		if (!gap)
		{
			m_write_data1_out(BIT(m_left[m_byte_pair_offset], 7 - m_bit_offset));
			m_write_data2_out(BIT(m_right[m_byte_pair_offset], 7 - m_bit_offset));
		}
	}

	m_bit_offset++;

	if (m_bit_offset == 8)
	{
		m_bit_offset = 0;
		m_byte_pair_offset++;

		// Infinite loop of the tape
		if (m_byte_pair_offset == MDV_TAPE_PAIRS)
			m_byte_pair_offset = 0;
	}
}

void microdrive_image_device::clk_w(int state)
{
	LOGMASKED(LOG_COMMS, "CLK: %u\n", state);

	if (m_clk && !state) // Falling edge
	{
		m_comms_out = m_comms_in;
		LOG("COMMS OUT: %u\n", m_comms_out);
		m_write_comms_out(m_comms_out);
		if (m_comms_out && is_loaded())
		{
			LOG("motor ON\n");
			m_gap_level = -1; // force a gap emission on the first tick
			m_bit_timer->enable(true);
		}
		else if (m_comms_out && !is_loaded())
		{
			// no cartridge: the head sees a gap
			LOG("selected, no cartridge, gap set\n");
			m_write_gap_out(1);
		}
		else if (!m_comms_out && is_loaded()) {
			LOG("motor OFF\n");
			m_bit_timer->enable(false);
		}
	}
	m_clk = state;
}

void microdrive_image_device::comms_in_w(int state)
{
	LOGMASKED(LOG_COMMS, "COMMS IN: %u\n", state);
	m_comms_in = state;
}

void microdrive_image_device::erase_w(int state)
{
	LOGMASKED(LOG_COMMS, "ERASE: %u\n", state);
	m_erase = state;
}

void microdrive_image_device::read_write_w(int state)
{
	LOGMASKED(LOG_COMMS, "READ/WRITE: %u (1=write)\n", state);
	m_read_write = state;
}

int microdrive_image_device::data1_r() const
{
	int data = 0;
	if (m_comms_out && !m_read_write && !m_erased[m_byte_pair_offset])
	{
		data = BIT(m_left[m_byte_pair_offset], 7 - m_bit_offset);
	}
	return data;
}

int microdrive_image_device::data2_r() const
{
	int data = 0;
	if (m_comms_out && !m_read_write && !m_erased[m_byte_pair_offset])
	{
		data = BIT(m_right[m_byte_pair_offset], 7 - m_bit_offset);
	}
	return data;
}
