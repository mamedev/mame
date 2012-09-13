/*************************************************************************

    Sega Z80-3D system

*************************************************************************/

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/discrete.h"

/* sprites are scaled in the analog domain; to give a better */
/* rendition of this, we scale in the X direction by this factor */
#define TURBO_X_SCALE		2



class turbo_state : public driver_device
{
public:
	turbo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_i8255_0(*this, "i8255_0"),
		m_i8255_1(*this, "i8255_1"),
		m_i8255_2(*this, "i8255_2"),
		m_i8255_3(*this, "i8255_3"),
		m_gfx1(*this, "gfx1"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_sprite_position(*this, "spritepos")
	{ }

	/* device/memory pointers */
	required_device<z80_device> m_maincpu;
	optional_device<z80_device> m_subcpu;
	required_device<i8255_device> m_i8255_0;
	required_device<i8255_device> m_i8255_1;
	optional_device<i8255_device> m_i8255_2;
	optional_device<i8255_device> m_i8255_3;
	required_memory_region m_gfx1;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_sprite_position;

	UINT8 *		m_buckrog_bitmap_ram;

	/* machine states */
	UINT8		m_i8279_scanlines;

	/* sound state */
	UINT8		m_turbo_osel;
	UINT8		m_turbo_bsel;
	UINT8		m_sound_state[3];

	/* video state */
	tilemap_t *	m_fg_tilemap;

	/* Turbo-specific states */
	UINT8		m_turbo_opa;
	UINT8		m_turbo_opb;
	UINT8		m_turbo_opc;
	UINT8		m_turbo_ipa;
	UINT8		m_turbo_ipb;
	UINT8		m_turbo_ipc;
	UINT8		m_turbo_fbpla;
	UINT8		m_turbo_fbcol;
	UINT8		m_turbo_speed;
	UINT8		m_turbo_collision;
	UINT8		m_turbo_last_analog;
	UINT8		m_turbo_accel;

	/* Subroc-specific states */
	UINT8		m_subroc3d_col;
	UINT8		m_subroc3d_ply;
	UINT8		m_subroc3d_flip;
	UINT8		m_subroc3d_mdis;
	UINT8		m_subroc3d_mdir;
	UINT8		m_subroc3d_tdis;
	UINT8		m_subroc3d_tdir;
	UINT8		m_subroc3d_fdis;
	UINT8		m_subroc3d_fdir;
	UINT8		m_subroc3d_hdis;
	UINT8		m_subroc3d_hdir;

	/* Buck Rogers-specific states */
	UINT8		m_buckrog_fchg;
	UINT8		m_buckrog_mov;
	UINT8		m_buckrog_obch;
	UINT8		m_buckrog_command;
	UINT8		m_buckrog_myship;
	int m_last_sound_a;

	DECLARE_WRITE8_MEMBER(scanlines_w);
	DECLARE_WRITE8_MEMBER(digit_w);
	DECLARE_READ8_MEMBER(turbo_collision_r);
	DECLARE_WRITE8_MEMBER(turbo_collision_clear_w);
	DECLARE_WRITE8_MEMBER(turbo_analog_reset_w);
	DECLARE_WRITE8_MEMBER(turbo_coin_and_lamp_w);
	DECLARE_READ8_MEMBER(buckrog_cpu2_command_r);
	DECLARE_READ8_MEMBER(buckrog_port_2_r);
	DECLARE_READ8_MEMBER(buckrog_port_3_r);
	DECLARE_WRITE8_MEMBER(turbo_videoram_w);
	DECLARE_WRITE8_MEMBER(buckrog_bitmap_w);
	DECLARE_WRITE8_MEMBER(turbo_ppi0a_w);
	DECLARE_WRITE8_MEMBER(turbo_ppi0b_w);
	DECLARE_WRITE8_MEMBER(turbo_ppi0c_w);
	DECLARE_WRITE8_MEMBER(turbo_ppi1a_w);
	DECLARE_WRITE8_MEMBER(turbo_ppi1b_w);
	DECLARE_WRITE8_MEMBER(turbo_ppi1c_w);
	DECLARE_WRITE8_MEMBER(turbo_ppi3c_w);
	DECLARE_WRITE8_MEMBER(subroc3d_ppi0a_w);
	DECLARE_WRITE8_MEMBER(subroc3d_ppi0c_w);
	DECLARE_WRITE8_MEMBER(subroc3d_ppi0b_w);
	DECLARE_WRITE8_MEMBER(buckrog_ppi0a_w);
	DECLARE_WRITE8_MEMBER(buckrog_ppi0b_w);
	DECLARE_WRITE8_MEMBER(buckrog_ppi0c_w);
	DECLARE_WRITE8_MEMBER(buckrog_ppi1c_w);
	DECLARE_READ8_MEMBER(turbo_analog_r);
	DECLARE_WRITE8_MEMBER(buckrog_i8255_0_w);
	DECLARE_DRIVER_INIT(buckrog_enc);
	DECLARE_DRIVER_INIT(turbo_enc);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	DECLARE_VIDEO_START(turbo);
	DECLARE_PALETTE_INIT(turbo);
	DECLARE_PALETTE_INIT(subroc3d);
	DECLARE_MACHINE_RESET(buckrog);
	DECLARE_VIDEO_START(buckrog);
	DECLARE_PALETTE_INIT(buckrog);
};


/*----------- defined in audio/turbo.c -----------*/

MACHINE_CONFIG_EXTERN( turbo_samples );
MACHINE_CONFIG_EXTERN( subroc3d_samples );
MACHINE_CONFIG_EXTERN( buckrog_samples );

WRITE8_DEVICE_HANDLER( turbo_sound_a_w );
WRITE8_DEVICE_HANDLER( turbo_sound_b_w );
WRITE8_DEVICE_HANDLER( turbo_sound_c_w );

WRITE8_DEVICE_HANDLER( subroc3d_sound_a_w );
WRITE8_DEVICE_HANDLER( subroc3d_sound_b_w );
WRITE8_DEVICE_HANDLER( subroc3d_sound_c_w );

WRITE8_DEVICE_HANDLER( buckrog_sound_a_w );
WRITE8_DEVICE_HANDLER( buckrog_sound_b_w );


/*----------- defined in video/turbo.c -----------*/



SCREEN_UPDATE_IND16( turbo );


SCREEN_UPDATE_IND16( subroc3d );



SCREEN_UPDATE_IND16( buckrog );

