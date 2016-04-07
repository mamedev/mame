// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2008

   Hewlett Packard HP48 S/SX & G/GX/G+ and HP49 G

**********************************************************************/

#include "emu.h"
#include "cpu/saturn/saturn.h"
#include "sound/dac.h"
#include "machine/nvram.h"

#include "includes/hp48.h"

/* TODO:
   - IR I/O port
   - HP48 to HP48 serial connection
   - set card size when creating a new card image
   - some bits I/O RAM not implemented
   - printer
   - more accurate timing (accurate SATURN cycles, speedup when screen is off)
   - save-state: figure what RAM to store & where (currently, when exiting,
   base RAM is saved as nvram, RAM in expansion ports is saved as image,
   IO RAM is & CPU state are discarded; save-state saves base & IO RAM,
   CPU state, but not expansion port RAM)
   - more accurate IRQ, NMI, sleep and wake-up handling
   - HP49 G flash ROM write
   - HP49 xmodem
*/


/*
  HP48: graphical calculators by Hewlett Packard.

  Two main series: S series & G series.
  Each series is subdivised into an 'expandable' model (SX,GX) and a
  cheaper 'non-expandable' model (S,G).

  Timeline:
  - 1990-03-06 HP48SX (discontinued in ????)
  - 1991-04-02 HP48S  (discontinued in 1993)
  - 1993-06-01 HP48G  (discontinued in 1999)
               HP48GX (discontinued in 2003)
  - 1998-03-30 HP48G+ (discontinued in 2003)

  Common characteristics:
  - HP family name: Charlemagne
  - CPU: Saturn, 4-bit processor, chip version: 1LT8
  - BUS:
      . 20-bit address bus
      . nibble (4-bit) addressable
      => 512 KB address space (i.e., 1 Mnibbles)
      . daisy-chained modules
  - Screen: 131x64 LCD, 2-color, 64 Hz refresh
  - Keyboard: 49 keys
  - IR I/O port
  - 4-wire serial I/O port
  - 1-bit beeper
  - Power-source: 3 AAA cells
  - Continuous memory


  S series specific:
  - Clarke IC
  - clock speed: 2 MHz
  - 256 KB ROM (various revisions, identical in S and SX)

  - S model specific:
    + Codename: Shorty
    + 32 KB RAM base, not expandable (in theory)
    + no expansion port

  - SX model specific:
    + Codename: Charlemagne
    + 32 KB base RAM, expandable to 288 KB
    + 2 expansion ports (port 1 and port 2)
      up to 128 KB RAM per port, can be merged with base RAM

  G series specific:
  - Yorke IC
  - clock speed: ~4 MHz
  - 512 KB ROM (various revisions, identical in G and GX)

  - G model specific:
    + Codename: Alcuin
    + 32 KB RAM, not expandable (in theory!)
    + no expansion port

  - GX model specific:
    + Codename: Hammer
    + 128 KB base RAM, expandable up to 4 MB (banked)
    + 2 expansion ports
      . port 1: up to 128 KB RAM, can be merged with base RAM
      . port 2: up to   4 MB RAM, in 32 banks of 128 KB, cannot be merged

  - G+ model specific:
    + Codename: Plus
    + 128 KB RAM, not expandable
    + no expansion port



  HP49 G:
  similar to HP48 G with the following differences:
  - 2048 KB flash ROM (re-writtable)
  - 512 KB RAM (not expandable)
  - Keyboard: 51 keys
  - no IR I/O port
  - rewritten optimized OS makes the HP49 G much faster than HP48 despite
  having the same CPU and clock rate (4 MHz York Saturn)

*/


/*
   More about memory.

   The base RAM is treated as NVRAM: stored in a .nv file in your nvram
   directory.
   When first starting the emulator, or after nvram file has been deleted,
   you'll get a "Try to recover memory?". This is expected.
   Answer NO (F key): there is nothing to recover, but it will setup the
   system-reserved RAM part correctly.
   If the emulator is shut down properly (i.e., when the calculator is
   not busy), the message should not appear next time it is run.
   All the variables and port 0 should be conserved (in the .nv file),
   but not the stack.
   Stopping and restarting the emulator loosely corresponds to hitting the
   reset button.

   Expansion RAMs are treated as images, not NVRAM.
   The -p1 (resp. -p2) image option allows mapping some file to port 1
   (resp. port 2).
   The file size should be between 32 KB and 128 KB (or 4 GB for port 2 on GX)
   and be a power of two.
*/


/*
  More about serial connection.

  Normally, files are exchanged by serial connection to a computer running a
  kermit communication program (xmodem is also allowed for the G/GX).
  We emulate both the serial link and the communication program.
  Thus, the emulated HP48 can directly upload/download image files.
  The -k image option corresponds to the kermit protocol (SEND and RECV
  commands on the HP48).
  The -x image option corresponds to the xmodem protocol (XSEND and XRECV
  commands on the HP48).
*/



/*

  References:

  - Voyage au centre de la HP48 S/SX, by Paul Courbis & S?bastien Lalande
    (English version: HP48 machine language - a journey to the center
     of the HP48 s/sx)
    available at http://www.courbis.com

  - Voyage au centre de la HP48 G/GX, by Paul Courbis
    available at http://www.courbis.com

  - Guide to the Saturn Processor (With HP48 Applications)
    by Matthew Mastracci

  - Introduction to Saturn Assembly Language
    by Gilbert Fernandes

  - http://www.hpcalc.org/

*/




/**************************** inputs *******************************/

