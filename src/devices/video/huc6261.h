// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Hudson/NEC HuC6261 interface and definitions

**********************************************************************/


#ifndef __HUC6261_H_
#define __HUC6261_H_

#include "emu.h"
#include "video/huc6270.h"


/* Screen timing stuff */
#define HUC6261_WPF         1365    /* width of a line in frame including blanking areas */
#define HUC6261_LPF         263     /* max number of lines in a single frame */


#define MCFG_HUC6261_VDC1(_tag) \
	huc6261_device::set_vdc1_tag(*device, _tag);

#define MCFG_HUC6261_VDC2(_tag) \
	huc6261_device::set_vdc2_tag(*device, _tag);


class huc6261_device :  public device_t,
						public device_video_interface
{
public:
	// construction/destruction
	huc6261_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_vdc1_tag(device_t &device, const char *tag) { downcast<huc6261_device &>(device).m_huc6270_a_tag = tag; }
	static void set_vdc2_tag(device_t &device, const char *tag) { downcast<huc6261_device &>(device).m_huc6270_b_tag = tag; }

	void video_update(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );

	inline UINT32 yuv2rgb(UINT32 yuv);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	const char *m_huc6270_a_tag;
	const char *m_huc6270_b_tag;

	huc6270_device *m_huc6270_a;
	huc6270_device *m_huc6270_b;
	int     m_last_h;
	int     m_last_v;
	int     m_height;

	UINT16  m_palette[512];
	UINT16  m_address;
	UINT16  m_palette_latch;
	UINT16  m_register;
	UINT16  m_control;
	UINT8   m_priority[7];

	UINT8   m_pixels_per_clock; /* Number of pixels to output per colour clock */
	UINT16  m_pixel_data;
	UINT8   m_pixel_clock;

	emu_timer   *m_timer;
	std::unique_ptr<bitmap_rgb32>  m_bmp;
	INT32   m_uv_lookup[65536][3];
};


extern const device_type HUC6261;


#endif
