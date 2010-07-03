/*** m6800: Portable 6800 class emulator *************************************/

#pragma once

#ifndef __M6800_H__
#define __M6800_H__


enum
{
	M6800_PC=1, M6800_S, M6800_A, M6800_B, M6800_X, M6800_CC,
	M6800_WAI_STATE
};

enum
{
	M6800_IRQ_LINE = 0,				/* IRQ line number */
	M6800_TIN_LINE					/* P20/Tin Input Capture line (eddge sense)     */
									/* Active eddge is selecrable by internal reg.  */
									/* raise eddge : CLEAR_LINE  -> ASSERT_LINE     */
									/* fall  eddge : ASSERT_LINE -> CLEAR_LINE      */
									/* it is usuali to use PULSE_LINE state         */
};

/* By default, on a port write port bits which are not set as output in the DDR */
/* are set to the value returned by a read from the same port. If you need to */
/* know the DDR for e.g. port 1, do m6803_internal_registers_r(M6801_DDR1) */

enum
{
	M6803_DDR1	= 0x00,
	M6803_DDR2	= 0x01,
	M6803_DDR3	= 0x04,
	M6803_DDR4	= 0x05
};

enum
{
	M6803_PORT1 = 0x100,
	M6803_PORT2,
	M6803_PORT3,
	M6803_PORT4
};

DECLARE_LEGACY_CPU_DEVICE(M6800, m6800);
DECLARE_LEGACY_CPU_DEVICE(M6801, m6801);
DECLARE_LEGACY_CPU_DEVICE(M6802, m6802);
DECLARE_LEGACY_CPU_DEVICE(M6803, m6803);
DECLARE_LEGACY_CPU_DEVICE(M6808, m6808);
DECLARE_LEGACY_CPU_DEVICE(HD63701, hd63701);
DECLARE_LEGACY_CPU_DEVICE(NSC8105, nsc8105);

/* FIMXE: these should be replaced to use m6803 ones */
#define HD63701_DDR1 M6803_DDR1
#define HD63701_DDR2 M6803_DDR2
#define HD63701_DDR3 M6803_DDR3
#define HD63701_DDR4 M6803_DDR4

#define HD63701_PORT1 M6803_PORT1
#define HD63701_PORT2 M6803_PORT2
#define HD63701_PORT3 M6803_PORT3
#define HD63701_PORT4 M6803_PORT4

READ8_HANDLER( hd63701_internal_registers_r );
WRITE8_HANDLER( hd63701_internal_registers_w );


CPU_DISASSEMBLE( m6800 );
CPU_DISASSEMBLE( m6801 );
CPU_DISASSEMBLE( m6802 );
CPU_DISASSEMBLE( m6803 );
CPU_DISASSEMBLE( m6808 );
CPU_DISASSEMBLE( hd63701 );
CPU_DISASSEMBLE( nsc8105 );


#if 0
/* Wonder if we need it */
/****************************************************************************
 * For now make the 6801 using the m6800 variables and functions
 ****************************************************************************/
#define M6801_A 					M6800_A
#define M6801_B 					M6800_B
#define M6801_PC					M6800_PC
#define M6801_S 					M6800_S
#define M6801_X 					M6800_X
#define M6801_CC					M6800_CC
#define M6801_WAI_STATE 			M6800_WAI_STATE
#define M6801_NMI_STATE 			M6800_NMI_STATE
#define M6801_IRQ_STATE 			M6800_IRQ_STATE

#define M6801_WAI					M6800_WAI
#define M6801_IRQ_LINE				M6800_IRQ_LINE

/****************************************************************************
 * For now make the 6802 using the m6800 variables and functions
 ****************************************************************************/
#define M6802_A 					M6800_A
#define M6802_B 					M6800_B
#define M6802_PC					M6800_PC
#define M6802_S 					M6800_S
#define M6802_X 					M6800_X
#define M6802_CC					M6800_CC
#define M6802_WAI_STATE 			M6800_WAI_STATE
#define M6802_NMI_STATE 			M6800_NMI_STATE
#define M6802_IRQ_STATE 			M6800_IRQ_STATE

#define M6802_WAI					M6800_WAI
#define M6802_IRQ_LINE				M6800_IRQ_LINE

