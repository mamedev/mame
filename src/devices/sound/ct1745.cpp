// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Creative Labs CT1745 SB16 Mixer chip

References:
- https://the.earth.li/~tfm/oldpage/sb_mixer.html

TODO:
- Pinout reference;
- IRQ select & mask for card host;
- Get a tag reference for CD and (ISA/C-Bus) buzzer speaker volume controls;
- Mic/Line In;
- Stereo control and low-pass filter;
- Input/Output Control (left/right rerouting);
- Gain Control + AGC;
- Bass/Treble;
- Track down really unavailable regs, return 0x80 in index status;
- Track down interactions with LLE DSP (UART?);

**************************************************************************************************/

#include "emu.h"
#include "ct1745.h"

DEFINE_DEVICE_TYPE(CT1745, ct1745_mixer_device, "ct1745", "Creative Labs CT1745 SB16 Mixer")

ct1745_mixer_device::ct1745_mixer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CT1745, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_mixer_interface(mconfig, *this)
	, m_irq_status_cb(*this, 0)
	, m_fm(*this, finder_base::DUMMY_TAG)
	, m_ldac(*this, finder_base::DUMMY_TAG)
	, m_rdac(*this, finder_base::DUMMY_TAG)
{
	m_space_config = address_space_config("regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(ct1745_mixer_device::map), this));
}

device_memory_interface::space_config_vector ct1745_mixer_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void ct1745_mixer_device::device_start()
{
	save_item(NAME(m_index));
	save_pointer(NAME(m_master_level), 2);
	save_pointer(NAME(m_dac_level), 2);
	save_pointer(NAME(m_fm_level), 2);
	save_pointer(NAME(m_cd_level), 2);
	save_pointer(NAME(m_linein_level), 2);
	save_item(NAME(m_mic_level));
	save_item(NAME(m_pc_speaker_level));
	save_item(NAME(m_output_sw));
	save_pointer(NAME(m_input_sw), 2);
	save_pointer(NAME(m_input_gain), 2);
	save_pointer(NAME(m_output_gain), 2);
	save_item(NAME(m_agc));
	save_pointer(NAME(m_treble), 2);
	save_pointer(NAME(m_bass), 2);
}

void ct1745_mixer_device::device_reset()
{
	m_index = 0;

	reset_state();
}

void ct1745_mixer_device::reset_state()
{
	m_master_level[0] = m_master_level[1] = 0xc0;
	m_dac_level[0] = m_dac_level[1] = 0xc0;
	m_fm_level[0] = m_fm_level[1] = 0xc0;
	m_cd_level[0] = m_cd_level[1] = 0xc0;
	m_linein_level[0] = m_linein_level[1] = 0;
	m_mic_level = 0;
	m_pc_speaker_level = 0x80;
	m_output_sw = 0x1f;
	m_input_sw[0] = 0x15;
	m_input_sw[1] = 0x0b;
	m_input_gain[0] = m_input_gain[1] = 0x00;
	m_output_gain[0] = m_output_gain[1] = 0x00;
	m_treble[0] = m_treble[1] = 0x80;
	m_bass[0] = m_bass[1] = 0x80;
	m_agc = false;

	//m_dac_speaker_off = true;
	update_gain_levels();
}

void ct1745_mixer_device::dac_speaker_off_cb(int state)
{
	m_dac_speaker_off = !!state;
	update_gain_levels();
}

u8 ct1745_mixer_device::read(offs_t offset)
{
	if (offset == 0)
		return m_index;

	return space().read_byte(m_index);
}

void ct1745_mixer_device::write(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		m_index = data;
		return;
	}

	space().write_byte(m_index, data);
	if (!BIT(m_index, 7))
		update_gain_levels();
}

// TODO: bare copy of HLE version, convert to sound_stream_update instead
void ct1745_mixer_device::update_gain_levels()
{
	float lmain = m_master_level[0]/248.0;
	float rmain = m_master_level[1]/248.0;

	m_ldac->set_output_gain(ALL_OUTPUTS, m_dac_speaker_off ? 0.0 : lmain*(m_dac_level[0]/248.0f));
	m_rdac->set_output_gain(ALL_OUTPUTS, m_dac_speaker_off ? 0.0 : rmain*(m_dac_level[1]/248.0f));
	m_fm->set_output_gain(0, lmain*(m_fm_level[0]/248.0f));
	m_fm->set_output_gain(1, rmain*(m_fm_level[1]/248.0f));
	m_fm->set_output_gain(2, lmain*(m_fm_level[0]/248.0f));
	m_fm->set_output_gain(3, rmain*(m_fm_level[1]/248.0f));

	// TODO: convert to LIVE_AUDIO_VIEW
	//popmessage("%f %f (%02x %02x) | %02x %02x | %02x %02x", lmain, rmain, m_master_level[0], m_master_level[1], m_dac_level[0], m_dac_level[1], m_fm_level[0], m_fm_level[1]);
}

