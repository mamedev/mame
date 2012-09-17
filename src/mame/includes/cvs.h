/***************************************************************************

    Century CVS System

****************************************************************************/

#include "sound/dac.h"

#define CVS_S2636_Y_OFFSET     (3)
#define CVS_S2636_X_OFFSET     (-26)
#define CVS_MAX_STARS          250

struct cvs_star
{
	int x, y, code;
};

class cvs_state : public driver_device
{
public:
	cvs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_video_ram(*this, "video_ram"),
		  m_bullet_ram(*this, "bullet_ram"),
		  m_fo_state(*this, "fo_state"),
		  m_cvs_4_bit_dac_data(*this, "4bit_dac"),
		  m_tms5110_ctl_data(*this, "tms5110_ctl"),
		  m_dac3_state(*this, "dac3_state") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_video_ram;
	required_shared_ptr<UINT8> m_bullet_ram;
	required_shared_ptr<UINT8> m_fo_state;
	optional_shared_ptr<UINT8> m_cvs_4_bit_dac_data;
	optional_shared_ptr<UINT8> m_tms5110_ctl_data;
	optional_shared_ptr<UINT8> m_dac3_state;

	/* video-related */
	struct cvs_star m_stars[CVS_MAX_STARS];
	bitmap_ind16   m_collision_background;
	bitmap_ind16   m_background_bitmap;
	bitmap_ind16   m_scrolled_collision_background;
	int        m_collision_register;
	int        m_total_stars;
	int        m_stars_on;
	UINT8      m_scroll_reg;
	int        m_stars_scroll;

	/* misc */
	emu_timer  *m_cvs_393hz_timer;
	UINT8      m_cvs_393hz_clock;

	UINT8      m_character_banking_mode;
	UINT16     m_character_ram_page_start;
	UINT16     m_speech_rom_bit_address;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	device_t *m_speech;
	dac_device *m_dac3;
	device_t *m_tms;
	device_t *m_s2636_0;
	device_t *m_s2636_1;
	device_t *m_s2636_2;

	/* memory */
	UINT8      m_color_ram[0x400];
	UINT8      m_palette_ram[0x10];
	UINT8      m_character_ram[3 * 0x800];	/* only half is used, but
                                               by allocating twice the amount,
                                               we can use the same gfx_layout */
	DECLARE_READ8_MEMBER(cvs_input_r);
	DECLARE_READ8_MEMBER(cvs_393hz_clock_r);
	DECLARE_WRITE8_MEMBER(cvs_speech_rom_address_lo_w);
	DECLARE_WRITE8_MEMBER(cvs_speech_rom_address_hi_w);
	DECLARE_READ8_MEMBER(cvs_speech_command_r);
	DECLARE_WRITE8_MEMBER(audio_command_w);
	DECLARE_READ8_MEMBER(cvs_video_or_color_ram_r);
	DECLARE_WRITE8_MEMBER(cvs_video_or_color_ram_w);
	DECLARE_READ8_MEMBER(cvs_bullet_ram_or_palette_r);
	DECLARE_WRITE8_MEMBER(cvs_bullet_ram_or_palette_w);
	DECLARE_READ8_MEMBER(cvs_s2636_0_or_character_ram_r);
	DECLARE_WRITE8_MEMBER(cvs_s2636_0_or_character_ram_w);
	DECLARE_READ8_MEMBER(cvs_s2636_1_or_character_ram_r);
	DECLARE_WRITE8_MEMBER(cvs_s2636_1_or_character_ram_w);
	DECLARE_READ8_MEMBER(cvs_s2636_2_or_character_ram_r);
	DECLARE_WRITE8_MEMBER(cvs_s2636_2_or_character_ram_w);
	DECLARE_WRITE8_MEMBER(cvs_video_fx_w);
	DECLARE_READ8_MEMBER(cvs_collision_r);
	DECLARE_READ8_MEMBER(cvs_collision_clear);
	DECLARE_WRITE8_MEMBER(cvs_scroll_w);
	DECLARE_READ8_MEMBER(tms_clock_r);
	DECLARE_WRITE8_MEMBER(cvs_4_bit_dac_data_w);
	DECLARE_WRITE8_MEMBER(cvs_unknown_w);
	DECLARE_WRITE8_MEMBER(cvs_tms5110_ctl_w);
	DECLARE_WRITE8_MEMBER(cvs_tms5110_pdc_w);
	DECLARE_DRIVER_INIT(raiders);
	DECLARE_DRIVER_INIT(huncholy);
	DECLARE_DRIVER_INIT(hero);
	DECLARE_DRIVER_INIT(superbik);
	DECLARE_DRIVER_INIT(hunchbaka);
	DECLARE_MACHINE_START(cvs);
	DECLARE_MACHINE_RESET(cvs);
	DECLARE_VIDEO_START(cvs);
	DECLARE_PALETTE_INIT(cvs);
	UINT32 screen_update_cvs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in drivers/cvs.c -----------*/




/*----------- defined in video/cvs.c -----------*/



void cvs_init_stars( running_machine &machine );
void cvs_scroll_stars( running_machine &machine );
void cvs_update_stars(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, const pen_t star_pen, bool update_always);




