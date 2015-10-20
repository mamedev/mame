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
#include "cpu/powerpc/ppc.h"
#include "sound/3812intf.h"
#include "machine/mc146818.h"
#include "machine/pckeybrd.h"
#include "bus/lpci/mpc105.h"
#include "bus/scsi/scsi.h"

/* Devices */
#include "bus/scsi/scsicd.h"
#include "bus/scsi/scsihd.h"
#include "formats/pc_dsk.h"
#include "machine/8042kbdc.h"

READ8_MEMBER(bebox_state::at_dma8237_1_r)  { return m_dma8237_2->read(space, offset / 2); }
WRITE8_MEMBER(bebox_state::at_dma8237_1_w) { m_dma8237_2->write(space, offset / 2, data); }

static ADDRESS_MAP_START( bebox_mem, AS_PROGRAM, 64, bebox_state )
	AM_RANGE(0x7FFFF0F0, 0x7FFFF0F7) AM_READWRITE(bebox_cpu0_imask_r, bebox_cpu0_imask_w )
	AM_RANGE(0x7FFFF1F0, 0x7FFFF1F7) AM_READWRITE(bebox_cpu1_imask_r, bebox_cpu1_imask_w )
	AM_RANGE(0x7FFFF2F0, 0x7FFFF2F7) AM_READ(bebox_interrupt_sources_r )
	AM_RANGE(0x7FFFF3F0, 0x7FFFF3F7) AM_READWRITE(bebox_crossproc_interrupts_r, bebox_crossproc_interrupts_w )
	AM_RANGE(0x7FFFF4F0, 0x7FFFF4F7) AM_WRITE(bebox_processor_resets_w )

	AM_RANGE(0x80000000, 0x8000001F) AM_DEVREADWRITE8("dma8237_1", am9517a_device, read, write, U64(0xffffffffffffffff) )
	AM_RANGE(0x80000020, 0x8000003F) AM_DEVREADWRITE8("pic8259_1", pic8259_device, read, write, U64(0xffffffffffffffff) )
	AM_RANGE(0x80000040, 0x8000005f) AM_DEVREADWRITE8("pit8254", pit8254_device, read, write, U64(0xffffffffffffffff) )
	AM_RANGE(0x80000060, 0x8000006F) AM_DEVREADWRITE8("kbdc", kbdc8042_device, data_r, data_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x80000070, 0x8000007F) AM_DEVREADWRITE8("rtc", mc146818_device, read, write , U64(0xffffffffffffffff) )
	AM_RANGE(0x80000080, 0x8000009F) AM_READWRITE8(bebox_page_r, bebox_page_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x800000A0, 0x800000BF) AM_DEVREADWRITE8("pic8259_2", pic8259_device, read, write, U64(0xffffffffffffffff) )
	AM_RANGE(0x800000C0, 0x800000DF) AM_READWRITE8(at_dma8237_1_r, at_dma8237_1_w, U64(0xffffffffffffffff))
	AM_RANGE(0x800001F0, 0x800001F7) AM_DEVREADWRITE16("ide", ide_controller_device, read_cs0, write_cs0, U64(0xffffffffffffffff) )
	AM_RANGE(0x800002F8, 0x800002FF) AM_DEVREADWRITE8( "ns16550_1", ns16550_device, ins8250_r, ins8250_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x80000380, 0x80000387) AM_DEVREADWRITE8( "ns16550_2", ns16550_device, ins8250_r, ins8250_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x80000388, 0x8000038F) AM_DEVREADWRITE8( "ns16550_3", ns16550_device, ins8250_r, ins8250_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x800003b0, 0x800003bf) AM_DEVREADWRITE8("vga", cirrus_gd5428_device, port_03b0_r, port_03b0_w, U64(0xffffffffffffffff))
	AM_RANGE(0x800003c0, 0x800003cf) AM_DEVREADWRITE8("vga", cirrus_gd5428_device, port_03c0_r, port_03c0_w, U64(0xffffffffffffffff))
	AM_RANGE(0x800003d0, 0x800003df) AM_DEVREADWRITE8("vga", cirrus_gd5428_device, port_03d0_r, port_03d0_w, U64(0xffffffffffffffff))
	AM_RANGE(0x800003F0, 0x800003F7) AM_DEVREADWRITE16("ide", ide_controller_device, read_cs1, write_cs1, U64(0xffffffffffffffff) )
	AM_RANGE(0x800003F0, 0x800003F7) AM_DEVICE8( "smc37c78", smc37c78_device, map, U64(0xffffffffffffffff) )
	AM_RANGE(0x800003F8, 0x800003FF) AM_DEVREADWRITE8( "ns16550_0",ns16550_device,  ins8250_r, ins8250_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x80000480, 0x8000048F) AM_READWRITE8(bebox_80000480_r, bebox_80000480_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x80000CF8, 0x80000CFF) AM_DEVREADWRITE("pcibus", pci_bus_device, read_64be, write_64be )
	//AM_RANGE(0x800042E8, 0x800042EF) AM_DEVWRITE8("cirrus", cirrus_device, cirrus_42E8_w, U64(0xffffffffffffffff) )

	AM_RANGE(0xBFFFFFF0, 0xBFFFFFFF) AM_READ(bebox_interrupt_ack_r )
	AM_RANGE(0xC00A0000, 0XC00BFFFF) AM_DEVREADWRITE8("vga", cirrus_gd5428_device, mem_r, mem_w, U64(0xffffffffffffffff) )
	AM_RANGE(0xC1000000, 0XC11FFFFF) AM_DEVREADWRITE8("vga", cirrus_gd5428_device, mem_linear_r, mem_linear_w, U64(0xffffffffffffffff) )
	AM_RANGE(0xFFF00000, 0xFFF03FFF) AM_ROMBANK("bank2")
	AM_RANGE(0xFFF04000, 0xFFFFFFFF) AM_READWRITE8(bebox_flash_r, bebox_flash_w, U64(0xffffffffffffffff) )
