// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Williams first-generation blitter

***************************************************************************/

#ifndef MAME_MIDWAY_WILLIAMSBLITTER_H
#define MAME_MIDWAY_WILLIAMSBLITTER_H

#pragma once

class williams_blitter_device : public device_t {
public:
	template <typename T> void set_cpu_tag(T &&cpu_tag) { m_cpu.set_tag(std::forward<T>(cpu_tag)); }
	template <typename T> void set_vram_tag(T &&vram_tag) { m_vram.set_tag(std::forward<T>(vram_tag)); }
	template <typename T> void set_proms_tag(T &&proms_tag) { m_proms.set_tag(std::forward<T>(proms_tag)); }

	void map(address_map &map) ATTR_COLD;

	void remap_select_w(u8 data);
	void window_enable_w(u8 data);

protected:
	williams_blitter_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock = 0);

	void set_clip_address(u16 clip_address) { m_clip_address = clip_address; }
	void set_size_xor(u32 size_xor) { m_size_xor = size_xor; }

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// control byte bit definitions
	enum
	{
		CONTROLBYTE_NO_EVEN         = 7,
		CONTROLBYTE_NO_ODD          = 6,
		CONTROLBYTE_SHIFT           = 5,
		CONTROLBYTE_SOLID           = 4,
		CONTROLBYTE_FOREGROUND_ONLY = 3,
		CONTROLBYTE_SLOW            = 2, // 2us blits instead of 1us
		CONTROLBYTE_DST_STRIDE_256  = 1,
		CONTROLBYTE_SRC_STRIDE_256  = 0
	};

	required_device<cpu_device> m_cpu;
	required_shared_ptr<u8> m_vram;
	optional_region_ptr<u8> m_proms;

	void control_w(address_space &space, offs_t offset, u8 data);
	inline void blit_pixel(address_space &space, int dstaddr, int srcdata);
	int blit_core(address_space &space, int w, int h);

	u16 m_clip_address;
	u8 m_window_enable;
	u8 m_control;
	bool m_no_even;
	bool m_no_odd;
	bool m_solid;
	bool m_fg_only;
	u8 m_solid_color;
	u16 m_sstart;
	u16 m_dstart;
	u8 m_width;
	u8 m_height;
	u8 m_size_xor;
	u8 m_remap_index;
	const u8 *m_remap;
	std::unique_ptr<u8[]> m_remap_lookup;
};

class williams_blitter_sc1_device : public williams_blitter_device {
public:
	template <typename T, typename U>
	williams_blitter_sc1_device(const machine_config &mconfig, const char *tag, device_t *owner, u16 clip_address, T &&cpu_tag, U &&vram_tag)
		: williams_blitter_sc1_device(mconfig, tag, owner)
	{
		set_size_xor(4);
		set_clip_address(clip_address);
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_vram_tag(std::forward<U>(vram_tag));
	}

	template <typename T, typename U, typename V>
	williams_blitter_sc1_device(const machine_config &mconfig, const char *tag, device_t *owner, u16 clip_address, T &&cpu_tag, U &&vram_tag, V &&proms_tag)
		: williams_blitter_sc1_device(mconfig, tag, owner)
	{
		set_size_xor(4);
		set_clip_address(clip_address);
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_vram_tag(std::forward<U>(vram_tag));
		set_proms_tag(std::forward<V>(proms_tag));
	}

	williams_blitter_sc1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class williams_blitter_sc2_device : public williams_blitter_device {
public:
	template <typename T, typename U>
	williams_blitter_sc2_device(const machine_config &mconfig, const char *tag, device_t *owner, u16 clip_address, T &&cpu_tag, U &&vram_tag)
		: williams_blitter_sc2_device(mconfig, tag, owner)
	{
		set_size_xor(0);
		set_clip_address(clip_address);
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_vram_tag(std::forward<U>(vram_tag));
	}

	template <typename T, typename U, typename V>
	williams_blitter_sc2_device(const machine_config &mconfig, const char *tag, device_t *owner, u16 clip_address, T &&cpu_tag, U &&vram_tag, V &&proms_tag)
		: williams_blitter_sc2_device(mconfig, tag, owner)
	{
		set_size_xor(0);
		set_clip_address(clip_address);
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_vram_tag(std::forward<U>(vram_tag));
		set_proms_tag(std::forward<V>(proms_tag));
	}

	williams_blitter_sc2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

DECLARE_DEVICE_TYPE(WILLIAMS_BLITTER_SC1, williams_blitter_sc1_device)
DECLARE_DEVICE_TYPE(WILLIAMS_BLITTER_SC2, williams_blitter_sc2_device)

#endif // MAME_MIDWAY_WILLIAMSBLITTER_H
