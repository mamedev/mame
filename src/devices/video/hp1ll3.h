// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    HP 1LL3-0005 GPU emulation.

 ***************************************************************************/

#ifndef MAME_VIDEO_HP1LL3_H
#define MAME_VIDEO_HP1LL3_H

#pragma once


///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_HP1LL3_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, HP1LL3, 0)

///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************


// ======================> hp1ll3_device

class hp1ll3_device : public device_t, public device_video_interface
{
public:
	// construction/destruction
	hp1ll3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void command(int command);

	inline void point(int x, int y, int px);
	void label(uint8_t chr, int width);
	void fill(int x, int y, int w, int h, int arg);
	void line(int x_from, int y_from, int x_to, int y_to);
	void bitblt(int dstx, int dsty, uint16_t srcaddr, int width, int height, int op);

	uint16_t m_conf[12], m_input[2];
	int m_input_ptr, m_memory_ptr, m_conf_ptr;
	int m_command, m_horiz_pix_total, m_vert_pix_total;

	uint16_t m_sad;
	uint16_t m_org;
	uint16_t m_dad;
	uint16_t m_rr;
	uint16_t m_fad, m_fontdata, m_fontheight;
	uint16_t m_udl;

	bool m_enable_video, m_enable_cursor, m_enable_sprite;
	uint16_t m_cursor_x, m_cursor_y, m_saved_x;
	uint16_t m_sprite_x, m_sprite_y;
	struct {
		uint16_t width, height, org_x, org_y, width_w;
	} m_window;
	std::unique_ptr<uint16_t[]> m_videoram;

	bool m_busy;

	bitmap_ind16 m_bitmap, m_cursor, m_sprite;
};


// device type definition
DECLARE_DEVICE_TYPE(HP1LL3, hp1ll3_device)

#endif // MAME_VIDEO_HP1LL3_H
