
#include "tmpz84c011.h"

// how do we actually install default handlers for logging?
/*
READ8_MEMBER(tmpz84c011_device::porta_default_r) { logerror("%s read port A but no handler assigned\n", machine().describe_context()); return 0xff; }
READ8_MEMBER(tmpz84c011_device::portb_default_r) { logerror("%s read port B but no handler assigned\n", machine().describe_context()); return 0xff; }
READ8_MEMBER(tmpz84c011_device::portc_default_r) { logerror("%s read port C but no handler assigned\n", machine().describe_context()); return 0xff; }
READ8_MEMBER(tmpz84c011_device::portd_default_r) { logerror("%s read port D but no handler assigned\n", machine().describe_context()); return 0xff; }
READ8_MEMBER(tmpz84c011_device::porte_default_r) { logerror("%s read port E but no handler assigned\n", machine().describe_context()); return 0xff; }

WRITE8_MEMBER(tmpz84c011_device::porta_default_w) { logerror("%s write %02x to port A but no handler assigned\n", machine().describe_context(), data); }
WRITE8_MEMBER(tmpz84c011_device::portb_default_w) { logerror("%s write %02x to port B but no handler assigned\n", machine().describe_context(), data); }
WRITE8_MEMBER(tmpz84c011_device::portc_default_w) { logerror("%s write %02x to port C but no handler assigned\n", machine().describe_context(), data); }
WRITE8_MEMBER(tmpz84c011_device::portd_default_w) { logerror("%s write %02x to port D but no handler assigned\n", machine().describe_context(), data); }
WRITE8_MEMBER(tmpz84c011_device::porte_default_w) { logerror("%s write %02x to port E but no handler assigned\n", machine().describe_context(), data); }
*/

READ8_MEMBER(tmpz84c011_device::tmpz84c011_pio_r)
{
	int portdata = 0xff;

	switch (offset)
	{
		case 0:         /* PA_0 */
			portdata = m_inports0();
			break;
		case 1:         /* PB_0 */
			portdata = m_inports1();
			break;
		case 2:         /* PC_0 */
			portdata = m_inports2();
			break;
		case 3:         /* PD_0 */
			portdata = m_inports3();
			break;
		case 4:         /* PE_0 */
			portdata = m_inports4();
			break;
	}

	return portdata;
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_pio_w)
{
	switch (offset)
	{
		case 0:         /* PA_0 */
			m_outports0(data);
			break;
		case 1:         /* PB_0 */
			m_outports1(data);
			break;
		case 2:         /* PC_0 */
			m_outports2(data);
			break;
		case 3:         /* PD_0 */
			m_outports3(data);
			break;
		case 4:         /* PE_0 */
			m_outports4(data);
			break;
	}
}

/* CPU interface */
READ8_MEMBER(tmpz84c011_device::tmpz84c011_0_pa_r)
{
	return (tmpz84c011_pio_r(space,0) & ~m_pio_dir[0]) | (m_pio_latch[0] & m_pio_dir[0]);
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_0_pb_r)
{
	return (tmpz84c011_pio_r(space,1) & ~m_pio_dir[1]) | (m_pio_latch[1] & m_pio_dir[1]);
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_0_pc_r)
{
	return (tmpz84c011_pio_r(space,2) & ~m_pio_dir[2]) | (m_pio_latch[2] & m_pio_dir[2]);
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_0_pd_r)
{
	return (tmpz84c011_pio_r(space,3) & ~m_pio_dir[3]) | (m_pio_latch[3] & m_pio_dir[3]);
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_0_pe_r)
{
	return (tmpz84c011_pio_r(space,4) & ~m_pio_dir[4]) | (m_pio_latch[4] & m_pio_dir[4]);
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_0_pa_w)
{
	m_pio_latch[0] = data;
	tmpz84c011_pio_w(space, 0, data);
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_0_pb_w)
{
	m_pio_latch[1] = data;
	tmpz84c011_pio_w(space, 1, data);
}
WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_0_pc_w)
{
	m_pio_latch[2] = data;
	tmpz84c011_pio_w(space, 2, data);
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_0_pd_w)
{
	m_pio_latch[3] = data;
	tmpz84c011_pio_w(space, 3, data);
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_0_pe_w)
{
	m_pio_latch[4] = data;
	tmpz84c011_pio_w(space, 4, data);
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_0_dir_pa_r)
{
	return m_pio_dir[0];
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_0_dir_pb_r)
{
	return m_pio_dir[1];
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_0_dir_pc_r)
{
	return m_pio_dir[2];
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_0_dir_pd_r)
{
	return m_pio_dir[3];
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_0_dir_pe_r)
{
	return m_pio_dir[4];
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_0_dir_pa_w)
{
	m_pio_dir[0] = data;
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_0_dir_pb_w)
{
	m_pio_dir[1] = data;
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_0_dir_pc_w)
{
	m_pio_dir[2] = data;
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_0_dir_pd_w)
{
	m_pio_dir[3] = data;
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_0_dir_pe_w)
{
	m_pio_dir[4] = data;
}



