/***************************************************************************

    ppc.h

    Interface file for the universal machine language-based
    PowerPC emulator.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __PPC_H__
#define __PPC_H__



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* general constants */
#define PPC_MAX_FASTRAM			4
#define PPC_MAX_HOTSPOTS		16


/* interrupt types */
#define PPC_IRQ					0		/* external IRQ */
#define PPC_IRQ_LINE_0			0		/* (4XX) external IRQ0 */
#define PPC_IRQ_LINE_1			1		/* (4XX) external IRQ1 */
#define PPC_IRQ_LINE_2			2		/* (4XX) external IRQ2 */
#define PPC_IRQ_LINE_3			3		/* (4XX) external IRQ3 */
#define PPC_IRQ_LINE_4			4		/* (4XX) external IRQ4 */


/* register enumeration */
enum
{
	PPC_PC = 1,
	PPC_R0,
	PPC_R1,
	PPC_R2,
	PPC_R3,
	PPC_R4,
	PPC_R5,
	PPC_R6,
	PPC_R7,
	PPC_R8,
	PPC_R9,
	PPC_R10,
	PPC_R11,
	PPC_R12,
	PPC_R13,
	PPC_R14,
	PPC_R15,
	PPC_R16,
	PPC_R17,
	PPC_R18,
	PPC_R19,
	PPC_R20,
	PPC_R21,
	PPC_R22,
	PPC_R23,
	PPC_R24,
	PPC_R25,
	PPC_R26,
	PPC_R27,
	PPC_R28,
	PPC_R29,
	PPC_R30,
	PPC_R31,
	PPC_CR,
	PPC_LR,
	PPC_CTR,
	PPC_XER,

	PPC_F0,
	PPC_F1,
	PPC_F2,
	PPC_F3,
	PPC_F4,
	PPC_F5,
	PPC_F6,
	PPC_F7,
	PPC_F8,
	PPC_F9,
	PPC_F10,
	PPC_F11,
	PPC_F12,
	PPC_F13,
	PPC_F14,
	PPC_F15,
	PPC_F16,
	PPC_F17,
	PPC_F18,
	PPC_F19,
	PPC_F20,
	PPC_F21,
	PPC_F22,
	PPC_F23,
	PPC_F24,
	PPC_F25,
	PPC_F26,
	PPC_F27,
	PPC_F28,
	PPC_F29,
	PPC_F30,
	PPC_F31,
	PPC_FPSCR,

	PPC_MSR,
	PPC_SRR0,
	PPC_SRR1,
	PPC_SPRG0,
	PPC_SPRG1,
	PPC_SPRG2,
	PPC_SPRG3,
	PPC_SDR1,
	PPC_EXIER,
	PPC_EXISR,
	PPC_EVPR,
	PPC_IOCR,
	PPC_TBL,
	PPC_TBH,
	PPC_DEC,

	PPC_SR0,
	PPC_SR1,
	PPC_SR2,
	PPC_SR3,
	PPC_SR4,
	PPC_SR5,
	PPC_SR6,
	PPC_SR7,
	PPC_SR8,
	PPC_SR9,
	PPC_SR10,
	PPC_SR11,
	PPC_SR12,
	PPC_SR13,
	PPC_SR14,
	PPC_SR15
};


/* compiler-specific options */
#define PPCDRC_STRICT_VERIFY		0x0001			/* verify all instructions */
#define PPCDRC_FLUSH_PC				0x0002			/* flush the PC value before each memory access */
#define PPCDRC_ACCURATE_SINGLES		0x0004			/* do excessive rounding to make single-precision results "accurate" */


/* common sets of options */
#define PPCDRC_COMPATIBLE_OPTIONS	(PPCDRC_STRICT_VERIFY | PPCDRC_FLUSH_PC | PPCDRC_ACCURATE_SINGLES)
#define PPCDRC_FASTEST_OPTIONS		(0)



/***************************************************************************
    STRUCTURES AND TYPEDEFS
***************************************************************************/

typedef void (*ppc4xx_spu_tx_handler)(device_t *device, UINT8 data);

typedef struct _powerpc_config powerpc_config;
struct _powerpc_config
{
	UINT32		bus_frequency;
	read32_device_func	dcr_read_func;
	write32_device_func	dcr_write_func;
};

typedef void (*ppc_dcstore_handler)(device_t *device, UINT32 address);
typedef UINT32 (*ppc4xx_dma_read_handler)(device_t *device, int width);
typedef void (*ppc4xx_dma_write_handler)(device_t *device, int width, UINT32 data);


/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

void ppcdrc_set_options(device_t *device, UINT32 options);
void ppcdrc_add_fastram(device_t *device, offs_t start, offs_t end, UINT8 readonly, void *base);
void ppcdrc_add_hotspot(device_t *device, offs_t pc, UINT32 opcode, UINT32 cycles);

void ppc4xx_spu_set_tx_handler(device_t *device, ppc4xx_spu_tx_handler handler);
void ppc4xx_spu_receive_byte(device_t *device, UINT8 byteval);

void ppc_set_dcstore_callback(device_t *device, ppc_dcstore_handler handler);
void ppc4xx_set_dma_read_handler(device_t *device, int channel, ppc4xx_dma_read_handler handler, int rate);
void ppc4xx_set_dma_write_handler(device_t *device, int channel, ppc4xx_dma_write_handler handler, int rate);


DECLARE_LEGACY_CPU_DEVICE(PPC403GA, ppc403ga);
DECLARE_LEGACY_CPU_DEVICE(PPC403GCX, ppc403gcx);

DECLARE_LEGACY_CPU_DEVICE(PPC405GP, ppc405gp);

DECLARE_LEGACY_CPU_DEVICE(PPC601, ppc601);
DECLARE_LEGACY_CPU_DEVICE(PPC602, ppc602);
DECLARE_LEGACY_CPU_DEVICE(PPC603, ppc603);
DECLARE_LEGACY_CPU_DEVICE(PPC603E, ppc603e);
DECLARE_LEGACY_CPU_DEVICE(PPC603R, ppc603r);
DECLARE_LEGACY_CPU_DEVICE(PPC604, ppc604);
DECLARE_LEGACY_CPU_DEVICE(MPC8240, mpc8240);


#endif	/* __PPC_H__ */
