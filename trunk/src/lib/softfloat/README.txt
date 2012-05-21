MAME note: this package is derived from the following original SoftFloat 
package and has been "re-packaged" to work with MAME's conventions and
build system.  The source files come from bits64/ and bits64/templates
in the original distribution as MAME requires a compiler with a 64-bit
integer type.


Package Overview for SoftFloat Release 2b

John R. Hauser
2002 May 27


----------------------------------------------------------------------------
Overview

SoftFloat is a software implementation of floating-point that conforms to
the IEC/IEEE Standard for Binary Floating-Point Arithmetic.  SoftFloat is
distributed in the form of C source code.  Compiling the SoftFloat sources
generates two things:

-- A SoftFloat object file (typically `softfloat.o') containing the complete
   set of IEC/IEEE floating-point routines.

-- A `timesoftfloat' program for evaluating the speed of the SoftFloat
   routines.  (The SoftFloat module is linked into this program.)

The SoftFloat package is documented in four text files:

   SoftFloat.txt          Documentation for using the SoftFloat functions.
   SoftFloat-source.txt   Documentation for compiling SoftFloat.
   SoftFloat-history.txt  History of major changes to SoftFloat.
   timesoftfloat.txt      Documentation for using `timesoftfloat'.

Other files in the package comprise the source code for SoftFloat.

Please be aware that some work is involved in porting this software to other
targets.  It is not just a matter of getting `make' to complete without
error messages.  I would have written the code that way if I could, but
there are fundamental differences between systems that can't be hidden.
You should not attempt to compile SoftFloat without first reading both
`SoftFloat.txt' and `SoftFloat-source.txt'.


----------------------------------------------------------------------------
Legal Notice

SoftFloat was written by me, John R. Hauser.  This work was made possible in
part by the International Computer Science Institute, located at Suite 600,
1947 Center Street, Berkeley, California 94704.  Funding was partially
provided by the National Science Foundation under grant MIP-9311980.  The
original version of this code was written as part of a project to build
a fixed-point vector processor in collaboration with the University of
California at Berkeley, overseen by Profs. Nelson Morgan and John Wawrzynek.

THIS SOFTWARE IS DISTRIBUTED AS IS, FOR FREE.  Although reasonable effort
has been made to avoid it, THIS SOFTWARE MAY CONTAIN FAULTS THAT WILL AT
TIMES RESULT IN INCORRECT BEHAVIOR.  USE OF THIS SOFTWARE IS RESTRICTED TO
PERSONS AND ORGANIZATIONS WHO CAN AND WILL TAKE FULL RESPONSIBILITY FOR ALL
LOSSES, COSTS, OR OTHER PROBLEMS THEY INCUR DUE TO THE SOFTWARE, AND WHO
FURTHERMORE EFFECTIVELY INDEMNIFY JOHN HAUSER AND THE INTERNATIONAL COMPUTER
SCIENCE INSTITUTE (possibly via similar legal warning) AGAINST ALL LOSSES,
COSTS, OR OTHER PROBLEMS INCURRED BY THEIR CUSTOMERS AND CLIENTS DUE TO THE
SOFTWARE.

Derivative works are acceptable, even for commercial purposes, provided
that the minimal documentation requirements stated in the source code are
satisfied.


----------------------------------------------------------------------------
Contact Information

At the time of this writing, the most up-to-date information about
SoftFloat and the latest release can be found at the Web page `http://
www.cs.berkeley.edu/~jhauser/arithmetic/SoftFloat.html'.


