/*
  tms9900.h

  C Header file for TMS9900 core
*/

#pragma once

#ifndef __TMS9900_H__
#define __TMS9900_H__


/*#define TI990_9_ID    0*//* early implementation, used in a few real-world applications, 1974 */
                          /* very similar to mapper-less 990/10 and tms9900, but the Load process */
                          /* is different */
                          /* ("ti990/9" is likely to be a nickname) */
#define TI990_10_ID		1 /* original multi-chip implementation for minicomputer systems, 1975 */
/*#define TI990_12_ID       2*//* multi-chip implementation, faster than 990/10. huge instruction set */
                          /* (144 instructions, with up to 16 additional custom instructions simulteanously) */
                          /* 1979 (or before) */
#define TMS9900_ID      3 /* mono-chip implementation, 1976 */
#define TMS9940_ID      4 /* microcontroller with 2kb ROM, 128b RAM, decrementer, CRU bus, 1979 */
#define TMS9980_ID      5 /* 8-bit variant of tms9900.  Two distinct chips actually : tms9980a, */
                          /* and tms9981 with an extra clock and simplified power supply */
#define TMS9985_ID      6 /* 9940 with 8kb ROM, 256b RAM, and a 8-bit external bus, c. 1978 (never released) */
#define TMS9989_ID      7 /* improved 9980, used in bombs, missiles, and other *nice* hardware */
/*#define SBP68689_ID     8*//* improved 9989, built as an ASIC as 9989 was running scarce */
#define TMS9995_ID      9 /* tms9985-like, with many improvements (but no ROM) */
#define TMS99000_ID     10/* improved mono-chip implementation, meant to replace 990/10, 1981 */
                          /* This chip is available in several variants (tms99105, tms99110...), */
                          /* which are similar but emulate additional instructions thanks */
                          /* to the so-called macrostore feature. */
#define TMS99105A_ID	11
#define TMS99110A_ID	12


/* NPW 25-May-2002 - Added these to get it to compile under windows */
#define TI9940_ID		TMS9940_ID
#define TI9985_ID		TMS9985_ID


enum
{
	TMS9900_PC=1, TMS9900_WP, TMS9900_STATUS, TMS9900_IR,
	TMS9900_R0, TMS9900_R1, TMS9900_R2, TMS9900_R3,
	TMS9900_R4, TMS9900_R5, TMS9900_R6, TMS9900_R7,
	TMS9900_R8, TMS9900_R9, TMS9900_R10, TMS9900_R11,
	TMS9900_R12, TMS9900_R13, TMS9900_R14, TMS9900_R15
};

typedef void (*ti99xx_idle_func)(device_t *device, int state);
typedef void (*ti99xx_rset_func)(device_t *device);
typedef void (*ti99xx_lrex_func)(device_t *device);
typedef void (*ti99xx_ckon_ckof_func)(device_t *device, int state);
typedef void (*ti99xx_error_interrupt_func)(device_t *device, int state);


DECLARE_LEGACY_CPU_DEVICE(TI990_10L, ti990_10l);

/*
    structure with the parameters ti990_10_reset wants.
*/

typedef struct ti990_10reset_param
{
	ti99xx_idle_func	idle_callback;
	ti99xx_rset_func	rset_callback;
	ti99xx_lrex_func	lrex_callback;
	ti99xx_ckon_ckof_func	ckon_ckof_callback;
	ti99xx_error_interrupt_func	error_interrupt_callback;
} ti990_10reset_param;

/* accessor for the internal ROM */
extern READ16_HANDLER(ti990_10_internal_r);

/* CRU accessor for the mapper registers (R12 base 0x1fa0) */
extern READ8_HANDLER(ti990_10_mapper_cru_r);
extern WRITE8_HANDLER(ti990_10_mapper_cru_w);
/* CRU accessor for the error interrupt register (R12 base 0x1fc0) */
extern READ8_HANDLER(ti990_10_eir_cru_r);
extern WRITE8_HANDLER(ti990_10_eir_cru_w);



DECLARE_LEGACY_CPU_DEVICE(TMS9900L, tms9900l);

/*
    structure with optional parameters for tms9900_reset.
*/
typedef struct tms9900reset_param
{
	ti99xx_idle_func	idle_callback;
} tms9900reset_param;



//DECLARE_LEGACY_CPU_DEVICE(TMS9940, tms9940);

/*
    structure with optional parameters for tms9940_reset.
*/
typedef struct tms9940reset_param
{
	ti99xx_idle_func	idle_callback;
} tms9940reset_param;



DECLARE_LEGACY_CPU_DEVICE(TMS9980L, tms9980al);

/*
    structure with optional parameters for tms9980a_reset.
*/
typedef struct tms9980areset_param
{
	ti99xx_idle_func	idle_callback;
} tms9980areset_param;



//DECLARE_LEGACY_CPU_DEVICE(TMS9985, tms9985);

/*//
    structure with optional parameters for tms9985_reset.
*/
typedef struct tms9985reset_param
{
	ti99xx_idle_func	idle_callback;
} tms9985reset_param;



//DECLARE_LEGACY_CPU_DEVICE(TMS9989, tms9989);

/*
    structure with optional parameters for tms9989_reset.
*/
typedef struct tms9989reset_param
{
	ti99xx_idle_func	idle_callback;
} tms9989reset_param;



DECLARE_LEGACY_CPU_DEVICE(TMS9995L, tms9995l);

/*
  structure with the parameters tms9995_reset wants.
*/
typedef struct tms9995reset_param
{
	/* auto_wait_state : a non-zero value makes tms9995 generate a wait state automatically on each
       memory access */
	int auto_wait_state;

	ti99xx_idle_func	idle_callback;

	/* on the tms9995-mp9537, internal RAM and decrementer register are
        disabled.  This chip is used by the ti99/8 so that internal RAM does
        not prevent the mapper from working correctly. */
	int is_mp9537;
} tms9995reset_param;

/* accessor for the first 252 bytes of internal RAM */
extern READ8_HANDLER(tms9995_internal1_r);
extern WRITE8_HANDLER(tms9995_internal1_w);
/* accessors for the last 4 bytes of internal RAM */
extern READ8_HANDLER(tms9995_internal2_r);
extern WRITE8_HANDLER(tms9995_internal2_w);



//DECLARE_LEGACY_CPU_DEVICE(TMS99000, tms99000);

/*
    structure with optional parameters for tms99000_reset.
*/
typedef struct tms99000reset_param
{
	ti99xx_idle_func	idle_callback;
} tms99000reset_param;



//DECLARE_LEGACY_CPU_DEVICE(TMS99105A, tms99105a);

/*
    structure with optional parameters for tms99105a_reset.
*/
typedef struct tms99105areset_param
{
	ti99xx_idle_func	idle_callback;
} tms99105areset_param;



//DECLARE_LEGACY_CPU_DEVICE(TMS99110A, tms99110a);

/*
    structure with optional parameters for tms99110a_reset.
*/
typedef struct tms99110areset_param
{
	ti99xx_idle_func	idle_callback;
} tms99110areset_param;


unsigned Dasm9900 (char *buffer, unsigned pc, int model_id, const UINT8 *oprom, const UINT8 *opram);

#endif /* __TMS9900_H__ */
