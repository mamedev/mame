// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Fairlight CMI-01A Channel Controller Card

***************************************************************************/

#ifndef MAME_AUDIO_CMI01A_H
#define MAME_AUDIO_CMI01A_H

#include "machine/6821pia.h"
#include "machine/6840ptm.h"

#define ENV_DIR_DOWN            0
#define ENV_DIR_UP              1


class cmi01a_device : public device_t, public device_sound_interface {
public:
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

	required_device_array<pia6821_device, 2> m_pia;
	required_device<ptm6840_device> m_ptm;

	required_device_array<pia6821_device, 2> m_cmi02_pia;

	sound_stream* m_stream;

private:
	DECLARE_WRITE_LINE_MEMBER(cmi01a_irq);

	void zx_timer_cb();
	void run_voice();
	void update_wave_addr(int inc);

	emu_timer * m_zx_timer;
	uint8_t       m_zx_flag;
	uint8_t       m_zx_ff;

	std::unique_ptr<uint8_t[]>    m_wave_ram;
	uint16_t  m_segment_cnt;
	uint8_t   m_new_addr;     // Flag
	uint8_t   m_env_dir_ctrl;
	uint8_t   m_vol_latch;
	uint8_t   m_flt_latch;
	uint8_t m_rp;
	uint8_t m_ws;
	int     m_dir;

	double  m_freq;
	bool    m_active;

	int     m_ptm_o1;

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
};

// device type definition
DECLARE_DEVICE_TYPE(CMI01A_CHANNEL_CARD, cmi01a_device)

#endif // MAME_AUDIO_CMI01A_H
