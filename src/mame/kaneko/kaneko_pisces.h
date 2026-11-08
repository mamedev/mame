// license:BSD-3-Clause
// copyright-holders:Luca Elia, Andrea Bogazzi
#ifndef MAME_KANEKO_KANEKO_PISCES_H
#define MAME_KANEKO_KANEKO_PISCES_H

#pragma once

class kaneko_pisces_hle_device : public device_t
{
public:
	template <typename T, typename U, typename V, typename W>
	kaneko_pisces_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&maincpu_tag, U &&subcpu_tag, V &&subdata_tag, W &&bg15_tag)
		: kaneko_pisces_hle_device(mconfig, tag, owner, uint32_t(0))
	{
		m_maincpu.set_tag(std::forward<T>(maincpu_tag));
		m_subcpu.set_tag(std::forward<U>(subcpu_tag));
		m_subdata.set_tag(std::forward<V>(subdata_tag));
		m_bg15.set_tag(std::forward<W>(bg15_tag));
	}

	kaneko_pisces_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_special()
	{
		assert(!configured());
		// SE reuses the lookup word for decoding.
		m_bg_decode_param_offset = 0;
		m_field_origin_x = 32;
	}

	// HLE decoder context: address of the task currently writing BG15.
	auto decoder_task_callback() { return m_decoder_task_cb.bind(); }

	void init_w(uint8_t data);
	void main_nmi_w(uint8_t data);
	void sub_nmi_w(uint8_t data);

	// Called at the start of vblank, before the sub CPU's level 5 interrupt.
	void vblank();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr uint32_t GIRL_BITMAP_OFFSET = 0xe000;
	static constexpr uint32_t GIRL_BITMAP_ADDR = 0x10e000;
	static constexpr uint32_t BG15_COVERAGE_SIZE = 0x2000;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_memory_region m_subdata;
	required_shared_ptr<uint16_t> m_bg15;
	devcb_read32 m_decoder_task_cb;

	uint16_t m_old_mcu_nmi1;
	uint16_t m_old_mcu_nmi2;

	// Immutable ROM lookup metadata and per-variant protocol/layout settings.
	std::vector<uint32_t> m_bg_image_offsets;
	uint8_t m_bg_decode_param_offset;
	int m_field_origin_x;

	uint32_t m_girl_bitmap_pending_addr;
	uint32_t m_girl_bitmap_pending_param;
	uint32_t m_girl_bitmap_pending_task;
	std::unique_ptr<uint8_t []> m_bg15_coverage;

	void scan_background_images();
	void write_kaneko();
	void process_main_queue();
	void process_sub_queue();
	void track_bg15_coverage(offs_t address, uint16_t data, uint16_t mem_mask);
	uint16_t generate_girl_bitmap(uint32_t dest_addr);
};

DECLARE_DEVICE_TYPE(KANEKO_PISCES_HLE, kaneko_pisces_hle_device)

#endif // MAME_KANEKO_KANEKO_PISCES_H
