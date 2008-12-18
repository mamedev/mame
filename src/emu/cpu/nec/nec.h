/* ASG 971222 -- rewrote this interface */
#ifndef __NEC_H_
#define __NEC_H_

#include "cpuintrf.h"

typedef struct _nec_config nec_config;
struct _nec_config
{
	const UINT8*	v25v35_decryptiontable; // internal decryption table
};

#define NEC_INPUT_LINE_POLL 20

enum
{
	NEC_PC=0,
	NEC_IP, NEC_AW, NEC_CW, NEC_DW, NEC_BW, NEC_SP, NEC_BP, NEC_IX, NEC_IY,
	NEC_FLAGS, NEC_ES, NEC_CS, NEC_SS, NEC_DS,
	NEC_VECTOR, NEC_PENDING
};

/* Public functions */
extern CPU_GET_INFO( v20 );
extern CPU_GET_INFO( v25 );
extern CPU_GET_INFO( v30 );
extern CPU_GET_INFO( v33 );
extern CPU_GET_INFO( v35 );

#define CPU_V20 CPU_GET_INFO_NAME( v20 )
#define CPU_V25 CPU_GET_INFO_NAME( v25 )
#define CPU_V30 CPU_GET_INFO_NAME( v30 )
#define CPU_V33 CPU_GET_INFO_NAME( v33 )
#define CPU_V35 CPU_GET_INFO_NAME( v35 )

#endif
