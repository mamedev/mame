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
*/

#include "driver.h"
#include "cpu/powerpc/ppc.h"
#include "machine/pci.h"
#include "memconv.h"
#include "machine/idectrl.h"
#include "machine/timekpr.h"
#include "video/voodoo.h"



static UINT8 backup_ram[0x2000];

static VIDEO_UPDATE(viper)
{
	const device_config *device = device_list_find_by_tag(screen->machine->config->devicelist, VOODOO_GRAPHICS, "voodoo");
	return voodoo_update(device, bitmap, cliprect) ? 0 : UPDATE_HAS_NOT_CHANGED;
}



/*****************************************************************************/

static UINT32 mpc8240_regs[256/4];
static UINT32 mpc8240_pci_r(int function, int reg, UINT32 mem_mask)
{
//  printf("MPC8240: PCI read %d, %02X, %08X\n", function, reg, mem_mask);

	switch (reg)
	{
	}

	return mpc8240_regs[reg/4];
}

static void mpc8240_pci_w(int function, int reg, UINT32 data, UINT32 mem_mask)
{
//  printf("MPC8240: PCI write %d, %02X, %08X, %08X\n", function, reg, data, mem_mask);
	COMBINE_DATA(mpc8240_regs + (reg/4));
}


static READ64_HANDLER( pci_config_addr_r )
{
	return pci_64be_r(machine, 0, U64(0x00000000ffffffff));
}

static WRITE64_HANDLER( pci_config_addr_w )
{
	pci_64be_w(machine, 0, data, U64(0x00000000ffffffff));
}

static READ64_HANDLER( pci_config_data_r )
{
	return pci_64be_r(machine, 1, U64(0xffffffff00000000)) << 32;
}

static WRITE64_HANDLER( pci_config_data_w )
{
	pci_64be_w(machine, 1, data >> 32, U64(0xffffffff00000000));
}



static UINT32 epic_iack;

static READ32_HANDLER( epic_r )
{
	int reg;
	reg = offset * 4;

	//printf("EPIC: read %08X, %08X at %08X\n", reg, mem_mask, activecpu_get_pc());

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

	//printf("EPIC: write %08X, %08X, %08X at %08X\n", data, reg, mem_mask, activecpu_get_pc());

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
	return read64be_with_32le_handler(epic_r, machine, offset, mem_mask);
}
static WRITE64_HANDLER(epic_64be_w)
{
	write64be_with_32le_handler(epic_w, machine, offset, data, mem_mask);
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
				fatalerror("cf_card_data_r: IDE reg %02X at %08X\n", offset & 0xf, activecpu_get_pc());
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
				fatalerror("cf_card_data_w: IDE reg %02X, %04X at %08X\n", offset & 0xf, (UINT16)(data >> 16), activecpu_get_pc());
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
					printf("compact_flash_r: IDE reg %02X at %08X\n", offset & 0xf, activecpu_get_pc());
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
				fatalerror("compact_flash_r: reg %02X at %08X\n", reg, activecpu_get_pc());
			}
		}
	}
	return r;
}

static WRITE64_DEVICE_HANDLER(cf_card_w)
{
	//printf("compact_flash_w: %08X%08X, %08X, %08X%08X at %08X\n", (UINT32)(data>>32), (UINT32)(data), offset, (UINT32)(mem_mask >> 32), (UINT32)(mem_mask), activecpu_get_pc());

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
					fatalerror("compact_flash_w: IDE reg %02X, data %04X at %08X\n", offset & 0xf, (UINT16)((data >> 16) & 0xffff), activecpu_get_pc());
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
					fatalerror("compact_flash_w: reg %02X, data %04X at %08X\n", offset, (UINT16)((data >> 16) & 0xffff), activecpu_get_pc());
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
static UINT32 voodoo3_pci_r(int function, int reg, UINT32 mem_mask)
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
			fatalerror("voodoo3_pci_r: %08X at %08X", reg, activecpu_get_pc());
	}
	return 0;
}

static void voodoo3_pci_w(int function, int reg, UINT32 data, UINT32 mem_mask)
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
			fatalerror("voodoo3_pci_w: %08X, %08X at %08X", data, reg, activecpu_get_pc());
	}
}

#if 0
static READ64_HANDLER(voodoo3_io_r)
{
	return read64be_with_32le_handler(banshee_io_0_r, machine, offset, mem_mask);
}
static WRITE64_HANDLER(voodoo3_io_w)
{
//  printf("voodoo3_io_w: %08X%08X, %08X at %08X\n", (UINT32)(data >> 32), (UINT32)(data), offset, activecpu_get_pc());
	write64be_with_32le_handler(banshee_io_0_w, machine, offset, data, mem_mask);
}

