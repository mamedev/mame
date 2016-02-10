// license:BSD-3-Clause
// copyright-holders:Peter Ferrie
/* Queen */

/*

Produttore  STG
N.revisione
CPU main PCB is a standard EPIA
ROMs    epia BIOS + solid state HD

1x VIA EPIA5000EAG (main PCB) with:
VT8231 South Bridge
VIA Eden Processor
VIA EPIA Companion Chip VT1612A (Audio CODEC)
VIA EPIA Companion Chip VT6103 (Networking)
processor speed is 533MHz <- likely to be a Celeron or a Pentium III class CPU -AS

 it's a 2002 era PC at least based on the BIOS,
  almost certainly newer than the standard 'PENTIUM' CPU

- update by Peter Ferrie:
- split BIOS region into 64kb blocks and implement missing PAM registers
- VIA Apollo VXPro chipset is not compatible with Intel i430.

*/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/lpci.h"
#include "machine/pcshare.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"
#include "video/pc_vga.h"


class queen_state : public pcat_base_state
{
public:
	queen_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag)
	{
	}

	std::unique_ptr<UINT32[]> m_bios_ram;
	std::unique_ptr<UINT32[]> m_bios_ext_ram;
	UINT8 m_mtxc_config_reg[256];
	UINT8 m_piix4_config_reg[4][256];

	DECLARE_WRITE32_MEMBER( bios_ext_ram_w );

	DECLARE_WRITE32_MEMBER( bios_ram_w );
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void intel82439tx_init();
};


// Intel 82439TX System Controller (MTXC)

static UINT8 mtxc_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
	queen_state *state = busdevice->machine().driver_data<queen_state>();
//  osd_printf_debug("MTXC: read %d, %02X\n", function, reg);

	return state->m_mtxc_config_reg[reg];
}

static void mtxc_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	queen_state *state = busdevice->machine().driver_data<queen_state>();
	printf("MTXC: write %d, %02X, %02X\n",  function, reg, data);

	/*
	memory banking with North Bridge:
	0x63 (PAM)  xx-- ---- BIOS area 0xf0000-0xfffff
	            --xx ---- BIOS extension 0xe0000 - 0xeffff
	            ---- xx-- ISA add-on BIOS 0xd0000 - 0xdffff
	            ---- --xx ISA add-on BIOS 0xc0000 - 0xcffff

	10 -> 1 = Write Enable, 0 = Read Enable
	*/

	if (reg == 0x63)
	{
		if (data & 0x20)        // enable RAM access to region 0xf0000 - 0xfffff
			state->membank("bios_bank")->set_base(state->m_bios_ram.get());
		else                    // disable RAM access (reads go to BIOS ROM)
			state->membank("bios_bank")->set_base(state->memregion("bios")->base() + 0x30000);
		if (data & 0x80)        // enable RAM access to region 0xe0000 - 0xeffff
			state->membank("bios_ext")->set_base(state->m_bios_ext_ram.get());
		else
			state->membank("bios_ext")->set_base(state->memregion("bios")->base() + 0x20000);
	}

	state->m_mtxc_config_reg[reg] = data;
}

void queen_state::intel82439tx_init()
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
	if ((function >= 4) && (function <= 7))
	{
		return 0; // BIOS performs a brute-force scan for devices
	}

	queen_state *state = busdevice->machine().driver_data<queen_state>();
//  osd_printf_debug("PIIX4: read %d, %02X\n", function, reg);
	return state->m_piix4_config_reg[function][reg];
}

static void piix4_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	queen_state *state = busdevice->machine().driver_data<queen_state>();
//  osd_printf_debug("%s:PIIX4: write %d, %02X, %02X\n", machine.describe_context(), function, reg, data);
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


WRITE32_MEMBER(queen_state::bios_ext_ram_w)
{
	if (m_mtxc_config_reg[0x63] & 0x40)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext_ram.get() + offset);
	}
}


WRITE32_MEMBER(queen_state::bios_ram_w)
{
	if (m_mtxc_config_reg[0x63] & 0x10)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ram.get() + offset);
	}
}

