// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*
   Taito Type-Zero hardware

   Driver by Ville Linde


Landing High Japan PCB info
===========================

Taito Landing High

Top board
    silkscreened        TYPE-ZERO MOTHER PCB

    stickered       298100308
                K11X0886A
                JC101

.5  29LV400BC-90        stamped E68-05-1
.6  29LV400BC-90        stamped E68-04-1
.24 PALV 18V8-10JC      stamped E68-06


IC30    Taito TCG020AGP
IC26    IDT7024 S35J V9928P
IC10    IBM EMPPC603eBG-100
IC53    ADV7120KP30 9926 F101764.1
IC16,17         M54256V32A-10
IC15,25,31,39,45,49,44  48D4811650GF-A10-9BT
IC27,36         D4564163G5-A10n-9JF
IC 41,46,42,47      D4516161AG5-A10B-9F
IC43            QSV991-7JRI
66.6667 Oscillator near IC43

Bottom board
    silkscreened        JC101 DAUGHTER PCB

    stickered       K91J0775A
                LANDING H.JAPAN

                299100308

                M43J0741A
                LANDING H.JAPAN

.14 27c1001     stickered   E82
                    03*

.15 27c1001     stickered   E82
                    04*

.44 PALCE16V8H  stamped     E82-01

.45 PALCE22V10H stamped     E82-02

IC40    Toshiba TMP95C063F
IC55    Panasonic MN89306
EPSON 9X5C oscillator near IC55
IC56    HY57V161610D TC-10
IC22    ID7133 SA70J
25.000 oscillator near IC22
IC11    Xilinx XC9572
IC5 HY5118164C JC-60
IC10    ZOOM ZSG-2
IC20    ZOOM ZFX 2 HD 96NE2VJ
IC26    TM TECH  UA4464V T224162B-28J
IC7 Panasonic MN1020819DA E68-01
20.000 oscillator near IC7



Power Shovel additional I/O PCB info
====================================

TMP95C063F
OKI 6295 x 2 each with a 1.056MHz resonator

6.2MHz OSC
18.4320MHz OSC

HIN239CB (+5v Powered RS-232 Transmitter/Receiver - 120kbps)
LC321664AM-80 (1Meg (65536 words x 16bits) DRAM)
74HC4040A (12-Stage Binary Ripple Counter)

E74-07.IC6 & E74-08.IC8 are the OKI samples and are identical
E74-06.IC2 is the TMP95C063 program code.



Rizing Ping Pong
Taito 2002

This game runs on Taito Type Zero hardware.

PCB Layout
----------

TYPE ZERO MOTHER PCB
K11X0878A
J1100365A
K11X0951A (Sticker)
|-----------------------------------------------------------|
|   D4811650 D4811650 D4811650 D4811650 D4811650  E87-02.IC6|
|   D4811650 D4811650 D4811650                              |
|                                                           |
|G                                     M54V25632            |
|                        |-----------|            E87-01.IC5|
|   AD8073               | TAITO     |                      |
|                        | TCG020AGP | M54V25632            |
|        HY57V161610     |           |                      |
|   ADV7120KP30          |           |                      |
|        HY57V161610     |           |                      |
|        HY57V161610     |-----------|                      |
|        HY57V161610      V54C365164                        |
|                         V54C365164                        |
|                                                           |
|                                PAL16V8                    |
|               QS5V991          (E68-06)                   |
|            66.6667MHz                                     |
|                                  |-------|M51955A         |
|                                  |       |      |-------| |
|                                  |IDT7024|      |PPC603E| |
|                                  |       |      |       | |
|                                  |-------|      |-------| |
|                                                           |
|-----------------------------------------------------------|


TYPE ZERO DAUGHTER PCB
K9100745A
J9100491A
RIZINGPINGPONG K91J0905A (Sticker)
RIZINGPINGPONG M43J0775A (Sticker)
|-----------------------------------------------------------|
|RESET_SW  5.5V        RTC64613                             |
|          SUPERCAP  MB3790               F14-02.IC15       |
|DS14C239              LC3564                               |
|                                         F14-01.IC14       |
|        IDC40       LC321664                               |
|TD62064                                               20MHz|
|          TOSHIBA            IDT7133         MN1020819     |
|          TMP95C063                          (E68-01)      |
|                     PAL16V8                               |
|                     (E68-03)  25MHz XC9572                |
|                                             HY5118164     |
|             3.686MHz                                      |
|                                                           |
|C        SN74S1057                                         |
|                                                           |
|                                                           |
|                                 ZOOM      ZOOM            |
|                      V53C16258  ZFX-2     ZSG-2           |
|                                                           |
|                 MC3418          GM71C17403  GM71C17403    |
|           LA2650     LC78834M                             |
|                                 GM71C17403  GM71C17403    |
|                                                           |
|-----------------------------------------------------------|
Notes:
      Connectors C (daughter) and G (main) join to filter board
      IDC40 - IDE HDD connector for Quantum Fireball LCT 4.3AT LA04A011 Rev 01-A
              Serial number 691934013492 DGPXX
              Sticker on HDD reads.....
              4.3GB QUANTUM
              QML04300LA-A
              RIZING PING PONG EXP
              M9005557A VER. 2.01O  (note '01O' is slightly scratched and could be incorrect)


*/

#include "emu.h"
#include "bus/ata/ataintf.h"
#include "bus/ata/hdd.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/tlcs900/tmp95c063.h"
#include "machine/nvram.h"
#include "video/poly.h"
#include "screen.h"

#define LOG_PPC_TO_TLCS_COMMANDS	(1U << 1)
#define LOG_TLCS_TO_PPC_COMMANDS	(1U << 2)
#define LOG_VIDEO_REG_1_RD			(1U << 3)
#define LOG_VIDEO_REG_2_RD			(1U << 4)
#define LOG_VIDEO_REG_UNK_RD		(1U << 5)
#define LOG_VIDEO_REG_1_WR			(1U << 6)
#define LOG_VIDEO_REG_2_WR			(1U << 7)
#define LOG_VIDEO_REG_UNK_WR		(1U << 8)
#define LOG_VIDEO_CHIP_UNK_RD		(1U << 9)
#define LOG_VIDEO_CHIP_UNK_WR		(1U << 10)
#define LOG_DIRECT_FIFO				(1U << 11)
#define LOG_TNL_FIFO				(1U << 12)
#define LOG_RTC_UNK_RD				(1U << 13)
#define LOG_RTC_UNK_WR				(1U << 14)
#define LOG_VIDEO_MEM_UNK_RD		(1U << 15)
#define LOG_VIDEO_MEM_UNK_WR		(1U << 16)

#define VERBOSE (0)

#include "logmacro.h"


