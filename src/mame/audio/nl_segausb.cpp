// license:CC0
// copyright-holders:Aaron Giles

//
// Netlist for Sega Universal Sound Board
//
// The Sega Universal Sound Board was used by several early G80
// vector and raster games, notably Star Trek and Tac/Scan,
// among others. It is largely implemented as a MAME device, but
// the analog audio section is implemented here.
//
// Known problems/issues:
//
//    * WIP.
//

#include "netlist/devices/net_lib.h"
#include "nl_segausb.h"



//
// Optimizations
//

#define USE_AFUNC_MIXING (1)
#define ENABLE_NOISE_FRONTIERS (1)
#define UNDERCLOCK_NOISE_GEN (1)
#define ENABLE_FRONTIERS (0)



//
// Hacks
//



NETLIST_START(segausb)

	SOLVER(Solver, 1000)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 2e-5)

	ANALOG_INPUT(I_U12_DAC, 0)      // AD7524
	ANALOG_INPUT(I_U13_DAC, 0)      // AD7524
	ANALOG_INPUT(I_U14_DAC, 0)      // AD7524
	TTL_INPUT(I_U2B_SEL, 0)         // 74LS74

	ANALOG_INPUT(I_U24_DAC, 0)      // AD7524
	ANALOG_INPUT(I_U25_DAC, 0)      // AD7524
	ANALOG_INPUT(I_U26_DAC, 0)      // AD7524
	TTL_INPUT(I_U38B_SEL, 0)        // 74LS74

	ANALOG_INPUT(I_U27_DAC, 0)      // AD7524
	ANALOG_INPUT(I_U28_DAC, 0)      // AD7524
	ANALOG_INPUT(I_U29_DAC, 0)      // AD7524
	TTL_INPUT(I_U2A_SEL, 0)         // 74LS74

	NET_C(GND, I_U2B_SEL.GND, I_U38B_SEL.GND, I_U2A_SEL.GND)
	NET_C(I_V5, I_U2B_SEL.VCC, I_U38B_SEL.VCC, I_U2A_SEL.VCC)

	TTL_INPUT(I_U41_OUT0, 0)        // 8253 PIT U41
	TTL_INPUT(I_U41_OUT1, 0)
	TTL_INPUT(I_U41_OUT2, 0)

	TTL_INPUT(I_U42_OUT0, 0)        // 8253 PIT U42
	TTL_INPUT(I_U42_OUT1, 0)
	TTL_INPUT(I_U42_OUT2, 0)

	TTL_INPUT(I_U43_OUT0, 0)        // 8253 PIT U43
	TTL_INPUT(I_U43_OUT1, 0)
	TTL_INPUT(I_U43_OUT2, 0)

	NET_C(GND, I_U41_OUT0.GND, I_U41_OUT1.GND, I_U41_OUT2.GND)
	NET_C(I_V5, I_U41_OUT0.VCC, I_U41_OUT1.VCC, I_U41_OUT2.VCC)

	NET_C(GND, I_U42_OUT0.GND, I_U42_OUT1.GND, I_U42_OUT2.GND)
	NET_C(I_V5, I_U42_OUT0.VCC, I_U42_OUT1.VCC, I_U42_OUT2.VCC)

	NET_C(GND, I_U43_OUT0.GND, I_U43_OUT1.GND, I_U43_OUT2.GND)
	NET_C(I_V5, I_U43_OUT0.VCC, I_U43_OUT1.VCC, I_U43_OUT2.VCC)

	ANALOG_INPUT(I_V5, 5)
	ANALOG_INPUT(I_V12, 12)
	ANALOG_INPUT(I_VM12, -12)

	RES(R7, RES_K(100))
	RES(R8, RES_K(100))
	RES(R9, RES_K(100))
	RES(R10, RES_K(100))
	RES(R12, RES_K(100))
	RES(R13, RES_K(100))
	RES(R14, RES_K(33))
	RES(R15, RES_K(100))
	RES(R16, RES_K(100))
	RES(R17, RES_K(5.6))
	RES(R18, RES_K(100))
	RES(R19, RES_K(100))
	RES(R20, RES_K(10))
	RES(R21, RES_K(10))
	RES(R22, RES_K(10))
	RES(R23, RES_K(1))
	RES(R24, RES_K(1))
	RES(R25, RES_K(100))
	RES(R26, RES_K(5.6))
	RES(R27, RES_K(10))
	RES(R28, RES_K(100))
	RES(R29, RES_K(100))
	RES(R30, RES_K(1))
	RES(R31, RES_K(100))
	RES(R32, RES_K(100))
	RES(R33, RES_K(1))
	RES(R34, RES_K(100))
	RES(R35, RES_K(100))
	RES(R36, RES_K(100))
	RES(R37, RES_K(33))
	RES(R38, RES_K(100))
	RES(R39, RES_K(100))
	RES(R40, RES_K(10))
	RES(R41, RES_K(10))
	RES(R42, RES_K(100))
	RES(R43, RES_K(100))
	RES(R44, RES_K(33))
	RES(R45, RES_K(100))
	RES(R46, RES_K(100))
	RES(R47, RES_K(5.6))
	RES(R48, RES_K(100))
	RES(R49, RES_K(100))
	RES(R50, RES_K(10))
	RES(R51, RES_K(10))
	RES(R52, RES_K(10))
	RES(R53, RES_K(1))
	RES(R54, RES_K(100))
	RES(R55, RES_K(100))
	RES(R56, RES_K(1))
	RES(R59, RES_K(1))
	RES(R60, RES_K(2.2))
	RES(R61, RES_K(33))
	RES(R62, 270)
	RES(R63, RES_K(1))
	RES(R64, RES_K(2.7))
	RES(R65, RES_K(2.7))

