/*
    Konami Viper System

    Driver by Ville Linde



    Games on this hardware:

    Game ID       Year    Game
    ------------------------------------------------------------------------------------------------------
    GK922         2000    Code One Dispatch
    G????         2001    ParaParaParadise 2nd Mix
    GM941         2001    GTI Club 2
    G?A00         2001    Police 911 (USA) / Police 24/7 (World) / Keisatsukan Shinjuku 24ji (Japan)
    GKA13         2001    Silent Scope EX (USA/World) / Sogeki (Japan)
    G?A29         2001    Mocap Boxing
    G?A30         2001    Tsurugi
    GMA41         2001    Thrill Drive 2
    G?A45         2001    Boxing Mania
    G?B11         2001    Police 911 2 (USA) / Police 24/7 2 (World) / Keisatsukan Shinjuku 24ji 2 (Japan)
    G?B33         2001    Mocap Golf
    G?B41         2001    Jurassic Park 3
    G?B4x         2002    Xtrial Racing
    G?C00         2003    Pop'n Music 9
    G?C09         2002    Mahjong Fight Club
    G?C22         2002    World Combat (USA/Japan/Korea) / Warzaid (Europe)

DASM code snippets:

00FE0B8C: addi      r31,r3,0x0000
00FE0B90: lwz       r3,0x0040(r1)
00FE0B94: cmpi      r31,0x0000 ;offending check, understand where r3 comes from!
00FE0B98: lwz       r4,0x0044(r1)
00FE0B9C: addic     r5,r1,0x0058
00FE0BA0: bne       0x00FE0C00

*/

#include "emu.h"
#include "cpu/powerpc/ppc.h"
#include "machine/pci.h"
#include "memconv.h"
#include "machine/idectrl.h"
#include "machine/timekpr.h"
#include "video/voodoo.h"

//#define VIPER_DEBUG_LOG

static VIDEO_UPDATE(viper)
{
	running_device *device = screen->machine->device("voodoo");
	return voodoo_update(device, bitmap, cliprect) ? 0 : UPDATE_HAS_NOT_CHANGED;
}


/*****************************************************************************/

static UINT32 mpc8240_regs[256/4];
static UINT32 mpc8240_pci_r(running_device *busdevice, running_device *device, int function, int reg, UINT32 mem_mask)
{
	#ifdef VIPER_DEBUG_LOG
	printf("MPC8240: PCI read %d, %02X, %08X\n", function, reg, mem_mask);
	#endif

	switch (reg)
	{
	}

	return mpc8240_regs[reg/4];
}

static void mpc8240_pci_w(running_device *busdevice, running_device *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
	#ifdef VIPER_DEBUG_LOG
	printf("MPC8240: PCI write %d, %02X, %08X, %08X\n", function, reg, data, mem_mask);
	#endif

	COMBINE_DATA(mpc8240_regs + (reg/4));
}


static READ64_DEVICE_HANDLER( pci_config_addr_r )
{
	return pci_64be_r(device, 0, U64(0x00000000ffffffff));
}

static WRITE64_DEVICE_HANDLER( pci_config_addr_w )
{
	pci_64be_w(device, 0, data, U64(0x00000000ffffffff));
}

static READ64_DEVICE_HANDLER( pci_config_data_r )
{
	return pci_64be_r(device, 1, U64(0xffffffff00000000)) << 32;
}

static WRITE64_DEVICE_HANDLER( pci_config_data_w )
{
	pci_64be_w(device, 1, data >> 32, U64(0xffffffff00000000));
}



static UINT32 epic_iack;

static READ32_HANDLER( epic_r )
{
	int reg;
	reg = offset * 4;

	#ifdef VIPER_DEBUG_LOG
	printf("EPIC: read %08X, %08X at %08X\n", reg, mem_mask, cpu_get_pc(space->cpu));
	#endif

	switch (reg >> 16)
	{
		// 0x60000 - 0x6FFFF
		case 0x6:
		{
			switch (reg & 0xffff)
			{
				case 0x00a0:			// IACK
					return epic_iack;

			}
			break;
		}
	}

	return 0;
}

static WRITE32_HANDLER( epic_w )
{
	int reg;
	reg = offset * 4;

	#ifdef VIPER_DEBUG_LOG
	printf("EPIC: write %08X, %08X, %08X at %08X\n", data, reg, mem_mask, cpu_get_pc(space->cpu));
	#endif

	switch (reg >> 16)
	{
		// 0x60000 - 0x6FFFF
		case 0x6:
		{
			switch (reg & 0xffff)
			{
				case 0x00b0:			// EOI
					epic_iack = 0xff;
					break;
			}
			break;
		}
	}
}

