/***************************************************************************

    drivers/bebox.c

    BeBox

***************************************************************************/

/* Core includes */
#include "emu.h"
#include "includes/bebox.h"

/* Components */
#include "video/pc_vga.h"
#include "video/cirrus.h"
#include "cpu/powerpc/ppc.h"
#include "sound/3812intf.h"
#include "machine/ins8250.h"
#include "machine/pic8259.h"
#include "machine/mc146818.h"
#include "machine/pc_fdc.h"
#include "machine/pci.h"
#include "machine/8237dma.h"
#include "machine/pckeybrd.h"
#include "machine/8042kbdc.h"
#include "machine/pit8253.h"
#include "machine/idectrl.h"
#include "machine/mpc105.h"
#include "machine/intelfsh.h"
#include "machine/scsibus.h"
#include "machine/53c810.h"

/* Devices */
#include "machine/scsicd.h"
#include "machine/scsihd.h"
#include "imagedev/flopdrv.h"
#include "formats/pc_dsk.h"
#include "machine/ram.h"

static READ8_HANDLER(at_dma8237_1_r)  { return i8237_r(space->machine().device("dma8237_2"), offset / 2); }
static WRITE8_HANDLER(at_dma8237_1_w) { i8237_w(space->machine().device("dma8237_2"), offset / 2, data); }

static ADDRESS_MAP_START( bebox_mem, AS_PROGRAM, 64, bebox_state )
	AM_RANGE(0x7FFFF0F0, 0x7FFFF0F7) AM_READWRITE_LEGACY(bebox_cpu0_imask_r, bebox_cpu0_imask_w )
	AM_RANGE(0x7FFFF1F0, 0x7FFFF1F7) AM_READWRITE_LEGACY(bebox_cpu1_imask_r, bebox_cpu1_imask_w )
	AM_RANGE(0x7FFFF2F0, 0x7FFFF2F7) AM_READ_LEGACY(bebox_interrupt_sources_r )
	AM_RANGE(0x7FFFF3F0, 0x7FFFF3F7) AM_READWRITE_LEGACY(bebox_crossproc_interrupts_r, bebox_crossproc_interrupts_w )
	AM_RANGE(0x7FFFF4F0, 0x7FFFF4F7) AM_WRITE_LEGACY(bebox_processor_resets_w )

	AM_RANGE(0x80000000, 0x8000001F) AM_DEVREADWRITE8_LEGACY("dma8237_1", i8237_r, i8237_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x80000020, 0x8000003F) AM_DEVREADWRITE8_LEGACY("pic8259_master", pic8259_r, pic8259_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x80000040, 0x8000005f) AM_DEVREADWRITE8_LEGACY("pit8254", pit8253_r, pit8253_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x80000060, 0x8000006F) AM_READWRITE8_LEGACY(kbdc8042_8_r, kbdc8042_8_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x80000070, 0x8000007F) AM_DEVREADWRITE8("rtc", mc146818_device, read, write , U64(0xffffffffffffffff) )
	AM_RANGE(0x80000080, 0x8000009F) AM_READWRITE8_LEGACY(bebox_page_r, bebox_page_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x800000A0, 0x800000BF) AM_DEVREADWRITE8_LEGACY("pic8259_slave", pic8259_r, pic8259_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x800000C0, 0x800000DF) AM_READWRITE8_LEGACY(at_dma8237_1_r, at_dma8237_1_w, U64(0xffffffffffffffff))
	AM_RANGE(0x800001F0, 0x800001F7) AM_READWRITE8_LEGACY(bebox_800001F0_r, bebox_800001F0_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x800002F8, 0x800002FF) AM_DEVREADWRITE8( "ns16550_1", ns16550_device, ins8250_r, ins8250_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x80000380, 0x80000387) AM_DEVREADWRITE8( "ns16550_2", ns16550_device, ins8250_r, ins8250_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x80000388, 0x8000038F) AM_DEVREADWRITE8( "ns16550_3", ns16550_device, ins8250_r, ins8250_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x800003F0, 0x800003F7) AM_READWRITE_LEGACY(bebox_800003F0_r, bebox_800003F0_w )
	AM_RANGE(0x800003F8, 0x800003FF) AM_DEVREADWRITE8( "ns16550_0",ns16550_device,  ins8250_r, ins8250_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x80000480, 0x8000048F) AM_READWRITE8_LEGACY(bebox_80000480_r, bebox_80000480_w, U64(0xffffffffffffffff) )
	AM_RANGE(0x80000CF8, 0x80000CFF) AM_DEVREADWRITE("pcibus", pci_bus_device, read_64be, write_64be )
	//AM_RANGE(0x800042E8, 0x800042EF) AM_DEVWRITE8_LEGACY("cirrus", cirrus_42E8_w, U64(0xffffffffffffffff) )

	AM_RANGE(0xBFFFFFF0, 0xBFFFFFFF) AM_READ_LEGACY(bebox_interrupt_ack_r )

	AM_RANGE(0xFFF00000, 0xFFF03FFF) AM_ROMBANK("bank2")
	AM_RANGE(0xFFF04000, 0xFFFFFFFF) AM_READWRITE8_LEGACY(bebox_flash_r, bebox_flash_w, U64(0xffffffffffffffff) )