static ADDRESS_MAP_START( queen_map, AS_PROGRAM, 32, queen_state )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_DEVREADWRITE8("vga", vga_device, mem_r, mem_w, 0xffffffff)
	AM_RANGE(0x000e0000, 0x000effff) AM_ROMBANK("bios_ext") AM_WRITE(bios_ext_ram_w)
	AM_RANGE(0x000f0000, 0x000fffff) AM_ROMBANK("bios_bank") AM_WRITE(bios_ram_w)
	AM_RANGE(0x00100000, 0x01ffffff) AM_RAM
	AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)    /* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START( queen_io, AS_IO, 32, queen_state )
	AM_IMPORT_FROM(pcat32_io_common)
	AM_RANGE(0x00e8, 0x00ef) AM_NOP

	AM_RANGE(0x0170, 0x0177) AM_DEVREADWRITE("ide2", ide_controller_32_device, read_cs0, write_cs0)
	AM_RANGE(0x01f0, 0x01f7) AM_DEVREADWRITE16("ide", ide_controller_device, read_cs0, write_cs0, 0xffffffff)
	AM_RANGE(0x0370, 0x0377) AM_DEVREADWRITE("ide2", ide_controller_32_device, read_cs1, write_cs1)
	AM_RANGE(0x03b0, 0x03bf) AM_DEVREADWRITE8("vga", vga_device, port_03b0_r, port_03b0_w, 0xffffffff)
	AM_RANGE(0x03c0, 0x03cf) AM_DEVREADWRITE8("vga", vga_device, port_03c0_r, port_03c0_w, 0xffffffff)
	AM_RANGE(0x03d0, 0x03df) AM_DEVREADWRITE8("vga", vga_device, port_03d0_r, port_03d0_w, 0xffffffff)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVREADWRITE16("ide", ide_controller_device, read_cs1, write_cs1, 0xffffffff)

	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_legacy_device, read, write)
ADDRESS_MAP_END

void queen_state::machine_start()
{
	m_bios_ram = std::make_unique<UINT32[]>(0x10000/4);
	m_bios_ext_ram = std::make_unique<UINT32[]>(0x10000/4);

	intel82439tx_init();
}

void queen_state::machine_reset()
{
	membank("bios_bank")->set_base(memregion("bios")->base() + 0x30000);
	membank("bios_ext")->set_base(memregion("bios")->base() + 0x20000);
}



static MACHINE_CONFIG_START( queen, queen_state )
	MCFG_CPU_ADD("maincpu", PENTIUM3, 533000000/16) // Celeron or Pentium 3, 533 Mhz
	MCFG_CPU_PROGRAM_MAP(queen_map)
	MCFG_CPU_IO_MAP(queen_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_1", pic8259_device, inta_cb)

	MCFG_FRAGMENT_ADD( pcat_common )

	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE(0, nullptr, intel82439tx_pci_r, intel82439tx_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(7, nullptr, intel82371ab_pci_r, intel82371ab_pci_w)

	MCFG_IDE_CONTROLLER_ADD("ide", ata_devices, "hdd", nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("pic8259_2", pic8259_device, ir6_w))

	MCFG_IDE_CONTROLLER_32_ADD("ide2", ata_devices, nullptr, nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("pic8259_2", pic8259_device, ir7_w))

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_vga )
MACHINE_CONFIG_END




ROM_START( queen )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "bios-original.bin", 0x00000, 0x40000, CRC(feb542d4) SHA1(3cc5d8aeb0e3b7d9ed33248a4f3dc507d29debd9) )

	ROM_REGION( 0x8000, "video_bios", ROMREGION_ERASEFF ) // TODO: no VGA card is hooked up, to be removed
//  ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, BAD_DUMP CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
//  ROM_CONTINUE(                                 0x0001, 0x4000 )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "pqiidediskonmodule", 0,SHA1(a56efcc711b1c5a2e63160b3088001a8c4fb56c2) )
ROM_END


GAME( 2002?, queen,  0,    queen, at_keyboard, driver_device,  0, ROT0, "STG", "Queen?", MACHINE_IS_SKELETON )
