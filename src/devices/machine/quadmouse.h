// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/**********************************************************************

    Generic quadrature mouse axis emulation

    Note: Axis only, no buttons, do the buttons in the owning device

**********************************************************************/

#ifndef MAME_MACHINE_QUADMOUSE_H
#define MAME_MACHINE_QUADMOUSE_H

#pragma once

class quadencoder_device : public device_t
{
public:
	quadencoder_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	// Signal `mn` leads signal `pl` when the value increases, and vice versa
	// when it decreases.
	auto write_mn() { return m_mn_cb.bind(); }
	auto write_pl() { return m_pl_cb.bind(); }

	bool mn_r() { return m_mn; }
	bool pl_r() { return m_pl; }

	// This device can be associated with an input port by attaching this
	// callback to said port.
	DECLARE_INPUT_CHANGED_MEMBER(changed);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_write_line m_mn_cb, m_pl_cb;
	emu_timer *m_timer;
	attotime m_time;
	s32 m_delta;
	bool m_mn, m_pl;

	TIMER_CALLBACK_MEMBER(tick);
};

class quadmouse_device : public device_t
{
public:
	// construction/destruction
	quadmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto write_up()    { return m_up_cb.bind();    }
	auto write_down()  { return m_down_cb.bind();  }
	auto write_left()  { return m_left_cb.bind();  }
	auto write_right() { return m_right_cb.bind(); }

	bool up_r()     { return m_enc_y->mn_r(); }
	bool down_r()   { return m_enc_y->pl_r(); }
	bool left_r()   { return m_enc_x->mn_r(); }
	bool right_r()  { return m_enc_x->pl_r(); }

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

private:
	required_device<quadencoder_device> m_enc_x, m_enc_y;

	required_ioport m_port_x, m_port_y;

	devcb_write_line m_up_cb, m_down_cb, m_left_cb, m_right_cb;
};

DECLARE_DEVICE_TYPE(QUADENCODER, quadencoder_device)
DECLARE_DEVICE_TYPE(QUADMOUSE, quadmouse_device)

#endif // MAME_MACHINE_QUADMOUSE_H
