/*
 * Documentation settings
 */

/*!
 * \defgroup devices Devices
 *
 */

/*!
 * \page devices Devices
 *
 * Below is a list of all the devices currently supported ...
 *
 *         - \subpage AFUNC
 *         - \subpage ANALOG_INPUT
 *         - \subpage CAP
 *         - \subpage CCCS
 *         - \subpage CD4001_DIP
 *         - \subpage CD4001_NOR
 *         - \subpage CD4016_DIP
 *         - \subpage CD4020_DIP
 *         - \subpage CD4020
 *         - \subpage CD4020_WI
 *         - \subpage CD4066_DIP
 *         - \subpage CD4066_GATE
 *         - \subpage CD4316_DIP
 *         - \subpage CD4316_GATE
 *         - \subpage CD4538_DIP
 *         - \subpage CLOCK
 *         - \subpage CS
 *         - \subpage DIODE
 *         - \subpage DUMMY_INPUT
 *         - \subpage EPROM_2716_DIP
 *         - \subpage EPROM_2716
 *         - \subpage EXTCLOCK
 *         - \subpage FRONTIER_DEV
 *         - \subpage GND
 *         - \subpage IND
 *         - \subpage LM324_DIP
 *         - \subpage LM358_DIP
 *         - \subpage LM3900
 *         - \subpage LM747A_DIP
 *         - \subpage LM747_DIP
 *         - \subpage LOGD
 *         - \subpage LOGIC_INPUT
 *         - \subpage LOG
 *         - \subpage LVCCS
 *         - \subpage MAINCLOCK
 *         - \subpage MB3614_DIP
 *         - \subpage MC14584B_DIP
 *         - \subpage MC14584B_GATE
 *         - \subpage MM5837_DIP
 *         - \subpage NE555_DIP
 *         - \subpage NE555
 *         - \subpage NETDEV_DELAY
 *         - \subpage NETDEV_RSFF
 *         - \subpage OPAMP
 *         - \subpage opamp_layout_1_11_6
 *         - \subpage opamp_layout_1_7_4
 *         - \subpage opamp_layout_1_8_5
 *         - \subpage opamp_layout_2_13_9_4
 *         - \subpage opamp_layout_2_8_4
 *         - \subpage opamp_layout_4_4_11
 *         - \subpage PARAMETER
 *         - \subpage POT2
 *         - \subpage POT
 *         - \subpage PROM_82S115_DIP
 *         - \subpage PROM_82S115
 *         - \subpage PROM_82S123_DIP
 *         - \subpage PROM_82S123
 *         - \subpage PROM_82S126_DIP
 *         - \subpage PROM_82S126
 *         - \subpage QBJT_EB
 *         - \subpage QBJT_SW
 *         - \subpage R2R_DAC
 *         - \subpage RAM_2102A_DIP
 *         - \subpage RAM_2102A
 *         - \subpage RES
 *         - \subpage RES_SWITCH
 *         - \subpage SN74LS629_DIP
 *         - \subpage SN74LS629
 *         - \subpage SOLVER
 *         - \subpage SWITCH2
 *         - \subpage SWITCH
 *         - \subpage TTL_7400_DIP
 *         - \subpage TTL_7400_GATE
 *         - \subpage TTL_7400_NAND
 *         - \subpage TTL_7402_DIP
 *         - \subpage TTL_7402_GATE
 *         - \subpage TTL_7402_NOR
 *         - \subpage TTL_7404_DIP
 *         - \subpage TTL_7404_GATE
 *         - \subpage TTL_7404_INVERT
 *         - \subpage TTL_7408_AND
 *         - \subpage TTL_7408_DIP
 *         - \subpage TTL_7408_GATE
 *         - \subpage TTL_74107A
 *         - \subpage TTL_74107_DIP
 *         - \subpage TTL_74107
 *         - \subpage TTL_7410_DIP
 *         - \subpage TTL_7410_GATE
 *         - \subpage TTL_7410_NAND
 *         - \subpage TTL_7411_AND
 *         - \subpage TTL_7411_DIP
 *         - \subpage TTL_7411_GATE
 *         - \subpage TTL_74123_DIP
 *         - \subpage TTL_74123
 *         - \subpage TTL_74153_DIP
 *         - \subpage TTL_74153
 *         - \subpage TTL_74161_DIP
 *         - \subpage TTL_74161
 *         - \subpage TTL_74165_DIP
 *         - \subpage TTL_74165
 *         - \subpage TTL_74166_DIP
 *         - \subpage TTL_74166
 *         - \subpage TTL_7416_DIP
 *         - \subpage TTL_7416_GATE
 *         - \subpage TTL_74174_DIP
 *         - \subpage TTL_74174
 *         - \subpage TTL_74175_DIP
 *         - \subpage TTL_74175
 *         - \subpage TTL_74192_DIP
 *         - \subpage TTL_74192
 *         - \subpage TTL_74193_DIP
 *         - \subpage TTL_74193
 *         - \subpage TTL_74194_DIP
 *         - \subpage TTL_74194
 *         - \subpage TTL_7420_DIP
 *         - \subpage TTL_7420_GATE
 *         - \subpage TTL_7420_NAND
 *         - \subpage TTL_7425_DIP
 *         - \subpage TTL_7425_GATE
 *         - \subpage TTL_7425_NOR
 *         - \subpage TTL_74260_DIP
 *         - \subpage TTL_74260_GATE
 *         - \subpage TTL_74260_NOR
 *         - \subpage TTL_74279_DIP
 *         - \subpage TTL_7427_DIP
 *         - \subpage TTL_7427_GATE
 *         - \subpage TTL_7427_NOR
 *         - \subpage TTL_7430_DIP
 *         - \subpage TTL_7430_GATE
 *         - \subpage TTL_7430_NAND
 *         - \subpage TTL_7432_DIP
 *         - \subpage TTL_7432_GATE
 *         - \subpage TTL_7432_OR
 *         - \subpage TTL_74365_DIP
 *         - \subpage TTL_74365
 *         - \subpage TTL_7437_DIP
 *         - \subpage TTL_7437_GATE
 *         - \subpage TTL_7437_NAND
 *         - \subpage TTL_7448_DIP
 *         - \subpage TTL_7448
 *         - \subpage TTL_7450_ANDORINVERT
 *         - \subpage TTL_7450_DIP
 *         - \subpage TTL_7473A_DIP
 *         - \subpage TTL_7473A
 *         - \subpage TTL_7473_DIP
 *         - \subpage TTL_7473
 *         - \subpage TTL_7474_DIP
 *         - \subpage TTL_7474
 *         - \subpage TTL_7475_DIP
 *         - \subpage TTL_7475
 *         - \subpage TTL_7477_DIP
 *         - \subpage TTL_7477
 *         - \subpage TTL_7483_DIP
 *         - \subpage TTL_7483
 *         - \subpage TTL_7485_DIP
 *         - \subpage TTL_7485
 *         - \subpage TTL_7486_DIP
 *         - \subpage TTL_7486_GATE
 *         - \subpage TTL_7486_XOR
 *         - \subpage TTL_7490_DIP
 *         - \subpage TTL_7490
 *         - \subpage TTL_7493_DIP
 *         - \subpage TTL_7493
 *         - \subpage TTL_82S16_DIP
 *         - \subpage TTL_82S16
 *         - \subpage TTL_9310_DIP
 *         - \subpage TTL_9310
 *         - \subpage TTL_9312_DIP
 *         - \subpage TTL_9312
 *         - \subpage TTL_9316_DIP
 *         - \subpage TTL_9316
 *         - \subpage TTL_9322_DIP
 *         - \subpage TTL_9322
 *         - \subpage TTL_9334_DIP
 *         - \subpage TTL_9334
 *         - \subpage TTL_9602_DIP
 *         - \subpage TTL_AM2847_DIP
 *         - \subpage TTL_AM2847
 *         - \subpage TTL_INPUT
 *         - \subpage TTL_TRISTATE3
 *         - \subpage TTL_TRISTATE
 *         - \subpage UA741_DIP10
 *         - \subpage UA741_DIP14
 *         - \subpage UA741_DIP8
 *         - \subpage VCCS
 *         - \subpage VCVS
 *         - \subpage VS
 *
 * \mainpage Netlist



##Netlist

###A mixed signal circuit simulation.

- D: Device
- O: Rail output (output)
- I: Infinite impedance input (input)
- T: Terminal (finite impedance)

The following example shows a typical connection between several devices:

    +---+     +---+     +---+     +---+     +---+
    |   |     |   |     |   |     |   |     |   |
    | D |     | D |     | D |     | D |     | D |
    |   |     |   |     |   |     |   |     |   |
    +-O-+     +-I-+     +-I-+     +-T-+     +-T-+
      |         |         |         |         |
    +-+---------+---------+---------+---------+-+
    | rail net                                  |
    +-------------------------------------------+

A rail net is a net which is driven by exactly one output with an
(idealized) internal resistance of zero.
Ideally, it can deliver infinite current.

A infinite resistance input does not source or sink current.

Terminals source or sink finite (but never zero) current.

The system differentiates between analog and logic input and outputs and
analog terminals. Analog and logic devices can not be connected to the
same net. Instead, proxy devices are inserted automatically:

    +---+     +---+
    |   |     |   |
    | D1|     | D2|
    | A |     | L |
    +-O-+     +-I-+
      |         |
    +-+---------+---+
    | rail net      |
    +---------------+

is converted into

                +----------+
                |          |
    +---+     +-+-+        |   +---+
    |   |     | L |  A-L   |   |   |
    | D1|     | D | Proxy  |   | D2|
    | A |     | A |        |   |   |
    +-O-+     +-I-+        |   +-I-+
      |         |          |     |
    +-+---------+--+     +-+-----+-------+
    | rail net (A) |     | rail net (L)  |
    +--------------|     +---------------+

This works both analog to logic as well as logic to analog.

The above is an advanced implementation of the existing discrete
subsystem in MAME. Instead of relying on a fixed time-step, analog devices
could either connect to fixed time-step clock or use an internal clock
to update them. This would however introduce macro devices for RC, diodes
and transistors again.

Instead, the following approach in case of a pure terminal/input network
is taken:

    +---+     +---+     +---+     +---+     +---+
    |   |     |   |     |   |     |   |     |   |
    | D |     | D |     | D |     | D |     | D |
    |   |     |   |     |   |     |   |     |   |
    +-T-+     +-I-+     +-I-+     +-T-+     +-T-+
      |         |         |         |         |
     '+'        |         |        '-'       '-'
    +-+---------+---------+---------+---------+-+
    | Calculated net                            |
    +-------------------------------------------+

Netlist uses the following basic two terminal device:

         (k)
    +-----T-----+
    |     |     |
    |  +--+--+  |
    |  |     |  |
    |  R     |  |
    |  R     |  |
    |  R     I  |
    |  |     I  |  Device n
    |  V+    I  |
    |  V     |  |
    |  V-    |  |
    |  |     |  |
    |  +--+--+  |
    |     |     |
    +-----T-----+
         (l)

This is a resistance in series to a voltage source and paralleled by a
current source. This is suitable to model voltage sources, current sources,
resistors, capacitors, inductances and diodes.

\f[
    I_{n,l} = - I_{n,k} = ( V_k - V^N - V_l ) \frac{1}{R^n} + I^n
\f]

Now, the sum of all currents for a given net must be 0:

\f[
    \sum_n I_{n,l} = 0 = \sum_{n,k} (V_k - V^n - V_l ) \frac{1}{R^n} + I^n
\f]

With \f$ G^n = \frac{1}{R^n} \f$ and \f$ \sum_n  G^n = G^{tot} \f$ and \f$k=k(n)\f$

\f[
    0 = - V_l G^{tot} + \sum_n (V_{k(n)} - V^n) G^n + I^n)
\f]

and with \f$ l=l(n)\f$  and fixed \f$ k\f$

\f[
    0 =  -V_k G^{tot} + sum_n( V_{l(n)} + V^n ) G^n - I^n)
\f]

These equations represent a linear Matrix equation (with more math).

In the end the solution of the analog subsystem boils down to

\f[
      \mathbf{\it{(G - D) v = i}}
\f]

with G being the conductance matrix, D a diagonal matrix with the total
conductance on the diagonal elements, V the net voltage vector and I the
current vector.

By using solely two terminal devices, we can simplify the whole calculation
significantly. A BJT now is a four terminal device with two terminals being
connected internally.

The system is solved using an iterative approach:

G V - D V = I

assuming V=Vn=Vo

Vn = D-1 (I - G Vo)

Each terminal thus has three properties:

a) Resistance
b) Voltage source
c) Current source/sink

Going forward, the approach can be extended e.g. to use a linear
equation solver.

The formal representation of the circuit will stay the same, thus scales.

*/
