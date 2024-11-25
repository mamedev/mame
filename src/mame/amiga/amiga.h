// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Mariusz Wojcieszek
/***************************************************************************
Amiga Computer / Arcadia Game System

Driver by:

Ernesto Corvi & Mariusz Wojcieszek

***************************************************************************/

#ifndef MAME_AMIGA_AMIGA_H
#define MAME_AMIGA_AMIGA_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68020.h"
#include "cpu/m68000/m68030.h"
#include "cpu/m68000/m68040.h"
#include "machine/bankdev.h"
#include "bus/rs232/rs232.h"
#include "bus/centronics/ctronics.h"
#include "machine/mos6526.h"
#include "machine/amigafdc.h"
#include "machine/amiga_copper.h"
#include "machine/msm6242.h"
#include "machine/akiko.h"
#include "machine/i2cmem.h"
#include "machine/8364_paula.h"
#include "amigaaga.h"
#include "emupal.h"
#include "screen.h"


/*
    A = Angus
    D = Denise
    P = Paula

    R = read-only
    ER = early read
    W = write-only
    S = strobe
*/
#define REG_BLTDDAT     (0x000/2)   /* ER A      Blitter destination early read (dummy address) */
#define REG_DMACONR     (0x002/2)   /* R  A   P  DMA control (and blitter status) read */
#define REG_VPOSR       (0x004/2)   /* R  A      Read vert most signif. bit (and frame flop) */
#define REG_VHPOSR      (0x006/2)   /* R  A      Read vert and horiz. position of beam */
#define REG_DSKDATR     (0x008/2)   /* ER     P  Disk data early read (dummy address) */
#define REG_JOY0DAT     (0x00A/2)   /* R    D    Joystick-mouse 0 data (vert,horiz) */
#define REG_JOY1DAT     (0x00C/2)   /* R    D    Joystick-mouse 1 data (vert,horiz) */
#define REG_CLXDAT      (0x00E/2)   /* R    D    Collision data register (read and clear) */
#define REG_ADKCONR     (0x010/2)   /* R      P  Audio, disk control register read */
#define REG_POT0DAT     (0x012/2)   /* R      P  Pot counter pair 0 data (vert,horiz) */
#define REG_POT1DAT     (0x014/2)   /* R      P  Pot counter pair 1 data (vert,horiz) */
#define REG_POTGOR      (0x016/2)   /* R      P  Pot port data read (formerly POTINP) */
#define REG_SERDATR     (0x018/2)   /* R      P  Serial port data and status read */
#define REG_DSKBYTR     (0x01A/2)   /* R      P  Disk data byte and status read */
#define REG_INTENAR     (0x01C/2)   /* R      P  Interrupt enable bits read */
#define REG_INTREQR     (0x01E/2)   /* R      P  Interrupt request bits read */
#define REG_DSKPTH      (0x020/2)   /* W  A      Disk pointer (high 3 bits) */
#define REG_DSKPTL      (0x022/2)   /* W  A      Disk pointer (low 15 bits) */
#define REG_DSKLEN      (0x024/2)   /* W      P  Disk length */
#define REG_DSKDAT      (0x026/2)   /* W      P  Disk DMA data write */
#define REG_REFPTR      (0x028/2)   /* W  A      Refresh pointer */
#define REG_VPOSW       (0x02A/2)   /* W  A      Write vert most signif. bit (and frame flop) */
#define REG_VHPOSW      (0x02C/2)   /* W  A      Write vert and horiz position of beam */
#define REG_COPCON      (0x02E/2)   /* W  A      Coprocessor control register (CDANG) */
#define REG_SERDAT      (0x030/2)   /* W      P  Serial port data and stop bits write */
#define REG_SERPER      (0x032/2)   /* W      P  Serial port period and control */
#define REG_POTGO       (0x034/2)   /* W      P  Pot port data write and start */
#define REG_JOYTEST     (0x036/2)   /* W    D    Write to all 4 joystick-mouse counters at once */
#define REG_STREQU      (0x038/2)   /* S    D    Strobe for horiz sync with VB and EQU */
#define REG_STRVBL      (0x03A/2)   /* S    D    Strobe for horiz sync with VB (vert. blank) */
#define REG_STRHOR      (0x03C/2)   /* S    D P  Strobe for horiz sync */
#define REG_STRLONG     (0x03E/2)   /* S    D    Strobe for identification of long horiz. line. */
#define REG_BLTCON0     (0x040/2)   /* W  A      Blitter control register 0 */
#define REG_BLTCON1     (0x042/2)   /* W  A      Blitter control register 1 */
#define REG_BLTAFWM     (0x044/2)   /* W  A      Blitter first word mask for source A */
#define REG_BLTALWM     (0x046/2)   /* W  A      Blitter last word mask for source A */
#define REG_BLTCPTH     (0x048/2)   /* W  A      Blitter pointer to source C (high 3 bits) */
#define REG_BLTCPTL     (0x04A/2)   /* W  A      Blitter pointer to source C (low 15 bits) */
#define REG_BLTBPTH     (0x04C/2)   /* W  A      Blitter pointer to source B (high 3 bits) */
#define REG_BLTBPTL     (0x04E/2)   /* W  A      Blitter pointer to source B (low 15 bits) */
#define REG_BLTAPTH     (0x050/2)   /* W  A      Blitter pointer to source A (high 3 bits) */
#define REG_BLTAPTL     (0x052/2)   /* W  A      Blitter pointer to source A (low 15 bits) */
#define REG_BLTDPTH     (0x054/2)   /* W  A      Blitter pointer to destination D (high 3 bits) */
#define REG_BLTDPTL     (0x056/2)   /* W  A      Blitter pointer to destination D (low 15 bits) */
#define REG_BLTSIZE     (0x058/2)   /* W  A      Blitter start and size (window width, height) */
#define REG_BLTCON0L    (0x05A/2)   /* W  A      Blitter control 0, lower 8 bits (minterms) */
#define REG_BLTSIZV     (0x05C/2)   /* W  A      Blitter V size (for 15 bit vertical size) (ECS) */
#define REG_BLTSIZH     (0x05E/2)   /* W  A      Blitter H size and start (for 11 bit H size) (ECS) */
#define REG_BLTCMOD     (0x060/2)   /* W  A      Blitter modulo for source C */
#define REG_BLTBMOD     (0x062/2)   /* W  A      Blitter modulo for source B */
#define REG_BLTAMOD     (0x064/2)   /* W  A      Blitter modulo for source A */
#define REG_BLTDMOD     (0x066/2)   /* W  A      Blitter modulo for destination D */
#define REG_BLTCDAT     (0x070/2)   /* W  A      Blitter source C data register */
#define REG_BLTBDAT     (0x072/2)   /* W  A      Blitter source B data reglster */
#define REG_BLTADAT     (0x074/2)   /* W  A      Blitter source A data register */
#define REG_DENISEID    (0x07C/2)   /* R    D    Denise ID: OCS = <open bus>, ECS = 0xFC, AGA = 0xF8 */
#define REG_DSKSYNC     (0x07E/2)   /* W      P  Disk sync pattern register for disk read */
#define REG_COP1LCH     (0x080/2)   /* W  A      Coprocessor first location register (high 3 bits) */
#define REG_COP1LCL     (0x082/2)   /* W  A      Coprocessor first location register (low 15 bits) */
#define REG_COP2LCH     (0x084/2)   /* W  A      Coprocessor second location register (high 3 bits) */
#define REG_COP2LCL     (0x086/2)   /* W  A      Coprocessor second location register (low 15 bits) */
#define REG_COPJMP1     (0x088/2)   /* S  A      Coprocessor restart at first location */
#define REG_COPJMP2     (0x08A/2)   /* S  A      Coprocessor restart at second location */
#define REG_COPINS      (0x08C/2)   /* W  A      Coprocessor instruction fetch identify */
#define REG_DIWSTRT     (0x08E/2)   /* W  A      Display window start (upper left vert-horiz position) */
#define REG_DIWSTOP     (0x090/2)   /* W  A      Display window stop (lower right vert.-horiz. position) */
#define REG_DDFSTRT     (0x092/2)   /* W  A      Display bit plane data fetch start (horiz. position) */
#define REG_DDFSTOP     (0x094/2)   /* W  A      Display bit plane data fetch stop (horiz. position) */
#define REG_DMACON      (0x096/2)   /* W  A D P  DMA control write (clear or set) */
#define REG_CLXCON      (0x098/2)   /* W    D    Collision control */
#define REG_INTENA      (0x09A/2)   /* W      P  Interrupt enable bits (clear or set bits) */
#define REG_INTREQ      (0x09C/2)   /* W      P  Interrupt request bits (clear or set bits) */
#define REG_ADKCON      (0x09E/2)   /* W      P  Audio, disk, UART control */
#define REG_AUD0LCH     (0x0A0/2)   /* W  A      Audio channel 0 location (high 3 bits) */
#define REG_AUD0LCL     (0x0A2/2)   /* W  A      Audio channel 0 location (low 15 bits) */
#define REG_AUD0LEN     (0x0A4/2)   /* W      P  Audio channel 0 length */
#define REG_AUD0PER     (0x0A6/2)   /* W      P  Audio channel 0 period */
#define REG_AUD0VOL     (0x0A8/2)   /* W      P  Audio channel 0 volume */
#define REG_AUD0DAT     (0x0AA/2)   /* W      P  Audio channel 0 data */
#define REG_AUD1LCH     (0x0B0/2)   /* W  A      Audio channel 1 location (high 3 bits) */
#define REG_AUD1LCL     (0x0B2/2)   /* W  A      Audio channel 1 location (low 15 bits) */
#define REG_AUD1LEN     (0x0B4/2)   /* W      P  Audio channel 1 length */
#define REG_AUD1PER     (0x0B6/2)   /* W      P  Audio channel 1 period */
#define REG_AUD1VOL     (0x0B8/2)   /* W      P  Audio channel 1 volume */
#define REG_AUD1DAT     (0x0BA/2)   /* W      P  Audio channel 1 data */
#define REG_AUD2LCH     (0x0C0/2)   /* W  A      Audio channel 2 location (high 3 bits) */
#define REG_AUD2LCL     (0x0C2/2)   /* W  A      Audio channel 2 location (low 15 bits) */
#define REG_AUD2LEN     (0x0C4/2)   /* W      P  Audio channel 2 length */
#define REG_AUD2PER     (0x0C6/2)   /* W      P  Audio channel 2 period */
#define REG_AUD2VOL     (0x0C8/2)   /* W      P  Audio channel 2 volume */
#define REG_AUD2DAT     (0x0CA/2)   /* W      P  Audio channel 2 data */
#define REG_AUD3LCH     (0x0D0/2)   /* W  A      Audio channel 3 location (high 3 bits) */
#define REG_AUD3LCL     (0x0D2/2)   /* W  A      Audio channel 3 location (low 15 bits) */
#define REG_AUD3LEN     (0x0D4/2)   /* W      P  Audio channel 3 length */
#define REG_AUD3PER     (0x0D6/2)   /* W      P  Audio channel 3 period */
#define REG_AUD3VOL     (0x0D8/2)   /* W      P  Audio channel 3 volume */
#define REG_AUD3DAT     (0x0DA/2)   /* W      P  Audio channel 3 data */
#define REG_BPL1PTH     (0x0E0/2)   /* W  A      Bit plane 1 pointer (high 3 bits) */
#define REG_BPL1PTL     (0x0E2/2)   /* W  A      Bit plane 1 pointer (low 15 bits) */
#define REG_BPL2PTH     (0x0E4/2)   /* W  A      Bit plane 2 pointer (high 3 bits) */
#define REG_BPL2PTL     (0x0E6/2)   /* W  A      Bit plane 2 pointer (low 15 bits) */
#define REG_BPL3PTH     (0x0E8/2)   /* W  A      Bit plane 3 pointer (high 3 bits) */
#define REG_BPL3PTL     (0x0EA/2)   /* W  A      Bit plane 3 pointer (low 15 bits) */
#define REG_BPL4PTH     (0x0EC/2)   /* W  A      Bit plane 4 pointer (high 3 bits) */
#define REG_BPL4PTL     (0x0EE/2)   /* W  A      Bit plane 4 pointer (low 15 bits) */
#define REG_BPL5PTH     (0x0F0/2)   /* W  A      Bit plane 5 pointer (high 3 bits) */
#define REG_BPL5PTL     (0x0F2/2)   /* W  A      Bit plane 5 pointer (low 15 bits) */
#define REG_BPL6PTH     (0x0F4/2)   /* W  A      Bit plane 6 pointer (high 3 bits) */
#define REG_BPL6PTL     (0x0F6/2)   /* W  A      Bit plane 6 pointer (low 15 bits) */
#define REG_BPLCON0     (0x100/2)   /* W  A D    Bit plane control register (misc. control bits) */
#define REG_BPLCON1     (0x102/2)   /* W    D    Bit plane control reg. (scroll value PF1, PF2) */
#define REG_BPLCON2     (0x104/2)   /* W    D    Bit plane control reg. (priority control) */
#define REG_BPLCON3     (0x106/2)   /* W    D    Bit plane control reg (enhanced features) */
#define REG_BPL1MOD     (0x108/2)   /* W  A      Bit plane modulo (odd planes) */
#define REG_BPL2MOD     (0x10A/2)   /* W  A      Bit plane modulo (even planes) */
#define REG_BPLCON4     (0x10C/2)   /* W    D    Bit plane control reg. (display masks) */
#define REG_BPL1DAT     (0x110/2)   /* W    D    Bit plane 1 data (parallel-to-serial convert) */
#define REG_BPL2DAT     (0x112/2)   /* W    D    Bit plane 2 data (parallel-to-serial convert) */
#define REG_BPL3DAT     (0x114/2)   /* W    D    Bit plane 3 data (parallel-to-serial convert) */
#define REG_BPL4DAT     (0x116/2)   /* W    D    Bit plane 4 data (parallel-to-serial convert) */
#define REG_BPL5DAT     (0x118/2)   /* W    D    Bit plane 5 data (parallel-to-serial convert) */
#define REG_BPL6DAT     (0x11A/2)   /* W    D    Bit plane 6 data (parallel-to-serial convert) */
#define REG_BPL7DAT     (0x11C/2)   /* W    D    Bit plane 7 data (parallel-to-serial convert) */
#define REG_BPL8DAT     (0x11E/2)   /* W    D    Bit plane 8 data (parallel-to-serial convert) */
#define REG_SPR0PTH     (0x120/2)   /* W  A      Sprite 0 pointer (high 3 bits) */
#define REG_SPR0PTL     (0x122/2)   /* W  A      Sprite 0 pointer (low 15 bits) */
#define REG_SPR1PTH     (0x124/2)   /* W  A      Sprite 1 pointer (high 3 bits) */
#define REG_SPR1PTL     (0x126/2)   /* W  A      Sprite 1 pointer (low 15 bits) */
#define REG_SPR2PTH     (0x128/2)   /* W  A      Sprite 2 pointer (high 3 bits) */
#define REG_SPR2PTL     (0x12A/2)   /* W  A      Sprite 2 pointer (low 15 bits) */
#define REG_SPR3PTH     (0x12C/2)   /* W  A      Sprite 3 pointer (high 3 bits) */
#define REG_SPR3PTL     (0x12E/2)   /* W  A      Sprite 3 pointer (low 15 bits) */
#define REG_SPR4PTH     (0x130/2)   /* W  A      Sprite 4 pointer (high 3 bits) */
#define REG_SPR4PTL     (0x132/2)   /* W  A      Sprite 4 pointer (low 15 bits) */
#define REG_SPR5PTH     (0x134/2)   /* W  A      Sprite 5 pointer (high 3 bits) */
#define REG_SPR5PTL     (0x136/2)   /* W  A      Sprite 5 pointer (low 15 bits) */
#define REG_SPR6PTH     (0x138/2)   /* W  A      Sprite 6 pointer (high 3 bits) */
#define REG_SPR6PTL     (0x13A/2)   /* W  A      Sprite 6 pointer (low 15 bits) */
#define REG_SPR7PTH     (0x13C/2)   /* W  A      Sprite 7 pointer (high 3 bits) */
#define REG_SPR7PTL     (0x13E/2)   /* W  A      Sprite 7 pointer (low 15 bits) */
#define REG_SPR0POS     (0x140/2)   /* W  A D    Sprite 0 vert-horiz start position data */
#define REG_SPR0CTL     (0x142/2)   /* W  A D    Sprite 0 vert stop position and control data */
#define REG_SPR0DATA    (0x144/2)   /* W    D    Sprite 0 image data register A */
#define REG_SPR0DATB    (0x146/2)   /* W    D    Sprite 0 image data register B */
#define REG_SPR1POS     (0x148/2)   /* W  A D    Sprite 1 vert-horiz start position data */
#define REG_SPR1CTL     (0x14A/2)   /* W  A D    Sprite 1 vert stop position and control data */
#define REG_SPR1DATA    (0x14C/2)   /* W    D    Sprite 1 image data register A */
#define REG_SPR1DATB    (0x14E/2)   /* W    D    Sprite 1 image data register B */
#define REG_SPR2POS     (0x150/2)   /* W  A D    Sprite 2 vert-horiz start position data */
#define REG_SPR2CTL     (0x152/2)   /* W  A D    Sprite 2 vert stop position and control data */
#define REG_SPR2DATA    (0x154/2)   /* W    D    Sprite 2 image data register A */
#define REG_SPR2DATB    (0x156/2)   /* W    D    Sprite 2 image data register B */
#define REG_SPR3POS     (0x158/2)   /* W  A D    Sprite 3 vert-horiz start position data */
#define REG_SPR3CTL     (0x15A/2)   /* W  A D    Sprite 3 vert stop position and control data */
#define REG_SPR3DATA    (0x15C/2)   /* W    D    Sprite 3 image data register A */
#define REG_SPR3DATB    (0x15E/2)   /* W    D    Sprite 3 image data register B */
#define REG_SPR4POS     (0x160/2)   /* W  A D    Sprite 4 vert-horiz start position data */
#define REG_SPR4CTL     (0x162/2)   /* W  A D    Sprite 4 vert stop position and control data */
#define REG_SPR4DATA    (0x164/2)   /* W    D    Sprite 4 image data register A */
#define REG_SPR4DATB    (0x166/2)   /* W    D    Sprite 4 image data register B */
#define REG_SPR5POS     (0x168/2)   /* W  A D    Sprite 5 vert-horiz start position data */
#define REG_SPR5CTL     (0x16A/2)   /* W  A D    Sprite 5 vert stop position and control data */
#define REG_SPR5DATA    (0x16C/2)   /* W    D    Sprite 5 image data register A */
#define REG_SPR5DATB    (0x16E/2)   /* W    D    Sprite 5 image data register B */
#define REG_SPR6POS     (0x170/2)   /* W  A D    Sprite 6 vert-horiz start position data */
#define REG_SPR6CTL     (0x172/2)   /* W  A D    Sprite 6 vert stop position and control data */
#define REG_SPR6DATA    (0x174/2)   /* W    D    Sprite 6 image data register A */
#define REG_SPR6DATB    (0x176/2)   /* W    D    Sprite 6 image data register B */
#define REG_SPR7POS     (0x178/2)   /* W  A D    Sprite 7 vert-horiz start position data */
#define REG_SPR7CTL     (0x17A/2)   /* W  A D    Sprite 7 vert stop position and control data */
#define REG_SPR7DATA    (0x17C/2)   /* W    D    Sprite 7 image data register A */
#define REG_SPR7DATB    (0x17E/2)   /* W    D    Sprite 7 image data register B */
#define REG_COLOR00     (0x180/2)   /* W    D    Color table 00 */
#define REG_COLOR01     (0x182/2)   /* W    D    Color table 01 */
#define REG_COLOR02     (0x184/2)   /* W    D    Color table 02 */
#define REG_COLOR03     (0x186/2)   /* W    D    Color table 03 */
#define REG_COLOR04     (0x188/2)   /* W    D    Color table 04 */
#define REG_COLOR05     (0x18A/2)   /* W    D    Color table 05 */
#define REG_COLOR06     (0x18C/2)   /* W    D    Color table 06 */
#define REG_COLOR07     (0x18E/2)   /* W    D    Color table 07 */
#define REG_COLOR08     (0x190/2)   /* W    D    Color table 08 */
#define REG_COLOR09     (0x192/2)   /* W    D    Color table 09 */
#define REG_COLOR10     (0x194/2)   /* W    D    Color table 10 */
#define REG_COLOR11     (0x196/2)   /* W    D    Color table 11 */
#define REG_COLOR12     (0x198/2)   /* W    D    Color table 12 */
#define REG_COLOR13     (0x19A/2)   /* W    D    Color table 13 */
#define REG_COLOR14     (0x19C/2)   /* W    D    Color table 14 */
#define REG_COLOR15     (0x19E/2)   /* W    D    Color table 15 */
#define REG_COLOR16     (0x1A0/2)   /* W    D    Color table 16 */
#define REG_COLOR17     (0x1A2/2)   /* W    D    Color table 17 */
#define REG_COLOR18     (0x1A4/2)   /* W    D    Color table 18 */
#define REG_COLOR19     (0x1A6/2)   /* W    D    Color table 19 */
#define REG_COLOR20     (0x1A8/2)   /* W    D    Color table 20 */
#define REG_COLOR21     (0x1AA/2)   /* W    D    Color table 21 */
#define REG_COLOR22     (0x1AC/2)   /* W    D    Color table 22 */
#define REG_COLOR23     (0x1AE/2)   /* W    D    Color table 23 */
#define REG_COLOR24     (0x1B0/2)   /* W    D    Color table 24 */
#define REG_COLOR25     (0x1B2/2)   /* W    D    Color table 25 */
#define REG_COLOR26     (0x1B4/2)   /* W    D    Color table 26 */
#define REG_COLOR27     (0x1B6/2)   /* W    D    Color table 27 */
#define REG_COLOR28     (0x1B8/2)   /* W    D    Color table 28 */
#define REG_COLOR29     (0x1BA/2)   /* W    D    Color table 29 */
#define REG_COLOR30     (0x1BC/2)   /* W    D    Color table 30 */
#define REG_COLOR31     (0x1BE/2)   /* W    D    Color table 31 */
#define REG_BEAMCON0    (0x1dc/2)   // W  A      Programmable signal generator (ECS Agnus)
#define REG_DIWHIGH     (0x1E4/2)   /* W  A D    Display window upper bits for start/stop */
#define REG_FMODE       (0x1FC/2)   /* W  A D    Fetch mode */

