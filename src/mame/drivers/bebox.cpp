// license:BSD-3-Clause
// copyright-holders:Nathan Woods, R. Belmont
/***************************************************************************

    drivers/bebox.c

    BeBox

***************************************************************************/

/* Core includes */
#include "emu.h"
#include "includes/bebox.h"

/* Components */
#include "video/clgd542x.h"
#include "bus/lpci/cirrus.h"
#include "sound/ymopl.h"
#include "machine/mc146818.h"
#include "machine/pckeybrd.h"
#include "bus/lpci/mpc105.h"
#include "bus/scsi/scsi.h"
#include "speaker.h"

/* Devices */
#include "bus/scsi/scsicd.h"
#include "bus/scsi/scsihd.h"
#include "formats/pc_dsk.h"
#include "machine/8042kbdc.h"

uint8_t bebox_state::at_dma8237_1_r(offs_t offset) { return m_dma8237[1]->read(offset / 2); }
void bebox_state::at_dma8237_1_w(offs_t offset, uint8_t data) { m_dma8237[1]->write(offset / 2, data); }

void bebox_state::main_mem(address_map &map)
{
	map(0x7FFFF0F0, 0x7FFFF0F7).rw(FUNC(bebox_state::bebox_cpu0_imask_r), FUNC(bebox_state::bebox_cpu0_imask_w));
	map(0x7FFFF1F0, 0x7FFFF1F7).rw(FUNC(bebox_state::bebox_cpu1_imask_r), FUNC(bebox_state::bebox_cpu1_imask_w));
	map(0x7FFFF2F0, 0x7FFFF2F7).r(FUNC(bebox_state::bebox_interrupt_sources_r));
	map(0x7FFFF3F0, 0x7FFFF3F7).rw(FUNC(bebox_state::bebox_crossproc_interrupts_r), FUNC(bebox_state::bebox_crossproc_interrupts_w));
	map(0x7FFFF4F0, 0x7FFFF4F7).w(FUNC(bebox_state::bebox_processor_resets_w));

	map(0x80000000, 0x8000001F).rw(m_dma8237[0], FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x80000020, 0x8000003F).rw(m_pic8259[0], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x80000040, 0x8000005f).rw(m_pit8254, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x80000060, 0x8000006F).rw("kbdc", FUNC(kbdc8042_device::data_r), FUNC(kbdc8042_device::data_w));
	map(0x80000070, 0x8000007F).rw("rtc", FUNC(mc146818_device::read), FUNC(mc146818_device::write));
	map(0x80000080, 0x8000009F).rw(FUNC(bebox_state::bebox_page_r), FUNC(bebox_state::bebox_page_w));
	map(0x800000A0, 0x800000BF).rw(m_pic8259[1], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x800000C0, 0x800000DF).rw(FUNC(bebox_state::at_dma8237_1_r), FUNC(bebox_state::at_dma8237_1_w));
	map(0x800001F0, 0x800001F7).rw("ide", FUNC(ide_controller_device::cs0_r), FUNC(ide_controller_device::cs0_w));
	map(0x800002F8, 0x800002FF).rw("ns16550_1", FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x80000380, 0x80000387).rw("ns16550_2", FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x80000388, 0x8000038F).rw("ns16550_3", FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x800003b0, 0x800003bf).rw(m_vga, FUNC(cirrus_gd5428_device::port_03b0_r), FUNC(cirrus_gd5428_device::port_03b0_w));
	map(0x800003c0, 0x800003cf).rw(m_vga, FUNC(cirrus_gd5428_device::port_03c0_r), FUNC(cirrus_gd5428_device::port_03c0_w));
	map(0x800003d0, 0x800003df).rw(m_vga, FUNC(cirrus_gd5428_device::port_03d0_r), FUNC(cirrus_gd5428_device::port_03d0_w));
	map(0x800003F0, 0x800003F7).rw("ide", FUNC(ide_controller_device::cs1_r), FUNC(ide_controller_device::cs1_w));
	map(0x800003F0, 0x800003F7).m(m_smc37c78, FUNC(smc37c78_device::map));
	map(0x800003F8, 0x800003FF).rw("ns16550_0", FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x80000480, 0x8000048F).rw(FUNC(bebox_state::bebox_80000480_r), FUNC(bebox_state::bebox_80000480_w));
	map(0x80000CF8, 0x80000CFF).rw(m_pcibus, FUNC(pci_bus_device::read_64be), FUNC(pci_bus_device::write_64be));
	//map(0x800042E8, 0x800042EF).w("cirrus", FUNC(cirrus_device::cirrus_42E8_w));

	map(0xBFFFFFF0, 0xBFFFFFFF).r(FUNC(bebox_state::bebox_interrupt_ack_r));
	map(0xC00A0000, 0xC00BFFFF).rw(m_vga, FUNC(cirrus_gd5428_device::mem_r), FUNC(cirrus_gd5428_device::mem_w));
	map(0xC1000000, 0xC11FFFFF).rw(m_vga, FUNC(cirrus_gd5428_device::mem_linear_r), FUNC(cirrus_gd5428_device::mem_linear_w));
	map(0xFFF00000, 0xFFF03FFF).bankr("bank2");
	map(0xFFF04000, 0xFFFFFFFF).rw(FUNC(bebox_state::bebox_flash_r), FUNC(bebox_state::bebox_flash_w));
}