//  CAP(C6, CAP_P(100))
//  CAP(C7, CAP_P(100))
//  CAP(C8, CAP_P(100))
	CAP(C9, CAP_U(0.01))
	CAP(C13, CAP_U(0.01))
	CAP(C14, CAP_U(1))
	CAP(C15, CAP_U(1))
	CAP(C16, CAP_U(0.01))
	CAP(C17, CAP_U(0.01))
//  CAP(C19, CAP_P(100))
//  CAP(C20, CAP_P(100))
//  CAP(C21, CAP_P(100))
//  CAP(C22, CAP_P(100))
//  CAP(C23, CAP_P(100))
//  CAP(C24, CAP_P(100))
	CAP(C25, CAP_U(0.01))
	CAP(C32, CAP_U(0.01))
	CAP(C33, CAP_U(1))
	CAP(C34, CAP_U(1))
	CAP(C35, CAP_U(1))
	CAP(C36, CAP_U(1))
	CAP(C51, CAP_U(0.15))
	CAP(C52, CAP_U(0.1))
	CAP(C53, CAP_U(0.082))
	CAP(C54, CAP_U(1))
	CAP(C55, CAP_U(0.15))
	CAP(C56, CAP_U(0.15))

//  TL082_DIP(U1)           // Op. Amp.
//  NET_C(U1.7, I_V12)
//  NET_C(U1.4, I_VM12)

//  TTL_74LS74(U2)          // Dual D-Type Positive Edge-Triggered Flip-Flop -- not emulated
//  NET_C(U2.7, GND)
//  NET_C(U2.14, I_V5)

	TL082_DIP(U3)           // Op. Amp.
	NET_C(U3.8, I_V12)
	NET_C(U3.4, I_VM12)

	TL082_DIP(U4)           // Op. Amp.
	NET_C(U4.8, I_V12)
	NET_C(U4.4, I_VM12)

	TL082_DIP(U5)           // Op. Amp.
	NET_C(U5.8, I_V12)
	NET_C(U5.4, I_VM12)

	TL082_DIP(U6)           // Op. Amp.
	NET_C(U6.8, I_V12)
	NET_C(U6.4, I_VM12)

	CD4053_DIP(U7)          // 3x analog demuxer
	NET_C(U7.16, I_V5)
	NET_C(U7.6, GND)        // INH
	NET_C(U7.7, I_V12)      // VEE
	NET_C(U7.8, GND)

	CD4053_DIP(U8)          // 3x analog demuxer
	NET_C(U8.16, I_V5)
	NET_C(U8.6, GND)        // INH
	NET_C(U8.7, I_V12)      // VEE
	NET_C(U8.8, GND)

	TL082_DIP(U9)           // Op. Amp.
	NET_C(U9.8, I_V12)
	NET_C(U9.4, I_VM12)

//  TTL_74LS139_DIP(U10)    // Dual 1-of-4 Decoder -- not emulated

//  TTL_74LS139_DIP(U11)    // Dual 1-of-4 Decoder -- not emulated

//  AD7524_DIP(U12)         // DAC -- not emulated
//  NET_C(U12.3, GND)
//  NET_C(U12.14, I_V5)

//  AD7524_DIP(U13)         // DAC -- not emulated
//  NET_C(U12.3, GND)
//  NET_C(U12.14, I_V5)

