// license:BSD-3-Clause
// copyright-holders: AJR
/**********************************************************************

    OKI MSM6253 8-Bit 4-Channel A/D Converter

***********************************************************************
                              ____   ____
                /OSC OUT   1 |*   \_/    | 18  OSC OUT
                   D-GND   2 |           | 17  OSC IN
                   A-GND   3 |           | 16  /RD
                     IN0   4 |           | 15  /WR
                     IN1   5 | MSM6253RS | 14  ALE
                     IN2   6 |           | 13  /CS
                     IN3   7 |           | 12  A1
                      Vr   8 |           | 11  A0
                     Vdd   9 |___________| 10  S.O.

**********************************************************************/

#ifndef MAME_MACHINE_MSM6253_H
#define MAME_MACHINE_MSM6253_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> msm6253_device

class msm6253_device : public device_t
{
public:
	typedef device_delegate<ioport_value ()> port_read_delegate;

	// construction/destruction
	msm6253_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	template <unsigned P> void set_input_tag(const char *tag) { m_analog_ports[P].set_tag(tag); }
	template <unsigned P, typename... T> void set_input_cb(T &&... args) { m_analog_input_cb[P].set(std::forward<T>(args)...); }

	// write handlers
	WRITE8_MEMBER(address_w);
	WRITE8_MEMBER(select_w);

	// read handlers
	bool shift_out();
	READ8_MEMBER(d0_r);
	READ8_MEMBER(d7_r);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// helpers
	template<int port> ioport_value port_read();

	// input configuration
	optional_ioport_array<4> m_analog_ports;
	port_read_delegate::array<4> m_analog_input_cb;

	// private data
	u8 m_shift_register;
};

// device type definition
DECLARE_DEVICE_TYPE(MSM6253, msm6253_device)

#endif // DEVICES_MACHINE_MSM6253_H
