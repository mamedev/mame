// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Angelo Salese
/**************************************************************************************************

Hudson/NEC HuC6272 "King" device

TODO:
- Convert base mapping to address_map;
- Convert I/O to space address, and make it honor mem_mask;
- CD-ROM DRIVE:FX is bus/nscsi/pcfxcd.cpp (KING is SCSI ID 7, the drive ID 2);
  \- Derivative design of PCE drive, which in turn is a derivative of PC-8801-30 (cd drive)
     and PC-8801-31 (interface);
- BG drawing is done a line at a time, mid-line register changes are not reflected;
- Microprogram only selects which CG/BAT words are fetched, no cycle timing;
- Rotation mode ignores the microprogram;
- Extended dot modes (64K/16M "extdot") are not drawn;
- (check Photo CD, Audio CD & backup RAM screens);
- Verify ADPCM transfers;
- Verify CD-DA hookup;

Notes:
- save pcfx_king.dmp,0:huc6272:kram,0x80000 to dump KRAM contents

ADPCM related patents:
- https://patents.google.com/patent/US5692099
- https://patents.google.com/patent/US6453286
- https://patents.google.com/patent/US5548655A

**************************************************************************************************/

#include "emu.h"
#include "huc6272.h"

#include "bus/nscsi/devices.h"
#include "bus/nscsi/pcfxcd.h"

#include <algorithm>

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(HUC6272, huc6272_device, "huc6272", "Hudson HuC6272 \"King\"")

huc6272_device::huc6272_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HUC6272, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_mixer_interface(mconfig, *this)
	, m_huc6271(*this, finder_base::DUMMY_TAG)
	, m_register(0)
	, m_kram_addr_r(0)
	, m_kram_addr_w(0)
	, m_kram_inc_r(0)
	, m_kram_inc_w(0)
	, m_kram_page_r(0)
	, m_kram_page_w(0)
	, m_kram_load_reg(0)
	, m_kram_write_reg(0)
	, m_page_setting(0)
	, m_kram_mode(0)
	, m_program_space_config("microprg", ENDIANNESS_LITTLE, 16, 4, -1, address_map_constructor(FUNC(huc6272_device::microprg_map), this))
	, m_data_space_config("kram", ENDIANNESS_LITTLE, 16, 19, -1, address_map_constructor(FUNC(huc6272_device::kram_map), this))
	, m_io_space_config("io", ENDIANNESS_LITTLE, 32, 7, -2, address_map_constructor(FUNC(huc6272_device::io_map), this))
	, m_microprg_ram(*this, "microprg_ram")
	, m_kram_page(*this, "kram_page%u", 0U)
	, m_scsibus(*this, "scsi")
	, m_scsicb(*this, "scsi:7:scsicb")
	, m_irq_changed_cb(*this)
	, m_irq_state(false)
{
}

void huc6272_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:2").option_set("cdrom", NSCSI_CDROM_PCFX).machine_config(
		[this](device_t *device)
		{
			// the drive's audio passes through the KING mixer (this device's two outputs)
			device->subdevice<cdda_device>("cdda")->add_route(0, "^^", 1.0, 0);
			device->subdevice<cdda_device>("cdda")->add_route(1, "^^", 1.0, 1);
			// sub-code goes to KING directly, not over SCSI
			downcast<nscsi_cdrom_pcfx_device &>(*device).subcode_cb().set(*this, FUNC(huc6272_device::scsi_subcode_w));
		});
	NSCSI_CONNECTOR(config, "scsi:7", default_scsi_devices, "scsicb", true)
		.option_add_internal("scsicb", NSCSI_CB)
		.machine_config([this](device_t *device)
		{
			// the callback device reports every line on any change; scsi_lines_changed() edge-detects
			nscsi_callback_device &cb = downcast<nscsi_callback_device &>(*device);
			cb.rst_callback().set(*this, FUNC(huc6272_device::scsi_line_w));
			cb.bsy_callback().set(*this, FUNC(huc6272_device::scsi_line_w));
			cb.req_callback().set(*this, FUNC(huc6272_device::scsi_req_edge_w));
			cb.msg_callback().set(*this, FUNC(huc6272_device::scsi_line_w));
			cb.cd_callback().set(*this, FUNC(huc6272_device::scsi_line_w));
			cb.io_callback().set(*this, FUNC(huc6272_device::scsi_line_w));
			cb.sel_callback().set(*this, FUNC(huc6272_device::scsi_line_w));
		});
}

void huc6272_device::device_start()
{
	m_scsi_line_timer = timer_alloc(FUNC(huc6272_device::scsi_lines_changed), this);

	save_item(NAME(m_register));
	save_item(NAME(m_kram_addr_r));
	save_item(NAME(m_kram_inc_r));
	save_item(NAME(m_kram_page_r));
	save_item(NAME(m_kram_addr_w));
	save_item(NAME(m_kram_inc_w));
	save_item(NAME(m_kram_page_w));
	save_item(NAME(m_kram_load_reg));
	save_item(NAME(m_kram_write_reg));
	save_item(NAME(m_page_setting));
	save_item(NAME(m_kram_mode));
	save_item(NAME(m_irq_state));

	save_item(STRUCT_MEMBER(m_scsi, data));
	save_item(STRUCT_MEMBER(m_scsi, ctrl));
	save_item(STRUCT_MEMBER(m_scsi, mode));
	save_item(STRUCT_MEMBER(m_scsi, phase));
	save_item(STRUCT_MEMBER(m_scsi, data_cache));
	save_item(STRUCT_MEMBER(m_scsi, ack));
	save_item(STRUCT_MEMBER(m_scsi, drq));
	save_item(STRUCT_MEMBER(m_scsi, dma_receive));
	save_item(STRUCT_MEMBER(m_scsi, dma_send));
	save_item(STRUCT_MEMBER(m_scsi, irq));
	save_item(STRUCT_MEMBER(m_scsi, subchannel_ctrl));
	save_item(STRUCT_MEMBER(m_scsi, subchannel_data));
	save_item(STRUCT_MEMBER(m_scsi, subchannel_irq));
	save_item(STRUCT_MEMBER(m_scsi, bus));
	save_item(STRUCT_MEMBER(m_scsi, req_last));
	save_item(STRUCT_MEMBER(m_scsi, req_rose));
	save_item(STRUCT_MEMBER(m_dma, addr));
	save_item(STRUCT_MEMBER(m_dma, size));
	save_item(STRUCT_MEMBER(m_dma, ctrl));
	save_item(STRUCT_MEMBER(m_dma, latch));
	save_item(STRUCT_MEMBER(m_dma, flipflop));
	save_item(STRUCT_MEMBER(m_dma, irq));

	save_item(STRUCT_MEMBER(m_bg, bat));
	save_item(STRUCT_MEMBER(m_bg, cg));
	save_item(STRUCT_MEMBER(m_bg, size));
	save_item(STRUCT_MEMBER(m_bg, xscroll));
	save_item(STRUCT_MEMBER(m_bg, yscroll));
	save_item(NAME(m_bg_mode));
	save_item(NAME(m_bg_priority));
	save_item(NAME(m_bg_scroll_mode));
	save_item(NAME(m_bg0sub_bat));
	save_item(NAME(m_bg0sub_cg));
	save_item(NAME(m_bg_affine));

	save_item(STRUCT_MEMBER(m_rainbow, control));
	save_item(STRUCT_MEMBER(m_rainbow, address));
	save_item(STRUCT_MEMBER(m_rainbow, start_line));
	save_item(STRUCT_MEMBER(m_rainbow, block_count));
	save_item(STRUCT_MEMBER(m_rainbow, raster_irq_line));
	save_item(STRUCT_MEMBER(m_rainbow, read_pos));
	save_item(STRUCT_MEMBER(m_rainbow, busy));
	save_item(STRUCT_MEMBER(m_rainbow, blocks));
	save_item(STRUCT_MEMBER(m_rainbow, start_pending));
	save_item(STRUCT_MEMBER(m_rainbow, raster_irq));

	save_item(NAME(m_micro_prg.index));
	save_item(NAME(m_micro_prg.ctrl));

	save_item(NAME(m_adpcm.rate));
	save_item(NAME(m_adpcm.status));
	save_item(NAME(m_adpcm.interrupt));
	save_item(NAME(m_adpcm.divider));
	save_item(NAME(m_adpcm.tick));
	for (int adpcm = 0; adpcm < 2; adpcm++)
	{
		save_item(NAME(m_adpcm.playing[adpcm]), adpcm);
		save_item(NAME(m_adpcm.control[adpcm]), adpcm);
		save_item(NAME(m_adpcm.start[adpcm]), adpcm);
		save_item(NAME(m_adpcm.end[adpcm]), adpcm);
		save_item(NAME(m_adpcm.imm[adpcm]), adpcm);
		save_item(NAME(m_adpcm.halfword[adpcm]), adpcm);
		save_item(NAME(m_adpcm.have_halfword[adpcm]), adpcm);
		save_item(NAME(m_adpcm.nibble[adpcm]), adpcm);
		save_item(NAME(m_adpcm.addr[adpcm]), adpcm);
	}
}


