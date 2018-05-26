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
	DECLARE_READ8_MEMBER(opn_porta_r);
	DECLARE_WRITE8_MEMBER(opn_portb_w);

private:
	uint8_t m_joy_sel;
};

//DECLARE_DEVICE_TYPE(PC9801_SND, pc9801_snd_device)
INPUT_PORTS_EXTERN(pc9801_joy_port);

#endif