static READ64_HANDLER(epic_64be_r)
{
	return read64be_with_32le_handler(epic_r, space, offset, mem_mask);
}
static WRITE64_HANDLER(epic_64be_w)
{
	write64be_with_32le_handler(epic_w, space, offset, data, mem_mask);
}


static int cf_card_ide = 0;

static const UINT8 cf_card_tuples[] =
{
	0x01,		// Device Tuple
	0x01,		// Tuple size
	0xd0,		// Device Type Func Spec

	0x1a,		// Config Tuple
	0xff,		// Tuple size (last?)
	0x03,		// CCR base size
	0x00,		// last config index?
	0x00, 0x01, 0x00, 0x00,		// CCR base (0x00000100)
};

static READ64_DEVICE_HANDLER(cf_card_data_r)
{
	UINT64 r = 0;

	if (ACCESSING_BITS_16_31)
	{
		switch (offset & 0xf)
		{
			case 0x8:	// Duplicate Even RD Data
			{
				r |= ide_bus_r(device, 0, 0) << 16;
				break;
			}

			default:
			{
				fatalerror("%s:cf_card_data_r: IDE reg %02X\n", cpuexec_describe_context(device->machine), offset & 0xf);
			}
		}
	}
	return r;
}

static WRITE64_DEVICE_HANDLER(cf_card_data_w)
{
	if (ACCESSING_BITS_16_31)
	{
		switch (offset & 0xf)
		{
			case 0x8:	// Duplicate Even RD Data
			{
				ide_bus_w(device, 0, 0, (data >> 16) & 0xffff);
				break;
			}

			default:
			{
				fatalerror("%s:cf_card_data_w: IDE reg %02X, %04X\n", cpuexec_describe_context(device->machine), offset & 0xf, (UINT16)(data >> 16));
			}
		}
	}
}

static READ64_DEVICE_HANDLER(cf_card_r)
{
	UINT64 r = 0;

	if (ACCESSING_BITS_16_31)
	{
		if (cf_card_ide)
		{
			switch (offset & 0xf)
			{
				case 0x0:	// Even RD Data
				case 0x1:	// Error
				case 0x2:	// Sector Count
				case 0x3:	// Sector No.
				case 0x4:	// Cylinder Low
				case 0x5:	// Cylinder High
				case 0x6:	// Select Card/Head
				case 0x7:	// Status
				{
					r |= ide_bus_r(device, 0, offset & 7) << 16;
					break;
				}

				//case 0x8: // Duplicate Even RD Data
				//case 0x9: // Duplicate Odd RD Data

				case 0xd:	// Duplicate Error
				{
					r |= ide_bus_r(device, 0, 1) << 16;
					break;
				}
				case 0xe:	// Alt Status
				case 0xf:	// Drive Address
				{
					r |= ide_bus_r(device, 1, offset & 7) << 16;
					break;
				}

				default:
				{
					printf("%s:compact_flash_r: IDE reg %02X\n", cpuexec_describe_context(device->machine), offset & 0xf);
				}
			}
		}
		else
		{
			int reg = offset;

			//printf("cf_r: %04X\n", reg);

			if ((reg >> 1) < sizeof(cf_card_tuples))
			{
				r |= cf_card_tuples[reg >> 1] << 16;
			}
			else
			{
				fatalerror("%s:compact_flash_r: reg %02X\n", cpuexec_describe_context(device->machine), reg);
			}
		}
	}
	return r;
}

static WRITE64_DEVICE_HANDLER(cf_card_w)
{
	#ifdef VIPER_DEBUG_LOG
	printf("%s:compact_flash_w: %08X%08X, %08X, %08X%08X\n", cpuexec_describe_context(device->machine), (UINT32)(data>>32), (UINT32)(data), offset, (UINT32)(mem_mask >> 32), (UINT32)(mem_mask));
	#endif

	if (ACCESSING_BITS_16_31)
	{
		if (offset < 0x10)
		{
			switch (offset & 0xf)
			{
				case 0x0:	// Even WR Data
				case 0x1:	// Features
				case 0x2:	// Sector Count
				case 0x3:	// Sector No.
				case 0x4:	// Cylinder Low
				case 0x5:	// Cylinder High
				case 0x6:	// Select Card/Head
				case 0x7:	// Command
				{
					ide_bus_w(device, 0, offset & 7, (data >> 16) & 0xffff);
					break;
				}

				//case 0x8: // Duplicate Even WR Data
				//case 0x9: // Duplicate Odd WR Data

				case 0xd:	// Duplicate Features
				{
					ide_bus_w(device, 0, 1, (data >> 16) & 0xffff);
					break;
				}
				case 0xe:	// Device Ctl
				case 0xf:	// Reserved
				{
					ide_bus_w(device, 1, offset & 7, (data >> 16) & 0xffff);
					break;
				}

				default:
				{
					fatalerror("%s:compact_flash_w: IDE reg %02X, data %04X\n", cpuexec_describe_context(device->machine), offset & 0xf, (UINT16)((data >> 16) & 0xffff));
				}
			}
		}
		else if (offset >= 0x100)
		{
			switch (offset)
			{
				case 0x100:
				{
					if ((data >> 16) & 0x80)
					{
						cf_card_ide = 1;

						// soft reset
						// sector count register is set to 0x01
						// sector number register is set to 0x01
						// cylinder low register is set to 0x00
						// cylinder high register is set to 0x00

						ide_bus_w(device, 1, 6, 0x04);
					}
					break;
				}
				default:
				{
					fatalerror("%s:compact_flash_w: reg %02X, data %04X\n", cpuexec_describe_context(device->machine), offset, (UINT16)((data >> 16) & 0xffff));
				}
			}
		}
	}
}

