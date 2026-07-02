// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    microdrv.cpp

    MAME interface to the Sinclair Microdrive image abstraction code
    (this seems to be QL format specific and hardcoded to a specific
     size, Spectrum might need a different implementation?)

*********************************************************************/

#include "emu.h"
#include "microdrv.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define LOG 1
#define LOG_COMMS 0

#define MDV_SECTOR_COUNT            255
#define MDV_SECTOR_LENGTH           686
#define MDV_IMAGE_LENGTH            (MDV_SECTOR_COUNT * MDV_SECTOR_LENGTH)

#define MDV_OFFSET_HEADER_PREAMBLE  0
#define MDV_OFFSET_HEADER           MDV_OFFSET_HEADER_PREAMBLE + MDV_PREAMBLE_LENGTH
#define MDV_OFFSET_DATA_PREAMBLE    28
#define MDV_OFFSET_DATA             MDV_OFFSET_DATA_PREAMBLE + MDV_PREAMBLE_LENGTH
#define MDV_OFFSET_FILLER           566

// 40us/byte from Minerva sources; both tracks shift one bit per tick, so a
// byte pair (two stream bytes) takes 8 ticks = 80us -> 100kHz per track
#define MDV_BITRATE                 100000

// Physical gaps (erased tape regions) are not stored in QLAY images, they
// have to be synthesized. Durations are in bit ticks (10us each).
#define MDV_GAP_SECTOR_TICKS        480 // 4.8ms inter-sector gap
#define MDV_GAP_BLOCK_TICKS         280 // 2.8ms gap between sector header and block preamble

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// device type definition
DEFINE_DEVICE_TYPE(MICRODRIVE, microdrive_image_device, "microdrive_image", "Sinclair Microdrive")

//-------------------------------------------------
//  microdrive_image_device - constructor
//-------------------------------------------------

microdrive_image_device::microdrive_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	microtape_image_device(mconfig, MICRODRIVE, tag, owner, clock),
	m_write_comms_out(*this),
	m_write_data1_out(*this),
	m_write_data2_out(*this),
	m_write_gap_out(*this)
{
}

//-------------------------------------------------
//  microdrive_image_device - destructor
//-------------------------------------------------

microdrive_image_device::~microdrive_image_device()
{
}


void microdrive_image_device::device_start()
{
	// allocate track buffers
	m_left = std::make_unique<uint8_t[]>(MDV_IMAGE_LENGTH / 2);
	m_right = std::make_unique<uint8_t[]>(MDV_IMAGE_LENGTH / 2);

	// allocate timers
	m_bit_timer = timer_alloc(FUNC(microdrive_image_device::bit_timer), this);
	m_bit_timer->adjust(attotime::zero, 0, attotime::from_hz(MDV_BITRATE));
	m_bit_timer->enable(0);

	m_clk = 0;
	m_comms_in = 0;
	m_comms_out = 0;
	m_gap_ticks = 0;

	// register for state saving
	save_item(NAME(m_clk));
	save_item(NAME(m_comms_in));
	save_item(NAME(m_comms_out));
	save_item(NAME(m_bit_offset));
	save_item(NAME(m_byte_offset));
	save_item(NAME(m_gap_ticks));
}

std::pair<std::error_condition, std::string> microdrive_image_device::call_load()
{
	if (length() != MDV_IMAGE_LENGTH)
		return std::make_pair(image_error::INVALIDLENGTH, std::string());

	for (int i = 0; i < MDV_IMAGE_LENGTH / 2; i++)
	{
		fread(m_left.get() + i, 1);
		fread(m_right.get() + i, 1);
	}

	m_bit_offset = 0;
	m_byte_offset = 0;
	m_clk = 0;
	m_gap_ticks = 0;

	return std::make_pair(std::error_condition(), std::string());
}

void microdrive_image_device::call_unload()
{
	m_bit_timer->enable(false);

	// ejecting while the drive is selected leaves the head over nothing
	if (m_comms_out)
		m_write_gap_out(1);

	if (m_left.get())
		memset(m_left.get(), 0, MDV_IMAGE_LENGTH / 2);

	if (m_right.get())
		memset(m_right.get(), 0, MDV_IMAGE_LENGTH / 2);
}

