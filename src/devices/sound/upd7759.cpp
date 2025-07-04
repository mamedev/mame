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


#define MASK_LOG_STATE (1U << 1)
#define MASK_LOG_DRQ   (1U << 2)
//#define VERBOSE (MASK_LOG_STATE|MASK_LOG_DRQ)

#include "logmacro.h"

#define LOG_STATE(...)  LOGMASKED(MASK_LOG_STATE, __VA_ARGS__)
#define LOG_DRQ(...)    LOGMASKED(MASK_LOG_DRQ, __VA_ARGS__)

/************************************************************

    Constants

*************************************************************/

// step value fractional bits
#define FRAC_BITS       20
#define FRAC_ONE        (1 << FRAC_BITS)
#define FRAC_MASK       (FRAC_ONE - 1)


upd775x_device::upd775x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_channel(nullptr)
	, m_sample_offset_shift(0)
	, m_pos(0)
	, m_step(0)
	, m_fifo_in(0)
	, m_reset(1)
	, m_start(1)
	, m_drq(0)
	, m_state(STATE_IDLE)
	, m_clocks_left(0)
	, m_nibbles_left(0)
	, m_repeat_count(0)
	, m_post_drq_state(0)
	, m_post_drq_clocks(0)
	, m_req_sample(0)
	, m_last_sample(0)
	, m_block_header(0)
	, m_sample_rate(0)
	, m_first_valid_header(0)
	, m_offset(0)
	, m_repeat_offset(0)
	, m_start_delay(0)
	, m_mode(MODE_STAND_ALONE)
	, m_adpcm_state(0)
	, m_adpcm_data(0)
	, m_sample(0)
	, m_md(1)
{
}

DEFINE_DEVICE_TYPE(UPD7759, upd7759_device, "upd7759", "NEC uPD7759")

upd7759_device::upd7759_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: upd7759_device(mconfig, UPD7759, tag, owner, clock)
{
}


upd7759_device::upd7759_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: upd775x_device(mconfig, type, tag, owner, clock)
	, m_drqcallback(*this)
	, m_timer(nullptr)
{
}


DEFINE_DEVICE_TYPE(UPD7756, upd7756_device, "upd7756", "NEC uPD7756")

upd7756_device::upd7756_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: upd7756_device(mconfig, UPD7756, tag, owner, clock)
{
}

