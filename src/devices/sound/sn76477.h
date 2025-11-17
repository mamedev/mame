// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*****************************************************************************

    Texas Instruments SN76477 emulator

    SN76477 pin layout. There is a corresponding interface variable with the
    same name. The only exception is noise_clock which must be programmatically
    set.  The other pins have programmatic equivalents as well.
    The name of the function is SN76477_<pinname>_w.
    All capacitor functions can also specify a fixed voltage on the cap.
    The name of this function is SN76477_<pinname>_voltage_w

                      +-------------------+
          envelope_1  | 1      | |      28|  envelope_2
                      | 2 GND   -       27|  mixer_c
         noise_clock  | 3               26|  mixer_a
     noise_clock_res  | 4               25|  mixer_b
    noise_filter_res  | 5               24|  one_shot_res
    noise_filter_cap  | 6               23|  one_shot_cap
           decay_res  | 7               22|  vco
    attack_decay_cap  | 8               21|  slf_cap
              enable o| 9               20|  slf_res
          attack_res  |10               19|  pitch_voltage
       amplitude_res  |11               18|  vco_res
        feedback_res  |12               17|  vco_cap
                      |13 OUTPUT        16|  vco_voltage
                      |14 Vcc   +5V OUT 15|
                      +-------------------+

    All resistor values in Ohms
    All capacitor values in Farads
    Use RES_K, RES_M and CAP_U, CAP_N, CAP_P macros in rescap.h to convert
    magnitudes, eg. 220k = RES_K(220), 47nF = CAP_N(47)

 *****************************************************************************/

#ifndef MAME_SOUND_SN76477_H
#define MAME_SOUND_SN76477_H

#pragma once

#include "machine/rescap.h"

#include "wavwrite.h"


/*****************************************************************************
 *
 *  Interface definition
 *
 *****************************************************************************/