/*  KEYBOARD
    --------

    keyboard layout for 48 models

   -------------------------------------------------
   |   A   |   B   |   C   |   D   |   E   |   F   |
   |-------+-------+-------+-------+-------+-------|
   |  MTH  |  PRG  |  CST  |  VAR  |   up  |  NXT  |
   |-------+-------+-------+-------+-------+-------|
   |   '   |  STO  |  EVAL |  left | down  | right |
   |-------+-------+-------+-------+-------+-------|
   |  SIN  |  COS  |  TAN  |  sqrt |  y^x  |  1/x  |
   |---------------+-------+-------+-------+-------|
   |     ENTER     |  +/-  |  EEX  |  DEL  |  <=   |
   |-----------------------------------------------|
   |  alpha  |   7    |    8   |    9    |    /    |
   |---------+--------+--------+---------+---------|
   |   red   |   4    |    5   |    6    |    *    |
   |---------+--------+--------+---------+---------|
   |  green  |   1    |    2   |    3    |    -    |
   |---------+--------+--------+---------+---------|
   |   ON    |   0    |    .   |   SPC   |    +    |
   -------------------------------------------------

   * 49 keys
   * including 3 modifier keys:
     - a  blue/green shift (upper right)
     - an orange/red shift (upper left)
     - a  white alpha key (lower right)

   Difference between G and S series: a few red and blue shifted keys.

*/

/* S/SX */
static INPUT_PORTS_START( hp48sx_kbd )

		PORT_START( "LINE0" ) /* OUT = 0x001 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "+      { }  : :" )
	PORT_CODE ( KEYCODE_EQUALS )
	PORT_CODE ( KEYCODE_PLUS_PAD )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "SPC    \xCF\x80 \xE2\x88\xA1" /* pi, angle */ )
	PORT_CODE ( KEYCODE_SPACE )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( ".      ,  \xE2\x86\xb5" /* return arrow */ )
	PORT_CODE ( KEYCODE_STOP )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "0      =  \xE2\x86\x92" /* right arrow */ )
	PORT_CODE ( KEYCODE_0 )
	PORT_CODE ( KEYCODE_0_PAD )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "'  M   UP  HOME" )
	PORT_CODE ( KEYCODE_QUOTE )
	PORT_CODE ( KEYCODE_M )

		PORT_BIT ( 0xffe0, 0, IPT_UNUSED )


		PORT_START( "LINE1" ) /* OUT = 0x002 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "-      << >>  \" \"" )
	PORT_CODE ( KEYCODE_MINUS )
	PORT_CODE ( KEYCODE_MINUS_PAD )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "3      CMD  MENU" )
	PORT_CODE ( KEYCODE_3 )
	PORT_CODE ( KEYCODE_3_PAD )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "2      STACK ARG" )
	PORT_CODE ( KEYCODE_2 )
	PORT_CODE ( KEYCODE_2_PAD )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "1       RAD POLAR" )
	PORT_CODE ( KEYCODE_1 )
	PORT_CODE ( KEYCODE_1_PAD )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "A" )
	PORT_CODE ( KEYCODE_A )

	PORT_BIT  ( 32, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "right shift" )
	PORT_CODE ( KEYCODE_RSHIFT )

		PORT_BIT ( 0xffc0, 0, IPT_UNUSED )


		PORT_START( "LINE2" ) /* OUT = 0x004 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "*      [ ]     _" )
	PORT_CODE ( KEYCODE_ASTERISK )
	PORT_CODE ( KEYCODE_CLOSEBRACE )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "6              UNITS" )
	PORT_CODE ( KEYCODE_6_PAD )
	PORT_CODE ( KEYCODE_6 )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "5              STAT" )
	PORT_CODE ( KEYCODE_5_PAD )
	PORT_CODE ( KEYCODE_5 )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "4              TIME" )
	PORT_CODE ( KEYCODE_4_PAD )
	PORT_CODE ( KEYCODE_4 )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "MTH   G  PRINT")
	PORT_CODE ( KEYCODE_G )

	PORT_BIT  ( 32, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "left shift" )
	PORT_CODE ( KEYCODE_LSHIFT )

	PORT_BIT ( 0xffc0, 0, IPT_UNUSED )


		PORT_START( "LINE3" ) /* OUT = 0x008 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "/      ( )     #" )
	PORT_CODE ( KEYCODE_SLASH )
	PORT_CODE ( KEYCODE_SLASH_PAD )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "9              ALGEBRA" )
	PORT_CODE ( KEYCODE_9_PAD )
	PORT_CODE ( KEYCODE_9 )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "8              PLOT" )
	PORT_CODE ( KEYCODE_8_PAD )
	PORT_CODE ( KEYCODE_8 )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "7              SOLVE" )
	PORT_CODE ( KEYCODE_7_PAD )
	PORT_CODE ( KEYCODE_7 )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "SIN   S  ASIN  \xE2\x88\x82" /* delta */ )
	PORT_CODE ( KEYCODE_S )

	PORT_BIT  ( 32, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "\xCE\xB1       USER ENTRY" /* alpha */ )
	PORT_CODE ( KEYCODE_LALT )

	PORT_BIT ( 0xffc0, 0, IPT_UNUSED )


		PORT_START( "LINE4" ) /* OUT = 0x010 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME (" \xE2\x87\x90       DROP  CLR" /* double left arrow */ )
	PORT_CODE ( KEYCODE_BACKSPACE )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "DEL        PURGE" )
	PORT_CODE ( KEYCODE_DEL )
	PORT_CODE ( KEYCODE_DEL_PAD )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "EEX  Z   2D   3D" )
	PORT_CODE ( KEYCODE_Z )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "+/-  Y   EDIT  VISIT" )
	PORT_CODE ( KEYCODE_Y )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "ENTER     EQUATION MATRIX")
	PORT_CODE ( KEYCODE_ENTER )
	PORT_CODE ( KEYCODE_ENTER_PAD )

	PORT_BIT ( 0xffe0, 0, IPT_UNUSED )


		PORT_START( "LINE5" ) /* OUT = 0x020 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "1/x  X   e^x   LN" )
	PORT_CODE ( KEYCODE_X )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "y^x  W   10^x  LOG" )
	PORT_CODE ( KEYCODE_W )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "\xE2\x88\x9A  V   x^2  sqrt(x,y)" /* square root */ )
	PORT_CODE ( KEYCODE_V )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "TAN  U   ATAN  \xE2\x88\x91" /* sum */ )
	PORT_CODE ( KEYCODE_U )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "COS  T   ACOS  \xE2\x88\xAB" /* integral */ )
	PORT_CODE ( KEYCODE_T )

		PORT_BIT ( 0xffe0, 0, IPT_UNUSED )


		PORT_START( "LINE6" ) /* OUT = 0x040 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "\xE2\x86\x92  R        SWAP" /* right arrow */ )
	PORT_CODE ( KEYCODE_RIGHT )
	PORT_CODE ( KEYCODE_R )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "\xE2\x86\x93  Q        REVIEW" /* down arrow */ )
	PORT_CODE ( KEYCODE_DOWN )
	PORT_CODE ( KEYCODE_Q)

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "\xE2\x86\x90  P        GRAPH" /* left arrow */ )
	PORT_CODE ( KEYCODE_LEFT )
	PORT_CODE ( KEYCODE_P )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "EVAL O   \xE2\x86\x92NUM UNDO" )
	PORT_CODE ( KEYCODE_O )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "STO  N   DEF   RCL" )
	PORT_CODE ( KEYCODE_N )

		PORT_BIT ( 0xffe0, 0, IPT_UNUSED )


		PORT_START( "LINE7" ) /* OUT = 0x080 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "NXT  L   PREV" )
	PORT_CODE ( KEYCODE_L )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "\xE2\x86\x91  K        LIBRARY" /* up arrow */ )
	PORT_CODE ( KEYCODE_UP )
	PORT_CODE ( KEYCODE_K )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "VAR  J         MEMORY" )
	PORT_CODE ( KEYCODE_J )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "CST  I         MODES" )
	PORT_CODE ( KEYCODE_I )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "PRG  H         I/O" )
	PORT_CODE ( KEYCODE_H )

		PORT_BIT ( 0xffe0, 0, IPT_UNUSED )


		PORT_START( "LINE8" ) /* OUT = 0x100 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "F" )
	PORT_CODE ( KEYCODE_F )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "E" )
	PORT_CODE ( KEYCODE_E )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "D" )
	PORT_CODE ( KEYCODE_D )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "C" )
	PORT_CODE ( KEYCODE_C )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "B" )
	PORT_CODE ( KEYCODE_B )

		PORT_BIT ( 0xffe0, 0, IPT_UNUSED )


		PORT_START( "ON" ) /* ON key, appears on all OUT lines */

	PORT_BIT  ( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "ON  CANCEL CONT OFF" )
	PORT_CODE ( KEYCODE_ESC )
	PORT_CODE ( KEYCODE_HOME )

		PORT_BIT ( 0x7fff, 0, IPT_UNUSED )

