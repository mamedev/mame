// license:BSD-3-Clause
// copyright-holders:superctr
/*
    Yamaha YM3413 "LDSP"

    Reverb/delay DSP with external delay RAM, found in the TG100, SY77, FX900 and many other
    Yamaha products.

    The chip runs a 64-step program once per sample, one step per CLK period (CLK = 64 * fs).
    There is no program flow control. Each step has four control bytes and a signed 8-bit
    coefficient. Each pair of steps has a 16-bit delay memory address (the RAM is 8 bits wide
    and takes two clocks per word), which is offset by a counter that decrements once per
    sample.

    Control data port (CDI, clocked by XCLK): 32-bit words, of which the TG100 only ever sends
    00 80 <command> <data>.
        00  control. FE before loading a program, FC after (meaning unknown)
        01  unknown, only ever 05
        02  pointer into the selected bank
        03  bank select: 0-3 control bytes, 6 delay addresses, 7 coefficients
        04  write data at pointer, post-increment. For bank 6, low byte of the address
        05  bank 6 only, high byte of the address (sent before the low byte)
        06  output level: ((v >> 2 & 7) | 8) << (v >> 5), 2048 = unity

    Datapath: register file R (8 words) is loaded from the memory read data or from the serial
    input. Register file T (16 words) is loaded from the accumulator. A single multiplier takes
    a word from either file and the step's coefficient (1.7 fixed point); the product is either
    added to the accumulator or loaded into it, optionally together with a word from T.
    Whatever is stored to T or to delay memory in a step is the accumulator as left by the
    previous step, optionally doubled, saturated to 16 bits.

    Control bytes:
        byte 0  7    T write address bit 0
                654  R read address
                3    R write enable
                210  R write address
        byte 1  7654 T read address
                3    T write enable
                210  T write address bits 3-1
        byte 2  5    stored value is accumulator * 2
                4    accumulator = product (otherwise product + T, if bit 0 is clear)
                2    T read word goes to the output level multiplier, accumulator holds
                1    multiplier input is T instead of R
                0    accumulator += product
        byte 3  7    memory write (otherwise read)
                6    memory access in this pair of steps
                5    serial input channel
                3    R write data is the serial input instead of memory read data
                2    latch output
                0    output channel
        Byte 2 bits 7, 6, 3 and byte 3 bits 4, 1 are never set by the TG100.

    Unknowns and assumptions:
    - Word size, accumulator width, truncation and saturation behavior. (Truncation toward zero
      was picked because the alternatives never decay to silence, not because it was measured.)
    - Alignment of the program to the sample sync input, and the step at which the address
      counter decrements. The TG100 programs only produce the intended delay lengths if it
      decrements somewhere in steps 55-60, which is what is done here.
    - Memory read data is available to R three steps after the memory access.
    - Which of SI0/SI1 and SO0/SO1 the channel bits refer to. The TG100 only connects SI0 and
      SO0 and still gets stereo output, so the bits are taken to select the left/right word
      of one pin here. The never used bits next to them may select the pin.
    - Output has not been verified against the real chip.
*/

#include "emu.h"
#include "ym3413.h"

#include <algorithm>

#define LOG_CMD     (1U << 1)
#define LOG_UNKNOWN (1U << 2)

#define VERBOSE (LOG_UNKNOWN)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(YM3413, ym3413_device, "ym3413", "Yamaha YM3413 LDSP")

ym3413_device::ym3413_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, YM3413, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_stream(nullptr),
	m_ram_words(0x4000),
	m_ram_mask(0)
{
}

void ym3413_device::device_start()
{
	if (!m_ram_words || (m_ram_words & (m_ram_words - 1)) || m_ram_words > 0x10000)
		fatalerror("%s: invalid delay RAM size\n", tag());

	m_ram_mask = m_ram_words - 1;
	m_ram = make_unique_clear<s16 []>(m_ram_words);

	m_stream = stream_alloc(2, 2, clock() / STEPS);

	save_pointer(NAME(m_ram), m_ram_words);
	save_item(NAME(m_cd_buf));
	save_item(NAME(m_cd_count));
	save_item(NAME(m_control));
	save_item(NAME(m_mode));
	save_item(NAME(m_pointer));
	save_item(NAME(m_bank));
	save_item(NAME(m_level));
	save_item(NAME(m_micro));
	save_item(NAME(m_coef));
	save_item(NAME(m_addr));
	save_item(NAME(m_r));
	save_item(NAME(m_t));
	save_item(NAME(m_acc));
	save_item(NAME(m_mdr));
	save_item(NAME(m_mdr_next));
	save_item(NAME(m_mdr_step));
	save_item(NAME(m_out_bus));
	save_item(NAME(m_out));
	save_item(NAME(m_in));
	save_item(NAME(m_counter));
}

void ym3413_device::device_reset()
{
	m_cd_count = 0;

	m_control = 0;
	m_mode = 0;
	m_pointer = 0;
	m_bank = 0;
	m_level = 0;
	std::fill_n(&m_micro[0][0], 4 * STEPS, 0);
	std::fill(std::begin(m_coef), std::end(m_coef), 0);
	std::fill(std::begin(m_addr), std::end(m_addr), 0);

	std::fill(std::begin(m_r), std::end(m_r), 0);
	std::fill(std::begin(m_t), std::end(m_t), 0);
	m_acc = 0;
	m_mdr = m_mdr_next = 0;
	m_mdr_step = 0xff;
	m_out_bus = 0;
	m_out[0] = m_out[1] = 0;
	m_in[0] = m_in[1] = 0;
	m_counter = 0;
}

void ym3413_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / STEPS);
}

