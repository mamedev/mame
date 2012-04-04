/*************************************************************************

    Atari Crystal Castles hardware

*************************************************************************/

#include "cpu/m6502/m6502.h"
#include "machine/x2212.h"

class ccastles_state : public driver_device
{
public:
	ccastles_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_nvram_4b(*this, "nvram_4b"),
		  m_nvram_4a(*this, "nvram_4a") { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_spriteram;

	/* video-related */
	const UINT8 *m_syncprom;
	const UINT8 *m_wpprom;
	const UINT8 *m_priprom;
	bitmap_ind16 m_spritebitmap;
	double m_rweights[3];
	double m_gweights[3];
	double m_bweights[3];
	UINT8 m_video_control[8];
	UINT8 m_bitmode_addr[2];
	UINT8 m_hscroll;
	UINT8 m_vscroll;

	/* misc */
	int      m_vblank_start;
	int      m_vblank_end;
	emu_timer *m_irq_timer;
	UINT8    m_irq_state;
	UINT8    m_nvram_store[2];

	/* devices */
	required_device<m6502_device> m_maincpu;
	required_device<x2212_device> m_nvram_4b;
	required_device<x2212_device> m_nvram_4a;
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_WRITE8_MEMBER(ccounter_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_READ8_MEMBER(leta_r);
	DECLARE_WRITE8_MEMBER(nvram_recall_w);
	DECLARE_WRITE8_MEMBER(nvram_store_w);
	DECLARE_READ8_MEMBER(nvram_r);
	DECLARE_WRITE8_MEMBER(nvram_w);
};


/*----------- defined in video/ccastles.c -----------*/


VIDEO_START( ccastles );
SCREEN_UPDATE_IND16( ccastles );

WRITE8_HANDLER( ccastles_hscroll_w );
WRITE8_HANDLER( ccastles_vscroll_w );
WRITE8_HANDLER( ccastles_video_control_w );

WRITE8_HANDLER( ccastles_paletteram_w );
WRITE8_HANDLER( ccastles_videoram_w );

READ8_HANDLER( ccastles_bitmode_r );
WRITE8_HANDLER( ccastles_bitmode_w );
WRITE8_HANDLER( ccastles_bitmode_addr_w );
