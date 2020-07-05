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

#define ENV_DIR_DOWN            0
#define ENV_DIR_UP              1

#define CHANNEL_STATUS_LOAD		1
#define CHANNEL_STATUS_RUN		2

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

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

protected:
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	static const device_timer_id TIMER_ZX = 0;
	static const device_timer_id TIMER_EOSI = 1;

	required_device<input_merger_device> m_irq_merger;
	required_device_array<pia6821_device, 2> m_pia;
	required_device<ptm6840_device> m_ptm;

	required_device_array<pia6821_device, 2> m_cmi02_pia;

	sound_stream* m_stream;

private:
	DECLARE_WRITE_LINE_MEMBER(cmi01a_irq);

	void tick_ediv();
	void set_eclk(bool eclk);

	void zx_timer_cb();
	void eosi_timer_cb();
	void run_voice();
	void update_wave_addr(int inc);

	uint32_t    m_channel;
	emu_timer * m_zx_timer;
	uint8_t     m_zx_flag;
	uint8_t     m_zx_ff;

	emu_timer * m_eosi_timer;

	std::unique_ptr<uint8_t[]>    m_wave_ram;
	uint16_t  m_segment_cnt;
	uint8_t   m_new_addr;     // Flag
	uint8_t   m_env_dir_ctrl;
	uint8_t   m_vol_latch;
	uint8_t   m_flt_latch;
	uint8_t	m_rp;
	uint8_t	m_ws;
	int     m_dir;
	int     m_pia0_cb2_state;

	double  m_freq;
	uint8_t m_status;

	int     m_ptm_o1;
	int     m_ptm_o2;
	int     m_ptm_o3;

	bool 	m_eclk;
	bool	m_ediv_out;

	devcb_write_line m_irq_cb;

	void rp_w(uint8_t data);
	void ws_dir_w(uint8_t data);
	DECLARE_READ_LINE_MEMBER( tri_r );
	DECLARE_WRITE_LINE_MEMBER( pia_0_ca2_w );
	DECLARE_WRITE_LINE_MEMBER( pia_0_cb2_w );

	DECLARE_READ_LINE_MEMBER( eosi_r );
	DECLARE_READ_LINE_MEMBER( zx_r );
	void pia_1_a_w(uint8_t data);
	void pia_1_b_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( ptm_o1 );
	DECLARE_WRITE_LINE_MEMBER( ptm_o2 );
	DECLARE_WRITE_LINE_MEMBER( ptm_o3 );
	DECLARE_WRITE_LINE_MEMBER( ptm_irq );
};

// device type definition
DECLARE_DEVICE_TYPE(CMI01A_CHANNEL_CARD, cmi01a_device)

#endif // MAME_AUDIO_CMI01A_H
