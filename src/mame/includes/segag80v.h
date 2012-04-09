/*************************************************************************

    Sega vector hardware

*************************************************************************/

#include "machine/segag80.h"

class segag80v_state : public driver_device
{
public:
	segag80v_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_mainram;
	device_t *m_usb;
	UINT8 m_mult_data[2];
	UINT16 m_mult_result;
	UINT8 m_spinner_select;
	UINT8 m_spinner_sign;
	UINT8 m_spinner_count;
	segag80_decrypt_func m_decrypt;
	UINT8 *m_vectorram;
	size_t m_vectorram_size;
	int m_min_x;
	int m_min_y;
	DECLARE_WRITE8_MEMBER(mainram_w);
	DECLARE_WRITE8_MEMBER(vectorram_w);
	DECLARE_READ8_MEMBER(mangled_ports_r);
	DECLARE_WRITE8_MEMBER(spinner_select_w);
	DECLARE_READ8_MEMBER(spinner_input_r);
	DECLARE_READ8_MEMBER(elim4_input_r);
	DECLARE_WRITE8_MEMBER(multiply_w);
	DECLARE_READ8_MEMBER(multiply_r);
	DECLARE_WRITE8_MEMBER(coin_count_w);
	DECLARE_WRITE8_MEMBER(unknown_w);
	DECLARE_CUSTOM_INPUT_MEMBER(elim4_joint_coin_r);
};


/*----------- defined in audio/segag80v.c -----------*/

WRITE8_HANDLER( elim1_sh_w );
WRITE8_HANDLER( elim2_sh_w );
WRITE8_HANDLER( spacfury1_sh_w );
WRITE8_HANDLER( spacfury2_sh_w );
WRITE8_HANDLER( zektor1_sh_w );
WRITE8_HANDLER( zektor2_sh_w );


/*----------- defined in video/segag80v.c -----------*/

VIDEO_START( segag80v );
SCREEN_UPDATE_RGB32( segag80v );