/****************************************************************************
 * For now make the 6803 using the m6800 variables and functions
 ****************************************************************************/
#define M6803_A 					M6800_A
#define M6803_B 					M6800_B
#define M6803_PC					M6800_PC
#define M6803_S 					M6800_S
#define M6803_X 					M6800_X
#define M6803_CC					M6800_CC
#define M6803_WAI_STATE 			M6800_WAI_STATE
#define M6803_NMI_STATE 			M6800_NMI_STATE
#define M6803_IRQ_STATE 			M6800_IRQ_STATE

#define M6803_WAI					M6800_WAI
#define M6803_IRQ_LINE				M6800_IRQ_LINE
#define M6803_TIN_LINE				M6800_TIN_LINE

/* By default, on a port write port bits which are not set as output in the DDR */
/* are set to the value returned by a read from the same port. If you need to */
/* know the DDR for e.g. port 1, do m6803_internal_registers_r(M6801_DDR1) */

#define M6803_DDR1	0x00
#define M6803_DDR2	0x01
#define M6803_DDR3	0x04
#define M6803_DDR4	0x05

#define M6803_PORT1 0x100
#define M6803_PORT2 0x101
#define M6803_PORT3 0x102
#define M6803_PORT4 0x103

/****************************************************************************
 * For now make the 6808 using the m6800 variables and functions
 ****************************************************************************/
#define M6808_A 					M6800_A
#define M6808_B 					M6800_B
#define M6808_PC					M6800_PC
#define M6808_S 					M6800_S
#define M6808_X 					M6800_X
#define M6808_CC					M6800_CC
#define M6808_WAI_STATE 			M6800_WAI_STATE
#define M6808_NMI_STATE 			M6800_NMI_STATE
#define M6808_IRQ_STATE 			M6800_IRQ_STATE

#define M6808_WAI                   M6800_WAI
#define M6808_IRQ_LINE              M6800_IRQ_LINE

/****************************************************************************
 * For now make the HD63701 using the m6800 variables and functions
 ****************************************************************************/
#define HD63701_A					 M6800_A
#define HD63701_B					 M6800_B
#define HD63701_PC					 M6800_PC
#define HD63701_S					 M6800_S
#define HD63701_X					 M6800_X
#define HD63701_CC					 M6800_CC
#define HD63701_WAI_STATE			 M6800_WAI_STATE
#define HD63701_NMI_STATE			 M6800_NMI_STATE
#define HD63701_IRQ_STATE			 M6800_IRQ_STATE

#define HD63701_WAI 				 M6800_WAI
#define HD63701_SLP 				 M6800_SLP
#define HD63701_IRQ_LINE			 M6800_IRQ_LINE
#define HD63701_TIN_LINE			 M6800_TIN_LINE

#define HD63701_DDR1 M6803_DDR1
#define HD63701_DDR2 M6803_DDR2
#define HD63701_DDR3 M6803_DDR3
#define HD63701_DDR4 M6803_DDR4

#define HD63701_PORT1 M6803_PORT1
#define HD63701_PORT2 M6803_PORT2
#define HD63701_PORT3 M6803_PORT3
#define HD63701_PORT4 M6803_PORT4

READ8_HANDLER( hd63701_internal_registers_r );
WRITE8_HANDLER( hd63701_internal_registers_w );


/****************************************************************************
 * For now make the NSC8105 using the m6800 variables and functions
 ****************************************************************************/
#define NSC8105_A					 M6800_A
#define NSC8105_B					 M6800_B
#define NSC8105_PC					 M6800_PC
#define NSC8105_S					 M6800_S
#define NSC8105_X					 M6800_X
#define NSC8105_CC					 M6800_CC
#define NSC8105_WAI_STATE			 M6800_WAI_STATE
#define NSC8105_NMI_STATE			 M6800_NMI_STATE
#define NSC8105_IRQ_STATE			 M6800_IRQ_STATE

#define NSC8105_WAI 				 M6800_WAI
#define NSC8105_IRQ_LINE			 M6800_IRQ_LINE
#define NSC8105_TIN_LINE			 M6800_TIN_LINE

#endif
#endif /* __M6800_H__ */

