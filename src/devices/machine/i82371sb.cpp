// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "i82371sb.h"
#include "cpu/i386/i386.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE(I82371SB_ISA, i82371sb_isa_device, "i82371sb_isa", "Intel 82371 southbridge ISA bridge")

void i82371sb_isa_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x4c, 0x4c).rw(this, FUNC(i82371sb_isa_device::iort_r), FUNC(i82371sb_isa_device::iort_w));
	map(0x4e, 0x4f).rw(this, FUNC(i82371sb_isa_device::xbcs_r), FUNC(i82371sb_isa_device::xbcs_w));
	map(0x60, 0x63).rw(this, FUNC(i82371sb_isa_device::pirqrc_r), FUNC(i82371sb_isa_device::pirqrc_w));
	map(0x68, 0x68).rw(this, FUNC(i82371sb_isa_device::tom_r), FUNC(i82371sb_isa_device::tom_w));
	map(0x6a, 0x6b).rw(this, FUNC(i82371sb_isa_device::mstat_r), FUNC(i82371sb_isa_device::mstat_w));
	map(0x70, 0x70).rw(this, FUNC(i82371sb_isa_device::mbirq0_r), FUNC(i82371sb_isa_device::mbirq0_w));
	map(0x76, 0x77).rw(this, FUNC(i82371sb_isa_device::mbdma_r), FUNC(i82371sb_isa_device::mbdma_w));
	map(0x82, 0x82).rw(this, FUNC(i82371sb_isa_device::dlc_r), FUNC(i82371sb_isa_device::dlc_w));
	map(0xa0, 0xa0).rw(this, FUNC(i82371sb_isa_device::smicntl_r), FUNC(i82371sb_isa_device::smicntl_w));
	map(0xa2, 0xa3).rw(this, FUNC(i82371sb_isa_device::smien_r), FUNC(i82371sb_isa_device::smien_w));
	map(0xa4, 0xa7).rw(this, FUNC(i82371sb_isa_device::see_r), FUNC(i82371sb_isa_device::see_w));
	map(0xa8, 0xa8).rw(this, FUNC(i82371sb_isa_device::ftmr_r), FUNC(i82371sb_isa_device::ftmr_w));
	map(0xaa, 0xab).rw(this, FUNC(i82371sb_isa_device::smireq_r), FUNC(i82371sb_isa_device::smireq_w));
	map(0xac, 0xac).rw(this, FUNC(i82371sb_isa_device::ctltmr_r), FUNC(i82371sb_isa_device::ctltmr_w));
	map(0xae, 0xae).rw(this, FUNC(i82371sb_isa_device::cthtmr_r), FUNC(i82371sb_isa_device::cthtmr_w));
}

