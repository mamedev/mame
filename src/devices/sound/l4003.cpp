// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**************************************************************************************************

    Akai/Roger Linn L4003 DSP
    Emulation by R. Belmont

    This is the audio chip used in the Akai MPC60, with 16 voices of 12-bit sample playback.
    There are many similarities to the later L6028 chip and many of the registers are either
    binary identical or very similar.  The main difference is the sense of the volume envelope
    is reversed, where 0 here is max volume and 0xffff is silence.

    The chip has a total of 10 unique outputs, 8 individual outputs and a stereo pair.  Each voice
    can be sent to an individual output, the stereo pair, or both.

    The chip has no DMA and is interfaced to the CPU by 2 16-bit wide ports.  As with the later
    L6028 chip, there is no byte lane select, reads and writes are always 16 bits wide.

    The wave data bus is 12 bits wide.  Configurations of 512K or 1024K 12-bit words are supported.
    (768K or 1536K bytes).  For ease of emulation, we make those 16-bit words.

    Voice registers (low 4 bits are the voice number)
    000x - ssss ssss ssss ssss
    s - start offset in samples from the start of memory (low word)

    001x - ssss ssss ssss ssss
    s - start offset in samples from the start of memory (high word)

    002x - pitch in 4.12 fixed point where 0x1000 is 40.0 kHz

    003x - eeee eeee ???? oooo
    e - send level
    o - send destination where 0 = 7, 1 = 8, 2 = 9, ... f = 8

    004x - Envelope volume (0 = max volume, 0xffff = silent)

    005x - Volume envelope target, same units as register 004x

    006x - Volume envelope rate in 13.3 fixed point, and inverted.
           So this value XOR 0xffff gives the 13.3 fixed point rate
           in envelope steps per sample period

    007x - Main stereo left and right volumes

    011x - Readback of register 001x

    015x - Readback of register 005x

    016x - Readback of register 006x

    0200 - Wave RAM write (uses voice 1's start address as the pointer)

    0300 - Wave RAM read (uses voice 1's start address as the pointer)

**************************************************************************************************/

#include "emu.h"
#include "l4003.h"
#include "debugger.h"

#define LOG_REGISTERS           (1U << 1)
#define LOG_REGISTERS_HIFREQ    (1U << 2)
#define LOG_SAMPLE_READ         (1U << 3)
#define LOG_SAMPLE_WRITE        (1U << 4)
#define LOG_KEYON               (1U << 5)
#define LOG_ENVELOPE            (1U << 6)
#define LOG_VOLUME              (1U << 7)

#define VERBOSE (0)

//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(L4003, l4003_sound_device, "l4003", "Akai/Roger Linn L4003 wavetable sound")

static constexpr uint32_t OUTPUT_SCALE = 0x7fff;

l4003_sound_device::l4003_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, L4003, tag, owner, clock)
	  , device_sound_interface(mconfig, *this)
	  , device_memory_interface(mconfig, *this)
	  , m_stream(nullptr)
	  , m_mem_config("l4003", ENDIANNESS_LITTLE, 16, 21)
	  , m_data(0)
	  , m_control(0)
	  , m_sample_rate(0)
{
	for (auto &voice : m_voices)
	{
		voice.start = 0;
		voice.step = 0;
		voice.pos = 0;
		voice.frac = 0;
		voice.l_volume = 0;
		voice.r_volume = 0;
		voice.env_volume = 0xffff;
		voice.env_target = 0xffff;
		voice.env_step = 0;
		voice.env_pos = 0;
		voice.send_dest = 0;
		voice.send_level = 0;
		voice.filt_state = 0;
	}
}

void l4003_sound_device::map(address_map &map)
{
	map(0x0000, 0x0001).rw(FUNC(l4003_sound_device::data_r), FUNC(l4003_sound_device::data_w));
	map(0x0002, 0x0003).rw(FUNC(l4003_sound_device::control_r), FUNC(l4003_sound_device::control_w));
}

device_memory_interface::space_config_vector l4003_sound_device::memory_space_config() const
{
	return space_config_vector{std::make_pair(AS_DATA, &m_mem_config)};
}

void l4003_sound_device::device_start()
{
	space(AS_DATA).cache(m_cache);

	// Allocate the stream
	m_sample_rate = clock() / 896.0f;   // 35.84 MHz clock / 896 = 40.0 kHz sample rate
	m_stream = stream_alloc(0, 10, m_sample_rate, STREAM_SYNCHRONOUS);

	save_item(STRUCT_MEMBER(m_voices, start));
	save_item(STRUCT_MEMBER(m_voices, step));
	save_item(STRUCT_MEMBER(m_voices, pos));
	save_item(STRUCT_MEMBER(m_voices, frac));
	save_item(STRUCT_MEMBER(m_voices, l_volume));
	save_item(STRUCT_MEMBER(m_voices, r_volume));
	save_item(STRUCT_MEMBER(m_voices, env_volume));
	save_item(STRUCT_MEMBER(m_voices, env_target));
	save_item(STRUCT_MEMBER(m_voices, env_step));
	save_item(STRUCT_MEMBER(m_voices, env_pos));
	save_item(STRUCT_MEMBER(m_voices, send_dest));
	save_item(STRUCT_MEMBER(m_voices, send_level));
	save_item(STRUCT_MEMBER(m_voices, filt_state));
	save_item(NAME(m_data));
	save_item(NAME(m_control));
}

void l4003_sound_device::device_reset()
{
}

void l4003_sound_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < NUM_VOICES; i++)
	{
		l4003_voice &vptr = m_voices[i];

		const uint32_t step = vptr.step;
		uint32_t pos = vptr.pos;
		uint32_t frac = vptr.frac;

		for (int j = 0; j < stream.samples(); j++)
		{
			uint32_t address;
			int32_t sample;

			pos += (frac >> 12);
			frac &= 0xfff;

			address = (vptr.start << 1) + pos;
			sample = (int16_t)m_cache.read_word(address & 0x1fffff);

			frac += step;

			// The following is a pretty literal decompile of the assembly code in the MPC3000
			// which massages the MPC60 samples into 16 bits.  It's presumed to be simulating internal
			// processing of the L4003 since it sounds closer than just playing the 12-bit samples
			// as 12-bit linear.
			const int32_t temp =
				(int32_t)(((uint32_t)(uint16_t)sample << 16) | (uint16_t)vptr.filt_state) >> 8;
			const int32_t new_state = temp + (int32_t)(((int64_t)vptr.filt_state * 0x9e227) >> 20);

			int32_t temp2 =
				(int32_t)(((int64_t)new_state * 0x5b708) >> 20) +
				(int32_t)(((int64_t)vptr.filt_state * 0x645d) >> 20);

			vptr.filt_state = new_state;
			temp2 = (temp2 * 19) >> 4;

			if (temp2 >= 0)
			{
				if ((uint32_t)temp2 >= ((uint32_t)OUTPUT_SCALE << 8))
				{
					sample = (int16_t)(OUTPUT_SCALE - 1);
				}
				else
				{
					sample = (int16_t)((temp2 << 7) / (int32_t)OUTPUT_SCALE);
				}
			}
			else
			{
				if ((temp2 >> 8) <= -(int32_t)OUTPUT_SCALE)
				{
					sample = (int16_t)-(int16_t)(OUTPUT_SCALE - 1);
				}
				else
				{
					sample = (int16_t)((temp2 << 7) / (int32_t)OUTPUT_SCALE);
				}
			}

			// volume envelope processing
			const uint16_t env_step = (uint16_t)vptr.env_step ^ 0xffff;
			vptr.env_pos += env_step;

			const int steps = (uint32_t)vptr.env_pos >> 3;
			if (vptr.env_volume < vptr.env_target)
			{
				vptr.env_volume += std::min(steps, (vptr.env_target - vptr.env_volume));
			}
			else if (vptr.env_volume > vptr.env_target)
			{
				vptr.env_volume -= std::min(steps, (vptr.env_volume - vptr.env_target));
			}
			vptr.env_pos &= 0x7;

			const int64_t left = (sample * (uint64_t(vptr.l_volume) * uint64_t(0xffff - vptr.env_volume))) >> 24;
			const int64_t right = (sample * (uint64_t(vptr.r_volume) * uint64_t(0xffff - vptr.env_volume))) >> 24;
			stream.add_int(0, j, left, 32768);
			stream.add_int(1, j, right, 32768);

			if (vptr.send_level > 0)
			{
				const int dest = vptr.send_dest & 0xf;
				if (dest != 0xf)
				{
					const int64_t send = (sample * (uint64_t(vptr.send_level) * uint64_t(0xffff - vptr.env_volume))) >> 24;
					stream.add_int(2 + (dest - 7), j, send, 32768);
				}
			}
		}

		vptr.pos = pos;
		vptr.frac = frac;
	}
}

