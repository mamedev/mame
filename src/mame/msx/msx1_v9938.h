// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MSX_MSX1_V9938_H
#define MAME_MSX_MSX1_V9938_H

#include "msx.h"

class msx1_v9938_state : public msx_state
{
public:
	msx1_v9938_state(const machine_config &mconfig, device_type type, const char *tag);

	void ax200(machine_config &mconfig);
	void ax200m(machine_config &mconfig);
	void cx5m128(machine_config &config);
	void cx5miib(machine_config &config);
	void svi738(machine_config &config);
	void svi738ar(machine_config &config);
	void tadpc200a(machine_config &config);
	void y503iir(machine_config &config);
	void y503iir2(machine_config &config);
	void yis503ii(machine_config &config);

protected:
	void msx1_v9938(ay8910_type ay8910_type, machine_config &config);
	void msx1_v9938_pal(ay8910_type ay8910_type, machine_config &config);

	void io_map(address_map &map);

	optional_device<v9938_device> m_v9938;
};

#endif // MAME_MSX_MSX1_V9938_H