void i82371sb_isa_device::internal_io_map(address_map &map)
{
	map(0x0000, 0x001f).rw("dma8237_1", FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x003f).rw("pic8259_master", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x005f).rw("pit8254", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0061, 0x0061).rw(this, FUNC(i82371sb_isa_device::at_portb_r), FUNC(i82371sb_isa_device::at_portb_w));
	map(0x0080, 0x009f).rw(this, FUNC(i82371sb_isa_device::at_page8_r), FUNC(i82371sb_isa_device::at_page8_w));
	map(0x00a0, 0x00bf).rw("pic8259_slave", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x00b2, 0x00b3).rw(this, FUNC(i82371sb_isa_device::read_apmcapms), FUNC(i82371sb_isa_device::write_apmcapms));
	map(0x00c0, 0x00df).rw(this, FUNC(i82371sb_isa_device::at_dma8237_2_r), FUNC(i82371sb_isa_device::at_dma8237_2_w));
	map(0x00e0, 0x00ef).noprw();
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(i82371sb_isa_device::device_add_mconfig)
	MCFG_DEVICE_ADD("pit8254", PIT8254, 0)
	MCFG_PIT8253_CLK0(4772720/4) /* heartbeat IRQ */
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(*this, i82371sb_isa_device, at_pit8254_out0_changed))
	MCFG_PIT8253_CLK1(4772720/4) /* dram refresh */
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(*this, i82371sb_isa_device, at_pit8254_out1_changed))
	MCFG_PIT8253_CLK2(4772720/4) /* pio port c pin 4, and speaker polling enough */
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(*this, i82371sb_isa_device, at_pit8254_out2_changed))

	MCFG_DEVICE_ADD( "dma8237_1", AM9517A, XTAL(14'318'181)/3 )
	MCFG_I8237_OUT_HREQ_CB(WRITELINE("dma8237_2", am9517a_device, dreq0_w))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(*this, i82371sb_isa_device, at_dma8237_out_eop))
	MCFG_I8237_IN_MEMR_CB(READ8(*this, i82371sb_isa_device, pc_dma_read_byte))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(*this, i82371sb_isa_device, pc_dma_write_byte))
	MCFG_I8237_IN_IOR_0_CB(READ8(*this, i82371sb_isa_device, pc_dma8237_0_dack_r))
	MCFG_I8237_IN_IOR_1_CB(READ8(*this, i82371sb_isa_device, pc_dma8237_1_dack_r))
	MCFG_I8237_IN_IOR_2_CB(READ8(*this, i82371sb_isa_device, pc_dma8237_2_dack_r))
	MCFG_I8237_IN_IOR_3_CB(READ8(*this, i82371sb_isa_device, pc_dma8237_3_dack_r))
	MCFG_I8237_OUT_IOW_0_CB(WRITE8(*this, i82371sb_isa_device, pc_dma8237_0_dack_w))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(*this, i82371sb_isa_device, pc_dma8237_1_dack_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(*this, i82371sb_isa_device, pc_dma8237_2_dack_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(*this, i82371sb_isa_device, pc_dma8237_3_dack_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(*this, i82371sb_isa_device, pc_dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(*this, i82371sb_isa_device, pc_dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(*this, i82371sb_isa_device, pc_dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(*this, i82371sb_isa_device, pc_dack3_w))

	MCFG_DEVICE_ADD( "dma8237_2", AM9517A, XTAL(14'318'181)/3 )
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(*this, i82371sb_isa_device, pc_dma_hrq_changed))
	MCFG_I8237_IN_MEMR_CB(READ8(*this, i82371sb_isa_device, pc_dma_read_word))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(*this, i82371sb_isa_device, pc_dma_write_word))
	MCFG_I8237_IN_IOR_1_CB(READ8(*this, i82371sb_isa_device, pc_dma8237_5_dack_r))
	MCFG_I8237_IN_IOR_2_CB(READ8(*this, i82371sb_isa_device, pc_dma8237_6_dack_r))
	MCFG_I8237_IN_IOR_3_CB(READ8(*this, i82371sb_isa_device, pc_dma8237_7_dack_r))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(*this, i82371sb_isa_device, pc_dma8237_5_dack_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(*this, i82371sb_isa_device, pc_dma8237_6_dack_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(*this, i82371sb_isa_device, pc_dma8237_7_dack_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(*this, i82371sb_isa_device, pc_dack4_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(*this, i82371sb_isa_device, pc_dack5_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(*this, i82371sb_isa_device, pc_dack6_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(*this, i82371sb_isa_device, pc_dack7_w))

	MCFG_DEVICE_ADD("pic8259_master", PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(INPUTLINE(":maincpu", 0))
	MCFG_PIC8259_IN_SP_CB(VCC)
	MCFG_PIC8259_CASCADE_ACK_CB(READ8(*this, i82371sb_isa_device, get_slave_ack))

	MCFG_DEVICE_ADD("pic8259_slave", PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(WRITELINE("pic8259_master", pic8259_device, ir2_w))
	MCFG_PIC8259_IN_SP_CB(GND)

//  MCFG_BUS_MASTER_IDE_CONTROLLER_ADD("ide", ata_devices, "hdd", nullptr, false)
//  MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE("pic8259_slave", pic8259_device, ir6_w))
//  MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE(":maincpu", AS_PROGRAM)
//
//  MCFG_BUS_MASTER_IDE_CONTROLLER_ADD("ide2", ata_devices, "cdrom", nullptr, false)
//  MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE("pic8259_slave", pic8259_device, ir7_w))
//  MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE(":maincpu", AS_PROGRAM)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DEVICE_ADD("isabus", ISA16, 0)
	MCFG_ISA16_CPU(":maincpu")
	MCFG_ISA16_BUS_CUSTOM_SPACES()
	MCFG_ISA_OUT_IRQ2_CB(WRITELINE("pic8259_slave",  pic8259_device, ir2_w)) // in place of irq 2 on at irq 9 is used
	MCFG_ISA_OUT_IRQ3_CB(WRITELINE("pic8259_master", pic8259_device, ir3_w))
	MCFG_ISA_OUT_IRQ4_CB(WRITELINE("pic8259_master", pic8259_device, ir4_w))
	MCFG_ISA_OUT_IRQ5_CB(WRITELINE("pic8259_master", pic8259_device, ir5_w))
	MCFG_ISA_OUT_IRQ6_CB(WRITELINE("pic8259_master", pic8259_device, ir6_w))
	MCFG_ISA_OUT_IRQ7_CB(WRITELINE("pic8259_master", pic8259_device, ir7_w))
	MCFG_ISA_OUT_IRQ10_CB(WRITELINE("pic8259_slave", pic8259_device, ir3_w))
	MCFG_ISA_OUT_IRQ11_CB(WRITELINE("pic8259_slave", pic8259_device, ir4_w))
	MCFG_ISA_OUT_IRQ12_CB(WRITELINE("pic8259_slave", pic8259_device, ir5_w))
	MCFG_ISA_OUT_IRQ14_CB(WRITELINE("pic8259_slave", pic8259_device, ir6_w))
	MCFG_ISA_OUT_IRQ15_CB(WRITELINE("pic8259_slave", pic8259_device, ir7_w))
	MCFG_ISA_OUT_DRQ0_CB(WRITELINE("dma8237_1", am9517a_device, dreq0_w))
	MCFG_ISA_OUT_DRQ1_CB(WRITELINE("dma8237_1", am9517a_device, dreq1_w))
	MCFG_ISA_OUT_DRQ2_CB(WRITELINE("dma8237_1", am9517a_device, dreq2_w))
	MCFG_ISA_OUT_DRQ3_CB(WRITELINE("dma8237_1", am9517a_device, dreq3_w))
	MCFG_ISA_OUT_DRQ5_CB(WRITELINE("dma8237_2", am9517a_device, dreq1_w))
	MCFG_ISA_OUT_DRQ6_CB(WRITELINE("dma8237_2", am9517a_device, dreq2_w))
	MCFG_ISA_OUT_DRQ7_CB(WRITELINE("dma8237_2", am9517a_device, dreq3_w))
MACHINE_CONFIG_END


i82371sb_isa_device::i82371sb_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, I82371SB_ISA, tag, owner, clock),
	m_smi_callback(*this),
	m_boot_state_hook(*this),
	m_maincpu(*this, ":maincpu"),
	m_pic8259_master(*this, "pic8259_master"),
	m_pic8259_slave(*this, "pic8259_slave"),
	m_dma8237_1(*this, "dma8237_1"),
	m_dma8237_2(*this, "dma8237_2"),
	m_pit8254(*this, "pit8254"),
	m_isabus(*this, "isabus"),
	m_speaker(*this, "speaker"),
	m_at_spkrdata(0), m_pit_out2(0), m_dma_channel(0), m_cur_eop(false), m_dma_high_byte(0), m_at_speaker(0), m_refresh(false), m_channel_check(0), m_nmi_enabled(0)
{
}

void i82371sb_isa_device::device_start()
{
	pci_device::device_start();
	m_smi_callback.resolve();
	m_boot_state_hook.resolve();
}

void i82371sb_isa_device::device_reset()
{
	pci_device::device_reset();

	iort = 0x4d;
	xbcs = 0x0003;
	memset(pirqrc, 0x80, sizeof(pirqrc));
	tom = 0x02;
	mstat = 0x0000;
	mbirq0 = 0x80;
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

WRITE8_MEMBER (i82371sb_isa_device::boot_state_w)
{
	m_boot_state_hook((offs_t)0, data);
}

WRITE8_MEMBER (i82371sb_isa_device::nop_w)
{
}

READ8_MEMBER (i82371sb_isa_device::iort_r)
{
	return iort;
}

WRITE8_MEMBER(i82371sb_isa_device::iort_w)
{
	iort = data;
	logerror("iort = %02x\n", iort);
}

READ16_MEMBER (i82371sb_isa_device::xbcs_r)
{
	return xbcs;
}

WRITE16_MEMBER(i82371sb_isa_device::xbcs_w)
{
	COMBINE_DATA(&xbcs);
	logerror("xbcs = %04x\n", xbcs);
}

READ8_MEMBER (i82371sb_isa_device::pirqrc_r)
{
	return pirqrc[offset];
}

WRITE8_MEMBER(i82371sb_isa_device::pirqrc_w)
{
	pirqrc[offset] = data;
	logerror("pirqrc[%d] = %02x\n", offset, pirqrc[offset]);
}

READ8_MEMBER (i82371sb_isa_device::tom_r)
{
	return tom;
}

WRITE8_MEMBER(i82371sb_isa_device::tom_w)
{
	tom = data;
	logerror("tom = %02x\n", tom);
}

READ16_MEMBER (i82371sb_isa_device::mstat_r)
{
	return mstat;
}

WRITE16_MEMBER(i82371sb_isa_device::mstat_w)
{
	COMBINE_DATA(&mstat);
	logerror("mstat = %04x\n", mstat);
}

READ8_MEMBER (i82371sb_isa_device::mbirq0_r)
{
	return mbirq0;
}

WRITE8_MEMBER(i82371sb_isa_device::mbirq0_w)
{
	mbirq0 = data;
	logerror("mbirq0 = %02x\n", mbirq0);
}

READ8_MEMBER (i82371sb_isa_device::mbdma_r)
{
	return mbdma[offset];
}

WRITE8_MEMBER(i82371sb_isa_device::mbdma_w)
{
	mbdma[offset] = data;
	logerror("mbdma[%d] = %02x\n", offset, mbdma);
}

READ8_MEMBER (i82371sb_isa_device::apicbase_r)
{
	return apicbase;
}

WRITE8_MEMBER(i82371sb_isa_device::apicbase_w)
{
	apicbase = data;
	logerror("apicbase = %02x\n", apicbase);
}

READ8_MEMBER (i82371sb_isa_device::dlc_r)
{
	return dlc;
}

WRITE8_MEMBER(i82371sb_isa_device::dlc_w)
{
	dlc = data;
	logerror("dlc = %02x\n", dlc);
}

READ8_MEMBER (i82371sb_isa_device::smicntl_r)
{
	return smicntl;
}

WRITE8_MEMBER(i82371sb_isa_device::smicntl_w)
{
	smicntl = data;
	update_smireq_line();
	logerror("smicntl = %02x\n", smicntl);
}

READ16_MEMBER (i82371sb_isa_device::smien_r)
{
	return smien;
}

WRITE16_MEMBER(i82371sb_isa_device::smien_w)
{
	COMBINE_DATA(&smien);
	update_smireq_line();
	logerror("smien = %04x\n", smien);
}

READ32_MEMBER (i82371sb_isa_device::see_r)
{
	return see;
}

WRITE32_MEMBER(i82371sb_isa_device::see_w)
{
	COMBINE_DATA(&see);
	logerror("see = %08x\n", see);
}

READ8_MEMBER (i82371sb_isa_device::ftmr_r)
{
	return ftmr;
}

WRITE8_MEMBER(i82371sb_isa_device::ftmr_w)
{
	ftmr = data;
	logerror("ftmr = %02x\n", ftmr);
}

READ16_MEMBER (i82371sb_isa_device::smireq_r)
{
	return smireq;
}

WRITE16_MEMBER(i82371sb_isa_device::smireq_w)
{
	COMBINE_DATA(&smireq);
	update_smireq_line();
	logerror("smireq = %04x\n", smireq);
}

READ8_MEMBER (i82371sb_isa_device::ctltmr_r)
{
	return ctlmtr;
}

WRITE8_MEMBER(i82371sb_isa_device::ctltmr_w)
{
	ctlmtr = data;
	logerror("ctlmtr = %02x\n", ctlmtr);
}

READ8_MEMBER (i82371sb_isa_device::cthtmr_r)
{
	return cthmtr;
}

WRITE8_MEMBER(i82371sb_isa_device::cthtmr_w)
{
	cthmtr = data;
	logerror("cthmtr = %02x\n", cthmtr);
}

void i82371sb_isa_device::map_bios(address_space *memory_space, uint32_t start, uint32_t end)
{
	uint32_t mask = m_region->bytes() - 1;
	memory_space->install_rom(start, end, m_region->base() + (start & mask));
}

void i82371sb_isa_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	io_space->install_readwrite_handler(0, 0xffff, read16_delegate(FUNC(isa16_device::io16_r), &(*m_isabus)), write16_delegate(FUNC(isa16_device::io16_w), &(*m_isabus)));
	memory_space->install_readwrite_handler(0, 0xfffff, read16_delegate(FUNC(isa16_device::mem16_r), &(*m_isabus)), write16_delegate(FUNC(isa16_device::mem16_w), &(*m_isabus)));

	// assume that map_extra of the southbridge is called before the one of the northbridge
	map_bios(memory_space, 0xfffc0000, 0xffffffff);
	map_bios(memory_space, 0x000e0000, 0x000fffff);
	io_space->install_device(0, 0xff, *this, &i82371sb_isa_device::internal_io_map);

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
READ8_MEMBER( i82371sb_isa_device::get_slave_ack )
{
	if (offset==2) // IRQ = 2
		return m_pic8259_slave->acknowledge();

	return 0x00;
}

void i82371sb_isa_device::at_speaker_set_spkrdata(uint8_t data)
{
	m_at_spkrdata = data ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}



WRITE_LINE_MEMBER( i82371sb_isa_device::at_pit8254_out0_changed )
{
	if (m_pic8259_master)
		m_pic8259_master->ir0_w(state);
}

WRITE_LINE_MEMBER( i82371sb_isa_device::at_pit8254_out1_changed )
{
	if(state)
		m_refresh = !m_refresh;
}

WRITE_LINE_MEMBER( i82371sb_isa_device::at_pit8254_out2_changed )
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}

READ8_MEMBER( i82371sb_isa_device::at_page8_r )
{
	uint8_t data = m_at_pages[offset % 0x10];

	switch(offset % 8)
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


WRITE8_MEMBER( i82371sb_isa_device::at_page8_w )
{
	m_at_pages[offset % 0x10] = data;

	switch(offset % 8)
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


WRITE_LINE_MEMBER( i82371sb_isa_device::pc_dma_hrq_changed )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237_2->hack_w( state );
}

READ8_MEMBER(i82371sb_isa_device::pc_dma_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return 0xff;
	uint8_t result;
	offs_t page_offset = ((offs_t) m_dma_offset[0][m_dma_channel]) << 16;

	result = prog_space.read_byte(page_offset + offset);
	return result;
}


WRITE8_MEMBER(i82371sb_isa_device::pc_dma_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t) m_dma_offset[0][m_dma_channel]) << 16;

	prog_space.write_byte(page_offset + offset, data);
}


READ8_MEMBER(i82371sb_isa_device::pc_dma_read_word)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return 0xff;
	uint16_t result;
	offs_t page_offset = ((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16;

	result = prog_space.read_word((page_offset & 0xfe0000) | (offset << 1));
	m_dma_high_byte = result & 0xFF00;

	return result & 0xFF;
}


WRITE8_MEMBER(i82371sb_isa_device::pc_dma_write_word)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16;

	prog_space.write_word((page_offset & 0xfe0000) | (offset << 1), m_dma_high_byte | data);
}


READ8_MEMBER( i82371sb_isa_device::pc_dma8237_0_dack_r ) { return m_isabus->dack_r(0); }
READ8_MEMBER( i82371sb_isa_device::pc_dma8237_1_dack_r ) { return m_isabus->dack_r(1); }
READ8_MEMBER( i82371sb_isa_device::pc_dma8237_2_dack_r ) { return m_isabus->dack_r(2); }
READ8_MEMBER( i82371sb_isa_device::pc_dma8237_3_dack_r ) { return m_isabus->dack_r(3); }
READ8_MEMBER( i82371sb_isa_device::pc_dma8237_5_dack_r ) { return m_isabus->dack_r(5); }
READ8_MEMBER( i82371sb_isa_device::pc_dma8237_6_dack_r ) { return m_isabus->dack_r(6); }
READ8_MEMBER( i82371sb_isa_device::pc_dma8237_7_dack_r ) { return m_isabus->dack_r(7); }


WRITE8_MEMBER( i82371sb_isa_device::pc_dma8237_0_dack_w ) { m_isabus->dack_w(0, data); }
WRITE8_MEMBER( i82371sb_isa_device::pc_dma8237_1_dack_w ) { m_isabus->dack_w(1, data); }
WRITE8_MEMBER( i82371sb_isa_device::pc_dma8237_2_dack_w ) { m_isabus->dack_w(2, data); }
WRITE8_MEMBER( i82371sb_isa_device::pc_dma8237_3_dack_w ) { m_isabus->dack_w(3, data); }
WRITE8_MEMBER( i82371sb_isa_device::pc_dma8237_5_dack_w ) { m_isabus->dack_w(5, data); }
WRITE8_MEMBER( i82371sb_isa_device::pc_dma8237_6_dack_w ) { m_isabus->dack_w(6, data); }
WRITE8_MEMBER( i82371sb_isa_device::pc_dma8237_7_dack_w ) { m_isabus->dack_w(7, data); }

WRITE_LINE_MEMBER( i82371sb_isa_device::at_dma8237_out_eop )
{
	m_cur_eop = state == ASSERT_LINE;
	if(m_dma_channel != -1)
		m_isabus->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE );
}

void i82371sb_isa_device::pc_select_dma_channel(int channel, bool state)
{
	if(!state) {
		m_dma_channel = channel;
		if(m_cur_eop)
			m_isabus->eop_w(channel, ASSERT_LINE );
	} else if(m_dma_channel == channel) {
		m_dma_channel = -1;
		if(m_cur_eop)
			m_isabus->eop_w(channel, CLEAR_LINE );
	}
}


WRITE_LINE_MEMBER( i82371sb_isa_device::pc_dack0_w ) { pc_select_dma_channel(0, state); }
WRITE_LINE_MEMBER( i82371sb_isa_device::pc_dack1_w ) { pc_select_dma_channel(1, state); }
WRITE_LINE_MEMBER( i82371sb_isa_device::pc_dack2_w ) { pc_select_dma_channel(2, state); }
WRITE_LINE_MEMBER( i82371sb_isa_device::pc_dack3_w ) { pc_select_dma_channel(3, state); }
WRITE_LINE_MEMBER( i82371sb_isa_device::pc_dack4_w ) { m_dma8237_1->hack_w( state ? 0 : 1); } // it's inverted
WRITE_LINE_MEMBER( i82371sb_isa_device::pc_dack5_w ) { pc_select_dma_channel(5, state); }
WRITE_LINE_MEMBER( i82371sb_isa_device::pc_dack6_w ) { pc_select_dma_channel(6, state); }
WRITE_LINE_MEMBER( i82371sb_isa_device::pc_dack7_w ) { pc_select_dma_channel(7, state); }

READ8_MEMBER( i82371sb_isa_device::at_portb_r )
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

WRITE8_MEMBER( i82371sb_isa_device::at_portb_w )
{
	m_at_speaker = data;
	m_pit8254->write_gate2(BIT(data, 0));
	at_speaker_set_spkrdata( BIT(data, 1));
	m_channel_check = BIT(data, 3);
	m_isabus->set_nmi_state((m_nmi_enabled==0) && (m_channel_check==0));
}

READ8_MEMBER( i82371sb_isa_device::at_dma8237_2_r )
{
	return m_dma8237_2->read( space, offset / 2);
}

WRITE8_MEMBER( i82371sb_isa_device::at_dma8237_2_w )
{
	m_dma8237_2->write( space, offset / 2, data);
}

READ8_MEMBER( i82371sb_isa_device::read_apmcapms )
{
	if (offset == 0)
		return apmc;
	else
		return apms;
}

WRITE8_MEMBER( i82371sb_isa_device::write_apmcapms )
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
