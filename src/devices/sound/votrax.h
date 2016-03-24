// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    votrax.h

    Simple VOTRAX SC-01 simulator based on sample fragments.

***************************************************************************/

#pragma once

#ifndef __VOTRAX_H__
#define __VOTRAX_H__

#include "sound/samples.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VOTRAX_SC01_REQUEST_CB(_devcb) \
	devcb = &votrax_sc01_device::set_request_callback(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> votrax_sc01_device

class votrax_sc01_device :  public device_t,
							public device_sound_interface
{
public:
	// construction/destruction
	votrax_sc01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_request_callback(device_t &device, _Object object) { return downcast<votrax_sc01_device &>(device).m_request_cb.set_callback(object); }

	// writers
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE8_MEMBER( inflection_w );
	DECLARE_READ_LINE_MEMBER( request ) { m_stream->update(); return m_request_state; }

protected:
	// device-level overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal helpers
	void update_subphoneme_clock_period();
	static double bits_to_caps(UINT32 value, int caps_count, const double *caps_values);
	static void shift_hist(double val, double *hist_array, int hist_size);
	static void filter_s_to_z(const double *k, double fs, double *a, double *b);
	static double apply_filter(const double *x, const double *y, const double *a, const double *b);

	// internal state
	sound_stream *              m_stream;               // output stream
	emu_timer *                 m_phoneme_timer;        // phoneme timer
	const UINT8 *               m_rom;                  // pointer to our ROM

	// inputs
	UINT8                       m_inflection;           // 2-bit inflection value
	UINT8                       m_phoneme;              // 6-bit phoneme value

	// outputs
	devcb_write_line           m_request_cb;           // callback for request
	UINT8                       m_request_state;        // request as seen to the outside world
	UINT8                       m_internal_request;     // request managed by stream timing

	// timing circuit
	UINT32                      m_master_clock_freq;    // frequency of the master clock
	UINT8                       m_master_clock;         // master clock
	UINT16                      m_counter_34;           // ripple counter @ 34
	UINT8                       m_latch_70;             // 4-bit latch @ 70
	UINT8                       m_latch_72;             // 4-bit latch @ 72
	UINT8                       m_beta1;                // beta1 clock state
	UINT8                       m_p2;                   // P2 clock state
	UINT8                       m_p1;                   // P1 clock state
	UINT8                       m_phi2;                 // phi2 clock state
	UINT8                       m_phi1;                 // phi1 clock state
	UINT8                       m_phi2_20;              // alternate phi2 clock state (20kHz)
	UINT8                       m_phi1_20;              // alternate phi1 clock state (20kHz)
	UINT32                      m_subphoneme_period;    // period of the subphoneme timer
	UINT32                      m_subphoneme_count;     // number of ticks executed already
	UINT8                       m_clock_88;             // subphoneme clock output @ 88
	UINT8                       m_latch_42;             // D flip-flop @ 42
	UINT8                       m_counter_84;           // 4-bit phoneme counter @ 84
	UINT8                       m_latch_92;             // 2-bit latch @ 92

	// low parameter clocking
	bool                        m_srff_132;             // S/R flip-flop @ 132
	bool                        m_srff_114;             // S/R flip-flop @ 114
	bool                        m_srff_112;             // S/R flip-flop @ 112
	bool                        m_srff_142;             // S/R flip-flop @ 142
	UINT8                       m_latch_80;             // phoneme timing latch @ 80

	// glottal circuit
	UINT8                       m_counter_220;          // 4-bit counter @ 220
	UINT8                       m_counter_222;          // 4-bit counter @ 222
	UINT8                       m_counter_224;          // 4-bit counter @ 224
	UINT8                       m_counter_234;          // 4-bit counter @ 234
	UINT8                       m_counter_236;          // 4-bit counter @ 236
	UINT8                       m_fgate;                // FGATE signal
	UINT8                       m_glottal_sync;         // Glottal Sync signal

	// transition circuit
	UINT8                       m_0625_clock;           // state of 0.625kHz clock
	UINT8                       m_counter_46;           // 4-bit counter in block @ 46
	UINT8                       m_latch_46;             // 4-bit latch in block @ 46
	UINT8                       m_ram[8];               // RAM to hold parameters
	UINT8                       m_latch_168;            // 4-bit latch @ 168
	UINT8                       m_latch_170;            // 4-bit latch @ 170
	UINT8                       m_f1;                   // latched 4-bit F1 value
	UINT8                       m_f2;                   // latched 5-bit F2 value
	UINT8                       m_fc;                   // latched 4-bit FC value
	UINT8                       m_f3;                   // latched 4-bit F3 value
	UINT8                       m_f2q;                  // latched 4-bit F2Q value
	UINT8                       m_va;                   // latched 4-bit VA value
	UINT8                       m_fa;                   // latched 4-bit FA value

	// noise generator circuit
	UINT8                       m_noise_clock;          // clock input to noise generator
	UINT32                      m_shift_252;            // shift register @ 252
	UINT8                       m_counter_250;          // 4-bit counter @ 250

	// stages outputs history
	double                      m_ni_hist[4];
	double                      m_no_hist[4];
	double                      m_va_hist[4];
	double                      m_s1_hist[4];
	double                      m_s2g_hist[4];
	double                      m_s2ni_hist[4];
	double                      m_s2n_hist[4];
	double                      m_s2_hist[4];
	double                      m_s3_hist[4];
	double                      m_s4i_hist[4];
	double                      m_s4_hist[4];

	// static tables
	static const char *const s_phoneme_table[64];
	static const double s_glottal_wave[16];
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type VOTRAX_SC01;


#endif /* __VOTRAX_H__ */