TIMER_CALLBACK_MEMBER(microdrive_image_device::bit_timer)
{
	// Synthesized gap: the head is over an erased region
	if (m_gap_ticks > 0)
	{
		m_gap_ticks--;
		if (m_gap_ticks == 0)
			m_write_gap_out(0); // End of gap, preamble will follow
		return;
	}

	m_write_data1_out(BIT(m_left[m_byte_offset], 7 - m_bit_offset));
	m_write_data2_out(BIT(m_right[m_byte_offset], 7 - m_bit_offset));

	m_bit_offset++;

	if (m_bit_offset == 8)
	{
		m_bit_offset = 0;
		m_byte_offset++;

		// Infinite loop of the tape
		if (m_byte_offset == MDV_IMAGE_LENGTH / 2)
			m_byte_offset = 0;

		int sector_offset = m_byte_offset % (MDV_SECTOR_LENGTH / 2);

		if (sector_offset == MDV_OFFSET_DATA_PREAMBLE / 2)
		{
			// Gap between the sector header and the block preamble
			m_write_gap_out(1);
			m_gap_ticks = MDV_GAP_BLOCK_TICKS;
		}
		else if (sector_offset == MDV_OFFSET_FILLER / 2)
		{
			// The QLAY filler replaces the erased part between sectors
			// skip it and emit a gap
			m_byte_offset += (MDV_SECTOR_LENGTH - MDV_OFFSET_FILLER) / 2;
			if (m_byte_offset >= MDV_IMAGE_LENGTH / 2)
				m_byte_offset = 0;
			m_write_gap_out(1);
			m_gap_ticks = MDV_GAP_SECTOR_TICKS;
		}
	}
}

void microdrive_image_device::clk_w(int state)
{
	if (LOG_COMMS) logerror("Microdrive '%s' CLK: %u\n", tag(), state);

	if (m_clk && !state) // Falling edge
	{
		m_comms_out = m_comms_in;
		if (LOG) logerror("Microdrive '%s' COMMS OUT: %u\n", tag(), m_comms_out);
		m_write_comms_out(m_comms_out); // daisy chain for drive selection

		if (m_comms_out && is_loaded())
		{
			if (LOG) logerror("Microdrive '%s' motor ON\n", tag());
			m_bit_timer->enable(true);
		}
		else if (m_comms_out && !is_loaded())
		{
			// no cartridge: the head sees a gap
			if (LOG) logerror("Microdrive '%s' selected, no cartridge, gap set\n", tag());
			m_write_gap_out(1);
		}
		else if (!m_comms_out && is_loaded()) {
			if (LOG) logerror("Microdrive '%s' motor OFF\n", tag());
			m_bit_timer->enable(false);
		}
	}
	m_clk = state;
}

void microdrive_image_device::comms_in_w(int state)
{
	if (LOG_COMMS) logerror("Microdrive '%s' COMMS IN: %u\n", tag(), state);
	m_comms_in = state;
}

void microdrive_image_device::erase_w(int state)
{
	if (LOG_COMMS) logerror("Microdrive '%s' ERASE: %u\n", tag(), state);
	m_erase = state;
}

void microdrive_image_device::read_write_w(int state)
{
	if (LOG_COMMS) logerror("Microdrive '%s' READ/WRITE: %u (1=read)\n", tag(), state);
	m_read_write = state;
}

void microdrive_image_device::data1_w(int state)
{
	if (m_comms_out && !m_read_write)
	{
		// TODO
	}
}

void microdrive_image_device::data2_w(int state)
{
	if (m_comms_out && !m_read_write)
	{
		// TODO
	}
}

int microdrive_image_device::data1_r()
{
	int data = 0;
	if (m_comms_out && m_read_write)
	{
		data = BIT(m_left[m_byte_offset], 7 - m_bit_offset);
	}
	return data;
}

int microdrive_image_device::data2_r()
{
	int data = 0;
	if (m_comms_out && m_read_write)
	{
		data = BIT(m_right[m_byte_offset], 7 - m_bit_offset);
	}
	return data;
}
