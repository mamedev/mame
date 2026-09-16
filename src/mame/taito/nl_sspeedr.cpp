// license:CC0-1.0
// copyright-holders:Colin Douglas Howell

#include "netlist/devices/net_lib.h"


// This is a netlist description for the sound circuits of Midway's Super
// Speed Race, based on the schematics in _Midway's Super Speed Race Parts and
// Operating Manual_. These circuits are generally quite similar to those in
// Midway's earlier games 280-ZZZAP and Laguna Racer (see nl_280zzzap.cpp),
// but the Super Speed Race circuits have some significant changes, and their
// board layout is different.

// The components are all labeled as they are on the Super Speed Race
// schematics, except that since the noise generator discrete components are
// on the main motherboard and thus may have the same labels as components on
// the audio board, a prefix "NOISE_" has been added to their labels. Each
// LM3900 IC contains 4 op-amps, so I have labeled these op-amps as _1 to _4,
// the same scheme used in the LM3900 datasheet, using the pin numbers on the
// schematic to identify the specific op-amp. Likewise, each CD4016 IC
// contains 4 switches, so I identify a specific switch from its pin numbers
// on the schematic; these are labeled _A through _D in the CD4016 datasheet,
// so I use that scheme.

// The sound circuitry can be divided into three sections: engine sound
// generation and control, generation of noise-based sound effects (skidding
// and crashes), and the final mix and amplification of all sounds.

// For detailed descriptions of these circuits, see the 280-ZZZAP/Laguna Racer
// netlist, nl_280zzzap.cpp. I do not repeat those descriptions here.


// Include special frontiers within engine sound oscillators whose purpose is
// to improve solution convergence by reducing the oscillators' inherent
// numeric instability. Also adjusts values of resistors associated with these
// oscillators to maintain their frequency and voltage levels.

#define CONVERGENCE_FRONTIERS   1

// Enable a voltage limiter on the output of the op-amp which generates the
// post-crash sound. This removes voltage glitches from the output which would
// otherwise be audible when the netlist is run with a fixed 48 kHz timestep,
// as it is in MAME.

#define REMOVE_POST_CRASH_NOISE_GLITCHES  1


static NETLIST_START(sspeedr_noisegen)
{

	// See p. 25 of the Super Speed Race manual for the motherboard
	// schematic, which includes the noise generator.

	// This is a digital noise source of 12-volt amplitude, formed by a
	// linear-feedback shift register driven by a 4 kHz clock.

	CD4006_DIP(H5_SHFTREG)
	CD4070_DIP(J5_XORx4)

	NET_C(I_V12.Q, H5_SHFTREG.14, J5_XORx4.14)
	NET_C(GND, H5_SHFTREG.7, J5_XORx4.7)
	NET_C(GND, J5_XORx4.8, J5_XORx4.9)

	RES(NOISE_R22, RES_K(680))
	RES(NOISE_R23, RES_K(56))
	RES(NOISE_R24, RES_K(27))
	RES(NOISE_R25, RES_K(220))
	RES(NOISE_R62, RES_M(1))

	// The following is a very low-valued dummy resistance inserted to
	// make the noise source's startup circuit voltage behave properly;
	// without this it just stays at zero from the beginning and the
	// generator refuses to start:
	RES(NOISE_RDUMMY, RES_R(1))

	CAP(NOISE_C8, CAP_U(0.001))
	CAP(NOISE_C9, CAP_U(0.1))
	DIODE(NOISE_CR3, "1N4148")

	NET_C(GND, J5_XORx4.6)
	NET_C(J5_XORx4.4, J5_XORx4.12, NOISE_C8.1)
	NET_C(I_V12.Q, J5_XORx4.13)
	NET_C(J5_XORx4.11, H5_SHFTREG.3, NOISE_R23.1)
	NET_C(NOISE_R23.2, NOISE_C8.2, NOISE_R22.1)
	NET_C(NOISE_R22.2, J5_XORx4.5)

	NET_C(H5_SHFTREG.13, H5_SHFTREG.5)
	NET_C(H5_SHFTREG.12, H5_SHFTREG.1, J5_XORx4.1)
	NET_C(H5_SHFTREG.8, J5_XORx4.2)
	NET_C(H5_SHFTREG.10, H5_SHFTREG.6)
	NET_C(NOISE_R24.2, H5_SHFTREG.4)
	NET_C(J5_XORx4.3, NOISE_R25.1)
	NET_C(NOISE_R25.2, NOISE_CR3.K, NOISE_R24.1)
	NET_C(NOISE_C9.2, NOISE_CR3.A, NOISE_R62.1)
	NET_C(I_V12.Q, NOISE_RDUMMY.1)
	NET_C(NOISE_RDUMMY.2, NOISE_C9.1)
	NET_C(NOISE_R62.2, GND)

	ALIAS(NOISE_OUT, H5_SHFTREG.10)

}


