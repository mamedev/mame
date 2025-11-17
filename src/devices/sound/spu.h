// license:BSD-3-Clause
// copyright-holders:pSXAuthor, R. Belmont
#ifndef MAME_SOUND_SPU_H
#define MAME_SOUND_SPU_H

#pragma once

// ======================> spu_device

class spu_stream_buffer;
class psxcpu_device;

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

	sound_stream_flags m_stream_flags;

protected:
	class reverb;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_stop() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;

	float spu_base_frequency_hz;
	float linear_rate[108];
	float pos_exp_rate[100];
	float neg_exp_rate[108];
	float decay_rate[16];
	float linear_release_rate[27];
	float exp_release_rate[27];

	// internal state
	devcb_write_line m_irq_handler;

	std::unique_ptr<unsigned char []> spu_ram;
	reverb *rev;
	unsigned int taddr;
	unsigned int sample_t;

	spu_stream_buffer *xa_buffer;
	spu_stream_buffer *cdda_buffer;
	unsigned int xa_cnt;
	unsigned int cdda_cnt;
	unsigned int xa_freq;
	unsigned int cdda_freq;
	unsigned int xa_channels;
	unsigned int xa_spf;
	unsigned int xa_out_ptr;
	unsigned int cur_frame_sample;
	unsigned int cur_generate_sample;
	unsigned int dirty_flags;

	uint16_t m_cd_out_ptr;

	signed short xa_last[4];
	bool status_enabled, xa_playing, cdda_playing;
	int xa_voll, xa_volr, changed_xa_vol;
	voiceinfo *voice;
	std::unique_ptr<sample_cache * []> cache;
	float samples_per_frame;
	float samples_per_cycle;

	static float freq_multiplier;

	std::unique_ptr<unsigned char []> output_buf[4];
	unsigned int output_head;
	unsigned int output_tail;
	unsigned int output_size;
	unsigned int cur_qsz;

	unsigned int noise_t;
	signed short noise_cur;
	int noise_seed;

	#pragma pack(push,spureg,1)

	struct voicereg
	{
		unsigned short vol_l;   // 0
		unsigned short vol_r;   // 2
		unsigned short pitch;   // 4
		unsigned short addr;    // 6
		unsigned short adsl;    // 8
		unsigned short srrr;    // a
		unsigned short curvol;  // c
		unsigned short repaddr; // e
	};

	union
	{
		unsigned char reg[0x200];
		struct
		{
			voicereg voice[24];
			unsigned short mvol_l;
			unsigned short mvol_r;
			unsigned short rvol_l;
			unsigned short rvol_r;
			unsigned int keyon;
			unsigned int keyoff;
			unsigned int fm;
			unsigned int noise;
			unsigned int reverb;
			unsigned int chon;
			unsigned short _unknown;
			unsigned short reverb_addr;
			unsigned short irq_addr;
			unsigned short trans_addr;
			unsigned short data;
			unsigned short ctrl;
			unsigned int status;
			signed short cdvol_l;
			signed short cdvol_r;
			signed short exvol_l;
			signed short exvol_r;
		} spureg;
	};

	#pragma pack(pop,spureg)

	struct reverb_params;
	struct reverb_preset;

	reverb_preset *cur_reverb_preset;

	sound_stream *m_stream;

	static reverb_preset reverb_presets[];
	static reverb_params *spu_reverb_cfg;

	float ms_to_rate(float ms) const { return 1.0f / (ms * (spu_base_frequency_hz / 1000.0f)); }
	float s_to_rate(float s) const { return ms_to_rate(s * 1000.0f); }

	void key_on(const int v);
	void key_off(const int v);
	bool update_envelope(const int v);
	void write_data(const unsigned short data);
	void generate(void *ptr, const unsigned int sz);
	void generate_voice(const unsigned int v, void *ptr, void *noiseptr, void *outxptr, const unsigned int sz);
	void generate_noise(void *ptr, const unsigned int num);
	bool process_voice(const unsigned int v, const unsigned int sz, void *ptr, void *fmnoise_ptr, void *outxptr, unsigned int *tleft);
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
	void flush_cache(sample_cache *sc, const unsigned int istart, const unsigned int iend);
	void invalidate_cache(const unsigned int st, const unsigned int en);

	void set_xa_format(const float freq, const int channels);

	void init_stream();
	void kill_stream();

	void update_vol(const unsigned int addr);

	void flush_output_buffer();

	sample_loop_cache *get_loop_cache(sample_cache *cache, const unsigned int lpen, sample_cache *lpcache, const unsigned int lpst);
#if 0
	void write_cache_pointer(outfile *fout, cache_pointer *cp, sample_loop_cache *lc=nullptr);
	void read_cache_pointer(infile *fin, cache_pointer *cp, sample_loop_cache **lc=nullptr);
#endif

	void generate_linear_rate_table();
	void generate_pos_exp_rate_table();
	void generate_neg_exp_rate_table();
	void generate_decay_rate_table();
	void generate_linear_release_rate_table();
	void generate_exp_release_rate_table();

	float get_linear_rate(const int n);
	float get_linear_rate_neg_phase(const int n);
	float get_pos_exp_rate(const int n);
	float get_pos_exp_rate_neg_phase(const int n);
	float get_neg_exp_rate(const int n);
	float get_neg_exp_rate_neg_phase(const int n);
	float get_decay_rate(const int n);
	float get_sustain_level(const int n);
	float get_linear_release_rate(const int n);
	float get_exp_release_rate(const int n);
	reverb_preset *find_reverb_preset(const unsigned short *param);

public:
	spu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, psxcpu_device *cpu);
	spu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }
	void set_stream_flags(sound_stream_flags flags) { m_stream_flags = flags; }

	void dma_read( uint32_t *ram, uint32_t n_address, int32_t n_size );
	void dma_write( uint32_t *ram, uint32_t n_address, int32_t n_size );

	void reinit_sound();
	void kill_sound();

	void start_dma(uint8_t *mainram, bool to_spu, uint32_t size);
	bool play_xa(const unsigned int sector, const unsigned char *sec);
	bool play_cdda(const unsigned int sector, const unsigned char *sec);
	void flush_xa(const unsigned int sector=0);
	void flush_cdda(const unsigned int sector=0);

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data);
};

// device type definition
DECLARE_DEVICE_TYPE(SPU, spu_device)

#endif // MAME_SOUND_SPU_H
