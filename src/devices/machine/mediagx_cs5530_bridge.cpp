// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

[Cyrix/National Semiconductor/AMD] [MediaGX/Geode] [Cx/CS]5530 bridge implementation (southbridge)

**************************************************************************************************/

#include "emu.h"
#include "mediagx_cs5530_bridge.h"

#include "speaker.h"

#define LOG_MAP    (1U << 1) // log full remaps

#define VERBOSE (LOG_GENERAL | LOG_MAP)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)

DEFINE_DEVICE_TYPE(MEDIAGX_CS5530_BRIDGE, mediagx_cs5530_bridge_device, "mediagx_cs5530_bridge", "MediaGX CS5530 Bridge")

mediagx_cs5530_bridge_device::mediagx_cs5530_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MEDIAGX_CS5530_BRIDGE, tag, owner, clock)
//  , m_smi_callback(*this)
//  , m_nmi_callback(*this)
//  , m_stpclk_callback(*this)
	, m_boot_state_hook(*this)
	, m_rtcale(*this)
	, m_rtccs_read(*this, 0xff)
	, m_rtccs_write(*this)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
	, m_ide(*this, finder_base::DUMMY_TAG)
	, m_kbdc(*this, finder_base::DUMMY_TAG)
	, m_pic8259_master(*this, "pic8259_master")
	, m_pic8259_slave(*this, "pic8259_slave")
	, m_dma8237_1(*this, "dma8237_1")
	, m_dma8237_2(*this, "dma8237_2")
	, m_pit8254(*this, "pit8254")
	, m_isabus(*this, "isabus")
	, m_speaker(*this, "speaker")
{
}