static READ64_HANDLER(voodoo3_r)
{
	return read64be_with_32le_handler(banshee_0_r, machine, offset, mem_mask);
}
static WRITE64_HANDLER(voodoo3_w)
{
//  printf("voodoo3_w: %08X%08X, %08X at %08X\n", (UINT32)(data >> 32), (UINT32)(data), offset, activecpu_get_pc());
	write64be_with_32le_handler(banshee_0_w, machine,  offset, data, mem_mask);
}

static READ64_HANDLER(voodoo3_lfb_r)
{
	return read64be_with_32le_handler(banshee_fb_0_r, machine, offset, mem_mask);
}
static WRITE64_HANDLER(voodoo3_lfb_w)
{
//  printf("voodoo3_lfb_w: %08X%08X, %08X at %08X\n", (UINT32)(data >> 32), (UINT32)(data), offset, activecpu_get_pc());
	write64be_with_32le_handler(banshee_fb_0_w, machine, offset, data, mem_mask);
}
#endif


static READ64_HANDLER(m48t58_r)
{
	UINT64 r = 0;

	if (ACCESSING_BITS_32_63)
	{
		r |= (UINT64)timekeeper_0_32be_r(machine, (offset * 2) + 0, (UINT32)(mem_mask >> 32)) << 32;
	}
	if (ACCESSING_BITS_0_31)
	{
		r |= timekeeper_0_32be_r(machine, (offset * 2) + 1, (UINT32)(mem_mask));
	}

	return r;
}

static WRITE64_HANDLER(m48t58_w)
{
	if (ACCESSING_BITS_32_63)
	{
		timekeeper_0_32be_w(machine, (offset * 2) + 0, (UINT32)(data >> 32), (UINT32)(mem_mask >> 32));
	}
	if (ACCESSING_BITS_0_31)
	{
		timekeeper_0_32be_w(machine, (offset * 2) + 1, (UINT32)(data), (UINT32)(mem_mask));
	}
}

/*****************************************************************************/

static ADDRESS_MAP_START(viper_map, ADDRESS_SPACE_PROGRAM, 64)
	AM_RANGE(0x00000000, 0x00ffffff) AM_MIRROR(0x1000000) AM_RAM
	AM_RANGE(0x80000000, 0x800fffff) AM_READWRITE(epic_64be_r, epic_64be_w)
	AM_RANGE(0x82000000, 0x83ffffff) AM_DEVREADWRITE32(VOODOO_GRAPHICS, "voodoo", banshee_r, banshee_w, U64(0xffffffffffffffff))
	AM_RANGE(0x84000000, 0x85ffffff) AM_DEVREADWRITE32(VOODOO_GRAPHICS, "voodoo", banshee_fb_r, banshee_fb_w, U64(0xffffffffffffffff))
	AM_RANGE(0xfe800000, 0xfe8000ff) AM_DEVREADWRITE32(VOODOO_GRAPHICS, "voodoo", banshee_io_r, banshee_io_w, U64(0xffffffffffffffff))
	AM_RANGE(0xfec00000, 0xfedfffff) AM_READWRITE(pci_config_addr_r, pci_config_addr_w)
	AM_RANGE(0xfee00000, 0xfeefffff) AM_READWRITE(pci_config_data_r, pci_config_data_w)
	AM_RANGE(0xff300000, 0xff300fff) AM_DEVREADWRITE(IDE_CONTROLLER, "ide", ata_r, ata_w)
	AM_RANGE(0xffe10000, 0xffe10007) AM_READ(unk1_r)
	AM_RANGE(0xffe30000, 0xffe31fff) AM_READWRITE(m48t58_r, m48t58_w)
	AM_RANGE(0xffe40000, 0xffe4000f) AM_NOP
	AM_RANGE(0xffe50000, 0xffe50007) AM_WRITE(unk2_w)
	AM_RANGE(0xffe80000, 0xffe80007) AM_WRITE(unk1a_w)
	AM_RANGE(0xffe88000, 0xffe88007) AM_WRITE(unk1b_w)
	AM_RANGE(0xfff00000, 0xfff3ffff) AM_ROM AM_REGION("user1", 0)		// Boot ROM
ADDRESS_MAP_END

/*****************************************************************************/



static const powerpc_config viper_ppc_cfg =
{
	66000000
};

