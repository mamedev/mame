// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MSX_MSX1_BRUC100_H
#define MAME_MSX_MSX1_BRUC100_H

#include "bus/msx_slot/bruc100.h"
#include "msx.h"

class bruc100_state : public msx_state
{
public:
	bruc100_state(const machine_config &mconfig, device_type type, const char *tag);

	void bruc100(machine_config &config);
	void bruc100a(machine_config &config);

private:
	required_device<msx_slot_bruc100_device> m_bruc100_firm;

	void io_map(address_map &map);
	void port90_w(u8 data);
};

#endif // MAME_MSX_MSX1_BRUC100_H
