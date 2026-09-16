// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_VIDEO_JANGOU_BLITTER_H
#define MAME_VIDEO_JANGOU_BLITTER_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> jangou_blitter_device

class jangou_blitter_device : public device_t
{
public:
	// construction/destruction
	jangou_blitter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void blit_v1_regs(address_map &map) ATTR_COLD;
	void blit_v2_regs(address_map &map) ATTR_COLD;

	void vregs_w(offs_t offset, uint8_t data);
	void bltflip_w(uint8_t data);
	int status_r();

	const uint8_t &blit_buffer(unsigned y, unsigned x) const { return m_blit_buffer[(256 * y) + x]; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_blit_buffer[256 * 256];

	void plot_gfx_pixel( uint8_t pix, int x, int y );
	uint8_t gfx_nibble( uint32_t niboffset );
	void trigger_write(void);
	uint8_t m_pen_data[0x10];
	uint8_t m_x,m_y,m_width,m_height;
	uint32_t m_src_addr;
	//uint8_t m_blit_data[7];
	uint8_t *m_gfxrom;
	uint32_t m_gfxrommask;
	bool m_bltflip;

	// blitter write accessors
	void x_w(uint8_t data);
	void y_w(uint8_t data);
	void height_and_trigger_w(uint8_t data);
	void width_w(uint8_t data);
	void src_lo_address_w(uint8_t data);
	void src_md_address_w(uint8_t data);
	void src_hi_address_w(uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(JANGOU_BLITTER, jangou_blitter_device)

#endif // MAME_VIDEO_JANGOU_BLITTER_H
