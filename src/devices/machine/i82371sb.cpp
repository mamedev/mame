// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "i82371sb.h"

#include "speaker.h"

DEFINE_DEVICE_TYPE(I82371SB_ISA, i82371sb_isa_device, "i82371sb_isa", "Intel 82371 southbridge ISA bridge")

void i82371sb_isa_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x06, 0x07).rw(FUNC(i82371sb_isa_device::status_r), FUNC(i82371sb_isa_device::status_w));
	map(0x10, 0x4b).noprw();
	map(0x4c, 0x4c).rw(FUNC(i82371sb_isa_device::iort_r), FUNC(i82371sb_isa_device::iort_w));
	map(0x4e, 0x4f).rw(FUNC(i82371sb_isa_device::xbcs_r), FUNC(i82371sb_isa_device::xbcs_w));
	map(0x60, 0x63).rw(FUNC(i82371sb_isa_device::pirqrc_r), FUNC(i82371sb_isa_device::pirqrc_w));
	map(0x69, 0x69).rw(FUNC(i82371sb_isa_device::tom_r), FUNC(i82371sb_isa_device::tom_w));
	map(0x6a, 0x6b).rw(FUNC(i82371sb_isa_device::mstat_r), FUNC(i82371sb_isa_device::mstat_w));
	map(0x70, 0x71).rw(FUNC(i82371sb_isa_device::mbirq01_r), FUNC(i82371sb_isa_device::mbirq01_w));
	map(0x76, 0x77).rw(FUNC(i82371sb_isa_device::mbdma_r), FUNC(i82371sb_isa_device::mbdma_w));
	map(0x78, 0x79).rw(FUNC(i82371sb_isa_device::pcsc_r), FUNC(i82371sb_isa_device::pcsc_w));
	map(0x80, 0x80).rw(FUNC(i82371sb_isa_device::apicbase_r), FUNC(i82371sb_isa_device::apicbase_w));
	map(0x82, 0x82).rw(FUNC(i82371sb_isa_device::dlc_r), FUNC(i82371sb_isa_device::dlc_w));
	map(0xa0, 0xa0).rw(FUNC(i82371sb_isa_device::smicntl_r), FUNC(i82371sb_isa_device::smicntl_w));
	map(0xa2, 0xa3).rw(FUNC(i82371sb_isa_device::smien_r), FUNC(i82371sb_isa_device::smien_w));
	map(0xa4, 0xa7).rw(FUNC(i82371sb_isa_device::see_r), FUNC(i82371sb_isa_device::see_w));
	map(0xa8, 0xa8).rw(FUNC(i82371sb_isa_device::ftmr_r), FUNC(i82371sb_isa_device::ftmr_w));
	map(0xaa, 0xab).rw(FUNC(i82371sb_isa_device::smireq_r), FUNC(i82371sb_isa_device::smireq_w));
	map(0xac, 0xac).rw(FUNC(i82371sb_isa_device::ctltmr_r), FUNC(i82371sb_isa_device::ctltmr_w));
	map(0xae, 0xae).rw(FUNC(i82371sb_isa_device::cthtmr_r), FUNC(i82371sb_isa_device::cthtmr_w));
}