void huc6272_device::device_reset()
{
	// every register is 0 after reset, except $05 PHASE (1) and $07 (FFh); $0A reads 3FFFFh, equivalent to the 0 kept here
	m_register = 0;
	m_kram_addr_r = m_kram_addr_w = 0;
	m_kram_inc_r = m_kram_inc_w = 0;
	m_kram_page_r = m_kram_page_w = 0;
	m_kram_load_reg = m_kram_write_reg = 0;
	m_page_setting = 0;
	m_kram_mode = 0;
	m_micro_prg.index = 0;
	m_adpcm = decltype(m_adpcm)();
	for (auto &bg : m_bg)
		bg = bg_t();
	m_bg_mode = 0;
	m_bg_priority = 0;
	m_bg_scroll_mode = 0;
	m_bg0sub_bat = 0;
	m_bg0sub_cg = 0;
	std::fill(std::begin(m_bg_affine), std::end(m_bg_affine), 0);
	m_micro_prg.ctrl = 0;
	m_rainbow = rainbow_t();

	m_scsi = scsi_t();
	m_dma = dma_t();
	m_in_scsi_update = false;
	m_scsicb->write(0);
	m_scsicb->atn_w(0);
	m_scsicb->sel_w(0);
	m_scsicb->ack_w(0);
	m_scsicb->rst_w(0);
	m_scsi.bus = scsi_bus_signals();
	interrupt_update();
}

device_memory_interface::space_config_vector huc6272_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_space_config),
		std::make_pair(AS_DATA,    &m_data_space_config),
		std::make_pair(AS_IO,      &m_io_space_config)
	};
}

void huc6272_device::microprg_map(address_map &map)
{
	if (!has_configured_map(0))
		map(0x00, 0x0f).ram().share(m_microprg_ram);
}

//**************************************************************************
//  Memory maps
//**************************************************************************

void huc6272_device::kram_map(address_map &map)
{
	if (!has_configured_map(1))
	{
		map(0x000000, 0x03ffff).ram().share(m_kram_page[0]);
		map(0x040000, 0x07ffff).ram().share(m_kram_page[1]);
	}
}

// TODO: are dword transfers supported by the chip?
void huc6272_device::amap(address_map &map)
{
	map(0x00, 0x01).rw(FUNC(huc6272_device::status_r), FUNC(huc6272_device::register_select_w));
	map(0x02, 0x03).r(FUNC(huc6272_device::status2_r));
	map(0x04, 0x07).rw(FUNC(huc6272_device::data_r), FUNC(huc6272_device::data_w));
}

void huc6272_device::io_map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(huc6272_device::scsi_data_r), FUNC(huc6272_device::scsi_data_w));
	map(0x01, 0x01).rw(FUNC(huc6272_device::scsi_ctrl_r), FUNC(huc6272_device::scsi_ctrl_w));
	map(0x02, 0x02).rw(FUNC(huc6272_device::scsi_mode_r), FUNC(huc6272_device::scsi_mode_w));
	map(0x03, 0x03).rw(FUNC(huc6272_device::scsi_phase_r), FUNC(huc6272_device::scsi_phase_w));
	map(0x04, 0x04).r(FUNC(huc6272_device::scsi_bus_status_r));
	map(0x05, 0x05).rw(FUNC(huc6272_device::scsi_bus_r), FUNC(huc6272_device::scsi_bus_w));
	map(0x06, 0x06).r(FUNC(huc6272_device::scsi_input_data_r));
	map(0x07, 0x07).rw(FUNC(huc6272_device::scsi_irq_ack_r), FUNC(huc6272_device::scsi_dma_start_w));
	map(0x08, 0x08).rw(FUNC(huc6272_device::scsi_subchannel_r), FUNC(huc6272_device::scsi_subchannel_w));
	map(0x09, 0x09).rw(FUNC(huc6272_device::dma_address_r), FUNC(huc6272_device::dma_address_w));
	map(0x0a, 0x0a).rw(FUNC(huc6272_device::dma_size_r), FUNC(huc6272_device::dma_size_w));
	map(0x0b, 0x0b).rw(FUNC(huc6272_device::dma_control_r), FUNC(huc6272_device::dma_control_w));
	map(0x0c, 0x0c).rw(FUNC(huc6272_device::kram_read_address_r), FUNC(huc6272_device::kram_read_address_w));
	map(0x0d, 0x0d).rw(FUNC(huc6272_device::kram_write_address_r), FUNC(huc6272_device::kram_write_address_w));
	map(0x0e, 0x0e).rw(FUNC(huc6272_device::kram_read_data_r), FUNC(huc6272_device::kram_write_data_w));
	map(0x0f, 0x0f).rw(FUNC(huc6272_device::kram_page_setup_r), FUNC(huc6272_device::kram_page_setup_w));

	map(0x10, 0x10).rw(FUNC(huc6272_device::bg_mode_r), FUNC(huc6272_device::bg_mode_w));
	map(0x12, 0x12).w(FUNC(huc6272_device::bg_priority_w));
	map(0x13, 0x13).w(FUNC(huc6272_device::microprogram_address_w));
	map(0x14, 0x14).w(FUNC(huc6272_device::microprogram_data_w));
	map(0x15, 0x15).rw(FUNC(huc6272_device::microprogram_control_r), FUNC(huc6272_device::microprogram_control_w));
	map(0x16, 0x16).w(FUNC(huc6272_device::bg_scroll_mode_w));

	map(0x20, 0x20).w(FUNC(huc6272_device::bg_bat_w<0>));
	map(0x21, 0x21).w(FUNC(huc6272_device::bg_cg_w<0>));
	map(0x22, 0x22).w(FUNC(huc6272_device::bg0sub_bat_w));
	map(0x23, 0x23).w(FUNC(huc6272_device::bg0sub_cg_w));

	map(0x24, 0x24).w(FUNC(huc6272_device::bg_bat_w<1>));
	map(0x25, 0x25).w(FUNC(huc6272_device::bg_cg_w<1>));

	map(0x28, 0x28).w(FUNC(huc6272_device::bg_bat_w<2>));
	map(0x29, 0x29).w(FUNC(huc6272_device::bg_cg_w<2>));

	map(0x2a, 0x2a).w(FUNC(huc6272_device::bg_bat_w<3>));
	map(0x2b, 0x2b).w(FUNC(huc6272_device::bg_cg_w<3>));

	map(0x2c, 0x2f).w(FUNC(huc6272_device::bg_size_w));

	map(0x30, 0x37).w(FUNC(huc6272_device::bg_scroll_w));
	map(0x38, 0x3d).w(FUNC(huc6272_device::bg_affine_w));
	map(0x40, 0x40).w(FUNC(huc6272_device::rainbow_control_w));
	map(0x41, 0x41).w(FUNC(huc6272_device::rainbow_address_w));
	map(0x42, 0x42).w(FUNC(huc6272_device::rainbow_start_line_w));
	map(0x43, 0x43).w(FUNC(huc6272_device::rainbow_block_count_w));
	map(0x44, 0x44).w(FUNC(huc6272_device::raster_irq_line_w));

	map(0x50, 0x50).w(FUNC(huc6272_device::adpcm_control_w));
	map(0x51, 0x52).w(FUNC(huc6272_device::adpcm_channel_control_w));
	map(0x53, 0x53).r(FUNC(huc6272_device::adpcm_status_r));

	map(0x58, 0x58).w(FUNC(huc6272_device::adpcm_start_address_w<0>));
	map(0x59, 0x59).w(FUNC(huc6272_device::adpcm_end_address_w<0>));
	map(0x5a, 0x5a).w(FUNC(huc6272_device::adpcm_imm_address_w<0>));

	map(0x5c, 0x5c).w(FUNC(huc6272_device::adpcm_start_address_w<1>));
	map(0x5d, 0x5d).w(FUNC(huc6272_device::adpcm_end_address_w<1>));
	map(0x5e, 0x5e).w(FUNC(huc6272_device::adpcm_imm_address_w<1>));

	// KRAM mode: D0 selects 1M or 4M DRAM; the IPL sets 4M before any KRAM access, so only the value is kept
	map(0x61, 0x61).lrw32(
		NAME([this] () -> u32 { return m_kram_mode; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_0_15)
				m_kram_mode = data & 1;
		}));
}


