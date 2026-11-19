// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Toshiba T6721A C2MOS Voice Synthesizing LSI emulation

**********************************************************************/

#include "emu.h"
#include "t6721a.h"

#include <algorithm>
#include <cstdlib>
#include <iterator>

DEFINE_DEVICE_TYPE(T6721A, t6721a_device, "t6721a", "Toshiba T6721A")

namespace {

constexpr unsigned PARAMETER_BITS_96[12] = { 7, 7, 10, 10, 10, 8, 8, 8, 7, 7, 7, 7 };
constexpr unsigned PARAMETER_BITS_48[12] = { 6, 6, 7, 5, 5, 4, 4, 4, 4, 3, 0, 0 };
constexpr int32_t PULSE_AMP = 6523;

// 48-bit parameter tables estimated from the 96-bit Magic Voice vocabulary by quantile matching
constexpr uint8_t ENERGY_48[64] =
{
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
	2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 8, 8, 8,
	10, 10, 12, 12, 13, 14, 16, 17, 19, 22, 24, 28, 30, 36, 41, 43,
	46, 51, 57, 64, 67, 71, 79, 88, 97, 108, 120, 126, 126, 126, 126, 127
};

constexpr uint8_t PITCH_48[64] =
{
	0, 21, 22, 22, 24, 26, 27, 28, 28, 29, 29, 30, 31, 32, 33, 34,
	34, 35, 36, 37, 38, 39, 40, 41, 43, 44, 45, 46, 47, 48, 49, 49,
	50, 51, 52, 52, 53, 54, 55, 56, 56, 57, 58, 58, 59, 59, 60, 60,
	60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75
};

constexpr int16_t COEFFICIENTS_48[264] =
{
	8064, 4152, 2648, 2264, 1736, 1136, 464, 128, 128, 96, 32, -448, -1344, -1928, -2200, -2360,
	-2536, -2816, -2984, -3064, -3248, -3744, -4304, -4512, -4512, -4776, -5344, -5688, -5760, -5904, -6256, -6656,
	-6872, -7072, -7336, -7664, -8072, -8360, -8656, -9008, -9232, -9432, -9608, -9728, -9912, -10168, -10448, -10768,
	-11168, -11544, -11808, -12072, -12328, -12536, -12752, -13016, -13400, -13784, -14000, -14136, -14336, -14608, -14840, -14968,
	-15024, -15072, -15120, -15200, -15328, -15416, -15464, -15488, -15496, -15512, -15520, -15520, -15520, -15520, -15520, -15560,
	-15648, -15704, -15728, -15760, -15776, -15784, -15800, -15816, -15832, -15840, -15856, -15904, -15952, -15968, -15968, -15984,
	-16016, -16032, -16032, -16032, -16032, -16032, -16040, -16056, -16064, -16064, -16064, -16072, -16088, -16128, -16192, -16224,
	-16225, -16230, -16236, -16241, -16247, -16253, -16259, -16265, -16271, -16276, -16282, -16288, -16294, -16300, -16305, -16311,
	15104, 14960, 14672, 14448, 14224, 13872, 13400, 12904, 12376, 11824, 11320, 10544, 9448, 8544, 7792, 6696,
	5424, 4576, 3968, 2992, 1616, 200, -992, -2040, -3176, -4640, -6264, -7392, -7960, -8608, -9320, -9664,
	9696, 7512, 6384, 6088, 5832, 5152, 4248, 3496, 2760, 1976, 1320, 872, 480, 80, -408, -1080,
	-1832, -2712, -3640, -4512, -5312, -6016, -6832, -7760, -8600, -9336, -9984, -10720, -11416, -11816, -12024, -12096,
	-8448, -7232, -6016, -4512, -3136, -1920, -608, 768, 2016, 3200, 4736, 6208, 7200, 8064, 8928, 9856,
	8704, 6304, 4896, 3808, 2784, 1920, 928, 32, -704, -1504, -2496, -3680, -4832, -6208, -8064, -10112,
	9600, 9152, 8192, 7200, 6368, 5504, 4672, 3616, 2144, 928, 352, 64, -256, -960, -2048, -2944,
	6144, 2688, 1024, 320, -320, -1088, -1920, -2496, -2944, -3456, -3968, -4608, -5184, -5376, -5376, -5376,
	-5120, -4800, -3968, -2816, -1408, -64, 1472, 3840
};

constexpr unsigned COEFFICIENT_OFFSETS_48[8] = { 0, 128, 160, 192, 208, 224, 240, 256 };

constexpr unsigned SPEED_PERCENT[16] = { 100, 70, 80, 90, 100, 110, 120, 130, 140, 150, 155, 100, 100, 100, 100, 100 };

} // anonymous namespace