/* DMACON bit layout */
#define DMACON_AUD0EN   0x0001
#define DMACON_AUD1EN   0x0002
#define DMACON_AUD2EN   0x0004
#define DMACON_AUD3EN   0x0008
#define DMACON_DSKEN    0x0010
#define DMACON_SPREN    0x0020
#define DMACON_BLTEN    0x0040
#define DMACON_COPEN    0x0080
#define DMACON_BPLEN    0x0100
#define DMACON_DMAEN    0x0200
#define DMACON_BLTPRI   0x0400
#define DMACON_RSVED1   0x0800
#define DMACON_RSVED2   0x1000
#define DMACON_BZERO    0x2000
#define DMACON_BBUSY    0x4000
#define DMACON_SETCLR   0x8000

/* BPLCON0 bit layout */
#define BPLCON0_RSVED1  0x0001
#define BPLCON0_ERSY    0x0002
#define BPLCON0_LACE    0x0004
#define BPLCON0_LPEN    0x0008
#define BPLCON0_BPU3    0x0010
#define BPLCON0_RSVED3  0x0020
#define BPLCON0_RSVED4  0x0040
#define BPLCON0_RSVED5  0x0080
#define BPLCON0_GAUD    0x0100
#define BPLCON0_COLOR   0x0200
#define BPLCON0_DBLPF   0x0400
#define BPLCON0_HOMOD   0x0800
#define BPLCON0_BPU0    0x1000
#define BPLCON0_BPU1    0x2000
#define BPLCON0_BPU2    0x4000
#define BPLCON0_HIRES   0x8000

