// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
#ifndef MAME_VIDEO_HANTRO_G1_H
#define MAME_VIDEO_HANTRO_G1_H
#pragma once

class ISVCDecoder;

class hantro_g1_device : public device_t
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	hantro_g1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_dma_space(T &&tag, int space) { m_dma.set_tag(std::forward<T>(tag), space); }
	auto irq_callback() { return m_irq.bind(); }

	void map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

private:
	static constexpr u32 HISTORY_SIZE = 16 * 1024 * 1024;

	u32 read(offs_t offset);
	void write(offs_t offset, u32 data, u32 mem_mask = ~0U);
	void update_irq();
	void release_decoder();
	bool reset_decoder();
	bool decode_packet(u8 const *data, u32 size, u8 **planes, u32 &width, u32 &height, u32 *strides);
	bool post_process(u8 const *const *planes = nullptr, u32 width = 0, u32 height = 0, u32 const *strides = nullptr);
	TIMER_CALLBACK_MEMBER(decode_done);
	TIMER_CALLBACK_MEMBER(pp_done);

	required_address_space m_dma;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_dma_access;
	devcb_write_line m_irq;
	emu_timer *m_decode_timer, *m_pp_timer;
	bool m_irq_state;
	u32 m_regs[256];
	// Replay data will reconstruct the host H.264 reference pictures on load.
	std::unique_ptr<u8[]> m_history;
	u32 m_history_size;
	u32 m_history_units;
	u32 m_jobs;
	ISVCDecoder *m_decoder;
};
DECLARE_DEVICE_TYPE(HANTRO_G1, hantro_g1_device)

#endif // MAME_VIDEO_HANTRO_G1_H
