// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

VT82C586B PCIC ISA section

TODO:
- win98se: PS/2 keyboard becomes unresponsive after a while during install;

**************************************************************************************************/

#include "emu.h"
#include "vt82c586b_isa.h"

#include "bus/pc_kbd/keyboards.h"
#include "speaker.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

DEFINE_DEVICE_TYPE(VT82C586B_ISA, vt82c586b_isa_device, "vt82c586b_isa", "VT82C586B \"PIPC\" PCI-to-ISA bridge")

vt82c586b_isa_device::vt82c586b_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, VT82C586B_ISA, tag, owner, clock)
//  , m_smi_callback(*this)
//  , m_nmi_callback(*this)
//  , m_stpclk_callback(*this)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
	, m_pic(*this, "pic%u", 0U)
	, m_dma(*this, "dma%u", 0U)
	, m_pit(*this, "pit")
	, m_keybc(*this, "keybc")
	, m_ps2_con(*this, "ps2_con")
	, m_aux_con(*this, "aux_con")
	, m_rtc(*this, "rtc")
	, m_isabus(*this, "isabus")
	, m_speaker(*this, "speaker")
	, m_write_a20m(*this)
	, m_write_cpureset(*this)
	, m_write_pcirst(*this)
	, m_boot_state_hook(*this)
{
}