void i82371sb_isa_device::internal_io_map(address_map &map)
{
	map(0x0000, 0x001f).rw("dma8237_1", FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x0021).rw("pic8259_master", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
//  map(0x002e, 0x002f) Super I/O config
	map(0x0040, 0x0043).rw("pit8254", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
//  map(0x004e, 0x004f) Alt Super I/O config, watchdog-ish in thinkpad600e
	map(0x0061, 0x0061).rw(FUNC(i82371sb_isa_device::at_portb_r), FUNC(i82371sb_isa_device::at_portb_w));
//  map(0x0070, 0x0070) RTC address, bit 7 NMI enable
//  map(0x0071, 0x0071) RTC data
//  map(0x0078, 0x0079) Board Configuration
	map(0x0080, 0x009f).rw(FUNC(i82371sb_isa_device::at_page8_r), FUNC(i82371sb_isa_device::at_page8_w));
	map(0x00a0, 0x00a1).rw("pic8259_slave", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x00b2, 0x00b3).rw(FUNC(i82371sb_isa_device::read_apmcapms), FUNC(i82371sb_isa_device::write_apmcapms));
	// Up to $de according to TC430HX spec?
	map(0x00c0, 0x00df).rw(FUNC(i82371sb_isa_device::at_dma8237_2_r), FUNC(i82371sb_isa_device::at_dma8237_2_w));
	map(0x00e0, 0x00ef).noprw();
//  map(0x00f0, 0x00f0) Reset Numeric Error
//  map(0x0270, 0x0273) I/O read port for PnP
	map(0x04d0, 0x04d1).rw(FUNC(i82371sb_isa_device::eisa_irq_read), FUNC(i82371sb_isa_device::eisa_irq_write));
	map(0x0cf9, 0x0cf9).rw(FUNC(i82371sb_isa_device::reset_control_r), FUNC(i82371sb_isa_device::reset_control_w));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void i82371sb_isa_device::device_add_mconfig(machine_config &config)
{
	PIT8254(config, m_pit8254);
	m_pit8254->set_clk<0>(4772720 / 4); // heartbeat IRQ
	m_pit8254->out_handler<0>().set(FUNC(i82371sb_isa_device::at_pit8254_out0_changed));
	m_pit8254->set_clk<1>(4772720 / 4); // DRAM refresh
	m_pit8254->out_handler<1>().set(FUNC(i82371sb_isa_device::at_pit8254_out1_changed));
	m_pit8254->set_clk<2>(4772720 / 4); // PIO port C pin 4, and speaker polling enough
	m_pit8254->out_handler<2>().set(FUNC(i82371sb_isa_device::at_pit8254_out2_changed));

	AM9517A(config, m_dma8237_1, XTAL(14'318'181) / 3);
	m_dma8237_1->out_hreq_callback().set(m_dma8237_2, FUNC(am9517a_device::dreq0_w));
	m_dma8237_1->out_eop_callback().set(FUNC(i82371sb_isa_device::at_dma8237_out_eop));
	m_dma8237_1->in_memr_callback().set(FUNC(i82371sb_isa_device::pc_dma_read_byte));
	m_dma8237_1->out_memw_callback().set(FUNC(i82371sb_isa_device::pc_dma_write_byte));
	m_dma8237_1->in_ior_callback<0>().set(FUNC(i82371sb_isa_device::pc_dma8237_0_dack_r));
	m_dma8237_1->in_ior_callback<1>().set(FUNC(i82371sb_isa_device::pc_dma8237_1_dack_r));
	m_dma8237_1->in_ior_callback<2>().set(FUNC(i82371sb_isa_device::pc_dma8237_2_dack_r));
	m_dma8237_1->in_ior_callback<3>().set(FUNC(i82371sb_isa_device::pc_dma8237_3_dack_r));
	m_dma8237_1->out_iow_callback<0>().set(FUNC(i82371sb_isa_device::pc_dma8237_0_dack_w));
	m_dma8237_1->out_iow_callback<1>().set(FUNC(i82371sb_isa_device::pc_dma8237_1_dack_w));
	m_dma8237_1->out_iow_callback<2>().set(FUNC(i82371sb_isa_device::pc_dma8237_2_dack_w));
	m_dma8237_1->out_iow_callback<3>().set(FUNC(i82371sb_isa_device::pc_dma8237_3_dack_w));
	m_dma8237_1->out_dack_callback<0>().set(FUNC(i82371sb_isa_device::pc_dack0_w));
	m_dma8237_1->out_dack_callback<1>().set(FUNC(i82371sb_isa_device::pc_dack1_w));
	m_dma8237_1->out_dack_callback<2>().set(FUNC(i82371sb_isa_device::pc_dack2_w));
	m_dma8237_1->out_dack_callback<3>().set(FUNC(i82371sb_isa_device::pc_dack3_w));

	AM9517A(config, m_dma8237_2, XTAL(14'318'181) / 3);
	m_dma8237_2->out_hreq_callback().set(FUNC(i82371sb_isa_device::pc_dma_hrq_changed));
	m_dma8237_2->in_memr_callback().set(FUNC(i82371sb_isa_device::pc_dma_read_word));
	m_dma8237_2->out_memw_callback().set(FUNC(i82371sb_isa_device::pc_dma_write_word));
	m_dma8237_2->in_ior_callback<1>().set(FUNC(i82371sb_isa_device::pc_dma8237_5_dack_r));
	m_dma8237_2->in_ior_callback<2>().set(FUNC(i82371sb_isa_device::pc_dma8237_6_dack_r));
	m_dma8237_2->in_ior_callback<3>().set(FUNC(i82371sb_isa_device::pc_dma8237_7_dack_r));
	m_dma8237_2->out_iow_callback<1>().set(FUNC(i82371sb_isa_device::pc_dma8237_5_dack_w));
	m_dma8237_2->out_iow_callback<2>().set(FUNC(i82371sb_isa_device::pc_dma8237_6_dack_w));
	m_dma8237_2->out_iow_callback<3>().set(FUNC(i82371sb_isa_device::pc_dma8237_7_dack_w));
	m_dma8237_2->out_dack_callback<0>().set(FUNC(i82371sb_isa_device::pc_dack4_w));
	m_dma8237_2->out_dack_callback<1>().set(FUNC(i82371sb_isa_device::pc_dack5_w));
	m_dma8237_2->out_dack_callback<2>().set(FUNC(i82371sb_isa_device::pc_dack6_w));
	m_dma8237_2->out_dack_callback<3>().set(FUNC(i82371sb_isa_device::pc_dack7_w));

	PIC8259(config, m_pic8259_master);
	m_pic8259_master->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic8259_master->in_sp_callback().set_constant(1);
	m_pic8259_master->read_slave_ack_callback().set(FUNC(i82371sb_isa_device::get_slave_ack));

	PIC8259(config, m_pic8259_slave);
	m_pic8259_slave->out_int_callback().set(m_pic8259_master, FUNC(pic8259_device::ir2_w));
	m_pic8259_slave->in_sp_callback().set_constant(0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	ISA16(config, m_isabus, 0);
	m_isabus->irq3_callback().set(FUNC(i82371sb_isa_device::pc_irq3_w));
	m_isabus->irq4_callback().set(FUNC(i82371sb_isa_device::pc_irq4_w));
	m_isabus->irq5_callback().set(FUNC(i82371sb_isa_device::pc_irq5_w));
	m_isabus->irq6_callback().set(FUNC(i82371sb_isa_device::pc_irq6_w));
	m_isabus->irq7_callback().set(FUNC(i82371sb_isa_device::pc_irq7_w));
	m_isabus->irq2_callback().set(FUNC(i82371sb_isa_device::pc_irq9_w));
	m_isabus->irq10_callback().set(FUNC(i82371sb_isa_device::pc_irq10_w));
	m_isabus->irq11_callback().set(FUNC(i82371sb_isa_device::pc_irq11_w));
	m_isabus->irq12_callback().set(FUNC(i82371sb_isa_device::pc_irq12m_w));
	m_isabus->irq14_callback().set(FUNC(i82371sb_isa_device::pc_irq14_w));
	m_isabus->irq15_callback().set(FUNC(i82371sb_isa_device::pc_irq15_w));
	m_isabus->drq0_callback().set(m_dma8237_1, FUNC(am9517a_device::dreq0_w));
	m_isabus->drq1_callback().set(m_dma8237_1, FUNC(am9517a_device::dreq1_w));
	m_isabus->drq2_callback().set(m_dma8237_1, FUNC(am9517a_device::dreq2_w));
	m_isabus->drq3_callback().set(m_dma8237_1, FUNC(am9517a_device::dreq3_w));
	m_isabus->drq5_callback().set(m_dma8237_2, FUNC(am9517a_device::dreq1_w));
	m_isabus->drq6_callback().set(m_dma8237_2, FUNC(am9517a_device::dreq2_w));
	m_isabus->drq7_callback().set(m_dma8237_2, FUNC(am9517a_device::dreq3_w));
	m_isabus->iochck_callback().set(FUNC(i82371sb_isa_device::iochck_w));
}

i82371sb_isa_device::i82371sb_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82371sb_isa_device(mconfig, I82371SB_ISA, tag, owner, clock)
{
	set_ids(0x80867000, 0x03, 0x060100, 0x00000000);
}

void i82371sb_isa_device::device_config_complete()
{
	auto isabus = m_isabus.finder_target();
	isabus.first.subdevice<isa16_device>(isabus.second)->set_memspace(m_maincpu, AS_PROGRAM);
	isabus.first.subdevice<isa16_device>(isabus.second)->set_iospace(m_maincpu, AS_IO);

	pci_device::device_config_complete();
}

i82371sb_isa_device::i82371sb_isa_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, type, tag, owner, clock)
	, m_smi_callback(*this)
	, m_nmi_callback(*this)
	, m_stpclk_callback(*this)
	, m_boot_state_hook(*this)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_pic8259_master(*this, "pic8259_master")
	, m_pic8259_slave(*this, "pic8259_slave")
	, m_dma8237_1(*this, "dma8237_1")
	, m_dma8237_2(*this, "dma8237_2")
	, m_pit8254(*this, "pit8254")
	, m_isabus(*this, "isabus")
	, m_speaker(*this, "speaker")
	, m_at_spkrdata(0)
	, m_pit_out2(0)
	, m_dma_channel(0)
	, m_cur_eop(false)
	, m_dma_high_byte(0)
	, m_eisa_irq_mode(0)
	, m_at_speaker(0)
	, m_refresh(false)
	, m_channel_check(0)
	, m_nmi_enabled(0)
{
}

void i82371sb_isa_device::device_start()
{
	pci_device::device_start();
	m_pci_root->set_pin_mapper(pci_pin_mapper(*this, FUNC(i82371sb_isa_device::pin_mapper)));
	m_pci_root->set_irq_handler(pci_irq_handler(*this, FUNC(i82371sb_isa_device::irq_handler)));
}

void i82371sb_isa_device::device_reset()
{
	pci_device::device_reset();

	status = 0x0280;
	iort = 0x4d;
	xbcs = 0x0003;
	memset(pirqrc, 0x80, sizeof(pirqrc));
	tom = 0x02;
	mstat = 0x0000;
	mbirq0 = 0x80;
	mbirq1 = 0x80;
	memset(mbdma, 0x0c, sizeof(mbdma));
	pcsc = 0x0002;
	apicbase = 0x00;
	dlc = 0x00;
	smicntl = 0x08;
	smien = 0x0000;
	apmc = 0x00;
	apms = 0x00;
	see = 0x00000000;
	ftmr = 0x0f;
	smireq = 0x0000;
	ctlmtr = 0x00;
	cthmtr = 0x00;
	reset_control = 0;

	m_at_spkrdata = 0;
	m_pit_out2 = 1;
	m_dma_channel = -1;
	m_cur_eop = false;
	m_nmi_enabled = 0;
	m_refresh = false;
}

void i82371sb_isa_device::reset_all_mappings()
{
	pci_device::reset_all_mappings();
}

void i82371sb_isa_device::boot_state_w(uint8_t data)
{
	m_boot_state_hook((offs_t)0, data);
}

void i82371sb_isa_device::nop_w(uint8_t data)
{
}

void i82371sb_isa_device::status_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// bits 11-13 are clear on write
	status = status & ~(data & (0x3800 & mem_mask));
}

uint8_t i82371sb_isa_device::iort_r()
{
	return iort;
}

void i82371sb_isa_device::iort_w(uint8_t data)
{
	iort = data;
	logerror("iort = %02x\n", iort);
}

uint16_t i82371sb_isa_device::xbcs_r()
{
	return xbcs;
}

void i82371sb_isa_device::xbcs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&xbcs);
	logerror("xbcs = %04x\n", xbcs);
}