static WRITE64_HANDLER(unk2_w)
{
	if (ACCESSING_BITS_56_63)
	{
		cf_card_ide = 0;
	}
}




static READ64_DEVICE_HANDLER(ata_r)
{
	UINT64 r = 0;

	if (ACCESSING_BITS_16_31)
	{
		int reg = (offset >> 4) & 0x7;

		r |= ide_bus_r(device, (offset & 0x80) ? 1 : 0, reg) << 16;
	}

	return r;
}

static WRITE64_DEVICE_HANDLER(ata_w)
{
	if (ACCESSING_BITS_16_31)
	{
		int reg = (offset >> 4) & 0x7;

		ide_bus_w(device, (offset & 0x80) ? 1 : 0, reg, (UINT16)(data >> 16));
	}
}

static int unk1_bit = 0;
static READ64_HANDLER(unk1_r)
{
	UINT64 r = 0;
	//return 0;//U64(0x0000400000000000);

	if (ACCESSING_BITS_40_47)
	{
		r |= (UINT64)(unk1_bit << 5) << 40;
	}

	return r;
}

static WRITE64_HANDLER(unk1a_w)
{
	if (ACCESSING_BITS_56_63)
	{
		unk1_bit = 1;
	}
}

static WRITE64_HANDLER(unk1b_w)
{
	if (ACCESSING_BITS_56_63)
	{
		unk1_bit = 0;
	}
}

static UINT32 voodoo3_pci_reg[0x100];
static UINT32 voodoo3_pci_r(running_device *busdevice, running_device *device, int function, int reg, UINT32 mem_mask)
{
	switch (reg)
	{
		case 0x00:		// PCI Vendor ID (0x121a = 3dfx), Device ID (0x0005 = Voodoo 3)
		{
			return 0x0005121a;
		}
		case 0x08:		// Device class code
		{
			return 0x03000000;
		}
		case 0x10:		// memBaseAddr0
		{
			return voodoo3_pci_reg[0x10/4];
		}
		case 0x14:		// memBaseAddr1
		{
			return voodoo3_pci_reg[0x14/4];
		}
		case 0x18:		// memBaseAddr1
		{
			return voodoo3_pci_reg[0x18/4];
		}
		case 0x40:		// fabId
		{
			return voodoo3_pci_reg[0x40/4];
		}
		case 0x50:		// cfgScratch
		{
			return voodoo3_pci_reg[0x50/4];
		}

		default:
			fatalerror("voodoo3_pci_r: %08X at %08X", reg, cpu_get_pc(device->machine->device("maincpu")));
	}
	return 0;
}

static void voodoo3_pci_w(running_device *busdevice, running_device *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
//  printf("voodoo3_pci_w: %08X, %08X\n", reg, data);

	switch (reg)
	{
		case 0x04:		// Command register
		{
			voodoo3_pci_reg[0x04/4] = data;
			break;
		}
		case 0x10:		// memBaseAddr0
		{
			if (data == 0xffffffff)
			{
				voodoo3_pci_reg[0x10/4] = 0xfe000000;
			}
			else
			{
				voodoo3_pci_reg[0x10/4] = data;
			}
			break;
		}
		case 0x14:		// memBaseAddr1
		{
			if (data == 0xffffffff)
			{
				voodoo3_pci_reg[0x14/4] = 0xfe000008;
			}
			else
			{
				voodoo3_pci_reg[0x14/4] = data;
			}
			break;
		}
		case 0x18:		// ioBaseAddr
		{
			if (data == 0xffffffff)
			{
				voodoo3_pci_reg[0x18/4] = 0xffffff01;
			}
			else
			{
				voodoo3_pci_reg[0x18/4] = data;
			}
			break;
		}
		case 0x3c:		// InterruptLine
		{
			break;
		}
		case 0x40:		// fabId
		{
			voodoo3_pci_reg[0x40/4] = data;
			break;
		}
		case 0x50:		// cfgScratch
		{
			voodoo3_pci_reg[0x50/4] = data;
			break;
		}

		default:
			fatalerror("voodoo3_pci_w: %08X, %08X at %08X", data, reg, cpu_get_pc(device->machine->device("maincpu")));
	}
}

