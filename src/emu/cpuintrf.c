/***************************************************************************

    cpuintrf.c

    Core CPU interface functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"

/* mingw-gcc defines this */
#undef i386


/*************************************
 *
 *  Include headers from all CPUs
 *
 *************************************/

CPU_GET_INFO( dummy );
CPU_GET_INFO( z80 );
CPU_GET_INFO( z180 );
CPU_GET_INFO( i8080 );
CPU_GET_INFO( i8085 );
CPU_GET_INFO( m6502 );
CPU_GET_INFO( m65c02 );
CPU_GET_INFO( m65sc02 );
CPU_GET_INFO( m65ce02 );
CPU_GET_INFO( m6509 );
CPU_GET_INFO( m6510 );
CPU_GET_INFO( m6510t );
CPU_GET_INFO( m7501 );
CPU_GET_INFO( m8502 );
CPU_GET_INFO( n2a03 );
CPU_GET_INFO( deco16 );
CPU_GET_INFO( m4510 );
CPU_GET_INFO( h6280 );
CPU_GET_INFO( i8086 );
CPU_GET_INFO( i8088 );
CPU_GET_INFO( i80186 );
CPU_GET_INFO( i80188 );
CPU_GET_INFO( i80286 );
CPU_GET_INFO( v20 );
CPU_GET_INFO( v25 );
CPU_GET_INFO( v30 );
CPU_GET_INFO( v33 );
CPU_GET_INFO( v35 );
CPU_GET_INFO( v60 );
CPU_GET_INFO( v70 );
CPU_GET_INFO( i8035 );
CPU_GET_INFO( i8048 );
CPU_GET_INFO( i8648 );
CPU_GET_INFO( i8748 );
CPU_GET_INFO( mb8884 );
CPU_GET_INFO( i8039 );
CPU_GET_INFO( i8049 );
CPU_GET_INFO( i8749 );
CPU_GET_INFO( n7751 );
CPU_GET_INFO( m58715 );
CPU_GET_INFO( i8041 );
CPU_GET_INFO( i8741 );
CPU_GET_INFO( i8042 );
CPU_GET_INFO( i8242 );
CPU_GET_INFO( i8742 );
CPU_GET_INFO( i8031 );
CPU_GET_INFO( i8032 );
CPU_GET_INFO( i8051 );
CPU_GET_INFO( i8052 );
CPU_GET_INFO( i8751 );
CPU_GET_INFO( i8752 );
CPU_GET_INFO( i80c31 );
CPU_GET_INFO( i80c32 );
CPU_GET_INFO( i80c51 );
CPU_GET_INFO( i80c52 );
CPU_GET_INFO( i87c51 );
CPU_GET_INFO( i87c52 );
CPU_GET_INFO( at89c4051 );
CPU_GET_INFO( ds5002fp );
CPU_GET_INFO( m6800 );
CPU_GET_INFO( m6801 );
CPU_GET_INFO( m6802 );
CPU_GET_INFO( m6803 );
CPU_GET_INFO( m6808 );
CPU_GET_INFO( hd63701 );
CPU_GET_INFO( nsc8105 );
CPU_GET_INFO( m6805 );
CPU_GET_INFO( m68705 );
CPU_GET_INFO( hd63705 );
CPU_GET_INFO( hd6309 );
CPU_GET_INFO( m6809 );
CPU_GET_INFO( m6809e );
CPU_GET_INFO( konami );
CPU_GET_INFO( m68000 );
CPU_GET_INFO( m68008 );
CPU_GET_INFO( m68010 );
CPU_GET_INFO( m68ec020 );
CPU_GET_INFO( m68020 );
CPU_GET_INFO( m68040 );
CPU_GET_INFO( t11 );
CPU_GET_INFO( s2650 );
CPU_GET_INFO( tms34010 );
CPU_GET_INFO( tms34020 );
CPU_GET_INFO( ti990_10 );
CPU_GET_INFO( tms9900 );
CPU_GET_INFO( tms9940 );
CPU_GET_INFO( tms9980a );
CPU_GET_INFO( tms9985 );
CPU_GET_INFO( tms9989 );
CPU_GET_INFO( tms9995 );
CPU_GET_INFO( tms99105a );
CPU_GET_INFO( tms99110a );
CPU_GET_INFO( tms99000 );
CPU_GET_INFO( z8000 );
CPU_GET_INFO( tms32010 );
CPU_GET_INFO( tms32025 );
CPU_GET_INFO( tms32026 );
CPU_GET_INFO( tms32031 );
CPU_GET_INFO( tms32032 );
CPU_GET_INFO( tms32051 );
CPU_GET_INFO( ccpu );
CPU_GET_INFO( adsp2100 );
CPU_GET_INFO( adsp2101 );
CPU_GET_INFO( adsp2104 );
CPU_GET_INFO( adsp2105 );
CPU_GET_INFO( adsp2115 );
CPU_GET_INFO( adsp2181 );
CPU_GET_INFO( psxcpu );
CPU_GET_INFO( asap );
CPU_GET_INFO( upd7810 );
CPU_GET_INFO( upd7807 );
CPU_GET_INFO( upd7801 );
CPU_GET_INFO( upd78c05 );
CPU_GET_INFO( upd78c06 );
CPU_GET_INFO( jaguargpu );
CPU_GET_INFO( jaguardsp );
CPU_GET_INFO( cquestsnd );
CPU_GET_INFO( cquestrot );
CPU_GET_INFO( cquestlin );
CPU_GET_INFO( r3000be );
CPU_GET_INFO( r3000le );
CPU_GET_INFO( r3041be );
CPU_GET_INFO( r3041le );
CPU_GET_INFO( r4600be );
CPU_GET_INFO( r4600le );
CPU_GET_INFO( r4650be );
CPU_GET_INFO( r4650le );
CPU_GET_INFO( r4700be );
CPU_GET_INFO( r4700le );
CPU_GET_INFO( r5000be );
CPU_GET_INFO( r5000le );
CPU_GET_INFO( qed5271be );
CPU_GET_INFO( qed5271le );
CPU_GET_INFO( rm7000be );
CPU_GET_INFO( rm7000le );
CPU_GET_INFO( arm );
CPU_GET_INFO( arm7 );
CPU_GET_INFO( sh1 );
CPU_GET_INFO( sh2 );
CPU_GET_INFO( sh4 );
CPU_GET_INFO( dsp32c );
CPU_GET_INFO( pic16C54 );
CPU_GET_INFO( pic16C55 );
CPU_GET_INFO( pic16C56 );
CPU_GET_INFO( pic16C57 );
CPU_GET_INFO( pic16C58 );
CPU_GET_INFO( g65816 );
CPU_GET_INFO( spc700 );
CPU_GET_INFO( e116t );
CPU_GET_INFO( e116xt );
CPU_GET_INFO( e116xs );
CPU_GET_INFO( e116xsr );
CPU_GET_INFO( e132n );
CPU_GET_INFO( e132t );
CPU_GET_INFO( e132xn );
CPU_GET_INFO( e132xt );
CPU_GET_INFO( e132xs );
CPU_GET_INFO( e132xsr );
CPU_GET_INFO( gms30c2116 );
CPU_GET_INFO( gms30c2132 );
CPU_GET_INFO( gms30c2216 );
CPU_GET_INFO( gms30c2232 );
CPU_GET_INFO( i386 );
CPU_GET_INFO( i486 );
CPU_GET_INFO( pentium );
CPU_GET_INFO( mediagx );
CPU_GET_INFO( i960 );
CPU_GET_INFO( h8_3002 );
CPU_GET_INFO( h8_3007 );
CPU_GET_INFO( h8_3044 );
CPU_GET_INFO( v810 );
CPU_GET_INFO( m37702 );
CPU_GET_INFO( m37710 );
CPU_GET_INFO( ppc403ga );
CPU_GET_INFO( ppc403gcx );
CPU_GET_INFO( ppc601 );
CPU_GET_INFO( ppc602 );
CPU_GET_INFO( ppc603 );
CPU_GET_INFO( ppc603e );
CPU_GET_INFO( ppc603r );
CPU_GET_INFO( ppc604 );
CPU_GET_INFO( mpc8240 );
CPU_GET_INFO( SE3208 );
CPU_GET_INFO( mc68hc11 );
CPU_GET_INFO( adsp21062 );
CPU_GET_INFO( dsp56k );
CPU_GET_INFO( rsp );
CPU_GET_INFO( alpha8201 );
CPU_GET_INFO( alpha8301 );
CPU_GET_INFO( cdp1802 );
CPU_GET_INFO( cop420 );
CPU_GET_INFO( cop421 );
CPU_GET_INFO( cop410 );
CPU_GET_INFO( cop411 );
CPU_GET_INFO( tmp90840 );
CPU_GET_INFO( tmp90841 );
CPU_GET_INFO( tmp91640 );
CPU_GET_INFO( tmp91641 );
CPU_GET_INFO( apexc );
CPU_GET_INFO( cp1610 );
CPU_GET_INFO( f8 );
CPU_GET_INFO( lh5801 );
CPU_GET_INFO( pdp1 );
CPU_GET_INFO( saturn );
CPU_GET_INFO( sc61860 );
CPU_GET_INFO( tx0_64kw );
CPU_GET_INFO( tx0_8kw );
CPU_GET_INFO( lr35902 );
CPU_GET_INFO( tms7000 );
CPU_GET_INFO( tms7000_exl );
CPU_GET_INFO( sm8500 );
CPU_GET_INFO( v30mz );
CPU_GET_INFO( mb8841 );
CPU_GET_INFO( mb8842 );
CPU_GET_INFO( mb8843 );
CPU_GET_INFO( mb8844 );
CPU_GET_INFO( mb86233 );
CPU_GET_INFO( ssp1601 );
CPU_GET_INFO( minx );
CPU_GET_INFO( cxd8661r );



