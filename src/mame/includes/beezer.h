// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
#include "machine/6522via.h"
#include "cpu/m6809/m6809.h"
#include "machine/watchdog.h"

class beezer_sound_device;

class beezer_state : public driver_device
{
public:
	beezer_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_watchdog(*this, "watchdog"),
		m_custom(*this, "custom"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<uint8_t> m_videoram;
	int m_pbus;
	int m_banklatch;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<beezer_sound_device> m_custom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void beezer_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void beezer_map_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t beezer_line_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void init_beezer();
	virtual void machine_start() override;
	uint32_t screen_update_beezer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void beezer_interrupt(timer_device &timer, void *ptr, int32_t param);
	uint8_t b_via_0_pa_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t b_via_0_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void b_via_0_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void b_via_0_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t b_via_1_pa_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t b_via_1_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void b_via_1_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void b_via_1_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
};

/*----------- defined in audio/beezer.c -----------*/

/* 6840 variables */
struct sh6840_timer_channel_beez
{
	uint8_t   cr;
	uint8_t   state;
	uint8_t   leftovers;
	uint16_t  timer;
	uint32_t  clocks;
	uint8_t   int_flag;
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

class beezer_sound_device : public device_t,
									public device_sound_interface
{
public:
	beezer_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~beezer_sound_device() {}

	uint8_t sh6840_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sh6840_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sfxctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void timer1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t noise_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	//void update_irq_state(int state);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
private:
	// internal state
	cpu_device *m_maincpu;

	/* IRQ variable */
	//uint8_t m_ptm_irq_state;

	struct sh6840_timer_channel_beez m_sh6840_timer[3];
	uint8_t m_sh6840_volume[4];
	uint8_t m_sh6840_MSB_latch;
	uint8_t m_sh6840_LSB_latch;
	uint32_t m_sh6840_LFSR;
	uint32_t m_sh6840_LFSR_clocks;
	uint32_t m_sh6840_clocks_per_sample;
	uint32_t m_sh6840_clock_count;

	uint32_t m_sh6840_latchwrite;
	uint32_t m_sh6840_latchwriteold;
	uint32_t m_sh6840_noiselatch1;
	uint32_t m_sh6840_noiselatch3;

	/* sound streaming variables */
	sound_stream *m_stream;
	//double m_freq_to_step;

	int sh6840_update_noise(int clocks);
};

extern const device_type BEEZER;