uint8_t i82371sb_isa_device::pirqrc_r(offs_t offset)
{
	return pirqrc[offset];
}

void i82371sb_isa_device::pirqrc_w(offs_t offset, uint8_t data)
{
	pirqrc[offset] = data;
	logerror("pirqrc[%d] = %02x\n", offset, pirqrc[offset]);
}

uint8_t i82371sb_isa_device::tom_r()
{
	return tom;
}

void i82371sb_isa_device::tom_w(uint8_t data)
{
	tom = data;
	logerror("tom = %02x\n", tom);
}

uint16_t i82371sb_isa_device::mstat_r()
{
	return mstat;
}

void i82371sb_isa_device::mstat_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&mstat);
	logerror("mstat = %04x\n", mstat);
}

uint8_t i82371sb_isa_device::mbirq01_r(offs_t offset)
{
	if (offset == 0)
		return mbirq0;
	else
		return mbirq1;
}

void i82371sb_isa_device::mbirq01_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		mbirq0 = data;
		logerror("mbirq0 = %02x\n", mbirq0);
	}
	else if (offset == 1)
	{
		mbirq1 = data;
		logerror("mbirq1 = %02x\n", mbirq1);
	}
}

uint8_t i82371sb_isa_device::mbdma_r(offs_t offset)
{
	return mbdma[offset];
}

