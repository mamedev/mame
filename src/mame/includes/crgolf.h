/*************************************************************************

    Kitco Crowns Golf hardware

**************************************************************************/

#define MASTER_CLOCK		18432000


class crgolf_state : public driver_device
{
public:
	crgolf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_color_select(*this, "color_select"),
		m_screen_flip(*this, "screen_flip"),
		m_screen_select(*this, "screen_select"),
		m_screenb_enable(*this, "screenb_enable"),
		m_screena_enable(*this, "screena_enable"){ }

	/* memory pointers */
	UINT8 *  m_videoram_a;
	UINT8 *  m_videoram_b;
	required_shared_ptr<UINT8> m_color_select;
	required_shared_ptr<UINT8> m_screen_flip;
	required_shared_ptr<UINT8> m_screen_select;
	required_shared_ptr<UINT8> m_screenb_enable;
	required_shared_ptr<UINT8> m_screena_enable;

	/* misc */
	UINT8    m_port_select;
	UINT8    m_main_to_sound_data;
	UINT8    m_sound_to_main_data;
	UINT16   m_sample_offset;
	UINT8    m_sample_count;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	DECLARE_WRITE8_MEMBER(rom_bank_select_w);
	DECLARE_READ8_MEMBER(switch_input_r);
	DECLARE_READ8_MEMBER(analog_input_r);
	DECLARE_WRITE8_MEMBER(switch_input_select_w);
	DECLARE_WRITE8_MEMBER(unknown_w);
	DECLARE_WRITE8_MEMBER(main_to_sound_w);
	DECLARE_READ8_MEMBER(main_to_sound_r);
	DECLARE_WRITE8_MEMBER(sound_to_main_w);
	DECLARE_READ8_MEMBER(sound_to_main_r);
	DECLARE_WRITE8_MEMBER(crgolf_videoram_w);
	DECLARE_READ8_MEMBER(crgolf_videoram_r);
	DECLARE_WRITE8_MEMBER(crgolfhi_sample_w);
};

/*----------- defined in video/crgolf.c -----------*/


MACHINE_CONFIG_EXTERN( crgolf_video );
