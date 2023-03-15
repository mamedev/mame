// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    NEC PC-9801
    common functions for CBUS sound boards -26, -86, -118

***************************************************************************/

#ifndef MAME_BUS_CBUS_PC9801_SND_H
#define MAME_BUS_CBUS_PC9801_SND_H

#pragma once

class pc9801_snd_device : public device_t
{
public:
	pc9801_snd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

protected:
	u8 opn_porta_r();
	void opn_portb_w(u8 data);
	u16 m_io_base;
	virtual u16 read_io_base() = 0;

private:
	u8 m_joy_sel;
};

//DECLARE_DEVICE_TYPE(PC9801_SND, pc9801_snd_device)
INPUT_PORTS_EXTERN(pc9801_joy_port);

#endif
