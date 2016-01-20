// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Midway Quicksilver skeleton driver

    TODO:
    - offrthnd: illegal opcode tripped just after that PIIX4 is recognized

    Main CPU : Intel Celeron 333/366MHz
    Motherboard : Intel SE440BX-2
    RAM : 64MB PC100-222-620 non-ecc
    Sound: Integrated YMF740G
    Networking: SMC EZ Card 10 / SMC1208T (probably 10ec:8029 1113:1208)
    Graphics Chips : Quantum Obsidian 3DFX
    Storage : Hard Drive

    Chipsets (440BX AGPset):
    - 82371EB PCI-ISA bridge
    - 82371EB Power Management Controller
    - 82371AB/EB Universal Host Controller (USB UHCI)
    - 82371AB/EB PCI Bus Master IDE Controller

***************************************************************************/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/lpci.h"
#include "machine/pcshare.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"
#include "video/pc_vga.h"

class midqslvr_state : public pcat_base_state
{
public:
	midqslvr_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag)
	{
	}

	std::unique_ptr<UINT32[]> m_bios_ram;
	std::unique_ptr<UINT32[]> m_bios_ext1_ram;
	std::unique_ptr<UINT32[]> m_bios_ext2_ram;
	std::unique_ptr<UINT32[]> m_bios_ext3_ram;
	std::unique_ptr<UINT32[]> m_bios_ext4_ram;
	std::unique_ptr<UINT32[]> m_isa_ram1;
	std::unique_ptr<UINT32[]> m_isa_ram2;
	UINT8 m_mtxc_config_reg[256];
	UINT8 m_piix4_config_reg[4][256];

	DECLARE_WRITE32_MEMBER( isa_ram1_w );
	DECLARE_WRITE32_MEMBER( isa_ram2_w );

	DECLARE_WRITE32_MEMBER( bios_ext1_ram_w );
	DECLARE_WRITE32_MEMBER( bios_ext2_ram_w );
	DECLARE_WRITE32_MEMBER( bios_ext3_ram_w );
	DECLARE_WRITE32_MEMBER( bios_ext4_ram_w );

	DECLARE_WRITE32_MEMBER( bios_ram_w );
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void intel82439tx_init();
};


// Intel 82439TX System Controller (MTXC)

static UINT8 mtxc_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
	midqslvr_state *state = busdevice->machine().driver_data<midqslvr_state>();
//  osd_printf_debug("MTXC: read %d, %02X\n", function, reg);

	if((reg & 0xfc) == 0 && function == 0) // return vendor ID
		return (0x71008086 >> (reg & 3)*8) & 0xff;

	return state->m_mtxc_config_reg[reg];
}

