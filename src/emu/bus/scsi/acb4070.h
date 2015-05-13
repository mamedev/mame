// license:BSD-3-Clause
// copyright-holders:smf
#pragma once

#ifndef __ACB4070__
#define __ACB4070__

#include "scsihd.h"

class acb4070_device : public scsihd_device
{
public:
	// construction/destruction
	acb4070_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void ExecCommand();
	virtual void WriteData( UINT8 *data, int dataLength );

private:
	struct adaptec_sense_t
	{
		// parameter list
		UINT8       reserved1[3];
		UINT8       length;

		// descriptor list
		UINT8       density;
		UINT8       reserved2[4];
		UINT8       block_size[3];

		// drive parameter list
		UINT8       format_code;
		UINT8       cylinder_count[2];
		UINT8       head_count;
		UINT8       reduced_write[2];
		UINT8       write_precomp[2];
		UINT8       landing_zone;
		UINT8       step_pulse_code;
		UINT8       bit_flags;
		UINT8       sectors_per_track;
	};
};

// device type definition
extern const device_type ACB4070;

#endif