void i82371sb_isa_device::mbdma_w(offs_t offset, uint8_t data)
{
	mbdma[offset] = data;
	logerror("mbdma[%d] = %02x\n", offset, mbdma);
}

uint16_t i82371sb_isa_device::pcsc_r()
{
	return pcsc;
}

void i82371sb_isa_device::pcsc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&pcsc);
	logerror("pcsc = %04x\n", pcsc);
}

uint8_t i82371sb_isa_device::apicbase_r()
{
	return apicbase;
}

void i82371sb_isa_device::apicbase_w(uint8_t data)
{
	apicbase = data;
	logerror("apicbase = %02x\n", apicbase);
}

uint8_t i82371sb_isa_device::dlc_r()
{
	return dlc;
}

void i82371sb_isa_device::dlc_w(uint8_t data)
{
	dlc = data;
	logerror("dlc = %02x\n", dlc);
}

uint8_t i82371sb_isa_device::smicntl_r()
{
	return smicntl;
}

void i82371sb_isa_device::smicntl_w(uint8_t data)
{
	smicntl = data;
	update_smireq_line();
	logerror("smicntl = %02x\n", smicntl);
}

uint16_t i82371sb_isa_device::smien_r()
{
	return smien;
}

void i82371sb_isa_device::smien_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&smien);
	update_smireq_line();
	logerror("smien = %04x\n", smien);
}

uint32_t i82371sb_isa_device::see_r()
{
	return see;
}

void i82371sb_isa_device::see_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&see);
	logerror("see = %08x\n", see);
}

uint8_t i82371sb_isa_device::ftmr_r()
{
	return ftmr;
}

void i82371sb_isa_device::ftmr_w(uint8_t data)
{
	ftmr = data;
	logerror("ftmr = %02x\n", ftmr);
}

uint16_t i82371sb_isa_device::smireq_r()
{
	return smireq;
}

void i82371sb_isa_device::smireq_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&smireq);
	update_smireq_line();
	logerror("smireq = %04x\n", smireq);
}

uint8_t i82371sb_isa_device::ctltmr_r()
{
	return ctlmtr;
}

void i82371sb_isa_device::ctltmr_w(uint8_t data)
{
	ctlmtr = data;
	logerror("ctlmtr = %02x\n", ctlmtr);
}

uint8_t i82371sb_isa_device::cthtmr_r()
{
	return cthmtr;
}

void i82371sb_isa_device::cthtmr_w(uint8_t data)
{
	cthmtr = data;
	logerror("cthmtr = %02x\n", cthmtr);
}

void i82371sb_isa_device::map_bios(address_space *memory_space, uint32_t start, uint32_t end)
{
	uint32_t mask = m_region->bytes() - 1;
	memory_space->install_rom(start, end, m_region->base() + (start & mask));
}

void i82371sb_isa_device::map_extra(
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
	io_space->install_device(0, 0xffff, *this, &i82371sb_isa_device::internal_io_map);

#if 0
	if(fwh_dec_en1 & 0x80) {
		map_bios(memory_space, 0xfff80000, 0xffffffff, 7);
		map_bios(memory_space, 0xffb80000, 0xffbfffff, 7);
		map_bios(memory_space, 0x000e0000, 0x000fffff, 7);
	}
	if(fwh_dec_en1 & 0x40) {
		map_bios(memory_space, 0xfff00000, 0xfff7ffff, 6);
		map_bios(memory_space, 0xffb00000, 0xffb7ffff, 6);
	}
	if(fwh_dec_en1 & 0x20) {
		map_bios(memory_space, 0xffe80000, 0xffefffff, 5);
		map_bios(memory_space, 0xffa80000, 0xffafffff, 5);
	}
	if(fwh_dec_en1 & 0x10) {
		map_bios(memory_space, 0xffe00000, 0xffe7ffff, 4);
		map_bios(memory_space, 0xffa00000, 0xffa7ffff, 4);
	}
	if(fwh_dec_en1 & 0x08) {
		map_bios(memory_space, 0xffd80000, 0xffdfffff, 3);
		map_bios(memory_space, 0xff980000, 0xff9fffff, 3);
	}
	if(fwh_dec_en1 & 0x04) {
		map_bios(memory_space, 0xffd00000, 0xffd7ffff, 2);
		map_bios(memory_space, 0xff900000, 0xff97ffff, 2);
	}
	if(fwh_dec_en1 & 0x02) {
		map_bios(memory_space, 0xffc80000, 0xffcfffff, 1);
		map_bios(memory_space, 0xff880000, 0xff8fffff, 1);
	}
	if(fwh_dec_en1 & 0x01) {
		map_bios(memory_space, 0xffc00000, 0xffc7ffff, 0);
		map_bios(memory_space, 0xff800000, 0xff87ffff, 0);
	}


	if(acpi_cntl & 0x10)
		acpi->map_device(memory_window_start, memory_window_end, 0, memory_space, io_window_start, io_window_end, pmbase, io_space);
	if(gpio_cntl & 0x10)
		logerror("%s: Warning: gpio range enabled at %04x-%04x\n", tag(), gpio_base, gpio_base+63);

	uint32_t hpet = 0xfed00000 + ((gen_cntl & 0x00018000) >> 3);
	logerror("%s: Warning: hpet at %08x-%08x\n", tag(), hpet, hpet+0x3ff);

	if(lpc_en & 0x1000)
		logerror("%s: Warning: superio at 2e-2f\n", tag());
	if(lpc_en & 0x0800)
		logerror("%s: Warning: mcu at 62/66\n", tag());
	if(lpc_en & 0x0400)
		logerror("%s: Warning: mcu at 60/64\n", tag());
	if(lpc_en & 0x0200)
		logerror("%s: Warning: gameport at 208-20f\n", tag());
	if(lpc_en & 0x0100)
		logerror("%s: Warning: gameport at 200-207\n", tag());

	if(lpc_en & 0x0008) {
		uint16_t fdc = lpc_if_fdd_lpt_range & 0x10 ? 0x370 : 0x3f0;
		logerror("%s: Warning: floppy at %04x-%04x\n", tag(), fdc, fdc+7);
	}

	if(lpc_en & 0x0004) {
		static const uint16_t lpt_pos[4] = { 0x378, 0x278, 0x3bc, 0x000 };
		uint16_t lpt = lpt_pos[lpc_if_fdd_lpt_range & 3];
		if(lpt)
			logerror("%s: Warning: lpt at %04x-%04x %04x-%04x\n", tag(), lpt, lpt+7, lpt+0x400, lpt+0x407);
	}

	static const uint16_t com_pos[8] = { 0x3f8, 0x2f8, 0x220, 0x228, 0x238, 0x2e8, 0x338, 0x3e8 };

	if(lpc_en & 0x0002) {
		uint16_t comb = com_pos[(lpc_if_com_range >> 4) & 7];
		logerror("%s: Warning: comb at %04x-%04x\n", tag(), comb, comb+7);
	}

	if(lpc_en & 0x0001) {
		uint16_t coma = com_pos[lpc_if_com_range & 7];
		logerror("%s: Warning: coma at %04x-%04x\n", tag(), coma, coma+7);
	}

	rtc->map_device(memory_window_start, memory_window_end, 0, memory_space, io_window_start, io_window_end, 0, io_space);
	if(rtc_conf & 4)
		rtc->map_extdevice(memory_window_start, memory_window_end, 0, memory_space, io_window_start, io_window_end, 0, io_space);
	pit->map_device(memory_window_start, memory_window_end, 0, memory_space, io_window_start, io_window_end, 0, io_space);
#endif
}