#if 0
static READ64_HANDLER(voodoo3_io_r)
{
	return read64be_with_32le_handler(banshee_io_0_r, space->machine, offset, mem_mask);
}
static WRITE64_HANDLER(voodoo3_io_w)
{
//  printf("voodoo3_io_w: %08X%08X, %08X at %08X\n", (UINT32)(data >> 32), (UINT32)(data), offset, cpu_get_pc(space->cpu));
	write64be_with_32le_handler(banshee_io_0_w, space->machine, offset, data, mem_mask);
}

static READ64_HANDLER(voodoo3_r)
{
	return read64be_with_32le_handler(banshee_0_r, space->machine, offset, mem_mask);
}
static WRITE64_HANDLER(voodoo3_w)
{
//  printf("voodoo3_w: %08X%08X, %08X at %08X\n", (UINT32)(data >> 32), (UINT32)(data), offset, cpu_get_pc(space->cpu));
	write64be_with_32le_handler(banshee_0_w, space->machine,  offset, data, mem_mask);
}

static READ64_HANDLER(voodoo3_lfb_r)
{
	return read64be_with_32le_handler(banshee_fb_0_r, space->machine, offset, mem_mask);
}
static WRITE64_HANDLER(voodoo3_lfb_w)
{
//  printf("voodoo3_lfb_w: %08X%08X, %08X at %08X\n", (UINT32)(data >> 32), (UINT32)(data), offset, cpu_get_pc(space->cpu));
	write64be_with_32le_handler(banshee_fb_0_w, space->machine, offset, data, mem_mask);
}
#endif


/*****************************************************************************/

static ADDRESS_MAP_START(viper_map, ADDRESS_SPACE_PROGRAM, 64)
	AM_RANGE(0x00000000, 0x00ffffff) AM_MIRROR(0x1000000) AM_RAM
	AM_RANGE(0x80000000, 0x800fffff) AM_READWRITE(epic_64be_r, epic_64be_w)
	AM_RANGE(0x82000000, 0x83ffffff) AM_DEVREADWRITE32("voodoo", banshee_r, banshee_w, U64(0xffffffffffffffff))
	AM_RANGE(0x84000000, 0x85ffffff) AM_DEVREADWRITE32("voodoo", banshee_fb_r, banshee_fb_w, U64(0xffffffffffffffff))
	AM_RANGE(0xfe800000, 0xfe8000ff) AM_DEVREADWRITE32("voodoo", banshee_io_r, banshee_io_w, U64(0xffffffffffffffff))
	AM_RANGE(0xfec00000, 0xfedfffff) AM_DEVREADWRITE("pcibus", pci_config_addr_r, pci_config_addr_w)
	AM_RANGE(0xfee00000, 0xfeefffff) AM_DEVREADWRITE("pcibus", pci_config_data_r, pci_config_data_w)
	AM_RANGE(0xff300000, 0xff300fff) AM_DEVREADWRITE("ide", ata_r, ata_w)
	AM_RANGE(0xffe10000, 0xffe10007) AM_READ(unk1_r)
	AM_RANGE(0xffe30000, 0xffe31fff) AM_DEVREADWRITE8("m48t58",timekeeper_r, timekeeper_w, U64(0xffffffffffffffff))
	AM_RANGE(0xffe40000, 0xffe4000f) AM_NOP
	AM_RANGE(0xffe50000, 0xffe50007) AM_WRITE(unk2_w)
	AM_RANGE(0xffe80000, 0xffe80007) AM_WRITE(unk1a_w)
	AM_RANGE(0xffe88000, 0xffe88007) AM_WRITE(unk1b_w)
	AM_RANGE(0xfff00000, 0xfff3ffff) AM_ROM AM_REGION("user1", 0)		// Boot ROM
ADDRESS_MAP_END

/*****************************************************************************/

static INPUT_PORTS_START( viper )
INPUT_PORTS_END

/*****************************************************************************/


static const powerpc_config viper_ppc_cfg =
{
	66000000
};

static INTERRUPT_GEN(viper_vblank)
{

}

static void ide_interrupt(running_device *device, int state)
{
}

