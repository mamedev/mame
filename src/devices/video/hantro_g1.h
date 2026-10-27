// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
#ifndef MAME_VIDEO_HANTRO_G1_H
#define MAME_VIDEO_HANTRO_G1_H
#pragma once

class ISVCDecoder;

class hantro_g1_device : public device_t
{
public:
	hantro_g1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_dma_space(T &&tag, int space) { m_dma.set_tag(std::forward<T>(tag), space); }
	auto irq_callback() { return m_irq.bind(); }

	void map(address_map &map);

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void device_stop() override ATTR_COLD;

private:
	u32 read(offs_t offset);
	void write(offs_t offset, u32 data, u32 mem_mask = ~0U);
	void update_irq();
	void postload();
	bool reset_decoder();
	bool decode_packet(u8 const *data, u32 size, u8 **planes, unsigned &width, unsigned &height, unsigned *strides);
	bool post_process(u8 const *const *planes = nullptr, unsigned width = 0, unsigned height = 0, unsigned const *strides = nullptr);
	TIMER_CALLBACK_MEMBER(decode_done);
	TIMER_CALLBACK_MEMBER(pp_done);
	required_address_space m_dma;
	devcb_write_line m_irq;
	emu_timer *m_decode_timer = nullptr, *m_pp_timer = nullptr;
	u32 m_regs[256]{};
	// Replay data will reconstruct the host H.264 reference pictures on load.
	std::unique_ptr<u8[]> m_history;
	u32 m_history_size = 0, m_history_units = 0, m_jobs = 0;
	ISVCDecoder *m_decoder = nullptr;
};
DECLARE_DEVICE_TYPE(HANTRO_G1, hantro_g1_device)
#endif
