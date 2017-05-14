// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_BUS_SCSI_ACB4070_H
#define MAME_BUS_SCSI_ACB4070_H

#pragma once

#include "scsihd.h"

class acb4070_device : public scsihd_device
{
public:
	// construction/destruction
	acb4070_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void ExecCommand() override;
	virtual void WriteData( uint8_t *data, int dataLength ) override;

private:
	struct adaptec_sense_t
	{
		// parameter list
		uint8_t       reserved1[3];
		uint8_t       length;

		// descriptor list
		uint8_t       density;
		uint8_t       reserved2[4];
		uint8_t       block_size[3];

		// drive parameter list
		uint8_t       format_code;
		uint8_t       cylinder_count[2];
		uint8_t       head_count;
		uint8_t       reduced_write[2];
		uint8_t       write_precomp[2];
		uint8_t       landing_zone;
		uint8_t       step_pulse_code;
		uint8_t       bit_flags;
		uint8_t       sectors_per_track;
	};
};

// device type definition
DECLARE_DEVICE_TYPE(ACB4070, acb4070_device)

#endif // MAME_BUS_SCSI_ACB4070_H