upd7756_device::upd7756_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: upd775x_device(mconfig, type, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd775x_device::device_start()
{
	m_channel = stream_alloc(0, 1, clock()/4);

	m_step = 4 * FRAC_ONE;

	m_clock_period = clock() ? attotime::from_hz(clock()) : attotime::zero;

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
	save_item(NAME(m_mode));
	save_item(NAME(m_md));
}

void upd775x_device::device_clock_changed()
{
	m_clock_period = clock() ? attotime::from_hz(clock()) : attotime::zero;

	m_channel->set_sample_rate(clock() / 4);
}

void upd775x_device::rom_bank_pre_change()
{
	m_channel->update();
}

void upd7759_device::device_start()
{
	upd775x_device::device_start();

	m_sample_offset_shift = 1;

	m_timer = timer_alloc(FUNC(upd7759_device::drq_update), this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd775x_device::device_reset()
{
	m_pos                = 0;
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
	m_mode               = MODE_STAND_ALONE;
}

void upd7759_device::device_reset()
{
	upd775x_device::device_reset();

	m_timer->adjust(attotime::never);

	if (m_drq)
	{
		m_drq = 0;
		m_drqcallback(m_drq);
	}
}

void upd7756_device::device_reset()
{
	upd775x_device::device_reset();

	m_drq = 0;
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
	m_sample += upd775x_step[m_adpcm_state][data];
	m_adpcm_state += upd775x_state_table[data];

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
		// Idle state: we stick around here while there's nothing to do
		case STATE_IDLE:
			m_clocks_left = 4;
			break;

		// drop DRQ state: update to the intended state
		case STATE_DROP_DRQ:
			m_drq = 0;

			m_clocks_left = m_post_drq_clocks;
			m_state = m_post_drq_state;
			break;

		// Start state: we begin here as soon as a sample is triggered
		case STATE_START:
			m_req_sample = (m_mode == MODE_STAND_ALONE) ? m_fifo_in : 0x10;
			LOG_STATE("req_sample = %02X\n", m_req_sample);

			/* 35+ cycles after we get here, the /DRQ goes low
			 *     (first byte (number of samples in ROM) should be sent in response)
			 *
			 * (35 is the minimum number of cycles I found during heavy tests.
			 * Depending on the state the chip was in just before the /MD was set to 0 (reset, standby
			 * or just-finished-playing-previous-sample) this number can range from 35 up to ~24000).
			 * It also varies slightly from test to test, but not much - a few cycles at most.) */
			m_clocks_left = 70 + m_start_delay;
			m_state = STATE_FIRST_REQ;
			break;

		/* First request state: issue a request for the first byte */
		/* The expected response will be the index of the last sample */
		case STATE_FIRST_REQ:
			LOG_STATE("first data request\n");
			m_drq = 1;

			m_clocks_left = 44;
			m_state = STATE_LAST_SAMPLE;
			break;

		/* Last sample state: latch the last sample value and issue a request for the second byte */
		/* The second byte read will be just a dummy */
		case STATE_LAST_SAMPLE:
			m_last_sample = (m_mode == MODE_STAND_ALONE) ? read_byte(0) : m_fifo_in;
			LOG_STATE("last_sample = %02X, requesting dummy 1\n", m_last_sample);
			m_drq = 1;

			m_clocks_left = 28;
			m_state = (m_req_sample > m_last_sample) ? STATE_IDLE : STATE_DUMMY1;
			break;

		/* First dummy state: ignore any data here and issue a request for the third byte */
		/* The expected response will be the MSB of the sample address */
		case STATE_DUMMY1:
			LOG_STATE("dummy1, requesting offset_hi\n");
			m_drq = 1;

			m_clocks_left = 32;
			m_state = STATE_ADDR_MSB;
			break;

		/* Address MSB state: latch the MSB of the sample address and issue a request for the fourth byte */
		/* The expected response will be the LSB of the sample address */
		case STATE_ADDR_MSB:
			m_offset = ((m_mode == MODE_STAND_ALONE) ? read_byte(m_req_sample * 2 + 5) : m_fifo_in) << (8 + m_sample_offset_shift);
			LOG_STATE("offset_hi = %02X, requesting offset_lo\n", m_offset >> (8 + m_sample_offset_shift));
			m_drq = 1;

			m_clocks_left = 44;
			m_state = STATE_ADDR_LSB;
			break;

		/* Address LSB state: latch the LSB of the sample address and issue a request for the fifth byte */
		/* The expected response will be just a dummy */
		case STATE_ADDR_LSB:
			m_offset |= ((m_mode == MODE_STAND_ALONE) ? read_byte(m_req_sample * 2 + 6) : m_fifo_in) << m_sample_offset_shift;
			LOG_STATE("offset_lo = %02X, requesting dummy 2\n", (m_offset >> m_sample_offset_shift) & 0xff);
			m_drq = 1;

			m_clocks_left = 36;
			m_state = STATE_DUMMY2;
			break;

		/* Second dummy state: ignore any data here and issue a request for the sixth byte */
		/* The expected response will be the first block header */
		case STATE_DUMMY2:
			m_offset++;
			m_first_valid_header = 0;
			LOG_STATE("dummy2, requesting block header\n");
			m_drq = 1;

			m_clocks_left = 36;
			m_state = STATE_BLOCK_HEADER;
			break;

		// Block header state: latch the header and issue a request for the first byte afterwards
		case STATE_BLOCK_HEADER:

			if (m_repeat_count)
			{
				m_repeat_count--;
				m_offset = m_repeat_offset;
			}
			m_block_header = (m_mode == MODE_STAND_ALONE) ? read_byte(m_offset++) : m_fifo_in;
			LOG_STATE("header (@%05X) = %02X, requesting next byte\n", m_offset, m_block_header);
			m_drq = 1;

			switch (m_block_header & 0xc0)
			{
				case 0x00:  // silence
					m_clocks_left = 1024 * ((m_block_header & 0x3f) + 1);
					m_state = (m_block_header == 0 && m_first_valid_header) ? STATE_IDLE : STATE_BLOCK_HEADER;
					m_sample = 0;
					m_adpcm_state = 0;
					break;

				case 0x40:  // 256 nibbles
					m_sample_rate = (m_block_header & 0x3f) + 1;
					m_nibbles_left = 256;
					m_clocks_left = 36; // just a guess
					m_state = STATE_NIBBLE_MSN;
					break;

				case 0x80:  // n nibbles
					m_sample_rate = (m_block_header & 0x3f) + 1;
					m_clocks_left = 36; // just a guess
					m_state = STATE_NIBBLE_COUNT;
					break;

				case 0xc0:  // repeat loop
					m_repeat_count = (m_block_header & 7) + 1;
					m_repeat_offset = m_offset;
					m_clocks_left = 36; // just a guess
					m_state = STATE_BLOCK_HEADER;
					break;
			}

			if (m_block_header != 0)
				m_first_valid_header = 1;
			break;

		/* Nibble count state: latch the number of nibbles to play and request another byte */
		/* The expected response will be the first data byte */
		case STATE_NIBBLE_COUNT:
			m_nibbles_left = ((m_mode == MODE_STAND_ALONE) ? read_byte(m_offset++) : m_fifo_in) + 1;
			LOG_STATE("nibble_count = %u, requesting next byte\n", (unsigned)m_nibbles_left);
			m_drq = 1;

			m_clocks_left = 36; // just a guess
			m_state = STATE_NIBBLE_MSN;
			break;

		/* MSN state: latch the data for this pair of samples and request another byte */
		/* The expected response will be the next sample data or another header */
		case STATE_NIBBLE_MSN:
			m_adpcm_data = (m_mode == MODE_STAND_ALONE) ? read_byte(m_offset++) : m_fifo_in;
			update_adpcm(m_adpcm_data >> 4);
			m_drq = 1;

			m_clocks_left = m_sample_rate * 4;
			if (--m_nibbles_left == 0)
				m_state = STATE_BLOCK_HEADER;
			else
				m_state = STATE_NIBBLE_LSN;
			break;

		// LSN state: process the lower nibble
		case STATE_NIBBLE_LSN:
			update_adpcm(m_adpcm_data & 15);

			m_clocks_left = m_sample_rate * 4;
			if (--m_nibbles_left == 0)
				m_state = STATE_BLOCK_HEADER;
			else
				m_state = STATE_NIBBLE_MSN;
			break;
	}

	if (m_drq)
	{
		m_post_drq_state = m_state;
		m_post_drq_clocks = m_clocks_left - 21;
		m_state = STATE_DROP_DRQ;
		m_clocks_left = 21;
	}
}

TIMER_CALLBACK_MEMBER(upd7759_device::drq_update)
{
	m_channel->update();

	uint8_t olddrq = m_drq;
	int old_state = m_state;

	advance_state();

	LOG_STATE("upd7759_slave_update: DRQ %d->%d\n", olddrq, m_drq);
	if (olddrq != m_drq)
	{
		LOG_DRQ("DRQ changed %d->%d\n", olddrq, m_drq);
		m_drqcallback(m_drq);
	}

	if (m_state != STATE_IDLE || old_state != STATE_IDLE)
		m_timer->adjust(m_clock_period * m_clocks_left);
}



/************************************************************

    I/O handlers

*************************************************************/

void upd775x_device::reset_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(upd775x_device::internal_reset_w), this), state);
}