void mediagx_cs5530_bridge_device::device_add_mconfig(machine_config &config)
{
	// 8254-equivalent
	PIT8254(config, m_pit8254);
	m_pit8254->set_clk<0>(4772720 / 4); // heartbeat IRQ
	m_pit8254->out_handler<0>().set(FUNC(mediagx_cs5530_bridge_device::at_pit8254_out0_changed));
	m_pit8254->set_clk<1>(4772720 / 4); // DRAM refresh
	m_pit8254->out_handler<1>().set(FUNC(mediagx_cs5530_bridge_device::at_pit8254_out1_changed));
	m_pit8254->set_clk<2>(4772720 / 4); // PIO port C pin 4, and speaker polling enough
	m_pit8254->out_handler<2>().set(FUNC(mediagx_cs5530_bridge_device::at_pit8254_out2_changed));

	// 8237-equivalent
	AM9517A(config, m_dma8237_1, XTAL(14'318'181) / 3);
	m_dma8237_1->out_hreq_callback().set(m_dma8237_2, FUNC(am9517a_device::dreq0_w));
	m_dma8237_1->out_eop_callback().set(FUNC(mediagx_cs5530_bridge_device::at_dma8237_out_eop));
	m_dma8237_1->in_memr_callback().set(FUNC(mediagx_cs5530_bridge_device::pc_dma_read_byte));
	m_dma8237_1->out_memw_callback().set(FUNC(mediagx_cs5530_bridge_device::pc_dma_write_byte));
	m_dma8237_1->in_ior_callback<0>().set(FUNC(mediagx_cs5530_bridge_device::pc_dma8237_0_dack_r));
	m_dma8237_1->in_ior_callback<1>().set(FUNC(mediagx_cs5530_bridge_device::pc_dma8237_1_dack_r));
	m_dma8237_1->in_ior_callback<2>().set(FUNC(mediagx_cs5530_bridge_device::pc_dma8237_2_dack_r));
	m_dma8237_1->in_ior_callback<3>().set(FUNC(mediagx_cs5530_bridge_device::pc_dma8237_3_dack_r));
	m_dma8237_1->out_iow_callback<0>().set(FUNC(mediagx_cs5530_bridge_device::pc_dma8237_0_dack_w));
	m_dma8237_1->out_iow_callback<1>().set(FUNC(mediagx_cs5530_bridge_device::pc_dma8237_1_dack_w));
	m_dma8237_1->out_iow_callback<2>().set(FUNC(mediagx_cs5530_bridge_device::pc_dma8237_2_dack_w));
	m_dma8237_1->out_iow_callback<3>().set(FUNC(mediagx_cs5530_bridge_device::pc_dma8237_3_dack_w));
	m_dma8237_1->out_dack_callback<0>().set(FUNC(mediagx_cs5530_bridge_device::pc_dack0_w));
	m_dma8237_1->out_dack_callback<1>().set(FUNC(mediagx_cs5530_bridge_device::pc_dack1_w));
	m_dma8237_1->out_dack_callback<2>().set(FUNC(mediagx_cs5530_bridge_device::pc_dack2_w));
	m_dma8237_1->out_dack_callback<3>().set(FUNC(mediagx_cs5530_bridge_device::pc_dack3_w));

	AM9517A(config, m_dma8237_2, XTAL(14'318'181) / 3);
	m_dma8237_2->out_hreq_callback().set(FUNC(mediagx_cs5530_bridge_device::pc_dma_hrq_changed));
	m_dma8237_2->in_memr_callback().set(FUNC(mediagx_cs5530_bridge_device::pc_dma_read_word));
	m_dma8237_2->out_memw_callback().set(FUNC(mediagx_cs5530_bridge_device::pc_dma_write_word));
	m_dma8237_2->in_ior_callback<1>().set(FUNC(mediagx_cs5530_bridge_device::pc_dma8237_5_dack_r));
	m_dma8237_2->in_ior_callback<2>().set(FUNC(mediagx_cs5530_bridge_device::pc_dma8237_6_dack_r));
	m_dma8237_2->in_ior_callback<3>().set(FUNC(mediagx_cs5530_bridge_device::pc_dma8237_7_dack_r));
	m_dma8237_2->out_iow_callback<1>().set(FUNC(mediagx_cs5530_bridge_device::pc_dma8237_5_dack_w));
	m_dma8237_2->out_iow_callback<2>().set(FUNC(mediagx_cs5530_bridge_device::pc_dma8237_6_dack_w));
	m_dma8237_2->out_iow_callback<3>().set(FUNC(mediagx_cs5530_bridge_device::pc_dma8237_7_dack_w));
	m_dma8237_2->out_dack_callback<0>().set(FUNC(mediagx_cs5530_bridge_device::pc_dack4_w));
	m_dma8237_2->out_dack_callback<1>().set(FUNC(mediagx_cs5530_bridge_device::pc_dack5_w));
	m_dma8237_2->out_dack_callback<2>().set(FUNC(mediagx_cs5530_bridge_device::pc_dack6_w));
	m_dma8237_2->out_dack_callback<3>().set(FUNC(mediagx_cs5530_bridge_device::pc_dack7_w));

	// 8259A-equivalent
	PIC8259(config, m_pic8259_master);
	m_pic8259_master->out_int_callback().set_inputline(m_host_cpu, 0);
	m_pic8259_master->in_sp_callback().set_constant(1);
	m_pic8259_master->read_slave_ack_callback().set(FUNC(mediagx_cs5530_bridge_device::get_slave_ack));

	PIC8259(config, m_pic8259_slave);
	m_pic8259_slave->out_int_callback().set(m_pic8259_master, FUNC(pic8259_device::ir2_w));
	m_pic8259_slave->in_sp_callback().set_constant(0);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	ISA16(config, m_isabus, 0);
	m_isabus->irq3_callback().set(FUNC(mediagx_cs5530_bridge_device::pc_irq3_w));
	m_isabus->irq4_callback().set(FUNC(mediagx_cs5530_bridge_device::pc_irq4_w));
	m_isabus->irq5_callback().set(FUNC(mediagx_cs5530_bridge_device::pc_irq5_w));
	m_isabus->irq6_callback().set(FUNC(mediagx_cs5530_bridge_device::pc_irq6_w));
	m_isabus->irq7_callback().set(FUNC(mediagx_cs5530_bridge_device::pc_irq7_w));
	m_isabus->irq2_callback().set(FUNC(mediagx_cs5530_bridge_device::pc_irq9_w));
	m_isabus->irq10_callback().set(FUNC(mediagx_cs5530_bridge_device::pc_irq10_w));
	m_isabus->irq11_callback().set(FUNC(mediagx_cs5530_bridge_device::pc_irq11_w));
	m_isabus->irq12_callback().set(FUNC(mediagx_cs5530_bridge_device::pc_irq12m_w));
	m_isabus->irq14_callback().set(FUNC(mediagx_cs5530_bridge_device::pc_irq14_w));
	m_isabus->irq15_callback().set(FUNC(mediagx_cs5530_bridge_device::pc_irq15_w));
	m_isabus->iochck_callback().set(FUNC(mediagx_cs5530_bridge_device::iochck_w));
}

