// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII Dual J/K flip-flop 74109 emulation
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#define JKFF_DEBUG      0   //!< define 1 to debug the transitions

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
	JKFF_0,                 //!< no inputs or outputs
	JKFF_CLK    = (1 << 0), //!< clock signal
	JKFF_J      = (1 << 1), //!< J input
	JKFF_K      = (1 << 2), //!< K' input
	JKFF_S      = (1 << 3), //!< S' input
	JKFF_C      = (1 << 4), //!< C' input
	JKFF_Q      = (1 << 5), //!< Q  output
	JKFF_Q0     = (1 << 6)  //!< Q' output
}   jkff_t;

#else   // ALTO2_DEFINE_CONSTANTS

#ifndef _A2JKFF_H_
#define _A2JKFF_H_

#if JKFF_DEBUG
/**
 * @brief simulate a 74109 J-K flip-flop with set and reset inputs
 *
 * @param s0 is the previous state of the FF's in- and outputs
 * @param s1 is the next state
 * @return returns the next state and probably modified Q output
 */
static inline jkff_t update_jkff(UINT8 s0, UINT8 s1, const char* jkff_name)
{
	switch (s1 & (JKFF_C | JKFF_S))
	{
	case JKFF_C | JKFF_S:   /* C' is 1, and S' is 1 */
		if (((s0 ^ s1) & s1) & JKFF_CLK) {
			/* rising edge of the clock */
			switch (s1 & (JKFF_J | JKFF_K))
			{
			case 0:
				/* both J and K' are 0: set Q to 0, Q' to 1 */
				s1 = (s1 & ~JKFF_Q) | JKFF_Q0;
				if (s0 & JKFF_Q) {
					LOG((LOG_DISK,9,"\t\t%s J:0 K':0 -> Q:0\n", jkff_name));
				}
				break;
			case JKFF_J:
				/* J is 1, and K' is 0: toggle Q */
				if (s0 & JKFF_Q)
					s1 = (s1 & ~JKFF_Q) | JKFF_Q0;
				else
					s1 = (s1 | JKFF_Q) & ~JKFF_Q0;
				LOG((LOG_DISK,9,"\t\t%s J:0 K':1 flip-flop Q:%d\n", jkff_name, (s1 & JKFF_Q) ? 1 : 0));
				break;
			case JKFF_K:
				if ((s0 ^ s1) & JKFF_Q) {
					LOG((LOG_DISK,9,"\t\t%s J:0 K':1 keep Q:%d\n", jkff_name, (s1 & JKFF_Q) ? 1 : 0));
				}
				/* J is 0, and K' is 1: keep Q as is */
				if (s0 & JKFF_Q)
					s1 = (s1 | JKFF_Q) & ~JKFF_Q0;
				else
					s1 = (s1 & ~JKFF_Q) | JKFF_Q0;
				break;
			case JKFF_J | JKFF_K:
				/* both J and K' are 1: set Q to 1 */
				s1 = (s1 | JKFF_Q) & ~JKFF_Q0;
				if (!(s0 & JKFF_Q)) {
					LOG((LOG_DISK,9,"\t\t%s J:1 K':1 -> Q:1\n", jkff_name));
				}
				break;
			}
		} else {
			/* keep Q */
			s1 = (s1 & ~JKFF_Q) | (s0 & JKFF_Q);
		}
		break;
	case JKFF_S:
		/* S' is 1, C' is 0: set Q to 0, Q' to 1 */
		s1 = (s1 & ~JKFF_Q) | JKFF_Q0;
		if (s0 & JKFF_Q) {
			LOG((LOG_DISK,9,"\t\t%s C':0 -> Q:0\n", jkff_name));
		}
		break;
	case JKFF_C:
		/* S' is 0, C' is 1: set Q to 1, Q' to 0 */
		s1 = (s1 | JKFF_Q) & ~JKFF_Q0;
		if (!(s0 & JKFF_Q)) {
			LOG((LOG_DISK,9,"\t\t%s S':0 -> Q:1\n", jkff_name));
		}
		break;
	case 0:
	default:
		/* unstable state (what to do?) */
		s1 = s1 | JKFF_Q | JKFF_Q0;
		LOG((LOG_DISK,9,"\t\t%s C':0 S':0 -> Q:1 and Q':1 <unstable>\n", jkff_name));
		break;
	}
	return static_cast<jkff_t>(s1);
}
#else   // JKFF_DEBUG
/**
 * @brief simulate a 74109 J-K flip-flop with set and reset inputs
 *
 * @param s0 is the previous state of the FF's in- and outputs
 * @param s1 is the next state
 * @return returns the next state and probably modified Q output
 */
static inline jkff_t update_jkff(UINT8 s0, UINT8 s1, const char*)
{
	switch (s1 & (JKFF_C | JKFF_S))
	{
	case JKFF_C | JKFF_S:   /* C' is 1, and S' is 1 */
		if (((s0 ^ s1) & s1) & JKFF_CLK) {
			/* rising edge of the clock */
			switch (s1 & (JKFF_J | JKFF_K))
			{
			case 0:
				/* both J and K' are 0: set Q to 0, Q' to 1 */
				s1 = (s1 & ~JKFF_Q) | JKFF_Q0;
				break;
			case JKFF_J:
				/* J is 1, and K' is 0: toggle Q */
				if (s0 & JKFF_Q)
					s1 = (s1 & ~JKFF_Q) | JKFF_Q0;
				else
					s1 = (s1 | JKFF_Q) & ~JKFF_Q0;
				break;
			case JKFF_K:
				/* J is 0, and K' is 1: keep Q as is */
				if (s0 & JKFF_Q)
					s1 = (s1 | JKFF_Q) & ~JKFF_Q0;
				else
					s1 = (s1 & ~JKFF_Q) | JKFF_Q0;
				break;
			case JKFF_J | JKFF_K:
				/* both J and K' are 1: set Q to 1 */
				s1 = (s1 | JKFF_Q) & ~JKFF_Q0;
				break;
			}
		} else {
			/* keep Q */
			s1 = (s1 & ~JKFF_Q) | (s0 & JKFF_Q);
		}
		break;
	case JKFF_S:
		/* S' is 1, C' is 0: set Q to 0, Q' to 1 */
		s1 = (s1 & ~JKFF_Q) | JKFF_Q0;
		break;
	case JKFF_C:
		/* S' is 0, C' is 1: set Q to 1, Q' to 0 */
		s1 = (s1 | JKFF_Q) & ~JKFF_Q0;
		break;
	case 0:
	default:
		/* unstable state (what to do?) */
		s1 = s1 | JKFF_Q | JKFF_Q0;
		break;
	}
	return static_cast<jkff_t>(s1);
}
#endif  // JKFF_DEBUG

#endif  // _A2JKFF_H_
#endif  // ALTO2_DEFINE_CONSTANTS