void huc6272_device::write_microprg_data(offs_t address, u16 data)
{
	space(AS_PROGRAM).write_word(address, data);
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

/*
 * -x-- ---- ---- ---- SCSI IRQ pending
 * --x- ---- ---- ---- DMA IRQ pending
 * ---x ---- ---- ---- CD Sub Channel IRQ pending
 * ---- x--- ---- ---- Raster IRQ pending
 * ---- -x-- ---- ---- ADPCM IRQ pending
 * ---- ---- -xxx xxxx register read-back
 */
u16 huc6272_device::status_r(offs_t offset)
{
	u16 res = m_register & 0x7f;
	res |= (m_adpcm.interrupt << 10);
	if (m_scsi.subchannel_irq)
		res |= 0x1000;
	// HACK: the GMAKER manual latches SCDMA.I and SCSIBK.I independently, but reporting both
	// at once confuses Anime Freak FX Vol. 4, so SCSIBK.I is held back while SCDMA.I is set
	// (here and in the bus status register)
	if (m_dma.irq && BIT(m_dma.ctrl, 1))
		res |= 0x2000;
	else if (m_scsi.irq)
		res |= 0x4000;
	if (m_rainbow.raster_irq)
		res |= 0x0800;

	// reading acknowledges the raster interrupt
	if (m_rainbow.raster_irq && !machine().side_effects_disabled())
	{
		m_rainbow.raster_irq = false;
		interrupt_update();
	}
	return res;
}

/*
 * xxxx xxxx ---- ---- Sub Channel Buffer
 * ---- ---- x--- ---- SCSI RST flag
 * ---- ---- -x-- ---- SCSI BUSY flag
 * ---- ---- --x- ---- SCSI REQ flag
 * ---- ---- ---x ---- SCSI MSG flag
 * ---- ---- ---- x--- SCSI CD flag
 * ---- ---- ---- -x-- SCSI IO flag
 * ---- ---- ---- --x- SCSI SEL flag
 * ---- ---- ^^^^ ^^^^ \- same as reg $5
 */
u16 huc6272_device::status2_r(offs_t offset)
{
	u16 res = scsi_bus_signals() | (m_scsi.subchannel_data << 8);

	// reading the sub-code byte acknowledges the sub-code interrupt
	if (m_scsi.subchannel_irq && !machine().side_effects_disabled())
	{
		m_scsi.subchannel_irq = false;
		interrupt_update();
	}
	return res;
}

void huc6272_device::register_select_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_register);
}

u32 huc6272_device::data_r(offs_t offset, u32 mem_mask)
{
	return space(AS_IO).read_dword(m_register, mem_mask);
}

void huc6272_device::data_w(offs_t offset, u32 data, u32 mem_mask)
{
	space(AS_IO).write_dword(m_register, data, mem_mask);
}

/*
 * I/O handlers
 */

/*
 * SCSI controller
 *
 * KING is the SCSI initiator; the PC-FX CD drive is the only target.
 */

// RST, BSY, REQ, MSG, C/D, I/O and SEL, same layout as the low byte of status 2
u8 huc6272_device::scsi_bus_signals()
{
	const u32 ctrl = m_scsibus->ctrl_r();
	return
			((ctrl & nscsi_device_interface::S_RST) ? 0x80 : 0) |
			((ctrl & nscsi_device_interface::S_BSY) ? 0x40 : 0) |
			((ctrl & nscsi_device_interface::S_REQ) ? 0x20 : 0) |
			((ctrl & nscsi_device_interface::S_MSG) ? 0x10 : 0) |
			((ctrl & nscsi_device_interface::S_CTL) ? 0x08 : 0) |
			((ctrl & nscsi_device_interface::S_INP) ? 0x04 : 0) |
			((ctrl & nscsi_device_interface::S_SEL) ? 0x02 : 0);
}

// any control line callback but REQ; all of them fire on every bus change
void huc6272_device::scsi_line_w(int state)
{
	// KING must not answer REQ from inside the drive's own ctrl_w(S_REQ) call: that would
	// re-enter nscsi_full_device::step(), and the trailing step(false) in target_send_byte()/
	// target_recv_byte() would then skip scsi_data_byte_period(), so the transfer would run
	// unpaced. The reaction is deferred by a zero-time timer instead.
	m_scsi_line_timer->adjust(attotime::zero);
}

// REQ callback: REQ can fall and rise again before the deferred handler runs, so the
// rising edge is latched here.
void huc6272_device::scsi_req_edge_w(int state)
{
	if (state && !m_scsi.req_last)
		m_scsi.req_rose = true;
	m_scsi.req_last = state;
	m_scsi_line_timer->adjust(attotime::zero);
}

TIMER_CALLBACK_MEMBER(huc6272_device::scsi_lines_changed)
{
	const u8 bus = scsi_bus_signals();
	const u8 changed = bus ^ m_scsi.bus;
	const bool req_rose = m_scsi.req_rose;
	m_scsi.bus = bus;
	m_scsi.req_rose = false;

	if (req_rose || BIT(changed, 5))
		scsi_req_w(BIT(bus, 5));
	else if (changed)
		scsi_dma_update();
}

bool huc6272_device::scsi_phase_match()
{
	const u8 bus = scsi_bus_signals();
	return BIT(bus, 2) == BIT(m_scsi.phase, 0) && BIT(bus, 3) == BIT(m_scsi.phase, 1) && BIT(bus, 4) == BIT(m_scsi.phase, 2);
}

void huc6272_device::scsi_set_ack(bool state)
{
	m_scsi.ack = state;
	m_scsicb->ack_w(state);
}

