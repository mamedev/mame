//
// Documentation settings
//

///
/// \mainpage Netlist
///

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