TIMER_CALLBACK_MEMBER(upd775x_device::internal_reset_w)
{
	m_channel->update();

	uint8_t oldreset = m_reset;
	m_reset = (param != 0);

	if (oldreset && !m_reset)
		device_reset();
}


void upd775x_device::start_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(upd775x_device::internal_start_w), this), state);
}

void upd775x_device::internal_start_w(int state)
{
	m_channel->update();

	uint8_t oldstart = m_start;
	m_start = (state != 0);

	LOG_STATE("upd7759_start_w: %d->%d\n", oldstart, m_start);

	if (m_state == STATE_IDLE && m_mode == MODE_STAND_ALONE && oldstart && !m_start && m_reset)
	{
		m_state = STATE_START;
	}
}


void upd775x_device::port_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(upd775x_device::internal_port_w), this), data);
}

TIMER_CALLBACK_MEMBER(upd775x_device::internal_port_w)
{
	m_channel->update();

	m_fifo_in = param;
}


void upd7759_device::md_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(upd7759_device::internal_md_w), this), state);
}

TIMER_CALLBACK_MEMBER(upd7759_device::internal_md_w)
{
	m_channel->update();

	uint8_t old_md = m_md;
	m_md = (param != 0);

	LOG_STATE("upd7759_md_w: %d->%d\n", old_md, m_md);

	if (m_state == STATE_IDLE && m_reset)
	{
		if (old_md && !m_md)
		{
			m_mode = MODE_SLAVE;
			m_state = STATE_START;
			m_timer->adjust(attotime::zero);
		}
		else if (!old_md && m_md)
		{
			m_mode = MODE_STAND_ALONE;
		}
	}
}


int upd775x_device::busy_r()
{
	m_channel->update();

	// return /BUSY
	return (m_state == STATE_IDLE);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void upd775x_device::sound_stream_update(sound_stream &stream)
{
	constexpr sound_stream::sample_t sample_scale = 128.0 / 32768.0;
	sound_stream::sample_t sample = sound_stream::sample_t(m_sample) * sample_scale;
	int32_t clocks_left = m_clocks_left;
	uint32_t step = m_step;
	uint32_t pos = m_pos;

	u32 index = 0;
	if (m_state != STATE_IDLE)
		for ( ; index < stream.samples(); index++)
		{
			stream.put(0, index, sample);

			pos += step;

			while ((m_mode == MODE_STAND_ALONE) && pos >= FRAC_ONE)
			{
				int clocks_this_time = pos >> FRAC_BITS;
				if (clocks_this_time > clocks_left)
					clocks_this_time = clocks_left;

				pos -= clocks_this_time * FRAC_ONE;
				clocks_left -= clocks_this_time;

				if (clocks_left == 0)
				{
					advance_state();
					if (m_state == STATE_IDLE)
						break;

					clocks_left = m_clocks_left;
					sample = sound_stream::sample_t(m_sample) * sample_scale;
				}
			}
		}

	// if we got out early, just zap the rest of the buffer
	for (; index < stream.samples(); index++)
		stream.put(0, index, 0);

	m_clocks_left = clocks_left;
	m_pos = pos;
}