ADDRESS_MAP_END

// The following is a gross hack to let the BeBox boot ROM identify the processors correctly.
// This needs to be done in a better way if someone comes up with one.

READ64_MEMBER(bebox_state::bb_slave_64be_r)
{
	// 2e94 is the real address, 2e84 is where the PC appears to be under full DRC
	if ((space.device().safe_pc() == 0xfff02e94) || (space.device().safe_pc() == 0xfff02e84))
	{
		return 0x108000ff;  // indicate slave CPU
	}

	return m_pcibus->read_64be(space, offset, mem_mask);
}

static ADDRESS_MAP_START( bebox_slave_mem, AS_PROGRAM, 64, bebox_state )
	AM_RANGE(0x80000cf8, 0x80000cff) AM_READ(bb_slave_64be_r)
	AM_RANGE(0x80000cf8, 0x80000cff) AM_DEVWRITE("pcibus", pci_bus_device, write_64be )
	AM_IMPORT_FROM(bebox_mem)
ADDRESS_MAP_END

#define BYTE_REVERSE32(x)       (((x >> 24) & 0xff) | \
								((x >> 8) & 0xff00) | \
								((x << 8) & 0xff0000) | \
								((x << 24) & 0xff000000))

LSI53C810_FETCH_CB(bebox_state::scsi_fetch)
{
	UINT32 result = m_ppc1->space(AS_PROGRAM).read_dword(dsp & 0x7FFFFFFF);
	return BYTE_REVERSE32(result);
}


LSI53C810_IRQ_CB(bebox_state::scsi_irq_callback)
{
	bebox_set_irq_bit(21, state);
}


LSI53C810_DMA_CB(bebox_state::scsi_dma_callback)
{
}

