/***************************************************************************

        MC-80.xx by Miodrag Milanovic

        15/05/2009 Initial implementation
        12/05/2009 Skeleton driver.

****************************************************************************/

#include "includes/mc80.h"

/*****************************************************************************/
/*                            Implementation for MC80.2x                     */
/*****************************************************************************/

static IRQ_CALLBACK( mc8020_irq_callback )
{
	return 0x00;
}

MACHINE_RESET_MEMBER(mc80_state,mc8020)
{
	machine().device("maincpu")->execute().set_irq_acknowledge_callback(mc8020_irq_callback);
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

Z80CTC_INTERFACE( mc8020_ctc_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_DRIVER_LINE_MEMBER(mc80_state, ctc_z0_w),
	DEVCB_DRIVER_LINE_MEMBER(mc80_state, ctc_z1_w),
	DEVCB_DRIVER_LINE_MEMBER(mc80_state,ctc_z2_w)
};


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

Z80PIO_INTERFACE( mc8020_z80pio_intf )
{
	DEVCB_NULL,	/* callback when change interrupt status */
	DEVCB_DRIVER_MEMBER(mc80_state, mc80_port_a_r),
	DEVCB_DRIVER_MEMBER(mc80_state, mc80_port_a_w),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(mc80_state, mc80_port_b_r),
	DEVCB_DRIVER_MEMBER(mc80_state, mc80_port_b_w),
	DEVCB_NULL
};

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

static IRQ_CALLBACK( mc8030_irq_callback )
{
	return 0x20;
}

MACHINE_RESET_MEMBER(mc80_state,mc8030)
{
	machine().device("maincpu")->execute().set_irq_acknowledge_callback(mc8030_irq_callback);
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

Z80PIO_INTERFACE( mc8030_zve_z80pio_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),	/* callback when change interrupt status */
	DEVCB_DRIVER_MEMBER(mc80_state, zve_port_a_r),
	DEVCB_DRIVER_MEMBER(mc80_state, zve_port_a_w),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(mc80_state, zve_port_b_r),
	DEVCB_DRIVER_MEMBER(mc80_state, zve_port_b_w),
	DEVCB_NULL
};

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

Z80PIO_INTERFACE( mc8030_asp_z80pio_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),	/* callback when change interrupt status */
	DEVCB_DRIVER_MEMBER(mc80_state, asp_port_a_r),
	DEVCB_DRIVER_MEMBER(mc80_state, asp_port_a_w),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(mc80_state, asp_port_b_r),
	DEVCB_DRIVER_MEMBER(mc80_state, asp_port_b_w),
	DEVCB_NULL
};

Z80CTC_INTERFACE( mc8030_zve_z80ctc_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_NULL,	// for user
	DEVCB_NULL,	// for user
	DEVCB_NULL	// for user
};

Z80CTC_INTERFACE( mc8030_asp_z80ctc_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_NULL,	// to SIO CLK CH A
	DEVCB_NULL,	// to SIO CLK CH B
	DEVCB_NULL	// KMBG (??)
};

// SIO CH A in = keyboard; out = beeper; CH B = IFSS (??)
const z80sio_interface mc8030_asp_z80sio_intf =
{
	DEVCB_NULL,	/* interrupt handler */
	DEVCB_NULL,			/* DTR changed handler */
	DEVCB_NULL,			/* RTS changed handler */
	DEVCB_NULL,			/* BREAK changed handler */
	DEVCB_NULL,			/* transmit handler */
	DEVCB_NULL			/* receive handler */
};