/*
    Interesting mem areas

    0x40080400...0x400807fc: PPC Interrupt handler table for TLCS900 interrupts (called from 0x4002ea80)
        0x40080440 (0x10): 0x4003b274       (0xfc06f7 on TLCS sets 0x1000)
        0x400804c0 (0x30): 0x4003f1e4       (INT1 handler on TLCS sets 0x3010)  (comms, could be IEEE1394)
        0x40080500 (0x40): 0x4002ed94       (debug/trace)
        0x40080504 (0x41): 0x4002ede4       (debug/break)
        0x40080540 (0x50): 0x4003c21c                                                           0x5001: sets 0x00000001 in 0x40049c58
        0x40080740 (0xd0): 0x4002ecc0       (INT3 handler on TLCS sets 0xd000)  (VBlank?)
        0x400807c0 (0xf0): 0x4002eae4       (0xfc0a44 on TLCS sets 0xf001)

    PPC -> TLCS Commands:
        0x1000:            0x4003a9e0()                             HDD Reset
        0x1001:            0x4003aa14()                             HDD Init. Writes HDD status and error to io_shared[0x1a00].
        0x1010:            0x4003aaf4(), 0x4003ab8c()               HDD Read Sector
        0x1020:            0x4003ac1c()                             HDD Write Sector
        0x4000:            ?                                        Related to sound loading?
        0x4001:            0x400429b0()                             MBox related
        0x4002:            ?                                        Loads a byte from MBox ram. Address in io_shared[0x1c04]
        0x4003:            ?                                        Writes a byte into MBox ram. Address in io_shared[0x1c04], data in io_shared[0x1c06].
        0x4004:            0x40042bc4(), 0x40042c50()               Play music/sound? Params in io_shared[0x1c08...0x1c12]
        0x5000:            0x4003bc68()                             Reset RTC?
        0x5010:            0x4003bb68()                             Read RTC
        0x5020:            0x4003bbec()                             Write RTC
        0x6000:            ?                                        Backup RAM Read. Address in io_shared[0x1d00].
        0x6010:            ?                                        Backup RAM Write. Address in io_shared[0x1d00].
        0x7000:            0x4003d1c4()
        0x7001:            0x4003d28c()
        0x7002:            0x4003d4ac()
        0x7003:            0x4003d508()
        0x7004:            0x4003d554()
        0x7005:            0x4003d168()
        0x8000:            ?                                        Used by vibration (force feedback?) on pwrshovl
        0x9100:            ?                                        Dendego3 speedometer and brake meter. io_shared[0x1c3c] = speed, io_shared[0x1c3e] = brake
        0xa000:            ?                                        Used by vibration (force feedback?) on pwrshovl
        0xf000:            0x4002f328() TLCS_Init
        0xf010:            0x4002f074()                             Enables TLCS watchdog timer
        0xf011:            0x4002f0a8()                             Disables TLCS watchdog
        0xf020:            ?
        0xf055:            0x4002f0dc()                             Enables TLCS watchdog and infloops the TLCS
        0xf0ff:            ?                                        TLCS soft reset?

    TLCS -> PPC Commands:
        0x10xx:            0xfc06f7()                               HDD Command ack?
                                                                    0x40094ec8:
                                                                        0x00:   IDLE
                                                                        0x10:   READ
                                                                        0x11:   READ SECTOR
                                                                        0x12:   READ ALL
                                                                        0x20:   WRITE
                                                                        0x21:   WRITE SECTOR
                                                                        0x22:   WRITE ALL
                                                                        0x30:   VENDER
                                                                        0xff:   ERROR
                                                                        0x??:   UNKNOWN
        0x3010:            0xfc071c() int1 handler                  Comm IRQ, IEEE1394
        0x40xx:            ?                                        Debug
        0x41xx:            ?                                        Debug
        0x5001:            ?                                        RTC?
        0xd000:            0xfc06b4() int3 handler                  VBlank? Runs a counter on the PPC side
        0xf001:            0xfc0a44()                               PPC sync?


    BG2 intro packet writes:

    construction?
    0x4012f390()
        -> 0x401304f0() scene graph parser?
            -> 0x40130d60()
                -> 0x401aff48()     R3 = dest, R4 = source, R5 = length

    0x40174674()
    0x401ba294()
                            R3 = 0x000007d0, R4 = 0x40707bac, R5 = 0x00000398

                            f1 00 00 63 0c 00 00 7f 0c 00 00 00 08 00 00 00
                            00 40 01 3e 00 00 81 00 00 00 7f 00 00 7f 00 00
                            00 40 3e 3e 00 00 7f 00 00 00 7f 00 00 7f 00 00
                            00 40 3e 00 00 00 7f 00 00 00 81 00 00 7f 00 00
                            00 40 01 00 00 00 81 00 00 00 81 00 00 7f 00 00
                            f1 00 00 69 00 00 7f 00 0c 7f 00 00 0d 00 00 7f
                            00 40 00 3f 00 00 e0 40 00 00 00 00 00 7f 7f 00
                            00 40 00 00 00 00 e0 40 00 00 00 00 00 7f 81 00
                            00 40 1f 00 00 00 1f c0 00 00 00 00 00 7f 81 00
                            00 40 1f 3f 00 00 1f c0 00 00 00 00 00 7f 7f 00
                            ...

                            R3 = 0x00000b68, R4 = 0x40707e58, R5 = 0x00001000

                            e1 00 00 73 00 81 00 00 04 00 00 00 0c 00 00 00
                            00 40 06 00 00 94 fd d1 00 d2 82 ce 00 2e 03 ef
                            00 40 06 00 00 99 fd d1 00 c0 82 ce 00 df 00 40
                            00 40 06 02 00 91 fd d1 00 ef 88 9e 00 c7 fc 40
                            00 40 06 1f 00 88 fd d1 00 00 b8 60 00 26 03 ef
                            e0 00 00 73 00 81 00 00 04 00 00 00 0c 00 00 00
                            00 40 06 1f 00 8b fd d1 00 00 b8 60 00 d0 fc 40
                            00 40 06 1f 00 88 fd d1 00 00 b8 60 00 26 03 ef
                            00 40 06 02 00 91 fd d1 00 ef 88 9e 00 c7 fc 40
                            ...

                            R3 = 0x00001b68, R4 = 0x40707e58, R5 = 0x00000d19
                            R3 = 0x00075770, R4 = 0x40443e2c, R5 = 0x00000014
                            R3 = 0x00075784, R4 = 0x40443e7c, R5 = 0x00000014
                            R3 = 0x00075798, R4 = 0x40443ecc, R5 = 0x00000014
                            R3 = 0x000757ac, R4 = 0x40443f1c, R5 = 0x00000014
                            R3 = 0x000757c0, R4 = 0x40443e6c, R5 = 0x00000014

                            R3 = 0x00062ca0, R4 = 0x409cfa70, R5 = 0x000001e0
                            R3 = 0x00008ca0, R4 = 0x409f8f70, R5 = 0x000002c5
                            R3 = 0x00002ba4, R4 = 0x40fffaf0, R5 = 0x00000028
                            R3 = 0x0001f4a0, R4 = 0x40a06350, R5 = 0x00000340
                            R3 = 0x00002bcc, R4 = 0x40fffaf0, R5 = 0x00000028

                            R3 = 0x00000190, R4 = 0x40fff8f8, R5 = 0x00000014

    0x40188514()
                            R3 = 0x0000d818, R4 = 0x40ffcce0, R5 = 0x00000bc0

                            00 00 00 dd 00 00 00 00 00 00 00 00 00 00 00 00
                            00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                            00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                            00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                            ...

                            R3 = 0x0001494c, R4 = 0x40ffcce0, R5 = 0x00000bc0

    0x40116c28():       Draws 2D images
                            R3 = 0x009e0c7a, R4 = 0x40a7c3bc, R5 = 0x0000007c

    0x40116a80():       Clears 2D images?
                            R3 = 0x009f6308, R4 = 0x40a9a784, R5 = 0x000000f0



        -> 0x40116430():    R3 = dest (|= 0x30800000), R4 = source, R5 = num words
            -> 0x401162ac() WriteVideoMem:      R3 = source, R4 = dest, R5 = num words
                -> 0x401064b0() Write video data using FIFO:        R3 = source, R4 = dest, R5 = num words (must be divisible by 8)
                -> 0x40106668() Write video data using write port:  R3 = source, R4 = dest, R5 = num words



    0x40106788(): FIFO Packet write:            R3 = source, R4 = select between cmd 0 and 1, R5 = num words (must be divisible by 8)



    BG2: 4x4 Matrices at 0x40223218
            word 0: ?? (0x62e6c, 0xf20a0, ...)      anded by 0x1fffff on read, or'd by 0xff800000, or'd by 0x400000 if no matrix, store -> [0x00]
            word 1: ?? (usually zero)
            word 2: float (often 1.0)               multiplied by 31.0f, converted to (int), anded by 0x1f, or by 0x80 if word 1 == 1, store -> [0x08]
            word 3: float                           if (exponent < 1, mult by 0.5), store -> [0x04]
            word 4: usually 0x00000001              if 0, matrix not stored
            16x float: 4x4 matrix
                matrix[0][0] -> store [0x10]
                matrix[1][0] -> store [0x14]
                matrix[2][0] -> store [0x18]
                matrix[3][0] -> store [0x1c]
                ...
            word 0: Pointer to another matrix
            word 1: Pointer to another matrix

        0x401afffc(): Write

    0x40118af0():
        -> 0x401195e0(): Read   (Transposes matrix, only columns 0-2 stored + 4 extra command words)

        -> 0x40118080: Read, store to [0x40362314+], never read


    Display list top?:      0x403408c0 (pointer to at 0x4020b80c)
                 bottom?:   0x403608e4 (pointer to at 0x4020b81c)

    0x40123fcc():
        -> 0x401186c8():    Display list parser?

    0x40117fb4():   Insert command to disp list. R3 = command word

    Display list format
    -------------------
        0xffff0010: ?

        0xffff0011: Draw Polygon Object
                    Word 0: Polygon data address (21 bits) in screen RAM (in dwords). If 0x400000 set, no matrix.
                    Word 1: Object scale (float), if <1.0f multiply scale by 2.0f
                    Word 2: 0x80 = object alpha enable, 0x1f = object alpha (0x1f = opaque)
                    Word 3: ?
                    Words 4...15: 4x3 matrix

        0xffff0020: ? (no parameters)

        0xffff0021: Draw Pretransformed Polygon (used for drawing the background images)
                    Word 0: Address? (if 0x400 set, 24 words)
                    Word 1: Texture number (* 0x4000 to address the texture)
                    Word 2: ?
                    Word 3: ? (usually 0)

        Vertex data: 4 or 5 groups of 4 words
                     Word 0: XY coordinates. X in high 16 bits, Y in low 16 bits.
                     Word 1: Z value?
                     Word 2: UV coordinates. U in high 16 bits, V in low 16 bits (possibly shifted left by 4?)
                     Word 3: ? (usually 0)

        0xffff0022: ? (1 word)
                    Word 0: ? (Set to 0x00000001 before drawing the background. Z-buffer read disable?)

        0xffff0030: ? (2 + n words) FIFO write?
                    Word 0: ?
                    Word 1: Word count
                    Words 2...n: ?

        0xffff0080: Display List Call?

        0xffff00ff: End of List?

    Polygon data format
    -------------------

    Word 0:
                    ---x---- -------- -------- --------     1: this is the last polygon of the model
                    ------x- -------- -------- --------     ?
                    -------x -------- -------- --------     0: triangle, 1: quad
                    -------- -------- -----xxx xxxxxxxx     Texture number (* 0x4000 to address the texture)

    Word 1:
                    x------- -------- -------- --------     ?
                    ----xx-- -------- -------- --------     ?
                    -------- xxxxxxxx -------- --------     Polygon Normal X (signed 1.7 fixed point)
                    -------- -------- xxxxxxxx --------     Polygon Normal Y (signed 1.7 fixed point)
                    -------- -------- -------- xxxxxxxx     Polygon Normal Z (signed 1.7 fixed point)

    Word 2:
                    x------- -------- -------- --------     ?
                    ----x--- -------- -------- --------     ?
                    -----x-- -------- -------- --------     ?
                    ------x- -------- -------- --------     Enable environment mapping?
                    -------x -------- -------- --------     ?
                    -------- xxxxxxxx -------- --------     Tangent X (signed 1.7 fixed point)
                    -------- -------- xxxxxxxx --------     Tangent Y (signed 1.7 fixed point)
                    -------- -------- -------- xxxxxxxx     Tangent Z (signed 1.7 fixed point)

    Word 3:
                    x------- -------- -------- --------     ?
                    ----xx-- -------- -------- --------     Tex mode? (more than 2 bits?)
                                                            00: Tex 0: color, Tex 1: color with pre-lighting (used by BG2 attract mode trackpieces)
                                                            01: Tex 0: alpha mask?, Tex 1: inverse alpha? (used by car shadow)
                                                            10: Tex 0: alpha mask, Tex 1: color (additive alpha, used by headlights, etc.)
                                                            11: Tex 0: color, Tex 1: environment map? (used by cars)
                    -------x -------- -------- --------     ?
                    -------- xxxxxxxx -------- --------     Bi-normal X (signed 1.7 fixed point)
                    -------- -------- xxxxxxxx --------     Bi-normal Y (signed 1.7 fixed point)
                    -------- -------- -------- xxxxxxxx     Bi-normal Z (signed 1.7 fixed point)

                    e1000096 0c43cca2 02000000 0c000000     Car model (with env map?)
                    e100009a 007f0000 000083ea 0c001784     Part with bump-mapping
                    f1000068 0c007f00 0c7f0000 0500007f     Car shadow
                    e100006a 0c00007f 0c000000 08000000     Light
                    e1000127 0c00007f 0c000000 08000000     Fireworks (alpha mask?)
                    e1000185 0cee7d03 0c000000 00000000     Trackpiece
                    f1000068 0c007f00 0c7e0000 0400007f     Car shadow (BG2 0x1f160)

    Vertex data: 3x (triangle) or 4x (quad) 4 words
    Word 0:
                    -------- -------- -------- xxxxxxxx     Texture V coordinate (0..63 with max 4x repeat)
                    -------- -------- xxxxxxxx --------     Texture U coordinate (0..63 with max 4x repeat)
                    -------- xxxxxxxx -------- --------     Usually 0x40. HUD elements set 0xff. Emissive?
                    xxxxxxxx -------- -------- --------     ? (seen 0x00, 0x83, 0x40)

    Word 1:
                    -------- -------- xxxxxxxx xxxxxxxx     Vertex X coordinate (signed 8.8 fixed point)
                    -------- xxxxxxxx -------- --------     Normal X (signed 1.7 fixed point)
                    xxxxxxxx -------- -------- --------     ? (seen 0x00, 0x83, 0x40)

    Word 2:
                    -------- -------- xxxxxxxx xxxxxxxx     Vertex Y coordinate (signed 8.8 fixed point)
                    -------- xxxxxxxx -------- --------     Normal Y (signed 1.7 fixed point)
                    xxxxxxxx -------- -------- --------     ? (seen 0x00, 0x83, 0x40)

    Word 3:
                    -------- -------- xxxxxxxx xxxxxxxx     Vertex Z coordinate (signed 8.8 fixed point)
                    -------- xxxxxxxx -------- --------     Normal Z (signed 1.7 fixed point)
                    xxxxxxxx -------- -------- --------     ? (seen 0x00, 0x83, 0x40)

    3D registers
    ------------

    0x00000100:     xxxxxxxx xxxxxxxx -------- --------     Viewport Y?
                    -------- -------- xxxxxxxx xxxxxxxx     Viewport X?

    0x00000101:     xxxxxxxx xxxxxxxx -------- --------     Viewport center X? (usually 255 or 240)
                    -------- -------- xxxxxxxx xxxxxxxx     Viewport center Y? (usually 200 or 191)

    0x00000102:     xxxxxxxx xxxxxxxx -------- --------     ? (usually 0x100)
                    -------- -------- xxxxxxxx xxxxxxxx     ? (usually 0x100 or 0xc0)

    0x00000103:

    0x00000104:

    0x10000100:     -------x xxxxxxxx -------- --------     \ Screen space light vector? (changes during camera movement)
                    -------- -------- -------x xxxxxxxx     / (int)(N * 127.0f)

    0x10000101:     -------- -------x -------- --------     ?
                    -------- -------- -------- -xxxxxxx     ? (int)(N * 127.0f)

    0x10000102:     xxxxxxxx xxxxxxxx -------- --------     Diffuse light color (ARGB1555)
                    -------- -------- xxxxxxxx xxxxxxxx     Ambient light color (ARGB1555)

    0x10000103:     x------- -------- -------- --------     ?
                    -xxxxxxx xxxxxxxx -------- --------     ?
                    -------- -------- ---xxxxx --------     \ ? converted from floats to int 0..31
                    -------- -------- -------- ---xxxxx     /

    0x10000104:

    0x10000105:
*/


namespace {

#define PPC_TLCS_COMM_TRIGGER           12345
#define TLCS_PPC_COMM_TRIGGER           12346

struct PLANE
{
	float x, y, z, d;
};

typedef float VECTOR3[3];

struct taitotz_polydata
{
	u32 texture;
	u32 alpha;
	u32 flags;
	int diffuse_r, diffuse_g, diffuse_b;
	int ambient_r, ambient_g, ambient_b;
	int specular_r, specular_g, specular_b;
	VECTOR3 light;
};

class taitotz_renderer;

class taitotz_state : public driver_device
{
public:
	taitotz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_screen(*this, "screen"),
		m_maincpu(*this, "maincpu"),
		m_iocpu(*this, "iocpu"),
		m_work_ram(*this, "work_ram"),
		m_mbox_ram(*this, "mbox_ram"),
		m_ata(*this, "ata"),
		m_scr_base(0),
		m_hdd_serial_number(nullptr)
	{
	}

	void taitotz(machine_config &config);
	void landhigh(machine_config &config);

	void init_batlgr2a();
	void init_batlgr2();
	void init_pwrshovl();
	void init_batlgear();
	void init_dendego3();
	void init_landhigh();
	void init_landhigha();
	void init_raizpin();
	void init_raizpinj();
	void init_styphp();

	required_device<screen_device> m_screen;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	static constexpr bool ENABLE_DEBUG_PRINTS = false;
	static constexpr bool ENABLE_DEBUG_RAM = false;

	void landhigh_tlcs900h_mem(address_map &map) ATTR_COLD;
	void ppc603e_mem(address_map &map) ATTR_COLD;
	void tlcs900h_mem(address_map &map) ATTR_COLD;

	u64 ppc_common_r(offs_t offset, u64 mem_mask = ~0);
	void ppc_common_w(offs_t offset, u64 data, u64 mem_mask = ~0);
	u64 ieee1394_r(offs_t offset, u64 mem_mask = ~0);
	void ieee1394_w(offs_t offset, u64 data, u64 mem_mask = ~0);

	void dump_video_ram();
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_tile(u32 pos, u32 tile);
	void vblank(int state);
	u64 video_chip_r(offs_t offset, u64 mem_mask = ~0);
	void video_chip_w(offs_t offset, u64 data, u64 mem_mask = ~0);
	u64 video_fifo_r(offs_t offset, u64 mem_mask = ~0);
	void video_fifo_w(offs_t offset, u64 data, u64 mem_mask = ~0);
	u32 video_mem_r(u32 address);
	void video_mem_w(u32 address, u32 data);
	u32 video_reg_r(u32 reg);
	void video_reg_w(u32 reg, u32 data);

	u16 tlcs_ide0_r(offs_t offset, u16 mem_mask = ~0);
	u16 tlcs_ide1_r(offs_t offset, u16 mem_mask = ~0);
	u8 tlcs_common_r(offs_t offset);
	void tlcs_common_w(offs_t offset, u8 data);
	u8 tlcs_rtc_r(offs_t offset);
	void tlcs_rtc_w(offs_t offset, u8 data);
	void ide_interrupt(int state);

	void init_taitotz_152();
	void init_taitotz_111a();

	required_device<ppc_device> m_maincpu;
	required_device<tmp95c063_device> m_iocpu;
	required_shared_ptr<u64> m_work_ram;
	required_shared_ptr<u16> m_mbox_ram;
	required_device<ata_interface_device> m_ata;

	std::unique_ptr<u32[]> m_screen_ram;
	std::unique_ptr<u32[]> m_frame_ram;
	std::unique_ptr<u32[]> m_texture_ram;
	u32 m_video_unk_reg[0x10]{};

	u32 m_video_fifo_ptr = 0;
	u32 m_video_ram_ptr = 0;
	u32 m_video_reg = 0;
	u32 m_scr_base = 0;

	u16 m_io_share_ram[0x2000]{};

	const char *m_hdd_serial_number = nullptr;

	u8 m_rtcdata[8]{};

	u32 m_reg105 = 0;

	std::unique_ptr<taitotz_renderer> m_renderer;
};

class taitotz_renderer : public poly_manager<float, taitotz_polydata, 6>
{
public:
	taitotz_renderer(taitotz_state &state, int width, int height, u32 *scrram, u32 *texram)
		: poly_manager<float, taitotz_polydata, 6>(state.machine()),
			m_state(state)
	{
		m_fb = std::make_unique<bitmap_rgb32>(width, height);
		m_zbuffer = std::make_unique<bitmap_ind32>(width, height);
		m_texture = texram;
		m_screen_ram = scrram;

		m_diffuse_intensity = 224;
		m_ambient_intensity = 32;
		m_specular_intensity = 256;
		m_specular_power = 20;

		m_cliprect = m_state.m_screen->visible_area();

		setup_viewport(0, 0, 256, 192, 256, 192);
	}

	void render_displaylist(const rectangle &cliprect);
	void draw_object(u32 address, float scale, u8 alpha);
	float line_plane_intersection(const vertex_t *v1, const vertex_t *v2, PLANE cp);
	int clip_polygon(const vertex_t *v, int num_vertices, PLANE cp, vertex_t *vout);
	void setup_viewport(int x, int y, int width, int height, int center_x, int center_y);
	void draw_scanline_noz(s32 scanline, const extent_t &extent, const taitotz_polydata &extradata, int threadid);
	void draw_scanline(s32 scanline, const extent_t &extent, const taitotz_polydata &extradata, int threadid);

