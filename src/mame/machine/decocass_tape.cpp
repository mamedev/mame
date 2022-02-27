// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, David Haywood
/***********************************************************************

    DECO Cassette System machine

 ***********************************************************************/

#include "emu.h"
#include "machine/decocass_tape.h"

#include "cpu/m6502/m6502.h"
#include "cpu/mcs48/mcs48.h"

#include <sstream>

#define LOG_CASSETTE_STATE      0

/***************************************************************************
    CASSETTE DEVICE INTERFACE
***************************************************************************/

/* number of tape clock pulses per second */
#define TAPE_CLOCKRATE                  4800
#define TAPE_CLOCKS_PER_BIT             2
#define TAPE_CLOCKS_PER_BYTE            (8 * TAPE_CLOCKS_PER_BIT)
#define TAPE_MSEC_TO_CLOCKS(x)          ((x) * TAPE_CLOCKRATE / 1000)


/* Note on a tapes leader-BOT-data-EOT-trailer format:
 * A cassette has a transparent piece of tape on both ends,
 * leader and trailer. And data tapes also have BOT and EOT
 * holes, shortly before the leader and trailer.
 * The holes and clear tape are detected using a photo-resitor.
 * When rewinding, the BOT/EOT signal will show a short
 * pulse and if rewind continues a constant high signal later.
 * The specs say the holes are "> 2ms" in length.
 */

/* duration of the clear LEADER (and trailer) of the tape */
#define REGION_LEADER_START_CLOCK       0
#define REGION_LEADER_LEN_CLOCKS        TAPE_MSEC_TO_CLOCKS(1000)   /* 1s */
#define REGION_LEADER_END_CLOCK         (REGION_LEADER_START_CLOCK+REGION_LEADER_LEN_CLOCKS)

/* duration of the GAP between leader and BOT/EOT */
#define REGION_LEADER_GAP_START_CLOCK   REGION_LEADER_END_CLOCK
#define REGION_LEADER_GAP_LEN_CLOCKS    TAPE_MSEC_TO_CLOCKS(1500)   /* 1.5s */
#define REGION_LEADER_GAP_END_CLOCK     (REGION_LEADER_GAP_START_CLOCK+REGION_LEADER_GAP_LEN_CLOCKS)

/* duration of BOT/EOT holes */
#define REGION_BOT_START_CLOCK          REGION_LEADER_GAP_END_CLOCK
#define REGION_BOT_LEN_CLOCKS           TAPE_MSEC_TO_CLOCKS(2.5)    /* 0.0025s */
#define REGION_BOT_END_CLOCK            (REGION_BOT_START_CLOCK+REGION_BOT_LEN_CLOCKS)

/* gap between BOT/EOT and first/last data block */
#define REGION_BOT_GAP_START_CLOCK      REGION_BOT_END_CLOCK
#define REGION_BOT_GAP_LEN_CLOCKS       TAPE_MSEC_TO_CLOCKS(300)    /* 300ms */
#define REGION_BOT_GAP_END_CLOCK        (REGION_BOT_GAP_START_CLOCK+REGION_BOT_GAP_LEN_CLOCKS)

static uint16_t tape_crc16_byte(uint16_t crc, uint8_t data);

DEFINE_DEVICE_TYPE(DECOCASS_TAPE, decocass_tape_device, "decocass_tape", "DECO Cassette Tape")

decocass_tape_device::decocass_tape_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DECOCASS_TAPE, tag, owner, clock),
	m_tape_timer(nullptr),
	m_speed(0),
	m_region(REGION_LEADER),
	m_bitnum(0),
	m_clockpos(0),
	m_numclocks(0),
	m_tape_data(*this, DEVICE_SELF)
{
	for (auto & elem : m_crc16)
	elem = 0;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void decocass_tape_device::device_start()
{
	int curblock, offs, numblocks;

	/* fetch the data pointer */
	m_tape_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(decocass_tape_device::tape_clock_callback), this));
	if (!m_tape_data.found())
		return;

	/* scan for the first non-empty block in the image */
	for (offs = m_tape_data.bytes() - 1; offs >= 0; offs--)
		if (m_tape_data[offs] != 0)
			break;
	numblocks = ((offs | 0xff) + 1) / 256;
	assert(numblocks < std::size(m_crc16));

	/* compute the total length */
	m_numclocks = REGION_BOT_GAP_END_CLOCK + numblocks * BYTE_BLOCK_TOTAL * 16 + REGION_BOT_GAP_END_CLOCK;

	/* compute CRCs for each block */
	for (curblock = 0; curblock < numblocks; curblock++)
	{
		uint16_t crc = 0;
		int testval;

		/* first CRC the 256 bytes of data */
		for (offs = 256 * curblock; offs < 256 * curblock + 256; offs++)
			crc = tape_crc16_byte(crc, m_tape_data[offs]);

		/* then find a pair of bytes that will bring the CRC to 0 (any better way than brute force?) */
		for (testval = 0; testval < 0x10000; testval++)
			if (tape_crc16_byte(tape_crc16_byte(crc, testval >> 8), testval) == 0)
				break;
		m_crc16[curblock] = testval;
	}

	/* register states */
	save_item(NAME(m_speed));
	save_item(NAME(m_bitnum));
	save_item(NAME(m_clockpos));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void decocass_tape_device::device_reset()
{
	/* turn the tape off */
	change_speed(0);
}

