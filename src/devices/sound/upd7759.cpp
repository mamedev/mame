// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, Mike Balfour, Howie Cohen, Olivier Galibert, Aaron Giles
/************************************************************

    NEC uPD7759/55/56/P56/57/58 ADPCM Speech Processor
    by: Juergen Buchmueller, Mike Balfour, Howie Cohen,
        Olivier Galibert, and Aaron Giles

    TODO:
    - is there a doable method to dump the internal maskrom? :(
      As far as we know, decapping is the only option
    - low-level emulation
    - watchdog? - according to uPD775x datasheet, the chip goes into standy mode
      if CS/ST/RESET have not been accessed for more than 3 seconds
    - convert to MAME modern device

*************************************************************

    uPD7759 Description:

    The uPD7759 is a speech processing LSI that utilizes ADPCM to produce
    speech or other sampled sounds.  It can directly address up to 1Mbit
    (128k) of external data ROM, or the host CPU can control the speech
    data transfer.  The uPD7759 is usually hooked up to a 640 kHz clock and
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

    The uPD7759 can run in two modes, master (also known as standalone)
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

    It seems that the uPD7759 reads at least part of the rom header at
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
    P56CR  20-pin DIP  256 Kbit ROM (OTP) - dumping the ROM is trivial
    P56G   24-pin SOP  256 Kbit ROM (OTP) - "
    57C    18-pin DIP  512 Kbit ROM
    57G    24-pin SOP  512 Kbit ROM
    58C    18-pin DIP    1 Mbit ROM
    58G    24-pin SOP    1 Mbit ROM

*************************************************************/

#include "emu.h"
#include "upd7759.h"


#define DEBUG_STATES    (0)
#define DEBUG_METHOD    osd_printf_debug



/************************************************************

    Constants

*************************************************************/

/* step value fractional bits */
#define FRAC_BITS       20
#define FRAC_ONE        (1 << FRAC_BITS)
#define FRAC_MASK       (FRAC_ONE - 1)


upd775x_device::upd775x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_sound_interface(mconfig, *this),
		m_channel(NULL),
		m_sample_offset_shift(0),
		m_pos(0),
		m_step(0),
		m_fifo_in(0),
		m_reset(0),
		m_start(0),
		m_drq(0),
		m_state(0),
		m_clocks_left(0),
		m_nibbles_left(0),
		m_repeat_count(0),
		m_post_drq_state(0),
		m_post_drq_clocks(0),
		m_req_sample(0),
		m_last_sample(0),
		m_block_header(0),
		m_sample_rate(0),
		m_first_valid_header(0),
		m_offset(0),
		m_repeat_offset(0),
		m_adpcm_state(0),
		m_adpcm_data(0),
		m_sample(0),
		m_rom(NULL),
		m_rombase(NULL),
		m_romoffset(0),
		m_rommask(0),
		m_drqcallback(*this)
{
}

const device_type UPD7759 = &device_creator<upd7759_device>;

upd7759_device::upd7759_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: upd775x_device(mconfig, UPD7759, "uPD7759", tag, owner, clock, "upd7759", __FILE__),
		m_timer(NULL)
{
}


upd7759_device::upd7759_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: upd775x_device(mconfig, type, name, tag, owner, clock, shortname, source),
		m_timer(NULL)
{
}


const device_type UPD7756 = &device_creator<upd7756_device>;

