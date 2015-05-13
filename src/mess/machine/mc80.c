// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        MC-80.xx by Miodrag Milanovic

        15/05/2009 Initial implementation
        12/05/2009 Skeleton driver.

****************************************************************************/

#include "includes/mc80.h"

/*****************************************************************************/
/*                            Implementation for MC80.2x                     */
/*****************************************************************************/

IRQ_CALLBACK_MEMBER(mc80_state::mc8020_irq_callback)
{
	return 0x00;
}

MACHINE_RESET_MEMBER(mc80_state,mc8020)
{
}

WRITE_LINE_MEMBER( mc80_state::ctc_z0_w )
{
}

WRITE_LINE_MEMBER( mc80_state::ctc_z1_w )
{
}

WRITE_LINE_MEMBER(mc80_state::ctc_z2_w)
{
	downcast<z80ctc_device *>(machine().device("z80ctc"))->trg0(state);
	downcast<z80ctc_device *>(machine().device("z80ctc"))->trg1(state);
}

READ8_MEMBER( mc80_state::mc80_port_b_r )
{
	return 0;
}

READ8_MEMBER( mc80_state::mc80_port_a_r )
{
	return 0;
}

WRITE8_MEMBER( mc80_state::mc80_port_a_w )
{
}

WRITE8_MEMBER( mc80_state::mc80_port_b_w )
{
}

/*****************************************************************************/
/*                            Implementation for MC80.3x                     */
/*****************************************************************************/

WRITE8_MEMBER( mc80_state::mc8030_zve_write_protect_w )
{
}

WRITE8_MEMBER( mc80_state::mc8030_vis_w )
{
	// reg C
	// 7 6 5 4 -- module
	//         3 - 0 left half, 1 right half
	//           2 1 0
	//           =====
	//           0 0 0 - dark
	//           0 0 1 - light
	//           0 1 0 - in reg pixel
	//           0 1 1 - negate in reg pixel
	//           1 0 x - operation code in B reg
	// reg B
	//
	UINT16 addr = ((offset & 0xff00) >> 2) | ((offset & 0x08) << 2) | (data >> 3);
	static const UINT8 val[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
	int c = offset & 1;
	m_p_videoram[addr] = m_p_videoram[addr] | (val[data & 7]*c);
}

WRITE8_MEMBER( mc80_state::mc8030_eprom_prog_w )
{
}

IRQ_CALLBACK_MEMBER(mc80_state::mc8030_irq_callback )
{
	return 0x20;
}

MACHINE_RESET_MEMBER(mc80_state,mc8030)
{
}

READ8_MEMBER( mc80_state::zve_port_a_r )
{
	return 0xff;
}

READ8_MEMBER( mc80_state::zve_port_b_r )
{
	return 0xff;
}

WRITE8_MEMBER( mc80_state::zve_port_a_w )
{
}

WRITE8_MEMBER( mc80_state::zve_port_b_w )
{
}

READ8_MEMBER( mc80_state::asp_port_a_r )
{
	return 0xff;
}

READ8_MEMBER( mc80_state::asp_port_b_r )
{
	return 0xff;
}

WRITE8_MEMBER( mc80_state::asp_port_a_w )
{
}

WRITE8_MEMBER( mc80_state::asp_port_b_w )
{
}
