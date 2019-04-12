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

/*
   We are using setaddress this way: The address is the object to be
   written; as address we use the values of the DBIN and IAQ lines. That
   means we do not need separate dbin and iaq setting methods.
*/
enum
{
	TMS99xx_BUS_WRITE = 0,
	TMS99xx_BUS_DBIN = 1,
	TMS99xx_BUS_IAQ = 2
};
#endif /* __TMS99COMMON_H__ */