	void push_tnl_fifo(u32 data);
	void push_direct_poly_fifo(u32 data);

	void render_tnl_object(u32 address, float scale, u8 alpha);

	void draw(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	int direct_fifo_ptr() const { return m_direct_fifo_ptr; }

	static float dot_product_vec3(VECTOR3 a, const VECTOR3 b);
	static float clamp_pos(float v);
	static u32 generate_texel_address(int iu, int iv);

private:
	enum
	{
		POLY_Z      = 0,
		POLY_U      = 1,
		POLY_V      = 2,
		POLY_NX     = 3,
		POLY_NY     = 4,
		POLY_NZ     = 5
	};

	static constexpr bool ENABLE_LIGHTING = true;

	taitotz_state &m_state;
	std::unique_ptr<bitmap_rgb32> m_fb;
	std::unique_ptr<bitmap_ind32> m_zbuffer;
	u32 *m_texture = nullptr;
	u32 *m_screen_ram = nullptr;

	rectangle m_cliprect;

	PLANE m_clip_plane[6]{};
	float m_matrix[4][3]{};

	float m_diffuse_intensity = 0;
	float m_ambient_intensity = 0;
	float m_specular_intensity = 0;
	float m_specular_power = 0;

	int m_ambient_r = 0;
	int m_ambient_g = 0;
	int m_ambient_b = 0;
	int m_diffuse_r = 0;
	int m_diffuse_g = 0;
	int m_diffuse_b = 0;
	int m_specular_r = 0;
	int m_specular_g = 0;
	int m_specular_b = 0;

	float m_vp_center_x = 0;
	float m_vp_center_y = 0;
	float m_vp_focus = 0;
	float m_vp_x = 0;
	float m_vp_y = 0;
	float m_vp_mul = 0;

	u32 m_reg_100 = 0;
	u32 m_reg_101 = 0;
	u32 m_reg_102 = 0;

	u32 m_reg_10000100 = 0;
	u32 m_reg_10000101 = 0;

	u32 m_tnl_fifo[64]{};
	u32 m_direct_fifo[64]{};
	int m_tnl_fifo_ptr = 0;
	int m_direct_fifo_ptr = 0;

	static const float DOT3_TEX_TABLE[32];
};

void taitotz_state::dump_video_ram()
{
	FILE *file = fopen("screen_ram.bin","wb");
	for (int i = 0; i < 0x200000; i++)
	{
		fputc((u8)(m_screen_ram[i] >> 24), file);
		fputc((u8)(m_screen_ram[i] >> 16), file);
		fputc((u8)(m_screen_ram[i] >> 8), file);
		fputc((u8)(m_screen_ram[i] >> 0), file);
	}
	fclose(file);

	file = fopen("frame_ram.bin","wb");
	for (int i = 0; i < 0x80000; i++)
	{
		fputc((u8)(m_frame_ram[i] >> 24), file);
		fputc((u8)(m_frame_ram[i] >> 16), file);
		fputc((u8)(m_frame_ram[i] >> 8), file);
		fputc((u8)(m_frame_ram[i] >> 0), file);
	}
	fclose(file);

	file = fopen("texture_ram.bin","wb");
	for (int i = 0; i < 0x800000; i++)
	{
		fputc((u8)(m_texture_ram[i] >> 24), file);
		fputc((u8)(m_texture_ram[i] >> 16), file);
		fputc((u8)(m_texture_ram[i] >> 8), file);
		fputc((u8)(m_texture_ram[i] >> 0), file);
	}
	fclose(file);
}

void taitotz_state::video_start()
{
	int width = m_screen->width();
	int height = m_screen->height();

	m_screen_ram = std::make_unique<u32[]>(0x200000);
	m_frame_ram = std::make_unique<u32[]>(0x80000);
	m_texture_ram = std::make_unique<u32[]>(0x800000);

	m_renderer = std::make_unique<taitotz_renderer>(*this, width, height, m_screen_ram.get(), m_texture_ram.get());

	if (ENABLE_DEBUG_RAM)
	{
		machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&taitotz_state::dump_video_ram, this));
	}

	m_video_reg = 0;
}

const float taitotz_renderer::DOT3_TEX_TABLE[32] =
{
	-0.500000f, -0.466666f, -0.433333f, -0.400000f, -0.366666f, -0.333333f, -0.300000f, -0.266666f,
	-0.233333f, -0.200000f, -0.166666f, -0.133333f, -0.100000f, -0.066666f, -0.033333f, -0.000000f,
	 0.000000f,  0.033333f,  0.066666f,  0.100000f,  0.133333f,  0.166666f,  0.200000f,  0.233333f,
	 0.266666f,  0.300000f,  0.333333f,  0.366666f,  0.400000f,  0.433333f,  0.466666f,  0.500000f,
};

float taitotz_renderer::dot_product_vec3(VECTOR3 a, const VECTOR3 b)
{
	return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
}

float taitotz_renderer::clamp_pos(float v)
{
	if (v < 0.0f)
		return 0.0f;
	else
		return v;
}

u32 taitotz_renderer::generate_texel_address(int iu, int iv)
{
	// generate texel address from U and V
	u32 addr = 0;
	addr += (iu & 0x01) ? 1 : 0;
	addr += (iu >> 1) * 4;
	addr += (iv & 0x01) ? 2 : 0;
	addr += (iv >> 1) * 128;

	return addr;
}

void taitotz_renderer::draw_scanline_noz(s32 scanline, const extent_t &extent, const taitotz_polydata &extradata, int threadid)
{
	u32 *const fb = &m_fb->pix(scanline);

	float u = extent.param[POLY_U].start;
	float v = extent.param[POLY_V].start;
	float const du = extent.param[POLY_U].dpdx;
	float const dv = extent.param[POLY_V].dpdx;

	u32 *texram = &m_texture[extradata.texture * 0x1000];

	int shift = 16;     // TODO: subtexture

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		int iu = (int)(u) & 0x3f;
		int iv = (int)(v) & 0x3f;

		u32 addr = generate_texel_address(iu, iv);

		u32 texel = (texram[addr] >> shift) & 0xffff;
		if (!(texel & 0x8000))
		{
			int r = (texel & 0x7c00) << 9;
			int g = (texel & 0x03e0) << 6;
			int b = (texel & 0x001f) << 3;
			fb[x] = 0xff000000 | r | g | b;
		}

		u += du;
		v += dv;
	}
}

void taitotz_renderer::draw_scanline(s32 scanline, const extent_t &extent, const taitotz_polydata &extradata, int threadid)
{
	u32 *const fb = &m_fb->pix(scanline);
	float *const zb = (float*)&m_zbuffer->pix(scanline);

	float ooz = extent.param[POLY_Z].start;
	float uoz = extent.param[POLY_U].start;
	float voz = extent.param[POLY_V].start;
	const float dooz = extent.param[POLY_Z].dpdx;
	const float duoz = extent.param[POLY_U].dpdx;
	const float dvoz = extent.param[POLY_V].dpdx;

	float nx = extent.param[POLY_NX].start;
	float ny = extent.param[POLY_NY].start;
	float nz = extent.param[POLY_NZ].start;
	const float dnx = extent.param[POLY_NX].dpdx;
	const float dny = extent.param[POLY_NY].dpdx;
	const float dnz = extent.param[POLY_NZ].dpdx;

	u32 *const texram = &m_texture[extradata.texture * 0x1000];
	u32 alpha = extradata.alpha & 0x1f;
	u32 alpha_enable = extradata.alpha & 0x80;

	VECTOR3 view = { -0.0f, -0.0f, -1.0f };

	// TODO: these might get swapped around, the color texture doesn't always seem to be tex0
	static constexpr int TEX_SHIFTS[4][2] = { { 16, 0 }, { 16, 0 }, { 16, 0 }, { 0, 16 } };
	int texmode = extradata.flags & 3;
	int tex0_shift = TEX_SHIFTS[texmode][0];
	int tex1_shift = TEX_SHIFTS[texmode][1];

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		if (ooz > zb[x])
		{
			float z = 1.0f / ooz;
			float u = uoz * z;
			float v = voz * z;
			int iu = (int)(u) & 0x3f;
			int iv = (int)(v) & 0x3f;

			u32 addr = generate_texel_address(iu, iv);

			u32 texel = texram[addr];
			u32 texel0 = (texel >> tex0_shift) & 0xffff;
			if (!(texel0 & 0x8000))
			{
				// extract texel0 RGB
				int r0 = (texel0 & 0x7c00) >> 7;
				int g0 = (texel0 & 0x03e0) >> 2;
				int b0 = (texel0 & 0x001f) << 3;

				if (ENABLE_LIGHTING)
				{
					// fetch texture1 and apply normal map
					u32 texel1 = (texel >> tex1_shift) & 0xffff;

					const float bumpx = DOT3_TEX_TABLE[(texel1 & 0x7c00) >> 10];
					const float bumpy = DOT3_TEX_TABLE[(texel1 & 0x03e0) >>  5];
					const float bumpz = DOT3_TEX_TABLE[(texel1 & 0x001f)];

					// FIXME!!! the normal needs to be in tangent-space so the bump normal can be applied!!!
					VECTOR3 normal;
					normal[0] = nx + bumpx;
					normal[1] = ny + bumpy;
					normal[2] = nz + bumpz;

					// normalize
					float l = 1.0f / sqrtf(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
					normal[0] *= l;
					normal[1] *= l;
					normal[2] *= l;

					// calculate per-pixel lighting
					float dot = dot_product_vec3(normal, extradata.light);

					// calculate half-angle vector for specular
					VECTOR3 half;
					half[0] = extradata.light[0] + view[0];
					half[1] = extradata.light[1] + view[1];
					half[2] = extradata.light[2] + view[2];
					// normalize it
					l = 1.0f / sqrtf(half[0] * half[0] + half[1] * half[1] + half[2] * half[2]);
					half[0] *= l;
					half[1] *= l;
					half[2] *= l;

					// calculate specularity
					const int specular = (int)(pow(clamp_pos(dot_product_vec3(normal, half)), m_specular_power) * m_specular_intensity);
					const int intensity = (int)(dot * m_diffuse_intensity);

					// apply lighting
					r0 = ((r0 * intensity * extradata.diffuse_r) >> 16) + ((r0 * extradata.ambient_r) >> 8) + ((specular * extradata.specular_r) >> 8);
					g0 = ((g0 * intensity * extradata.diffuse_g) >> 16) + ((g0 * extradata.ambient_g) >> 8) + ((specular * extradata.specular_g) >> 8);
					b0 = ((b0 * intensity * extradata.diffuse_b) >> 16) + ((b0 * extradata.ambient_b) >> 8) + ((specular * extradata.specular_b) >> 8);
					r0 = std::clamp(r0, 0, 255);
					g0 = std::clamp(g0, 0, 255);
					b0 = std::clamp(b0, 0, 255);
				}

				// texture alpha mask
				if (texmode == 2)
				{
					// extract texel0 RGB
					int r0 = (texel0 & 0x7c00) >> 7;
					int g0 = (texel0 & 0x03e0) >> 2;
					int b0 = (texel0 & 0x001f) << 3;

					// fetch texture1
					u32 texel1 = (texel >> tex1_shift) & 0xffff;

					if (!(texel1 & 0x8000))
					{
						int sr = (fb[x] >> 16) & 0xff;
						int sg = (fb[x] >>  8) & 0xff;
						int sb = fb[x] & 0xff;

						sr += r0;
						sg += g0;
						sb += b0;

						if (sr > 255) sr = 255;
						if (sg > 255) sg = 255;
						if (sb > 255) sb = 255;

						// write to framebuffer
						fb[x] = 0xff000000 | (sr << 16) | (sg << 8) | sb;
					}
				}
				else if (texmode == 1)
				{
					// TEXEL0: RGB alpha levels
					// TEXEL1: RGB color

					int r0 = (texel0 >> 10) & 0x1f;
					int g0 = (texel0 >> 5) & 0x1f;
					int b0 = texel0 & 0x1f;

					// fetch texture1
					u32 texel1 = (texel >> tex1_shift) & 0xffff;

					int r1 = (texel1 & 0x7c00) >> 7;
					int g1 = (texel1 & 0x03e0) >> 2;
					int b1 = (texel1 & 0x001f) << 3;

					int sr = (fb[x] >> 16) & 0xff;
					int sg = (fb[x] >>  8) & 0xff;
					int sb = fb[x] & 0xff;

					r0 = ((r1 * (31 - r0)) >> 5) + ((sr * r0) >> 5);
					g0 = ((g1 * (31 - g0)) >> 5) + ((sg * g0) >> 5);
					b0 = ((b1 * (31 - b0)) >> 5) + ((sb * b0) >> 5);

					// write to framebuffer
					fb[x] = 0xff000000 | (r0 << 16) | (g0 << 8) | b0;
				}
				else if (texmode == 0)
				{
					// extract texel0 RGB
					int r0 = (texel0 & 0x7c00) >> 7;
					int g0 = (texel0 & 0x03e0) >> 2;
					int b0 = (texel0 & 0x001f) << 3;

					// write to framebuffer
					fb[x] = 0xff000000 | (r0 << 16) | (g0 << 8) | b0;
				}
				else
				{
					// polygon alpha blending
					if (alpha_enable && alpha < 0x1f)
					{
						int sr = (fb[x] >> 16) & 0xff;
						int sg = (fb[x] >>  8) & 0xff;
						int sb = fb[x] & 0xff;
						int a = alpha + 1;
						r0 = ((r0 * a) >> 5) + ((sr * (31 - a)) >> 5);
						g0 = ((g0 * a) >> 5) + ((sg * (31 - a)) >> 5);
						b0 = ((b0 * a) >> 5) + ((sb * (31 - a)) >> 5);
					}


					// write to framebuffer
					fb[x] = 0xff000000 | (r0 << 16) | (g0 << 8) | b0;
				}
			}

			zb[x] = ooz;
		}

		ooz += dooz;
		uoz += duoz;
		voz += dvoz;

		nx += dnx;
		ny += dny;
		nz += dnz;
	}
}

static inline int is_point_inside(float x, float y, float z, PLANE cp)
{
	float s = (x * cp.x) + (y * cp.y) + (z * cp.z) + cp.d;
	if (s >= 0.0f)
		return 1;
	else
		return 0;
}