static INTERRUPT_GEN(viper_vblank)
{

}

static void ide_interrupt(const device_config *device, int state)
{
}

static MACHINE_RESET(viper)
{
	devtag_reset(machine, IDE_CONTROLLER, "ide");
}

static MACHINE_DRIVER_START(viper)

	/* basic machine hardware */
	MDRV_CPU_ADD("main", MPC8240, 200000000)
	MDRV_CPU_CONFIG(viper_ppc_cfg)
	MDRV_CPU_PROGRAM_MAP(viper_map, 0)
	MDRV_CPU_VBLANK_INT("main", viper_vblank)

	MDRV_MACHINE_RESET(viper)

	MDRV_NVRAM_HANDLER(timekeeper_0)

	MDRV_IDE_CONTROLLER_ADD("ide", ide_interrupt)
	MDRV_3DFX_VOODOO_3_ADD("voodoo", STD_VOODOO_3_CLOCK, 16, "main")

 	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(800, 600)
	MDRV_SCREEN_VISIBLE_AREA(0, 799, 0, 599)

	MDRV_PALETTE_LENGTH(65536)

	MDRV_VIDEO_UPDATE(viper)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

MACHINE_DRIVER_END

/*****************************************************************************/

static const struct pci_device_info mpc8240 =
{
	mpc8240_pci_r,
	mpc8240_pci_w
};

static const struct pci_device_info voodoo3 =
{
	voodoo3_pci_r,
	voodoo3_pci_w
};

static DRIVER_INIT(viper)
{
	UINT8 *nvram;

	pci_init();
	pci_add_device(0, 0, &mpc8240);
	pci_add_device(0, 12, &voodoo3);

	timekeeper_init(machine, 0, TIMEKEEPER_M48T58, backup_ram);

	nvram = memory_region(machine, "user2");
	memcpy(backup_ram, nvram, 0x2000);
}

static DRIVER_INIT(vipercf)
{
	const device_config *ide = device_list_find_by_tag(machine->config->devicelist, IDE_CONTROLLER, "ide");

	DRIVER_INIT_CALL(viper);

	memory_install_readwrite64_device_handler( ide, 0, ADDRESS_SPACE_PROGRAM, 0xff000000, 0xff000fff, 0, 0, cf_card_data_r, cf_card_data_w );
	memory_install_readwrite64_device_handler( ide, 0, ADDRESS_SPACE_PROGRAM, 0xff200000, 0xff200fff, 0, 0, cf_card_r, cf_card_w );
}

static DRIVER_INIT(ppp2nd)
{
	DRIVER_INIT_CALL(viper);

	/*
    backup_ram[0x0000] = 0x50;  // P
    backup_ram[0x0001] = 0x33;  // 3
    backup_ram[0x0002] = 0x5f;  // _
    backup_ram[0x0003] = 0x32;  // 2
    backup_ram[0x0004] = 0x6e;  // n
    backup_ram[0x0005] = 0x64;  // d
    backup_ram[0x0006] = 0x4d;  // M
    backup_ram[0x0007] = 0x49;  // I
    backup_ram[0x0008] = 0x58;  // X
    backup_ram[0x0009] = 0x00;  //
    backup_ram[0x000a] = 0x00;  //
    backup_ram[0x000b] = 0x00;  //
    backup_ram[0x000c] = 0x00;  //
    backup_ram[0x000d] = 0x00;  //
    backup_ram[0x000e] = 0x00;  //
    backup_ram[0x000f] = 0x00;  //

    backup_ram[0x0100] = 0x39;
    backup_ram[0x0101] = 0x89;
    backup_ram[0x0102] = 0x3a;
    backup_ram[0x0103] = 0xaa;
    */
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

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
ROM_END


/* Viper games with hard disk */
ROM_START(ppp2nd)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */

	DISK_REGION( "ide" )
		DISK_IMAGE( "ppp2nd", 0, MD5(06012243d4b64ebd9b81e9781fb5624d) SHA1(54b45e2df3f4239191000900181a94227a351c67))
ROM_END

/* Viper games with Compact Flash card */
ROM_START(boxingm)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
		ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(c24e29fc) SHA1(efb6ecaf25cbdf9d8dfcafa85e38a195fa5ff6c4))

	DISK_REGION( "ide" )
		DISK_IMAGE( "a45jaa02", 0, MD5(db0e9de39e2b4e6bd9f6f768472ab24a) SHA1(046c766fdb1b6607e794c598d5d603215b8e81a3) )