INPUT_PORTS_END



/* G/GX/G+ */
static INPUT_PORTS_START( hp48gx_kbd )

		PORT_START( "LINE0" ) /* OUT = 0x001 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "+      { }  : :" )
	PORT_CODE ( KEYCODE_EQUALS )
	PORT_CODE ( KEYCODE_PLUS_PAD )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "SPC    \xCF\x80 \xE2\x88\xA1" /* pi, angle */ )
	PORT_CODE ( KEYCODE_SPACE )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( ".      ,  \xE2\x86\xb5" /* return arrow */ )
	PORT_CODE ( KEYCODE_STOP )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "0      =  \xE2\x86\x92" /* right arrow */ )
	PORT_CODE ( KEYCODE_0 )
	PORT_CODE ( KEYCODE_0_PAD )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "'  M   UP  HOME" )
	PORT_CODE ( KEYCODE_QUOTE )
	PORT_CODE ( KEYCODE_M )

		PORT_BIT ( 0xffe0, 0, IPT_UNUSED )


		PORT_START( "LINE1" ) /* OUT = 0x002 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "-      << >>  \" \"" )
	PORT_CODE ( KEYCODE_MINUS )
	PORT_CODE ( KEYCODE_MINUS_PAD )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "3             EQ LIB" )
	PORT_CODE ( KEYCODE_3 )
	PORT_CODE ( KEYCODE_3_PAD )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "2             LIBRARY" )
	PORT_CODE ( KEYCODE_2 )
	PORT_CODE ( KEYCODE_2_PAD )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "1             I/O" )
	PORT_CODE ( KEYCODE_1 )
	PORT_CODE ( KEYCODE_1_PAD )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "A" )
	PORT_CODE ( KEYCODE_A )

	PORT_BIT  ( 32, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "right shift" )
	PORT_CODE ( KEYCODE_RSHIFT )

		PORT_BIT ( 0xffc0, 0, IPT_UNUSED )


		PORT_START( "LINE2" ) /* OUT = 0x004 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "*      [ ]     _" )
	PORT_CODE ( KEYCODE_ASTERISK )
	PORT_CODE ( KEYCODE_CLOSEBRACE )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "6              UNITS" )
	PORT_CODE ( KEYCODE_6_PAD )
	PORT_CODE ( KEYCODE_6 )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "5              STAT" )
	PORT_CODE ( KEYCODE_5_PAD )
	PORT_CODE ( KEYCODE_5 )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "4              TIME" )
	PORT_CODE ( KEYCODE_4_PAD )
	PORT_CODE ( KEYCODE_4 )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "MTH   G  RAD   POLAR")
	PORT_CODE ( KEYCODE_G )

	PORT_BIT  ( 32, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "left shift" )
	PORT_CODE ( KEYCODE_LSHIFT )

	PORT_BIT ( 0xffc0, 0, IPT_UNUSED )


		PORT_START( "LINE3" ) /* OUT = 0x008 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "/      ( )     #" )
	PORT_CODE ( KEYCODE_SLASH )
	PORT_CODE ( KEYCODE_SLASH_PAD )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "9              SYMBOLIC" )
	PORT_CODE ( KEYCODE_9_PAD )
	PORT_CODE ( KEYCODE_9 )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "8              PLOT" )
	PORT_CODE ( KEYCODE_8_PAD )
	PORT_CODE ( KEYCODE_8 )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "7              SOLVE" )
	PORT_CODE ( KEYCODE_7_PAD )
	PORT_CODE ( KEYCODE_7 )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "SIN   S  ASIN  \xE2\x88\x82" /* delta */ )
	PORT_CODE ( KEYCODE_S )

	PORT_BIT  ( 32, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "\xCE\xB1       USER ENTRY" /* alpha */ )
	PORT_CODE ( KEYCODE_LALT )

	PORT_BIT ( 0xffc0, 0, IPT_UNUSED )


		PORT_START( "LINE4" ) /* OUT = 0x010 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME (" \xE2\x87\x90       DROP" /* double left arrow */ )
	PORT_CODE ( KEYCODE_BACKSPACE )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "DEL            CLEAR" )
	PORT_CODE ( KEYCODE_DEL )
	PORT_CODE ( KEYCODE_DEL_PAD )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "EEX  Z   PURG  ARG" )
	PORT_CODE ( KEYCODE_Z )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "+/-  Y   EDIT  CMD" )
	PORT_CODE ( KEYCODE_Y )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "ENTER     EQUATION MATRIX")
	PORT_CODE ( KEYCODE_ENTER )
	PORT_CODE ( KEYCODE_ENTER_PAD )

	PORT_BIT ( 0xffe0, 0, IPT_UNUSED )


		PORT_START( "LINE5" ) /* OUT = 0x020 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "1/x  X   e^x   LN" )
	PORT_CODE ( KEYCODE_X )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "y^x  W   10^x  LOG" )
	PORT_CODE ( KEYCODE_W )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "\xE2\x88\x9A  V   x^2  sqrt(x,y)" /* square root */ )
	PORT_CODE ( KEYCODE_V )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "TAN  U   ATAN  \xE2\x88\x91" /* sum */ )
	PORT_CODE ( KEYCODE_U )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "COS  T   ACOS  \xE2\x88\xAB" /* integral */ )
	PORT_CODE ( KEYCODE_T )

		PORT_BIT ( 0xffe0, 0, IPT_UNUSED )


		PORT_START( "LINE6" ) /* OUT = 0x040 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "\xE2\x86\x92  R        SWAP" /* right arrow */ )
	PORT_CODE ( KEYCODE_RIGHT )
	PORT_CODE ( KEYCODE_R )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "\xE2\x86\x93  Q        VIEW" /* down arrow */ )
	PORT_CODE ( KEYCODE_DOWN )
	PORT_CODE ( KEYCODE_Q)

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "\xE2\x86\x90  P        PICTURE" /* left arrow */ )
	PORT_CODE ( KEYCODE_LEFT )
	PORT_CODE ( KEYCODE_P )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "EVAL O   \xE2\x86\x92NUM UNDO" )
	PORT_CODE ( KEYCODE_O )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "STO  N   DEF   RCL" )
	PORT_CODE ( KEYCODE_N )

		PORT_BIT ( 0xffe0, 0, IPT_UNUSED )


		PORT_START( "LINE7" ) /* OUT = 0x080 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "NXT  L   PREV  MENU" )
	PORT_CODE ( KEYCODE_L )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "\xE2\x86\x91  K          STACK" /* up arrow */ )
	PORT_CODE ( KEYCODE_UP )
	PORT_CODE ( KEYCODE_K )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "VAR  J         MEMORY" )
	PORT_CODE ( KEYCODE_J )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "CST  I         MODES" )
	PORT_CODE ( KEYCODE_I )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "PRG  H         CHARS" )
	PORT_CODE ( KEYCODE_H )

		PORT_BIT ( 0xffe0, 0, IPT_UNUSED )


		PORT_START( "LINE8" ) /* OUT = 0x100 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "F" )
	PORT_CODE ( KEYCODE_F )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "E" )
	PORT_CODE ( KEYCODE_E )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "D" )
	PORT_CODE ( KEYCODE_D )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "C" )
	PORT_CODE ( KEYCODE_C )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "B" )
	PORT_CODE ( KEYCODE_B )

		PORT_BIT ( 0xffe0, 0, IPT_UNUSED )


		PORT_START( "ON" ) /* ON key, appears on all OUT lines */

	PORT_BIT  ( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "ON  CANCEL CONT OFF" )
	PORT_CODE ( KEYCODE_ESC )
	PORT_CODE ( KEYCODE_HOME )

		PORT_BIT ( 0x7fff, 0, IPT_UNUSED )