/* INTENA/INTREQ bit layout */
#define INTENA_TBE      0x0001
#define INTENA_DSKBLK   0x0002
#define INTENA_SOFT     0x0004
#define INTENA_PORTS    0x0008
#define INTENA_COPER    0x0010
#define INTENA_VERTB    0x0020
#define INTENA_BLIT     0x0040
#define INTENA_AUD0     0x0080
#define INTENA_AUD1     0x0100
#define INTENA_AUD2     0x0200
#define INTENA_AUD3     0x0400
#define INTENA_RBF      0x0800
#define INTENA_DSKSYN   0x1000
#define INTENA_EXTER    0x2000
#define INTENA_INTEN    0x4000
#define INTENA_SETCLR   0x8000

#define MAX_PLANES 6 /* 0 to 6, inclusive ( but we count from 0 to 5 ) */


class amiga_state : public driver_device
{
public:
	amiga_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_agnus_id(AGNUS_NTSC)
		, m_denise_id(DENISE)
		, m_maincpu(*this, "maincpu")
		, m_cia_0(*this, "cia_0")
		, m_cia_1(*this, "cia_1")
		, m_rs232(*this, "rs232")
		, m_centronics(*this, "centronics")
//      , m_agnus(*this, "agnus")
//      , m_denise(*this, "denise")
		, m_copper(*this, "copper")
//      , m_blitter(*this, "blitter")
		, m_paula(*this, "paula")
		, m_fdc(*this, "fdc")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_overlay(*this, "overlay")
		, m_chipset(*this, "chipset")
		, m_input_device(*this, "input")
		, m_joy0dat_port(*this, "joy_0_dat")
		, m_joy1dat_port(*this, "joy_1_dat")
		, m_potgo_port(*this, "potgo")
		, m_pot0dat_port(*this, "POT0DAT")
		, m_pot1dat_port(*this, "POT1DAT")
		, m_joy_ports(*this, "p%u_joy", 1)
		, m_p1_mouse_x(*this, "p1_mouse_x")
		, m_p1_mouse_y(*this, "p1_mouse_y")
		, m_p2_mouse_x(*this, "p2_mouse_x")
		, m_p2_mouse_y(*this, "p2_mouse_y")
		, m_hvpos(*this, "HVPOS")
		, m_power_led(*this, "power_led")
		, m_chip_ram_mask(0)
		, m_cia_0_irq(0)
		, m_cia_1_irq(0)
		, m_pot0x(0), m_pot1x(0), m_pot0y(0), m_pot1y(0)
		, m_pot0dat(0x0000)
		, m_pot1dat(0x0000)
		, m_centronics_busy(0)
		, m_centronics_perror(0)
		, m_centronics_select(0)
		, m_gayle_reset(false)
		, m_diw()
		, m_diwhigh_valid(false)
		, m_previous_lof(true)
		, m_rx_shift(0)
		, m_tx_shift(0)
		, m_rx_state(0)
		, m_tx_state(0)
		, m_rx_previous(1)
	{
		std::fill(std::begin(m_custom_regs), std::end(m_custom_regs), 0);
	}

