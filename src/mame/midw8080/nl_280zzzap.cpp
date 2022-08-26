// license:CC0
// copyright-holders:Colin Douglas Howell

#include "netlist/devices/net_lib.h"


// This is a netlist description for the sound circuits of Midway's 280-ZZZAP
// and Laguna Racer, based on Midway's schematics "280 ZZZAP Game Logic P.C.
// 0610-00907A", "Laguna Racer Game Logic A084-90700-A622", and "Laguna Racer
// Game Logic A084-90700-B622". Midway PCB drawings for Laguna Racer and a
// photo by Andrew Welburn of a 280-ZZZAP game logic board were also used to
// help determine correct values for certain components. The netlist should
// also apply to Taito/Midway's Super Speed Race with only modest changes,
// because that game uses very similar sound circuits to 280-ZZZAP and Laguna
// Racer.

// (Incidentally, the 280-ZZZAP schematic has some notes on changes made to
// the circuitry during production, a few of which were to audio circuits.
// This netlist uses the final state of the schematic, but it could be
// adjusted to the earlier state by modifying a couple of component values.)

// Due to its complexity, this netlist is rather CPU intensive. Various
// speedups have been employed to allow the game to run at full speed on
// modern hardware with some CPU to spare. The main sources of complexity are
// the op-amps (15 of them, all of which are National Semiconductor LM3900
// current-differencing "Norton-type" op-amps) and the Motorola MC3340
// Electronic Attenuator, the latter of which is emulated at its internal
// component level, which includes 10 bipolar junction transistors. Several of
// the circuits are oscillators which run at frequencies up to hundreds of
// hertz and which switch state abruptly, increasing the computing demands of
// the emulation.

// Midway's 280-ZZZAP and Laguna Racer schematics don't label discrete
// components, though they do label ICs with their PCB coordinates, as well as
// IC pins. Midway's Super Speed Race schematic, however, does label discrete
// components, most of which directly correspond to the 280-ZZZAP and Laguna
// Racer ones. I have used the Super Speed Race labels for the 280-ZZZAP
// discrete components. IC components I have labeled with their 280-ZZZAP PCB
// coordinates, consistent with the 280-ZZZAP schematic. Each LM3900 IC
// contains 4 op-amps, so I have labeled these op-amps as _1 to _4, the same
// scheme used in the LM3900 datasheet, using the pin numbers on the schematic
// to identify the specific op-amp. Likewise, each CD4016 IC contains 4
// switches, so I identify a specific switch from its pin numbers on the
// schematic; these are labeled _A through _D in the CD4016 datasheet, so I
// use that scheme.

// The sound circuitry can be divided into three sections: engine sound
// generation and control, generation of noise-based sound effects (skidding
// and crashes), and the final mix and amplification of all sounds.


// When comparing the sound circuitry and components between 280-ZZZAP and
// Laguna Racer, here are some things to note. There are two different
// versions of the Laguna Racer game logic board (where the sound circuits
// reside): A622 and B622. Midway's A622 schematic was clearly copied directly
// from 280-ZZZAP's--even the letter indicators for change notes in
// 280-ZZZAP's circuitry remain, though the actual notes were blanked out.
// B622's schematics show sound circuitry differences from 280-ZZZAP and A622,
// and these differences are consistent with Midway's PCB drawings for *both*
// the A622 and B622 games boards. Some of them are also consistent with
// Welburn's 280-ZZZAP PCB photo. So it seems that actual 280-ZZZAP boards
// lie somewhere between the 280-ZZZAP schematic and the B622 Laguna Racer
// schematic.


// Include special frontiers within engine sound oscillators whose purpose is
// to improve solution convergence by reducing the oscillators' inherent
// numeric instability. Also adjusts values of resistors associated with these
// oscillators to maintain their frequency and voltage levels.

#define CONVERGENCE_FRONTIERS   1

// Use the fast high-level emulation of the analog noise generator, based on
// passing the noise signal to a simple AFUNC that outputs a high or low level
// according to the sign of the noise. This sounds the same as an accurate run
// of the component-level noise generator, but it is fast enough to run in
// real time.

#define FAST_HLE_NOISE_GEN  1

// Enable a voltage limiter on the output of the op-amp which generates the
// post-crash sound. This removes voltage glitches from the output which would
// otherwise be audible when the netlist is run with a fixed 48 kHz timestep,
// as it is in MAME.

#define REMOVE_POST_CRASH_NOISE_GLITCHES  1


// Netlists for two different games are defined in this file. These netlists
// are mostly alike but differ in a few key components. I use Aaron Giles'
// scheme for combining the two definitions in one file.
//
// Initial compilation includes this section.
//

#ifndef SOUND_VARIANT


//
// Now include ourselves twice, once for 280-ZZZAP and once for Laguna Racer.
//

#define VARIANT_280ZZZAP  0
#define VARIANT_LAGUNAR   1

#define SOUND_VARIANT       (VARIANT_280ZZZAP)
#include "nl_280zzzap.cpp"

#undef SOUND_VARIANT
#define SOUND_VARIANT       (VARIANT_LAGUNAR)
#include "nl_280zzzap.cpp"


#else


//
// Main netlist
//

#if (SOUND_VARIANT == VARIANT_280ZZZAP)

