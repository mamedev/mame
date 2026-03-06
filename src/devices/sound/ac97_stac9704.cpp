// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "ac97_stac9704.h"

#include "speaker.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"


DEFINE_DEVICE_TYPE(AC97_STAC9704, ac97_stac9704_device, "ac97_stac9704", "SigmaTel STAC9704 AC'97 Codec")

ac97_stac9704_device::ac97_stac9704_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_pcm(*this, finder_base::DUMMY_TAG)
{
}

ac97_stac9704_device::ac97_stac9704_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ac97_stac9704_device(mconfig, AC97_STAC9704, tag, owner, clock)
{
	m_space_config = address_space_config("ac97_regs", ENDIANNESS_LITTLE, 16, 7, -1, address_map_constructor(FUNC(ac97_stac9704_device::mixer_map), this));
	m_vendor_id1 = 0x8384;
	m_vendor_id2 = 0x7604;
}

device_memory_interface::space_config_vector ac97_stac9704_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void ac97_stac9704_device::device_start()
{
	save_item(NAME(m_index_r));
	save_item(NAME(m_index_w));
	save_item(NAME(m_data_w));

	save_item(NAME(m_master_vol));
	save_item(NAME(m_lnlvl_vol));
	save_item(NAME(m_master_mono_vol));
	save_item(NAME(m_pc_beep_vol));
	save_item(NAME(m_phone_vol));
	save_item(NAME(m_mic_vol));
	save_item(NAME(m_linein_vol));
	save_item(NAME(m_cd_vol));
	save_item(NAME(m_video_vol));
	save_item(NAME(m_aux_vol));
	save_item(NAME(m_pcm_out_vol));
	save_item(NAME(m_record_sel)),
	save_item(NAME(m_record_gain));
	save_item(NAME(m_general_purpose));
	save_item(NAME(m_3d_control));
	save_item(NAME(m_power_ctrl));
}

void ac97_stac9704_device::device_reset()
{
	// undefined for index and data interface

	m_master_vol = 0x8000;
	m_lnlvl_vol = 0x8000;
	m_master_mono_vol = 0x8000;
	m_pc_beep_vol = 0;
	m_phone_vol = 0x8008;
	m_mic_vol = 0x8008;
	m_linein_vol = 0x8808;
	m_cd_vol = 0x8808;
	m_video_vol = 0x8808;
	m_aux_vol = 0x8808;
	m_pcm_out_vol = 0x8808;
	m_record_sel = 0,
	m_record_gain = 0x8000;
	m_general_purpose = 0;
	m_3d_control = 0;
	m_power_ctrl = 0x000f;

	update_gain_levels();
}

// TODO: the whole thing is asynchronous, bit 15 goes high if the codec is busy
u32 ac97_stac9704_device::codec_write_r(offs_t offset, u32 mem_mask)
{
	return (m_index_w) | (m_data_w << 16) | (0 << 15);
}

void ac97_stac9704_device::codec_write_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_16_31)
		m_data_w = data >> 16;

	if (BIT(data, 15) && ACCESSING_BITS_0_15)
	{
		m_index_w = data & 0x7f;
		space().write_word(m_index_w, m_data_w);
	}
}

u32 ac97_stac9704_device::codec_read_r(offs_t offset, u32 mem_mask)
{
	return (m_index_r) | (space().read_word(m_index_r) << 16) | (0 << 15);
}

void ac97_stac9704_device::codec_read_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (BIT(data, 15) && ACCESSING_BITS_0_15)
	{
		m_index_r = data & 0x7f;
	}
}


