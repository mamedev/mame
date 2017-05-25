// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Cinematronics vector hardware

*************************************************************************/

#include "cpu/ccpu/ccpu.h"
#include "sound/ay8910.h"
#include "sound/samples.h"
#include "video/vector.h"
#include "screen.h"

class cinemat_state : public driver_device
{
public:
	cinemat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ay1(*this, "ay1")
		, m_samples(*this, "samples")
		, m_vector(*this, "vector")
		, m_screen(*this, "screen")
		, m_rambase(*this, "rambase")
		, m_analog_x(*this, "ANALOGX")
		, m_analog_y(*this, "ANALOGY")
	{ }

	required_device<ccpu_cpu_device> m_maincpu;
	optional_device<ay8910_device> m_ay1;
	optional_device<samples_device> m_samples;
	required_device<vector_device> m_vector;
	required_device<screen_device> m_screen;
	optional_shared_ptr<uint16_t> m_rambase;

	optional_ioport m_analog_x;
	optional_ioport m_analog_y;

	typedef void (cinemat_state::*sound_func)(uint8_t sound_val, uint8_t bits_changed);

	sound_func m_sound_handler;
	uint8_t m_sound_control;
	uint32_t m_current_shift;
	uint32_t m_last_shift;
	uint32_t m_last_shift2;
	uint32_t m_current_pitch;
	uint32_t m_last_frame;
	uint8_t m_sound_fifo[16];
	uint8_t m_sound_fifo_in;
	uint8_t m_sound_fifo_out;
	uint8_t m_last_portb_write;
	float m_target_volume;
	float m_current_volume;
	uint8_t m_coin_detected;
	uint8_t m_coin_last_reset;
	uint8_t m_mux_select;
	int m_gear;
	int m_color_mode;
	rgb_t m_vector_color;
	int16_t m_lastx;
	int16_t m_lasty;
	uint8_t m_last_control;
	int m_qb3_lastx;
	int m_qb3_lasty;
	DECLARE_READ8_MEMBER(inputs_r);
	DECLARE_READ8_MEMBER(switches_r);
	DECLARE_READ8_MEMBER(coin_input_r);
	DECLARE_WRITE8_MEMBER(coin_reset_w);
	DECLARE_WRITE8_MEMBER(mux_select_w);
	DECLARE_READ8_MEMBER(speedfrk_wheel_r);
	DECLARE_READ8_MEMBER(speedfrk_gear_r);
	DECLARE_READ8_MEMBER(sundance_inputs_r);
	DECLARE_READ8_MEMBER(boxingb_dial_r);
	DECLARE_READ8_MEMBER(qb3_frame_r);
	DECLARE_WRITE8_MEMBER(qb3_ram_bank_w);
	DECLARE_WRITE8_MEMBER(cinemat_vector_control_w);
	DECLARE_WRITE8_MEMBER(cinemat_sound_control_w);
	DECLARE_WRITE8_MEMBER(qb3_sound_w);
	DECLARE_READ8_MEMBER(joystick_read);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_DRIVER_INIT(speedfrk);
	DECLARE_DRIVER_INIT(boxingb);
	DECLARE_DRIVER_INIT(tailg);
	DECLARE_DRIVER_INIT(sundance);
	DECLARE_DRIVER_INIT(qb3);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void sound_start() override;
	virtual void video_start() override;
	DECLARE_SOUND_RESET(spacewar);
	DECLARE_SOUND_RESET(barrier);
	DECLARE_SOUND_RESET(speedfrk);
	DECLARE_SOUND_RESET(starhawk);
	DECLARE_SOUND_RESET(sundance);
	DECLARE_SOUND_RESET(tailg);
	DECLARE_SOUND_RESET(warrior);
	DECLARE_SOUND_RESET(armora);
	DECLARE_SOUND_RESET(ripoff);
	DECLARE_SOUND_RESET(starcas);
	DECLARE_SOUND_RESET(solarq);
	DECLARE_SOUND_RESET(boxingb);
	DECLARE_SOUND_RESET(wotw);
	DECLARE_SOUND_RESET(demon);
	DECLARE_SOUND_RESET(qb3);
	DECLARE_VIDEO_START(cinemat_16level);
	DECLARE_VIDEO_START(cinemat_64level);
	DECLARE_VIDEO_START(cinemat_color);
	DECLARE_VIDEO_START(cinemat_qb3color);
	uint32_t screen_update_cinemat(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spacewar(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_READ8_MEMBER(sound_porta_r);
	DECLARE_READ8_MEMBER(sound_portb_r);
	DECLARE_WRITE8_MEMBER(sound_portb_w);
	DECLARE_WRITE8_MEMBER(sound_output_w);
	TIMER_CALLBACK_MEMBER(synced_sound_w);
	void generic_init(sound_func sound_handler);
	void cinemat_vector_callback(int16_t sx, int16_t sy, int16_t ex, int16_t ey, uint8_t shift);
	void spacewar_sound_w(uint8_t sound_val, uint8_t bits_changed);
	void barrier_sound_w(uint8_t sound_val, uint8_t bits_changed);
	void speedfrk_sound_w(uint8_t sound_val, uint8_t bits_changed);
	void starhawk_sound_w(uint8_t sound_val, uint8_t bits_changed);
	void sundance_sound_w(uint8_t sound_val, uint8_t bits_changed);
	void tailg_sound_w(uint8_t sound_val, uint8_t bits_changed);
	void warrior_sound_w(uint8_t sound_val, uint8_t bits_changed);
	void armora_sound_w(uint8_t sound_val, uint8_t bits_changed);
	void ripoff_sound_w(uint8_t sound_val, uint8_t bits_changed);
	void starcas_sound_w(uint8_t sound_val, uint8_t bits_changed);
	void solarq_sound_w(uint8_t sound_val, uint8_t bits_changed);
	void boxingb_sound_w(uint8_t sound_val, uint8_t bits_changed);
	void wotw_sound_w(uint8_t sound_val, uint8_t bits_changed);
	void demon_sound_w(uint8_t sound_val, uint8_t bits_changed);
};

/*----------- defined in audio/cinemat.c -----------*/
MACHINE_CONFIG_EXTERN( spacewar_sound );
MACHINE_CONFIG_EXTERN( barrier_sound );
MACHINE_CONFIG_EXTERN( speedfrk_sound );
MACHINE_CONFIG_EXTERN( starhawk_sound );
MACHINE_CONFIG_EXTERN( sundance_sound );
MACHINE_CONFIG_EXTERN( tailg_sound );
MACHINE_CONFIG_EXTERN( warrior_sound );
MACHINE_CONFIG_EXTERN( armora_sound );
MACHINE_CONFIG_EXTERN( ripoff_sound );
MACHINE_CONFIG_EXTERN( starcas_sound );
MACHINE_CONFIG_EXTERN( solarq_sound );
MACHINE_CONFIG_EXTERN( boxingb_sound );
MACHINE_CONFIG_EXTERN( wotw_sound );
MACHINE_CONFIG_EXTERN( demon_sound );
MACHINE_CONFIG_EXTERN( qb3_sound );
