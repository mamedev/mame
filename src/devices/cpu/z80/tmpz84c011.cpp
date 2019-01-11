// license:BSD-3-Clause
// copyright-holders:David Haywood,hap
/***************************************************************************

    Toshiba TMPZ84C011, MPUZ80/TLCS-Z80 ASSP Family
    Z80 CPU, CTC, CGC, I/O8x5

    TODO:
    - CGC (clock generator/controller)

***************************************************************************/

#include "emu.h"
#include "tmpz84c011.h"

DEFINE_DEVICE_TYPE(TMPZ84C011, tmpz84c011_device, "tmpz84c011", "Toshiba TMPZ84C011")

void tmpz84c011_device::tmpz84c011_internal_io_map(address_map &map)
{
	map(0x10, 0x13).mirror(0xff00).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));

	map(0x50, 0x50).mirror(0xff00).rw(FUNC(tmpz84c011_device::tmpz84c011_pa_r), FUNC(tmpz84c011_device::tmpz84c011_pa_w));
	map(0x51, 0x51).mirror(0xff00).rw(FUNC(tmpz84c011_device::tmpz84c011_pb_r), FUNC(tmpz84c011_device::tmpz84c011_pb_w));
	map(0x52, 0x52).mirror(0xff00).rw(FUNC(tmpz84c011_device::tmpz84c011_pc_r), FUNC(tmpz84c011_device::tmpz84c011_pc_w));
	map(0x30, 0x30).mirror(0xff00).rw(FUNC(tmpz84c011_device::tmpz84c011_pd_r), FUNC(tmpz84c011_device::tmpz84c011_pd_w));
	map(0x40, 0x40).mirror(0xff00).rw(FUNC(tmpz84c011_device::tmpz84c011_pe_r), FUNC(tmpz84c011_device::tmpz84c011_pe_w));
	map(0x54, 0x54).mirror(0xff00).rw(FUNC(tmpz84c011_device::tmpz84c011_dir_pa_r), FUNC(tmpz84c011_device::tmpz84c011_dir_pa_w));
	map(0x55, 0x55).mirror(0xff00).rw(FUNC(tmpz84c011_device::tmpz84c011_dir_pb_r), FUNC(tmpz84c011_device::tmpz84c011_dir_pb_w));
	map(0x56, 0x56).mirror(0xff00).rw(FUNC(tmpz84c011_device::tmpz84c011_dir_pc_r), FUNC(tmpz84c011_device::tmpz84c011_dir_pc_w));
	map(0x34, 0x34).mirror(0xff00).rw(FUNC(tmpz84c011_device::tmpz84c011_dir_pd_r), FUNC(tmpz84c011_device::tmpz84c011_dir_pd_w));
	map(0x44, 0x44).mirror(0xff00).rw(FUNC(tmpz84c011_device::tmpz84c011_dir_pe_r), FUNC(tmpz84c011_device::tmpz84c011_dir_pe_w));
}


tmpz84c011_device::tmpz84c011_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z80_device(mconfig, TMPZ84C011, tag, owner, clock),
	m_io_space_config( "io", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(tmpz84c011_device::tmpz84c011_internal_io_map), this)),
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

device_memory_interface::space_config_vector tmpz84c011_device::memory_space_config() const
{
	auto r = z80_device::memory_space_config();
	r.back().second = &m_io_space_config;
	return r;
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
void tmpz84c011_device::device_add_mconfig(machine_config &config)
{
	Z80CTC(config, m_ctc, DERIVED_CLOCK(1,1));
	m_ctc->intr_callback().set_inputline(DEVICE_SELF, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(FUNC(tmpz84c011_device::zc0_cb_trampoline_w));
	m_ctc->zc_callback<1>().set(FUNC(tmpz84c011_device::zc1_cb_trampoline_w));
	m_ctc->zc_callback<2>().set(FUNC(tmpz84c011_device::zc2_cb_trampoline_w));
}
