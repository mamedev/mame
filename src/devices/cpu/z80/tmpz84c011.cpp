// license:BSD-3-Clause
// copyright-holders:David Haywood,hap
/***************************************************************************

    Toshiba TMPZ84C011, MPUZ80/TLCS-Z80 ASSP Family
    Z80 CPU, CTC, CGC, I/O8x5

    TODO:
    - CGC (clock generator/controller)

***************************************************************************/

#include "tmpz84c011.h"

const device_type TMPZ84C011 = &device_creator<tmpz84c011_device>;

static ADDRESS_MAP_START( tmpz84c011_internal_io_map, AS_IO, 8, tmpz84c011_device )
	AM_RANGE(0x10, 0x13) AM_MIRROR(0xff00) AM_DEVREADWRITE("tmpz84c011_ctc", z80ctc_device, read, write)

	AM_RANGE(0x50, 0x50) AM_MIRROR(0xff00) AM_READWRITE(tmpz84c011_pa_r, tmpz84c011_pa_w)
	AM_RANGE(0x51, 0x51) AM_MIRROR(0xff00) AM_READWRITE(tmpz84c011_pb_r, tmpz84c011_pb_w)
	AM_RANGE(0x52, 0x52) AM_MIRROR(0xff00) AM_READWRITE(tmpz84c011_pc_r, tmpz84c011_pc_w)
	AM_RANGE(0x30, 0x30) AM_MIRROR(0xff00) AM_READWRITE(tmpz84c011_pd_r, tmpz84c011_pd_w)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0xff00) AM_READWRITE(tmpz84c011_pe_r, tmpz84c011_pe_w)
	AM_RANGE(0x54, 0x54) AM_MIRROR(0xff00) AM_READWRITE(tmpz84c011_dir_pa_r, tmpz84c011_dir_pa_w)
	AM_RANGE(0x55, 0x55) AM_MIRROR(0xff00) AM_READWRITE(tmpz84c011_dir_pb_r, tmpz84c011_dir_pb_w)
	AM_RANGE(0x56, 0x56) AM_MIRROR(0xff00) AM_READWRITE(tmpz84c011_dir_pc_r, tmpz84c011_dir_pc_w)
	AM_RANGE(0x34, 0x34) AM_MIRROR(0xff00) AM_READWRITE(tmpz84c011_dir_pd_r, tmpz84c011_dir_pd_w)
	AM_RANGE(0x44, 0x44) AM_MIRROR(0xff00) AM_READWRITE(tmpz84c011_dir_pe_r, tmpz84c011_dir_pe_w)
ADDRESS_MAP_END


tmpz84c011_device::tmpz84c011_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: z80_device(mconfig, TMPZ84C011, "TMPZ84C011", tag, owner, clock, "tmpz84c011", __FILE__),
	m_io_space_config( "io", ENDIANNESS_LITTLE, 8, 16, 0, ADDRESS_MAP_NAME( tmpz84c011_internal_io_map ) ),
	m_ctc(*this, "tmpz84c011_ctc"),
	m_outportsa(*this),
	m_outportsb(*this),
	m_outportsc(*this),
	m_outportsd(*this),
	m_outportse(*this),
	m_inportsa(*this),
	m_inportsb(*this),
	m_inportsc(*this),
	m_inportsd(*this),
	m_inportse(*this),
	m_zc0_cb(*this),
	m_zc1_cb(*this),
	m_zc2_cb(*this)
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

	m_zc0_cb.resolve_safe();
	m_zc1_cb.resolve_safe();
	m_zc2_cb.resolve_safe();

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

	// initialize I/O
	tmpz84c011_dir_pa_w(*m_io, 0, 0); tmpz84c011_pa_w(*m_io, 0, 0);
	tmpz84c011_dir_pb_w(*m_io, 0, 0); tmpz84c011_pb_w(*m_io, 0, 0);
	tmpz84c011_dir_pc_w(*m_io, 0, 0); tmpz84c011_pc_w(*m_io, 0, 0);
	tmpz84c011_dir_pd_w(*m_io, 0, 0); tmpz84c011_pd_w(*m_io, 0, 0);
	tmpz84c011_dir_pe_w(*m_io, 0, 0); tmpz84c011_pe_w(*m_io, 0, 0);
}


/* CPU interface */
static MACHINE_CONFIG_FRAGMENT( tmpz84c011 )
	MCFG_DEVICE_ADD("tmpz84c011_ctc", Z80CTC, DERIVED_CLOCK(1,1) )
	MCFG_Z80CTC_INTR_CB(INPUTLINE(DEVICE_SELF, INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(tmpz84c011_device, zc0_cb_trampoline_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(tmpz84c011_device, zc1_cb_trampoline_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(tmpz84c011_device, zc2_cb_trampoline_w))
MACHINE_CONFIG_END

machine_config_constructor tmpz84c011_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( tmpz84c011 );
}
