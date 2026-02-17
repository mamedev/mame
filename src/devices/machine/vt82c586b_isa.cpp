// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

VT82C586B PCIC ISA portion

**************************************************************************************************/

#include "emu.h"
#include "vt82c586b_isa.h"

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
	, m_boot_state_hook(*this)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
//	, m_kbdc(*this, "kbdc")
	, m_pic(*this, "pic%u", 0U)
	, m_dma(*this, "dma%u", 0U)
	, m_pit(*this, "pit")
	, m_isabus(*this, "isabus")
	, m_speaker(*this, "speaker")
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

	// TODO: integrated Keyboard Controller
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
}

void vt82c586b_isa_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	// ...
}

void vt82c586b_isa_device::internal_io_map(address_map &map)
{
	map(0x0000, 0x001f).rw(m_dma[0], FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x0021).rw(m_pic[0], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x005f).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
//	map(0x0060, 0x0060).rw(m_kbdc, FUNC(kbdc8042_device::port60_r), FUNC(kbdc8042_device::port60_w));
	map(0x0061, 0x0061).rw(FUNC(vt82c586b_isa_device::at_portb_r), FUNC(vt82c586b_isa_device::at_portb_w));
//	map(0x0064, 0x0064).rw(m_kbdc, FUNC(kbdc8042_device::port64_r), FUNC(kbdc8042_device::port64_w));
//	map(0x0070, 0x0070).lw8(
//		NAME([this] (u8 data) {
//			m_nmi_enabled = BIT(data, 7);
//			m_rtcale(data);
//		})
//	);
//	map(0x0071, 0x0071).lrw8(
//		NAME([this] () {
//			return m_rtccs_read();
//		}),
//		NAME([this] (u8 data) {
//			m_rtccs_write(data);
//		})
//	);
	map(0x0080, 0x008f).rw(FUNC(vt82c586b_isa_device::at_page8_r), FUNC(vt82c586b_isa_device::at_page8_w));
//	map(0x0092, 0x0092).lrw8(
//		NAME([this] () {
//			//LOG("Fast init $92 read\n");
//			return m_fast_init;
//		}),
//		NAME([this] (u8 data) {
//			LOG("Fast init $92 write %02x\n", data);
//			m_host_cpu->set_input_line(INPUT_LINE_A20, BIT(data, 1));
//			m_fast_init = data;
//		})
//	);
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
	map_bios(memory_space, 0xffffffff - m_region->bytes() + 1, 0xffffffff);
	map_bios(memory_space, 0x000e0000, 0x000fffff);
	m_isabus->remap(AS_IO, 0, 0xffff);
	io_space->install_device(0, 0xffff, *this, &vt82c586b_isa_device::internal_io_map);
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