void ac97_stac9704_device::mixer_map(address_map &map)
{
	map(0x00, 0x00).lrw16(
		NAME([this] (offs_t offset) {
			if (!machine().side_effects_disabled())
				LOG("REG #00: Reset read ID\n");
			// TODO: unverified
			// just bit 15 known to be low
			return 0x1704;
		}),
		NAME([this] (offs_t offset, u16 data) {
			(void)data;
			LOG("REG #00: Reset issued\n");
			this->reset();
		})
	);
	map(0x02, 0x02).lrw16(
		NAME([this] (offs_t offset) {
			return m_master_vol;
		}),
		NAME([this] (offs_t offset, u16 data) {
			LOG("REG #02: Master Volume %04x\n", data);
			m_master_vol = data & 0x9f1f;
			update_gain_levels();
		})
	);
	map(0x04, 0x04).lrw16(
		NAME([this] (offs_t offset) {
			return m_lnlvl_vol;
		}),
		NAME([this] (offs_t offset, u16 data) {
			LOG("REG #04: LNLVL Volume %04x\n", data);
			m_lnlvl_vol = data & 0x9f1f;
		})
	);
	map(0x06, 0x06).lrw16(
		NAME([this] (offs_t offset) {
			return m_master_mono_vol;
		}),
		NAME([this] (offs_t offset, u16 data) {
			LOG("REG #06: Master Volume Mono %04x\n", data);
			m_master_mono_vol = data & 0x901f;
		})
	);
	map(0x0a, 0x0a).lrw16(
		NAME([this] (offs_t offset) {
			return m_pc_beep_vol;
		}),
		NAME([this] (offs_t offset, u16 data) {
			LOG("REG #0A: PC_BEEP Volume %04x\n", data);
			m_pc_beep_vol = data & 0x801e;
		})
	);
	map(0x0c, 0x0c).lrw16(
		NAME([this] (offs_t offset) {
			return m_phone_vol;
		}),
		NAME([this] (offs_t offset, u16 data) {
			LOG("REG #0C: Phone Volume %04x\n", data);
			m_phone_vol = data & 0x801f;
		})
	);
	map(0x0e, 0x0e).lrw16(
		NAME([this] (offs_t offset) {
			return m_mic_vol;
		}),
		NAME([this] (offs_t offset, u16 data) {
			LOG("REG #0E: Mic Volume %04x\n", data);
			m_mic_vol = data & 0x805f;
		})
	);
	map(0x10, 0x10).lrw16(
		NAME([this] (offs_t offset) {
			return m_linein_vol;
		}),
		NAME([this] (offs_t offset, u16 data) {
			LOG("REG #10: Line In Volume %04x\n", data);
			m_linein_vol = data & 0x9f1f;
		})
	);
	map(0x12, 0x12).lrw16(
		NAME([this] (offs_t offset) {
			return m_cd_vol;
		}),
		NAME([this] (offs_t offset, u16 data) {
			LOG("REG #12: CD Volume %04x\n", data);
			m_cd_vol = data & 0x9f1f;
		})
	);
	map(0x14, 0x14).lrw16(
		NAME([this] (offs_t offset) {
			return m_video_vol;
		}),
		NAME([this] (offs_t offset, u16 data) {
			LOG("REG #14: Video Volume %04x\n", data);
			m_video_vol = data & 0x9f1f;
		})
	);
	map(0x16, 0x16).lrw16(
		NAME([this] (offs_t offset) {
			return m_aux_vol;
		}),
		NAME([this] (offs_t offset, u16 data) {
			LOG("REG #16: AUX Volume %04x\n", data);
			m_aux_vol = data & 0x9f1f;
		})
	);
	map(0x18, 0x18).lrw16(
		NAME([this] (offs_t offset) {
			return m_pcm_out_vol;
		}),
		NAME([this] (offs_t offset, u16 data) {
			LOG("REG #18: PCM Out Volume %04x\n", data);
			m_pcm_out_vol = data & 0x9f1f;
			update_gain_levels();
		})
	);
	map(0x1a, 0x1a).lrw16(
		NAME([this] (offs_t offset) {
			return m_record_sel;
		}),
		NAME([this] (offs_t offset, u16 data) {
			LOG("REG #1A: Record Select %04x\n", data);
			m_record_sel = data & 0x0707;
		})
	);
	map(0x1c, 0x1c).lrw16(
		NAME([this] (offs_t offset) {
			return m_record_gain;
		}),
		NAME([this] (offs_t offset, u16 data) {
			LOG("REG #1C: Record Gain %04x\n", data);
			m_record_gain = data & 0x8f0f;
		})
	);
	map(0x20, 0x20).lrw16(
		NAME([this] (offs_t offset) {
			return m_general_purpose;
		}),
		NAME([this] (offs_t offset, u16 data) {
			LOG("REG #20: General Purpose %04x\n", data);
			m_general_purpose = data & 0x2380;
		})
	);
	map(0x22, 0x22).lrw16(
		NAME([this] (offs_t offset) {
			return m_3d_control;
		}),
		NAME([this] (offs_t offset, u16 data) {
			// under AC97 3D Stereo Enhancement of PCI Audio Properties
			LOG("REG #22: 3D Control %04x\n", data);
			m_3d_control = data & 0x0003;
		})
	);
	map(0x26, 0x26).lrw16(
		NAME([this] (offs_t offset) {
			// make REF / ANL / DAC and ADC always ready for now
			return m_power_ctrl | 0xf;
		}),
		NAME([this] (offs_t offset, u16 data) {
			LOG("REG #26: Powerdown Control %04x\n", data);
			m_power_ctrl = data & 0xff00;
		})
	);
	map(0x7c, 0x7c).lr16(NAME([this] () {
		if (!machine().side_effects_disabled()) { LOG("REG #7C: Vendor ID1 read\n"); } return m_vendor_id1; }));
	map(0x7e, 0x7e).lr16(NAME([this] () {
		if (!machine().side_effects_disabled()) { LOG("REG #7E: Vendor ID2 read\n"); } return m_vendor_id2; }));
}

void ac97_stac9704_device::update_gain_levels()
{
	float mainL = (0x1f - ((m_master_vol & 0x1f00) >> 8)) / 31.0;
	float mainR = (0x1f - (m_master_vol & 0x1f)) / 31.0;
	bool global_mute = !!BIT(m_master_vol, 15);

	if (global_mute || BIT(m_pcm_out_vol, 15))
	{
		m_pcm->set_output_gain(0, 0.0);
		m_pcm->set_output_gain(1, 0.0);
	}
	else
	{
		float pcmL = (0x1f - ((m_pcm_out_vol & 0x1f00) >> 8)) / 31.0;
		float pcmR = (0x1f - (m_pcm_out_vol & 0x1f)) / 31.0;
		m_pcm->set_output_gain(0, mainL * pcmL);
		m_pcm->set_output_gain(1, mainR * pcmR);
	}
}