void ct1745_mixer_device::map(address_map &map)
{
	map(0x00, 0x00).lw8(
		NAME([this] (offs_t offset, u8 data) {
			// TODO: requires that the index is loaded by a set time first.
			if (!BIT(data, 0))
				reset_state();
		})
	);
// SB1/2 compatibility section
	map(0x04, 0x04).lrw8(
		NAME([this] (offs_t offset) {
			return (m_dac_level[0] & 0xf0) | (m_dac_level[1] >> 4);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_dac_level[0] = (data & 0xf0) | 8;
			m_dac_level[1] = ((data & 0x0f) << 4) | 8;
		})
	);

//  map(0x06, 0x06) FM Output Control?
//  map(0x0a, 0x0a) Microphone level?
//  map(0x0c, 0x0c) Input/Filter Select
	// RMW (to the index!?) by ibm5170_cdrom:zyclunt, should be "Output Filter/Stereo Select"
//  map(0x0e, 0x0e).lr8(NAME([] () { return 0x02; })); // Output/Stereo Select
	map(0x22, 0x22).lrw8(
		NAME([this] (offs_t offset) {
			return (m_master_level[0] & 0xf0) | (m_master_level[1] >> 4);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_master_level[0] = (data & 0xf0) | 8;
			m_master_level[1] = ((data & 0x0f) << 4) | 8;
		})
	);
	map(0x26, 0x26).lrw8(
		NAME([this] (offs_t offset) {
			return (m_fm_level[0] & 0xf0) | (m_fm_level[1] >> 4);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_fm_level[0] = (data & 0xf0) | 8;
			m_fm_level[1] = ((data & 0x0f) << 4) | 8;
		})
	);
	map(0x26, 0x26).lrw8(
		NAME([this] (offs_t offset) {
			return (m_cd_level[0] & 0xf0) | (m_cd_level[1] >> 4);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_cd_level[0] = (data & 0xf0) | 8;
			m_cd_level[1] = ((data & 0x0f) << 4) | 8;
		})
	);
	map(0x2e, 0x2e).lrw8(
		NAME([this] (offs_t offset) {
			return (m_linein_level[0] & 0xf0) | (m_linein_level[1] >> 4);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_linein_level[0] = (data & 0xf0) | 8;
			m_linein_level[1] = ((data & 0x0f) << 4) | 8;
		})
	);
	map(0x30, 0x31).lrw8(
		NAME([this] (offs_t offset) {
			return m_master_level[offset & 1];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_master_level[offset & 1] = data & 0xf8;
		})
	);
	map(0x32, 0x33).lrw8(
		NAME([this] (offs_t offset) {
			return m_dac_level[offset & 1];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_dac_level[offset & 1] = data & 0xf8;
		})
	);
	map(0x34, 0x35).lrw8(
		NAME([this] (offs_t offset) {
			return m_fm_level[offset & 1];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_fm_level[offset & 1] = data & 0xf8;
		})
	);
	map(0x36, 0x37).lrw8(
		NAME([this] (offs_t offset) {
			return m_cd_level[offset & 1];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_cd_level[offset & 1] = data & 0xf8;
		})
	);
	map(0x38, 0x39).lrw8(
		NAME([this] (offs_t offset) {
			return m_linein_level[offset & 1];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_linein_level[offset & 1] = data & 0xf8;
		})
	);
	map(0x3a, 0x3a).lrw8(
		NAME([this] (offs_t offset) {
			return m_mic_level;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_mic_level = data & 0xf8;
		})
	);
	map(0x3b, 0x3b).lrw8(
		NAME([this] (offs_t offset) {
			return m_pc_speaker_level;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_pc_speaker_level = data & 0xc0;
		})
	);
	map(0x3c, 0x3c).lrw8(
		NAME([this] (offs_t offset) {
			return m_output_sw;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_output_sw = data & 0x1f;
		})
	);
	// NOTE: start section where L/R are A0 swapped ...
	map(0x3d, 0x3e).lrw8(
		NAME([this] (offs_t offset) {
			return m_input_sw[(offset & 1) ^ 1];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_input_sw[(offset & 1) ^ 1] = data & 0x7f;
		})
	);
	map(0x3f, 0x40).lrw8(
		NAME([this] (offs_t offset) {
			return m_input_gain[(offset & 1) ^ 1];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_input_gain[(offset & 1) ^ 1] = data & 0xc0;
		})
	);
	map(0x41, 0x42).lrw8(
		NAME([this] (offs_t offset) {
			return m_output_gain[(offset & 1) ^ 1];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_output_gain[(offset & 1) ^ 1] = data & 0xc0;
		})
	);
	// ... end section
	map(0x43, 0x43).lrw8(
		NAME([this] (offs_t offset) {
			return m_agc;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_agc = !!BIT(data, 0);
		})
	);
	map(0x44, 0x45).lrw8(
		NAME([this] (offs_t offset) {
			return m_treble[offset & 1];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_treble[offset & 1] = data & 0xf0;
		})
	);
	map(0x46, 0x47).lrw8(
		NAME([this] (offs_t offset) {
			return m_bass[offset & 1];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_bass[offset & 1] = data & 0xf0;
		})
	);

	// PnP ports
	// IRQ/DMA select
	map(0x80, 0x80).lr8(NAME([] () { return 0x12; }));
	map(0x81, 0x81).lr8(NAME([] () { return 0x22; }));
	// IRQ Status
	map(0x82, 0x82).lr8(NAME([this] () { return m_irq_status_cb(); }));

	// ...
}
