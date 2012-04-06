
#include "sound/okim6295.h"

struct f2_tempsprite
{
	int code, color;
	int flipx, flipy;
	int x, y;
	int zoomx, zoomy;
	int primask;
};

class taitof2_state : public driver_device
{
public:
	taitof2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_oki(*this, "oki") { }

	/* memory pointers */
	UINT16 *        m_sprite_extension;
	UINT16 *        m_spriteram;
	UINT16 *        m_spriteram_buffered;
	UINT16 *        m_spriteram_delayed;
	UINT16 *        m_cchip2_ram;	// for megablst only
//  UINT16 *        m_paletteram;    // currently this uses generic palette handling
	size_t          m_spriteram_size;
	size_t          m_spriteext_size;


	/* video-related */
	struct f2_tempsprite *m_spritelist;
	int             m_sprite_type;

	UINT16          m_spritebank[8];
//  UINT16          m_spritebank_eof[8];
	UINT16          m_spritebank_buffered[8];

	INT32           m_sprites_disabled;
	INT32			m_sprites_active_area;
	INT32			m_sprites_master_scrollx;
	INT32			m_sprites_master_scrolly;
	/* remember flip status over frames because driftout can fail to set it */
	INT32           m_sprites_flipscreen;

	/* On the left hand screen edge (assuming horiz screen, no
       screenflip: in screenflip it is the right hand edge etc.)
       there may be 0-3 unwanted pixels in both tilemaps *and*
       sprites. To erase this we use f2_hide_pixels (0 to +3). */

	INT32           m_hide_pixels;
	INT32           m_flip_hide_pixels;	/* Different in some games */

	INT32           m_pivot_xdisp;	/* Needed in games with a pivot layer */
	INT32           m_pivot_ydisp;

	INT32           m_game;

	UINT8           m_tilepri[6]; // todo - move into taitoic.c
	UINT8           m_spritepri[6]; // todo - move into taitoic.c
	UINT8           m_spriteblendmode; // todo - move into taitoic.c

	int             m_prepare_sprites;

	/* misc */
	INT32           m_mjnquest_input;
	int             m_last[2];
	int             m_nibble;
	INT32           m_driveout_sound_latch;
	INT32           m_oki_bank;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	optional_device<okim6295_device> m_oki;
	device_t *m_tc0100scn;
	device_t *m_tc0100scn_1;
	device_t *m_tc0100scn_2;
	device_t *m_tc0360pri;
	device_t *m_tc0280grd;
	device_t *m_tc0430grw;
	device_t *m_tc0480scp;
	DECLARE_WRITE16_MEMBER(growl_coin_word_w);
	DECLARE_WRITE16_MEMBER(taitof2_4p_coin_word_w);
	DECLARE_WRITE16_MEMBER(ninjak_coin_word_w);
	DECLARE_READ16_MEMBER(ninjak_input_r);
	DECLARE_READ16_MEMBER(cameltry_paddle_r);
	DECLARE_READ16_MEMBER(mjnquest_dsw_r);
	DECLARE_READ16_MEMBER(mjnquest_input_r);
	DECLARE_WRITE16_MEMBER(mjnquest_inputselect_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_READ8_MEMBER(driveout_sound_command_r);
	DECLARE_WRITE8_MEMBER(oki_bank_w);
	DECLARE_WRITE16_MEMBER(driveout_sound_command_w);
	DECLARE_WRITE16_MEMBER(cchip2_word_w);
	DECLARE_READ16_MEMBER(cchip2_word_r);
	DECLARE_WRITE16_MEMBER(taitof2_sprite_extension_w);
	DECLARE_WRITE16_MEMBER(taitof2_spritebank_w);
	DECLARE_WRITE16_MEMBER(koshien_spritebank_w);
};

/*----------- defined in video/taito_f2.c -----------*/

VIDEO_START( taitof2_default );
VIDEO_START( taitof2_quiz );
VIDEO_START( taitof2_finalb );
VIDEO_START( taitof2_megab );
VIDEO_START( taitof2_solfigtr );
VIDEO_START( taitof2_koshien );
VIDEO_START( taitof2_driftout );
VIDEO_START( taitof2_dondokod );
VIDEO_START( taitof2_thundfox );
VIDEO_START( taitof2_growl );
VIDEO_START( taitof2_yuyugogo );
VIDEO_START( taitof2_mjnquest );
VIDEO_START( taitof2_footchmp );
VIDEO_START( taitof2_hthero );
VIDEO_START( taitof2_ssi );
VIDEO_START( taitof2_gunfront );
VIDEO_START( taitof2_ninjak );
VIDEO_START( taitof2_pulirula );
VIDEO_START( taitof2_metalb );
VIDEO_START( taitof2_qzchikyu );
VIDEO_START( taitof2_yesnoj );
VIDEO_START( taitof2_deadconx );
VIDEO_START( taitof2_deadconxj );
VIDEO_START( taitof2_dinorex );
SCREEN_VBLANK( taitof2_no_buffer );
SCREEN_VBLANK( taitof2_full_buffer_delayed );
SCREEN_VBLANK( taitof2_partial_buffer_delayed );
SCREEN_VBLANK( taitof2_partial_buffer_delayed_thundfox );
SCREEN_VBLANK( taitof2_partial_buffer_delayed_qzchikyu );

SCREEN_UPDATE_IND16( taitof2 );
SCREEN_UPDATE_IND16( taitof2_pri );
SCREEN_UPDATE_IND16( taitof2_pri_roz );
SCREEN_UPDATE_IND16( taitof2_ssi );
SCREEN_UPDATE_IND16( taitof2_thundfox );
SCREEN_UPDATE_IND16( taitof2_deadconx );
SCREEN_UPDATE_IND16( taitof2_metalb );
SCREEN_UPDATE_IND16( taitof2_yesnoj );

