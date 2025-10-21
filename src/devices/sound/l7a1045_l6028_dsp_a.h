// license:BSD-3-Clause
// copyright-holders:R. Belmont, O. Galibert
#ifndef MAME_SOUND_L7A1045_L6028_DSP_A_H
#define MAME_SOUND_L7A1045_L6028_DSP_A_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> l7a1045_sound_device

class l7a1045_sound_device : public device_t,
							public device_sound_interface
{
public:
	l7a1045_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map);

	auto drq_handler_cb() { return m_drq_handler.bind(); }

	uint8_t dma_r_cb(offs_t offset);
	void dma_w_cb(offs_t offset, uint8_t data);
	uint16_t dma_r16_cb(offs_t offset, uint16_t mem_mask);
	void dma_w16_cb(offs_t offset, uint16_t data, uint16_t mem_mask);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static const int NUM_VOICES = 32;
	static const int REGS_PER_VOICE = 16;

	struct l7a1045_voice
	{
		constexpr l7a1045_voice() { }

		uint32_t loop_start = 0;
		uint32_t start = 0;
		uint32_t end = 0;
		uint32_t step = 0;
		uint32_t pos = 0;
		uint32_t frac = 0;
		uint16_t l_volume = 0;
		uint16_t r_volume = 0;
		uint16_t env_volume = 0;
		uint16_t env_target = 0;
		uint16_t env_step = 0;
		uint32_t env_pos = 0;
		uint16_t flt_freq = 0;
		uint16_t flt_target = 0;
		uint16_t flt_step = 0;
		uint32_t flt_pos = 0;
		uint8_t flt_resonance = 0;
		int32_t b = 0, l = 0; // filter state
		uint8_t send_dest = 0;
		uint8_t send_level = 0;
		uint8_t sample_type = 0; // 0 = 16-bit, 1 = 12-bit non-linear
	};

	devcb_write_line m_drq_handler;
	sound_stream    *m_stream;
	l7a1045_voice   m_voice[NUM_VOICES];
	uint32_t        m_key;
	required_region_ptr<uint8_t> m_rom;

	uint8_t m_cur_channel;
	uint8_t m_cur_register;
	double  m_sample_rate;

	uint64_t m_regs[REGS_PER_VOICE][NUM_VOICES];

	uint32_t m_ram_mask;

	void voice_select_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t voiceregs_r(offs_t offset);
	void voiceregs_w(offs_t offset, uint16_t data);
	uint16_t control_r();
	void control_w(uint16_t data);
	void atomic_w(uint16_t data);

	void recalc_loop_start(l7a1045_voice *vptr);
};

DECLARE_DEVICE_TYPE(L7A1045, l7a1045_sound_device)

#endif // MAME_SOUND_L7A1045_L6028_DSP_A_H
