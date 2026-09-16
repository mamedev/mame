// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Sony CXD1095 CMOS I/O Port Expander

    CXD1095Q: 64-pin quad flat package

       1-2              NC
       3-9              PB1-PB7
        10              GND
     11-18              PC0-PC7
        19              NC

     20-24              PD0-PD4
        25              GND
        26              Vdd
     27-29              PD5-PD7
     30-32              D0-D2

     33-34              NC
     35-39              D3-D7
        40              /CLR
        41              /RST
        42              GND
        43              /WR
        44              /RD
        45              /CS
     46-48              A0-A2
     49-50              PX0-PX1
        51              NC

     52-53              PX2-PX3
     54-56              PA0-PA2
        57              GND
        58              Vdd
     59-63              PA3-PA7
        64              PB0

**********************************************************************/

#ifndef MAME_MACHINE_CXD1095_H
#define MAME_MACHINE_CXD1095_H

#pragma once

class cxd1095_device : public device_t
{
public:
	// construction/destruction
	cxd1095_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration
	template <std::size_t Port> auto in_port_cb() { static_assert(Port >= 0 && Port < 5, "invalid port"); return m_input_cb[Port].bind(); }
	template <std::size_t Port> auto out_port_cb() { static_assert(Port >= 0 && Port < 5, "invalid port"); return m_output_cb[Port].bind(); }
	auto in_porta_cb() { return in_port_cb<0>(); }
	auto in_portb_cb() { return in_port_cb<1>(); }
	auto in_portc_cb() { return in_port_cb<2>(); }
	auto in_portd_cb() { return in_port_cb<3>(); }
	auto in_porte_cb() { return in_port_cb<4>(); }
	auto out_porta_cb() { return out_port_cb<0>(); }
	auto out_portb_cb() { return out_port_cb<1>(); }
	auto out_portc_cb() { return out_port_cb<2>(); }
	auto out_portd_cb() { return out_port_cb<3>(); }
	auto out_porte_cb() { return out_port_cb<4>(); }

	// memory handlers
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// input/output callbacks
	devcb_read8::array<5> m_input_cb;
	devcb_write8::array<5> m_output_cb;

	// internal state
	u8                  m_data_latch[5];
	u8                  m_data_dir[5];
};

DECLARE_DEVICE_TYPE(CXD1095, cxd1095_device)

#endif // MAME_MACHINE_CXD1095_H
