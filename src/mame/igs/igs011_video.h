// license:BSD-3-Clause
// copyright-holders:Luca Elia, Olivier Galibert
#ifndef MAME_IGS_IGS011_VIDEO_H
#define MAME_IGS_IGS011_VIDEO_H

#pragma once

class igs011_device : public device_t, public device_gfx_interface
{
public:
	igs011_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configurations
	template <typename T> void set_host_space(T &&tag, int index) { m_host_space.set_tag(std::forward<T>(tag), index); }
	auto restore_space_callback() { return m_restore_space_cb.bind(); }

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void blitter_pen_hi_w(u8 data);
	void priority_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 layers_r(offs_t offset);
	void layers_w(offs_t offset, u8 data);
	u16 priority_ram_r(offs_t offset);
	void priority_ram_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void prot_w(offs_t offset, u8 data);
	u16 prot_r();
	void prot_addr_w(u16 data);

	void map(address_map &map) ATTR_COLD;

	void lhb2_gfx_decrypt() ATTR_COLD;
	void drgnwrld_gfx_decrypt() ATTR_COLD;
	void vbowl_gfx_unpack() ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

private:
	void blit_x_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void blit_y_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void blit_gfx_lo_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void blit_gfx_hi_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void blit_w_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void blit_h_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void blit_depth_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void blit_pen_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void blit_flags_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void prot_addr_change_w(s32 data);
	void prot_mem_range_set();

	// blitter
	struct blitter_t
	{
		blitter_t()
			: x(0), y(0), w(0), h(0)
			, gfx_lo(0), gfx_hi(0)
			, depth(0)
			, pen(0)
			, flags(0)
		{
		}

		u16 x, y, w, h;
		u16 gfx_lo, gfx_hi;
		u16 depth;
		u16 pen;
		u16 flags;
	};

	blitter_t m_blitter;

	u16 m_priority;
	u8 m_blitter_pen_hi;

	// protection
	u8 m_prot;
	u8 m_prot_swap;
	u32 m_prot_addr;
	s32 m_prev_prot_addr;

	memory_share_array_creator<u8, 4> m_layer_ram;
	memory_share_creator<u16> m_priority_ram;

	required_region_ptr<u8> m_gfx;
	optional_region_ptr<u8> m_gfx_hi;

	required_address_space m_host_space;

	devcb_write32 m_restore_space_cb;
};

DECLARE_DEVICE_TYPE(IGS011, igs011_device)

#endif // MAME_IGS_IGS011_VIDEO_H