// Southbridge
uint8_t i82371sb_isa_device::get_slave_ack(offs_t offset)
{
	if (offset == 2) // IRQ = 2
		return m_pic8259_slave->acknowledge();

	return 0x00;
}

void i82371sb_isa_device::at_speaker_set_spkrdata(uint8_t data)
{
	m_at_spkrdata = data ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}

void i82371sb_isa_device::at_pit8254_out0_changed(int state)
{
	if (m_pic8259_master)
		m_pic8259_master->ir0_w(state);
}

void i82371sb_isa_device::at_pit8254_out1_changed(int state)
{
	if (state)
		m_refresh = !m_refresh;
}

void i82371sb_isa_device::at_pit8254_out2_changed(int state)
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}

uint8_t i82371sb_isa_device::at_page8_r(offs_t offset)
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

void i82371sb_isa_device::at_page8_w(offs_t offset, uint8_t data)
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

void i82371sb_isa_device::pc_dma_hrq_changed(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237_2->hack_w(state);
}

uint8_t i82371sb_isa_device::pc_dma_read_byte(offs_t offset)
{
	address_space &prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if (m_dma_channel == -1)
		return 0xff;
	uint8_t result;
	offs_t page_offset = ((offs_t)m_dma_offset[0][m_dma_channel]) << 16;

	result = prog_space.read_byte(page_offset + offset);
	return result;
}

void i82371sb_isa_device::pc_dma_write_byte(offs_t offset, uint8_t data)
{
	address_space &prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if (m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t)m_dma_offset[0][m_dma_channel]) << 16;

	prog_space.write_byte(page_offset + offset, data);
}

uint8_t i82371sb_isa_device::pc_dma_read_word(offs_t offset)
{
	address_space &prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if (m_dma_channel == -1)
		return 0xff;
	uint16_t result;
	offs_t page_offset = ((offs_t)m_dma_offset[1][m_dma_channel & 3]) << 16;

	result = prog_space.read_word((page_offset & 0xfe0000) | (offset << 1));
	m_dma_high_byte = result & 0xFF00;

	return result & 0xFF;
}

void i82371sb_isa_device::pc_dma_write_word(offs_t offset, uint8_t data)
{
	address_space &prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if (m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t)m_dma_offset[1][m_dma_channel & 3]) << 16;

	prog_space.write_word((page_offset & 0xfe0000) | (offset << 1), m_dma_high_byte | data);
}

uint8_t i82371sb_isa_device::pc_dma8237_0_dack_r() { return m_isabus->dack_r(0); }
uint8_t i82371sb_isa_device::pc_dma8237_1_dack_r() { return m_isabus->dack_r(1); }
uint8_t i82371sb_isa_device::pc_dma8237_2_dack_r() { return m_isabus->dack_r(2); }
uint8_t i82371sb_isa_device::pc_dma8237_3_dack_r() { return m_isabus->dack_r(3); }
uint8_t i82371sb_isa_device::pc_dma8237_5_dack_r() { return m_isabus->dack_r(5); }
uint8_t i82371sb_isa_device::pc_dma8237_6_dack_r() { return m_isabus->dack_r(6); }
uint8_t i82371sb_isa_device::pc_dma8237_7_dack_r() { return m_isabus->dack_r(7); }

