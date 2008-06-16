/***************************************************************************

    cpuintrf.c

    Core CPU interface functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "driver.h"



/*************************************
 *
 *  Include headers from all CPUs
 *
 *************************************/

void dummy_get_info(UINT32 state, cpuinfo *info);
void z80_get_info(UINT32 state, cpuinfo *info);
void z180_get_info(UINT32 state, cpuinfo *info);
void i8080_get_info(UINT32 state, cpuinfo *info);
void i8085_get_info(UINT32 state, cpuinfo *info);
void m6502_get_info(UINT32 state, cpuinfo *info);
void m65c02_get_info(UINT32 state, cpuinfo *info);
void m65sc02_get_info(UINT32 state, cpuinfo *info);
void m65ce02_get_info(UINT32 state, cpuinfo *info);
void m6509_get_info(UINT32 state, cpuinfo *info);
void m6510_get_info(UINT32 state, cpuinfo *info);
void m6510t_get_info(UINT32 state, cpuinfo *info);
void m7501_get_info(UINT32 state, cpuinfo *info);
void m8502_get_info(UINT32 state, cpuinfo *info);
void n2a03_get_info(UINT32 state, cpuinfo *info);
void deco16_get_info(UINT32 state, cpuinfo *info);
void m4510_get_info(UINT32 state, cpuinfo *info);
void h6280_get_info(UINT32 state, cpuinfo *info);
void i8086_get_info(UINT32 state, cpuinfo *info);
void i8088_get_info(UINT32 state, cpuinfo *info);
void i80186_get_info(UINT32 state, cpuinfo *info);
void i80188_get_info(UINT32 state, cpuinfo *info);
void i80286_get_info(UINT32 state, cpuinfo *info);
void v20_get_info(UINT32 state, cpuinfo *info);
void v25_get_info(UINT32 state, cpuinfo *info);
void v30_get_info(UINT32 state, cpuinfo *info);
void v33_get_info(UINT32 state, cpuinfo *info);
void v35_get_info(UINT32 state, cpuinfo *info);
void v60_get_info(UINT32 state, cpuinfo *info);
void v70_get_info(UINT32 state, cpuinfo *info);
void i8035_get_info(UINT32 state, cpuinfo *info);
void i8039_get_info(UINT32 state, cpuinfo *info);
void i8048_get_info(UINT32 state, cpuinfo *info);
void i8749_get_info(UINT32 state, cpuinfo *info);
void n7751_get_info(UINT32 state, cpuinfo *info);
void mb8884_get_info(UINT32 state, cpuinfo *info);
void m58715_get_info(UINT32 state, cpuinfo *info);
void i8x41_get_info(UINT32 state, cpuinfo *info);
void i8051_get_info(UINT32 state, cpuinfo *info);
void i8052_get_info(UINT32 state, cpuinfo *info);
void i8751_get_info(UINT32 state, cpuinfo *info);
void i8752_get_info(UINT32 state, cpuinfo *info);
void ds5002fp_get_info(UINT32 state, cpuinfo *info);
void m6800_get_info(UINT32 state, cpuinfo *info);
void m6801_get_info(UINT32 state, cpuinfo *info);
void m6802_get_info(UINT32 state, cpuinfo *info);
void m6803_get_info(UINT32 state, cpuinfo *info);
void m6808_get_info(UINT32 state, cpuinfo *info);
void hd63701_get_info(UINT32 state, cpuinfo *info);
void nsc8105_get_info(UINT32 state, cpuinfo *info);
void m6805_get_info(UINT32 state, cpuinfo *info);
void m68705_get_info(UINT32 state, cpuinfo *info);
void hd63705_get_info(UINT32 state, cpuinfo *info);
void hd6309_get_info(UINT32 state, cpuinfo *info);
void m6809_get_info(UINT32 state, cpuinfo *info);
void m6809e_get_info(UINT32 state, cpuinfo *info);
void konami_get_info(UINT32 state, cpuinfo *info);
void m68000_get_info(UINT32 state, cpuinfo *info);
void m68008_get_info(UINT32 state, cpuinfo *info);
void m68010_get_info(UINT32 state, cpuinfo *info);
void m68ec020_get_info(UINT32 state, cpuinfo *info);
void m68020_get_info(UINT32 state, cpuinfo *info);
void m68040_get_info(UINT32 state, cpuinfo *info);
void t11_get_info(UINT32 state, cpuinfo *info);
void s2650_get_info(UINT32 state, cpuinfo *info);
void tms34010_get_info(UINT32 state, cpuinfo *info);
void tms34020_get_info(UINT32 state, cpuinfo *info);
void ti990_10_get_info(UINT32 state, cpuinfo *info);
void tms9900_get_info(UINT32 state, cpuinfo *info);
void tms9940_get_info(UINT32 state, cpuinfo *info);
void tms9980a_get_info(UINT32 state, cpuinfo *info);
void tms9985_get_info(UINT32 state, cpuinfo *info);
void tms9989_get_info(UINT32 state, cpuinfo *info);
void tms9995_get_info(UINT32 state, cpuinfo *info);
void tms99105a_get_info(UINT32 state, cpuinfo *info);
void tms99110a_get_info(UINT32 state, cpuinfo *info);
void tms99000_get_info(UINT32 state, cpuinfo *info);
void z8000_get_info(UINT32 state, cpuinfo *info);
void tms32010_get_info(UINT32 state, cpuinfo *info);
void tms32025_get_info(UINT32 state, cpuinfo *info);
void tms32026_get_info(UINT32 state, cpuinfo *info);
void tms32031_get_info(UINT32 state, cpuinfo *info);
void tms32032_get_info(UINT32 state, cpuinfo *info);
void tms32051_get_info(UINT32 state, cpuinfo *info);
void ccpu_get_info(UINT32 state, cpuinfo *info);
void adsp2100_get_info(UINT32 state, cpuinfo *info);
void adsp2101_get_info(UINT32 state, cpuinfo *info);
void adsp2104_get_info(UINT32 state, cpuinfo *info);
void adsp2105_get_info(UINT32 state, cpuinfo *info);
void adsp2115_get_info(UINT32 state, cpuinfo *info);
void adsp2181_get_info(UINT32 state, cpuinfo *info);
void psxcpu_get_info(UINT32 state, cpuinfo *info);
void asap_get_info(UINT32 state, cpuinfo *info);
void upd7810_get_info(UINT32 state, cpuinfo *info);
void upd7807_get_info(UINT32 state, cpuinfo *info);
void upd7801_get_info(UINT32 state, cpuinfo *info);
void upd78c05_get_info(UINT32 state, cpuinfo *info);
void upd78c06_get_info(UINT32 state, cpuinfo *info);
void jaguargpu_get_info(UINT32 state, cpuinfo *info);
void jaguardsp_get_info(UINT32 state, cpuinfo *info);
void r3000be_get_info(UINT32 state, cpuinfo *info);
void r3000le_get_info(UINT32 state, cpuinfo *info);
void r3041be_get_info(UINT32 state, cpuinfo *info);
void r3041le_get_info(UINT32 state, cpuinfo *info);
void r4600be_get_info(UINT32 state, cpuinfo *info);
void r4600le_get_info(UINT32 state, cpuinfo *info);
void r4650be_get_info(UINT32 state, cpuinfo *info);
void r4650le_get_info(UINT32 state, cpuinfo *info);
void r4700be_get_info(UINT32 state, cpuinfo *info);
void r4700le_get_info(UINT32 state, cpuinfo *info);
void r5000be_get_info(UINT32 state, cpuinfo *info);
void r5000le_get_info(UINT32 state, cpuinfo *info);
void qed5271be_get_info(UINT32 state, cpuinfo *info);
void qed5271le_get_info(UINT32 state, cpuinfo *info);
void rm7000be_get_info(UINT32 state, cpuinfo *info);
void rm7000le_get_info(UINT32 state, cpuinfo *info);
void arm_get_info(UINT32 state, cpuinfo *info);
void arm7_get_info(UINT32 state, cpuinfo *info);
void sh2_get_info(UINT32 state, cpuinfo *info);
void sh4_get_info(UINT32 state, cpuinfo *info);
void dsp32c_get_info(UINT32 state, cpuinfo *info);
void pic16C54_get_info(UINT32 state, cpuinfo *info);
void pic16C55_get_info(UINT32 state, cpuinfo *info);
void pic16C56_get_info(UINT32 state, cpuinfo *info);
void pic16C57_get_info(UINT32 state, cpuinfo *info);
void pic16C58_get_info(UINT32 state, cpuinfo *info);
void g65816_get_info(UINT32 state, cpuinfo *info);
void spc700_get_info(UINT32 state, cpuinfo *info);
void e116t_get_info(UINT32 state, cpuinfo *info);
void e116xt_get_info(UINT32 state, cpuinfo *info);
void e116xs_get_info(UINT32 state, cpuinfo *info);
void e116xsr_get_info(UINT32 state, cpuinfo *info);
void e132n_get_info(UINT32 state, cpuinfo *info);
void e132t_get_info(UINT32 state, cpuinfo *info);
void e132xn_get_info(UINT32 state, cpuinfo *info);
void e132xt_get_info(UINT32 state, cpuinfo *info);
void e132xs_get_info(UINT32 state, cpuinfo *info);
void e132xsr_get_info(UINT32 state, cpuinfo *info);
void gms30c2116_get_info(UINT32 state, cpuinfo *info);
void gms30c2132_get_info(UINT32 state, cpuinfo *info);
void gms30c2216_get_info(UINT32 state, cpuinfo *info);
void gms30c2232_get_info(UINT32 state, cpuinfo *info);
void i386_get_info(UINT32 state, cpuinfo *info);
void i486_get_info(UINT32 state, cpuinfo *info);
void pentium_get_info(UINT32 state, cpuinfo *info);
void mediagx_get_info(UINT32 state, cpuinfo *info);
void i960_get_info(UINT32 state, cpuinfo *info);
void h8_3002_get_info(UINT32 state, cpuinfo *info);
void h8_3007_get_info(UINT32 state, cpuinfo *info);
void h8_3044_get_info(UINT32 state, cpuinfo *info);
void v810_get_info(UINT32 state, cpuinfo *info);
void m37702_get_info(UINT32 state, cpuinfo *info);
void m37710_get_info(UINT32 state, cpuinfo *info);
void ppc403ga_get_info(UINT32 state, cpuinfo *info);
void ppc403gcx_get_info(UINT32 state, cpuinfo *info);
void ppc601_get_info(UINT32 state, cpuinfo *info);
void ppc602_get_info(UINT32 state, cpuinfo *info);
void ppc603_get_info(UINT32 state, cpuinfo *info);
void ppc603e_get_info(UINT32 state, cpuinfo *info);
void ppc603r_get_info(UINT32 state, cpuinfo *info);
void ppc604_get_info(UINT32 state, cpuinfo *info);
void mpc8240_get_info(UINT32 state, cpuinfo *info);
void SE3208_get_info(UINT32 state, cpuinfo *info);
void mc68hc11_get_info(UINT32 state, cpuinfo *info);
void adsp21062_get_info(UINT32 state, cpuinfo *info);
void dsp56k_get_info(UINT32 state, cpuinfo *info);
void rsp_get_info(UINT32 state, cpuinfo *info);
void alpha8201_get_info(UINT32 state, cpuinfo *info);
void alpha8301_get_info(UINT32 state, cpuinfo *info);
void cdp1802_get_info(UINT32 state, cpuinfo *info);
void cop420_get_info(UINT32 state, cpuinfo *info);
void cop421_get_info(UINT32 state, cpuinfo *info);
void cop410_get_info(UINT32 state, cpuinfo *info);
void cop411_get_info(UINT32 state, cpuinfo *info);
void tmp90840_get_info(UINT32 state, cpuinfo *info);
void tmp90841_get_info(UINT32 state, cpuinfo *info);
void tmp91640_get_info(UINT32 state, cpuinfo *info);
void tmp91641_get_info(UINT32 state, cpuinfo *info);
void apexc_get_info(UINT32 state, cpuinfo *info);
void cp1610_get_info(UINT32 state, cpuinfo *info);
void f8_get_info(UINT32 state, cpuinfo *info);
void lh5801_get_info(UINT32 state, cpuinfo *info);
void pdp1_get_info(UINT32 state, cpuinfo *info);
void saturn_get_info(UINT32 state, cpuinfo *info);
void sc61860_get_info(UINT32 state, cpuinfo *info);
void tx0_64kw_get_info(UINT32 state, cpuinfo *info);
void tx0_8kw_get_info(UINT32 state, cpuinfo *info);
void lr35902_get_info(UINT32 state, cpuinfo *info);
void tms7000_get_info(UINT32 state, cpuinfo *info);
void tms7000_exl_get_info(UINT32 state, cpuinfo *info);
void sm8500_get_info(UINT32 state, cpuinfo *info);
void v30mz_get_info(UINT32 state, cpuinfo *info);
void mb8841_get_info(UINT32 state, cpuinfo *info);
void mb8842_get_info(UINT32 state, cpuinfo *info);
void mb8843_get_info(UINT32 state, cpuinfo *info);
void mb8844_get_info(UINT32 state, cpuinfo *info);
void mb86233_get_info(UINT32 state, cpuinfo *info);
void ssp1601_get_info(UINT32 state, cpuinfo *info);
void minx_get_info(UINT32 state, cpuinfo *info);
void cxd8661r_get_info(UINT32 state, cpuinfo *info);



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