t6721a_device::t6721a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, T6721A, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_read_rom(*this, 0xff),
	m_write_bsy(*this),
	m_write_eos(*this),
	m_write_phi2(*this),
	m_write_dtrd(*this),
	m_write_apd(*this),
	m_stream(nullptr)
{
}

void t6721a_device::device_start()
{
	m_stream = stream_alloc(0, 1, clock() / 80);
	m_serial_timer = timer_alloc(FUNC(t6721a_device::serial_tick), this);
	m_frame_timer = timer_alloc(FUNC(t6721a_device::frame_tick), this);
	m_busy_timer = timer_alloc(FUNC(t6721a_device::busy_tick), this);
	m_eos_timer = timer_alloc(FUNC(t6721a_device::eos_tick), this);

	save_item(NAME(m_status));
	save_item(NAME(m_command));
	save_item(NAME(m_arguments));
	save_item(NAME(m_speed));
	save_item(NAME(m_condition1));
	save_item(NAME(m_condition2));
	save_item(NAME(m_address));
	save_item(NAME(m_rom_bit));
	save_item(NAME(m_rom_data));
	save_item(NAME(m_rom_read));
	save_item(NAME(m_serial_rom));
	save_item(NAME(m_speaking));
	save_item(NAME(m_first_frame));
	save_item(NAME(m_all_ones));
	save_item(NAME(m_busy));
	save_item(NAME(m_eos));
	save_item(NAME(m_apd));
	save_item(NAME(m_di));
	save_item(NAME(m_serial_bits));
	save_item(NAME(m_nibble_bit));
	save_item(NAME(m_parameter));
	save_item(NAME(m_parameter_bits));
	save_item(NAME(m_parameters));
	save_item(NAME(m_sample));
	save_item(NAME(m_samples));
	save_item(NAME(m_pitch_count));
	save_item(NAME(m_noise));
	save_item(NAME(m_glottal));
	save_item(NAME(m_previous));
	save_item(NAME(m_target));
	save_item(NAME(m_delay));
}