// The following is a gross hack to let the BeBox boot ROM identify the processors correctly.
// This needs to be done in a better way if someone comes up with one.

uint64_t bebox_state::bb_slave_64be_r(offs_t offset, uint64_t mem_mask)
{
	// 2e94 is the real address, 2e84 is where the PC appears to be under full DRC
	if ((m_ppc[1]->pc() == 0xfff02e94) || (m_ppc[1]->pc() == 0xfff02e84))
	{
		return 0x108000ff;  // indicate slave CPU
	}

	return m_pcibus->read_64be(offset, mem_mask);
}

void bebox_state::slave_mem(address_map &map)
{
	main_mem(map);
	map(0x80000cf8, 0x80000cff).r(FUNC(bebox_state::bb_slave_64be_r));
	map(0x80000cf8, 0x80000cff).w(m_pcibus, FUNC(pci_bus_device::write_64be));
}

#define BYTE_REVERSE32(x)       (((x >> 24) & 0xff) | \
								((x >> 8) & 0xff00) | \
								((x << 8) & 0xff0000) | \
								((x << 24) & 0xff000000))

uint32_t bebox_state::scsi_fetch(uint32_t dsp)
{
	const uint32_t result = m_ppc[0]->space(AS_PROGRAM).read_dword(dsp & 0x7FFFFFFF);
	return BYTE_REVERSE32(result);
}


void bebox_state::scsi_irq_callback(int state)
{
	bebox_set_irq_bit(21, state);
}


void bebox_state::scsi_dma_callback(uint32_t src, uint32_t dst, int length, int byteswap)
{
}

void bebox_state::mpc105_config(device_t *device)
{
	mpc105_device &mpc105 = *downcast<mpc105_device *>(device);
	mpc105.set_cpu(":ppc1");
	mpc105.set_bank_base_default(0);
}

void bebox_state::cirrus_config(device_t *device)
{
	pci_cirrus_svga_device &cirruspci = *downcast<pci_cirrus_svga_device *>(device);
	cirruspci.set_vga(m_vga);
}

/*************************************
 *
 *  Keyboard
 *
 *************************************/

WRITE_LINE_MEMBER(bebox_state::bebox_keyboard_interrupt)
{
	bebox_set_irq_bit(16, state);
	m_pic8259[0]->ir1_w(state);
}

pci_connector_device &bebox_state::add_pci_slot(machine_config &config, const char *tag, size_t index, const char *default_tag)
{
	pci_connector_device &pcislot(PCI_CONNECTOR(config, tag, index));
	pcislot.option_add("mpc105", MPC105);
	pcislot.option_add("cirrus", PCI_CIRRUS_SVGA);
	pcislot.set_default_option(default_tag);
	pcislot.set_fixed(true);
	return pcislot;
}

