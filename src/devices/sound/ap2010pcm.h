// license:BSD-3-Clause
// copyright-holders:QUFB
/**********************************************************************

    PCM audio functions of the AP2010 LSI

    According to the Advanced Pico Beena's 2005-04-05 press release, it supports
    CELP with sample rates 8kHz and 16kHz. It is used as output for OGG files,
    which are decoded by the BIOS to signed 16-bit big endian PCM.

**********************************************************************/

#ifndef MAME_SOUND_AP2010PCM_H
#define MAME_SOUND_AP2010PCM_H

#pragma once

class ap2010pcm_device : public device_t, public device_sound_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	ap2010pcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint32_t reg_r(offs_t offset);
	void reg_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	// FIXME: Games check this against 0x1ff, but samples are lost with that limit
	static inline constexpr uint16_t FIFO_MAX_SIZE = 0x800;

	uint16_t fifo_pop();
	uint16_t fifo_fast_pop();
	void fifo_push(uint16_t sample);
	void fifo_fast_push(uint16_t sample);

	std::unique_ptr<uint32_t[]> m_regs;

	float m_volume;

	uint32_t m_sample_rate;

	uint16_t m_fifo_data[FIFO_MAX_SIZE];
	uint16_t m_fifo_size;
	uint16_t m_fifo_head;
	uint16_t m_fifo_tail;

	uint16_t m_fifo_fast_data[FIFO_MAX_SIZE];
	uint16_t m_fifo_fast_size;
	uint16_t m_fifo_fast_head;
	uint16_t m_fifo_fast_tail;

	sound_stream *m_stream;
};

DECLARE_DEVICE_TYPE(AP2010PCM, ap2010pcm_device)

#endif // MAME_SOUND_AP2010PCM_H