void vt82c586b_isa_device::device_add_mconfig(machine_config &config)
{
	PIT8254(config, m_pit);
	m_pit->set_clk<0>(4772720 / 4); // heartbeat IRQ
	m_pit->out_handler<0>().set(FUNC(vt82c586b_isa_device::at_pit8254_out0_changed));
	m_pit->set_clk<1>(4772720 / 4); // DRAM refresh
	m_pit->out_handler<1>().set(FUNC(vt82c586b_isa_device::at_pit8254_out1_changed));
	m_pit->set_clk<2>(4772720 / 4); // PIO port C pin 4, and speaker polling enough
	m_pit->out_handler<2>().set(FUNC(vt82c586b_isa_device::at_pit8254_out2_changed));

	// 8237-equivalent
	AM9517A(config, m_dma[0], XTAL(14'318'181) / 3);
	m_dma[0]->out_hreq_callback().set(m_dma[1], FUNC(am9517a_device::dreq0_w));
	m_dma[0]->out_eop_callback().set(FUNC(vt82c586b_isa_device::at_dma8237_out_eop));
	m_dma[0]->in_memr_callback().set(FUNC(vt82c586b_isa_device::pc_dma_read_byte));
	m_dma[0]->out_memw_callback().set(FUNC(vt82c586b_isa_device::pc_dma_write_byte));
	m_dma[0]->in_ior_callback<0>().set(FUNC(vt82c586b_isa_device::pc_dma8237_0_dack_r));
	m_dma[0]->in_ior_callback<1>().set(FUNC(vt82c586b_isa_device::pc_dma8237_1_dack_r));
	m_dma[0]->in_ior_callback<2>().set(FUNC(vt82c586b_isa_device::pc_dma8237_2_dack_r));
	m_dma[0]->in_ior_callback<3>().set(FUNC(vt82c586b_isa_device::pc_dma8237_3_dack_r));
	m_dma[0]->out_iow_callback<0>().set(FUNC(vt82c586b_isa_device::pc_dma8237_0_dack_w));
	m_dma[0]->out_iow_callback<1>().set(FUNC(vt82c586b_isa_device::pc_dma8237_1_dack_w));
	m_dma[0]->out_iow_callback<2>().set(FUNC(vt82c586b_isa_device::pc_dma8237_2_dack_w));
	m_dma[0]->out_iow_callback<3>().set(FUNC(vt82c586b_isa_device::pc_dma8237_3_dack_w));
	m_dma[0]->out_dack_callback<0>().set(FUNC(vt82c586b_isa_device::pc_dack0_w));
	m_dma[0]->out_dack_callback<1>().set(FUNC(vt82c586b_isa_device::pc_dack1_w));
	m_dma[0]->out_dack_callback<2>().set(FUNC(vt82c586b_isa_device::pc_dack2_w));
	m_dma[0]->out_dack_callback<3>().set(FUNC(vt82c586b_isa_device::pc_dack3_w));

	AM9517A(config, m_dma[1], XTAL(14'318'181) / 3);
	m_dma[1]->out_hreq_callback().set(FUNC(vt82c586b_isa_device::pc_dma_hrq_changed));
	m_dma[1]->in_memr_callback().set(FUNC(vt82c586b_isa_device::pc_dma_read_word));
	m_dma[1]->out_memw_callback().set(FUNC(vt82c586b_isa_device::pc_dma_write_word));
	m_dma[1]->in_ior_callback<1>().set(FUNC(vt82c586b_isa_device::pc_dma8237_5_dack_r));
	m_dma[1]->in_ior_callback<2>().set(FUNC(vt82c586b_isa_device::pc_dma8237_6_dack_r));
	m_dma[1]->in_ior_callback<3>().set(FUNC(vt82c586b_isa_device::pc_dma8237_7_dack_r));
	m_dma[1]->out_iow_callback<1>().set(FUNC(vt82c586b_isa_device::pc_dma8237_5_dack_w));
	m_dma[1]->out_iow_callback<2>().set(FUNC(vt82c586b_isa_device::pc_dma8237_6_dack_w));
	m_dma[1]->out_iow_callback<3>().set(FUNC(vt82c586b_isa_device::pc_dma8237_7_dack_w));
	m_dma[1]->out_dack_callback<0>().set(FUNC(vt82c586b_isa_device::pc_dack4_w));
	m_dma[1]->out_dack_callback<1>().set(FUNC(vt82c586b_isa_device::pc_dack5_w));
	m_dma[1]->out_dack_callback<2>().set(FUNC(vt82c586b_isa_device::pc_dack6_w));
	m_dma[1]->out_dack_callback<3>().set(FUNC(vt82c586b_isa_device::pc_dack7_w));

	// 8259A-equivalent
	PIC8259(config, m_pic[0]);
	m_pic[0]->out_int_callback().set_inputline(m_host_cpu, 0);
	m_pic[0]->in_sp_callback().set_constant(1);
	m_pic[0]->read_slave_ack_callback().set(FUNC(vt82c586b_isa_device::get_slave_ack));

	PIC8259(config, m_pic[1]);
	m_pic[1]->out_int_callback().set(m_pic[0], FUNC(pic8259_device::ir2_w));
	m_pic[1]->in_sp_callback().set_constant(0);

	// TODO: VT82887
	DS12885EXT(config, m_rtc, XTAL(32'768));
	m_rtc->irq().set(m_pic[1], FUNC(pic8259_device::ir0_w));
	m_rtc->set_century_index(0x32);

	// TODO: VT82C42, clock configurable
	PS2_KEYBOARD_CONTROLLER(config, m_keybc, DERIVED_CLOCK(1, 4));
	m_keybc->set_default_bios_tag("compaq");
	m_keybc->hot_res().set([this] (int state) { m_host_cpu->set_input_line(INPUT_LINE_RESET, state); });
	m_keybc->gate_a20().set(FUNC(vt82c586b_isa_device::keyboard_gatea20));
	m_keybc->kbd_irq().set(m_pic[0], FUNC(pic8259_device::ir1_w));
	m_keybc->kbd_clk().set(m_ps2_con, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_keybc->kbd_data().set(m_ps2_con, FUNC(pc_kbdc_device::data_write_from_mb));
	m_keybc->aux_irq().set(m_pic[1], FUNC(pic8259_device::ir4_w));
	m_keybc->aux_clk().set(m_aux_con, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_keybc->aux_data().set(m_aux_con, FUNC(pc_kbdc_device::data_write_from_mb));

	PC_KBDC(config, m_ps2_con, pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL);
	m_ps2_con->out_clock_cb().set(m_keybc, FUNC(ps2_keyboard_controller_device::kbd_clk_w));
	m_ps2_con->out_data_cb().set(m_keybc, FUNC(ps2_keyboard_controller_device::kbd_data_w));

	// TODO: verify me, probably doesn't work
	PC_KBDC(config, m_aux_con, ps2_mice, nullptr);
	m_aux_con->out_clock_cb().set(m_keybc, FUNC(ps2_keyboard_controller_device::aux_clk_w));
	m_aux_con->out_data_cb().set(m_keybc, FUNC(ps2_keyboard_controller_device::aux_data_w));

	ISA16(config, m_isabus, 0);
	m_isabus->irq3_callback().set(FUNC(vt82c586b_isa_device::pc_irq3_w));
	m_isabus->irq4_callback().set(FUNC(vt82c586b_isa_device::pc_irq4_w));
	m_isabus->irq5_callback().set(FUNC(vt82c586b_isa_device::pc_irq5_w));
	m_isabus->irq6_callback().set(FUNC(vt82c586b_isa_device::pc_irq6_w));
	m_isabus->irq7_callback().set(FUNC(vt82c586b_isa_device::pc_irq7_w));
	m_isabus->irq2_callback().set(FUNC(vt82c586b_isa_device::pc_irq9_w));
	m_isabus->irq10_callback().set(FUNC(vt82c586b_isa_device::pc_irq10_w));
	m_isabus->irq11_callback().set(FUNC(vt82c586b_isa_device::pc_irq11_w));
	m_isabus->irq12_callback().set(FUNC(vt82c586b_isa_device::pc_irq12m_w));
	m_isabus->irq14_callback().set(FUNC(vt82c586b_isa_device::pc_irq14_w));
	m_isabus->irq15_callback().set(FUNC(vt82c586b_isa_device::pc_irq15_w));
	m_isabus->drq0_callback().set(m_dma[0], FUNC(am9517a_device::dreq0_w));
	m_isabus->drq1_callback().set(m_dma[0], FUNC(am9517a_device::dreq1_w));
	m_isabus->drq2_callback().set(m_dma[0], FUNC(am9517a_device::dreq2_w));
	m_isabus->drq3_callback().set(m_dma[0], FUNC(am9517a_device::dreq3_w));
	m_isabus->drq5_callback().set(m_dma[1], FUNC(am9517a_device::dreq1_w));
	m_isabus->drq6_callback().set(m_dma[1], FUNC(am9517a_device::dreq2_w));
	m_isabus->drq7_callback().set(m_dma[1], FUNC(am9517a_device::dreq3_w));
	m_isabus->iochck_callback().set(FUNC(vt82c586b_isa_device::iochck_w));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
}

void vt82c586b_isa_device::device_config_complete()
{
	auto isabus = m_isabus.finder_target();
	isabus.first.subdevice<isa16_device>(isabus.second)->set_memspace(m_host_cpu, AS_PROGRAM);
	isabus.first.subdevice<isa16_device>(isabus.second)->set_iospace(m_host_cpu, AS_IO);

	pci_device::device_config_complete();
}

void vt82c586b_isa_device::device_start()
{
	pci_device::device_start();

	m_pci_root->set_pin_mapper(pci_pin_mapper(*this, FUNC(vt82c586b_isa_device::pin_mapper)));
	m_pci_root->set_irq_handler(pci_irq_handler(*this, FUNC(vt82c586b_isa_device::irq_handler)));

	save_item(NAME(m_isa_bus_control));
	save_item(NAME(m_isa_test_mode));
	save_item(NAME(m_isa_clock_control));
	save_item(NAME(m_rom_decode_control));
	save_item(NAME(m_keybc_control));
	save_item(NAME(m_dma_control_typef));
	save_item(NAME(m_misc_control));
	save_item(NAME(m_ide_irq_routing));
	save_item(NAME(m_xd_power_on));
	save_item(NAME(m_rtc_test_mode));
	save_item(NAME(m_dma_linebuffer_disable));

	save_item(NAME(m_nmi_enabled));
	save_item(NAME(m_port92));

	save_item(NAME(m_pirqrc));
	save_item(NAME(m_mirq));
	save_item(NAME(m_mirq_pin_config));
}

void vt82c586b_isa_device::device_reset()
{
	pci_device::device_reset();

	// Special Cycle Enable, Bus Master, Memory & I/O Space
	// TODO: has special meaning in test mode, otherwise not really writeable (always 1)
	command = 0x000f;
	command_mask = 0x000f;
	// medium DEVSEL#
	status = 0x0200;

	m_isa_bus_control = 0;
	m_isa_test_mode = 0;
	m_isa_clock_control = 0;
	m_rom_decode_control = 0;
	m_keybc_control = 0;
	m_dma_control_typef = 0;
	std::fill(std::begin(m_misc_control), std::end(m_misc_control), 0);
	m_ide_irq_routing = 0x00 | (1 << 2) | (0 << 0);
	std::fill(std::begin(m_pirqrc), std::end(m_pirqrc), 0);
	std::fill(std::begin(m_mirq), std::end(m_mirq), 0);
	m_mirq_pin_config = 0;

	// TODO: strapped
	m_xd_power_on = 0xb7;

	m_nmi_enabled = false;
	m_port92 = 0;
	remap_cb();
}

void vt82c586b_isa_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x40, 0x40).lrw8(
		NAME([this] () { return m_isa_bus_control; }),
		NAME([this] (offs_t offset, u8 data) {
			m_isa_bus_control = data;
			LOG("40h: ISA Bus Control %02x\n", data);
		})
	);
	map(0x41, 0x41).lrw8(
		NAME([this] () { return m_isa_test_mode; }),
		NAME([this] (offs_t offset, u8 data) {
			m_isa_test_mode = data;
			LOG("41h: ISA Test Mode %02x\n", data);
			remap_cb();
			// TODO: bit 3 for Double DMA clock
		})
	);
	map(0x42, 0x42).lrw8(
		NAME([this] () { return m_isa_clock_control; }),
		NAME([this] (offs_t offset, u8 data) {
			m_isa_clock_control = data & 0xaf;
			LOG("42h: ISA Clock Control %02x\n", data);
			// TODO: reprogram ISA bus clock
		})
	);
	map(0x43, 0x43).lrw8(
		NAME([this] () { return m_rom_decode_control; }),
		NAME([this] (offs_t offset, u8 data) {
			m_rom_decode_control = data & 0xaf;
			LOG("43h: ROM Decode Control %02x\n", data);
			// TODO: remap cb here
		})
	);
	map(0x44, 0x44).lrw8(
		NAME([this] () { return m_keybc_control; }),
		NAME([this] (offs_t offset, u8 data) {
			m_keybc_control = data & 0x88;
			LOG("44h: Keyboard Controller Control %02x\n", data);
		})
	);
	map(0x45, 0x45).lrw8(
		NAME([this] () { return m_dma_control_typef; }),
		NAME([this] (offs_t offset, u8 data) {
			m_dma_control_typef = data;
			LOG("45h: Type F DMA Control %02x\n", data);
		})
	);
	map(0x46, 0x46).lrw8(
		NAME([this] () { return m_misc_control[0]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_misc_control[0] = data & 0xf3;
			LOG("46h: Miscellaneous Control 1 %02x\n", data);
		})
	);
	// bit 5 EISA $4d0/$4d1 enable
	// bit 3 <reserved> but can be written to
	// bit 0 PCI reset
	map(0x47, 0x47).lrw8(
		NAME([this] () { return m_misc_control[1]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_misc_control[1] = data;
			LOG("47h: Miscellaneous Control 2 %02x\n", data);
			// after saving to CMOS
			if (BIT(data, 0))
			{
				m_write_pcirst(1);
				m_write_pcirst(0);
			}

			// TODO: remap cb for EISA port
		})
	);
	// --x- ---- MASTER# Pin function (3041 silicon)
	// ---x ---- IRQ8# source
	// ---- x--- Extra RTC ports $74/$75
	// ---- -x-- Integrated USB Controller Disable
	// ---- --x- Integrated IDE Controller Disable
	// ---- ---x 512K PCI Memory Decode
	map(0x48, 0x48).lrw8(
		NAME([this] () { return m_misc_control[2]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_misc_control[2] = data & 0x3f;
			LOG("48h: Miscellaneous Control 3 %02x\n", data);
			// TODO: remap cb for bit 3
		})
	);
	map(0x4a, 0x4a).lrw8(
		NAME([this] () { return m_ide_irq_routing; }),
		NAME([this] (offs_t offset, u8 data) {
			m_ide_irq_routing = data;
			LOG("4Ah: IDE Interrupt Routing %02x\n", data);
		})
	);
	map(0x4c, 0x4c).lrw8(
		NAME([this] () { return m_pci_memory_hole_bottom; }),
		NAME([this] (offs_t offset, u8 data) {
			m_pci_memory_hole_bottom = data;
			LOG("4Ch: ISA DMA/Master Memory Access Control 1 (Bottom) %02x\n", data << 16);
		})
	);
	map(0x4d, 0x4d).lrw8(
		NAME([this] () { return m_pci_memory_hole_top; }),
		NAME([this] (offs_t offset, u8 data) {
			m_pci_memory_hole_top = data;
			LOG("4Dh: ISA DMA/Master Memory Access Control 2 (Top) %02x\n", data << 16);
		})
	);
	map(0x4e, 0x4f).lrw16(
		NAME([this] () { return m_pci_memory_access_control_3; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_pci_memory_access_control_3);
			LOG("4Eh: ISA DMA/Master Memory Access Control 3 %04x & %04x\n", data, mem_mask);
		})
	);

	// PnP Control
	// port 50 reserved (default 0x04)
	map(0x54, 0x54).lrw8(
		NAME([this] () { return m_pirq_select; }),
		NAME([this] (offs_t offset, u8 data) {
			m_pirq_select = data & 0xf;
			LOG("54h: PCI IRQ Edge / Level Select\n");
			// TODO: unclear what this really does (edge would invert the line?)
			LOG("\tPIRQA# %s\n", BIT(data, 3) ? "edge" : "level");
			LOG("\tPIRQB# %s\n", BIT(data, 2) ? "edge" : "level");
			LOG("\tPIRQC# %s\n", BIT(data, 1) ? "edge" : "level");
			LOG("\tPIRQD# %s\n", BIT(data, 0) ? "edge" : "level");
		})
	);
	map(0x55, 0x55).lrw8(
		NAME([this] () { return (m_pirqrc[3] << 4) | (m_mirq[0]); }),
		NAME([this] (offs_t offset, u8 data) {
			m_pirqrc[3] = (data & 0xf0) >> 4;
			m_mirq[0] = data & 0xf;
			LOG("55h: PnP IRQ Routing 1 PIRQD# %d MIRQ0 %d\n", m_pirqrc[3], m_mirq[0]);
		})
	);
	map(0x56, 0x56).lrw8(
		NAME([this] () { return (m_pirqrc[0] << 4) | (m_pirqrc[1]); }),
		NAME([this] (offs_t offset, u8 data) {
			m_pirqrc[0] = (data & 0xf0) >> 4;
			m_pirqrc[1] = data & 0xf;
			LOG("56h: PnP IRQ Routing 2 PIRQA# %d PIRQB# %d\n", m_pirqrc[0], m_pirqrc[1]);
		})
	);
	map(0x57, 0x57).lrw8(
		NAME([this] () { return (m_pirqrc[2] << 4) | (m_mirq[1]); }),
		NAME([this] (offs_t offset, u8 data) {
			m_pirqrc[2] = (data & 0xf0) >> 4;
			m_mirq[1] = data & 0xf;
			LOG("57h: PnP IRQ Routing 3 PIRQC# %d MIRQ1 %d\n", m_pirqrc[2], m_mirq[1]);
		})
	);
	map(0x58, 0x58).lrw8(
		NAME([this] () { return m_mirq[2]; }),
		NAME([this] (offs_t offset, u8 data) {
			m_mirq[2] = data & 0xf;
			LOG("58h: PnP IRQ Routing 4 MIRQ2 %d\n", m_mirq[2]);
		})
	);
	map(0x59, 0x59).lrw8(
		NAME([this] () { return m_mirq_pin_config; }),
		NAME([this] (offs_t offset, u8 data) {
			m_mirq_pin_config = data & 0xf;
			LOG("59h: MIRQ Pin configuration %02x\n", data);
		})
	);
	map(0x5a, 0x5a).lrw8(
		NAME([this] () { return m_xd_power_on; }),
		NAME([this] (offs_t offset, u8 data) {
			// TODO: XD0 ~ XD2 / XD4 ~ XD7 strap configurations
			m_xd_power_on = data;
			remap_cb();
		})
	);
	map(0x5b, 0x5b).lrw8(
		NAME([this] () { return m_rtc_test_mode; }),
		NAME([this] (offs_t offset, u8 data) {
			m_rtc_test_mode = data & 7;
			LOG("5Bh: Internal RTC Test Mode %02x\n", data);
		})
	);
	// NOTE: on 3041 Silicon Only
	map(0x5c, 0x5c).lrw8(
		NAME([this] () { return m_dma_linebuffer_disable; }),
		NAME([this] (offs_t offset, u8 data) {
			m_dma_linebuffer_disable = BIT(data, 0);
			LOG("5Ch: DMA Control %02x\n", data);
		})
	);
	map(0x60, 0x6f).lrw16(
		NAME([this] (offs_t offset) -> u16 {
			if (offset == 4)
				return 0;
			return m_ddma_control[offset]; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			if (offset == 4)
				return;
			COMBINE_DATA(&m_ddma_control[offset]);
			m_ddma_control[offset] &= 0xfff8;
			LOG("%02X: Distributed DMA Ch. %d %04x & %04x -> Base %04x Enable %d\n"
				, (offset * 2) + 0x60
				, offset
				, data
				, mem_mask
				, m_ddma_control[offset] >> 4
				, BIT(m_ddma_control[offset], 3)
			);
		})
	);
	// 0x70 ~ 0x73 Subsystem ID
}

void vt82c586b_isa_device::internal_io_map(address_map &map)
{
	map(0x0000, 0x001f).rw(m_dma[0], FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x0021).rw(m_pic[0], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x005f).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
//  map(0x0060, 0x0060) keyboard
	map(0x0061, 0x0061).rw(FUNC(vt82c586b_isa_device::at_portb_r), FUNC(vt82c586b_isa_device::at_portb_w));
//  map(0x0064, 0x0064) keyboard
//  map(0x0070, 0x0073) RTC
	map(0x0080, 0x008f).rw(FUNC(vt82c586b_isa_device::at_page8_r), FUNC(vt82c586b_isa_device::at_page8_w));
	map(0x0092, 0x0092).rw(FUNC(vt82c586b_isa_device::port92_r), FUNC(vt82c586b_isa_device::port92_w));
	map(0x00a0, 0x00a1).rw(m_pic[1], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x00c0, 0x00df).rw(FUNC(vt82c586b_isa_device::at_dma8237_2_r), FUNC(vt82c586b_isa_device::at_dma8237_2_w));

	// timestamp for this southbridge
	map(0x00eb, 0x00eb).lw8(NAME([] (offs_t offset, u8 data) { }));
}

void vt82c586b_isa_device::map_bios(address_space *memory_space, uint32_t start, uint32_t end)
{
	uint32_t mask = m_region->bytes() - 1;
	memory_space->install_rom(start, end, m_region->base() + (start & mask));
}

void vt82c586b_isa_device::map_extra(
		uint64_t memory_window_start,
		uint64_t memory_window_end,
		uint64_t memory_offset,
		address_space *memory_space,
		uint64_t io_window_start,
		uint64_t io_window_end,
		uint64_t io_offset,
		address_space *io_space)
{
	// assume that map_extra of the southbridge is called before the one of the northbridge
	m_isabus->remap(AS_PROGRAM, 0, 1 << 24);
	// TODO: really dictated by ROM Decode Control
	map_bios(memory_space, 0xffffffff - m_region->bytes() + 1, 0xffffffff);
	map_bios(memory_space, 0x000e0000, 0x000fffff);
	m_isabus->remap(AS_IO, 0, 0xffff);
	io_space->install_device(0, 0xffff, *this, &vt82c586b_isa_device::internal_io_map);

	// Internal PS/2
	// TODO: BIT 1 enables internal AUX too
	if (BIT(m_xd_power_on, 0))
	{
		m_isabus->install_device(0x60, 0x60, read8smo_delegate(*m_keybc, FUNC(ps2_keyboard_controller_device::data_r)), write8smo_delegate(*m_keybc, FUNC(ps2_keyboard_controller_device::data_w)));
		m_isabus->install_device(0x64, 0x64, read8smo_delegate(*m_keybc, FUNC(ps2_keyboard_controller_device::status_r)), write8smo_delegate(*m_keybc, FUNC(ps2_keyboard_controller_device::command_w)));
	}

	// Internal RTC
	if (BIT(m_xd_power_on, 2))
	{
		io_space->install_readwrite_handler(0x70, 0x70,
			read8sm_delegate(*this, FUNC(vt82c586b_isa_device::rtc_index_r<0>)),
			write8sm_delegate(*this, FUNC(vt82c586b_isa_device::rtc_index_w<0>))
		);
		io_space->install_readwrite_handler(0x71, 0x71,
			read8sm_delegate(*this, FUNC(vt82c586b_isa_device::rtc_data_r<0>)),
			write8sm_delegate(*this, FUNC(vt82c586b_isa_device::rtc_data_w<0>))
		);
		io_space->install_readwrite_handler(0x72, 0x72,
			read8sm_delegate(*this, FUNC(vt82c586b_isa_device::rtc_index_r<1>)),
			write8sm_delegate(*this, FUNC(vt82c586b_isa_device::rtc_index_w<1>))
		);
		io_space->install_readwrite_handler(0x73, 0x73,
			read8sm_delegate(*this, FUNC(vt82c586b_isa_device::rtc_data_r<1>)),
			write8sm_delegate(*this, FUNC(vt82c586b_isa_device::rtc_data_w<1>))
		);
		// TODO: ports $74-$75
	}
}

template <unsigned E> u8 vt82c586b_isa_device::rtc_index_r(offs_t offset)
{
	if (E)
		return m_rtc_index;

	// supposedly w/o but ls5amvp3 still reads it for timestamp needs
	return m_rtc_index & 0x7f;
}

template <unsigned E> void vt82c586b_isa_device::rtc_index_w(offs_t offset, u8 data)
{
	m_rtc_index = data & 0x7f;
	// NOTE: active low
	if (!E)
	{
		m_nmi_enabled = BIT(~data, 7);
		if (!m_nmi_enabled)
			m_host_cpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
	else
		m_rtc_index |= data & 0x80;
}

template <unsigned E> u8 vt82c586b_isa_device::rtc_data_r(offs_t offset)
{
	const u8 rtc_address = m_rtc_index & (E ? 0xff : 0x7f);
	return m_rtc->read_direct(rtc_address);
}

template <unsigned E> void vt82c586b_isa_device::rtc_data_w(offs_t offset, u8 data)
{
	const u8 rtc_address = m_rtc_index & (E ? 0xff : 0x7f);
	m_rtc->write_direct(rtc_address, data);
}

/*
 * xx-- ---- HDD Activity LED Status (0) off, any other setting On
 * ---- x--- Power-On Password Bytes Inaccessible
 * ---- --x- Fast A20
 * ---- ---x Fast Reset (if bit 5 of ISA Test Mode is also on)
 */
u8 vt82c586b_isa_device::port92_r(offs_t offset)
{
	return m_port92;
}

void vt82c586b_isa_device::port92_w(offs_t offset, u8 data)
{
	fast_gatea20(BIT(data, 1));

	if (!BIT(m_port92, 0) && BIT(data, 0) && BIT(m_isa_test_mode, 5))
	{
		// pulse reset line
		m_write_cpureset(1);
		m_write_cpureset(0);
	}

	m_port92 = data & 0xcb;
}

/*
 * Southbridge stuff
 *
 */

void vt82c586b_isa_device::pc_irq1_w(int state)   { m_pic[0]->ir1_w(state); }
void vt82c586b_isa_device::pc_irq3_w(int state)   { m_pic[0]->ir3_w(state); }
void vt82c586b_isa_device::pc_irq4_w(int state)   { m_pic[0]->ir4_w(state); }
void vt82c586b_isa_device::pc_irq5_w(int state)   { m_pic[0]->ir5_w(state); }
void vt82c586b_isa_device::pc_irq6_w(int state)   { m_pic[0]->ir6_w(state); }
void vt82c586b_isa_device::pc_irq7_w(int state)   { m_pic[0]->ir7_w(state); }
void vt82c586b_isa_device::pc_irq8n_w(int state)  { m_pic[1]->ir0_w(state); }
void vt82c586b_isa_device::pc_irq9_w(int state)   { m_pic[1]->ir1_w(state); }
void vt82c586b_isa_device::pc_irq10_w(int state)  { m_pic[1]->ir2_w(state); }
void vt82c586b_isa_device::pc_irq11_w(int state)  { m_pic[1]->ir3_w(state); }
void vt82c586b_isa_device::pc_irq12m_w(int state) { m_pic[1]->ir4_w(state); }
void vt82c586b_isa_device::pc_irq14_w(int state)  { m_pic[1]->ir6_w(state); }
void vt82c586b_isa_device::pc_irq15_w(int state)  { m_pic[1]->ir7_w(state); }

uint8_t vt82c586b_isa_device::get_slave_ack(offs_t offset)
{
	if (offset == 2) // IRQ = 2
		return m_pic[1]->acknowledge();

	return 0x00;
}

void vt82c586b_isa_device::at_pit8254_out0_changed(int state)
{
	if (m_pic[0])
		m_pic[0]->ir0_w(state);
}

void vt82c586b_isa_device::at_pit8254_out1_changed(int state)
{
	if (state)
		m_refresh = !m_refresh;
}

void vt82c586b_isa_device::at_pit8254_out2_changed(int state)
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}

void vt82c586b_isa_device::at_speaker_set_spkrdata(uint8_t data)
{
	m_at_spkrdata = data ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}

uint8_t vt82c586b_isa_device::at_portb_r()
{
	uint8_t data = m_at_speaker;

	data &= ~0xd0; /* AT BIOS don't likes this being set */
	/* 0x10 is the dram refresh line bit on the 5170, just a timer here, 15.085us. */
	data |= m_refresh ? 0x10 : 0;
	if (m_pit_out2)
		data |= 0x20;
	else
		data &= ~0x20; /* ps2m30 wants this */

	return data;
}

void vt82c586b_isa_device::at_portb_w(uint8_t data)
{
	m_at_speaker = data;
	m_pit->write_gate2(BIT(data, 0));
	at_speaker_set_spkrdata(BIT(data, 1));
	m_channel_check = BIT(data, 3);
	if (m_channel_check)
		m_host_cpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void vt82c586b_isa_device::iochck_w(int state)
{
	if (!state && !m_channel_check && m_nmi_enabled)
		m_host_cpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

uint8_t vt82c586b_isa_device::at_page8_r(offs_t offset)
{
	uint8_t data = m_at_pages[offset % 0x10];

	switch (offset % 8)
	{
	case 1:
		data = m_dma_offset[BIT(offset, 3)][2];
		break;
	case 2:
		data = m_dma_offset[BIT(offset, 3)][3];
		break;
	case 3:
		data = m_dma_offset[BIT(offset, 3)][1];
		break;
	case 7:
		data = m_dma_offset[BIT(offset, 3)][0];
		break;
	}
	return data;
}

void vt82c586b_isa_device::at_page8_w(offs_t offset, uint8_t data)
{
	m_at_pages[offset % 0x10] = data;

	switch (offset % 8)
	{
	case 0:
		m_boot_state_hook((offs_t)0, data);
		break;
	case 1:
		m_dma_offset[BIT(offset, 3)][2] = data;
		break;
	case 2:
		m_dma_offset[BIT(offset, 3)][3] = data;
		break;
	case 3:
		m_dma_offset[BIT(offset, 3)][1] = data;
		break;
	case 7:
		m_dma_offset[BIT(offset, 3)][0] = data;
		break;
	}
}

void vt82c586b_isa_device::pc_dma_hrq_changed(int state)
{
	m_host_cpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma[1]->hack_w(state);
}

uint8_t vt82c586b_isa_device::pc_dma_read_byte(offs_t offset)
{
	address_space &prog_space = m_host_cpu->space(AS_PROGRAM); // get the right address space
	if (m_dma_channel == -1)
		return 0xff;
	uint8_t result;
	offs_t page_offset = ((offs_t)m_dma_offset[0][m_dma_channel]) << 16;

	result = prog_space.read_byte(page_offset + offset);
	return result;
}

void vt82c586b_isa_device::pc_dma_write_byte(offs_t offset, uint8_t data)
{
	address_space &prog_space = m_host_cpu->space(AS_PROGRAM); // get the right address space
	if (m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t)m_dma_offset[0][m_dma_channel]) << 16;

	prog_space.write_byte(page_offset + offset, data);
}

uint8_t vt82c586b_isa_device::pc_dma_read_word(offs_t offset)
{
	address_space &prog_space = m_host_cpu->space(AS_PROGRAM); // get the right address space
	if (m_dma_channel == -1)
		return 0xff;
	uint16_t result;
	offs_t page_offset = ((offs_t)m_dma_offset[1][m_dma_channel & 3]) << 16;

	result = prog_space.read_word((page_offset & 0xfe0000) | (offset << 1));
	m_dma_high_byte = result & 0xFF00;

	return result & 0xFF;
}

void vt82c586b_isa_device::pc_dma_write_word(offs_t offset, uint8_t data)
{
	address_space &prog_space = m_host_cpu->space(AS_PROGRAM); // get the right address space
	if (m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t)m_dma_offset[1][m_dma_channel & 3]) << 16;

	prog_space.write_word((page_offset & 0xfe0000) | (offset << 1), m_dma_high_byte | data);
}

uint8_t vt82c586b_isa_device::pc_dma8237_0_dack_r() { return m_isabus->dack_r(0); }
uint8_t vt82c586b_isa_device::pc_dma8237_1_dack_r() { return m_isabus->dack_r(1); }
uint8_t vt82c586b_isa_device::pc_dma8237_2_dack_r() { return m_isabus->dack_r(2); }
uint8_t vt82c586b_isa_device::pc_dma8237_3_dack_r() { return m_isabus->dack_r(3); }
uint8_t vt82c586b_isa_device::pc_dma8237_5_dack_r() { return m_isabus->dack_r(5); }
uint8_t vt82c586b_isa_device::pc_dma8237_6_dack_r() { return m_isabus->dack_r(6); }
uint8_t vt82c586b_isa_device::pc_dma8237_7_dack_r() { return m_isabus->dack_r(7); }

void vt82c586b_isa_device::pc_dma8237_0_dack_w(uint8_t data) { m_isabus->dack_w(0, data); }
void vt82c586b_isa_device::pc_dma8237_1_dack_w(uint8_t data) { m_isabus->dack_w(1, data); }
void vt82c586b_isa_device::pc_dma8237_2_dack_w(uint8_t data) { m_isabus->dack_w(2, data); }
void vt82c586b_isa_device::pc_dma8237_3_dack_w(uint8_t data) { m_isabus->dack_w(3, data); }
void vt82c586b_isa_device::pc_dma8237_5_dack_w(uint8_t data) { m_isabus->dack_w(5, data); }
void vt82c586b_isa_device::pc_dma8237_6_dack_w(uint8_t data) { m_isabus->dack_w(6, data); }
void vt82c586b_isa_device::pc_dma8237_7_dack_w(uint8_t data) { m_isabus->dack_w(7, data); }

void vt82c586b_isa_device::at_dma8237_out_eop(int state)
{
	m_cur_eop = state == ASSERT_LINE;
	if (m_dma_channel != -1)
		m_isabus->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE);
}