static void mtxc_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	midqslvr_state *state = busdevice->machine().driver_data<midqslvr_state>();
	printf("MTXC: write %d, %02X, %02X\n",  function, reg, data);

	/*
	memory banking with North Bridge:
	0x59 (PAM0) xxxx ---- BIOS area 0xf0000-0xfffff
	            ---- xxxx Reserved
	0x5a (PAM1) xxxx ---- ISA add-on BIOS 0xc4000 - 0xc7fff
	            ---- xxxx ISA add-on BIOS 0xc0000 - 0xc3fff
	0x5b (PAM2) xxxx ---- ISA add-on BIOS 0xcc000 - 0xcffff
	            ---- xxxx ISA add-on BIOS 0xc8000 - 0xcbfff
	0x5c (PAM3) xxxx ---- ISA add-on BIOS 0xd4000 - 0xd7fff
	            ---- xxxx ISA add-on BIOS 0xd0000 - 0xd3fff
	0x5d (PAM4) xxxx ---- ISA add-on BIOS 0xdc000 - 0xdffff
	            ---- xxxx ISA add-on BIOS 0xd8000 - 0xdbfff
	0x5e (PAM5) xxxx ---- BIOS extension 0xe4000 - 0xe7fff
	            ---- xxxx BIOS extension 0xe0000 - 0xe3fff
	0x5f (PAM6) xxxx ---- BIOS extension 0xec000 - 0xeffff
	            ---- xxxx BIOS extension 0xe8000 - 0xebfff

	3210 -> 3 = reserved, 2 = Cache Enable, 1 = Write Enable, 0 = Read Enable
	*/

	switch(reg)
	{
		case 0x59: // PAM0
		{
			if (data & 0x10)        // enable RAM access to region 0xf0000 - 0xfffff
				state->membank("bios_bank")->set_base(state->m_bios_ram.get());
			else                    // disable RAM access (reads go to BIOS ROM)
				state->membank("bios_bank")->set_base(state->memregion("bios")->base() + 0x70000);
			break;
		}
		case 0x5a: // PAM1
		{
			if (data & 0x1)
				state->membank("video_bank1")->set_base(state->m_isa_ram1.get());
			else
				state->membank("video_bank1")->set_base(state->memregion("video_bios")->base() + 0);

			if (data & 0x10)
				state->membank("video_bank2")->set_base(state->m_isa_ram2.get());
			else
				state->membank("video_bank2")->set_base(state->memregion("video_bios")->base() + 0x4000);

			break;
		}
		case 0x5e: // PAM5
		{
			if (data & 0x1)
				state->membank("bios_ext1")->set_base(state->m_bios_ext1_ram.get());
			else
				state->membank("bios_ext1")->set_base(state->memregion("bios")->base() + 0x60000);

			if (data & 0x10)
				state->membank("bios_ext2")->set_base(state->m_bios_ext2_ram.get());
			else
				state->membank("bios_ext2")->set_base(state->memregion("bios")->base() + 0x64000);

			break;
		}
		case 0x5f: // PAM6
		{
			if (data & 0x1)
				state->membank("bios_ext3")->set_base(state->m_bios_ext3_ram.get());
			else
				state->membank("bios_ext3")->set_base(state->memregion("bios")->base() + 0x68000);

			if (data & 0x10)
				state->membank("bios_ext4")->set_base(state->m_bios_ext4_ram.get());
			else
				state->membank("bios_ext4")->set_base(state->memregion("bios")->base() + 0x6c000);

			break;
		}
	}

	state->m_mtxc_config_reg[reg] = data;
}

void midqslvr_state::intel82439tx_init()
{
	m_mtxc_config_reg[0x60] = 0x02;
	m_mtxc_config_reg[0x61] = 0x02;
	m_mtxc_config_reg[0x62] = 0x02;
	m_mtxc_config_reg[0x63] = 0x02;
	m_mtxc_config_reg[0x64] = 0x02;
	m_mtxc_config_reg[0x65] = 0x02;
}

static UINT32 intel82439tx_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
	UINT32 r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= mtxc_config_r(busdevice, device, function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= mtxc_config_r(busdevice, device, function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= mtxc_config_r(busdevice, device, function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= mtxc_config_r(busdevice, device, function, reg + 0) << 0;
	}
	return r;
}

static void intel82439tx_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		mtxc_config_w(busdevice, device, function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		mtxc_config_w(busdevice, device, function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		mtxc_config_w(busdevice, device, function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		mtxc_config_w(busdevice, device, function, reg + 0, (data >> 0) & 0xff);
	}
}

// Intel 82371AB PCI-to-ISA / IDE bridge (PIIX4)

static UINT8 piix4_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
	midqslvr_state *state = busdevice->machine().driver_data<midqslvr_state>();
	address_space &space = state->m_maincpu->space( AS_PROGRAM );

	function &= 3;

	if((reg & 0xfc) == 0) // return vendor ID
		return (((0x71108086 | (function & 3) << 16) >> (reg & 3)*8) & 0xff);

	if(reg == 0xe)
	{
		const UINT8 header_type_val[4] = { 0x80, 0x00, 0x00, 0x00 };
		return header_type_val[function];
	}

	if((reg & 0xfc) == 0x8)
	{
		/* TODO: reg 8 indicates Revision ID */
		const UINT32 class_code_val[4] = { 0x06010000, 0x01018000, 0x0c030000, 0x06800000 };

		return (((class_code_val[function]) >> (reg & 3)*8) & 0xff);
	}

	printf("%08x PIIX4: read %d, %02X\n", space.device().safe_pc(), function, reg);

	return state->m_piix4_config_reg[function][reg];
}

static void piix4_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	midqslvr_state *state = busdevice->machine().driver_data<midqslvr_state>();
	printf("PIIX4: write %d, %02X, %02X\n", function, reg, data);

	function &= 3;

	state->m_piix4_config_reg[function][reg] = data;
}

