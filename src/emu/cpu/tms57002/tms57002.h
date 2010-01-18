#pragma once

#ifndef __TMS57002_H__
#define __TMS57002_H__


enum {
	TMS57002_PC=1
};

CPU_GET_INFO(tms57002);
#define CPU_TMS57002 CPU_GET_INFO_NAME(tms57002)

WRITE8_DEVICE_HANDLER(tms57002_data_w);
READ8_DEVICE_HANDLER(tms57002_data_r);

WRITE8_DEVICE_HANDLER(tms57002_pload_w);
WRITE8_DEVICE_HANDLER(tms57002_cload_w);
READ8_DEVICE_HANDLER(tms57002_empty_r);
READ8_DEVICE_HANDLER(tms57002_dready_r);

void tms57002_sync(running_device *cpu);

#endif

