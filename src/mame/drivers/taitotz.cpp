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
EPSON 9X5C pscillator near IC55
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

E74-07.IC6 & E74-08.IC8 are the OKI samples and are indentical
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
#include "cpu/powerpc/ppc.h"
#include "cpu/tlcs900/tlcs900.h"
#include "machine/ataintf.h"
#include "machine/idehd.h"
#include "machine/nvram.h"
#include "video/poly.h"

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
                                0 = Polygon data address (21 bits) in screen RAM (in dwords). If 0x400000 set, no matrix.
                                1 = Object scale (float), if <1.0f multiply scale by 2.0f
                                2 = 0x80 = object alpha enable, 0x1f = object alpha (0x1f = opaque)
                                3 = ?
                                4...15 = 4x3 matrix

                    0xffff0020: ? (no parameters)

                    0xffff0021: Draw Pretransformed Polygon (used for drawing the background images)
                                0 = address? (if 0x400 set, 24 words)
                                1 = Texture number (* 0x4000 to address the texture)
                                2 = ?
                                3 = ? (usually 0)

                                Vertex data: 4x or 5x 4 words
                                0 = XY coordinates. X = high 16 bits, Y = low 16 bits
                                1 = Z value?
                                2 = UV coordinates. U = high 16 bits, V = low 16 bits (possibly shifted left by 4?)
                                3 = ? (usually 0)

                    0xffff0022: ? (1 word)
                                0 = ? (Set to 0x00000001 before drawing the background. Z-buffer read disable?)

                    0xffff0030: ? (2 + n words) FIFO write?
                                0 = ?
                                1 = num of words
                                n words = ?

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
                    -------- xxxxxxxx -------- --------     Usually 0x40. HUD elements set 0xff. Self-illumination?
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

#define LOG_PPC_TO_TLCS_COMMANDS        1
#define LOG_TLCS_TO_PPC_COMMANDS        1

#define LOG_DISPLAY_LIST                0
#define ENABLE_LIGHTING                 1

#define PPC_TLCS_COMM_TRIGGER           12345
#define TLCS_PPC_COMM_TRIGGER           12346

struct PLANE
{
	float x, y, z, d;
};

typedef float VECTOR3[3];

struct taitotz_polydata
{
	UINT32 texture;
	UINT32 alpha;
	UINT32 flags;
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
		m_maincpu(*this, "maincpu"),
		m_iocpu(*this, "iocpu"),
		m_work_ram(*this, "work_ram"),
		m_mbox_ram(*this, "mbox_ram"),
		m_ata(*this, "ata"),
		m_screen(*this, "screen")
	{
	}

	required_device<ppc_device> m_maincpu;
	required_device<cpu_device> m_iocpu;
	required_shared_ptr<UINT64> m_work_ram;
	required_shared_ptr<UINT16> m_mbox_ram;
	required_device<ata_interface_device> m_ata;
	required_device<screen_device> m_screen;

	DECLARE_READ64_MEMBER(ppc_common_r);
	DECLARE_WRITE64_MEMBER(ppc_common_w);
	DECLARE_READ64_MEMBER(ieee1394_r);
	DECLARE_WRITE64_MEMBER(ieee1394_w);
	DECLARE_READ64_MEMBER(video_chip_r);
	DECLARE_WRITE64_MEMBER(video_chip_w);
	DECLARE_READ64_MEMBER(video_fifo_r);
	DECLARE_WRITE64_MEMBER(video_fifo_w);

	std::unique_ptr<UINT32[]> m_screen_ram;
	std::unique_ptr<UINT32[]> m_frame_ram;
	std::unique_ptr<UINT32[]> m_texture_ram;
	UINT32 m_video_unk_reg[0x10];

	UINT32 m_video_fifo_ptr;
	UINT32 m_video_ram_ptr;
	UINT32 m_video_reg;
	UINT32 m_scr_base;

	UINT64 m_video_fifo_mem[4];

	UINT16 m_io_share_ram[0x2000];

	const char *m_hdd_serial_number;

	DECLARE_READ8_MEMBER(tlcs_common_r);
	DECLARE_WRITE8_MEMBER(tlcs_common_w);
	DECLARE_READ8_MEMBER(tlcs_rtc_r);
	DECLARE_WRITE8_MEMBER(tlcs_rtc_w);

	UINT8 m_rtcdata[8];


	UINT32 m_reg105;
	UINT32 m_displist_addr;
	int m_count;

	std::unique_ptr<taitotz_renderer> m_renderer;
	DECLARE_DRIVER_INIT(batlgr2a);
	DECLARE_DRIVER_INIT(batlgr2);
	DECLARE_DRIVER_INIT(pwrshovl);
	DECLARE_DRIVER_INIT(batlgear);
	DECLARE_DRIVER_INIT(landhigh);
	DECLARE_DRIVER_INIT(raizpin);
	DECLARE_DRIVER_INIT(styphp);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_taitotz(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(taitotz_vbi);
	DECLARE_READ16_MEMBER(tlcs_ide0_r);
	DECLARE_READ16_MEMBER(tlcs_ide1_r);
	DECLARE_WRITE_LINE_MEMBER(ide_interrupt);
	void draw_tile(UINT32 pos, UINT32 tile);
	UINT32 video_mem_r(UINT32 address);
	void video_mem_w(UINT32 address, UINT32 data);
	UINT32 video_reg_r(UINT32 reg);
	void video_reg_w(UINT32 reg, UINT32 data);
	void init_taitotz_152();
	void init_taitotz_111a();
};

class taitotz_renderer : public poly_manager<float, taitotz_polydata, 6, 50000>
{
public:
	taitotz_renderer(taitotz_state &state, int width, int height, UINT32 *texram)
		: poly_manager<float, taitotz_polydata, 6, 50000>(state.machine()),
			m_state(state)
	{
		m_zbuffer = std::make_unique<bitmap_ind32>(width, height);
		m_texture = texram;

		m_diffuse_intensity = 224;
		m_ambient_intensity = 32;
		m_specular_intensity = 256;
		m_specular_power = 20;
	}

	void set_fb(bitmap_rgb32 *fb) { m_fb = fb; }
	void render_displaylist(const rectangle &cliprect);
	void draw_object(UINT32 address, float scale, UINT8 alpha);
	float line_plane_intersection(const vertex_t *v1, const vertex_t *v2, PLANE cp);
	int clip_polygon(const vertex_t *v, int num_vertices, PLANE cp, vertex_t *vout);
	void setup_viewport(int x, int y, int width, int height, int center_x, int center_y);
	void draw_scanline_noz(INT32 scanline, const extent_t &extent, const taitotz_polydata &extradata, int threadid);
	void draw_scanline(INT32 scanline, const extent_t &extent, const taitotz_polydata &extradata, int threadid);

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

	//static const float ZBUFFER_MAX = 10000000000.0f;

	taitotz_state &m_state;
	bitmap_rgb32 *m_fb;
	std::unique_ptr<bitmap_ind32> m_zbuffer;
	UINT32 *m_texture;

	PLANE m_clip_plane[6];
	float m_matrix[4][3];

	float m_diffuse_intensity;
	float m_ambient_intensity;
	float m_specular_intensity;
	float m_specular_power;

	int m_ambient_r;
	int m_ambient_g;
	int m_ambient_b;
	int m_diffuse_r;
	int m_diffuse_g;
	int m_diffuse_b;
	int m_specular_r;
	int m_specular_g;
	int m_specular_b;

	float m_vp_center_x;
	float m_vp_center_y;
	float m_vp_focus;
	float m_vp_x;
	float m_vp_y;
	float m_vp_mul;

	UINT32 m_reg_100;
	UINT32 m_reg_101;
	UINT32 m_reg_102;