// the data bus is only driven when "assert data bus" is set, or by a DMA send
void huc6272_device::scsi_update_data_bus()
{
	if (m_scsi.dma_send)
		m_scsicb->write(m_scsi.data_cache);
	else
		m_scsicb->write(BIT(m_scsi.ctrl, 0) ? m_scsi.data : 0);
}

void huc6272_device::scsi_req_w(int state)
{
	// phase mismatch interrupt, raised on REQ assertion while in DMA mode
	if (state && BIT(m_scsi.mode, 1) && !scsi_phase_match())
	{
		m_scsi.irq = true;
		interrupt_update();
	}

	scsi_dma_update();
}

void huc6272_device::dma_write_byte(u8 data)
{
	if (!m_dma.flipflop)
	{
		m_dma.latch = data;
	}
	else
	{
		const u32 page = BIT(m_page_setting, 0);
		space(AS_DATA).write_word((m_dma.addr & 0x3ffff) | (page << 18), m_dma.latch | (data << 8));
		m_dma.addr = ((m_dma.addr + 1) & 0x1ffff) | (m_dma.addr & 0x20000);
		m_dma.size = (m_dma.size - 2) & 0x3ffff;
		if (!m_dma.size)
		{
			m_dma.irq = true;
			m_dma.ctrl &= ~1;
			interrupt_update();
			return;
		}
	}
	m_dma.flipflop ^= 1;
}

// advance a DMA/pseudo-DMA handshake according to the current bus state
void huc6272_device::scsi_dma_update()
{
	if (m_in_scsi_update)
		return;
	m_in_scsi_update = true;

	const u8 bus = scsi_bus_signals();
	const bool req = BIT(bus, 5);
	const bool cd = BIT(bus, 3);
	const bool io = BIT(bus, 2);

	if (m_scsi.dma_receive)
	{
		if (!cd && io)
		{
			if (req && !m_scsi.ack)
			{
				if (!m_scsi.drq)
				{
					m_scsi.drq = true;
					m_scsi.data_cache = m_scsicb->read();
					if (BIT(m_dma.ctrl, 0))
					{
						m_scsi.drq = false;
						dma_write_byte(m_scsi.data_cache);
						scsi_set_ack(true);
					}
				}
			}
			else if (m_scsi.ack && !req)
			{
				scsi_set_ack(false);
			}
		}
	}
	else if (m_scsi.dma_send)
	{
		if (!io)
		{
			if (req && !m_scsi.ack)
			{
				if (!m_scsi.drq)
				{
					scsi_update_data_bus();
					scsi_set_ack(true);
					m_scsi.drq = true;
				}
			}
			else if (m_scsi.ack && !req)
			{
				scsi_set_ack(false);
			}
		}
	}

	m_in_scsi_update = false;
}

u32 huc6272_device::scsi_data_r(offs_t offset)
{
	return m_scsicb->read();
}

void huc6272_device::scsi_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (BIT(m_scsi.ctrl, 7) || !ACCESSING_BITS_0_7)
		return;

	m_scsi.data = data & 0xff;
	scsi_update_data_bus();
}

/*
 * x--- ---- RST
 * ---x ---- ACK
 * ---- -x-- SEL
 * ---- --x- ATN
 * ---- ---x assert data bus
 */
u32 huc6272_device::scsi_ctrl_r(offs_t offset)
{
	return m_scsi.ctrl;
}

void huc6272_device::scsi_ctrl_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (!ACCESSING_BITS_0_7)
		return;

	if (BIT(data, 7))
	{
		// RST also resets the internal SCSI registers
		if (!BIT(m_scsi.ctrl, 7))
		{
			m_scsi.data = 0;
			scsi_set_mode(0);
			m_scsi.phase = 0;
			m_scsi.data_cache = 0;
		}
		m_scsi.ctrl = 0x80;
	}
	else
	{
		m_scsi.ctrl = data & 0x97;
		m_scsicb->atn_w(BIT(data, 1));
		m_scsicb->sel_w(BIT(data, 2));
		scsi_set_ack(BIT(data, 4));
	}
	scsi_update_data_bus();
	m_scsicb->rst_w(BIT(data, 7));
	scsi_dma_update();
}

/*
 * ---- --x- DMA mode
 * ---- ---x sequential DMA
 */
u32 huc6272_device::scsi_mode_r(offs_t offset)
{
	return m_scsi.mode;
}

void huc6272_device::scsi_mode_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (BIT(m_scsi.ctrl, 7) || !ACCESSING_BITS_0_7)
		return;

	scsi_set_mode(data & 0xff);
}

void huc6272_device::scsi_set_mode(u8 data)
{
	if (!BIT(data, 1) && BIT(m_scsi.mode, 1))
	{
		// leaving DMA mode aborts any transfer in progress
		m_scsi.irq = false;
		scsi_set_ack(false);
		m_scsi.drq = false;
		m_scsi.dma_receive = false;
		m_scsi.dma_send = false;
		scsi_update_data_bus();
		interrupt_update();
	}

	m_scsi.mode = data;
}

/*
 * ---- -x-- MSG
 * ---- --x- C/D
 * ---- ---x I/O
 * expected bus phase, compared against the bus for the phase match flag and interrupt
 */
u32 huc6272_device::scsi_phase_r(offs_t offset)
{
	return m_scsi.phase;
}

void huc6272_device::scsi_phase_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (BIT(m_scsi.ctrl, 7) || !ACCESSING_BITS_0_7)
		return;

	m_scsi.phase = data & 7;
}

u32 huc6272_device::scsi_bus_status_r(offs_t offset)
{
	return scsi_bus_signals();
}

/*
 * ---- ---- xxxx xxxx ---- ---- ---- ---- (R) DMA data, acknowledges a pseudo-DMA receive
 *                                         (W) DMA send data
 * ---- ---- ---- ---- ---- ---- -x-- ---- (R) DRQ
 * ---- ---- ---- ---- ---- ---- ---x ---- (R) SCSI interrupt
 * ---- ---- ---- ---- ---- ---- ---- x--- (R) phase match
 * ---- ---- ---- ---- ---- ---- ---- --x- (R) ATN
 * ---- ---- ---- ---- ---- ---- ---- ---x (R) ACK
 * any write to the low half starts a DMA send (target receive)
 */
u32 huc6272_device::scsi_bus_r(offs_t offset, u32 mem_mask)
{
	if (BIT(m_scsi.ctrl, 7))
		return 0;

	u32 res = 0;
	if (ACCESSING_BITS_0_15)
	{
		// KING is the only initiator, so the bus ACK and ATN are its own outputs
		res |= m_scsi.ack ? 0x01 : 0;
		res |= BIT(m_scsi.ctrl, 1) ? 0x02 : 0;
		if ((m_scsi.dma_receive || m_scsi.dma_send) && m_scsi.drq)
			res |= 0x40;
		if (!m_dma.irq && m_scsi.irq)
			res |= 0x10;
		if (scsi_phase_match())
			res |= 0x08;
	}

	if (ACCESSING_BITS_16_23)
	{
		res |= m_scsi.data_cache << 16;
		if (m_scsi.dma_receive && !machine().side_effects_disabled())
		{
			m_scsi.drq = false;
			scsi_set_ack(true);
			scsi_dma_update();
		}
	}

	return res;
}

void huc6272_device::scsi_bus_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (BIT(m_scsi.ctrl, 7))
		return;

	if (ACCESSING_BITS_0_15)
	{
		m_scsi.dma_receive = false;
		m_scsi.dma_send = true;
		m_scsi.drq = true;
	}

	if (ACCESSING_BITS_16_23)
	{
		if (m_scsi.dma_send && m_scsi.drq)
		{
			m_scsi.data_cache = (data >> 16) & 0xff;
			m_scsi.drq = false;
		}
	}

	scsi_dma_update();
}

