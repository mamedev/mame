// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "xadasm.h"

/*

The CPU implements the following opcodes

ADD Rd, Rs                  Add regs direct                                                         2 3     0000 S001  dddd ssss
ADD Rd, [Rs]                Add reg-ind to reg                                                      2 4     0000 S010  dddd 0sss
ADD [Rd], Rs                Add reg to reg-ind                                                      2 4     0000 S010  ssss 1ddd
ADD Rd, [Rs+offset8]        Add reg-ind w/ 8-bit offs to reg                                        3 6     0000 S100  dddd 0sss
ADD [Rd+offset8], Rs        Add reg to reg-ind w/ 8-bit offs                                        3 6     0000 S100  ssss 1ddd
ADD Rd, [Rs+offset16]       Add reg-ind w/ 16-bit offs to reg                                       4 6     0000 S101  dddd 0sss
ADD [Rd+offset16], Rs       Add reg to reg-ind w/ 16-bit offs                                       4 6     0000 S101  ssss 1ddd
ADD Rd, [Rs+]               Add reg-ind w/ autoinc to reg                                           2 5     0000 S011  dddd 0sss
ADD [Rd+], Rs               Add reg-ind w/ autoinc to reg                                           2 5     0000 S011  ssss 1ddd
ADD direct, Rs              Add reg to mem                                                          3 4     0000 S110  ssss 1DDD
ADD Rd, direct              Add mem to reg                                                          3 4     0000 S110  dddd 0DDD

ADD Rd, #data8              Add 8-bit imm data to reg                                               3 3     1001 0001  dddd 0000  iiii iiii
ADD Rd, #data16             Add 16-bit imm data to reg                                              4 3     1001 1001  dddd 0000  iiii iiii  iiii iiii
ADD [Rd], #data8            Add 8-bit imm data to reg-ind                                           3 4     1001 0010  0ddd 0000  iiii iiii
ADD [Rd], #data16           Add 16-bit imm data to reg-ind                                          4 4     1001 1010  0ddd 0000  iiii iiii  iiii iiii
ADD [Rd+], #data8           Add 8-bit imm data to reg-ind w/ autoinc                                3 5     1001 0011  0ddd 0000  iiii iiii
ADD [Rd+], #data16          Add 16-bit imm data to reg-ind w/ autoinc                               4 5     1001 1011  0ddd 0000  iiii iiii  iiii iiii
ADD [Rd+offset8], #data8    Add 8-bit imm data to reg-ind w/ 8-bit offs                             4 6     1001 0100  0ddd 0000  oooo oooo  iiii iiii
ADD [Rd+offset8], #data16   Add 16-bit imm data to reg-ind w/ 8-bit offs                            5 6     1001 1100  0ddd 0000  oooo oooo  iiii iiii  iiii iiii
ADD [Rd+offset16], #data8   Add 8-bit imm data to reg-ind w/ 16-bit offs                            5 6     1001 0101  0ddd 0000  oooo oooo  oooo oooo  iiii iiii
ADD [Rd+offset16], #data16  Add 16-bit imm data to reg-ind w/ 16-bit offs                           6 6     1001 1101  0ddd 0000  oooo oooo  oooo oooo  iiii iiii  iiii iiii
ADD direct, #data8          Add 8-bit imm data to mem                                               4 4     1001 0110  0DDD 0000  DDDD DDDD  iiii iiii
ADD direct, #data16         Add 16-bit imm data to mem                                              5 4     1001 1110  0DDD 0000  DDDD DDDD  iiii iiii  iiii iiii

ADDC Rd, Rs                 Add regs direct w/ carry                                                2 3     0001 S001  dddd ssss
ADDC Rd, [Rs]               Add reg-ind to reg w/ carry                                             2 4     0001 S010  dddd 0sss
ADDC [Rd], Rs               Add reg to reg-ind w/ carry                                             2 4     0001 S010  ssss 1ddd
ADDC Rd, [Rs+offset8]       Add reg-ind w/ 8-bit offs to reg w/ carry                               3 6     0001 S100  dddd 0sss
ADDC [Rd+offset8], Rs       Add reg to reg-ind w/ 8-bit offs w/ carry                               3 6     0001 S100  ssss 1ddd
ADDC Rd, [Rs+offset16]      Add reg-ind w/ 16-bit offs to reg w/ carry                              4 6     0001 S101  dddd 0sss
ADDC [Rd+offset16], Rs      Add reg to reg-ind w/ 16-bit offs w/ carry                              4 6     0001 S101  ssss 1ddd
ADDC Rd, [Rs+]              Add reg-ind w/ autoinc to reg w/ carry                                  2 5     0001 S011  dddd 0sss
ADDC [Rd+], Rs              Add reg-ind w/ autoinc to reg w/ carry                                  2 5     0001 S011  ssss 1ddd
ADDC direct, Rs             Add reg to mem w/ carry                                                 3 4     0001 S110  ssss 1DDD
ADDC Rd, direct             Add mem to reg w/ carry                                                 3 4     0001 S110  dddd 0DDD

ADDC Rd, #data8             Add 8-bit imm data to reg w/ carry                                      3 3     1001 0001  dddd 0001  iiii iiii
ADDC Rd, #data16            Add 16-bit imm data to reg w/ carry                                     4 3     1001 1001  dddd 0001  iiii iiii  iiii iiii
ADDC [Rd], #data8           Add 16-bit imm data to reg-ind w/ carry                                 3 4     1001 0010  0ddd 0001  iiii iiii
ADDC [Rd], #data16          Add 16-bit imm data to reg-ind w/ carry                                 4 4     1001 1010  0ddd 0001  iiii iiii  iiii iiii
ADDC [Rd+], #data8          Add 8-bit imm data to reg-ind and autoinc w/ carry                      3 5     1001 0011  0ddd 0001  iiii iiii
ADDC [Rd+], #data16         Add 16-bit imm data to reg-ind and autoinc w/ carry                     4 5     1001 1011  0ddd 0001  iiii iiii  iiii iiii
ADDC [Rd+offset8], #data8   Add 8-bit imm data to reg-ind w/ 8-bit offs and carry                   4 6     1001 0100  0ddd 0001  oooo oooo  iiii iiii
ADDC [Rd+offset8], #data16  Add 16-bit imm data to reg-ind w/ 8-bit offs and carry                  5 6     1001 1100  0ddd 0001  oooo oooo  iiii iiii  iiii iiii
ADDC [Rd+offset16], #data8  Add 8-bit imm data to reg-ind w/ 16-bit offs and carry                  5 6     1001 0101  0ddd 0001  oooo oooo  oooo oooo  iiii iiii
ADDC [Rd+offset16], #data16 Add 16-bit imm data to reg-ind w/ 16-bit offs and carry                 6 6     1001 1101  0ddd 0001  oooo oooo  oooo oooo  iiii iiii  iiii iiii
ADDC direct, #data8         Add 8-bit imm data to mem w/ carry                                      4 4     1001 0110  0DDD 0001  DDDD DDDD  iiii iiii
ADDC direct, #data16        Add 16-bit imm data to mem w/ carry                                     5 4     1001 1110  0DDD 0001  DDDD DDDD  iiii iiii  iiii iiii

ADDS Rd, #data4             Add 4-bit signed imm data to reg                                        2 3     1010 S001  dddd iiii
ADDS [Rd], #data4           Add 4-bit signed imm data to reg-ind                                    2 4     1010 S010  0ddd iiii
ADDS [Rd+], #data4          Add 4-bit signed imm data to reg-ind w/ autoinc                         2 5     1010 S011  0ddd iiii
ADDS [Rd+offset8], #data4   Add reg-ind w/ 8-bit offs to 4-bit signed imm data                      3 6     1010 S100  0ddd iiii  oooo oooo
ADDS [Rd+offset16], #data4  Add reg-ind w/ 16-bit offs to 4-bit signed imm data                     4 6     1010 S101  0ddd iiii  oooo oooo  oooo oooo
ADDS direct, #data4         Add 4-bit signed imm data to mem                                        3 4     1010 S110  0DDD iiii  DDDD DDDD

AND Rd, Rs                  Logical AND regs direct                                                 2 3     0101 S001  dddd ssss
AND Rd, [Rs]                Logical AND reg-ind to reg                                              2 4     0101 S010  dddd 0sss
AND [Rd], Rs                Logical AND reg to reg-ind                                              2 4     0101 S010  ssss 1ddd
AND Rd, [Rs+offset8]        Logical AND reg-ind w/ 8-bit offs to reg                                3 6     0101 S100  dddd 0sss
AND [Rd+offset8], Rs        Logical AND reg to reg-ind w/ 8-bit offs                                3 6     0101 S100  ssss 1ddd
AND Rd, [Rs+offset16]       Logical AND reg-ind w/ 16-bit offs to reg                               4 6     0101 S101  dddd 0sss
AND [Rd+offset16], Rs       Logical AND reg to reg-ind w/ 16-bit offs                               4 6     0101 S101  ssss 1ddd
AND Rd, [Rs+]               Logical AND reg-ind w/ autoinc to reg                                   2 5     0101 S011  dddd 0sss
AND [Rd+], Rs               Logical AND reg-ind w/ autoinc to reg                                   2 5     0101 S011  ssss 1ddd
AND direct, Rs              Logical AND reg to mem                                                  3 4     0101 S110  ssss 1DDD
AND Rd, direct              Logical AND mem to reg                                                  3 4     0101 S110  dddd 0DDD

AND Rd, #data8              Logical AND 8-bit imm data to reg                                       3 3     1001 0001  dddd 0101  iiii iiii
AND Rd, #data16             Logical AND 16-bit imm data to reg                                      4 3     1001 1001  dddd 0101  iiii iiii  iiii iiii
AND [Rd], #data8            Logical AND 8-bit imm data to reg-ind                                   3 4     1001 0010  0ddd 0101  iiii iiii
AND [Rd], #data16           Logical AND 16-bit imm data to reg-ind                                  4 4     1001 1010  0ddd 0101  iiii iiii  iiii iiii
AND [Rd+], #data8           Logical AND 8-bit imm data to reg-ind and autoinc                       3 5     1001 0011  0ddd 0101  iiii iiii
AND [Rd+], #data16          Logical AND 16-bit imm data to reg-ind and autoinc                      4 5     1001 1011  0ddd 0101  iiii iiii  iiii iiii
AND [Rd+offset8], #data8    Logical AND 8-bit imm data to reg-ind w/ 8-bit offs                     4 6     1001 0100  0ddd 0101  oooo oooo  iiii iiii
AND [Rd+offset8], #data16   Logical AND 16-bit imm data to reg-ind w/ 8-bit offs                    5 6     1001 1100  0ddd 0101  oooo oooo  iiii iiii  iiii iiii
AND [Rd+offset16], #data8   Logical AND 8-bit imm data to reg-ind w/ 16-bit offs                    5 6     1001 0101  0ddd 0101  oooo oooo  oooo oooo  iiii iiii
AND [Rd+offset16], #data16  Logical AND 16-bit imm data to reg-ind w/ 16-bit offs                   6 6     1001 1101  0ddd 0101  oooo oooo  oooo oooo  iiii iiii  iiii iiii
AND direct, #data8          Logical AND 8-bit imm data to mem                                       4 4     1001 0110  0DDD 0101  DDDD DDDD  iiii iiii
AND direct, #data16         Logical AND 16-bit imm data to mem                                      5 4     1001 1110  0DDD 0101  DDDD DDDD  iiii iiii  iiii iiii

ANL C, bit                  Logical AND bit to carry                                                3 4     0000 1000  0100 00bb  bbbb bbbb

ANL C, /bit                 Logical AND complement of a bit to carry                                3 4

ASL Rd, Rs                  Logical left shift dest reg by the value in the src reg                 2 a*    1100 SS01  dddd ssss
ASL Rd, #data4              Logical left shift reg by the 4-bit imm value                           2 a*    1101 SS01  dddd iiii
ASL Rd, #data5              Logical left shift reg by the 5-bit imm value                           2 a*    1101 1101  dddi iiii

ASR Rd, Rs                  Arithmetic shift right dest reg by the count in the src                 2 a*    1100 SS10  dddd ssss
ASR Rd, #data4              Arithmetic shift right reg by the 4-bit imm count                       2 a*    1101 SS10  dddd iiii
ASR Rd, #data5              Arithmetic shift right reg by the 5-bit imm count                       2 a*    1101 1110  dddi iiii

CMP Rd, Rs                  Compare dest and src regs                                               2 3
CMP [Rd], Rs                Compare reg w/ reg-ind                                                  2 4
CMP Rd, [Rs]                Compare reg-ind w/ reg                                                  2 4
CMP [Rd+offset8], Rs        Compare reg w/ reg-ind w/ 8-bit offs                                    3 6
CMP [Rd+offset16], Rs       Compare reg w/ reg-ind w/ 16-bit offs                                   4 6
CMP Rd, [Rs+offset8]        Compare reg-ind w/ 8-bit offs w/ reg                                    3 6
CMP Rd,[Rs+offset16]        Compare reg-ind w/ 16-bit offs w/ reg                                   4 6
CMP Rd, [Rs+]               Compare autoinc reg-ind w/ reg                                          2 5
CMP [Rd+], Rs               Compare reg w/ autoinc reg-ind                                          2 5
CMP direct, Rs              Compare reg w/ mem                                                      3 4
CMP Rd, direct              Compare mem w/ reg                                                      3 4

CMP Rd, #data8              Compare 8-bit imm data to reg                                           3 3
CMP Rd, #data16             Compare 16-bit imm data to reg                                          4 3
CMP [Rd], #data8            Compare 8 -bit imm data to reg-ind                                      3 4
CMP [Rd], #data16           Compare 16-bit imm data to reg-ind                                      4 4
CMP [Rd+], #data8           Compare 8-bit imm data to reg-ind w/ autoinc                            3 5
CMP [Rd+], #data16          Compare 16-bit imm data to reg-ind w/ autoinc                           4 5
CMP [Rd+offset8], #data8    Compare 8-bit imm data to reg-ind w/ 8-bit offs                         4 6
CMP [Rd+offset8], #data16   Compare 16-bit imm data to reg-ind w/ 8-bit offs                        5 6
CMP [Rd+offset16], #data8   Compare 8-bit imm data to reg-ind w/ 16-bit offs                        5 6
CMP [Rd+offset16], #data16  Compare 16-bit imm data to reg-ind w/ 16-bit offs                       6 6
CMP direct, #data8          Compare 8-bit imm data to mem                                           4 4
CMP direct, #data16         Compare 16-bit imm data to mem                                          5 4

DA Rd                       Decimal Adjust byte reg                                                 2 4

DIV.w Rd, Rs                16x8 signed reg divide                                                  2 14
DIV.w Rd, #data8            16x8 signed divide reg w/ imm word                                      3 14
DIV.d Rd, Rs                32x16 signed double reg divide                                          2 24
DIV.d Rd, #data16           32x16 signed double reg divide w/ imm word                              4 24

DIVU.b Rd, Rs               8x8 unsigned reg divide                                                 2 12
DIVU.b Rd, #data8           8X8 unsigned reg divide w/ imm byte                                     3 12
DIVU.w Rd, Rs               16X8 unsigned reg divide                                                2 12
DIVU.w Rd, #data8           16X8 unsigned reg divide w/ imm byte                                    3 12
DIVU.d Rd, Rs               32X16 unsigned double reg divide                                        2 22
DIVU.d Rd, #data16          32X16 unsigned double reg divide w/ imm word                            4 22

LEA Rd, Rs+offset8          Load 16-bit effective address w/ 8-bit offs to reg                      3 3
LEA Rd, Rs+offset16         Load 16-bit effective address w/ 16-bit offs to reg                     4 3

MUL.w Rd, Rs                16X16 signed multiply of reg contents                                   2 12
MUL.w Rd, #data16           16X16 signed multiply 16-bit imm data w/ reg                            4 12

MULU.b Rd, Rs               8X8 unsigned multiply of reg contents                                   2 12
MULU.b Rd, #data8           8X8 unsigned multiply of 8-bit imm data w/ reg                          3 12
MULU.w Rd, Rs               16X16 unsigned reg multiply                                             2 12
MULU.w Rd, #data16          16X16 unsigned multiply 16-bit imm data w/ reg                          4 12

NEG Rd                      Negate (twos complement) reg                                            2 3

SEXT Rd                     Sign extend last operation to reg                                       2 3     1001 S000  dddd 1001

SUB Rd, Rs                  Subtract regs direct                                                    2 3     0010 S001  dddd ssss
SUB Rd, [Rs]                Subtract reg-ind to reg                                                 2 4     0010 S010  dddd 0sss
SUB [Rd], Rs                Subtract reg to reg-ind                                                 2 4     0010 S010  ssss 1ddd
SUB Rd, [Rs+offset8]        Subtract reg-ind w/ 8-bit offs to reg                                   3 6     0010 S100  dddd 0sss
SUB [Rd+offset8], Rs        Subtract reg to reg-ind w/ 8-bit offs                                   3 6     0010 S100  ssss 1ddd
SUB Rd, [Rs+offset16]       Subtract reg-ind w/ 16-bit offs to reg                                  4 6     0010 S101  dddd 0sss
SUB [Rd+offset16], Rs       Subtract reg to reg-ind w/ 16-bit offs                                  4 6     0010 S101  ssss 1ddd
SUB Rd, [Rs+]               Subtract reg-ind w/ autoinc to reg                                      2 5     0010 S011  dddd 0sss
SUB [Rd+], Rs               Subtract reg-ind w/ autoinc to reg                                      2 5     0010 S011  ssss 1ddd
SUB direct, Rs              Subtract reg to mem                                                     3 4     0010 S110  ssss 1DDD
SUB Rd, direct              Subtract mem to reg                                                     3 4     0010 S110  dddd 0DDD

SUB Rd, #data8              Subtract 8-bit imm data to reg                                          3 3     1001 0001  dddd 0010  iiii iiii
SUB Rd, #data16             Subtract 16-bit imm data to reg                                         4 3     1001 1001  dddd 0010  iiii iiii  iiii iiii
SUB [Rd], #data8            Subtract 8-bit imm data to reg-ind                                      3 4     1001 0010  0ddd 0010  iiii iiii
SUB [Rd], #data16           Subtract 16-bit imm data to reg-ind                                     4 4     1001 1010  0ddd 0010  iiii iiii  iiii iiii
SUB [Rd+], #data8           Subtract 8-bit imm data to reg-ind w/ autoinc                           3 5     1001 0011  0ddd 0010  iiii iiii
SUB [Rd+], #data16          Subtract 16-bit imm data to reg-ind w/ autoinc                          4 5     1001 1011  0ddd 0010  iiii iiii  iiii iiii
SUB [Rd+offset8], #data8    Subtract 8-bit imm data to reg-ind w/ 8-bit offs                        4 6     1001 0100  0ddd 0010  oooo oooo  iiii iiii
SUB [Rd+offset8], #data16   Subtract 16-bit imm data to reg-ind w/ 8-bit offs                       5 6     1001 1100  0ddd 0010  oooo oooo  iiii iiii  iiii iiii
SUB [Rd+offset16], #data8   Subtract 8-bit imm data to reg-ind w/ 16-bit offs                       5 6     1001 0101  0ddd 0010  oooo oooo  oooo oooo  iiii iiii
SUB [Rd+offset16], #data16  Subtract 16-bit imm data to reg-ind w/ 16-bit offs                      6 6     1001 1101  0ddd 0010  oooo oooo  oooo oooo  iiii iiii  iiii iiii
SUB direct, #data8          Subtract 8-bit imm data to mem                                          4 4     1001 0110  0DDD 0010  DDDD DDDD  iiii iiii
SUB direct, #data16         Subtract 16-bit imm data to mem                                         5 4     1001 1110  0DDD 0010  DDDD DDDD  iiii iiii  iiii iiii

SUBB Rd, Rs                 Subtract w/ borrow regs direct                                          2 3
SUBB Rd, [Rs]               Subtract w/ borrow reg-ind to reg                                       2 4
SUBB [Rd], Rs               Subtract w/ borrow reg to reg-ind                                       2 4
SUBB Rd, [Rs+offset8]       Subtract w/ borrow reg-ind w/ 8-bit offs to reg                         3 6
SUBB [Rd+offset8], Rs       Subtract w/ borrow reg to reg-ind w/ 8-bit offs                         3 6
SUBB Rd, [Rs+offset16]      Subtract w/ borrow reg-ind w/ 16-bit offs to reg                        4 6
SUBB [Rd+offset16], Rs      Subtract w/ borrow reg to reg-ind w/ 16-bit offs                        4 6
SUBB Rd, [Rs+]              Subtract w/ borrow reg-ind w/ autoinc to reg                            2 5
SUBB [Rd+], Rs              Subtract w/ borrow reg-ind w/ autoinc to reg                            2 5
SUBB direct, Rs             Subtract w/ borrow reg to mem                                           3 4
SUBB Rd, direct             Subtract w/ borrow mem to reg                                           3 4

SUBB Rd, #data8             Subtract w/ borrow 8-bit imm data to reg                                3 3
SUBB Rd, #data16            Subtract w/ borrow 16-bit imm data to reg                               4 3
SUBB [Rd], #data8           Subtract w/ borrow 8-bit imm data to reg-ind                            3 4
SUBB [Rd], #data16          Subtract w/ borrow 16-bit imm data to reg-ind                           4 4
SUBB [Rd+], #data8          Subtract w/ borrow 8-bit imm data to reg-ind w/ autoinc                 3 5
SUBB [Rd+], #data16         Subtract w/ borrow 16-bit imm data to reg-ind w/ autoinc                4 5
SUBB [Rd+offset8], #data8   Subtract w/ borrow 8-bit imm data to reg-ind w/ 8-bit offs              4 6
SUBB [Rd+offset8], #data16  Subtract w/ borrow 16-bit imm data to reg-ind w/ 8-bit offs             5 6
SUBB [Rd+offset16], #data8  Subtract w/ borrow 8-bit imm data to reg-ind w/ 16-bit offs             5 6
SUBB [Rd+offset16], #data16 Subtract w/ borrow 16-bit imm data to reg-ind w/ 16-bit offs            6 6
SUBB direct, #data8         Subtract w/ borrow 8-bit imm data to mem                                4 4
SUBB direct, #data16        Subtract w/ borrow 16-bit imm data to mem                               5 4

CPL Rd                      Complement (ones complement) reg                                        2 3

LSR Rd, Rs                  Logical right shift dest reg by the value in the src reg                2 a*
LSR Rd, #data4              Logical right shift reg by the 4-bit imm value                          2 a*
NORM Rd, Rs                 Logical shift left dest reg by the value in the src reg until MSB set   2 a*

OR Rd, Rs                   Logical OR regs                                                         2 3
OR Rd, [Rs]                 Logical OR reg-ind to reg                                               2 4
OR [Rd], Rs                 Logical OR reg to reg-ind                                               2 4
OR Rd, [Rs+offset8]         Logical OR reg-ind w/ 8-bit offs to reg                                 3 6
OR [Rd+offset8], Rs         Logical OR reg to reg-ind w/ 8-bit offs                                 3 6
OR Rd, [Rs+offset16]        Logical OR reg-ind w/ 16-bit offs to reg                                4 6
OR [Rd+offset16], Rs        Logical OR reg to reg-ind w/ 16-bit offs                                4 6
OR Rd, [Rs+]                Logical OR reg-ind w/ autoinc to reg                                    2 5
OR [Rd+], Rs                Logical OR reg-ind w/ autoinc to reg                                    2 5
OR direct, Rs               Logical OR reg to mem                                                   3 4
OR Rd, direct               Logical OR mem to reg                                                   3 4

OR Rd, #data8               Logical OR 8-bit imm data to reg                                        3 3
OR Rd, #data16              Logical OR 16-bit imm data to reg                                       4 3
OR [Rd], #data8             Logical OR 8-bit imm data to reg-ind                                    3 4
OR [Rd], #data16            Logical OR 16-bit imm data to reg-ind                                   4 4
OR [Rd+], #data8            Logical OR 8-bit imm data to reg-ind w/ autoinc                         3 5
OR [Rd+], #data16           Logical OR 16-bit imm data to reg-ind w/ autoinc                        4 5
OR [Rd+offset8], #data8     Logical OR 8-bit imm data to reg-ind w/ 8-bit offs                      4 6
OR [Rd+offset8], #data16    Logical OR 16-bit imm data to reg-ind w/ 8-bit offs                     5 6
OR [Rd+offset16], #data8    Logical OR 8-bit imm data to reg-ind w/ 16-bit offs                     5 6
OR [Rd+offset16], #data16   Logical OR 16-bit imm data to reg-ind w/ 16-bit offs                    6 6
OR direct, #data8           Logical OR 8-bit imm data to mem                                        4 4
OR direct, #data16          Logical OR 16-bit imm data to mem                                       5 4

RL Rd, #data4               Rotate left reg by the 4-bit imm value                                  2 a*

RLC Rd, #data4              Rotate left reg though carry by the 4-bit imm value                     2 a*

RR Rd, #data4               Rotate right reg by the 4-bit imm value                                 2 a*

RRC Rd, #data4              Rotate right reg though carry by the 4-bit imm value                    2 a*

XOR Rd, Rs                  Logical XOR regs                                                        2 3
XOR Rd, [Rs]                Logical XOR reg-ind to reg                                              2 4
XOR [Rd], Rs                Logical XOR reg to reg-ind                                              2 4
XOR Rd, [Rs+offset8]        Logical XOR reg-ind w/ 8-bit offs to reg                                3 6
XOR [Rd+offset8], Rs        Logical XOR reg to reg-ind w/ 8-bit offs                                3 6
XOR Rd, [Rs+offset16]       Logical XOR reg-ind w/ 16-bit offs to reg                               4 6
XOR [Rd+offset16], Rs       Logical XOR reg to reg-ind w/ 16-bit offs                               4 6
XOR Rd, [Rs+]               Logical XOR reg-ind w/ autoinc to reg                                   2 5
XOR [Rd+], Rs               Logical XOR reg-ind w/ autoinc to reg                                   2 5
XOR direct, Rs              Logical XOR reg to mem                                                  3 4
XOR Rd, direct              Logical XOR mem to reg                                                  3 4

XOR Rd, #data8              Logical XOR 8-bit imm data to reg                                       3 3
XOR Rd, #data16             Logical XOR 16-bit imm data to reg                                      4 3
XOR [Rd], #data8            Logical XOR 8-bit imm data to reg-ind                                   3 4
XOR [Rd], #data16           Logical XOR 16-bit imm data to reg-ind                                  4 4
XOR [Rd+], #data8           Logical XOR 8-bit imm data to reg-ind w/ autoinc                        3 5
XOR [Rd+], #data16          Logical XOR 16-bit imm data to reg-ind w/ autoinc                       4 5
XOR [Rd+offset8], #data8    Logical XOR 8-bit imm data to reg-ind w/ 8-bit offs                     4 6
XOR [Rd+offset8], #data16   Logical XOR 16-bit imm data to reg-ind w/ 8-bit offs                    5 6
XOR [Rd+offset16], #data8   Logical XOR 8-bit imm data to reg-ind w/ 16-bit offs                    5 6
XOR [Rd+offset16], #data16  Logical XOR 16-bit imm data to reg-ind w/ 16-bit offs                   6 6
XOR direct, #data8          Logical XOR 8-bit imm data to mem                                       4 4
XOR direct, #data16         Logical XOR 16-bit imm data to mem                                      5 4

MOV Rd, Rs                  Move reg to reg                                                         2 3
MOV Rd, [Rs]                Move reg-ind to reg                                                     2 3
MOV [Rd], Rs                Move reg to reg-ind                                                     2 3
MOV Rd, [Rs+offset8]        Move reg-ind w/ 8-bit offs to reg                                       3 5
MOV [Rd+offset8], Rs        Move reg to reg-ind w/ 8-bit offs                                       3 5
MOV Rd, [Rs+offset16]       Move reg-ind w/ 16-bit offs to reg                                      4 5
MOV [Rd+offset16], Rs       Move reg to reg-ind w/ 16-bit offs                                      4 5
MOV Rd, [Rs+]               Move reg-ind w/ autoinc to reg                                          2 4
MOV [Rd+], Rs               Move reg-ind w/ autoinc to reg                                          2 4
MOV direct, Rs              Move reg to mem                                                         3 4
MOV Rd, direct              Move mem to reg                                                         3 4
MOV [Rd+], [Rs+]            Move reg-ind to reg-ind, both pointers autoinc                          2 6
MOV direct, [Rs]            Move reg-ind to mem                                                     3 4
MOV [Rd], direct            Move mem to reg-ind                                                     3 4

MOV Rd, #data8              Move 8-bit imm data to reg                                              3 3
MOV Rd, #data16             Move 16-bit imm data to reg                                             4 3
MOV [Rd], #data8            Move 16-bit imm data to reg-ind                                         3 3
MOV [Rd], #data16           Move 16-bit imm data to reg-ind                                         4 3
MOV [Rd+], #data8           Move 8-bit imm data to reg-ind w/ autoinc                               3 4
MOV [Rd+], #data16          Move 16-bit imm data to reg-ind w/ autoinc                              4 4
MOV [Rd+offset8], #data8    Move 8-bit imm data to reg-ind w/ 8-bit offs                            4 5
MOV [Rd+offset8], #data16   Move 16-bit imm data to reg-ind w/ 8-bit offs                           5 5
MOV [Rd+offset16], #data8   Move 8-bit imm data to reg-ind w/ 16-bit offs                           5 5
MOV [Rd+offset16], #data16  Move 16-bit imm data to reg-ind w/ 16-bit offs                          6 5
MOV direct, #data8          Move 8-bit imm data to mem                                              4 3
MOV direct, #data16         Move 16-bit imm data to mem                                             5 3
MOV direct, direct          Move mem to mem                                                         4 4
MOV Rd, USP                 Move User Stack Pointer to reg (system mode only)                       2 3
MOV USP, Rs                 Move reg to User Stack Pointer (system mode only)                       2 3

MOVC Rd, [Rs+]              Move data from WS:Rs address of code mem to reg w/ autoinc              2 4
MOVC A, [A+DPTR]            Move data from code mem to the accumulator ind w/ DPTR                  2 6
MOVC A, [A+PC]              Move data from code mem to the accumulator ind w/ PC                    2 6

MOVS Rd, #data4             Move 4-bit sign-extended imm data to reg                                2 3
MOVS [Rd], #data4           Move 4-bit sign-extended imm data to reg-ind                            2 3
MOVS [Rd+], #data4          Move 4-bit sign-extended imm data to reg-ind w/ autoinc                 2 4
MOVS [Rd+offset8], #data4   Move reg-ind w/ 8-bit offs to 4-bit sign-extended imm data              3 5
MOVS [Rd+offset16], #data4  Move reg-ind w/ 16-bit offs to 4-bit sign-extended imm data             4 5
MOVS direct, #data4         Move 4-bit sign-extended imm data to mem                                3 3

MOVX Rd, [Rs]               Move external data from mem to reg                                      2 6
MOVX [Rd], Rs               Move external data from reg to mem                                      2 6

PUSH direct                 Push the mem content (b/w) onto the current stack                       3 5

PUSHU direct                Push the mem content (b/w) onto the user stack                          3 5

PUSH Rlist                  Push regs (b/w) onto the current stack                                  2 b*

PUSHU Rlist                 Push regs (b/w) from the user stack                                     2 b*

POP direct                  Pop the mem content (b/w) from the current stack                        3 5

POPU direct                 Pop the mem content (b/w) from the user stack                           3 5

POP Rlist                   Pop regs (b/w) from the current stack                                   2 c*

POPU Rlist                  Pop regs (b/w) from the user stack                                      2 c*

XCH Rd, Rs                  Exchange contents of two regs                                           2 5
XCH Rd, [Rs]                Exchange contents of a reg-ind address w/ a reg                         2 6
XCH Rd, direct              Exchange contents of mem w/ a reg                                       3 6

BCC rel8                    Branch if the carry flag is clear                                       2 6t/3nt   1111 0000  rrrr rrrr
BCS rel8                    Branch if the carry flag is set                                         2 6t/3nt   1111 0001  rrrr rrrr
BNE rel8                    Branch if the zero flag is not set                                      2 6t/3nt   1111 0010  rrrr rrrr
BEQ rel8                    Branch if the zero flag is set                                          2 6t/3nt   1111 0011  rrrr rrrr
BNV rel8                    Branch if overflow flag is clear                                        2 6t/3nt   1111 0100  rrrr rrrr
BOV rel8                    Branch if overflow flag is set                                          2 6t/3nt   1111 0101  rrrr rrrr
BPL rel8                    Branch if the negative flag is clear                                    2 6t/3nt   1111 0110  rrrr rrrr
BMI rel8                    Branch if the negative flag is set                                      2 6t/3nt   1111 0111  rrrr rrrr
BG rel8                     Branch if greater than (unsigned)                                       2 6t/3nt   1111 1000  rrrr rrrr
BL rel8                     Branch if less than or equal to (unsigned)                              2 6t/3nt   1111 1001  rrrr rrrr
BGE rel8                    Branch if greater than or equal to (signed)                             2 6t/3nt   1111 1010  rrrr rrrr
BLT rel8                    Branch if less than (signed)                                            2 6t/3nt   1111 1011  rrrr rrrr
BGT rel8                    Branch if greater than (signed)                                         2 6t/3nt   1111 1100  rrrr rrrr
BLE rel8                    Branch if less than or equal to (signed)                                2 6t/3nt   1111 1101  rrrr rrrr

BR rel8                     Short unconditional branch                                              2 6        1111 1110  rrrr rrrr

CALL [Rs]                   Subroutine call ind w/ a reg                                            2 8/5(PZ)  1100 0110  0000 0sss
CALL rel16                  Relative call (range +/- 64K)                                           3 7/4(PZ)  1100 0101  rrrr rrrr  rrrr rrrr

CJNE Rd,direct,rel8         Compare dir byte to reg and jump if not equal                           4 10t/7nt  1110 S010  dddd 0DDD  DDDD DDDD  rrrr rrrr
CJNE Rd,#data8,rel8         Compare imm byte to reg and jump if not equal                           4 9t/6nt   1110 0011  dddd 0000  rrrr rrrr  iiii iiii
CJNE Rd,#data16,rel8        Compare imm word to reg and jump if not equal                           5 9t/6nt   1110 1011  dddd 0000  rrrr rrrr  iiii iiii  iiii iiii
CJNE [Rd],#data8,rel8       Compare imm word to reg-ind and jump if not equal                       4 10t/7nt  1110 0011  0ddd 1000  rrrr rrrr  iiii iiii
CJNE [Rd],#data16,rel8      Compare imm word to reg-ind and jump if not equal                       5 10t/7nt  1110 1011  0ddd 1000  rrrr rrrr  iiii iiii  iiii iiii

DJNZ Rd,rel8                Decrement reg and jump if not zero                                      3 8t/5nt   1000 S111  dddd 1000  rrrr rrrr
DJNZ direct,rel8            Decrement mem and jump if not zero                                      4 9t/5nt   1110 S010  0000 1DDD  DDDD DDDD  rrrr rrrr

FCALL addr24                Far call (full 24-bit address space)                                    4 12/8(PZ) 1100 0100  aaaa aaaa  AAAA AAAA  AAAA AAAA

FJMP addr24                 Far jump (full 24-bit address space)                                    4 6        1101 0100  aaaa aaaa  AAAA AAAA  AAAA AAAA

JB bit,rel8                 Jump if bit set                                                         4 10t/6nt

JBC bit,rel8                Jump if bit set and then clear the bit                                  4 11t/7nt

JMP rel16                   Long unconditional branch                                               3 6
JMP [Rs]                    Jump ind to the address in the reg (64K)                                2 7
JMP [A+DPTR]                Jump ind relative to the DPTR                                           2 5
JMP [[Rs+]]                 Jump double-ind to the address (pointer to a pointer)                   2 8

JNB bit,rel8                Jump if bit not set                                                     4 10t/6nt
JNZ rel8                    Jump if accumulator not equal zero                                      2 6t/3nt
JZ rel8                     Jump if accumulator equals zero                                         2 6t/3nt

NOP                         No operation                                                            1 3

RET                         Return from subroutine                                                  2 8/6(PZ)

RETI                        Return from interrupt                                                   2 10/8(PZ)

CLR bit                     Clear bit                                                               3 4

MOV C, bit                  Move bit to the carry flag                                              3 4

MOV bit, C                  Move carry to bit                                                       3 4

ORL C, bit                  Logical OR a bit to carry                                               3 4
ORL C, /bit                 Logical OR complement of a bit to carry                                 3 4

SETB bit                    Sets the bit specified                                                  3 4

BKPT                        Cause the breakpoint trap to be executed.                               1 23/19(PZ)
RESET                       Causes a hardware Reset (same as external Reset)                        2 18
TRAP #data4                 Causes 1 of 16 hardware traps to be executed                            2 23/19(PZ)

*a) For 8 and 16 bit shifts, it is 4+1 per additional two bits. For 32-bit shifts, it is 6+1 per additional two bits.
*b) 3 clocks per reg pushed.
*c) 4 clocks for the first reg and two clocks for each additional reg.

*/

u32 xa_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t xa_disassembler::disassemble(std::ostream& stream, offs_t pc, const data_buffer& opcodes, const data_buffer& params)
{
	const u8 opcode = opcodes.r8(pc);

	switch (opcode)
	{
	default:
	{
		util::stream_format(stream, "unhandled (%02x)", opcode);
		break;
	}
	}

	return 1;
}

