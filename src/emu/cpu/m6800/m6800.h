/*** m6800: Portable 6800 class emulator *************************************/

#pragma once

#ifndef __M6800_H__
#define __M6800_H__


struct m6801_interface
{
	devcb_write_line		out_sc2_func;
};
#define M6801_INTERFACE(name) const m6801_interface (name) =


enum
{
	M6800_PC=1, M6800_S, M6800_A, M6800_B, M6800_X, M6800_CC,
	M6800_WAI_STATE
};

enum
{
	M6800_IRQ_LINE = 0,				/* IRQ line number */
	M6801_TIN_LINE,					/* P20/Tin Input Capture line (eddge sense)     */
									/* Active eddge is selecrable by internal reg.  */
									/* raise eddge : CLEAR_LINE  -> ASSERT_LINE     */
									/* fall  eddge : ASSERT_LINE -> CLEAR_LINE      */
									/* it is usuali to use PULSE_LINE state         */
	M6801_SC1_LINE
};

enum
{
	M6801_MODE_0 = 0,
	M6801_MODE_1,
	M6801_MODE_2,
	M6801_MODE_3,
	M6801_MODE_4,
	M6801_MODE_5,
	M6801_MODE_6,
	M6801_MODE_7
};

enum
{
	M6801_PORT1 = 0x100,
	M6801_PORT2,
	M6801_PORT3,
	M6801_PORT4
};

DECLARE_LEGACY_CPU_DEVICE(M6800, m6800);
DECLARE_LEGACY_CPU_DEVICE(M6801, m6801);
DECLARE_LEGACY_CPU_DEVICE(M6802, m6802);
DECLARE_LEGACY_CPU_DEVICE(M6803, m6803);
DECLARE_LEGACY_CPU_DEVICE(M6808, m6808);
DECLARE_LEGACY_CPU_DEVICE(HD6301, hd6301);
DECLARE_LEGACY_CPU_DEVICE(HD63701, hd63701);
DECLARE_LEGACY_CPU_DEVICE(NSC8105, nsc8105);

DECLARE_LEGACY_CPU_DEVICE(HD6303R, hd6303r);
DECLARE_LEGACY_CPU_DEVICE(HD6303Y, hd6303y);


DECLARE_READ8_HANDLER( m6801_io_r );
DECLARE_WRITE8_HANDLER( m6801_io_w );

CPU_DISASSEMBLE( m6800 );
CPU_DISASSEMBLE( m6801 );
CPU_DISASSEMBLE( m6802 );
CPU_DISASSEMBLE( m6803 );
CPU_DISASSEMBLE( m6808 );
CPU_DISASSEMBLE( hd6301 );
CPU_DISASSEMBLE( hd63701 );
CPU_DISASSEMBLE( nsc8105 );

#endif /* __M6800_H__ */