void i82371sb_isa_device::pc_dma8237_0_dack_w(uint8_t data) { m_isabus->dack_w(0, data); }
void i82371sb_isa_device::pc_dma8237_1_dack_w(uint8_t data) { m_isabus->dack_w(1, data); }
void i82371sb_isa_device::pc_dma8237_2_dack_w(uint8_t data) { m_isabus->dack_w(2, data); }
void i82371sb_isa_device::pc_dma8237_3_dack_w(uint8_t data) { m_isabus->dack_w(3, data); }
void i82371sb_isa_device::pc_dma8237_5_dack_w(uint8_t data) { m_isabus->dack_w(5, data); }
void i82371sb_isa_device::pc_dma8237_6_dack_w(uint8_t data) { m_isabus->dack_w(6, data); }
void i82371sb_isa_device::pc_dma8237_7_dack_w(uint8_t data) { m_isabus->dack_w(7, data); }

void i82371sb_isa_device::at_dma8237_out_eop(int state)
{
	m_cur_eop = state == ASSERT_LINE;
	if (m_dma_channel != -1)
		m_isabus->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE);
}

void i82371sb_isa_device::pc_select_dma_channel(int channel, bool state)
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

void i82371sb_isa_device::pc_dack0_w(int state) { pc_select_dma_channel(0, state); }
void i82371sb_isa_device::pc_dack1_w(int state) { pc_select_dma_channel(1, state); }
void i82371sb_isa_device::pc_dack2_w(int state) { pc_select_dma_channel(2, state); }
void i82371sb_isa_device::pc_dack3_w(int state) { pc_select_dma_channel(3, state); }
void i82371sb_isa_device::pc_dack4_w(int state) { m_dma8237_1->hack_w( state ? 0 : 1); } // it's inverted
void i82371sb_isa_device::pc_dack5_w(int state) { pc_select_dma_channel(5, state); }
void i82371sb_isa_device::pc_dack6_w(int state) { pc_select_dma_channel(6, state); }
void i82371sb_isa_device::pc_dack7_w(int state) { pc_select_dma_channel(7, state); }

void i82371sb_isa_device::redirect_irq(int irq, int state)
{
	switch (irq)
	{
	case 0:
	case 1:
	case 2:
	case 8:
	case 13:
		break;
	case 3:
		m_pic8259_master->ir3_w(state);
		break;
	case 4:
		m_pic8259_master->ir4_w(state);
		break;
	case 5:
		m_pic8259_master->ir5_w(state);
		break;
	case 6:
		m_pic8259_master->ir6_w(state);
		break;
	case 7:
		m_pic8259_master->ir7_w(state);
		break;
	case 9:
		m_pic8259_slave->ir1_w(state);
		break;
	case 10:
		m_pic8259_slave->ir2_w(state);
		break;
	case 11:
		m_pic8259_slave->ir3_w(state);
		break;
	case 12:
		m_pic8259_slave->ir4_w(state);
		break;
	case 14:
		m_pic8259_slave->ir6_w(state);
		break;
	case 15:
		m_pic8259_slave->ir7_w(state);
		break;
	}
}

void i82371sb_isa_device::pc_pirqa_w(int state)
{
	int irq = pirqrc[0] & 15;

	if (pirqrc[0] & 128)
		return;
	redirect_irq(irq, state);
}

void i82371sb_isa_device::pc_pirqb_w(int state)
{
	int irq = pirqrc[1] & 15;

	if (pirqrc[1] & 128)
		return;
	redirect_irq(irq, state);
}

void i82371sb_isa_device::pc_pirqc_w(int state)
{
	int irq = pirqrc[2] & 15;

	if (pirqrc[2] & 128)
		return;
	redirect_irq(irq, state);
}

void i82371sb_isa_device::pc_pirqd_w(int state)
{
	int irq = pirqrc[3] & 15;

	if (pirqrc[3] & 128)
		return;
	redirect_irq(irq, state);
}

void i82371sb_isa_device::pc_mirq0_w(int state)
{
	int irq = mbirq0 & 15;

	if (mbirq0 & (128 | 32))
		return;
	redirect_irq(irq, state);
}

// FIXME: this is PIIX specific, doesn't exist on PIIX3
void i82371sb_isa_device::pc_mirq1_w(int state)
{
	int irq = mbirq1 & 15;

	if (mbirq1 & 128)
		return;
	redirect_irq(irq, state);
}

void i82371sb_isa_device::pc_ferr_w(int state)
{
	if (!(xbcs & 32))
		return;
	m_pic8259_slave->ir5_w(state);
}

void i82371sb_isa_device::pc_extsmi_w(int state)
{
}

void i82371sb_isa_device::pc_irq1_w(int state)   { m_pic8259_master->ir1_w(state); }
void i82371sb_isa_device::pc_irq3_w(int state)   { m_pic8259_master->ir3_w(state); }
void i82371sb_isa_device::pc_irq4_w(int state)   { m_pic8259_master->ir4_w(state); }
void i82371sb_isa_device::pc_irq5_w(int state)   { m_pic8259_master->ir5_w(state); }
void i82371sb_isa_device::pc_irq6_w(int state)   { m_pic8259_master->ir6_w(state); }
void i82371sb_isa_device::pc_irq7_w(int state)   { m_pic8259_master->ir7_w(state); }
void i82371sb_isa_device::pc_irq8n_w(int state)  { m_pic8259_slave->ir0_w(state); }
void i82371sb_isa_device::pc_irq9_w(int state)   { m_pic8259_slave->ir1_w(state); }
void i82371sb_isa_device::pc_irq10_w(int state)  { m_pic8259_slave->ir2_w(state); }
void i82371sb_isa_device::pc_irq11_w(int state)  { m_pic8259_slave->ir3_w(state); }
void i82371sb_isa_device::pc_irq12m_w(int state) { m_pic8259_slave->ir4_w(state); }
void i82371sb_isa_device::pc_irq14_w(int state)  { m_pic8259_slave->ir6_w(state); }
void i82371sb_isa_device::pc_irq15_w(int state)  { m_pic8259_slave->ir7_w(state); }

