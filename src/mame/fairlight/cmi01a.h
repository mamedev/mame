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
	cmi01a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 channel)
		: cmi01a_device(mconfig, tag, owner, clock)
	{
		m_channel = channel;
	}

	cmi01a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto irq_callback() { return m_irq_cb.bind(); }

	void write(offs_t offset, u8 data);
	u8 read(offs_t offset);

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	void set_master_osc(double mosc) { m_mosc = mosc; }

protected:
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	required_device<input_merger_device> m_irq_merger;
	required_device_array<pia6821_device, 2> m_pia;
	required_device<ptm6840_device> m_ptm;

	sound_stream* m_stream;

private:
	void cmi01a_irq(int state);

	TIMER_CALLBACK_MEMBER(bcas_q2_tick);
	TIMER_CALLBACK_MEMBER(update_sample);

	void load_envelope();
	void update_envelope_tri();
	void clock_envelope();
	void tick_ediv();
	void update_eclk();

	void pulse_zcint();
	void pulse_gzx();
	void reset_waveform_segment();
	void check_segment_load();
	void wpe_w(int state);
	void notload_w(int state);
	int notload_r();

	void zx_tick();
	void run_voice();
	void update_wave_addr(int inc);
	void set_segment_cnt(u16 segment_cnt);

	void rp_w(u8 data);
	u8 rp_r();
	void ws_dir_w(u8 data);
	u8 ws_dir_r();
	int tri_r();
	void pia_0_cb2_w(int state);
	void eload_w(int state);
	int eload_r();

	int eosi_r();
	int zx_r();
	u8 pia_1_a_r();
	void pia_1_a_w(u8 data);
	u8 pia_1_b_r();
	void pia_1_b_w(u8 data);

	void ptm_o1(int state);
	void ptm_o2(int state);
	void ptm_o3(int state);

	TIMER_CALLBACK_MEMBER(zcint_pulse_cb);
	TIMER_CALLBACK_MEMBER(rstb_pulse_cb);

	void dump_state();

	u32         m_channel;
	double      m_mosc = 0.0;
	int         m_zx_ff = 0;
	bool        m_rstb = true;

	emu_timer * m_zcint_pulse_timer = nullptr;
	emu_timer * m_rstb_pulse_timer = nullptr;
	emu_timer * m_bcas_q2_timer = nullptr;
	emu_timer * m_sample_timer = nullptr;

	u8        m_current_sample = 0;
	u16       m_current_sample_addr = 0;

	std::unique_ptr<u8[]>    m_wave_ram;
	u16       m_segment_cnt = 0;
	int       m_new_addr = 0;
	u8        m_vol_latch = 0;
	u8        m_flt_latch = 0;
	u8        m_rp = 0;
	u8        m_ws = 0;
	int       m_dir = 0;
	int       m_env_dir = 0;
	u8        m_env = 0;
	int       m_pia0_cb2_state = 0;

	bool      m_bcas_q2 = false;
	bool      m_bcas_q1 = false;

	int       m_ptm_o1 = 0;
	int       m_ptm_o2 = 0;
	int       m_ptm_o3 = 0;

	bool      m_load = false;
	bool      m_nload = false;
	bool      m_run = false;
	bool      m_gzx = false;
	bool      m_wpe = false;
	bool      m_nwpe = true;
	bool      m_tri = false;
	bool      m_pia1_ca2 = false;

	bool      m_eclk = false;
	bool      m_env_clk = false;
	bool      m_ediv_out = false;
	u8        m_ediv_rate = 0;
	bool      m_envdiv_toggles[6];

	u16       m_pitch = 0;
	u8        m_octave = 0;

	devcb_write_line m_irq_cb;

	FILE *m_pitch_log = nullptr;
	int m_pitch_index = 1;
	int m_pitch_crossing = 1;
};

// device type definition
DECLARE_DEVICE_TYPE(CMI01A_CHANNEL_CARD, cmi01a_device)

#endif // MAME_AUDIO_CMI01A_H