	/* chip RAM access */
	uint16_t read_chip_ram(offs_t byteoffs)
	{
		// We use rand() here so that an attempt to go beyond the allocated chip RAM
		// (hopefully) doesn't go unnoticed.
		// FIXME: most likely open bus instead.
		return EXPECTED(byteoffs < m_chip_ram.bytes()) ? m_chip_ram.read(byteoffs >> 1) : machine().rand();
	}

	void write_chip_ram(offs_t byteoffs, uint16_t data)
	{
		if (EXPECTED(byteoffs < m_chip_ram.bytes()))
			m_chip_ram.write(byteoffs >> 1, data);
	}

	uint16_t chip_ram_r(offs_t offset, uint16_t mem_mask = ~0)
	{
		return read_chip_ram(offset & ~1) & mem_mask;
	}

	void chip_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0)
	{
		uint16_t val = read_chip_ram(offset & ~1) & ~mem_mask;
		write_chip_ram(offset & ~1, val | data);
	}

	/* sprite states */
	uint8_t m_sprite_comparitor_enable_mask = 0;
	uint8_t m_sprite_dma_reload_mask = 0;
	uint8_t m_sprite_dma_live_mask = 0;
	uint8_t m_sprite_ctl_written = 0;
	uint32_t m_sprite_shiftreg[8]{};
	uint8_t m_sprite_remain[8]{};

	/* copper states */
	uint32_t m_copper_pc = 0;
	uint8_t m_copper_waiting = 0;
	uint8_t m_copper_waitblit = 0;
	uint16_t m_copper_waitval = 0;
	uint16_t m_copper_waitmask = 0;
	uint16_t m_copper_pending_offset = 0;
	uint16_t m_copper_pending_data = 0;
	int m_wait_offset = 0;

	/* playfield states */
	int m_last_scanline = 0;
	rgb_t m_ham_color;

	/* misc states */
	uint16_t m_genlock_color = 0;

	/* separate 6 in-order bitplanes into 2 x 3-bit bitplanes in two nibbles */
	// FIXME: we instantiate 256 entries so that it pleases AGA
	uint8_t m_separate_bitplanes[2][256];

	/* aga */
	int m_aga_diwhigh_written = 0;
	rgb_t m_aga_palette[256]{};
	rgb_t m_aga_ehb_palette[32 + 32]{};
	uint64_t m_aga_bpldat[8]{};
	uint16_t m_aga_sprdata[8][4]{};
	uint16_t m_aga_sprdatb[8][4]{};
	int m_aga_sprite_fetched_words = 0;
	int m_aga_sprite_dma_used_words[8]{};

	DECLARE_VIDEO_START( amiga );
	DECLARE_VIDEO_START( amiga_aga );
	void amiga_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void update_screenmode();

	TIMER_CALLBACK_MEMBER( scanline_callback );
	TIMER_CALLBACK_MEMBER( irq_process_callback );
	TIMER_CALLBACK_MEMBER( blitter_process_callback );
	void update_irqs();

	template <int P> ioport_value amiga_joystick_convert();
	ioport_value floppy_drive_status();

	void m68k_reset(int state);
	void kbreset_w(int state);

	uint16_t cia_r(offs_t offset, uint16_t mem_mask = ~0);
	void cia_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gayle_cia_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cia_0_port_a_write(uint8_t data);
	void cia_0_irq(int state);
	uint8_t cia_1_port_a_read();
	void cia_1_port_a_write(uint8_t data);
	void cia_1_irq(int state);

	void rs232_rx_w(int state);
	void rs232_dcd_w(int state);
	void rs232_dsr_w(int state);
	void rs232_ri_w(int state);
	void rs232_cts_w(int state);

	void centronics_ack_w(int state);
	void centronics_busy_w(int state);
	void centronics_perror_w(int state);
	void centronics_select_w(int state);

	uint16_t custom_chip_r(offs_t offset);
	void custom_chip_w(offs_t offset, uint16_t data);

	void paula_int_w(offs_t channel, u8 state);

	uint16_t rom_mirror_r(offs_t offset, uint16_t mem_mask = ~0);
	uint32_t rom_mirror32_r(offs_t offset, uint32_t mem_mask = ~0);

	void fdc_dskblk_w(int state);
	void fdc_dsksyn_w(int state);

	// standard clocks
	static constexpr XTAL CLK_28M_PAL = XTAL(28'375'160);
	static constexpr XTAL CLK_7M_PAL = CLK_28M_PAL / 4;
	static constexpr XTAL CLK_C1_PAL = CLK_28M_PAL / 8;
	static constexpr XTAL CLK_E_PAL = CLK_7M_PAL / 10;

	static constexpr XTAL CLK_28M_NTSC = XTAL(28'636'363);
	static constexpr XTAL CLK_7M_NTSC = CLK_28M_NTSC / 4;
	static constexpr XTAL CLK_C1_NTSC = CLK_28M_NTSC / 8;
	static constexpr XTAL CLK_E_NTSC = CLK_7M_NTSC / 10;

	// screen layout
	enum
	{
		SCREEN_WIDTH = 910,
		SCREEN_HEIGHT_PAL = 625,
		SCREEN_HEIGHT_NTSC = 525,
		VBLANK_PAL = 58, // 52
		VBLANK_NTSC = 42,
		HBLANK = 186
	};

	emu_timer *m_blitter_timer = nullptr;

	uint16_t m_agnus_id = 0;
	uint16_t m_denise_id = 0;

	void blitter_setup();

	void amiga_base(machine_config &config);
	void pal_video(machine_config &config);
	void ntsc_video(machine_config &config);
	void overlay_1mb_map(address_map &map) ATTR_COLD;
	void overlay_1mb_map32(address_map &map) ATTR_COLD;
	void overlay_2mb_map16(address_map &map) ATTR_COLD;
	void overlay_2mb_map32(address_map &map) ATTR_COLD;
	void overlay_512kb_map(address_map &map) ATTR_COLD;
