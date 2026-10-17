// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Angelo Salese
/**************************************************************************************************

Hudson/NEC HuC6272 "King" device

TODO:
- Use NSCSI instead of legacy one;
\- SEL acknowledges with 0x84, bit 7 controller type is unknown at this time
  (bit 2 should select the CD drive);
- SCSI sub-channel register ($08) is a stub;
- Convert base mapping to address_map;
- Convert I/O to space address, and make it honor mem_mask;
- subclass "SCSICD" into SCSI-2 "CD-ROM DRIVE:FX"
  \- Derivative design of PCE drive, which in turn is a derivative of PC-8801-30 (cd drive)
     and PC-8801-31 (interface);
- Implement video routines drawing and interface:
  \- BIOS main menu draws BG0 only as backdrop of the PCE VDCs with 16M mode (5);
  \- (check Photo CD, Audio CD & backup RAM screens);
- Implement video mixing with other PCFX chips;
- Implement microprogram (layer timings, sort of Sega Saturn VRAM cycle patterns);
- Implement Rainbow transfers (NEC logo on POST);
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

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(HUC6272, huc6272_device, "huc6272", "Hudson HuC6272 \"King\"")

huc6272_device::huc6272_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HUC6272, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_huc6271(*this, finder_base::DUMMY_TAG)
	, m_cdda(*this, "cdda")
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
	, m_program_space_config("microprg", ENDIANNESS_LITTLE, 16, 4, -1, address_map_constructor(FUNC(huc6272_device::microprg_map), this))
	, m_data_space_config("kram", ENDIANNESS_LITTLE, 16, 19, -1, address_map_constructor(FUNC(huc6272_device::kram_map), this))
	, m_io_space_config("io", ENDIANNESS_LITTLE, 32, 7, -2, address_map_constructor(FUNC(huc6272_device::io_map), this))
	, m_microprg_ram(*this, "microprg_ram")
	, m_kram_page(*this, "kram_page%u", 0U)
	, m_scsibus(*this, "scsi")
	, m_scsi_data_in(*this, "scsi_data_in")
	, m_scsi_data_out(*this, "scsi_data_out")
	, m_scsi_ctrl_in(*this, "scsi_ctrl_in")
	, m_scsi_cmd_in(*this, "scsi_cmd_in")
	, m_irq_changed_cb(*this)
{
}

void huc6272_device::cdrom_config(device_t *device)
{
	cdda_device *cdda = device->subdevice<cdda_device>("cdda");
	cdda->add_route(0, "^^cdda", 1.0, 0);
	cdda->add_route(1, "^^cdda", 1.0, 1);
}

void huc6272_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, m_cdda, 2).front();

	scsi_port_device &scsibus(SCSI_PORT(config, "scsi"));
	scsibus.set_data_input_buffer("scsi_data_in");
	scsibus.rst_handler().set("scsi_ctrl_in", FUNC(input_buffer_device::write_bit7));
	scsibus.bsy_handler().set("scsi_ctrl_in", FUNC(input_buffer_device::write_bit6));
	scsibus.req_handler().set("scsi_ctrl_in", FUNC(input_buffer_device::write_bit5));
	scsibus.req_handler().append(FUNC(huc6272_device::scsi_req_w));
	scsibus.msg_handler().set("scsi_ctrl_in", FUNC(input_buffer_device::write_bit4));
	scsibus.cd_handler().set("scsi_ctrl_in", FUNC(input_buffer_device::write_bit3));
	scsibus.io_handler().set("scsi_ctrl_in", FUNC(input_buffer_device::write_bit2));
	scsibus.sel_handler().set("scsi_ctrl_in", FUNC(input_buffer_device::write_bit1));

	scsibus.rst_handler().append("scsi_cmd_in", FUNC(input_buffer_device::write_bit7));
	scsibus.ack_handler().set("scsi_cmd_in", FUNC(input_buffer_device::write_bit4));
	scsibus.sel_handler().append("scsi_cmd_in", FUNC(input_buffer_device::write_bit2));
	scsibus.atn_handler().set("scsi_cmd_in", FUNC(input_buffer_device::write_bit1));
	scsibus.bsy_handler().append("scsi_cmd_in", FUNC(input_buffer_device::write_bit0));

	output_latch_device &scsiout(OUTPUT_LATCH(config, "scsi_data_out"));
	scsibus.set_output_latch(scsiout);

	INPUT_BUFFER(config, "scsi_cmd_in");
	INPUT_BUFFER(config, "scsi_ctrl_in");
	INPUT_BUFFER(config, "scsi_data_in");

	scsibus.set_slot_device(1, "cdrom", SCSICD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_0));
	scsibus.slot(1).set_option_machine_config("cdrom", cdrom_config);
}

void huc6272_device::device_start()
{
	save_item(NAME(m_register));
	save_item(NAME(m_kram_addr_r));
	save_item(NAME(m_kram_inc_r));
	save_item(NAME(m_kram_page_r));
	save_item(NAME(m_kram_addr_w));
	save_item(NAME(m_kram_inc_w));
	save_item(NAME(m_kram_page_w));
	save_item(NAME(m_page_setting));

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
	save_item(STRUCT_MEMBER(m_dma, addr));
	save_item(STRUCT_MEMBER(m_dma, size));
	save_item(STRUCT_MEMBER(m_dma, ctrl));
	save_item(STRUCT_MEMBER(m_dma, latch));
	save_item(STRUCT_MEMBER(m_dma, flipflop));
	save_item(STRUCT_MEMBER(m_dma, irq));

	save_item(STRUCT_MEMBER(m_bg, bat_address));
	save_item(STRUCT_MEMBER(m_bg, cg_address));
	save_item(STRUCT_MEMBER(m_bg, mode));
	save_item(STRUCT_MEMBER(m_bg, height));
	save_item(STRUCT_MEMBER(m_bg, width));
	save_item(STRUCT_MEMBER(m_bg, xscroll));
	save_item(STRUCT_MEMBER(m_bg, yscroll));
	save_item(STRUCT_MEMBER(m_bg, priority));

	save_item(NAME(m_bg0sub.bat_address));
	save_item(NAME(m_bg0sub.cg_address));
	save_item(NAME(m_bg0sub.height));
	save_item(NAME(m_bg0sub.width));

	save_item(NAME(m_micro_prg.index));
	save_item(NAME(m_micro_prg.ctrl));

	save_item(NAME(m_adpcm.rate));
	save_item(NAME(m_adpcm.status));
	save_item(NAME(m_adpcm.interrupt));
	for (int adpcm = 0; adpcm < 2; adpcm++)
	{
		save_item(NAME(m_adpcm.playing[adpcm]), adpcm);
		save_item(NAME(m_adpcm.control[adpcm]), adpcm);
		save_item(NAME(m_adpcm.start[adpcm]), adpcm);
		save_item(NAME(m_adpcm.end[adpcm]), adpcm);
		save_item(NAME(m_adpcm.imm[adpcm]), adpcm);
		save_item(NAME(m_adpcm.input[adpcm]), adpcm);
		save_item(NAME(m_adpcm.nibble[adpcm]), adpcm);
		save_item(NAME(m_adpcm.pos[adpcm]), adpcm);
		save_item(NAME(m_adpcm.addr[adpcm]), adpcm);
	}
}


void huc6272_device::device_reset()
{
	for (int i = 0; i < 4; i++)
	{
		m_bg[i].bat_address = 0;
		m_bg[i].cg_address = 0;
		m_bg[i].height = 1;
		m_bg[i].width = 1;
		m_bg[i].mode = 0;
		m_bg[i].priority = 0;
	}

	m_bg0sub.bat_address = 0;
	m_bg0sub.cg_address = 0;
	m_bg0sub.height = 1;
	m_bg0sub.width = 1;

	m_scsi = scsi_t();
	m_dma = dma_t();
	m_in_scsi_update = false;
	m_scsi_data_out->write(0);
	m_scsibus->write_atn(0);
	m_scsibus->write_sel(0);
	m_scsibus->write_ack(0);
	m_scsibus->write_rst(0);
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

	map(0x10, 0x10).w(FUNC(huc6272_device::bg_mode_w));
	map(0x12, 0x12).w(FUNC(huc6272_device::bg_priority_w));
	map(0x13, 0x13).w(FUNC(huc6272_device::microprogram_address_w));
	map(0x14, 0x14).w(FUNC(huc6272_device::microprogram_data_w));
	map(0x15, 0x15).rw(FUNC(huc6272_device::microprogram_control_r), FUNC(huc6272_device::microprogram_control_w));
//  map(0x16, 0x16) wrap-around enable

	map(0x20, 0x20).w(FUNC(huc6272_device::bg_bat_w<0>));
	map(0x21, 0x21).w(FUNC(huc6272_device::bg_cg_w<0>));
	map(0x22, 0x22).w(FUNC(huc6272_device::bg0sub_bat_w));
	map(0x23, 0x23).w(FUNC(huc6272_device::bg0sub_cg_w));

	map(0x24, 0x24).w(FUNC(huc6272_device::bg_bat_w<1>));
	map(0x25, 0x25).w(FUNC(huc6272_device::bg_cg_w<1>));

	map(0x26, 0x26).w(FUNC(huc6272_device::bg_bat_w<2>));
	map(0x27, 0x27).w(FUNC(huc6272_device::bg_cg_w<2>));

	map(0x28, 0x28).w(FUNC(huc6272_device::bg_bat_w<3>));
	map(0x29, 0x29).w(FUNC(huc6272_device::bg_cg_w<3>));

	map(0x2c, 0x2f).w(FUNC(huc6272_device::bg_size_w));

	map(0x30, 0x37).w(FUNC(huc6272_device::bg_scroll_w));
//  map(0x38, 0x3b) BG affine coefficients
//  map(0x3c, 0x3d) BG affine center X/Y
//  map(0x40, 0x44) Rainbow regs

	map(0x50, 0x50).w(FUNC(huc6272_device::adpcm_control_w));
	map(0x51, 0x52).w(FUNC(huc6272_device::adpcm_channel_control_w));
	map(0x53, 0x53).r(FUNC(huc6272_device::adpcm_status_r));

	map(0x58, 0x58).w(FUNC(huc6272_device::adpcm_start_address_w<0>));
	map(0x59, 0x59).w(FUNC(huc6272_device::adpcm_end_address_w<0>));
	map(0x5a, 0x5a).w(FUNC(huc6272_device::adpcm_imm_address_w<0>));

	map(0x5c, 0x5c).w(FUNC(huc6272_device::adpcm_start_address_w<1>));
	map(0x5d, 0x5d).w(FUNC(huc6272_device::adpcm_end_address_w<1>));
	map(0x5e, 0x5e).w(FUNC(huc6272_device::adpcm_imm_address_w<1>));

//  map(0x61, 0x61) KRAM mode (undocumented, used by backup RAM menu)
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
	// TODO: sub-channel and raster IRQs
	// only one of DMA/SCSI is reported at a time (Anime Freak FX Vol. 4 gets confused otherwise)
	if (m_dma.irq && BIT(m_dma.ctrl, 1))
		res |= 0x2000;
	else if (m_scsi.irq)
		res |= 0x4000;
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
	u16 res = scsi_bus_signals();
	// TODO: sub-code data
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
	return m_scsi_ctrl_in->read() & 0xfe;
}

bool huc6272_device::scsi_phase_match()
{
	const u8 bus = scsi_bus_signals();
	return BIT(bus, 2) == BIT(m_scsi.phase, 0) && BIT(bus, 3) == BIT(m_scsi.phase, 1) && BIT(bus, 4) == BIT(m_scsi.phase, 2);
}

void huc6272_device::scsi_set_ack(bool state)
{
	m_scsi.ack = state;
	m_scsibus->write_ack(state ? 1 : 0);
}

// the data bus is only driven when "assert data bus" is set, or by a DMA send
void huc6272_device::scsi_update_data_bus()
{
	if (m_scsi.dma_send)
		m_scsi_data_out->write(m_scsi.data_cache);
	else
		m_scsi_data_out->write(BIT(m_scsi.ctrl, 0) ? m_scsi.data : 0);
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
		m_dma.latch = data;
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
					m_scsi.data_cache = m_scsi_data_in->read();
					if (BIT(m_dma.ctrl, 0))
					{
						m_scsi.drq = false;
						dma_write_byte(m_scsi.data_cache);
						scsi_set_ack(true);
					}
				}
			}
			else if (m_scsi.ack && !req)
				scsi_set_ack(false);
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
				scsi_set_ack(false);
		}
	}

	m_in_scsi_update = false;
}

u32 huc6272_device::scsi_data_r(offs_t offset)
{
	return m_scsi_data_in->read();
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
		m_scsibus->write_atn(BIT(data, 1));
		m_scsibus->write_sel(BIT(data, 2));
		scsi_set_ack(BIT(data, 4));
	}
	scsi_update_data_bus();
	m_scsibus->write_rst(BIT(data, 7));
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
		const u8 cmd = m_scsi_cmd_in->read();
		res |= BIT(cmd, 4) ? 0x01 : 0;
		res |= BIT(cmd, 1) ? 0x02 : 0;
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

// TODO: CD sub-channel data, needs a hookup to the drive's Q/P-W stream
u32 huc6272_device::scsi_subchannel_r(offs_t offset)
{
	return 0;
}

void huc6272_device::scsi_subchannel_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_scsi.subchannel_ctrl = data & 3;
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
		m_kram_addr_r += util::sext<s32>(m_kram_inc_r & 0x3ff, 10);
		m_kram_addr_r &= 0x3ffff;
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
	m_kram_addr_w += util::sext<s32>(m_kram_inc_w & 0x3ff, 10);
	m_kram_addr_w &= 0x3ffff;
}

/*
 * ---x ---- ---- ---- ADPCM page setting
 * ---- ---x ---- ---- RAINBOW page setting
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
void huc6272_device::bg_mode_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		for(int i = 0; i < 2; i++)
			m_bg[i].mode = (data >> i * 4) & 0x0f;
	}

	if (ACCESSING_BITS_8_15)
	{
		for(int i = 2; i < 4; i++)
			m_bg[i].mode = (data >> i * 4) & 0x0f;
	}
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
 * NB: there's another priority reg in '6261, is above layer vs. layer priority?
 */
