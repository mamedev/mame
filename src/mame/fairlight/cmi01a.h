// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Fairlight CMI-01A Channel Controller Card

***************************************************************************/

#ifndef MAME_FAIRLIGHT_CMI01A_H
#define MAME_FAIRLIGHT_CMI01A_H

#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/input_merger.h"

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

	virtual void sound_stream_update(sound_stream &stream) override;

	void set_master_osc(double mosc) { m_mosc = mosc; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<input_merger_device> m_irq_merger;
	required_device_array<pia6821_device, 2> m_pia;
	required_device<ptm6840_device> m_ptm;

	sound_stream* m_stream;

private:
	enum : int
	{
		ENV_DIR_UP,
		ENV_DIR_DOWN
	};

	void cmi01a_irq(int state);

	TIMER_CALLBACK_MEMBER(update_sample);

	void notload_w(int state);
	int notload_r();

	void run_voice();

	void rp_w(u8 data);
	u8 rp_r();
	void ws_dir_w(u8 data);
	u8 ws_dir_r();
	int tri_r();
	void run_w(int state);
	void permit_eload_w(int state);

	int eosi_r();
	int zx_r();
	u8 pitch_octave_r();
	void pitch_octave_w(u8 data);
	u8 pitch_lsb_r();
	void pitch_lsb_w(u8 data);

	void ptm_o1(int state);
	void ptm_o2(int state);
	void ptm_o3(int state);

	// New functions below this line
	void update_rstb_pulser();
	void set_run_load_xor(const bool run_load_xor);
	TIMER_CALLBACK_MEMBER(rstb_pulse_cb);
	void set_not_rstb(const bool not_rstb);

	void update_bcas_q1_enable();
	TIMER_CALLBACK_MEMBER(bcas_q1_tick);

	void set_zx_flipflop_clock(const bool zx_ff_clk);
	void set_zx_flipflop_state(const bool zx_ff);

	void pulse_zcint();
	TIMER_CALLBACK_MEMBER(zcint_pulse_cb);
	void set_not_zcint(const bool not_zcint);

	void set_not_load(const bool not_load);

	void update_gzx();
	void set_gzx(const bool gzx);

	void update_not_eload();
	void set_not_eload(const bool not_eload);
	void try_load_envelope();
	void set_envelope(const u8 env);
	void update_envelope_divider();
	void set_envelope_dir(const int env_dir);
	void update_envelope_clock();
	void clock_envelope();
	void tick_ediv();

	void update_envelope_tri();
	void not_wpe_w(int state);
	void update_upper_wave_addr_load();
	void set_upper_wave_addr_load(const bool upper_wave_addr_load);
	void try_load_upper_wave_addr();
	void set_wave_addr_lsb(const u8 wave_addr_lsb);
	void set_wave_addr_msb(const u8 wave_addr_msb);
	void set_wave_addr_msb_clock(const bool wave_addr_msb_clock);
	void update_filters();

	void set_zx(const bool zx);
	void update_ptm_c1();

	u32         m_channel;

	emu_timer * m_zcint_pulse_timer = nullptr;
	emu_timer * m_rstb_pulse_timer = nullptr;
	emu_timer * m_bcas_q1_timer = nullptr;
	emu_timer * m_sample_timer = nullptr;

	devcb_write_line m_irq_cb;

	std::unique_ptr<u8[]>    m_wave_ram;
	u8          m_current_sample = 0;

	double      m_mosc = 0.0;
	u16         m_pitch = 0;
	u8          m_octave = 0;

	bool        m_zx_ff_clk = false;
	bool        m_zx_ff = false;
	bool        m_zx = false;
	bool        m_gzx = false;

	bool        m_run = false;
	bool        m_not_rstb = true;
	bool        m_not_load = false;
	bool        m_not_zcint = true;
	bool        m_not_wpe = true;
	bool        m_new_addr = false;

	bool        m_tri = false;
	bool        m_permit_eload = false;
	bool        m_not_eload = true;

	bool        m_bcas_q1_enabled = true;
	bool        m_bcas_q1 = false;
	bool        m_bcas_q2 = false;

	int         m_env_dir = 0;
	u8          m_env = 0;
	u8          m_env_divider = 0;
	bool        m_ediv_out = false;
	bool        m_envdiv_toggles[6];
	bool        m_eclk = false;
	bool        m_env_clk = false;

	u8          m_wave_addr_lsb = 0;
	u8          m_wave_addr_msb = 0;
	bool        m_upper_wave_addr_load = false;
	bool        m_wave_addr_msb_clock = true;
	bool        m_run_load_xor = true;
	bool        m_delayed_inverted_run_load = false;

	bool        m_ptm_c1 = false;
	bool        m_ptm_o1 = false;
	bool        m_ptm_o2 = false;
	bool        m_ptm_o3 = false;

	u8          m_vol_latch = 0;
	u8          m_flt_latch = 0;
	u8          m_rp = 0;
	u8          m_ws = 0;
	int         m_dir = 0;

	double      m_ha0 = 0;
	double      m_ha1 = 0;
	double      m_hb0 = 0;
	double      m_hb1 = 0;
	double      m_hc0 = 0;
	double      m_hc1 = 0;

	double      m_ka0 = 0;
	double      m_ka1 = 0;
	double      m_ka2 = 0;
	double      m_kb0 = 0;
	double      m_kb1 = 0;
	double      m_kb2 = 0;
};

// device type definition
DECLARE_DEVICE_TYPE(CMI01A_CHANNEL_CARD, cmi01a_device)

#endif // MAME_FAIRLIGHT_CMI01A_H
