// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Exidy 6502 hardware

    TODO:
    - use ptm6840_device

*************************************************************************/

#include "emu.h"
#include "exidysound.h"

#include "cpu/m6502/m6502.h"
#include "cpu/z80/z80.h"
#include "machine/input_merger.h"
#include "machine/rescap.h"

#include "speaker.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define LOG_VICTORY_SOUND       (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

#define CRYSTAL_OSC             (3.579545_MHz_XTAL)
#define SH8253_CLOCK            (CRYSTAL_OSC / 2)
#define SH6840_CLOCK            (CRYSTAL_OSC / 4)
#define SH6532_CLOCK            (CRYSTAL_OSC / 4)
#define CVSD_CLOCK              (1.0 / (0.693 * (RES_K(2.4) + 2.0 * RES_K(20)) * CAP_P(2200)))
#define CVSD_Z80_CLOCK          (CRYSTAL_OSC / 2)
#define BASE_VOLUME             (32767 / 6)



/*************************************
 *
 *  6840 clock counting helper
 *
 *************************************/

inline void exidy_sound_device::sh6840_apply_clock(exidy_sound_device::sh6840_timer_channel *t, int clocks)
{
	// dual 8-bit case
	if (t->cr & 0x04)
	{
		// handle full decrements
		while (clocks > t->counter.b.l)
		{
			clocks -= t->counter.b.l + 1;
			t->counter.b.l = t->timer;

			// decrement MSB
			if (!t->counter.b.h--)
			{
				t->state = 0;
				t->counter.w = t->timer;
			}

			// state goes high when MSB is 0
			else if (!t->counter.b.h)
			{
				t->state = 1;
				t->clocks++;
			}
		}

		// subtract off the remainder
		t->counter.b.l -= clocks;
	}

	// 16-bit case
	else
	{
		// handle full decrements
		while (clocks > t->counter.w)
		{
			clocks -= t->counter.w + 1;
			t->state ^= 1;
			t->clocks += t->state;
			t->counter.w = t->timer;
		}

		// subtract off the remainder
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
	uint32_t newxor;
	int noise_clocks = 0;

	// loop over clocks
	for (int i = 0; i < clocks; i++)
	{
		// shift the LFSR. its a LOOOONG LFSR, so we need four longs to hold it all!
		// first we grab new sample, then shift the high bits, then the low ones;
		// finally or in the result and see if we've had a 0->1 transition
		newxor = (m_sh6840_LFSR_3 ^ m_sh6840_LFSR_2) >> 31; // high bits of 3 and 2 xored is new xor
		m_sh6840_LFSR_3 <<= 1;
		m_sh6840_LFSR_3 |= m_sh6840_LFSR_2 >> 31;
		m_sh6840_LFSR_2 <<= 1;
		m_sh6840_LFSR_2 |= m_sh6840_LFSR_1 >> 31;
		m_sh6840_LFSR_1 <<= 1;
		m_sh6840_LFSR_1 |= m_sh6840_LFSR_0 >> 31;
		m_sh6840_LFSR_0 <<= 1;
		m_sh6840_LFSR_0 |= newxor ^ m_sh6840_LFSR_oldxor;
		m_sh6840_LFSR_oldxor = newxor;
		//printf("LFSR: %4x, %4x, %4x, %4x\n", m_sh6840_LFSR_3, m_sh6840_LFSR_2, m_sh6840_LFSR_1, m_sh6840_LFSR_0);
		// if we clocked 0->1, that will serve as an external clock
		if ((m_sh6840_LFSR_2 & 0x03) == 0x01) // tap is at 96th bit
		{
			noise_clocks++;
		}
	}
	return noise_clocks;
}



/*************************************
 *
 *  Audio startup routines
 *
 *************************************/

DEFINE_DEVICE_TYPE(EXIDY, exidy_sound_device, "exidy_sfx", "Exidy SFX")

exidy_sound_device::exidy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: exidy_sound_device(mconfig, EXIDY, tag, owner, clock)
{
}

exidy_sound_device::exidy_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_sh6840_MSB_latch(0)
	, m_sh6840_LSB_latch(0)
	, m_sh6840_LFSR_oldxor(0)
	, m_sh6840_LFSR_0(0xffffffff)
	, m_sh6840_LFSR_1(0xffffffff)
	, m_sh6840_LFSR_2(0xffffffff)
	, m_sh6840_LFSR_3(0xffffffff)
	, m_sh6840_clocks_per_sample(0)
	, m_sh6840_clock_count(0)
	, m_sfxctrl(0)
{
}

exidy_sound_device::~exidy_sound_device()
{
}

exidy_sh8253_sound_device::exidy_sh8253_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: exidy_sound_device(mconfig, type, tag, owner, clock)
	, m_riot(*this, "riot")
	, m_pia(*this, "pia")
	, m_pit(*this, "pit")
{
}


void exidy_sh8253_sound_device::device_add_mconfig(machine_config &config)
{
	exidy_sound_device::device_add_mconfig(config);

	MOS6532(config, m_riot, SH6532_CLOCK);
	m_riot->irq_wr_callback().set("audioirq", FUNC(input_merger_device::in_w<0>));

	PIA6821(config, m_pia);
	m_pia->irqa_handler().set("audioirq", FUNC(input_merger_device::in_w<1>));

	PIT8253(config, m_pit, SH6532_CLOCK);
	m_pit->set_clk<0>(SH8253_CLOCK);
	m_pit->set_clk<1>(SH8253_CLOCK);
	m_pit->set_clk<2>(SH8253_CLOCK);
	m_pit->out_handler<0>().set(FUNC(exidy_sh8253_sound_device::pit_out<0>));
	m_pit->out_handler<1>().set(FUNC(exidy_sh8253_sound_device::pit_out<1>));
	m_pit->out_handler<2>().set(FUNC(exidy_sh8253_sound_device::pit_out<2>));

	INPUT_MERGER_ANY_HIGH(config, "audioirq").output_handler().set_inputline("audiocpu", m6502_device::IRQ_LINE); // open collector

	SPEAKER(config, "mono").front_center();

	this->add_route(ALL_OUTPUTS, "mono", 0.50);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void exidy_sound_device::device_start()
{
	int sample_rate = SH8253_CLOCK.value();

	m_sh6840_clocks_per_sample = int(SH6840_CLOCK.dvalue() / double(sample_rate) * double(1 << 24));

	// allocate the stream
	m_stream = stream_alloc(0, 1, sample_rate);

	save_item(STRUCT_MEMBER(m_sh6840_timer, cr));
	save_item(STRUCT_MEMBER(m_sh6840_timer, state));
	save_item(STRUCT_MEMBER(m_sh6840_timer, leftovers));
	save_item(STRUCT_MEMBER(m_sh6840_timer, timer));
	save_item(STRUCT_MEMBER(m_sh6840_timer, clocks));
	save_item(NAME(m_sh6840_timer[0].counter.w));
	save_item(NAME(m_sh6840_timer[1].counter.w));
	save_item(NAME(m_sh6840_timer[2].counter.w));

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
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void exidy_sound_device::device_reset()
{
	// 6840
	for (auto &channel : m_sh6840_timer)
	{
		channel.cr = 0;
		channel.state = 0;
		channel.leftovers = 0;
		channel.timer = 0;
		channel.clocks = 0;
		channel.counter.w = 0;
	}
	m_sh6840_MSB_latch = 0;
	m_sh6840_LSB_latch = 0;
	m_sh6840_volume[0] = 0;
	m_sh6840_volume[1] = 0;
	m_sh6840_volume[2] = 0;
	m_sh6840_clock_count = 0;
	m_sfxctrl = 0;

	// LFSR
	m_sh6840_LFSR_oldxor = 0;
	m_sh6840_LFSR_0 = 0xffffffff;
	m_sh6840_LFSR_1 = 0xffffffff;
	m_sh6840_LFSR_2 = 0xffffffff;
	m_sh6840_LFSR_3 = 0xffffffff;
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void exidy_sound_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	sh6840_timer_channel *sh6840_timer = m_sh6840_timer;

	// hack to skip the expensive lfsr noise generation unless at least one of the 3 channels actually depends on it being generated
	bool noisy = ((sh6840_timer[0].cr & sh6840_timer[1].cr & sh6840_timer[2].cr & 0x02) == 0);
	auto &buffer = outputs[0];

	// loop over samples
	for (int sampindex = 0; sampindex < buffer.samples(); sampindex++)
	{
		sh6840_timer_channel *t;
		int clocks;
		s32 sample = 0;

		// determine how many 6840 clocks this sample
		m_sh6840_clock_count += m_sh6840_clocks_per_sample;
		int clocks_this_sample = m_sh6840_clock_count >> 24;
		m_sh6840_clock_count &= (1 << 24) - 1;

		// skip if nothing enabled
		if ((sh6840_timer[0].cr & 0x01) == 0)
		{
			int noise_clocks_this_sample = 0;
			uint32_t chan0_clocks;

			// generate E-clocked noise if configured to do so
			if (noisy && !(m_sfxctrl & 0x01))
				noise_clocks_this_sample = sh6840_update_noise(clocks_this_sample);

			// handle timer 0 if enabled
			t = &sh6840_timer[0];
			chan0_clocks = t->clocks;
			clocks = (t->cr & 0x02) ? clocks_this_sample : noise_clocks_this_sample;
			sh6840_apply_clock(t, clocks);
			if (t->state && !(m_sfxctrl & 0x02) && (t->cr & 0x80))
				sample += m_sh6840_volume[0];

			// generate channel 0-clocked noise if configured to do so
			if (noisy && (m_sfxctrl & 0x01))
				noise_clocks_this_sample = sh6840_update_noise(t->clocks - chan0_clocks);

			// handle timer 1 if enabled
			t = &sh6840_timer[1];
			clocks = (t->cr & 0x02) ? clocks_this_sample : noise_clocks_this_sample;
			sh6840_apply_clock(t, clocks);
			if (t->state && (t->cr & 0x80))
				sample += m_sh6840_volume[1];

			// handle timer 2 if enabled
			t = &sh6840_timer[2];
			clocks = (t->cr & 0x02) ? clocks_this_sample : noise_clocks_this_sample;

			// prescale
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

		// music (if present)
		sample += generate_music_sample();

		// stash
		buffer.put_int(sampindex, sample, 32768);
	}
}

s32 exidy_sh8253_sound_device::generate_music_sample()
{
	s32 sample = 0;
	if (BIT(m_pit_out, 0))
		sample += BASE_VOLUME;
	if (BIT(m_pit_out, 1))
		sample += BASE_VOLUME;
	if (BIT(m_pit_out, 2))
		sample += BASE_VOLUME;
	return sample;
}



/*************************************
 *
 *  8253 timer handlers
 *
 *************************************/

void exidy_sh8253_sound_device::sh8253_w(offs_t offset, uint8_t data)
{
	m_stream->update();
	m_pit->write(offset, data);
}

template <unsigned N>
void exidy_sh8253_sound_device::pit_out(int state)
{
	m_stream->update();
	if (state)
		m_pit_out |= u8(1) << N;
	else
		m_pit_out &= ~(u8(1) << N);
}



/*************************************
 *
 *  6840 timer handlers
 *
 *************************************/

uint8_t exidy_sound_device::sh6840_r(offs_t offset)
{
	// force an update of the stream
	m_stream->update();

	switch (offset)
	{
		// offset 0: Motorola datasheet says it isn't used, Hitachi datasheet says it reads as 0s always
		case 0:
			return 0;

		// offset 1 reads the status register: bits 2 1 0 correspond to ints on channels 2,1,0, and bit 7 is an 'OR' of bits 2,1,0
		case 1:
			logerror("%s:exidy_sh6840_r - unexpected read, status register is TODO!\n", machine().describe_context());
			return 0;

		// offsets 2,4,6 read channel 0,1,2 MSBs and latch the LSB
		case 2: case 4: case 6:
			if (!machine().side_effects_disabled())
				m_sh6840_LSB_latch = m_sh6840_timer[((offset>>1)-1)].counter.b.l;
			return m_sh6840_timer[((offset>>1)-1)].counter.b.h;

		// offsets 3,5,7 read the LSB latch
		default: // case 3,5,7
			return m_sh6840_LSB_latch;
	}
}


void exidy_sound_device::sh6840_w(offs_t offset, uint8_t data)
{
	sh6840_timer_channel *sh6840_timer = m_sh6840_timer;

	// force an update of the stream
	m_stream->update();

	switch (offset)
	{
		// offset 0 writes to either channel 0 control or channel 2 control
		case 0:
			if (sh6840_timer[1].cr & 0x01)
				sh6840_timer[0].cr = data;
			else
				sh6840_timer[2].cr = data;

			// only support mode 0 and 2
			if (((data >> 3) & 5) != 0)
				fatalerror("exidy_sh6840_w - channel %d configured for mode %d\n", (sh6840_timer[1].cr & 0x01) ? 0 : 2, (data >> 3) & 7);
			break;

		// offset 1 writes to channel 1 control
		case 1:
			sh6840_timer[1].cr = data;

			// only support mode 0 and 2
			if (((data >> 3) & 5) != 0)
				fatalerror("exidy_sh6840_w - channel 1 configured for mode %d\n", (data >> 3) & 7);
			break;

		// offsets 2/4/6 write to the common MSB latch
		case 2:
		case 4:
		case 6:
			m_sh6840_MSB_latch = data;
			break;

		// offsets 3/5/7 write to the LSB controls
		case 3:
		case 5:
		case 7:
		{
			// latch the timer value
			int ch = (offset - 3) / 2;
			sh6840_timer[ch].timer = (m_sh6840_MSB_latch << 8) | (data & 0xff);

			// if CR4 is clear, the value is loaded immediately
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

void exidy_sound_device::sfxctrl_w(offs_t offset, uint8_t data)
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
 *  Venture, etc.
 *
 *************************************/

DEFINE_DEVICE_TYPE(EXIDY_VENTURE, venture_sound_device, "venture_sound", "Exidy SFX+PSG")

venture_sound_device::venture_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: venture_sound_device(mconfig, EXIDY_VENTURE, tag, owner, clock)
{
}

venture_sound_device::venture_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: exidy_sh8253_sound_device(mconfig, type, tag, owner, clock)
	, m_pa_callback(*this)
	, m_pb_callback(*this)
	, m_ca2_callback(*this)
	, m_cb2_callback(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void exidy_sh8253_sound_device::device_start()
{
	exidy_sound_device::device_start();

	save_item(NAME(m_pit_out));

	m_pit_out = 0;

	m_pit->write_gate0(1);
	m_pit->write_gate1(1);
	m_pit->write_gate2(1);
}


//-------------------------------------------------
//  I/O handlers
//-------------------------------------------------

void venture_sound_device::filter_w(uint8_t data)
{
	logerror("exidy_sound_filter_w = %02X\n", data);
}


//-------------------------------------------------
//  Address maps
//-------------------------------------------------

void venture_sound_device::venture_audio_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x007f).mirror(0x0780).m(m_riot, FUNC(mos6532_device::ram_map));
	map(0x0800, 0x081f).mirror(0x07e0).m(m_riot, FUNC(mos6532_device::io_map));
	map(0x1000, 0x1003).mirror(0x07fc).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1800, 0x1803).mirror(0x07fc).w(FUNC(venture_sound_device::sh8253_w)).nopr(); // no /RD
	map(0x2000, 0x2000).mirror(0x07ff).w(FUNC(venture_sound_device::filter_w));
	map(0x2800, 0x2807).mirror(0x07f8).rw(FUNC(venture_sound_device::sh6840_r), FUNC(venture_sound_device::sh6840_w));
	map(0x3000, 0x3003).mirror(0x07fc).w(FUNC(venture_sound_device::sfxctrl_w));
	map(0x5800, 0x7fff).rom();
}


//-------------------------------------------------
//  Machine config
//-------------------------------------------------

void venture_sound_device::device_add_mconfig(machine_config &config)
{
	exidy_sh8253_sound_device::device_add_mconfig(config);

	m6502_device &audiocpu(M6502(config, "audiocpu", 3.579545_MHz_XTAL / 4));
	audiocpu.set_addrmap(AS_PROGRAM, &venture_sound_device::venture_audio_map);

	m_pia->writepa_handler().set(FUNC(venture_sound_device::pia_pa_w));
	m_pia->writepb_handler().set(FUNC(venture_sound_device::pia_pb_w));
	m_pia->ca2_handler().set(FUNC(venture_sound_device::pia_ca2_w));
	m_pia->cb2_handler().set(FUNC(venture_sound_device::pia_cb2_w));
}



/*************************************
 *
 *  CVSD sound for Mouse Trap
 *
 *************************************/

DEFINE_DEVICE_TYPE(EXIDY_MTRAP, mtrap_sound_device, "mtrap_sound", "Exidy SFX+PSG+CVSD")

mtrap_sound_device::mtrap_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: venture_sound_device(mconfig, EXIDY_MTRAP, tag, owner, clock)
	, m_cvsd_timer(*this, "cvsd_timer")
	, m_cvsd(*this, "cvsd")
	, m_cvsd_filter(*this, "cvsd_filter")
	, m_cvsd_filter2(*this, "cvsd_filter2")
	, m_cvsdcpu(*this, "cvsdcpu")
	, m_cvsd_data(0)
	, m_cvsd_clk(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mtrap_sound_device::device_start()
{
	venture_sound_device::device_start();

	save_item(NAME(m_cvsd_data));
	save_item(NAME(m_cvsd_clk));
}


//-------------------------------------------------
//  I/O handlers
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER(mtrap_sound_device::cvsd_timer)
{
	m_cvsd_clk ^= 1;
	m_cvsd->clock_w(m_cvsd_clk);
}

void mtrap_sound_device::voiceio_w(offs_t offset, uint8_t data)
{
	if (!(offset & 0x10))
		m_cvsd->digit_w(data & 1);

	if (!(offset & 0x20))
		m_riot->pb_bit_w<0>(data & 1);
}


uint8_t mtrap_sound_device::voiceio_r(offs_t offset)
{
	uint8_t retval = 0xff; // this should probably be open bus
	if (!(offset & 0x80))
	{
		retval &= 0xf0;
		retval |= bitswap(m_cvsd_data,3,0,2,1);
	}

	if (!(offset & 0x40))
	{
		retval &= 0x7f;
		retval |= (m_cvsd_clk << 7);
	}

	return retval;
}

void mtrap_sound_device::cvsd_data_w(uint8_t data)
{
	m_cvsd_data = data;
	m_cvsdcpu->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  Address maps
//-------------------------------------------------

void mtrap_sound_device::cvsd_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x3fff).rom().region("cvsdcpu", 0);
}


void mtrap_sound_device::cvsd_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).rw(FUNC(mtrap_sound_device::voiceio_r), FUNC(mtrap_sound_device::voiceio_w));
}


//-------------------------------------------------
//  Machine config
//-------------------------------------------------

void mtrap_sound_device::device_add_mconfig(machine_config &config)
{
	venture_sound_device::device_add_mconfig(config);

	m_riot->pa_wr_callback().set(FUNC(mtrap_sound_device::cvsd_data_w));

	Z80(config, m_cvsdcpu, CVSD_Z80_CLOCK);
	m_cvsdcpu->set_addrmap(AS_PROGRAM, &mtrap_sound_device::cvsd_map);
	m_cvsdcpu->set_addrmap(AS_IO, &mtrap_sound_device::cvsd_iomap);

	// this is a 555 timer with 53% duty cycle, within margin of error of 50% duty cycle; the handler clocks on both clock edges, hence * 2.0
	TIMER(config, m_cvsd_timer).configure_periodic(FUNC(mtrap_sound_device::cvsd_timer), attotime::from_hz(CVSD_CLOCK * 2.0));

	// audio hardware
	FILTER_BIQUAD(config, m_cvsd_filter2).opamp_mfb_lowpass_setup(RES_K(10), RES_K(3.9), RES_K(18), CAP_N(20), CAP_N(2.2));
	m_cvsd_filter2->add_route(ALL_OUTPUTS, "mono", 1.0);
	FILTER_BIQUAD(config, m_cvsd_filter).opamp_mfb_lowpass_setup(RES_K(10), RES_K(3.9), RES_K(18), CAP_N(20), CAP_N(2.2));
	m_cvsd_filter->add_route(ALL_OUTPUTS, m_cvsd_filter2, 1.0);
	MC3417(config, m_cvsd, 0).add_route(ALL_OUTPUTS, m_cvsd_filter, 0.3086); // each filter has gain of 1.8 for total gain of 3.24, 0.3086 cancels this out.
}



/*************************************
 *
 *  Victory
 *
 *************************************/

DEFINE_DEVICE_TYPE(EXIDY_VICTORY, victory_sound_device, "victory_sound", "Exidy SFX+PSG+Speech")

victory_sound_device::victory_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: exidy_sh8253_sound_device(mconfig, EXIDY_VICTORY, tag, owner, clock)
	, m_tms(*this, "tms")
	, m_sound_response_ack_clk(0)
	, m_pia_ca1(0)
	, m_pia_cb1(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void victory_sound_device::device_start()
{
	exidy_sh8253_sound_device::device_start();

	save_item(NAME(m_sound_response_ack_clk));
	save_item(NAME(m_pia_ca1));
	save_item(NAME(m_pia_cb1));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void victory_sound_device::device_reset()
{
	exidy_sh8253_sound_device::device_reset();

	// the flip-flop @ F4 is reset
	m_sound_response_ack_clk = 0;
	m_pia_cb1 = 1;
	m_pia->cb1_w(m_pia_cb1);

	// these two lines shouldn't be needed, but it avoids the log entry
	// as the sound CPU checks port A before the main CPU ever writes to it
	m_pia->porta_w(0);
	m_pia_ca1 = 1;
	m_pia->ca1_w(m_pia_ca1);
}


//-------------------------------------------------
//  I/O handlers
//-------------------------------------------------

uint8_t victory_sound_device::response_r()
{
	uint8_t ret = m_pia->b_output();

	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_VICTORY_SOUND, "%s:!!!! Sound response read = %02X\n", machine().describe_context(), ret);

		m_pia_cb1 = 0;
		m_pia->cb1_w(m_pia_cb1);
	}

	return ret;
}

uint8_t victory_sound_device::status_r()
{
	uint8_t ret = (m_pia_ca1 << 7) | (m_pia_cb1 << 6);
	LOGMASKED(LOG_VICTORY_SOUND, "%s:!!!! Sound status read = %02X\n", machine().describe_context(), ret);

	return ret;
}

TIMER_CALLBACK_MEMBER(victory_sound_device::delayed_command_w)
{
	m_pia->porta_w(param);
	m_pia_ca1 = 0;
	m_pia->ca1_w(m_pia_ca1);
}

void victory_sound_device::command_w(uint8_t data)
{
	LOGMASKED(LOG_VICTORY_SOUND, "%s:!!!! Sound command = %02X\n", machine().describe_context(), data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(victory_sound_device::delayed_command_w), this), data);
}

void victory_sound_device::irq_clear_w(int state)
{
	LOGMASKED(LOG_VICTORY_SOUND, "%s:!!!! Sound IRQ clear = %02X\n", machine().describe_context(), state);

	if (!state)
	{
		m_pia_ca1 = 1;
		m_pia->ca1_w(m_pia_ca1);
	}
}

void victory_sound_device::main_ack_w(int state)
{
	LOGMASKED(LOG_VICTORY_SOUND, "%s:!!!! Sound Main ACK W = %02X\n", machine().describe_context(), state);

	if (m_sound_response_ack_clk && !state)
	{
		m_pia_cb1 = 1;
		m_pia->cb1_w(m_pia_cb1);
	}

	m_sound_response_ack_clk = state;
}


//-------------------------------------------------
//  Address maps
//-------------------------------------------------

void victory_sound_device::victory_audio_map(address_map &map)
{
	map(0x0000, 0x007f).mirror(0x0f00).m(m_riot, FUNC(mos6532_device::ram_map));
	map(0x0080, 0x00ff).mirror(0x0f00).ram();
	map(0x1000, 0x101f).mirror(0x0fe0).m(m_riot, FUNC(mos6532_device::io_map));
	map(0x2000, 0x2003).mirror(0x0ffc).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x3000, 0x3003).mirror(0x0ffc).w(FUNC(victory_sound_device::sh8253_w)).nopr(); // no /RD
	map(0x4000, 0x4fff).noprw();
	map(0x5000, 0x5007).mirror(0x0ff8).rw(FUNC(victory_sound_device::sh6840_r), FUNC(victory_sound_device::sh6840_w));
	map(0x6000, 0x6003).mirror(0x0ffc).w(FUNC(victory_sound_device::sfxctrl_w));
	map(0x7000, 0xafff).noprw();
	map(0xb000, 0xffff).rom();
}


//-------------------------------------------------
//  Machine config
//-------------------------------------------------

void victory_sound_device::device_add_mconfig(machine_config &config)
{
	exidy_sh8253_sound_device::device_add_mconfig(config);

	m6502_device &audiocpu(M6502(config, "audiocpu", 3.579545_MHz_XTAL / 4));
	audiocpu.set_addrmap(AS_PROGRAM, &victory_sound_device::victory_audio_map);

	m_riot->pa_wr_callback().set(m_tms, FUNC(tms5220_device::data_w));
	m_riot->pa_rd_callback().set(m_tms, FUNC(tms5220_device::status_r));
	m_riot->pb_wr_callback<0>().set(m_tms, FUNC(tms5220_device::rsq_w));
	m_riot->pb_wr_callback<1>().set(m_tms, FUNC(tms5220_device::wsq_w));
	m_riot->pb_rd_callback<2>().set(m_tms, FUNC(tms5220_device::readyq_r));
	m_riot->pb_rd_callback<3>().set(m_tms, FUNC(tms5220_device::intq_r));

	m_pia->ca2_handler().set(FUNC(victory_sound_device::irq_clear_w));
	m_pia->cb2_handler().set(FUNC(victory_sound_device::main_ack_w));

	TMS5220(config, m_tms, 640000).add_route(ALL_OUTPUTS, "mono", 1.0);
}