float taitotz_renderer::line_plane_intersection(const vertex_t *v1, const vertex_t *v2, PLANE cp)
{
	float x = v1->x - v2->x;
	float y = v1->y - v2->y;
	float z = v1->p[POLY_Z] - v2->p[POLY_Z];
	float t = ((cp.x * v1->x) + (cp.y * v1->y) + (cp.z * v1->p[POLY_Z])) / ((cp.x * x) + (cp.y * y) + (cp.z * z));
	return t;
}

int taitotz_renderer::clip_polygon(const vertex_t *v, int num_vertices, PLANE cp, vertex_t *vout)
{
	vertex_t clipv[10];
	int clip_verts = 0;
	int previ = num_vertices - 1;
	for (int i = 0; i < num_vertices; i++)
	{
		int v1_in = is_point_inside(v[i].x, v[i].y, v[i].p[POLY_Z], cp);
		int v2_in = is_point_inside(v[previ].x, v[previ].y, v[previ].p[POLY_Z], cp);

		if (v1_in && v2_in)         // edge is completely inside the volume
		{
			clipv[clip_verts] = v[i];
			++clip_verts;
		}
		else if (!v1_in && v2_in)   // edge is entering the volume
		{
			// insert vertex at intersection point
			float t = line_plane_intersection(&v[i], &v[previ], cp);
			clipv[clip_verts].x = v[i].x + ((v[previ].x - v[i].x) * t);
			clipv[clip_verts].y = v[i].y + ((v[previ].y - v[i].y) * t);
			clipv[clip_verts].p[POLY_Z] = v[i].p[POLY_Z] + ((v[previ].p[POLY_Z] - v[i].p[POLY_Z]) * t);
			clipv[clip_verts].p[POLY_U] = v[i].p[POLY_U] + ((v[previ].p[POLY_U] - v[i].p[POLY_U]) * t);
			clipv[clip_verts].p[POLY_V] = v[i].p[POLY_V] + ((v[previ].p[POLY_V] - v[i].p[POLY_V]) * t);

			clipv[clip_verts].p[POLY_NX] = v[i].p[POLY_NX] + ((v[previ].p[POLY_NX] - v[i].p[POLY_NX]) * t);
			clipv[clip_verts].p[POLY_NY] = v[i].p[POLY_NY] + ((v[previ].p[POLY_NY] - v[i].p[POLY_NY]) * t);
			clipv[clip_verts].p[POLY_NZ] = v[i].p[POLY_NZ] + ((v[previ].p[POLY_NZ] - v[i].p[POLY_NZ]) * t);

			++clip_verts;
		}
		else if (v1_in && !v2_in)   // edge is leaving the volume
		{
			// insert vertex at intersection point
			float t = line_plane_intersection(&v[i], &v[previ], cp);
			clipv[clip_verts].x = v[i].x + ((v[previ].x - v[i].x) * t);
			clipv[clip_verts].y = v[i].y + ((v[previ].y - v[i].y) * t);
			clipv[clip_verts].p[POLY_Z] = v[i].p[POLY_Z] + ((v[previ].p[POLY_Z] - v[i].p[POLY_Z]) * t);
			clipv[clip_verts].p[POLY_U] = v[i].p[POLY_U] + ((v[previ].p[POLY_U] - v[i].p[POLY_U]) * t);
			clipv[clip_verts].p[POLY_V] = v[i].p[POLY_V] + ((v[previ].p[POLY_V] - v[i].p[POLY_V]) * t);

			clipv[clip_verts].p[POLY_NX] = v[i].p[POLY_NX] + ((v[previ].p[POLY_NX] - v[i].p[POLY_NX]) * t);
			clipv[clip_verts].p[POLY_NY] = v[i].p[POLY_NY] + ((v[previ].p[POLY_NY] - v[i].p[POLY_NY]) * t);
			clipv[clip_verts].p[POLY_NZ] = v[i].p[POLY_NZ] + ((v[previ].p[POLY_NZ] - v[i].p[POLY_NZ]) * t);

			++clip_verts;

			// insert the existing vertex
			clipv[clip_verts] = v[i];
			++clip_verts;
		}

		previ = i;
	}
	memcpy(&vout[0], &clipv[0], sizeof(vout[0]) * clip_verts);
	return clip_verts;
}

void taitotz_renderer::setup_viewport(int x, int y, int width, int height, int center_x, int center_y)
{
	m_vp_center_x = center_x;
	m_vp_center_y = center_y;
	m_vp_focus = width * 2;
	m_vp_mul = 512.0f / m_vp_focus;
	m_vp_x = x;
	m_vp_y = y;

	// set up clip planes
	float angleh =  atan2(width, m_vp_focus) - 0.0001;
	float anglev =  atan2(height, m_vp_focus) - 0.0001;
	float sh = sin(angleh);
	float sv = sin(anglev);
	float ch = cos(angleh);
	float cv = cos(anglev);

	// left
	m_clip_plane[0].x = ch;
	m_clip_plane[0].y = 0.0f;
	m_clip_plane[0].z = sh;
	m_clip_plane[0].d = 0.0f;

	// right
	m_clip_plane[1].x = -ch;
	m_clip_plane[1].y = 0.0f;
	m_clip_plane[1].z = sh;
	m_clip_plane[1].d = 0.0f;

	// top
	m_clip_plane[2].x = 0.0f;
	m_clip_plane[2].y = cv;
	m_clip_plane[2].z = sv;
	m_clip_plane[2].d = 0.0f;

	// bottom
	m_clip_plane[3].x = 0.0f;
	m_clip_plane[3].y = -cv;
	m_clip_plane[3].z = sv;
	m_clip_plane[3].d = 0.0f;

	// Z-near
	m_clip_plane[4].x = 0.0f;
	m_clip_plane[4].y = 0.0f;
	m_clip_plane[4].z = 1.0f;
	m_clip_plane[4].d = 0.1f;
}

void taitotz_renderer::render_tnl_object(u32 address, float scale, u8 alpha)
{
	u32 *src = &m_screen_ram[address];
	vertex_t v[10];

	bool end = false;
	int index = 0;
	do
	{
		taitotz_polydata &extra = object_data().next();

		int num_verts;

		if (src[index] & 0x10000000)
			end = true;

		if (src[index] & 0x01000000)
			num_verts = 4;
		else
			num_verts = 3;

		int texture = src[index] & 0x7ff;
		int tex_switch = (src[index+3] >> 26) & 0x3;

		index += 4;

		for (int i=0; i < num_verts; i++)
		{
			// texture coords
			u8 tu = src[index] >> 8;
			u8 tv = src[index] & 0xff;
			v[i].p[POLY_U] = (float)(tu);
			v[i].p[POLY_V] = (float)(tv);

			// coords
			s16 x = src[index + 1] & 0xffff;
			s16 y = src[index + 2] & 0xffff;
			s16 z = src[index + 3] & 0xffff;
			float px = (x * scale) / 256.0f;
			float py = (y * scale) / 256.0f;
			float pz = (z * scale) / 256.0f;

			// normals
			s8 inx = (src[index + 1] >> 16) & 0xff;
			s8 iny = (src[index + 2] >> 16) & 0xff;
			s8 inz = (src[index + 3] >> 16) & 0xff;
			float nx = inx / 128.0f;
			float ny = iny / 128.0f;
			float nz = inz / 128.0f;

			// transform
			v[i].x          = (px * m_matrix[0][0]) + (py * m_matrix[1][0]) + (pz * m_matrix[2][0]) + m_matrix[3][0];
			v[i].y          = (px * m_matrix[0][1]) + (py * m_matrix[1][1]) + (pz * m_matrix[2][1]) + m_matrix[3][1];
			v[i].p[POLY_Z]  = (px * m_matrix[0][2]) + (py * m_matrix[1][2]) + (pz * m_matrix[2][2]) + m_matrix[3][2];

			v[i].p[POLY_NX] = (nx * m_matrix[0][0]) + (ny * m_matrix[1][0]) + (nz * m_matrix[2][0]);
			v[i].p[POLY_NY] = (nx * m_matrix[0][1]) + (ny * m_matrix[1][1]) + (nz * m_matrix[2][1]);
			v[i].p[POLY_NZ] = (nx * m_matrix[0][2]) + (ny * m_matrix[1][2]) + (nz * m_matrix[2][2]);

			index += 4;
		}

		// clip against viewport frustum
		num_verts = clip_polygon(v, num_verts, m_clip_plane[0], v);
		num_verts = clip_polygon(v, num_verts, m_clip_plane[1], v);
		num_verts = clip_polygon(v, num_verts, m_clip_plane[2], v);
		num_verts = clip_polygon(v, num_verts, m_clip_plane[3], v);
		num_verts = clip_polygon(v, num_verts, m_clip_plane[4], v);

		// apply homogeneous Z-transform on coords and perspective correction on UV coords
		for (int i=0; i < num_verts; i++)
		{
			float ooz = 1.0f / v[i].p[POLY_Z];
			v[i].x = (((v[i].x * ooz) * m_vp_focus) * m_vp_mul) + m_vp_center_x;
			v[i].y = (((v[i].y * ooz) * m_vp_focus) * m_vp_mul) + m_vp_center_y;
			v[i].p[POLY_Z] = ooz;
			v[i].p[POLY_U] *= ooz;
			v[i].p[POLY_V] *= ooz;
		}

		extra.texture = texture;
		extra.alpha = alpha;
		extra.flags = tex_switch;
		extra.diffuse_r = 0xff;
		extra.diffuse_g = 0xff;
		extra.diffuse_b = 0xff;
		extra.ambient_r = 0x00;
		extra.ambient_g = 0x00;
		extra.ambient_b = 0x00;
		extra.specular_r = 0xff;
		extra.specular_g = 0xff;
		extra.specular_b = 0xff;
		extra.light[0] = 0.0f;
		extra.light[1] = 0.0f;
		extra.light[2] = -1.0f;

		for (int i = 2; i < num_verts; i++)
		{
			render_triangle<6>(m_cliprect, render_delegate(&taitotz_renderer::draw_scanline, this), v[0], v[i - 1], v[i]);
		}
	}
	while (!end);
}

void taitotz_renderer::draw(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	wait();
	copybitmap(bitmap, *m_fb, 0, 0, 0, 0, cliprect);

	float zvalue = 0.0f;
	m_zbuffer->fill(*(int*)&zvalue, cliprect);
}

void taitotz_renderer::push_tnl_fifo(u32 data)
{
	m_tnl_fifo[m_tnl_fifo_ptr] = data;
	m_tnl_fifo_ptr++;

	if (m_tnl_fifo_ptr >= 16)
	{
		m_matrix[0][0] = u2f(m_tnl_fifo[4]);
		m_matrix[1][0] = u2f(m_tnl_fifo[5]);
		m_matrix[2][0] = u2f(m_tnl_fifo[6]);
		m_matrix[3][0] = u2f(m_tnl_fifo[7]);

		m_matrix[0][1] = u2f(m_tnl_fifo[8]);
		m_matrix[1][1] = u2f(m_tnl_fifo[9]);
		m_matrix[2][1] = u2f(m_tnl_fifo[10]);
		m_matrix[3][1] = u2f(m_tnl_fifo[11]);

		m_matrix[0][2] = u2f(m_tnl_fifo[12]);
		m_matrix[1][2] = u2f(m_tnl_fifo[13]);
		m_matrix[2][2] = u2f(m_tnl_fifo[14]);
		m_matrix[3][2] = u2f(m_tnl_fifo[15]);

		float scale = u2f(m_tnl_fifo[1]);
		if (scale < 1.0f)
			scale *= 2.0f;

		u32 alpha = m_tnl_fifo[2];
		render_tnl_object(m_tnl_fifo[0] & 0x1fffff, scale, alpha);

//      printf("TNL FIFO: %08X, %f, %08X, %08X\n", m_tnl_fifo[0], u2f(m_tnl_fifo[1]), m_tnl_fifo[2], m_tnl_fifo[3]);
		m_tnl_fifo_ptr = 0;
	}
}

void taitotz_renderer::push_direct_poly_fifo(u32 data)
{
	m_direct_fifo[m_direct_fifo_ptr] = data;
	m_direct_fifo_ptr++;

	int num_verts = ((m_direct_fifo[0] >> 8) & 7) + 1;
	int expected_size = 4 + num_verts * 4;
	if (m_direct_fifo_ptr >= expected_size)
	{
		vertex_t v[8];
		taitotz_polydata &extra = object_data().next();

		int index = 4;
		for (int i = 0; i < num_verts; i++)
		{
			v[i].x = (m_direct_fifo[index + 0] >> 16) & 0xffff;
			v[i].y = m_direct_fifo[index + 0] & 0xffff;
			v[i].p[POLY_U] = (m_direct_fifo[index + 2] >> 20) & 0xfff;
			v[i].p[POLY_V] = (m_direct_fifo[index + 2] >> 4) & 0xfff;
			//u16 z = m_direct_fifo[index+1] & 0xffff;

			index += 4;
		}

		extra.texture = m_direct_fifo[1] & 0x7ff;

		for (int i = 2; i < num_verts; i++)
		{
			render_triangle<3>(m_cliprect, render_delegate(&taitotz_renderer::draw_scanline_noz, this), v[0], v[i-1], v[i]);
		}

		m_direct_fifo_ptr = 0;
	}
}

u32 taitotz_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_renderer->draw(bitmap, cliprect);

	u16 const *screen_src = (u16*)&m_screen_ram[m_scr_base];

	for (int j = 0; j < 384; j++)
	{
		u32 *const fb = &bitmap.pix(j);
		for (int i = 0; i < 512; i++)
		{
			u16 p = *screen_src++;
			if (p & 0x8000)     // draw 2D framebuffer if there's an opaque pixel
			{
				int r = ((p >> 10) & 0x1f) << (3 + 16);
				int g = ((p >> 5) & 0x1f) << (3 + 8);
				int b = (p & 0x1f) << 3;
				fb[i ^ 1] = 0xff000000 | r | g | b;
			}
		}
	}

	return 0;
}

void taitotz_state::draw_tile(u32 pos, u32 tile)
{
	int tileu = (tile & 0x1f) << 4;
	int tilev = ((tile >> 5)) << 4;

	int tilex = (pos & 0x1f) << 4;
	int tiley = ((pos >> 5) & 0x1f) << 4;

	u16 *src_tile = (u16*)&m_screen_ram[0x180000];
	u16 *dst = (u16*)&m_screen_ram[m_scr_base];

	int v = tilev;
	for (int j = tiley; j < tiley + 16; j++, v++)
	{
		int u = tileu;
		for (int i = tilex; i < tilex + 16; i++, u++)
		{
			u16 p = src_tile[v * 512 + u];
			dst[j * 512 + i] = p;
		}
	}
}

