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
	votrax_sc01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_request_callback(device_t &device, _Object object) { return downcast<votrax_sc01_device &>(device).m_request_cb.set_callback(object); }

	// writers
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void inflection_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	int request() { m_stream->update(); return m_request_state; }

protected:
	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal helpers
	void update_subphoneme_clock_period();
	static double bits_to_caps(uint32_t value, int caps_count, const double *caps_values);
	static void shift_hist(double val, double *hist_array, int hist_size);
	static void filter_s_to_z(const double *k, double fs, double *a, double *b);
	static double apply_filter(const double *x, const double *y, const double *a, const double *b);

	// internal state
	sound_stream *              m_stream;               // output stream
	emu_timer *                 m_phoneme_timer;        // phoneme timer
	const uint8_t *               m_rom;                  // pointer to our ROM

	// inputs
	uint8_t                       m_inflection;           // 2-bit inflection value
	uint8_t                       m_phoneme;              // 6-bit phoneme value

	// outputs
	devcb_write_line           m_request_cb;           // callback for request
	uint8_t                       m_request_state;        // request as seen to the outside world
	uint8_t                       m_internal_request;     // request managed by stream timing

	// timing circuit
	uint32_t                      m_master_clock_freq;    // frequency of the master clock
	uint8_t                       m_master_clock;         // master clock
	uint16_t                      m_counter_34;           // ripple counter @ 34
	uint8_t                       m_latch_70;             // 4-bit latch @ 70
	uint8_t                       m_latch_72;             // 4-bit latch @ 72
	uint8_t                       m_beta1;                // beta1 clock state
	uint8_t                       m_p2;                   // P2 clock state
	uint8_t                       m_p1;                   // P1 clock state
	uint8_t                       m_phi2;                 // phi2 clock state
	uint8_t                       m_phi1;                 // phi1 clock state
	uint8_t                       m_phi2_20;              // alternate phi2 clock state (20kHz)
	uint8_t                       m_phi1_20;              // alternate phi1 clock state (20kHz)
	uint32_t                      m_subphoneme_period;    // period of the subphoneme timer
	uint32_t                      m_subphoneme_count;     // number of ticks executed already
	uint8_t                       m_clock_88;             // subphoneme clock output @ 88
	uint8_t                       m_latch_42;             // D flip-flop @ 42
	uint8_t                       m_counter_84;           // 4-bit phoneme counter @ 84
	uint8_t                       m_latch_92;             // 2-bit latch @ 92

	// low parameter clocking
	bool                        m_srff_132;             // S/R flip-flop @ 132
	bool                        m_srff_114;             // S/R flip-flop @ 114
	bool                        m_srff_112;             // S/R flip-flop @ 112
	bool                        m_srff_142;             // S/R flip-flop @ 142
	uint8_t                       m_latch_80;             // phoneme timing latch @ 80

	// glottal circuit
	uint8_t                       m_counter_220;          // 4-bit counter @ 220
	uint8_t                       m_counter_222;          // 4-bit counter @ 222
	uint8_t                       m_counter_224;          // 4-bit counter @ 224
	uint8_t                       m_counter_234;          // 4-bit counter @ 234
	uint8_t                       m_counter_236;          // 4-bit counter @ 236
	uint8_t                       m_fgate;                // FGATE signal
	uint8_t                       m_glottal_sync;         // Glottal Sync signal

	// transition circuit
	uint8_t                       m_0625_clock;           // state of 0.625kHz clock
	uint8_t                       m_counter_46;           // 4-bit counter in block @ 46
	uint8_t                       m_latch_46;             // 4-bit latch in block @ 46
	uint8_t                       m_ram[8];               // RAM to hold parameters
	uint8_t                       m_latch_168;            // 4-bit latch @ 168
	uint8_t                       m_latch_170;            // 4-bit latch @ 170
	uint8_t                       m_f1;                   // latched 4-bit F1 value
	uint8_t                       m_f2;                   // latched 5-bit F2 value
	uint8_t                       m_fc;                   // latched 4-bit FC value
	uint8_t                       m_f3;                   // latched 4-bit F3 value
	uint8_t                       m_f2q;                  // latched 4-bit F2Q value
	uint8_t                       m_va;                   // latched 4-bit VA value
	uint8_t                       m_fa;                   // latched 4-bit FA value

	// noise generator circuit
	uint8_t                       m_noise_clock;          // clock input to noise generator
	uint32_t                      m_shift_252;            // shift register @ 252
	uint8_t                       m_counter_250;          // 4-bit counter @ 250

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
