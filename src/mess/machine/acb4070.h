#pragma once

#ifndef __ACB4070__
#define __ACB4070__

#include "machine/scsihd.h"

class acb4070_device : public scsihd_device
{
public:
	// construction/destruction
	acb4070_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void ExecCommand( int *transferLength );
};

// device type definition
extern const device_type ACB4070;

#endif