/*
    Video chip memory map

    0x0800000...0x9fffff        Screen RAM
        0x980000...987fff       Char RAM
        0x9c0000...9fffff       Tile RAM?
    0x1000000...0x17fffff       Texture RAM
    0x1800000...0x187ffff       Frame RAM

    landhigh puts fullscreen images into 0x9c0000
    batlgr2 into 0x9e0000
*/

u32 taitotz_state::video_mem_r(u32 address)
{
	if (address >= 0x800000 && address < 0x1000000)
	{
		return m_screen_ram[address - 0x800000];
	}
	else if (address >= 0x1000000 && address < 0x1800000)
	{
		return m_texture_ram[address - 0x1000000];
	}
	else if (address >= 0x1800000 && address < 0x1880000)
	{
		return m_frame_ram[address - 0x1800000];
	}
	else
	{
		LOGMASKED(LOG_VIDEO_MEM_UNK_RD, "%s: video_mem_r: unknown address %08x\n", address);
		return 0;
	}
}

void taitotz_state::video_mem_w(u32 address, u32 data)
{
	if (address >= 0x800000 && address < 0x1000000)
	{
		m_screen_ram[address - 0x800000] = data;
	}
	else if (address >= 0x1000000 && address < 0x1800000)
	{
		m_texture_ram[address - 0x1000000] = data;
	}
	else if (address >= 0x1800000 && address < 0x1880000)
	{
		m_frame_ram[address - 0x1800000] = data;
	}
	else
	{
		LOGMASKED(LOG_VIDEO_MEM_UNK_WR, "%s: video_mem_w: unknown address %08x = %08x\n", address, data);
	}
}

u32 taitotz_state::video_reg_r(u32 reg)
{
	u32 data = 0;
	switch ((reg >> 28) & 0xf)
	{
	case 0x1:
		if (reg == 0x10000105)      // Gets spammed a lot. Probably a status register.
		{
			m_reg105 ^= 0xffffffff;
			return m_reg105;
		}

		data = 0xffffffff;
		LOGMASKED(LOG_VIDEO_REG_1_RD, "%s: video_reg_r: reg %08x: %08x\n", machine().describe_context(), reg, data);
		return data;

	case 0x2:
	{
		int subreg = reg & 0xfffffff;
		if (subreg < 0x10)
		{
			data = m_video_unk_reg[subreg];
			LOGMASKED(LOG_VIDEO_REG_2_RD, "%s: video_reg_r: reg %08x: %08x\n", machine().describe_context(), reg, data);
		}
		else
		{
			LOGMASKED(LOG_VIDEO_REG_2_RD, "%s: video_reg_r: unhandled reg %08x: %08x\n", machine().describe_context(), reg, data);
		}
		return data;
	}
	case 0xb:
		return video_mem_r(reg & 0xfffffff);

	default:
		LOGMASKED(LOG_VIDEO_REG_UNK_RD, "%s: video_reg_r: unknown reg %08x: %08x\n", machine().describe_context(), reg, data);
		break;
	}

	return data;
}

/*
video_reg_w: r: A0000000 d: 01FF029F
video_reg_w: r: A0000000 d: 02200278
video_reg_w: r: A0000000 d: 019001C2
video_reg_w: r: A0000000 d: 019F01B3
video_reg_w: r: A0000000 d: 00000000
video_reg_w: r: A0000000 d: 00001C00
video_reg_w: r: A0000000 d: 00000000
video_reg_w: r: A0000000 d: 00070000
video_reg_w: r: A0000000 d: 00000000
video_reg_w: r: 10000104 d: 00000000
video_reg_w: r: 10000105 d: 00000001

video_reg_w: r: 20000007 d: 00070000
video_reg_w: r: 20000000 d: 01F0029B
video_reg_w: r: 20000001 d: 02140261
video_reg_w: r: 20000002 d: 018101BE
video_reg_w: r: 20000003 d: 019501AA
video_reg_w: r: 20000004 d: 00000000
*/

void taitotz_state::video_reg_w(u32 reg, u32 data)
{
	switch ((reg >> 28) & 0xf)
	{
	case 0x1:       // Register write?
		if (reg == 0x10000105)
		{
			// This register gets spammed a lot. VB IRQ ack or buffer swap?
		}
		else
		{
			LOGMASKED(LOG_VIDEO_REG_1_WR, "%s: video_reg_w: reg %08x = %08x\n", machine().describe_context(), reg, data);
		}
		break;

	case 0x2:       // ???
	{
		int subreg = reg & 0xfffffff;
		if (subreg < 0x10)
		{
			m_video_unk_reg[subreg] = data;
			LOGMASKED(LOG_VIDEO_REG_2_WR, "%s: video_reg_w: reg %08x = %08x\n", machine().describe_context(), reg, data);
		}
		else
		{
			LOGMASKED(LOG_VIDEO_REG_2_WR, "%s: video_reg_w: unhandled reg %08x = %08x\n", machine().describe_context(), reg, data);
		}
		break;
	}
	case 0x3:       // Draw 16x16 tile
		draw_tile((data >> 12) & 0xfff, data & 0xfff);
		break;
	case 0xb:       // RAM write?
		video_mem_w(m_video_ram_ptr, data);
		m_video_ram_ptr++;
		break;
	default:
		LOGMASKED(LOG_VIDEO_REG_UNK_WR, "%s: video_reg_w: unknown reg %08x = %08x\n", machine().describe_context(), reg, data);
		break;
	}
}

u64 taitotz_state::video_chip_r(offs_t offset, u64 mem_mask)
{
	u64 r = 0;
	u32 reg = offset * 8;

	if (ACCESSING_BITS_0_31)
	{
		reg += 4;

		switch (reg)
		{
		case 0x14:
			r |= 0xff;      // more busy flags? (value & 0x11ff == 0xff expected)
			break;

		default:
			LOGMASKED(LOG_VIDEO_CHIP_UNK_RD, "%s: video_chip_r: unknown reg %08x & %08x%08x\n", machine().describe_context(), reg, (u32)(mem_mask >> 32), (u32)mem_mask);
			break;
		}
	}

	if (ACCESSING_BITS_32_63)
	{
		switch (reg)
		{
		case 0x0:
			r |= (u64)video_reg_r(m_video_reg) << 32;
			break;

		case 0x10:
			r |= 0x000000ff00000000ULL;      // busy flags? landhigh expects this
			break;

		default:
			LOGMASKED(LOG_VIDEO_CHIP_UNK_RD, "%s: video_chip_r: unknown reg %08x & %08x%08x\n", machine().describe_context(), reg, (u32)(mem_mask >> 32), (u32)mem_mask);
			break;
		}
	}

	return r;
}

void taitotz_state::video_chip_w(offs_t offset, u64 data, u64 mem_mask)
{
	u32 reg = offset * 8;
	u32 regdata;

	if (ACCESSING_BITS_0_31)
	{
		reg += 4;
		regdata = (u32)data;
		switch (reg)
		{
			default:
				LOGMASKED(LOG_VIDEO_CHIP_UNK_WR, "%s: video_chip_w: unknown port %02x = %08x%08x & %08x%08x\n", machine().describe_context(), reg, (u32)(data >> 32), (u32)data, (u32)(mem_mask >> 32), (u32)mem_mask);
				break;
		}
	}

	if (ACCESSING_BITS_32_63)
	{
		regdata = (u32)(data >> 32);
		switch (reg)
		{
		case 0:
			video_reg_w(m_video_reg, regdata);
			break;
		case 0x8:
			m_video_reg = regdata;
			m_video_fifo_ptr = 0;

			switch ((m_video_reg >> 28) & 0xf)
			{
			case 0xb:
				m_video_ram_ptr = m_video_reg & 0xfffffff;
				break;
			case 0x0:
				break;
			case 0x1:
				break;
			case 0x2:
				break;
			case 0x3:
				// video_chip_w: port 0x08: 30000001        gets spammed a lot
				break;
			default:
				LOGMASKED(LOG_VIDEO_CHIP_UNK_WR, "%s: video_chip_w: port %02x = %08x%08x & %08x%08x (reg 8)\n", machine().describe_context(), reg, (u32)(data >> 32), (u32)data, (u32)(mem_mask >> 32), (u32)mem_mask);
				break;
			}
			break;
		default:
			LOGMASKED(LOG_VIDEO_CHIP_UNK_WR, "%s: video_chip_w: unknown port %02x = %08x%08x & %08x%08x\n", machine().describe_context(), reg, (u32)(data >> 32), (u32)data, (u32)(mem_mask >> 32), (u32)mem_mask);
			break;
		}
	}
}

u64 taitotz_state::video_fifo_r(offs_t offset, u64 mem_mask)
{
	u64 r = 0;
	if (ACCESSING_BITS_32_63)
	{
		r |= (u64)video_mem_r(m_video_ram_ptr) << 32;
		m_video_ram_ptr++;
	}
	if (ACCESSING_BITS_0_31)
	{
		r |= (u64)video_mem_r(m_video_ram_ptr);
		m_video_ram_ptr++;
	}

	return r;
}

void taitotz_state::video_fifo_w(offs_t offset, u64 data, u64 mem_mask)
{
	int command = (m_video_reg >> 28) & 0xf;
	if (command == 0xb)
	{
		// FIFO mem access
		if (ACCESSING_BITS_32_63)
		{
			if (m_video_fifo_ptr >= 8)
			{
				video_mem_w(m_video_ram_ptr, (u32)(data >> 32));
				m_video_ram_ptr++;
			}
			m_video_fifo_ptr++;
		}
		if (ACCESSING_BITS_0_31)
		{
			if (m_video_fifo_ptr >= 8)
			{
				video_mem_w(m_video_ram_ptr, (u32)data);
				m_video_ram_ptr++;
			}
			m_video_fifo_ptr++;
		}
	}
	else if (command == 0x1)
	{
		// Direct Polygon FIFO
		LOGMASKED(LOG_DIRECT_FIFO, "%s: Direct polygon FIFO write: %08x%08x & %08x%08x\n", machine().describe_context(), (u32)(data >> 32), (u32)data, (u32)(mem_mask >> 32), (u32)mem_mask);
		if (ACCESSING_BITS_32_63)
		{
			if (m_video_fifo_ptr >= 8)
			{
				m_renderer->push_direct_poly_fifo((u32)(data >> 32));
				LOGMASKED(LOG_DIRECT_FIFO, "%s: push_direct_poly_fifo: %08x [%d]\n", machine().describe_context(), (u32)(data >> 32), m_renderer->direct_fifo_ptr());
			}
			m_video_fifo_ptr++;
		}
		if (ACCESSING_BITS_0_31)
		{
			if (m_video_fifo_ptr >= 8)
			{
				m_renderer->push_direct_poly_fifo((u32)data);
				LOGMASKED(LOG_DIRECT_FIFO, "%s: push_direct_poly_fifo: %08x [%d]\n", machine().describe_context(), (u32)data, m_renderer->direct_fifo_ptr());
			}
			m_video_fifo_ptr++;
		}
	}
	else if (command == 0x0)
	{
		// T&L FIFO
		LOGMASKED(LOG_TNL_FIFO, "%s: T&L FIFO write: %08x%08x & %08x%08x\n", machine().describe_context(), (u32)(data >> 32), (u32)data, (u32)(mem_mask >> 32), (u32)mem_mask);
		if (ACCESSING_BITS_32_63)
		{
			if (m_video_fifo_ptr >= 8)
			{
				m_renderer->push_tnl_fifo((u32)(data >> 32));
				LOGMASKED(LOG_TNL_FIFO, "%s: push_tnl_fifo: %08x\n", machine().describe_context(), (u32)(data >> 32));
			}
			m_video_fifo_ptr++;
		}
		if (ACCESSING_BITS_0_31)
		{
			if (m_video_fifo_ptr >= 8)
			{
				m_renderer->push_tnl_fifo((u32)data);
				LOGMASKED(LOG_TNL_FIFO, "%s: push_tnl_fifo: %08x\n", machine().describe_context(), (u32)data);
			}
			m_video_fifo_ptr++;
		}
	}
	else
	{
		LOGMASKED(LOG_DIRECT_FIFO, "%s: Unknown FIFO write: %08x%08x & %08x%08x\n", machine().describe_context(), (u32)(data >> 32), (u32)data, (u32)(mem_mask >> 32), (u32)mem_mask);
		if (ACCESSING_BITS_32_63)
		{
			if (m_video_fifo_ptr >= 8)
			{
				printf("FIFO write with cmd %02X: %08X\n", command, (u32)(data >> 32));
			}
			m_video_fifo_ptr++;
		}
		if (ACCESSING_BITS_0_31)
		{
			if (m_video_fifo_ptr >= 8)
			{
				printf("FIFO write with cmd %02X: %08X\n", command, (u32)data);
			}
			m_video_fifo_ptr++;
		}
	}
}

u64 taitotz_state::ieee1394_r(offs_t offset, u64 mem_mask)
{
	if (offset == 4)
	{
		return ~0ULL;
	}

	return 0;
}

void taitotz_state::ieee1394_w(offs_t offset, u64 data, u64 mem_mask)
{
}

u64 taitotz_state::ppc_common_r(offs_t offset, u64 mem_mask)
{
	u64 res = 0;

	if (ACCESSING_BITS_0_15)
	{
		res |= m_io_share_ram[offset * 2 + 1];
	}
	if (ACCESSING_BITS_32_47)
	{
		res |= (u64)m_io_share_ram[offset * 2] << 32;
	}

	return res;
}