ADDRESS_MAP_END

// The following is a gross hack to let the BeBox boot ROM identify the processors correctly.
// This needs to be done in a better way if someone comes up with one.

static READ64_HANDLER(bb_slave_64be_r)
{
	pci_bus_device *device = space->machine().device<pci_bus_device>("pcibus");

	// 2e94 is the real address, 2e84 is where the PC appears to be under full DRC
	if ((cpu_get_pc(&space->device()) == 0xfff02e94) || (cpu_get_pc(&space->device()) == 0xfff02e84))
	{
		return 0x108000ff;	// indicate slave CPU
	}

	return device->read_64be(*space, offset, mem_mask);
}

static ADDRESS_MAP_START( bebox_slave_mem, AS_PROGRAM, 64, bebox_state )
	AM_RANGE(0x80000cf8, 0x80000cff) AM_READ_LEGACY(bb_slave_64be_r)
	AM_RANGE(0x80000cf8, 0x80000cff) AM_DEVWRITE("pcibus", pci_bus_device, write_64be )
	AM_IMPORT_FROM(bebox_mem)
ADDRESS_MAP_END

#define BYTE_REVERSE32(x)		(((x >> 24) & 0xff) | \
								((x >> 8) & 0xff00) | \
								((x << 8) & 0xff0000) | \
								((x << 24) & 0xff000000))

static UINT32 scsi53c810_fetch(running_machine &machine, UINT32 dsp)
{
	UINT32 result;
	result = machine.device("ppc1")->memory().space(AS_PROGRAM)->read_dword(dsp & 0x7FFFFFFF);
	return BYTE_REVERSE32(result);
}


static void scsi53c810_irq_callback(running_machine &machine, int value)
{
	bebox_set_irq_bit(machine, 21, value);
}


static void scsi53c810_dma_callback(running_machine &machine, UINT32 src, UINT32 dst, int length, int byteswap)
{
}


static const SCSIBus_interface scsibus_intf =
{
};

static const struct LSI53C810interface lsi53c810_intf =
{
	&scsi53c810_irq_callback,
	&scsi53c810_dma_callback,
	&scsi53c810_fetch,
};


static const floppy_interface bebox_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(pc),
	NULL,
	NULL
};

const struct mpc105_interface mpc105_config =
{
	"ppc1",
	0
};

static SLOT_INTERFACE_START( pci_devices )
	SLOT_INTERFACE_INTERNAL("mpc105", MPC105)
	SLOT_INTERFACE("cirrus", CIRRUS)