ROM_END

ROM_START(code1d)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */

	DISK_REGION( "ide" )
		DISK_IMAGE( "gk922d02", 0, MD5(c2713080273300a963fbf96dc22f70d7) SHA1(fcf451e8d49a93ca25a2177f2eb014da6ca6bcb3) )
ROM_END

ROM_START(code1db)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */

	DISK_REGION( "ide" )
		DISK_IMAGE( "gk922b02", 0, MD5(c1a9b406300c2e258be1cca00746ad6c) SHA1(ce949cdbe16ff8539e7fa46ccc58a06a90c7edde) )
ROM_END

ROM_START(gticlub2)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
		ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(d0604e84) SHA1(18d1183f1331af3e655a56692eb7ab877b4bc239))

	DISK_REGION( "ide" )
		DISK_IMAGE( "gm941b02", 0, MD5(8403c7f5fa5c254a30d87b59f8d1fedc) SHA1(217ef1628e8a377d22f537507dbe18fa9fe01fa2) )
ROM_END

ROM_START(jpark3)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
		ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(55d1681d) SHA1(26868cf0d14f23f06b81f2df0b4186924439bb43))

	DISK_REGION( "ide" )
		DISK_IMAGE( "b41c02", 0, MD5(4909b6c4007cc24f8cc514aa0ea26170) SHA1(386059033832c70063ff38363d473050162835e9) )
ROM_END

ROM_START(mocapglf)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */

	DISK_REGION( "ide" )
		DISK_IMAGE( "b33a02", 0, MD5(5e7d62ab51dae4e9ace35d7e52b08b5d) SHA1(a56df1917fb476cbacfe24f1a2f2cacc770eb80a) )
ROM_END

ROM_START(mocapb)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
		ROM_LOAD("nvram.u39", 0x000000, 0x2000, CRC(14b9fe68) SHA1(3c59e6df1bb46bc1835c13fd182b1bb092c08759))

	DISK_REGION( "ide" )
		DISK_IMAGE( "a29b02", 0, MD5(18f7070bbde3c3ca38d71425bdc074ab) SHA1(167b4b9f0960503db37c22e81ce4149925c5ec71) )
ROM_END

ROM_START(mocapbj)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
		ROM_LOAD("nvram.u39", 0x000000, 0x2000, CRC(2f7cdf27) SHA1(0b69d8728be12909e235268268a312982f81d46a))

	DISK_REGION( "ide" )
		DISK_IMAGE( "a29a02", 0, MD5(ba06338534dbad61071d39907a3aed85) SHA1(8ab0b63f6e4d1401380a286736964f0cd668a70e) )
ROM_END

ROM_START(p911)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
		ROM_LOAD("nvram.u39", 0x000000, 0x2000, CRC(cca056ca) SHA1(de1a00d84c1311d48bbe6d24f5b36e22ecf5e85a))

	DISK_REGION( "ide" )
		DISK_IMAGE( "a00aad02", 0, MD5(894ec7584841d7e1775e76edc53e739e) SHA1(2fc73fdaf9bc9ab6942672458206517bf8d851e1) )
ROM_END

ROM_START(p911uc)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */

	DISK_REGION( "ide" )
		DISK_IMAGE( "a00aac02", 0, MD5(c1d1a18932490c9d6185d0c3b55b50cf) SHA1(5cbdfce13ce2cb5c45ea66e20e092d5a5fe69c5d) )
ROM_END

ROM_START(p911e)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */

	DISK_REGION( "ide" )
		DISK_IMAGE( "a00eaa02", 0, MD5(5c870c38a408591e080516ae6a5f28e5) SHA1(8ff93666c33c7ee3ac2e4eeca9077c8296036187) )
ROM_END

ROM_START(p911j)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
		ROM_LOAD("nvram.u39", 0x000000, 0x2000, CRC(9ecf70dc) SHA1(4769a99b0cc28563e219860b8d480f32d1e21f60))

	DISK_REGION( "ide" )
		DISK_IMAGE( "a00jac02", 0, MD5(6c4b7c934159a54c93d5aa386e44969a) SHA1(80217103ad96b06855a57763c702140cbe4a2494) )
ROM_END

ROM_START(p9112)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */

	DISK_REGION( "ide" )
		DISK_IMAGE( "b11a02", 0, MD5(7ddf6139577bce2062b2f9a9ac99e56a) SHA1(56d9c47f3c7fceb2d8c36a8fa205c90595923c6f) )
ROM_END