//  AD7524_DIP(U14)         // DAC -- not emulated
//  NET_C(U12.3, GND)
//  NET_C(U12.14, I_V5)

	CD4053_DIP(U15)         // 3x analog demuxer
	NET_C(U15.16, I_V5)
	NET_C(U15.6, GND)       // INH
	NET_C(U15.7, I_V12)     // VEE
	NET_C(U15.8, GND)

	CD4053_DIP(U16)         // 3x analog demuxer
	NET_C(U16.16, I_V5)
	NET_C(U16.6, GND)       // INH
	NET_C(U16.7, I_V12)     // VEE
	NET_C(U16.8, GND)

	TL082_DIP(U17)          // Op. Amp.
	NET_C(U17.8, I_V12)
	NET_C(U17.4, I_VM12)

	TL082_DIP(U18)          // Op. Amp.
	NET_C(U18.8, I_V12)
	NET_C(U18.4, I_VM12)

	TL082_DIP(U19)          // Op. Amp.
	NET_C(U19.8, I_V12)
	NET_C(U19.4, I_VM12)

	TL082_DIP(U20)          // Op. Amp.
	NET_C(U20.8, I_V12)
	NET_C(U20.4, I_VM12)

	TL082_DIP(U21)          // Op. Amp.
	NET_C(U21.8, I_V12)
	NET_C(U21.4, I_VM12)

	TL082_DIP(U22)          // Op. Amp.
	NET_C(U22.8, I_V12)
	NET_C(U22.4, I_VM12)

	TL082_DIP(U23)          // Op. Amp.
	NET_C(U23.8, I_V12)
	NET_C(U23.4, I_VM12)

//  AD7524_DIP(U24)         // DAC -- not emulated
//  NET_C(U24.3, GND)
//  NET_C(U24.14, I_V5)

//  AD7524_DIP(U25)         // DAC -- not emulated
//  NET_C(U25.3, GND)
//  NET_C(U25.14, I_V5)

//  AD7524_DIP(U26)         // DAC -- not emulated
//  NET_C(U26.3, GND)
//  NET_C(U26.14, I_V5)

//  AD7524_DIP(U27)         // DAC -- not emulated
//  NET_C(U27.3, GND)
//  NET_C(U27.14, I_V5)

//  AD7524_DIP(U28)         // DAC -- not emulated
//  NET_C(U28.3, GND)
//  NET_C(U28.14, I_V5)

//  AD7524_DIP(U29)         // DAC -- not emulated
//  NET_C(U29.3, GND)
//  NET_C(U29.14, I_V5)

	CD4053_DIP(U30)         // 3x analog demuxer
	NET_C(U30.16, I_V5)
	NET_C(U30.6, GND)       // INH
	NET_C(U30.7, I_V12)     // VEE
	NET_C(U30.8, GND)

	CD4053_DIP(U31)         // 3x analog demuxer
	NET_C(U31.16, I_V5)
	NET_C(U31.6, GND)       // INH
	NET_C(U31.7, I_V12)     // VEE
	NET_C(U31.8, GND)

//  TTL_74LS74(U38)         // Dual D-Type Positive Edge-Triggered Flip-Flop -- not emulated
//  NET_C(U38.7, GND)
//  NET_C(U38.14, I_V5)

	TL081_DIP(U49)          // Op. Amp.
	NET_C(U49.7, I_V12)
	NET_C(U49.4, I_VM12)

	MM5837_DIP(U60)
	NET_C(U60.2, I_VM12)
	NET_C(U60.4, I_V12)
#if (UNDERCLOCK_NOISE_GEN)
	// officially runs at 48-112kHz, but little noticeable difference
	// in exchange for a big performance boost
	PARAM(U60.FREQ, 24000)
#endif

	//
	// Sheet 6, noise source
	//

	NET_C(U60.1, GND)
	NET_C(U60.3, R65.1)
	NET_C(R65.2, R64.2, R63.2, R62.2, C53.2, C52.1)
	NET_C(R64.1, C54.1)
	NET_C(C54.2, GND)
	NET_C(R63.1, C55.2, C56.2)
	NET_C(C55.1, GND)
	NET_C(C56.1, GND)
	NET_C(R62.1, C51.2)
	NET_C(C51.1, GND)
	NET_C(C53.1, GND)
	NET_C(C52.2, R61.2, U49.3)
	NET_C(R61.1, GND)
	NET_C(U49.2, R60.1, R59.2)
	NET_C(R60.2, U49.6)
	NET_C(R59.1, GND)
#if (ENABLE_NOISE_FRONTIERS)
	AFUNC(NOISEFUNC, 1, "A0")
	NET_C(R60.2, NOISEFUNC.A0)
	ALIAS(NOISE, NOISEFUNC.Q)
#else
	ALIAS(NOISE, R60.2)