SLOT_INTERFACE_END

static const ide_config ide_intf = 
{
	bebox_ide_interrupt, 
	NULL, 
	0
};

static MACHINE_CONFIG_START( bebox, bebox_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("ppc1", PPC603, 66000000)	/* 66 MHz */
	MCFG_CPU_PROGRAM_MAP(bebox_mem)

	MCFG_CPU_ADD("ppc2", PPC603, 66000000)	/* 66 MHz */
	MCFG_CPU_PROGRAM_MAP(bebox_slave_mem)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_PIT8254_ADD( "pit8254", bebox_pit8254_config )

	MCFG_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, bebox_dma8237_1_config )

	MCFG_I8237_ADD( "dma8237_2", XTAL_14_31818MHz/3, bebox_dma8237_2_config )

	MCFG_PIC8259_ADD( "pic8259_master", bebox_pic8259_master_config )

	MCFG_PIC8259_ADD( "pic8259_slave", bebox_pic8259_slave_config )

	MCFG_NS16550_ADD( "ns16550_0", bebox_uart_inteface_0, 0 )	/* TODO: Verify model */
	MCFG_NS16550_ADD( "ns16550_1", bebox_uart_inteface_1, 0 )	/* TODO: Verify model */
	MCFG_NS16550_ADD( "ns16550_2", bebox_uart_inteface_2, 0 )	/* TODO: Verify model */
	MCFG_NS16550_ADD( "ns16550_3", bebox_uart_inteface_3, 0 )	/* TODO: Verify model */

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_vga )

	MCFG_MACHINE_START( bebox )
	MCFG_MACHINE_RESET( bebox )

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ym3812", YM3812, 3579545)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_FUJITSU_29F016A_ADD("flash")

	MCFG_SCSIBUS_ADD("scsi", scsibus_intf)
	MCFG_SCSIDEV_ADD("scsi:harddisk1", SCSIHD, SCSI_ID_0)
	MCFG_SCSIDEV_ADD("scsi:cdrom", SCSICD, SCSI_ID_3)
	MCFG_LSI53C810_ADD( "scsi:lsi53c810", lsi53c810_intf)

	MCFG_IDE_CONTROLLER_ADD( "ide", ide_intf, ide_image_devices, "hdd", NULL, false )	/* FIXME */

	/* pci */
	MCFG_PCI_BUS_ADD("pcibus", 0)
	MCFG_PCI_BUS_DEVICE("pcibus:0", pci_devices, "mpc105", NULL, &mpc105_config, 0, true)
	MCFG_PCI_BUS_DEVICE("pcibus:1", pci_devices, "cirrus", NULL, NULL,	   0, true)

	/*MCFG_PCI_BUS_DEVICE(12, NULL, scsi53c810_pci_read, scsi53c810_pci_write)*/

	MCFG_SMC37C78_ADD("smc37c78", pc_fdc_upd765_connected_1_drive_interface)

	MCFG_LEGACY_FLOPPY_DRIVE_ADD(FLOPPY_0, bebox_floppy_interface)

	MCFG_MC146818_ADD( "rtc", MC146818_STANDARD )

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32M")
	MCFG_RAM_EXTRA_OPTIONS("8M,16M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bebox2, bebox )
	MCFG_CPU_REPLACE("ppc1", PPC603E, 133000000)	/* 133 MHz */
	MCFG_CPU_PROGRAM_MAP(bebox_mem)

	MCFG_CPU_REPLACE("ppc2", PPC603E, 133000000)	/* 133 MHz */
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
COMP( 1995,  bebox,    0,      0,      bebox,    bebox, bebox_state,    bebox,   "Be Inc",  "BeBox Dual603-66", GAME_NOT_WORKING )
COMP( 1996,  bebox2,   bebox,  0,      bebox2,   bebox, bebox_state,    bebox,   "Be Inc",  "BeBox Dual603-133", GAME_NOT_WORKING )
