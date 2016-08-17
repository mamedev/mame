// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef PC_JOY_SW_H_
#define PC_JOY_SW_H_

#include "pc_joy.h"

class pc_mssw_pad_device :  public device_t,
							public device_pc_joy_interface
{
public:
	pc_mssw_pad_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ioport_constructor device_input_ports() const override;

	virtual UINT8 btn() override { return m_state; }
	// timing is guessed, calibrated for at486
	virtual void port_write() override { if(!m_active) { m_timer->adjust(attotime::from_usec(50), 0, attotime::from_usec(5)); m_active = true; } }

protected:
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr) override;
	virtual void device_reset() override;

private:
	required_ioport m_btn1;
	required_ioport m_btn2;
	required_ioport m_btn3;
	required_ioport m_btn4;
	required_ioport m_conf;
	emu_timer *m_timer;
	int m_count;
	UINT8 m_state;
	bool m_active;
};

extern const device_type PC_MSSW_PAD;

#endif /* PC_JOY_SW_H_ */
