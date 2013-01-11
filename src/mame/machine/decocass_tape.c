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

/* regions within the virtual tape */
enum tape_region
{
	REGION_LEADER,              /* in clear leader section */
	REGION_LEADER_GAP,          /* in gap between leader and BOT */
	REGION_BOT,                 /* in BOT hole */
	REGION_BOT_GAP,             /* in gap between BOT hole and data */
	REGION_DATA_BLOCK_0,        /* in data block 0 */
	REGION_DATA_BLOCK_255 = REGION_DATA_BLOCK_0 + 255,
	REGION_EOT_GAP,             /* in gap between data and EOT hole */
	REGION_EOT,                 /* in EOT hole */
	REGION_TRAILER_GAP,         /* in gap between trailer and EOT */
	REGION_TRAILER              /* in clear trailer section */
};


/* bytes within a data block on a virtual tape */
enum tape_byte
{
	BYTE_PRE_GAP_0,             /* 34 bytes of gap, clock held to 0, no data */
	BYTE_PRE_GAP_33 = BYTE_PRE_GAP_0 + 33,
	BYTE_LEADIN,                /* 1 leadin byte, clocked value 0x00 */
	BYTE_HEADER,                /* 1 header byte, clocked value 0xAA */
	BYTE_DATA_0,                /* 256 bytes of data, clocked */
	BYTE_DATA_255 = BYTE_DATA_0 + 255,
	BYTE_CRC16_MSB,             /* 2 bytes of CRC, clocked MSB first, then LSB */
	BYTE_CRC16_LSB,
	BYTE_TRAILER,               /* 1 trailer byte, clocked value 0xAA */
	BYTE_LEADOUT,               /* 1 leadout byte, clocked value 0x00 */
	BYTE_LONGCLOCK,             /* 1 longclock byte, clock held to 1, no data */
	BYTE_POSTGAP_0,             /* 34 bytes of gap, no clock, no data */
	BYTE_POSTGAP_33 = BYTE_POSTGAP_0 + 33,
	BYTE_BLOCK_TOTAL            /* total number of bytes in block */
};


/* state of the tape */
struct tape_state
{
	running_machine *   machine;            /* pointer back to the machine */
	emu_timer *         timer;              /* timer for running the tape */
	INT8                speed;              /* speed: <-1=fast rewind, -1=reverse, 0=stopped, 1=normal, >1=fast forward */
	tape_region         region;             /* current region */
	tape_byte           bytenum;            /* byte number within a datablock */
	UINT8               bitnum;             /* bit number within a byte */
	UINT32              clockpos;           /* the current clock position of the tape */
	UINT32              numclocks;          /* total number of clocks on the entire tape */
	UINT16              crc16[256];         /* CRC16 for each block */
};


/* number of tape clock pulses per second */
#define TAPE_CLOCKRATE                  4800
#define TAPE_CLOCKS_PER_BIT             2
#define TAPE_CLOCKS_PER_BYTE            (8 * TAPE_CLOCKS_PER_BIT)
#define TAPE_MSEC_TO_CLOCKS(x)          ((x) * TAPE_CLOCKRATE / 1000)


/* Note on a tapes leader-BOT-data-EOT-trailer format:
 * A cassette has a transparent piece of tape on both ends,
 * leader and trailer. And data tapes also have BOT and EOT
 * holes, shortly before the the leader and trailer.
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


/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, an IDE controller
-------------------------------------------------*/

INLINE tape_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == DECOCASS_TAPE);

	return (tape_state *)downcast<decocass_tape_device *>(device)->token();
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

static const char *tape_describe_state(tape_state *tape)
{
	static char buffer[40];
	char temprname[40];
	const char *rname = temprname;

	if (tape->region == REGION_LEADER)
		rname = "LEAD";
	else if (tape->region == REGION_LEADER_GAP)
		rname = "LGAP";
	else if (tape->region == REGION_BOT)
		rname = "BOT ";
	else if (tape->region == REGION_BOT_GAP)
		rname = "BGAP";
	else if (tape->region == REGION_TRAILER)
		rname = "TRLR";
	else if (tape->region == REGION_TRAILER_GAP)
		rname = "TGAP";
	else if (tape->region == REGION_EOT)
		rname = "EOT ";
	else if (tape->region == REGION_EOT_GAP)
		rname = "EGAP";
	else
	{
		char tempbname[40];
		const char *bname = tempbname;
		int clk;

		if (tape->bytenum <= BYTE_PRE_GAP_33)
			sprintf(tempbname, "PR%02d", tape->bytenum - BYTE_PRE_GAP_0);
		else if (tape->bytenum == BYTE_LEADIN)
			bname = "LDIN";
		else if (tape->bytenum == BYTE_HEADER)
			bname = "HEAD";
		else if (tape->bytenum <= BYTE_DATA_255)
			sprintf(tempbname, "BY%02X", tape->bytenum - BYTE_DATA_0);
		else if (tape->bytenum == BYTE_CRC16_MSB)
			bname = "CRCM";
		else if (tape->bytenum == BYTE_CRC16_LSB)
			bname = "CRCL";
		else if (tape->bytenum == BYTE_TRAILER)
			bname = "TRLR";
		else if (tape->bytenum == BYTE_LEADOUT)
			bname = "LOUT";
		else if (tape->bytenum == BYTE_LONGCLOCK)
			bname = "LONG";
		else
			sprintf(tempbname, "PO%02d", tape->bytenum - BYTE_POSTGAP_0);

		/* in the main data area, the clock alternates at the clock rate */
		if (tape->bytenum >= BYTE_LEADIN && tape->bytenum <= BYTE_LEADOUT)
			clk = ((UINT32)(tape->clockpos - REGION_BOT_GAP_END_CLOCK) & 1) ? 0 : 1;
		else if (tape->bytenum == BYTE_LONGCLOCK)
			clk = 1;
		else
			clk = 0;

		sprintf(temprname, "BL%02X.%4s.%d.%d", tape->region - REGION_DATA_BLOCK_0, bname, tape->bitnum, clk);
	}

	sprintf(buffer, "{%9d=%s}", tape->clockpos, rname);
	return buffer;
}


