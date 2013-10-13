/***************************************************************************

    cdu76s.h

    Sony CDU-76S

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __CDU76S_H__
#define __CDU76S_H__

#include "scsicd.h"
#include "t10mmc.h"

class sony_cdu76s_device : public scsicd_device
{
public:
	sony_cdu76s_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void ExecCommand();
	virtual void ReadData( UINT8 *data, int dataLength );
};

// device type definition
extern const device_type CDU76S;

#endif
