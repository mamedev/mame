#ifndef UPD71071_H_
#define UPD71071_H_

#include "emu.h"

typedef struct _upd71071_interface upd71071_intf;
struct _upd71071_interface
{
	const char* cputag;
	int clock;
	UINT16 (*dma_read[4])(running_machine &machine);
	void (*dma_write[4])(running_machine &machine, UINT16 data);
};

int upd71071_dmarq(device_t* device,int state,int channel);

DECLARE_LEGACY_DEVICE(UPD71071, upd71071);

#define MCFG_UPD71071_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, UPD71071, 0) \
	MCFG_DEVICE_CONFIG(_config)

READ8_DEVICE_HANDLER(upd71071_r);
WRITE8_DEVICE_HANDLER(upd71071_w);

#endif /*UPD71071_H_*/
