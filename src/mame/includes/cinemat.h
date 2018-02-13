// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Cinematronics vector hardware

*************************************************************************/

#include "cpu/ccpu/ccpu.h"
#include "machine/74259.h"
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
		, m_outlatch(*this, "outlatch")
		, m_samples(*this, "samples")
		, m_vector(*this, "vector")
		, m_screen(*this, "screen")
		, m_rambase(*this, "rambase")
		, m_analog_x(*this, "ANALOGX")
		, m_analog_y(*this, "ANALOGY")
	{ }

	required_device<ccpu_cpu_device> m_maincpu;
	optional_device<ay8910_device> m_ay1;
	required_device<ls259_device> m_outlatch;
	optional_device<samples_device> m_samples;
	required_device<vector_device> m_vector;
	required_device<screen_device> m_screen;
	optional_shared_ptr<uint16_t> m_rambase;

	optional_ioport m_analog_x;
	optional_ioport m_analog_y;

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
	WRITE_LINE_MEMBER(coin_reset_w);
	WRITE_LINE_MEMBER(mux_select_w);
	DECLARE_READ8_MEMBER(speedfrk_wheel_r);
	DECLARE_READ8_MEMBER(speedfrk_gear_r);
	DECLARE_READ8_MEMBER(sundance_inputs_r);
	DECLARE_READ8_MEMBER(boxingb_dial_r);
	DECLARE_READ8_MEMBER(qb3_frame_r);
	DECLARE_WRITE8_MEMBER(qb3_ram_bank_w);
	DECLARE_WRITE_LINE_MEMBER(vector_control_w);
	DECLARE_READ8_MEMBER(joystick_read);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_DRIVER_INIT(speedfrk);
	DECLARE_DRIVER_INIT(boxingb);
	DECLARE_DRIVER_INIT(sundance);
	DECLARE_DRIVER_INIT(qb3);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void sound_start() override;
	virtual void sound_reset() override;
	virtual void video_start() override;
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
	void cinemat_vector_callback(int16_t sx, int16_t sy, int16_t ex, int16_t ey, uint8_t shift);
	DECLARE_WRITE_LINE_MEMBER(spacewar_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(spacewar_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(spacewar_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(spacewar_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(spacewar_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(barrier_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(barrier_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(barrier_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(speedfrk_start_led_w);
	DECLARE_WRITE_LINE_MEMBER(speedfrk_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(speedfrk_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(starhawk_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(starhawk_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(starhawk_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(starhawk_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(starhawk_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(starhawk_sound7_w);
	DECLARE_WRITE_LINE_MEMBER(sundance_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(sundance_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(sundance_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(sundance_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(sundance_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(sundance_sound7_w);
	DECLARE_WRITE_LINE_MEMBER(tailg_sound_w);
	DECLARE_WRITE_LINE_MEMBER(warrior_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(warrior_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(warrior_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(warrior_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(warrior_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(armora_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(armora_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(armora_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(armora_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(armora_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(ripoff_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(ripoff_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(ripoff_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(ripoff_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(ripoff_sound7_w);
	DECLARE_WRITE_LINE_MEMBER(starcas_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(starcas_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(starcas_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(starcas_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(starcas_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(solarq_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(solarq_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(solarq_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(boxingb_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(boxingb_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(boxingb_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(boxingb_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(boxingb_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(wotw_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(wotw_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(wotw_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(wotw_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(wotw_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(demon_sound4_w);
	DECLARE_WRITE8_MEMBER(qb3_sound_fifo_w);
	void cinemat_nojmi_4k(machine_config &config);
	void cinemat_jmi_4k(machine_config &config);
	void cinemat_nojmi_8k(machine_config &config);
	void cinemat_jmi_8k(machine_config &config);
	void cinemat_jmi_16k(machine_config &config);
	void cinemat_jmi_32k(machine_config &config);
	void qb3(machine_config &config);
	void ripoff(machine_config &config);
	void demon(machine_config &config);
	void wotwc(machine_config &config);
	void wotw(machine_config &config);
	void boxingb(machine_config &config);
	void speedfrk(machine_config &config);
	void sundance(machine_config &config);
	void starcas(machine_config &config);
	void spacewar(machine_config &config);
	void solarq(machine_config &config);
	void tailg(machine_config &config);
	void warrior(machine_config &config);
	void starhawk(machine_config &config);
	void barrier(machine_config &config);
	void armora(machine_config &config);
	void spacewar_sound(machine_config &config);
	void barrier_sound(machine_config &config);
	void speedfrk_sound(machine_config &config);
	void starhawk_sound(machine_config &config);
	void sundance_sound(machine_config &config);
	void tailg_sound(machine_config &config);
	void warrior_sound(machine_config &config);
	void armora_sound(machine_config &config);
	void ripoff_sound(machine_config &config);
	void starcas_sound(machine_config &config);
	void solarq_sound(machine_config &config);
	void boxingb_sound(machine_config &config);
	void wotw_sound(machine_config &config);
	void demon_sound(machine_config &config);
	void qb3_sound(machine_config &config);
	void data_map(address_map &map);
	void data_map_qb3(address_map &map);
	void demon_sound_map(address_map &map);
	void demon_sound_ports(address_map &map);
	void io_map(address_map &map);
	void io_map_qb3(address_map &map);
	void program_map_16k(address_map &map);
	void program_map_32k(address_map &map);
	void program_map_4k(address_map &map);
	void program_map_8k(address_map &map);
};