void bebox_state::bebox_peripherals(machine_config &config)
{
	config.set_maximum_quantum(attotime::from_hz(60));

	PIT8254(config, m_pit8254, 0);
	m_pit8254->set_clk<0>(4772720/4); /* heartbeat IRQ */
	m_pit8254->set_clk<1>(4772720/4); /* dram refresh */
	m_pit8254->set_clk<2>(4772720/4); /* pio port c pin 4, and speaker polling */
	m_pit8254->out_handler<0>().set(FUNC(bebox_state::bebox_timer0_w));
	m_pit8254->out_handler<2>().set("kbdc", FUNC(kbdc8042_device::write_out2));

	AM9517A(config, m_dma8237[0], XTAL(14'318'181)/3);
	m_dma8237[0]->out_hreq_callback().set(FUNC(bebox_state::bebox_dma_hrq_changed));
	m_dma8237[0]->out_eop_callback().set(FUNC(bebox_state::bebox_dma8237_out_eop));
	m_dma8237[0]->in_memr_callback().set(FUNC(bebox_state::bebox_dma_read_byte));
	m_dma8237[0]->out_memw_callback().set(FUNC(bebox_state::bebox_dma_write_byte));
	m_dma8237[0]->in_ior_callback<2>().set(m_smc37c78, FUNC(smc37c78_device::dma_r));
	m_dma8237[0]->out_iow_callback<2>().set(m_smc37c78, FUNC(smc37c78_device::dma_w));
	m_dma8237[0]->out_dack_callback<0>().set(FUNC(bebox_state::pc_dack0_w));
	m_dma8237[0]->out_dack_callback<1>().set(FUNC(bebox_state::pc_dack1_w));
	m_dma8237[0]->out_dack_callback<2>().set(FUNC(bebox_state::pc_dack2_w));
	m_dma8237[0]->out_dack_callback<3>().set(FUNC(bebox_state::pc_dack3_w));

	AM9517A(config, m_dma8237[1], XTAL(14'318'181)/3);

	PIC8259(config, m_pic8259[0], 0);
	m_pic8259[0]->out_int_callback().set(FUNC(bebox_state::bebox_pic8259_master_set_int_line));
	m_pic8259[0]->in_sp_callback().set_constant(1);
	m_pic8259[0]->read_slave_ack_callback().set(m_pic8259[1], FUNC(pic8259_device::acknowledge));

	PIC8259(config, m_pic8259[1], 0);
	m_pic8259[1]->out_int_callback().set(FUNC(bebox_state::bebox_pic8259_slave_set_int_line));
	m_pic8259[1]->in_sp_callback().set_constant(0);

	NS16550(config, "ns16550_0", 0);   /* TODO: Verify model */
	NS16550(config, "ns16550_1", 0);   /* TODO: Verify model */
	NS16550(config, "ns16550_2", 0);   /* TODO: Verify model */
	NS16550(config, "ns16550_3", 0);   /* TODO: Verify model */

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_vga, FUNC(cirrus_gd5428_device::screen_update));

	CIRRUS_GD5428(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x200000);

	speaker_device &speaker(SPEAKER(config, "mono"));
	speaker.front_center();

	ym3812_device &ym3812(YM3812(config, "ym3812", 3579545));
	ym3812.add_route(ALL_OUTPUTS, speaker, 1.0);

	FUJITSU_29F016A(config, "flash");

	scsi_port_device &scsibus(SCSI_PORT(config, "scsi"));
	scsibus.set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_0));
	scsibus.set_slot_device(2, "cdrom", SCSICD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_3));

	lsi53c810_device &scsictrl(LSI53C810(config, "lsi53c810"));
	scsictrl.set_irq_callback(FUNC(bebox_state::scsi_irq_callback));
	scsictrl.set_dma_callback(FUNC(bebox_state::scsi_dma_callback));
	scsictrl.set_fetch_callback(FUNC(bebox_state::scsi_fetch));
	scsictrl.set_scsi_port("scsi");

	ide_controller_device &idectrl(IDE_CONTROLLER(config, "ide"));
	idectrl.options(ata_devices, "hdd", nullptr, false);
	idectrl.irq_handler().set(FUNC(bebox_state::bebox_ide_interrupt));

	/* pci */
	PCI_BUS(config, m_pcibus, 0);
	m_pcibus->set_busnum(0);

	pci_connector_device &pcislot0 = add_pci_slot(config, "pcibus:0", 0, "mpc105");
	pcislot0.set_option_machine_config("mpc105", [this](device_t *device) { mpc105_config(device); });
	pci_connector_device &pcislot1 = add_pci_slot(config, "pcibus:1", 1, "cirrus");
	pcislot1.set_option_machine_config("cirrus", [this](device_t *device) { cirrus_config(device); });

	/*PCI_BUS_LEGACY_DEVICE(12, nullptr, scsi53c810_pci_read, scsi53c810_pci_write)*/

	SMC37C78(config, m_smc37c78, 24'000'000);
	m_smc37c78->intrq_wr_callback().set(FUNC(bebox_state::fdc_interrupt));
	m_smc37c78->drq_wr_callback().set(m_dma8237[0], FUNC(am9517a_device::dreq2_w));

	floppy_connector &fdc(FLOPPY_CONNECTOR(config, "smc37c78:0"));
	fdc.option_add("35hd", FLOPPY_35_HD);
	fdc.set_default_option("35hd");
	fdc.set_formats(floppy_image_device::default_pc_floppy_formats);

	MC146818(config, "rtc", 32.768_kHz_XTAL);

	kbdc8042_device &kbdc(KBDC8042(config, "kbdc"));
	kbdc.set_keyboard_type(kbdc8042_device::KBDC8042_STANDARD);
	kbdc.system_reset_callback().set_inputline(m_ppc[0], INPUT_LINE_RESET);
	kbdc.input_buffer_full_callback().set(FUNC(bebox_state::bebox_keyboard_interrupt));

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("32M");
	m_ram->set_extra_options("8M,16M");
}