void ym3413_device::cd_w(u8 data)
{
	m_cd_buf[m_cd_count++] = data;
	if (m_cd_count == 4)
	{
		m_cd_count = 0;
		if (m_cd_buf[0] != 0x00 || m_cd_buf[1] != 0x80)
			LOGMASKED(LOG_UNKNOWN, "unknown control word header %02x %02x (%02x %02x)\n", m_cd_buf[0], m_cd_buf[1], m_cd_buf[2], m_cd_buf[3]);
		command(m_cd_buf[2], m_cd_buf[3]);
	}
}

void ym3413_device::command(u8 cmd, u8 data)
{
	m_stream->update();
	LOGMASKED(LOG_CMD, "command %02x %02x\n", cmd, data);

	switch (cmd)
	{
	case 0x00:
		m_control = data;
		break;

	case 0x01:
		m_mode = data;
		break;

	case 0x02:
		m_pointer = data;
		break;

	case 0x03:
		m_bank = data;
		break;

	case 0x04:
		switch (m_bank)
		{
		case 0: case 1: case 2: case 3:
			m_micro[m_bank][m_pointer & (STEPS - 1)] = data;
			m_pointer++;
			break;

		case 6:
			// pointer unit for this bank is not known, it has only been seen written from 0
			m_addr[(m_pointer >> 1) & (STEPS / 2 - 1)] = (m_addr[(m_pointer >> 1) & (STEPS / 2 - 1)] & 0xff00) | data;
			m_pointer += 2;
			break;

		case 7:
			m_coef[m_pointer & (STEPS - 1)] = s8(data);
			m_pointer++;
			break;

		default:
			LOGMASKED(LOG_UNKNOWN, "write %02x to unknown bank %02x (pointer %02x)\n", data, m_bank, m_pointer);
			m_pointer++;
			break;
		}
		break;

	case 0x05:
		if (m_bank == 6)
			m_addr[(m_pointer >> 1) & (STEPS / 2 - 1)] = (m_addr[(m_pointer >> 1) & (STEPS / 2 - 1)] & 0x00ff) | (u16(data) << 8);
		else
			LOGMASKED(LOG_UNKNOWN, "command 05 %02x with bank %02x selected\n", data, m_bank);
		break;

	case 0x06:
		m_level = data;
		break;

	default:
		LOGMASKED(LOG_UNKNOWN, "unknown command %02x %02x\n", cmd, data);
		break;
	}
}

s16 ym3413_device::export_acc(s32 acc, bool shift)
{
	// The fraction is truncated toward zero. How the chip does it is not known: plainly dropping
	// the bits or rounding to nearest both leave a limit cycle (and the former a DC offset of
	// some -80 LSB) circulating in the reverb forever, this is the only choice that decays
	// to silence.
	const unsigned bits = shift ? (COEF_FRAC - 1) : COEF_FRAC;
	const s32 word = (acc < 0) ? -(-acc >> bits) : (acc >> bits);
	return std::clamp<s32>(word, -0x8000, 0x7fff);
}

void ym3413_device::run_frame(s16 in_l, s16 in_r)
{
	constexpr s32 acc_max = (s32(1) << (ACC_BITS - 1)) - 1;
	constexpr s32 acc_min = -(s32(1) << (ACC_BITS - 1));

	for (unsigned step = 0; step < STEPS; step++)
	{
		const u8 b0 = m_micro[0][step];
		const u8 b1 = m_micro[1][step];
		const u8 b2 = m_micro[2][step];
		const u8 b3 = m_micro[3][step];

		if (step == FRAME_STEP)
		{
			m_in[0] = in_l;
			m_in[1] = in_r;
			m_counter--;
		}

		if (step == m_mdr_step)
		{
			m_mdr = m_mdr_next;
			m_mdr_step = 0xff;
		}

		// register reads see the state before this step's writes
		const s16 r_read = m_r[(b0 >> 4) & 7];
		const s16 t_read = m_t[b1 >> 4];
		const s16 store = export_acc(m_acc, BIT(b2, 5));

		// delay memory, one access per pair of steps
		if (!(step & 1) && BIT(b3, 6))
		{
			const u32 address = (m_addr[step >> 1] + m_counter) & m_ram_mask;
			if (BIT(b3, 7))
			{
				m_ram[address] = store;
			}
			else
			{
				m_mdr_next = m_ram[address];
				m_mdr_step = (step + 2) & (STEPS - 1);
			}
		}

		if (BIT(b3, 2))
			m_out[b3 & 1] = m_out_bus;

		if (BIT(b2, 2))
		{
			m_out_bus = t_read;
		}
		else
		{
			s32 sum = s32(BIT(b2, 1) ? t_read : r_read) * m_coef[step];
			if (!BIT(b2, 4))
				sum += BIT(b2, 0) ? m_acc : (s32(t_read) << COEF_FRAC);
			m_acc = std::clamp(sum, acc_min, acc_max);
		}

		if (BIT(b0, 3))
			m_r[b0 & 7] = BIT(b3, 3) ? m_in[BIT(b3, 5)] : m_mdr;
		if (BIT(b1, 3))
			m_t[((b1 & 7) << 1) | (b0 >> 7)] = store;
	}
}

void ym3413_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		const s16 in_l = std::clamp<s32>(lround(stream.get(0, i) * 32768.0), -0x8000, 0x7fff);
		const s16 in_r = std::clamp<s32>(lround(stream.get(1, i) * 32768.0), -0x8000, 0x7fff);

		run_frame(in_l, in_r);

		const s32 gain = (((m_level >> 2) & 7) | 8) << (m_level >> 5);
		stream.put_int_clamp(0, i, (m_out[0] * gain) >> 11, 32768);
		stream.put_int_clamp(1, i, (m_out[1] * gain) >> 11, 32768);
	}
}