u32 huc6272_device::scsi_input_data_r(offs_t offset)
{
	return m_scsi.data_cache;
}

u32 huc6272_device::scsi_irq_ack_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		m_scsi.irq = false;
		interrupt_update();
	}
	return 0xff;
}

// starts a DMA receive (initiator receive), only in DMA mode
void huc6272_device::scsi_dma_start_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (BIT(m_scsi.ctrl, 7))
		return;

	if (BIT(m_scsi.mode, 1))
	{
		m_scsi.dma_receive = true;
		m_scsi.dma_send = false;
		scsi_dma_update();
	}
}

// sub-code symbol latched from the drive at 7.35 kHz; reading it acknowledges the interrupt
u32 huc6272_device::scsi_subchannel_r(offs_t offset)
{
	if (m_scsi.subchannel_irq && !machine().side_effects_disabled())
	{
		m_scsi.subchannel_irq = false;
		interrupt_update();
	}
	return m_scsi.subchannel_data;
}

// bit 0 = SUBC EBL (latch sub-code), bit 1 = SUB INT (interrupt on each symbol)
void huc6272_device::scsi_subchannel_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_scsi.subchannel_ctrl = data & 3;
		m_scsi.subchannel_irq = false;
		interrupt_update();
	}
}

void huc6272_device::scsi_subcode_w(u8 data)
{
	if (!BIT(m_scsi.subchannel_ctrl, 0))
		return;

	m_scsi.subchannel_data = data;
	if (BIT(m_scsi.subchannel_ctrl, 1))
	{
		m_scsi.subchannel_irq = true;
		interrupt_update();
	}
}

u32 huc6272_device::dma_address_r(offs_t offset)
{
	return m_dma.addr;
}

void huc6272_device::dma_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma.addr);
	m_dma.addr &= 0x3ffff;
}

u32 huc6272_device::dma_size_r(offs_t offset)
{
	return m_dma.size;
}

void huc6272_device::dma_size_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma.size);
	m_dma.size &= 0x3fffe;
	m_dma.flipflop = 0;
}

/*
 * ---- --x- DMA end interrupt enable
 * ---- ---x DMA enable (SCSI to KRAM)
 * reads return the DMA end interrupt flag, and acknowledge it
 */
u32 huc6272_device::dma_control_r(offs_t offset, u32 mem_mask)
{
	u32 res = 0;
	if (ACCESSING_BITS_0_15)
	{
		res = m_dma.irq ? 1 : 0;
		if (!machine().side_effects_disabled())
		{
			m_dma.irq = false;
			interrupt_update();
		}
	}
	return res;
}

void huc6272_device::dma_control_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (!ACCESSING_BITS_0_15)
		return;

	m_dma.ctrl = data & 3;
	m_dma.irq = false;
	m_dma.flipflop = 0;
	interrupt_update();
	scsi_dma_update();
}

u32 huc6272_device::kram_read_address_r(offs_t offset)
{
	return (m_kram_addr_r & 0x3ffff)
		| ((m_kram_inc_r & 0x3ff) << 18)
		| ((m_kram_page_r & 1) << 31);
}

void huc6272_device::kram_read_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_kram_load_reg);
	m_kram_addr_r = (m_kram_load_reg & 0x0003ffff);
	m_kram_inc_r =  (m_kram_load_reg & 0x0ffc0000) >> 18;
	m_kram_page_r = BIT(m_kram_load_reg, 31);
}

u32 huc6272_device::kram_write_address_r(offs_t offset)
{
	return (m_kram_addr_w & 0x3ffff)
		| ((m_kram_inc_w & 0x3ff) << 18)
		| ((m_kram_page_w & 1) << 31);
}

void huc6272_device::kram_write_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_kram_write_reg);
	m_kram_addr_w = (m_kram_write_reg & 0x0003ffff);
	m_kram_inc_w =  (m_kram_write_reg & 0x0ffc0000) >> 18;
	m_kram_page_w = BIT(m_kram_write_reg, 31);
}

// TODO: is this really 32-bit access?
// writes in 16-bit units audio/photo CD submenus
u16 huc6272_device::kram_read_data_r(offs_t offset, u16 mem_mask)
{
	const u16 res = space(AS_DATA).read_word(((m_kram_addr_r) | (m_kram_page_r << 18)) << 0, mem_mask);
	if (!machine().side_effects_disabled())
	{
		m_kram_addr_r = (m_kram_addr_r & 0x20000) | ((m_kram_addr_r + util::sext<s32>(m_kram_inc_r & 0x3ff, 10)) & 0x1ffff);
	}
	return res;
}

void huc6272_device::kram_write_data_w(offs_t offset, u16 data, u16 mem_mask)
{
	space(AS_DATA).write_word(
		((m_kram_addr_w) | (m_kram_page_w << 18)) << 0,
		data,
		mem_mask
	);
	// increments wrap within the A/B bank
	m_kram_addr_w = (m_kram_addr_w & 0x20000) | ((m_kram_addr_w + util::sext<s32>(m_kram_inc_w & 0x3ff, 10)) & 0x1ffff);
}

/*
 * ---x ---- ---- ---- RAINBOW page setting
 * ---- ---x ---- ---- ADPCM page setting
 * ---- ---- ---x ---- BG page setting
 * ---- ---- ---- ---x SCSI page setting
 */
u32 huc6272_device::kram_page_setup_r(offs_t offset)
{
	return m_page_setting;
}

void huc6272_device::kram_page_setup_w(offs_t offset, u32 data, u32 mem_mask)
{
	// TODO: dispatch in individual bits instead
	COMBINE_DATA(&m_page_setting);
}

/*
 * xxxx ---- ---- ---- BG3 mode setting
 * ---- xxxx ---- ---- BG2 mode setting
 * ---- ---- xxxx ---- BG1 mode setting
 * ---- ---- ---- xxxx BG0 mode setting
 *
 * 0000 - <unused>
 * 0001 - 4 color palette
 * 0010 - 16 color palette
 * 0011 - 256 color palette
 * 0100 - 64k color
 * 0101 - 16M color
 * 1001 - 4 color palette block mode
 * 1010 - 16 color palette block mode
 * 1011 - 256 color palette block mode
 * others - <invalid>
 */
u32 huc6272_device::bg_mode_r(offs_t offset)
{
	return m_bg_mode;
}

void huc6272_device::bg_mode_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
		m_bg_mode = (m_bg_mode & ~mem_mask) | (data & mem_mask);
}

/*
 * ---x ---- ---- ---- BG0 rotation enable
 * ---- xxx- ---- ---- BG3 priority
 * ---- ---x xx-- ---- BG2 priority
 * ---- ---- --xx x--- BG1 priority
 * ---- ---- ---- -xxx BG0 priority
 * ---- ---- ---- -000 hidden
 * ---- ---- ---- -001 farthest back
 * ---- ---- ---- -100 farthest forward
 * ---- ---- ---- -1xx <prohibited>
 * this orders the KING layers between themselves, the '6261 then orders KING against the other chips
 */
void huc6272_device::bg_priority_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
		m_bg_priority = data & 0x1fff;
}

void huc6272_device::microprogram_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_micro_prg.index = data & 0xf;
}

/*
 * microprogram, one entry per KRAM access slot of an 8 dot cycle, 0-7 for KRAM page A, 8-f for page B
 * ---x ---- ---- no operation
 * ---- xx-- ---- BG layer
 * ---- --x- ---- rotation (affine) access
 * ---- ---x ---- BAT access
 * ---- ---- x--- (CG) fetch through BAT
 * ---- ---- -xxx (CG) word offset
 */
