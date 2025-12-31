// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Angelo Salese
/**************************************************************************************************

Hudson/NEC HuC6272 "King" device

TODO:
- Use NSCSI instead of legacy one;
\- SEL acknowledges with 0x84, bit 7 controller type is unknown at this time
  (bit 2 should select the CD drive);
- Convert base mapping to address_map;
- Convert I/O to space address, and make it honor mem_mask;
- subclass "SCSICD" into SCSI-2 "CD-ROM DRIVE:FX"
  \- Fails detection of PC-FX discs, detects as normal audio CDs;
  \- During POST it tries an unhandled 0x44 "Read Header";
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

huc6272_device::huc6272_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HUC6272, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_huc6271(*this, finder_base::DUMMY_TAG)
	, m_cdda_l(*this, "cdda_l")
	, m_cdda_r(*this, "cdda_r")
	, m_program_space_config("microprg", ENDIANNESS_LITTLE, 16, 4, 0, address_map_constructor(FUNC(huc6272_device::microprg_map), this))
	, m_data_space_config("kram", ENDIANNESS_LITTLE, 16, 19, -1, address_map_constructor(FUNC(huc6272_device::kram_map), this))
	, m_io_space_config("io", ENDIANNESS_LITTLE, 32, 7, -2, address_map_constructor(FUNC(huc6272_device::io_map), this))
	, m_microprg_ram(*this, "microprg_ram")
	, m_kram_page0(*this, "kram_page0")
	, m_kram_page1(*this, "kram_page1")
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
	cdda->add_route(0, "^^cdda_l", 1.0);
	cdda->add_route(1, "^^cdda_r", 1.0);
}

void huc6272_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, m_cdda_l).front_left();
	SPEAKER(config, m_cdda_r).front_right();

	scsi_port_device &scsibus(SCSI_PORT(config, "scsi"));
	scsibus.set_data_input_buffer("scsi_data_in");
	scsibus.rst_handler().set("scsi_ctrl_in", FUNC(input_buffer_device::write_bit7));
	scsibus.bsy_handler().set("scsi_ctrl_in", FUNC(input_buffer_device::write_bit6));
	scsibus.req_handler().set("scsi_ctrl_in", FUNC(input_buffer_device::write_bit5));
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
		map(0x00, 0x0f).ram().share("microprg_ram");
}

//**************************************************************************
//  Memory maps
//**************************************************************************

void huc6272_device::kram_map(address_map &map)
{
	if (!has_configured_map(1))
	{
		map(0x000000, 0x03ffff).ram().share("kram_page0");
		map(0x040000, 0x07ffff).ram().share("kram_page1");
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
	map(0x01, 0x01).rw(FUNC(huc6272_device::scsi_cmd_status_r), FUNC(huc6272_device::scsi_initiate_cmd_w));
//  map(0x02, 0x02) SCSI DMA mode
	map(0x03, 0x03).w(FUNC(huc6272_device::scsi_target_cmd_w));
	map(0x05, 0x05).rw(FUNC(huc6272_device::scsi_bus_r), FUNC(huc6272_device::scsi_bus_w));
//  map(0x06, 0x06) SCSI input data
//  map(0x07, 0x07) SCSI DMA trigger
//  map(0x08, 0x08) SCSI subcode
//  map(0x09, 0x09) SCSI DMA start address
//  map(0x0a, 0x0a) SCSI DMA size
//  map(0x0b, 0x0b) SCSI DMA control
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


void huc6272_device::write_microprg_data(offs_t address, uint16_t data)
{
	space(AS_PROGRAM).write_word(address << 1, data);
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
	// TODO: other IRQs
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
	u16 res = (m_scsi_ctrl_in->read() & 0xff);
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

u32 huc6272_device::scsi_data_r(offs_t offset)
{
	return m_scsi_data_in->read() & 0xff;
}

void huc6272_device::scsi_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_scsi_data_out->write(data & 0xff);
}

u32 huc6272_device::scsi_cmd_status_r(offs_t offset)
{
	return m_scsi_cmd_in->read() & 0xff;
}

void huc6272_device::scsi_initiate_cmd_w(offs_t offset, u32 data, u32 mem_mask)
{
	//m_scsibus->write_bsy(BIT(data, 0)); // bus?
	m_scsibus->write_atn(BIT(data, 1));
	m_scsibus->write_sel(BIT(data, 2));
	m_scsibus->write_ack(BIT(data, 4));
	m_scsibus->write_rst(BIT(data, 7));
}

void huc6272_device::scsi_target_cmd_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_scsibus->write_io(BIT(data, 0));
	m_scsibus->write_cd(BIT(data, 1));
	m_scsibus->write_msg(BIT(data, 2)); // Misc?
}

u32 huc6272_device::scsi_bus_r(offs_t offset)
{
	u32 res = m_scsi_ctrl_in->read() & 0xff;
	res |= m_scsi_data_in->read() << 16;
	return res;
}

void huc6272_device::scsi_bus_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
	{
		// TODO: bits 7-0: SCSI DMA trigger?
		LOG("SCSI DMA trigger %04x & %08x\n", data, mem_mask);
	}

	if (ACCESSING_BITS_16_23)
		m_scsi_data_out->write((data >> 16) & 0xff);
}

u32 huc6272_device::kram_read_address_r(offs_t offset)
{
	return (m_kram_addr_r & 0x3ffff)
		| ((m_kram_inc_r & 0x1ff) << 18)
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
		| ((m_kram_inc_w & 0x1ff) << 18)
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
	u16 res = space(AS_DATA).read_word(((m_kram_addr_r) | (m_kram_page_r << 18)) << 0, mem_mask);
	if (!machine().side_effects_disabled())
	{
		m_kram_addr_r += (m_kram_inc_r & 0x200)
			? ((m_kram_inc_r & 0x1ff) - 0x200)
			: (m_kram_inc_r & 0x1ff);
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
	m_kram_addr_w += (m_kram_inc_w & 0x200)
		? ((m_kram_inc_w & 0x1ff) - 0x200)
		: (m_kram_inc_w & 0x1ff);
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
		uint8_t layer_n = (offset & 6) >> 1;

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
	uint8_t channel = 1 - (offset & 1);
	m_adpcm.control[channel] = data & 0x7;
	if (BIT(m_adpcm.control[channel], 1) == 0)
		m_adpcm.status &= ~(1 << (2 * channel));

	if (BIT(m_adpcm.control[channel], 2) == 0)
		m_adpcm.status &= ~(1 << (2 * channel + 1));
}

u32 huc6272_device::adpcm_status_r(offs_t offset)
{
	u32 res = m_adpcm.status;

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
uint8_t huc6272_device::adpcm_update(int chan)
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

uint8_t huc6272_device::adpcm_update_0()
{
	return adpcm_update(0);
}

uint8_t huc6272_device::adpcm_update_1()
{
	return adpcm_update(1);
}

void huc6272_device::cdda_update(offs_t offset, uint8_t data)
{
	if (offset)
		m_cdda_r->set_input_gain(0, float(data & 0x3f) / 63.0);
	else
		m_cdda_l->set_input_gain(0, float(data & 0x3f) / 63.0);
}

void huc6272_device::interrupt_update()
{
	m_irq_changed_cb(m_adpcm.interrupt ? ASSERT_LINE : CLEAR_LINE);
}
