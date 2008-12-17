#pragma once

#ifndef __I88INTF_H__
#define __I88INTF_H__

#include "i86intf.h"

/* Public functions */
CPU_GET_INFO( i8088 );
#define CPU_I8088 CPU_GET_INFO_NAME( i8088 )

#endif /* __I88INTF_H__ */
