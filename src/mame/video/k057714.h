// license:BSD-3-Clause
// copyright-holders:Ville Linde

#pragma once
#ifndef __K057714_H__
#define __K057714_H__

class k057714_device : public device_t
{
public:
	k057714_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template<class _Object> static devcb_base &static_set_irq_callback(device_t &device, _Object object) { return downcast<k057714_device &>(device).m_irq.set_callback(object); }

	int draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint32_t read(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void write(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void fifo_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	struct framebuffer
	{
		uint32_t base;
		int width;
		int height;
	};

protected:
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

private:
	void execute_command(uint32_t *cmd);
	void execute_display_list(uint32_t addr);
	void draw_object(uint32_t *cmd);
	void fill_rect(uint32_t *cmd);
	void draw_character(uint32_t *cmd);
	void fb_config(uint32_t *cmd);

	std::unique_ptr<uint32_t[]> m_vram;
	uint32_t m_vram_read_addr;
	uint32_t m_vram_fifo0_addr;
	uint32_t m_vram_fifo1_addr;
	uint32_t m_vram_fifo0_mode;
	uint32_t m_vram_fifo1_mode;
	uint32_t m_command_fifo0[4];
	uint32_t m_command_fifo0_ptr;
	uint32_t m_command_fifo1[4];
	uint32_t m_command_fifo1_ptr;
	uint32_t m_ext_fifo_addr;
	uint32_t m_ext_fifo_count;
	uint32_t m_ext_fifo_line;
	uint32_t m_ext_fifo_num_lines;
	uint32_t m_ext_fifo_width;

	framebuffer m_frame[4];
	uint32_t m_fb_origin_x;
	uint32_t m_fb_origin_y;

	devcb_write_line m_irq;
};

extern const device_type K057714;

#define MCFG_K057714_IRQ_CALLBACK(_devcb) \
	devcb = &k057714_device::static_set_irq_callback(*device, DEVCB_##_devcb);


#endif