static NETLIST_START(280zzzap_schematics)
{

#else // (SOUND_VARIANT == VARIANT_LAGUNAR)

static NETLIST_START(lagunar_schematics)
{

#endif

	// **** Conversion of accelerator level to "engine speed" control
	// **** voltage for engine sound, with capacitor-based "engine
	// **** inertia" and gear shift changes.

	// The accelerator pedal input gives a 4-bit digital value which the
	// microprocessor stores in 74174 latch F5. The 4-bit value from the
	// latch output is converted by a resistor network to an analog
	// current of about 5.5 to 43 microamps, depending on the accelerator
	// level, with 5.5 µA being "zero" and each additional accelerator
	// increment adding another 2.5 µA. This current is then converted to
	// a corresponding analog voltage by the first LM3900 op-amp H5_3 at a
	// ratio of about 0.25 volts per accelerator increment. This op-amp's
	// output charges or discharges capacitor C18, whose voltage provides
	// the reference input for the second op-amp H5_4. The charging and
	// discharging of C18 is how the engine sound's pitch and volume are
	// changed, so more accelerator will charge it faster, and less
	// accelerator will charge it slower or make it discharge faster.
	// Also, it can discharge faster than it can charge (RC time constants
	// of about 0.5 seconds vs. 2.2 seconds), so the engine will lose
	// speed faster than it will gain it. Normally ENGINE_SOUND_OFF is
	// low, but if it goes high (if you crash, or when the game is in
	// attract mode), it closes CD4016 switch G5_A, which almost instantly
	// discharges C18 through 270-ohm resistor R56 and cuts off the engine
	// sound completely.
	//
	// Op-amp H5_4's output, which is driven to match the input current
	// from capacitor C18, provides the actual speed voltage signal to the
	// engine sound oscillators. This voltage is shifted down in level
	// from H5_3, ranging from 0.7 to 4.1 V. It also accounts for gear
	// shift, via capacitor C19 and the two adjacent CD4016 switches G5_B
	// and G5_C. When in low gear, LO_SHIFT is active, switch G5_C is
	// closed, and C19 is discharged via 10K resistor R60. HI_SHIFT is
	// inactive, switch G5_B is open, and all the feedback current to H5_4
	// runs through its 560K feedback resistor R58. However, when in high
	// gear, the state of the two switches is reversed, and C19 is free to
	// slowly charge through 560K resistor R59 while supplying additional
	// feedback current to the op-amp without a resistor. This has the
	// effect of temporarily depressing (or slowing the growth of) H5_4's
	// output voltage while C19 is being charged until it eventually
	// reaches equilibrium, at which point the output voltage will be the
	// same as in the low-gear case. The effect is to slow down the engine
	// speed, as you would expect in a higher gear. Likewise, if the
	// accelerator is reduced in high gear, lowering the op-amp's input
	// current, the fall in the op-amp's output voltage is temporarily
	// delayed while C19 is charged until it has sufficiently discharged
	// through R59. Thus the rate of engine slowing is also lowered.

	// Some component values here were changed in the later version of the
	// Laguna Racer game logic board or in Super Speed Race.

	CD4016_DIP(G5)

	LM3900(H5_3)
	LM3900(H5_4)

	RES(R42, RES_K(1))
	RES(R43, RES_K(1))
	RES(R44, RES_K(1))
	RES(R45, RES_K(1))
	RES(R62, RES_K(1))
	RES(R61, RES_K(1))

	// For the following resistors R51 and R46-R49, the values given are
	// those shown in the Laguna Racer PCB drawings, the Laguna Racer B622
	// board schematic, the Super Speed Race schematic, and Andrew
	// Welburn's 280-ZZZAP PCB photo. In the 280-ZZZAP schematic (and the
	// Laguna Racer A622 board schematic, a near copy), R51 and R48 are
	// 480 Kohms and R46 is 2 Mohms, but these seem to be errors in that
	// schematic. 480 Kohm is not even a standard resistor value for any
	// tolerance band, so it could not have been used on production
	// boards.
	RES(R51, RES_K(470))
	RES(R46, RES_M(2.2))
	RES(R47, RES_M(1))
	RES(R48, RES_K(470))
	RES(R49, RES_K(240))

	RES(R50, RES_K(100))
	// Super Speed Race also has a 1 uF capacitor in parallel with R50.
	RES(R53, RES_K(47))
	RES(R52, RES_K(10))
	RES(R56, RES_R(270))

	RES(R54, RES_K(560))
	RES(R55, RES_M(4.7))

	RES(R57, RES_M(2.7))
	RES(R58, RES_K(560))

	RES(R59, RES_K(560))
	RES(R60, RES_K(10))

	// The size of capacitor C18 varies with the game: 47 uF in 280-ZZZAP,
	// 22 uF in Laguna Racer and Super Speed Race:

#if (SOUND_VARIANT == VARIANT_280ZZZAP)

	CAP(C18, CAP_U(47))

#else // (SOUND_VARIANT == VARIANT_LAGUNAR)

	CAP(C18, CAP_U(22))

#endif

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
	NET_C(H5_3.PLUS, R46.2, R47.2, R48.2, R49.2, R51.2)
	NET_C(H5_3.MINUS, R50.2)
	NET_C(H5_3.OUT, R50.1, R53.1, D4.K)
	// Super Speed Race also has a 1 uF capacitor in parallel with R50.
	NET_C(R53.2, R52.1, C18.1, R56.2, R54.1)
	NET_C(R52.2, D4.A)
	NET_C(H5_4.PLUS, R54.2, R55.2)
	NET_C(H5_4.MINUS, R58.2, R57.2, G5.4)
	// The following are only local connections to H5_4.OUT. It also runs
	// to all the oscillators; those connections are listed further down.
	NET_C(H5_4.OUT, D5.A, R59.1)
	NET_C(D5.K, R58.1)
	NET_C(C19.1, R59.2, R60.2)

	NET_C(G5.1, GND)
	NET_C(G5.2, R56.1)
	NET_C(G5.13, ENGINE_SOUND_OFF)

	NET_C(G5.5, R62.2, HI_SHIFT)

	NET_C(G5.3, G5.8, C19.2)
	NET_C(G5.9, R60.1)
	NET_C(G5.6, R61.2, LO_SHIFT)


	// **** Engine sound generation, using three triangle-wave oscillators
	// **** running at frequency and amplitude determined by engine speed
	// **** control voltage and modulated by MC3340 amplifier/attenuator.

	// Engine sound oscillators. There are three of these, at different
	// frequencies; all three are triangle-wave oscillators whose
	// frequency and amplitude are controlled by the "engine speed"
	// voltage signal output from LM3900 op-amp H5_4. Each oscillator is a
	// cascaded pair of LM3900s with a CD4016 switch. The switch feeds
	// into the reference input of the first op-amp, whose output is also
	// the final oscillator output; the second op-amp acts as a Schmitt
	// trigger on the first op-amp's output, generating a square wave that
	// controls the switch and determines whether the triangle wave is
	// rising or falling.

	// The two lower-frequency oscillators are summed together. One has
	// four times the frequency of the other, producing a "ragged"
	// triangle wave. The combined signal is fed into the signal input of
	// the MC3340 voltage-controlled amplifier/attenuator. The third,
	// highest-frequency oscillator drives the MC3340's control input,
	// amplitude-modulating the signal input from the first two
	// oscillators. Greater voltages on this control signal produce
	// greater attenuation in a logarithmic fashion. The final output
	// signal from the MC3340 thus has a waveform of narrow needle-like
	// spikes which grow taller and shorter according to the original
	// input waveform.

	// The Schmitt triggers in these oscillator circuits are numerically
	// unstable at the switching points, which makes numeric convergence
	// difficult. The standard way to guarantee convergence with such a
	// circuit is to use dynamic timestepping with very short minimum
	// timesteps. In this case that solution is unacceptably slow, so an
	// alternate solution is used instead. To remove the unstable behavior
	// of the Schmitt triggers in the solver, a "frontier" is inserted at
	// the output of each Schmitt trigger op-amp. With this, a change in
	// the op-amp's output level, caused by a change in its inputs, will
	// not affect either its feedback input or the CD4016 switch during
	// the same timestep. Instead, it will only take effect when computing
	// the following timestep. This greatly improves the stability and
	// performance of the solver while only slightly altering the
	// frequency and voltage levels of the oscillator's triangle-wave
	// output. These waveform changes are compensated for by slightly
	// adjusting the value of the resistors which determine the Schmitt
	// trigger's switch points, bringing the waveform very close to what
	// it would be if the standard approach of dynamic timestepping were
	// used instead.

	// These are the remaining connections from H5_4.OUT:
	NET_C(H5_4.OUT, R36.1, R37.1, R31.1, R32.1, R29.1, R30.1)

	MC3340_DIP(MC3340_H2)

	CD4016_DIP(J4)

	// First oscillator (bottommost in schematics).
	// Schematic shows frequency of "76 Hz", but when modeled without a
	// frontier, the maximum in low gear is 87 Hz.
	// With a frontier, uncompensated, this decreases to 86 Hz.

	LM3900(J5_2)
	LM3900(J5_1)

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

	// Note the connections to this oscillator's CD4016 switch at J4 have
	// mislabeled pin numbers on the schematics for 280-ZZZAP and both
	// Laguna Racer versions; they are shown as pins 3, 4, 5, which are
	// for switch B, the same switch which is used by the middle
	// oscillator. The Super Speed Race schematic correctly shows this
	// oscillator using pins 10, 11, 12, which are for switch D of the
	// CD4016 (located at B1 in that game). It seems very unlikely that
	// the earlier games had two oscillators sharing the same switch; that
	// shouldn't work at all. I assume that this was a schematic error
	// which was not corrected until much later.

	NET_C(R37.2, J4.10)
	NET_C(J4.11, J5_2.PLUS)
	NET_C(R36.2, J5_2.MINUS, C17.1)
	NET_C(J5_2.OUT, C17.2, R38.1, D3.A)
	NET_C(R38.2, J5_1.MINUS)
	NET_C(J5_1.OUT, R40.1, J4.12)
	NET_C(I_V5.Q, R39.1)
	NET_C(J5_1.PLUS, R39.2, R40.2)
	NET_C(R41.1, D3.K, D2.K, C16.1)

	// Second oscillator (middle in schematics).
	// Schematic shows frequency of "315 Hz", but when modeled without a
	// frontier, the maximum in low gear is 343 Hz.
	// With a frontier, uncompensated, this decreases to 329 Hz.

	LM3900(J3_3)
	LM3900(J3_4)

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

	NET_C(R32.2, J4.4)
	NET_C(J4.3, J3_3.PLUS)
	NET_C(R31.2, J3_3.MINUS, C15.1)
	NET_C(J3_3.OUT, C15.2, R33.1, D2.A)
	NET_C(R33.2, J3_4.MINUS)
	NET_C(J3_4.OUT, R35.1, J4.5)
	NET_C(I_V5.Q, R34.1)
	NET_C(J3_4.PLUS, R34.2, R35.2)
	NET_C(C16.2, MC3340_H2.1)  // to MC3340 input pin

	// Third oscillator (topmost in schematics).

	// This is the amplitude-modulation oscillator, whose frequency
	// differs slightly between 280-ZZZAP and the later games, Laguna
	// Racer and Super Speed Race. This difference distinctly changes the
	// tone of the engine sound.
	// The 280-ZZZAP and Laguna Racer schematics all show a frequency of
	// "428 Hz", but for 280-ZZZAP, when equipped with a 110 Kohm resistor
	// and modeled without a frontier, the maximum in low gear is 465 Hz;
	// with a frontier, uncompensated, this decreases to 442 Hz.
	// For Laguna Racer, when equipped with a 100 Kohm resistor and
	// modeled without a frontier, the maximum low gear frequency is 511
	// Hz; with a frontier, uncompensated, it's 482 Hz.

	LM3900(J3_2)
	LM3900(J3_1)

	RES(R29, RES_K(220))

	// The value of resistor R30 is what changes the engine sound between
	// 280-ZZZAP and the later games. The 280-ZZZAP schematic indicates
	// that it is 110 Kohm in that game. This resistor is hidden on Andrew
	// Welburn's 280-ZZZAP PCB photo, but the sound resulting from 110
	// Kohm is more consistent with actual 280-ZZZAP machines than that
	// from 100 Kohm.
	// The resistor is 100 Kohm in the B622 Laguna Racer and Super Speed
	// Race schematics and on the PCB drawings for both Laguna Racer board
	// versions, so I assume all Laguna Racer versions had this value.
#if (SOUND_VARIANT == VARIANT_280ZZZAP)

	RES(R30, RES_K(110))

#else // (SOUND_VARIANT == VARIANT_LAGUNAR)

	RES(R30, RES_K(100))

#endif

	RES(R28, RES_K(100))

#if CONVERGENCE_FRONTIERS
	// Schmitt trigger resistors changed to compensate for waveform changes
	// from use of frontier.
	// Since the different games have different oscillator frequencies,
	// the compensated values of these resistors differ slightly as well.
#if (SOUND_VARIANT == VARIANT_280ZZZAP)
	RES(R26, RES_K(455))
	RES(R27, RES_K(284))
#else // (SOUND_VARIANT == VARIANT_LAGUNAR)
	RES(R26, RES_K(453))
	RES(R27, RES_K(286))
#endif
#else
	RES(R26, RES_K(470))
	RES(R27, RES_K(270))
#endif

	CAP(C14, CAP_U(0.01))

	RES(R23, RES_K(10))
	NET_C(R23.1, I_V12.Q)

	RES(R25, RES_K(3.3))
	NET_C(R25.2, GND)

	CAP(C13, CAP_U(10))

	RES(R22, RES_R(470))

	DIODE(D1, 1N914)

	NET_C(I_V5.Q, R22.1)
	NET_C(ENGINE_SOUND_OFF, R22.2, D1.A)

	NET_C(R30.2, J4.1)
	NET_C(J4.2, J3_2.PLUS)
	NET_C(R29.2, J3_2.MINUS, C14.1)
	NET_C(J3_2.OUT, C14.2, R28.1, C13.1)
	NET_C(R28.2, J3_1.MINUS)
	NET_C(J3_1.OUT, R27.1, J4.13)
	NET_C(I_V5.Q, R26.1)
	NET_C(J3_1.PLUS, R26.2, R27.2)
	NET_C(D1.K, R23.2, C13.2, R25.1, MC3340_H2.2)  // to MC3340 ctrl pin

	// The MC3340's output is the complete engine sound, which is sent to
	// the final mix.


	// **** Noise generation and noise-based sound effects: tire skid
	// **** (NOISE_CR_1), boom from crash (BOOM), post-crash noise
	// **** (NOISE_CR_2).

	// The noise generator circuit for 280-ZZZAP and Laguna Racer is based
	// on a reverse-biased 9.1-volt 1N5239 zener diode which generates a
	// noisy current for an LM3900 Norton op-amp input. This op-amp then
	// amplifies the noise so strongly that its output saturates, randomly
	// oscillating between the maximum and mininum values, producing a
	// quasi-digital random waveform which is then smoothed and filtered
	// by another op-amp to form a continuous analog noise signal.
	// Following circuits re-filter and reshape this noise for the
	// noise-based sound effects.
	//
	// The noise of zener diodes is not well controlled during their
	// manufacture, and it can vary widely in strength from one part to
	// another even within a single production lot. However, this
	// op-amp-based noise generator takes zener noise of unpredictable
	// strength and converts it into an analog noise stream whose strength
	// is predictable, determined by the response of the op-amp circuits.
	//
	// Super Speed Race generates its noise in a different fashion, using
	// a linear-feedback shift register on the main motherboard to
	// generate a true digital random noise signal. However, once this
	// digital noise has been generated, it gets smoothed and filtered
	// into continuous analog form in the same way and is further
	// processed through similar sound effect circuits.
	//
	// Since the noise generator discrete components are unlabeled on the
	// 280-ZZZAP and Laguna Racer schematics and are not present in Super
	// Speed Race, I've chosen my own labels for them.
	//
	// The netlist library supports zener diodes, but not zener noise. The
	// noise is simulated using an additional source of noise voltage.
	// This noise is handled in one of two ways, depending on how the
	// netlist is configured.
	//
	// If the netlist is configured for detailed circuit simulation, the
	// noise source is inserted in series between the zener diode's output
	// and the op-amp's non-inverting input. This netlist configuration
	// tries to accurately reproduce the amplified noise signal which the
	// op-amp would generate. Because of the rapidly changing noise signal
	// and the op-amp's high gain, however, producing an accurate,
	// glitch-free signal requires using dynamic time-stepping with a very
	// short minimum time step on the order of 10 nanoseconds. This has an
	// unacceptable impact on performance and is unsuitable for real-time
	// operation, like in a normal MAME session.
	//
	// For such operation, a simpler high-level emulation configuration is
	// used. Because of the very high gain of the op-amp, the real output
	// signal will always lie at either the op-amp's maximum or minimum
	// output value, almost never being in between. So we take the same
	// input noise signal and replace the op-amp with a simple AFUNC()
	// module which only checks the input signal's sign and outputs the
	// corresponding maximum or minimum value. This version runs much
	// faster, not requiring dynamic time-stepping at all for a
	// glitch-free result, and its output is very similar to running the
	// detailed emulation with a short minimum timestep. Once the output
	// has been smoothed and filtered, the two configurations will be
	// effectively identical, and there will be no difference in the
	// resulting sound effects except for how fast they are computed.

	// 24 kHz noise clock for the noise source, chosen to retain noise
	// frequencies as high as possible for 48 kHz sample rate.
	CLOCK(NCLK, 24000)
	NET_C(I_V5.Q, NCLK.VCC)
	NET_C(GND, NCLK.GND)

	// Normally-distributed noise of 10 millivolts RMS voltage. With the
	// zener passing about 25 microamps of current, the real noise may be
	// even stronger than this, but this is strong enough to cause the
	// op-amp to saturate its output, which this noise generator is
	// designed to do.
	// (If the simplified noise generator netlist is being used, the noise
	// signal is re-centered on zero volts, and its amplitude no longer
	// matters, only its sign.)
	SYS_NOISE_MT_N(NOISE, 0.01)

	NET_C(NCLK.Q, NOISE.I)

	// Both the quasi-digital noise signal produced by the 280-ZZZAP and
	// Laguna Racer noise generators and the digital noise signal produced
	// by the Super Speed Race noise generator enter at the upstream end
	// of capacitor C1.

	CAP(C1, CAP_U(10))

#if FAST_HLE_NOISE_GEN

	// Simplified high-level emulation of the noise generator: oscillate
	// between full on and full off according to the sign of the noise
	// input.

	NET_C(NOISE.1, A_NOISE.A0)
	NET_C(NOISE.2, GND)
	AFUNC(A_NOISE, 1, "if(A0 > 0, 4.5, 0.03)")

	NET_C(A_NOISE.Q, C1.1)

#else

	// Simple model of a 1N5239 9.1-volt Zener diode. The 1N5239 is
	// specified to conduct 20 mA of current at its nominal breakdown
	// voltage of 9.1 V. The model produces an exponential I-V curve,
	// passing through this point, which has the same general shape as
	// that of a normal forward-biased diode. NBV is an exponent scale
	// factor; its value here of 1 gives the curve a steep rise and a
	// relatively sharp knee. Actual breakdown I-V curves have an even
	// steeper rise and sharper knee, too steep and sharp to be
	// represented by an exponential, but this model is good enough for
	// this emulation, since the diode operates very close to a single
	// point on the curve.
	ZDIODE(ZD_1N5239, "D(BV=9.1 IBV=0.020 NBV=1)")

	RES(RNOISE0, RES_K(100))
	CAP(CNOISE0, CAP_U(10))
	NET_C(CNOISE0.2, GND)

	NET_C(I_V12.Q, RNOISE0.1)
	NET_C(RNOISE0.2, CNOISE0.1, ZD_1N5239.K)

	LM3900(H4_2)

	NET_C(I_V5.Q, H4_2.VCC)
	NET_C(GND, H4_2.GND)

	RES(RNOISE1, RES_K(56))
	RES(RNOISE2, RES_K(47))
	RES(RNOISE3, RES_K(1))

	CAP(CNOISE, CAP_U(10))

	NET_C(CNOISE.1, RNOISE1.1, RNOISE2.1)
	NET_C(CNOISE.2, GND)
	NET_C(H4_2.MINUS, RNOISE1.2)
	NET_C(ZD_1N5239.A, NOISE.1)
	NET_C(H4_2.PLUS, NOISE.2)
	NET_C(H4_2.OUT, RNOISE2.2, RNOISE3.1, C1.1)
	NET_C(RNOISE3.2, GND)

#endif

	NET_C(C1.2, R1.1)

	// The noise generator is followed by a single-amplifier active
	// low-pass filter with a corner frequency of about 6.3 kHz, a very
	// broad Q of 0.014, and a gain of about 0.8. This filter attenuates
	// the very highest noise frequencies, converting the initial
	// quasi-digital noise waveform into a smoother analog noise waveform
	// that is still of pretty high frequency.

	RES(R1, RES_K(330))  // 680 Kohm in Super Speed Race

	LM3900(H4_1)

	CAP(C2, CAP_P(6800))
	RES(R2, RES_K(10))
	RES(R3, RES_K(820))
	RES(R4, RES_K(270))
	CAP(C3, CAP_P(220))

	NET_C(R1.2, C2.1, R2.1, R4.1)
	NET_C(C2.2, GND)
	NET_C(H4_1.MINUS, R2.2, C3.1)
	NET_C(I_V5.Q, R3.1)
	NET_C(H4_1.PLUS, R3.2)
	NET_C(H4_1.OUT, C3.2, R4.2, R5.1, R17.1, C5.1, C4.1)

	// The smoothed analog noise is passed to three different sound-effect
	// circuits, each of which is also an active-filter type.

	// First noise circuit: tire skid (NOISE_CR_1)

	// This is a two-amplifier active bandpass filter with a center
	// frequency of about 1 kHz and a high Q value of 25, giving a
	// narrow bandwidth of 40 Hz. The gain is about 15.

	// The result is a high-pitched "tire skid" screeching sound.

	// The circuit appears to be taken practically verbatim from page 19 of
	// National Semiconductor's Application Note 72 about the LM3900.

	CD4016_DIP(G4)

	LM3900(H4_3)

	RES(R5, RES_K(39))
	RES(R6, RES_R(62))
	RES(R7, RES_K(82))
	RES(R8, RES_K(39))

	CAP(C7, CAP_U(0.1))
	CAP(C8, CAP_U(0.1))

	NET_C(R5.2, R6.1, C7.1, C8.1, R12.1)
	NET_C(R6.2, GND)
	NET_C(H4_3.MINUS, C7.2, R8.1)
	NET_C(I_V5.Q, R7.1)
	NET_C(H4_3.PLUS, R7.2)
	NET_C(H4_3.OUT, R8.2, C8.2, R9.1)

	// Super Speed Race has an extra CD4016 switch (controlled by the same
	// NOISE_CR_1 control line as the one at the end of this circuit) and
	// a 0.1 uF capacitor between H4_3.OUT and R9.1 here.

	LM3900(H4_4)

	// For resistor R11, the value given is that shown in the Laguna Racer
	// PCB drawings, the Laguna Racer B622 board schematic, the Super
	// Speed Race schematic, and Andrew Welburn's 280-ZZZAP PCB photo. In
	// the 280-ZZZAP schematic and the near-copy Laguna Racer A622 board
	// schematic, R11 is 100 Kohms, but this seems to be a schematic
	// error. (The original National Semiconductor Application Note 72
	// from which this circuit was apparently taken also uses 120 Kohm for
	// R11.)
	RES(R9, RES_K(39))
	RES(R10, RES_K(62))   // 240 Kohm in Super Speed Race
	RES(R11, RES_K(120))
	RES(R12, RES_K(62))

	NET_C(H4_4.MINUS, R9.2, R11.1)
	NET_C(I_V5.Q, R10.1)
	NET_C(H4_4.PLUS, R10.2)
	NET_C(H4_4.OUT, R11.2, R12.2, G4.4)
	NET_C(G4.3, R63.1)
	NET_C(G4.5, NOISE_CR_1)

	// Second noise circuit: post-crash noise (NOISE_CR_2)

	// This circuit is peculiar. It's structured like a single-amplifier
	// active low-pass filter, with a corner frequency of about 1 kHz and
	// a gain of 100--but its Q factor turns out to be a small *negative*
	// number. (I'm not sure what effect this has on a filter, but it
	// might indicate instability.)

	// The result is saturated, heavily clipped noise with frequencies
	// mainly below 1 kHz.

	// I don't know why the circuit was designed this way, or whether it
	// was deliberate or a design or production error which the makers
	// decided they liked or at least could accept. It doesn't look like
	// they fixed it later; this same circuit is unchanged in the
	// schematics for all three games.

	RES(R17, RES_K(10))

	CAP(C5, CAP_U(10))
	CAP(C4, CAP_U(0.022))

	LM3900(H5_2)

	RES(R16, RES_K(1.5))
	RES(R15, RES_K(15))
	RES(R14, RES_M(2.7))
	RES(R13, RES_K(150))

	CAP(C6, CAP_U(0.01))
	CAP(C9, CAP_U(0.001))

	NET_C(C4.2, R16.1)
	NET_C(R16.2, C6.1, R15.1, R13.1)
	NET_C(C6.2, GND)
	NET_C(H5_2.MINUS, R15.2, C9.1)
	NET_C(I_V5.Q, R14.1)
	NET_C(H5_2.PLUS, R14.2)
#if REMOVE_POST_CRASH_NOISE_GLITCHES
	// With the static time-stepping used to ensure acceptable performance
	// with MAME, this part of the netlist will generate extra spikes on
	// the op-amp output with voltages outside of the real op-amp's output
	// range. These spikes give the sound an unwanted "grittiness" not in
	// the original, so I've added a voltage-limiting AFUNC to the op-amp
	// output beyond its feedback connection. This gives a smoother sound,
	// closer to the original.

	NET_C(H5_2.OUT, C9.2, R13.2, H5_2_LIM.A0)
	AFUNC(H5_2_LIM, 1, "max(min(A0, 4.5), 0)")
	NET_C(H5_2_LIM.Q, G4.8)
#else
	NET_C(H5_2.OUT, C9.2, R13.2, G4.8)
#endif
	NET_C(G4.9, R64.1)
	NET_C(G4.6, NOISE_CR_2)

	// Third noise circuit: boom from crash (BOOM)

	// This is a single-amplifier active bandpass filter with a center
	// frequency of about 60 Hz and a high Q value of about 19, giving a
	// narrow 3 Hz bandwidth. The gain is also very high, a little over
	// 200.

	// The filter is normally cut off from the noise signal, and thus it
	// remains quiet. When the BOOM signal is activated, CD4016 switch
	// G4_A opens, letting in the noise to be filtered and amplified until
	// the switch is cut off again, generating a loud, fairly deep "boom".
	// (The "boom" doesn't have much decay, though; it gets cut abruptly.)

	LM3900(H5_1)

	// Resistor R18, part of the BOOM filter, is 2.2 Kohms on the
	// 280-ZZZAP schematic as well as in Laguna Racer, but the 280-ZZZAP
	// schematic has a note "E" saying that the first 325 machines had a
	// 6.8 Kohm resistor here instead. Andrew Welburn's 280-ZZZAP board
	// photo may show such a resistor. With this larger resistor, the
	// filter's values are: center frequency 54 Hz, Q 16.9, gain 73.5; so
	// the filter is slightly broader, pitched a bit lower, and has only a
	// third as much gain. Still, the basic effect is the same.
	RES(R18, RES_K(2.2))  // 20 Kohm in Super Speed Race

	RES(R19, RES_K(1))
	RES(R20, RES_M(3.3))
	RES(R21, RES_M(1))    // 1.5 Mohm in Super Speed Race

	CAP(C10, CAP_U(0.1))
	CAP(C11, CAP_U(0.1))

	NET_C(R17.2, C5.2, G4.1)
	NET_C(G4.2, R18.1)
	NET_C(G4.13, BOOM)
	NET_C(R18.2, R19.1, C10.1, C11.1)
	NET_C(R19.2, GND)
	NET_C(H5_1.MINUS, C10.2, R21.1)
	NET_C(I_V5.Q, R20.1)
	NET_C(H5_1.PLUS, R20.2)
	NET_C(H5_1.OUT, R21.2, C11.2, R65.1)


	// **** Final mix of sound effects and sound preamplification.

	// The preamplification stage after the final mix has two LM3900
	// op-amps. The audio power amplifier beyond it is a National
	// Semiconductor LM377 integrated dual power amp, with two 2-watt
	// power amps on the same chip. Both of these power amps drive the
	// single speaker from opposite ends in a push-pull fashion, a
	// so-called "bridge configuration" which effectively converts the
	// dual 2-watt power amp into a single 4-watt amp.

	// The two power amps get their inputs from the outputs of the two
	// LM3900 op-amps. The first LM3900, J5_3, pre-amplifies the signal
	// from the final mix and has a master volume potentiometer to control
	// its gain. Its output gets sent directly to one power amp and is
	// also passed to the second LM3900, J5_4, which inverts the signal
	// without changing the gain. The inverted signal is sent to the
	// second power amp that drives the speaker's other end. The first
	// LM3900 is AC-coupled to the second, and both are AC-coupled to the
	// power amps. As a result, the output signal of the second LM3900
	// more closely resembles the final output in waveform and tone
	// quality than the first LM3900's output does, so I have chosen the
	// second LM3900 as the final output for the emulation. The power amps
	// themselves are not emulated.

	// The master volume potentiometer is user-adjustable and is set to
	// its midpoint by default. Higher settings may produce clipped,
	// distorted output, but since the power amps are configured for a
	// voltage gain of 20 and use a 20 V power supply, this is likely true
	// for the original hardware as well.

	RES(R63, RES_K(12))  // 3 Kohm in Super Speed Race
	RES(R64, RES_K(150))

	// Resistor R65, the mixing resistor for the BOOM effect, is 12 Kohms
	// on the 280-ZZZAP schematic and in Laguna Racer and Super Speed
	// Race, but the 280-ZZZAP schematic has a note "F" saying that the
	// first 325 machines had a 4.3 Kohm resistor here instead. Andrew
	// Welburn's 280-ZZZAP board photo shows such a resistor. The lower
	// resistor makes the BOOM effect much louder, countering the reduced
	// gain on its generating filter described above. In fact, it makes it
	// loud enough to be clipped by the post-mixer op-amps; turning down
	// the volume to prevent this clipping makes all other sounds quieter
	// by comparison. This may explain why both the mixing resistor and
	// the filter resistor were changed in later machines.
	RES(R65, RES_K(12))

	RES(R66, RES_K(33))

	CAP(C20, CAP_U(10))
	CAP(C21, CAP_U(10))

	NET_C(R63.2, R64.2, R65.2, C20.1)
	NET_C(MC3340_H2.7, R66.1)  // MC3340 output pin
	NET_C(R66.2, C21.1)

	LM3900(J5_3)
	LM3900(J5_4)

	RES(R67, RES_K(2))
	CAP(C22, CAP_U(10))
	RES(R68, RES_K(220))
	RES(R74, RES_K(220))

	NET_C(I_V5.Q, R67.1)
	NET_C(R67.2, C22.1, R68.1, R74.1)
	NET_C(C22.2, GND)
	NET_C(J5_3.PLUS, R68.2)
	NET_C(J5_4.PLUS, R74.2)

	RES(R69, RES_K(100))
	POT2(R70, RES_K(10))  // Master volume potentiometer (2 terminals)
	CAP(C23, CAP_U(10))
	CAP(C24, CAP_U(0.1))
	RES(R73, RES_K(100))
	RES(R75, RES_K(100))

	NET_C(R70.2, C23.2)
	NET_C(J5_3.MINUS, C20.2, C21.2, R69.1, R70.1)
	NET_C(J5_3.OUT, C23.1, R69.2, C24.1)
	NET_C(C24.2, R73.1)
	NET_C(J5_4.MINUS, R73.2, R75.1)
	NET_C(J5_4.OUT, R75.2)

	ALIAS(OUTPUT, J5_4.OUT)

}


#if (SOUND_VARIANT == VARIANT_280ZZZAP)

NETLIST_START(280zzzap)
{

#else // (SOUND_VARIANT == VARIANT_LAGUNAR)

NETLIST_START(lagunar)
{

#endif

	SOLVER(Solver, 48000)

#if !(FAST_HLE_NOISE_GEN)
	PARAM(Solver.DYNAMIC_TS, 1)
	// 10 ns is the minimum timestep to avoid significant spikes outside
	// the allowed LM3900 output region of 0-4.5 V; even 20 ns gives
	// frequent spikes into the 4.5-5 volt region.
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 1e-8)
#endif

	// All together, loosening both tolerances and reducing accuracy
	// increases speed by ~10%, but it also causes audible "crackling".
//  PARAM(Solver.RELTOL, 1e-2) // default 1e-3 (several % faster, but < quality)
//  PARAM(Solver.VNTOL, 5e-3)  // default 1e-7 (several % faster, but < quality)
//  PARAM(Solver.ACCURACY, 1e-3)  // default 1e-7  (few % faster)

//  PARAM(Solver.DYNAMIC_TS, 1)
//  PARAM(Solver.DYNAMIC_LTE, 1e-4)  // default 1e-5
//  PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 1e-8)  // default 1e-6

	ANALOG_INPUT(I_V12, 12)
	ANALOG_INPUT(I_V5, 5)

#if (SOUND_VARIANT == VARIANT_280ZZZAP)

	LOCAL_SOURCE(280zzzap_schematics)
	INCLUDE(280zzzap_schematics)

#else // (SOUND_VARIANT == VARIANT_LAGUNAR)

	LOCAL_SOURCE(lagunar_schematics)
	INCLUDE(lagunar_schematics)

#endif

	// The MC3340 gets 12-volt power in 280-ZZZAP and Laguna Racer.
	// In Super Speed Race it gets 5-volt power.
	NET_C(I_V12.Q, MC3340_H2.8)
	NET_C(GND, MC3340_H2.3)

	// Logic inputs which represent output pins from 74174 latches at F4
	// and F5
	LOGIC_INPUT(I_F4_2, 0, "74XX")  // BOOM
	LOGIC_INPUT(I_F4_5, 0, "74XX")  // labeled "ENGINE SOUND", but really
					// an "engine sound off" flag
	LOGIC_INPUT(I_F4_7, 0, "74XX")  // NOISE CR 1
	LOGIC_INPUT(I_F4_10, 0, "74XX")  // NOISE CR 2
	LOGIC_INPUT(I_F5_2, 0, "74XX")  // PEDAL_BIT0
	LOGIC_INPUT(I_F5_5, 0, "74XX")  // PEDAL_BIT1
	LOGIC_INPUT(I_F5_7, 0, "74XX")  // PEDAL_BIT2
	LOGIC_INPUT(I_F5_10, 0, "74XX")  // PEDAL_BIT3
	LOGIC_INPUT(I_F5_12, 0, "74XX")  // HI SHIFT
	LOGIC_INPUT(I_F5_15, 0, "74XX")  // LO SHIFT

	ALIAS(I_BOOM, I_F4_2.IN)
	ALIAS(I_ENGINE_SOUND_OFF, I_F4_5.IN)
	ALIAS(I_NOISE_CR_1, I_F4_7.IN)
	ALIAS(I_NOISE_CR_2, I_F4_10.IN)
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
	ALIAS(PEDAL_BIT0, I_F5_2.Q)
	ALIAS(PEDAL_BIT1, I_F5_5.Q)
	ALIAS(PEDAL_BIT2, I_F5_7.Q)
	ALIAS(PEDAL_BIT3, I_F5_10.Q)
	ALIAS(HI_SHIFT, I_F5_12.Q)
	ALIAS(LO_SHIFT, I_F5_15.Q)

	// Power pins for logic inputs:
	NET_C(I_V5.Q,
		  I_F4_2.VCC, I_F4_5.VCC, I_F4_7.VCC, I_F4_10.VCC,
		  I_F5_2.VCC, I_F5_5.VCC, I_F5_7.VCC, I_F5_10.VCC,
		  I_F5_12.VCC, I_F5_15.VCC)
	NET_C(GND,
		  I_F4_2.GND, I_F4_5.GND, I_F4_7.GND, I_F4_10.GND,
		  I_F5_2.GND, I_F5_5.GND, I_F5_7.GND, I_F5_10.GND,
		  I_F5_12.GND, I_F5_15.GND)

	// Power inputs for the LM3900 op-amps. These aren't shown on the
	// schematics, but it looks like 5-volt power is needed to get proper
	// results.
	// (H4_2, the noise generator op-amp, is not listed here because it's
	// inside the #else clause of the #if FAST_HLE_NOISE_GEN conditional.)
	NET_C(I_V5.Q,
		H4_1.VCC, H4_3.VCC, H4_4.VCC,
		H5_1.VCC, H5_2.VCC, H5_3.VCC, H5_4.VCC,
		J3_1.VCC, J3_2.VCC, J3_3.VCC, J3_4.VCC,
		J5_1.VCC, J5_2.VCC, J5_3.VCC, J5_4.VCC)
	NET_C(GND,
		H4_1.GND, H4_3.GND, H4_4.GND,
		H5_1.GND, H5_2.GND, H5_3.GND, H5_4.GND,
		J3_1.GND, J3_2.GND, J3_3.GND, J3_4.GND,
		J5_1.GND, J5_2.GND, J5_3.GND, J5_4.GND)

	// Power inputs for the CD4016 switches. Again, these aren't shown on
	// the schematics, but 5-volt power must be used for the switches to
	// handle the voltage levels they are switching.
	NET_C(I_V5.Q, G5.14, J4.14, G4.14)
	NET_C(GND, G5.7, J4.7, G4.7)
	// Switches G5_D, J4_C, and G4_D are unused.
	NET_C(GND, G5.10, G5.11, G5.12, J4.6, J4.8, J4.9, G4.10, G4.11, G4.12)

	// Frontier after output of noise generator.
	// FIXME: Anomaly - with FAST_HLE_NOISE_GEN set, this frontier cannot
	// be removed, because doing so kills the noise entirely, even though
	// the AFUNC is still generating the same output. Perhaps an AFUNC
	// output by itself won't change the capacitor state?
	OPTIMIZE_FRONTIER(C1.1, RES_M(1), 50)

	// Frontier before skid screech generator.
	OPTIMIZE_FRONTIER(R5.1, RES_K(39), 50)
	// (Adding a frontier *after* the skid screech generator makes the
	// sudden jumps in signal level on skids much larger, so the resulting
	// clicks are louder and more objectionable. Also it does little or
	// nothing for speed. That's why I don't have such a frontier.)

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
#if (SOUND_VARIANT == VARIANT_280ZZZAP)
	OPTIMIZE_FRONTIER(R30.1, RES_K(110), 50)
#else // (SOUND_VARIANT == VARIANT_LAGUNAR)
	OPTIMIZE_FRONTIER(R30.1, RES_K(100), 50)
#endif

#if CONVERGENCE_FRONTIERS
	// Frontiers at Schmitt trigger op-amp outputs of engine sound
	// oscillators, to eliminate numeric instability and speed convergence
	// rather than to partition matrices. The resistor values given are
	// the modified ones which compensate for how the frontiers would
	// otherwise change the oscillator waveforms.
	OPTIMIZE_FRONTIER(R40.1, RES_K(275), 50)
	OPTIMIZE_FRONTIER(R35.1, RES_K(281), 50)
#if (SOUND_VARIANT == VARIANT_280ZZZAP)
	OPTIMIZE_FRONTIER(R27.1, RES_K(284), 50)
#else // (SOUND_VARIANT == VARIANT_LAGUNAR)
	OPTIMIZE_FRONTIER(R27.1, RES_K(286), 50)
#endif
#endif

}


#endif