FLOPPY_FORMATS_MEMBER( bebox_state::floppy_formats )
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( bebox_floppies )
	SLOT_INTERFACE( "35hd", FLOPPY_35_HD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( mpc105_config )
	MCFG_MPC105_CPU( "ppc1" )
	MCFG_MPC105_BANK_BASE_DEFAULT( 0 )
MACHINE_CONFIG_END


/*************************************
 *
 *  Keyboard
 *
 *************************************/

WRITE_LINE_MEMBER(bebox_state::bebox_keyboard_interrupt)
{
	bebox_set_irq_bit(16, state);
	m_pic8259_1->ir1_w(state);
}

static SLOT_INTERFACE_START( pci_devices )
	SLOT_INTERFACE_INTERNAL("mpc105", MPC105)
	SLOT_INTERFACE("cirrus", CIRRUS)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( bebox, bebox_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("ppc1", PPC603, 66000000)  /* 66 MHz */
	MCFG_CPU_PROGRAM_MAP(bebox_mem)

	MCFG_CPU_ADD("ppc2", PPC603, 66000000)  /* 66 MHz */
	MCFG_CPU_PROGRAM_MAP(bebox_slave_mem)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_DEVICE_ADD("pit8254", PIT8254, 0)
	MCFG_PIT8253_CLK0(4772720/4) /* heartbeat IRQ */
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(bebox_state, bebox_timer0_w))
	MCFG_PIT8253_CLK1(4772720/4) /* dram refresh */
	MCFG_PIT8253_CLK2(4772720/4) /* pio port c pin 4, and speaker polling enough */
	MCFG_PIT8253_OUT2_HANDLER(DEVWRITELINE("kbdc", kbdc8042_device, write_out2))

	MCFG_DEVICE_ADD( "dma8237_1", AM9517A, XTAL_14_31818MHz/3 )
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(bebox_state, bebox_dma_hrq_changed))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(bebox_state, bebox_dma8237_out_eop))
	MCFG_I8237_IN_MEMR_CB(READ8(bebox_state, bebox_dma_read_byte))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(bebox_state, bebox_dma_write_byte))
	MCFG_I8237_IN_IOR_2_CB(READ8(bebox_state, bebox_dma8237_fdc_dack_r))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(bebox_state, bebox_dma8237_fdc_dack_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(bebox_state, pc_dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(bebox_state, pc_dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(bebox_state, pc_dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(bebox_state, pc_dack3_w))

	MCFG_DEVICE_ADD( "dma8237_2", AM9517A, XTAL_14_31818MHz/3 )

	MCFG_PIC8259_ADD( "pic8259_1", WRITELINE(bebox_state,bebox_pic8259_master_set_int_line), VCC, READ8(bebox_state,get_slave_ack) )

	MCFG_PIC8259_ADD( "pic8259_2", WRITELINE(bebox_state,bebox_pic8259_slave_set_int_line), GND, NULL )

	MCFG_DEVICE_ADD( "ns16550_0", NS16550, 0 )   /* TODO: Verify model */
	MCFG_DEVICE_ADD( "ns16550_1", NS16550, 0 )   /* TODO: Verify model */
	MCFG_DEVICE_ADD( "ns16550_2", NS16550, 0 )   /* TODO: Verify model */
	MCFG_DEVICE_ADD( "ns16550_3", NS16550, 0 )   /* TODO: Verify model */

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_cirrus_gd5428 )


	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ym3812", YM3812, 3579545)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_FUJITSU_29F016A_ADD("flash")

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE2, "cdrom", SCSICD, SCSI_ID_3)

	MCFG_DEVICE_ADD("lsi53c810", LSI53C810, 0)
	MCFG_LSI53C810_IRQ_CB(bebox_state, scsi_irq_callback)
	MCFG_LSI53C810_DMA_CB(bebox_state, scsi_dma_callback)
	MCFG_LSI53C810_FETCH_CB(bebox_state, scsi_fetch)
	MCFG_LEGACY_SCSI_PORT("scsi")

	MCFG_IDE_CONTROLLER_ADD( "ide", ata_devices, "hdd", NULL, false ) /* FIXME */
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(bebox_state, bebox_ide_interrupt))

	/* pci */
	MCFG_PCI_BUS_ADD("pcibus", 0)
	MCFG_PCI_BUS_DEVICE("pcibus:0", pci_devices, "mpc105", true)
	MCFG_SLOT_OPTION_MACHINE_CONFIG("mpc105", mpc105_config)

	MCFG_PCI_BUS_DEVICE("pcibus:1", pci_devices, "cirrus", true)

	/*MCFG_PCI_BUS_DEVICE(12, NULL, scsi53c810_pci_read, scsi53c810_pci_write)*/

	MCFG_SMC37C78_ADD("smc37c78")
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(bebox_state, fdc_interrupt))
	MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE("dma8237_1", am9517a_device, dreq2_w))
	MCFG_FLOPPY_DRIVE_ADD("smc37c78:0", bebox_floppies, "35hd", bebox_state::floppy_formats)

	MCFG_MC146818_ADD( "rtc", XTAL_32_768kHz )

	MCFG_DEVICE_ADD("kbdc", KBDC8042, 0)
	MCFG_KBDC8042_KEYBOARD_TYPE(KBDC8042_STANDARD)
	MCFG_KBDC8042_SYSTEM_RESET_CB(INPUTLINE("ppc1", INPUT_LINE_RESET))
	MCFG_KBDC8042_INPUT_BUFFER_FULL_CB(WRITELINE(bebox_state, bebox_keyboard_interrupt))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32M")
	MCFG_RAM_EXTRA_OPTIONS("8M,16M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bebox2, bebox )
	MCFG_CPU_REPLACE("ppc1", PPC603E, 133000000)    /* 133 MHz */
	MCFG_CPU_PROGRAM_MAP(bebox_mem)

	MCFG_CPU_REPLACE("ppc2", PPC603E, 133000000)    /* 133 MHz */
	MCFG_CPU_PROGRAM_MAP(bebox_slave_mem)
MACHINE_CONFIG_END

static INPUT_PORTS_START( bebox )
	PORT_INCLUDE( at_keyboard )
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

/*     YEAR   NAME      PARENT  COMPAT  MACHINE   INPUT     INIT    COMPANY             FULLNAME */
COMP( 1995,  bebox,    0,      0,      bebox,    bebox, bebox_state,    bebox,   "Be Inc",  "BeBox Dual603-66", MACHINE_NOT_WORKING )
COMP( 1996,  bebox2,   bebox,  0,      bebox2,   bebox, bebox_state,    bebox,   "Be Inc",  "BeBox Dual603-133", MACHINE_NOT_WORKING )
