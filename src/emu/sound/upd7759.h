#pragma once

#ifndef __UPD7759_H__
#define __UPD7759_H__

/* There are two modes for the uPD7759, selected through the !MD pin.
   This is the mode select input.  High is stand alone, low is slave.
   We're making the assumption that nobody switches modes through
   software. */

#define UPD7759_STANDARD_CLOCK		640000

typedef struct _upd7759_interface upd7759_interface;
struct _upd7759_interface
{
	void (*drqcallback)(const device_config *device, int param);	/* drq callback (per chip, slave mode only) */
};

void upd7759_set_bank_base(int which, offs_t base);

void upd7759_reset_w(int num, UINT8 data);

void upd7759_port_w(int num, UINT8 data);
void upd7759_start_w(int num, UINT8 data);
int upd7759_busy_r(int num);

WRITE8_HANDLER( upd7759_0_reset_w );
WRITE8_HANDLER( upd7759_0_port_w );
WRITE8_HANDLER( upd7759_0_start_w );
READ8_HANDLER( upd7759_0_busy_r );

#endif /* __UPD7759_H__ */