//
// Main netlist
//


static NETLIST_START(sspeedr_schematics)
{

	// **** Conversion of accelerator level to "engine speed" control
	// **** voltage for engine sound, with capacitor-based "engine
	// **** inertia" and gear shift changes.

	CD4016_DIP(A2)

	LM3900(C1_3)
	LM3900(C1_4)

	RES(R42, RES_K(1))
	RES(R43, RES_K(1))
	RES(R44, RES_K(1))
	RES(R45, RES_K(1))
	RES(R62, RES_K(1))
	RES(R61, RES_K(1))

	RES(R51, RES_K(470))
	RES(R46, RES_M(2.2))
	RES(R47, RES_M(1))
	RES(R48, RES_K(470))
	RES(R49, RES_K(240))

	RES(R50, RES_K(100))
	CAP(C30, CAP_U(1))
	RES(R53, RES_K(47))
	RES(R52, RES_K(10))
	RES(R56, RES_R(270))

	RES(R54, RES_K(560))
	RES(R55, RES_M(4.7))

	RES(R57, RES_M(2.7))
	RES(R58, RES_K(560))

	RES(R59, RES_K(560))
	RES(R60, RES_K(10))

	CAP(C18, CAP_U(22))

	NET_C(C18.2, GND)

	CAP(C19, CAP_U(2.2))

	DIODE(D4, 1N914)
	DIODE(D5, 1N914)

	NET_C(I_V5.Q, R42.1, R43.1, R44.1, R45.1, R62.1, R61.1, R51.1,
		R55.1, R57.1)

	NET_C(PEDAL_BIT0, R42.2, R46.1)
	NET_C(PEDAL_BIT1, R43.2, R47.1)
	NET_C(PEDAL_BIT2, R44.2, R48.1)
	NET_C(PEDAL_BIT3, R45.2, R49.1)
	NET_C(C1_3.PLUS, R46.2, R47.2, R48.2, R49.2, R51.2)
	NET_C(C1_3.MINUS, R50.2, C30.2)
	NET_C(C1_3.OUT, R50.1, C30.1, R53.1, D4.K)
	NET_C(R53.2, R52.1, C18.1, R56.2, R54.1)
	NET_C(R52.2, D4.A)
	NET_C(C1_4.PLUS, R54.2, R55.2)
	NET_C(C1_4.MINUS, R58.2, R57.2, A2.4)
	// The following are only local connections to C1_4.OUT. It also runs
	// to all the oscillators; those connections are listed further down.
	NET_C(C1_4.OUT, D5.A, R59.1)
	NET_C(D5.K, R58.1)
	NET_C(C19.1, R59.2, R60.2)

	NET_C(A2.1, GND)
	NET_C(A2.2, R56.1)
	NET_C(A2.13, ENGINE_SOUND_OFF)

	NET_C(A2.5, R62.2, HI_SHIFT)

	NET_C(A2.3, A2.8, C19.2)
	NET_C(A2.9, R60.1)
	NET_C(A2.6, R61.2, LO_SHIFT)


	// **** Engine sound generation, using three triangle-wave oscillators
	// **** running at frequency and amplitude determined by engine speed
	// **** control voltage and modulated by MC3340 amplifier/attenuator.

	// These are the remaining connections from C1_4.OUT:
	NET_C(C1_4.OUT, R36.1, R37.1, R31.1, R32.1, R29.1, R30.1)

	MC3340_DIP(MC3340_F1)

	CD4016_DIP(B1)

	// First oscillator (bottommost in schematics).

	LM3900(C1_2)
	LM3900(C1_1)

	RES(R36, RES_K(560))
	RES(R37, RES_K(270))
	RES(R38, RES_K(100))
	RES(R41, RES_K(10))
	NET_C(R41.2, GND)

#if CONVERGENCE_FRONTIERS
	// Schmitt trigger resistors changed to compensate for waveform changes
	// from use of frontier:
	RES(R39, RES_K(465.0))
	RES(R40, RES_K(275.0))
#else
	RES(R39, RES_K(470))
	RES(R40, RES_K(270))
#endif

	CAP(C17, CAP_U(0.022))

	DIODE(D3, 1N914)

	NET_C(R37.2, B1.10)
	NET_C(B1.11, C1_2.PLUS)
	NET_C(R36.2, C1_2.MINUS, C17.1)
	NET_C(C1_2.OUT, C17.2, R38.1, D3.A)
	NET_C(R38.2, C1_1.MINUS)
	NET_C(C1_1.OUT, R40.1, B1.12)
	NET_C(I_V5.Q, R39.1)
	NET_C(C1_1.PLUS, R39.2, R40.2)
	NET_C(R41.1, D3.K, D2.K, C16.1)

	// Second oscillator (middle in schematics).

	LM3900(A1_3)
	LM3900(A1_4)

	RES(R31, RES_K(300))
	RES(R32, RES_K(150))
	RES(R33, RES_K(100))

#if CONVERGENCE_FRONTIERS
	// Schmitt trigger resistors changed to compensate for waveform changes
	// from use of frontier:
	RES(R34, RES_K(460))
	RES(R35, RES_K(281))
#else
	RES(R34, RES_K(470))
	RES(R35, RES_K(270))
#endif

	CAP(C15, CAP_U(0.01))

	CAP(C16, CAP_U(10))

	DIODE(D2, 1N914)

	NET_C(R32.2, B1.4)
	NET_C(B1.3, A1_3.PLUS)
	NET_C(R31.2, A1_3.MINUS, C15.1)
	NET_C(A1_3.OUT, C15.2, R33.1, D2.A)
	NET_C(R33.2, A1_4.MINUS)
	NET_C(A1_4.OUT, R35.1, B1.5)
	NET_C(I_V5.Q, R34.1)
	NET_C(A1_4.PLUS, R34.2, R35.2)
	NET_C(C16.2, MC3340_F1.1)  // to MC3340 input pin

	// Third oscillator (topmost in schematics).

	LM3900(A1_2)
	LM3900(A1_1)

	RES(R29, RES_K(220))

	RES(R30, RES_K(100))

	RES(R28, RES_K(100))

#if CONVERGENCE_FRONTIERS
	// Schmitt trigger resistors changed to compensate for waveform changes
	// from use of frontier.
	RES(R26, RES_K(453))
	RES(R27, RES_K(286))
#else
	RES(R26, RES_K(470))
	RES(R27, RES_K(270))
#endif

	CAP(C14, CAP_U(0.01))

	RES(R23, RES_K(10))
	NET_C(R23.1, I_V5.Q)

	RES(R25, RES_K(3.3))
	NET_C(R25.2, GND)

	CAP(C13, CAP_U(10))

	RES(R22, RES_R(470))

	DIODE(D1, 1N914)

	NET_C(I_V5.Q, R22.1)
	NET_C(ENGINE_SOUND_OFF, R22.2, D1.A)

	NET_C(R30.2, B1.1)
	NET_C(B1.2, A1_2.PLUS)
	NET_C(R29.2, A1_2.MINUS, C14.1)
	NET_C(A1_2.OUT, C14.2, R28.1, C13.1)
	NET_C(R28.2, A1_1.MINUS)
	NET_C(A1_1.OUT, R27.1, B1.13)
	NET_C(I_V5.Q, R26.1)
	NET_C(A1_1.PLUS, R26.2, R27.2)
	NET_C(D1.K, R23.2, C13.2, R25.1, MC3340_F1.2)  // to MC3340 ctrl pin

	// The MC3340's output is the complete engine sound, which is sent to
	// the final mix.


	// **** Noise generation and noise-based sound effects: tire skid
	// **** (NOISE_CR_1), boom from crash (BOOM), post-crash noise
	// **** (NOISE_CR_2).

	// The noise generator circuit for Super Speed Race is a
	// linear-feedback shift register on the main motherboard which
	// generates a digital random noise signal. This digital noise gets
	// smoothed and filtered into continuous analog form in the same way
	// as in 280-ZZZAP and Laguna Racer, and the noise-based sound effect
	// circuits are similar.

	CAP(C1, CAP_U(10))

	LOCAL_SOURCE(sspeedr_noisegen)
	INCLUDE(sspeedr_noisegen)

	NET_C(NOISE_OUT, C1.1)

	NET_C(C1.2, R1.1)

	// The noise generator is followed by a single-amplifier active
	// low-pass filter which converts the digital noise waveform into a
	// smoother analog noise waveform.

	RES(R1, RES_K(680))

	LM3900(H2_1)

	CAP(C2, CAP_P(6800))
	RES(R2, RES_K(10))
	RES(R3, RES_K(820))
	RES(R4, RES_K(270))
	CAP(C3, CAP_P(220))

	NET_C(R1.2, C2.1, R2.1, R4.1)
	NET_C(C2.2, GND)
	NET_C(H2_1.MINUS, R2.2, C3.1)
	NET_C(I_V5.Q, R3.1)
	NET_C(H2_1.PLUS, R3.2)
	NET_C(H2_1.OUT, C3.2, R4.2, R5.1, R17.1, C5.1, C4.1)

	// The smoothed analog noise is passed to three different sound-effect
	// circuits, each of which is also an active-filter type.

	// First noise circuit: tire skid (NOISE_CR_1), a two-amplifier active
	// bandpass filter.

	CD4016_DIP(E2)

	LM3900(H2_3)

	RES(R5, RES_K(39))
	RES(R6, RES_R(62))
	RES(R7, RES_K(82))
	RES(R8, RES_K(39))

	CAP(C7, CAP_U(0.1))
	CAP(C8, CAP_U(0.1))
	CAP(C25, CAP_U(0.1))

	NET_C(R5.2, R6.1, C7.1, C8.1, R12.1)
	NET_C(R6.2, GND)
	NET_C(H2_3.MINUS, C7.2, R8.1)
	NET_C(I_V5.Q, R7.1)
	NET_C(H2_3.PLUS, R7.2)
	NET_C(H2_3.OUT, R8.2, C8.2, E2.10)
	NET_C(E2.11, C25.1)
	NET_C(C25.2, R9.1)

	LM3900(J1_2)

	// Unlike the other LM3900s, those on chip J1 get 12-volt power.
	NET_C(I_V12.Q, J1_2.VCC)
	NET_C(GND, J1_2.GND)

	RES(R9, RES_K(39))
	RES(R10, RES_K(240))
	RES(R11, RES_K(120))
	RES(R12, RES_K(62))

	NET_C(J1_2.MINUS, R9.2, R11.1)
	NET_C(I_V5.Q, R10.1)
	NET_C(J1_2.PLUS, R10.2)
	NET_C(J1_2.OUT, R11.2, R12.2, E2.4)
	NET_C(E2.3, R63.1)
	NET_C(E2.5, E2.12, NOISE_CR_1)

	// Second noise circuit: secondary crash noise (NOISE_CR_2), a
	// single-amplifier active low-pass filter.

	RES(R17, RES_K(10))

	CAP(C5, CAP_U(10))
	CAP(C4, CAP_U(0.022))

	LM3900(H2_2)

	RES(R16, RES_K(1.5))
	RES(R15, RES_K(15))
	RES(R14, RES_M(2.7))
	RES(R13, RES_K(150))

	CAP(C6, CAP_U(0.01))
	CAP(C9, CAP_U(0.001))

	NET_C(C4.2, R16.1)
	NET_C(R16.2, C6.1, R15.1, R13.1)
	NET_C(C6.2, GND)
	NET_C(H2_2.MINUS, R15.2, C9.1)
	NET_C(I_V5.Q, R14.1)
	NET_C(H2_2.PLUS, R14.2)
#if REMOVE_POST_CRASH_NOISE_GLITCHES
	// With the static time-stepping used to ensure acceptable performance
	// with MAME, this part of the netlist will generate extra spikes on
	// the op-amp output with voltages outside of the real op-amp's output
	// range. These spikes give the sound an unwanted "grittiness" not in
	// the original, so I've added a voltage-limiting AFUNC to the op-amp
	// output beyond its feedback connection. This gives a smoother sound,
	// closer to the original.

	NET_C(H2_2.OUT, C9.2, R13.2, H2_2_LIM.A0)
	AFUNC(H2_2_LIM, 1, "max(min(A0, 4.5), 0)")
	NET_C(H2_2_LIM.Q, E2.8)
#else
	NET_C(H2_2.OUT, C9.2, R13.2, E2.8)
#endif
	NET_C(E2.9, R64.1)
	NET_C(E2.6, NOISE_CR_2)

	// Third noise circuit: boom from crash (BOOM), a single-amplifier
	// active bandpass filter.

	LM3900(H2_4)

	RES(R18, RES_K(20))

	RES(R19, RES_K(1))
	RES(R20, RES_M(3.3))
	RES(R21, RES_M(1.5))

	CAP(C10, CAP_U(0.1))
	CAP(C11, CAP_U(0.1))

	NET_C(R17.2, C5.2, E2.1)
	NET_C(E2.2, R18.1)
	NET_C(E2.13, BOOM)
	NET_C(R18.2, R19.1, C10.1, C11.1)
	NET_C(R19.2, GND)
	NET_C(H2_4.MINUS, C10.2, R21.1)
	NET_C(I_V5.Q, R20.1)
	NET_C(H2_4.PLUS, R20.2)
	NET_C(H2_4.OUT, R21.2, C11.2, R65.1)


	// **** Final mix of sound effects and sound preamplification.

	RES(R63, RES_K(3))
	RES(R64, RES_K(150))

	RES(R65, RES_K(12))

	RES(R66, RES_K(33))

	CAP(C20, CAP_U(1.0))
	CAP(C21, CAP_U(10))

	NET_C(R63.2, R64.2, R65.2, C20.1)
	NET_C(MC3340_F1.7, R66.1)  // MC3340 output pin
	NET_C(R66.2, C21.1)

	LM3900(J1_4)
	LM3900(J1_1)

	NET_C(I_V12.Q, J1_4.VCC, J1_1.VCC)
	NET_C(GND, J1_4.GND, J1_1.GND)

	RES(R67, RES_K(2))
	CAP(C22, CAP_U(10))
	RES(R68, RES_K(220))
	RES(R74, RES_K(220))

	NET_C(I_V12.Q, R67.1)
	NET_C(R67.2, C22.1, R68.1, R74.1)
	NET_C(C22.2, GND)
	NET_C(J1_4.PLUS, R68.2)
	NET_C(J1_1.PLUS, R74.2)

	RES(R69, RES_K(100))
	POT2(R70, RES_K(50))  // Master volume potentiometer (2 terminals)
	CAP(C23, CAP_U(10))
	CAP(C24, CAP_U(0.1))
	RES(R73, RES_K(100))
	RES(R75, RES_K(100))

	DIODE(D6, 1N914)
	DIODE(D7, 1N914)
	RES(R71, RES_K(10))
	RES(R72, RES_K(10))

	NET_C(SILENCE, D6.A, D7.A)
	NET_C(D6.K, R71.1)
	NET_C(D7.K, R72.1)

	NET_C(R70.2, C23.2)
	NET_C(J1_4.MINUS, C20.2, C21.2, R69.1, R70.1, R71.2)
	NET_C(J1_4.OUT, C23.1, R69.2, C24.1)
	NET_C(C24.2, R73.1)
	NET_C(J1_1.MINUS, R73.2, R75.1, R72.2)
	NET_C(J1_1.OUT, R75.2)

	ALIAS(OUTPUT, J1_1.OUT)

}


