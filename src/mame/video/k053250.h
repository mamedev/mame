// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef __K053250_H__
#define __K053250_H__

//
//  Konami 053250 road generator
//

#include "emu.h"

#define MCFG_K053250_ADD(_tag, _palette_tag, _screen_tag, offx, offy)  \
	MCFG_DEVICE_ADD(_tag, K053250, 0) \
	MCFG_GFX_PALETTE(_palette_tag) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag) \
	k053250_device::static_set_offsets(*device, offx, offy);

class k053250_device :  public device_t,
						public device_gfx_interface,
						public device_video_interface
{
public:
	k053250_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void static_set_offsets(device_t &device, int offx, int offy);

	DECLARE_READ16_MEMBER(reg_r);
	DECLARE_WRITE16_MEMBER(reg_w);
	DECLARE_READ16_MEMBER(ram_r);
	DECLARE_WRITE16_MEMBER(ram_w);
	DECLARE_READ16_MEMBER(rom_r);

	void draw( bitmap_rgb32 &bitmap, const rectangle &cliprect, int colorbase, int flags, bitmap_ind8 &priority_bitmap, int priority );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// configuration
	int m_offx, m_offy;

	// internal state
	dynamic_buffer m_unpacked_rom;
	std::vector<UINT16> m_ram;
	UINT16 *m_buffer[2];
	UINT8 m_regs[8];
	UINT8 m_page;
	INT32 m_frame;

	// internal helpers
	void unpack_nibbles();
	void dma(int limiter);
	static void pdraw_scanline32(bitmap_rgb32 &bitmap, const pen_t *pal_base, UINT8 *source,
									const rectangle &cliprect, int linepos, int scroll, int zoom,
									UINT32 clipmask, UINT32 wrapmask, UINT32 orientation, bitmap_ind8 &priority, UINT8 pri);
};

extern const device_type K053250;

#endif
