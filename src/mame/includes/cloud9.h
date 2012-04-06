/*************************************************************************

    Atari Cloud 9 (prototype) hardware

*************************************************************************/

#include "cpu/m6502/m6502.h"
#include "machine/x2212.h"

class cloud9_state : public driver_device
{
public:
	cloud9_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_nvram(*this, "nvram") { }

	/* memory pointers */
	UINT8 *     m_videoram;
	UINT8 *     m_spriteram;
	UINT8 *     m_paletteram;

	/* video-related */
	const UINT8 *m_syncprom;
	const UINT8 *m_wpprom;
	const UINT8 *m_priprom;
	bitmap_ind16 m_spritebitmap;
	double      m_rweights[3];
	double		m_gweights[3];
	double		m_bweights[3];
	UINT8       m_video_control[8];
	UINT8       m_bitmode_addr[2];

	/* misc */
	int         m_vblank_start;
	int         m_vblank_end;
	emu_timer   *m_irq_timer;
	UINT8       m_irq_state;

	/* devices */
	required_device<m6502_device> m_maincpu;
	required_device<x2212_device> m_nvram;
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_WRITE8_MEMBER(cloud9_led_w);
	DECLARE_WRITE8_MEMBER(cloud9_coin_counter_w);
	DECLARE_READ8_MEMBER(leta_r);
	DECLARE_WRITE8_MEMBER(nvram_recall_w);
	DECLARE_WRITE8_MEMBER(nvram_store_w);
	DECLARE_WRITE8_MEMBER(cloud9_video_control_w);
	DECLARE_WRITE8_MEMBER(cloud9_paletteram_w);
	DECLARE_WRITE8_MEMBER(cloud9_videoram_w);
	DECLARE_READ8_MEMBER(cloud9_bitmode_r);
	DECLARE_WRITE8_MEMBER(cloud9_bitmode_w);
	DECLARE_WRITE8_MEMBER(cloud9_bitmode_addr_w);
};


/*----------- defined in video/cloud9.c -----------*/

VIDEO_START( cloud9 );
SCREEN_UPDATE_IND16( cloud9 );



