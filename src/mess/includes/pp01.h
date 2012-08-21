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
};


/*----------- defined in machine/pp01.c -----------*/
extern const struct pit8253_config pp01_pit8253_intf;
extern const i8255_interface pp01_ppi8255_interface;
extern MACHINE_START( pp01 );
extern MACHINE_RESET( pp01 );
/*----------- defined in video/pp01.c -----------*/

extern VIDEO_START( pp01 );
extern SCREEN_UPDATE_IND16( pp01 );
extern PALETTE_INIT( pp01 );

#endif