void vt82c586b_isa_device::pc_select_dma_channel(int channel, bool state)
{
	m_isabus->dack_line_w(channel, state);

	if (!state)
	{
		m_dma_channel = channel;
		if (m_cur_eop)
			m_isabus->eop_w(channel, ASSERT_LINE);
	}
	else if (m_dma_channel == channel)
	{
		m_dma_channel = -1;
		if (m_cur_eop)
			m_isabus->eop_w(channel, CLEAR_LINE);
	}
}

void vt82c586b_isa_device::pc_dack0_w(int state) { pc_select_dma_channel(0, state); }
void vt82c586b_isa_device::pc_dack1_w(int state) { pc_select_dma_channel(1, state); }
void vt82c586b_isa_device::pc_dack2_w(int state) { pc_select_dma_channel(2, state); }
void vt82c586b_isa_device::pc_dack3_w(int state) { pc_select_dma_channel(3, state); }
void vt82c586b_isa_device::pc_dack4_w(int state) { m_dma[0]->hack_w( state ? 0 : 1); } // it's inverted
void vt82c586b_isa_device::pc_dack5_w(int state) { pc_select_dma_channel(5, state); }
void vt82c586b_isa_device::pc_dack6_w(int state) { pc_select_dma_channel(6, state); }
void vt82c586b_isa_device::pc_dack7_w(int state) { pc_select_dma_channel(7, state); }

