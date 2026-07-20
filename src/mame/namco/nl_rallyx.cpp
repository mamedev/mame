// license:CC0-1.0
// copyright-holders:Ricardo Bittencourt

//
// Netlist for the Rally-X explosion ("BANG") sound.
//
// Reconstructed from the hand-drawn schematic of the Rally-X sound board. The
// chain is a digital pseudo-random noise generator gated by the BANG trigger,
// a single-supply uA741 gain stage, and a Fujitsu MB3730 bridge-tied-load power
// amplifier driving the speaker:
//
//   * 11A CD4006  18-stage shift register wired as an LFSR noise source,
//     clocked by the "4V" video bit (~2 kHz).
//   * 11C CD4070  quad XOR providing the LFSR feedback and reset.
//   * 11D CD4066  quad bilateral switch; the BANG line gates the noise out.
//   * uA741       single-supply op-amp wired as a differentiator: the input
//     coupling cap C5 drives the virtual-ground (inverting) input and R41 is
//     the feedback resistor, so the stage differentiates the gated noise;
//     C6 across R41 tames the high-frequency gain.
//   * MB3730_SIL  14W BTL power amp (opamp_lib); the speaker is bridged across
//     OUT-M (pin 6) and OUT-I (pin 5).
//
// The Namco WSG ("software" music/effects, the AUDIO net on the schematic) is
// mixed into the power-amp input through R28/C28 at the R30/VR1 junction; MAME
// drives it via the I_AUDIO analog input (NETLIST_STREAM_INPUT on cin0).
//
// The audible output is the differential voltage across the bridged speaker,
// taken via a unity VCVS (OUTPUT = OUT-M - OUT-I).
//
// Known approximations: only the speaker load (8 ohm) is assumed; everything
// else, including the +12V supply, is taken from the schematic.
//

#include "netlist/devices/net_lib.h"
#include "nl_rallyx.h"