ROM_START(popn9)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */

	DISK_REGION( "ide" )
		DISK_IMAGE( "c00jab", 0, MD5(f16cfaae05d29fdeeb7289265bf45712) SHA1(fd5b374731975e956b6e4e7102bc8350bfd8b5f4) )
ROM_END

ROM_START(sscopex)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
		ROM_LOAD("nvram.u39", 0x000000, 0x2000, CRC(7b0e1ac8) SHA1(1ea549964539e27f87370e9986bfa44eeed037cd))

	DISK_REGION( "ide" )
		DISK_IMAGE( "gka13c02", 0, MD5(994ced1f61dfe5ea10eac4b407359392) SHA1(179145b0e98c2ab5a7fc5c93cbc0c5398f961f65) )
ROM_END

ROM_START(sogeki)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
		ROM_LOAD("nvram.u39", 0x000000, 0x2000, CRC(2f325c55) SHA1(0bc44f40f981a815c8ce64eae95ae55db510c565))

	DISK_REGION( "ide" )
		DISK_IMAGE( "a13b02", 0, MD5(b476764d13ebf3fc3ec52e1fc2b36f96) SHA1(16965f5f164f4f8191e93274f8c99d84def15c70) )
ROM_END

ROM_START(thrild2)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
		ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(d5de9b8e) SHA1(768bcd46a6ad20948f60f5e0ecd2f7b9c2901061))

	DISK_REGION( "ide" )
		DISK_IMAGE( "gma41b02", 0, MD5(a3c39be28464fa737390e2e1e2aeb683) SHA1(0a451e33f5a6b708a54b70cc247e4d49ada0025a) )
ROM_END

ROM_START(thrild2a)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
		ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(3b7b0969) SHA1(3f11b6420ad3e3ee6f807e1ae14908bdc6e86d8f))

	DISK_REGION( "ide" )
		DISK_IMAGE( "gma41a02", 0, MD5(e73843d28afaef6c6af2513b6cf064e6) SHA1(abdaba1f539cf5d723fa51e939c0ffcbf5cdf28f) )
ROM_END

ROM_START(tsurugi)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
		ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(c123342c) SHA1(55416767608fe0311a362854a16b214b04435a31))

	DISK_REGION( "ide" )
		DISK_IMAGE( "a30eab02", 0, MD5(81593f4d5a10f7dea124bef0b02d15e6) SHA1(2202d8f8fa7cc752d4ebae16eac6818a5de06b98) )
ROM_END

ROM_START(tsurugij)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */

	DISK_REGION( "ide" )
		DISK_IMAGE( "a30jac02", 0, MD5(c7400ccdb4b045bbf2de4fa9d63a9e87) SHA1(64cc19941c2f3b39cbbe158042f3b312a678752d) )
ROM_END

ROM_START(wcombat)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
		ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(4f8b5858) SHA1(68066241c6f9db7f45e55b3c5da101987f4ce53c))

	DISK_REGION( "ide" )
		DISK_IMAGE( "c22d02", 0, MD5(f3b63ac0b2f613c9183356e7cf966ba6) SHA1(f7a6e8eba877f213075bd4836a62a898f17a27dc) )
ROM_END

ROM_START(wcombak)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
		ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(ebd4d645) SHA1(2fa7e2c6b113214f3eb1900c8ceef4d5fcf0bb76))

	DISK_REGION( "ide" )
		DISK_IMAGE( "c22c02", 0, MD5(e108a01a0a3ada065f42b0b4b16a4480) SHA1(6caff33b9b22fc56a619d58cfb8fb5aca2ecd289) )
ROM_END

ROM_START(wcombaj)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
		ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(bd8a6640) SHA1(2d409197ef3fb07d984d27fa943f29c7a711d715))

	DISK_REGION( "ide" )
		DISK_IMAGE( "c22a02", 0, MD5(0df54ceaa46c00e9b849af4bea6df1eb) SHA1(63ba75403ff714b2169f3c2e8ae3246a035da2d3) )
ROM_END

ROM_START(xtrial)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
		ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(33708a93) SHA1(715968e3c9c15edf628fa6ac655dc0864e336c6c))

	DISK_REGION( "ide" )
		DISK_IMAGE( "b4xb02", 0, MD5(c915e4946231fee8d3df00d5be08c177) SHA1(8631efafc19cc34ab8fbee2ed912d35de1f8b161) )
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