INPUT_PORTS_END




/*
   keyboard layout for 49 G model

   -------------------------------------------------
   |   F1  |   F2  |   F3  |   F4  |   F5  |  F6   |
   |-----------------------------------------------|
   |   APPS  |  MODE  |  TOOL  |        up         |
   |---------+--------+--------| left        right |
   |   VAR   |  STO   |  NXT   |       down        |
   |---------+--------+--------+-------------------|
   |   HIST  |  CAT   |  EQW   |  SYMB   |   <=    |
   |---------+--------+--------+---------+---------|
   |   y^x   |  sqrt  |  SIN   |  COS    |   TAN   |
   |---------+--------+--------+---------+---------|
   |   EEX   |  +/-   |   X    |  1/x    |    /    |
   |---------+--------+--------+---------+---------|
   |  alpha  |   7    |   8    |   9     |    *    |
   |---------+--------+--------+---------+---------|
   |   blue  |   4    |   5    |   6     |    -    |
   |---------+--------+--------+---------+---------|
   |   red   |   1    |   2    |   3     |    +    |
   |---------+--------+--------+---------+---------|
   |   ON    |   0    |    .   |  SPC    |  ENTER  |
   -------------------------------------------------

   * 51 keys
   * including 3 modifier keys:
     - a red shift (upper right)
     - a blue shift (upper left)
     - a cyan alpha key (lower right)
*/