void taitotz_state::ppc_common_w(offs_t offset, u64 data, u64 mem_mask)
{
	if (ACCESSING_BITS_0_15)
	{
		m_io_share_ram[offset * 2 + 1] = (u16)data;
	}
	if (ACCESSING_BITS_32_47)
	{
		m_io_share_ram[offset * 2] = (u16)(data >> 32);
	}

	if (offset == 0x7ff)
	{
		const u16 tlcs_cmd = m_io_share_ram[0xfff];
		if (tlcs_cmd != 0x0000 && tlcs_cmd != 0x1010 && tlcs_cmd != 0x1020 && tlcs_cmd != 0x6000 && tlcs_cmd != 0x6010 &&
			tlcs_cmd != 0x7004 && tlcs_cmd != 0x4001 && tlcs_cmd != 0x4002 && tlcs_cmd != 0x4003)
		{
			LOGMASKED(LOG_PPC_TO_TLCS_COMMANDS, "PPC -> TLCS cmd %04x\n", tlcs_cmd);
		}

		if (tlcs_cmd == 0x4000)
		{
			LOGMASKED(LOG_PPC_TO_TLCS_COMMANDS, "   %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n",
					m_io_share_ram[0x1c34 / 2], m_io_share_ram[0x1c36 / 2], m_io_share_ram[0x1c38 / 2], m_io_share_ram[0x1c3a / 2],
					m_io_share_ram[0x1c2c / 2], m_io_share_ram[0x1c2e / 2], m_io_share_ram[0x1c30 / 2], m_io_share_ram[0x1c32 / 2],
					m_io_share_ram[0x1c24 / 2], m_io_share_ram[0x1c26 / 2]);
			LOGMASKED(LOG_PPC_TO_TLCS_COMMANDS, "   %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n",
					m_io_share_ram[0x1c28 / 2], m_io_share_ram[0x1c2a / 2], m_io_share_ram[0x1c1c / 2], m_io_share_ram[0x1c1e / 2],
					m_io_share_ram[0x1c20 / 2], m_io_share_ram[0x1c22 / 2], m_io_share_ram[0x1c14 / 2], m_io_share_ram[0x1c16 / 2],
					m_io_share_ram[0x1c18 / 2], m_io_share_ram[0x1c1a / 2]);
		}

		// hacky way to handle some commands for now
		if (tlcs_cmd == 0x4001)
		{
			m_io_share_ram[0xfff] = 0x0000;
			m_io_share_ram[0xe00] = 0xffff;
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		}
		else if (tlcs_cmd == 0x4004 || tlcs_cmd == 0x4000)
		{
			m_io_share_ram[0xfff] = 0x0000;
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		}
		else if (tlcs_cmd == 0x7004)
		{
			// this command seems to turn off interrupts on TLCS...
			m_io_share_ram[0xfff] = 0x0000;
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		}
		else
		{
			// normally just raise INT0 on TLCS and let it handle the command
			m_iocpu->set_input_line(TLCS900_INT0, ASSERT_LINE);
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);

			// The PPC always goes to busy loop waiting for TLCS here, so we can free up the timeslice.
			// Only do it for HDD access and backup RAM for now...
			if (tlcs_cmd == 0x1010 || tlcs_cmd == 0x1020 ||
				tlcs_cmd == 0x6000 || tlcs_cmd == 0x6010)
			{
				//m_maincpu->spin_until_trigger(PPC_TLCS_COMM_TRIGGER);
				//m_maincpu->spin_until_interrupt();
			}

			// pwrshovl sometimes writes commands during command handling... make sure that doesn't happen
			if (tlcs_cmd == 0x0000)
			{
				m_maincpu->spin_until_time(attotime::from_usec(100));
			}

			machine().scheduler().trigger(TLCS_PPC_COMM_TRIGGER);
		}
	}

	if (ENABLE_DEBUG_PRINTS)
	{
		// debug hookup
		if ((u8)m_io_share_ram[0xd82] == 0xff)
		{
			for (int i = 0; i < 0x80; i++)
			{
				u16 w = m_io_share_ram[0x900 + i];
				printf("%c%c", (s8)w, (s8)(w >> 8));
			}
			printf("\n");
			m_io_share_ram[0xd82] = 0;
		}
		if ((u8)m_io_share_ram[0xd8a] == 0xff)
		{
			for (int i = 0; i < 0x80; i++)
			{
				u16 w = m_io_share_ram[0xb00 + i];
				printf("%c%c", (s8)w, (s8)(w >> 8));
			}
			printf("\n");
			m_io_share_ram[0xd8a] = 0;
		}
	}
}

// BAT Config:
// IBAT0 U: 0xf0001fff   L: 0xf0000023   (0xf0000000...0xffffffff)
// IBAT1 U: 0xe0001fff   L: 0xe0000023   (0xe0000000...0xefffffff)
// IBAT2 U: 0x40001fff   L: 0x40000003   (0x40000000...0x4fffffff)
// IBAT3 U: 0xa0001fff   L: 0xa0000023   (0xa0000000...0xafffffff)
// DBAT0 U: 0x00001fff   L: 0x00000022   (0x00000000...0x0fffffff)
// DBAT1 U: 0x10001fff   L: 0x10000002   (0x10000000...0x1fffffff)
// DBAT2 U: 0x40001fff   L: 0x40000002   (0x40000000...0x4fffffff)
// DBAT3 U: 0xa0001fff   L: 0xa0000022   (0xa0000000...0xafffffff)

// 0x10000000...0x1000001f: texture FIFO?
// 0x40000000...0x400fffff: BIOS Work RAM
// 0x40100000...0x40ffffff: User Work RAM

void taitotz_state::ppc603e_mem(address_map &map)
{
	map(0x00000000, 0x0000001f).rw(FUNC(taitotz_state::video_chip_r), FUNC(taitotz_state::video_chip_w));
	map(0x10000000, 0x1000001f).rw(FUNC(taitotz_state::video_fifo_r), FUNC(taitotz_state::video_fifo_w));
	map(0x40000000, 0x40ffffff).ram().share("work_ram");                // Work RAM
	map(0xa4000000, 0xa40000ff).rw(FUNC(taitotz_state::ieee1394_r), FUNC(taitotz_state::ieee1394_w));       // IEEE1394 network
	map(0xa8000000, 0xa8003fff).rw(FUNC(taitotz_state::ppc_common_r), FUNC(taitotz_state::ppc_common_w));   // Common RAM (with TLCS-900)
	map(0xac000000, 0xac0fffff).rom().region("user1", 0);               // Apparently this should be flash ROM read/write access
	map(0xfff00000, 0xffffffff).rom().region("user1", 0);
}

u8 taitotz_state::tlcs_common_r(offs_t offset)
{
	if (offset & 1)
	{
		return (u8)(m_io_share_ram[offset / 2] >> 8);
	}
	else
	{
		return (u8)(m_io_share_ram[offset / 2]);
	}
}

void taitotz_state::tlcs_common_w(offs_t offset, u8 data)
{
	if (offset & 1)
	{
		m_io_share_ram[offset / 2] &= 0x00ff;
		m_io_share_ram[offset / 2] |= (u16)data << 8;
	}
	else
	{
		m_io_share_ram[offset / 2] &= 0xff00;
		m_io_share_ram[offset / 2] |= data;
	}

	if (offset == 0x1ffd)
	{
		if (m_io_share_ram[0xffe] != 0xd000 &&
			m_io_share_ram[0xffe] != 0x1011 &&
			m_io_share_ram[0xffe] != 0x1012 &&
			m_io_share_ram[0xffe] != 0x1022)
		{
			LOGMASKED(LOG_TLCS_TO_PPC_COMMANDS, "TLCS -> PPC cmd %04X\n", m_io_share_ram[0xffe]);
		}

		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		m_iocpu->set_input_line(TLCS900_INT0, CLEAR_LINE);

		m_iocpu->set_input_line(TLCS900_INT3, CLEAR_LINE);

		// The PPC is now free to continue running
		//machine().scheduler().trigger(PPC_TLCS_COMM_TRIGGER);
		//m_iocpu->yield();
	}

	if (offset == 0x1ffe)
	{
		if (m_io_share_ram[0xfff] == 0 && m_io_share_ram[0xffe] == 0x1012)
		{
			//m_iocpu->spin_until_trigger(TLCS_PPC_COMM_TRIGGER);
			m_iocpu->yield();
			machine().scheduler().trigger(PPC_TLCS_COMM_TRIGGER);
		}
	}
}

// RTC could be Epson RTC-64613, same as taitopjc.cpp
u8 taitotz_state::tlcs_rtc_r(offs_t offset)
{
	switch (offset)
	{
		// NOTE: bcd numbers
		case 0x00:      return m_rtcdata[0];        // milliseconds?
		case 0x01:      return m_rtcdata[1];        // seconds
		case 0x02:      return m_rtcdata[2];        // minutes
		case 0x03:      return m_rtcdata[3];        // hours
		case 0x04:      return m_rtcdata[4];        // day of the week
		case 0x05:      return m_rtcdata[5];        // day
		case 0x06:      return m_rtcdata[6];        // month
		case 0x07:      return m_rtcdata[7];        // year

		case 0x0e:      return 0;

		default:
			LOGMASKED(LOG_RTC_UNK_RD, "%s: tlcs_rtc_r: unknown reg %02x\n", offset);
			break;
	}

	return 0;
}

void taitotz_state::tlcs_rtc_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0x00:      m_rtcdata[0] = data; break;
		case 0x01:      m_rtcdata[1] = data; break;
		case 0x02:      m_rtcdata[2] = data; break;
		case 0x03:      m_rtcdata[3] = data; break;
		case 0x04:      m_rtcdata[4] = data; break;
		case 0x05:      m_rtcdata[5] = data; break;
		case 0x06:      m_rtcdata[6] = data; break;
		case 0x07:      m_rtcdata[7] = data; break;
		case 0x0e:
			break;

		default:
			LOGMASKED(LOG_RTC_UNK_WR, "%s: tlcs_rtc_w: unknown reg %02x = %02x\n", offset, data);
			break;
	}
}

u16 taitotz_state::tlcs_ide0_r(offs_t offset, u16 mem_mask)
{
	u16 d = m_ata->cs0_r(offset, mem_mask);
	if (offset == 7)
	{
		// Type Zero doesn't like the index bit. It's defined as vendor-specific, so it probably shouldn't be up.
		// The status check explicitly checks for 0x50 (drive ready, seek complete).
		d &= ~0x2;
	}
	return d;
}

u16 taitotz_state::tlcs_ide1_r(offs_t offset, u16 mem_mask)
{
	u16 d = m_ata->cs1_r(offset, mem_mask);
	if (offset == 6)
	{
		// Type Zero doesn't like the index bit. It's defined as vendor-specific, so it probably shouldn't be up.
		// The status check explicitly checks for 0x50 (drive ready, seek complete).
		d &= ~0x2;
	}
	return d;
}

// TLCS900 interrupt vectors
// 0xfc0150:    Reset
// 0xfc0120:    SWI1-7          -
// 0xfc0120:    NMI             -
// 0xfc0120:    INTWD           -
// 0xfc0122:    INT0            PPC communication
// 0xfc0137:    INT1            IEEE1394?
// 0xfc013c:    INT2            IDE
// 0xfc0141:    INT3            VBlank? Drives a counter on PPC side
// 0xfc0146:    INT4            -
// 0xfc0147:    INT5            -
// 0xfc0148:    INT6            -
// 0xfc0149:    INT7            -
// 0xfc014a:    INT8            Sound chip interrupt?
// 0xfc0120:    INTAD           -
// 0xfc0120:    INTTR8-A        -
// 0xfc0120:    INTT0           -
// 0xfc0467:    INTT1           A/D Conversion
// 0xfc0120:    INTT2-7         -
// 0xfc0120:    INTTC0-3        -
// 0xfc0d1d:    INTRX0          Serial 0 receive
// 0xfc0ca5:    INTTX0          Serial 0 transmit
// 0xfc0d55:    INTRX1          Serial 1 receive
// 0xfc0ce1:    INTTX1          Serial 1 transmit

void taitotz_state::tlcs900h_mem(address_map &map)
{
	map(0x010000, 0x02ffff).ram();                                                  // Work RAM
	map(0x040000, 0x041fff).ram().share("nvram");                                   // Backup RAM
	map(0x044000, 0x04400f).rw(FUNC(taitotz_state::tlcs_rtc_r), FUNC(taitotz_state::tlcs_rtc_w));
	map(0x060000, 0x061fff).rw(FUNC(taitotz_state::tlcs_common_r), FUNC(taitotz_state::tlcs_common_w));
	map(0x064000, 0x064fff).ram().share("mbox_ram");                                // MBox
	map(0x068000, 0x06800f).w(m_ata, FUNC(ata_interface_device::cs0_w)).r(FUNC(taitotz_state::tlcs_ide0_r));
	map(0x06c000, 0x06c00f).w(m_ata, FUNC(ata_interface_device::cs1_w)).r(FUNC(taitotz_state::tlcs_ide1_r));
	map(0xfc0000, 0xffffff).rom().region("io_cpu", 0);
}

void taitotz_state::landhigh_tlcs900h_mem(address_map &map)
{
	map(0x200000, 0x21ffff).ram();                                                  // Work RAM
	map(0x400000, 0x401fff).ram().share("nvram");                                   // Backup RAM
	map(0x404000, 0x40400f).rw(FUNC(taitotz_state::tlcs_rtc_r), FUNC(taitotz_state::tlcs_rtc_w));
	map(0x900000, 0x901fff).rw(FUNC(taitotz_state::tlcs_common_r), FUNC(taitotz_state::tlcs_common_w));
	map(0x910000, 0x910fff).ram().share("mbox_ram");                                // MBox
	map(0x908000, 0x90800f).w(m_ata, FUNC(ata_interface_device::cs0_w)).r(FUNC(taitotz_state::tlcs_ide0_r));
	map(0x918000, 0x91800f).w(m_ata, FUNC(ata_interface_device::cs1_w)).r(FUNC(taitotz_state::tlcs_ide1_r));
	map(0xfc0000, 0xffffff).rom().region("io_cpu", 0);
}

static INPUT_PORTS_START( taitotz )
	PORT_START("INPUTS1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x00000002, IP_ACTIVE_LOW) // Test Button
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )                                    // Coin A
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )                                    // Coin B
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS3")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(1)      // Trig1 / U
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)   // Trig2 / D
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)  // Trig3 / R
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)   // Trig4 / L
	PORT_BIT( 0x000000e0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS4")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON5 )                                  // View 3
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON6 )                                  // View 4
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON7 )                                  // View 5
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON8 )                                  // View 6
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON3 )                                  // View 1
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON4 )                                  // View 2
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON1 )                                  // Select 1
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON2 )                                  // Select 2

	PORT_START("ANALOG1")
	PORT_START("ANALOG2")
	PORT_START("ANALOG3")
	PORT_START("ANALOG4")
	PORT_START("ANALOG5")
	PORT_START("ANALOG6")
	PORT_START("ANALOG7")
	PORT_START("ANALOG8")
INPUT_PORTS_END