uint8_t i82371sb_isa_device::at_portb_r()
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

void i82371sb_isa_device::at_portb_w(uint8_t data)
{
	m_at_speaker = data;
	m_pit8254->write_gate2(BIT(data, 0));
	at_speaker_set_spkrdata(BIT(data, 1));
	m_channel_check = BIT(data, 3);
	if (m_channel_check)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void i82371sb_isa_device::iochck_w(int state)
{
	if (!state && !m_channel_check && m_nmi_enabled)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

uint8_t i82371sb_isa_device::at_dma8237_2_r(offs_t offset)
{
	return m_dma8237_2->read(offset / 2);
}

void i82371sb_isa_device::at_dma8237_2_w(offs_t offset, uint8_t data)
{
	m_dma8237_2->write(offset / 2, data);
}

uint8_t i82371sb_isa_device::eisa_irq_read(offs_t offset)
{
	if (offset == 0)
		return m_eisa_irq_mode & 0xff;
	else
		return m_eisa_irq_mode >> 8;
}

void i82371sb_isa_device::eisa_irq_write(offs_t offset, uint8_t data)
{
	if (offset == 0)
		m_eisa_irq_mode = (m_eisa_irq_mode & 0xff00) | data;
	else
		m_eisa_irq_mode = (m_eisa_irq_mode & 0x00ff) | (data << 8);
	// TODO: update m_pic8259_master and m_pic8259_slave with the new configuration
}

uint8_t i82371sb_isa_device::read_apmcapms(offs_t offset)
{
	if (offset == 0)
		return apmc;
	else
		return apms;
}

void i82371sb_isa_device::write_apmcapms(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		apmc = data;
		if (smien & 128)
			smireq |= (1 << 7);
		update_smireq_line();
	}
	else
		apms = data;
	logerror("write apmcapms %d %02x\n", offset, data);
}

uint8_t i82371sb_isa_device::reset_control_r(offs_t offset)
{
	return reset_control;
}

void i82371sb_isa_device::reset_control_w(offs_t offset, uint8_t data)
{
	reset_control = data;
	logerror("reset_control = %02x\n", reset_control);
}

void i82371sb_isa_device::update_smireq_line()
{
	int m_smireq = smireq & 511;
	int m_smien = smien & 511;
	int needed = m_smireq & m_smien;
	int m_csmigate = smicntl & 1;

	if (needed && m_csmigate)
		m_smi_callback(1);
	else
		m_smi_callback(0);
}

int i82371sb_isa_device::pin_mapper(int pin)
{
	if(pin < 0 || pin >= 4 || (pirqrc[pin] & 0x80))
		return -1;
	return pirqrc[pin];
}

void i82371sb_isa_device::irq_handler(int line, int state)
{
	if(line < 0 && line >= 16)
		return;

	logerror("irq_handler %d %d\n", line, state);
	redirect_irq(line, state);
}


DEFINE_DEVICE_TYPE(I82371SB_IDE, i82371sb_ide_device, "i82371sb_ide", "Intel 82371 southbridge IDE interface")

void i82371sb_ide_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x06, 0x07).rw(FUNC(i82371sb_ide_device::status_r), FUNC(i82371sb_ide_device::status_w));
	map(0x0d, 0x0d).rw(FUNC(i82371sb_ide_device::latency_timer_r), FUNC(i82371sb_ide_device::latency_timer_w));
	map(0x10, 0x1f).noprw();
	map(0x20, 0x23).rw(FUNC(i82371sb_ide_device::bmiba_r), FUNC(i82371sb_ide_device::bmiba_w));
	map(0x24, 0x3b).noprw();
	map(0x40, 0x41).rw(FUNC(i82371sb_ide_device::idetim_primary_r), FUNC(i82371sb_ide_device::idetim_primary_w));
	map(0x42, 0x43).rw(FUNC(i82371sb_ide_device::idetim_secondary_r), FUNC(i82371sb_ide_device::idetim_secondary_w));
	map(0x44, 0x44).rw(FUNC(i82371sb_ide_device::sidetim_r), FUNC(i82371sb_ide_device::sidetim_w));
}

void i82371sb_ide_device::internal_io_map(address_map &map)
{
	map(0x0170, 0x0177).rw(FUNC(i82371sb_ide_device::ide2_read32_cs0_r), FUNC(i82371sb_ide_device::ide2_write32_cs0_w));
	map(0x01f0, 0x01f7).rw(FUNC(i82371sb_ide_device::ide1_read32_cs0_r), FUNC(i82371sb_ide_device::ide1_write32_cs0_w));
	map(0x0376, 0x0376).rw(FUNC(i82371sb_ide_device::ide2_read_cs1_r), FUNC(i82371sb_ide_device::ide2_write_cs1_w));
	map(0x03f6, 0x03f6).rw(FUNC(i82371sb_ide_device::ide1_read_cs1_r), FUNC(i82371sb_ide_device::ide1_write_cs1_w));
}

void i82371sb_ide_device::device_add_mconfig(machine_config &config)
{
	BUS_MASTER_IDE_CONTROLLER(config, m_ide1).options(ata_devices, "hdd", nullptr, false);
	m_ide1->irq_handler().set(FUNC(i82371sb_ide_device::primary_int));

	BUS_MASTER_IDE_CONTROLLER(config, m_ide2).options(ata_devices, "cdrom", nullptr, false);
	m_ide2->irq_handler().set(FUNC(i82371sb_ide_device::secondary_int));
}

