// license:BSD-3-Clause
// copyright-holders:Ville Linde

#pragma once
#ifndef __K057714_H__
#define __K057714_H__

class k057714_device : public device_t
{
public:
	k057714_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	template<class _Object> static devcb_base &static_set_irq_callback(device_t &device, _Object object) { return downcast<k057714_device &>(device).m_irq.set_callback(object); }

	int draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ32_MEMBER(read);
	DECLARE_WRITE32_MEMBER(write);
	DECLARE_WRITE32_MEMBER(fifo_w);

	struct framebuffer
	{
		UINT32 base;
		int width;
		int height;
	};

protected:
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();

private:
	void execute_command(UINT32 *cmd);
	void execute_display_list(UINT32 addr);
	void draw_object(UINT32 *cmd);
	void fill_rect(UINT32 *cmd);
	void draw_character(UINT32 *cmd);
	void fb_config(UINT32 *cmd);

	UINT32 *m_vram;
	UINT32 m_vram_read_addr;
	UINT32 m_vram_fifo0_addr;
	UINT32 m_vram_fifo1_addr;
	UINT32 m_vram_fifo0_mode;
	UINT32 m_vram_fifo1_mode;
	UINT32 m_command_fifo0[4];
	UINT32 m_command_fifo0_ptr;
	UINT32 m_command_fifo1[4];
	UINT32 m_command_fifo1_ptr;
	UINT32 m_ext_fifo_addr;
	UINT32 m_ext_fifo_count;
	UINT32 m_ext_fifo_line;
	UINT32 m_ext_fifo_num_lines;
	UINT32 m_ext_fifo_width;

	framebuffer m_frame[4];
	UINT32 m_fb_origin_x;
	UINT32 m_fb_origin_y;

	devcb_write_line m_irq;
};

extern const device_type K057714;

#define MCFG_K057714_IRQ_CALLBACK(_devcb) \
	devcb = &k057714_device::static_set_irq_callback(*device, DEVCB_##_devcb);


#endif