static INPUT_PORTS_START( landhigh )
	PORT_START("INPUTS1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x00000002, IP_ACTIVE_LOW) // Test Button
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )                                    // Coin
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS3")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON3 )                                  // Speak
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON4 )                                  // Help
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON1 )                                  // Flap Up
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON2 )                                  // Flap Down
	PORT_BIT( 0x000000e0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS4")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON5 )                                  // ID Button
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON6 )                                  // Lever Sync
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_START1 )                                   // Start
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ANALOG1")
	PORT_BIT( 0x3ff, 0x000, IPT_AD_STICK_X ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG2")
	PORT_BIT( 0x3ff, 0x000, IPT_AD_STICK_Y ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG3")
	PORT_BIT( 0x3ff, 0x000, IPT_PEDAL ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG4")
	PORT_BIT( 0x3ff, 0x000, IPT_PEDAL2 ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG5")
	PORT_BIT( 0x3ff, 0x000, IPT_PEDAL3 ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG6")
	PORT_START("ANALOG7")
	PORT_START("ANALOG8")
INPUT_PORTS_END

static INPUT_PORTS_START( batlgr2 )
	PORT_START("INPUTS1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x00000002, IP_ACTIVE_LOW) // Test Button
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )                                    // Coin
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS3")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 )                                  // Shift Down
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON1 )                                  // Shift Up
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON3 )                                  // View
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x000000e0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS4")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_START1 )                                   // Start
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ANALOG1")       // Steering
	PORT_BIT( 0x3ff, 0x200, IPT_PADDLE ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG2")       // Gas Pedal
	PORT_BIT( 0x3ff, 0x000, IPT_PEDAL) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG3")       // Brake Pedal
	PORT_BIT( 0x3ff, 0x000, IPT_PEDAL2 ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG4")
	PORT_START("ANALOG5")
	PORT_START("ANALOG6")
	PORT_START("ANALOG7")
	PORT_START("ANALOG8")
INPUT_PORTS_END

static INPUT_PORTS_START( pwrshovl )
	PORT_START("INPUTS1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x00000002, IP_ACTIVE_LOW) // Test Button
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )                                    // Coin A
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS3")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_PLAYER(1)                   // P1-Foot-Sw: Left-Down
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)                  // P1-Foot-Sw: Left-Up
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1)                  // P1-Foot-Sw: Right-Down
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)                  // P1-Foot-Sw: Right-Up
	PORT_BIT( 0x000000e0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS4")
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)                  // P2-Foot-Sw: Left-Down
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)                  // P2-Foot-Sw: Left-Up
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2)                  // P2-Foot-Sw: Right-Down
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)                  // P2-Foot-Sw: Right-Up
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)                   // P1 View
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)                   // P2 View
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_START1 )                                   // P1 Start
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START2 )                                   // P2 Start

	PORT_START("ANALOG1")       // P1-Lever Left-X
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_X ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG2")       // P1-Lever Left-Y
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_Y ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG3")       // P1-Lever Right-X
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_X ) PORT_PLAYER(2) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG4")       // P1_Lever Right-Y
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_Y ) PORT_PLAYER(2) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG5")       // P2-Lever Left-X
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_X ) PORT_PLAYER(3) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG6")       // P2-Lever Left-Y
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_Y ) PORT_PLAYER(3) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG7")       // P2-Lever Right-X
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_X ) PORT_PLAYER(4) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG8")       // P2_Lever Right-Y
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_Y ) PORT_PLAYER(4) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)
INPUT_PORTS_END

