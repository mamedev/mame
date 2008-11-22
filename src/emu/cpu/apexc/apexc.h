/* register names for apexc_get_reg & apexc_set_reg */
#pragma once

#ifndef __APEXC_H__
#define __APEXC_H__

enum
{
	APEXC_CR =1,	/* control register */
	APEXC_A,		/* acumulator */
	APEXC_R,		/* register */
	APEXC_ML,		/* memory location */
	APEXC_WS,		/* working store */
	APEXC_STATE,	/* whether CPU is running */

	APEXC_ML_FULL	/* read-only pseudo-register for exclusive use by the control panel code
                    in the apexc driver : enables it to get the complete address computed
                    from the contents of ML and WS */
};

CPU_GET_INFO( apexc );

#ifndef SUPPORT_ODD_WORD_SIZES
#define apexc_readmem(address)	memory_read_dword_32be(apexc.program, (address)<<2)
#define apexc_writemem(address, data)	memory_write_dword_32be(apexc.program, (address)<<2, (data))
/* eewww ! - Fortunately, there is no memory mapped I/O, so we can simulate masked write
without danger */
#define apexc_writemem_masked(address, data, mask)										\
	apexc_writemem((address), (apexc_readmem(address) & ~(mask)) | ((data) & (mask)))
#else
#define apexc_readmem(address)	cpu_readmem13_32(address)
#define apexc_writemem(address, data)	cpu_writemem13_32((address), (data))
#define apexc_writemem_masked(address, data, mask)	cpu_writemem13_32masked((address), (data), (mask))
#endif

CPU_DISASSEMBLE( apexc );

#define apexc_readop(address)	apexc_readmem(address)

#endif /* __APEXC_H__ */
