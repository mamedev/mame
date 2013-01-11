/*
 * ACB4070 + RLL drive
 *
 */

#include "emu.h"
#include "machine/acb4070.h"

// device type definition
const device_type ACB4070 = &device_creator<acb4070_device>;

acb4070_device::acb4070_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: scsihd_device(mconfig, ACB4070, "ACB4070", tag, owner, clock)
{
}

#define ACB4070_CMD_WRITE_DATA_BUFFER ( 0x13 )
#define ACB4070_CMD_READ_DATA_BUFFER ( 0x14 )

#define TRANSFERLENGTH_DATA_BUFFER  0x0400

void acb4070_device::ExecCommand( int *transferLength )
{
	switch( command[ 0 ] )
	{
	case ACB4070_CMD_WRITE_DATA_BUFFER:
		SetPhase( SCSI_PHASE_DATAOUT );
		*transferLength = TRANSFERLENGTH_DATA_BUFFER;
		break;

	case ACB4070_CMD_READ_DATA_BUFFER:
		SetPhase( SCSI_PHASE_DATAIN );
		*transferLength = TRANSFERLENGTH_DATA_BUFFER;
		break;

	default:
		scsihd_device::ExecCommand( transferLength );
		break;
	}
}