void huc6272_device::bg_priority_w(offs_t offset, u32 data, u32 mem_mask)
{
	// TODO: fix access
	// TODO: rotation enable

	if (ACCESSING_BITS_0_15)
	{
		for(int i = 0; i < 4; i++)
			m_bg[i].priority = (data >> i * 3) & 0x07;
	}
}

void huc6272_device::microprogram_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_micro_prg.index = data & 0xf;
}

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

// TODO: limit?
template <unsigned N> void huc6272_device::bg_bat_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_bg[N].bat_address = data * 1024;
}

template <unsigned N> void huc6272_device::bg_cg_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_bg[N].cg_address = data * 1024;
}

void huc6272_device::bg0sub_bat_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_bg0sub.bat_address = data * 1024;
}

void huc6272_device::bg0sub_cg_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_bg0sub.cg_address = data * 1024;
}

void huc6272_device::bg_size_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_bg[offset].height = 1 << (data & 0x000f);
		m_bg[offset].width = 1 << ((data & 0x00f0) >> 4);
	}

	if(offset == 0 && ACCESSING_BITS_8_15)
	{
		m_bg0sub.height = 1 << ((data & 0x0f00) >> 8);
		m_bg0sub.width = 1 << ((data & 0xf000) >> 12);
	}
}

void huc6272_device::bg_scroll_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
	{
		const u8 layer_n = (offset & 6) >> 1;

		if(offset & 1)
			m_bg[layer_n].yscroll = data & 0xffff;
		else
			m_bg[layer_n].xscroll = data & 0xffff;
	}
}

