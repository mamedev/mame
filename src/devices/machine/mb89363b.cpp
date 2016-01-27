// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
  (this acts as a trampoline to 2x i8255 chips)


    Manufacturer: Fujitsu
    Part Number: MB89363 / MB89363B / MB89363R
    Package: Surface Mount QFP80 / QFP64P (MB89363R)
    Description: 8-bit x 3 x 2 (6 x 8-bit) parallel data I/O port VLSI chip
                 Parallel Communication Interface
                 Extended I/O

    Note: MB89363B is compatible with 8255

    Pin Assignment:
                                          +5v
                         P P P P P P P P P V   P P P P P P P P P
                     N N 5 4 4 4 4 4 4 4 4 C N 1 1 1 1 1 1 1 1 2 N N
                     C C 3 0 1 2 3 4 5 6 7 C C 7 6 5 4 3 2 1 0 3 C C

                     | | ^ ^ ^ ^ ^ ^ ^ ^ ^ | | ^ ^ ^ ^ ^ ^ ^ ^ ^ | |
                     | | | | | | | | | | | | | | | | | | | | | | | |
                     | | v v v v v v v v v | | v v v v v v v v v | |
                .-------------------------------------------------------.
                |    6 6 6 6 6 5 5 5 5 5 5 5 5 5 5 4 4 4 4 4 4 4 4 4    |
                |    4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1    |
      P52   <-> | 65                                                 40 | <->    P22
      P51   <-> | 66                                                 39 | <->    P21
      P50   <-> | 67                                                 38 | <->    P20
      P54   <-> | 68                                                 37 | <->    P24
      P55   <-> | 69                                                 36 | <->    P25
      P56   <-> | 70                                                 35 | <->    P26
      P57   <-> | 71                                                 34 | <->    P27
       NC   --- | 72                   MB89363B                      33 | ---    NC
       NC   --- | 73                                                 32 | <--    RSLCT1
      GND   --> | 74                                                 31 | <--    RSLCT0
      CS2   --> | 75                                                 30 | <--    GND
        R   --> | 76                                                 29 | <--    CS1
      P30   <-> | 77                                                 28 | <->    P00
      P31   <-> | 78                                                 27 | <->    P01
      P32   <-> | 79                                                 26 | <->    P02
      P33   <-> | 80                                                 25 | <->    P03
                 \                     1 1 1 1 1 1 1 1 1 1 2 2 2 2 2    |
                  \  1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4    |
                   -----------------------------------------------------'
                     ^ ^ ^ ^ ^ ^ | | ^ | ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ |
                     | | | | | | | | | | | | | | | | | | | | | | | |
                     v v v v | | | | | | | v v v v v v v v v v v v |

                     P P P P W R N N R N O D D D D D D D D P P P P N
                     3 3 3 3   S C C H C U B B B B B B B B 0 0 0 0 C
                     4 5 6 7   T     /   S 0 1 2 3 4 5 6 7 7 6 5 4
                                     R   /
                                     L   I
                                         N
                                         S

    Block Diagram / Pin Descriptions:
    http://www.mess.org/_media/datasheets/fujitsu/mb89363b_partial.pdf

    D.C. Characteristics:
    (Recommended operating conditions unless otherwise noted)
    (VCC = +5V +- 10%, GND = 0V, TA = -40o C to 85o C)
                                         Value
    Parameter            Symbol      Min       Max            Unit      Test Condition
    ----------------------------------------------------------------------------------
    Input Low Voltage    ViL         -0.3      0.8            V
    Input High Voltage   ViH         2.2       VCC +0.3       V
    Output Low Voltage   VoL         -         0.4            V         IoL = 2.5mA
    Output High Voltage  VoH         3.0       -              V         IoH =-2.5mA

    Sources:
    http://www.emb-tech.co.jp/pc104/96dio.pdf
    http://www.pb5800.com/resources/2350ser01.pdf
    http://www.diagramasde.com/diagramas/otros2/TS-850S%20Service%20Manual%20.pdf
*/

#include "emu.h"
#include "machine/mb89363b.h"



extern const device_type MB89363B = &device_creator<mb89363b_device>;


