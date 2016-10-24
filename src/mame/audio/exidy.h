// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#include "machine/6532riot.h"
#include "machine/6821pia.h"
#include "sound/hc55516.h"
#include "sound/tms5220.h"

/* 6840 variables */
struct sh6840_timer_channel
{
	uint8_t   cr;
	uint8_t   state;
	uint8_t   leftovers;
	uint16_t  timer;
	uint32_t  clocks;
	union
	{
#ifdef LSB_FIRST
		struct { uint8_t l, h; } b;
#else
		struct { uint8_t h, l; } b;
#endif
		uint16_t w;
	} counter;
};

struct sh8253_timer_channel
{
	uint8_t   clstate;
	uint8_t   enable;
	uint16_t  count;
	uint32_t  step;
	uint32_t  fraction;
};

class exidy_sound_device : public device_t,
									public device_sound_interface
{
public:
	exidy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	exidy_sound_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	~exidy_sound_device() {}

	uint8_t sh6840_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sh6840_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sfxctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void update_irq_state(int state);

	void r6532_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t r6532_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void r6532_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t r6532_portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void r6532_irq(int state);

	void sh8253_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sh8253_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	void common_sh_start();
	void common_sh_reset();

	void sh6840_register_state_globals();
	void sh8253_register_state_globals();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	cpu_device *m_maincpu;

	/* 6532 variables */
	riot6532_device *m_riot;

	/* IRQ variable */
	uint8_t m_riot_irq_state;

	/* 8253 variables */
	int m_has_sh8253;
	struct sh8253_timer_channel m_sh8253_timer[3];

	/* 5220/CVSD variables */
	hc55516_device *m_cvsd;
	tms5220_device *m_tms;
	pia6821_device *m_pia0;
	pia6821_device *m_pia1;

	/* sound streaming variables */
	sound_stream *m_stream;
	double m_freq_to_step;

private:
	// internal state
	struct sh6840_timer_channel m_sh6840_timer[3];
	int16_t m_sh6840_volume[3];
	uint8_t m_sh6840_MSB_latch;
	uint8_t m_sh6840_LSB_latch;
	uint8_t m_sh6840_LFSR_oldxor;
	uint32_t m_sh6840_LFSR_0;
	uint32_t m_sh6840_LFSR_1;
	uint32_t m_sh6840_LFSR_2;
	uint32_t m_sh6840_LFSR_3;
	uint32_t m_sh6840_clocks_per_sample;
	uint32_t m_sh6840_clock_count;

	uint8_t m_sfxctrl;

	inline int sh6840_update_noise(int clocks);
};

extern const device_type EXIDY;

class venture_sound_device : public exidy_sound_device
{
public:
	venture_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mtrap_voiceio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mtrap_voiceio_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void filter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
};

extern const device_type EXIDY_VENTURE;

class victory_sound_device : public exidy_sound_device
{
public:
	victory_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t response_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_clear_w(int state);
	void main_ack_w(int state);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	uint8_t m_victory_sound_response_ack_clk; /* 7474 @ F4 */

	void delayed_command_w(void *ptr, int32_t param);

	int m_pia1_ca1;
	int m_pia1_cb1;
};

extern const device_type EXIDY_VICTORY;

MACHINE_CONFIG_EXTERN( venture_audio );

MACHINE_CONFIG_EXTERN( mtrap_cvsd_audio );

MACHINE_CONFIG_EXTERN( victory_audio );