#endif

	//
	// Sheet 7, top-left
	//

	NET_C(I_U42_OUT0, C14.1)
	NET_C(C14.2, R20.2, U3.5)
	NET_C(R20.1, GND)
	NET_C(U3.6, U3.7)
	AFUNC(DAC_U12, 2, "A0*A1")
	NET_C(DAC_U12.A0, U3.7)
	NET_C(DAC_U12.A1, I_U12_DAC)
	NET_C(DAC_U12.Q, R12.2)

	NET_C(I_U42_OUT1, C15.1)
	NET_C(C15.2, R21.2, U4.5)
	NET_C(R21.1, GND)
	NET_C(U4.6, U4.7)
	AFUNC(DAC_U13, 2, "A0*A1")
	NET_C(DAC_U13.A0, U4.7)
	NET_C(DAC_U13.A1, I_U13_DAC)
	NET_C(DAC_U13.Q, R13.2)

	NET_C(I_U42_OUT2, U15.10, U15.9)
	NET_C(U15.15, U15.14, R32.1)
	NET_C(U15.1, R33.1)
	NET_C(U15.12, NOISE)
	NET_C(U15.13, U16.3)
	NET_C(U15.11, U16.11, U16.10, U16.9, I_U2B_SEL)
	NET_C(R33.2, C13.2, R32.2, R31.1, U15.4)
	NET_C(C13.1, GND)
	NET_C(U15.3, R30.1)
	NET_C(R31.2, R30.2, C9.2, U6.3)
	NET_C(C9.1, GND)
	NET_C(U6.2, R22.2, R17.1)
	NET_C(R22.1, GND)
	NET_C(R17.2, U6.1, U16.14)

	NET_C(U16.13, U16.5, R9.1)
	NET_C(U16.12, R19.2)
	NET_C(U16.15, NOISE)
	NET_C(U16.1, R18.2)
	NET_C(R18.1, R19.1, U5.6, R15.2)
	NET_C(U5.5, GND)
	NET_C(R15.1, U5.7)
	AFUNC(DAC_U14, 2, "A0*A1")
	PARAM(DAC_U14.THRESH, 1e-5)
	NET_C(DAC_U14.A0, U5.7)
	NET_C(DAC_U14.A1, I_U14_DAC)
	NET_C(DAC_U14.Q, R14.2)
	NET_C(R14.1, R13.1, R12.1, U6.6, R16.1)
	NET_C(U6.5, GND)
	NET_C(U6.7, R16.2, U16.4)

	//
	// Sheet 7, bottom-left
	//

	NET_C(I_U41_OUT0, C34.1)
	NET_C(C34.2, R41.2, U19.5)
	NET_C(R41.1, GND)
	NET_C(U19.6, U19.7)
	AFUNC(DAC_U26, 2, "A0*A1")
	NET_C(DAC_U26.A0, U19.7)
	NET_C(DAC_U26.A1, I_U26_DAC)
	NET_C(DAC_U26.Q, R39.2)

	NET_C(I_U41_OUT1, C33.1)
	NET_C(C33.2, R40.2, U18.5)
	NET_C(R40.1, GND)
	NET_C(U18.6, U18.7)
	AFUNC(DAC_U25, 2, "A0*A1")
	NET_C(DAC_U25.A0, U18.7)
	NET_C(DAC_U25.A1, I_U25_DAC)
	NET_C(DAC_U25.Q, R38.2)

	NET_C(I_U41_OUT2, U8.10, U8.9)
	NET_C(U8.15, U8.14, R25.1)
	NET_C(U8.1, R24.1)
	NET_C(U8.12, NOISE)
	NET_C(U8.13, U7.3)
	NET_C(U8.11, U7.11, U7.10, U7.9, I_U38B_SEL)
	NET_C(R24.2, C16.2, R25.2, R29.1, U8.4)
	NET_C(C16.1, GND)
	NET_C(U8.3, R23.1)
	NET_C(R23.2, R29.2, C17.2, U9.3)
	NET_C(C17.1, GND)
	NET_C(U9.2, R27.2, R26.1)
	NET_C(R27.1, GND)
	NET_C(R26.2, U9.1, U7.14)

	NET_C(U7.13, U7.5, R8.1)
	NET_C(U7.12, R35.2)
	NET_C(U7.15, NOISE)
	NET_C(U7.1, R34.2)
	NET_C(R34.1, R35.1, U17.6, R36.2)
	NET_C(U17.5, GND)
	NET_C(R36.1, U17.7)
	AFUNC(DAC_U24, 2, "A0*A1")
	PARAM(DAC_U24.THRESH, 1e-5)
	NET_C(DAC_U24.A0, U17.7)
	NET_C(DAC_U24.A1, I_U24_DAC)
	NET_C(DAC_U24.Q, R37.2)
	NET_C(R37.1, R38.1, R39.1, U9.6, R28.1)
	NET_C(U9.5, GND)
	NET_C(U9.7, R28.2, U7.4)

	//
	// Sheet 7, top-right
	//

	NET_C(I_U43_OUT0, C35.1)
	NET_C(C35.2, R50.2, U20.5)
	NET_C(R50.1, GND)
	NET_C(U20.6, U20.7)
	AFUNC(DAC_U27, 2, "A0*A1")
	NET_C(DAC_U27.A0, U20.7)
	NET_C(DAC_U27.A1, I_U27_DAC)
	NET_C(DAC_U27.Q, R42.2)

	NET_C(I_U43_OUT1, C36.1)
	NET_C(C36.2, R51.2, U21.5)
	NET_C(R51.1, GND)
	NET_C(U21.6, U21.7)
	AFUNC(DAC_U28, 2, "A0*A1")
	NET_C(DAC_U28.A0, U21.7)
	NET_C(DAC_U28.A1, I_U28_DAC)
	NET_C(DAC_U28.Q, R43.2)

	NET_C(I_U43_OUT2, U30.10, U30.9)
	NET_C(U30.15, U30.14, R55.1)
	NET_C(U30.1, R56.1)
	NET_C(U30.12, NOISE)
	NET_C(U30.13, U31.3)
	NET_C(U30.11, U31.11, U31.10, U31.9, I_U2A_SEL)
	NET_C(R54.2, C32.2, R55.2, R56.2, U30.4)
	NET_C(C32.1, GND)
	NET_C(U30.3, R53.1)
	NET_C(R53.2, R54.1, C25.2, U23.3)
	NET_C(C25.1, GND)
	NET_C(U23.2, R52.2, R47.1)
	NET_C(R52.1, GND)
	NET_C(R47.2, U23.1, U31.14)

	NET_C(U31.13, U31.5, R10.1)
	NET_C(U31.12, R49.2)
	NET_C(U31.15, NOISE)
	NET_C(U31.1, R48.2)
	NET_C(R48.1, R49.1, U22.6, R45.2)
	NET_C(U22.5, GND)
	NET_C(R45.1, U22.7)
	AFUNC(DAC_U29, 2, "A0*A1")
	PARAM(DAC_U29.THRESH, 1e-5)
	NET_C(DAC_U29.A0, U22.7)
	NET_C(DAC_U29.A1, I_U29_DAC)
	NET_C(DAC_U29.Q, R44.2)
	NET_C(R42.1, R43.1, R44.1, U23.6, R46.1)
	NET_C(U23.5, GND)
	NET_C(U23.7, R46.2, U31.4)