void huc6272_device::adpcm_control_w(offs_t offset, u32 data, u32 mem_mask)
{
	for (int i = 0; i < 2; i++)
	{
		m_adpcm.playing[i] = BIT(data, i);
		if (!m_adpcm.playing[i])
		{
			m_adpcm.input[i] = 0;
			m_adpcm.pos[i] = 0;
			m_adpcm.nibble[i] = 32;
		}
		else
		{
			m_adpcm.addr[i] = m_adpcm.start[i];
		}
	}

	m_adpcm.rate = (data & 0xc) >> 2;
}

void huc6272_device::adpcm_channel_control_w(offs_t offset, u32 data, u32 mem_mask)
{
	const u8 channel = 1 - (offset & 1);
	m_adpcm.control[channel] = data & 0x7;
	if (BIT(m_adpcm.control[channel], 1) == 0)
		m_adpcm.status &= ~(1 << (2 * channel));

	if (BIT(m_adpcm.control[channel], 2) == 0)
		m_adpcm.status &= ~(1 << (2 * channel + 1));
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

// TODO: verify mask ranges
template <unsigned N> void huc6272_device::adpcm_start_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_adpcm.start[N] = (data * 256) & 0x3ffff;
}

template <unsigned N> void huc6272_device::adpcm_end_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_adpcm.end[N] = data & 0x3ffff;
}