static MACHINE_RESET(viper)
{
	devtag_reset(machine, "ide");
}

static MACHINE_CONFIG_START( viper, driver_device )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", MPC8240, 200000000)
	MDRV_CPU_CONFIG(viper_ppc_cfg)
	MDRV_CPU_PROGRAM_MAP(viper_map)
	MDRV_CPU_VBLANK_INT("screen", viper_vblank)

	MDRV_MACHINE_RESET(viper)

	MDRV_PCI_BUS_ADD("pcibus", 0)
	MDRV_PCI_BUS_DEVICE(0, NULL, mpc8240_pci_r, mpc8240_pci_w)
	MDRV_PCI_BUS_DEVICE(12, "voodoo", voodoo3_pci_r, voodoo3_pci_w)

	MDRV_IDE_CONTROLLER_ADD("ide", ide_interrupt)
	MDRV_3DFX_VOODOO_3_ADD("voodoo", STD_VOODOO_3_CLOCK, 16, "screen")
	MDRV_3DFX_VOODOO_CPU("maincpu")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(800, 600)
	MDRV_SCREEN_VISIBLE_AREA(0, 799, 0, 599)

	MDRV_PALETTE_LENGTH(65536)

	MDRV_VIDEO_UPDATE(viper)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_M48T58_ADD( "m48t58" )
MACHINE_CONFIG_END

/*****************************************************************************/

static DRIVER_INIT(viper)
{
//  memory_install_readwrite64_device_handler( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), ide, 0xff200000, 0xff207fff, 0, 0, hdd_r, hdd_w ); //TODO
}

static DRIVER_INIT(vipercf)
{
	running_device *ide = machine->device("ide");

	DRIVER_INIT_CALL(viper);

	memory_install_readwrite64_device_handler( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), ide, 0xff000000, 0xff000fff, 0, 0, cf_card_data_r, cf_card_data_w );
	memory_install_readwrite64_device_handler( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), ide, 0xff200000, 0xff200fff, 0, 0, cf_card_r, cf_card_w );
}


/*****************************************************************************/

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios+1)) /* Note '+1' */

#define VIPER_BIOS \
	ROM_REGION64_BE(0x40000, "user1", 0)	/* Boot ROM */ \
	ROM_SYSTEM_BIOS(0, "Viper BIOS", "GM941B01") \
		ROM_LOAD_BIOS(0, "941b01.u25", 0x00000, 0x40000, CRC(233e5159) SHA1(66ff268d5bf78fbfa48cdc3e1b08f8956cfd6cfb))


ROM_START(kviper)
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
ROM_END


/* Viper games with hard disk */
ROM_START(ppp2nd)
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */

	DISK_REGION( "ide" )
	DISK_IMAGE( "ppp2nd", 0, SHA1(b8b90483d515c83eac05ffa617af19612ea990b0))
ROM_END

/* Viper games with Compact Flash card */
ROM_START(boxingm) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a45jaa_nvram.u39", 0x00000, 0x2000, CRC(c24e29fc) SHA1(efb6ecaf25cbdf9d8dfcafa85e38a195fa5ff6c4))

	DISK_REGION( "ide" )
	DISK_IMAGE( "a45a02", 0, SHA1(9af2481f53de705ae48fad08d8dd26553667c2d0) )
ROM_END

ROM_START(code1d) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x00000, 0x2000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "922d02", 0, SHA1(01f35e324c9e8567da0f51b3e68fff1562c32116) )
ROM_END

ROM_START(code1db) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x00000, 0x2000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "922b02", 0, SHA1(4d288b5dcfab3678af662783e7083a358eee99ce) )
ROM_END

ROM_START(gticlub2) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(d0604e84) SHA1(18d1183f1331af3e655a56692eb7ab877b4bc239)) //old dump, probably has non-default settings.
	ROM_LOAD("941jab_nvram.u39", 0x00000, 0x2000, CRC(6c4a852f) SHA1(2753dda42cdd81af22dc6780678f1ddeb3c62013))

	DISK_REGION( "ide" )
	DISK_IMAGE( "941b02", 0,  SHA1(943bc9b1ea7273a8382b94c8a75010dfe296df14) )
ROM_END

ROM_START(gticlub2ea) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("941eaa_nvram.u39", 0x00000, 0x2000, CRC(5ee7004d) SHA1(92e0ce01049308f459985d466fbfcfac82f34a47))

	DISK_REGION( "ide" )
	DISK_IMAGE( "941a02", 0,  NO_DUMP )
ROM_END

