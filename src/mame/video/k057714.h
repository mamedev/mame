// license:BSD-3-Clause
// copyright-holders:Ville Linde
#ifndef MAME_MACHINE_K057714_H
#define MAME_MACHINE_K057714_H

#pragma once


class k057714_device : public device_t
{
public:
	k057714_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	auto irq_callback() { return m_irq.bind(); }

	int draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ32_MEMBER(read);
	DECLARE_WRITE32_MEMBER(write);
	DECLARE_WRITE32_MEMBER(fifo_w);

	struct framebuffer
	{
		uint32_t base;
		int width;
		int height;
		int x;
		int y;
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

	void draw_frame(int frame, bitmap_ind16 &bitmap, const rectangle &cliprect, bool inverse_trans);

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
	uint32_t m_layer_select;
	uint32_t m_reg_6c;

	devcb_write_line m_irq;
};

DECLARE_DEVICE_TYPE(K057714, k057714_device)


#endif // MAME_MACHINE_K057714_H
