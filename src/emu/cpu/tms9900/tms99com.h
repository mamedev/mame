// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    Common definitions for all TMS processors

    Types of TMS99xx processors:
    TI990/9    Early implementation, used in a few real-world applications, 1974
               very similar to mapper-less 990/10 and tms9900, but the Load
               process is different

    TI990/10   Original multi-chip implementation for minicomputer systems, 1975

    TI990/12   Multi-chip implementation, faster than 990/10. Huge instruction set

    TMS9900    Mono-chip implementation, 1976. Used in the TI-99/4(A) computer.

    TMS9940    Microcontroller with 2kb ROM, 128b RAM, decrementer, CRU bus, 1979

    TMS9980    8-bit variant of tms9900.  Two distinct chips actually : tms9980a
               and tms9981 with an extra clock and simplified power supply

    TMS9985    9940 with 8kb ROM, 256b RAM, and a 8-bit external bus, c. 1978 (never released)

    TMS9989    Improved 9980, used in military hardware.

    SBP68689   Improved 9989, built as an ASIC as 9989 was running scarce

    TMS9995    TMS9985-like, with many improvements (but no ROM). Used in the
               TI-99/8 prototype and the Geneve computer.

    TMS99000   Improved mono-chip implementation, meant to replace 990/10, 1981
    TMS99105   This chip is available in several variants which are similar
    TMS99110   but emulate additional instructions, thanks to the so-called
               macrostore feature.

    In this implementation we only consider TMS9900, 9980, and 9995. The
    remaining types are implemented on an own code base as they introduce
    significant changes (e.g. privileged mode, address mapper).
*/

#ifndef __TMS99COMMON_H__
#define __TMS99COMMON_H__

#define MCFG_TMS99xx_ADD(_tag, _device, _clock, _prgmap, _iomap )       \
	MCFG_DEVICE_ADD(_tag, _device, _clock)      \
	MCFG_DEVICE_PROGRAM_MAP(_prgmap)            \
	MCFG_DEVICE_IO_MAP(_iomap)

#define MCFG_TMS99xx_EXTOP_HANDLER( _extop) \
	devcb = &tms99xx_device::static_set_extop_callback( *device, DEVCB_##_extop );

#define MCFG_TMS99xx_INTLEVEL_HANDLER( _intlevel ) \
	devcb = &tms99xx_device::static_set_intlevel_callback( *device, DEVCB_##_intlevel );

#define MCFG_TMS99xx_IAQ_HANDLER( _iaq )    \
	devcb = &tms99xx_device::static_set_iaq_callback( *device, DEVCB_##_iaq );

#define MCFG_TMS99xx_CLKOUT_HANDLER( _clkout ) \
	devcb = &tms99xx_device::static_set_clkout_callback( *device, DEVCB_##_clkout );

#define MCFG_TMS99xx_WAIT_HANDLER( _wait ) \
	devcb = &tms99xx_device::static_set_wait_callback( *device, DEVCB_##_wait );

#define MCFG_TMS99xx_HOLDA_HANDLER( _holda ) \
	devcb = &tms99xx_device::static_set_holda_callback( *device, DEVCB_##_holda );

#define MCFG_TMS99xx_DBIN_HANDLER( _dbin ) \
	devcb = &tms99xx_device::static_set_dbin_callback( *device, DEVCB_##_dbin );

enum
{
	TI990_10_ID = 1,
	TMS9900_ID = 3,
	TMS9940_ID = 4,
	TMS9980_ID = 5,
	TMS9985_ID = 6,
	TMS9989_ID = 7,
	TMS9995_ID = 9,
	TMS99000_ID = 10,
	TMS99105A_ID = 11,
	TMS99110A_ID = 12
};

enum
{
	IDLE_OP = 2,
	RSET_OP = 3,
	CKOF_OP = 5,
	CKON_OP = 6,
	LREX_OP = 7
};

#endif /* __TMS99COMMON_H__ */