ROM_START(jpark3) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("b41ebc_nvram.u39", 0x00000, 0x2000, CRC(55d1681d) SHA1(26868cf0d14f23f06b81f2df0b4186924439bb43))

	DISK_REGION( "ide" )
	DISK_IMAGE( "b41c02", 0, SHA1(fb6b0b43a6f818041d644bcd711f6a727348d3aa) )
ROM_END

/* This CF card has sticker B33A02 */
ROM_START(mocapglf) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("b33uaa_nvram.u39", 0x00000, 0x1ff8, BAD_DUMP CRC(0f0ba988) SHA1(5618c03b21fc2ba14b2e159cee3aab7f53c2c34d)) //data looks plain bad (compared to the other games)

	DISK_REGION( "ide" )
	DISK_IMAGE( "b33a02", 0, SHA1(819d8fac5d2411542c1b989105cffe38a5545fc2) )
ROM_END

ROM_START(mocapb) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a29aaa_nvram.u39", 0x000000, 0x2000, CRC(14b9fe68) SHA1(3c59e6df1bb46bc1835c13fd182b1bb092c08759)) //supposed to be aab version?

	DISK_REGION( "ide" )
	DISK_IMAGE( "a29b02", 0, SHA1(f0c04310caf2cca804fde20805eb30a44c5a6796) ) //missing bootloader
ROM_END

ROM_START(mocapbj) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a29jaa_nvram.u39", 0x000000, 0x2000, CRC(2f7cdf27) SHA1(0b69d8728be12909e235268268a312982f81d46a))

	DISK_REGION( "ide" )
	DISK_IMAGE( "a29a02", 0, SHA1(00afad399737652b3e17257c70a19f62e37f3c97) )
ROM_END

ROM_START(p911) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00uad_nvram.u39", 0x000000, 0x2000, CRC(cca056ca) SHA1(de1a00d84c1311d48bbe6d24f5b36e22ecf5e85a))

	DISK_REGION( "ide" )
	DISK_IMAGE( "a00uad02", 0, SHA1(6acb8dc41920e7025b87034a3a62b185ef0109d9) )
ROM_END

ROM_START(p911uc) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00uac_nvram.u39", 0x000000, 0x2000,  NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "a00uac02", 0, SHA1(b268789416dbf8886118a634b911f0ee254970de) )
ROM_END

ROM_START(p911kc) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00kac_nvram.u39", 0x000000, 0x2000,  CRC(8ddc921c) SHA1(901538da237679fc74966a301278b36d1335671f) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "a00kac02", 0, SHA1(b268789416dbf8886118a634b911f0ee254970de) )
ROM_END

ROM_START(p911e) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00eaa_nvram.u39", 0x000000, 0x2000,  NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "a00eaa02", 0, SHA1(81565a2dce2e2b0a7927078a784354948af1f87c) )
ROM_END

ROM_START(p911j) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00jaa_nvram.u39", 0x000000, 0x2000, CRC(9ecf70dc) SHA1(4769a99b0cc28563e219860b8d480f32d1e21f60))

	DISK_REGION( "ide" )
	DISK_IMAGE( "a00jac02", 0, SHA1(d962d3a8ea84c380767d0fe336296911c289c224) )
ROM_END

ROM_START(p9112) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x000000, 0x2000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "b11a02", 0, SHA1(57665664321b78c1913d01f0d2c0b8d3efd42e04) )
ROM_END

ROM_START(popn9) //Note: this is actually a Konami Pyson HW! (PlayStation 2-based) move out of here.
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x000000, 0x2000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "c00jab", 0, BAD_DUMP SHA1(3763aaded9b45388a664edd84a3f7f8ff4101be4) )
ROM_END

ROM_START(sscopex)
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a13uaa_nvram.u39", 0x000000, 0x2000, CRC(7b0e1ac8) SHA1(1ea549964539e27f87370e9986bfa44eeed037cd))

	DISK_REGION( "ide" )
	DISK_IMAGE( "a13c02", 0, SHA1(d740784fa51a3f43695ea95e23f92ef05f43284a) )
ROM_END

//TODO: sscopexb + many nvram clone versions.

ROM_START(sogeki) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x000000, 0x2000, CRC(2f325c55) SHA1(0bc44f40f981a815c8ce64eae95ae55db510c565))

	DISK_REGION( "ide" )
	DISK_IMAGE( "a13b02", 0, SHA1(c25a61b76d365794c2da4a9e7de88a5519e944ec) )
ROM_END

ROM_START(thrild2) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a41ebb_nvram.u39", 0x00000, 0x2000, CRC(22f59ac0) SHA1(e14ea2ba95b72edf0a3331ab82c192760bfdbce3))
//  a41eba_nvram == a41ebb_nvram

	DISK_REGION( "ide" )
	DISK_IMAGE( "a41b02", 0, SHA1(0426f4bb9001cf457f44e2c22e3d7575b8049aa3) )
