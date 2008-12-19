#pragma once

#ifndef __I386INTF_H__
#define __I386INTF_H__

#define INPUT_LINE_A20		1

#include "cpuintrf.h"

// mingw has this defined for 32-bit compiles
#undef i386

CPU_GET_INFO( i386 );
CPU_GET_INFO( i486 );
CPU_GET_INFO( pentium );
CPU_GET_INFO( mediagx );

#define CPU_I386 CPU_GET_INFO_NAME( i386 )
#define CPU_I486 CPU_GET_INFO_NAME( i486 )
#define CPU_PENTIUM CPU_GET_INFO_NAME( pentium )
#define CPU_MEDIAGX CPU_GET_INFO_NAME( mediagx )



#endif /* __I386INTF_H__ */
