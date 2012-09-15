/***************************************************************************

    cubeqcpu.h
    Interface file for the Cube Quest CPUs
    Written by Phil Bennett

***************************************************************************/

#ifndef _CUBEQCPU_H
#define _CUBEQCPU_H


/***************************************************************************
    COMPILE-TIME DEFINITIONS
***************************************************************************/


/***************************************************************************
    GLOBAL CONSTANTS
***************************************************************************/


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	CQUESTSND_PC = 1,
	CQUESTSND_Q,
	CQUESTSND_RAM0,
	CQUESTSND_RAM1,
	CQUESTSND_RAM2,
	CQUESTSND_RAM3,
	CQUESTSND_RAM4,
	CQUESTSND_RAM5,
	CQUESTSND_RAM6,
	CQUESTSND_RAM7,
	CQUESTSND_RAM8,
	CQUESTSND_RAM9,
	CQUESTSND_RAMA,
	CQUESTSND_RAMB,
	CQUESTSND_RAMC,
	CQUESTSND_RAMD,
	CQUESTSND_RAME,
	CQUESTSND_RAMF,
	CQUESTSND_RTNLATCH,
	CQUESTSND_ADRCNTR,
	CQUESTSND_DINLATCH,
};

enum
{
	CQUESTROT_PC = 1,
	CQUESTROT_Q,
	CQUESTROT_RAM0,
	CQUESTROT_RAM1,
	CQUESTROT_RAM2,
	CQUESTROT_RAM3,
	CQUESTROT_RAM4,
	CQUESTROT_RAM5,
	CQUESTROT_RAM6,
	CQUESTROT_RAM7,
	CQUESTROT_RAM8,
	CQUESTROT_RAM9,
	CQUESTROT_RAMA,
	CQUESTROT_RAMB,
	CQUESTROT_RAMC,
	CQUESTROT_RAMD,
	CQUESTROT_RAME,
	CQUESTROT_RAMF,
	CQUESTROT_SEQCNT,
	CQUESTROT_DYNADDR,
	CQUESTROT_DYNDATA,
	CQUESTROT_YRLATCH,
	CQUESTROT_YDLATCH,
	CQUESTROT_DINLATCH,
	CQUESTROT_DSRCLATCH,
	CQUESTROT_RSRCLATCH,
	CQUESTROT_LDADDR,
	CQUESTROT_LDDATA,
};

enum
{
	CQUESTLIN_FGPC = 1,
	CQUESTLIN_BGPC,
	CQUESTLIN_Q,
	CQUESTLIN_RAM0,
	CQUESTLIN_RAM1,
	CQUESTLIN_RAM2,
	CQUESTLIN_RAM3,
	CQUESTLIN_RAM4,
	CQUESTLIN_RAM5,
	CQUESTLIN_RAM6,
	CQUESTLIN_RAM7,
	CQUESTLIN_RAM8,
	CQUESTLIN_RAM9,
	CQUESTLIN_RAMA,
	CQUESTLIN_RAMB,
	CQUESTLIN_RAMC,
	CQUESTLIN_RAMD,
	CQUESTLIN_RAME,
	CQUESTLIN_RAMF,
	CQUESTLIN_FADLATCH,
	CQUESTLIN_BADLATCH,
	CQUESTLIN_SREG,
	CQUESTLIN_XCNT,
	CQUESTLIN_YCNT,
	CQUESTLIN_CLATCH,
	CQUESTLIN_ZLATCH,
};


/***************************************************************************
    CONFIGURATION STRUCTURE
***************************************************************************/

typedef void (*cubeqst_dac_w_func)(device_t *, UINT16);

struct cubeqst_snd_config
{
	cubeqst_dac_w_func	dac_w;
	const char *		sound_data_region;

};

struct cubeqst_lin_config
{
	const char *		rot_cpu_tag;
};

struct cubeqst_rot_config
{
	const char *		lin_cpu_tag;
};

/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

extern READ16_DEVICE_HANDLER( cubeqcpu_sndram_r );
extern WRITE16_DEVICE_HANDLER( cubeqcpu_sndram_w );

extern READ16_DEVICE_HANDLER( cubeqcpu_rotram_r );
extern WRITE16_DEVICE_HANDLER( cubeqcpu_rotram_w );

void cubeqcpu_swap_line_banks(device_t *device);

void cubeqcpu_clear_stack(device_t *device);
UINT8 cubeqcpu_get_ptr_ram_val(device_t *device, int i);
UINT32* cubeqcpu_get_stack_ram(device_t *device);

DECLARE_LEGACY_CPU_DEVICE(CQUESTSND, cquestsnd);
DECLARE_LEGACY_CPU_DEVICE(CQUESTROT, cquestrot);
DECLARE_LEGACY_CPU_DEVICE(CQUESTLIN, cquestlin);

#endif /* _CUBEQCPU_H */