/*-------------------------------------------------
    tape_clock_callback - called once per clock
    to increment/decrement the tape location
-------------------------------------------------*/

static TIMER_CALLBACK( tape_clock_callback )
{
	device_t *device = (device_t *)ptr;
	tape_state *tape = get_safe_token(device);

	/* advance by one clock in the desired direction */
	if (tape->speed < 0 && tape->clockpos > 0)
		tape->clockpos--;
	else if (tape->speed > 0 && tape->clockpos < tape->numclocks)
		tape->clockpos++;

	/* look for states before the start of data */
	if (tape->clockpos < REGION_LEADER_END_CLOCK)
		tape->region = REGION_LEADER;
	else if (tape->clockpos < REGION_LEADER_GAP_END_CLOCK)
		tape->region = REGION_LEADER_GAP;
	else if (tape->clockpos < REGION_BOT_END_CLOCK)
		tape->region = REGION_BOT;
	else if (tape->clockpos < REGION_BOT_GAP_END_CLOCK)
		tape->region = REGION_BOT_GAP;

	/* look for states after the end of data */
	else if (tape->clockpos >= tape->numclocks - REGION_LEADER_END_CLOCK)
		tape->region = REGION_TRAILER;
	else if (tape->clockpos >= tape->numclocks - REGION_LEADER_GAP_END_CLOCK)
		tape->region = REGION_TRAILER_GAP;
	else if (tape->clockpos >= tape->numclocks - REGION_BOT_END_CLOCK)
		tape->region = REGION_EOT;
	else if (tape->clockpos >= tape->numclocks - REGION_BOT_GAP_END_CLOCK)
		tape->region = REGION_EOT_GAP;

	/* everything else is data */
	else
	{
		UINT32 dataclock = tape->clockpos - REGION_BOT_GAP_END_CLOCK;

		/* compute the block number */
		tape->region = (tape_region)(REGION_DATA_BLOCK_0 + dataclock / (TAPE_CLOCKS_PER_BYTE * BYTE_BLOCK_TOTAL));
		dataclock -= (tape->region - REGION_DATA_BLOCK_0) * TAPE_CLOCKS_PER_BYTE * BYTE_BLOCK_TOTAL;

		/* compute the byte within the block */
		tape->bytenum = (tape_byte)(dataclock / TAPE_CLOCKS_PER_BYTE);
		dataclock -= tape->bytenum * TAPE_CLOCKS_PER_BYTE;

		/* compute the bit within the byte */
		tape->bitnum = dataclock / TAPE_CLOCKS_PER_BIT;
	}

	/* log */
	if (LOG_CASSETTE_STATE)
		tape_describe_state(tape);
}


/*-------------------------------------------------
    tape_get_status_bits - return the 3 status
    bits from the tape
-------------------------------------------------*/

