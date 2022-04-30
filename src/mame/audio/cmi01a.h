// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Fairlight CMI-01A Channel Controller Card

***************************************************************************/

#ifndef MAME_AUDIO_CMI01A_H
#define MAME_AUDIO_CMI01A_H

#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/input_merger.h"

#define ENV_DIR_UP              0
#define ENV_DIR_DOWN            1

#define CHANNEL_STATUS_LOAD     1
#define CHANNEL_STATUS_RUN      2

class cmi01a_device : public device_t, public device_sound_interface {
public:
	cmi01a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t channel)
		: cmi01a_device(mconfig, tag, owner, clock)
	{
		m_channel = channel;
	}

	cmi01a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_callback() { return m_irq_cb.bind(); }

	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

protected:
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	static const device_timer_id TIMER_ZX = 0;
	static const device_timer_id TIMER_EOSI = 1;
	static const device_timer_id TIMER_BCAS = 2;

	required_device<input_merger_device> m_irq_merger;
	required_device_array<pia6821_device, 2> m_pia;
	required_device<ptm6840_device> m_ptm;

	required_device_array<pia6821_device, 2> m_cmi02_pia;

	sound_stream* m_stream;

private:
	DECLARE_WRITE_LINE_MEMBER(cmi01a_irq);

	void bcas_tick();
	void reset_bcas_counter();

	void clock_envelope();
	void tick_ediv();
	void set_eclk(bool eclk);
	void update_eclk();

	void pulse_zcint();
	void pulse_gzx();
	void reset_waveform_segment();
	void check_segment_load();
	void wpe_w(int state);
	void load_w(int state);

	void zx_timer_cb();
	void eosi_timer_cb();
	void run_voice();
	void update_wave_addr(int inc);

	uint32_t    m_channel;
	emu_timer * m_zx_timer = nullptr;
	uint8_t     m_zx_flag = 0;
	uint8_t     m_zx_ff = 0;

	emu_timer * m_eosi_timer = nullptr;

	emu_timer * m_bcas_timer = nullptr;

	std::unique_ptr<uint8_t[]>    m_wave_ram;
	uint16_t  m_segment_cnt = 0;
	uint8_t   m_new_addr = 0;     // Flag
	uint8_t   m_vol_latch = 0;
	uint8_t   m_flt_latch = 0;
	uint8_t   m_rp = 0;
	uint8_t   m_ws = 0;
	int       m_dir = 0;
	int       m_env_dir = 0;
	uint8_t   m_env = 0;
	int       m_pia0_cb2_state = 0;

	uint8_t   m_bcas_q1_ticks = 0;
	uint8_t   m_bcas_q1 = 0;
	uint8_t   m_bcas_q2_ticks = 0;
	uint8_t   m_bcas_q2 = 0;

	double    m_freq = 0;

	int       m_ptm_o1 = 0;
	int       m_ptm_o2 = 0;
	int       m_ptm_o3 = 0;

	bool      m_load = 0;
	bool      m_run = 0;
	bool      m_gzx = 0;
	bool      m_nwpe = 0;
	bool      m_tri = 0;
	bool      m_pia1_ca2 = 0;

	bool      m_eclk = false;
	bool      m_env_clk = false;
	bool      m_ediv_out = false;
	uint8_t   m_ediv_rate = 0;
	uint8_t   m_ediv_count = 0;

	uint16_t  m_pitch = 0;
	uint8_t   m_octave = 0;

	devcb_write_line m_irq_cb;

	void rp_w(uint8_t data);
	void ws_dir_w(uint8_t data);
	uint8_t ws_dir_r();
	DECLARE_READ_LINE_MEMBER( tri_r );
	DECLARE_WRITE_LINE_MEMBER( pia_0_cb2_w );
	DECLARE_WRITE_LINE_MEMBER( eload_w );

	DECLARE_READ_LINE_MEMBER( eosi_r );
	DECLARE_READ_LINE_MEMBER( zx_r );
	uint8_t pia_1_a_r();
	void pia_1_a_w(uint8_t data);
	void pia_1_b_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( ptm_o1 );
	DECLARE_WRITE_LINE_MEMBER( ptm_o2 );
	DECLARE_WRITE_LINE_MEMBER( ptm_o3 );
	DECLARE_WRITE_LINE_MEMBER( ptm_irq );

	static const uint8_t s_7497_rate_table[64][64];
};

// device type definition
DECLARE_DEVICE_TYPE(CMI01A_CHANNEL_CARD, cmi01a_device)

#endif // MAME_AUDIO_CMI01A_H
