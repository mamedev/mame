// license:BSD-3-Clause
// copyright-holders:tim lindner
/**********************************************************************

    Radio Shack Color Graphics Printer CGP-220, parallel interface

**********************************************************************/

#ifndef MAME_BUS_CENTRONICS_CGP220_H
#define MAME_BUS_CENTRONICS_CGP220_H

#pragma once

#include "ctronics.h"
#include "machine/cgp220.h"


class centronics_cgp220_device : public device_t, public device_centronics_peripheral_interface
{
public:
	centronics_cgp220_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void input_strobe(int state) override { m_printer->strobe_w(state); }
	virtual void input_data0(int state) override { m_printer->data_bit_w<0>(state); }
	virtual void input_data1(int state) override { m_printer->data_bit_w<1>(state); }
	virtual void input_data2(int state) override { m_printer->data_bit_w<2>(state); }
	virtual void input_data3(int state) override { m_printer->data_bit_w<3>(state); }
	virtual void input_data4(int state) override { m_printer->data_bit_w<4>(state); }
	virtual void input_data5(int state) override { m_printer->data_bit_w<5>(state); }
	virtual void input_data6(int state) override { m_printer->data_bit_w<6>(state); }
	virtual void input_data7(int state) override { m_printer->data_bit_w<7>(state); }

private:
	required_device<cgp220_device> m_printer;
};

DECLARE_DEVICE_TYPE(CENTRONICS_CGP220, centronics_cgp220_device)

#endif // MAME_BUS_CENTRONICS_CGP220_H