ROM_END

ROM_START(thrild2a) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a41aaa_nvram.u39", 0x00000, 0x2000, CRC(d5de9b8e) SHA1(768bcd46a6ad20948f60f5e0ecd2f7b9c2901061))

	DISK_REGION( "ide" )
	DISK_IMAGE( "a41a02", 0, SHA1(bbb71e23bddfa07dfa30b6565a35befd82b055b8) )
ROM_END

/* This CF card has sticker 941EAA02 */
ROM_START(thrild2c) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("941eaa_nvram.u39", 0x00000, 0x2000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "a41c02", 0, SHA1(ab3020e8709768c0fd2467573e92b679a05944e5) )
ROM_END

ROM_START(tsurugi) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a30eab_nvram.u39", 0x00000, 0x2000, CRC(c123342c) SHA1(55416767608fe0311a362854a16b214b04435a31))

	DISK_REGION( "ide" )
	DISK_IMAGE( "a30b02", 0, SHA1(d2be83b7323c365ba445de7697c3fb8eb83d0212) )
ROM_END

ROM_START(tsurugij) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a30jac_nvram.u39", 0x00000, 0x2000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "a30c02", 0, SHA1(533b5669b00884a800df9ba29651777a76559862) )
ROM_END

/* This CF card has sticker C22D02 */
ROM_START(wcombat) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(4f8b5858) SHA1(68066241c6f9db7f45e55b3c5da101987f4ce53c))

	DISK_REGION( "ide" )
	DISK_IMAGE( "c22d02", 0, BAD_DUMP SHA1(85d2a8b5ec4cfd932190486cad991f0c180ca6b3) )
ROM_END

ROM_START(wcombatk) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(ebd4d645) SHA1(2fa7e2c6b113214f3eb1900c8ceef4d5fcf0bb76))

	DISK_REGION( "ide" )
	DISK_IMAGE( "c22c02", 0, BAD_DUMP SHA1(8bd1dfbf926ad5b28fa7dafd7e31c475325ec569) )
ROM_END

ROM_START(wcombatj) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(bd8a6640) SHA1(2d409197ef3fb07d984d27fa943f29c7a711d715))

	DISK_REGION( "ide" )
	DISK_IMAGE( "c22a02", 0, BAD_DUMP SHA1(b607fb2ddfd0bd552b7a736cea4ac1aa3ea021bd) )
ROM_END

ROM_START(xtrial) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("b4xjab_nvram.u39", 0x00000, 0x2000, CRC(33708a93) SHA1(715968e3c9c15edf628fa6ac655dc0864e336c6c))

	DISK_REGION( "ide" )
	DISK_IMAGE( "b4xb02", 0, SHA1(d8d54f3f16b762bf0187fe29b2f8696015c0a940) )
ROM_END

/* Viper Satellite Terminal games */

/*
Mahjong Fight Club (Konami Viper h/w)
Konami, 2002

PCB number - GM941-PWB(A)C Copyright 1999 Konami Made In Japan

Mahjong Fight Club is a multi player Mahjong battle game for up to 8 players. A
single PCB will not boot unless all of the other units are connected and powered
on, although how exactly they're connected is unknown. There is probably a
master unit that talks to all of the 8 satellite units. At the moment I have
only 2 of the 8 satellite units so I can't confirm that.
However, I don't have access to the main unit anyway as it was not included in
the auction we won :(

The Viper hardware can accept additional PCBs inside the metal box depending on
the game. For Mahjong Fight Club, no additional PCBs are present or required.

The main CPU is a Motorola XPC8240LZU200E
The main graphics chip is heatsinked. It's a BGA chip, and might be something
like a Voodoo chip? Maybe :-)
There's 1 Konami chip stamped 056879
There's also a bunch of video RAMs and several PLCC FPGAs or CPLDs
There's also 1 PLCC44 chip stamped PC16552

Files
-----
c09jad04.bin is a 64M Compact Flash card. The image was simply copied from the
card as it is PC readable. The card contains only 1 file named c09jad04.bin

941b01.u25 is the BIOS, held in a 2MBit PLCC32 Fujitsu MBM29F002 EEPROM and
surface mounted at location U25. The BIOS is common to ALL Viper games.

nvram.u39 is a ST M48T58Y Timekeeper NVRAM soldered-in at location U39. The
codes at the start of the image (probably just the first 16 or 32 bytes) are
used as a simple (and very weak) protection check to stop game swaps. The
contents of the NVRAM is different for ALL games on this hardware.

Some games use a dongle and swapping games won't work unless the dongle is also provided.
The following games comes with a dongle....
Mahjong Fight Club

For non-dongled games, I have verified the following games will work when the
CF card and NVRAM are swapped....
*/

