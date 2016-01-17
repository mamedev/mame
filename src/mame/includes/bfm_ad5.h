// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "cpu/m68000/m68000.h"
#include "includes/bfm_sc45.h"


class adder5_state : public driver_device
{
public:
	adder5_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
public:
	DECLARE_DRIVER_INIT(ad5);
	INTERRUPT_GEN_MEMBER(ad5_fake_timer_int);
};