static UINT32 intel82371ab_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
	UINT32 r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 0) << 0;
	}
	return r;
}

static void intel82371ab_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		piix4_config_w(busdevice, device, function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		piix4_config_w(busdevice, device, function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		piix4_config_w(busdevice, device, function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		piix4_config_w(busdevice, device, function, reg + 0, (data >> 0) & 0xff);
	}
}


WRITE32_MEMBER(midqslvr_state::isa_ram1_w)
{
	if (m_mtxc_config_reg[0x5a] & 0x2)      // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_isa_ram1.get() + offset);
	}
}

WRITE32_MEMBER(midqslvr_state::isa_ram2_w)
{
	if (m_mtxc_config_reg[0x5a] & 0x2)      // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_isa_ram2.get() + offset);
	}
}

WRITE32_MEMBER(midqslvr_state::bios_ext1_ram_w)
{
	if (m_mtxc_config_reg[0x5e] & 0x2)      // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext1_ram.get() + offset);
	}
}


WRITE32_MEMBER(midqslvr_state::bios_ext2_ram_w)
{
	if (m_mtxc_config_reg[0x5e] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext2_ram.get() + offset);
	}
}


WRITE32_MEMBER(midqslvr_state::bios_ext3_ram_w)
{
	if (m_mtxc_config_reg[0x5f] & 0x2)      // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext3_ram.get() + offset);
	}
}


WRITE32_MEMBER(midqslvr_state::bios_ext4_ram_w)
{
	if (m_mtxc_config_reg[0x5f] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext4_ram.get() + offset);
	}
}


WRITE32_MEMBER(midqslvr_state::bios_ram_w)
{
	if (m_mtxc_config_reg[0x59] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ram.get() + offset);
	}
}

