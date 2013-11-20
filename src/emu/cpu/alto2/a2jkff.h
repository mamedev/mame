/*****************************************************************************
 *
 *   Xerox AltoII Dual J/K flip-flop 74109 emulation
 *
 *   Copyright © Jürgen Buchmüller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 *
 *****************************************************************************/
#ifndef _A2JKFF_H_
#define _A2JKFF_H_

/**
 * @brief enumeration of the inputs and outputs of a JK flip-flop type 74109
 * <PRE>
 * 74109
 * Dual J-/K flip-flops with set and reset.
 *
 *       +----------+           +-----------------------------+
 * /1RST |1  +--+ 16| VCC       | J |/K |CLK|/SET|/RST| Q |/Q |
 *    1J |2       15| /2RST     |---+---+---+----+----+---+---|
 *   /1K |3       14| 2J        | X | X | X |  0 |  0 | 1 | 1 |
 *  1CLK |4   74  13| /2K       | X | X | X |  0 |  1 | 1 | 0 |
 * /1SET |5  109  12| 2CLK      | X | X | X |  1 |  0 | 0 | 1 |
 *    1Q |6       11| /2SET     | 0 | 0 | / |  1 |  1 | 0 | 1 |
 *   /1Q |7       10| 2Q        | 0 | 1 | / |  1 |  1 | - | - |
 *   GND |8        9| /2Q       | 1 | 0 | / |  1 |  1 |/Q | Q |
 *       +----------+           | 1 | 1 | / |  1 |  1 | 1 | 0 |
 *                              | X | X |!/ |  1 |  1 | - | - |
 *                              +-----------------------------+
 *
 * [This information is part of the GIICM]
 * </PRE>
 */
typedef enum {
	JKFF_0,					//!< no inputs or outputs
	JKFF_CLK	= (1 << 0),	//!< clock signal
	JKFF_J		= (1 << 1),	//!< J input
	JKFF_K		= (1 << 2),	//!< K' input
	JKFF_S		= (1 << 3),	//!< S' input
	JKFF_C		= (1 << 4),	//!< C' input
	JKFF_Q		= (1 << 5),	//!< Q  output
	JKFF_Q0		= (1 << 6)	//!< Q' output
}	jkff_t;

#endif // A2JKFF_H
