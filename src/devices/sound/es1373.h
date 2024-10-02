// license:BSD-3-Clause
// copyright-holders:Ted Green
// Creative Labs Ensonic AudioPCI97 ES1373
#ifndef MAME_SOUND_ES1373_H
#define MAME_SOUND_ES1373_H

#pragma once

#include "machine/pci.h"

class es1373_device : public pci_device, public device_sound_interface
{
public:
	es1373_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	auto irq_handler() { return m_irq_handler.bind(); }

	uint32_t reg_r(offs_t offset, uint32_t mem_mask = ~0);
	void reg_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	TIMER_CALLBACK_MEMBER(delayed_stream_update);

	// Sound stream
	sound_stream *m_stream;

private:
	struct chan_info {
		int number;
		bool enable;
		bool int_en;
		bool loop_en;
		bool initialized;
		uint8_t  format;       // Format of channel
		uint32_t buf_wptr;     // Address to sample cache memory
		uint32_t buf_rptr;     // Address to sample cache memory
		uint16_t buf_count;    // Number of samples that have been played
		uint16_t buf_size;     // Number of samples minus one to play
		uint32_t pci_addr;     // PCI Address for system memory accesses
		uint16_t pci_count;    // Number of 32 bits transferred
		uint16_t pci_size;     // Total number of words (32 bits) minus one in system memory
	};

	void transfer_pci_audio(chan_info& chan, int type);
	uint32_t calc_size(const uint8_t &format);
	void send_audio_out(chan_info& chan, uint32_t intr_mask, write_stream_view &outL, write_stream_view &outR);

	emu_timer *m_timer;
	address_space *m_memory_space;
	devcb_write_line m_irq_handler;
	int m_irq_num;
	void map(address_map &map) ATTR_COLD;
	uint16_t m_ac97_regs[0x80];
	uint32_t m_es_regs[0x10];
	uint32_t m_sound_cache[0x40];
	uint16_t m_src_ram[0x80];
	chan_info m_dac1;
	chan_info m_dac2;
	chan_info m_adc;
};

DECLARE_DEVICE_TYPE(ES1373, es1373_device)

#endif // MAME_SOUND_ES1373_H