#if (USE_AFUNC_MIXING)
	// This prevents the individual sound nets from combining
	// at the mixing resistors.
	AFUNC(MIXFUNC, 4, "(A0+A1+A2+A3)/4")
	NET_C(MIXFUNC.A0, R7.1)
	NET_C(MIXFUNC.A1, R10.2)
	NET_C(MIXFUNC.A2, R9.2)
	NET_C(MIXFUNC.A3, R8.2)
	ALIAS(OUTPUT, MIXFUNC.Q)
#else
	NET_C(R7.1, R10.2, R9.2, R8.2)
	ALIAS(OUTPUT, R8.2)
#endif

	//
	// Unconnected inputs
	//
	NET_C(GND, R7.2)
	NET_C(GND, U3.2, U3.3, U4.2, U4.3, U5.2, U5.3)
	NET_C(GND, U17.2, U17.3, U18.2, U18.3, U19.2, U19.3)
	NET_C(GND, U20.2, U20.3, U21.2, U21.3, U22.2, U22.3)

	// unconnected inputs to analog switch -- what's the right approach?
	NET_C(GND, U8.2, U8.5, U7.2)
	NET_C(GND, U15.2, U15.5, U16.2)
	NET_C(GND, U30.2, U30.5, U31.2)
/*
    OPTIMIZE_FRONTIER(R48.1, RES_M(1), 50)
    OPTIMIZE_FRONTIER(R49.1, RES_M(1), 50)
    OPTIMIZE_FRONTIER(R18.1, RES_M(1), 50)
    OPTIMIZE_FRONTIER(R19.1, RES_M(1), 50)
    OPTIMIZE_FRONTIER(R34.1, RES_M(1), 50)
    OPTIMIZE_FRONTIER(R35.1, RES_M(1), 50)
*/

NETLIST_END()
