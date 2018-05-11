// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
#ifndef MAME_VIDEO_TOPCAT_H_
#define MAME_VIDEO_TOPCAT_H_

#pragma once

#define MCFG_TOPCAT_FB_WIDTH(_pixels) \
	downcast<topcat_device &>(*device).set_fb_width(_pixels);

#define MCFG_TOPCAT_FB_HEIGHT(_pixels) \
	downcast<topcat_device &>(*device).set_fb_height(_pixels);

#define MCFG_TOPCAT_PLANES(_planes) \
	downcast<topcat_device &>(*device).set_planes(_planes);

class topcat_device : public device_t
{
public:
	topcat_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_fb_width(int _pixels) { m_fb_width = _pixels; }
	void set_fb_height(int _pixels) { m_fb_height = _pixels; }
	void set_planes(int _planes) { m_planes = _planes; }

protected:
	topcat_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override;
	virtual void device_reset() override;

	DECLARE_READ8_MEMBER(address_r);
	DECLARE_WRITE8_MEMBER(address_w);
	DECLARE_READ8_MEMBER(register_r);
	DECLARE_WRITE8_MEMBER(register_w);

private:
	int m_fb_width;
	int m_fb_height;
	int m_planes;
	int m_plane_mask;
};

DECLARE_DEVICE_TYPE(TOPCAT, topcat_device)
#endif // MAME_VIDEO_TOPCAT_H_
