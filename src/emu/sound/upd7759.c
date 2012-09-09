/************************************************************

    NEC uPD7759/55/56/P56/57/58 ADPCM Speech Processor
    by: Juergen Buchmueller, Mike Balfour, Howie Cohen,
        Olivier Galibert, and Aaron Giles

    TODO:
    - is there a doable method to dump the internal maskrom? :(
    - low-level emulation
    - watchdog? - according to uPD775x datasheet, the chip goes into standy mode
      if CS/ST/RESET have not been accessed for more than 3 seconds
    - convert to MAME modern device

*************************************************************

    uPD7759 Description:

    The UPD7759 is a speech processing LSI that utilizes ADPCM to produce
    speech or other sampled sounds.  It can directly address up to 1Mbit
    (128k) of external data ROM, or the host CPU can control the speech
    data transfer.  The UPD7759 is usually hooked up to a 640 kHz clock and
    has one 8-bit input port, a start pin, a busy pin, and a clock output.

    The chip is composed of 3 parts:
    - a clock divider
    - a rom-reading engine
    - an adpcm engine
    - a 4-to-9 bit adpcm converter

    The clock divider takes the base 640KHz clock and divides it first
    by a fixed divisor of 4 and then by a value between 9 and 32.  The
    result gives a clock between 5KHz and 17.78KHz.  It's probably
    possible, but not recommended and certainly out-of-spec, to push the
    chip harder by reducing the divider.

    The rom-reading engine reads one byte every two divided clock cycles.
    The factor two comes from the fact that a byte has two nibbles, i.e.
    two samples.

    The apdcm engine takes bytes and interprets them as commands:

        00000000                    sample end
        00dddddd                    silence
        01ffffff                    send the 256 following nibbles to the converter
        10ffffff nnnnnnnn           send the n+1 following nibbles to the converter
        11---rrr --ffffff nnnnnnnn  send the n+1 following nibbles to the converter, and repeat r+1 times

    "ffffff" is sent to the clock divider to be the base clock for the
    adpcm converter, i.e., it's the sampling rate.  If the number of
    nibbles to send is odd the last nibble is ignored.  The commands
    are always 8-bit aligned.

    "dddddd" is the duration of the silence.  The base speed is unknown,
    1ms sounds reasonably.  It does not seem linked to the adpcm clock
    speed because there often is a silence before any 01 or 10 command.

    The adpcm converter converts nibbles into 9-bit DAC values.  It has
    an internal state of 4 bits that's used in conjunction with the
    nibble to lookup which of the 256 possible steps is used.  Then
    the state is changed according to the nibble value.  Essentially, the
    higher the state, the bigger the steps are, and using big steps
    increase the state.  Conversely, using small steps reduces the state.
    This allows the engine to be a little more adaptative than a
    classical ADPCM algorithm.

    The UPD7759 can run in two modes, master (also known as standalone)
    and slave.  The mode is selected through the "md" pin.  No known
    game changes modes on the fly, and it's unsure if that's even
    possible to do.


    Master mode:

    The output of the rom reader is directly connected to the adpcm
    converter.  The controlling cpu only sends a sample number and the
    7759 plays it.

    The sample rom has a header at the beginning of the form

        nn 5a a5 69 55

    where nn is the number of the last sample.  This is then followed by
    a vector of 2-bytes msb-first values, one per sample.  Multiplying
    them by two gives the sample start offset in the rom.  A 0x00 marks
    the end of each sample.

    It seems that the UPD7759 reads at least part of the rom header at
    startup.  Games doing rom banking are careful to reset the chip after
    each change.


    Slave mode:

    The rom reader is completely disconnected.  The input port is
    connected directly to the adpcm engine.  The first write to the input
    port activates the engine (the value itself is ignored).  The engine
    activates the clock output and waits for commands.  The clock speed
    is unknown, but its probably a divider of 640KHz.  We use 40KHz here
    because 80KHz crashes altbeast.  The chip probably has an internal
    fifo to the converter and suspends the clock when the fifo is full.
    The first command is always 0xFF.  A second 0xFF marks the end of the
    sample and the engine stops.  OTOH, there is a 0x00 at the end too.
    Go figure.

*************************************************************

    The other chip models don't support slave mode, and have an internal ROM.
    Other than that, they are thought to be nearly identical to uPD7759.
    
    55C    18-pin DIP   96 Kbit ROM
    55G    24-pin SOP   96 Kbit ROM
    56C    18-pin DIP  256 Kbit ROM
    56G    24-pin SOP  256 Kbit ROM
    P56CR  20-pin DIP  256 Kbit ROM (OTP)
    P56G   24-pin SOP  256 Kbit ROM (OTP)
    57C    18-pin DIP  512 Kbit ROM
    57G    24-pin SOP  512 Kbit ROM
    58C    18-pin DIP    1 Mbit ROM
    58G    24-pin SOP    1 Mbit ROM

*************************************************************/

