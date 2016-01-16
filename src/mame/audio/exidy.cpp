// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Exidy 6502 hardware

*************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/rescap.h"
#include "cpu/m6502/m6502.h"
#include "audio/exidy.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define CRYSTAL_OSC             (XTAL_3_579545MHz)
#define SH8253_CLOCK            (CRYSTAL_OSC / 2)
#define SH6840_CLOCK            (CRYSTAL_OSC / 4)
#define SH6532_CLOCK            (CRYSTAL_OSC / 4)
#define CVSD_CLOCK              (1.0 / (0.693 * (RES_K(2.4) + 2.0 * RES_K(20)) * CAP_P(2200)))
#define CVSD_Z80_CLOCK          (CRYSTAL_OSC / 2)
#define BASE_VOLUME             (32767 / 6)

enum
{
	RIOT_IDLE,
	RIOT_COUNT,
	RIOT_POST_COUNT
};


/*************************************
 *
 *  Interrupt generation helper
 *
 *************************************/

WRITE_LINE_MEMBER( exidy_sound_device::update_irq_state )
{
	machine().device("audiocpu")->execute().set_input_line(M6502_IRQ_LINE, (m_pia1->irq_b_state() | m_riot_irq_state) ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  6840 clock counting helper
 *
 *************************************/

static inline void sh6840_apply_clock(struct sh6840_timer_channel *t, int clocks)
{
	/* dual 8-bit case */
	if (t->cr & 0x04)
	{
		/* handle full decrements */
		while (clocks > t->counter.b.l)
		{
			clocks -= t->counter.b.l + 1;
			t->counter.b.l = t->timer;

			/* decrement MSB */
			if (!t->counter.b.h--)
			{
				t->state = 0;
				t->counter.w = t->timer;
			}

			/* state goes high when MSB is 0 */
			else if (!t->counter.b.h)
			{
				t->state = 1;
				t->clocks++;
			}
		}

		/* subtract off the remainder */
		t->counter.b.l -= clocks;
	}

	/* 16-bit case */
	else
	{
		/* handle full decrements */
		while (clocks > t->counter.w)
		{
			clocks -= t->counter.w + 1;
			t->state ^= 1;
			t->clocks += t->state;
			t->counter.w = t->timer;
		}

		/* subtract off the remainder */
		t->counter.w -= clocks;
	}
}



/*************************************
 *
 *  Noise generation helper
 *
 *************************************/

inline int exidy_sound_device::sh6840_update_noise(int clocks)
{
	UINT32 newxor;
	int noise_clocks = 0;
	int i;

	/* loop over clocks */
	for (i = 0; i < clocks; i++)
	{
		/* shift the LFSR. its a LOOOONG LFSR, so we need
		* four longs to hold it all!
		* first we grab new sample, then shift the high bits,
		* then the low ones; finally or in the result and see if we've
		* had a 0->1 transition */
		newxor = (m_sh6840_LFSR_3 ^ m_sh6840_LFSR_2) >> 31; /* high bits of 3 and 2 xored is new xor */
		m_sh6840_LFSR_3 <<= 1;
		m_sh6840_LFSR_3 |= m_sh6840_LFSR_2 >> 31;
		m_sh6840_LFSR_2 <<= 1;
		m_sh6840_LFSR_2 |= m_sh6840_LFSR_1 >> 31;
		m_sh6840_LFSR_1 <<= 1;
		m_sh6840_LFSR_1 |= m_sh6840_LFSR_0 >> 31;
		m_sh6840_LFSR_0 <<= 1;
		m_sh6840_LFSR_0 |= newxor ^ m_sh6840_LFSR_oldxor;
		m_sh6840_LFSR_oldxor = newxor;
		/*printf("LFSR: %4x, %4x, %4x, %4x\n", sh6840_LFSR_3, sh6840_LFSR_2, sh6840_LFSR_1, sh6840_LFSR_0);*/
		/* if we clocked 0->1, that will serve as an external clock */
		if ((m_sh6840_LFSR_2 & 0x03) == 0x01) /* tap is at 96th bit */
		{
			noise_clocks++;
		}
	}
	return noise_clocks;
}



/*************************************
 *
 *  6840 state saving
 *
 *************************************/

void exidy_sound_device::sh6840_register_state_globals()
{
	save_item(NAME(m_sh6840_volume));
	save_item(NAME(m_sh6840_MSB_latch));
	save_item(NAME(m_sh6840_LSB_latch));
	save_item(NAME(m_sh6840_LFSR_oldxor));
	save_item(NAME(m_sh6840_LFSR_0));
	save_item(NAME(m_sh6840_LFSR_1));
	save_item(NAME(m_sh6840_LFSR_2));
	save_item(NAME(m_sh6840_LFSR_3));
	save_item(NAME(m_sh6840_clock_count));
	save_item(NAME(m_sfxctrl));
	save_item(NAME(m_sh6840_timer[0].cr));
	save_item(NAME(m_sh6840_timer[0].state));
	save_item(NAME(m_sh6840_timer[0].leftovers));
	save_item(NAME(m_sh6840_timer[0].timer));
	save_item(NAME(m_sh6840_timer[0].clocks));
	save_item(NAME(m_sh6840_timer[0].counter.w));
	save_item(NAME(m_sh6840_timer[1].cr));
	save_item(NAME(m_sh6840_timer[1].state));
	save_item(NAME(m_sh6840_timer[1].leftovers));
	save_item(NAME(m_sh6840_timer[1].timer));
	save_item(NAME(m_sh6840_timer[1].clocks));
	save_item(NAME(m_sh6840_timer[1].counter.w));
	save_item(NAME(m_sh6840_timer[2].cr));
	save_item(NAME(m_sh6840_timer[2].state));
	save_item(NAME(m_sh6840_timer[2].leftovers));
	save_item(NAME(m_sh6840_timer[2].timer));
	save_item(NAME(m_sh6840_timer[2].clocks));
	save_item(NAME(m_sh6840_timer[2].counter.w));
}

/*************************************
 *
 *  Audio startup routines
 *
 *************************************/

void exidy_sound_device::common_sh_start()
{
	int sample_rate = SH8253_CLOCK;

	m_sh6840_clocks_per_sample = (int)((double)SH6840_CLOCK / (double)sample_rate * (double)(1 << 24));

	/* allocate the stream */
	m_stream = machine().sound().stream_alloc(*this, 0, 1, sample_rate);
	m_maincpu = machine().device<cpu_device>("maincpu");

	sh6840_register_state_globals();
}

const device_type EXIDY = &device_creator<exidy_sound_device>;

exidy_sound_device::exidy_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, EXIDY, "Exidy SFX", tag, owner, clock, "exidy_sfx", __FILE__),
		device_sound_interface(mconfig, *this)
{
}

exidy_sound_device::exidy_sound_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_sound_interface(mconfig, *this),
		m_riot_irq_state(0),
		m_stream(nullptr),
		m_freq_to_step(0),
		m_sh6840_MSB_latch(0),
		m_sh6840_LSB_latch(0),
		m_sh6840_LFSR_oldxor(0),
		m_sh6840_LFSR_0(0xffffffff),
		m_sh6840_LFSR_1(0xffffffff),
		m_sh6840_LFSR_2(0xffffffff),
		m_sh6840_LFSR_3(0xffffffff),
		m_sh6840_clocks_per_sample(0),
		m_sh6840_clock_count(0),
		m_sfxctrl(0)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void exidy_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void exidy_sound_device::device_start()
{
	/* indicate no additional hardware */
	m_has_sh8253  = FALSE;
	m_tms = nullptr;
	m_cvsd = nullptr;

	common_sh_start();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void exidy_sound_device::device_reset()
{
	common_sh_reset();
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void exidy_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	struct sh6840_timer_channel *sh6840_timer = m_sh6840_timer;

	/* hack to skip the expensive lfsr noise generation unless at least one of the 3 channels actually depends on it being generated */
	int noisy = ((sh6840_timer[0].cr & sh6840_timer[1].cr & sh6840_timer[2].cr & 0x02) == 0);
	stream_sample_t *buffer = outputs[0];

	/* loop over samples */
	while (samples--)
	{
		struct sh6840_timer_channel *t;
		struct sh8253_timer_channel *c;
		int clocks_this_sample;
		int clocks;
		INT16 sample = 0;

		/* determine how many 6840 clocks this sample */
		m_sh6840_clock_count += m_sh6840_clocks_per_sample;
		clocks_this_sample = m_sh6840_clock_count >> 24;
		m_sh6840_clock_count &= (1 << 24) - 1;

		/* skip if nothing enabled */
		if ((sh6840_timer[0].cr & 0x01) == 0)
		{
			int noise_clocks_this_sample = 0;
			UINT32 chan0_clocks;

			/* generate E-clocked noise if configured to do so */
			if (noisy && !(m_sfxctrl & 0x01))
				noise_clocks_this_sample = sh6840_update_noise(clocks_this_sample);

			/* handle timer 0 if enabled */
			t = &sh6840_timer[0];
			chan0_clocks = t->clocks;
			clocks = (t->cr & 0x02) ? clocks_this_sample : noise_clocks_this_sample;
			sh6840_apply_clock(t, clocks);
			if (t->state && !(m_sfxctrl & 0x02) && (t->cr & 0x80))
				sample += m_sh6840_volume[0];

			/* generate channel 0-clocked noise if configured to do so */
			if (noisy && (m_sfxctrl & 0x01))
				noise_clocks_this_sample = sh6840_update_noise(t->clocks - chan0_clocks);

			/* handle timer 1 if enabled */
			t = &sh6840_timer[1];
			clocks = (t->cr & 0x02) ? clocks_this_sample : noise_clocks_this_sample;
			sh6840_apply_clock(t, clocks);
			if (t->state && (t->cr & 0x80))
				sample += m_sh6840_volume[1];

			/* handle timer 2 if enabled */
			t = &sh6840_timer[2];
			clocks = (t->cr & 0x02) ? clocks_this_sample : noise_clocks_this_sample;
			/* prescale */
			if (t->cr & 0x01)
			{
				clocks += t->leftovers;
				t->leftovers = clocks % 8;
				clocks /= 8;
			}
			sh6840_apply_clock(t, clocks);
			if (t->state && (t->cr & 0x80))
				sample += m_sh6840_volume[2];
		}

		/* music (if present) */
		if (m_has_sh8253)
		{
			/* music channel 0 */
			c = &m_sh8253_timer[0];
			if (c->enable)
			{
				c->fraction += c->step;
				if (c->fraction & 0x0800000)
					sample += BASE_VOLUME;
			}

			/* music channel 1 */
			c = &m_sh8253_timer[1];
			if (c->enable)
			{
				c->fraction += c->step;
				if (c->fraction & 0x0800000)
					sample += BASE_VOLUME;
			}

			/* music channel 2 */
			c = &m_sh8253_timer[2];
			if (c->enable)
			{
				c->fraction += c->step;
				if (c->fraction & 0x0800000)
					sample += BASE_VOLUME;
			}
		}

		/* stash */
		*buffer++ = sample;
	}
}



/*************************************
 *
 *  Audio reset routines
 *
 *************************************/

void exidy_sound_device::common_sh_reset()
{
	/* 6840 */
	memset(m_sh6840_timer, 0, sizeof(m_sh6840_timer));
	m_sh6840_MSB_latch = 0;
	m_sh6840_LSB_latch = 0;
	m_sh6840_volume[0] = 0;
	m_sh6840_volume[1] = 0;
	m_sh6840_volume[2] = 0;
	m_sh6840_clock_count = 0;
	m_sfxctrl = 0;

	/* LFSR */
	m_sh6840_LFSR_oldxor = 0;
	m_sh6840_LFSR_0 = 0xffffffff;
	m_sh6840_LFSR_1 = 0xffffffff;
	m_sh6840_LFSR_2 = 0xffffffff;
	m_sh6840_LFSR_3 = 0xffffffff;
}


/*************************************
 *
 *  6532 interface
 *
 *************************************/

void exidy_sound_device::r6532_irq(int state)
{
	m_riot_irq_state = (state == ASSERT_LINE) ? 1 : 0;
	update_irq_state(0);
}


WRITE8_MEMBER( exidy_sound_device::r6532_porta_w )
{
	if (m_cvsd != nullptr)
		space.machine().device("cvsdcpu")->execute().set_input_line(INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);

	if (m_tms != nullptr)
	{
		logerror("(%f)%s:TMS5220 data write = %02X\n", space.machine().time().as_double(), space.machine().describe_context(), m_riot->porta_out_get());
		m_tms->data_w(space, 0, data);
	}
}

READ8_MEMBER( exidy_sound_device::r6532_porta_r )
{
	if (m_tms != nullptr)
	{
		logerror("(%f)%s:TMS5220 status read = %02X\n", space.machine().time().as_double(), space.machine().describe_context(), m_tms->status_r(space, 0));
		return m_tms->status_r(space, 0);
	}
	else
		return 0xff;
}

WRITE8_MEMBER( exidy_sound_device::r6532_portb_w )
{
	if (m_tms != nullptr)
	{
		m_tms->rsq_w(data & 0x01);
		m_tms->wsq_w((data >> 1) & 0x01);
	}
}


READ8_MEMBER( exidy_sound_device::r6532_portb_r )
{
	UINT8 newdata = m_riot->portb_in_get();
	if (m_tms != nullptr)
	{
		newdata &= ~0x0c;
		if (m_tms->readyq_r()) newdata |= 0x04;
		if (m_tms->intq_r()) newdata |= 0x08;
	}
	return newdata;
}


/*************************************
 *
 *  8253 state saving
 *
 *************************************/


void exidy_sound_device::sh8253_register_state_globals()
{
	save_item(NAME(m_sh8253_timer[0].clstate));
	save_item(NAME(m_sh8253_timer[0].enable));
	save_item(NAME(m_sh8253_timer[0].count));
	save_item(NAME(m_sh8253_timer[0].step));
	save_item(NAME(m_sh8253_timer[0].fraction));
	save_item(NAME(m_sh8253_timer[1].clstate));
	save_item(NAME(m_sh8253_timer[1].enable));
	save_item(NAME(m_sh8253_timer[1].count));
	save_item(NAME(m_sh8253_timer[1].step));
	save_item(NAME(m_sh8253_timer[1].fraction));
	save_item(NAME(m_sh8253_timer[2].clstate));
	save_item(NAME(m_sh8253_timer[2].enable));
	save_item(NAME(m_sh8253_timer[2].count));
	save_item(NAME(m_sh8253_timer[2].step));
	save_item(NAME(m_sh8253_timer[2].fraction));
}

/*************************************
 *
 *  8253 timer handlers
 *
 *************************************/

WRITE8_MEMBER( exidy_sound_device::sh8253_w )
{
	int chan;

	m_stream->update();

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
			chan = offset;
			if (!m_sh8253_timer[chan].clstate)
			{
				m_sh8253_timer[chan].clstate = 1;
				m_sh8253_timer[chan].count = (m_sh8253_timer[chan].count & 0xff00) | (data & 0x00ff);
			}
			else
			{
				m_sh8253_timer[chan].clstate = 0;
				m_sh8253_timer[chan].count = (m_sh8253_timer[chan].count & 0x00ff) | ((data << 8) & 0xff00);
				if (m_sh8253_timer[chan].count)
					m_sh8253_timer[chan].step = m_freq_to_step * (double)SH8253_CLOCK / (double)m_sh8253_timer[chan].count;
				else
					m_sh8253_timer[chan].step = 0;
			}
			break;

		case 3:
			chan = (data & 0xc0) >> 6;
			m_sh8253_timer[chan].enable = ((data & 0x0e) != 0);
			break;
	}
}


READ8_MEMBER( exidy_sound_device::sh8253_r )
{
	logerror("8253(R): %x\n",offset);

	return 0;
}



/*************************************
 *
 *  6840 timer handlers
 *
 *************************************/

READ8_MEMBER( exidy_sound_device::sh6840_r )
{
	/* force an update of the stream */
	m_stream->update();

	switch (offset)
	{
		/* offset 0: Motorola datasheet says it isn't used, Hitachi datasheet says it reads as 0s always*/
		case 0:
		return 0;
		/* offset 1 reads the status register: bits 2 1 0 correspond to ints on channels 2,1,0, and bit 7 is an 'OR' of bits 2,1,0 */
		case 1:
		logerror("%04X:exidy_sh6840_r - unexpected read, status register is TODO!\n", m_maincpu->pc());
		return 0;
		/* offsets 2,4,6 read channel 0,1,2 MSBs and latch the LSB*/
		case 2: case 4: case 6:
		m_sh6840_LSB_latch = m_sh6840_timer[((offset>>1)-1)].counter.b.l;
		return m_sh6840_timer[((offset>>1)-1)].counter.b.h;
		/* offsets 3,5,7 read the LSB latch*/
		default: /* case 3,5,7 */
		return m_sh6840_LSB_latch;
	}
}


WRITE8_MEMBER( exidy_sound_device::sh6840_w )
{
	struct sh6840_timer_channel *sh6840_timer = m_sh6840_timer;

	/* force an update of the stream */
	m_stream->update();

	switch (offset)
	{
		/* offset 0 writes to either channel 0 control or channel 2 control */
		case 0:
			if (sh6840_timer[1].cr & 0x01)
				sh6840_timer[0].cr = data;
			else
				sh6840_timer[2].cr = data;

			/* only support mode 0 and 2 */
			if (((data >> 3) & 5) != 0)
				fatalerror("exidy_sh6840_w - channel %d configured for mode %d\n", (sh6840_timer[1].cr & 0x01) ? 0 : 2, (data >> 3) & 7);
			break;

		/* offset 1 writes to channel 1 control */
		case 1:
			sh6840_timer[1].cr = data;

			/* only support mode 0 and 2 */
			if (((data >> 3) & 5) != 0)
				fatalerror("exidy_sh6840_w - channel 1 configured for mode %d\n", (data >> 3) & 7);
			break;

		/* offsets 2/4/6 write to the common MSB latch */
		case 2:
		case 4:
		case 6:
			m_sh6840_MSB_latch = data;
			break;

		/* offsets 3/5/7 write to the LSB controls */
		case 3:
		case 5:
		case 7:
		{
			/* latch the timer value */
			int ch = (offset - 3) / 2;
			sh6840_timer[ch].timer = (m_sh6840_MSB_latch << 8) | (data & 0xff);

			/* if CR4 is clear, the value is loaded immediately */
			if (!(sh6840_timer[ch].cr & 0x10))
				sh6840_timer[ch].counter.w = sh6840_timer[ch].timer;
			break;
		}
	}
}



/*************************************
 *
 *  External sound effect controls
 *
 *************************************/

WRITE8_MEMBER( exidy_sound_device::sfxctrl_w )
{
	m_stream->update();

	switch (offset)
	{
		case 0:
			m_sfxctrl = data;
			break;

		case 1:
		case 2:
		case 3:
			m_sh6840_volume[offset - 1] = ((data & 7) * BASE_VOLUME) / 7;
			break;
	}
}



/*************************************
 *
 *  Sound filter control
 *
 *************************************/

WRITE8_MEMBER( venture_sound_device::filter_w )
{
	logerror("exidy_sound_filter_w = %02X\n", data);
}



/*************************************
 *
 *  Venture, etc.
 *
 *************************************/


const device_type EXIDY_VENTURE = &device_creator<venture_sound_device>;

venture_sound_device::venture_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: exidy_sound_device(mconfig, EXIDY_VENTURE, "Exidy SFX+PSG", tag, owner, clock, "venture_sound", __FILE__)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void venture_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void venture_sound_device::device_start()
{
	common_sh_start();

	m_riot = machine().device<riot6532_device>("riot");

	m_has_sh8253  = TRUE;
	m_tms = nullptr;
	m_pia0 = machine().device<pia6821_device>("pia0");
	m_pia1 = machine().device<pia6821_device>("pia1");

	/* determine which sound hardware is installed */
	m_cvsd = machine().device<hc55516_device>("cvsd");

	/* 8253 */
	m_freq_to_step = (double)(1 << 24) / (double)SH8253_CLOCK;

	save_item(NAME(m_riot_irq_state));
	sh8253_register_state_globals();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void venture_sound_device::device_reset()
{
	common_sh_reset();

	/* PIA */
	//machine().device("pia0")->reset();
	m_pia0->reset();
	//machine().device("pia1")->reset();
	m_pia1->reset();

	/* 6532 */
	m_riot->reset();

	/* 8253 */
	memset(m_sh8253_timer, 0, sizeof(m_sh8253_timer));
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void venture_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	exidy_sound_device::sound_stream_update(stream, inputs, outputs, samples);
}




static ADDRESS_MAP_START( venture_audio_map, AS_PROGRAM, 8, driver_device )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x0780) AM_RAM
	AM_RANGE(0x0800, 0x087f) AM_MIRROR(0x0780) AM_DEVREADWRITE("riot", riot6532_device, read, write)
	AM_RANGE(0x1000, 0x1003) AM_MIRROR(0x07fc) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
	AM_RANGE(0x1800, 0x1803) AM_MIRROR(0x07fc) AM_DEVREADWRITE("custom", venture_sound_device, sh8253_r, sh8253_w)
	AM_RANGE(0x2000, 0x27ff) AM_DEVWRITE("custom", venture_sound_device, filter_w)
	AM_RANGE(0x2800, 0x2807) AM_MIRROR(0x07f8) AM_DEVREADWRITE("custom", venture_sound_device, sh6840_r, sh6840_w)
	AM_RANGE(0x3000, 0x3003) AM_MIRROR(0x07fc) AM_DEVWRITE("custom", venture_sound_device, sfxctrl_w)
	AM_RANGE(0x5800, 0x7fff) AM_ROM
ADDRESS_MAP_END


MACHINE_CONFIG_FRAGMENT( venture_audio )

	MCFG_CPU_ADD("audiocpu", M6502, 3579545/4)
	MCFG_CPU_PROGRAM_MAP(venture_audio_map)

	MCFG_DEVICE_ADD("riot", RIOT6532, SH6532_CLOCK)
	MCFG_RIOT6532_IN_PA_CB(DEVREAD8("custom", exidy_sound_device, r6532_porta_r))
	MCFG_RIOT6532_OUT_PA_CB(DEVWRITE8("custom", exidy_sound_device, r6532_porta_w))
	MCFG_RIOT6532_IN_PB_CB(DEVREAD8("custom", exidy_sound_device, r6532_portb_r))
	MCFG_RIOT6532_OUT_PB_CB(DEVWRITE8("custom", exidy_sound_device, r6532_portb_w))
	MCFG_RIOT6532_IRQ_CB(DEVWRITELINE("custom", exidy_sound_device, r6532_irq))

	MCFG_DEVICE_ADD("pia0", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(DEVWRITE8("pia1", pia6821_device, portb_w))
	MCFG_PIA_WRITEPB_HANDLER(DEVWRITE8("pia1", pia6821_device, porta_w))
	MCFG_PIA_CA2_HANDLER(DEVWRITELINE("pia1", pia6821_device, cb1_w))
	MCFG_PIA_CB2_HANDLER(DEVWRITELINE("pia1", pia6821_device, ca1_w))

	MCFG_DEVICE_ADD("pia1", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(DEVWRITE8("pia0", pia6821_device, portb_w))
	MCFG_PIA_WRITEPB_HANDLER(DEVWRITE8("pia0", pia6821_device, porta_w))
	MCFG_PIA_CA2_HANDLER(DEVWRITELINE("pia0", pia6821_device, cb1_w))
	MCFG_PIA_CB2_HANDLER(DEVWRITELINE("pia0", pia6821_device, ca1_w))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("custom", exidy_sound_device, update_irq_state))

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("custom", EXIDY_VENTURE, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  CVSD sound for Mouse Trap
 *
 *************************************/

WRITE8_MEMBER( venture_sound_device::mtrap_voiceio_w )
{
	if (!(offset & 0x10))
		m_cvsd->digit_w(data & 1);

	if (!(offset & 0x20))
		m_riot->portb_in_set(data & 1, 0xff);
}


READ8_MEMBER( venture_sound_device::mtrap_voiceio_r )
{
	if (!(offset & 0x80))
	{
		UINT8 porta = m_riot->porta_out_get();
		UINT8 data = (porta & 0x06) >> 1;
		data |= (porta & 0x01) << 2;
		data |= (porta & 0x08);
		return data;
	}

	if (!(offset & 0x40))
		return m_cvsd->clock_state_r() << 7;

	return 0;
}


static ADDRESS_MAP_START( cvsd_map, AS_PROGRAM, 8, driver_device )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x3fff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( cvsd_iomap, AS_IO, 8, driver_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0xff) AM_DEVREADWRITE("custom", venture_sound_device, mtrap_voiceio_r, mtrap_voiceio_w)
ADDRESS_MAP_END


MACHINE_CONFIG_FRAGMENT( mtrap_cvsd_audio )

	MCFG_CPU_ADD("cvsdcpu", Z80, CVSD_Z80_CLOCK)
	MCFG_CPU_PROGRAM_MAP(cvsd_map)
	MCFG_CPU_IO_MAP(cvsd_iomap)

	/* audio hardware */
	MCFG_SOUND_ADD("cvsd", MC3417, CVSD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END



/*************************************
 *
 *  Victory
 *
 *************************************/

#define VICTORY_AUDIO_CPU_CLOCK     (XTAL_3_579545MHz / 4)
#define VICTORY_LOG_SOUND           0



READ8_MEMBER( victory_sound_device::response_r )
{
	UINT8 ret = m_pia1->b_output();

	if (VICTORY_LOG_SOUND) logerror("%04X:!!!! Sound response read = %02X\n", m_maincpu->pcbase(), ret);

	m_pia1_cb1 = 0;
	m_pia1->cb1_w(m_pia1_cb1);

	return ret;
}


READ8_MEMBER( victory_sound_device::status_r )
{
	UINT8 ret = (m_pia1_ca1 << 7) | (m_pia1_cb1 << 6);

	if (VICTORY_LOG_SOUND) logerror("%04X:!!!! Sound status read = %02X\n", m_maincpu->pcbase(), ret);

	return ret;
}


TIMER_CALLBACK_MEMBER( victory_sound_device::delayed_command_w )
{
	m_pia1->porta_w(param);
	m_pia1_ca1 = 0;
	m_pia1->ca1_w(m_pia1_ca1);
}

WRITE8_MEMBER( victory_sound_device::command_w )
{
	if (VICTORY_LOG_SOUND) logerror("%04X:!!!! Sound command = %02X\n", m_maincpu->pcbase(), data);

	space.machine().scheduler().synchronize(timer_expired_delegate(FUNC(victory_sound_device::delayed_command_w), this), data);
}


WRITE_LINE_MEMBER( victory_sound_device::irq_clear_w )
{
	if (VICTORY_LOG_SOUND) logerror("%s:!!!! Sound IRQ clear = %02X\n", machine().describe_context(), state);

	if (!state)
	{
		m_pia1_ca1 = 1;
		m_pia1->ca1_w(m_pia1_ca1);
	}
}


WRITE_LINE_MEMBER( victory_sound_device::main_ack_w )
{
	if (VICTORY_LOG_SOUND) logerror("%s:!!!! Sound Main ACK W = %02X\n", machine().describe_context(), state);

	if (m_victory_sound_response_ack_clk && !state)
	{
		m_pia1_cb1 = 1;
		m_pia1->cb1_w(m_pia1_cb1);
	}

	m_victory_sound_response_ack_clk = state;
}


const device_type EXIDY_VICTORY = &device_creator<victory_sound_device>;

victory_sound_device::victory_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: exidy_sound_device(mconfig, EXIDY_VICTORY, "Exidy SFX+PSG+Speech", tag, owner, clock, "victory_sound", __FILE__),
	m_victory_sound_response_ack_clk(0)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void victory_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void victory_sound_device::device_start()
{
	save_item(NAME(m_victory_sound_response_ack_clk));
	save_item(NAME(m_pia1_ca1));
	save_item(NAME(m_pia1_cb1));

	common_sh_start();

	m_riot = machine().device<riot6532_device>("riot");

	m_has_sh8253  = TRUE;
	m_tms = nullptr;
	m_pia0 = machine().device<pia6821_device>("pia0");
	m_pia1 = machine().device<pia6821_device>("pia1");

	/* determine which sound hardware is installed */
	m_cvsd = machine().device<hc55516_device>("cvsd");

	/* 8253 */
	m_freq_to_step = (double)(1 << 24) / (double)SH8253_CLOCK;

	save_item(NAME(m_riot_irq_state));
	sh8253_register_state_globals();

	m_tms = machine().device<tms5220_device>("tms");
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void victory_sound_device::device_reset()
{
	common_sh_reset();
	m_pia1->reset();
	m_riot->reset();
	memset(m_sh8253_timer, 0, sizeof(m_sh8253_timer));

	/* the flip-flop @ F4 is reset */
	m_victory_sound_response_ack_clk = 0;
	m_pia1_cb1 = 1;
	m_pia1->cb1_w(m_pia1_cb1);

	/* these two lines shouldn't be needed, but it avoids the log entry
	   as the sound CPU checks port A before the main CPU ever writes to it */
	m_pia1->porta_w(0);
	m_pia1_ca1 = 1;
	m_pia1->ca1_w(m_pia1_ca1);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void victory_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	exidy_sound_device::sound_stream_update(stream, inputs, outputs, samples);
}




static ADDRESS_MAP_START( victory_audio_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x0f00) AM_RAM
	AM_RANGE(0x1000, 0x107f) AM_MIRROR(0x0f80) AM_DEVREADWRITE("riot", riot6532_device, read, write)
	AM_RANGE(0x2000, 0x2003) AM_MIRROR(0x0ffc) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
	AM_RANGE(0x3000, 0x3003) AM_MIRROR(0x0ffc) AM_DEVREADWRITE("custom", victory_sound_device, sh8253_r, sh8253_w)
	AM_RANGE(0x4000, 0x4fff) AM_NOP
	AM_RANGE(0x5000, 0x5007) AM_MIRROR(0x0ff8) AM_DEVREADWRITE("custom", victory_sound_device, sh6840_r, sh6840_w)
	AM_RANGE(0x6000, 0x6003) AM_MIRROR(0x0ffc) AM_DEVWRITE("custom", victory_sound_device, sfxctrl_w)
	AM_RANGE(0x7000, 0xafff) AM_NOP
	AM_RANGE(0xb000, 0xffff) AM_ROM
ADDRESS_MAP_END


MACHINE_CONFIG_FRAGMENT( victory_audio )

	MCFG_CPU_ADD("audiocpu", M6502, VICTORY_AUDIO_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(victory_audio_map)

	MCFG_DEVICE_ADD("riot", RIOT6532, SH6532_CLOCK)
	MCFG_RIOT6532_IN_PA_CB(DEVREAD8("custom", exidy_sound_device, r6532_porta_r))
	MCFG_RIOT6532_OUT_PA_CB(DEVWRITE8("custom", exidy_sound_device, r6532_porta_w))
	MCFG_RIOT6532_IN_PB_CB(DEVREAD8("custom", exidy_sound_device, r6532_portb_r))
	MCFG_RIOT6532_OUT_PB_CB(DEVWRITE8("custom", exidy_sound_device, r6532_portb_w))
	MCFG_RIOT6532_IRQ_CB(DEVWRITELINE("custom", exidy_sound_device, r6532_irq))

	MCFG_DEVICE_ADD("pia1", PIA6821, 0)
	MCFG_PIA_CA2_HANDLER(DEVWRITELINE("custom", victory_sound_device, irq_clear_w))
	MCFG_PIA_CB2_HANDLER(DEVWRITELINE("custom", victory_sound_device, main_ack_w))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("custom", exidy_sound_device, update_irq_state))

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("custom", EXIDY_VICTORY, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("tms", TMS5220, 640000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END
