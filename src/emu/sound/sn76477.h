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

#pragma once

#ifndef __SN76477_H__
#define __SN76477_H__

#include "machine/rescap.h"



/*****************************************************************************
 *
 *  Interface definition
 *
 *****************************************************************************/

struct sn76477_interface
{
	double m_intf_noise_clock_res;
	double m_intf_noise_filter_res;
	double m_intf_noise_filter_cap;
	double m_intf_decay_res;
	double m_intf_attack_decay_cap;
	double m_intf_attack_res;
	double m_intf_amplitude_res;
	double m_intf_feedback_res;
	double m_intf_vco_voltage;
	double m_intf_vco_cap;
	double m_intf_vco_res;
	double m_intf_pitch_voltage;
	double m_intf_slf_res;
	double m_intf_slf_cap;
	double m_intf_one_shot_cap;
	double m_intf_one_shot_res;
	UINT32 m_intf_vco;
	UINT32 m_intf_mixer_a;
	UINT32 m_intf_mixer_b;
	UINT32 m_intf_mixer_c;
	UINT32 m_intf_envelope_1;
	UINT32 m_intf_envelope_2;
	UINT32 m_intf_enable;
};

class sn76477_device : public device_t,
									public device_sound_interface,
									public sn76477_interface
{
public:
	sn76477_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~sn76477_device() {}

	/* these functions take 0 or 1 as a logic input */
	WRITE_LINE_MEMBER( enable_w );      /* active LO, 0 = enabled, 1 = disabled */
	WRITE_LINE_MEMBER( mixer_a_w );
	WRITE_LINE_MEMBER( mixer_b_w );
	WRITE_LINE_MEMBER( mixer_c_w );
	WRITE_LINE_MEMBER( envelope_1_w );
	WRITE_LINE_MEMBER( envelope_2_w );
	WRITE_LINE_MEMBER( vco_w );         /* 0 = external, 1 = controlled by SLF */
	WRITE_LINE_MEMBER( noise_clock_w ); /* noise clock write, if noise_clock_res = 0 */

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
	#define SN76477_EXTERNAL_VOLTAGE_DISCONNECT   (-1.0)    /* indicates that the voltage is internally computed,
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
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	/* chip's external interface */
	UINT32 m_enable;
	UINT32 m_envelope_mode;
	UINT32 m_vco_mode;
	UINT32 m_mixer_mode;

	double m_one_shot_res;
	double m_one_shot_cap;
	UINT32 m_one_shot_cap_voltage_ext;

	double m_slf_res;
	double m_slf_cap;
	UINT32 m_slf_cap_voltage_ext;

	double m_vco_voltage;
	double m_vco_res;
	double m_vco_cap;
	UINT32 m_vco_cap_voltage_ext;

	double m_noise_clock_res;
	UINT32 m_noise_clock_ext;
	UINT32 m_noise_clock;
	double m_noise_filter_res;
	double m_noise_filter_cap;
	UINT32 m_noise_filter_cap_voltage_ext;

	double m_attack_res;
	double m_decay_res;
	double m_attack_decay_cap;
	UINT32 m_attack_decay_cap_voltage_ext;

	double m_amplitude_res;
	double m_feedback_res;
	double m_pitch_voltage;

	// internal state
	double m_one_shot_cap_voltage;        /* voltage on the one-shot cap */
	UINT32 m_one_shot_running_ff;         /* 1 = one-shot running, 0 = stopped */

	double m_slf_cap_voltage;             /* voltage on the SLF cap */
	UINT32 m_slf_out_ff;                  /* output of the SLF */

	double m_vco_cap_voltage;             /* voltage on the VCO cap */
	UINT32 m_vco_out_ff;                  /* output of the VCO */
	UINT32 m_vco_alt_pos_edge_ff;         /* keeps track of the # of positive edges for VCO Alt envelope */

	double m_noise_filter_cap_voltage;    /* voltage on the noise filter cap */
	UINT32 m_real_noise_bit_ff;           /* the current noise bit before filtering */
	UINT32 m_filtered_noise_bit_ff;       /* the noise bit after filtering */
	UINT32 m_noise_gen_count;             /* noise freq emulation */

	double m_attack_decay_cap_voltage;    /* voltage on the attack/decay cap */

	UINT32 m_rng;                         /* current value of the random number generator */

	/* others */
	sound_stream *m_channel;              /* returned by stream_create() */
	int m_our_sample_rate;                    /* from machine.sample_rate() */

	wav_file *m_file;                     /* handle of the wave file to produce */

	double compute_one_shot_cap_charging_rate();
	double compute_one_shot_cap_discharging_rate();
	double compute_slf_cap_charging_rate();
	double compute_slf_cap_discharging_rate();
	double compute_vco_cap_charging_discharging_rate();
	double compute_vco_duty_cycle();
	UINT32 compute_noise_gen_freq();
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
	void add_wav_data(INT16 data_l, INT16 data_r);

	void intialize_noise();
	inline UINT32 generate_next_real_noise_bit();

	void state_save_register();

	void _SN76477_enable_w(UINT32 data);
	void _SN76477_vco_w(UINT32 data);
	void _SN76477_mixer_a_w(UINT32 data);
	void _SN76477_mixer_b_w(UINT32 data);
	void _SN76477_mixer_c_w(UINT32 data);
	void _SN76477_envelope_1_w(UINT32 data);
	void _SN76477_envelope_2_w(UINT32 data);
	void _SN76477_one_shot_res_w(double data);
	void _SN76477_one_shot_cap_w(double data);
	void _SN76477_slf_res_w(double data);
	void _SN76477_slf_cap_w(double data);
	void _SN76477_vco_res_w(double data);
	void _SN76477_vco_cap_w(double data);
	void _SN76477_vco_voltage_w(double data);
	void _SN76477_noise_clock_res_w(double data);
	void _SN76477_noise_filter_res_w(double data);
	void _SN76477_noise_filter_cap_w(double data);
	void _SN76477_decay_res_w(double data);
	void _SN76477_attack_res_w(double data);
	void _SN76477_attack_decay_cap_w(double data);
	void _SN76477_amplitude_res_w(double data);
	void _SN76477_feedback_res_w(double data);
	void _SN76477_pitch_voltage_w(double data);
	void SN76477_test_enable_w(UINT32 data);
};

extern const device_type SN76477;


#endif/* __SN76477_H__ */
