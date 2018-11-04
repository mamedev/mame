.. _debugger-expressions-list:

Debugger Expressions Guide
==========================


Expressions can be used anywhere a numeric parameter is expected. The syntax for expressions is very close to standard C-style syntax with full operator ordering and parentheses. There are a few operators missing (notably the trinary ? : operator), and a few new ones (memory accessors). The table below lists all the operators in their order, highest precedence operators first.

|
| ( ) : standard parentheses
| ++ -- : postfix increment/decrement
| ++ -- ~ ! - + b@ w@ d@ q@ : prefix inc/dec, binary NOT, logical NOT, unary +/-, memory access
| * / % : multiply, divide, modulus
| + - : add, subtract
| << >> : shift left/right
| < <= > >= : less than, less than or equal, greater than, greater than or equal
| == != : equal, not equal
| & : binary AND
| ^ : binary XOR
| | : binary OR
| && : logical AND
| || : logical OR
| = \*= /= %= += -= <<= >>= &= \|= ^= : assignment
| , : separate terms, function parameters
|
|


Differences from C Behaviors
----------------------------


- First, all math is performed on full 64-bit unsigned values, so things like **a < 0** won't work as expected.

- Second, the logical operators **&&** and **||** do not have short-circuit properties -- both halves are always evaluated.

- Finally, the new memory operators work like this:

  - **b!<addr>** refers to the byte at <addr> but does *NOT* suppress side effects such as reading a mailbox clearing the pending flag, or reading a FIFO removing an item.

  - **b@<addr>** refers to the byte at <addr> while suppressing side effects.

  - Similarly, **w@** and **w!** refer to a *word* in memory, **d@** and **d!** refer to a *dword* in memory, and **q@** and **q!** refer to a *qword* in memory.

    The memory operators can be used as both lvalues and rvalues, so you can write **b\@100 = ff** to store a byte in memory. By default these operators read from the program memory space, but you can override that by prefixing them with a 'd' or an 'i'.

    As such, **dw\@300** refers to data memory word at address 300 and **id\@400** refers to an I/O memory dword at address 400.

