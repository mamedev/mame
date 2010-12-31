#pragma once

#ifndef __TMS57002_H__
#define __TMS57002_H__


enum {
	TMS57002_PC=1
};

DECLARE_LEGACY_CPU_DEVICE(TMS57002, tms57002);

WRITE8_DEVICE_HANDLER(tms57002_data_w);
READ8_DEVICE_HANDLER(tms57002_data_r);

WRITE8_DEVICE_HANDLER(tms57002_pload_w);
WRITE8_DEVICE_HANDLER(tms57002_cload_w);
READ8_DEVICE_HANDLER(tms57002_empty_r);
READ8_DEVICE_HANDLER(tms57002_dready_r);

void tms57002_sync(device_t *cpu);

#endif

