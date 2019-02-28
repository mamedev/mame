// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    SunPlus SPG2xx-series SoC peripheral emulation

    TODO:
        - Serial UART
        - I2C
        - SPI

    Known SunPlus SPG2xx/u'nSP-based systems:

         D - SPG240 - Radica Skateboarder (Sunplus QL8041C die)
        ND - SPG243 - Some form of Leapfrog "edutainment" system
        ND - SPG243 - Star Wars: Clone Wars
        ND - SPG243 - Toy Story
        ND - SPG243 - Animal Art Studio
        ND - SPG243 - Finding Nemo
         D - SPG243 - The Batman
         D - SPG243 - Wall-E
         D - SPG243 - KenSingTon / Siatronics / Jungle Soft Vii
 Partial D - SPG200 - VTech V.Smile
        ND - unknown - Zone 40
         D - SPG243 - Zone 60
         D - SPG243 - Wireless 60
        ND - unknown - Wireless Air 60
        ND - Likely many more

**********************************************************************/

#ifndef MAME_MACHINE_SPG2XX_H
#define MAME_MACHINE_SPG2XX_H

#pragma once

#include "cpu/unsp/unsp.h"
#include "sound/okiadpcm.h"
#include "screen.h"

class spg2xx_device : public device_t, public device_sound_interface
{
public:
	spg2xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void set_pal(bool pal) { m_pal_flag = pal ? 1 : 0; }
	void set_rowscroll_offset(int offset) { m_rowscrolloffset = offset; }

	void map(address_map &map);

	auto porta_out() { return m_porta_out.bind(); }
	auto portb_out() { return m_portb_out.bind(); }
	auto portc_out() { return m_portc_out.bind(); }
	auto porta_in() { return m_porta_in.bind(); }
	auto portb_in() { return m_portb_in.bind(); }
	auto portc_in() { return m_portc_in.bind(); }

	template <size_t Line> auto adc_in() { return m_adc_in[Line].bind(); }

	auto eeprom_w() { return m_eeprom_w.bind(); }
	auto eeprom_r() { return m_eeprom_r.bind(); }

	auto uart_tx() { return m_uart_tx.bind(); }

	auto chip_select() { return m_chip_sel.bind(); }

	void uart_rx(uint8_t data);

	void extint_w(int channel, bool state);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank);