template <unsigned N> void huc6272_device::adpcm_imm_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_adpcm.imm[N] = (data * 64) & 0x3ffff;
}

// TODO: verify me
// (interrupt_update fns are untested by the BIOS main menu)
u8 huc6272_device::adpcm_update(int chan)
{
	if (!m_adpcm.playing[chan])
		return 0;

	const unsigned rate = (1 << m_adpcm.rate);
	m_adpcm.pos[chan]++;
	if (m_adpcm.pos[chan] >= rate)
	{
		m_adpcm.nibble[chan] += 4;
		if (m_adpcm.nibble[chan] >= 32)
			m_adpcm.nibble[chan] = 0;

		if (m_adpcm.nibble[chan] == 0)
		{
			const u32 offset = ((m_page_setting & 0x1000) << 6) | (m_adpcm.addr[chan] << 0);
			m_adpcm.input[chan] = (space(AS_DATA).read_word(offset) << 0) | (space(AS_DATA).read_word(offset + 1)) << 16;
			m_adpcm.addr[chan] = (m_adpcm.addr[chan] & 0x20000) | ((m_adpcm.addr[chan] + 2) & 0x1ffff);
			if (m_adpcm.addr[chan] == m_adpcm.imm[chan])
			{
				m_adpcm.status |= (1 << (chan*2+1));
				if (BIT(m_adpcm.control[chan], 2))
				{
					m_adpcm.interrupt = 1;
					interrupt_update();
				}
			}
			if (m_adpcm.addr[chan] > m_adpcm.end[chan])
			{
				m_adpcm.status |= (1 << (chan*2));
				if (BIT(m_adpcm.control[chan], 1))
				{
					m_adpcm.interrupt = 1;
					interrupt_update();
				}

				if (BIT(m_adpcm.control[chan], 0)) // Ring Buffer
				{
					m_adpcm.addr[chan] = m_adpcm.start[chan];
				}
				else
				{
					m_adpcm.playing[chan] = 0;
					return 0;
				}
			}
			//m_adpcm.nibble[chan] = 0;
		}

		m_adpcm.pos[chan] = 0;
	}

	return (m_adpcm.input[chan] >> m_adpcm.nibble[chan]) & 0xf;
}

u8 huc6272_device::adpcm_update_0()
{
	return adpcm_update(0);
}

u8 huc6272_device::adpcm_update_1()
{
	return adpcm_update(1);
}

void huc6272_device::cdda_update(offs_t offset, u8 data)
{
	m_cdda->set_input_gain(offset, float(data & 0x3f) / 63.0);
}

void huc6272_device::interrupt_update()
{
	const bool state = m_adpcm.interrupt || m_scsi.irq || (m_dma.irq && BIT(m_dma.ctrl, 1));
	m_irq_changed_cb(state ? ASSERT_LINE : CLEAR_LINE);
}