#include "emu.h"
#include "upd7759.h"


#define DEBUG_STATES	(0)
#define DEBUG_METHOD	mame_printf_debug



/************************************************************

    Constants

*************************************************************/

/* step value fractional bits */
#define FRAC_BITS		20
#define FRAC_ONE		(1 << FRAC_BITS)
#define FRAC_MASK		(FRAC_ONE - 1)

/* chip states */
enum
{
	STATE_IDLE,
	STATE_DROP_DRQ,
	STATE_START,
	STATE_FIRST_REQ,
	STATE_LAST_SAMPLE,
	STATE_DUMMY1,
	STATE_ADDR_MSB,
	STATE_ADDR_LSB,
	STATE_DUMMY2,
	STATE_BLOCK_HEADER,
	STATE_NIBBLE_COUNT,
	STATE_NIBBLE_MSN,
	STATE_NIBBLE_LSN
};



/************************************************************

    Type definitions

*************************************************************/

typedef struct _upd7759_state upd7759_state;
struct _upd7759_state
{
	device_t *device;
	sound_stream *channel;					/* stream channel for playback */

	/* chip configuration */
	UINT8		sample_offset_shift;		/* header sample address shift (access data > 0xffff) */
	
	/* internal clock to output sample rate mapping */
	UINT32		pos;						/* current output sample position */
	UINT32		step;						/* step value per output sample */
	attotime	clock_period;				/* clock period */
	emu_timer *timer;						/* timer */

	/* I/O lines */
	UINT8		fifo_in;					/* last data written to the sound chip */
	UINT8		reset;						/* current state of the RESET line */
	UINT8		start;						/* current state of the START line */
	UINT8		drq;						/* current state of the DRQ line */
	void (*drqcallback)(device_t *device, int param);			/* drq callback */

	/* internal state machine */
	INT8		state;						/* current overall chip state */
	INT32		clocks_left;				/* number of clocks left in this state */
	UINT16		nibbles_left;				/* number of ADPCM nibbles left to process */
	UINT8		repeat_count;				/* number of repeats remaining in current repeat block */
	INT8		post_drq_state;				/* state we will be in after the DRQ line is dropped */
	INT32		post_drq_clocks;			/* clocks that will be left after the DRQ line is dropped */
	UINT8		req_sample;					/* requested sample number */
	UINT8		last_sample;				/* last sample number available */
	UINT8		block_header;				/* header byte */
	UINT8		sample_rate;				/* number of UPD clocks per ADPCM nibble */
	UINT8		first_valid_header;			/* did we get our first valid header yet? */
	UINT32		offset;						/* current ROM offset */
	UINT32		repeat_offset;				/* current ROM repeat offset */

	/* ADPCM processing */
	INT8		adpcm_state;				/* ADPCM state index */
	UINT8		adpcm_data;					/* current byte of ADPCM data */
	INT16		sample;						/* current sample value */

	/* ROM access */
	UINT8 *		rom;						/* pointer to ROM data or NULL for slave mode */
	UINT8 *		rombase;					/* pointer to ROM data or NULL for slave mode */
	UINT32		romoffset;					/* ROM offset to make save/restore easier */
	UINT32		rommask;					/* maximum address offset */
};



/************************************************************

    Local variables

*************************************************************/

