// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII mouse hardware (MOUSE)
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef _A2MOUSE_H_
#define _A2MOUSE_H_
/**
 * @brief PROM madr.a32 contains a lookup table to translate mouse motions
 *
 * <PRE>
 * The 4 mouse motion signals MX1, MX2, MY1, and MY2 are connected
 * to a 256x4 PROM's (3601, SN74387) address lines A0, A2, A4, and A6.
 * The previous (latched) state of the 4 signals is connected to the
 * address lines A1, A3, A5, and A7.
 *
 *                  SN74387
 *               +---+--+---+
 *               |   +--+   |
 *  MY2     A6  -|1       16|-  Vcc
 *               |          |
 *  LMY1    A5  -|2       15|-  A7     LMY2
 *               |          |
 *  MY1     A4  -|3       14|-  FE1'   0
 *               |          |
 *  LMX2    A3  -|4       13|-  FE2'   0
 *               |          |
 *  MX1     A0  -|5       12|-  D0     BUS[12]
 *               |          |
 *  LMX1    A1  -|6       11|-  D1     BUS[13]
 *               |          |
 *  MX2     A2  -|7       10|-  D2     BUS[14]
 *               |          |
 *         GND  -|8        9|-  D3     BUS[15]
 *               |          |
 *               +----------+
 *
 * A motion to the west will first toggle MX2, then MX1.
 * sequence: 04 -> 0d -> 0b -> 02
 * A motion to the east will first toggle MX1, then MX2.
 * sequence: 01 -> 07 -> 0e -> 08
 *
 * A motion to the north will first toggle MY2, then MY1.
 * sequence: 40 -> d0 -> b0 -> 20
 * A motion to the south will first toggle MY1, then MY2.
 * sequence: 10 -> 70 -> e0 -> 80
 * </PRE>
 */
UINT8* m_madr_a32;

//! mouse context
struct {
	int x;                                      //!< current X coordinate
	int y;                                      //!< current Y coordinate
	int dx;                                     //!< destination X coordinate (real mouse X)
	int dy;                                     //!< destination Y coordinate (real mouse Y)
	UINT8 latch;                                //!< current latch value
	UINT8 phase;                                //!< current read latch phase
}   m_mouse;

UINT16 mouse_read();                            //!< return the mouse motion flags
void init_mouse();                              //!< initialize the mouse context
void exit_mouse();                              //!< deinitialize the mouse context
void reset_mouse();                             //!< reset the mouse context
#endif  // _A2MOUSE_H_
#endif  // ALTO2_DEFINE_CONSTANTS
