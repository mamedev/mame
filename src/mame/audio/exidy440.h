// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/* channel_data structure holds info about each 6844 DMA channel */
struct m6844_channel_data
{
	int active;
	int address;
	int counter;
	UINT8 control;
	int start_address;
	int start_counter;
};


/* channel_data structure holds info about each active sound channel */
struct sound_channel_data
{
	INT16 *base;
	int offset;
	int remaining;
};


/* sound_cache_entry structure contains info on each decoded sample */
struct sound_cache_entry
{
	struct sound_cache_entry *next;
	int address;
	int length;
	int bits;
	int frequency;
	INT16 data[1];
};

class exidy440_sound_device : public device_t,
									public device_sound_interface
{
public:
	exidy440_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~exidy440_sound_device() {}

	DECLARE_READ8_MEMBER( sound_command_r );
	DECLARE_READ8_MEMBER( sound_volume_r );
	DECLARE_WRITE8_MEMBER( sound_volume_w );
	DECLARE_WRITE8_MEMBER( sound_interrupt_clear_w );
	DECLARE_READ8_MEMBER( m6844_r );
	DECLARE_WRITE8_MEMBER( m6844_w );
	DECLARE_WRITE8_MEMBER( sound_banks_w );

	void exidy440_sound_command(UINT8 param);
	UINT8 exidy440_sound_command_ack();

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_stop() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	UINT8 m_sound_command;
	UINT8 m_sound_command_ack;

	UINT8 m_sound_banks[4];
	//UINT8 m_m6844_data[0x20];
	UINT8 m_sound_volume[0x10];
	std::unique_ptr<INT32[]> m_mixer_buffer_left;
	std::unique_ptr<INT32[]> m_mixer_buffer_right;
	sound_cache_entry *m_sound_cache;
	sound_cache_entry *m_sound_cache_end;
	sound_cache_entry *m_sound_cache_max;

	/* 6844 description */
	m6844_channel_data m_m6844_channel[4];
	UINT8 m_m6844_priority;
	UINT8 m_m6844_interrupt;
	UINT8 m_m6844_chain;

	/* sound interface parameters */
	sound_stream *m_stream;
	sound_channel_data m_sound_channel[4];

	/* debugging */
	FILE *m_debuglog;

	/* channel frequency is configurable */
	int m_channel_frequency[4];

	void m6844_update();
	void m6844_finished(m6844_channel_data *channel);
	void play_cvsd(int ch);
	void stop_cvsd(int ch);

	void reset_sound_cache();
	INT16 *add_to_sound_cache(UINT8 *input, int address, int length, int bits, int frequency);
	INT16 *find_or_add_to_sound_cache(int address, int length, int bits, int frequency);

	void decode_and_filter_cvsd(UINT8 *data, int bytes, int maskbits, int frequency, INT16 *dest);
	void fir_filter(INT32 *input, INT16 *output, int count);

	void add_and_scale_samples(int ch, INT32 *dest, int samples, int volume);
	void mix_to_16(int length, stream_sample_t *dest_left, stream_sample_t *dest_right);
};

extern const device_type EXIDY440;

MACHINE_CONFIG_EXTERN( exidy440_audio );
