// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, superctr
/**********************************************************************

    Super A'Can sound driver

**********************************************************************/

#ifndef MAME_FUNTECH_UM6619_SOUND_H
#define MAME_FUNTECH_UM6619_SOUND_H

#pragma once

class umc6619_sound_device : public device_t, public device_sound_interface
{
public:
	umc6619_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto ram_read() { return m_ram_read.bind(); }
	auto timer_irq_handler() { return m_timer_irq_handler.bind(); }
	auto dma_irq_handler() { return m_dma_irq_handler.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	TIMER_CALLBACK_MEMBER(channel_irq);

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
		uint8_t  unk_upper_05;
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

	std::string print_audio_state();
};

DECLARE_DEVICE_TYPE(UMC6619_SOUND, umc6619_sound_device)

#endif // MAME_FUNTECH_UM6619_SOUND_H