class sn76477_device : public device_t,
						public device_sound_interface
{
public:
	sn76477_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_noise_params(double clock_res, double filter_res, double filter_cap)
	{
		m_noise_clock_res = clock_res;
		m_noise_filter_res = filter_res;
		m_noise_filter_cap = filter_cap;
	}
	void set_decay_res(double decay_res) { m_decay_res = decay_res; }
	void set_attack_params(double decay_cap, double res)
	{
		m_attack_decay_cap = decay_cap;
		m_attack_res = res;
	}
	void set_amp_res(double amp_res) { m_amplitude_res = amp_res; }
	void set_feedback_res(double feedback_res) { m_feedback_res = feedback_res; }
	void set_vco_params(double volt, double cap, double res)
	{
		m_vco_voltage = volt;
		m_vco_cap = cap;
		m_vco_res = res;
	}
	void set_pitch_voltage(double volt) { m_pitch_voltage = volt; }
	void set_slf_params(double cap, double res)
	{
		m_slf_cap = cap;
		m_slf_res = res;
	}
	void set_oneshot_params(double cap, double res)
	{
		m_one_shot_cap = cap;
		m_one_shot_res = res;
	}
	void set_vco_mode(uint32_t mode) { m_vco_mode = mode; }
	void set_mixer_params(uint32_t a, uint32_t b, uint32_t c)
	{
		m_mixer_a = a;
		m_mixer_b = b;
		m_mixer_c = c;
	}
	void set_envelope_params(uint32_t env1, uint32_t env2)
	{
		m_envelope_1 = env1;
		m_envelope_2 = env2;
	}
	void set_enable(uint32_t enable) { m_enable = enable; }


	/* these functions take 0 or 1 as a logic input */
	void enable_w(int state);      /* active LO, 0 = enabled, 1 = disabled */
	void mixer_a_w(int state);
	void mixer_b_w(int state);
	void mixer_c_w(int state);
	void envelope_1_w(int state);
	void envelope_2_w(int state);
	void vco_w(int state);         /* 0 = external, 1 = controlled by SLF */
	void noise_clock_w(int state); /* noise clock write, if noise_clock_res = 0 */

	/* these functions take a resistor value in Ohms */
	void one_shot_res_w(double data);
	void slf_res_w(double data);
	void vco_res_w(double data);
	void noise_clock_res_w(double data);  /* = 0 if the noise gen is clocked via noise_clock */
	void noise_filter_res_w(double data);
	void decay_res_w(double data);
	void attack_res_w(double data);
	void amplitude_res_w(double data);
	void feedback_res_w(double data);

	/* these functions take a capacitor value in Farads or the voltage on it in Volts */
	static constexpr double EXTERNAL_VOLTAGE_DISCONNECT = -1.0;  /* indicates that the voltage is internally computed,
	                                                           can be used in all the functions that take a
	                                                           voltage on a capacitor */
	void one_shot_cap_w(double data);
	void one_shot_cap_voltage_w(double data);
	void slf_cap_w(double data);
	void slf_cap_voltage_w(double data);
	void vco_cap_w(double data);
	void vco_cap_voltage_w(double data);
	void noise_filter_cap_w(double data);
	void noise_filter_cap_voltage_w(double data);
	void attack_decay_cap_w(double data);
	void attack_decay_cap_voltage_w(double data);

	/* these functions take a voltage value in Volts */
	void vco_voltage_w(double data);
	void pitch_voltage_w(double data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	/* chip's external interface */
	uint32_t m_enable;
	uint32_t m_envelope_mode;
	uint32_t m_vco_mode;
	uint32_t m_mixer_mode;

	double m_one_shot_res;
	double m_one_shot_cap;
	uint32_t m_one_shot_cap_voltage_ext;

	double m_slf_res;
	double m_slf_cap;
	uint32_t m_slf_cap_voltage_ext;

	double m_vco_voltage;
	double m_vco_res;
	double m_vco_cap;
	uint32_t m_vco_cap_voltage_ext;

	double m_noise_clock_res;
	uint32_t m_noise_clock_ext;
	uint32_t m_noise_clock;
	double m_noise_filter_res;
	double m_noise_filter_cap;
	uint32_t m_noise_filter_cap_voltage_ext;

	double m_attack_res;
	double m_decay_res;
	double m_attack_decay_cap;
	uint32_t m_attack_decay_cap_voltage_ext;

	double m_amplitude_res;
	double m_feedback_res;
	double m_pitch_voltage;

	// internal state
	double m_one_shot_cap_voltage;        // voltage on the one-shot cap
	uint32_t m_one_shot_running_ff;       // 1 = one-shot running, 0 = stopped

	double m_slf_cap_voltage;             // voltage on the SLF cap
	uint32_t m_slf_out_ff;                // output of the SLF

	double m_vco_cap_voltage;             // voltage on the VCO cap
	uint32_t m_vco_out_ff;                // output of the VCO
	uint32_t m_vco_alt_pos_edge_ff;       // keeps track of the # of positive edges for VCO Alt envelope

	double m_noise_filter_cap_voltage;    // voltage on the noise filter cap
	uint32_t m_real_noise_bit_ff;         // the current noise bit before filtering
	uint32_t m_filtered_noise_bit_ff;     // the noise bit after filtering
	uint32_t m_noise_gen_count;           // noise freq emulation

	double m_attack_decay_cap_voltage;    // voltage on the attack/decay cap

	uint32_t m_rng;                       // current value of the random number generator

	// configured by the drivers and used to setup m_mixer_mode & m_envelope_mode at start
	uint32_t m_mixer_a;
	uint32_t m_mixer_b;
	uint32_t m_mixer_c;
	uint32_t m_envelope_1;
	uint32_t m_envelope_2;

	/* others */
	sound_stream *m_channel;              // returned by stream_create()
	int m_our_sample_rate;                // from machine.sample_rate()

	util::wav_file_ptr m_file;            // handle of the wave file to produce

	double compute_one_shot_cap_charging_rate();
	double compute_one_shot_cap_discharging_rate();
	double compute_slf_cap_charging_rate();
	double compute_slf_cap_discharging_rate();
	double compute_vco_cap_charging_discharging_rate();
	double compute_vco_duty_cycle();
	uint32_t compute_noise_gen_freq();
	double compute_noise_filter_cap_charging_rate();
	double compute_noise_filter_cap_discharging_rate();
	double compute_attack_decay_cap_charging_rate();
	double compute_attack_decay_cap_discharging_rate();
	double compute_center_to_peak_voltage_out();

	void log_enable_line();
	void log_mixer_mode();
	void log_envelope_mode();
	void log_vco_mode();
	void log_one_shot_time();
	void log_slf_freq();
	void log_vco_pitch_voltage();
	void log_vco_duty_cycle();
	void log_vco_freq();
	void log_vco_ext_voltage();
	void log_noise_gen_freq();
	void log_noise_filter_freq();
	void log_attack_time();
	void log_decay_time();
	void log_voltage_out();
	void log_complete_state();

	void open_wav_file();
	void close_wav_file();
	void add_wav_data(int16_t data_l, int16_t data_r);

	void intialize_noise();
	inline uint32_t generate_next_real_noise_bit();

	void state_save_register();
};

DECLARE_DEVICE_TYPE(SN76477, sn76477_device)

#endif // MAME_SOUND_SN76477_H