mb89363b_device::mb89363b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MB89363B, "MB89363 I/O", tag, owner, clock, "mb89363b", __FILE__),
		m_i8255_a(*this, "i8255_a"),
		m_i8255_b(*this, "i8255_b"),
		m_in_a_pa_cb(*this),
		m_in_a_pb_cb(*this),
		m_in_a_pc_cb(*this),
		m_out_a_pa_cb(*this),
		m_out_a_pb_cb(*this),
		m_out_a_pc_cb(*this),
		m_in_b_pa_cb(*this),
		m_in_b_pb_cb(*this),
		m_in_b_pc_cb(*this),
		m_out_b_pa_cb(*this),
		m_out_b_pb_cb(*this),
		m_out_b_pc_cb(*this)
{
}


READ8_MEMBER(mb89363b_device::i8255_a_port_a_r) { return m_in_a_pa_cb(space, offset); }
READ8_MEMBER(mb89363b_device::i8255_a_port_b_r) { return m_in_a_pb_cb(space, offset); }
READ8_MEMBER(mb89363b_device::i8255_a_port_c_r) { return m_in_a_pc_cb(space, offset); }
WRITE8_MEMBER(mb89363b_device::i8255_a_port_a_w) { m_out_a_pa_cb(space, offset, data); }
WRITE8_MEMBER(mb89363b_device::i8255_a_port_b_w) { m_out_a_pb_cb(space, offset, data); }
WRITE8_MEMBER(mb89363b_device::i8255_a_port_c_w) { m_out_a_pc_cb(space, offset, data); }
READ8_MEMBER(mb89363b_device::i8255_b_port_a_r) { return m_in_b_pa_cb(space, offset); }
READ8_MEMBER(mb89363b_device::i8255_b_port_b_r) { return m_in_b_pb_cb(space, offset); }
READ8_MEMBER(mb89363b_device::i8255_b_port_c_r) { return m_in_b_pc_cb(space, offset); }
WRITE8_MEMBER(mb89363b_device::i8255_b_port_a_w) { m_out_b_pa_cb(space, offset, data); }
WRITE8_MEMBER(mb89363b_device::i8255_b_port_b_w) { m_out_b_pb_cb(space, offset, data); }
WRITE8_MEMBER(mb89363b_device::i8255_b_port_c_w) { m_out_b_pc_cb(space, offset, data); }


READ8_MEMBER( mb89363b_device::read )
{
	if (offset & 4)
		return m_i8255_b->read(space, offset & 3);
	else
		return m_i8255_a->read(space, offset & 3);
}

WRITE8_MEMBER( mb89363b_device::write )
{
	if (offset & 4)
		m_i8255_b->write(space, offset & 3, data);
	else
		m_i8255_a->write(space, offset & 3, data);
}


static MACHINE_CONFIG_FRAGMENT( mb89363b )
	MCFG_DEVICE_ADD("i8255_a", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(mb89363b_device, i8255_a_port_a_r))
	MCFG_I8255_IN_PORTB_CB(READ8(mb89363b_device, i8255_a_port_b_r))
	MCFG_I8255_IN_PORTC_CB(READ8(mb89363b_device, i8255_a_port_c_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(mb89363b_device, i8255_a_port_a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(mb89363b_device, i8255_a_port_b_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(mb89363b_device, i8255_a_port_c_w))

	MCFG_DEVICE_ADD("i8255_b", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(mb89363b_device, i8255_b_port_a_r))
	MCFG_I8255_IN_PORTB_CB(READ8(mb89363b_device, i8255_b_port_b_r))
	MCFG_I8255_IN_PORTC_CB(READ8(mb89363b_device, i8255_b_port_c_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(mb89363b_device, i8255_b_port_a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(mb89363b_device, i8255_b_port_b_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(mb89363b_device, i8255_b_port_c_w))
MACHINE_CONFIG_END

machine_config_constructor mb89363b_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mb89363b );
}

void mb89363b_device::device_start()
{
	m_in_a_pa_cb.resolve_safe(0xff);
	m_in_a_pb_cb.resolve_safe(0xff);
	m_in_a_pc_cb.resolve_safe(0xff);
	m_out_a_pa_cb.resolve_safe();
	m_out_a_pb_cb.resolve_safe();
	m_out_a_pc_cb.resolve_safe();

	m_in_b_pa_cb.resolve_safe(0xff);
	m_in_b_pb_cb.resolve_safe(0xff);
	m_in_b_pc_cb.resolve_safe(0xff);
	m_out_b_pa_cb.resolve_safe();
	m_out_b_pb_cb.resolve_safe();
	m_out_b_pc_cb.resolve_safe();

}

void mb89363b_device::device_reset()
{
}