/*-------------------------------------------------
    tape_crc16_byte - accumulate 8 bits worth of
    CRC data
-------------------------------------------------*/

static uint16_t tape_crc16_byte(uint16_t crc, uint8_t data)
{
	int bit;

	for (bit = 0; bit < 8; bit++)
	{
		crc = (crc >> 1) | (crc << 15);
		crc ^= (data << 7) & 0x80;
		if (crc & 0x80)
			crc ^= 0x0120;
		data >>= 1;
	}
	return crc;
}

/*-------------------------------------------------
    tape_describe_state - create a string that
    describes the state of the tape
-------------------------------------------------*/

std::string decocass_tape_device::describe_state()
{
	std::ostringstream buffer;
	util::stream_format(buffer, "{%9d=", m_clockpos);

	switch (m_region)
	{
	case REGION_LEADER:
		buffer << "LEAD}";
		break;
	case REGION_LEADER_GAP:
		buffer << "LGAP}";
		break;
	case REGION_BOT:
		buffer << "BOT }";
		break;
	case REGION_BOT_GAP:
		buffer << "BGAP}";
		break;
	case REGION_TRAILER:
		buffer << "TRLR}";
		break;
	case REGION_TRAILER_GAP:
		buffer << "TGAP}";
		break;
	case REGION_EOT:
		buffer << "EOT }";
		break;
	case REGION_EOT_GAP:
		buffer << "EGAP}";
		break;
	default:
		util::stream_format(buffer, "BL%02X.", m_region - REGION_DATA_BLOCK_0);

		if (m_bytenum <= BYTE_PRE_GAP_33)
			util::stream_format(buffer, "PR%02d", m_bytenum - BYTE_PRE_GAP_0);
		else if (m_bytenum == BYTE_LEADIN)
			buffer << "LDIN";
		else if (m_bytenum == BYTE_HEADER)
			buffer << "HEAD";
		else if (m_bytenum <= BYTE_DATA_255)
			util::stream_format(buffer, "BY%02X", m_bytenum - BYTE_DATA_0);
		else if (m_bytenum == BYTE_CRC16_MSB)
			buffer << "CRCM";
		else if (m_bytenum == BYTE_CRC16_LSB)
			buffer << "CRCL";
		else if (m_bytenum == BYTE_TRAILER)
			buffer << "TRLR";
		else if (m_bytenum == BYTE_LEADOUT)
			buffer << "LOUT";
		else if (m_bytenum == BYTE_LONGCLOCK)
			buffer << "LONG";
		else
			util::stream_format(buffer, "PO%02d", m_bytenum - BYTE_POSTGAP_0);

		{
			// in the main data area, the clock alternates at the clock rate
			int clk;
			if (m_bytenum >= BYTE_LEADIN && m_bytenum <= BYTE_LEADOUT)
				clk = BIT(~uint32_t(m_clockpos - REGION_BOT_GAP_END_CLOCK), 0);
			else if (m_bytenum == BYTE_LONGCLOCK)
				clk = 1;
			else
				clk = 0;

			util::stream_format(buffer, ".%d.%d}", m_bitnum, clk);
		}
	}

	return std::move(buffer).str();
}


