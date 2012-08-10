/*************************************************************************

    Sega vector hardware

*************************************************************************/

#include "machine/segag80.h"

class segag80v_state : public driver_device
{
public:
	segag80v_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_mainram(*this, "mainram"),
		m_vectorram(*this, "vectorram"){ }

	required_shared_ptr<UINT8> m_mainram;
	device_t *m_usb;
	UINT8 m_mult_data[2];
	UINT16 m_mult_result;
	UINT8 m_spinner_select;
	UINT8 m_spinner_sign;
	UINT8 m_spinner_count;
	segag80_decrypt_func m_decrypt;
	required_shared_ptr<UINT8> m_vectorram;
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
	DECLARE_WRITE8_MEMBER(elim1_sh_w);
	DECLARE_WRITE8_MEMBER(elim2_sh_w);
	DECLARE_WRITE8_MEMBER(zektor1_sh_w);
	DECLARE_WRITE8_MEMBER(zektor2_sh_w);
	DECLARE_WRITE8_MEMBER(spacfury1_sh_w);
	DECLARE_WRITE8_MEMBER(spacfury2_sh_w);
	DECLARE_INPUT_CHANGED_MEMBER(service_switch);
	DECLARE_WRITE8_MEMBER(usb_ram_w);
	DECLARE_DRIVER_INIT(zektor);
	DECLARE_DRIVER_INIT(startrek);
	DECLARE_DRIVER_INIT(elim4);
	DECLARE_DRIVER_INIT(elim2);
	DECLARE_DRIVER_INIT(tacscan);
	DECLARE_DRIVER_INIT(spacfury);
};


/*----------- defined in audio/segag80v.c -----------*/



/*----------- defined in video/segag80v.c -----------*/

VIDEO_START( segag80v );
SCREEN_UPDATE_RGB32( segag80v );
