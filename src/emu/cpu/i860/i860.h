#pragma once

#ifndef __I860_H__
#define __I860_H__

#include "cpuintrf.h"

enum
{
	I860_PC = 0
};

CPU_GET_INFO( i860 );
#define CPU_I860 CPU_GET_INFO_NAME( i860 )

#endif /* __I860_H__ */