static ADDRESS_MAP_START( tmpz84c011_internal_io_map, AS_IO, 8, tmpz84c011_device )
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("ctc", z80ctc_device, read, write) AM_MIRROR(0xff00)

	AM_RANGE(0x50, 0x50) AM_READWRITE(tmpz84c011_0_pa_r, tmpz84c011_0_pa_w) AM_MIRROR(0xff00)
	AM_RANGE(0x51, 0x51) AM_READWRITE(tmpz84c011_0_pb_r, tmpz84c011_0_pb_w) AM_MIRROR(0xff00)
	AM_RANGE(0x52, 0x52) AM_READWRITE(tmpz84c011_0_pc_r, tmpz84c011_0_pc_w) AM_MIRROR(0xff00)
	AM_RANGE(0x30, 0x30) AM_READWRITE(tmpz84c011_0_pd_r, tmpz84c011_0_pd_w) AM_MIRROR(0xff00)
	AM_RANGE(0x40, 0x40) AM_READWRITE(tmpz84c011_0_pe_r, tmpz84c011_0_pe_w) AM_MIRROR(0xff00)
	AM_RANGE(0x54, 0x54) AM_READWRITE(tmpz84c011_0_dir_pa_r, tmpz84c011_0_dir_pa_w) AM_MIRROR(0xff00)
	AM_RANGE(0x55, 0x55) AM_READWRITE(tmpz84c011_0_dir_pb_r, tmpz84c011_0_dir_pb_w) AM_MIRROR(0xff00)
	AM_RANGE(0x56, 0x56) AM_READWRITE(tmpz84c011_0_dir_pc_r, tmpz84c011_0_dir_pc_w) AM_MIRROR(0xff00)
	AM_RANGE(0x34, 0x34) AM_READWRITE(tmpz84c011_0_dir_pd_r, tmpz84c011_0_dir_pd_w) AM_MIRROR(0xff00)
	AM_RANGE(0x44, 0x44) AM_READWRITE(tmpz84c011_0_dir_pe_r, tmpz84c011_0_dir_pe_w) AM_MIRROR(0xff00)
ADDRESS_MAP_END


tmpz84c011_device::tmpz84c011_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80_device(mconfig, TMPZ84C011, "TMPZ84C011", tag, owner, clock, "tmpz84c011", __FILE__),
	m_io_space_config( "io", ENDIANNESS_LITTLE, 8, 16, 0, ADDRESS_MAP_NAME( tmpz84c011_internal_io_map ) ),
	m_outports0(*this),
	m_outports1(*this),
	m_outports2(*this),
	m_outports3(*this),
	m_outports4(*this),
	m_inports0(*this),
	m_inports1(*this),
	m_inports2(*this),
	m_inports3(*this),
	m_inports4(*this),
	m_intr_cb(*this),
	m_zc0_cb(*this),
	m_zc1_cb(*this),
	m_zc2_cb(*this)
{
}

WRITE_LINE_MEMBER( tmpz84c011_device::intr_cb_trampoline_w ) { m_intr_cb(state); }
WRITE_LINE_MEMBER( tmpz84c011_device::zc0_cb_trampoline_w ) { m_zc0_cb(state); }
WRITE_LINE_MEMBER( tmpz84c011_device::zc1_cb_trampoline_w ) { m_zc1_cb(state); }
WRITE_LINE_MEMBER( tmpz84c011_device::zc2_cb_trampoline_w ) { m_zc2_cb(state); }



const device_type TMPZ84C011 = &device_creator<tmpz84c011_device>;

static MACHINE_CONFIG_FRAGMENT( tmpz84c011 )
	MCFG_DEVICE_ADD("ctc", Z80CTC, DERIVED_CLOCK(1,1) )
	MCFG_Z80CTC_INTR_CB(WRITELINE(tmpz84c011_device, intr_cb_trampoline_w))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(tmpz84c011_device, zc0_cb_trampoline_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(tmpz84c011_device, zc1_cb_trampoline_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(tmpz84c011_device, zc2_cb_trampoline_w))


MACHINE_CONFIG_END

machine_config_constructor tmpz84c011_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( tmpz84c011 );
}


void tmpz84c011_device::device_start()
{
	z80_device::device_start();

	m_outports0.resolve_safe();
	m_outports1.resolve_safe();
	m_outports2.resolve_safe();
	m_outports3.resolve_safe();
	m_outports4.resolve_safe();

	m_inports0.resolve_safe(0);
	m_inports1.resolve_safe(0);
	m_inports2.resolve_safe(0);
	m_inports3.resolve_safe(0);
	m_inports4.resolve_safe(0);

	m_intr_cb.resolve_safe();
	m_zc0_cb.resolve_safe();
	m_zc1_cb.resolve_safe();
	m_zc2_cb.resolve_safe();

	save_item(NAME(m_pio_dir[0]));
	save_item(NAME(m_pio_latch[0]));
	save_item(NAME(m_pio_dir[1]));
	save_item(NAME(m_pio_latch[1]));
	save_item(NAME(m_pio_dir[2]));
	save_item(NAME(m_pio_latch[2]));
	save_item(NAME(m_pio_dir[3]));
	save_item(NAME(m_pio_latch[3]));
	save_item(NAME(m_pio_dir[4]));
	save_item(NAME(m_pio_latch[4]));


}

void tmpz84c011_device::device_reset()
{
	z80_device::device_reset();

	// initialize TMPZ84C011 PIO
	for (int i = 0; i < 5; i++)
	{
		m_pio_dir[i] = m_pio_latch[i] = 0;
		tmpz84c011_pio_w(*m_io, i, 0);
	}
}