void i82371sb_ide_device::device_config_complete()
{
	auto ide1 = m_ide1.finder_target();
	auto ide2 = m_ide2.finder_target();
	ide1.first.subdevice<bus_master_ide_controller_device>(ide1.second)->set_bus_master_space(m_maincpu, AS_PROGRAM);
	ide2.first.subdevice<bus_master_ide_controller_device>(ide2.second)->set_bus_master_space(m_maincpu, AS_PROGRAM);

	pci_device::device_config_complete();
}

i82371sb_ide_device::i82371sb_ide_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, type, tag, owner, clock)
	, latency_timer(0)
	, bmiba(1)
	, idetim_primary(0)
	, idetim_secondary(0)
	, sidetim(0)
	, m_irq_pri_callback(*this)
	, m_irq_sec_callback(*this)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_ide1(*this, "ide1")
	, m_ide2(*this, "ide2")
{
}

i82371sb_ide_device::i82371sb_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i82371sb_ide_device(mconfig, I82371SB_IDE, tag, owner, clock)
{
	set_ids(0x80867010, 0, 0x010180, 0x00000000);
}

void i82371sb_ide_device::device_start()
{
	pci_device::device_start();
	status = 0x0280;
	command = 2;
	command_mask = 5;
}

void i82371sb_ide_device::device_reset()
{
}

void i82371sb_ide_device::reset_all_mappings()
{
}

void i82371sb_ide_device::map_extra(
		uint64_t memory_window_start,
		uint64_t memory_window_end,
		uint64_t memory_offset,
		address_space *memory_space,
		uint64_t io_window_start,
		uint64_t io_window_end,
		uint64_t io_offset,
		address_space *io_space)
{
	io_space->install_device(0, 0x3ff, *this, &i82371sb_ide_device::internal_io_map);
	if (command & 1)
	{
		offs_t m_base = bmiba & 0xfff0;

		io_space->install_readwrite_handler(m_base, m_base + 0x7, read32s_delegate(*m_ide1, FUNC(bus_master_ide_controller_device::bmdma_r)), write32s_delegate(*m_ide1, FUNC(bus_master_ide_controller_device::bmdma_w)), 0xffffffff);
		io_space->install_readwrite_handler(m_base + 0x8, m_base + 0xf, read32s_delegate(*m_ide2, FUNC(bus_master_ide_controller_device::bmdma_r)), write32s_delegate(*m_ide2, FUNC(bus_master_ide_controller_device::bmdma_w)), 0xffffffff);
	}
}

void i82371sb_ide_device::primary_int(int state)
{
	m_irq_pri_callback(state);
}

void i82371sb_ide_device::secondary_int(int state)
{
	m_irq_sec_callback(state);
}

void i82371sb_ide_device::status_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// bits 11-13 are clear on write
	status = status & ~(data & (0x3800 & mem_mask));
	logerror("status = %04x\n", status);
}

uint8_t i82371sb_ide_device::latency_timer_r()
{
	return latency_timer;
}

void i82371sb_ide_device::latency_timer_w(uint8_t data)
{
	latency_timer = data;
	logerror("latency_timer = %02x\n", latency_timer);
}

uint32_t i82371sb_ide_device::bmiba_r()
{
	return bmiba;
}

void i82371sb_ide_device::bmiba_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	mem_mask &= 0xfff0;
	COMBINE_DATA(&bmiba);
	logerror("bmiba = %04x\n", bmiba);
	if (command & 1)
		remap_cb();
}

uint16_t i82371sb_ide_device::idetim_primary_r()
{
	return idetim_primary;
}

void i82371sb_ide_device::idetim_primary_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&idetim_primary);
	logerror("idetim_primary = %04x\n", idetim_primary);
}

uint16_t i82371sb_ide_device::idetim_secondary_r()
{
	return idetim_secondary;
}

void i82371sb_ide_device::idetim_secondary_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&idetim_secondary);
	logerror("idetim_secondary = %04x\n", idetim_secondary);
}

uint8_t i82371sb_ide_device::sidetim_r()
{
	return sidetim;
}

void i82371sb_ide_device::sidetim_w(uint8_t data)
{
	sidetim = data;
	logerror("sidetim = %02x\n", sidetim);
}

uint32_t i82371sb_ide_device::ide1_read32_cs0_r(offs_t offset, uint32_t mem_mask)
{
	if (!(command & 1))
		return 0xffffffff;
	return m_ide1->read_cs0(offset, mem_mask);
}

void i82371sb_ide_device::ide1_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (!(command & 1))
		return;
	m_ide1->write_cs0(offset, data, mem_mask);
}

uint32_t i82371sb_ide_device::ide2_read32_cs0_r(offs_t offset, uint32_t mem_mask)
{
	if (!(command & 1))
		return 0xffffffff;
	return m_ide2->read_cs0(offset, mem_mask);
}

void i82371sb_ide_device::ide2_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (!(command & 1))
		return;
	m_ide2->write_cs0(offset, data, mem_mask);
}

uint8_t i82371sb_ide_device::ide1_read_cs1_r()
{
	if (!(command & 1))
		return 0xff;
	return m_ide1->read_cs1(1, 0xff0000) >> 16;
}

void i82371sb_ide_device::ide1_write_cs1_w(uint8_t data)
{
	if (!(command & 1))
		return;
	m_ide1->write_cs1(1, data << 16, 0xff0000);
}

uint8_t i82371sb_ide_device::ide2_read_cs1_r()
{
	if (!(command & 1))
		return 0xff;
	return m_ide2->read_cs1(1, 0xff0000) >> 16;
}

void i82371sb_ide_device::ide2_write_cs1_w(uint8_t data)
{
	if (!(command & 1))
		return;
	m_ide2->write_cs1(1, data << 16, 0xff0000);
}