void mediagx_cs5530_bridge_device::device_config_complete()
{
	auto isabus = m_isabus.finder_target();
	isabus.first.subdevice<isa16_device>(isabus.second)->set_memspace(m_host_cpu, AS_PROGRAM);
	isabus.first.subdevice<isa16_device>(isabus.second)->set_iospace(m_host_cpu, AS_IO);

	pci_device::device_config_complete();
}

void mediagx_cs5530_bridge_device::device_reset()
{
	pci_device::device_reset();

	command = 0x0000;
	status = 0x0280;

	m_nmi_enabled = false;
}

void mediagx_cs5530_bridge_device::config_map(address_map &map)
{
	pci_device::config_map(map);
//  map(0x40, 0x42) PCI Function Control
//  map(0x43, 0x43) USB Shadow
//  map(0x44, 0x44) Reset Control

//  map(0x50, 0x50) PIT Control/ISA CLK divider
//  map(0x51, 0x51) ISA I/O Recovery Control
//  map(0x52, 0x52) ROM/AT Logic Control
//  map(0x53, 0x53) Alternate CPU Support
//  map(0x5a, 0x5b) Decode Control
	map(0x5a, 0x5b).lrw8(
		NAME([this] (offs_t offset) {
			return m_decode_control[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_decode_control[offset] = data;
			remap_cb();
		})
	);
//  map(0x5c, 0x5d) PCI Interrupt Steering

//  map(0x70, 0x71) GPCS Base Address
//  map(0x72, 0x72) GPCS Control

//  map(0x80, 0x83) Power Management Enable
//  map(0x84, 0x87) Second Level Power Management Status Mirror (r/o)
//  map(0x88, 0x89) General Purpose Timer 1 Count/Control
//  map(0x8a, 0x8b) General Purpose Timer 2 Count/Control
//  map(0x8c, 0x8c) IRQ Speedup Timer Count
//  map(0x8d, 0x8d) Video Speedup Timer Count
//  map(0x8e, 0x8e) VGA Timer COunt

//  map(0x90, 0x90) GPIO Pin Direction
//  map(0x91, 0x91) GPIO Pin Data
//  map(0x92, 0x92) GPIO Control 1
//  map(0x93, 0x93) Miscellaneous Device Control
//  map(0x94, 0x95) Suspend Modulation OFF/ON Count
//  map(0x96, 0x96) Suspend Configuration
//  map(0x97, 0x97) GPIO Control 2

//  map(0x98, 0x99) Primary HDD Idle Timer Count
//  map(0x9a, 0x9b) Floppy Disk Idle Timer Count
//  map(0x9c, 0x9d) Parallel / Serial Idle Timer Count
//  map(0x9e, 0x9f) Keyboard / Mouse Idle Timer Count
//  map(0xa0, 0xa5) User Defined Device # Idle Timer Count
//  map(0xa6, 0xa7) Video Idle Timer Count
//  map(0xa8, 0xa9) Video Overflow Count
//  map(0xac, 0xad) Secondary HDD Idle Timer Count
//  map(0xae, 0xae) CPU Suspend Command (w/o)
//  map(0xaf, 0xaf) Suspend Notebook Command (w/o)

//  map(0xb4, 0xb7) Floppy Port Shadows (r/o)
//  map(0xb8, 0xb8) DMA Shadow (r/o)
//  map(0xb9, 0xb9) PIC Shadow (r/o)
//  map(0xba, 0xba) PIT Shadow (r/o)
//  map(0xbb, 0xbb) RTC Index Shadow (r/o)
//  map(0xbc, 0xbc) Clock Stop Control

//  map(0xc0, 0xcb) User Defined Device # Base Address
//  map(0xcc, 0xce) User Defined Device # Control

//  map(0xd0, 0xd0) Software SMI (w/o)
//  map(0xec, 0xec) Timer Test

//  map(0xf4, 0xf7) Second Level Power Management Status
}

// TODO: keyboard & RTC ports should map thru map_extra subtractive/positive decoding
void mediagx_cs5530_bridge_device::internal_io_map(address_map &map)
{
	map(0x0000, 0x001f).rw("dma8237_1", FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x0021).rw("pic8259_master", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x005f).rw("pit8254", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0060, 0x0060).rw(m_kbdc, FUNC(kbdc8042_device::port60_r), FUNC(kbdc8042_device::port60_w));
	map(0x0061, 0x0061).rw(FUNC(mediagx_cs5530_bridge_device::at_portb_r), FUNC(mediagx_cs5530_bridge_device::at_portb_w));
	map(0x0064, 0x0064).rw(m_kbdc, FUNC(kbdc8042_device::port64_r), FUNC(kbdc8042_device::port64_w));
	map(0x0070, 0x0070).lw8(
		NAME([this] (u8 data) {
			m_nmi_enabled = BIT(data, 7);
			// TODO: trap write for RTC shadow reg
			m_rtcale(data);
		})
	);
	map(0x0071, 0x0071).lrw8(
		NAME([this] () {
			return m_rtccs_read();
		}),
		NAME([this] (u8 data) {
			m_rtccs_write(data);
		})
	);
	map(0x0080, 0x008f).rw(FUNC(mediagx_cs5530_bridge_device::at_page8_r), FUNC(mediagx_cs5530_bridge_device::at_page8_w));
	// TODO: port decoding driven by PCI register $52
	map(0x0092, 0x0092).lrw8(
		NAME([this] () {
			//LOG("Fast init $92 read\n");
			return m_fast_init;
		}),
		NAME([this] (u8 data) {
			LOG("Fast init $92 write %02x\n", data);
			m_host_cpu->set_input_line(INPUT_LINE_A20, BIT(data, 1));
			m_fast_init = data;
		})
	);
	map(0x00a0, 0x00a1).rw("pic8259_slave", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x00c0, 0x00df).rw(FUNC(mediagx_cs5530_bridge_device::at_dma8237_2_r), FUNC(mediagx_cs5530_bridge_device::at_dma8237_2_w));
//  map(0x04d0, 0x04d1).rw(FUNC(mediagx_cs5530_bridge_device::eisa_irq_read), FUNC(mediagx_cs5530_bridge_device::eisa_irq_write));
	map(0x00e0, 0x00ef).noprw();
//  map(0x121c, 0x121f) ACPI Timer count register (on rev 1.3+)
}

void mediagx_cs5530_bridge_device::pc_irq1_w(int state)   { m_pic8259_master->ir1_w(state); }
void mediagx_cs5530_bridge_device::pc_irq3_w(int state)   { m_pic8259_master->ir3_w(state); }
void mediagx_cs5530_bridge_device::pc_irq4_w(int state)   { m_pic8259_master->ir4_w(state); }
void mediagx_cs5530_bridge_device::pc_irq5_w(int state)   { m_pic8259_master->ir5_w(state); }
void mediagx_cs5530_bridge_device::pc_irq6_w(int state)   { m_pic8259_master->ir6_w(state); }
void mediagx_cs5530_bridge_device::pc_irq7_w(int state)   { m_pic8259_master->ir7_w(state); }
void mediagx_cs5530_bridge_device::pc_irq8n_w(int state)  { m_pic8259_slave->ir0_w(state); }
void mediagx_cs5530_bridge_device::pc_irq9_w(int state)   { m_pic8259_slave->ir1_w(state); }
void mediagx_cs5530_bridge_device::pc_irq10_w(int state)  { m_pic8259_slave->ir2_w(state); }
void mediagx_cs5530_bridge_device::pc_irq11_w(int state)  { m_pic8259_slave->ir3_w(state); }
void mediagx_cs5530_bridge_device::pc_irq12m_w(int state) { m_pic8259_slave->ir4_w(state); }
void mediagx_cs5530_bridge_device::pc_irq14_w(int state)  { m_pic8259_slave->ir6_w(state); }
void mediagx_cs5530_bridge_device::pc_irq15_w(int state)  { m_pic8259_slave->ir7_w(state); }

uint8_t mediagx_cs5530_bridge_device::get_slave_ack(offs_t offset)
{
	if (offset == 2) // IRQ = 2
		return m_pic8259_slave->acknowledge();

	return 0x00;
}

void mediagx_cs5530_bridge_device::at_pit8254_out0_changed(int state)
{
	if (m_pic8259_master)
		m_pic8259_master->ir0_w(state);
}

void mediagx_cs5530_bridge_device::at_pit8254_out1_changed(int state)
{
	if (state)
		m_refresh = !m_refresh;
}

void mediagx_cs5530_bridge_device::at_pit8254_out2_changed(int state)
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}

