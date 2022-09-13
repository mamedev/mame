# The netlist FAQ

## Glossary

BTANB: Bugs That Aren't Bugs. Bugs in the hardware (or software) that are 
faithfully emulated, therefore not a bug in the emulator.

## General

### Have you considered maybe supporting part randomization in the netlist core?

Yes I did. And decided against it. Which features? Forced randomness - will this
be different at every restart? And it would require another parameter per device
(specification, e.g. for resistors 5%, 10%, 20%). This would cause more cache
usage.

And I am convinced we would see more BTANBs.

### How do frontiers work?

Frontiers divides a netlist into sub netlists, i.e. a number of smaller netlists.
Frontiers work best if you have a low impedance to high impedance transition.
This is best illustrated by an example. Consider the following mixing stage

	                 R1
	      S1 >-----1RRRR2---------+
	                              |
	                 R2           |
	      S2 >-----1RRRR2---------+----------> Out
	                              |
	                              R
	                           R3 R
	                              R
	                              |
	                             GND

With `OPTIMIZE_FRONTIER(R2.2, R3, R2)` this becomes:

	                 R1
	      S1 >-----1RRRR2--------------------------------+
	                                                     |
	                       ##########################    |
	                 R2    #                    R2  #    |
	      S2 >-----1RRRR2-----+-->AnIn AnOut>--RRRR------+----------> Out
	                       #  |                     #    |
	                       #  R                     #    R
	                       #  R R3                  # R3 R
	                       #  R                     #    R
	                       #  |                     #    |
	                       # GND          Frontier  #   GND
	                       #                        #
	                       ##########################

As a result, provided there are no other connections between the parts
generating S1 and S2 the "S2 part" will now have a separate solver.

The size (aka number of nets) of the solver for S1 will be smaller.
The size of the solver for S2 and the rest of the circuit will be smaller
as well.

Frontiers assume that there is little to no feedback from the "out" terminal to the "in"
terminal. This is a safe assumption if the "in" terminal e.g. is connected to an
op-amp output (typically < 500 Ohm) and R2 in the example above is in the 10 KOhm range.

## MAME specific

### Is there an example of a netlist that takes input from an AY-8910/2 and handles it?

There are quite a few:

- nl_kidniki.cpp
- nl_konami.cpp
- 1942.cpp

Basically the AY8910 driver has to be configured to push resistor values to the
input stream. The konami drivers (scramble, frogger and friends) do that and
should be used as a reference. 1942 connects outputs and may be an even better example.

## Models

### Is there are JFET model?

No, there is currently no JFET model in netlist. They are close to depletion 
mode MOSFETs with very low gate capacitance so you may try a generic n-mosfet 
with a negative trigger voltage. Example:

	MOSFET(Q21, "NMOS(VTO=-1.0)")

Writing an analog model is a very time-consuming task. The simplified model above should
deliver reasonable results.

## Operational Amplifiers

### What's UGF? Would like to understand this black magic a little better. :)

UGF = Unity Gain Frequency, usually identical to gain-bandwidth product for
op-amps. The idea is that the an op-amp's open-loop gain (its amplification
factor when used without any negative feedback) is proportional to the
frequency of its input signal. For slowly-varying, near-DC signals, this gain
will be extremely high (in this case, a gain of 10000 at 1 Hz), which is part
of what makes an op-amp effective. In practice the op-amp's actual gain will
be limited by whatever negative feedback is applied, but its open-loop gain
is a key quantity in computing its actual response. Because the op-amp is
limited in how fast it can respond to its input, its open-loop gain will
drop as the frequency of the input signal rises, eventually falling to a
gain of 1 (output = input) at the unity gain frequency. In this case
couriersud set the unity gain frequency is 10 kHz, which is much lower than
reality, in order to make the op-amp response in the oscillators slower and
easier for the netlist solver to handle without using very small time steps.
According to Texas Instruments, the TL084's actual gain-bandwidth product
(remember, this is the same as the UGF) is typically 3 MHz.

## Documentation

### What is the preferred documentation format?

The preferred documentation for devices is to use the netlist documentation format.
This will ensure that the devices are properly documented in the Doxygen
documentation created by `make doc`

An example entry is given here:

	//- Identifier: SN74LS629_DIP
	//- Title: SN74LS629 VOLTAGE-CONTROLLED OSCILLATORS
	//- Description: Please add a detailed description
	//-    FIXME: Missing description
	//-
	//- Pinalias: 2FC,1FC,1RNG,1CX1,1CX2,1ENQ,1Y,OSC_GND,GND,2Y,2ENQ,2CX2,2CX1,2RNG,OSC_VCC,VCC
	//- Package: DIP
	//- Param: A.CAP
	//-    Capacitor value of capacitor connected to 1CX1 and 1CX2 pins
	//- Param: B.CAP
	//-    Capacitor value of capacitor connected to 2CX1 and 2CX2 pins
	//- Limitations:
	//-    The capacitor inputs are NC. Capacitor values need to be specified as
	//-    ```
	//-    SN74LS629_DIP(X)
	//-    PARAM(X.A.CAP, CAP_U(1))
	//-    PARAM(X.B.CAP, CAP_U(2))
	//-    ```
	//-
	//- Example: 74ls629.cpp,74ls629_example
	//-
	//- FunctionTable:
	//-    http://pdf.datasheetcatalog.com/datasheets/400/335051_DS.pdf
	//-
	static NETLIST_START(SN74LS629_DIP)
{

If you add an example in the examples folder this will be included in the 
documentation as well.