static ADDRESS_MAP_START(midqslvr_map, AS_PROGRAM, 32, midqslvr_state)
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_DEVREADWRITE8("vga", vga_device, mem_r, mem_w, 0xffffffff)
	AM_RANGE(0x000c0000, 0x000c3fff) AM_ROMBANK("video_bank1") AM_WRITE(isa_ram1_w)
	AM_RANGE(0x000c4000, 0x000c7fff) AM_ROMBANK("video_bank2") AM_WRITE(isa_ram2_w)
	AM_RANGE(0x000e0000, 0x000e3fff) AM_ROMBANK("bios_ext1") AM_WRITE(bios_ext1_ram_w)
	AM_RANGE(0x000e4000, 0x000e7fff) AM_ROMBANK("bios_ext2") AM_WRITE(bios_ext2_ram_w)
	AM_RANGE(0x000e8000, 0x000ebfff) AM_ROMBANK("bios_ext3") AM_WRITE(bios_ext3_ram_w)
	AM_RANGE(0x000ec000, 0x000effff) AM_ROMBANK("bios_ext4") AM_WRITE(bios_ext4_ram_w)
	AM_RANGE(0x000f0000, 0x000fffff) AM_ROMBANK("bios_bank") AM_WRITE(bios_ram_w)
	AM_RANGE(0x00100000, 0x01ffffff) AM_RAM
	AM_RANGE(0xfff80000, 0xffffffff) AM_ROM AM_REGION("bios", 0)    /* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(midqslvr_io, AS_IO, 32, midqslvr_state)
	AM_IMPORT_FROM(pcat32_io_common)
	AM_RANGE(0x00e8, 0x00ef) AM_NOP

	AM_RANGE(0x01f0, 0x01f7) AM_DEVREADWRITE16("ide", ide_controller_device, read_cs0, write_cs0, 0xffffffff)
	AM_RANGE(0x03b0, 0x03bf) AM_DEVREADWRITE8("vga", vga_device, port_03b0_r, port_03b0_w, 0xffffffff)
	AM_RANGE(0x03c0, 0x03cf) AM_DEVREADWRITE8("vga", vga_device, port_03c0_r, port_03c0_w, 0xffffffff)
	AM_RANGE(0x03d0, 0x03df) AM_DEVREADWRITE8("vga", vga_device, port_03d0_r, port_03d0_w, 0xffffffff)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVREADWRITE16("ide", ide_controller_device, read_cs1, write_cs1, 0xffffffff)

	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_legacy_device, read, write)
ADDRESS_MAP_END

void midqslvr_state::machine_start()
{
	m_bios_ram = std::make_unique<UINT32[]>(0x10000/4);
	m_bios_ext1_ram = std::make_unique<UINT32[]>(0x4000/4);
	m_bios_ext2_ram = std::make_unique<UINT32[]>(0x4000/4);
	m_bios_ext3_ram = std::make_unique<UINT32[]>(0x4000/4);
	m_bios_ext4_ram = std::make_unique<UINT32[]>(0x4000/4);
	m_isa_ram1 = std::make_unique<UINT32[]>(0x4000/4);
	m_isa_ram2 = std::make_unique<UINT32[]>(0x4000/4);
	intel82439tx_init();

}

void midqslvr_state::machine_reset()
{
	membank("bios_bank")->set_base(memregion("bios")->base() + 0x70000);
	membank("bios_ext1")->set_base(memregion("bios")->base() + 0x60000);
	membank("bios_ext2")->set_base(memregion("bios")->base() + 0x64000);
	membank("bios_ext3")->set_base(memregion("bios")->base() + 0x68000);
	membank("bios_ext4")->set_base(memregion("bios")->base() + 0x6c000);
	membank("video_bank1")->set_base(memregion("video_bios")->base() + 0);
	membank("video_bank2")->set_base(memregion("video_bios")->base() + 0x4000);
}

static MACHINE_CONFIG_START( midqslvr, midqslvr_state )
	MCFG_CPU_ADD("maincpu", PENTIUM, 333000000) // actually Celeron 333
	MCFG_CPU_PROGRAM_MAP(midqslvr_map)
	MCFG_CPU_IO_MAP(midqslvr_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_1", pic8259_device, inta_cb)

	MCFG_FRAGMENT_ADD( pcat_common )

	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE( 0, nullptr, intel82439tx_pci_r, intel82439tx_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(31, nullptr, intel82371ab_pci_r, intel82371ab_pci_w)

	MCFG_IDE_CONTROLLER_ADD("ide", ata_devices, "hdd", nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("pic8259_2", pic8259_device, ir6_w))

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_vga )
MACHINE_CONFIG_END


ROM_START( offrthnd )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD( "lh28f004sct.u8b1", 0x000000, 0x080000, CRC(ab04a343) SHA1(ba77933400fe470f45ab187bc0d315922caadb12) )

	ROM_REGION( 0x8000, "video_bios", ROMREGION_ERASEFF ) // TODO: no VGA card is hooked up, to be removed
//  ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, BAD_DUMP CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
//  ROM_CONTINUE(                                 0x0001, 0x4000 )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "offrthnd", 0, SHA1(d88f1c5b75361a1e310565a8a5a09c674a4a1a22) )
ROM_END

ROM_START( hydrthnd )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD( "lh28f004sct.u8b1", 0x000000, 0x080000, CRC(ab04a343) SHA1(ba77933400fe470f45ab187bc0d315922caadb12) )

	ROM_REGION( 0x8000, "video_bios", ROMREGION_ERASEFF ) // TODO: no VGA card is hooked up, to be removed
//  ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, BAD_DUMP CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
//  ROM_CONTINUE(                                 0x0001, 0x4000 )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "hydro", 0,  SHA1(d481d178782943c066b41764628a419cd55f676d) )
ROM_END

ROM_START( arctthnd )
	ROM_REGION32_LE(0x80000, "bios", ROMREGION_ERASEFF)
	ROM_LOAD( "m29f002bt.u6", 0x040000, 0x040000, CRC(012c9290) SHA1(cdee6f19d5e5ea5bb1dd6a5ec397ac70b3452790) )

	ROM_REGION( 0x8000, "video_bios", ROMREGION_ERASEFF ) // TODO: no VGA card is hooked up, to be removed
//  ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, BAD_DUMP CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
//  ROM_CONTINUE(                                 0x0001, 0x4000 )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "arctthnd", 0,  SHA1(f4373e57c3f453ac09c735b5d8d99ff811416a23) )
ROM_END

// this also required a dongle to work
ROM_START( ultarctc )
	ROM_REGION32_LE(0x80000, "bios", ROMREGION_ERASEFF)
	ROM_LOAD( "m29f002bt.u6", 0x040000, 0x040000, CRC(012c9290) SHA1(cdee6f19d5e5ea5bb1dd6a5ec397ac70b3452790) )

	ROM_REGION( 0x8000, "video_bios", ROMREGION_ERASEFF ) // TODO: no VGA card is hooked up, to be removed
//  ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, BAD_DUMP CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
//  ROM_CONTINUE(                                 0x0001, 0x4000 )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "uarctict", 0, SHA1(8557a1d7ae8dc41c879350cb1c228f4c27a0dd09) )
ROM_END

// this is an update CD, We don't know if it updates the HDD image we have, if the image we have is already an updated version, if it
// requires a specific version we don't have, or even if it updates a regular Arctic Thunder to Ultimate.
ROM_START( ultarctcup )
	ROM_REGION32_LE(0x80000, "bios", ROMREGION_ERASEFF)
	ROM_LOAD( "m29f002bt.u6", 0x040000, 0x040000, CRC(012c9290) SHA1(cdee6f19d5e5ea5bb1dd6a5ec397ac70b3452790) )

	ROM_REGION( 0x8000, "video_bios", ROMREGION_ERASEFF ) // TODO: no VGA card is hooked up, to be removed
//  ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, BAD_DUMP CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
//  ROM_CONTINUE(                                 0x0001, 0x4000 )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "uarctict", 0, SHA1(8557a1d7ae8dc41c879350cb1c228f4c27a0dd09) )

	DISK_REGION( "cd" )
	DISK_IMAGE( "040503_1309", 0, SHA1(453adb81e204b0580ad02c2d98f68525757ec2a1) )
// sourced from these
//    ROM_LOAD( "040503_1309.CUE", 0x0000, 0x000004d, CRC(4a9e2de5) SHA1(04d3d90ad4b235c0ac4606557e16a1410d018fa9) )
//    ROM_LOAD( "040503_1309.BIN", 0x0000, 0x6bd9960, CRC(48a63422) SHA1(9d1cacf07526c5bddf4205c667a9010802f74859) )

ROM_END

// there are almost certainly multiple versions of these; updates were offered on floppy disk.  The version numbers for the existing CHDs are unknown.
GAME(1999, hydrthnd,    0,        midqslvr, at_keyboard, driver_device, 0, ROT0, "Midway Games", "Hydro Thunder", MACHINE_IS_SKELETON)

GAME(2000, offrthnd,    0,        midqslvr, at_keyboard, driver_device, 0, ROT0, "Midway Games", "Offroad Thunder", MACHINE_IS_SKELETON)

GAME(2001, arctthnd,    0,        midqslvr, at_keyboard, driver_device, 0, ROT0, "Midway Games", "Arctic Thunder (v1.002)", MACHINE_IS_SKELETON)

GAME(2001, ultarctc,    0,        midqslvr, at_keyboard, driver_device, 0, ROT0, "Midway Games", "Ultimate Arctic Thunder", MACHINE_IS_SKELETON)
GAME(2004, ultarctcup,  ultarctc, midqslvr, at_keyboard, driver_device, 0, ROT0, "Midway Games", "Ultimate Arctic Thunder Update CD ver 1.950 (5/3/04)", MACHINE_IS_SKELETON)
