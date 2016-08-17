// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, David Haywood
/***********************************************************************

    DECO Cassette System machine

 ***********************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/decocass_tape.h"

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

static UINT16 tape_crc16_byte(UINT16 crc, UINT8 data);

const device_type DECOCASS_TAPE = &device_creator<decocass_tape_device>;

decocass_tape_device::decocass_tape_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DECOCASS_TAPE, "DECO Cassette Tape", tag, owner, clock, "decocass_tape", __FILE__),
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
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void decocass_tape_device::device_config_complete()
{
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
	assert(numblocks < ARRAY_LENGTH(m_crc16));

	/* compute the total length */
	m_numclocks = REGION_BOT_GAP_END_CLOCK + numblocks * BYTE_BLOCK_TOTAL * 16 + REGION_BOT_GAP_END_CLOCK;

	/* compute CRCs for each block */
	for (curblock = 0; curblock < numblocks; curblock++)
	{
		UINT16 crc = 0;
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

static UINT16 tape_crc16_byte(UINT16 crc, UINT8 data)
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

const char *decocass_tape_device::describe_state()
{
	static char buffer[40];
	char temprname[40];
	const char *rname = temprname;

	if (m_region == REGION_LEADER)
		rname = "LEAD";
	else if (m_region == REGION_LEADER_GAP)
		rname = "LGAP";
	else if (m_region == REGION_BOT)
		rname = "BOT ";
	else if (m_region == REGION_BOT_GAP)
		rname = "BGAP";
	else if (m_region == REGION_TRAILER)
		rname = "TRLR";
	else if (m_region == REGION_TRAILER_GAP)
		rname = "TGAP";
	else if (m_region == REGION_EOT)
		rname = "EOT ";
	else if (m_region == REGION_EOT_GAP)
		rname = "EGAP";
	else
	{
		char tempbname[40];
		const char *bname = tempbname;
		int clk;

		if (m_bytenum <= BYTE_PRE_GAP_33)
			sprintf(tempbname, "PR%02d", m_bytenum - BYTE_PRE_GAP_0);
		else if (m_bytenum == BYTE_LEADIN)
			bname = "LDIN";
		else if (m_bytenum == BYTE_HEADER)
			bname = "HEAD";
		else if (m_bytenum <= BYTE_DATA_255)
			sprintf(tempbname, "BY%02X", m_bytenum - BYTE_DATA_0);
		else if (m_bytenum == BYTE_CRC16_MSB)
			bname = "CRCM";
		else if (m_bytenum == BYTE_CRC16_LSB)
			bname = "CRCL";
		else if (m_bytenum == BYTE_TRAILER)
			bname = "TRLR";
		else if (m_bytenum == BYTE_LEADOUT)
			bname = "LOUT";
		else if (m_bytenum == BYTE_LONGCLOCK)
			bname = "LONG";
		else
			sprintf(tempbname, "PO%02d", m_bytenum - BYTE_POSTGAP_0);

		/* in the main data area, the clock alternates at the clock rate */
		if (m_bytenum >= BYTE_LEADIN && m_bytenum <= BYTE_LEADOUT)
			clk = ((UINT32)(m_clockpos - REGION_BOT_GAP_END_CLOCK) & 1) ? 0 : 1;
		else if (m_bytenum == BYTE_LONGCLOCK)
			clk = 1;
		else
			clk = 0;

		sprintf(temprname, "BL%02X.%4s.%d.%d", m_region - REGION_DATA_BLOCK_0, bname, m_bitnum, clk);
	}

	sprintf(buffer, "{%9d=%s}", m_clockpos, rname);
	return buffer;
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
		UINT32 dataclock = m_clockpos - REGION_BOT_GAP_END_CLOCK;

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
		describe_state();
}


/*-------------------------------------------------
    tape_get_status_bits - return the 3 status
    bits from the tape
-------------------------------------------------*/

UINT8 decocass_tape_device::get_status_bits()
{
	UINT8 tape_bits = 0;

	/* bit 0x20 is the BOT/EOT signal, which is also set in the leader/trailer area */
	if (m_region == REGION_LEADER || m_region == REGION_BOT || m_region == REGION_EOT || m_region == REGION_TRAILER)
		tape_bits |= 0x20;

	/* bit 0x40 is the clock, which is only valid in some areas of the data block */
	/* bit 0x80 is the data, which is only valid in some areas of the data block */
	if (m_region >= REGION_DATA_BLOCK_0 && m_region <= REGION_DATA_BLOCK_255)
	{
		int blocknum = m_region - REGION_DATA_BLOCK_0;
		UINT8 byteval = 0x00;

		/* in the main data area, the clock alternates at the clock rate */
		if (m_bytenum >= BYTE_LEADIN && m_bytenum <= BYTE_LEADOUT)
			tape_bits |= ((UINT32)(m_clockpos - REGION_BOT_GAP_END_CLOCK) & 1) ? 0x00 : 0x40;

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
    tape_is_present - return TRUE if the tape is
    present
-------------------------------------------------*/

UINT8 decocass_tape_device::is_present()
{
	return m_tape_data.found();
}


/*-------------------------------------------------
    tape_change_speed - alter the speed of tape
    playback
-------------------------------------------------*/

void decocass_tape_device::change_speed(INT8 newspeed)
{
	attotime newperiod;
	INT8 absnewspeed;

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
