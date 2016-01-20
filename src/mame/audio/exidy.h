// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#include "machine/6532riot.h"
#include "machine/6821pia.h"
#include "sound/hc55516.h"
#include "sound/tms5220.h"

/* 6840 variables */
struct sh6840_timer_channel
{
	UINT8   cr;
	UINT8   state;
	UINT8   leftovers;
	UINT16  timer;
	UINT32  clocks;
	union
	{
#ifdef LSB_FIRST
		struct { UINT8 l, h; } b;
#else
		struct { UINT8 h, l; } b;
#endif
		UINT16 w;
	} counter;
};

struct sh8253_timer_channel
{
	UINT8   clstate;
	UINT8   enable;
	UINT16  count;
	UINT32  step;
	UINT32  fraction;
};

class exidy_sound_device : public device_t,
									public device_sound_interface
{
public:
	exidy_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	exidy_sound_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	~exidy_sound_device() {}

	DECLARE_READ8_MEMBER( sh6840_r );
	DECLARE_WRITE8_MEMBER( sh6840_w );
	DECLARE_WRITE8_MEMBER( sfxctrl_w );

	DECLARE_WRITE_LINE_MEMBER( update_irq_state );

	DECLARE_WRITE8_MEMBER( r6532_porta_w );
	DECLARE_READ8_MEMBER( r6532_porta_r );
	DECLARE_WRITE8_MEMBER( r6532_portb_w );
	DECLARE_READ8_MEMBER( r6532_portb_r );
	void r6532_irq(int state);

	DECLARE_WRITE8_MEMBER( sh8253_w );
	DECLARE_READ8_MEMBER( sh8253_r );

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
	UINT8 m_riot_irq_state;

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
	INT16 m_sh6840_volume[3];
	UINT8 m_sh6840_MSB_latch;
	UINT8 m_sh6840_LSB_latch;
	UINT8 m_sh6840_LFSR_oldxor;
	UINT32 m_sh6840_LFSR_0;
	UINT32 m_sh6840_LFSR_1;
	UINT32 m_sh6840_LFSR_2;
	UINT32 m_sh6840_LFSR_3;
	UINT32 m_sh6840_clocks_per_sample;
	UINT32 m_sh6840_clock_count;

	UINT8 m_sfxctrl;

	inline int sh6840_update_noise(int clocks);
};

extern const device_type EXIDY;

class venture_sound_device : public exidy_sound_device
{
public:
	venture_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( mtrap_voiceio_w );
	DECLARE_READ8_MEMBER( mtrap_voiceio_r );

	DECLARE_WRITE8_MEMBER( filter_w );

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
	victory_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( response_r );
	DECLARE_READ8_MEMBER( status_r );
	DECLARE_WRITE8_MEMBER( command_w );
	DECLARE_WRITE_LINE_MEMBER( irq_clear_w );
	DECLARE_WRITE_LINE_MEMBER( main_ack_w );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	UINT8 m_victory_sound_response_ack_clk; /* 7474 @ F4 */

	TIMER_CALLBACK_MEMBER( delayed_command_w );

	int m_pia1_ca1;
	int m_pia1_cb1;
};

extern const device_type EXIDY_VICTORY;

MACHINE_CONFIG_EXTERN( venture_audio );

MACHINE_CONFIG_EXTERN( mtrap_cvsd_audio );

MACHINE_CONFIG_EXTERN( victory_audio );
