// license:BSD-3-Clause
// copyright-holders:Ville Linde

#pragma once
#ifndef __K057714_H__
#define __K057714_H__

class k057714_device : public device_t
{
public:
	k057714_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	static void static_set_cpu_tag(device_t &device, const char *tag) { downcast<k057714_device &>(device).m_cputag = tag; }

	int draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ32_MEMBER(read);
	DECLARE_WRITE32_MEMBER(write);

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

	const char* m_cputag;
	device_t* m_cpu;

	framebuffer m_frame[4];
	UINT32 m_fb_origin_x;
	UINT32 m_fb_origin_y;
};

extern const device_type K057714;

#define MCFG_K057714_CPU_TAG(_tag) \
	k057714_device::static_set_cpu_tag(*device, _tag);

#endif
