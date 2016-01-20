// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Curt Coder
#pragma once

#ifndef __MSBC1__
#define __MSBC1__

#define MC68000R12_TAG  "u50"
#define MK68564_0_TAG   "u14"
#define MK68564_1_TAG   "u15"
#define MC68230L10_TAG  "u16"

class msbc1_state : public driver_device
{
public:
	msbc1_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,MC68000R12_TAG)
	{
	}

	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
};

#endif
