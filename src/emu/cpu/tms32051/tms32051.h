#pragma once

#ifndef __TMS32051_H__
#define __TMS32051_H__

#if (HAS_TMS32051)
void tms32051_get_info(UINT32 state, cpuinfo *info);
#endif

offs_t tms32051_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

#endif /* __TMS32051_H__ */
