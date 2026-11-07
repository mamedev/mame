// license:CC0-1.0
// copyright-holders:Peter Wilhelmsen
/*
 * Spitfire sound: netlist implementation.
 *
 * The netlist supplies a 12 kHz / 17-stage noise source, filtered noise effects,
 * 80 Hz engine flutter, 25/160 ms shot/impact envelopes, 5/10 ms level smoothing
 * and a 20 Hz output coupling network. AFUNC blocks stand in for the unknown
 * gain-control/mixer stages, not for the MC1358 or a verified transistor circuit.
 * The falling effect crossfades two RC filters over 300 ms; this approximates
 * the former continuously varying single-pole filter.
 *
 * SHOT and IMPACT receive a short charge pulse for each corresponding ROM
 * write. FALL and ENGINE are levels. ENABLE discharges the transient envelopes
 * when a round ends; the driver also clears the level inputs at that time.
 */

#include "netlist/devices/net_lib.h"
#include "nl_spitfire.h"

NETLIST_START(spitfire)
{
	SOLVER(Solver, 48000)
	ANALOG_INPUT(V5, 5)

	TTL_INPUT(SHOT, 0)
	TTL_INPUT(IMPACT, 0)
	TTL_INPUT(FALL, 0)
	TTL_INPUT(ENGINE, 0)
	TTL_INPUT(ENABLE, 0)
	NET_C(V5, SHOT.VCC, IMPACT.VCC, FALL.VCC, ENGINE.VCC, ENABLE.VCC)
	NET_C(GND, SHOT.GND, IMPACT.GND, FALL.GND, ENGINE.GND, ENABLE.GND)

	TTL_7404_INVERT(OFF)
	TTL_7404_INVERT(FALL_OFF)
	NET_C(V5, OFF.VCC, FALL_OFF.VCC)
	NET_C(GND, OFF.GND, FALL_OFF.GND)
	NET_C(ENABLE, OFF.A)
	NET_C(FALL, FALL_OFF.A)

	// Cascade 4 + 4 + 4 + 5 stages. XNOR feedback at stages 12 and 17
	// gives a maximal sequence and starts from the register's all-zero state.
	CLOCK(NOISE_CLOCK, 12000)
	CD4006(NOISE_REG)
	TTL_7486_XOR(NOISE_XOR)
	TTL_7404_INVERT(NOISE_INV)
	NET_C(V5, NOISE_CLOCK.VCC, NOISE_REG.VDD, NOISE_XOR.VCC, NOISE_INV.VCC)
	NET_C(GND, NOISE_CLOCK.GND, NOISE_REG.VSS, NOISE_XOR.GND, NOISE_INV.GND)
	NET_C(NOISE_CLOCK, NOISE_REG.CLOCK)
	NET_C(NOISE_REG.D1P4, NOISE_REG.D3)
	NET_C(NOISE_REG.D3P4, NOISE_REG.D4)
	NET_C(NOISE_REG.D4P4, NOISE_REG.D2, NOISE_XOR.A)
	NET_C(NOISE_REG.D2P5, NOISE_XOR.B)
	NET_C(NOISE_XOR.Q, NOISE_INV.A)
	NET_C(NOISE_INV.Q, NOISE_REG.D1)
	AFUNC(NOISE, 1, "if(A0>2.5,1,-1)")
	NET_C(NOISE_REG.D2P5, NOISE.A0)

	// Shared 600 Hz and 4 kHz filters: impact uses the low-pass output,
	// and gunfire uses the difference between the two filtered signals.
	RES(R_NOISE_LOW, 2650)
	CAP(C_NOISE_LOW, CAP_U(0.1))
	RES(R_NOISE_HIGH, 398)
	CAP(C_NOISE_HIGH, CAP_U(0.1))
	NET_C(NOISE.Q, R_NOISE_LOW.1, R_NOISE_HIGH.1)
	NET_C(R_NOISE_LOW.2, C_NOISE_LOW.1)
	NET_C(R_NOISE_HIGH.2, C_NOISE_HIGH.1)
	NET_C(GND, C_NOISE_LOW.2, C_NOISE_HIGH.2)

	// Charge in approximately 10 us; decay in 25 ms / 160 ms.
	// The 100 us host pulses permit consecutive writes to retrigger either
	// envelope without requiring a zero write in that player's address bank.
	CAP(C_SHOT, CAP_U(1))
	RES(R_SHOT, RES_K(25))
	SYS_DSW(SHOT_CHARGE, SHOT.Q, V5, C_SHOT.1)
	PARAM(SHOT_CHARGE.RON, 10)
	SYS_DSW(SHOT_CLEAR, OFF.Q, GND, C_SHOT.1)
	PARAM(SHOT_CLEAR.RON, 10)
	NET_C(C_SHOT.1, R_SHOT.1)
	NET_C(GND, C_SHOT.2, R_SHOT.2)

	CAP(C_IMPACT, CAP_U(1))
	RES(R_IMPACT, RES_K(160))
	SYS_DSW(IMPACT_CHARGE, IMPACT.Q, V5, C_IMPACT.1)
	PARAM(IMPACT_CHARGE.RON, 10)
	SYS_DSW(IMPACT_CLEAR, OFF.Q, GND, C_IMPACT.1)
	PARAM(IMPACT_CLEAR.RON, 10)
	NET_C(C_IMPACT.1, R_IMPACT.1)
	NET_C(GND, C_IMPACT.2, R_IMPACT.2)

	// 80 Hz, 25% high duty: the extended clock's base half-period is
	// 1/320 s; the low and high intervals last three and one of these.
	EXTCLOCK(FLUTTER, 160, "3,1")
	TTL_7404_INVERT(FLUTTER_INV)
	NET_C(V5, FLUTTER_INV.VCC)
	NET_C(GND, FLUTTER_INV.GND)
	NET_C(FLUTTER, FLUTTER_INV.A)
	AFUNC(ENGINE_CHOP, 2, "A0*if(A1<2,1,0.25)")
	NET_C(NOISE.Q, ENGINE_CHOP.A0)
	NET_C(FLUTTER_INV.Q, ENGINE_CHOP.A1)
	RES(R_ENGINE, 4550)
	CAP(C_ENGINE, CAP_U(0.1))
	NET_C(ENGINE_CHOP.Q, R_ENGINE.1)
	NET_C(R_ENGINE.2, C_ENGINE.1)
	NET_C(GND, C_ENGINE.2)
	AFUNC(ENGINE_GATE, 1, "if(A0>2,1,0)")
	NET_C(ENGINE, ENGINE_GATE.A0)
	RES(R_ENGINE_LEVEL, RES_K(5))
	CAP(C_ENGINE_LEVEL, CAP_U(1))
	NET_C(ENGINE_GATE.Q, R_ENGINE_LEVEL.1)
	NET_C(R_ENGINE_LEVEL.2, C_ENGINE_LEVEL.1)
	NET_C(GND, C_ENGINE_LEVEL.2)

	// 16.667 uA into 1 uF gives a 0..5 V ramp in 300 ms. A diode limits
	// the ramp during long commands; the crossfade itself saturates at 5 V.
	VCCS(FALL_RAMP, 1.0 / 300000.0)
	CAP(C_FALL_RAMP, CAP_U(1))
	RES(R_FALL_RAMP, RES_M(100))
	DIODE(D_FALL_LIMIT, "1N4148")
	NET_C(V5, FALL_RAMP.IP, D_FALL_LIMIT.K)
	NET_C(GND, FALL_RAMP.IN, FALL_RAMP.ON, C_FALL_RAMP.2, R_FALL_RAMP.2)
	NET_C(FALL_RAMP.OP, C_FALL_RAMP.1, R_FALL_RAMP.1, D_FALL_LIMIT.A)
	SYS_DSW(FALL_RESET, FALL_OFF.Q, GND, C_FALL_RAMP.1)
	PARAM(FALL_RESET.RON, 10)
	AFUNC(FALL_BLEND, 1, "min(1,max(0,A0/5))")
	NET_C(C_FALL_RAMP.1, FALL_BLEND.A0)
	RES(R_FALL_HIGH, 884)
	CAP(C_FALL_HIGH, CAP_U(0.1))
	RES(R_FALL_LOW, 3980)
	CAP(C_FALL_LOW, CAP_U(0.1))
	NET_C(NOISE.Q, R_FALL_HIGH.1, R_FALL_LOW.1)
	NET_C(R_FALL_HIGH.2, C_FALL_HIGH.1)
	NET_C(R_FALL_LOW.2, C_FALL_LOW.1)
	NET_C(GND, C_FALL_HIGH.2, C_FALL_LOW.2)
	AFUNC(FALL_GATE, 1, "if(A0>2,1,0)")
	NET_C(FALL, FALL_GATE.A0)
	RES(R_FALL_LEVEL, RES_K(10))
	CAP(C_FALL_LEVEL, CAP_U(1))
	NET_C(FALL_GATE.Q, R_FALL_LEVEL.1)
	NET_C(R_FALL_LEVEL.2, C_FALL_LEVEL.1)
	NET_C(GND, C_FALL_LEVEL.2)

	// Behavioural VCAs: envelopes are 0..5 V, level controls 0..1 V.
	AFUNC(SHOT_VCA, 3, "0.38*(A0-A1)*A2/5")
	NET_C(C_NOISE_HIGH.1, SHOT_VCA.A0)
	NET_C(C_NOISE_LOW.1, SHOT_VCA.A1)
	NET_C(C_SHOT.1, SHOT_VCA.A2)
	AFUNC(IMPACT_VCA, 2, "0.48*A0*A1/5")
	NET_C(C_NOISE_LOW.1, IMPACT_VCA.A0)
	NET_C(C_IMPACT.1, IMPACT_VCA.A1)
	AFUNC(ENGINE_VCA, 2, "0.23*A0*A1")
	NET_C(C_ENGINE.1, ENGINE_VCA.A0)
	NET_C(C_ENGINE_LEVEL.1, ENGINE_VCA.A1)
	AFUNC(FALL_VCA, 4, "0.30*(A0*(1-A2)+A1*A2)*A3")
	NET_C(C_FALL_HIGH.1, FALL_VCA.A0)
	NET_C(C_FALL_LOW.1, FALL_VCA.A1)
	NET_C(FALL_BLEND.Q, FALL_VCA.A2)
	NET_C(C_FALL_LEVEL.1, FALL_VCA.A3)
	AFUNC(MIX, 4, "A0+A1+A2+A3")
	NET_C(SHOT_VCA.Q, MIX.A0)
	NET_C(IMPACT_VCA.Q, MIX.A1)
	NET_C(ENGINE_VCA.Q, MIX.A2)
	NET_C(FALL_VCA.Q, MIX.A3)
	// Finite drive resistance keeps both capacitor terminals in the analog
	// solver; directly coupling a changing AFUNC rail would lose its steps.
	RES(R_OUT_DRIVE, 100)
	CAP(C_OUT, CAP_U(1))
	RES(R_OUT, RES_K(7.96))
	NET_C(MIX.Q, R_OUT_DRIVE.1)
	NET_C(R_OUT_DRIVE.2, C_OUT.1)
	NET_C(C_OUT.2, R_OUT.1)
	NET_C(GND, R_OUT.2)
	ALIAS(OUTPUT, R_OUT.1)
}