/*-------------------------------------------------
    tape_clock_callback - called once per clock
    to increment/decrement the tape location
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER( decocass_tape_device::tape_clock_callback )
{
	/* advance by one clock in the desired direction */
	if (m_speed < 0 && m_clockpos > 0)
		m_clockpos--;
	else if (m_speed > 0 && m_clockpos < m_numclocks)
		m_clockpos++;

	/* look for states before the start of data */
	if (m_clockpos < REGION_LEADER_END_CLOCK)
		m_region = REGION_LEADER;
	else if (m_clockpos < REGION_LEADER_GAP_END_CLOCK)
		m_region = REGION_LEADER_GAP;
	else if (m_clockpos < REGION_BOT_END_CLOCK)
		m_region = REGION_BOT;
	else if (m_clockpos < REGION_BOT_GAP_END_CLOCK)
		m_region = REGION_BOT_GAP;

	/* look for states after the end of data */
	else if (m_clockpos >= m_numclocks - REGION_LEADER_END_CLOCK)
		m_region = REGION_TRAILER;
	else if (m_clockpos >= m_numclocks - REGION_LEADER_GAP_END_CLOCK)
		m_region = REGION_TRAILER_GAP;
	else if (m_clockpos >= m_numclocks - REGION_BOT_END_CLOCK)
		m_region = REGION_EOT;
	else if (m_clockpos >= m_numclocks - REGION_BOT_GAP_END_CLOCK)
		m_region = REGION_EOT_GAP;

	/* everything else is data */
	else
	{
		uint32_t dataclock = m_clockpos - REGION_BOT_GAP_END_CLOCK;

		/* compute the block number */
		m_region = (tape_region)(REGION_DATA_BLOCK_0 + dataclock / (TAPE_CLOCKS_PER_BYTE * BYTE_BLOCK_TOTAL));
		dataclock -= (m_region - REGION_DATA_BLOCK_0) * TAPE_CLOCKS_PER_BYTE * BYTE_BLOCK_TOTAL;

		/* compute the byte within the block */
		m_bytenum = (tape_byte)(dataclock / TAPE_CLOCKS_PER_BYTE);
		dataclock -= m_bytenum * TAPE_CLOCKS_PER_BYTE;

		/* compute the bit within the byte */
		m_bitnum = dataclock / TAPE_CLOCKS_PER_BIT;
	}

	/* log */
	if (LOG_CASSETTE_STATE)
		describe_state(); // FIXME: is this supposed to actually do something with the result?
}


/*-------------------------------------------------
    tape_get_status_bits - return the 3 status
    bits from the tape
-------------------------------------------------*/

uint8_t decocass_tape_device::get_status_bits()
{
	uint8_t tape_bits = 0;

	/* bit 0x20 is the BOT/EOT signal, which is also set in the leader/trailer area */
	if (m_region == REGION_LEADER || m_region == REGION_BOT || m_region == REGION_EOT || m_region == REGION_TRAILER)
		tape_bits |= 0x20;

	/* bit 0x40 is the clock, which is only valid in some areas of the data block */
	/* bit 0x80 is the data, which is only valid in some areas of the data block */
	if (m_region >= REGION_DATA_BLOCK_0 && m_region <= REGION_DATA_BLOCK_255)
	{
		int blocknum = m_region - REGION_DATA_BLOCK_0;
		uint8_t byteval = 0x00;

		/* in the main data area, the clock alternates at the clock rate */
		if (m_bytenum >= BYTE_LEADIN && m_bytenum <= BYTE_LEADOUT)
			tape_bits |= ((uint32_t)(m_clockpos - REGION_BOT_GAP_END_CLOCK) & 1) ? 0x00 : 0x40;

		/* in the longclock area, the clock holds high */
		else if (m_bytenum == BYTE_LONGCLOCK)
			tape_bits |= 0x40;

		/* everywhere else, the clock holds to 0 */
		else
		{
				/* nothing */
		}

		/* lead-in and lead-out bytes are 0xAA */
		if (m_bytenum == BYTE_HEADER || m_bytenum == BYTE_TRAILER)
			byteval = 0xaa;

		/* data block bytes are data */
		else if (m_bytenum >= BYTE_DATA_0 && m_bytenum <= BYTE_DATA_255)
			byteval = m_tape_data[blocknum * 256 + (m_bytenum - BYTE_DATA_0)];

		/* CRC MSB */
		else if (m_bytenum == BYTE_CRC16_MSB)
			byteval = m_crc16[blocknum] >> 8;

		/* CRC LSB */
		else if (m_bytenum == BYTE_CRC16_LSB)
			byteval = m_crc16[blocknum];

		/* select the appropriate bit from the byte and move to the upper bit */
		if ((byteval >> m_bitnum) & 1)
			tape_bits |= 0x80;
	}
	return tape_bits;
}


/*-------------------------------------------------
    tape_is_present - return true if the tape is
    present
-------------------------------------------------*/

bool decocass_tape_device::is_present()
{
	return m_tape_data.found();
}


/*-------------------------------------------------
    tape_change_speed - alter the speed of tape
    playback
-------------------------------------------------*/

void decocass_tape_device::change_speed(int8_t newspeed)
{
	attotime newperiod;
	int8_t absnewspeed;

	/* do nothing if speed has not changed */
	if (m_speed == newspeed)
		return;

	/* compute how fast to run the tape timer */
	absnewspeed = (newspeed < 0) ? -newspeed : newspeed;
	if (newspeed == 0)
		newperiod = attotime::never;
	else
		newperiod = attotime::from_hz(TAPE_CLOCKRATE * absnewspeed);

	/* set the new speed */
	m_tape_timer->adjust(newperiod, 0, newperiod);
	m_speed = newspeed;
}
