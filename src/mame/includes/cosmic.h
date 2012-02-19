/*************************************************************************

    Cosmic Guerilla & other Universal boards (in cosmic.c)

*************************************************************************/

#include "sound/samples.h"

#define COSMICG_MASTER_CLOCK     XTAL_9_828MHz
#define Z80_MASTER_CLOCK         XTAL_10_816MHz


class cosmic_state : public driver_device
{
public:
	cosmic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *        m_videoram;
	UINT8 *        m_spriteram;
	size_t         m_videoram_size;
	size_t         m_spriteram_size;

	/* video-related */
	pen_t          (*m_map_color)(running_machine &machine, UINT8 x, UINT8 y);
	int            m_color_registers[3];
	int            m_background_enable;
	int            m_magspot_pen_mask;

	/* sound-related */
	int            m_sound_enabled;
	int            m_march_select;
	int            m_gun_die_select;
	int            m_dive_bomb_b_select;

	/* misc */
	UINT32         m_pixel_clock;

	/* devices */
	samples_device *m_samples;
	device_t *m_dac;
};


/*----------- defined in video/cosmic.c -----------*/

WRITE8_HANDLER( cosmic_color_register_w );
WRITE8_HANDLER( cosmic_background_enable_w );

PALETTE_INIT( panic );
PALETTE_INIT( cosmica );
PALETTE_INIT( cosmicg );
PALETTE_INIT( magspot );
PALETTE_INIT( nomnlnd );

SCREEN_UPDATE_IND16( panic );
SCREEN_UPDATE_IND16( magspot );
SCREEN_UPDATE_IND16( devzone );
SCREEN_UPDATE_IND16( cosmica );
SCREEN_UPDATE_IND16( cosmicg );
SCREEN_UPDATE_IND16( nomnlnd );