static const int upd7759_step[16][16] =
{
	{ 0,  0,  1,  2,  3,   5,   7,  10,  0,   0,  -1,  -2,  -3,   -5,   -7,  -10 },
	{ 0,  1,  2,  3,  4,   6,   8,  13,  0,  -1,  -2,  -3,  -4,   -6,   -8,  -13 },
	{ 0,  1,  2,  4,  5,   7,  10,  15,  0,  -1,  -2,  -4,  -5,   -7,  -10,  -15 },
	{ 0,  1,  3,  4,  6,   9,  13,  19,  0,  -1,  -3,  -4,  -6,   -9,  -13,  -19 },
	{ 0,  2,  3,  5,  8,  11,  15,  23,  0,  -2,  -3,  -5,  -8,  -11,  -15,  -23 },
	{ 0,  2,  4,  7, 10,  14,  19,  29,  0,  -2,  -4,  -7, -10,  -14,  -19,  -29 },
	{ 0,  3,  5,  8, 12,  16,  22,  33,  0,  -3,  -5,  -8, -12,  -16,  -22,  -33 },
	{ 1,  4,  7, 10, 15,  20,  29,  43, -1,  -4,  -7, -10, -15,  -20,  -29,  -43 },
	{ 1,  4,  8, 13, 18,  25,  35,  53, -1,  -4,  -8, -13, -18,  -25,  -35,  -53 },
	{ 1,  6, 10, 16, 22,  31,  43,  64, -1,  -6, -10, -16, -22,  -31,  -43,  -64 },
	{ 2,  7, 12, 19, 27,  37,  51,  76, -2,  -7, -12, -19, -27,  -37,  -51,  -76 },
	{ 2,  9, 16, 24, 34,  46,  64,  96, -2,  -9, -16, -24, -34,  -46,  -64,  -96 },
	{ 3, 11, 19, 29, 41,  57,  79, 117, -3, -11, -19, -29, -41,  -57,  -79, -117 },
	{ 4, 13, 24, 36, 50,  69,  96, 143, -4, -13, -24, -36, -50,  -69,  -96, -143 },
	{ 4, 16, 29, 44, 62,  85, 118, 175, -4, -16, -29, -44, -62,  -85, -118, -175 },
	{ 6, 20, 36, 54, 76, 104, 144, 214, -6, -20, -36, -54, -76, -104, -144, -214 },
};

static const int upd7759_state_table[16] = { -1, -1, 0, 0, 1, 2, 2, 3, -1, -1, 0, 0, 1, 2, 2, 3 };



INLINE upd7759_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == UPD7759);
	return (upd7759_state *)downcast<upd7759_device *>(device)->token();
}


/************************************************************

    ADPCM sample updater

*************************************************************/

INLINE void update_adpcm(upd7759_state *chip, int data)
{
	/* update the sample and the state */
	chip->sample += upd7759_step[chip->adpcm_state][data];
	chip->adpcm_state += upd7759_state_table[data];

	/* clamp the state to 0..15 */
	if (chip->adpcm_state < 0)
		chip->adpcm_state = 0;
	else if (chip->adpcm_state > 15)
		chip->adpcm_state = 15;
}



/************************************************************

    Master chip state machine

*************************************************************/

