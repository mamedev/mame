// license:BSD-3-Clause
// copyright-holders:pSXAuthor, R. Belmont
#pragma once

#ifndef __SPU_H__
#define __SPU_H__

#include "spureverb.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SPU_IRQ_HANDLER(_devcb) \
	devcb = &spu_device::set_irq_handler(*device, DEVCB_##_devcb);

#define MCFG_SPU_ADD(_tag, _clock) \
	MCFG_DEVICE_MODIFY( "maincpu" ) \
	MCFG_PSX_SPU_READ_HANDLER(DEVREAD16(_tag, spu_device, read)) \
	MCFG_PSX_SPU_WRITE_HANDLER(DEVWRITE16(_tag, spu_device, write)) \
	MCFG_DEVICE_ADD(_tag, SPU, _clock) \
	MCFG_SPU_IRQ_HANDLER(DEVWRITELINE("maincpu:irq", psxirq_device, intin9)) \
	MCFG_PSX_DMA_CHANNEL_READ( "maincpu", 4, psx_dma_read_delegate( FUNC( spu_device::dma_read ), (spu_device *) device ) ) \
	MCFG_PSX_DMA_CHANNEL_WRITE( "maincpu", 4, psx_dma_write_delegate( FUNC( spu_device::dma_write ), (spu_device *) device ) )

// ======================> spu_device

const unsigned int spu_base_frequency_hz=44100;

class stream_buffer;

class spu_device : public device_t, public device_sound_interface
{
	struct sample_cache;
	struct sample_loop_cache;
	struct cache_pointer;
	struct voiceinfo;

	enum
	{
		dirtyflag_voice_mask=0x00ffffff,
		dirtyflag_reverb=0x01000000,
		dirtyflag_ram=0x02000000,
		dirtyflag_irq=0x04000000
	};

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	virtual void device_stop() override;

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// internal state
	devcb_write_line m_irq_handler;

	unsigned char *spu_ram;
	reverb *rev;
	unsigned int taddr, sample_t;

	stream_buffer *xa_buffer, *cdda_buffer;
	unsigned int xa_cnt,cdda_cnt,
					xa_freq,cdda_freq,
					xa_channels,
					xa_spf,
					xa_out_ptr,
					cur_frame_sample,
					cur_generate_sample,
					dirty_flags;

	UINT16 m_cd_out_ptr;

	signed short xa_last[4];
	bool status_enabled,
				xa_playing,
				cdda_playing;
	int xa_voll,xa_volr,
			changed_xa_vol;
	voiceinfo *voice;
	sample_cache **cache;
	float samples_per_frame,
				samples_per_cycle;

	static float freq_multiplier;

	unsigned char *output_buf[4];
	unsigned int output_head,
								output_tail,
								output_size,
								cur_qsz;

	unsigned int noise_t;
	signed short noise_cur;
	int noise_seed;

	#pragma pack(push,spureg,1)

	struct voicereg
	{
		unsigned short vol_l,   // 0
										vol_r, // 2
										pitch, // 4
										addr,  // 6
										adsl,  // 8
										srrr,  // a
										curvol, // c
										repaddr; // e
	};

	union
	{
		unsigned char reg[0x200];
		struct
		{
			voicereg voice[24];
			unsigned short mvol_l,mvol_r,
											rvol_l,rvol_r;
			unsigned int keyon,
										keyoff,
										fm,
										noise,
										reverb,
										chon;
			unsigned short _unknown,
											reverb_addr,
											irq_addr,
											trans_addr,
											data,
											ctrl;
			unsigned int status;
			signed short cdvol_l,cdvol_r,
										exvol_l,exvol_r;
		} spureg;
	};

	#pragma pack(pop,spureg)

	struct reverb_preset
	{
		const char *name;
		unsigned short param[32];
		reverb_params cfg;
	};

	reverb_preset *cur_reverb_preset;

	static reverb_preset reverb_presets[];

	void key_on(const int v);
	void key_off(const int v);
	bool update_envelope(const int v);
	void write_data(const unsigned short data);
	void generate(void *ptr, const unsigned int sz);
	void generate_voice(const unsigned int v,
											void *ptr,
											void *noiseptr,
											void *outxptr,
											const unsigned int sz);
	void generate_noise(void *ptr, const unsigned int num);
	bool process_voice(const unsigned int v,
											const unsigned int sz,
											void *ptr,
											void *fmnoise_ptr,
											void *outxptr,
											unsigned int *tleft);
	void process_until(const unsigned int tsample);
	void update_voice_loop(const unsigned int v);
	bool update_voice_state(const unsigned int v);
	void update_voice_state();
	void update_voice_events(voiceinfo *vi);
	void update_irq_event();
	unsigned int get_irq_distance(const voiceinfo *vi);
	void generate_xa(void *ptr, const unsigned int sz);
	void generate_cdda(void *ptr, const unsigned int sz);
	void decode_xa_mono(const unsigned char *xa, unsigned char *ptr);
	void decode_xa_stereo(const unsigned char *xa, unsigned char *ptr);
	void update_key();
	void update_reverb();
	void update_timing();

	bool translate_sample_addr(const unsigned int addr, cache_pointer *cp);
	sample_cache *get_sample_cache(const unsigned int addr);
	void flush_cache(sample_cache *sc,
										const unsigned int istart,
										const unsigned int iend);
	void invalidate_cache(const unsigned int st, const unsigned int en);

	void set_xa_format(const float freq, const int channels);

	void init_stream();
	void kill_stream();

	void update_vol(const unsigned int addr);

	void flush_output_buffer();

	sample_loop_cache *get_loop_cache(sample_cache *cache,
																		const unsigned int lpen,
																		sample_cache *lpcache,
																		const unsigned int lpst);
#if 0
	void write_cache_pointer(outfile *fout,
														cache_pointer *cp,
														sample_loop_cache *lc=NULL);
	void read_cache_pointer(infile *fin,
													cache_pointer *cp,
													sample_loop_cache **lc=NULL);
#endif
	static float get_linear_rate(const int n);
	static float get_linear_rate_neg_phase(const int n);
	static float get_pos_exp_rate(const int n);
	static float get_pos_exp_rate_neg_phase(const int n);
	static float get_neg_exp_rate(const int n);
	static float get_neg_exp_rate_neg_phase(const int n);
	static float get_decay_rate(const int n);
	static float get_sustain_level(const int n);
	static float get_linear_release_rate(const int n);
	static float get_exp_release_rate(const int n);
	static reverb_preset *find_reverb_preset(const unsigned short *param);

public:
	spu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<spu_device &>(device).m_irq_handler.set_callback(object); }

	void dma_read( UINT32 *ram, UINT32 n_address, INT32 n_size );
	void dma_write( UINT32 *ram, UINT32 n_address, INT32 n_size );

	void reinit_sound();
	void kill_sound();

	void start_dma(UINT8 *mainram, bool to_spu, UINT32 size);
	bool play_xa(const unsigned int sector, const unsigned char *sec);
	bool play_cdda(const unsigned int sector, const unsigned char *sec);
	void flush_xa(const unsigned int sector=0);
	void flush_cdda(const unsigned int sector=0);

	sound_stream *m_stream;

	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );
};

extern reverb_params *spu_reverb_cfg;

// device type definition
extern const device_type SPU;

#endif
