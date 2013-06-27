/*****************************************************************************
 *
 *   sh4->h
 *   Portable Hitachi SH-4 (SH7750 family) emulator interface
 *
 *   By R. Belmont, based on sh2.c by Juergen Buchmueller, Mariusz Wojcieszek,
 *      Olivier Galibert, Sylvain Glaize, and James Forshaw.
 *
 *****************************************************************************/

#ifndef __SH4_H__
#define __SH4_H__


#define SH4_INT_NONE    -1
enum
{
	SH4_IRL0=0, SH4_IRL1, SH4_IRL2, SH4_IRL3, SH4_IRLn
};

enum
{
	SH4_PC=1, SH4_SR, SH4_PR, SH4_GBR, SH4_VBR, SH4_DBR, SH4_MACH, SH4_MACL,
	SH4_R0, SH4_R1, SH4_R2, SH4_R3, SH4_R4, SH4_R5, SH4_R6, SH4_R7,
	SH4_R8, SH4_R9, SH4_R10, SH4_R11, SH4_R12, SH4_R13, SH4_R14, SH4_R15, SH4_EA,
	SH4_R0_BK0, SH4_R1_BK0, SH4_R2_BK0, SH4_R3_BK0, SH4_R4_BK0, SH4_R5_BK0, SH4_R6_BK0, SH4_R7_BK0,
	SH4_R0_BK1, SH4_R1_BK1, SH4_R2_BK1, SH4_R3_BK1, SH4_R4_BK1, SH4_R5_BK1, SH4_R6_BK1, SH4_R7_BK1,
	SH4_SPC, SH4_SSR, SH4_SGR, SH4_FPSCR, SH4_FPUL, SH4_FR0, SH4_FR1, SH4_FR2, SH4_FR3, SH4_FR4, SH4_FR5,
	SH4_FR6, SH4_FR7, SH4_FR8, SH4_FR9, SH4_FR10, SH4_FR11, SH4_FR12, SH4_FR13, SH4_FR14, SH4_FR15,
	SH4_XF0, SH4_XF1, SH4_XF2, SH4_XF3, SH4_XF4, SH4_XF5, SH4_XF6, SH4_XF7,
	SH4_XF8, SH4_XF9, SH4_XF10, SH4_XF11, SH4_XF12, SH4_XF13, SH4_XF14, SH4_XF15
};

enum
{
	SH4_INTC_NMI=23,
	SH4_INTC_IRLn0,
	SH4_INTC_IRLn1,
	SH4_INTC_IRLn2,
	SH4_INTC_IRLn3,
	SH4_INTC_IRLn4,
	SH4_INTC_IRLn5,
	SH4_INTC_IRLn6,
	SH4_INTC_IRLn7,
	SH4_INTC_IRLn8,
	SH4_INTC_IRLn9,
	SH4_INTC_IRLnA,
	SH4_INTC_IRLnB,
	SH4_INTC_IRLnC,
	SH4_INTC_IRLnD,
	SH4_INTC_IRLnE,

	SH4_INTC_IRL0,
	SH4_INTC_IRL1,
	SH4_INTC_IRL2,
	SH4_INTC_IRL3,

	SH4_INTC_HUDI,
	SH4_INTC_GPOI,

	SH4_INTC_DMTE0,
	SH4_INTC_DMTE1,
	SH4_INTC_DMTE2,
	SH4_INTC_DMTE3,
	SH4_INTC_DMTE4,
	SH4_INTC_DMTE5,
	SH4_INTC_DMTE6,
	SH4_INTC_DMTE7,

	SH4_INTC_DMAE,

	SH4_INTC_TUNI3,
	SH4_INTC_TUNI4,
	SH4_INTC_TUNI0,
	SH4_INTC_TUNI1,
	SH4_INTC_TUNI2,
	SH4_INTC_TICPI2,
	SH4_INTC_ATI,
	SH4_INTC_PRI,
	SH4_INTC_CUI,
	SH4_INTC_SCI1ERI,
	SH4_INTC_SCI1RXI,

	SH4_INTC_SCI1TXI,
	SH4_INTC_SCI1TEI,
	SH4_INTC_SCIFERI,
	SH4_INTC_SCIFRXI,
	SH4_INTC_SCIFBRI,
	SH4_INTC_SCIFTXI,
	SH4_INTC_ITI,
	SH4_INTC_RCMI,
	SH4_INTC_ROVI
};

#define SH4_FPU_PZERO 0
#define SH4_FPU_NZERO 1
#define SH4_FPU_DENORM 2
#define SH4_FPU_NORM 3
#define SH4_FPU_PINF 4
#define SH4_FPU_NINF 5
#define SH4_FPU_qNaN 6
#define SH4_FPU_sNaN 7

enum
{
	SH4_IOPORT_16=8*0,
	SH4_IOPORT_4=8*1,
	SH4_IOPORT_DMA=8*2,
	// future use
	SH4_IOPORT_SCI=8*3,
	SH4_IOPORT_SCIF=8*4
};

struct sh4_config
{
	int md2;
	int md1;
	int md0;
	int md6;
	int md4;
	int md3;
	int md5;
	int md7;
	int md8;
	int clock;
};

struct sh4_device_dma
{
	UINT32 length;
	UINT32 size;
	void *buffer;
	int channel;
};

struct sh4_ddt_dma
{
	UINT32 source;
	UINT32 length;
	UINT32 size;
	UINT32 destination;
	void *buffer;
	int direction;
	int channel;
	int mode;
};

typedef void (*sh4_ftcsr_callback)(UINT32);

DECLARE_LEGACY_CPU_DEVICE(SH3LE, sh3);
DECLARE_LEGACY_CPU_DEVICE(SH3BE, sh3be);
DECLARE_LEGACY_CPU_DEVICE(SH4LE, sh4);
DECLARE_LEGACY_CPU_DEVICE(SH4BE, sh4be);

DECLARE_WRITE32_HANDLER( sh4_internal_w );
DECLARE_READ32_HANDLER( sh4_internal_r );

DECLARE_WRITE32_HANDLER( sh3_internal_w );
DECLARE_READ32_HANDLER( sh3_internal_r );

DECLARE_WRITE32_HANDLER( sh3_internal_high_w );
DECLARE_READ32_HANDLER( sh3_internal_high_r );


void sh4_set_frt_input(device_t *device, int state);
void sh4_set_irln_input(device_t *device, int value);
void sh4_set_ftcsr_callback(device_t *device, sh4_ftcsr_callback callback);
int sh4_dma_data(device_t *device, struct sh4_device_dma *s);
void sh4_dma_ddt(device_t *device, struct sh4_ddt_dma *s);

/***************************************************************************
    COMPILER-SPECIFIC OPTIONS
***************************************************************************/

#define SH4DRC_STRICT_VERIFY    0x0001          /* verify all instructions */
#define SH4DRC_FLUSH_PC         0x0002          /* flush the PC value before each memory access */
#define SH4DRC_STRICT_PCREL     0x0004          /* do actual loads on MOVLI/MOVWI instead of collapsing to immediates */

#define SH4DRC_COMPATIBLE_OPTIONS   (SH4DRC_STRICT_VERIFY | SH4DRC_FLUSH_PC | SH4DRC_STRICT_PCREL)
#define SH4DRC_FASTEST_OPTIONS  (0)

void sh4drc_set_options(device_t *device, UINT32 options);
void sh4drc_add_pcflush(device_t *device, offs_t address);

#endif /* __SH4_H__ */