static INPUT_PORTS_START( styphp )
	PORT_START("INPUTS1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x00000002, IP_ACTIVE_LOW) // Test Button
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )                                    // Coin
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS3")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 )                                  // Shift Down
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON1 )                                  // Shift Up
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON3 )                                  // View
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON4 )                                  // Side Brake
	PORT_BIT( 0x000000e0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS4")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_START1 )                                   // Start
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ANALOG1")       // Steering
	PORT_BIT( 0x3ff, 0x200, IPT_PADDLE ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG2")       // Gas Pedal
	PORT_BIT( 0x3ff, 0x000, IPT_PEDAL) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG3")       // Brake Pedal
	PORT_BIT( 0x3ff, 0x000, IPT_PEDAL2 ) PORT_MINMAX(0x000,0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5)

	PORT_START("ANALOG4")
	PORT_START("ANALOG5")
	PORT_START("ANALOG6")
	PORT_START("ANALOG7")
	PORT_START("ANALOG8")
INPUT_PORTS_END

static INPUT_PORTS_START(dendego3)
	PORT_START("INPUTS1")
	PORT_BIT(0x00000001, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_SERVICE_NO_TOGGLE(0x00000002, IP_ACTIVE_LOW) // Test Button
	PORT_BIT(0x00000004, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00000008, IP_ACTIVE_LOW, IPT_SERVICE) PORT_NAME("Service") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x00000010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00000020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00000040, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00000080, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INPUTS2")
	PORT_BIT(0x00000001, IP_ACTIVE_LOW, IPT_COIN1)                                    // Coin
	PORT_BIT(0x00000002, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00000004, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00000008, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00000010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00000020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00000040, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00000080, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INPUTS3")
	PORT_BIT(0x00000001, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00000002, IP_ACTIVE_LOW, IPT_START1)                                   // Start
	PORT_BIT(0x00000004, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00000008, IP_ACTIVE_LOW, IPT_BUTTON7)                                  // Train Horn
	PORT_BIT(0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1)                                  // "NOCH0"
	PORT_BIT(0x000000e0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INPUTS4")
	PORT_BIT(0x00000001, IP_ACTIVE_LOW, IPT_BUTTON3)                                  // "NOCH2"
	PORT_BIT(0x00000002, IP_ACTIVE_LOW, IPT_BUTTON5)                                  // "NOCH4"
	PORT_BIT(0x00000004, IP_ACTIVE_LOW, IPT_BUTTON2)                                  // "NOCH1"
	PORT_BIT(0x00000008, IP_ACTIVE_LOW, IPT_BUTTON4)                                  // "NOCH3"
	PORT_BIT(0x00000010, IP_ACTIVE_LOW, IPT_BUTTON6)                                  // "NOCH5"
	PORT_BIT(0x00000020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00000040, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x00000080, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ANALOG1")
	PORT_BIT(0x3ff, 0x000, IPT_PADDLE) PORT_MINMAX(0x000, 0x3ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_NAME("Brake Lever")
	PORT_START("ANALOG2")
	PORT_START("ANALOG3")
	PORT_START("ANALOG4")
	PORT_START("ANALOG5")
	PORT_START("ANALOG6")
	PORT_START("ANALOG7")
	PORT_START("ANALOG8")
INPUT_PORTS_END

void taitotz_state::machine_reset()
{
	if (m_hdd_serial_number != nullptr)
	{
		ide_hdd_device *hdd = m_ata->subdevice<ata_slot_device>("0")->subdevice<ide_hdd_device>("hdd");
		u16 *identify_device = hdd->identify_device_buffer();

		for (int i = 0; i < 10; i++)
		{
			identify_device[10 + i] = (m_hdd_serial_number[i * 2] << 8) | m_hdd_serial_number[i * 2 + 1];
		}
	}
}

void taitotz_state::machine_start()
{
	// set conservative DRC options
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	// configure fast RAM regions for DRC
	m_maincpu->ppcdrc_add_fastram(0x40000000, 0x40ffffff, false, m_work_ram);
}

void taitotz_state::vblank(int state)
{
	if (state)
	{
		m_iocpu->set_input_line(TLCS900_INT3, ASSERT_LINE);
	}
}

void taitotz_state::ide_interrupt(int state)
{
	m_iocpu->set_input_line(TLCS900_INT2, state);
}

void taitotz_state::taitotz(machine_config &config)
{
	// IBM EMPPC603eBG-100
	PPC603E(config, m_maincpu, 100000000);
	m_maincpu->set_bus_frequency(XTAL(66'666'700)); // Multiplier 1.5, Bus = 66MHz, Core = 100MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &taitotz_state::ppc603e_mem);

	// TMP95C063F I/O CPU
	TMP95C063(config, m_iocpu, 25000000);
	m_iocpu->port9_read().set_ioport("INPUTS1");
	m_iocpu->portb_read().set_ioport("INPUTS2");
	m_iocpu->portd_read().set_ioport("INPUTS3");
	m_iocpu->porte_read().set_ioport("INPUTS4");
	m_iocpu->an_read<0>().set_ioport("ANALOG1");
	m_iocpu->an_read<1>().set_ioport("ANALOG2");
	m_iocpu->an_read<2>().set_ioport("ANALOG3");
	m_iocpu->an_read<3>().set_ioport("ANALOG4");
	m_iocpu->an_read<4>().set_ioport("ANALOG5");
	m_iocpu->an_read<5>().set_ioport("ANALOG6");
	m_iocpu->an_read<6>().set_ioport("ANALOG7");
	m_iocpu->an_read<7>().set_ioport("ANALOG8");
	m_iocpu->set_addrmap(AS_PROGRAM, &taitotz_state::tlcs900h_mem);

	// MN1020819DA sound CPU (not yet dumped/implemented)

	config.set_maximum_quantum(attotime::from_hz(120));

	ata_interface_device &ata(ATA_INTERFACE(config, "ata").options(ata_devices, "hdd", nullptr, true));
	ata.irq_handler().set(FUNC(taitotz_state::ide_interrupt));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 384);
	m_screen->set_visarea(0, 511, 0, 383);
	m_screen->set_screen_update(FUNC(taitotz_state::screen_update));
	m_screen->screen_vblank().set(FUNC(taitotz_state::vblank));
}

void taitotz_state::landhigh(machine_config &config)
{
	taitotz(config);
	m_iocpu->set_addrmap(AS_PROGRAM, &taitotz_state::landhigh_tlcs900h_mem);
}


// Init for BIOS v1.52
void taitotz_state::init_taitotz_152()
{
	u32 *rom = (u32*)memregion("user1")->base();
	rom[(0x2c87c ^ 4) / 4] = 0x38600000;    // skip sound load timeout...
//  rom[(0x2c620 ^ 4) / 4] = 0x48000014;    // ID check skip (not needed with correct serial number)

	if (ENABLE_DEBUG_PRINTS)
	{
		rom[(0x2c164 ^ 4) / 4] = 0x39600001;        // enable game debug output
		rom[(0x2c174 ^ 4) / 4] = 0x39200001;
		rom[(0x2c978 ^ 4) / 4] = 0x48000028;
	}
}

// Init for BIOS 1.11a
void taitotz_state::init_taitotz_111a()
{
	u32 *rom = (u32*)memregion("user1")->base();
	rom[(0x2b748^4)/4] = 0x480000b8;    // skip sound load timeout
}

static const char LANDHIGH_HDD_SERIAL[] =           // "824915746386        "
	{ 0x38, 0x32, 0x34, 0x39, 0x31, 0x35, 0x37, 0x34, 0x36, 0x33, 0x38, 0x36, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };

static const char LANDHIGHA_HDD_SERIAL[] =          // "824915546750        "
	{ 0x38, 0x32, 0x34, 0x39, 0x31, 0x35, 0x35, 0x34, 0x36, 0x37, 0x35, 0x30, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };

static const char BATLGR2_HDD_SERIAL[] =            // "            05412842"
	{ 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0x35, 0x34, 0x31, 0x32, 0x38, 0x34, 0x32 };

static const char BATLGR2A_HDD_SERIAL[] =           // "            05411645"
	{ 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0x35, 0x34, 0x31, 0x31, 0x36, 0x34, 0x35 };

static const char RAIZPIN_HDD_SERIAL[] =            // "691934013492        "
	{ 0x36, 0x39, 0x31, 0x39, 0x33, 0x34, 0x30, 0x31, 0x33, 0x34, 0x39, 0x32, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };

static const char RAIZPINJ_HDD_SERIAL[] =           // "824915745143        "
	{ 0x38, 0x32, 0x34, 0x39, 0x31, 0x35, 0x37, 0x34, 0x35, 0x31, 0x34, 0x33, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };

static const char STYPHP_HDD_SERIAL[] =             // "            05872160"
	{ 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0x35, 0x38, 0x37, 0x32, 0x31, 0x36, 0x30 };

void taitotz_state::init_dendego3()
{
	init_taitotz_152();

	m_hdd_serial_number = nullptr; // serial is in the CHD metadata

	m_scr_base = 0x1e0000;
}

void taitotz_state::init_landhigh()
{
	init_taitotz_152();

	m_hdd_serial_number = LANDHIGH_HDD_SERIAL;

	m_scr_base = 0x1c0000;
}

void taitotz_state::init_landhigha()
{
	init_taitotz_152();

	m_hdd_serial_number = LANDHIGHA_HDD_SERIAL;

	m_scr_base = 0x1c0000;
}

void taitotz_state::init_batlgear()
{
	init_taitotz_111a();

	// unknown, not used by BIOS 1.11a
	m_hdd_serial_number = nullptr;

	m_scr_base = 0x1c0000;
}

void taitotz_state::init_batlgr2()
{
	init_taitotz_152();

	m_hdd_serial_number = BATLGR2_HDD_SERIAL;

	m_scr_base = 0x1e0000;
}

void taitotz_state::init_batlgr2a()
{
	init_taitotz_152();

	m_hdd_serial_number = BATLGR2A_HDD_SERIAL;

	m_scr_base = 0x1e0000;
}

void taitotz_state::init_pwrshovl()
{
	init_taitotz_111a();

	// unknown, not used by BIOS 1.11a
	m_hdd_serial_number = nullptr;

	m_scr_base = 0x1c0000;
}

void taitotz_state::init_raizpin()
{
	init_taitotz_152();

	m_hdd_serial_number = RAIZPIN_HDD_SERIAL;

	m_scr_base = 0x1c0000;
}

void taitotz_state::init_raizpinj()
{
	init_taitotz_152();

	m_hdd_serial_number = RAIZPINJ_HDD_SERIAL;

	m_scr_base = 0x1c0000;
}

void taitotz_state::init_styphp()
{
	init_taitotz_152();

	m_hdd_serial_number = STYPHP_HDD_SERIAL;

	m_scr_base = 0x1e0000;
}


// Type-Zero System v1.52
#define TAITOTZ_BIOS_V152   \
	ROM_LOAD32_WORD_SWAP( "e68-05-1.ic6", 0x000000, 0x080000, CRC(6ad9b006) SHA1(f05a0ae26b6abaeda9c7944aee96c72b08fff7a5) )    \
	ROM_LOAD32_WORD_SWAP( "e68-04-1.ic5", 0x000002, 0x080000, CRC(c7c2dc6b) SHA1(bf88c818166c285130c5c73d6982f009da26e143) )

// Type-Zero System v1.11a (This was obtained from Battle Gear harddisk. An exact copy is also included in pwrshovl harddisk.)
#define TAITOTZ_BIOS_V111A  \
	ROM_LOAD32_WORD_SWAP( "ic6", 0x000000, 0x080000, CRC(29654245) SHA1(aaa34ff363eb96cf4a785fa6f9f7fc650b5ee93d) ) \
	ROM_LOAD32_WORD_SWAP( "ic5", 0x000002, 0x080000, CRC(8784804a) SHA1(fe9eed5289dcc89f2bc98cb752895b13e44b6097) )


ROM_START( taitotz )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V152

	ROM_REGION16_LE( 0x40000, "io_cpu", ROMREGION_ERASE00 )
	ROM_REGION( 0x10000, "sound_cpu", ROMREGION_ERASE00 ) // Undumped internal ROM
	ROM_REGION( 0x500, "plds", ROMREGION_ERASE00 )
	DISK_REGION( "ata:0:hdd" )
ROM_END

/*

The official drive for (at least) Raizin Ping Pong and Power Shovel is

Quantum Fireball 4.3AT

Formatted Capacity 4,310.43 MB
Logical Heads 9
Logical Cylinders 14,848
Logical Sectors/Track 63
Physical Heads 6
Physical Disks 3
Sectors Per Drive 8,418,816
Average Seek Time  10.0 ms (read)
Buffer Size     128K

*/

ROM_START( landhigh )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V152

	ROM_REGION16_LE( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e82-03.ic14", 0x000000, 0x020000, CRC(0de65b4d) SHA1(932316f7435259b723a29843d58b2e3dca92e7b7) )
	ROM_LOAD16_BYTE( "e82-04.ic15", 0x000001, 0x020000, CRC(b3cb0f3d) SHA1(80414f50a1593c6b849d9f37e94a32168699a5c1) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) // Undumped internal ROM
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	ROM_REGION( 0x500, "plds", 0 )
	ROM_LOAD( "e82-01.ic44", 0x000, 0x117, CRC(49eea30f) SHA1(ef97c792358f05b9214a2f58ee1e97e8208806c4) )
	ROM_LOAD( "e82-02.ic45", 0x117, 0x2dd, CRC(f581cff5) SHA1(468e0e6a3828f2dcda35c6d523154510f9c99db7) )
	ROM_LOAD( "e68-06.ic24", 0x3f4, 0x100, NO_DUMP )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "landhigh", 0, SHA1(7cea4ea5c3899e6ac774a4eb12821f44541d9c9c) )
ROM_END

ROM_START( landhigha )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V152

	ROM_REGION16_LE( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e82-03.ic14", 0x000000, 0x020000, CRC(0de65b4d) SHA1(932316f7435259b723a29843d58b2e3dca92e7b7) )
	ROM_LOAD16_BYTE( "e82-04.ic15", 0x000001, 0x020000, CRC(b3cb0f3d) SHA1(80414f50a1593c6b849d9f37e94a32168699a5c1) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) // Undumped internal ROM
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	ROM_REGION( 0x500, "plds", 0 )
	ROM_LOAD( "e82-01.ic44", 0x000, 0x117, CRC(49eea30f) SHA1(ef97c792358f05b9214a2f58ee1e97e8208806c4) )
	ROM_LOAD( "e82-02.ic45", 0x117, 0x2dd, CRC(f581cff5) SHA1(468e0e6a3828f2dcda35c6d523154510f9c99db7) )
	ROM_LOAD( "e68-06.ic24", 0x3f4, 0x100, NO_DUMP )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "landhigha", 0, SHA1(830ff12671a977a4c243491b68444f8ca69d0819) )
ROM_END

ROM_START( batlgear )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V111A

	ROM_REGION16_LE( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e68-07.ic14",  0x000000, 0x020000, CRC(554c6fd7) SHA1(9f203dead81c7ccf73d7fd462cab147cd17f890f) )
	ROM_LOAD16_BYTE( "e68-08.ic15",  0x000001, 0x020000, CRC(f1932380) SHA1(64d12e858af15a9ba8254917da13863ac7c9c050) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) // Undumped internal ROM
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "batlgear", 0, SHA1(eab283839ad3e0a3e6be11f6482570db334eacca) )
ROM_END

ROM_START( batlgr2 )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V152

	ROM_REGION16_LE( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e87-03.ic14",  0x000000, 0x020000, CRC(49ae7cd0) SHA1(15f07a6bb2044a85a2139481f1dc95a44520c929) )
	ROM_LOAD16_BYTE( "e87-04.ic15",  0x000001, 0x020000, CRC(59f8f75f) SHA1(f5595751b10c0033f460114c43f5e2c192fe61f1) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) // Undumped internal ROM
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	// BATTLE GEAR 2 M9005023A VER.2.04J
	// FUJITSU MPE3064AT
	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "bg2_204j", 0, SHA1(7ac100fba39ae0b93980c0af2f0212a731106912) )
ROM_END

ROM_START( batlgr2a )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V152

	ROM_REGION16_LE( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e87-03.ic14",  0x000000, 0x020000, CRC(49ae7cd0) SHA1(15f07a6bb2044a85a2139481f1dc95a44520c929) )
	ROM_LOAD16_BYTE( "e87-04.ic15",  0x000001, 0x020000, CRC(59f8f75f) SHA1(f5595751b10c0033f460114c43f5e2c192fe61f1) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) // Undumped internal ROM
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "bg2_201j", 0, SHA1(542d12682bd0f95143368578461c6a4fcc492fcc) )
ROM_END

ROM_START( pwrshovl )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V111A

	ROM_REGION16_LE( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e74-04++.ic14", 0x000000, 0x020000, CRC(ef21a261) SHA1(7398826dbf48014b9c7e9454f978f3e419ebc64b) ) // actually labeled E74-04**
	ROM_LOAD16_BYTE( "e74-05++.ic15", 0x000001, 0x020000, CRC(2466217d) SHA1(dc814da3a1679cff001f179d3c1641af985a6490) ) // actually labeled E74-05**

	ROM_REGION( 0x10000, "sound_cpu", 0 ) // Undumped internal ROM
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	ROM_REGION16_LE( 0x20000, "io_cpu2", 0 ) // another TMP95C063F, not hooked up yet
	ROM_LOAD( "e74-06.ic2", 0x000000, 0x020000, CRC(cd4a99d3) SHA1(ea280e05a68308c1c5f1fc0ee8a25b33923df635) ) // located on the I/O PCB

	ROM_REGION( 0x20000, "oki1", 0 )
	ROM_LOAD( "e74-07.ic6", 0x000000, 0x020000, CRC(ca5baccc) SHA1(4594b7a6232b912d698fff053f7e3f51d8e1bfb6) ) // located on the I/O PCB

	ROM_REGION( 0x20000, "oki2", 0 )
	ROM_LOAD( "e74-08.ic8", 0x000000, 0x020000, CRC(ca5baccc) SHA1(4594b7a6232b912d698fff053f7e3f51d8e1bfb6) ) // located on the I/O PCB

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "pwrshovl", 0, SHA1(360f63b39f645851c513b4644fb40601b9ba1412) )
ROM_END

ROM_START( pwrshovla )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V111A

	ROM_REGION16_LE( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e74-04++.ic14", 0x000000, 0x020000, CRC(ef21a261) SHA1(7398826dbf48014b9c7e9454f978f3e419ebc64b) ) // actually labeled E74-04**
	ROM_LOAD16_BYTE( "e74-05++.ic15", 0x000001, 0x020000, CRC(2466217d) SHA1(dc814da3a1679cff001f179d3c1641af985a6490) ) // actually labeled E74-05**

	ROM_REGION( 0x10000, "sound_cpu", 0 ) // Undumped internal ROM
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	ROM_REGION16_LE( 0x20000, "io_cpu2", 0 ) // another TMP95C063F, not hooked up yet
	ROM_LOAD( "e74-06.ic2", 0x000000, 0x020000, CRC(cd4a99d3) SHA1(ea280e05a68308c1c5f1fc0ee8a25b33923df635) ) // located on the I/O PCB

	ROM_REGION( 0x20000, "oki1", 0 )
	ROM_LOAD( "e74-07.ic6", 0x000000, 0x020000, CRC(ca5baccc) SHA1(4594b7a6232b912d698fff053f7e3f51d8e1bfb6) ) // located on the I/O PCB

	ROM_REGION( 0x20000, "oki2", 0 )
	ROM_LOAD( "e74-08.ic8", 0x000000, 0x020000, CRC(ca5baccc) SHA1(4594b7a6232b912d698fff053f7e3f51d8e1bfb6) ) // located on the I/O PCB

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "power shovel ver.2.07j", 0, SHA1(05410d4b4972262ef93400b02f21dd17d10b1c5e) )
ROM_END

ROM_START( raizpin )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V152

	ROM_REGION16_LE( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "f14-01.ic14",  0x000000, 0x020000, CRC(f86a184d) SHA1(46abd11430c08d4f384fb79a5a3a39e54f83b8d8) )
	ROM_LOAD16_BYTE( "f14-02.ic15",  0x000001, 0x020000, CRC(bd2d0dee) SHA1(652f810702598184551de9fd69436862d48c1608) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) // Undumped internal ROM
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "raizpin", 0, SHA1(883ebcda03026df31da1cdb95af521e100c171ed) )
ROM_END

ROM_START( raizpinj )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V152

	ROM_REGION16_LE( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "f14-01.ic14",  0x000000, 0x020000, CRC(f86a184d) SHA1(46abd11430c08d4f384fb79a5a3a39e54f83b8d8) )
	ROM_LOAD16_BYTE( "f14-02.ic15",  0x000001, 0x020000, CRC(bd2d0dee) SHA1(652f810702598184551de9fd69436862d48c1608) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) // Undumped internal ROM
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "raizin ping pong ver 2.01j", 0, SHA1(eddc803c2507d19f0a3e3cc217bb22a565c04f3e) )
ROM_END

ROM_START( styphp )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V152

	ROM_REGION16_LE( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e98-01.ic14", 0x000000, 0x020000, CRC(479b37ad) SHA1(a0e59d990665244a7919a104dc4b6869ccf90be1) )
	ROM_LOAD16_BYTE( "e98-02.ic15", 0x000001, 0x020000, CRC(d8da590f) SHA1(b33a67d81e388d7863adaf03041911c7f84d193b) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) // Undumped internal ROM
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "styphp", 0, SHA1(c232d3460e37523346132544b8e23a5f9b447150) )
ROM_END

ROM_START( dendego3 )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V152

	ROM_REGION16_LE( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e85-01.ic14", 0x000000, 0x020000, CRC(e16eba2a) SHA1(bd45117bb39cb98d93cdeb17dc72815e6000196b) )
	ROM_LOAD16_BYTE( "e85-02.ic15", 0x000001, 0x020000, CRC(aa44e992) SHA1(9d176c150c18b085e2c2058507a34151214c1a02) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) // Internal ROM
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	ROM_REGION16_LE( 0x20000, "io_cpu2", 0 ) // another TMP95C063F, not hooked up yet
	ROM_LOAD( "e85-03.ic2", 0x000000, 0x020000, CRC(47712427) SHA1(69756f0331ae0a47214d430bc8942937878ddee4) ) // located on the I/O PCB

	ROM_REGION( 0x20000, "oki1", 0 )
	ROM_LOAD( "e74-07.ic6", 0x000000, 0x020000, CRC(ca5baccc) SHA1(4594b7a6232b912d698fff053f7e3f51d8e1bfb6) ) // located on the I/O PCB

	DISK_REGION( "ata:0:hdd" ) // Fujitsu MPF3102AT
	DISK_IMAGE( "ddg3", 0, SHA1(468d699e02ef0a0242de4e7038613cc5d0545591) )
ROM_END

} // anonymous namespace


GAME( 1999, taitotz,   0,        taitotz,  taitotz,  taitotz_state, empty_init,    ROT0, "Taito", "Type Zero BIOS", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_IS_BIOS_ROOT )
GAME( 1998, batlgear,  taitotz,  taitotz,  batlgr2,  taitotz_state, init_batlgear, ROT0, "Taito", "Battle Gear (Ver 2.40 A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_NODEVICE_LAN )
GAME( 1999, landhigh,  taitotz,  landhigh, landhigh, taitotz_state, init_landhigh, ROT0, "Taito", "Landing High Japan (Ver 2.01 OK)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1999, landhigha, landhigh, landhigh, landhigh, taitotz_state, init_landhigha,ROT0, "Taito", "Landing High Japan (Ver 2.02 O)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1999, pwrshovl,  taitotz,  taitotz,  pwrshovl, taitotz_state, init_pwrshovl, ROT0, "Taito", "Power Shovel ni Norou!! - Power Shovel Simulator (v2.07J)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 1999/8/5 19:13:35
GAME( 1999, pwrshovla, pwrshovl, taitotz,  pwrshovl, taitotz_state, init_pwrshovl, ROT0, "Taito", "Power Shovel ni Norou!! - Power Shovel Simulator (v2.07J, alt)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // seem to be some differences in drive content, but identifies as the same revision, is it just user data changes??
GAME( 2000, batlgr2,   taitotz,  taitotz,  batlgr2,  taitotz_state, init_batlgr2,  ROT0, "Taito", "Battle Gear 2 (v2.04J)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_NODEVICE_LAN )
GAME( 2000, batlgr2a,  batlgr2,  taitotz,  batlgr2,  taitotz_state, init_batlgr2a, ROT0, "Taito", "Battle Gear 2 (v2.01J)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_NODEVICE_LAN )
GAME( 2000, dendego3,  taitotz,  taitotz,  dendego3, taitotz_state, init_dendego3, ROT0, "Taito", "Densha de GO 3! Tsukin-hen (V2.03J)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 2001/01/27 09:52:56
GAME( 2000, styphp,    taitotz,  taitotz,  styphp,   taitotz_state, init_styphp,   ROT0, "Taito", "Stunt Typhoon Plus (Ver 2.04 J)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2002, raizpin,   taitotz,  taitotz,  taitotz,  taitotz_state, init_raizpin,  ROT0, "Taito", "Raizin Ping Pong (V2.01O)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2002, raizpinj,  raizpin,  taitotz,  taitotz,  taitotz_state, init_raizpinj, ROT0, "Taito", "Raizin Ping Pong (V2.01J)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
