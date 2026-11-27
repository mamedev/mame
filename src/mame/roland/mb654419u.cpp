// license:BSD-3-Clause
// copyright-holders:superctr, Carl Lom
/***************************************************************************

    Fujitsu MB654419U (Roland TVF interface gate array)

    Between the SA-16 wave gate array, the TVF16 DSP and the DAC in the
    S-550, S-330 and W-30.

    Host interface (write only, even bytes):
      f000/f002  address, low and high byte
      f004/f006  data, low and high byte; the f006 write commits the pair,
                 or a program half-word to the TVF16 in the load state
      f008-f012  set-up, written once at start-up
      f00c       bit 7: TVF16 held in reset, the load state

    Every sample: the host's last address and data pair and the 16 voices
    to the TVF16's RAM 00-11 (D mode), the odd places negated, then FL0;
    the TVF16's eight outputs go to the DAC.

    TODO:
    - set-up registers f008-f012, f010 commands
    - timing within the SA-16's frame, 15 kHz mode, sampling (REC) mode

***************************************************************************/

#include "emu.h"
#include "mb654419u.h"

#include <algorithm>
#include <iterator>

#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(MB654419U, mb654419u_device, "mb654419u", "Fujitsu MB654419U TVF interface")

mb654419u_device::mb654419u_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MB654419U, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_dsp(*this, finder_base::DUMMY_TAG)
{
}

void mb654419u_device::device_start()
{
	m_stream = stream_alloc(NUM_VOICES, NUM_OUTPUTS, clock() / 224, STREAM_SYNCHRONOUS);

	m_address = 0;
	m_data = 0;
	std::fill(std::begin(m_pair), std::end(m_pair), 0);
	std::fill(std::begin(m_out), std::end(m_out), 0);
	m_out_count = 0;
	m_dac = 0;

	save_item(NAME(m_address));
	save_item(NAME(m_data));
	save_item(NAME(m_pair));
	save_item(NAME(m_out));
	save_item(NAME(m_out_count));
	save_item(NAME(m_dac));
	save_item(NAME(m_load));
}

void mb654419u_device::device_reset()
{
	m_load = true;
	m_dsp->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

void mb654419u_device::map(address_map &map)
{
	map(0x0, 0x1).w(FUNC(mb654419u_device::address_w));
	map(0x2, 0x3).w(FUNC(mb654419u_device::data_w));
	map(0x4, 0x5).w(FUNC(mb654419u_device::setup_w<0x4>));
	map(0x6, 0x6).w(FUNC(mb654419u_device::control_w));
	map(0x7, 0x9).w(FUNC(mb654419u_device::setup_w<0x7>));
}

void mb654419u_device::address_w(offs_t offset, u8 data)
{
	if (offset)
		m_address = (m_address & 0x00ff) | (data << 8);
	else
		m_address = (m_address & 0xff00) | data;
}

void mb654419u_device::data_w(offs_t offset, u8 data)
{
	if (!offset)
	{
		m_data = (m_data & 0xff00) | data;
		return;
	}

	m_data = (m_data & 0x00ff) | (data << 8);
	if (m_load)
	{
		LOG("%s: program %04x\n", machine().describe_context(), m_data);
		m_dsp->program_w(m_data);
	}
	else
	{
		LOG("%s: %02x = %04x\n", machine().describe_context(), m_address, m_data);
		m_pair[0] = m_address;
		m_pair[1] = m_data;
	}
}

void mb654419u_device::control_w(u8 data)
{
	LOG("%s: f00c = %02x\n", machine().describe_context(), data);
	if (BIT(data, 7) != m_load)
	{
		m_load = BIT(data, 7);
		m_dsp->set_input_line(INPUT_LINE_RESET, m_load ? ASSERT_LINE : CLEAR_LINE);
	}
}

template <u32 Base>
void mb654419u_device::setup_w(offs_t offset, u8 data)
{
	LOG("%s: f0%02x = %02x\n", machine().describe_context(), (Base + offset) * 2, data);
}

void mb654419u_device::dsp_w(u16 data)
{
	if (m_out_count < NUM_OUTPUTS)
		m_out[m_out_count++] = data;
	m_dac = data;
}

void mb654419u_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		for (int n = 0; n < NUM_OUTPUTS; n++)
			stream.put_int(n, i, s16(m_out[n]), 32768);
		m_out_count = 0;

		if (!m_load)
		{
			m_dsp->data_w(m_pair[0]);
			m_dsp->data_w(m_pair[1]);
			for (int place = 0; place < NUM_VOICES; place++)
			{
				const s32 sample = s32(stream.get((place - 2) & (NUM_VOICES - 1), i) * 32768.0);
				m_dsp->data_w(std::clamp<s32>(BIT(place, 0) ? -sample : sample, -0x8000, 0x7fff));
			}
			m_dsp->fl0_w(1);
			m_dsp->fl0_w(0);
		}
	}
}
