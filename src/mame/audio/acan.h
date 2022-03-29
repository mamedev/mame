// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, superctr
/**********************************************************************

    Super A'Can sound driver

**********************************************************************/

#ifndef MAME_AUDIO_ACAN_H
#define MAME_AUDIO_ACAN_H

#pragma once

class acan_sound_device : public device_t, public device_sound_interface
{
public:
	acan_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto ram_read() { return m_ram_read.bind(); }
	auto timer_irq_handler() { return m_timer_irq_handler.bind(); }
	auto dma_irq_handler() { return m_dma_irq_handler.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	struct acan_channel
	{
		uint16_t pitch;
		uint16_t length;
		uint16_t start_addr;
		uint16_t curr_addr;
		uint16_t end_addr;
		uint32_t addr_increment;
		uint32_t frac;
		uint8_t  register9;
		uint8_t  envelope[4];
		uint8_t  volume;
		uint8_t  volume_l;
		uint8_t  volume_r;
		bool     one_shot;
	};

	void keyon_voice(uint8_t voice);

	sound_stream *m_stream;
	emu_timer *m_timer;
	devcb_write_line m_timer_irq_handler;
	devcb_write_line m_dma_irq_handler;
	devcb_read8 m_ram_read;
	uint16_t m_active_channels;
	uint16_t m_dma_channels;
	acan_channel m_channels[16];
	uint8_t m_regs[256];
	std::unique_ptr<int32_t[]> m_mix;
};

DECLARE_DEVICE_TYPE(ACANSND, acan_sound_device)

#endif // MAME_AUDIO_ACAN_H