void huc6272_device::microprogram_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
	{
		write_microprg_data(m_micro_prg.index, data & 0xffff);
		m_micro_prg.index ++;
		m_micro_prg.index &= 0xf;
	}
}

u8 huc6272_device::microprogram_control_r(offs_t offset)
{
	return m_micro_prg.ctrl;
}

void huc6272_device::microprogram_control_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_micro_prg.ctrl = data & 1;
}

// ---- ---- ---- xxxx BG3-BG0 endless scroll (wrap-around)
void huc6272_device::bg_scroll_mode_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_bg_scroll_mode = data & 0xf;
}

// addresses are in 1K word units
template <unsigned N> void huc6272_device::bg_bat_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
		m_bg[N].bat = data & 0xffff;
}

template <unsigned N> void huc6272_device::bg_cg_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
		m_bg[N].cg = data & 0xffff;
}

void huc6272_device::bg0sub_bat_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
		m_bg0sub_bat = data & 0xffff;
}

void huc6272_device::bg0sub_cg_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
		m_bg0sub_cg = data & 0xffff;
}

/*
 * xxxx ---- ---- ---- BG0 sub screen width (BG0 only)
 * ---- xxxx ---- ---- BG0 sub screen height (BG0 only)
 * ---- ---- xxxx ---- width, as a power of 2
 * ---- ---- ---- xxxx height, as a power of 2
 */
void huc6272_device::bg_size_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
		m_bg[offset].size = data & (offset ? 0x00ff : 0xffff);
}

// BG0 scroll registers are 11 bits, the others 10 bits
void huc6272_device::bg_scroll_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
	{
		const u8 layer_n = (offset & 6) >> 1;
		const u16 mask = layer_n ? 0x3ff : 0x7ff;

		if (offset & 1)
			m_bg[layer_n].yscroll = data & mask;
		else
			m_bg[layer_n].xscroll = data & mask;
	}
}

// BG0 rotation coefficients A, B, C, D (8.8 fixed point) and center X, Y
void huc6272_device::bg_affine_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
		m_bg_affine[offset] = data & 0xffff;
}

/*
 * BG rendering
 *
 * Output pixels are tagged with their layer number in bits 31-28 (1-4 for BG0-3).
 * Palette modes return the palette index (without the '6261 palette offset),
 * direct color modes set bit 27 and return YUV888.
 */
static constexpr u32 BG_DIRECT = 1 << 27;

u16 huc6272_device::kram_word(int page, u32 base, u32 offset)
{
	return m_kram_page[page][(base & 0x20000) | ((base + offset) & 0x1ffff)];
}

void huc6272_device::bg_decode_8(u8 mode, const u16 *cg, u32 pbn, u32 layer_or, u32 *dest)
{
	switch (mode)
	{
		case 1: // 4 colors, 2bpp
			for (int i = 0; i < 8; i++)
			{
				const u8 pix = (cg[0] >> (14 - i * 2)) & 3;
				if (pix)
					dest[i] = layer_or | (((pbn << 2) + pix) & 0xff);
			}
			break;

		case 2: // 16 colors, 4bpp
			for (int i = 0; i < 8; i++)
			{
				const u8 pix = (cg[i >> 2] >> (12 - (i & 3) * 4)) & 0xf;
				if (pix)
					dest[i] = layer_or | (((pbn << 4) + pix) & 0xff);
			}
			break;

		case 3: // 256 colors, 8bpp
			for (int i = 0; i < 8; i++)
			{
				const u8 pix = (cg[i >> 1] >> (BIT(i, 0) ? 0 : 8)) & 0xff;
				if (pix)
					dest[i] = layer_or | pix;
			}
			break;

		case 4: // 64K colors, Y8 U4 V4 per pixel, Y=0 is transparent
			for (int i = 0; i < 8; i++)
			{
				if (cg[i] & 0xff00)
					dest[i] = layer_or | BG_DIRECT | ((cg[i] & 0xff00) << 8) | ((cg[i] & 0x00f0) << 8) | ((cg[i] & 0x000f) << 4);
			}
			break;

		case 5: // 16M colors, Y0 Y1 / U V per 2 pixels, Y=0 is transparent
			for (int i = 0; i < 8; i += 2)
			{
				const u16 y = cg[i];
				const u16 uv = cg[i + 1];
				if (y & 0xff00)
					dest[i] = layer_or | BG_DIRECT | ((y & 0xff00) << 8) | uv;
				if (y & 0x00ff)
					dest[i + 1] = layer_or | BG_DIRECT | ((y & 0x00ff) << 16) | uv;
			}
			break;
	}
}