NETLIST_START(sspeedr)
{

	SOLVER(Solver, 48000)

	ANALOG_INPUT(I_V12, 12)
	ANALOG_INPUT(I_V5, 5)

	LOCAL_SOURCE(sspeedr_schematics)
	INCLUDE(sspeedr_schematics)

	// The MC3340 gets 5-volt power in Super Speed Race.
	NET_C(I_V5.Q, MC3340_F1.8)
	NET_C(GND, MC3340_F1.3)

	// Logic inputs which represent output pins from 74174 latches at C2
	// and D2
	LOGIC_INPUT(I_F4_2, 0, "74XX")  // BOOM
	LOGIC_INPUT(I_F4_5, 0, "74XX")  // ENGINE SOUND OFF
	LOGIC_INPUT(I_F4_7, 0, "74XX")  // NOISE CR 1
	LOGIC_INPUT(I_F4_10, 0, "74XX")  // NOISE CR 2
	LOGIC_INPUT(I_F4_12, 0, "74XX")  // SILENCE
	LOGIC_INPUT(I_F5_2, 1, "74XX")  // PEDAL_BIT0
	LOGIC_INPUT(I_F5_5, 1, "74XX")  // PEDAL_BIT1
	LOGIC_INPUT(I_F5_7, 1, "74XX")  // PEDAL_BIT2
	LOGIC_INPUT(I_F5_10, 1, "74XX")  // PEDAL_BIT3
	LOGIC_INPUT(I_F5_12, 0, "74XX")  // HI SHIFT
	LOGIC_INPUT(I_F5_15, 1, "74XX")  // LO SHIFT

	ALIAS(I_BOOM, I_F4_2.IN)
	ALIAS(I_ENGINE_SOUND_OFF, I_F4_5.IN)
	ALIAS(I_NOISE_CR_1, I_F4_7.IN)
	ALIAS(I_NOISE_CR_2, I_F4_10.IN)
	ALIAS(I_SILENCE, I_F4_12.IN)
	ALIAS(I_PEDAL_BIT0, I_F5_2.IN)
	ALIAS(I_PEDAL_BIT1, I_F5_5.IN)
	ALIAS(I_PEDAL_BIT2, I_F5_7.IN)
	ALIAS(I_PEDAL_BIT3, I_F5_10.IN)
	ALIAS(I_HI_SHIFT, I_F5_12.IN)
	ALIAS(I_LO_SHIFT, I_F5_15.IN)

	ALIAS(BOOM, I_F4_2.Q)
	ALIAS(ENGINE_SOUND_OFF, I_F4_5.Q)
	ALIAS(NOISE_CR_1, I_F4_7.Q)
	ALIAS(NOISE_CR_2, I_F4_10.Q)
	ALIAS(SILENCE, I_F4_12.Q)
	ALIAS(PEDAL_BIT0, I_F5_2.Q)
	ALIAS(PEDAL_BIT1, I_F5_5.Q)
	ALIAS(PEDAL_BIT2, I_F5_7.Q)
	ALIAS(PEDAL_BIT3, I_F5_10.Q)
	ALIAS(HI_SHIFT, I_F5_12.Q)
	ALIAS(LO_SHIFT, I_F5_15.Q)

	// Power pins for logic inputs:
	NET_C(I_V5.Q,
		  I_F4_2.VCC, I_F4_5.VCC, I_F4_7.VCC, I_F4_10.VCC, I_F4_12.VCC,
		  I_F5_2.VCC, I_F5_5.VCC, I_F5_7.VCC, I_F5_10.VCC, I_F5_12.VCC,
		  I_F5_15.VCC)
	NET_C(GND,
		  I_F4_2.GND, I_F4_5.GND, I_F4_7.GND, I_F4_10.GND, I_F4_12.GND,
		  I_F5_2.GND, I_F5_5.GND, I_F5_7.GND, I_F5_10.GND, I_F5_12.GND,
		  I_F5_15.GND)

	// 5-volt power for most of the LM3900 op-amps.
	NET_C(I_V5.Q,
		H2_1.VCC, H2_3.VCC,
		H2_4.VCC, H2_2.VCC, C1_3.VCC, C1_4.VCC,
		A1_1.VCC, A1_2.VCC, A1_3.VCC, A1_4.VCC,
		C1_1.VCC, C1_2.VCC)
	NET_C(GND,
		H2_1.GND, H2_3.GND,
		H2_4.GND, H2_2.GND, C1_3.GND, C1_4.GND,
		A1_1.GND, A1_2.GND, A1_3.GND, A1_4.GND,
		C1_1.GND, C1_2.GND)

	// 5-volt power for the CD4016 switches.
	NET_C(I_V5.Q, A2.14, B1.14, E2.14)
	NET_C(GND, A2.7, B1.7, E2.7)
	// Switches A2_D and B1_C are unused.
	NET_C(GND, A2.10, A2.11, A2.12, B1.6, B1.8, B1.9)

	// Frontier after output of noise generator.
	OPTIMIZE_FRONTIER(C1.1, RES_M(1), 50)

	// Frontier before skid screech generator.
	OPTIMIZE_FRONTIER(R5.1, RES_K(39), 50)

	// Frontiers after NOISE CR 2 and BOOM generators.
	OPTIMIZE_FRONTIER(R64.1, RES_K(150), 50)
	OPTIMIZE_FRONTIER(R65.1, RES_K(12), 50)

	// Frontier after engine sound generation.
	OPTIMIZE_FRONTIER(R66.1, RES_K(33), 50)

	// Frontiers before MC3340 inputs.
	OPTIMIZE_FRONTIER(C16.1, RES_M(1), 50)
	OPTIMIZE_FRONTIER(C13.1, RES_M(1), 50)

	// Frontiers before engine sound op-amp oscillators.
	OPTIMIZE_FRONTIER(R36.1, RES_K(560), 50)
	OPTIMIZE_FRONTIER(R37.1, RES_K(270), 50)
	OPTIMIZE_FRONTIER(R31.1, RES_K(300), 50)
	OPTIMIZE_FRONTIER(R32.1, RES_K(150), 50)
	OPTIMIZE_FRONTIER(R29.1, RES_K(220), 50)
	OPTIMIZE_FRONTIER(R30.1, RES_K(100), 50)

#if CONVERGENCE_FRONTIERS
	// Frontiers at Schmitt trigger op-amp outputs of engine sound
	// oscillators, to eliminate numeric instability and speed convergence
	// rather than to partition matrices. The resistor values given are
	// the modified ones which compensate for how the frontiers would
	// otherwise change the oscillator waveforms.
	OPTIMIZE_FRONTIER(R40.1, RES_K(275), 50)
	OPTIMIZE_FRONTIER(R35.1, RES_K(281), 50)
	OPTIMIZE_FRONTIER(R27.1, RES_K(286), 50)
#endif

}