#define VERIFY_ACTIVECPU(name) \
	assert_always(activecpu >= 0, #name "() called with no active cpu!")

#define VERIFY_CPUNUM(name)	\
	assert_always(cpunum >= 0 && cpunum < totalcpu, #name "() called for invalid cpu num!")

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
	cpu_interface intf;		 		/* copy of the interface data */
	cpu_type cputype; 					/* type index of this CPU */
	int family; 					/* family index of this CPU */
	void *context;					/* dynamically allocated context buffer */
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
	{ CPU_DUMMY, dummy_get_info },
#if (HAS_Z80)
	{ CPU_Z80, z80_get_info },
#endif
#if (HAS_Z180)
	{ CPU_Z180, z180_get_info },
#endif
#if (HAS_8080)
	{ CPU_8080, i8080_get_info },
#endif
#if (HAS_8085A)
	{ CPU_8085A, i8085_get_info },
#endif
#if (HAS_M6502)
	{ CPU_M6502, m6502_get_info },
#endif
#if (HAS_M65C02)
	{ CPU_M65C02, m65c02_get_info },
#endif
#if (HAS_M65SC02)
	{ CPU_M65SC02, m65sc02_get_info },
#endif
#if (HAS_M65CE02)
	{ CPU_M65CE02, m65ce02_get_info },
#endif
#if (HAS_M6509)
	{ CPU_M6509, m6509_get_info },
#endif
#if (HAS_M6510)
	{ CPU_M6510, m6510_get_info },
#endif
#if (HAS_M6510T)
	{ CPU_M6510T, m6510t_get_info },
#endif
#if (HAS_M7501)
	{ CPU_M7501, m7501_get_info },
#endif
#if (HAS_M8502)
	{ CPU_M8502, m8502_get_info },
#endif
#if (HAS_N2A03)
	{ CPU_N2A03, n2a03_get_info },
#endif
#if (HAS_DECO16)
	{ CPU_DECO16, deco16_get_info },
#endif
#if (HAS_M4510)
	{ CPU_M4510, m4510_get_info },
#endif
#if (HAS_H6280)
	{ CPU_H6280, h6280_get_info },
#endif
#if (HAS_I8086)
	{ CPU_I8086, i8086_get_info },
#endif
#if (HAS_I8088)
	{ CPU_I8088, i8088_get_info },
#endif
#if (HAS_I80186)
	{ CPU_I80186, i80186_get_info },
#endif
#if (HAS_I80188)
	{ CPU_I80188, i80188_get_info },
#endif
#if (HAS_I80286)
	{ CPU_I80286, i80286_get_info },
#endif
#if (HAS_V20)
	{ CPU_V20, v20_get_info },
#endif
#if (HAS_V25)
	{ CPU_V25, v25_get_info },
#endif
#if (HAS_V30)
	{ CPU_V30, v30_get_info },
#endif
#if (HAS_V33)
	{ CPU_V33, v33_get_info },
#endif
#if (HAS_V35)
	{ CPU_V35, v35_get_info },
#endif
#if (HAS_V60)
	{ CPU_V60, v60_get_info },
#endif
#if (HAS_V70)
	{ CPU_V70, v70_get_info },
#endif
#if (HAS_I8035)
	{ CPU_I8035, i8035_get_info },
#endif
#if (HAS_I8039)
	{ CPU_I8039, i8039_get_info },
#endif
#if (HAS_I8048)
	{ CPU_I8048, i8048_get_info },
#endif
#if (HAS_I8749)
	{ CPU_I8749, i8749_get_info },
#endif
#if (HAS_N7751)
	{ CPU_N7751, n7751_get_info },
#endif
#if (HAS_MB8884)
	{ CPU_MB8884, mb8884_get_info },
#endif
#if (HAS_M58715)
	{ CPU_M58715, m58715_get_info },
#endif
#if (HAS_I8X41)
	{ CPU_I8X41, i8x41_get_info },
#endif
#if (HAS_I8051)
	{ CPU_I8051, i8051_get_info },
#endif
#if (HAS_I8052)
	{ CPU_I8052, i8052_get_info },
#endif
#if (HAS_I8751)
	{ CPU_I8751, i8751_get_info },
#endif
#if (HAS_I8752)
	{ CPU_I8752, i8752_get_info },
#endif
#if (HAS_DS5002FP)
	{ CPU_DS5002FP, ds5002fp_get_info },
#endif
#if (HAS_M6800)
	{ CPU_M6800, m6800_get_info },
#endif
#if (HAS_M6801)
	{ CPU_M6801, m6801_get_info },
#endif
#if (HAS_M6802)
	{ CPU_M6802, m6802_get_info },
#endif
#if (HAS_M6803)
	{ CPU_M6803, m6803_get_info },
#endif
#if (HAS_M6808)
	{ CPU_M6808, m6808_get_info },
#endif
#if (HAS_HD63701)
	{ CPU_HD63701, hd63701_get_info },
#endif
#if (HAS_NSC8105)
	{ CPU_NSC8105, nsc8105_get_info },
#endif
#if (HAS_M6805)
	{ CPU_M6805, m6805_get_info },
#endif
#if (HAS_M68705)
	{ CPU_M68705, m68705_get_info },
#endif
#if (HAS_HD63705)
	{ CPU_HD63705, hd63705_get_info },
#endif
#if (HAS_HD6309)
	{ CPU_HD6309, hd6309_get_info },
#endif
#if (HAS_M6809)
	{ CPU_M6809, m6809_get_info },
#endif
#if (HAS_M6809E)
	{ CPU_M6809E, m6809e_get_info },
#endif
#if (HAS_KONAMI)
	{ CPU_KONAMI, konami_get_info },
#endif
#if (HAS_M68000)
	{ CPU_M68000, m68000_get_info },
#endif
#if (HAS_M68008)
	{ CPU_M68008, m68008_get_info },
#endif
#if (HAS_M68010)
	{ CPU_M68010, m68010_get_info },
#endif
#if (HAS_M68EC020)
	{ CPU_M68EC020, m68ec020_get_info },
#endif
#if (HAS_M68020)
	{ CPU_M68020, m68020_get_info },
#endif
#if (HAS_M68040)
	{ CPU_M68040, m68040_get_info },
#endif
#if (HAS_T11)
	{ CPU_T11, t11_get_info },
#endif
#if (HAS_S2650)
	{ CPU_S2650, s2650_get_info },
#endif
#if (HAS_TMS34010)
	{ CPU_TMS34010, tms34010_get_info },
#endif
#if (HAS_TMS34020)
	{ CPU_TMS34020, tms34020_get_info },
#endif
#if (HAS_TI990_10)
	{ CPU_TI990_10, ti990_10_get_info },
#endif
#if (HAS_TMS9900)
	{ CPU_TMS9900, tms9900_get_info },
#endif
#if (HAS_TMS9940)
	{ CPU_TMS9940, tms9940_get_info },
#endif
#if (HAS_TMS9980)
	{ CPU_TMS9980, tms9980a_get_info },
#endif
#if (HAS_TMS9985)
	{ CPU_TMS9985, tms9985_get_info },
#endif
#if (HAS_TMS9989)
	{ CPU_TMS9989, tms9989_get_info },
#endif
#if (HAS_TMS9995)
	{ CPU_TMS9995, tms9995_get_info },
#endif
#if (HAS_TMS99105A)
	{ CPU_TMS99105A, tms99105a_get_info },
#endif
#if (HAS_TMS99110A)
	{ CPU_TMS99110A, tms99110a_get_info },
#endif
#if (HAS_TMS99000)
	{ CPU_TMS99000, tms99000_get_info },
#endif
#if (HAS_Z8000)
	{ CPU_Z8000, z8000_get_info },
#endif
#if (HAS_TMS32010)
	{ CPU_TMS32010, tms32010_get_info },
#endif
#if (HAS_TMS32025)
	{ CPU_TMS32025, tms32025_get_info },
#endif
#if (HAS_TMS32026)
	{ CPU_TMS32026, tms32026_get_info },
#endif
#if (HAS_TMS32031)
	{ CPU_TMS32031, tms32031_get_info },
#endif
#if (HAS_TMS32032)
	{ CPU_TMS32032, tms32032_get_info },
#endif
#if (HAS_TMS32051)
	{ CPU_TMS32051, tms32051_get_info },
#endif
#if (HAS_CCPU)
	{ CPU_CCPU, ccpu_get_info },
#endif
#if (HAS_ADSP2100)
	{ CPU_ADSP2100, adsp2100_get_info },
#endif
#if (HAS_ADSP2101)
	{ CPU_ADSP2101, adsp2101_get_info },
#endif
#if (HAS_ADSP2104)
	{ CPU_ADSP2104, adsp2104_get_info },
#endif
#if (HAS_ADSP2105)
	{ CPU_ADSP2105, adsp2105_get_info },
#endif
#if (HAS_ADSP2115)
	{ CPU_ADSP2115, adsp2115_get_info },
#endif
#if (HAS_ADSP2181)
	{ CPU_ADSP2181, adsp2181_get_info },
#endif
#if (HAS_PSXCPU)
	{ CPU_PSXCPU, psxcpu_get_info },
#endif
#if (HAS_ASAP)
	{ CPU_ASAP, asap_get_info },
#endif
#if (HAS_UPD7810)
	{ CPU_UPD7810, upd7810_get_info },
#endif
#if (HAS_UPD7807)
	{ CPU_UPD7807, upd7807_get_info },
#endif
#if (HAS_UPD7801)
	{ CPU_UPD7801, upd7801_get_info },
	{ CPU_UPD78C05, upd78c05_get_info },
	{ CPU_UPD78C06, upd78c06_get_info },
#endif
#if (HAS_JAGUAR)
	{ CPU_JAGUARGPU, jaguargpu_get_info },
	{ CPU_JAGUARDSP, jaguardsp_get_info },
#endif
#if (HAS_R3000)
	{ CPU_R3000BE, r3000be_get_info },
	{ CPU_R3000LE, r3000le_get_info },
#endif
#if (HAS_R3041)
	{ CPU_R3041BE, r3041be_get_info },
	{ CPU_R3041LE, r3041le_get_info },
#endif
#if (HAS_R4600)
	{ CPU_R4600BE, r4600be_get_info },
	{ CPU_R4600LE, r4600le_get_info },
#endif
#if (HAS_R4650)
	{ CPU_R4650BE, r4650be_get_info },
	{ CPU_R4650LE, r4650le_get_info },
#endif
#if (HAS_R4700)
	{ CPU_R4700BE, r4700be_get_info },
	{ CPU_R4700LE, r4700le_get_info },
#endif
#if (HAS_R5000)
	{ CPU_R5000BE, r5000be_get_info },
	{ CPU_R5000LE, r5000le_get_info },
#endif
#if (HAS_QED5271)
	{ CPU_QED5271BE, qed5271be_get_info },
	{ CPU_QED5271LE, qed5271le_get_info },
#endif
#if (HAS_RM7000)
	{ CPU_RM7000BE, rm7000be_get_info },
	{ CPU_RM7000LE, rm7000le_get_info },
#endif
#if (HAS_ARM)
	{ CPU_ARM, arm_get_info },
#endif
#if (HAS_ARM7)
	{ CPU_ARM7, arm7_get_info },
#endif
#if (HAS_SH2)
	{ CPU_SH2, sh2_get_info },
#endif
#if (HAS_SH4)
	{ CPU_SH4, sh4_get_info },
#endif
#if (HAS_DSP32C)
	{ CPU_DSP32C, dsp32c_get_info },
#endif
#if (HAS_PIC16C54)
	{ CPU_PIC16C54, pic16C54_get_info },
#endif
#if (HAS_PIC16C55)
	{ CPU_PIC16C55, pic16C55_get_info },
#endif
#if (HAS_PIC16C56)
	{ CPU_PIC16C56, pic16C56_get_info },
#endif
#if (HAS_PIC16C57)
	{ CPU_PIC16C57, pic16C57_get_info },
#endif
#if (HAS_PIC16C58)
	{ CPU_PIC16C58, pic16C58_get_info },
#endif
#if (HAS_G65816)
	{ CPU_G65816, g65816_get_info },
#endif
#if (HAS_SPC700)
	{ CPU_SPC700, spc700_get_info },
#endif
#if (HAS_E116T)
	{ CPU_E116T, e116t_get_info },
#endif
#if (HAS_E116XT)
	{ CPU_E116XT, e116xt_get_info },
#endif
#if (HAS_E116XS)
	{ CPU_E116XS, e116xs_get_info },
#endif
#if (HAS_E116XSR)
	{ CPU_E116XSR, e116xsr_get_info },
#endif
#if (HAS_E132N)
	{ CPU_E132N, e132n_get_info },
#endif
#if (HAS_E132T)
	{ CPU_E132T, e132t_get_info },
#endif
#if (HAS_E132XN)
	{ CPU_E132XN, e132xn_get_info },
#endif
#if (HAS_E132XT)
	{ CPU_E132XT, e132xt_get_info },
#endif
#if (HAS_E132XS)
	{ CPU_E132XS, e132xs_get_info },
#endif
#if (HAS_E132XSR)
	{ CPU_E132XSR, e132xsr_get_info },
#endif
#if (HAS_GMS30C2116)
	{ CPU_GMS30C2116, gms30c2116_get_info },
#endif
#if (HAS_GMS30C2132)
	{ CPU_GMS30C2132, gms30c2132_get_info },
#endif
#if (HAS_GMS30C2216)
	{ CPU_GMS30C2216, gms30c2216_get_info },
#endif
#if (HAS_GMS30C2232)
	{ CPU_GMS30C2232, gms30c2232_get_info },
#endif
#if (HAS_I386)
	{ CPU_I386, i386_get_info },
#endif
#if (HAS_I486)
	{ CPU_I486, i486_get_info },
#endif
#if (HAS_PENTIUM)
	{ CPU_PENTIUM, pentium_get_info },
#endif
#if (HAS_MEDIAGX)
	{ CPU_MEDIAGX, mediagx_get_info },
#endif
#if (HAS_I960)
	{ CPU_I960, i960_get_info },
#endif
#if (HAS_H83002)
	{ CPU_H83002, h8_3002_get_info },
	{ CPU_H83007, h8_3007_get_info },
	{ CPU_H83044, h8_3044_get_info },
#endif
#if (HAS_V810)
	{ CPU_V810, v810_get_info },
#endif
#if (HAS_M37702)
	{ CPU_M37702, m37702_get_info },
#endif
#if (HAS_M37710)
	{ CPU_M37710, m37710_get_info },
#endif
#if (HAS_PPC403GA)
	{ CPU_PPC403GA, ppc403ga_get_info },
#endif
#if (HAS_PPC403GCX)
	{ CPU_PPC403GCX, ppc403gcx_get_info },
#endif
#if (HAS_PPC601)
	{ CPU_PPC601, ppc601_get_info },
#endif
#if (HAS_PPC602)
	{ CPU_PPC602, ppc602_get_info },
#endif
#if (HAS_PPC603)
	{ CPU_PPC603, ppc603_get_info },
#endif
#if (HAS_PPC603E)
	{ CPU_PPC603E, ppc603e_get_info },
#endif
#if (HAS_PPC603R)
	{ CPU_PPC603R, ppc603r_get_info },
#endif
#if (HAS_PPC604)
	{ CPU_PPC604, ppc604_get_info },
#endif
#if (HAS_MPC8240)
	{ CPU_MPC8240, mpc8240_get_info },
#endif
#if (HAS_SE3208)
	{ CPU_SE3208, SE3208_get_info },
#endif
#if (HAS_MC68HC11)
	{ CPU_MC68HC11, mc68hc11_get_info },
#endif
#if (HAS_ADSP21062)
	{ CPU_ADSP21062, adsp21062_get_info },
#endif
#if (HAS_DSP56156)
	{ CPU_DSP56156, dsp56k_get_info },
#endif
#if (HAS_RSP)
	{ CPU_RSP, rsp_get_info },
#endif
#if (HAS_ALPHA8201)
	{ CPU_ALPHA8201, alpha8201_get_info },
#endif
#if (HAS_ALPHA8301)
	{ CPU_ALPHA8301, alpha8301_get_info },
#endif
#if (HAS_CDP1802)
	{ CPU_CDP1802, cdp1802_get_info },
#endif
#if (HAS_COP420)
	{ CPU_COP420, cop420_get_info },
#endif
#if (HAS_COP421)
	{ CPU_COP421, cop421_get_info },
#endif
#if (HAS_COP410)
	{ CPU_COP410, cop410_get_info },
#endif
#if (HAS_COP411)
	{ CPU_COP411, cop411_get_info },
#endif
#if (HAS_TLCS90)
	{ CPU_TMP90840, tmp90840_get_info },
	{ CPU_TMP90841, tmp90841_get_info },
	{ CPU_TMP91640, tmp91640_get_info },
	{ CPU_TMP91641, tmp91641_get_info },
#endif
#if (HAS_APEXC)
	{ CPU_APEXC, apexc_get_info },
#endif
#if (HAS_CP1610)
	{ CPU_CP1610, cp1610_get_info },
#endif
#if (HAS_F8)
	{ CPU_F8, f8_get_info },
#endif
#if (HAS_LH5801)
	{ CPU_LH5801, lh5801_get_info },
#endif
#if (HAS_PDP1)
	{ CPU_PDP1, pdp1_get_info },
#endif
#if (HAS_SATURN)
	{ CPU_SATURN, saturn_get_info },
#endif
#if (HAS_SC61860)
	{ CPU_SC61860, sc61860_get_info },
#endif
#if (HAS_TX0_64KW)
	{ CPU_TX0_64KW, tx0_64kw_get_info },
#endif
#if (HAS_TX0_8KW)
	{ CPU_TX0_8KW, tx0_8kw_get_info },
#endif
#if (HAS_LR35902)
	{ CPU_LR35902, lr35902_get_info },
#endif
#if (HAS_TMS7000)
	{ CPU_TMS7000, tms7000_get_info },
#endif
#if (HAS_TMS7000_EXL)
	{ CPU_TMS7000_EXL, tms7000_exl_get_info },
#endif
#if (HAS_SM8500)
	{ CPU_SM8500, sm8500_get_info },
#endif
#if (HAS_V30MZ)
	{ CPU_V30MZ, v30mz_get_info },
#endif
#if (HAS_MB8841)
	{ CPU_MB8841, mb8841_get_info },
#endif
#if (HAS_MB8842)
	{ CPU_MB8842, mb8842_get_info },
#endif
#if (HAS_MB8843)
	{ CPU_MB8843, mb8843_get_info },
#endif
#if (HAS_MB8844)
	{ CPU_MB8844, mb8844_get_info },
#endif
#if (HAS_MB86233)
	{ CPU_MB86233, mb86233_get_info },
#endif
#if (HAS_SSP1601)
	{ CPU_SSP1601, ssp1601_get_info },
#endif
#if (HAS_MINX)
	{ CPU_MINX, minx_get_info },
#endif
#if (HAS_CXD8661R)
	{ CPU_CXD8661R, cxd8661r_get_info },
#endif
};



/*************************************
 *
 *  Other variables we own
 *
 *************************************/

int activecpu;		/* index of active CPU (or -1) */
int executingcpu;	/* index of executing CPU (or -1) */
int totalcpu;		/* total number of CPUs */

static cpuintrf_data cpu[MAX_CPU];

static int cpu_active_context[CPU_COUNT];
static int cpu_context_stack[4];
static int cpu_context_stack_ptr;

static offs_t (*cpu_dasm_override[CPU_COUNT])(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

#define TEMP_STRING_POOL_ENTRIES 16
static char temp_string_pool[TEMP_STRING_POOL_ENTRIES][256];
static int temp_string_pool_index;



/*************************************
 *
 *  Set a new CPU context
 *
 *************************************/

INLINE void set_cpu_context(int cpunum)
{
	int newfamily = cpu[cpunum].family;
	int oldcontext = cpu_active_context[newfamily];

	/* if we need to change contexts, save the one that was there */
	if (oldcontext != cpunum && oldcontext != -1)
		(*cpu[oldcontext].intf.get_context)(cpu[oldcontext].context);

	/* swap memory spaces */
	activecpu = cpunum;
	memory_set_context(cpunum);

	/* if the new CPU's context is not swapped in, do it now */
	if (oldcontext != cpunum)
	{
		(*cpu[cpunum].intf.set_context)(cpu[cpunum].context);
		cpu_active_context[newfamily] = cpunum;
	}
}



/*************************************
 *
 *  Push/pop to a new CPU context
 *
 *************************************/

void cpuintrf_push_context(int cpunum)
{
	/* push the old context onto the stack */
	cpu_context_stack[cpu_context_stack_ptr++] = activecpu;

	/* do the rest only if this isn't the activecpu */
	if (cpunum != activecpu && cpunum != -1)
		set_cpu_context(cpunum);

	/* this is now the active CPU */
	activecpu = cpunum;
}


void cpuintrf_pop_context(void)
{
	/* push the old context onto the stack */
	int cpunum = cpu_context_stack[--cpu_context_stack_ptr];

	/* do the rest only if this isn't the activecpu */
	if (cpunum != activecpu && cpunum != -1)
		set_cpu_context(cpunum);

	/* this is now the active CPU */
	activecpu = cpunum;
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

		/* also reset the active CPU context info */
		cpu_active_context[cputype] = -1;
	}

	/* fill in any empty entries with the dummy CPU */
	for (mapindex = 0; mapindex < CPU_COUNT; mapindex++)
		if (cpuintrf[mapindex].get_info == NULL)
			cpuintrf[mapindex] = cpuintrf[CPU_DUMMY];

	/* zap the CPU data structure */
	memset(cpu, 0, sizeof(cpu));
	totalcpu = 0;
	memset(cpu_dasm_override, 0, sizeof(cpu_dasm_override));

	/* reset the context stack */
	memset(cpu_context_stack, -1, sizeof(cpu_context_stack));
	cpu_context_stack_ptr = 0;

	/* nothing active, nothing executing */
	activecpu = -1;
	executingcpu = -1;
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

void cpuintrf_set_dasm_override(int cpunum, offs_t (*dasm_override)(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram))
{
	cpu_dasm_override[cpunum] = dasm_override;
}



/*************************************
 *
 *  Initialize a single CPU
 *
 *************************************/

int cpuintrf_init_cpu(int cpunum, cpu_type cputype, int clock, const void *config, int (*irqcallback)(int))
{
	cpuinfo info;

	/* allocate a context buffer for the CPU */
	cpu[cpunum].context = auto_malloc(cpu[cpunum].intf.context_size);
	memset(cpu[cpunum].context, 0, cpu[cpunum].intf.context_size);

	/* initialize the CPU and stash the context */
	activecpu = cpunum;
	(*cpu[cpunum].intf.init)(cpunum, clock, config, irqcallback);
	(*cpu[cpunum].intf.get_context)(cpu[cpunum].context);
	activecpu = -1;

	/* get the instruction count pointer */
	info.icount = NULL;
	(*cpu[cpunum].intf.get_info)(CPUINFO_PTR_INSTRUCTION_COUNTER, &info);
	cpu[cpunum].intf.icount = info.icount;

	/* clear out the registered CPU for this family */
	cpu_active_context[cpu[cpunum].family] = -1;
	return 0;
}



/*************************************
 *
 *  Exit/free a single CPU
 *
 *************************************/

void cpuintrf_exit_cpu(int cpunum)
{
	/* if the CPU core defines an exit function, call it now */
	if (cpu[cpunum].intf.exit)
	{
		/* switch contexts to the CPU during the exit */
		cpuintrf_push_context(cpunum);
		(*cpu[cpunum].intf.exit)();
		cpuintrf_pop_context();
	}
}



/*************************************
 *
 *  Interfaces to the active CPU
 *
 *************************************/

/*--------------------------
    Get info accessors
--------------------------*/

INT64 activecpu_get_info_int(UINT32 state)
{
	cpuinfo info;

	VERIFY_ACTIVECPU(activecpu_get_info_int);
	info.i = 0;
	(*cpu[activecpu].intf.get_info)(state, &info);
	return info.i;
}

void *activecpu_get_info_ptr(UINT32 state)
{
	cpuinfo info;

	VERIFY_ACTIVECPU(activecpu_get_info_ptr);
	info.p = NULL;
	(*cpu[activecpu].intf.get_info)(state, &info);
	return info.p;
}

genf *activecpu_get_info_fct(UINT32 state)
{
	cpuinfo info;

	VERIFY_ACTIVECPU(activecpu_get_info_fct);
	info.f = NULL;
	(*cpu[activecpu].intf.get_info)(state, &info);
	return info.f;
}

const char *activecpu_get_info_string(UINT32 state)
{
	cpuinfo info;

	VERIFY_ACTIVECPU(activecpu_get_info_string);
	info.s = cpuintrf_temp_str();
	(*cpu[activecpu].intf.get_info)(state, &info);
	return info.s;
}


/*--------------------------
    Set info accessors
--------------------------*/

void activecpu_set_info_int(UINT32 state, INT64 data)
{
	cpuinfo info;
	VERIFY_ACTIVECPU(activecpu_set_info_int);
	info.i = data;
	(*cpu[activecpu].intf.set_info)(state, &info);
}

void activecpu_set_info_ptr(UINT32 state, void *data)
{
	cpuinfo info;
	VERIFY_ACTIVECPU(activecpu_set_info_ptr);
	info.p = data;
	(*cpu[activecpu].intf.set_info)(state, &info);
}

void activecpu_set_info_fct(UINT32 state, genf *data)
{
	cpuinfo info;
	VERIFY_ACTIVECPU(activecpu_set_info_fct);
	info.f = data;
	(*cpu[activecpu].intf.set_info)(state, &info);
}


/*--------------------------
    Adjust/get icount
--------------------------*/

void activecpu_adjust_icount(int delta)
{
	VERIFY_ACTIVECPU(activecpu_adjust_icount);
	*cpu[activecpu].intf.icount += delta;
}


int activecpu_get_icount(void)
{
	VERIFY_ACTIVECPU(activecpu_get_icount);
	return *cpu[activecpu].intf.icount;
}


/*--------------------------
    Reset banking pointers
--------------------------*/

void activecpu_reset_banking(void)
{
	VERIFY_ACTIVECPU(activecpu_reset_banking);
	memory_set_opbase(activecpu_get_physical_pc_byte());
}


/*--------------------------
    Input line setting
--------------------------*/

void activecpu_set_input_line(int irqline, int state)
{
	VERIFY_ACTIVECPU(activecpu_set_input_line);
	if (state != INTERNAL_CLEAR_LINE && state != INTERNAL_ASSERT_LINE)
	{
		logerror("activecpu_set_input_line called when cpu_set_input_line should have been used!\n");
		return;
	}
	activecpu_set_info_int(CPUINFO_INT_INPUT_STATE + irqline, state - INTERNAL_CLEAR_LINE);
}


/*--------------------------
    Get/set PC
--------------------------*/

offs_t activecpu_get_physical_pc_byte(void)
{
	offs_t pc;
	int shift;

	VERIFY_ACTIVECPU(activecpu_get_physical_pc_byte);
	shift = cpu[activecpu].intf.address_shift;
	pc = activecpu_get_reg(REG_PC);
	if (shift < 0)
		pc <<= -shift;
	else
		pc >>= shift;
	if (cpu[activecpu].intf.translate)
		(*cpu[activecpu].intf.translate)(ADDRESS_SPACE_PROGRAM, TRANSLATE_FETCH, &pc);
	return pc;
}


void activecpu_set_opbase(unsigned val)
{
	VERIFY_ACTIVECPU(activecpu_set_opbase);
	memory_set_opbase(val);
}


/*--------------------------
    Disassembly
--------------------------*/

offs_t activecpu_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	unsigned result;

	VERIFY_ACTIVECPU(activecpu_dasm);

	/* check for disassembler override */
	if (cpu_dasm_override[activecpu])
	{
		result = (*cpu_dasm_override[activecpu])(buffer, pc, oprom, opram);
		if (result != 0)
			return result;
	}

	if (cpu[activecpu].intf.disassemble != NULL)
	{
		result = (*cpu[activecpu].intf.disassemble)(buffer, pc, oprom, opram);
	}
	else
	{
		/* if no disassembler present, dump vanilla bytes */
		switch (activecpu_min_instruction_bytes())
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
	int shift = activecpu_addrbus_shift(ADDRESS_SPACE_PROGRAM);
	int bytes = (shift < 0) ? ((result & DASMFLAG_LENGTHMASK) << -shift) : ((result & DASMFLAG_LENGTHMASK) >> shift);
	assert(bytes >= activecpu_min_instruction_bytes());
	assert(bytes <= activecpu_max_instruction_bytes());
	(void) bytes; /* appease compiler */
}
#endif

	return result;
}



/*************************************
 *
 *  Interfaces to a specific CPU
 *
 *************************************/

/*--------------------------
    Get info accessors
--------------------------*/

INT64 cpunum_get_info_int(int cpunum, UINT32 state)
{
	cpuinfo info;

	VERIFY_CPUNUM(cpunum_get_info_int);
	cpuintrf_push_context(cpunum);
	info.i = 0;
	(*cpu[cpunum].intf.get_info)(state, &info);
	cpuintrf_pop_context();
	return info.i;
}

void *cpunum_get_info_ptr(int cpunum, UINT32 state)
{
	cpuinfo info;

	VERIFY_CPUNUM(cpunum_get_info_ptr);
	cpuintrf_push_context(cpunum);
	info.p = NULL;
	(*cpu[cpunum].intf.get_info)(state, &info);
	cpuintrf_pop_context();
	return info.p;
}

genf *cpunum_get_info_fct(int cpunum, UINT32 state)
{
	cpuinfo info;

	VERIFY_CPUNUM(cpunum_get_info_fct);
	cpuintrf_push_context(cpunum);
	info.f = NULL;
	(*cpu[cpunum].intf.get_info)(state, &info);
	cpuintrf_pop_context();
	return info.f;
}

const char *cpunum_get_info_string(int cpunum, UINT32 state)
{
	cpuinfo info;

	VERIFY_CPUNUM(cpunum_get_info_string);
	cpuintrf_push_context(cpunum);
	info.s = cpuintrf_temp_str();
	(*cpu[cpunum].intf.get_info)(state, &info);
	cpuintrf_pop_context();
	return info.s;
}


/*--------------------------
    Set info accessors
--------------------------*/

void cpunum_set_info_int(int cpunum, UINT32 state, INT64 data)
{
	cpuinfo info;
	VERIFY_CPUNUM(cpunum_set_info_int);
	info.i = data;
	cpuintrf_push_context(cpunum);
	(*cpu[cpunum].intf.set_info)(state, &info);
	cpuintrf_pop_context();
}

void cpunum_set_info_ptr(int cpunum, UINT32 state, void *data)
{
	cpuinfo info;
	VERIFY_CPUNUM(cpunum_set_info_ptr);
	info.p = data;
	cpuintrf_push_context(cpunum);
	(*cpu[cpunum].intf.set_info)(state, &info);
	cpuintrf_pop_context();
}

void cpunum_set_info_fct(int cpunum, UINT32 state, genf *data)
{
	cpuinfo info;
	VERIFY_CPUNUM(cpunum_set_info_ptr);
	info.f = data;
	cpuintrf_push_context(cpunum);
	(*cpu[cpunum].intf.set_info)(state, &info);
	cpuintrf_pop_context();
}


/*--------------------------
    Execute
--------------------------*/

int cpunum_execute(int cpunum, int cycles)
{
	int ran;
	VERIFY_CPUNUM(cpunum_execute);
	cpuintrf_push_context(cpunum);
	executingcpu = cpunum;
	memory_set_opbase(activecpu_get_physical_pc_byte());
	ran = (*cpu[cpunum].intf.execute)(cycles);
	executingcpu = -1;
	cpuintrf_pop_context();
	return ran;
}


/*--------------------------
    Reset and set IRQ ack
--------------------------*/

void cpunum_reset(int cpunum)
{
	VERIFY_CPUNUM(cpunum_reset);
	cpuintrf_push_context(cpunum);
	memory_set_opbase(0);
	(*cpu[cpunum].intf.reset)();
	cpuintrf_pop_context();
}


/*--------------------------
    Read a byte
--------------------------*/

UINT8 cpunum_read_byte(int cpunum, offs_t address)
{
	int result;
	VERIFY_CPUNUM(cpunum_read_byte);
	cpuintrf_push_context(cpunum);
	result = program_read_byte(address);
	cpuintrf_pop_context();
	return result;
}


/*--------------------------
    Write a byte
--------------------------*/

void cpunum_write_byte(int cpunum, offs_t address, UINT8 data)
{
	VERIFY_CPUNUM(cpunum_write_byte);
	cpuintrf_push_context(cpunum);
	program_write_byte(address, data);
	cpuintrf_pop_context();
}


/*--------------------------
    Get context pointer
--------------------------*/

void *cpunum_get_context_ptr(int cpunum)
{
	VERIFY_CPUNUM(cpunum_get_context_ptr);
	return (cpu_active_context[cpu[cpunum].family] == cpunum) ? NULL : cpu[cpunum].context;
}


/*--------------------------
    Get/set PC
--------------------------*/

offs_t cpunum_get_physical_pc_byte(int cpunum)
{
	offs_t pc;
	int shift;

	VERIFY_CPUNUM(cpunum_get_physical_pc_byte);
	shift = cpu[cpunum].intf.address_shift;
	cpuintrf_push_context(cpunum);
	pc = activecpu_get_info_int(CPUINFO_INT_PC);
	if (shift < 0)
		pc <<= -shift;
	else
		pc >>= shift;
	if (cpu[activecpu].intf.translate)
		(*cpu[activecpu].intf.translate)(ADDRESS_SPACE_PROGRAM, TRANSLATE_FETCH, &pc);
	cpuintrf_pop_context();
	return pc;
}


void cpunum_set_opbase(int cpunum, unsigned val)
{
	VERIFY_CPUNUM(cpunum_set_opbase);
	cpuintrf_push_context(cpunum);
	memory_set_opbase(val);
	cpuintrf_pop_context();
}


/*--------------------------
    Disassembly
--------------------------*/

offs_t cpunum_dasm(int cpunum, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	unsigned result;
	VERIFY_CPUNUM(cpunum_dasm);
	cpuintrf_push_context(cpunum);
	result = activecpu_dasm(buffer, pc, oprom, opram);
	cpuintrf_pop_context();
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

static void dummy_init(int index, int clock, const void *config, int (*irqcallback)(int)) { }
static void dummy_reset(void) { }
static void dummy_exit(void) { }
static int dummy_execute(int cycles) { return cycles; }
static void dummy_get_context(void *regs) { }
static void dummy_set_context(void *regs) { }

static offs_t dummy_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	strcpy(buffer, "???");
	return 1;
}

static void dummy_set_info(UINT32 state, cpuinfo *info)
{
}

void dummy_get_info(UINT32 state, cpuinfo *info)
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
		case CPUINFO_PTR_SET_INFO:						info->setinfo = dummy_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = dummy_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = dummy_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = dummy_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = dummy_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = dummy_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = dummy_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = dummy_dasm;			break;
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
