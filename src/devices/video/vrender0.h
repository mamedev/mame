// license:BSD-3-Clause
// copyright-holders:ElSemi, Angelo Salese
#ifndef MAME_VIDEO_VRENDER0_H
#define MAME_VIDEO_VRENDER0_H

#pragma once


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

class vr0video_device : public device_t,
						public device_video_interface,
						public device_memory_interface
{
public:
	enum
	{
		AS_TEXTURE = 0,
		AS_FRAME
	};

	vr0video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void execute_flipping();
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	auto idleskip_cb() { return m_idleskip_cb.bind(); }

	u16 flip_count_r();
	void flip_count_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void regs_map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override;

private:
	/*
	Pick a rare enough color to disable transparency (that way I save a cmp per loop to check
	if I must draw transparent or not. The palette build will take this color in account so
	no color in the palette will have this value
	*/
	static constexpr u16 NOTRANSCOLOR = 0xecda;

	struct quad_info
	{
		u32 dest = 0;
		u32 dx = 0, dy = 0;
		u32 endx = 0, endy = 0;
		u32 tx = 0;
		u32 ty = 0;
		u32 txdx = 0;
		u32 tydx = 0;
		u32 txdy = 0;
		u32 tydy = 0;
		u16 twidth = 0;
		u16 theight = 0;
		u32 texaddr = 0;
		u32 tile = 0;
		u16 *pal = nullptr;
		u32 trans_color = 0;
		u32 shade = 0;
		bool clamp = false;
		bool trans = false;
		u8 src_alpha = 0;
		u32 src_color = 0;
		u8 dst_alpha = 0;
		u32 dst_color = 0;
	};

	struct render_state_info
	{
		u32 tx = 0;
		u32 ty = 0;
		u32 txdx = 0;
		u32 tydx = 0;
		u32 txdy = 0;
		u32 tydy = 0;
		u32 src_alpha_color = 0;
		u32 src_blend = 0;
		u32 dst_alpha_color = 0;
		u32 dst_blend = 0;
		u32 shade_color = 0;
		u32 trans_color = 0;
		u32 tile_offset = 0;
		u32 font_offset = 0;
		u32 pal_offset = 0;
		u8 palette_bank = 0;
		bool texture_mode = false;
		u8 pixel_format = 0;
		u16 Width = 0;
		u16 Height = 0;
	};

	typedef void (vr0video_device::*_draw_template)(quad_info &);
	_draw_template m_draw_image[2][16];

	address_space_config m_texture_config;
	address_space_config m_frame_config;

	memory_access<23, 1, 0, ENDIANNESS_LITTLE>::cache m_texcache;
	memory_access<23, 1, 0, ENDIANNESS_LITTLE>::cache m_fbcache;
	memory_access<23, 1, 0, ENDIANNESS_LITTLE>::specific m_fbmem;

	devcb_write_line m_idleskip_cb;

	u16 m_internal_palette[256];
	u32 m_last_pal_update;

	render_state_info m_render_state;

	u16 m_queue_rear, m_queue_front;

	bool m_bank1_select;        // Select framebuffer bank1 address
	u8 m_display_bank;     // Current display bank

	bool m_draw_select;         // If true, device draws to Front buffer instead of Back
	bool m_render_reset;        // Reset pipeline FIFO
	bool m_render_start;        // Enable pipeline processing
	u8 m_dither_mode;      // applied on RGB888 to RGB565 conversions (00: 2x2, 01:4x4, 1x disable)
	u8 m_flip_count;       // number of framebuffer "syncs" loaded in the parameter RAM,
								// a.k.a. how many full (vblank) buffers are ready for the device to parse.

	u32 m_draw_dest;       // frameram pointer to draw buffer area
	u32 m_display_dest;    // frameram pointer to display buffer area
	bool m_flip_sync = false;

	emu_timer *m_pipeline_timer;

	// 2 bytes per framebuffer data
	inline u32 get_fb_addr(u16 x, u16 y) { return ((x & 0x3ff) | ((u32(y) & 0x1ff) << 10)) << 1; }
	inline u16 get_packet(u32 ptr) { return m_texcache.read_word(ptr << 1); }
	u16 do_alpha(quad_info &quad, u16 src, u16 dst);
	template <u8 Bpp, bool Tiled, u8 Blend> void draw_quad(quad_info &quad);
	void draw_quad_fill(quad_info &quad);
	int process_packet(u32 packet_ptr);

	u16 cmd_queue_front_r();
	void cmd_queue_front_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u16 cmd_queue_rear_r();

	u16 bank1_select_r();
	void bank1_select_w(u16 data);

	u16 display_bank_r();

	u16 render_control_r();
	void render_control_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	TIMER_CALLBACK_MEMBER(pipeline_cb);
};

DECLARE_DEVICE_TYPE(VIDEO_VRENDER0, vr0video_device)

#endif // MAME_VIDEO_VRENDER0_H