static void advance_state(upd7759_state *chip)
{
	switch (chip->state)
	{
		/* Idle state: we stick around here while there's nothing to do */
		case STATE_IDLE:
			chip->clocks_left = 4;
			break;

		/* drop DRQ state: update to the intended state */
		case STATE_DROP_DRQ:
			chip->drq = 0;

			chip->clocks_left = chip->post_drq_clocks;
			chip->state = chip->post_drq_state;
			break;

		/* Start state: we begin here as soon as a sample is triggered */
		case STATE_START:
			chip->req_sample = chip->rom ? chip->fifo_in : 0x10;
			if (DEBUG_STATES) DEBUG_METHOD("UPD7759: req_sample = %02X\n", chip->req_sample);

			/* 35+ cycles after we get here, the /DRQ goes low
             *     (first byte (number of samples in ROM) should be sent in response)
             *
             * (35 is the minimum number of cycles I found during heavy tests.
             * Depending on the state the chip was in just before the /MD was set to 0 (reset, standby
             * or just-finished-playing-previous-sample) this number can range from 35 up to ~24000).
             * It also varies slightly from test to test, but not much - a few cycles at most.) */
			chip->clocks_left = 70;	/* 35 - breaks cotton */
			chip->state = STATE_FIRST_REQ;
			break;

		/* First request state: issue a request for the first byte */
		/* The expected response will be the index of the last sample */
		case STATE_FIRST_REQ:
			if (DEBUG_STATES) DEBUG_METHOD("UPD7759: first data request\n");
			chip->drq = 1;

			/* 44 cycles later, we will latch this value and request another byte */
			chip->clocks_left = 44;
			chip->state = STATE_LAST_SAMPLE;
			break;

		/* Last sample state: latch the last sample value and issue a request for the second byte */
		/* The second byte read will be just a dummy */
		case STATE_LAST_SAMPLE:
			chip->last_sample = chip->rom ? chip->rom[0] : chip->fifo_in;
			if (DEBUG_STATES) DEBUG_METHOD("UPD7759: last_sample = %02X, requesting dummy 1\n", chip->last_sample);
			chip->drq = 1;

			/* 28 cycles later, we will latch this value and request another byte */
			chip->clocks_left = 28;	/* 28 - breaks cotton */
			chip->state = (chip->req_sample > chip->last_sample) ? STATE_IDLE : STATE_DUMMY1;
			break;

		/* First dummy state: ignore any data here and issue a request for the third byte */
		/* The expected response will be the MSB of the sample address */
		case STATE_DUMMY1:
			if (DEBUG_STATES) DEBUG_METHOD("UPD7759: dummy1, requesting offset_hi\n");
			chip->drq = 1;

			/* 32 cycles later, we will latch this value and request another byte */
			chip->clocks_left = 32;
			chip->state = STATE_ADDR_MSB;
			break;

		/* Address MSB state: latch the MSB of the sample address and issue a request for the fourth byte */
		/* The expected response will be the LSB of the sample address */
		case STATE_ADDR_MSB:
			chip->offset = (chip->rom ? chip->rom[chip->req_sample * 2 + 5] : chip->fifo_in) << (8 + chip->sample_offset_shift);
			if (DEBUG_STATES) DEBUG_METHOD("UPD7759: offset_hi = %02X, requesting offset_lo\n", chip->offset >> (8 + chip->sample_offset_shift));
			chip->drq = 1;

			/* 44 cycles later, we will latch this value and request another byte */
			chip->clocks_left = 44;
			chip->state = STATE_ADDR_LSB;
			break;

		/* Address LSB state: latch the LSB of the sample address and issue a request for the fifth byte */
		/* The expected response will be just a dummy */
		case STATE_ADDR_LSB:
			chip->offset |= (chip->rom ? chip->rom[chip->req_sample * 2 + 6] : chip->fifo_in) << chip->sample_offset_shift;
			if (DEBUG_STATES) DEBUG_METHOD("UPD7759: offset_lo = %02X, requesting dummy 2\n", (chip->offset >> chip->sample_offset_shift) & 0xff);
			if (chip->offset > chip->rommask) logerror("upd7759 offset %X > rommask %X\n",chip->offset, chip->rommask);
			chip->drq = 1;

			/* 36 cycles later, we will latch this value and request another byte */
			chip->clocks_left = 36;
			chip->state = STATE_DUMMY2;
			break;

		/* Second dummy state: ignore any data here and issue a request for the the sixth byte */
		/* The expected response will be the first block header */
		case STATE_DUMMY2:
			chip->offset++;
			chip->first_valid_header = 0;
			if (DEBUG_STATES) DEBUG_METHOD("UPD7759: dummy2, requesting block header\n");
			chip->drq = 1;

			/* 36?? cycles later, we will latch this value and request another byte */
			chip->clocks_left = 36;
			chip->state = STATE_BLOCK_HEADER;
			break;

		/* Block header state: latch the header and issue a request for the first byte afterwards */
		case STATE_BLOCK_HEADER:

			/* if we're in a repeat loop, reset the offset to the repeat point and decrement the count */
			if (chip->repeat_count)
			{
				chip->repeat_count--;
				chip->offset = chip->repeat_offset;
			}
			chip->block_header = chip->rom ? chip->rom[chip->offset++ & chip->rommask] : chip->fifo_in;
			if (DEBUG_STATES) DEBUG_METHOD("UPD7759: header (@%05X) = %02X, requesting next byte\n", chip->offset, chip->block_header);
			chip->drq = 1;

			/* our next step depends on the top two bits */
			switch (chip->block_header & 0xc0)
			{
				case 0x00:	/* silence */
					chip->clocks_left = 1024 * ((chip->block_header & 0x3f) + 1);
					chip->state = (chip->block_header == 0 && chip->first_valid_header) ? STATE_IDLE : STATE_BLOCK_HEADER;
					chip->sample = 0;
					chip->adpcm_state = 0;
					break;

				case 0x40:	/* 256 nibbles */
					chip->sample_rate = (chip->block_header & 0x3f) + 1;
					chip->nibbles_left = 256;
					chip->clocks_left = 36;	/* just a guess */
					chip->state = STATE_NIBBLE_MSN;
					break;

				case 0x80:	/* n nibbles */
					chip->sample_rate = (chip->block_header & 0x3f) + 1;
					chip->clocks_left = 36;	/* just a guess */
					chip->state = STATE_NIBBLE_COUNT;
					break;

				case 0xc0:	/* repeat loop */
					chip->repeat_count = (chip->block_header & 7) + 1;
					chip->repeat_offset = chip->offset;
					chip->clocks_left = 36;	/* just a guess */
					chip->state = STATE_BLOCK_HEADER;
					break;
			}

			/* set a flag when we get the first non-zero header */
			if (chip->block_header != 0)
				chip->first_valid_header = 1;
			break;

		/* Nibble count state: latch the number of nibbles to play and request another byte */
		/* The expected response will be the first data byte */
		case STATE_NIBBLE_COUNT:
			chip->nibbles_left = (chip->rom ? chip->rom[chip->offset++ & chip->rommask] : chip->fifo_in) + 1;
			if (DEBUG_STATES) DEBUG_METHOD("UPD7759: nibble_count = %u, requesting next byte\n", (unsigned)chip->nibbles_left);
			chip->drq = 1;

			/* 36?? cycles later, we will latch this value and request another byte */
			chip->clocks_left = 36;	/* just a guess */
			chip->state = STATE_NIBBLE_MSN;
			break;

		/* MSN state: latch the data for this pair of samples and request another byte */
		/* The expected response will be the next sample data or another header */
		case STATE_NIBBLE_MSN:
			chip->adpcm_data = chip->rom ? chip->rom[chip->offset++ & chip->rommask] : chip->fifo_in;
			update_adpcm(chip, chip->adpcm_data >> 4);
			chip->drq = 1;

			/* we stay in this state until the time for this sample is complete */
			chip->clocks_left = chip->sample_rate * 4;
			if (--chip->nibbles_left == 0)
				chip->state = STATE_BLOCK_HEADER;
			else
				chip->state = STATE_NIBBLE_LSN;
			break;

		/* LSN state: process the lower nibble */
		case STATE_NIBBLE_LSN:
			update_adpcm(chip, chip->adpcm_data & 15);

			/* we stay in this state until the time for this sample is complete */
			chip->clocks_left = chip->sample_rate * 4;
			if (--chip->nibbles_left == 0)
				chip->state = STATE_BLOCK_HEADER;
			else
				chip->state = STATE_NIBBLE_MSN;
			break;
	}

	/* if there's a DRQ, fudge the state */
	if (chip->drq)
	{
		chip->post_drq_state = chip->state;
		chip->post_drq_clocks = chip->clocks_left - 21;
		chip->state = STATE_DROP_DRQ;
		chip->clocks_left = 21;
	}
}



