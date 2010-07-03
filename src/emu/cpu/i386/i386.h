#pragma once

#ifndef __I386INTF_H__
#define __I386INTF_H__

#define INPUT_LINE_A20		1


// mingw has this defined for 32-bit compiles
#undef i386

DECLARE_LEGACY_CPU_DEVICE(I386, i386);
DECLARE_LEGACY_CPU_DEVICE(I486, i486);
DECLARE_LEGACY_CPU_DEVICE(PENTIUM, pentium);
DECLARE_LEGACY_CPU_DEVICE(MEDIAGX, mediagx);



#endif /* __I386INTF_H__ */
