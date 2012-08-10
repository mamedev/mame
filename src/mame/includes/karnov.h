/*************************************************************************

    Karnov - Wonder Planet - Chelnov

*************************************************************************/

#include "video/bufsprite.h"

class karnov_state : public driver_device
{
public:
	karnov_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") ,
		m_ram(*this, "ram"),
		m_videoram(*this, "videoram"),
		m_pf_data(*this, "pf_data"){ }

	required_device<buffered_spriteram16_device> m_spriteram;
	/* memory pointers */
	required_shared_ptr<UINT16> m_ram;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_pf_data;

	/* video-related */
	bitmap_ind16    *m_bitmap_f;
	tilemap_t     *m_fix_tilemap;
	int         m_flipscreen;
	UINT16      m_scroll[2];

	/* misc */
	UINT16      m_i8751_return;
	UINT16      m_i8751_needs_ack;
	UINT16      m_i8751_coin_pending;
	UINT16      m_i8751_command_queue;
	int         m_i8751_level;	// needed by chelnov
	int         m_microcontroller_id;
	int         m_coin_mask;
	int         m_latch;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	DECLARE_WRITE16_MEMBER(karnov_control_w);
	DECLARE_READ16_MEMBER(karnov_control_r);
	DECLARE_WRITE16_MEMBER(karnov_videoram_w);
	DECLARE_WRITE16_MEMBER(karnov_playfield_swap_w);
	DECLARE_DRIVER_INIT(wndrplnt);
	DECLARE_DRIVER_INIT(karnov);
	DECLARE_DRIVER_INIT(karnovj);
	DECLARE_DRIVER_INIT(chelnovu);
	DECLARE_DRIVER_INIT(chelnovj);
	DECLARE_DRIVER_INIT(chelnov);
};

enum {
	KARNOV = 0,
	KARNOVJ,
	CHELNOV,
	CHELNOVU,
	CHELNOVJ,
	WNDRPLNT
};


/*----------- defined in video/karnov.c -----------*/


void karnov_flipscreen_w(running_machine &machine, int data);

PALETTE_INIT( karnov );
VIDEO_START( karnov );
VIDEO_START( wndrplnt );
SCREEN_UPDATE_IND16( karnov );
