/*****************************************************************************
 *
 * includes/arcadia.h
 *
 ****************************************************************************/

#ifndef ARCADIA_H_
#define ARCADIA_H_

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "imagedev/cartslot.h"
#include "audio/arcadia.h"


// space vultures sprites above
// combat below and invisible
#define YPOS 0
//#define YBOTTOM_SIZE 24
// grand slam sprites left and right
// space vultures left
// space attack left
#define XPOS 32


class arcadia_state : public driver_device
{
public:
	arcadia_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	DECLARE_READ8_MEMBER(arcadia_vsync_r);
	DECLARE_READ8_MEMBER(arcadia_video_r);
	DECLARE_WRITE8_MEMBER(arcadia_video_w);
	int m_line;
	int m_charline;
	int m_shift;
	int m_ad_delay;
	int m_ad_select;
	int m_ypos;
	int m_graphics;
	int m_doublescan;
	int m_lines26;
	int m_multicolor;
	struct { int x, y; } m_pos[4];
	UINT8 m_bg[262][16+2*XPOS/8];
	UINT8 m_rectangle[0x40][8];
	UINT8 m_chars[0x40][8];
	int m_breaker;
	union
	{
		UINT8 data[0x400];
		struct
		{
			// 0x1800
			UINT8 chars1[13][16];
			UINT8 ram1[2][16];
			struct  { UINT8 y,x; } pos[4];
			UINT8 ram2[4];
			UINT8 vpos;
			UINT8 sound1, sound2;
			UINT8 char_line;
			// 0x1900
			UINT8 pad1a, pad1b, pad1c, pad1d;
			UINT8 pad2a, pad2b, pad2c, pad2d;
			UINT8 keys, unmapped3[0x80-9];
			UINT8 chars[8][8];
			UINT8 unknown[0x38];
			UINT8 pal[4];
			UINT8 collision_bg,
			collision_sprite;
			UINT8 ad[2];
			// 0x1a00
			UINT8 chars2[13][16];
			UINT8 ram3[3][16];
		} d;
	} m_reg;
	bitmap_ind16 *m_bitmap;
	DECLARE_DRIVER_INIT(arcadia);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_arcadia(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(arcadia_video_line);
};
#endif /* ARCADIA_H_ */
