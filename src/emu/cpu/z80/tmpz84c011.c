/***************************************************************************

    Toshiba TMPZ84C011, TLCS-Z80 ASSP Family
    Z80 CPU, CTC, CGC(6/8MHz), I/O8x5
    
    TODO:
    - CGC (clock generator/controller)

***************************************************************************/

#include "tmpz84c011.h"

const device_type TMPZ84C011 = &device_creator<tmpz84c011_device>;

static ADDRESS_MAP_START( tmpz84c011_internal_io_map, AS_IO, 8, tmpz84c011_device )
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("ctc", z80ctc_device, read, write) AM_MIRROR(0xff00)

	AM_RANGE(0x50, 0x50) AM_READWRITE(tmpz84c011_pa_r, tmpz84c011_pa_w) AM_MIRROR(0xff00)
	AM_RANGE(0x51, 0x51) AM_READWRITE(tmpz84c011_pb_r, tmpz84c011_pb_w) AM_MIRROR(0xff00)
	AM_RANGE(0x52, 0x52) AM_READWRITE(tmpz84c011_pc_r, tmpz84c011_pc_w) AM_MIRROR(0xff00)
	AM_RANGE(0x30, 0x30) AM_READWRITE(tmpz84c011_pd_r, tmpz84c011_pd_w) AM_MIRROR(0xff00)
	AM_RANGE(0x40, 0x40) AM_READWRITE(tmpz84c011_pe_r, tmpz84c011_pe_w) AM_MIRROR(0xff00)
	AM_RANGE(0x54, 0x54) AM_READWRITE(tmpz84c011_dir_pa_r, tmpz84c011_dir_pa_w) AM_MIRROR(0xff00)
	AM_RANGE(0x55, 0x55) AM_READWRITE(tmpz84c011_dir_pb_r, tmpz84c011_dir_pb_w) AM_MIRROR(0xff00)
	AM_RANGE(0x56, 0x56) AM_READWRITE(tmpz84c011_dir_pc_r, tmpz84c011_dir_pc_w) AM_MIRROR(0xff00)
	AM_RANGE(0x34, 0x34) AM_READWRITE(tmpz84c011_dir_pd_r, tmpz84c011_dir_pd_w) AM_MIRROR(0xff00)
	AM_RANGE(0x44, 0x44) AM_READWRITE(tmpz84c011_dir_pe_r, tmpz84c011_dir_pe_w) AM_MIRROR(0xff00)
ADDRESS_MAP_END


tmpz84c011_device::tmpz84c011_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80_device(mconfig, TMPZ84C011, "TMPZ84C011", tag, owner, clock, "tmpz84c011", __FILE__),
	m_ctc(*this, "ctc"),
	m_io_space_config( "io", ENDIANNESS_LITTLE, 8, 16, 0, ADDRESS_MAP_NAME( tmpz84c011_internal_io_map ) ),
	m_outportsa(*this),
	m_outportsb(*this),
	m_outportsc(*this),
	m_outportsd(*this),
	m_outportse(*this),
	m_inportsa(*this),
	m_inportsb(*this),
	m_inportsc(*this),
	m_inportsd(*this),
	m_inportse(*this)
{
	memset(m_pio_dir, 0, 5);
	memset(m_pio_latch, 0, 5);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tmpz84c011_device::device_start()
{
	z80_device::device_start();

	// resolve callbacks
	m_outportsa.resolve_safe();
	m_outportsb.resolve_safe();
	m_outportsc.resolve_safe();
	m_outportsd.resolve_safe();
	m_outportse.resolve_safe();

	m_inportsa.resolve_safe(0);
	m_inportsb.resolve_safe(0);
	m_inportsc.resolve_safe(0);
	m_inportsd.resolve_safe(0);
	m_inportse.resolve_safe(0);

	// register for save states
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


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tmpz84c011_device::device_reset()
{
	z80_device::device_reset();

	// initialize TMPZ84C011 PIO
	tmpz84c011_dir_pa_w(*m_io, 0, 0); tmpz84c011_pa_w(*m_io, 0, 0xff);
	tmpz84c011_dir_pb_w(*m_io, 0, 0); tmpz84c011_pb_w(*m_io, 0, 0xff);
	tmpz84c011_dir_pc_w(*m_io, 0, 0); tmpz84c011_pc_w(*m_io, 0, 0xff);
	tmpz84c011_dir_pd_w(*m_io, 0, 0); tmpz84c011_pd_w(*m_io, 0, 0xff);
	tmpz84c011_dir_pe_w(*m_io, 0, 0); tmpz84c011_pe_w(*m_io, 0, 0xff);
}


/* CPU interface */
READ8_MEMBER(tmpz84c011_device::tmpz84c011_pa_r)
{
	return (m_inportsa() & ~m_pio_dir[0]) | (m_pio_latch[0] & m_pio_dir[0]);
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_pb_r)
{
	return (m_inportsb() & ~m_pio_dir[1]) | (m_pio_latch[1] & m_pio_dir[1]);
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_pc_r)
{
	return (m_inportsc() & ~m_pio_dir[2]) | (m_pio_latch[2] & m_pio_dir[2]);
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_pd_r)
{
	return (m_inportsd() & ~m_pio_dir[3]) | (m_pio_latch[3] & m_pio_dir[3]);
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_pe_r)
{
	return (m_inportse() & ~m_pio_dir[4]) | (m_pio_latch[4] & m_pio_dir[4]);
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_pa_w)
{
	m_pio_latch[0] = data;
	m_outportsa(data | ~m_pio_dir[0]);
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_pb_w)
{
	m_pio_latch[1] = data;
	m_outportsb(data | ~m_pio_dir[1]);
}
WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_pc_w)
{
	m_pio_latch[2] = data;
	m_outportsc(data | ~m_pio_dir[2]);
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_pd_w)
{
	m_pio_latch[3] = data;
	m_outportsd(data | ~m_pio_dir[3]);
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_pe_w)
{
	m_pio_latch[4] = data;
	m_outportse(data | ~m_pio_dir[4]);
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_dir_pa_r)
{
	return m_pio_dir[0];
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_dir_pb_r)
{
	return m_pio_dir[1];
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_dir_pc_r)
{
	return m_pio_dir[2];
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_dir_pd_r)
{
	return m_pio_dir[3];
}

READ8_MEMBER(tmpz84c011_device::tmpz84c011_dir_pe_r)
{
	return m_pio_dir[4];
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_dir_pa_w)
{
	m_pio_dir[0] = data;
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_dir_pb_w)
{
	m_pio_dir[1] = data;
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_dir_pc_w)
{
	m_pio_dir[2] = data;
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_dir_pd_w)
{
	m_pio_dir[3] = data;
}

WRITE8_MEMBER(tmpz84c011_device::tmpz84c011_dir_pe_w)
{
	m_pio_dir[4] = data;
}


static MACHINE_CONFIG_FRAGMENT( tmpz84c011 )
	MCFG_DEVICE_ADD("ctc", Z80CTC, DERIVED_CLOCK(1,1) )
	MCFG_Z80CTC_INTR_CB(INPUTLINE(DEVICE_SELF, INPUT_LINE_IRQ0))
MACHINE_CONFIG_END

machine_config_constructor tmpz84c011_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( tmpz84c011 );
}