/*************************************
 *
 *  Debug logging
 *
 *************************************/

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)



/*************************************
 *
 *  Macros to help verify active CPU
 *
 *************************************/

#define VERIFY_CPU(name) \
	assert_always(cpu != NULL, #name "() called with invalid cpu!")

#define VERIFY_ACTIVECPU(name) \
	assert_always(activecpunum >= 0, #name "() called with no active cpu!")

#define VERIFY_CPUTYPE(name) \
	assert_always(cputype >= 0 && cputype < CPU_COUNT, #name "() called for invalid cpu type!")



/*************************************
 *
 *  Internal CPU info type
 *
 *************************************/

typedef struct _cpuintrf_data cpuintrf_data;
struct _cpuintrf_data
{
	int index;						/* index of this CPU */
	int family; 					/* family index of this CPU */
	cpu_interface intf;		 		/* copy of the interface data */
	cpu_type cputype; 					/* type index of this CPU */
	device_config *device;			/* dummy device for now */
	int *icount;
	cpu_disassemble_func dasm_override;
};



/*************************************
 *
 *  The core list of CPU interfaces
 *
 *************************************/

cpu_interface cpuintrf[CPU_COUNT];

static const struct
{
	int		cputype;
	void	(*get_info)(UINT32 state, cpuinfo *info);
} cpuintrf_map[] =
{
	{ CPU_DUMMY, CPU_GET_INFO_NAME(dummy) },
#if (HAS_Z80)
	{ CPU_Z80, CPU_GET_INFO_NAME(z80) },
#endif
#if (HAS_Z180)
	{ CPU_Z180, CPU_GET_INFO_NAME(z180) },
#endif
#if (HAS_8080)
	{ CPU_8080, CPU_GET_INFO_NAME(i8080) },
#endif
#if (HAS_8085A)
	{ CPU_8085A, CPU_GET_INFO_NAME(i8085) },
#endif
#if (HAS_M6502)
	{ CPU_M6502, CPU_GET_INFO_NAME(m6502) },
#endif
#if (HAS_M65C02)
	{ CPU_M65C02, CPU_GET_INFO_NAME(m65c02) },
#endif
#if (HAS_M65SC02)
	{ CPU_M65SC02, CPU_GET_INFO_NAME(m65sc02) },
#endif
#if (HAS_M65CE02)
	{ CPU_M65CE02, CPU_GET_INFO_NAME(m65ce02) },
#endif
#if (HAS_M6509)
	{ CPU_M6509, CPU_GET_INFO_NAME(m6509) },
#endif
#if (HAS_M6510)
	{ CPU_M6510, CPU_GET_INFO_NAME(m6510) },
#endif
#if (HAS_M6510T)
	{ CPU_M6510T, CPU_GET_INFO_NAME(m6510t) },
#endif
#if (HAS_M7501)
	{ CPU_M7501, CPU_GET_INFO_NAME(m7501) },
#endif
#if (HAS_M8502)
	{ CPU_M8502, CPU_GET_INFO_NAME(m8502) },
#endif
#if (HAS_N2A03)
	{ CPU_N2A03, CPU_GET_INFO_NAME(n2a03) },
#endif
#if (HAS_DECO16)
	{ CPU_DECO16, CPU_GET_INFO_NAME(deco16) },
#endif
#if (HAS_M4510)
	{ CPU_M4510, CPU_GET_INFO_NAME(m4510) },
#endif
#if (HAS_H6280)
	{ CPU_H6280, CPU_GET_INFO_NAME(h6280) },
#endif
#if (HAS_I8086)
	{ CPU_I8086, CPU_GET_INFO_NAME(i8086) },
#endif
#if (HAS_I8088)
	{ CPU_I8088, CPU_GET_INFO_NAME(i8088) },
#endif
#if (HAS_I80186)
	{ CPU_I80186, CPU_GET_INFO_NAME(i80186) },
#endif
#if (HAS_I80188)
	{ CPU_I80188, CPU_GET_INFO_NAME(i80188) },
#endif
#if (HAS_I80286)
	{ CPU_I80286, CPU_GET_INFO_NAME(i80286) },
#endif
#if (HAS_V20)
	{ CPU_V20, CPU_GET_INFO_NAME(v20) },
#endif
#if (HAS_V25)
	{ CPU_V25, CPU_GET_INFO_NAME(v25) },
#endif
#if (HAS_V30)
	{ CPU_V30, CPU_GET_INFO_NAME(v30) },
#endif
#if (HAS_V33)
	{ CPU_V33, CPU_GET_INFO_NAME(v33) },
#endif
#if (HAS_V35)
	{ CPU_V35, CPU_GET_INFO_NAME(v35) },
#endif
#if (HAS_V60)
	{ CPU_V60, CPU_GET_INFO_NAME(v60) },
#endif
#if (HAS_V70)
	{ CPU_V70, CPU_GET_INFO_NAME(v70) },
#endif
#if (HAS_I8035)
	{ CPU_I8035, CPU_GET_INFO_NAME(i8035) },
#endif
#if (HAS_I8041)
	{ CPU_I8041, CPU_GET_INFO_NAME(i8041) },
#endif
#if (HAS_I8048)
	{ CPU_I8048, CPU_GET_INFO_NAME(i8048) },
#endif
#if (HAS_I8648)
	{ CPU_I8648, CPU_GET_INFO_NAME(i8648) },
#endif
#if (HAS_I8748)
	{ CPU_I8748, CPU_GET_INFO_NAME(i8748) },
#endif
#if (HAS_MB8884)
	{ CPU_MB8884, CPU_GET_INFO_NAME(mb8884) },
#endif
#if (HAS_N7751)
	{ CPU_N7751, CPU_GET_INFO_NAME(n7751) },
#endif
#if (HAS_I8039)
	{ CPU_I8039, CPU_GET_INFO_NAME(i8039) },
#endif
#if (HAS_I8049)
	{ CPU_I8049, CPU_GET_INFO_NAME(i8049) },
#endif
#if (HAS_I8749)
	{ CPU_I8749, CPU_GET_INFO_NAME(i8749) },
#endif
#if (HAS_M58715)
	{ CPU_M58715, CPU_GET_INFO_NAME(m58715) },
#endif
#if (HAS_I8041)
	{ CPU_I8041, CPU_GET_INFO_NAME(i8041) },
#endif
#if (HAS_I8741)
	{ CPU_I8741, CPU_GET_INFO_NAME(i8741) },
#endif
#if (HAS_I8042)
	{ CPU_I8042, CPU_GET_INFO_NAME(i8042) },
#endif
#if (HAS_I8242)
	{ CPU_I8242, CPU_GET_INFO_NAME(i8242) },
#endif
#if (HAS_I8742)
	{ CPU_I8742, CPU_GET_INFO_NAME(i8742) },
#endif
#if (HAS_I8031)
	{ CPU_I8031, CPU_GET_INFO_NAME(i8031) },
#endif
#if (HAS_I8032)
	{ CPU_I8032, CPU_GET_INFO_NAME(i8032) },
#endif
#if (HAS_I8051)
	{ CPU_I8051, CPU_GET_INFO_NAME(i8051) },
#endif
#if (HAS_I8052)
	{ CPU_I8052, CPU_GET_INFO_NAME(i8052) },
#endif
#if (HAS_I8751)
	{ CPU_I8751, CPU_GET_INFO_NAME(i8751) },
#endif
#if (HAS_I8752)
	{ CPU_I8752, CPU_GET_INFO_NAME(i8752) },
#endif
#if (HAS_I80C31)
	{ CPU_I80C31, CPU_GET_INFO_NAME(i80c31) },
#endif
#if (HAS_I80C32)
	{ CPU_I80C32, CPU_GET_INFO_NAME(i80c32) },
#endif
#if (HAS_I80C51)
	{ CPU_I80C51, CPU_GET_INFO_NAME(i80c51) },
#endif
#if (HAS_I80C52)
	{ CPU_I80C52, CPU_GET_INFO_NAME(i80c52) },
#endif
#if (HAS_I87C51)
	{ CPU_I87C51, CPU_GET_INFO_NAME(i87c51) },
#endif
#if (HAS_I87C52)
	{ CPU_I87C52, CPU_GET_INFO_NAME(i87c52) },
#endif
#if (HAS_AT89C4051)
	{ CPU_AT89C4051, CPU_GET_INFO_NAME(at89c4051) },
#endif
#if (HAS_DS5002FP)
	{ CPU_DS5002FP, CPU_GET_INFO_NAME(ds5002fp) },
#endif
#if (HAS_M6800)
	{ CPU_M6800, CPU_GET_INFO_NAME(m6800) },
#endif
#if (HAS_M6801)
	{ CPU_M6801, CPU_GET_INFO_NAME(m6801) },
#endif
#if (HAS_M6802)
	{ CPU_M6802, CPU_GET_INFO_NAME(m6802) },
#endif
#if (HAS_M6803)
	{ CPU_M6803, CPU_GET_INFO_NAME(m6803) },
#endif
#if (HAS_M6808)
	{ CPU_M6808, CPU_GET_INFO_NAME(m6808) },
#endif
#if (HAS_HD63701)
	{ CPU_HD63701, CPU_GET_INFO_NAME(hd63701) },
#endif
#if (HAS_NSC8105)
	{ CPU_NSC8105, CPU_GET_INFO_NAME(nsc8105) },
#endif
#if (HAS_M6805)
	{ CPU_M6805, CPU_GET_INFO_NAME(m6805) },
#endif
#if (HAS_M68705)
	{ CPU_M68705, CPU_GET_INFO_NAME(m68705) },
#endif
#if (HAS_HD63705)
	{ CPU_HD63705, CPU_GET_INFO_NAME(hd63705) },
#endif
#if (HAS_HD6309)
	{ CPU_HD6309, CPU_GET_INFO_NAME(hd6309) },
#endif
#if (HAS_M6809)
	{ CPU_M6809, CPU_GET_INFO_NAME(m6809) },
#endif
#if (HAS_M6809E)
	{ CPU_M6809E, CPU_GET_INFO_NAME(m6809e) },
#endif
#if (HAS_KONAMI)
	{ CPU_KONAMI, CPU_GET_INFO_NAME(konami) },
#endif
#if (HAS_M68000)
	{ CPU_M68000, CPU_GET_INFO_NAME(m68000) },
#endif
#if (HAS_M68008)
	{ CPU_M68008, CPU_GET_INFO_NAME(m68008) },
#endif
#if (HAS_M68010)
	{ CPU_M68010, CPU_GET_INFO_NAME(m68010) },
#endif
#if (HAS_M68EC020)
	{ CPU_M68EC020, CPU_GET_INFO_NAME(m68ec020) },
#endif
#if (HAS_M68020)
	{ CPU_M68020, CPU_GET_INFO_NAME(m68020) },
#endif
#if (HAS_M68040)
	{ CPU_M68040, CPU_GET_INFO_NAME(m68040) },
#endif
#if (HAS_T11)
	{ CPU_T11, CPU_GET_INFO_NAME(t11) },
#endif
#if (HAS_S2650)
	{ CPU_S2650, CPU_GET_INFO_NAME(s2650) },
#endif
#if (HAS_TMS34010)
	{ CPU_TMS34010, CPU_GET_INFO_NAME(tms34010) },
#endif
#if (HAS_TMS34020)
	{ CPU_TMS34020, CPU_GET_INFO_NAME(tms34020) },
#endif
#if (HAS_TI990_10)
	{ CPU_TI990_10, ti990_CPU_GET_INFO_NAME(10) },
#endif
#if (HAS_TMS9900)
	{ CPU_TMS9900, CPU_GET_INFO_NAME(tms9900) },
#endif
#if (HAS_TMS9940)
	{ CPU_TMS9940, CPU_GET_INFO_NAME(tms9940) },
#endif
#if (HAS_TMS9980)
	{ CPU_TMS9980, CPU_GET_INFO_NAME(tms9980a) },
#endif
#if (HAS_TMS9985)
	{ CPU_TMS9985, CPU_GET_INFO_NAME(tms9985) },
#endif
#if (HAS_TMS9989)
	{ CPU_TMS9989, CPU_GET_INFO_NAME(tms9989) },
#endif
#if (HAS_TMS9995)
	{ CPU_TMS9995, CPU_GET_INFO_NAME(tms9995) },
#endif
#if (HAS_TMS99105A)
	{ CPU_TMS99105A, CPU_GET_INFO_NAME(tms99105a) },
#endif
#if (HAS_TMS99110A)
	{ CPU_TMS99110A, CPU_GET_INFO_NAME(tms99110a) },
#endif
#if (HAS_TMS99000)
	{ CPU_TMS99000, CPU_GET_INFO_NAME(tms99000) },
#endif
#if (HAS_Z8000)
	{ CPU_Z8000, CPU_GET_INFO_NAME(z8000) },
#endif
#if (HAS_TMS32010)
	{ CPU_TMS32010, CPU_GET_INFO_NAME(tms32010) },
#endif
#if (HAS_TMS32025)
	{ CPU_TMS32025, CPU_GET_INFO_NAME(tms32025) },
#endif
#if (HAS_TMS32026)
	{ CPU_TMS32026, CPU_GET_INFO_NAME(tms32026) },
#endif
#if (HAS_TMS32031)
	{ CPU_TMS32031, CPU_GET_INFO_NAME(tms32031) },
#endif
#if (HAS_TMS32032)
	{ CPU_TMS32032, CPU_GET_INFO_NAME(tms32032) },
#endif
#if (HAS_TMS32051)
	{ CPU_TMS32051, CPU_GET_INFO_NAME(tms32051) },
#endif
#if (HAS_CCPU)
	{ CPU_CCPU, CPU_GET_INFO_NAME(ccpu) },
#endif
#if (HAS_ADSP2100)
	{ CPU_ADSP2100, CPU_GET_INFO_NAME(adsp2100) },
#endif
#if (HAS_ADSP2101)
	{ CPU_ADSP2101, CPU_GET_INFO_NAME(adsp2101) },
#endif
#if (HAS_ADSP2104)
	{ CPU_ADSP2104, CPU_GET_INFO_NAME(adsp2104) },
#endif
#if (HAS_ADSP2105)
	{ CPU_ADSP2105, CPU_GET_INFO_NAME(adsp2105) },
#endif
#if (HAS_ADSP2115)
	{ CPU_ADSP2115, CPU_GET_INFO_NAME(adsp2115) },
#endif
#if (HAS_ADSP2181)
	{ CPU_ADSP2181, CPU_GET_INFO_NAME(adsp2181) },
#endif
#if (HAS_PSXCPU)
	{ CPU_PSXCPU, CPU_GET_INFO_NAME(psxcpu) },
#endif
#if (HAS_ASAP)
	{ CPU_ASAP, CPU_GET_INFO_NAME(asap) },
#endif
#if (HAS_UPD7810)
	{ CPU_UPD7810, CPU_GET_INFO_NAME(upd7810) },
#endif
#if (HAS_UPD7807)
	{ CPU_UPD7807, CPU_GET_INFO_NAME(upd7807) },
#endif
#if (HAS_UPD7801)
	{ CPU_UPD7801, CPU_GET_INFO_NAME(upd7801) },
	{ CPU_UPD78C05, CPU_GET_INFO_NAME(upd78c05) },
	{ CPU_UPD78C06, CPU_GET_INFO_NAME(upd78c06) },
#endif
#if (HAS_JAGUAR)
	{ CPU_JAGUARGPU, CPU_GET_INFO_NAME(jaguargpu) },
	{ CPU_JAGUARDSP, CPU_GET_INFO_NAME(jaguardsp) },
#endif
#if (HAS_CUBEQCPU)
	{ CPU_CQUESTSND, CPU_GET_INFO_NAME(cquestsnd) },
	{ CPU_CQUESTROT, CPU_GET_INFO_NAME(cquestrot) },
	{ CPU_CQUESTLIN, CPU_GET_INFO_NAME(cquestlin) },
#endif
#if (HAS_R3000)
	{ CPU_R3000BE, CPU_GET_INFO_NAME(r3000be) },
	{ CPU_R3000LE, CPU_GET_INFO_NAME(r3000le) },
#endif
#if (HAS_R3041)
	{ CPU_R3041BE, CPU_GET_INFO_NAME(r3041be) },
	{ CPU_R3041LE, CPU_GET_INFO_NAME(r3041le) },
#endif
#if (HAS_R4600)
	{ CPU_R4600BE, CPU_GET_INFO_NAME(r4600be) },
	{ CPU_R4600LE, CPU_GET_INFO_NAME(r4600le) },
#endif
#if (HAS_R4650)
	{ CPU_R4650BE, CPU_GET_INFO_NAME(r4650be) },
	{ CPU_R4650LE, CPU_GET_INFO_NAME(r4650le) },
#endif
#if (HAS_R4700)
	{ CPU_R4700BE, CPU_GET_INFO_NAME(r4700be) },
	{ CPU_R4700LE, CPU_GET_INFO_NAME(r4700le) },
#endif
#if (HAS_R5000)
	{ CPU_R5000BE, CPU_GET_INFO_NAME(r5000be) },
	{ CPU_R5000LE, CPU_GET_INFO_NAME(r5000le) },
#endif
#if (HAS_QED5271)
	{ CPU_QED5271BE, CPU_GET_INFO_NAME(qed5271be) },
	{ CPU_QED5271LE, CPU_GET_INFO_NAME(qed5271le) },
#endif
#if (HAS_RM7000)
	{ CPU_RM7000BE, CPU_GET_INFO_NAME(rm7000be) },
	{ CPU_RM7000LE, CPU_GET_INFO_NAME(rm7000le) },
#endif
#if (HAS_ARM)
	{ CPU_ARM, CPU_GET_INFO_NAME(arm) },
#endif
#if (HAS_ARM7)
	{ CPU_ARM7, CPU_GET_INFO_NAME(arm7) },
#endif
#if (HAS_SH1)
	{ CPU_SH1, CPU_GET_INFO_NAME(sh1) },
#endif
#if (HAS_SH2)
	{ CPU_SH2, CPU_GET_INFO_NAME(sh2) },
#endif
#if (HAS_SH4)
	{ CPU_SH4, CPU_GET_INFO_NAME(sh4) },
#endif
#if (HAS_DSP32C)
	{ CPU_DSP32C, CPU_GET_INFO_NAME(dsp32c) },
#endif
#if (HAS_PIC16C54)
	{ CPU_PIC16C54, CPU_GET_INFO_NAME(pic16C54) },
#endif
#if (HAS_PIC16C55)
	{ CPU_PIC16C55, CPU_GET_INFO_NAME(pic16C55) },
#endif
#if (HAS_PIC16C56)
	{ CPU_PIC16C56, CPU_GET_INFO_NAME(pic16C56) },
#endif
#if (HAS_PIC16C57)
	{ CPU_PIC16C57, CPU_GET_INFO_NAME(pic16C57) },
#endif
#if (HAS_PIC16C58)
	{ CPU_PIC16C58, CPU_GET_INFO_NAME(pic16C58) },
#endif
#if (HAS_G65816)
	{ CPU_G65816, CPU_GET_INFO_NAME(g65816) },
#endif
#if (HAS_SPC700)
	{ CPU_SPC700, CPU_GET_INFO_NAME(spc700) },
#endif
#if (HAS_E116T)
	{ CPU_E116T, CPU_GET_INFO_NAME(e116t) },
#endif
#if (HAS_E116XT)
	{ CPU_E116XT, CPU_GET_INFO_NAME(e116xt) },
#endif
#if (HAS_E116XS)
	{ CPU_E116XS, CPU_GET_INFO_NAME(e116xs) },
#endif
#if (HAS_E116XSR)
	{ CPU_E116XSR, CPU_GET_INFO_NAME(e116xsr) },
#endif
#if (HAS_E132N)
	{ CPU_E132N, CPU_GET_INFO_NAME(e132n) },
#endif
#if (HAS_E132T)
	{ CPU_E132T, CPU_GET_INFO_NAME(e132t) },
#endif
#if (HAS_E132XN)
	{ CPU_E132XN, CPU_GET_INFO_NAME(e132xn) },
#endif
#if (HAS_E132XT)
	{ CPU_E132XT, CPU_GET_INFO_NAME(e132xt) },
#endif
#if (HAS_E132XS)
	{ CPU_E132XS, CPU_GET_INFO_NAME(e132xs) },
#endif
#if (HAS_E132XSR)
	{ CPU_E132XSR, CPU_GET_INFO_NAME(e132xsr) },
#endif
#if (HAS_GMS30C2116)
	{ CPU_GMS30C2116, CPU_GET_INFO_NAME(gms30c2116) },
#endif
#if (HAS_GMS30C2132)
	{ CPU_GMS30C2132, CPU_GET_INFO_NAME(gms30c2132) },
#endif
#if (HAS_GMS30C2216)
	{ CPU_GMS30C2216, CPU_GET_INFO_NAME(gms30c2216) },
#endif
#if (HAS_GMS30C2232)
	{ CPU_GMS30C2232, CPU_GET_INFO_NAME(gms30c2232) },
#endif
#if (HAS_I386)
	{ CPU_I386, CPU_GET_INFO_NAME(i386) },
#endif
#if (HAS_I486)
	{ CPU_I486, CPU_GET_INFO_NAME(i486) },
#endif
#if (HAS_PENTIUM)
	{ CPU_PENTIUM, CPU_GET_INFO_NAME(pentium) },
#endif
#if (HAS_MEDIAGX)
	{ CPU_MEDIAGX, CPU_GET_INFO_NAME(mediagx) },
#endif
#if (HAS_I960)
	{ CPU_I960, CPU_GET_INFO_NAME(i960) },
#endif
#if (HAS_H83002)
	{ CPU_H83002, CPU_GET_INFO_NAME(h8_3002) },
	{ CPU_H83007, CPU_GET_INFO_NAME(h8_3007) },
	{ CPU_H83044, CPU_GET_INFO_NAME(h8_3044) },
#endif
#if (HAS_V810)
	{ CPU_V810, CPU_GET_INFO_NAME(v810) },
#endif
#if (HAS_M37702)
	{ CPU_M37702, CPU_GET_INFO_NAME(m37702) },
#endif
#if (HAS_M37710)
	{ CPU_M37710, CPU_GET_INFO_NAME(m37710) },
#endif
#if (HAS_PPC403GA)
	{ CPU_PPC403GA, CPU_GET_INFO_NAME(ppc403ga) },
#endif
#if (HAS_PPC403GCX)
	{ CPU_PPC403GCX, CPU_GET_INFO_NAME(ppc403gcx) },
#endif
#if (HAS_PPC601)
	{ CPU_PPC601, CPU_GET_INFO_NAME(ppc601) },
#endif
#if (HAS_PPC602)
	{ CPU_PPC602, CPU_GET_INFO_NAME(ppc602) },
#endif
#if (HAS_PPC603)
	{ CPU_PPC603, CPU_GET_INFO_NAME(ppc603) },
#endif
#if (HAS_PPC603E)
	{ CPU_PPC603E, CPU_GET_INFO_NAME(ppc603e) },
#endif
#if (HAS_PPC603R)
	{ CPU_PPC603R, CPU_GET_INFO_NAME(ppc603r) },
#endif
#if (HAS_PPC604)
	{ CPU_PPC604, CPU_GET_INFO_NAME(ppc604) },
#endif
#if (HAS_MPC8240)
	{ CPU_MPC8240, CPU_GET_INFO_NAME(mpc8240) },
#endif
#if (HAS_SE3208)
	{ CPU_SE3208, CPU_GET_INFO_NAME(SE3208) },
#endif
#if (HAS_MC68HC11)
	{ CPU_MC68HC11, CPU_GET_INFO_NAME(mc68hc11) },
#endif
#if (HAS_ADSP21062)
	{ CPU_ADSP21062, CPU_GET_INFO_NAME(adsp21062) },
#endif
#if (HAS_DSP56156)
	{ CPU_DSP56156, CPU_GET_INFO_NAME(dsp56k) },
#endif
#if (HAS_RSP)
	{ CPU_RSP, CPU_GET_INFO_NAME(rsp) },
#endif
#if (HAS_ALPHA8201)
	{ CPU_ALPHA8201, CPU_GET_INFO_NAME(alpha8201) },
#endif
#if (HAS_ALPHA8301)
	{ CPU_ALPHA8301, CPU_GET_INFO_NAME(alpha8301) },
#endif
#if (HAS_CDP1802)
	{ CPU_CDP1802, CPU_GET_INFO_NAME(cdp1802) },
#endif
#if (HAS_COP420)
	{ CPU_COP420, CPU_GET_INFO_NAME(cop420) },
#endif
#if (HAS_COP421)
	{ CPU_COP421, CPU_GET_INFO_NAME(cop421) },
#endif
#if (HAS_COP410)
	{ CPU_COP410, CPU_GET_INFO_NAME(cop410) },
#endif
#if (HAS_COP411)
	{ CPU_COP411, CPU_GET_INFO_NAME(cop411) },
#endif
#if (HAS_TLCS90)
	{ CPU_TMP90840, CPU_GET_INFO_NAME(tmp90840) },
	{ CPU_TMP90841, CPU_GET_INFO_NAME(tmp90841) },
	{ CPU_TMP91640, CPU_GET_INFO_NAME(tmp91640) },
	{ CPU_TMP91641, CPU_GET_INFO_NAME(tmp91641) },
#endif
#if (HAS_APEXC)
	{ CPU_APEXC, CPU_GET_INFO_NAME(apexc) },
#endif
#if (HAS_CP1610)
	{ CPU_CP1610, CPU_GET_INFO_NAME(cp1610) },
#endif
#if (HAS_F8)
	{ CPU_F8, CPU_GET_INFO_NAME(f8) },
#endif
#if (HAS_LH5801)
	{ CPU_LH5801, CPU_GET_INFO_NAME(lh5801) },
#endif
#if (HAS_PDP1)
	{ CPU_PDP1, CPU_GET_INFO_NAME(pdp1) },
#endif
#if (HAS_SATURN)
	{ CPU_SATURN, CPU_GET_INFO_NAME(saturn) },
#endif
#if (HAS_SC61860)
	{ CPU_SC61860, CPU_GET_INFO_NAME(sc61860) },
#endif
#if (HAS_TX0_64KW)
	{ CPU_TX0_64KW, CPU_GET_INFO_NAME(tx0_64kw) },
#endif
#if (HAS_TX0_8KW)
	{ CPU_TX0_8KW, CPU_GET_INFO_NAME(tx0_8kw) },
#endif
#if (HAS_LR35902)
	{ CPU_LR35902, CPU_GET_INFO_NAME(lr35902) },
#endif
#if (HAS_TMS7000)
	{ CPU_TMS7000, CPU_GET_INFO_NAME(tms7000) },
#endif
#if (HAS_TMS7000_EXL)
	{ CPU_TMS7000_EXL, CPU_GET_INFO_NAME(tms7000_exl) },
#endif
#if (HAS_SM8500)
	{ CPU_SM8500, CPU_GET_INFO_NAME(sm8500) },
#endif
#if (HAS_V30MZ)
	{ CPU_V30MZ, CPU_GET_INFO_NAME(v30mz) },
#endif
#if (HAS_MB8841)
	{ CPU_MB8841, CPU_GET_INFO_NAME(mb8841) },
#endif
#if (HAS_MB8842)
	{ CPU_MB8842, CPU_GET_INFO_NAME(mb8842) },
#endif
#if (HAS_MB8843)
	{ CPU_MB8843, CPU_GET_INFO_NAME(mb8843) },
#endif
#if (HAS_MB8844)
	{ CPU_MB8844, CPU_GET_INFO_NAME(mb8844) },
#endif
#if (HAS_MB86233)
	{ CPU_MB86233, CPU_GET_INFO_NAME(mb86233) },
#endif
#if (HAS_SSP1601)
	{ CPU_SSP1601, CPU_GET_INFO_NAME(ssp1601) },
#endif
#if (HAS_MINX)
	{ CPU_MINX, CPU_GET_INFO_NAME(minx) },
#endif
#if (HAS_CXD8661R)
	{ CPU_CXD8661R, CPU_GET_INFO_NAME(cxd8661r) },
#endif
};



/*************************************
 *
 *  Other variables we own
 *
 *************************************/

const device_config *executingcpu;
int activecpunum;		/* index of active CPU (or -1) */
int executingcpunum;	/* index of executing CPU (or -1) */
int totalcpu;		/* total number of CPUs */

static cpuintrf_data cpu[MAX_CPU];

static const device_config *cpu_context_stack[4];
static int cpu_context_stack_ptr;

#define TEMP_STRING_POOL_ENTRIES 16
static char temp_string_pool[TEMP_STRING_POOL_ENTRIES][256];
static int temp_string_pool_index;



/*************************************
 *
 *  Set a new CPU context
 *
 *************************************/

INLINE void set_cpu_context(const device_config *oldcpu, const device_config *newcpu)
{
	cpuintrf_data *cpudata = NULL;
	
	/* if nothing is changing, quick exit */
	if (newcpu == NULL || oldcpu == newcpu)
		return;

	/* save the old context if we have one */
	if (oldcpu != NULL)
	{
		cpudata = oldcpu->classtoken;
		(*cpudata->intf.get_context)(oldcpu->token);
	}

	/* swap memory spaces */
	newcpu->machine->activecpu = newcpu;

	cpudata = newcpu->classtoken;
	activecpunum = cpudata->index;
	memory_set_context(newcpu->machine, activecpunum);
	(*cpudata->intf.set_context)(newcpu->token);
}



/*************************************
 *
 *  Push/pop to a new CPU context
 *
 *************************************/

void cpu_push_context(const device_config *cpu)
{
	const device_config *oldcpu = cpu->machine->activecpu;
	cpu_context_stack[cpu_context_stack_ptr++] = oldcpu;
	set_cpu_context(oldcpu, cpu);
}


void cpu_pop_context(void)
{
	const device_config *cpu = cpu_context_stack[--cpu_context_stack_ptr];
	if (cpu != NULL)
	{
		const device_config *oldcpu = cpu->machine->activecpu;
		set_cpu_context(oldcpu, cpu);
	}
	else
	{
		Machine->activecpu = NULL;
		activecpunum = -1;
	}
}



/*************************************
 *
 *  Global temp string pool
 *
 *************************************/

char *cpuintrf_temp_str(void)
{
	char *string = &temp_string_pool[temp_string_pool_index++ % TEMP_STRING_POOL_ENTRIES][0];
	string[0] = 0;
	return string;
}



/*************************************
 *
 *  Initialize the global interface
 *
 *************************************/

void cpuintrf_init(running_machine *machine)
{
	int mapindex;

	/* reset the cpuintrf array */
	memset(cpuintrf, 0, sizeof(cpuintrf));

	/* build the cpuintrf array */
	for (mapindex = 0; mapindex < sizeof(cpuintrf_map) / sizeof(cpuintrf_map[0]); mapindex++)
	{
		cpu_type cputype = cpuintrf_map[mapindex].cputype;
		cpu_interface *intf = &cpuintrf[cputype];
		cpuinfo info;

		/* start with the get_info routine */
		intf->get_info = cpuintrf_map[mapindex].get_info;

		/* bootstrap the rest of the function pointers */
		info.setinfo = NULL;
		(*intf->get_info)(CPUINFO_PTR_SET_INFO, &info);
		intf->set_info = info.setinfo;

		info.getcontext = NULL;
		(*intf->get_info)(CPUINFO_PTR_GET_CONTEXT, &info);
		intf->get_context = info.getcontext;

		info.setcontext = NULL;
		(*intf->get_info)(CPUINFO_PTR_SET_CONTEXT, &info);
		intf->set_context = info.setcontext;

		info.init = NULL;
		(*intf->get_info)(CPUINFO_PTR_INIT, &info);
		intf->init = info.init;

		info.reset = NULL;
		(*intf->get_info)(CPUINFO_PTR_RESET, &info);
		intf->reset = info.reset;

		info.exit = NULL;
		(*intf->get_info)(CPUINFO_PTR_EXIT, &info);
		intf->exit = info.exit;

		info.execute = NULL;
		(*intf->get_info)(CPUINFO_PTR_EXECUTE, &info);
		intf->execute = info.execute;

		info.burn = NULL;
		(*intf->get_info)(CPUINFO_PTR_BURN, &info);
		intf->burn = info.burn;

		info.disassemble = NULL;
		(*intf->get_info)(CPUINFO_PTR_DISASSEMBLE, &info);
		intf->disassemble = info.disassemble;

		info.translate = NULL;
		(*intf->get_info)(CPUINFO_PTR_TRANSLATE, &info);
		intf->translate = info.translate;

		/* get other miscellaneous stuff */
		intf->context_size = cputype_context_size(cputype);
		intf->address_shift = cputype_addrbus_shift(cputype, ADDRESS_SPACE_PROGRAM);
	}

	/* fill in any empty entries with the dummy CPU */
	for (mapindex = 0; mapindex < CPU_COUNT; mapindex++)
		if (cpuintrf[mapindex].get_info == NULL)
			cpuintrf[mapindex] = cpuintrf[CPU_DUMMY];

	/* zap the CPU data structure */
	memset(cpu, 0, sizeof(cpu));
	totalcpu = 0;

	/* reset the context stack */
	memset(&cpu_context_stack[0], 0, sizeof(cpu_context_stack));
	cpu_context_stack_ptr = 0;

	/* nothing active, nothing executing */
	activecpunum = -1;
	executingcpu = NULL;
	executingcpunum = -1;
	totalcpu = 0;

	/* compute information about the CPUs now if we have a machine */
	if (machine != NULL)
	{
		/* loop over all defined CPUs */
		for (totalcpu = 0; totalcpu < CPU_COUNT; totalcpu++)
		{
			cpu_type cputype = machine->config->cpu[totalcpu].type;
			char familyname[256];
			int j;

			/* stop when we hit a dummy */
			if (cputype == CPU_DUMMY)
				break;

			/* fill in the type and interface */
			cpu[totalcpu].index = totalcpu;
			cpu[totalcpu].intf = cpuintrf[cputype];
			cpu[totalcpu].cputype = cputype;

			/* determine the family index */
			strcpy(familyname, cputype_core_file(cputype));
			for (j = 0; j < CPU_COUNT; j++)
				if (!strcmp(familyname, cputype_core_file(j)))
				{
					cpu[totalcpu].family = j;
					break;
				}
		}
	}
}



/*************************************
 *
 *  Set the disassembly override proc
 *
 *************************************/

void cpu_set_dasm_override(const device_config *cpu, cpu_disassemble_func dasm_override)
{
	cpuintrf_data *cpudata = cpu->classtoken;
	cpudata->dasm_override = dasm_override;
}



/*************************************
 *
 *  Initialize a single CPU
 *
 *************************************/

int cpuintrf_init_cpu(int cpunum, cpu_type cputype, int clock, const void *config, cpu_irq_callback irqcallback)
{
	cpuintrf_data *cpudata = &cpu[cpunum];
	cpuinfo info;

	/* create a fake device for the CPU */
	cpudata->device = auto_malloc(sizeof(*cpudata->device) + strlen(Machine->config->cpu[cpunum].tag));
	memset(cpudata->device, 0, sizeof(*cpudata->device));
	strcpy(cpudata->device->tag, Machine->config->cpu[cpunum].tag);
	cpudata->device->type = (device_type)cputype;
	cpudata->device->class = DEVICE_CLASS_CPU_CHIP;
	cpudata->device->static_config = config;
	cpudata->device->started = TRUE;
	cpudata->device->machine = Machine;
	cpudata->device->region = memory_region(Machine, cpudata->device->tag);
	cpudata->device->regionbytes = memory_region_length(Machine, cpudata->device->tag);
	
	/* allocate a context token */
	cpudata->device->token = auto_malloc(cpudata->intf.context_size);
	memset(cpudata->device->token, 0, cpudata->intf.context_size);
	
	/* set up the class token */
	cpudata->device->classtoken = &cpu[cpunum];
	
	/* put a pointer to the device in the machine */
	Machine->cpu[cpunum] = cpudata->device;

	/* initialize the CPU and stash the context */
	Machine->activecpu = Machine->cpu[cpunum];
	activecpunum = cpunum;
	(*cpudata->intf.init)(cpudata->device, cpunum, clock, irqcallback);
	(*cpudata->intf.get_context)(cpudata->device->token);
	Machine->activecpu = NULL;
	activecpunum = -1;

	/* get the instruction count pointer */
	info.icount = NULL;
	(*cpudata->intf.get_info)(CPUINFO_PTR_INSTRUCTION_COUNTER, &info);
	cpudata->icount = info.icount;
	return 0;
}



/*************************************
 *
 *  Exit/free a single CPU
 *
 *************************************/

void cpuintrf_exit_cpu(int cpunum)
{
	cpuintrf_data *cpudata = &cpu[cpunum];
	
	/* if the CPU core defines an exit function, call it now */
	if (cpudata->intf.exit)
	{
		/* switch contexts to the CPU during the exit */
		cpu_push_context(cpu->device);
		(*cpudata->intf.exit)(cpu->device);
		cpu_pop_context();
	}
}



/*************************************
 *
 *  Interfaces to the active CPU
 *
 *************************************/

/*--------------------------
    Adjust/get icount
--------------------------*/

void activecpu_adjust_icount(int delta)
{
	VERIFY_ACTIVECPU(activecpu_adjust_icount);
	*cpu[activecpunum].icount += delta;
}


int activecpu_get_icount(void)
{
	VERIFY_ACTIVECPU(activecpu_get_icount);
	return *cpu[activecpunum].icount;
}


/*--------------------------
    Reset banking pointers
--------------------------*/

void activecpu_reset_banking(void)
{
	VERIFY_ACTIVECPU(activecpu_reset_banking);
	memory_set_opbase(cpu_get_physical_pc_byte(Machine->activecpu));
}


/*************************************
 *
 *  Interfaces to a specific CPU
 *
 *************************************/

/*--------------------------
    Get info accessors
--------------------------*/

INT64 cpu_get_info_int(const device_config *cpu, UINT32 state)
{
	cpuintrf_data *cpudata = cpu->classtoken;
	cpuinfo info;

	VERIFY_CPU(cpu_get_info_int);
	cpu_push_context(cpu);
	info.i = 0;
	(*cpudata->intf.get_info)(state, &info);
	cpu_pop_context();
	return info.i;
}

void *cpu_get_info_ptr(const device_config *cpu, UINT32 state)
{
	cpuintrf_data *cpudata = cpu->classtoken;
	cpuinfo info;

	VERIFY_CPU(cpu_get_info_ptr);
	cpu_push_context(cpu);
	info.p = NULL;
	(*cpudata->intf.get_info)(state, &info);
	cpu_pop_context();
	return info.p;
}

genf *cpu_get_info_fct(const device_config *cpu, UINT32 state)
{
	cpuintrf_data *cpudata = cpu->classtoken;
	cpuinfo info;

	VERIFY_CPU(cpu_get_info_fct);
	cpu_push_context(cpu);
	info.f = NULL;
	(*cpudata->intf.get_info)(state, &info);
	cpu_pop_context();
	return info.f;
}

const char *cpu_get_info_string(const device_config *cpu, UINT32 state)
{
	cpuintrf_data *cpudata = cpu->classtoken;
	cpuinfo info;

	VERIFY_CPU(cpu_get_info_string);
	cpu_push_context(cpu);
	info.s = cpuintrf_temp_str();
	(*cpudata->intf.get_info)(state, &info);
	cpu_pop_context();
	return info.s;
}


/*--------------------------
    Set info accessors
--------------------------*/

void cpu_set_info_int(const device_config *cpu, UINT32 state, INT64 data)
{
	cpuintrf_data *cpudata = cpu->classtoken;
	cpuinfo info;

	VERIFY_CPU(cpu_set_info_int);
	info.i = data;
	cpu_push_context(cpu);
	(*cpudata->intf.set_info)(state, &info);
	cpu_pop_context();
}

void cpu_set_info_ptr(const device_config *cpu, UINT32 state, void *data)
{
	cpuintrf_data *cpudata = cpu->classtoken;
	cpuinfo info;

	VERIFY_CPU(cpu_set_info_ptr);
	info.p = data;
	cpu_push_context(cpu);
	(*cpudata->intf.set_info)(state, &info);
	cpu_pop_context();
}

void cpu_set_info_fct(const device_config *cpu, UINT32 state, genf *data)
{
	cpuintrf_data *cpudata = cpu->classtoken;
	cpuinfo info;

	VERIFY_CPU(cpu_set_info_ptr);
	info.f = data;
	cpu_push_context(cpu);
	(*cpudata->intf.set_info)(state, &info);
	cpu_pop_context();
}


/*--------------------------
    Execute
--------------------------*/

int cpu_execute(const device_config *cpu, int cycles)
{
	cpuintrf_data *cpudata = cpu->classtoken;
	int ran;

	VERIFY_CPU(cpu_execute);
	cpu_push_context(cpu);
	executingcpu = cpu;
	executingcpunum = cpudata->index;
	memory_set_opbase(cpu_get_physical_pc_byte(cpu));
	ran = (*cpudata->intf.execute)(cpu, cycles);
	executingcpu = NULL;
	executingcpunum = -1;
	cpu_pop_context();
	return ran;
}


/*--------------------------
    Reset and set IRQ ack
--------------------------*/

void cpu_reset(const device_config *cpu)
{
	cpuintrf_data *cpudata = cpu->classtoken;

	VERIFY_CPU(cpu_reset);
	cpu_push_context(cpu);
	memory_set_opbase(0);
	(*cpudata->intf.reset)(cpu);
	cpu_pop_context();
}


/*--------------------------
    Read a byte
--------------------------*/

UINT8 cpu_read_byte(const device_config *cpu, offs_t address)
{
	int result;

	VERIFY_CPU(cpu_read_byte);
	cpu_push_context(cpu);
	result = program_read_byte(address);
	cpu_pop_context();
	return result;
}


/*--------------------------
    Write a byte
--------------------------*/

void cpu_write_byte(const device_config *cpu, offs_t address, UINT8 data)
{
	VERIFY_CPU(cpu_write_byte);
	
	cpu_push_context(cpu);
	program_write_byte(address, data);
	cpu_pop_context();
}


/*--------------------------
    Get/set PC
--------------------------*/

offs_t cpu_get_physical_pc_byte(const device_config *cpu)
{
	cpuintrf_data *cpudata = cpu->classtoken;
	offs_t pc;
	int shift;

	VERIFY_CPU(cpu_get_physical_pc_byte);
	shift = cpudata->intf.address_shift;
	cpu_push_context(cpu);
	pc = cpu_get_info_int(cpu, CPUINFO_INT_PC);
	if (shift < 0)
		pc <<= -shift;
	else
		pc >>= shift;
	if (cpudata->intf.translate != NULL)
		(cpudata->intf.translate)(ADDRESS_SPACE_PROGRAM, TRANSLATE_FETCH, &pc);
	cpu_pop_context();
	return pc;
}


void cpu_set_opbase(const device_config *cpu, unsigned val)
{
	VERIFY_CPU(cpu_set_opbase);

	cpu_push_context(cpu);
	memory_set_opbase(val);
	cpu_pop_context();
}


/*--------------------------
    Disassembly
--------------------------*/

offs_t cpu_dasm(const device_config *cpu, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	cpuintrf_data *cpudata = cpu->classtoken;
	offs_t result;
	
	VERIFY_CPU(cpu_dasm);
	cpu_push_context(cpu);

	/* check for disassembler override */
	if (cpudata->dasm_override != NULL)
	{
		result = (*cpudata->dasm_override)(buffer, pc, oprom, opram);
		if (result != 0)
			return result;
	}

	if (cpudata->intf.disassemble != NULL)
		result = (*cpudata->intf.disassemble)(buffer, pc, oprom, opram);

	else
	{
		/* if no disassembler present, dump vanilla bytes */
		switch (cpu_get_min_opcode_bytes(cpu))
		{
			case 1:
			default:
				sprintf(buffer, "$%02X", (unsigned) *((UINT8 *) oprom));
				result = 1;
				break;

			case 2:
				sprintf(buffer, "$%04X", (unsigned) *((UINT16 *) oprom));
				result = 2;
				break;

			case 4:
				sprintf(buffer, "$%08X", (unsigned) *((UINT32 *) oprom));
				result = 4;
				break;
		}
	}

	/* make sure we get good results */
	assert((result & DASMFLAG_LENGTHMASK) != 0);
#ifdef MAME_DEBUG
{
	int shift = cpu_get_addrbus_shift(cpu, ADDRESS_SPACE_PROGRAM);
	int bytes = (shift < 0) ? ((result & DASMFLAG_LENGTHMASK) << -shift) : ((result & DASMFLAG_LENGTHMASK) >> shift);
	assert(bytes >= cpu_get_min_opcode_bytes(cpu));
	assert(bytes <= cpu_get_max_opcode_bytes(cpu));
	(void) bytes; /* appease compiler */
}
#endif

	cpu_pop_context();
	return result;
}



/*************************************
 *
 *  Interfaces to a specific CPU type
 *
 *************************************/

/*--------------------------
    Get info accessors
--------------------------*/

INT64 cputype_get_info_int(cpu_type cputype, UINT32 state)
{
	cpuinfo info;

	VERIFY_CPUTYPE(cputype_get_info_int);
	info.i = 0;
	(*cpuintrf[cputype].get_info)(state, &info);
	return info.i;
}

void *cputype_get_info_ptr(cpu_type cputype, UINT32 state)
{
	cpuinfo info;

	VERIFY_CPUTYPE(cputype_get_info_ptr);
	info.p = NULL;
	(*cpuintrf[cputype].get_info)(state, &info);
	return info.p;
}

genf *cputype_get_info_fct(cpu_type cputype, UINT32 state)
{
	cpuinfo info;

	VERIFY_CPUTYPE(cputype_get_info_fct);
	info.f = NULL;
	(*cpuintrf[cputype].get_info)(state, &info);
	return info.f;
}

const char *cputype_get_info_string(cpu_type cputype, UINT32 state)
{
	cpuinfo info;

	VERIFY_CPUTYPE(cputype_get_info_string);
	info.s = cpuintrf_temp_str();
	(*cpuintrf[cputype].get_info)(state, &info);
	return info.s;
}



/*************************************
 *
 *  Dummy CPU definition
 *
 *************************************/

struct dummy_context
{
	UINT32		dummy;
};

static struct dummy_context dummy_state;
static int dummy_icount;

static CPU_INIT( dummy ) { }
static CPU_RESET( dummy ) { }
static CPU_EXIT( dummy ) { }
static CPU_EXECUTE( dummy ) { return cycles; }
static CPU_GET_CONTEXT( dummy ) { }
static CPU_SET_CONTEXT( dummy ) { }

static CPU_DISASSEMBLE( dummy )
{
	strcpy(buffer, "???");
	return 1;
}

static CPU_SET_INFO( dummy )
{
}

CPU_GET_INFO( dummy )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(dummy_state); 			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 1;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = 0;							break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = 0;							break;
		case CPUINFO_INT_PC:							info->i = 0;							break;
		case CPUINFO_INT_SP:							info->i = 0;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(dummy); break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = CPU_GET_CONTEXT_NAME(dummy);	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = CPU_SET_CONTEXT_NAME(dummy);	break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(dummy);		break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(dummy);	break;
		case CPUINFO_PTR_EXIT:							info->exit = CPU_EXIT_NAME(dummy);		break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(dummy);break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(dummy); break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &dummy_icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "");					break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "no CPU");				break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "0.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "The MAME team");		break;

		case CPUINFO_STR_FLAGS:							strcpy(info->s, "--");					break;
	}
}
