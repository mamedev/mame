#pragma once

#ifndef __MSBC1__
#define __MSBC1__

#define MC68000R12_TAG	"u50"
#define MK68564_0_TAG	"u14"
#define MK68564_1_TAG	"u15"
#define MC68230L10_TAG	"u16"

class msbc1_state : public driver_device
{
public:
	msbc1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	virtual void machine_reset();
};

#endif