/************************************************************

    Stream callback

*************************************************************/

static STREAM_UPDATE( upd7759_update )
{
	upd7759_state *chip = (upd7759_state *)param;
	INT32 clocks_left = chip->clocks_left;
	INT16 sample = chip->sample;
	UINT32 step = chip->step;
	UINT32 pos = chip->pos;
	stream_sample_t *buffer = outputs[0];

	/* loop until done */
	if (chip->state != STATE_IDLE)
		while (samples != 0)
		{
			/* store the current sample */
			*buffer++ = sample << 7;
			samples--;

			/* advance by the number of clocks/output sample */
			pos += step;

			/* handle clocks, but only in standalone mode */
			while (chip->rom && pos >= FRAC_ONE)
			{
				int clocks_this_time = pos >> FRAC_BITS;
				if (clocks_this_time > clocks_left)
					clocks_this_time = clocks_left;

				/* clock once */
				pos -= clocks_this_time * FRAC_ONE;
				clocks_left -= clocks_this_time;

				/* if we're out of clocks, time to handle the next state */
				if (clocks_left == 0)
				{
					/* advance one state; if we hit idle, bail */
					advance_state(chip);
					if (chip->state == STATE_IDLE)
						break;

					/* reimport the variables that we cached */
					clocks_left = chip->clocks_left;
					sample = chip->sample;
				}
			}
		}

	/* if we got out early, just zap the rest of the buffer */
	if (samples != 0)
		memset(buffer, 0, samples * sizeof(*buffer));

	/* flush the state back */
	chip->clocks_left = clocks_left;
	chip->pos = pos;
}



