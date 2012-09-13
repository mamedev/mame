/*************************************************************************

    Cosmic Guerilla & other Universal boards (in cosmic.c)

*************************************************************************/

#include "sound/samples.h"
#include "sound/dac.h"

#define COSMICG_MASTER_CLOCK     XTAL_9_828MHz
#define Z80_MASTER_CLOCK         XTAL_10_816MHz


class cosmic_state : public driver_device
{
public:
	cosmic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	optional_shared_ptr<UINT8> m_spriteram;

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
	dac_device *m_dac;
	DECLARE_WRITE8_MEMBER(panic_sound_output_w);
	DECLARE_WRITE8_MEMBER(panic_sound_output2_w);
	DECLARE_WRITE8_MEMBER(cosmicg_output_w);
	DECLARE_WRITE8_MEMBER(cosmica_sound_output_w);
	DECLARE_READ8_MEMBER(cosmica_pixel_clock_r);
	DECLARE_READ8_MEMBER(cosmicg_port_0_r);
	DECLARE_READ8_MEMBER(magspot_coinage_dip_r);
	DECLARE_READ8_MEMBER(nomnlnd_port_0_1_r);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(cosmic_color_register_w);
	DECLARE_WRITE8_MEMBER(cosmic_background_enable_w);
	DECLARE_INPUT_CHANGED_MEMBER(panic_coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(cosmica_coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(cosmicg_coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_irq0);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_nmi);
	DECLARE_DRIVER_INIT(devzone);
	DECLARE_DRIVER_INIT(cosmicg);
	DECLARE_DRIVER_INIT(nomnlnd);
	DECLARE_DRIVER_INIT(cosmica);
	DECLARE_DRIVER_INIT(panic);
	DECLARE_MACHINE_START(cosmic);
	DECLARE_MACHINE_RESET(cosmic);
	DECLARE_PALETTE_INIT(cosmicg);
	DECLARE_PALETTE_INIT(panic);
	DECLARE_PALETTE_INIT(cosmica);
	DECLARE_PALETTE_INIT(magspot);
	DECLARE_PALETTE_INIT(nomnlnd);
};


/*----------- defined in video/cosmic.c -----------*/








SCREEN_UPDATE_IND16( panic );
SCREEN_UPDATE_IND16( magspot );
SCREEN_UPDATE_IND16( devzone );
SCREEN_UPDATE_IND16( cosmica );
SCREEN_UPDATE_IND16( cosmicg );
SCREEN_UPDATE_IND16( nomnlnd );