uint8_t vt82c586b_isa_device::at_dma8237_2_r(offs_t offset)
{
	return m_dma[1]->read(offset / 2);
}

void vt82c586b_isa_device::at_dma8237_2_w(offs_t offset, uint8_t data)
{
	m_dma[1]->write(offset / 2, data);
}

void vt82c586b_isa_device::fast_gatea20(int state)
{
	m_fast_gatea20 = state;
	m_write_a20m(m_fast_gatea20 | m_ext_gatea20);
}

void vt82c586b_isa_device::keyboard_gatea20(int state)
{
	m_ext_gatea20 = state;
	m_write_a20m(m_fast_gatea20 | m_ext_gatea20);
}

/*
 * Pin Mapper
 */

void vt82c586b_isa_device::redirect_irq(int irq, int state)
{
	switch (irq)
	{
	case 0:
	case 2:
	case 8:
	case 13:
		break;
	// Claims irq 1 to be selectable vs. other PCI ISAs
	case 1:
		m_pic[0]->ir1_w(state);
		break;
	case 3:
		m_pic[0]->ir3_w(state);
		break;
	case 4:
		m_pic[0]->ir4_w(state);
		break;
	case 5:
		m_pic[0]->ir5_w(state);
		break;
	case 6:
		m_pic[0]->ir6_w(state);
		break;
	case 7:
		m_pic[0]->ir7_w(state);
		break;
	case 9:
		m_pic[1]->ir1_w(state);
		break;
	case 10:
		m_pic[1]->ir2_w(state);
		break;
	case 11:
		m_pic[1]->ir3_w(state);
		break;
	case 12:
		m_pic[1]->ir4_w(state);
		break;
	case 14:
		m_pic[1]->ir6_w(state);
		break;
	case 15:
		m_pic[1]->ir7_w(state);
		break;
	}
}