void mediagx_cs5530_bridge_device::at_speaker_set_spkrdata(uint8_t data)
{
	m_at_spkrdata = data ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}

uint8_t mediagx_cs5530_bridge_device::at_portb_r()
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

void mediagx_cs5530_bridge_device::at_portb_w(uint8_t data)
{
	m_at_speaker = data;
	m_pit8254->write_gate2(BIT(data, 0));
	at_speaker_set_spkrdata(BIT(data, 1));
	m_channel_check = BIT(data, 3);
	if (m_channel_check)
		m_host_cpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void mediagx_cs5530_bridge_device::iochck_w(int state)
{
	if (!state && !m_channel_check && m_nmi_enabled)
		m_host_cpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

uint8_t mediagx_cs5530_bridge_device::at_page8_r(offs_t offset)
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

void mediagx_cs5530_bridge_device::at_page8_w(offs_t offset, uint8_t data)
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

void mediagx_cs5530_bridge_device::pc_dma_hrq_changed(int state)
{
	m_host_cpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237_2->hack_w(state);
}

uint8_t mediagx_cs5530_bridge_device::pc_dma_read_byte(offs_t offset)
{
	address_space &prog_space = m_host_cpu->space(AS_PROGRAM); // get the right address space
	if (m_dma_channel == -1)
		return 0xff;
	uint8_t result;
	offs_t page_offset = ((offs_t)m_dma_offset[0][m_dma_channel]) << 16;

	result = prog_space.read_byte(page_offset + offset);
	return result;
}

void mediagx_cs5530_bridge_device::pc_dma_write_byte(offs_t offset, uint8_t data)
{
	address_space &prog_space = m_host_cpu->space(AS_PROGRAM); // get the right address space
	if (m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t)m_dma_offset[0][m_dma_channel]) << 16;

	prog_space.write_byte(page_offset + offset, data);
}

uint8_t mediagx_cs5530_bridge_device::pc_dma_read_word(offs_t offset)
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

void mediagx_cs5530_bridge_device::pc_dma_write_word(offs_t offset, uint8_t data)
{
	address_space &prog_space = m_host_cpu->space(AS_PROGRAM); // get the right address space
	if (m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t)m_dma_offset[1][m_dma_channel & 3]) << 16;

	prog_space.write_word((page_offset & 0xfe0000) | (offset << 1), m_dma_high_byte | data);
}