upd7756_device::upd7756_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: upd775x_device(mconfig, UPD7756, "uPD7756", tag, owner, clock, "upd7756", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd775x_device::device_start()
{
}

void upd7759_device::device_start()
{
	m_drqcallback.resolve_safe();

	/* chip configuration */
	m_sample_offset_shift = (type() == UPD7759) ? 1 : 0;

	/* allocate a stream channel */
	m_channel = machine().sound().stream_alloc(*this, 0, 1, clock()/4);

	/* compute the stepping rate based on the chip's clock speed */
	m_step = 4 * FRAC_ONE;

	/* compute the clock period */
	m_clock_period = attotime::from_hz(clock());

	/* set the intial state */
	m_state = STATE_IDLE;

	/* compute the ROM base or allocate a timer */
	m_romoffset = 0;
	m_rom = m_rombase = region()->base();
	if (m_rombase == NULL)
	{
		assert(type() == UPD7759); // other chips do not support slave mode
		m_timer = timer_alloc(TIMER_SLAVE_UPDATE);
		m_rommask = 0;
	}
	else
	{
		UINT32 romsize = region()->bytes();
		if (romsize >= 0x20000) m_rommask = 0x1ffff;
		else m_rommask = romsize - 1;

		m_drqcallback.set_callback(DEVCB_NULL);
	}

	/* assume /RESET and /START are both high */
	m_reset = 1;
	m_start = 1;

	/* toggle the reset line to finish the reset */
	device_reset();

	save_item(NAME(m_pos));
	save_item(NAME(m_step));

	save_item(NAME(m_fifo_in));
	save_item(NAME(m_reset));
	save_item(NAME(m_start));
	save_item(NAME(m_drq));

	save_item(NAME(m_state));
	save_item(NAME(m_clocks_left));
	save_item(NAME(m_nibbles_left));
	save_item(NAME(m_repeat_count));
	save_item(NAME(m_post_drq_state));
	save_item(NAME(m_post_drq_clocks));
	save_item(NAME(m_req_sample));
	save_item(NAME(m_last_sample));
	save_item(NAME(m_block_header));
	save_item(NAME(m_sample_rate));
	save_item(NAME(m_first_valid_header));
	save_item(NAME(m_offset));
	save_item(NAME(m_repeat_offset));

	save_item(NAME(m_adpcm_state));
	save_item(NAME(m_adpcm_data));
	save_item(NAME(m_sample));

	save_item(NAME(m_romoffset));
	machine().save().register_postload(save_prepost_delegate(FUNC(upd7759_device::postload), this));
}


void upd7756_device::device_start()
{
	m_drqcallback.resolve_safe();

	/* chip configuration */
	m_sample_offset_shift = (type() == UPD7759) ? 1 : 0;

	/* allocate a stream channel */
	m_channel = machine().sound().stream_alloc(*this, 0, 1, clock()/4);

	/* compute the stepping rate based on the chip's clock speed */
	m_step = 4 * FRAC_ONE;

	/* compute the clock period */
	m_clock_period = attotime::from_hz(clock());

	/* set the intial state */
	m_state = STATE_IDLE;

	/* compute the ROM base or allocate a timer */
	m_romoffset = 0;
	m_rom = m_rombase = region()->base();
	if (m_rombase == NULL)
	{
		m_rommask = 0;
	}
	else
	{
		UINT32 romsize = region()->bytes();
		if (romsize >= 0x20000) m_rommask = 0x1ffff;
		else m_rommask = romsize - 1;

		m_drqcallback.set_callback(DEVCB_NULL);
	}

	/* assume /RESET and /START are both high */
	m_reset = 1;
	m_start = 1;

	/* toggle the reset line to finish the reset */
	device_reset();

	save_item(NAME(m_pos));
	save_item(NAME(m_step));

	save_item(NAME(m_fifo_in));
	save_item(NAME(m_reset));
	save_item(NAME(m_start));
	save_item(NAME(m_drq));

	save_item(NAME(m_state));
	save_item(NAME(m_clocks_left));
	save_item(NAME(m_nibbles_left));
	save_item(NAME(m_repeat_count));
	save_item(NAME(m_post_drq_state));
	save_item(NAME(m_post_drq_clocks));
	save_item(NAME(m_req_sample));
	save_item(NAME(m_last_sample));
	save_item(NAME(m_block_header));
	save_item(NAME(m_sample_rate));
	save_item(NAME(m_first_valid_header));
	save_item(NAME(m_offset));
	save_item(NAME(m_repeat_offset));

	save_item(NAME(m_adpcm_state));
	save_item(NAME(m_adpcm_data));
	save_item(NAME(m_sample));

	save_item(NAME(m_romoffset));
	machine().save().register_postload(save_prepost_delegate(FUNC(upd7759_device::postload), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd775x_device::device_reset()
{
}

void upd7759_device::device_reset()
{
	m_pos                = 0;
	m_fifo_in            = 0;
	m_drq                = 0;
	m_state              = STATE_IDLE;
	m_clocks_left        = 0;
	m_nibbles_left       = 0;
	m_repeat_count       = 0;
	m_post_drq_state     = STATE_IDLE;
	m_post_drq_clocks    = 0;
	m_req_sample         = 0;
	m_last_sample        = 0;
	m_block_header       = 0;
	m_sample_rate        = 0;
	m_first_valid_header = 0;
	m_offset             = 0;
	m_repeat_offset      = 0;
	m_adpcm_state        = 0;
	m_adpcm_data         = 0;
	m_sample             = 0;

	/* turn off any timer */
	if (m_timer)
		m_timer->adjust(attotime::never);
}

void upd7756_device::device_reset()
{
	m_pos                = 0;
	m_fifo_in            = 0;
	m_drq                = 0;
	m_state              = STATE_IDLE;
	m_clocks_left        = 0;
	m_nibbles_left       = 0;
	m_repeat_count       = 0;
	m_post_drq_state     = STATE_IDLE;
	m_post_drq_clocks    = 0;
	m_req_sample         = 0;
	m_last_sample        = 0;
	m_block_header       = 0;
	m_sample_rate        = 0;
	m_first_valid_header = 0;
	m_offset             = 0;
	m_repeat_offset      = 0;
	m_adpcm_state        = 0;
	m_adpcm_data         = 0;
	m_sample             = 0;
}


/************************************************************

    Local variables

*************************************************************/

static const int upd775x_step[16][16] =
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

static const int upd775x_state_table[16] = { -1, -1, 0, 0, 1, 2, 2, 3, -1, -1, 0, 0, 1, 2, 2, 3 };


/************************************************************

    ADPCM sample updater

*************************************************************/

void upd775x_device::update_adpcm(int data)
{
	/* update the sample and the state */
	m_sample += upd775x_step[m_adpcm_state][data];
	m_adpcm_state += upd775x_state_table[data];

	/* clamp the state to 0..15 */
	if (m_adpcm_state < 0)
		m_adpcm_state = 0;
	else if (m_adpcm_state > 15)
		m_adpcm_state = 15;
}



/************************************************************

    Master chip state machine

*************************************************************/

void upd775x_device::advance_state()
{
	switch (m_state)
	{
		/* Idle state: we stick around here while there's nothing to do */
		case STATE_IDLE:
			m_clocks_left = 4;
			break;

		/* drop DRQ state: update to the intended state */
		case STATE_DROP_DRQ:
			m_drq = 0;

			m_clocks_left = m_post_drq_clocks;
			m_state = m_post_drq_state;
			break;

		/* Start state: we begin here as soon as a sample is triggered */
		case STATE_START:
			m_req_sample = m_rom ? m_fifo_in : 0x10;
			if (DEBUG_STATES) DEBUG_METHOD("uPD7759: req_sample = %02X\n", m_req_sample);

			/* 35+ cycles after we get here, the /DRQ goes low
			 *     (first byte (number of samples in ROM) should be sent in response)
			 *
			 * (35 is the minimum number of cycles I found during heavy tests.
			 * Depending on the state the chip was in just before the /MD was set to 0 (reset, standby
			 * or just-finished-playing-previous-sample) this number can range from 35 up to ~24000).
			 * It also varies slightly from test to test, but not much - a few cycles at most.) */
			m_clocks_left = 70; /* 35 - breaks cotton */
			m_state = STATE_FIRST_REQ;
			break;

		/* First request state: issue a request for the first byte */
		/* The expected response will be the index of the last sample */
		case STATE_FIRST_REQ:
			if (DEBUG_STATES) DEBUG_METHOD("uPD7759: first data request\n");
			m_drq = 1;

			/* 44 cycles later, we will latch this value and request another byte */
			m_clocks_left = 44;
			m_state = STATE_LAST_SAMPLE;
			break;

		/* Last sample state: latch the last sample value and issue a request for the second byte */
		/* The second byte read will be just a dummy */
		case STATE_LAST_SAMPLE:
			m_last_sample = m_rom ? m_rom[0] : m_fifo_in;
			if (DEBUG_STATES) DEBUG_METHOD("uPD7759: last_sample = %02X, requesting dummy 1\n", m_last_sample);
			m_drq = 1;

			/* 28 cycles later, we will latch this value and request another byte */
			m_clocks_left = 28; /* 28 - breaks cotton */
			m_state = (m_req_sample > m_last_sample) ? STATE_IDLE : STATE_DUMMY1;
			break;

		/* First dummy state: ignore any data here and issue a request for the third byte */
		/* The expected response will be the MSB of the sample address */
		case STATE_DUMMY1:
			if (DEBUG_STATES) DEBUG_METHOD("uPD7759: dummy1, requesting offset_hi\n");
			m_drq = 1;

			/* 32 cycles later, we will latch this value and request another byte */
			m_clocks_left = 32;
			m_state = STATE_ADDR_MSB;
			break;

		/* Address MSB state: latch the MSB of the sample address and issue a request for the fourth byte */
		/* The expected response will be the LSB of the sample address */
		case STATE_ADDR_MSB:
			m_offset = (m_rom ? m_rom[m_req_sample * 2 + 5] : m_fifo_in) << (8 + m_sample_offset_shift);
			if (DEBUG_STATES) DEBUG_METHOD("uPD7759: offset_hi = %02X, requesting offset_lo\n", m_offset >> (8 + m_sample_offset_shift));
			m_drq = 1;

			/* 44 cycles later, we will latch this value and request another byte */
			m_clocks_left = 44;
			m_state = STATE_ADDR_LSB;
			break;

		/* Address LSB state: latch the LSB of the sample address and issue a request for the fifth byte */
		/* The expected response will be just a dummy */
		case STATE_ADDR_LSB:
			m_offset |= (m_rom ? m_rom[m_req_sample * 2 + 6] : m_fifo_in) << m_sample_offset_shift;
			if (DEBUG_STATES) DEBUG_METHOD("uPD7759: offset_lo = %02X, requesting dummy 2\n", (m_offset >> m_sample_offset_shift) & 0xff);
			if (m_offset > m_rommask) logerror("uPD7759 offset %X > rommask %X\n",m_offset, m_rommask);
			m_drq = 1;

			/* 36 cycles later, we will latch this value and request another byte */
			m_clocks_left = 36;
			m_state = STATE_DUMMY2;
			break;

		/* Second dummy state: ignore any data here and issue a request for the sixth byte */
		/* The expected response will be the first block header */
		case STATE_DUMMY2:
			m_offset++;
			m_first_valid_header = 0;
			if (DEBUG_STATES) DEBUG_METHOD("uPD7759: dummy2, requesting block header\n");
			m_drq = 1;

			/* 36?? cycles later, we will latch this value and request another byte */
			m_clocks_left = 36;
			m_state = STATE_BLOCK_HEADER;
			break;

		/* Block header state: latch the header and issue a request for the first byte afterwards */
		case STATE_BLOCK_HEADER:

			/* if we're in a repeat loop, reset the offset to the repeat point and decrement the count */
			if (m_repeat_count)
			{
				m_repeat_count--;
				m_offset = m_repeat_offset;
			}
			m_block_header = m_rom ? m_rom[m_offset++ & m_rommask] : m_fifo_in;
			if (DEBUG_STATES) DEBUG_METHOD("uPD7759: header (@%05X) = %02X, requesting next byte\n", m_offset, m_block_header);
			m_drq = 1;

			/* our next step depends on the top two bits */
			switch (m_block_header & 0xc0)
			{
				case 0x00:  /* silence */
					m_clocks_left = 1024 * ((m_block_header & 0x3f) + 1);
					m_state = (m_block_header == 0 && m_first_valid_header) ? STATE_IDLE : STATE_BLOCK_HEADER;
					m_sample = 0;
					m_adpcm_state = 0;
					break;

				case 0x40:  /* 256 nibbles */
					m_sample_rate = (m_block_header & 0x3f) + 1;
					m_nibbles_left = 256;
					m_clocks_left = 36; /* just a guess */
					m_state = STATE_NIBBLE_MSN;
					break;

				case 0x80:  /* n nibbles */
					m_sample_rate = (m_block_header & 0x3f) + 1;
					m_clocks_left = 36; /* just a guess */
					m_state = STATE_NIBBLE_COUNT;
					break;

				case 0xc0:  /* repeat loop */
					m_repeat_count = (m_block_header & 7) + 1;
					m_repeat_offset = m_offset;
					m_clocks_left = 36; /* just a guess */
					m_state = STATE_BLOCK_HEADER;
					break;
			}

			/* set a flag when we get the first non-zero header */
			if (m_block_header != 0)
				m_first_valid_header = 1;
			break;

		/* Nibble count state: latch the number of nibbles to play and request another byte */
		/* The expected response will be the first data byte */
		case STATE_NIBBLE_COUNT:
			m_nibbles_left = (m_rom ? m_rom[m_offset++ & m_rommask] : m_fifo_in) + 1;
			if (DEBUG_STATES) DEBUG_METHOD("uPD7759: nibble_count = %u, requesting next byte\n", (unsigned)m_nibbles_left);
			m_drq = 1;

			/* 36?? cycles later, we will latch this value and request another byte */
			m_clocks_left = 36; /* just a guess */
			m_state = STATE_NIBBLE_MSN;
			break;

		/* MSN state: latch the data for this pair of samples and request another byte */
		/* The expected response will be the next sample data or another header */
		case STATE_NIBBLE_MSN:
			m_adpcm_data = m_rom ? m_rom[m_offset++ & m_rommask] : m_fifo_in;
			update_adpcm(m_adpcm_data >> 4);
			m_drq = 1;

			/* we stay in this state until the time for this sample is complete */
			m_clocks_left = m_sample_rate * 4;
			if (--m_nibbles_left == 0)
				m_state = STATE_BLOCK_HEADER;
			else
				m_state = STATE_NIBBLE_LSN;
			break;

		/* LSN state: process the lower nibble */
		case STATE_NIBBLE_LSN:
			update_adpcm(m_adpcm_data & 15);

			/* we stay in this state until the time for this sample is complete */
			m_clocks_left = m_sample_rate * 4;
			if (--m_nibbles_left == 0)
				m_state = STATE_BLOCK_HEADER;
			else
				m_state = STATE_NIBBLE_MSN;
			break;
	}

	/* if there's a DRQ, fudge the state */
	if (m_drq)
	{
		m_post_drq_state = m_state;
		m_post_drq_clocks = m_clocks_left - 21;
		m_state = STATE_DROP_DRQ;
		m_clocks_left = 21;
	}
}

/************************************************************

    DRQ callback

*************************************************************/

void upd7759_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	UINT8 olddrq = m_drq;

	switch (id)
	{
		case TIMER_SLAVE_UPDATE:

		/* update the stream */
		m_channel->update();

		/* advance the state */
		advance_state();

		/* if the DRQ changed, update it */
		logerror("upd7759_slave_update: DRQ %d->%d\n", olddrq, m_drq);
		if (olddrq != m_drq)
			m_drqcallback(m_drq);

		/* set a timer to go off when that is done */
		if (m_state != STATE_IDLE)
			m_timer->adjust(m_clock_period * m_clocks_left);
			break;

		default:
			assert_always(FALSE, "Unknown id in upd7759_device::device_timer");
	}
}

/************************************************************

    Sound startup

*************************************************************/

void upd775x_device::postload()
{
	if (m_rombase)
		m_rom = m_rombase + m_romoffset;
}

/************************************************************

    I/O handlers

*************************************************************/

WRITE_LINE_MEMBER( upd775x_device::reset_w )
{
	/* update the reset value */
	UINT8 oldreset = m_reset;
	m_reset = (state != 0);

	/* update the stream first */
	m_channel->update();

	/* on the falling edge, reset everything */
	if (oldreset && !m_reset)
		device_reset();
}

WRITE_LINE_MEMBER( upd7759_device::start_w )
{
	/* update the start value */
	UINT8 oldstart = m_start;
	m_start = (state != 0);

	logerror("upd7759_start_w: %d->%d\n", oldstart, m_start);

	/* update the stream first */
	m_channel->update();

	/* on the rising edge, if we're idle, start going, but not if we're held in reset */
	if (m_state == STATE_IDLE && !oldstart && m_start && m_reset)
	{
		m_state = STATE_START;

		/* for slave mode, start the timer going */
		if (m_timer)
			m_timer->adjust(attotime::zero);
	}
}

WRITE_LINE_MEMBER( upd7756_device::start_w )
{
	/* update the start value */
	UINT8 oldstart = m_start;
	m_start = (state != 0);

	logerror("upd7759_start_w: %d->%d\n", oldstart, m_start);

	/* update the stream first */
	m_channel->update();

	/* on the rising edge, if we're idle, start going, but not if we're held in reset */
	if (m_state == STATE_IDLE && !oldstart && m_start && m_reset)
	{
		m_state = STATE_START;
	}
}


WRITE8_MEMBER( upd775x_device::port_w )
{
	/* update the FIFO value */
	m_fifo_in = data;
}


READ_LINE_MEMBER( upd775x_device::busy_r )
{
	/* return /BUSY */
	return (m_state == STATE_IDLE);
}


void upd775x_device::set_bank_base(UINT32 base)
{
	assert(m_rombase != NULL);
	m_rom = m_rombase + base;
	m_romoffset = base;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void upd775x_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	INT32 clocks_left = m_clocks_left;
	INT16 sample = m_sample;
	UINT32 step = m_step;
	UINT32 pos = m_pos;
	stream_sample_t *buffer = outputs[0];

	/* loop until done */
	if (m_state != STATE_IDLE)
		while (samples != 0)
		{
			/* store the current sample */
			*buffer++ = sample << 7;
			samples--;

			/* advance by the number of clocks/output sample */
			pos += step;

			/* handle clocks, but only in standalone mode */
			while (m_rom && pos >= FRAC_ONE)
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
					advance_state();
					if (m_state == STATE_IDLE)
						break;

					/* reimport the variables that we cached */
					clocks_left = m_clocks_left;
					sample = m_sample;
				}
			}
		}

	/* if we got out early, just zap the rest of the buffer */
	if (samples != 0)
		memset(buffer, 0, samples * sizeof(*buffer));

	/* flush the state back */
	m_clocks_left = clocks_left;
	m_pos = pos;
}

void upd7759_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	upd775x_device::sound_stream_update(stream, inputs, outputs, samples);
}

void upd7756_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	upd775x_device::sound_stream_update(stream, inputs, outputs, samples);
}