void t6721a_device::device_reset()
{
	m_stream->update();
	m_status = 1;
	m_command = CMD_NOP;
	m_arguments = 0;
	m_speed = 0;
	m_condition1 = 0;
	m_condition2 = 0;
	m_address = 0;
	m_rom_bit = 0;
	m_rom_data = 0;
	m_rom_read = false;
	m_serial_rom = false;
	m_speaking = false;
	m_first_frame = false;
	m_all_ones = true;
	m_busy = false;
	m_eos = 1;
	m_apd = 1;
	m_di = 1;
	m_serial_bits = 0;
	m_nibble_bit = 0;
	m_parameter = 0;
	m_parameter_bits = 0;
	m_sample = 0;
	m_samples = frame_samples();
	m_pitch_count = 0;
	m_noise = 1;
	m_glottal[0] = m_glottal[1] = 0;
	std::fill(std::begin(m_parameters), std::end(m_parameters), 0);
	std::fill(std::begin(m_previous), std::end(m_previous), 0);
	std::fill(std::begin(m_target), std::end(m_target), 0);
	std::fill(std::begin(m_delay), std::end(m_delay), 0);
	m_serial_timer->adjust(attotime::never);
	m_frame_timer->adjust(attotime::never);
	m_eos_timer->adjust(attotime::never);
	m_write_phi2(0);
	m_write_dtrd(1);
	m_write_eos(1);
	m_write_apd(1);
	set_busy(19'200);
}

void t6721a_device::device_clock_changed()
{
	if (m_stream)
		m_stream->set_sample_rate(clock() / 80);
}

unsigned t6721a_device::frame_samples() const
{
	return (BIT(m_condition2, 2) ? 80 : 160) * SPEED_PERCENT[m_speed] / 100;
}

void t6721a_device::set_busy(unsigned clocks)
{
	m_busy = true;
	m_write_bsy(0);
	m_busy_timer->adjust(clocks_to_attotime(clocks));
}

TIMER_CALLBACK_MEMBER(t6721a_device::busy_tick)
{
	m_busy = false;
	m_write_bsy(1);
}

int t6721a_device::bsy_r()
{
	return !m_busy;
}

void t6721a_device::set_eos(int state)
{
	if (m_eos != state)
	{
		m_eos = state;
		m_write_eos(state);
	}
}

TIMER_CALLBACK_MEMBER(t6721a_device::eos_tick)
{
	set_eos(1);
}

void t6721a_device::set_apd(int state)
{
	if (m_apd != state)
	{
		m_apd = state;
		m_write_apd(state);
	}
}

void t6721a_device::stop()
{
	m_speaking = false;
	m_status |= 1;
	m_serial_timer->adjust(attotime::never);
	m_frame_timer->adjust(attotime::never);
	m_write_phi2(0);
	m_write_dtrd(1);
	m_sample = 0;
	m_pitch_count = 0;
	m_glottal[0] = m_glottal[1] = 0;
	std::fill(std::begin(m_previous), std::end(m_previous), 0);
	std::fill(std::begin(m_target), std::end(m_target), 0);
	std::fill(std::begin(m_delay), std::end(m_delay), 0);
}

uint8_t t6721a_device::read()
{
	uint8_t const result = m_rom_read ? m_rom_data : m_status;
	if (machine().side_effects_disabled())
		return result;

	if (m_busy)
	{
		m_status |= 8;
		return result;
	}

	m_arguments = 0;
	if (m_rom_read)
	{
		begin_serial(true);
		set_busy(44);
	}
	else
	{
		set_busy(16);
	}
	return result;
}

void t6721a_device::write(uint8_t data)
{
	data &= 15;
	if ((m_status & 2) && data != CMD_SAGN)
		return;
	if (m_busy)
	{
		m_status |= 8;
		return;
	}
	m_stream->update();
	set_busy(16);

	if (m_arguments)
	{
		switch (m_command)
		{
		case CMD_ADLD:
			m_address |= uint32_t(data) << (4 * (5 - m_arguments));
			if (m_arguments == 1)
				m_rom_bit = m_address * 8;
			break;
		case CMD_SPLD:
			m_speed = data;
			break;
		case CMD_CNDT1:
			m_condition1 = data & 12;
			break;
		case CMD_CNDT2:
			m_condition2 = data;
			break;
		}
		--m_arguments;
		return;
	}

	m_command = data;
	m_rom_read = false;
	switch (data)
	{
	case CMD_NOP:
		break;
	case CMD_STRT:
		if (!m_speaking)
		{
			stop();
			m_speaking = true;
			m_first_frame = true;
			m_nibble_bit = 0;
			m_status &= ~1;
			m_eos_timer->adjust(attotime::never);
			set_eos(1);
			m_samples = frame_samples();
			m_frame_timer->adjust(clocks_to_attotime(m_samples * 80));
		}
		break;
	case CMD_STOP:
		stop();
		m_status &= 3;
		break;
	case CMD_ADLD:
		if (!m_speaking)
		{
			m_address = 0;
			m_arguments = 5;
		}
		break;
	case CMD_AAGN:
		set_apd(0);
		break;
	case CMD_SPLD:
	case CMD_CNDT1:
	case CMD_CNDT2:
		if (!m_speaking)
			m_arguments = 1;
		break;
	case CMD_RRDM:
		if (!m_speaking)
		{
			m_rom_read = true;
			m_nibble_bit = 0;
			begin_serial(true);
			set_busy(44);
		}
		break;
	case CMD_SPDN:
		stop();
		m_status = 3;
		m_speed = m_condition1 = m_condition2 = 0;
		m_address = m_rom_bit = 0;
		set_apd(1);
		break;
	case CMD_APDN:
		set_apd(1);
		break;
	case CMD_SAGN:
		m_status &= ~2;
		set_busy(19'200);
		break;
	default:
		m_status |= 8;
		break;
	}
}

void t6721a_device::di_w(int state)
{
	m_di = bool(state);
}

int t6721a_device::eos_r()
{
	return m_eos;
}

void t6721a_device::begin_serial(bool rom_read)
{
	m_serial_rom = rom_read;
	m_serial_bits = 0;
	m_parameter = 0;
	m_parameter_bits = 0;
	m_all_ones = true;
	std::fill(std::begin(m_parameters), std::end(m_parameters), 0);
	m_serial_timer->adjust(clocks_to_attotime(4));
}

TIMER_CALLBACK_MEMBER(t6721a_device::frame_tick)
{
	begin_serial(false);
	m_frame_timer->adjust(clocks_to_attotime(frame_samples() * 80));
}

TIMER_CALLBACK_MEMBER(t6721a_device::serial_tick)
{
	m_serial_timer->adjust(clocks_to_attotime(((m_nibble_bit + 1) & 3) ? 4 : 68));
	if (!m_nibble_bit)
		m_write_dtrd(0);
	m_write_phi2(1);
	int const bit = m_read_rom.isunset() ? m_di : BIT(m_read_rom((m_rom_bit >> 3) & 0xf'ffff), m_rom_bit & 7);
	m_write_phi2(0);
	m_rom_bit = (m_rom_bit + 1) & 0x7f'ffff;
	++m_serial_bits;
	m_nibble_bit = (m_nibble_bit + 1) & 3;
	if (!m_nibble_bit)
		m_write_dtrd(1);

	if (m_serial_rom)
	{
		m_parameters[0] |= bit << (m_serial_bits - 1);
		if (m_serial_bits == 4)
		{
			m_rom_data = m_parameters[0];
			m_serial_timer->adjust(attotime::never);
		}
	}
	else
	{
		accept_bit(bit);
	}
}

void t6721a_device::accept_bit(int bit)
{
	bool const bits96 = BIT(m_condition2, 3);
	if (!bits96 && m_first_frame && m_serial_bits == 1)
		return;

	m_all_ones &= bool(bit);
	if (bits96 && m_parameter == 1 && !m_parameter_bits && !m_parameters[0])
	{
		if (bit)
			end_of_speech();
		else
			silent_frame();
		return;
	}

	unsigned const *const widths = bits96 ? PARAMETER_BITS_96 : PARAMETER_BITS_48;
	m_parameters[m_parameter] |= bit << m_parameter_bits;
	if (++m_parameter_bits == widths[m_parameter])
	{
		m_parameter_bits = 0;
		++m_parameter;
		if (!bits96 && m_parameter == 1 && !m_parameters[0])
			silent_frame();
		else if (!bits96 && m_parameter == 1 && m_parameters[0] == 63)
			end_of_speech();
		else if (m_parameter == (!m_parameters[1] ? 6U : bits96 ? 12U : 10U))
			decode_frame();
	}
}

void t6721a_device::silent_frame()
{
	m_stream->update();
	m_serial_timer->adjust(attotime::never);
	if (m_first_frame)
	{
		m_first_frame = false;
		m_status &= ~4;
	}
	std::copy(std::begin(m_target), std::end(m_target), std::begin(m_previous));
	m_target[0] = 0;
	m_sample = 0;
}

void t6721a_device::end_of_speech()
{
	m_stream->update();
	m_serial_timer->adjust(attotime::never);
	if (m_first_frame)
	{
		m_first_frame = false;
		m_status &= ~4;
	}
	stop();
	set_eos(0);
	m_eos_timer->adjust(clocks_to_attotime(frame_samples() * 80));
}

void t6721a_device::decode_frame()
{
	m_stream->update();
	m_serial_timer->adjust(attotime::never);
	if (m_first_frame)
	{
		m_first_frame = false;
		m_status = (m_status & ~4) | (m_all_ones ? 4 : 0);
	}
	if (m_all_ones)
	{
		stop();
		return;
	}

	std::copy(std::begin(m_target), std::end(m_target), std::begin(m_previous));
	bool const bits96 = BIT(m_condition2, 3);
	m_target[0] = bits96 ? m_parameters[0] : ENERGY_48[m_parameters[0]];
	m_target[1] = bits96 ? m_parameters[1] : PITCH_48[m_parameters[1]];
	for (unsigned i = 2; i < 12; ++i)
	{
		if (i >= m_parameter)
			m_target[i] = 0;
		else if (bits96)
			m_target[i] = util::sext(m_parameters[i], PARAMETER_BITS_96[i]) * (1 << (15 - PARAMETER_BITS_96[i]));
		else
			m_target[i] = COEFFICIENTS_48[COEFFICIENT_OFFSETS_48[i - 2] + m_parameters[i]];
	}
	if (m_target[0] == 1 && m_target[1] == 126)
		m_target[0] = 0;
	m_sample = 0;
	m_samples = frame_samples();
}

void t6721a_device::sound_stream_update(sound_stream &stream)
{
	for (int samp = 0; samp < stream.samples(); ++samp)
	{
		int32_t output = 0;
		if (m_speaking)
		{
			unsigned const fraction = std::min(8U, unsigned(m_sample) * 8 / m_samples);
			int32_t values[12];
			for (unsigned i = 0; i < 12; ++i)
				values[i] = (m_previous[i] * int(8 - fraction) + m_target[i] * int(fraction)) / 8;

			m_noise = (m_noise >> 1) | ((BIT(m_noise, 0) ^ BIT(m_noise, 1)) << 14);
			if (!m_target[1])
			{
				output = BIT(m_noise, 0) ? 4096 : -4096;
			}
			else
			{
				unsigned const pitch = std::max(1, values[1]);
				m_pitch_count = (m_pitch_count + 1) % pitch;
				if (BIT(m_condition1, 3))
				{
					output = 4096 - std::abs(int(m_pitch_count * 16384 / pitch) - 8192);
				}
				else
				{
					int32_t const glottal = (m_pitch_count ? 0 : PULSE_AMP) + (m_glottal[0] * 778 - m_glottal[1] * 148) / 1024;
					m_glottal[1] = m_glottal[0];
					m_glottal[0] = glottal;
					output = glottal;
				}
			}
			output = output * values[0] / 127;
			int const stages = BIT(m_condition2, 0) ? 8 : 10;
			for (int i = stages - 1; i >= 0; --i)
			{
				output = std::clamp(output - int32_t(int64_t(values[i + 2]) * m_delay[i] / 16384), -16384, 16383);
				if (i + 1 < stages)
					m_delay[i + 1] = m_delay[i] + int32_t(int64_t(values[i + 2]) * output / 16384);
			}
			m_delay[0] = output;
			if (BIT(m_condition1, 2))
			{
				for (auto &delay : m_delay)
					delay -= delay / 256;
			}
			if (m_sample < m_samples)
				++m_sample;
		}
		stream.put_int(0, samp, m_apd ? 0 : (output & ~63), 16384);
	}
}