void huc6272_device::draw_bg_layer(int n, int raster, u32 *line)
{
	// out of range sizes behave as the minimum size
	static constexpr u8 SIZE_SHIFT[16] = { 3, 3, 3, 3, 4, 5, 6, 7, 8, 9, 10, 3, 3, 3, 3, 3 };
	static constexpr bool SIZE_INVALID[16] = { true, true, true, false, false, false, false, false, false, false, false, true, true, true, true, true };
	// CG words fetched per 8 dots for each mode
	static constexpr u8 CG_WORDS[8] = { 0, 1, 2, 4, 8, 8, 0, 0 };

	const u8 bgmode = (m_bg_mode >> (n * 4)) & 0xf;
	const u8 mode = bgmode & 7;
	if (mode == 0 || mode >= 6)
		return;

	const u32 layer_or = u32(n + 1) << 28;
	const int page = BIT(m_page_setting, 4);
	const bool block_mode = BIT(bgmode, 3);
	const bool endless = BIT(m_bg_scroll_mode, n);
	const bool rotate = (n == 0) && BIT(m_bg_priority, 12);
	const s32 xscroll = util::sext(m_bg[n].xscroll, n ? 10 : 11);
	const s32 yscroll = util::sext(m_bg[n].yscroll, n ? 10 : 11);
	const u32 yoffset = (yscroll + raster) & 0xffff;

	const u32 bat_offset = m_bg[n].bat * 1024;
	const u32 cg_offset = m_bg[n].cg * 1024;
	const u32 bat_sub_offset = n ? bat_offset : m_bg0sub_bat * 1024;
	const u32 cg_sub_offset = n ? cg_offset : m_bg0sub_cg * 1024;

	const u16 size = m_bg[n].size;
	const u16 sub_size = n ? (size << 8) : size;
	const u8 width_shift = SIZE_SHIFT[(size >> 4) & 0xf];
	const u8 height_shift = SIZE_SHIFT[size & 0xf];
	const bool width_invalid = SIZE_INVALID[(size >> 4) & 0xf];
	const u8 sub_width_shift = SIZE_SHIFT[(sub_size >> 12) & 0xf];
	const u8 sub_height_shift = SIZE_SHIFT[(sub_size >> 8) & 0xf];
	const bool sub_width_invalid = SIZE_INVALID[(sub_size >> 12) & 0xf];
	const u32 bat_width = (1 << width_shift) >> 3;
	const u32 bat_height = (1 << height_shift) >> 3;
	const u32 bat_sub_width = (1 << sub_width_shift) >> 3;
	const u32 bat_sub_height = (1 << sub_height_shift) >> 3;
	const u32 bat_mask = 0x7ff >> 3;
	const u32 bat_sub_width_test = endless ? (bat_mask + 1) : std::max(bat_width, bat_sub_width);
	const u32 bat_sub_height_test = endless ? (bat_mask + 1) : std::max(bat_height, bat_sub_height);

	// decode the microprogram: which CG word each of the 8 fetch slots loads, and if a BAT fetch happens at all
	struct slots_t { u16 mask[8]{}; u8 remap[8]{}; bool through_bat[8]{}; bool bat_fetch = false; };
	auto decode_slots = [this, n, rotate] (u32 cg_base, u32 bat_base)
	{
		slots_t slots;
		int count = 0;
		for (int i = 0; i < 8; i++)
		{
			const u16 op = m_microprg_ram[(BIT(cg_base, 17) ? 8 : 0) + i];
			if (((op >> 6) & 3) == n && !BIT(op, 8) && !BIT(op, 4))
			{
				// a mismatched rotation bit would compute wrong addresses, treat as no fetch
				slots.mask[count] = (BIT(op, 5) == rotate) ? 0xffff : 0;
				slots.remap[count] = op & 7;
				slots.through_bat[count] = BIT(op, 3);
				count++;
			}
		}
		for (int i = 0; i < 8; i++)
		{
			const u16 op = m_microprg_ram[(BIT(bat_base, 17) ? 8 : 0) + i];
			if (((op >> 6) & 3) == n && !BIT(op, 8) && BIT(op, 4) && BIT(op, 5) == rotate)
				slots.bat_fetch = true;
		}
		return slots;
	};
	const slots_t main_slots = decode_slots(cg_offset, bat_offset);
	const slots_t sub_slots = decode_slots(cg_sub_offset, bat_sub_offset);

	if (rotate)
	{
		if (mode > 4)
			return;

		const s32 a = s16(m_bg_affine[0]);
		const s32 b = s16(m_bg_affine[1]);
		const s32 c = s16(m_bg_affine[2]);
		const s32 d = s16(m_bg_affine[3]);
		const s32 cx = s16(m_bg_affine[4]);
		const s32 cy = s16(m_bg_affine[5]);
		const u32 bat_width_mask = endless ? (bat_width - 1) : 0xffff;
		const u32 bat_height_mask = endless ? (bat_height - 1) : 0xffff;
		const u32 bitmap_ymask = util::make_bitmask<u32>(height_shift);
		const s32 raw_x = s32(util::sext(m_bg[n].xscroll, 11)) - cx;
		const s32 raw_y = raster + s32(util::sext(m_bg[n].yscroll, 11)) - cy;
		s32 xaccum = raw_x * a + raw_y * b + (cx << 8);
		s32 yaccum = raw_y * d + raw_x * c + (cy << 8);

		for (int x = 0; x < 256; x++, xaccum += a, yaccum += c)
		{
			const u16 tx = xaccum >> 8;
			const u16 ty = yaccum >> 8;
			const u32 bx = (tx >> 3) & bat_width_mask;
			const u32 by = (ty >> 3) & bat_height_mask;

			if (!endless && (bx >= bat_width || by >= bat_height))
				continue;

			const u32 words = CG_WORDS[mode];
			u32 pbn = 0;
			u32 cg_addr;
			if (block_mode)
			{
				u16 bat = kram_word(page, bat_offset, bx + ((by << width_shift) >> 3));
				pbn = bat >> 12;
				bat &= 0xfff;
				cg_addr = bat * 8 * words + (ty & 7) * words;
			}
			else
			{
				cg_addr = bx * words + (((ty & bitmap_ymask) << width_shift) >> 3) * words;
			}

			u32 pix = 0;
			switch (mode)
			{
				case 1:
					pix = (kram_word(page, cg_offset, cg_addr) >> ((7 - (tx & 7)) * 2)) & 3;
					if (pix)
						pix = (pbn << 2) + pix;
					break;
				case 2:
					pix = (kram_word(page, cg_offset, cg_addr + ((tx >> 2) & 1)) >> ((3 - (tx & 3)) * 4)) & 0xf;
					if (pix)
						pix = (pbn << 4) + pix;
					break;
				case 3:
					pix = (kram_word(page, cg_offset, cg_addr + ((tx >> 1) & 3)) >> (BIT(tx, 0) ? 0 : 8)) & 0xff;
					break;
				case 4:
				{
					const u16 w = kram_word(page, cg_offset, cg_addr + (tx & 7));
					if (w & 0xff00)
						line[8 + x] = layer_or | BG_DIRECT | ((w & 0xff00) << 8) | ((w & 0x00f0) << 8) | ((w & 0x000f) << 4);
					continue;
				}
			}
			if (pix)
				line[8 + x] = layer_or | (pix & 0xff);
		}
		return;
	}

	const u32 words = CG_WORDS[mode];
	u32 bat_x = (xscroll >> 3) & bat_mask;
	const u32 bat_y = (yoffset >> 3) & bat_mask;
	const u32 ysmall = yoffset & 7;
	// linear (bitmap) addressing: CG words from the start of the bitmap to this line
	const u32 bitmap_y = ((yoffset & (width_invalid ? 0 : util::make_bitmask<u32>(height_shift))) << width_shift) * words / 8;
	const u32 bitmap_sub_y = ((yoffset & (sub_width_invalid ? 0 : util::make_bitmask<u32>(sub_height_shift))) << sub_width_shift) * words / 8;
	u32 *target = line + 8 - (xscroll & 7);

	for (int x = 0; x < 256 + 8; x += 8)
	{
		const bool in_main = bat_x < bat_width && bat_y < bat_height;
		const bool in_sub = !in_main && bat_x < bat_sub_width_test && bat_y < bat_sub_height_test;

		if (in_main || in_sub)
		{
			const slots_t &slots = in_main ? main_slots : sub_slots;
			const u32 b_off = in_main ? bat_offset : bat_sub_offset;
			const u32 c_off = in_main ? cg_offset : cg_sub_offset;
			const u32 bx = in_main ? bat_x : (bat_x & (bat_sub_width - 1));
			const u32 by = in_main ? bat_y : (bat_y & (bat_sub_height - 1));
			const u8 w_shift = in_main ? width_shift : sub_width_shift;
			const bool w_invalid = in_main ? width_invalid : sub_width_invalid;

			u16 bat = 0;
			if (slots.bat_fetch)
				bat = kram_word(page, b_off, block_mode ? (bx + (((w_invalid ? 0 : by) << w_shift) >> 3)) : 0);
			const u32 pbn = block_mode ? (bat >> 12) : 0;
			bat &= 0xfff;

			const u32 linear = bx * words + (in_main ? bitmap_y : bitmap_sub_y);
			const u32 through_bat = bat * 8 * words + ysmall * words;
			u16 cg[8];
			for (int i = 0; i < words; i++)
				cg[i] = kram_word(page, c_off, (slots.through_bat[i] ? through_bat : linear) + slots.remap[i]) & slots.mask[i];

			bg_decode_8(mode, cg, pbn, layer_or, target + x);
		}
		bat_x = (bat_x + 1) & bat_mask;
	}
}

/*
 * RAINBOW transfers
 *
 * $40 ---- --x- raster interrupt enable
 *     ---- ---x transfer enable
 * $41 KRAM word address of the stream
 * $42 line to start a transfer on
 * $43 number of 16 line blocks to transfer
 * $44 raster interrupt line
 */
void huc6272_device::rainbow_control_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (!ACCESSING_BITS_0_15)
		return;

	m_rainbow.control = data & 3;
	if (!BIT(data, 0))
		m_rainbow.blocks = 0;
	// resetting RINT clears the raster interrupt (so does a status read)
	if (!BIT(data, 1))
	{
		m_rainbow.raster_irq = false;
		interrupt_update();
	}
}

void huc6272_device::rainbow_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_rainbow.address);
	m_rainbow.address &= 0x3ffff;
}

void huc6272_device::rainbow_start_line_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
		m_rainbow.start_line = data & 0x1ff;
}

void huc6272_device::rainbow_block_count_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
		m_rainbow.block_count = data & 0x1f;
}

void huc6272_device::raster_irq_line_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
		m_rainbow.raster_irq_line = data & 0x1ff;
}

