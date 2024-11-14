// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi
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

//  void set_base(int8_t* base) { m_base = base; }

	void l7a1045_sound_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t l7a1045_sound_r(offs_t offset, uint16_t mem_mask = ~0);

	uint8_t dma_r_cb(offs_t offset);
	void dma_w_cb(offs_t offset, uint8_t data);
	uint16_t dma_r16_cb() { m_voice[0].pos++; return 0; }
	void dma_w16_cb(uint16_t data) { m_voice[0].pos++; }
	void dma_hreq_cb(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	struct l7a1045_voice
	{
		constexpr l7a1045_voice() { }

		uint32_t start = 0;
		uint32_t end = 0;
		bool mode = false;
		uint32_t pos = 0;
		uint32_t frac = 0;
		uint16_t l_volume = 0;
		uint16_t r_volume = 0;
	};

	sound_stream *m_stream;
	l7a1045_voice m_voice[32];
	uint32_t    m_key;
	required_region_ptr<uint8_t> m_rom;

	uint8_t m_audiochannel;
	uint8_t m_audioregister;

	struct l7a1045_48bit_data {
		uint16_t dat[3];
	};

	l7a1045_48bit_data m_audiodat[0x10][0x20];

	void sound_select_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_data_w(offs_t offset, uint16_t data);
	uint16_t sound_data_r(offs_t offset);
	void sound_status_w(uint16_t data);
};

DECLARE_DEVICE_TYPE(L7A1045, l7a1045_sound_device)

#endif // MAME_SOUND_L7A1045_L6028_DSP_A_H
