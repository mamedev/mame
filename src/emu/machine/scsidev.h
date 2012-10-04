/***************************************************************************

 scsidev.h

***************************************************************************/

#ifndef _SCSIDEV_H_
#define _SCSIDEV_H_

#include "emu.h"

// base handler
class scsidev_device : public device_t
{
public:
	// construction/destruction
	scsidev_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
};

#endif