// RAINBOW stream byte from KRAM, low byte of each word first
u8 huc6272_device::rainbow_fetch()
{
	const int page = BIT(m_page_setting, 12);
	const u32 pos = m_rainbow.read_pos;
	const u8 data = m_kram_page[page][(pos >> 1) & 0x3ffff] >> (BIT(pos, 0) * 8);
	m_rainbow.read_pos = ((pos + 1) & 0x3ffff) | (pos & 0x40000);
	return data;
}

/*
 * Called by the '6261 at the start of each line (raster = 0-262, active display is 22-261).
 * Raises the raster interrupt, runs the RAINBOW decode schedule (one 16 line block decoded
 * every 16 lines, displayed after the following buffer swap) and fetches its line.
 * Returns the line format (huc6271_device::FORMAT_*).
 */
int huc6272_device::rainbow_line(int raster, u32 *dest)
{
	if (raster == m_rainbow.raster_irq_line && BIT(m_rainbow.control, 1))
	{
		m_rainbow.raster_irq = true;
		interrupt_update();
	}

	if (raster == m_rainbow.start_line && BIT(m_rainbow.control, 0))
		m_rainbow.start_pending = true;

	if (raster >= 262)
	{
		std::fill_n(dest, 256, 0);
		return huc6271_device::FORMAT_NONE;
	}

	if (m_rainbow.busy)
	{
		m_rainbow.busy--;
		if (!m_rainbow.busy)
			m_huc6271->swap_buffers();
	}

	if (!m_rainbow.busy)
	{
		bool first = false;
		if (!m_rainbow.blocks && m_rainbow.start_pending)
		{
			m_rainbow.blocks = m_rainbow.block_count;
			if (m_rainbow.blocks)
			{
				m_rainbow.read_pos = m_rainbow.address << 1;
				first = true;
			}
		}

		if (m_rainbow.blocks)
		{
			m_rainbow.blocks--;
			m_rainbow.busy = 16;
			m_huc6271->decode_block(first, [this] () { return rainbow_fetch(); });
		}
	}

	m_rainbow.start_pending = false;
	return m_huc6271->fetch_line(dest);
}

// renders KING BG layers for display line "raster" (0 = first active line), 256 dots
void huc6272_device::render_bg_line(int raster, u32 *dest)
{
	u32 line[256 + 16];
	std::fill(std::begin(line), std::end(line), 0);

	if (BIT(m_micro_prg.ctrl, 0))
	{
		for (int prio = 1; prio <= 7; prio++)
		{
			for (int n = 0; n < 4; n++)
			{
				if (((m_bg_priority >> (n * 3)) & 7) == prio)
					draw_bg_layer(n, raster, line);
			}
		}
	}

	std::copy_n(&line[8], 256, dest);
}

/*
 * ADPCM
 * $50 ---- xx-- sample rate, a nibble every 2^n '6230 samples
 *     ---- --xx channel 1/0 play (starting playback reloads the address)
 * $51/$52 channel 0/1 buffer mode
 *     ---- -x-- intermediate address interrupt enable
 *     ---- --x- end address interrupt enable
 *     ---- ---x ring buffer (restart from the start address at the end)
 * $53 (R) status, channel 0 bits 1-0, channel 1 bits 3-2: intermediate, end reached
 */
void huc6272_device::adpcm_control_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (!ACCESSING_BITS_0_15)
		return;

	for (int i = 0; i < 2; i++)
	{
		if (!m_adpcm.playing[i] && BIT(data, i))
			m_adpcm.addr[i] = m_adpcm.start[i];
		m_adpcm.playing[i] = BIT(data, i);
	}

	m_adpcm.rate = (data & 0xc) >> 2;
}

void huc6272_device::adpcm_channel_control_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (!ACCESSING_BITS_0_15)
		return;

	m_adpcm.control[offset & 1] = data & 0x7;
	interrupt_update();
}

u32 huc6272_device::adpcm_status_r(offs_t offset)
{
	const u32 res = m_adpcm.status;

	if (!machine().side_effects_disabled())
	{
		m_adpcm.status = 0;
		m_adpcm.interrupt = 0;
		interrupt_update();
	}
	return res;
}

// start address in 256 word units, end address in words, intermediate address in 64 word units
// only the end address is wider than 16 bits; its halves are written independently
template <unsigned N> void huc6272_device::adpcm_start_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
		m_adpcm.start[N] = (data & 0x3ff) * 256;
}

template <unsigned N> void huc6272_device::adpcm_end_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_adpcm.end[N]);
	m_adpcm.end[N] &= 0x3ffff;
}

template <unsigned N> void huc6272_device::adpcm_imm_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
		m_adpcm.imm[N] = (data & 0xfff) * 64;
}

// fetch the next word of ADPCM data and handle the end/intermediate addresses
u16 huc6272_device::adpcm_fetch(int chan)
{
	const int page = BIT(m_page_setting, 8);
	const u16 data = m_kram_page[page][m_adpcm.addr[chan] & 0x3ffff];
	m_adpcm.addr[chan] = (m_adpcm.addr[chan] & 0x20000) | ((m_adpcm.addr[chan] + 1) & 0x1ffff);

	const u32 end = (m_adpcm.end[chan] & 0x20000) | ((m_adpcm.end[chan] + 1) & 0x1ffff);
	if (m_adpcm.addr[chan] == end)
	{
		if (BIT(m_adpcm.control[chan], 0))
			m_adpcm.addr[chan] = m_adpcm.start[chan];
		else
			m_adpcm.playing[chan] = 0;

		m_adpcm.status |= 1 << (chan * 2);
		if (BIT(m_adpcm.control[chan], 1))
		{
			m_adpcm.interrupt = 1;
			interrupt_update();
		}
	}
	else if (m_adpcm.addr[chan] == m_adpcm.imm[chan])
	{
		m_adpcm.status |= 1 << (chan * 2 + 1);
		if (BIT(m_adpcm.control[chan], 2))
		{
			m_adpcm.interrupt = 1;
			interrupt_update();
		}
	}

	return data;
}

u8 huc6272_device::adpcm_update(int chan)
{
	// both channels share the rate divider
	if (chan == 0)
	{
		m_adpcm.tick = 0;
		if (--m_adpcm.divider <= 0)
		{
			m_adpcm.divider += 1 << m_adpcm.rate;
			m_adpcm.tick = 1;
		}
	}

	const u8 rate = m_adpcm.rate << 5;
	if (!m_adpcm.tick)
		return rate;

	// the last word fetched keeps playing even if the channel was stopped
	if (!m_adpcm.have_halfword[chan] && !m_adpcm.playing[chan])
		return rate;

	if (!m_adpcm.nibble[chan])
	{
		m_adpcm.halfword[chan] = adpcm_fetch(chan);
		m_adpcm.have_halfword[chan] = 1;
	}

	const u8 data = (m_adpcm.halfword[chan] >> m_adpcm.nibble[chan]) & 0xf;
	m_adpcm.nibble[chan] = (m_adpcm.nibble[chan] + 4) & 0xf;
	if (!m_adpcm.nibble[chan])
		m_adpcm.have_halfword[chan] = 0;

	return 0x80 | rate | data;
}

u8 huc6272_device::adpcm_update_0()
{
	return adpcm_update(0);
}

u8 huc6272_device::adpcm_update_1()
{
	return adpcm_update(1);
}

void huc6272_device::interrupt_update()
{
	const bool state = m_adpcm.interrupt || m_scsi.irq || m_scsi.subchannel_irq || (m_dma.irq && BIT(m_dma.ctrl, 1)) || m_rainbow.raster_irq;
	if (state != m_irq_state)
	{
		m_irq_state = state;
		m_irq_changed_cb(state);
	}
}
