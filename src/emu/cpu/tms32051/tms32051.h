#ifndef _TMS32051_H
#define _TMS32051_H

#if (HAS_TMS32051)
void tms32051_get_info(UINT32 state, cpuinfo *info);
#endif

#ifdef MAME_DEBUG
offs_t tms32051_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif

#endif /* _TMS32051_H */