static INPUT_PORTS_START( hp49g_kbd )

		PORT_START( "LINE0" ) /* OUT = 0x001 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "ENTER   ANS  NUM")
	PORT_CODE ( KEYCODE_ENTER )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "+  { }  \xC2\xAB \xC2\xBB")  /* << >> */
	PORT_CODE ( KEYCODE_EQUALS )
	PORT_CODE ( KEYCODE_PLUS_PAD )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "-  ( )  _")
	PORT_CODE ( KEYCODE_MINUS )
	PORT_CODE ( KEYCODE_MINUS_PAD )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "*  [ ]  \" \"")
	PORT_CODE ( KEYCODE_ASTERISK )
	PORT_CODE ( KEYCODE_CLOSEBRACE )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "/  ABS  ARG  Z")
	PORT_CODE ( KEYCODE_SLASH )
	PORT_CODE ( KEYCODE_SLASH_PAD )
	PORT_CODE ( KEYCODE_Z)

	PORT_BIT  ( 32, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "TAN  ATAN  \xE2\x88\xAB  U") /* integral */
	PORT_CODE ( KEYCODE_U )

	PORT_BIT  ( 64, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "\xE2\x87\x90  DEL  CLEAR") /* double left arrow */
	PORT_CODE ( KEYCODE_BACKSPACE )
	PORT_CODE ( KEYCODE_DEL )
	PORT_CODE ( KEYCODE_DEL_PAD )

	PORT_BIT  ( 128, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "NXT  PREV  PASTE  L")
	PORT_CODE ( KEYCODE_L )

		PORT_BIT ( 0x7f00, 0, IPT_UNUSED )


		PORT_START( "LINE1" ) /* OUT = 0x002 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "SPC  \xCF\x80  ,")  /* pi */
	PORT_CODE ( KEYCODE_SPACE )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "3  #  BASE")
	PORT_CODE ( KEYCODE_3 )
	PORT_CODE ( KEYCODE_3_PAD )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "6  CONVERT  UNITS")
	PORT_CODE ( KEYCODE_6 )
	PORT_CODE ( KEYCODE_6_PAD )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "9  FINANCE  TIME")
	PORT_CODE ( KEYCODE_9 )
	PORT_CODE ( KEYCODE_9_PAD )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "1/x  \xE2\x89\xA5  >  Y") /* >= */
	PORT_CODE ( KEYCODE_Y )

	PORT_BIT  ( 32, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "COS  ACOS  \xE2\x88\x82  T") /* delta */
	PORT_CODE ( KEYCODE_T )

	PORT_BIT  ( 64, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "SYMB  MTH  EVAL  P")
	PORT_CODE ( KEYCODE_P )

	PORT_BIT  ( 128, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "STO\xE2\x8A\xB3  RCL  CUT  K")
	PORT_CODE ( KEYCODE_K )

		PORT_BIT ( 0x7f00, 0, IPT_UNUSED )


		PORT_START( "LINE2" ) /* OUT = 0x004 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( ".  : :  \xE2\x86\xb5") /* return arrow */
	PORT_CODE ( KEYCODE_STOP )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "2  DEF  LIB")
	PORT_CODE ( KEYCODE_2 )
	PORT_CODE ( KEYCODE_2_PAD )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "5  MATRICES  STAT")
	PORT_CODE ( KEYCODE_5 )
	PORT_CODE ( KEYCODE_5_PAD )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "8  EXP&LN  TRIG")
	PORT_CODE ( KEYCODE_8 )
	PORT_CODE ( KEYCODE_8_PAD )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "X  \xE2\x89\xA4  <  X") /* <= */
	PORT_CODE ( KEYCODE_X )

	PORT_BIT  ( 32, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "SIN  ASIN  \xE2\x88\x91  S") /* sum */
	PORT_CODE ( KEYCODE_S )

	PORT_BIT  ( 64, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "EQW  MTRW  '  O")
	PORT_CODE ( KEYCODE_O )

	PORT_BIT  ( 128, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "VAR  UPDIR  COPY  J")
	PORT_CODE ( KEYCODE_J )

		PORT_BIT ( 0x7f00, 0, IPT_UNUSED )


		PORT_START( "LINE3" ) /* OUT = 0x008 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "0  \xE2\x88\x9E  \xE2\x86\x92") /* infinity, right arrow */
	PORT_CODE ( KEYCODE_0 )
	PORT_CODE ( KEYCODE_0_PAD )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "1  ARITH  CMPLX")
	PORT_CODE ( KEYCODE_1 )
	PORT_CODE ( KEYCODE_1_PAD )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "4  CALC  ALG")
	PORT_CODE ( KEYCODE_4 )
	PORT_CODE ( KEYCODE_4_PAD )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "7  SSLV  NUMSLV")
	PORT_CODE ( KEYCODE_7 )
	PORT_CODE ( KEYCODE_7_PAD )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "+/-  \xE2\x89\xA0  =  W") /* =/ */
	PORT_CODE ( KEYCODE_W )

	PORT_BIT  ( 32, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "\xE2\x88\x9A  x^2  sqrt(x,y)  R") /* square root */
	PORT_CODE ( KEYCODE_R )

	PORT_BIT  ( 64, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "CAT  PRG  CHARS  N")
	PORT_CODE ( KEYCODE_N )

	PORT_BIT  ( 128, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "TOOL i  |  I")
	PORT_CODE ( KEYCODE_I )

		PORT_BIT ( 0x7f00, 0, IPT_UNUSED )


		PORT_START( "LINE4" ) /* OUT = 0x010 */

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "EEX  10^x  LOG  V")
	PORT_CODE ( KEYCODE_V )

	PORT_BIT  ( 32, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "y^x  e^x  LN  Q")
	PORT_CODE ( KEYCODE_Q )

	PORT_BIT  ( 64, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "HIST  CMD  UNDO  M")
	PORT_CODE ( KEYCODE_M )

	PORT_BIT  ( 128, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "MODE  CUSTOM  END  H")
	PORT_CODE ( KEYCODE_H )

		PORT_BIT ( 0x7f0f, 0, IPT_UNUSED )


		PORT_START( "LINE5" ) /* OUT = 0x020 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "F1  Y=  A")
	PORT_CODE ( KEYCODE_A )
	PORT_CODE ( KEYCODE_F1 )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "F2  WIN  B")
	PORT_CODE ( KEYCODE_B )
	PORT_CODE ( KEYCODE_F2 )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "F3  GRAPH  C")
	PORT_CODE ( KEYCODE_C )
	PORT_CODE ( KEYCODE_F3 )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "F4  2D/3D  D")
	PORT_CODE ( KEYCODE_D )
	PORT_CODE ( KEYCODE_F4 )

	PORT_BIT  ( 16, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "F5  TBLSET  E")
	PORT_CODE ( KEYCODE_E )
	PORT_CODE ( KEYCODE_F5 )

	PORT_BIT  ( 32, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "F6  TABLE  F")
	PORT_CODE ( KEYCODE_F )
	PORT_CODE ( KEYCODE_F6 )

	PORT_BIT  ( 128, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "APPS  FILES  BEGIN  G")
	PORT_CODE ( KEYCODE_G )

		PORT_BIT ( 0x7f40, 0, IPT_UNUSED )


		PORT_START( "LINE6" ) /* OUT = 0x040 */

	PORT_BIT  ( 1, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "\xE2\x86\x92") /* right arrow */
	PORT_CODE ( KEYCODE_RIGHT )

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "\xE2\x86\x93") /* down arrow */
	PORT_CODE ( KEYCODE_DOWN )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "\xE2\x86\x90") /* left arrow */
	PORT_CODE ( KEYCODE_LEFT )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "\xE2\x86\x91") /* up arrow */
	PORT_CODE ( KEYCODE_UP )

		PORT_BIT ( 0x7ff0, 0, IPT_UNUSED )


		PORT_START( "LINE7" ) /* OUT = 0x080 */

	PORT_BIT  ( 2, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "right shift")
	PORT_CODE ( KEYCODE_RSHIFT )

	PORT_BIT  ( 4, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "left shift")
	PORT_CODE ( KEYCODE_LSHIFT )

	PORT_BIT  ( 8, IP_ACTIVE_HIGH, IPT_KEYBOARD )
		PORT_NAME ( "\xCE\xB1  USER  ENTRY") /* alpha */
	PORT_CODE ( KEYCODE_LALT )

		PORT_BIT ( 0x7ff1, 0, IPT_UNUSED )


		PORT_START( "LINE8" ) /* OUT = 0x100 */

		PORT_BIT ( 0xffff, 0, IPT_UNUSED )


		PORT_START( "ON" ) /* ON key, appears on all OUT lines */

	PORT_BIT  ( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_NAME ( "ON  CONT  OFF  CANCEL" )
	PORT_CODE ( KEYCODE_ESC )
	PORT_CODE ( KEYCODE_HOME )

		PORT_BIT ( 0x7fff, 0, IPT_UNUSED )

INPUT_PORTS_END




/*  BATTERY
    -------
 */

static INPUT_PORTS_START( hp48_battery )
		PORT_START( "BATTERY" )
		PORT_CONFNAME ( 0x03, 0, "Battery status" )
		PORT_CONFSETTING ( 0x00, DEF_STR( Normal ) )
		PORT_CONFSETTING ( 0x01, DEF_STR( Low ) )
		PORT_CONFSETTING ( 0x02, "Very low" )
INPUT_PORTS_END



/*  MAIN
    -----
 */

static INPUT_PORTS_START( hp48sx )
		PORT_INCLUDE( hp48sx_kbd )
		PORT_INCLUDE( hp48_battery )
INPUT_PORTS_END

static INPUT_PORTS_START( hp48gx )
		PORT_INCLUDE( hp48gx_kbd )
		PORT_INCLUDE( hp48_battery )
INPUT_PORTS_END

static INPUT_PORTS_START( hp49g )
		PORT_INCLUDE( hp49g_kbd )
		PORT_INCLUDE( hp48_battery )
INPUT_PORTS_END



/**************************** I/O **********************************/

//static const xmodem_config hp48_xmodem_rs232_conf = { &hp48_rs232_start_recv_byte };
//static const kermit_config hp48_kermit_rs232_conf = { &hp48_rs232_start_recv_byte };


/**************************** ROMs *********************************/

/* In ROMs, nibbles are packed into bytes, the lowest significant nibble in each byte
   has the lowest address on the HP.

   Note that some ROMs you can find on the Internet has the reverse the two nibbles.
   First byte should read 32 in hexa, not 23.
 */


/* These ROMS are common to the G, GX, and G+ models.
   The ROM detects whether it runs a G or a GX by simply testing the memory:
   if there are 32 KB (i.e., addresses wraps-around at 0x10000), it is a G; if there are
   128 KB, it is a GX.
   When a G is detected, some specially optimized routines may be used (they use the fact that
   no extension may be physically present).
   The G+ model has always revision R.
 */
ROM_START ( hp48gx )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("r")

	/* in chronological order, from first revision, version r is default*/
	ROM_SYSTEM_BIOS( 0, "k", "Version K" )
	ROMX_LOAD( "gxrom-k", 0x00000, 0x80000, CRC(bdd5d2ee) SHA1(afa1498238e991b1e3d07fb8b4c227b115f7bcc1), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 1, "l", "Version L" )
	ROMX_LOAD( "gxrom-l", 0x00000, 0x80000, CRC(70958e6b) SHA1(8eebac69ff804086247b989bf320e57a2d8a59a7), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS( 2, "m", "Version M" )
	ROMX_LOAD( "gxrom-m", 0x00000, 0x80000, CRC(e21a09e4) SHA1(09932d543594e459eeb94a79654168cd15e79a87), ROM_BIOS(3) )

	/* there does not seem to exist an N revision? */

	ROM_SYSTEM_BIOS( 3, "p", "Version P" )
	ROMX_LOAD( "gxrom-p", 0x00000, 0x80000, CRC(022d46df) SHA1(877a536865641f096212d1ce7296f580afbd6a2d), ROM_BIOS(4) )

	ROM_SYSTEM_BIOS( 4, "r", "Version R" )
	ROMX_LOAD( "gxrom-r", 0x00000, 0x80000, CRC(00ee1a62) SHA1(5705fc9ea791916c4456ac35e22275862411db9b), ROM_BIOS(5) )

ROM_END

#define rom_hp48g  rom_hp48gx
#define rom_hp48gp rom_hp48gx


/* These ROMS are common to the S and SX models.
   The only difference is that, the S being later, it was only shipped with revisions
   E and later.

   (Note that G/GX revisions start at K, after the S/S revisions ends...)
 */

ROM_START ( hp48sx )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("j")

	/* in chronological order, from first revision, version j is default*/
	ROM_SYSTEM_BIOS( 0, "a", "Version A" )
	ROMX_LOAD( "sxrom-a", 0x00000, 0x40000, CRC(a87696c7) SHA1(3271b103ad99254d069e20171beb418ace72cc90), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 1, "b", "Version B" )
	ROMX_LOAD( "sxrom-b", 0x00000, 0x40000, CRC(034f6ce4) SHA1(acd256f2efee868ce402008f4131d94b312e60bc), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS( 2, "c", "Version C" )
	ROMX_LOAD( "sxrom-c", 0x00000, 0x40000, CRC(a9a0279d) SHA1(fee852d43ae6941d07a9d0d31f37e68e4f9051b1), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS( 3, "d", "Version D" )
	ROMX_LOAD( "sxrom-d", 0x00000, 0x40000, CRC(f8f5dc58) SHA1(3be5f895f4c731fd4c863237c7342cab4e8c42b1), ROM_BIOS(4) )

	ROM_SYSTEM_BIOS( 4, "e", "Version E" )
	ROMX_LOAD( "sxrom-e", 0x00000, 0x40000, CRC(704ffa08) SHA1(0d498d135bf729c1d775cce522528837729e2e94), ROM_BIOS(5) )

	ROM_SYSTEM_BIOS( 5, "j", "Version J" )
	ROMX_LOAD( "sxrom-j", 0x00000, 0x40000, CRC(1a6378ef) SHA1(5235f5379f1fd7edfe9bb6bf466b60d279163e73), ROM_BIOS(6) )

	/* no F, G, H, I revisions? */

ROM_END

#define rom_hp48s rom_hp48sx

ROM_START ( hp38g )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hp38g.rom", 0x00000, 0x80000, CRC(31d9affc) SHA1(bab3f5907a16cbb087943fd77230514af8fd5ac0))
ROM_END

ROM_START ( hp39g )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "hp39g.rom", 0x00000, 0x100000, CRC(28268fdc) SHA1(57a2b19075fe60307a9affa79d8e7cb550c621c3))
ROM_END

ROM_START ( hp49g )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "1.00", "Version C-1.00" )
	ROMX_LOAD( "hp49gv100.rom", 0x00000, 0x200000, CRC(64c9826a) SHA1(da25371b97d439fc0003cb786dba143ee2be9160), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "1.05", "Version C-1.05" )
	ROMX_LOAD( "hp49gv105.rom", 0x00000, 0x200000, CRC(cf777cac) SHA1(b1d063b6e95083799aa990e4a2718214a38a372f), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "1.10", "Version C-1.10" )
	ROMX_LOAD( "hp49gv110.rom", 0x00000, 0x200000, CRC(e391efbd) SHA1(d4abad60f38faf4cb2d2d97804a24f54589dfa10), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "1.16", "Version C-1.16" )
	ROMX_LOAD( "hp49gv116.rom", 0x00000, 0x200000, CRC(dcc0b39c) SHA1(46f64b4731f5964eb114060b733aab2b23b4180c), ROM_BIOS(4))
	ROM_SYSTEM_BIOS( 4, "1.18", "Version C-1.18" )
	ROMX_LOAD( "hp49gv118.rom", 0x00000, 0x200000, CRC(73a6a195) SHA1(3f283fe15a64c5cbc8c1b1254e10965957f58a84), ROM_BIOS(5))
	ROM_SYSTEM_BIOS( 5, "1.19", "Version B-1.19-6" )
	ROMX_LOAD( "hp49gv119.rom", 0x00000, 0x200000, CRC(75218a18) SHA1(ec0f661f0aa7158d1f6df61f24410260b5324fa9), ROM_BIOS(6))
ROM_END

/**************************** memory *******************************/

/* In memory, nibbles are unpacked: one nibble at each address.
   This is due to the way the SATURN emulation is done.
   As a consequence only the 4 lower bits of each byte is used, the 4 higher
   bits being zeros.
   Another consequence is that ROMs must be unpacked before use.

   Because of the complex memory manager, actual address mapping is done at
   run-time.
 */

static ADDRESS_MAP_START ( hp48, AS_PROGRAM, 8, hp48_state )

	AM_RANGE( 0x00000, 0xfffff ) AM_NOP /* configured at run-time */

ADDRESS_MAP_END



/*************************** driver ********************************/


static MACHINE_CONFIG_START( hp48_common, hp48_state )

	/* cpu */
	MCFG_CPU_ADD ( "maincpu", SATURN, 3937007 ) /* almost 4 MHz */
	MCFG_CPU_PROGRAM_MAP ( hp48)
	MCFG_SATURN_CONFIG( WRITE32(hp48_state, hp48_reg_out), READ32(hp48_state, hp48_reg_in),
						WRITELINE(hp48_state, hp48_mem_reset), WRITE32(hp48_state, hp48_mem_config),
						WRITE32(hp48_state, hp48_mem_unconfig), READ32(hp48_state, hp48_mem_id),
						WRITE32(hp48_state, hp48_mem_crc), WRITELINE(hp48_state, hp48_rsi) )

	/* memory */
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video */
	MCFG_SCREEN_ADD( "screen", RASTER )
	MCFG_SCREEN_REFRESH_RATE( 64 )
	MCFG_SCREEN_VBLANK_TIME(0)
	MCFG_SCREEN_SIZE ( 131, 64 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 130, 0, 63 )
	MCFG_SCREEN_UPDATE_DRIVER(hp48_state, screen_update_hp48)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD( "palette", 256 ) /* monochrome, but with varying contrast and grayscale */
	MCFG_PALETTE_INIT_OWNER(hp48_state, hp48)

	/* sound */
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( "dac",  DAC, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.) /* 1-bit beeper */
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hp48gx, hp48_common )
	MCFG_MACHINE_START_OVERRIDE  (hp48_state, hp48gx )

	/* expansion ports */
	MCFG_HP48_PORT_ADD ( "port1", 0, HP48_CE2,     128*1024 )
	MCFG_HP48_PORT_ADD ( "port2", 1, HP48_NCE3, 4*1024*1024 )

	/* serial I/O */
	//MCFG_XMODEM_ADD( "rs232_x", hp48_xmodem_rs232_conf )
	//MCFG_KERMIT_ADD( "rs232_k", hp48_kermit_rs232_conf )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hp48g, hp48_common )
	MCFG_MACHINE_START_OVERRIDE  (hp48_state, hp48g )

	/* serial I/O */
	//MCFG_XMODEM_ADD( "rs232_x", hp48_xmodem_rs232_conf )
	//MCFG_KERMIT_ADD( "rs232_k", hp48_kermit_rs232_conf )
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( hp48gp, hp48_common )
	MCFG_MACHINE_START_OVERRIDE  (hp48_state, hp48gp )

	/* serial I/O */
	//MCFG_XMODEM_ADD( "rs232_x", hp48_xmodem_rs232_conf )
	//MCFG_KERMIT_ADD( "rs232_k", hp48_kermit_rs232_conf )
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( hp48sx, hp48_common )
	MCFG_CPU_MODIFY     ( "maincpu" )
	MCFG_CPU_CLOCK      ( 2000000 )
	MCFG_MACHINE_START_OVERRIDE  (hp48_state, hp48sx )

	/* expansion ports */
	MCFG_HP48_PORT_ADD  ( "port1", 0, HP48_CE1, 128*1024)
	MCFG_HP48_PORT_ADD  ( "port2", 1, HP48_CE2, 128*1024)

	/* serial I/O */
	//MCFG_KERMIT_ADD( "rs232_k", hp48_kermit_rs232_conf )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hp48s, hp48_common )
	MCFG_CPU_MODIFY     ( "maincpu" )
	MCFG_CPU_CLOCK      ( 2000000 )
	MCFG_MACHINE_START_OVERRIDE  (hp48_state, hp48s )

	/* serial I/O */
	//MCFG_KERMIT_ADD( "rs232_k", hp48_kermit_rs232_conf )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hp49g, hp48_common )
	MCFG_CPU_MODIFY     ( "maincpu" )
	MCFG_MACHINE_START_OVERRIDE  (hp48_state, hp49g )

	/* serial I/O */
		//MCFG_XMODEM_ADD( "rs232_x", hp48_xmodem_rs232_conf )
		//MCFG_KERMIT_ADD( "rs232_k", hp48_kermit_rs232_conf )
MACHINE_CONFIG_END


COMP ( 1990, hp48sx, 0     , 0, hp48sx, hp48sx, hp48_state, hp48, "Hewlett Packard", "HP48SX", 0 )
COMP ( 1991, hp48s , hp48sx, 0, hp48s,  hp48sx, hp48_state, hp48, "Hewlett Packard", "HP48S", 0 )
COMP ( 1993, hp48gx, 0     , 0, hp48gx, hp48gx, hp48_state, hp48, "Hewlett Packard", "HP48GX", 0 )
COMP ( 1993, hp48g , hp48gx, 0, hp48g,  hp48gx, hp48_state, hp48, "Hewlett Packard", "HP48G", 0 )
COMP ( 1998, hp48gp, hp48gx, 0, hp48gp, hp48gx, hp48_state, hp48, "Hewlett Packard", "HP48G+", 0 )
COMP ( 1999, hp49g , 0,      0, hp49g,  hp49g,  hp48_state, hp48, "Hewlett Packard", "HP49G", 0 )
COMP ( 1995, hp38g , 0,      0, hp48g,  hp48gx, hp48_state, hp48, "Hewlett Packard", "HP38G", 0 )
COMP ( 2000, hp39g , 0,      0, hp48g,  hp48gx, hp48_state, hp48, "Hewlett Packard", "HP39G", MACHINE_NOT_WORKING )
