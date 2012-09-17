/*****************************************************************************
 *
 * includes/pp01.h
 *
 ****************************************************************************/

#ifndef PP01_H_
#define PP01_H_

#include "machine/pit8253.h"
#include "machine/i8255.h"

class pp01_state : public driver_device
{
public:
	pp01_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_video_scroll;
	UINT8 m_memory_block[16];
	UINT8 m_video_write_mode;
	UINT8 m_key_line;
	DECLARE_WRITE8_MEMBER(pp01_video_write_mode_w);
	DECLARE_WRITE8_MEMBER(pp01_video_r_1_w);
	DECLARE_WRITE8_MEMBER(pp01_video_g_1_w);
	DECLARE_WRITE8_MEMBER(pp01_video_b_1_w);
	DECLARE_WRITE8_MEMBER(pp01_video_r_2_w);
	DECLARE_WRITE8_MEMBER(pp01_video_g_2_w);
	DECLARE_WRITE8_MEMBER(pp01_video_b_2_w);
	DECLARE_WRITE8_MEMBER(pp01_mem_block_w);
	DECLARE_READ8_MEMBER(pp01_mem_block_r);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_pp01(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in machine/pp01.c -----------*/
extern const struct pit8253_config pp01_pit8253_intf;
extern const i8255_interface pp01_ppi8255_interface;

#endif