NETLIST_START(rallyx)
{
	SOLVER(Solver, 48000)

	ANALOG_INPUT(I_V5, 5)               // +5V logic rail

	CLOCK(CLK4V, 2000)                  // "4V" ~ 2 kHz (6.144MHz/384/2/4)
	LOGIC_INPUT(I_BANG, 0, "CD4XXX")    // explosion trigger (mainlatch bit 0)

	CD4006_DIP(SR)                      // 11A
	CD4070_DIP(XOR)                     // 11C
	CD4066_DIP(SW)                      // 11D
	RES(R26, RES_K(10))
	DIODE(D2, "1N914")
	DIODE(D1, "1S1588")
	RES(R24, RES_K(470))
	CAP(C36, CAP_U(0.1))
	CAP(C35, CAP_U(1))
	RES(R27, RES_K(150))
	RES(R31, RES_K(22))

	// Power (pin 14 = VDD, pin 7 = VSS on all three CD4xxx)
	NET_C(I_V5, SR.14, XOR.14, SW.14)
	NET_C(GND, SR.7, XOR.7, SW.7)

	// Power for the clock and the CD4XXX-family logic input
	NET_C(I_V5, CLK4V.VCC)
	NET_C(GND, CLK4V.GND)
	NET_C(I_V5, I_BANG.VDD)
	NET_C(GND, I_BANG.VSS)

	// 4V clock into the shift register
	NET_C(CLK4V.Q, SR.3)

	// Tie off unused gate/switch terminals (4070 gates B/C and spare 4066 sws)
	NET_C(GND, XOR.5, XOR.6, XOR.8, XOR.9, C35.2)
	NET_C(GND, SW.3, SW.4, SW.5, SW.8, SW.9, SW.6)

	NET_C(SR.1, SR.12, SW.13, XOR.2)
	NET_C(I_V5, XOR.13, SW.11)          // gate D in B = VCC (gate D = NOT nodeA)
	NET_C(SR.8, XOR.1)
	NET_C(XOR.11, D2.A)
	NET_C(D2.K, SR.4, R26.1)
	NET_C(R26.2, XOR.3)

	NET_C(SR.13, SR.5)                  // D1+4 -> D3
	NET_C(SR.10, SR.6)                  // D3+4 -> D4

	// D1/R24/C36 + gate D reset: when the LFSR reaches the all-zero state this
	// forces a 1 back into the shift register, so the noise can't get stuck.
	NET_C(SR.9, D1.A, R24.1)
	NET_C(D1.K, R24.2, C36.1, XOR.12)
	NET_C(C36.2, GND)

	// output path: +5V -> sw4 -> sw1 -> R27 -> R31 -> GND
	NET_C(SW.10, SW.1, C35.1)
	NET_C(SW.2, R27.1)
	NET_C(R27.2, R31.1)
	NET_C(R31.2, GND)

	// sw4 control = BANG
	NET_C(I_BANG.Q, SW.12)

	//
	// uA741 single-supply differentiator (+12V): C5 input cap into the virtual
	// ground, R41 feedback resistor (C6 across R41 limits the HF gain)
	//
	ANALOG_INPUT(I_V12, 12)             // +12V audio rail (schematic)

	UA741_DIP8(U741)
	RES(R41, RES_K(470))                // feedback resistor
	CAP(C6, CAP_N(10))                  // feedback cap (|| R41)
	CAP(C5, CAP_N(10))                  // input-coupling cap
	RES(R39, RES_K(68))                 // +12V -> pin3 bias
	RES(R40, RES_K(47))                 // pin3 bias -> GND
	CAP(C29, CAP_U(22))                 // pin3 bias decoupling
	CAP(C7, CAP_U(0.1))                 // +12V supply decoupling
	DIODE(D3, "1N914")                  // bias clamp across R39
	RES(R30, RES_K(4.7))                // output series R
	RES(VR1, RES_K(1))                  // volume pot (max) / downstream load

	// Namco WSG ("software" audio) injected by MAME and mixed at the R30/VR1
	// junction. I_AUDIO is an ideal source driven each sample by the WSG stream.
	ANALOG_INPUT(I_AUDIO, 0)            // WSG output (NETLIST_STREAM_INPUT cin0)
	RES(R28, RES_K(10))                 // WSG -> R30/VR1 junction mixing resistor
	CAP(C28, CAP_N(47))                 // 0.047uF junction filter (to GND)

	NET_C(I_V12, U741.7, C7.1)          // pin7 = +12V
	NET_C(GND, U741.4, C7.2)            // pin4 = GND

	NET_C(R27.2, C5.1, C6.1)
	NET_C(C5.2, U741.2, R41.1)          // pin2 (inverting)
	NET_C(U741.6, R41.2, C6.2, R30.1)   // pin6 (output) -> R41 || C6 feedback

	NET_C(I_V12, R39.1, D3.K)
	NET_C(R39.2, D3.A, R40.1, C29.1, U741.3)   // bias node -> pin3
	NET_C(R40.2, GND)
	NET_C(C29.2, GND)

	NET_C(R30.2, VR1.1)
	NET_C(VR1.2, GND)

	// Mix the WSG audio into the R30/VR1 junction (R28 series, C28 to GND)
	NET_C(I_AUDIO, R28.1)
	NET_C(R28.2, R30.2, C28.1)
	NET_C(C28.2, GND)

	//
	// MB3730 BTL power amp (opamp_lib) + bridged speaker
	//
	MB3730_SIL(U3730)
	PARAM(U3730.GAIN, 29)               // ~35dB bridged (vs datasheet 281/~55dB):
	                                    // lowers the explosion noise floor; the
	                                    // injected WSG audio is boosted to suit.

	CAP(C4, CAP_U(0.1))                 // pin1 input coupling
	CAP(C26, CAP_U(220))                // pin2 FB cap (AC gain corner)
	CAP(C27, CAP_U(22))                 // pin3 BYPASS decoupling
	RES(R70, 1.0)                       // Zobel on OUT M
	CAP(C31, CAP_U(0.1))
	RES(R69, 1.0)                       // Zobel on OUT-I
	CAP(C32, CAP_U(0.1))
	RES(SPK, 8.0)                       // speaker, bridged across outputs [assumed 8 ohm]

	NET_C(I_V12, U3730.7)               // pin7 Vcc
	NET_C(GND, U3730.4)                 // pin4 GND

	NET_C(R30.2, C4.1)
	NET_C(C4.2, U3730.1)                // pin1 IN

	NET_C(U3730.2, C26.1)               // pin2 FB
	NET_C(C26.2, GND)

	NET_C(U3730.3, C27.1)               // pin3 BYPASS
	NET_C(C27.2, GND)

	NET_C(U3730.6, R70.1, SPK.1)        // pin6 OUT M
	NET_C(R70.2, C31.1)
	NET_C(C31.2, GND)
	NET_C(U3730.5, R69.1, SPK.2)        // pin5 OUT-I
	NET_C(R69.2, C32.1)
	NET_C(C32.2, GND)

	//
	// Audible output = differential across the bridged speaker (OUT M - OUT-I)
	//
	VCVS(VDIFF, 1)
	NET_C(VDIFF.IP, SPK.1)
	NET_C(VDIFF.IN, SPK.2)
	NET_C(VDIFF.ON, GND)
	ALIAS(OUTPUT, VDIFF.OP)
}