void bebox_state::bebox(machine_config &config)
{
	PPC603(config, m_ppc[0], 66000000);  /* 66 MHz */
	m_ppc[0]->set_addrmap(AS_PROGRAM, &bebox_state::main_mem);

	PPC603(config, m_ppc[1], 66000000);  /* 66 MHz */
	m_ppc[1]->set_addrmap(AS_PROGRAM, &bebox_state::slave_mem);

	bebox_peripherals(config);
}

void bebox_state::bebox2(machine_config &config)
{
	PPC603E(config, m_ppc[0], 133000000);  /* 133 MHz */
	m_ppc[0]->set_addrmap(AS_PROGRAM, &bebox_state::main_mem);

	PPC603E(config, m_ppc[1], 133000000);  /* 133 MHz */
	m_ppc[1]->set_addrmap(AS_PROGRAM, &bebox_state::slave_mem);

	bebox_peripherals(config);
}

static INPUT_PORTS_START( bebox )
INPUT_PORTS_END

ROM_START(bebox)
	ROM_REGION(0x00200000, "user1", ROMREGION_64BIT | ROMREGION_BE)
	ROM_LOAD( "bootmain.rom", 0x000000, 0x20000, CRC(df2d19e0) SHA1(da86a7d23998dc953dd96a2ac5684faaa315c701) )
	ROM_REGION(0x4000, "user2", ROMREGION_64BIT | ROMREGION_BE)
	ROM_LOAD( "bootnub.rom", 0x000000, 0x4000, CRC(5348d09a) SHA1(1b637a3d7a2b072aa128dd5c037bbb440d525c1a) )
ROM_END

ROM_START(bebox2)
	ROM_REGION(0x00200000, "user1", ROMREGION_64BIT | ROMREGION_BE)
	ROM_LOAD( "bootmain.rom", 0x000000, 0x20000, CRC(df2d19e0) SHA1(da86a7d23998dc953dd96a2ac5684faaa315c701) )
	ROM_REGION(0x4000, "user2", ROMREGION_64BIT | ROMREGION_BE)
	ROM_LOAD( "bootnub.rom", 0x000000, 0x4000, CRC(5348d09a) SHA1(1b637a3d7a2b072aa128dd5c037bbb440d525c1a) )
ROM_END

/*    YEAR   NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY   FULLNAME */
COMP( 1995,  bebox,  0,      0,      bebox,   bebox, bebox_state, init_bebox, "Be Inc", "BeBox Dual603-66",  MACHINE_NOT_WORKING )
COMP( 1996,  bebox2, bebox,  0,      bebox2,  bebox, bebox_state, init_bebox, "Be Inc", "BeBox Dual603-133", MACHINE_NOT_WORKING )