ROM_START(mfightc)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */
		ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(9fb551a5) SHA1(a33d185e186d404c3bf62277d7e34e5ad0000b09))

	DISK_REGION( "ide" )
		DISK_IMAGE( "c09jad04", 0, MD5(a858ca8f04838e8b5b603ea12526016c) SHA1(97e9b57c72787737c976cf9d7e997aee50befd53) )
ROM_END

ROM_START(mfightcc)
	VIPER_BIOS

	ROM_REGION(0x2000, "user2", ROMREGION_ERASE00)		/* M48T58 Timekeeper NVRAM */

	DISK_REGION( "ide" )
		DISK_IMAGE( "c09jac04", 0, MD5(3d502e1f9c8dbd7a9179ffd754a3de1b) SHA1(d27f04cc25df9955c014b7b97f768324f8e47c5c) )
ROM_END

/*****************************************************************************/

/* Viper BIOS */
GAME(1999, kviper,   0,       viper, 0, viper,    ROT0, "Konami", "Konami Viper BIOS", GAME_IS_BIOS_ROOT)

GAME(2001, ppp2nd,   kviper,  viper, 0, ppp2nd,   ROT0,  "Konami", "ParaParaParadise 2nd Mix", GAME_NOT_WORKING|GAME_NO_SOUND)

GAME(2001, boxingm,  kviper,  viper, 0, vipercf,  ROT0,  "Konami", "Boxing Mania", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2000, code1d,   kviper,  viper, 0, vipercf,  ROT0,  "Konami", "Code One Dispatch", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2000, code1db,  code1d,  viper, 0, vipercf,  ROT0,  "Konami", "Code One Dispatch (ver B)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, gticlub2, kviper,  viper, 0, vipercf,  ROT0,  "Konami", "GTI Club 2", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, jpark3,   kviper,  viper, 0, vipercf,  ROT0,  "Konami", "Jurassic Park 3", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, mocapglf, kviper,  viper, 0, vipercf,  ROT0,  "Konami", "Mocap Golf", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, mocapb,   kviper,  viper, 0, vipercf,  ROT0,  "Konami", "Mocap Boxing (ver AAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, mocapbj,  mocapb,  viper, 0, vipercf,  ROT0,  "Konami", "Mocap Boxing (ver JAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, p911,     kviper,  viper, 0, vipercf,  ROT0,  "Konami", "Police 911", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, p911uc,   p911,    viper, 0, vipercf,  ROT0,  "Konami", "Police 911 (ver UAC)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, p911e,    p911,    viper, 0, vipercf,  ROT0,  "Konami", "Police 24/7 (ver EAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, p911j,    p911,    viper, 0, vipercf,  ROT0,  "Konami", "Keisatsukan Shinjuku 24ji (ver JAC)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, p9112,    kviper,  viper, 0, vipercf,  ROT0,  "Konami", "Police 911 2", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2003, popn9,    kviper,  viper, 0, vipercf,  ROT0,  "Konami", "Pop'n Music 9", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, sscopex,  kviper,  viper, 0, vipercf,  ROT0,  "Konami", "Silent Scope EX (ver UAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, sogeki,   sscopex, viper, 0, vipercf,  ROT0,  "Konami", "Sogeki (ver JAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, thrild2,  kviper,  viper, 0, vipercf,  ROT0,  "Konami", "Thrill Drive 2", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, thrild2a, thrild2, viper, 0, vipercf,  ROT0,  "Konami", "Thrill Drive 2 (ver A)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, tsurugi,  kviper,  viper, 0, vipercf,  ROT0,  "Konami", "Tsurugi (ver EAB)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2001, tsurugij, tsurugi, viper, 0, vipercf,  ROT0,  "Konami", "Tsurugi (ver JAC)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2002, wcombat,  kviper,  viper, 0, vipercf,  ROT0,  "Konami", "World Combat", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2002, wcombak,  wcombat, viper, 0, vipercf,  ROT0,  "Konami", "World Combat (ver KBC)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2002, wcombaj,  wcombat, viper, 0, vipercf,  ROT0,  "Konami", "World Combat (ver JAA)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2002, xtrial,   kviper,  viper, 0, vipercf,  ROT0,  "Konami", "Xtrial Racing", GAME_NOT_WORKING|GAME_NO_SOUND)

GAME(2002, mfightc,  kviper, viper,  0, vipercf,  ROT0,  "Konami", "Mahjong Fight Club (ver D)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME(2002, mfightcc, mfightc, viper, 0, vipercf,  ROT0,  "Konami", "Mahjong Fight Club (ver C)", GAME_NOT_WORKING|GAME_NO_SOUND)