uint8_t mediagx_cs5530_bridge_device::pc_dma8237_0_dack_r() { return m_isabus->dack_r(0); }
uint8_t mediagx_cs5530_bridge_device::pc_dma8237_1_dack_r() { return m_isabus->dack_r(1); }
uint8_t mediagx_cs5530_bridge_device::pc_dma8237_2_dack_r() { return m_isabus->dack_r(2); }
uint8_t mediagx_cs5530_bridge_device::pc_dma8237_3_dack_r() { return m_isabus->dack_r(3); }
uint8_t mediagx_cs5530_bridge_device::pc_dma8237_5_dack_r() { return m_isabus->dack_r(5); }
uint8_t mediagx_cs5530_bridge_device::pc_dma8237_6_dack_r() { return m_isabus->dack_r(6); }
uint8_t mediagx_cs5530_bridge_device::pc_dma8237_7_dack_r() { return m_isabus->dack_r(7); }

void mediagx_cs5530_bridge_device::pc_dma8237_0_dack_w(uint8_t data) { m_isabus->dack_w(0, data); }
void mediagx_cs5530_bridge_device::pc_dma8237_1_dack_w(uint8_t data) { m_isabus->dack_w(1, data); }
void mediagx_cs5530_bridge_device::pc_dma8237_2_dack_w(uint8_t data) { m_isabus->dack_w(2, data); }
void mediagx_cs5530_bridge_device::pc_dma8237_3_dack_w(uint8_t data) { m_isabus->dack_w(3, data); }
void mediagx_cs5530_bridge_device::pc_dma8237_5_dack_w(uint8_t data) { m_isabus->dack_w(5, data); }
void mediagx_cs5530_bridge_device::pc_dma8237_6_dack_w(uint8_t data) { m_isabus->dack_w(6, data); }
void mediagx_cs5530_bridge_device::pc_dma8237_7_dack_w(uint8_t data) { m_isabus->dack_w(7, data); }

void mediagx_cs5530_bridge_device::at_dma8237_out_eop(int state)
{
	m_cur_eop = state == ASSERT_LINE;
	if (m_dma_channel != -1)
		m_isabus->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE);
}

void mediagx_cs5530_bridge_device::pc_select_dma_channel(int channel, bool state)
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