/* This CF card has sticker C09JAD04 */
ROM_START(mfightc) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(9fb551a5) SHA1(a33d185e186d404c3bf62277d7e34e5ad0000b09)) //likely non-default settings
	ROM_LOAD("c09jad_nvram.u39", 0x00000, 0x2000, CRC(33e960b7) SHA1(a9a249e68c89b18d4685f1859fe35dc21df18e14))

	DISK_REGION( "ide" )
	DISK_IMAGE( "c09d04", 0, SHA1(7395b7a33e953f65827aea44461e49f8388464fb) )
ROM_END

/* This CF card has sticker C09JAC04 */
ROM_START(mfightcc) //*
	VIPER_BIOS

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
	ROM_LOAD("c09jac_nvram.u39", 0x00000, 0x2000, NO_DUMP )

	DISK_REGION( "ide" )
	DISK_IMAGE( "c09c04", 0, SHA1(bf5f7447d74399d34edd4eb6dfcca7f6fc2154f2) )
ROM_END

/*****************************************************************************/

/* Viper BIOS */
GAME(1999, kviper,    0,         viper, viper, viper,    ROT0,  "Konami", "Konami Viper BIOS", GAME_IS_BIOS_ROOT)

GAME(2001, ppp2nd,    kviper,    viper, viper, viper,    ROT0,  "Konami", "ParaParaParadise 2nd Mix", GAME_NOT_WORKING|GAME_NO_SOUND)

GAME(2001, boxingm,   kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Boxing Mania (ver JAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2000, code1d,    kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Code One Dispatch (ver D)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2000, code1db,   code1d,    viper, viper, vipercf,  ROT0,  "Konami", "Code One Dispatch (ver B)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, gticlub2,  kviper,    viper, viper, vipercf,  ROT0,  "Konami", "GTI Club 2 (ver JAB)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, gticlub2ea,gticlub2,  viper, viper, vipercf,  ROT0,  "Konami", "GTI Club 2 (ver EAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, jpark3,    kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Jurassic Park 3 (ver EBC)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, mocapglf,  kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Mocap Golf (ver UAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, mocapb,    kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Mocap Boxing (ver AAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, mocapbj,   mocapb,    viper, viper, vipercf,  ROT0,  "Konami", "Mocap Boxing (ver JAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, p911,      kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Police 911 (ver UAD)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, p911uc,    p911,      viper, viper, vipercf,  ROT0,  "Konami", "Police 911 (ver UAC)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, p911kc,    p911,      viper, viper, vipercf,  ROT0,  "Konami", "Police 911 (ver KAC)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, p911e,     p911,      viper, viper, vipercf,  ROT0,  "Konami", "Police 24/7 (ver EAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, p911j,     p911,      viper, viper, vipercf,  ROT0,  "Konami", "Keisatsukan Shinjuku 24ji (ver JAC)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, p9112,     kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Police 911 2 (ver A)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2003, popn9,     kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Pop'n Music 9 (ver JAB)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, sscopex,   kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Silent Scope EX (ver UAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, sogeki,    sscopex,   viper, viper, vipercf,  ROT0,  "Konami", "Sogeki (ver JAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, thrild2,   kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Thrill Drive 2 (ver EBB)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, thrild2a,  thrild2,   viper, viper, vipercf,  ROT0,  "Konami", "Thrill Drive 2 (ver AAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, thrild2c,  thrild2,   viper, viper, vipercf,  ROT0,  "Konami", "Thrill Drive 2 (ver EAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, tsurugi,   kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Tsurugi (ver EAB)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, tsurugij,  tsurugi,   viper, viper, vipercf,  ROT0,  "Konami", "Tsurugi (ver JAC)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2002, wcombat,   kviper,    viper, viper, vipercf,  ROT0,  "Konami", "World Combat (ver UAA?)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2002, wcombatk,  wcombat,   viper, viper, vipercf,  ROT0,  "Konami", "World Combat (ver KBC)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2002, wcombatj,  wcombat,   viper, viper, vipercf,  ROT0,  "Konami", "World Combat (ver JAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2002, xtrial,    kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Xtrial Racing (ver JAB)", GAME_NOT_WORKING|GAME_NO_SOUND)

GAME(2002, mfightc,   kviper,    viper, viper, vipercf,  ROT0,  "Konami", "Mahjong Fight Club (ver JAD)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2002, mfightcc,  mfightc,   viper, viper, vipercf,  ROT0,  "Konami", "Mahjong Fight Club (ver JAC)", GAME_NOT_WORKING|GAME_NO_SOUND)
