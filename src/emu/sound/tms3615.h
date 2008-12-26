#pragma once

#ifndef __TMS3615_H__
#define __TMS3615_H__

extern void tms3615_enable_w(int chip, int enable);

#define TMS3615_FOOTAGE_8	0
#define TMS3615_FOOTAGE_16	1

SND_GET_INFO( tms3615 );

#endif /* __TMS3615_H__ */
