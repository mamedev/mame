// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    votrax.h

    Votrax SC01A simulation

***************************************************************************/
#ifndef MAME_SOUND_VOTRAX_H
#define MAME_SOUND_VOTRAX_H

#pragma once


class votrax_sc01_device :  public device_t,
							public device_sound_interface
{
public:
	// construction/destruction
	votrax_sc01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto ar_callback() { return m_ar_cb.bind(); }

	void write(uint8_t data);
	void inflection_w(uint8_t data);
	int request() { m_stream->update(); return m_ar_state; }

protected:
	// overridable type for subclass
	votrax_sc01_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	TIMER_CALLBACK_MEMBER(phone_tick);

private:
	// Possible timer parameters
	enum {
		T_COMMIT_PHONE,
		T_END_OF_PHONE
	};

	static const char *const s_phone_table[64];
	static const double s_glottal_wave[9];

	sound_stream *m_stream;                         // Output stream
	emu_timer *m_timer;                             // General timer
	required_memory_region m_rom;                   // Internal ROM
	u32 m_mainclock;                                // Current main clock
	double m_sclock;                                // Stream sample clock (40KHz, main/18)
	double m_cclock;                                // 20KHz capacitor switching clock (main/36)
	u32 m_sample_count;                             // Sample counter, to cadence chip updates

	// Inputs
	u8 m_inflection;                                // 2-bit inflection value
	u8 m_phone;                                     // 6-bit phone value

	// Outputs
	devcb_write_line m_ar_cb;                       // Callback for ar
	bool m_ar_state;                                // Current ar state

	// "Unpacked" current rom values
	u8 m_rom_duration;                              // Duration in 5KHz units (main/144) of one tick, 16 ticks per phone, 7 bits
	u8 m_rom_vd, m_rom_cld;                         // Duration in ticks of the "voice" and "closure" delays, 4 bits
	u8 m_rom_fa, m_rom_fc, m_rom_va;                // Analog parameters, noise volume, noise freq cutoff and voice volume, 4 bits each
	u8 m_rom_f1, m_rom_f2, m_rom_f2q, m_rom_f3;     // Analog parameters, formant frequencies and Q, 4 bits each
	bool m_rom_closure;                             // Closure bit, true = silence at cld
	bool m_rom_pause;                               // Pause bit

	// Current interpolated values (8 bits each)
	u8 m_cur_fa, m_cur_fc, m_cur_va;
	u8 m_cur_f1, m_cur_f2, m_cur_f2q, m_cur_f3;

	// Current committed values
	u8 m_filt_fa, m_filt_fc, m_filt_va;             // Analog parameters, noise volume, noise freq cutoff and voice volume, 4 bits each
	u8 m_filt_f1, m_filt_f2, m_filt_f2q, m_filt_f3; // Analog parameters, formant frequencies/Q on 4 bits except f2 on 5 bits

	// Internal counters
	u16 m_phonetick;                                // 9-bits phone tick duration counter
	u8  m_ticks;                                    // 5-bits tick counter
	u8  m_pitch;                                    // 7-bits pitch counter
	u8  m_closure;                                  // 5-bits glottal closure counter
	u8  m_update_counter;                           // 6-bits counter for the 625Hz (main/1152) and 208Hz (main/3456) update timing generators

	// Internal state
	bool m_cur_closure;                             // Current internal closure state
	u16 m_noise;                                    // 15-bit noise shift register
	bool m_cur_noise;                               // Current noise output

	// Filter coefficients and level histories
	double m_voice_1[4];
	double m_voice_2[4];
	double m_voice_3[4];

	double m_noise_1[3];
	double m_noise_2[3];
	double m_noise_3[2];
	double m_noise_4[2];

	double m_vn_1[4];
	double m_vn_2[4];
	double m_vn_3[4];
	double m_vn_4[4];
	double m_vn_5[2];
	double m_vn_6[2];

	double m_f1_a[4],  m_f1_b[4];                   // F1 filtering
	double m_f2v_a[4], m_f2v_b[4];                  // F2 voice filtering
	double m_f2n_a[2], m_f2n_b[2];                  // F2 noise filtering
	double m_f3_a[4],  m_f3_b[4];                   // F3 filtering
	double m_f4_a[4],  m_f4_b[4];                   // F4 filtering
	double m_fx_a[1],  m_fx_b[2];                   // Final filtering
	double m_fn_a[3],  m_fn_b[3];                   // Noise shaping

	// Compute a total capacitor value based on which bits are currently active
	static double bits_to_caps(u32 value, std::initializer_list<double> caps_values) {
		double total = 0;
		for(double d : caps_values) {
			if(value & 1)
				total += d;
			value >>= 1;
		}
		return total;
	}

	// Shift a history of values by one and insert the new value at the front
	template<u32 N> static void shift_hist(double val, double (&hist_array)[N]) {
		for(u32 i=N-1; i>0; i--)
			hist_array[i] = hist_array[i-1];
		hist_array[0] = val;
	}

	// Apply a filter and compute the result. 'a' is applied to x (inputs) and 'b' to y (outputs)
	template<u32 Nx, u32 Ny, u32 Na, u32 Nb> static double apply_filter(const double (&x)[Nx], const double (&y)[Ny], const double (&a)[Na], const double (&b)[Nb]) {
		double total = 0;
		for(u32 i=0; i<Na; i++)
			total += x[i] * a[i];
		for(u32 i=1; i<Nb; i++)
			total -= y[i-1] * b[i];
		return total / b[0];
	}

	void build_standard_filter(double *a, double *b,
							   double c1t, // Unswitched cap, input, top
							   double c1b, // Switched cap, input, bottom
							   double c2t, // Unswitched cap, over first amp-op, top
							   double c2b, // Switched cap, over first amp-op, bottom
							   double c3,  // Cap between the two op-amps
							   double c4); // Cap over second op-amp

	void build_noise_shaper_filter(double *a, double *b,
								   double c1,  // Cap over first amp-op
								   double c2t, // Unswitched cap between amp-ops, input, top
								   double c2b, // Switched cap between amp-ops, input, bottom
								   double c3,  // Cap over second amp-op
								   double c4); // Switched cap after second amp-op

	void build_lowpass_filter(double *a, double *b,
							  double c1t,  // Unswitched cap, over amp-op, top
							  double c1b); // Switched cap, over amp-op, bottom

	void build_injection_filter(double *a, double *b,
								double c1b, // Switched cap, input, bottom
								double c2t, // Unswitched cap, over first amp-op, top
								double c2b, // Switched cap, over first amp-op, bottom
								double c3,  // Cap between the two op-amps
								double c4); // Cap over second op-amp


	static void interpolate(u8 &reg, u8 target);    // Do one interpolation step
	void chip_update();                             // Global update called at 20KHz (main/36)
	void filters_commit(bool force);                // Commit the currently computed interpolation values to the filters
	void phone_commit();                            // Commit the current phone id
	stream_buffer::sample_t analog_calc();                  // Compute one more sample
};

class votrax_sc01a_device : public votrax_sc01_device
{
public:
	votrax_sc01a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(VOTRAX_SC01, votrax_sc01_device)
DECLARE_DEVICE_TYPE(VOTRAX_SC01A, votrax_sc01a_device)

#endif // MAME_SOUND_VOTRAX_H