/************************************************************

    DRQ callback

*************************************************************/

static TIMER_CALLBACK( upd7759_slave_update )
{
	upd7759_state *chip = (upd7759_state *)ptr;
	UINT8 olddrq = chip->drq;

	/* update the stream */
	chip->channel->update();

	/* advance the state */
	advance_state(chip);

	/* if the DRQ changed, update it */
	logerror("upd7759_slave_update: DRQ %d->%d\n", olddrq, chip->drq);
	if (olddrq != chip->drq && chip->drqcallback)
		(*chip->drqcallback)(chip->device, chip->drq);

	/* set a timer to go off when that is done */
	if (chip->state != STATE_IDLE)
		chip->timer->adjust(chip->clock_period * chip->clocks_left);
}



/************************************************************

    Sound startup

*************************************************************/

static void upd7759_reset(upd7759_state *chip)
{
	chip->pos                = 0;
	chip->fifo_in            = 0;
	chip->drq                = 0;
	chip->state              = STATE_IDLE;
	chip->clocks_left        = 0;
	chip->nibbles_left       = 0;
	chip->repeat_count       = 0;
	chip->post_drq_state     = STATE_IDLE;
	chip->post_drq_clocks    = 0;
	chip->req_sample         = 0;
	chip->last_sample        = 0;
	chip->block_header       = 0;
	chip->sample_rate        = 0;
	chip->first_valid_header = 0;
	chip->offset             = 0;
	chip->repeat_offset      = 0;
	chip->adpcm_state        = 0;
	chip->adpcm_data         = 0;
	chip->sample             = 0;

	/* turn off any timer */
	if (chip->timer)
		chip->timer->adjust(attotime::never);
}


static DEVICE_RESET( upd7759 )
{
	upd7759_reset(get_safe_token(device));
}


static void upd7759_postload(upd7759_state *chip)
{
	if (chip->rombase)
		chip->rom = chip->rombase + chip->romoffset;
}


static void register_for_save(upd7759_state *chip, device_t *device)
{
	device->save_item(NAME(chip->pos));
	device->save_item(NAME(chip->step));

	device->save_item(NAME(chip->fifo_in));
	device->save_item(NAME(chip->reset));
	device->save_item(NAME(chip->start));
	device->save_item(NAME(chip->drq));

	device->save_item(NAME(chip->state));
	device->save_item(NAME(chip->clocks_left));
	device->save_item(NAME(chip->nibbles_left));
	device->save_item(NAME(chip->repeat_count));
	device->save_item(NAME(chip->post_drq_state));
	device->save_item(NAME(chip->post_drq_clocks));
	device->save_item(NAME(chip->req_sample));
	device->save_item(NAME(chip->last_sample));
	device->save_item(NAME(chip->block_header));
	device->save_item(NAME(chip->sample_rate));
	device->save_item(NAME(chip->first_valid_header));
	device->save_item(NAME(chip->offset));
	device->save_item(NAME(chip->repeat_offset));

	device->save_item(NAME(chip->adpcm_state));
	device->save_item(NAME(chip->adpcm_data));
	device->save_item(NAME(chip->sample));

	device->save_item(NAME(chip->romoffset));
	device->machine().save().register_postload(save_prepost_delegate(FUNC(upd7759_postload), chip));
}


