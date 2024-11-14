/*! \page primer_1 nltool primer

# First example

Let's start with a simple charge/discharge circuit. I denotes the input,
O denotes the output.

    I >----RRR----+-----> O
                  |
                  C
                  C
                  C
                  |
                 GND

Now in netlist syntax this looks like:

~~~{.cpp}
#include "netlist/devices/net_lib.h"

NETLIST_START(charge_discharge)
{

    ANALOG_INPUT(V5, 5) // Clock needs a 5V power supply

    SOLVER(solver, 48000) // Fixed frequency solver
    CLOCK(I, 200) // 200 Hz  clock as input, TTL logic output
    RES(R, RES_K(1))
    CAP(C, CAP_U(1))

    NET_C(I.Q, R.1)
    NET_C(R.2, C.1)

    NET_C(GND, C.2, I.GND)
    NET_C(V5, I.VCC)

    ALIAS(O, R.2) // Output O == C.1 == R.2
}
~~~

Save this example as e.g. test1.cpp. Now that's a c++ extension. The background
is simple. You can also compile netlists. Thats a feature which is used by MAME.
That's something we will cover later.

Now, to test this, run the following:

~~~
> nltool --cmd=run -t 0.05 -l O -l I test1.cpp
~~~

This will run the circuit for 50 ms and log input "I" to file log_I.log and
output "O" to log_O.log . The log files will be located in the current folder.

Next, run

~~~
> plot_nl O I
~~~

![](test1-50r.svg)

and enjoy the results.

# Recap

We have created our first netlist fragment. nltool was used to model the
periodically forced charge/discharge circuit for 50ms. Finally we plotted both input
and output voltages using the plot_nl command using the log files we generated.

*/
