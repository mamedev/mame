#pragma once

#ifndef __TMS32051_H__
#define __TMS32051_H__

CPU_GET_INFO( tms32051 );
#define CPU_TMS32051 CPU_GET_INFO_NAME( tms32051 )

CPU_DISASSEMBLE( tms32051 );

#endif /* __TMS32051_H__ */