static DEVICE_START( upd7759 )
{
	static const upd7759_interface defintrf = { 0 };
	const upd7759_interface *intf = (device->static_config() != NULL) ? (const upd7759_interface *)device->static_config() : &defintrf;
	upd7759_state *chip = get_safe_token(device);

	chip->device = device;

	/* chip configuration */
	chip->sample_offset_shift = (device->type() == UPD7759) ? 1 : 0;
	
	/* allocate a stream channel */
	chip->channel = device->machine().sound().stream_alloc(*device, 0, 1, device->clock()/4, chip, upd7759_update);

	/* compute the stepping rate based on the chip's clock speed */
	chip->step = 4 * FRAC_ONE;

	/* compute the clock period */
	chip->clock_period = attotime::from_hz(device->clock());

	/* set the intial state */
	chip->state = STATE_IDLE;

	/* compute the ROM base or allocate a timer */
	chip->romoffset = 0;
	chip->rom = chip->rombase = *device->region();
	if (chip->rombase == NULL)
	{
		assert(device->type() == UPD7759); // other chips do not support slave mode
		chip->timer = device->machine().scheduler().timer_alloc(FUNC(upd7759_slave_update), chip);
		chip->rommask = 0;

		/* set the DRQ callback */
		chip->drqcallback = intf->drqcallback;
	}
	else
	{
		UINT32 romsize = device->region()->bytes();
		if (romsize >= 0x20000) chip->rommask = 0x1ffff;
		else chip->rommask = romsize - 1;

		chip->drqcallback = NULL;
	}

	/* assume /RESET and /START are both high */
	chip->reset = 1;
	chip->start = 1;

	/* toggle the reset line to finish the reset */
	upd7759_reset(chip);

	register_for_save(chip, device);
}



/************************************************************

    I/O handlers

*************************************************************/

void upd7759_reset_w(device_t *device, UINT8 data)
{
	/* update the reset value */
	upd7759_state *chip = get_safe_token(device);
	UINT8 oldreset = chip->reset;
	chip->reset = (data != 0);

	/* update the stream first */
	chip->channel->update();

	/* on the falling edge, reset everything */
	if (oldreset && !chip->reset)
		upd7759_reset(chip);
}

void upd7759_start_w(device_t *device, UINT8 data)
{
	/* update the start value */
	upd7759_state *chip = get_safe_token(device);
	UINT8 oldstart = chip->start;
	chip->start = (data != 0);

	logerror("upd7759_start_w: %d->%d\n", oldstart, chip->start);

	/* update the stream first */
	chip->channel->update();

	/* on the rising edge, if we're idle, start going, but not if we're held in reset */
	if (chip->state == STATE_IDLE && !oldstart && chip->start && chip->reset)
	{
		chip->state = STATE_START;

		/* for slave mode, start the timer going */
		if (chip->timer)
			chip->timer->adjust(attotime::zero);
	}
}


WRITE8_DEVICE_HANDLER( upd7759_port_w )
{
	/* update the FIFO value */
	upd7759_state *chip = get_safe_token(device);
	chip->fifo_in = data;
}


int upd7759_busy_r(device_t *device)
{
	/* return /BUSY */
	upd7759_state *chip = get_safe_token(device);
	return (chip->state == STATE_IDLE);
}


void upd7759_set_bank_base(device_t *device, UINT32 base)
{
	upd7759_state *chip = get_safe_token(device);
	assert(chip->rombase != NULL);
	chip->rom = chip->rombase + base;
	chip->romoffset = base;
}

const device_type UPD7759 = &device_creator<upd7759_device>;

upd7759_device::upd7759_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, UPD7759, "UPD7759", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(upd7759_state));
}
upd7759_device::upd7759_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(upd7759_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void upd7759_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd7759_device::device_start()
{
	DEVICE_START_NAME( upd7759 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd7759_device::device_reset()
{
	DEVICE_RESET_NAME( upd7759 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void upd7759_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type UPD7756 = &device_creator<upd7756_device>;

upd7756_device::upd7756_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: upd7759_device(mconfig, UPD7756, "UPD7756", tag, owner, clock)
{
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void upd7756_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}
