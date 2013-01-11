#ifndef __K053250_H__
#define __K053250_H__

//
//  Konami 053250 road generator
//

#include "emu.h"

#define MCFG_K053250_ADD(_tag, screen_tag, offx, offy)  \
	MCFG_DEVICE_ADD(_tag, K053250, 0) \
	k053250_t::static_set_screen_tag(*device, screen_tag); \
	k053250_t::static_set_offsets(*device, offx, offy);

class k053250_t : public device_t
{
public:
	k053250_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void static_set_screen_tag(device_t &device, const char *screen_tag);
	static void static_set_offsets(device_t &device, int offx, int offy);

	DECLARE_READ16_MEMBER(reg_r);
	DECLARE_WRITE16_MEMBER(reg_w);
	DECLARE_READ16_MEMBER(ram_r);
	DECLARE_WRITE16_MEMBER(ram_w);
	DECLARE_READ16_MEMBER(rom_r);

	void draw( bitmap_rgb32 &bitmap, const rectangle &cliprect, int colorbase, int flags, int priority );

protected:
	void device_start();
	void device_reset();

private:
	UINT8 regs[8];
	UINT8 *unpacked;
	UINT32 unpacked_size;
	UINT16 *ram;
	UINT16 *buffer[2];
	UINT32 page;
	INT32 frame;
	int offx, offy;
	const char *screen_tag;
	screen_device *screen;

	void unpack_nibbles();
	void dma(int limiter);
	static void pdraw_scanline32(bitmap_rgb32 &bitmap, const pen_t *palette, UINT8 *source,
									const rectangle &cliprect, int linepos, int scroll, int zoom,
									UINT32 clipmask, UINT32 wrapmask, UINT32 orientation, bitmap_ind8 &priority, UINT8 pri);
};

extern const device_type K053250;

#endif