// NOTE: doesn't seem to have a disable method
int vt82c586b_isa_device::pin_mapper(int pin)
{
	if(pin < 0 || pin >= 4)
		return -1;
	return m_pirqrc[pin];
}

void vt82c586b_isa_device::irq_handler(int line, int state)
{
	if(line < 0 || line >= 16)
		return;

	logerror("irq_handler %d %d\n", line, state);
	redirect_irq(line, state);
}

void vt82c586b_isa_device::pc_pirqa_w(int state)
{
	int irq = m_pirqrc[0] & 15;

//  if (m_pirqrc[0] & 128)
//      return;
	redirect_irq(irq, state);
}

void vt82c586b_isa_device::pc_pirqb_w(int state)
{
	int irq = m_pirqrc[1] & 15;

//  if (m_pirqrc[1] & 128)
//      return;
	redirect_irq(irq, state);
}

void vt82c586b_isa_device::pc_pirqc_w(int state)
{
	int irq = m_pirqrc[2] & 15;

//  if (m_pirqrc[2] & 128)
//      return;
	redirect_irq(irq, state);
}

void vt82c586b_isa_device::pc_pirqd_w(int state)
{
	int irq = m_pirqrc[3] & 15;

//  if (m_pirqrc[3] & 128)
//      return;
	redirect_irq(irq, state);
}

void vt82c586b_isa_device::pc_mirq0_w(int state)
{
	int irq = m_mirq[0] & 15;

	// selects APICCS# if '1'
	if (BIT(m_mirq_pin_config, 0))
		return;
	redirect_irq(irq, state);
}

void vt82c586b_isa_device::pc_mirq1_w(int state)
{
	int irq = m_mirq[1] & 15;

	// selects KEYLOCK if '1'
	if (BIT(m_mirq_pin_config, 1))
		return;
	redirect_irq(irq, state);
}

void vt82c586b_isa_device::pc_mirq2_w(int state)
{
	int irq = m_mirq[2] & 15;

	// selects MASTER# if '1'
	if (BIT(m_mirq_pin_config, 2))
		return;
	redirect_irq(irq, state);
}