protected:
	spg2xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const uint32_t sprite_limit)
		: spg2xx_device(mconfig, type, tag, owner, clock)
	{
		m_sprite_limit = sprite_limit;
	}

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	enum
	{
		PAGE_ENABLE_MASK        = 0x0008,
		PAGE_WALLPAPER_MASK     = 0x0004,

		SPRITE_ENABLE_MASK      = 0x0001,
		SPRITE_COORD_TL_MASK    = 0x0002,

		PAGE_DEPTH_FLAG_MASK    = 0x3000,
		PAGE_DEPTH_FLAG_SHIFT   = 12,
		PAGE_TILE_HEIGHT_MASK   = 0x00c0,
		PAGE_TILE_HEIGHT_SHIFT  = 6,
		PAGE_TILE_WIDTH_MASK    = 0x0030,
		PAGE_TILE_WIDTH_SHIFT   = 4,
		TILE_X_FLIP             = 0x0004,
		TILE_Y_FLIP             = 0x0008
	};

	void audio_frame_tick();
	void audio_beat_tick();
	void audio_rampdown_tick(const uint32_t channel);
	bool audio_envelope_tick(address_space &space, const uint32_t channel);
	inline uint32_t get_rampdown_frame_count(const uint32_t channel);
	inline uint32_t get_envclk_frame_count(const uint32_t channel);

	// Audio getters
	bool get_channel_enable(const offs_t channel) const { return m_audio_regs[AUDIO_CHANNEL_ENABLE] & (1 << channel); }
	bool get_channel_status(const offs_t channel) const { return m_audio_regs[AUDIO_CHANNEL_STATUS] & (1 << channel); }
	bool get_manual_envelope_enable(const offs_t channel) const { return m_audio_regs[AUDIO_CHANNEL_ENV_MODE] & (1 << channel); }
	bool get_auto_envelope_enable(const offs_t channel) const { return !get_manual_envelope_enable(channel); }
	uint32_t get_envelope_clock(const offs_t channel) const;

	// Audio Mode getters
	uint16_t get_wave_addr_high(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_MODE] & AUDIO_WADDR_HIGH_MASK; }
	uint16_t get_loop_addr_high(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_MODE] & AUDIO_LADDR_HIGH_MASK) >> AUDIO_LADDR_HIGH_SHIFT; }
	uint16_t get_tone_mode(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_MODE] & AUDIO_TONE_MODE_MASK) >> AUDIO_TONE_MODE_SHIFT; }
	uint16_t get_16bit_bit(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_MODE] & AUDIO_16M_MASK) ? 1 : 0; }
	uint16_t get_adpcm_bit(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_MODE] & AUDIO_ADPCM_MASK) ? 1 : 0; }

	// Audio Pan getters
	uint16_t get_volume(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_PAN_VOL] & AUDIO_VOLUME_MASK; }
	uint16_t get_pan(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_PAN_VOL] & AUDIO_PAN_MASK) >> AUDIO_PAN_SHIFT; }

	// Audio Envelope0 Data getters
	uint16_t get_envelope_inc(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_ENVELOPE0] & AUDIO_ENVELOPE_INC_MASK; }
	uint16_t get_envelope_sign_bit(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE0] & AUDIO_ENVELOPE_SIGN_MASK) ? 1 : 0; }
	uint16_t get_envelope_target(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE0] & AUDIO_ENVELOPE_TARGET_MASK) >> AUDIO_ENVELOPE_TARGET_SHIFT; }
	uint16_t get_repeat_period_bit(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE0] & AUDIO_ENVELOPE_REPEAT_PERIOD_MASK) ? 1 : 0; }

	// Audio Envelope Data getters
	uint16_t get_edd(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_DATA] & AUDIO_EDD_MASK; }
	uint16_t get_envelope_count(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_DATA] & AUDIO_ENVELOPE_COUNT_MASK) >> AUDIO_ENVELOPE_COUNT_SHIFT; }
	void set_edd(const offs_t channel, uint8_t edd) { m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_DATA] = (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_DATA] & ~AUDIO_EDD_MASK) | edd; }
	void set_envelope_count(const offs_t channel, uint16_t count) { m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_DATA] = get_edd(channel) | (count << AUDIO_ENVELOPE_COUNT_SHIFT); }

	// Audio Envelope1 Data getters
	uint16_t get_envelope_load(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_ENVELOPE1] & AUDIO_ENVELOPE_LOAD_MASK; }
	uint16_t get_envelope_repeat_bit(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE1] & AUDIO_ENVELOPE_RPT_MASK) ? 1 : 0; }
	uint16_t get_envelope_repeat_count(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE1] & AUDIO_ENVELOPE_RPCNT_MASK) >> AUDIO_ENVELOPE_RPCNT_SHIFT; }
	inline void set_envelope_repeat_count(const offs_t channel, const uint16_t count) { m_audio_regs[(channel << 4) | AUDIO_ENVELOPE1] = (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE1] & ~AUDIO_ENVELOPE_RPCNT_MASK) | ((count << AUDIO_ENVELOPE_RPCNT_SHIFT) & AUDIO_ENVELOPE_RPCNT_MASK); }

	// Audio Envelope Address getters
	uint16_t get_envelope_addr_high(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_ADDR_HIGH] & AUDIO_EADDR_HIGH_MASK; }
	uint16_t get_audio_irq_enable_bit(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_ADDR_HIGH] & AUDIO_IRQ_EN_MASK) ? 1 : 0; }
	uint16_t get_audio_irq_addr(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_ADDR_HIGH] & AUDIO_IRQ_ADDR_MASK) >> AUDIO_IRQ_ADDR_SHIFT; }

	// Audio Envelope Loop getters
	uint16_t get_envelope_eaoffset(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_LOOP_CTRL] & AUDIO_EAOFFSET_MASK; }
	uint16_t get_rampdown_offset(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_LOOP_CTRL] & AUDIO_RAMPDOWN_OFFSET_MASK) >> AUDIO_RAMPDOWN_OFFSET_SHIFT; }
	void set_envelope_eaoffset(const offs_t channel, uint16_t eaoffset) { m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_LOOP_CTRL] = (m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_LOOP_CTRL] & ~AUDIO_RAMPDOWN_OFFSET_MASK) | (eaoffset & AUDIO_EAOFFSET_MASK); }

	// Audio ADPCM getters
	uint16_t get_point_number(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ADPCM_SEL] & AUDIO_POINT_NUMBER_MASK) >> AUDIO_POINT_NUMBER_SHIFT; }
	uint16_t get_adpcm36_bit(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_ADPCM_SEL] & AUDIO_ADPCM36_MASK) ? 1 : 0; }

	// Audio high-word getters
	uint16_t get_phase_high(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_PHASE_HIGH] & AUDIO_PHASE_HIGH_MASK; }
	uint16_t get_phase_accum_high(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_PHASE_ACCUM_HIGH] & AUDIO_PHASE_ACCUM_HIGH_MASK; }
	uint16_t get_target_phase_high(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_TARGET_PHASE_HIGH] & AUDIO_TARGET_PHASE_HIGH_MASK; }
	uint16_t get_rampdown_clock(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_RAMP_DOWN_CLOCK] & AUDIO_RAMP_DOWN_CLOCK_MASK; }

	// Audio ADPCM getters
	uint16_t get_phase_offset(const offs_t channel) const { return m_audio_regs[(channel << 4) | AUDIO_PHASE_CTRL] & AUDIO_PHASE_OFFSET_MASK; }
	uint16_t get_phase_sign_bit(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_PHASE_CTRL] & AUDIO_PHASE_SIGN_MASK) >> AUDIO_PHASE_SIGN_SHIFT; }
	uint16_t get_phase_time_step(const offs_t channel) const { return (m_audio_regs[(channel << 4) | AUDIO_PHASE_CTRL] & AUDIO_PHASE_TIME_STEP_MASK) >> AUDIO_PHASE_TIME_STEP_SHIFT; }

	// Audio combined getters
	uint32_t get_phase(const offs_t channel) const { return ((uint32_t)get_phase_high(channel) << 16) | m_audio_regs[(channel << 4) | AUDIO_PHASE]; }
	uint32_t get_phase_accum(const offs_t channel) const { return ((uint32_t)get_phase_accum_high(channel) << 16) | m_audio_regs[(channel << 4) | AUDIO_PHASE_ACCUM]; }
	uint32_t get_target_phase(const offs_t channel) const { return ((uint32_t)get_target_phase_high(channel) << 16) | m_audio_regs[(channel << 4) | AUDIO_TARGET_PHASE]; }
	uint32_t get_wave_addr(const offs_t channel) const { return ((uint32_t)get_wave_addr_high(channel) << 16) | m_audio_regs[(channel << 4) | AUDIO_WAVE_ADDR]; }
	uint32_t get_loop_addr(const offs_t channel) const { return ((uint32_t)get_loop_addr_high(channel) << 16) | m_audio_regs[(channel << 4) | AUDIO_LOOP_ADDR]; }
	uint32_t get_envelope_addr(const offs_t channel) const { return ((uint32_t)get_envelope_addr_high(channel) << 16) | m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_ADDR]; }

	enum
	{
		AUDIO_WAVE_ADDR             = 0x000,

		AUDIO_MODE                  = 0x001,
		AUDIO_WADDR_HIGH_MASK       = 0x003f,
		AUDIO_LADDR_HIGH_MASK       = 0x0fc0,
		AUDIO_LADDR_HIGH_SHIFT      = 6,
		AUDIO_TONE_MODE_MASK        = 0x3000,
		AUDIO_TONE_MODE_SHIFT       = 12,
		AUDIO_TONE_MODE_SW          = 0,
		AUDIO_TONE_MODE_HW_ONESHOT  = 1,
		AUDIO_TONE_MODE_HW_LOOP     = 2,
		AUDIO_16M_MASK              = 0x4000,
		AUDIO_ADPCM_MASK            = 0x8000,

		AUDIO_LOOP_ADDR             = 0x002,

		AUDIO_PAN_VOL               = 0x003,
		AUDIO_PAN_VOL_MASK          = 0x7f7f,
		AUDIO_VOLUME_MASK           = 0x007f,
		AUDIO_PAN_MASK              = 0x7f00,
		AUDIO_PAN_SHIFT             = 8,

		AUDIO_ENVELOPE0             = 0x004,
		AUDIO_ENVELOPE_INC_MASK     = 0x007f,
		AUDIO_ENVELOPE_SIGN_MASK    = 0x0080,
		AUDIO_ENVELOPE_TARGET_MASK  = 0x7f00,
		AUDIO_ENVELOPE_TARGET_SHIFT = 8,
		AUDIO_ENVELOPE_REPEAT_PERIOD_MASK = 0x8000,

		AUDIO_ENVELOPE_DATA         = 0x005,
		AUDIO_ENVELOPE_DATA_MASK    = 0xff7f,
		AUDIO_EDD_MASK              = 0x007f,
		AUDIO_ENVELOPE_COUNT_MASK   = 0xff00,
		AUDIO_ENVELOPE_COUNT_SHIFT  = 8,

		AUDIO_ENVELOPE1             = 0x006,
		AUDIO_ENVELOPE_LOAD_MASK    = 0x00ff,
		AUDIO_ENVELOPE_RPT_MASK     = 0x0100,
		AUDIO_ENVELOPE_RPCNT_MASK   = 0xfe00,
		AUDIO_ENVELOPE_RPCNT_SHIFT  = 9,

		AUDIO_ENVELOPE_ADDR_HIGH    = 0x007,
		AUDIO_EADDR_HIGH_MASK       = 0x003f,
		AUDIO_IRQ_EN_MASK           = 0x0040,
		AUDIO_IRQ_ADDR_MASK         = 0xff80,
		AUDIO_IRQ_ADDR_SHIFT        = 7,

		AUDIO_ENVELOPE_ADDR         = 0x008,
		AUDIO_WAVE_DATA_PREV        = 0x009,

		AUDIO_ENVELOPE_LOOP_CTRL    = 0x00a,
		AUDIO_EAOFFSET_MASK         = 0x01ff,
		AUDIO_RAMPDOWN_OFFSET_MASK  = 0xfe00,
		AUDIO_RAMPDOWN_OFFSET_SHIFT = 9,

		AUDIO_WAVE_DATA             = 0x00b,

		AUDIO_ADPCM_SEL             = 0x00d,
		AUDIO_ADPCM_SEL_MASK        = 0xfe00,
		AUDIO_POINT_NUMBER_MASK     = 0x7e00,
		AUDIO_POINT_NUMBER_SHIFT    = 9,
		AUDIO_ADPCM36_MASK          = 0x8000,

		AUDIO_PHASE_HIGH            = 0x200,
		AUDIO_PHASE_HIGH_MASK       = 0x0007,

		AUDIO_PHASE_ACCUM_HIGH      = 0x201,
		AUDIO_PHASE_ACCUM_HIGH_MASK = 0x0007,

		AUDIO_TARGET_PHASE_HIGH     = 0x202,
		AUDIO_TARGET_PHASE_HIGH_MASK= 0x0007,

		AUDIO_RAMP_DOWN_CLOCK       = 0x203,
		AUDIO_RAMP_DOWN_CLOCK_MASK  = 0x0007,

		AUDIO_PHASE                 = 0x204,
		AUDIO_PHASE_ACCUM           = 0x205,
		AUDIO_TARGET_PHASE          = 0x206,

		AUDIO_PHASE_CTRL            = 0x207,
		AUDIO_PHASE_OFFSET_MASK     = 0x0fff,
		AUDIO_PHASE_SIGN_MASK       = 0x1000,
		AUDIO_PHASE_SIGN_SHIFT      = 12,
		AUDIO_PHASE_TIME_STEP_MASK  = 0xe000,
		AUDIO_PHASE_TIME_STEP_SHIFT = 13,

		AUDIO_CHAN_OFFSET_MASK      = 0xf0f,

		AUDIO_CHANNEL_ENABLE            = 0x400,
		AUDIO_CHANNEL_ENABLE_MASK       = 0xffff,

		AUDIO_MAIN_VOLUME               = 0x401,
		AUDIO_MAIN_VOLUME_MASK          = 0x007f,

		AUDIO_CHANNEL_FIQ_ENABLE        = 0x402,
		AUDIO_CHANNEL_FIQ_ENABLE_MASK   = 0xffff,

		AUDIO_CHANNEL_FIQ_STATUS        = 0x403,
		AUDIO_CHANNEL_FIQ_STATUS_MASK   = 0xffff,

		AUDIO_BEAT_BASE_COUNT           = 0x404,
		AUDIO_BEAT_BASE_COUNT_MASK      = 0x07ff,

		AUDIO_BEAT_COUNT                = 0x405,
		AUDIO_BEAT_COUNT_MASK           = 0x3fff,
		AUDIO_BIS_MASK                  = 0x4000,
		AUDIO_BIE_MASK                  = 0x8000,

		AUDIO_ENVCLK0                   = 0x406,

		AUDIO_ENVCLK0_HIGH              = 0x407,
		AUDIO_ENVCLK0_HIGH_MASK         = 0xffff,

		AUDIO_ENVCLK1                   = 0x408,

		AUDIO_ENVCLK1_HIGH              = 0x409,
		AUDIO_ENVCLK1_HIGH_MASK         = 0xffff,

		AUDIO_ENV_RAMP_DOWN             = 0x40a,
		AUDIO_ENV_RAMP_DOWN_MASK        = 0xffff,

		AUDIO_CHANNEL_STOP              = 0x40b,
		AUDIO_CHANNEL_STOP_MASK         = 0xffff,

		AUDIO_CHANNEL_ZERO_CROSS        = 0x40c,
		AUDIO_CHANNEL_ZERO_CROSS_MASK   = 0xffff,

		AUDIO_CONTROL                   = 0x40d,
		AUDIO_CONTROL_MASK              = 0x9fe8,
		AUDIO_CONTROL_SATURATE_MASK     = 0x8000,
		AUDIO_CONTROL_SOFTCH_MASK       = 0x1000,
		AUDIO_CONTROL_COMPEN_MASK       = 0x0800,
		AUDIO_CONTROL_NOHIGH_MASK       = 0x0400,
		AUDIO_CONTROL_NOINT_MASK        = 0x0200,
		AUDIO_CONTROL_EQEN_MASK         = 0x0100,
		AUDIO_CONTROL_VOLSEL_MASK       = 0x00c0,
		AUDIO_CONTROL_VOLSEL_SHIFT      = 6,
		AUDIO_CONTROL_FOF_MASK          = 0x0020,
		AUDIO_CONTROL_INIT_MASK         = 0x0008,

		AUDIO_COMPRESS_CTRL             = 0x40e,
		AUDIO_COMPRESS_CTRL_PEAK_MASK   = 0x8000,
		AUDIO_COMPRESS_CTRL_THRESHOLD_MASK  = 0x7f00,
		AUDIO_COMPRESS_CTRL_THRESHOLD_SHIFT = 8,
		AUDIO_COMPRESS_CTRL_ATTSCALE_MASK   = 0x00c0,
		AUDIO_COMPRESS_CTRL_ATTSCALE_SHIFT  = 6,
		AUDIO_COMPRESS_CTRL_RELSCALE_MASK   = 0x0030,
		AUDIO_COMPRESS_CTRL_RELSCALE_SHIFT  = 4,
		AUDIO_COMPRESS_CTRL_DISZC_MASK      = 0x0008,
		AUDIO_COMPRESS_CTRL_RATIO_MASK      = 0x0007,

		AUDIO_CHANNEL_STATUS            = 0x40f,
		AUDIO_CHANNEL_STATUS_MASK       = 0xffff,

		AUDIO_WAVE_IN_L                 = 0x410,

		AUDIO_WAVE_IN_R                 = 0x411,
		AUDIO_SOFTIRQ_MASK              = 0x8000,
		AUDIO_SOFTIRQ_EN_MASK           = 0x4000,
		AUDIO_SOFT_PHASE_HIGH_MASK      = 0x0070,
		AUDIO_SOFT_PHASE_HIGH_SHIFT     = 4,
		AUDIO_FIFO_IRQ_THRESHOLD_MASK   = 0x000f,

		AUDIO_WAVE_OUT_L                = 0x412,
		AUDIO_WAVE_OUT_R                = 0x413,

		AUDIO_CHANNEL_REPEAT            = 0x414,
		AUDIO_CHANNEL_REPEAT_MASK       = 0xffff,

		AUDIO_CHANNEL_ENV_MODE          = 0x415,
		AUDIO_CHANNEL_ENV_MODE_MASK     = 0xffff,

		AUDIO_CHANNEL_TONE_RELEASE      = 0x416,
		AUDIO_CHANNEL_TONE_RELEASE_MASK = 0xffff,

		AUDIO_CHANNEL_ENV_IRQ           = 0x417,
		AUDIO_CHANNEL_ENV_IRQ_MASK      = 0xffff,

		AUDIO_CHANNEL_PITCH_BEND        = 0x418,
		AUDIO_CHANNEL_PITCH_BEND_MASK   = 0xffff,

		AUDIO_SOFT_PHASE                = 0x419,

		AUDIO_ATTACK_RELEASE            = 0x41a,
		AUDIO_RELEASE_TIME_MASK         = 0x00ff,
		AUDIO_ATTACK_TIME_MASK          = 0xff00,
		AUDIO_ATTACK_TIME_SHIFT         = 8,

		AUDIO_EQ_CUTOFF10               = 0x41b,
		AUDIO_EQ_CUTOFF10_MASK          = 0x7f7f,

		AUDIO_EQ_CUTOFF32               = 0x41c,
		AUDIO_EQ_CUTOFF32_MASK          = 0x7f7f,

		AUDIO_EQ_GAIN10                 = 0x41d,
		AUDIO_EQ_GAIN10_MASK            = 0x7f7f,

		AUDIO_EQ_GAIN32                 = 0x41e,
		AUDIO_EQ_GAIN32_MASK            = 0x7f7f
	};

	DECLARE_READ16_MEMBER(video_r);
	DECLARE_WRITE16_MEMBER(video_w);
	DECLARE_READ16_MEMBER(audio_r);
	DECLARE_WRITE16_MEMBER(audio_w);
	virtual DECLARE_READ16_MEMBER(io_r);
	virtual DECLARE_WRITE16_MEMBER(io_w);

	void check_extint_irq(int channel);
	void check_irqs(const uint16_t changed);
	inline void check_video_irq();

	void spg2xx_map(address_map &map);

	static const device_timer_id TIMER_TMB1 = 0;
	static const device_timer_id TIMER_TMB2 = 1;
	static const device_timer_id TIMER_SCREENPOS = 2;
	static const device_timer_id TIMER_BEAT = 3;
	static const device_timer_id TIMER_UART_TX = 4;
	static const device_timer_id TIMER_UART_RX = 5;
	static const device_timer_id TIMER_4KHZ = 6;
	static const device_timer_id TIMER_SRC_AB = 7;
	static const device_timer_id TIMER_SRC_C = 8;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void update_porta_special_modes();
	void update_portb_special_modes();
	void do_gpio(uint32_t offset, bool write);
	uint16_t do_special_gpio(uint32_t index, uint16_t mask);

	void update_timer_b_rate();
	void update_timer_ab_src();
	void update_timer_c_src();
	void increment_timer_a();

	void uart_transmit_tick();
	void uart_receive_tick();

	void system_timer_tick();

	void do_i2c();
	void do_cpu_dma(uint32_t len);

	void do_sprite_dma(uint32_t len);

	enum blend_enable_t : bool
	{
		BlendOff = false,
		BlendOn = true
	};

	enum rowscroll_enable_t : bool
	{
		RowScrollOff = false,
		RowScrollOn = true
	};

	enum flipx_t : bool
	{
		FlipXOff = false,
		FlipXOn = true
	};

	void apply_saturation(const rectangle &cliprect);
	void apply_fade(const rectangle &cliprect);

	template<blend_enable_t Blend, rowscroll_enable_t RowScroll, flipx_t FlipX>
	void blit(const rectangle &cliprect, uint32_t line, uint32_t xoff, uint32_t yoff, uint32_t attr, uint32_t ctrl, uint32_t bitmap_addr, uint16_t tile);
	void blit_page(const rectangle &cliprect, uint32_t scanline, int depth, uint32_t bitmap_addr, uint16_t *regs);
	void blit_sprite(const rectangle &cliprect, uint32_t scanline, int depth, uint32_t base_addr);
	void blit_sprites(const rectangle &cliprect, uint32_t scanline, int depth);

	uint8_t mix_channel(uint8_t a, uint8_t b);

	void stop_channel(const uint32_t channel);
	bool advance_channel(address_space &space, const uint32_t channel);
	bool fetch_sample(address_space &space, const uint32_t channel);
	void loop_channel(const uint32_t channel);

	uint32_t m_screenbuf[320 * 240];
	uint8_t m_rgb5_to_rgb8[32];
	uint32_t m_rgb555_to_rgb888[0x8000];

	bool m_hide_page0;
	bool m_hide_page1;
	bool m_hide_sprites;
	bool m_debug_sprites;
	bool m_debug_blit;
	bool m_debug_palette;
	uint8_t m_sprite_index_to_debug;

	bool m_debug_samples;
	bool m_debug_rates;

	uint16_t m_audio_regs[0x800];
	uint8_t m_sample_shift[16];
	uint32_t m_sample_count[16];
	uint32_t m_sample_addr[16];
	double m_channel_rate[16];
	double m_channel_rate_accum[16];
	uint32_t m_rampdown_frame[16];
	uint32_t m_envclk_frame[16];
	uint32_t m_envelope_addr[16];
	int m_channel_debug;
	uint16_t m_audio_curr_beat_base_count;

	uint16_t m_io_regs[0x200];
	uint8_t m_uart_rx_fifo[8];
	uint8_t m_uart_rx_fifo_start;
	uint8_t m_uart_rx_fifo_end;
	uint8_t m_uart_rx_fifo_count;
	bool m_uart_rx_available;
	bool m_uart_rx_irq;
	bool m_uart_tx_irq;

	bool m_extint[2];

	uint16_t m_video_regs[0x100];
	uint32_t m_sprite_limit;
	int m_rowscrolloffset; // auto racing in 'zone60' minigames needs this to be 15, the JAKKS games (Star Wars Revenge of the sith - Gunship Battle, Wheel of Fortune, Namco Ms. Pac-Man 5-in-1 Pole Position) need it to be 0, where does it come from?
	uint16_t m_pal_flag;

	devcb_write16 m_porta_out;
	devcb_write16 m_portb_out;
	devcb_write16 m_portc_out;
	devcb_read16 m_porta_in;
	devcb_read16 m_portb_in;
	devcb_read16 m_portc_in;

	devcb_read16 m_adc_in[2];

	devcb_write8 m_eeprom_w;
	devcb_read8 m_eeprom_r;

	devcb_write8 m_uart_tx;

	devcb_write8 m_chip_sel;

	uint16_t m_timer_a_preload;
	uint16_t m_timer_b_preload;
	uint16_t m_timer_b_divisor;
	uint16_t m_timer_b_tick_rate;

	emu_timer *m_tmb1;
	emu_timer *m_tmb2;
	emu_timer *m_timer_src_ab;
	emu_timer *m_timer_src_c;
	emu_timer *m_screenpos_timer;
	emu_timer *m_audio_beat;

	emu_timer *m_4khz_timer;
	uint32_t m_2khz_divider;
	uint32_t m_1khz_divider;
	uint32_t m_4hz_divider;

	uint32_t m_uart_baud_rate;
	emu_timer *m_uart_tx_timer;
	emu_timer *m_uart_rx_timer;

	sound_stream *m_stream;
	oki_adpcm_state m_adpcm[16];

	required_device<unsp_device> m_cpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint16_t> m_scrollram;
	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_spriteram;

	static const uint32_t s_rampdown_frame_counts[8];
	static const uint32_t s_envclk_frame_counts[16];
};

class spg24x_device : public spg2xx_device
{
public:
	template <typename T, typename U>
	spg24x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&screen_tag)
		: spg24x_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
		m_screen.set_tag(std::forward<U>(screen_tag));
	}

	spg24x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class spg28x_device : public spg2xx_device
{
public:
	template <typename T, typename U>
	spg28x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&screen_tag)
		: spg28x_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
		m_screen.set_tag(std::forward<U>(screen_tag));
	}

	spg28x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE16_MEMBER(io_w) override;
};

DECLARE_DEVICE_TYPE(SPG24X, spg24x_device)
DECLARE_DEVICE_TYPE(SPG28X, spg28x_device)

#endif // MAME_MACHINE_SPG2XX_H