protected:
	// A bit of a trick here: some registers are 32-bit. In order to efficiently
	// read them on both big-endian and little-endian systems, we store the custom
	// registers in 32-bit natural order. This means we need to XOR the register
	// address with 1 on little-endian systems.
	uint16_t &CUSTOM_REG(offs_t x) { return m_custom_regs[BYTE_XOR_BE(x)]; }
	int16_t &CUSTOM_REG_SIGNED(offs_t x) { return (int16_t &)CUSTOM_REG(x); }
	uint32_t &CUSTOM_REG_LONG(offs_t x) { return *(uint32_t *)&m_custom_regs[x]; }

	// agnus/alice chip id
	enum
	{
		AGNUS_PAL         = 0x00,
		AGNUS_NTSC        = 0x10,
		AGNUS_HR_PAL      = 0x20,
		AGNUS_HR_PAL_NEW  = 0x21,
		AGNUS_HR_NTSC     = 0x30,
		AGNUS_HR_NTSC_NEW = 0x31,
		ALICE_PAL         = 0x22,
		ALICE_PAL_NEW     = 0x23,
		ALICE_NTSC        = 0x32,
		ALICE_NTSC_NEW    = 0x33
	};

	// denise/lisa chip id
	enum
	{
		DENISE    = 0xffff, // actually this register doesn't exist on ocs
		DENISE_HR = 0x00fc,
		LISA      = 0x00f8
	};

	// chipset
	bool IS_OCS() const { return m_denise_id == 0xffff; }
	bool IS_ECS() const { return m_denise_id == 0xfc; }
	bool IS_AGA() const { return m_denise_id == 0xf8; }

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void custom_chip_reset();

	// interrupts
	void set_interrupt(int interrupt);
	virtual bool int2_pending();
	virtual bool int6_pending();
	void update_int2();
	void update_int6();

	virtual void vblank();

	virtual void potgo_w(uint16_t data) {}

	// joystick/mouse
	virtual uint16_t joy0dat_r();
	virtual uint16_t joy1dat_r();

	// serial
	virtual void rs232_tx(int state);

	// devices
	required_device<m68000_base_device> m_maincpu;
	required_device<mos8520_device> m_cia_0;
	required_device<mos8520_device> m_cia_1;
	optional_device<rs232_port_device> m_rs232;
	optional_device<centronics_device> m_centronics;
	required_device<amiga_copper_device> m_copper;
	required_device<paula_8364_device> m_paula;
	optional_device<amiga_fdc_device> m_fdc;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	required_device<address_map_bank_device> m_overlay;
	required_device<address_map_bank_device> m_chipset;

	// i/o ports
	optional_ioport m_input_device;
	optional_ioport m_joy0dat_port;
	optional_ioport m_joy1dat_port;
	optional_ioport m_potgo_port;
	optional_ioport m_pot0dat_port;
	optional_ioport m_pot1dat_port;
	optional_ioport_array<2> m_joy_ports;
	optional_ioport m_p1_mouse_x;
	optional_ioport m_p1_mouse_y;
	optional_ioport m_p2_mouse_x;
	optional_ioport m_p2_mouse_y;
	optional_ioport m_hvpos;

	output_finder<> m_power_led;
	memory_array m_chip_ram;
	uint32_t m_chip_ram_mask;

	int m_cia_0_irq = 0;
	int m_cia_1_irq = 0;

	uint16_t m_custom_regs[256]{};
	static const char *const s_custom_reg_names[0x100];

	void ocs_map(address_map &map) ATTR_COLD;
	void ecs_map(address_map &map) ATTR_COLD;
	void aga_map(address_map &map) ATTR_COLD;

	// TODO: move to Agnus/Alice
	u16 vposr_r();
	u16 vhposr_r();
	void vposw_w(u16 data);
	void bplcon0_w(u16 data);
	void aga_bplcon0_w(u16 data);

