// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_MACHINE_MAX186_H
#define MAME_MACHINE_MAX186_H

#pragma once

// Maxim MAX186 eight-channel, 12-bit serial ADC.
class max186_device : public device_t
{
public:
	max186_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// Input voltages relative to VREF (1.0 = full scale in unipolar mode).
	using input_delegate = device_delegate<double (u8 channel)>;
	template <typename... T> void set_input_callback(T &&... args) { m_input_cb.set(std::forward<T>(args)...); }

	void cs_w(int state);
	void din_w(int state) { m_din = bool(state); }
	void sclk_w(int state);
	void shdn_w(int state);
	int dout_r() const { return m_cs || !m_shdn ? 1 : m_dout; }
	int sstrb_r() const { return (m_cs && m_external_clock) || !m_shdn ? 1 : m_sstrb; }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	void convert();
	TIMER_CALLBACK_MEMBER(conversion_done);

	input_delegate m_input_cb;
	emu_timer *m_timer;
	u16 m_result;
	u8 m_command;
	u8 m_command_bits;
	u8 m_output_bits;
	bool m_cs;
	bool m_din;
	bool m_sclk;
	bool m_shdn;
	bool m_dout;
	bool m_sstrb;
	bool m_external_clock;
	bool m_busy;
	bool m_restart;
};

DECLARE_DEVICE_TYPE(MAX186, max186_device)

#endif // MAME_MACHINE_MAX186_H
