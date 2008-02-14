#ifndef __I386INTF_H
#define __I386INTF_H

#include "cpuintrf.h"

void i386_get_info(UINT32, cpuinfo*);
void i486_get_info(UINT32, cpuinfo*);
void pentium_get_info(UINT32, cpuinfo*);
void mediagx_get_info(UINT32, cpuinfo*);

#endif /* __I386INTF_H */