uint16_t l4003_sound_device::data_r()
{
	LOGMASKED(LOG_REGISTERS, "%s: read data, control %04x (%s)\n", tag(), m_control, machine().describe_context());

	return m_data;
}

void l4003_sound_device::data_w(uint16_t data)
{
	LOGMASKED(LOG_REGISTERS_HIFREQ, "%s: %04x to data\n", tag(), data);

	m_data = data;
}

uint16_t l4003_sound_device::control_r()
{
	m_stream->update();

	LOGMASKED(LOG_REGISTERS, "%s: read control\n", tag());

	return m_control;
}

void l4003_sound_device::control_w(uint16_t data)
{
	m_stream->update();

	LOGMASKED(LOG_REGISTERS_HIFREQ, "%s: %04x to control\n", tag(), data);

	m_control = data;

	if (data < 0x0200)
	{
		int voice = data & 0xf;
		l4003_voice &v = m_voices[voice];

		switch (data >> 4)
		{
			// set sample pointer low
			case 0x0000:
				v.start &= ~0xf;
				v.start |= (m_data >> 12);
				LOGMASKED(LOG_REGISTERS, "ch %d Sample ptr low %04x => %04x\n", voice, m_data, v.start);
				break;

			// set sample pointer high
			case 0x0001:
				v.start &= 0xf;
				v.start |= (m_data << 4);
				LOGMASKED(LOG_REGISTERS, "ch %d Sample ptr high %04x => %04x\n", voice, m_data, v.start);
				break;

			// pitch in 4.12 fixed-point format, similar to the L6028
			case 0x0002:
				// if the pitch is being set from 0 to nonzero, it's a key-on so reset some state
				if ((m_data != 0) && (v.step == 0))
				{
					LOGMASKED(LOG_KEYON, "ch %d: Key on @ pitch %04x start %08x env rate %04x target %04x L %02x R %02x\n", voice, m_data, v.start, v.env_step, v.env_target, v.l_volume, v.r_volume);
					v.pos = 0;
					v.frac = 0;
					v.env_pos = 0;
					v.filt_state = 0;
				}

				v.step = m_data;

				LOGMASKED(LOG_REGISTERS, "ch %d: Pitch %04x (%s)\n", voice, m_data, machine().describe_context());
				break;

			case 0x0003:
				v.send_level = (m_data >> 8) & 0xff;
				v.send_dest = m_data & 0xf;
				LOGMASKED(LOG_REGISTERS, "ch %d: Send volume %02x, dest %x (%s)\n", voice, v.send_level, v.send_dest, machine().describe_context());
				break;

			case 0x0004:
				v.env_volume = m_data;
				LOGMASKED(LOG_ENVELOPE, "ch %d: Envelope volume %04x (%s)\n", voice, v.env_volume, machine().describe_context());
				break;

			case 0x0005:
				v.env_target = m_data;
				LOGMASKED(LOG_ENVELOPE, "ch %d: Envelope target %04x (cur %04x) (%s)\n", voice, v.env_target, v.env_volume, machine().describe_context());
				break;

			case 0x0006:
				v.env_step = m_data;
				LOGMASKED(LOG_ENVELOPE, "ch %d: Envelope rate %04x (%s)\n", voice, v.env_step, machine().describe_context());
				break;

			case 0x0007:
				v.l_volume = m_data & 0xff;
				v.r_volume = (m_data >> 8) & 0xff;
				LOGMASKED(LOG_VOLUME, "ch %d: Volume L %02x R %02x (env %04x) (%s)\n", voice, v.l_volume, v.r_volume, v.env_volume, machine().describe_context());
				break;

// ----- Readbacks ----------------------------

			case 0x0011:
				m_data = v.start >> 4;
				LOGMASKED(LOG_REGISTERS, "ch %d: Sample ptr readback %04x (%s)\n", voice, v.start, machine().describe_context());
				break;

			case 0x0015:
				m_data = v.env_target;
				LOGMASKED(LOG_REGISTERS, "ch %d: Envelope target readback %04x (%s)\n", voice, v.env_target, machine().describe_context());
				break;

			case 0x0016:
				m_data = v.env_step;
				LOGMASKED(LOG_REGISTERS, "ch %d: Envelope rate readback %04x (%s)\n", voice, v.env_step, machine().describe_context());
				break;

			default:
				LOGMASKED(LOG_REGISTERS, "ch %d: Unknown register %04x (data %04x) (%s)\n", voice, data>>4, m_data, machine().describe_context());
				break;
			}
	}
	else
	{
		switch (data)
		{
			// write 12-bit sample to memory at voice 1's sample pointer
			case 0x0200:
				LOGMASKED(LOG_SAMPLE_WRITE, "Write sample %04x to sample ptr %04x\n", m_data, m_voices[1].start);
				m_cache.write_word(m_voices[1].start << 1, m_data);
				break;

			// read 12-bit sample from memory at voice 1's sample pointer (read the control register for this)
			case 0x0300:
				m_control = m_cache.read_word(m_voices[1].start << 1);
				LOGMASKED(LOG_SAMPLE_READ, "Read sample %04x from sample ptr %04x\n", m_control, m_voices[1].start);
				break;

			default:
				LOGMASKED(LOG_REGISTERS, "Unknown %04x to control high (data %04x) (%s)\n", data, m_data, machine().describe_context());
				break;
		}
	}
}

