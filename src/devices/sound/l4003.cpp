// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**************************************************************************************************

    Akai/Roger Linn L4003 DSP
    Emulation by R. Belmont

    This is the audio chip used in the Akai MPC60, with 16 voices of 12-bit sample playback.
    No filter, but some sort of "echo buffer" exists which is probably similar/identical to the
    delay effect on the later L6028 chip.

    The chip has a total of 10 unique outputs, 8 individual outputs and a stereo pair.  Each voice
    can be sent to an individual output, the stereo pair, or both.

    The chip has no DMA and is interfaced to the CPU by 2 16-bit wide ports.  As with the later
    L6028 chip, there is no byte lane select, reads and writes are always 16 bits wide.

    The wave data bus is 12 bits wide.  Configurations of 512K or 1024K 12-bit words are supported.
    (768K or 1536K bytes).  For ease of emulation, we make those 16-bit words.

    Voice registers (low 4 bits are the voice number)
    001x - ssss ssss ssss ssss
    s - loop start offset in samples from the start of memory

    011x - ssss ssss ssss ssss
    s - start offset in samples from the start of memory

    002x - key on/off

    003x - eeee eeee ???? oooo
    e - echo send level
    o - mix output where 0 = 7, 1 = 8, 2 = 9, ... f = 8

    004x - unknown

    005x - unknown

    006x - ???? ???? ???? ????
    This is somehow both volume and pitch.  Volume changes by 0x66 each step, pitch by 0x4c (102 and 76)

**************************************************************************************************/

#include "emu.h"
#include "l4003.h"
#include "debugger.h"

#define LOG_REGISTERS           (1U << 1)
#define LOG_REGISTERS_HIFREQ    (1U << 2)
#define LOG_SAMPLE_READ         (1U << 3)
#define LOG_SAMPLE_WRITE        (1U << 4)

#define VERBOSE (0)

//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(L4003, l4003_sound_device, "l4003", "Akai/Roger Linn L4003 wavetable sound")

l4003_sound_device::l4003_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, L4003, tag, owner, clock)
	  , device_sound_interface(mconfig, *this)
	  , device_memory_interface(mconfig, *this)
	  , m_stream(nullptr)
	  , m_mem_config("l4003", ENDIANNESS_LITTLE, 16, 21)
	  , m_data(0)
	  , m_control(0)
	  , m_key_status(0)
	  , m_sample_rate(0)
{
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
	m_stream = stream_alloc(0, 10, m_sample_rate);
}

void l4003_sound_device::device_reset()
{
}

void l4003_sound_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < NUM_VOICES; i++)
	{
		if (m_key_status & (1 << i))
		{
			l4003_voice &vptr = m_voices[i];

			uint32_t start = vptr.start;
			const uint32_t step = vptr.step;

			uint32_t pos = vptr.pos;
			uint32_t frac = vptr.frac;

			for (int j = 0; j < stream.samples(); j++)
			{
				uint32_t address;
				int32_t sample;

				pos += (frac >> 12);
				frac &= 0xfff;

				address = start + pos;
				sample = (int16_t)m_cache.read_word(address);

				frac += step;

				if (vptr.send_level == 0)
				{
					sample = 0;
				}

				const int64_t left = (sample * (uint64_t(vptr.volume) * uint64_t(vptr.env_volume))) >> 24;
				const int64_t right = (sample * (uint64_t(vptr.volume) * uint64_t(vptr.env_volume))) >> 24;
				stream.add_int(0, j, left, 32768);
				stream.add_int(1, j, right, 32768);

				if (vptr.send_level > 0)
				{
					const int dest = vptr.send_dest & 0xf;
					if (dest != 0xf)
					{
						const int64_t send = (sample * (uint64_t(vptr.send_level) * uint64_t(vptr.env_volume))) >> 24;
						stream.add_int(2 + (dest - 7), j, send, 32768);
					}
				}
			}

			vptr.pos = pos;
			vptr.frac = frac;
		}
	}
}

uint16_t l4003_sound_device::data_r()
{
	m_stream->update();

	LOGMASKED(LOG_REGISTERS, "%s: read data, control %04x (%s)\n", tag(), m_control, machine().describe_context());

	return m_data;
}

void l4003_sound_device::data_w(uint16_t data)
{
	m_stream->update();

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

			// key on/off
			case 0x0002:
				if (m_data & 0x1000)
				{
					m_key_status |= (1 << voice);

					v.pos = 0;
					v.frac = 0;
					v.step = 0x1000;
					v.env_volume = 0x7fff;
				}
				else
				{
					m_key_status &= ~(1 << voice);
				}

				LOGMASKED(LOG_REGISTERS, "ch %d: Key %s %04x (%s)\n", voice, (m_data & 0x1000) ? "on" : "off", m_data, machine().describe_context());
				break;

			case 0x0003:
				v.send_level = (m_data >> 8) & 0xff;
				v.send_dest = m_data & 0x0f;
				LOGMASKED(LOG_REGISTERS, "ch %d: Echo send volume %02x, dest %x (%s)\n", voice, v.send_level, v.send_dest, machine().describe_context());
				break;

			case 0x0006:
				v.volume = (m_data / 0x66) & 0xff;
				LOGMASKED(LOG_REGISTERS, "ch %d: Volume %02x (%s)\n", voice, v.volume, machine().describe_context());
				break;

			case 0x0011:
				v.loop_start = m_data;
				LOGMASKED(LOG_REGISTERS, "ch %d Loop start %04x\n", voice, m_data);
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