private:
	// blitter helpers
	uint32_t blit_ascending();
	uint32_t blit_descending();
	uint32_t blit_line();

	// video helpers
protected:
	void set_genlock_color(uint16_t color);
private:
	void sprite_dma_reset(int which);
	void sprite_enable_comparitor(int which, int enable);
	void fetch_sprite_data(int scanline, int sprite);
	void update_sprite_dma(int scanline);
	uint32_t interleave_sprite_data(uint16_t lobits, uint16_t hibits);
	int get_sprite_pixel(int x);
	uint8_t assemble_odd_bitplanes(int planes, int ebitoffs);
	uint8_t assemble_even_bitplanes(int planes, int ebitoffs);
	void fetch_bitplane_data(int plane);
	int update_ham(int newpix);
	void update_display_window();
	void render_scanline(bitmap_rgb32 &bitmap, int scanline);

	// AGA video helpers
	void aga_palette_write(int color_reg, uint16_t data);
	void aga_fetch_sprite_data(int scanline, int sprite);
	void aga_render_scanline(bitmap_rgb32 &bitmap, int scanline);
	void aga_update_sprite_dma(int scanline);
	int aga_get_sprite_pixel(int x);
	uint8_t aga_assemble_odd_bitplanes(int planes, int obitoffs);
	uint8_t aga_assemble_even_bitplanes(int planes, int ebitoffs);
	void aga_fetch_bitplane_data(int plane, u8 bitplane_fmode);
	rgb_t aga_update_ham(int newpix, int plane);

	enum
	{
		VPOSR_LOF = 0x8000  // long frame
	};

	enum
	{
		ADKCON_UARTBRK = 0x800  // send break
	};

	// serial port flags
	enum
	{
		SERDATR_RXD   = 0x0800, // serial data
		SERDATR_TSRE  = 0x1000, // transmit ready
		SERDATR_TBE   = 0x2000, // transmit buffer empty
		SERDATR_RBF   = 0x4000, // receive buffer full
		SERDATR_OVRUN = 0x8000  // receive buffer overrun
	};

	enum
	{
		SERPER_LONG = 0x8000    // 9-bit mode
	};

	static const uint16_t s_expand_byte[256];

	// pot counters
	int m_pot0x, m_pot1x, m_pot0y, m_pot1y;

	uint16_t m_pot0dat;
	uint16_t m_pot1dat;

	int m_centronics_busy;
	int m_centronics_perror;
	int m_centronics_select;

	emu_timer *m_irq_timer = nullptr;
	emu_timer *m_serial_timer = nullptr;
	emu_timer *m_scanline_timer = nullptr;

	bool m_gayle_reset;

	// display window
	rectangle m_diw;
	bool m_diwhigh_valid;

	bool m_previous_lof;
	bitmap_rgb32 m_flickerfixer;
	bitmap_rgb32 m_scanline_bitmap;

	uint16_t m_rx_shift;
	uint16_t m_tx_shift;

	int m_rx_state;
	int m_tx_state;
	int m_rx_previous;

	int m_rs232_dcd = 0;
	int m_rs232_dsr = 0;
	int m_rs232_ri = 0;
	int m_rs232_cts = 0;

	void serial_adjust();
	TIMER_CALLBACK_MEMBER(serial_shift);
	void rx_write(int level);

	uint32_t amiga_gethvpos();
};

#endif // MAME_AMIGA_AMIGA_H