void mediagx_cs5530_bridge_device::pc_dack0_w(int state) { pc_select_dma_channel(0, state); }
void mediagx_cs5530_bridge_device::pc_dack1_w(int state) { pc_select_dma_channel(1, state); }
void mediagx_cs5530_bridge_device::pc_dack2_w(int state) { pc_select_dma_channel(2, state); }
void mediagx_cs5530_bridge_device::pc_dack3_w(int state) { pc_select_dma_channel(3, state); }
void mediagx_cs5530_bridge_device::pc_dack4_w(int state) { m_dma8237_1->hack_w( state ? 0 : 1); } // it's inverted
void mediagx_cs5530_bridge_device::pc_dack5_w(int state) { pc_select_dma_channel(5, state); }
void mediagx_cs5530_bridge_device::pc_dack6_w(int state) { pc_select_dma_channel(6, state); }
void mediagx_cs5530_bridge_device::pc_dack7_w(int state) { pc_select_dma_channel(7, state); }

uint8_t mediagx_cs5530_bridge_device::at_dma8237_2_r(offs_t offset)
{
	return m_dma8237_2->read(offset / 2);
}

void mediagx_cs5530_bridge_device::at_dma8237_2_w(offs_t offset, uint8_t data)
{
	m_dma8237_2->write(offset / 2, data);
}

void mediagx_cs5530_bridge_device::map_bios(address_space *memory_space, uint32_t start, uint32_t end)
{
	uint32_t mask = m_region->bytes() - 1;
	memory_space->install_rom(start, end, m_region->base() + (start & mask));
}

void mediagx_cs5530_bridge_device::map_extra(
		uint64_t memory_window_start,
		uint64_t memory_window_end,
		uint64_t memory_offset,
		address_space *memory_space,
		uint64_t io_window_start,
		uint64_t io_window_end,
		uint64_t io_offset,
		address_space *io_space)
{
	m_isabus->remap(AS_PROGRAM, 0, 1 << 24);
	map_bios(memory_space, 0xffffffff - m_region->bytes() + 1, 0xffffffff);
	// TODO: BIOS window conditions + BIOS ROM
	if (BIT(m_decode_control[1], 5))
		LOGMAP("BIOS ROM positive decode\n");
	map_bios(memory_space, 0x000e0000, 0x000fffff);

	m_isabus->remap(AS_IO, 0, 0xffff);
	io_space->install_device(0, 0xffff, *this, &mediagx_cs5530_bridge_device::internal_io_map);

	if (BIT(m_decode_control[0], 0))
		LOGMAP("RTC positive decode $070 & $071\n");

	if (BIT(m_decode_control[0], 1))
	{
		LOGMAP("KBDC positive decode $060 & $064 (mailbox $62 & $66 %s)\n", BIT(m_decode_control[1], 7) ? "enabled" : "disabled");
	}

	if (BIT(m_decode_control[0], 2))
		LOGMAP("COM1 positive decode $3f8-$3ff\n");

	if (BIT(m_decode_control[0], 3))
		LOGMAP("COM2 positive decode $2f8-$2ff\n");

	if (BIT(m_decode_control[0], 4))
		LOGMAP("COM3 positive decode $3e8-$3ef\n");

	if (BIT(m_decode_control[0], 5))
		LOGMAP("COM4 positive decode $2e8-$2ef\n");

	if (BIT(m_decode_control[0], 6))
		LOGMAP("Primary FDC positive decode $3f2-$3f5 & $3f7\n");

	if (BIT(m_decode_control[0], 7))
		LOGMAP("Secondary FDC positive decode $372-$375 & $377\n");

	if (BIT(m_decode_control[1], 0))
		LOGMAP("LPT1 positive decode $378-$37f & $778-$77a\n");

	if (BIT(m_decode_control[1], 1))
		LOGMAP("LPT2 positive decode $278-$27f & $678-$67a\n");

	if (BIT(m_decode_control[1], 2))
		LOGMAP("LPT3 positive decode $3bc-$3be & $7bc-$7be\n");

	if (BIT(m_decode_control[1], 4))
	{
		LOGMAP("Secondary IDE positive decode $170-$177 & $376-$377\n");
		io_space->install_device(0, 0xffff, *m_ide, &mediagx_cs5530_ide_device::secondary_ide_map);
	}

	if (BIT(m_decode_control[1], 3))
	{
		LOGMAP("Primary IDE positive decode $1f0-$1f7 & $3f6-$3f7\n");
		io_space->install_device(0, 0xffff, *m_ide, &mediagx_cs5530_ide_device::primary_ide_map);
	}
}
