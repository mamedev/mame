// license:BSD-3-Clause
// copyright-holders:ElSemi
#ifndef MAME_SOUND_VRENDER0_H
#define MAME_SOUND_VRENDER0_H

#pragma once

#include <algorithm>

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> vr0sound_device

class vr0sound_device : public device_t,
						public device_sound_interface,
						public device_memory_interface
{
public:
	enum
	{
		AS_TEXTURE = 0,
		AS_FRAME
	};

	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	vr0sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto irq_callback() { return m_irq_cb.bind(); }

	u16 channel_r(offs_t offset);
	void channel_w(offs_t offset, u16 data, u16 mem_mask);

	u16 status_r(offs_t offset);
	void status_w(offs_t offset, u16 data);

	u16 noteon_r(offs_t offset);
	void noteon_w(offs_t offset, u16 data);

	u16 revfactor_r(offs_t offset);
	void revfactor_w(offs_t offset, u16 data, u16 mem_mask);

	u16 buffersaddr_r(offs_t offset);
	void buffersaddr_w(offs_t offset, u16 data, u16 mem_mask);

	u16 buffersize0_r(offs_t offset);
	void buffersize0_w(offs_t offset, u16 data, u16 mem_mask);

	u16 buffersize1_r(offs_t offset);
	void buffersize1_w(offs_t offset, u16 data, u16 mem_mask);

	u16 buffersize2_r(offs_t offset);
	void buffersize2_w(offs_t offset, u16 data, u16 mem_mask);

	u16 buffersize3_r(offs_t offset);
	void buffersize3_w(offs_t offset, u16 data, u16 mem_mask);

	u16 intmask_r(offs_t offset);
	void intmask_w(offs_t offset, u16 data, u16 mem_mask);

	u16 intpend_r(offs_t offset);
	void intpend_w(offs_t offset, u16 data, u16 mem_mask);

	u16 chnnum_r(offs_t offset);
	void chnnum_w(offs_t offset, u16 data, u16 mem_mask);

	u16 ctrl_r(offs_t offset);
	void ctrl_w(offs_t offset, u16 data, u16 mem_mask);

	void sound_map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override;

private:
	enum
	{
		MODE_LOOP = (1 << 0), // Loop Enable
		MODE_SUSTAIN = (1 << 1), // Run Sustain when Note Off (Not Implemented)
		MODE_ENVELOPE = (1 << 2), // Enable Envelope
		MODE_PINGPONG = (1 << 3), // Pingpong Loop (Not Implemented)
		MODE_ULAW = (1 << 4), // u-Law
		MODE_8BIT = (1 << 5), // 8 Bit (1) / 16 Bit (0) samples
		MODE_TEXTURE = (1 << 6) // Wave Source (1 = Texture memory, 0 = Frame memory)
	};

	enum
	{
		CTRL_RS = (1 << 15), // Enable Sound
		CTRL_TM = (1 << 5), // Texture Memory Select (1 = Texture memory, 0 = Frame memory)
		CTRL_RE = (1 << 4), // Reverb Enable (Not Implemented)
		CTRL_CW = (1 << 2), // 32bit Adder Wait (Not Implemented)
		CTRL_AW = (1 << 1), // 16bit Adder Wait (Not Implemented)
		CTRL_MW = (1 << 0) // Multiplier Wait (Not Implemented)
	};

	struct channel_t
	{
		memory_access<23, 1, 0, ENDIANNESS_LITTLE>::cache *cache;
		u32 cur_saddr = 0; // Current Address Pointer, 22.10 Fixed Point
		s32 env_vol = 0; // Envelope Volume (Overall Volume), S.7.16 Fixed Point
		u8 env_stage = 1; // Envelope Stage
		u16 ds_addr = 0; // Frequency, 6.10 Fixed Point
		u8 modes = 0; // modes
		bool ld = true; // Loop Direction (Not Implemented)
		u32 loop_begin = 0; // Loop Start Pointer, High 22 Bits
		u32 loop_end = 0; // Loop End Pointer, High 22 Bits
		u8 l_chn_vol = 0; // Left Volume, 7 bit unsigned
		u8 r_chn_vol = 0; // Right Volume, 7 bit unsigned
		s32 env_rate[4]{0}; // Envenloe Rate per Each stages, S.16 Fixed Point
		u8 env_target[4]{0}; // Envelope Target Volume per Each stages, High 7 Bits
		u16 read(offs_t offset);
		void write(offs_t offset, u16 data, u16 mem_mask);
	};

	address_space_config m_texture_config;
	address_space_config m_frame_config;

	memory_access<23, 1, 0, ENDIANNESS_LITTLE>::cache m_texcache;
	memory_access<23, 1, 0, ENDIANNESS_LITTLE>::cache m_fbcache;
	memory_access<23, 1, 0, ENDIANNESS_LITTLE>::cache *m_texcache_ctrl;

	channel_t m_channel[32];
	sound_stream *m_stream;
	devcb_write_line m_irq_cb;

	// Registers
	u32 m_status = 0; // Status (0 Idle, 1 Busy)
	u32 m_note_on = 0; // Note On (0 Off, 1 On) (Not Implemented)
	u8 m_rev_factor = 0; // Reverb Factor (Not Implemented)
	u32 m_buffer_addr = 0; // 21bit Reverb Buffer Start Address (Not Implemented)
	u16 m_buffer_size[4]{0}; // Reverb Buffer Size (Not Implemented)
	u32 m_int_mask = 0; // Interrupt Mask (0 Enable, 1 Disable)
	u32 m_int_pend = 0; // Interrupt Pending
	u8 m_max_chan = 0x1f; // Max Channels - 1
	u8 m_chan_clk_num = 0; // Clock Number per Channel
	u16 m_ctrl = 0; // 0x602 Control Functions
	void render_audio(sound_stream &stream);
};

DECLARE_DEVICE_TYPE(SOUND_VRENDER0, vr0sound_device)

#endif // MAME_SOUND_VRENDER0_H
