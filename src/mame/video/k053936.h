// license:BSD-3-Clause
// copyright-holders:David Haywood
#pragma once
#ifndef __K053936_H__
#define __K053936_H__

#define MCFG_K053936_ADD(_tag, _rozram_tag) \
	MCFG_DEVICE_ADD(_tag, K053936, 0)		 \
	downcast<k053936_device *>(device)->set_rozram_tag(_rozram_tag);

void K053936_0_zoom_draw(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,tilemap_t *tmap,int flags,uint32_t priority, int glfgreat_hack);
void K053936_wraparound_enable(int chip, int status);
void K053936_set_offset(int chip, int xoffs, int yoffs);

// GX specific implementations...
void K053936GP_set_offset(int chip, int xoffs, int yoffs);
void K053936GP_clip_enable(int chip, int status);
void K053936GP_set_cliprect(int chip, int minx, int maxx, int miny, int maxy);
void K053936GP_0_zoom_draw(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *tmap, int tilebpp, int blend, int alpha, int pixeldouble_output, uint16_t* temp_m_k053936_0_ctrl_16, uint16_t* temp_m_k053936_0_linectrl_16, uint16_t* temp_m_k053936_0_ctrl, uint16_t* temp_m_k053936_0_linectrl, palette_device &palette);


class k053936_device : public device_t
{
public:
	k053936_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k053936_device() {}

	void set_rozram_tag(const char *rozram_tag);

	DECLARE_ADDRESS_MAP(map, 16);

	DECLARE_WRITE16_MEMBER(counter_start_w);
	DECLARE_WRITE16_MEMBER(vadd_w);
	DECLARE_WRITE16_MEMBER(hadd_w);
	DECLARE_WRITE16_MEMBER(mode1_w);
	DECLARE_WRITE16_MEMBER(mode2_w);
	DECLARE_WRITE16_MEMBER(x_min_w);
	DECLARE_WRITE16_MEMBER(x_max_w);
	DECLARE_WRITE16_MEMBER(y_min_w);
	DECLARE_WRITE16_MEMBER(y_max_w);

	void bitmap_update(bitmap_ind16 *bitmap_x, bitmap_ind16 *bitmap_y, const rectangle &cliprect);

	// static configuration
	static void set_wrap(device_t &device, int wrap) { downcast<k053936_device &>(device).m_wrap = wrap; }
	static void set_offsets(device_t &device, int x_offset, int y_offset)
	{
		k053936_device &dev = downcast<k053936_device &>(device);
		dev.m_xoff = x_offset;
		dev.m_yoff = y_offset;
	}

	DECLARE_WRITE16_MEMBER( ctrl_w );
	DECLARE_READ16_MEMBER( ctrl_r );
	DECLARE_WRITE16_MEMBER( linectrl_w );
	DECLARE_READ16_MEMBER( linectrl_r );
	void zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t *tmap, int flags, uint32_t priority, int glfgreat_hack);
	// void wraparound_enable(int status);   unused? // shall we merge this into the configuration intf?
	// void set_offset(int xoffs, int yoffs); unused?   // shall we merge this into the configuration intf?

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	const char *m_rozram_tag;
	memory_share *m_rozram;

	uint16_t m_counter_start[2], m_vadd[2], m_hadd[2], m_mode1, m_mode2, m_min[2], m_max[2];

	// internal state
	std::unique_ptr<uint16_t[]>    m_ctrl;
	std::unique_ptr<uint16_t[]>    m_linectrl;
	int       m_wrap, m_xoff, m_yoff;
};

extern const device_type K053936;

#define MCFG_K053936_WRAP(_wrap) \
	k053936_device::set_wrap(*device, _wrap);

#define MCFG_K053936_OFFSETS(_xoffs, _yoffs) \
	k053936_device::set_offsets(*device, _xoffs, _yoffs);

#endif
