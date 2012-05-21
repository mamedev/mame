/**  Konami 053252  **/
/* CRT and interrupt control unit */
#pragma once

#ifndef __K053252_H__
#define __K053252_H__

#include "devlegcy.h"

DECLARE_LEGACY_DEVICE(K053252, k053252);



typedef struct _k053252_interface k053252_interface;
struct _k053252_interface
{
	const char         *screen;
	devcb_write_line   int1_en;
	devcb_write_line   int2_en;
	devcb_write_line   int1_ack;
	devcb_write_line   int2_ack;
//  devcb_write8       int_time;
	int                offsx;
	int                offsy;
};


#define MCFG_K053252_ADD(_tag, _clock, _interface) \
	MCFG_DEVICE_ADD(_tag, K053252, _clock) \
	MCFG_DEVICE_CONFIG(_interface)

/**  Konami 053252  **/
/* CRT and interrupt control unit */
READ8_DEVICE_HANDLER( k053252_r );	// CCU registers
WRITE8_DEVICE_HANDLER( k053252_w );



#endif	/* __K033906_H__ */
