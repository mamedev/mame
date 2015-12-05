// license:BSD-3-Clause
// copyright-holders:smf
#pragma once

#ifndef __D9060HD__
#define __D9060HD__

#include "scsihd.h"

class d9060hd_device : public scsihd_device
{
public:
	// construction/destruction
	d9060hd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void ExecCommand() override;
};

// device type definition
extern const device_type D9060HD;

#endif
