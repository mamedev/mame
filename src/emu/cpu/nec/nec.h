/* ASG 971222 -- rewrote this interface */
#ifndef __NEC_H_
#define __NEC_H_


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
DECLARE_LEGACY_CPU_DEVICE(V20, v20);
DECLARE_LEGACY_CPU_DEVICE(V25, v25);
DECLARE_LEGACY_CPU_DEVICE(V30, v30);
DECLARE_LEGACY_CPU_DEVICE(V33, v33);
DECLARE_LEGACY_CPU_DEVICE(V35, v35);

#endif