UINT8 tape_get_status_bits(device_t *device)
{
	tape_state *tape = get_safe_token(device);
	UINT8 tape_bits = 0;

	/* bit 0x20 is the BOT/EOT signal, which is also set in the leader/trailer area */
	if (tape->region == REGION_LEADER || tape->region == REGION_BOT || tape->region == REGION_EOT || tape->region == REGION_TRAILER)
		tape_bits |= 0x20;

	/* bit 0x40 is the clock, which is only valid in some areas of the data block */
	/* bit 0x80 is the data, which is only valid in some areas of the data block */
	if (tape->region >= REGION_DATA_BLOCK_0 && tape->region <= REGION_DATA_BLOCK_255)
	{
		int blocknum = tape->region - REGION_DATA_BLOCK_0;
		UINT8 byteval = 0x00;

		/* in the main data area, the clock alternates at the clock rate */
		if (tape->bytenum >= BYTE_LEADIN && tape->bytenum <= BYTE_LEADOUT)
			tape_bits |= ((UINT32)(tape->clockpos - REGION_BOT_GAP_END_CLOCK) & 1) ? 0x00 : 0x40;

		/* in the longclock area, the clock holds high */
		else if (tape->bytenum == BYTE_LONGCLOCK)
			tape_bits |= 0x40;

		/* everywhere else, the clock holds to 0 */
		else
			;

		/* lead-in and lead-out bytes are 0xAA */
		if (tape->bytenum == BYTE_HEADER || tape->bytenum == BYTE_TRAILER)
			byteval = 0xaa;

		/* data block bytes are data */
		else if (tape->bytenum >= BYTE_DATA_0 && tape->bytenum <= BYTE_DATA_255)
			byteval = static_cast<UINT8 *>(*device->region())[blocknum * 256 + (tape->bytenum - BYTE_DATA_0)];

		/* CRC MSB */
		else if (tape->bytenum == BYTE_CRC16_MSB)
			byteval = tape->crc16[blocknum] >> 8;

		/* CRC LSB */
		else if (tape->bytenum == BYTE_CRC16_LSB)
			byteval = tape->crc16[blocknum];

		/* select the appropriate bit from the byte and move to the upper bit */
		if ((byteval >> tape->bitnum) & 1)
			tape_bits |= 0x80;
	}
	return tape_bits;
}


/*-------------------------------------------------
    tape_is_present - return TRUE if the tape is
    present
-------------------------------------------------*/

UINT8 tape_is_present(device_t *device)
{
	return device->region() != NULL;
}


/*-------------------------------------------------
    tape_change_speed - alter the speed of tape
    playback
-------------------------------------------------*/

void tape_change_speed(device_t *device, INT8 newspeed)
{
	tape_state *tape = get_safe_token(device);
	attotime newperiod;
	INT8 absnewspeed;

	/* do nothing if speed has not changed */
	if (tape->speed == newspeed)
		return;

	/* compute how fast to run the tape timer */
	absnewspeed = (newspeed < 0) ? -newspeed : newspeed;
	if (newspeed == 0)
		newperiod = attotime::never;
	else
		newperiod = attotime::from_hz(TAPE_CLOCKRATE * absnewspeed);

	/* set the new speed */
	tape->timer->adjust(newperiod, 0, newperiod);
	tape->speed = newspeed;
}


/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( decocass_tape )
{
	tape_state *tape = get_safe_token(device);
	int curblock, offs, numblocks;

	/* validate some basic stuff */
	assert(device != NULL);
	assert(device->static_config() == NULL);

	/* fetch the data pointer */
	tape->timer = device->machine().scheduler().timer_alloc(FUNC(tape_clock_callback), (void *)device);
	if (device->region() == NULL)
		return;
	UINT8 *regionbase = *device->region();

	/* scan for the first non-empty block in the image */
	for (offs = device->region()->bytes() - 1; offs >= 0; offs--)
		if (regionbase[offs] != 0)
			break;
	numblocks = ((offs | 0xff) + 1) / 256;
	assert(numblocks < ARRAY_LENGTH(tape->crc16));

	/* compute the total length */
	tape->numclocks = REGION_BOT_GAP_END_CLOCK + numblocks * BYTE_BLOCK_TOTAL * 16 + REGION_BOT_GAP_END_CLOCK;

	/* compute CRCs for each block */
	for (curblock = 0; curblock < numblocks; curblock++)
	{
		UINT16 crc = 0;
		int testval;

		/* first CRC the 256 bytes of data */
		for (offs = 256 * curblock; offs < 256 * curblock + 256; offs++)
			crc = tape_crc16_byte(crc, regionbase[offs]);

		/* then find a pair of bytes that will bring the CRC to 0 (any better way than brute force?) */
		for (testval = 0; testval < 0x10000; testval++)
			if (tape_crc16_byte(tape_crc16_byte(crc, testval >> 8), testval) == 0)
				break;
		tape->crc16[curblock] = testval;
	}

	/* register states */
	device->save_item(NAME(tape->speed));
	device->save_item(NAME(tape->bitnum));
	device->save_item(NAME(tape->clockpos));
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( decocass_tape )
{
	/* turn the tape off */
	tape_change_speed(device, 0);
}


const device_type DECOCASS_TAPE = &device_creator<decocass_tape_device>;

decocass_tape_device::decocass_tape_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DECOCASS_TAPE, "DECO Cassette Tape", tag, owner, clock)
{
	m_token = global_alloc_clear(tape_state);
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
	DEVICE_START_NAME( decocass_tape )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void decocass_tape_device::device_reset()
{
	DEVICE_RESET_NAME( decocass_tape )(this);
}