	UINT32 m_reg_10000100;
	UINT32 m_reg_10000101;
};



/*
void taitotz_state::taitotz_exit()
{


    FILE *file;
    int i;

    file = fopen("screen_ram.bin","wb");
    for (i=0; i < 0x200000; i++)
    {
        fputc((UINT8)(m_screen_ram[i] >> 24), file);
        fputc((UINT8)(m_screen_ram[i] >> 16), file);
        fputc((UINT8)(m_screen_ram[i] >> 8), file);
        fputc((UINT8)(m_screen_ram[i] >> 0), file);
    }
    fclose(file);

    file = fopen("frame_ram.bin","wb");
    for (i=0; i < 0x80000; i++)
    {
        fputc((UINT8)(m_frame_ram[i] >> 24), file);
        fputc((UINT8)(m_frame_ram[i] >> 16), file);
        fputc((UINT8)(m_frame_ram[i] >> 8), file);
        fputc((UINT8)(m_frame_ram[i] >> 0), file);
    }
    fclose(file);

    file = fopen("texture_ram.bin","wb");
    for (i=0; i < 0x800000; i++)
    {
        fputc((UINT8)(m_texture_ram[i] >> 24), file);
        fputc((UINT8)(m_texture_ram[i] >> 16), file);
        fputc((UINT8)(m_texture_ram[i] >> 8), file);
        fputc((UINT8)(m_texture_ram[i] >> 0), file);
    }
    fclose(file);

}
*/
void taitotz_state::video_start()
{
	int width = m_screen->width();
	int height = m_screen->height();

	m_screen_ram = std::make_unique<UINT32[]>(0x200000);
	m_frame_ram = std::make_unique<UINT32[]>(0x80000);
	m_texture_ram = std::make_unique<UINT32[]>(0x800000);

	/* create renderer */
	m_renderer = std::make_unique<taitotz_renderer>(*this, width, height, m_texture_ram.get());

	//machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(taitotz_exit), &machine()));
}

static const float dot3_tex_table[32] =
{
	-0.500000f, -0.466666f, -0.433333f, -0.400000f, -0.366666f, -0.333333f, -0.300000f, -0.266666f,
	-0.233333f, -0.200000f, -0.166666f, -0.133333f, -0.100000f, -0.066666f, -0.033333f, -0.000000f,
		0.000000f,  0.033333f,  0.066666f,  0.100000f,  0.133333f,  0.166666f,  0.200000f,  0.233333f,
		0.266666f,  0.300000f,  0.333333f,  0.366666f,  0.400000f,  0.433333f,  0.466666f,  0.500000f,
};

static inline float dot_product_vec3(VECTOR3 a, VECTOR3 b)
{
	return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
}

// Fast inverse square-root
// Adapted from http://en.wikipedia.org/wiki/Fast_inverse_square_root
static inline float finvsqrt(float number)
{
	UINT32 i;
	float x2, y;
	const float threehalfs = 1.5f;

	x2 = number * 0.5f;
	y  = number;
	i  = *(UINT32*)&y;
	i  = 0x5f3759df - ( i >> 1 );
	y  = *(float*)&i;
	y  = y * ( threehalfs - ( x2 * y * y ) );
	return y;
}

#if 0
static inline void normalize_vec3(VECTOR3 *v)
{
	float l = finvsqrt(*v[0] * *v[0] + *v[1] * *v[1] + *v[2] * *v[2]);
	*v[0] *= l;
	*v[1] *= l;
	*v[2] *= l;
}
#endif

static inline float clamp_pos(float v)
{
	if (v < 0.0f)
		return 0.0f;
	else
		return v;
}

static inline UINT32 generate_texel_address(int iu, int iv)
{
	// generate texel address from U and V
	UINT32 addr = 0;
	addr += (iu & 0x01) ? 1 : 0;
	addr += (iu >> 1) * 4;
	addr += (iv & 0x01) ? 2 : 0;
	addr += (iv >> 1) * 128;

	return addr;
}

void taitotz_renderer::draw_scanline_noz(INT32 scanline, const extent_t &extent, const taitotz_polydata &extradata, int threadid)
{
	UINT32 *fb = &m_fb->pix32(scanline);

	float u = extent.param[POLY_U].start;
	float v = extent.param[POLY_V].start;
	float du = extent.param[POLY_U].dpdx;
	float dv = extent.param[POLY_V].dpdx;

	UINT32 *texram = &m_texture[extradata.texture * 0x1000];

	int shift = 16;     // TODO: subtexture

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		int iu = (int)(u) & 0x3f;
		int iv = (int)(v) & 0x3f;

		UINT32 addr = generate_texel_address(iu, iv);

		UINT32 texel = (texram[addr] >> shift) & 0xffff;
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

void taitotz_renderer::draw_scanline(INT32 scanline, const extent_t &extent, const taitotz_polydata &extradata, int threadid)
{
	UINT32 *fb = &m_fb->pix32(scanline);
	float *zb = (float*)&m_zbuffer->pix32(scanline);

	float ooz = extent.param[POLY_Z].start;
	float uoz = extent.param[POLY_U].start;
	float voz = extent.param[POLY_V].start;
	float dooz = extent.param[POLY_Z].dpdx;
	float duoz = extent.param[POLY_U].dpdx;
	float dvoz = extent.param[POLY_V].dpdx;

	float nx= extent.param[POLY_NX].start;
	float dnx = extent.param[POLY_NX].dpdx;
	float ny = extent.param[POLY_NY].start;
	float dny = extent.param[POLY_NY].dpdx;
	float nz = extent.param[POLY_NZ].start;
	float dnz = extent.param[POLY_NZ].dpdx;

	UINT32 *texram = &m_texture[extradata.texture * 0x1000];
	UINT32 alpha = extradata.alpha & 0x1f;
	UINT32 alpha_enable = extradata.alpha & 0x80;

	int texmode = extradata.flags & 0x3;

#if ENABLE_LIGHTING
	int diff_r = extradata.diffuse_r;
	int diff_g = extradata.diffuse_g;
	int diff_b = extradata.diffuse_b;
	int amb_r = extradata.ambient_r;
	int amb_g = extradata.ambient_g;
	int amb_b = extradata.ambient_b;
	int spec_r = extradata.specular_r;
	int spec_g = extradata.specular_g;
	int spec_b = extradata.specular_b;

	VECTOR3 view = { -0.0f, -0.0f, -1.0f };

	VECTOR3 light_vector;
	light_vector[0] = extradata.light[0];
	light_vector[1] = extradata.light[1];
	light_vector[2] = extradata.light[2];
#endif

	// TODO: these might get swapped around, the color texture doesn't always seem to be tex0
	//int tex0_shift = (extradata.flags & 1) ? 16 : 0;
	int tex0_shift, tex1_shift;
	switch (texmode)
	{
		case 0: tex0_shift = 16;  tex1_shift = 0; break;
		case 1: tex0_shift = 16; tex1_shift = 0; break;
		case 2: tex0_shift = 16; tex1_shift = 0;  break;
		default: tex0_shift = 0;  tex1_shift = 16; break;
	}

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		if (ooz > zb[x])
		{
			float z = 1.0f / ooz;
			float u = uoz * z;
			float v = voz * z;
			int iu = (int)(u) & 0x3f;
			int iv = (int)(v) & 0x3f;

			UINT32 addr = generate_texel_address(iu, iv);

			UINT32 texel = texram[addr];
			UINT32 texel0 = (texel >> tex0_shift) & 0xffff;
			if (!(texel0 & 0x8000))
			{
				// extract texel0 RGB
				int r0 = (texel0 & 0x7c00) >> 7;
				int g0 = (texel0 & 0x03e0) >> 2;
				int b0 = (texel0 & 0x001f) << 3;

#if ENABLE_LIGHTING
				// fetch texture1 and apply normal map
				UINT32 texel1 = (texel >> tex1_shift) & 0xffff;

				VECTOR3 normal;
				VECTOR3 half;

				float bumpx = dot3_tex_table[(texel1 & 0x7c00) >> 10];
				float bumpy = dot3_tex_table[(texel1 & 0x03e0) >>  5];
				float bumpz = dot3_tex_table[(texel1 & 0x001f)];

				// FIXME!!! the normal needs to be in tangent-space so the bump normal can be applied!!!
				normal[0] = nx + bumpx;
				normal[1] = ny + bumpy;
				normal[2] = nz + bumpz;

				// normalize
				float l = finvsqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
				normal[0] *= l;
				normal[1] *= l;
				normal[2] *= l;

				// calculate per-pixel lighting
				float dot = dot_product_vec3(normal, light_vector);


				// calculate half-angle vector for specular
				half[0] = light_vector[0] + view[0];
				half[1] = light_vector[1] + view[1];
				half[2] = light_vector[2] + view[2];
				// normalize it
				l = finvsqrt(half[0] * half[0] + half[1] * half[1] + half[2] * half[2]);
				half[0] *= l;
				half[1] *= l;
				half[2] *= l;

				// calculate specularity
				int specular = (int)(pow(clamp_pos(dot_product_vec3(normal, half)), m_specular_power) * m_specular_intensity);
				int intensity = (int)(dot * m_diffuse_intensity);

				// apply lighting
				r0 = ((r0 * intensity * diff_r) >> 16) + ((r0 * amb_r) >> 8) + ((specular * spec_r) >> 8);
				g0 = ((g0 * intensity * diff_g) >> 16) + ((g0 * amb_g) >> 8) + ((specular * spec_g) >> 8);
				b0 = ((b0 * intensity * diff_b) >> 16) + ((b0 * amb_b) >> 8) + ((specular * spec_b) >> 8);

				if (r0 > 255) r0 = 255;
				if (g0 > 255) g0 = 255;
				if (b0 > 255) b0 = 255;
				if (r0 < 0) r0 = 0;
				if (g0 < 0) g0 = 0;
				if (b0 < 0) b0 = 0;
#endif

				// texture alpha mask
				if (texmode == 2)
				{
					// extract texel0 RGB
					int r0 = (texel0 & 0x7c00) >> 7;
					int g0 = (texel0 & 0x03e0) >> 2;
					int b0 = (texel0 & 0x001f) << 3;

					// fetch texture1
					UINT32 texel1 = (texel >> tex1_shift) & 0xffff;

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
					UINT32 texel1 = (texel >> tex1_shift) & 0xffff;

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
	float t;
	int i;

	int previ = num_vertices - 1;

	for (i=0; i < num_vertices; i++)
	{
		int v1_in = is_point_inside(v[i].x, v[i].y, v[i].p[POLY_Z], cp);
		int v2_in = is_point_inside(v[previ].x, v[previ].y, v[previ].p[POLY_Z], cp);

		if (v1_in && v2_in)         /* edge is completely inside the volume */
		{
			clipv[clip_verts] = v[i];
			++clip_verts;
		}
		else if (!v1_in && v2_in)   /* edge is entering the volume */
		{
			/* insert vertex at intersection point */
			t = line_plane_intersection(&v[i], &v[previ], cp);
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
		else if (v1_in && !v2_in)   /* edge is leaving the volume */
		{
			/* insert vertex at intersection point */
			t = line_plane_intersection(&v[i], &v[previ], cp);
			clipv[clip_verts].x = v[i].x + ((v[previ].x - v[i].x) * t);
			clipv[clip_verts].y = v[i].y + ((v[previ].y - v[i].y) * t);
			clipv[clip_verts].p[POLY_Z] = v[i].p[POLY_Z] + ((v[previ].p[POLY_Z] - v[i].p[POLY_Z]) * t);
			clipv[clip_verts].p[POLY_U] = v[i].p[POLY_U] + ((v[previ].p[POLY_U] - v[i].p[POLY_U]) * t);
			clipv[clip_verts].p[POLY_V] = v[i].p[POLY_V] + ((v[previ].p[POLY_V] - v[i].p[POLY_V]) * t);

			clipv[clip_verts].p[POLY_NX] = v[i].p[POLY_NX] + ((v[previ].p[POLY_NX] - v[i].p[POLY_NX]) * t);
			clipv[clip_verts].p[POLY_NY] = v[i].p[POLY_NY] + ((v[previ].p[POLY_NY] - v[i].p[POLY_NY]) * t);
			clipv[clip_verts].p[POLY_NZ] = v[i].p[POLY_NZ] + ((v[previ].p[POLY_NZ] - v[i].p[POLY_NZ]) * t);

			++clip_verts;

			/* insert the existing vertex */
			clipv[clip_verts] = v[i];
			++clip_verts;
		}

		previ = i;
	}
	memcpy(&vout[0], &clipv[0], sizeof(vout[0]) * clip_verts);
	return clip_verts;
}

void taitotz_renderer::draw_object(UINT32 address, float scale, UINT8 alpha)
{
	const rectangle& visarea = m_state.m_screen->visible_area();

	UINT32 *src = &m_state.m_screen_ram[address];
	taitotz_renderer::vertex_t v[10];


	// fetch global light vector
	int ilx = (m_reg_10000100 >> 16) & 0x1ff;
	if (ilx & 0x100) ilx |= 0xfffffe00;
	int ily = m_reg_10000100 & 0x1ff;
	if (ily & 0x100) ily |= 0xfffffe00;
	int ilz = m_reg_10000101 & 0x7f;

	float light_x = (float)(ilx) / 127.0f;
	float light_y = (float)(ily) / 127.0f;
	float light_z = (float)(ilz) / 127.0f;

	// normalize
	float l = finvsqrt(light_x * light_x + light_y * light_y + light_z * light_z);
	light_x *= l;
	light_y *= l;
	light_z *= l;

	int end = 0;
	int index = 0;
	do
	{
		taitotz_polydata &extra = object_data_alloc();

		int num_verts;

		if (src[index] & 0x10000000)
			end = 1;

		if (src[index] & 0x01000000)
			num_verts = 4;
		else
			num_verts = 3;

		int texture = src[index] & 0x7ff;
		int tex_switch = (src[index+3] >> 26) & 3;

		/*
		INT8 polyinx = (src[index+1] >> 16) & 0xff;
		INT8 polyiny = (src[index+1] >> 8) & 0xff;
		INT8 polyinz = src[index+1] & 0xff;
		float polynx = (float)(polyinx) / 128.0f;
		float polyny = (float)(polyiny) / 128.0f;
		float polynz = (float)(polyinz) / 128.0f;
		*/

		index += 4;

		for (int i=0; i < num_verts; i++)
		{
			// texture coordinates
			UINT8 tu = (src[index] >> 8) & 0xff;
			UINT8 tv = (src[index] >> 0) & 0xff;
			v[i].p[POLY_U] = (float)(tu);
			v[i].p[POLY_V] = (float)(tv);

			// coordinates
			INT16 x = src[index + 1] & 0xffff;
			INT16 y = src[index + 2] & 0xffff;
			INT16 z = src[index + 3] & 0xffff;
			float px = ((float)(x) / 256.0f) * scale;
			float py = ((float)(y) / 256.0f) * scale;
			float pz = ((float)(z) / 256.0f) * scale;

			// normals
			INT8 inx = (src[index + 1] >> 16) & 0xff;
			INT8 iny = (src[index + 2] >> 16) & 0xff;
			INT8 inz = (src[index + 3] >> 16) & 0xff;
			float nx = (float)(inx) / 128.0f;
			float ny = (float)(iny) / 128.0f;
			float nz = (float)(inz) / 128.0f;

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
		extra.diffuse_r = m_diffuse_r;
		extra.diffuse_g = m_diffuse_g;
		extra.diffuse_b = m_diffuse_b;
		extra.ambient_r = m_ambient_r;
		extra.ambient_g = m_ambient_g;
		extra.ambient_b = m_ambient_b;
		extra.specular_r = m_specular_r;
		extra.specular_g = m_specular_g;
		extra.specular_b = m_specular_b;
		extra.light[0] = light_x;
		extra.light[1] = light_y;
		extra.light[2] = -light_z;

		for (int i=2; i < num_verts; i++)
		{
			render_triangle(visarea, render_delegate(FUNC(taitotz_renderer::draw_scanline), this), 6, v[0], v[i-1], v[i]);
		}
	}
	while (!end);
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
	//float angleh =  atan2(512/2, 512.0f) - 0.0001;
	//float anglev =  atan2(384/2, 512.0f) - 0.0001;

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

void taitotz_renderer::render_displaylist(const rectangle &cliprect)
{
	float zvalue = 0;//ZBUFFER_MAX;
	m_zbuffer->fill(*(int*)&zvalue, cliprect);

	const rectangle& visarea = m_state.m_screen->visible_area();
	vertex_t v[8];

	UINT32 *src = (UINT32*)&m_state.m_work_ram[0];

	UINT32 w[32];
	int j;
	int end = 0;

	UINT32 index = m_state.m_displist_addr / 4;

	setup_viewport(0, 0, 256, 192, 256, 192);


	m_specular_r = 0xff;
	m_specular_g = 0xff;
	m_specular_b = 0xff;
	m_diffuse_r = 0;
	m_diffuse_g = 0;
	m_diffuse_b = 0;
	m_ambient_r = 0;
	m_ambient_g = 0;
	m_ambient_b = 0;


#if LOG_DISPLAY_LIST
	printf("--------------------------------------------\n");
	printf("Start of displist\n");
#endif

	do
	{
		UINT32 cmd = src[index^1];
		index++;

		if (cmd == 0xffff0011)
		{
			for (j=0; j < 16; j++)
			{
				w[j] = src[index^1];
				index++;
			}

			m_matrix[0][0]= *(float*)&w[4];
			m_matrix[1][0]= *(float*)&w[5];
			m_matrix[2][0]= *(float*)&w[6];
			m_matrix[3][0]= *(float*)&w[7];

			m_matrix[0][1]= *(float*)&w[8];
			m_matrix[1][1]= *(float*)&w[9];
			m_matrix[2][1]= *(float*)&w[10];
			m_matrix[3][1]= *(float*)&w[11];

			m_matrix[0][2]= *(float*)&w[12];
			m_matrix[1][2]= *(float*)&w[13];
			m_matrix[2][2]= *(float*)&w[14];
			m_matrix[3][2]= *(float*)&w[15];

			float scale = *(float*)&w[1];
			if (scale < 1.0f)
				scale *= 2;

			UINT32 alpha = w[2];

			draw_object(w[0] & 0x1fffff, scale, alpha);

#if LOG_DISPLAY_LIST
			printf("0xffff0011:   %08X, %08X, %08X, %08X\n", w[0], w[1], w[2], w[3]);
#endif
		}
		else if (cmd == 0xffff0010)
		{
#if LOG_DISPLAY_LIST
			printf("0xffff0010\n");
#endif
		}
		else if (cmd == 0xffff0020)
		{
#if LOG_DISPLAY_LIST
			printf("0xffff0020\n");
#endif
		}
		else if (cmd == 0xffff0021)
		{
			taitotz_polydata &extra = object_data_alloc();

			int num_verts;
			w[0] = src[index^1];
			index++;
			w[1] = src[index^1];
			index++;
			w[2] = src[index^1];
			index++;
			w[3] = src[index^1];
			index++;

			num_verts = ((w[0] >> 8) & 0xf) + 1;

			for (j=0; j < num_verts; j++)
			{
				w[4] = src[index^1];
				index++;
				w[5] = src[index^1];
				index++;
				w[6] = src[index^1];
				index++;
				w[7] = src[index^1];
				index++;

				UINT16 x = (w[4] >> 16) & 0xffff;
				UINT16 y = (w[4] >>  0) & 0xffff;
				UINT16 tu = (w[6] >> 20) & 0xfff;
				UINT16 tv = (w[6] >>  4) & 0xfff;
//              UINT16 z = (w[5] & 0xffff);

				v[j].x = x;
				v[j].y = y;     // batlgear needs -50 modifier here (why?)
				v[j].p[POLY_U] = tu;
				v[j].p[POLY_V] = tv;
			}

			extra.texture = w[1] & 0x7ff;

			for (j=2; j < num_verts; j++)
			{
				render_triangle(visarea, render_delegate(FUNC(taitotz_renderer::draw_scanline_noz), this), 3, v[0], v[j-1], v[j]);
			}

#if LOG_DISPLAY_LIST
			printf("0xffff0021:   %08X, %08X, %08X, %08X\n", w[0], w[1], w[2], w[3]);
#endif
		}
		else if (cmd == 0xffff0022)
		{
			w[0] = src[index^1];
			index++;

#if LOG_DISPLAY_LIST
			printf("0xffff0022:    %08X\n", w[0]);
#endif
		}
		else if (cmd == 0xffff0030)
		{
			UINT32 address = src[index^1];
			index++;
			UINT32 num = src[index^1];
			index++;

#if LOG_DISPLAY_LIST
			printf("0xffff0030:   %08X = ", address);
#endif

			int addr = address;

			for (j=0; j < num; j++)
			{
				UINT32 word = src[index^1];
				index++;

				if (addr == 0x100)
				{
					m_reg_100 = word;
				}
				else if (addr == 0x101)
				{
					m_reg_101 = word;
				}
				else if (addr == 0x102)
				{
					m_reg_102 = word;
				}
				else if (addr == 0x10000100)
				{
					m_reg_10000100 = word;
				}
				else if (addr == 0x10000101)
				{
					m_reg_10000101 = word;
				}
				else if (addr == 0x10000102)
				{
					m_diffuse_r = ((word >> (16+10)) & 0x1f) << 3;
					m_diffuse_g = ((word >> (16+5)) & 0x1f) << 3;
					m_diffuse_b = ((word >> 16) & 0x1f) << 3;
					m_ambient_r = ((word >> 10) & 0x1f) << 3;
					m_ambient_g = ((word >> 5) & 0x1f) << 3;
					m_ambient_b = (word & 0x1f) << 3;
				}
				addr++;

#if LOG_DISPLAY_LIST
				printf("%08X, ", word);
#endif
			}
#if LOG_DISPLAY_LIST
			printf("\n");
#endif

			if (address == 0x100)
			{
				int vpw = (m_reg_101 >> 16) & 0xffff;
				int vph = (m_reg_101 >>  0) & 0xffff;
				int xw = (m_reg_100 >>  0) & 0xffff;
				int xh = (m_reg_100 >> 16) & 0xffff;

				setup_viewport(xw, xh, vpw-xw, vph-xh, vpw, vph);
			}

			/*
			if (address == 0x10000100)
			{
			    int in1 = (m_reg_10000100 >> 16) & 0x1ff;
			    if (in1 & 0x100) in1 |= 0xfffffe00;
			    int in2 = m_reg_10000100 & 0x1ff;
			    if (in2 & 0x100) in2 |= 0xfffffe00;
			    int in3 = m_reg_10000101 & 0x7f;

			    float n1 = (float)(in1) / 127.0f;
			    float n2 = (float)(in2) / 127.0f;
			    float n3 = (float)(in3) / 127.0f;

			    printf("UNK: %f, %f, %f\n", n1, n2, n3);
			}
			*/
		}
		else if (cmd == 0xffff0080)
		{
		}
		else if (cmd == 0xffff00ff)
		{
			end = 1;
			w[0] = src[index^1];
			index++;
			w[1] = src[index^1];
			index++;
		}
		else
		{
#if LOG_DISPLAY_LIST
			printf("%08X: %08X (unknown)\n", index, cmd);
#endif
			end = 1;
		}
	}
	while (!end);

	wait("render_polygons");
}

UINT32 taitotz_state::screen_update_taitotz(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x000000, cliprect);
	m_renderer->set_fb(&bitmap);
	m_renderer->render_displaylist(cliprect);


	UINT16 *screen_src = (UINT16*)&m_screen_ram[m_scr_base];

	for (int j=0; j < 384; j++)
	{
		UINT32 *fb = &bitmap.pix32(j);
		for (int i=0; i < 512; i++)
		{
			UINT16 p = *screen_src++;
			if (p & 0x8000)     // draw 2D framebuffer if there's an opaque pixel
			{
				int r = ((p >> 10) & 0x1f) << (3+16);
				int g = ((p >> 5) & 0x1f) << (3+8);
				int b = (p & 0x1f) << 3;
				fb[i^1] = 0xff000000 | r | g | b;
			}
		}
	}

	return 0;
}

void taitotz_state::draw_tile(UINT32 pos, UINT32 tile)
{
	int tileu = (tile & 0x1f) * 16;
	int tilev = ((tile >> 5)) * 16;

	int tilex = (pos & 0x1f) * 16;
	int tiley = ((pos >> 5) & 0x1f) * 16;

	UINT16 *src_tile = (UINT16*)&m_screen_ram[0x180000];
	UINT16 *dst = (UINT16*)&m_screen_ram[m_scr_base];

	int v = tilev;

	for (int j=tiley; j < (tiley+16); j++)
	{
		int u = tileu;
		for (int i=tilex; i < (tilex+16); i++)
		{
			UINT16 p = src_tile[((v*512) + u)];
			dst[(j*512) + i] = p;
			u++;
		}
		v++;
	}
}

/*
    Video chip memory map

    0x800000...0x9fffff         Screen RAM
        0x980000...7fff         Char RAM
        0x9c0000...fffff        Tile RAM?

    0x1000000...0x17fffff       Texture RAM

    0x1800000...0x187ffff       Frame RAM


    landhigh puts fullscreen images into 0x9c0000
    batlgr2 into 0x9e0000
*/

UINT32 taitotz_state::video_mem_r(UINT32 address)
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
		printf("video_mem_r: %08X\n", address);
		return 0;
	}
}

void taitotz_state::video_mem_w(UINT32 address, UINT32 data)
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
		printf("video_mem_w: %08X, %08X\n", address, data);
	}
}

UINT32 taitotz_state::video_reg_r(UINT32 reg)
{
	switch ((reg >> 28) & 0xf)
	{
		case 0x1:
		{
			if (reg == 0x10000105)      // Gets spammed a lot. Probably a status register.
			{
				m_reg105 ^= 0xffffffff;
				return m_reg105;
			}

			logerror("video_reg_r: reg: %08X\n", reg);
			return 0xffffffff;
		}
		case 0x2:
		{
			int subreg = reg & 0xfffffff;

			if (reg != 0x20000008)
				logerror("video_reg_r: reg: %08X\n", reg);

			if (subreg < 0x10)
			{
				return m_video_unk_reg[subreg];
			}
			break;
		}
		case 0xb:
		{
			return video_mem_r(reg & 0xfffffff);
		}
		default:
		{
			logerror("video_reg_r: reg: %08X\n", reg);
			break;
		}
	}

	return 0;
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

void taitotz_state::video_reg_w(UINT32 reg, UINT32 data)
{
	switch ((reg >> 28) & 0xf)
	{
	case 0x1:       // Register write?
		{
			if (reg == 0x10000105)
			{
				// This register gets spammed a lot. VB IRQ ack or buffer swap?
			}
			else
			{
				logerror("video_reg_w: reg: %08X data: %08X\n", reg, data);
			}
			break;
		}
	case 0x2:       // ???
		{
			int subreg = reg & 0xfffffff;
			if (subreg < 0x10)
			{
				m_video_unk_reg[subreg] = data;
			}
			logerror("video_reg_w: reg: %08X data: %08X\n", reg, data);
			break;
		}
	case 0x3:       // Draw 16x16 tile
		{
			UINT32 pos = (data >> 12) & 0xfff;
			UINT32 tile = data & 0xfff;
			draw_tile(pos, tile);
			break;
		}
	case 0xb:       // RAM write?
		{
			video_mem_w(m_video_ram_ptr, data);
			m_video_ram_ptr++;
			break;
		}
	default:
		{
			logerror("video_reg_w: reg: %08X data: %08X\n", reg, data);
			break;
		}
	}
}

READ64_MEMBER(taitotz_state::video_chip_r)
{
	UINT64 r = 0;
	UINT32 reg = offset * 8;

	if (ACCESSING_BITS_0_31)
	{
		reg += 4;

		switch (reg)
		{
			case 0x14:
				{
					r |= 0xffffffff;
					break;
				}

			default:
				printf("video_chip_r: %04X\n", reg);
				break;
		}
	}
	if (ACCESSING_BITS_32_63)
	{
		switch (reg)
		{
			case 0x0:
				{
					r |= (UINT64)(video_reg_r(m_video_reg)) << 32;
					break;
				}

			case 0x10:
				{
					r |= (UINT64)(0xff) << 32;      // busy flags? landhigh expects this
					break;
				}

			default:
				{
					printf("video_chip_r: %04X\n", reg);
					break;
				}
		}
	}

	return r;
}

WRITE64_MEMBER(taitotz_state::video_chip_w)
{
	UINT32 reg = offset * 8;
	UINT32 regdata;

	if (ACCESSING_BITS_0_31)
	{
		reg += 4;
		regdata = (UINT32)(data);
		switch (reg)
		{
			default:
			{
				logerror("video_chip_w: port 0x%02X: %08X\n", reg, regdata);
				break;
			}
		}
	}
	if (ACCESSING_BITS_32_63)
	{
		regdata = (UINT32)(data >> 32);
		switch (reg)
		{
			case 0:
			{
				video_reg_w(m_video_reg, regdata);
				break;
			}
			case 0x8:
			{
				m_video_reg = regdata;
				m_video_fifo_ptr = 0;

				switch ((m_video_reg >> 28) & 0xf)
				{
					case 0xb:
					{
						m_video_ram_ptr = m_video_reg & 0xfffffff;
						//logerror("video_chip_ram sel %08X at %08X\n", m_video_reg & 0x0fffffff, space.device().safe_pc());
						break;
					}
					case 0x0:
					{
						break;
					}
					case 0x1:
					{
						break;
					}
					case 0x2:
					{
						break;
					}
					case 0x3:
					{
						// video_chip_w: port 0x08: 30000001        gets spammed a lot
						break;
					}
					default:
					{
						logerror("video_chip_w: port 0x%02X: %08X\n", reg, regdata);
						break;
					}
				}
				break;
			}
			default:
			{
				logerror("video_chip_w: port 0x%02X: %08X\n", reg, regdata);
				break;
			}
		}
	}
}

READ64_MEMBER(taitotz_state::video_fifo_r)
{
	UINT64 r = 0;
	if (ACCESSING_BITS_32_63)
	{
		r |= (UINT64)(video_mem_r(m_video_ram_ptr)) << 32;
		m_video_ram_ptr++;
	}
	if (ACCESSING_BITS_0_31)
	{
		r |= (UINT64)(video_mem_r(m_video_ram_ptr));
		m_video_ram_ptr++;
	}

	return r;
}

WRITE64_MEMBER(taitotz_state::video_fifo_w)
{
	int command = (m_video_reg >> 28) & 0xf;
	if (command == 0xb)
	{
		// FIFO mem access

		if (ACCESSING_BITS_32_63)
		{
			if (m_video_fifo_ptr >= 8)
			{
				video_mem_w(m_video_ram_ptr, (UINT32)(data >> 32));
				m_video_ram_ptr++;
			}
			m_video_fifo_ptr++;
		}
		if (ACCESSING_BITS_0_31)
		{
			if (m_video_fifo_ptr >= 8)
			{
				video_mem_w(m_video_ram_ptr, (UINT32)(data));
				m_video_ram_ptr++;
			}
			m_video_fifo_ptr++;
		}
	}
	else if (command == 0x1)
	{
		// FIFO command/packet write

		if (ACCESSING_BITS_32_63)
		{
			if (m_video_fifo_ptr >= 8)
			{
				logerror("FIFO packet w: %08X at %08X\n", (UINT32)(data >> 32), space.device().safe_pc());
			}
			m_video_fifo_ptr++;
		}
		if (ACCESSING_BITS_0_31)
		{
			if (m_video_fifo_ptr >= 8)
			{
				logerror("FIFO packet w: %08X at %08X\n", (UINT32)(data), space.device().safe_pc());
			}
			m_video_fifo_ptr++;
		}
	}
	else
	{
		if (ACCESSING_BITS_32_63)
		{
			if (m_video_fifo_ptr >= 8)
			{
				printf("FIFO write with cmd %02X: %08X\n", command, (UINT32)(data >> 32));
			}
			m_video_fifo_ptr++;
		}
		if (ACCESSING_BITS_0_31)
		{
			if (m_video_fifo_ptr >= 8)
			{
				printf("FIFO write with cmd %02X: %08X\n", command, (UINT32)(data));
			}
			m_video_fifo_ptr++;
		}
	}

	//COMBINE_DATA(m_video_fifo_mem + offset);
}

READ64_MEMBER(taitotz_state::ieee1394_r)
{
	if (offset == 4)
	{
		return U64(0xffffffffffffffff);
	}

	//logerror("ieee1394_r: %08X, %08X%08X\n", offset, (UINT32)(mem_mask >> 32), (UINT32)(mem_mask));
	return 0;
}

WRITE64_MEMBER(taitotz_state::ieee1394_w)
{
	//logerror("ieee1394_w: %08X, %08X%08X, %08X%08X\n", offset, (UINT32)(data >> 32), (UINT32)(data), (UINT32)(mem_mask >> 32), (UINT32)(mem_mask));
	if (ACCESSING_BITS_32_63)
	{
	}
	if (ACCESSING_BITS_0_31)
	{
	}
}

READ64_MEMBER(taitotz_state::ppc_common_r)
{
	UINT64 res = 0;

	if (ACCESSING_BITS_0_15)
	{
		res |= m_io_share_ram[(offset * 2) + 1];
	}
	if (ACCESSING_BITS_32_47)
	{
		res |= (UINT64)(m_io_share_ram[(offset * 2) + 0]) << 32;
	}

	return res;
}

WRITE64_MEMBER(taitotz_state::ppc_common_w)
{
	if (ACCESSING_BITS_0_15)
	{
		m_io_share_ram[(offset * 2) + 1] = (UINT16)(data);
	}
	if (ACCESSING_BITS_32_47)
	{
		m_io_share_ram[(offset * 2) + 0] = (UINT16)(data >> 32);
	}

	if (offset == 0x7ff)
	{
#if LOG_PPC_TO_TLCS_COMMANDS
		if (m_io_share_ram[0xfff] != 0x0000 &&
			m_io_share_ram[0xfff] != 0x1010 &&
			m_io_share_ram[0xfff] != 0x1020 &&
			m_io_share_ram[0xfff] != 0x6000 &&
			m_io_share_ram[0xfff] != 0x6010 &&
			m_io_share_ram[0xfff] != 0x7004 &&
			m_io_share_ram[0xfff] != 0x4001 &&
			m_io_share_ram[0xfff] != 0x4002 &&
			m_io_share_ram[0xfff] != 0x4003)
		{
			printf("PPC -> TLCS cmd %04X\n", m_io_share_ram[0xfff]);
		}

		if (m_io_share_ram[0xfff] == 0x4000)
		{
			printf("   %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X\n",
					m_io_share_ram[0x1c34/2],
					m_io_share_ram[0x1c36/2],
					m_io_share_ram[0x1c38/2],
					m_io_share_ram[0x1c3a/2],
					m_io_share_ram[0x1c2c/2],
					m_io_share_ram[0x1c2e/2],
					m_io_share_ram[0x1c30/2],
					m_io_share_ram[0x1c32/2],
					m_io_share_ram[0x1c24/2],
					m_io_share_ram[0x1c26/2]
					);
			printf("   %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X\n",
					m_io_share_ram[0x1c28/2],
					m_io_share_ram[0x1c2a/2],
					m_io_share_ram[0x1c1c/2],
					m_io_share_ram[0x1c1e/2],
					m_io_share_ram[0x1c20/2],
					m_io_share_ram[0x1c22/2],
					m_io_share_ram[0x1c14/2],
					m_io_share_ram[0x1c16/2],
					m_io_share_ram[0x1c18/2],
					m_io_share_ram[0x1c1a/2]
					);
		}

		/*
		if (m_io_share_ram[0xfff] == 0x1010)
		{
		    printf("PPC -> TLCS cmd 1010:   %04X %04X %04X %04X\n", m_io_share_ram[0x1a02/2], m_io_share_ram[0x1a04/2], m_io_share_ram[0x1a06/2], m_io_share_ram[0x1a08/2]);
		}
		*/

#endif


		// hacky way to handle some commands for now
		if (m_io_share_ram[0xfff] == 0x4001)
		{
			m_io_share_ram[0xfff] = 0x0000;
			m_io_share_ram[0xe00] = 0xffff;
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		}
		else if (m_io_share_ram[0xfff] == 0x4004 || m_io_share_ram[0xfff] == 0x4000)
		{
			m_io_share_ram[0xfff] = 0x0000;
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		}
		else if (m_io_share_ram[0xfff] == 0x7004)
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
			if (m_io_share_ram[0xfff] == 0x1010 || m_io_share_ram[0xfff] == 0x1020 ||
				m_io_share_ram[0xfff] == 0x6000 || m_io_share_ram[0xfff] == 0x6010)
			{
				//m_maincpu->spin_until_trigger(PPC_TLCS_COMM_TRIGGER);
				m_maincpu->spin_until_interrupt();
			}

			// pwrshovl sometimes writes commands during command handling... make sure that doesn't happen
			if (m_io_share_ram[0xfff] == 0x0000)
			{
				m_maincpu->spin_until_time(attotime::from_usec(100));
			}

			machine().scheduler().trigger(TLCS_PPC_COMM_TRIGGER);
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

static ADDRESS_MAP_START( ppc603e_mem, AS_PROGRAM, 64, taitotz_state)
	AM_RANGE(0x00000000, 0x0000001f) AM_READWRITE(video_chip_r, video_chip_w)
	AM_RANGE(0x10000000, 0x1000001f) AM_READWRITE(video_fifo_r, video_fifo_w)
	AM_RANGE(0x40000000, 0x40ffffff) AM_RAM AM_SHARE("work_ram")                // Work RAM
	AM_RANGE(0xa4000000, 0xa40000ff) AM_READWRITE(ieee1394_r, ieee1394_w)       // IEEE1394 network
	AM_RANGE(0xa8000000, 0xa8003fff) AM_READWRITE(ppc_common_r, ppc_common_w)   // Common RAM (with TLCS-900)
	AM_RANGE(0xac000000, 0xac0fffff) AM_ROM AM_REGION("user1", 0)               // Apparently this should be flash ROM read/write access
	AM_RANGE(0xfff00000, 0xffffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END



READ8_MEMBER(taitotz_state::tlcs_common_r)
{
	if (offset & 1)
	{
		return (UINT8)(m_io_share_ram[offset / 2] >> 8);
	}
	else
	{
		return (UINT8)(m_io_share_ram[offset / 2]);
	}
}

WRITE8_MEMBER(taitotz_state::tlcs_common_w)
{
	if (offset & 1)
	{
		m_io_share_ram[offset / 2] &= 0x00ff;
		m_io_share_ram[offset / 2] |= (UINT16)(data) << 8;
	}
	else
	{
		m_io_share_ram[offset / 2] &= 0xff00;
		m_io_share_ram[offset / 2] |= data;
	}

	if (offset == 0x1ffd)
	{
#if LOG_TLCS_TO_PPC_COMMANDS
		if (m_io_share_ram[0xffe] != 0xd000 &&
			m_io_share_ram[0xffe] != 0x1011 &&
			m_io_share_ram[0xffe] != 0x1012 &&
			m_io_share_ram[0xffe] != 0x1022)
		{
			printf("TLCS -> PPC cmd %04X\n", m_io_share_ram[0xffe]);
			//printf("0x40080104 = %08X\n", (UINT32)(m_work_ram[0x80104/8]));
		}
#endif

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

// RTC could be Epson RTC-64613, same as taitopjc.c
READ8_MEMBER(taitotz_state::tlcs_rtc_r)
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
			printf("tlcs_rtc_r: %02X\n", offset);
			break;
	}

	return 0;
}

WRITE8_MEMBER(taitotz_state::tlcs_rtc_w)
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
			//printf("tlcs_rtc_w: %02X, %02X\n", offset, data);
			break;
	}
}

READ16_MEMBER(taitotz_state::tlcs_ide0_r)
{
	UINT16 d = m_ata->read_cs0(space, offset, mem_mask);
	if (offset == 7)
		d &= ~0x2;      // Type Zero doesn't like the index bit. It's defined as vendor-specific, so it probably shouldn't be up...
						// The status check explicitly checks for 0x50 (drive ready, seek complete).
	return d;
}

READ16_MEMBER(taitotz_state::tlcs_ide1_r)
{
	UINT16 d = m_ata->read_cs1(space, offset, mem_mask);
	if (offset == 6)
		d &= ~0x2;      // Type Zero doesn't like the index bit. It's defined as vendor-specific, so it probably shouldn't be up...
						// The status check explicitly checks for 0x50 (drive ready, seek complete).
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

static ADDRESS_MAP_START( tlcs900h_mem, AS_PROGRAM, 16, taitotz_state)
	AM_RANGE(0x010000, 0x02ffff) AM_RAM                                                     // Work RAM
	AM_RANGE(0x040000, 0x041fff) AM_RAM AM_SHARE("nvram")                                   // Backup RAM
	AM_RANGE(0x044000, 0x04400f) AM_READWRITE8(tlcs_rtc_r, tlcs_rtc_w, 0xffff)
	AM_RANGE(0x060000, 0x061fff) AM_READWRITE8(tlcs_common_r, tlcs_common_w, 0xffff)
	AM_RANGE(0x064000, 0x064fff) AM_RAM AM_SHARE("mbox_ram")                                // MBox
	AM_RANGE(0x068000, 0x06800f) AM_DEVWRITE("ata", ata_interface_device, write_cs0) AM_READ(tlcs_ide0_r)
	AM_RANGE(0x06c000, 0x06c00f) AM_DEVWRITE("ata", ata_interface_device, write_cs1) AM_READ(tlcs_ide1_r)
	AM_RANGE(0xfc0000, 0xffffff) AM_ROM AM_REGION("io_cpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( landhigh_tlcs900h_mem, AS_PROGRAM, 16, taitotz_state)
	AM_RANGE(0x200000, 0x21ffff) AM_RAM                                                     // Work RAM
	AM_RANGE(0x400000, 0x401fff) AM_RAM AM_SHARE("nvram")                                   // Backup RAM
	AM_RANGE(0x404000, 0x40400f) AM_READWRITE8(tlcs_rtc_r, tlcs_rtc_w, 0xffff)
	AM_RANGE(0x900000, 0x901fff) AM_READWRITE8(tlcs_common_r, tlcs_common_w, 0xffff)
	AM_RANGE(0x910000, 0x910fff) AM_RAM AM_SHARE("mbox_ram")                                // MBox
	AM_RANGE(0x908000, 0x90800f) AM_DEVWRITE("ata", ata_interface_device, write_cs0) AM_READ(tlcs_ide0_r)
	AM_RANGE(0x918000, 0x91800f) AM_DEVWRITE("ata", ata_interface_device, write_cs1) AM_READ(tlcs_ide1_r)
	AM_RANGE(0xfc0000, 0xffffff) AM_ROM AM_REGION("io_cpu", 0)
ADDRESS_MAP_END



static INPUT_PORTS_START( taitotz )
	PORT_START("INPUTS1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x00000002, IP_ACTIVE_LOW) /* Test Button */
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
INPUT_PORTS_END

static INPUT_PORTS_START( landhigh )
	PORT_START("INPUTS1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x00000002, IP_ACTIVE_LOW) /* Test Button */
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
INPUT_PORTS_END

static INPUT_PORTS_START( batlgr2 )
	PORT_START("INPUTS1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x00000002, IP_ACTIVE_LOW) /* Test Button */
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
INPUT_PORTS_END

static INPUT_PORTS_START( pwrshovl )
	PORT_START("INPUTS1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x00000002, IP_ACTIVE_LOW) /* Test Button */
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
	PORT_SERVICE_NO_TOGGLE( 0x00000002, IP_ACTIVE_LOW) /* Test Button */
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
INPUT_PORTS_END

void taitotz_state::machine_reset()
{
	if (m_hdd_serial_number != nullptr)
	{
		ide_hdd_device *hdd = m_ata->subdevice<ata_slot_device>("0")->subdevice<ide_hdd_device>("hdd");
		UINT16 *identify_device = hdd->identify_device_buffer();

		for (int i=0; i < 10; i++)
		{
			identify_device[10+i] = (m_hdd_serial_number[i*2] << 8) | m_hdd_serial_number[i*2+1];
		}
	}
}

void taitotz_state::machine_start()
{
	/* set conservative DRC options */
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	/* configure fast RAM regions for DRC */
	m_maincpu->ppcdrc_add_fastram(0x40000000, 0x40ffffff, FALSE, m_work_ram);
}


INTERRUPT_GEN_MEMBER(taitotz_state::taitotz_vbi)
{
	m_iocpu->set_input_line(TLCS900_INT3, ASSERT_LINE);
}

WRITE_LINE_MEMBER(taitotz_state::ide_interrupt)
{
	m_iocpu->set_input_line(TLCS900_INT2, state);
}

static MACHINE_CONFIG_START( taitotz, taitotz_state )
	/* IBM EMPPC603eBG-100 */
	MCFG_CPU_ADD("maincpu", PPC603E, 100000000)
	MCFG_PPC_BUS_FREQUENCY(XTAL_66_6667MHz)    /* Multiplier 1.5, Bus = 66MHz, Core = 100MHz */
	MCFG_CPU_PROGRAM_MAP(ppc603e_mem)

	/* TMP95C063F I/O CPU */
	MCFG_CPU_ADD("iocpu", TMP95C063, 25000000)
	MCFG_TMP95C063_PORT9_READ(IOPORT("INPUTS1"))
	MCFG_TMP95C063_PORTB_READ(IOPORT("INPUTS2"))
	MCFG_TMP95C063_PORTD_READ(IOPORT("INPUTS3"))
	MCFG_TMP95C063_PORTE_READ(IOPORT("INPUTS4"))

	MCFG_CPU_PROGRAM_MAP(tlcs900h_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", taitotz_state,  taitotz_vbi)

	/* MN1020819DA sound CPU */

	MCFG_QUANTUM_TIME(attotime::from_hz(120))

	MCFG_ATA_INTERFACE_ADD("ata", ata_devices, "hdd", nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(taitotz_state, ide_interrupt))

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 384)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 383)
	MCFG_SCREEN_UPDATE_DRIVER(taitotz_state, screen_update_taitotz)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( landhigh, taitotz )
	MCFG_CPU_MODIFY("iocpu")
	MCFG_CPU_PROGRAM_MAP(landhigh_tlcs900h_mem)
MACHINE_CONFIG_END


// Init for BIOS v1.52
void taitotz_state::init_taitotz_152()
{
	UINT32 *rom = (UINT32*)memregion("user1")->base();
	rom[(0x2c87c^4)/4] = 0x38600000;    // skip sound load timeout...
//  rom[(0x2c620^4)/4] = 0x48000014;    // ID check skip (not needed with correct serial number)
}

// Init for BIOS 1.11a
void taitotz_state::init_taitotz_111a()
{
	UINT32 *rom = (UINT32*)memregion("user1")->base();
	rom[(0x2b748^4)/4] = 0x480000b8;    // skip sound load timeout
}

static const char LANDHIGH_HDD_SERIAL[] =           // "824915746386        "
	{ 0x38, 0x32, 0x34, 0x39, 0x31, 0x35, 0x37, 0x34, 0x36, 0x33, 0x38, 0x36, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };

static const char BATLGR2_HDD_SERIAL[] =            // "            05412842"
	{ 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0x35, 0x34, 0x31, 0x32, 0x38, 0x34, 0x32 };

static const char BATLGR2A_HDD_SERIAL[] =           // "            05411645"
	{ 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0x35, 0x34, 0x31, 0x31, 0x36, 0x34, 0x35 };

static const char RAIZPIN_HDD_SERIAL[] =            // "691934013492        "
	{ 0x36, 0x39, 0x31, 0x39, 0x33, 0x34, 0x30, 0x31, 0x33, 0x34, 0x39, 0x32, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };

static const char STYPHP_HDD_SERIAL[] =             // "            05872160"
	{ 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0x35, 0x38, 0x37, 0x32, 0x31, 0x36, 0x30 };

DRIVER_INIT_MEMBER(taitotz_state,landhigh)
{
	init_taitotz_152();

	m_hdd_serial_number = LANDHIGH_HDD_SERIAL;

	m_scr_base = 0x1c0000;

	m_displist_addr = 0x5200b8;
}

DRIVER_INIT_MEMBER(taitotz_state,batlgear)
{
	init_taitotz_111a();

	// unknown, not used by BIOS 1.11a
	m_hdd_serial_number = nullptr;

	m_scr_base = 0x1c0000;

	m_displist_addr = 0x420038;
}

DRIVER_INIT_MEMBER(taitotz_state,batlgr2)
{
	init_taitotz_152();

	m_hdd_serial_number = BATLGR2_HDD_SERIAL;

	m_scr_base = 0x1e0000;

	m_displist_addr = 0x3608e4;
}

DRIVER_INIT_MEMBER(taitotz_state,batlgr2a)
{
	init_taitotz_152();

	m_hdd_serial_number = BATLGR2A_HDD_SERIAL;

	m_scr_base = 0x1e0000;

	m_displist_addr = 0x3608e4;
}

DRIVER_INIT_MEMBER(taitotz_state,pwrshovl)
{
	init_taitotz_111a();

	// unknown, not used by BIOS 1.11a
	m_hdd_serial_number = nullptr;

	m_scr_base = 0x1c0000;

	m_displist_addr = 0x4a989c;
}

DRIVER_INIT_MEMBER(taitotz_state,raizpin)
{
	init_taitotz_152();

	m_hdd_serial_number = RAIZPIN_HDD_SERIAL;

	m_scr_base = 0x1c0000;

	m_displist_addr = 0x33480c;
}

DRIVER_INIT_MEMBER(taitotz_state,styphp)
{
	init_taitotz_152();

	m_hdd_serial_number = STYPHP_HDD_SERIAL;

	m_scr_base = 0x1e0000;

	m_displist_addr = 0x490b70;
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

	ROM_REGION( 0x40000, "io_cpu", ROMREGION_ERASE00 )
	ROM_REGION( 0x10000, "sound_cpu", ROMREGION_ERASE00 ) /* Internal ROM :( */
	ROM_REGION( 0x500, "plds", ROMREGION_ERASE00 )
	DISK_REGION( "ata:0:hdd:image" )
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
Buffer Size 	128K

*/

ROM_START( landhigh )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V152

	ROM_REGION( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e82-03.ic14", 0x000000, 0x020000, CRC(0de65b4d) SHA1(932316f7435259b723a29843d58b2e3dca92e7b7) )
	ROM_LOAD16_BYTE( "e82-04.ic15", 0x000001, 0x020000, CRC(b3cb0f3d) SHA1(80414f50a1593c6b849d9f37e94a32168699a5c1) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	ROM_REGION( 0x500, "plds", 0 )
	ROM_LOAD( "e82-01.ic44", 0x000, 0x117, CRC(49eea30f) SHA1(ef97c792358f05b9214a2f58ee1e97e8208806c4) )
	ROM_LOAD( "e82-02.ic45", 0x117, 0x2dd, CRC(f581cff5) SHA1(468e0e6a3828f2dcda35c6d523154510f9c99db7) )
	ROM_LOAD( "e68-06.ic24", 0x3f4, 0x100, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "landhigh", 0, SHA1(7cea4ea5c3899e6ac774a4eb12821f44541d9c9c) )
ROM_END

ROM_START( batlgear )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V111A

	ROM_REGION( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e68-07.ic14",  0x000000, 0x020000, CRC(554c6fd7) SHA1(9f203dead81c7ccf73d7fd462cab147cd17f890f) )
	ROM_LOAD16_BYTE( "e68-08.ic15",  0x000001, 0x020000, CRC(f1932380) SHA1(64d12e858af15a9ba8254917da13863ac7c9c050) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "batlgear", 0, SHA1(eab283839ad3e0a3e6be11f6482570db334eacca) )
ROM_END

ROM_START( batlgr2 )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V152

	ROM_REGION( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e87-03.ic14",  0x000000, 0x020000, CRC(49ae7cd0) SHA1(15f07a6bb2044a85a2139481f1dc95a44520c929) )
	ROM_LOAD16_BYTE( "e87-04.ic15",  0x000001, 0x020000, CRC(59f8f75f) SHA1(f5595751b10c0033f460114c43f5e2c192fe61f1) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "bg2_204j", 0, SHA1(7ac100fba39ae0b93980c0af2f0212a731106912) )
ROM_END

ROM_START( batlgr2a )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V152

	ROM_REGION( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e87-03.ic14",  0x000000, 0x020000, CRC(49ae7cd0) SHA1(15f07a6bb2044a85a2139481f1dc95a44520c929) )
	ROM_LOAD16_BYTE( "e87-04.ic15",  0x000001, 0x020000, CRC(59f8f75f) SHA1(f5595751b10c0033f460114c43f5e2c192fe61f1) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "bg2_201j", 0, SHA1(542d12682bd0f95143368578461c6a4fcc492fcc) )
ROM_END

ROM_START( pwrshovl )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V111A

	ROM_REGION( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e74-04++.ic14", 0x000000, 0x020000, CRC(ef21a261) SHA1(7398826dbf48014b9c7e9454f978f3e419ebc64b) ) // actually labeled E74-04**
	ROM_LOAD16_BYTE( "e74-05++.ic15", 0x000001, 0x020000, CRC(2466217d) SHA1(dc814da3a1679cff001f179d3c1641af985a6490) ) // actually labeled E74-05**

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	ROM_REGION( 0x20000, "io_cpu2", 0 ) // another TMP95C063F, not hooked up yet
	ROM_LOAD( "e74-06.ic2", 0x000000, 0x020000, CRC(cd4a99d3) SHA1(ea280e05a68308c1c5f1fc0ee8a25b33923df635) ) // located on the I/O PCB

	ROM_REGION( 0x20000, "oki1", 0 )
	ROM_LOAD( "e74-07.ic6", 0x000000, 0x020000, CRC(ca5baccc) SHA1(4594b7a6232b912d698fff053f7e3f51d8e1bfb6) ) // located on the I/O PCB

	ROM_REGION( 0x20000, "oki2", 0 )
	ROM_LOAD( "e74-08.ic8", 0x000000, 0x020000, CRC(ca5baccc) SHA1(4594b7a6232b912d698fff053f7e3f51d8e1bfb6) ) // located on the I/O PCB

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "pwrshovl", 0, SHA1(360f63b39f645851c513b4644fb40601b9ba1412) )
ROM_END

ROM_START( pwrshovla )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V111A

	ROM_REGION( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e74-04++.ic14", 0x000000, 0x020000, CRC(ef21a261) SHA1(7398826dbf48014b9c7e9454f978f3e419ebc64b) ) // actually labeled E74-04**
	ROM_LOAD16_BYTE( "e74-05++.ic15", 0x000001, 0x020000, CRC(2466217d) SHA1(dc814da3a1679cff001f179d3c1641af985a6490) ) // actually labeled E74-05**

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	ROM_REGION( 0x20000, "io_cpu2", 0 ) // another TMP95C063F, not hooked up yet
	ROM_LOAD( "e74-06.ic2", 0x000000, 0x020000, CRC(cd4a99d3) SHA1(ea280e05a68308c1c5f1fc0ee8a25b33923df635) ) // located on the I/O PCB

	ROM_REGION( 0x20000, "oki1", 0 )
	ROM_LOAD( "e74-07.ic6", 0x000000, 0x020000, CRC(ca5baccc) SHA1(4594b7a6232b912d698fff053f7e3f51d8e1bfb6) ) // located on the I/O PCB

	ROM_REGION( 0x20000, "oki2", 0 )
	ROM_LOAD( "e74-08.ic8", 0x000000, 0x020000, CRC(ca5baccc) SHA1(4594b7a6232b912d698fff053f7e3f51d8e1bfb6) ) // located on the I/O PCB

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "POWER SHOVEL VER.2.07J", 0, SHA1(05410d4b4972262ef93400b02f21dd17d10b1c5e) )
ROM_END

ROM_START( raizpin )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V152

	ROM_REGION( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "f14-01.ic14",  0x000000, 0x020000, CRC(f86a184d) SHA1(46abd11430c08d4f384fb79a5a3a39e54f83b8d8) )
	ROM_LOAD16_BYTE( "f14-02.ic15",  0x000001, 0x020000, CRC(bd2d0dee) SHA1(652f810702598184551de9fd69436862d48c1608) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "raizpin", 0, SHA1(883ebcda03026df31da1cdb95af521e100c171ed) )
ROM_END

ROM_START( raizpinj )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V152

	ROM_REGION( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "f14-01.ic14",  0x000000, 0x020000, CRC(f86a184d) SHA1(46abd11430c08d4f384fb79a5a3a39e54f83b8d8) )
	ROM_LOAD16_BYTE( "f14-02.ic15",  0x000001, 0x020000, CRC(bd2d0dee) SHA1(652f810702598184551de9fd69436862d48c1608) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "RAIZIN PING PONG VER 2.01J", 0, SHA1(eddc803c2507d19f0a3e3cc217bb22a565c04f3e) )
ROM_END

ROM_START( styphp )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	TAITOTZ_BIOS_V152

	ROM_REGION( 0x40000, "io_cpu", 0 )
	ROM_LOAD16_BYTE( "e98-01.ic14", 0x000000, 0x020000, CRC(479b37ad) SHA1(a0e59d990665244a7919a104dc4b6869ccf90be1) )
	ROM_LOAD16_BYTE( "e98-02.ic15", 0x000001, 0x020000, CRC(d8da590f) SHA1(b33a67d81e388d7863adaf03041911c7f84d193b) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic7", 0x000000, 0x010000, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "styphp", 0, SHA1(c232d3460e37523346132544b8e23a5f9b447150) )
ROM_END

GAME( 1999, taitotz,  0, taitotz, taitotz, driver_device, 0, ROT0, "Taito", "Type Zero BIOS", MACHINE_NO_SOUND|MACHINE_NOT_WORKING|MACHINE_IS_BIOS_ROOT)
GAME( 1999, landhigh, taitotz, landhigh, landhigh, taitotz_state, landhigh, ROT0, "Taito", "Landing High Japan", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1999, batlgear, taitotz, taitotz,  batlgr2, taitotz_state,  batlgear, ROT0, "Taito", "Battle Gear", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1999, pwrshovl, taitotz, taitotz,  pwrshovl, taitotz_state, pwrshovl, ROT0, "Taito", "Power Shovel ni Norou!! - Power Shovel Simulator (v2.07J)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 1999/8/5 19:13:35
GAME( 1999, pwrshovla,pwrshovl,taitotz,  pwrshovl, taitotz_state, pwrshovl, ROT0, "Taito", "Power Shovel ni Norou!! - Power Shovel Simulator (v2.07J, alt)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // seem to be some differences in drive content, but identifies as the same revision, is it just user data changes??
GAME( 2000, batlgr2,  taitotz, taitotz,  batlgr2, taitotz_state,  batlgr2,  ROT0, "Taito", "Battle Gear 2 (v2.04J)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2000, batlgr2a, batlgr2, taitotz,  batlgr2, taitotz_state,  batlgr2a, ROT0, "Taito", "Battle Gear 2 (v2.01J)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2000, styphp,   taitotz, taitotz,  styphp,  taitotz_state,  styphp,   ROT0, "Taito", "Stunt Typhoon Plus", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2002, raizpin,  taitotz, taitotz,  taitotz, taitotz_state,  raizpin,  ROT0, "Taito", "Raizin Ping Pong (V2.01O)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2002, raizpinj, raizpin, taitotz,  taitotz, taitotz_state,  raizpin,  ROT0, "Taito", "Raizin Ping Pong (V2.01J)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // needs proper serial code
