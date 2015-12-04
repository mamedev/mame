// license:BSD-3-Clause
// copyright-holders:Guru
/***************************************************************************

X Tom 3D

TODO:
- clears a work RAM snippet then jumps to that snippet ... it doesn't do
  that if you soft reset emulation.
- understand how to load game ROMs

This game runs on PC-based hardware.
Major components are....

MAIN BOARD
----------
    CPU: Intel Celeron (socket 370) 333MHz
Chipset: Intel AGPset FW822443ZX, PCIset FW82371EB
    RAM: Samsung KMM366S823CTS 8M x 64-bit SDRAM DIMM
  Video: 3DFX 500-0013-04 PCB-mounted BGA
         EliteMT M32L1632512A video RAM (x4)
         14.31818MHz XTAL
   BIOS: Atmel 29C010 flash ROM
  Other: Holtek HT6542B i8042-based keyboard controller
         3V coin battery

SOUND BOARD
-----------
A40MX04 QFP84 CPLD
Yamaha YMZ280B + YAC516
16MHz XTAL
PIC12C508 (secured, not read)
Atmel 93C46 EEPROM
LM358 OP AMP (x3)

ROM BOARD
---------
MX29F1610MC 16M FlashROM (x7)

***************************************************************************/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/lpci.h"
#include "machine/pcshare.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"
#include "video/pc_vga.h"


class xtom3d_state : public pcat_base_state
{
public:
	xtom3d_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag)
			{ }

	UINT32 *m_bios_ram;
	UINT32 *m_bios_ext1_ram;
	UINT32 *m_bios_ext2_ram;
	UINT32 *m_bios_ext3_ram;
	UINT32 *m_bios_ext4_ram;
	UINT32 *m_isa_ram1;
	UINT32 *m_isa_ram2;
	UINT8 m_mtxc_config_reg[256];
	UINT8 m_piix4_config_reg[4][256];

	DECLARE_WRITE32_MEMBER( isa_ram1_w );
	DECLARE_WRITE32_MEMBER( isa_ram2_w );

	DECLARE_WRITE32_MEMBER( bios_ext1_ram_w );
	DECLARE_WRITE32_MEMBER( bios_ext2_ram_w );
	DECLARE_WRITE32_MEMBER( bios_ext3_ram_w );
	DECLARE_WRITE32_MEMBER( bios_ext4_ram_w );

	DECLARE_WRITE32_MEMBER( bios_ram_w );
	virtual void machine_start();
	virtual void machine_reset();
	void intel82439tx_init();
};

// Intel 82439TX System Controller (MTXC)

static UINT8 mtxc_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
	xtom3d_state *state = busdevice->machine().driver_data<xtom3d_state>();
//  osd_printf_debug("MTXC: read %d, %02X\n", function, reg);

	return state->m_mtxc_config_reg[reg];
}

static void mtxc_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	xtom3d_state *state = busdevice->machine().driver_data<xtom3d_state>();
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
				state->membank("bios_bank")->set_base(state->m_bios_ram);
			else                    // disable RAM access (reads go to BIOS ROM)
				state->membank("bios_bank")->set_base(state->memregion("bios")->base() + 0x10000);
			break;
		}
		case 0x5a: // PAM1
		{
			if (data & 0x1)
				state->membank("video_bank1")->set_base(state->m_isa_ram1);
			else
				state->membank("video_bank1")->set_base(state->memregion("video_bios")->base() + 0);

			if (data & 0x10)
				state->membank("video_bank2")->set_base(state->m_isa_ram2);
			else
				state->membank("video_bank2")->set_base(state->memregion("video_bios")->base() + 0x4000);

			break;
		}
		case 0x5e: // PAM5
		{
			if (data & 0x1)
				state->membank("bios_ext1")->set_base(state->m_bios_ext1_ram);
			else
				state->membank("bios_ext1")->set_base(state->memregion("bios")->base() + 0);

			if (data & 0x10)
				state->membank("bios_ext2")->set_base(state->m_bios_ext2_ram);
			else
				state->membank("bios_ext2")->set_base(state->memregion("bios")->base() + 0x4000);

			break;
		}
		case 0x5f: // PAM6
		{
			if (data & 0x1)
				state->membank("bios_ext3")->set_base(state->m_bios_ext3_ram);
			else
				state->membank("bios_ext3")->set_base(state->memregion("bios")->base() + 0x8000);

			if (data & 0x10)
				state->membank("bios_ext4")->set_base(state->m_bios_ext4_ram);
			else
				state->membank("bios_ext4")->set_base(state->memregion("bios")->base() + 0xc000);

			break;
		}
	}

	state->m_mtxc_config_reg[reg] = data;
}

void xtom3d_state::intel82439tx_init()
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
	xtom3d_state *state = busdevice->machine().driver_data<xtom3d_state>();
//  osd_printf_debug("PIIX4: read %d, %02X\n", function, reg);
	return state->m_piix4_config_reg[function][reg];
}

static void piix4_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	xtom3d_state *state = busdevice->machine().driver_data<xtom3d_state>();
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


WRITE32_MEMBER(xtom3d_state::isa_ram1_w)
{
	if (m_mtxc_config_reg[0x5a] & 0x2)      // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_isa_ram1 + offset);
	}
}

WRITE32_MEMBER(xtom3d_state::isa_ram2_w)
{
	if (m_mtxc_config_reg[0x5a] & 0x2)      // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_isa_ram2 + offset);
	}
}

WRITE32_MEMBER(xtom3d_state::bios_ext1_ram_w)
{
	if (m_mtxc_config_reg[0x5e] & 0x2)      // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext1_ram + offset);
	}
}


WRITE32_MEMBER(xtom3d_state::bios_ext2_ram_w)
{
	if (m_mtxc_config_reg[0x5e] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext2_ram + offset);
	}
}


WRITE32_MEMBER(xtom3d_state::bios_ext3_ram_w)
{
	if (m_mtxc_config_reg[0x5f] & 0x2)      // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext3_ram + offset);
	}
}


WRITE32_MEMBER(xtom3d_state::bios_ext4_ram_w)
{
	if (m_mtxc_config_reg[0x5f] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext4_ram + offset);
	}
}


WRITE32_MEMBER(xtom3d_state::bios_ram_w)
{
	if (m_mtxc_config_reg[0x59] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ram + offset);
	}
}

static ADDRESS_MAP_START(xtom3d_map, AS_PROGRAM, 32, xtom3d_state)
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
	AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)    /* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(xtom3d_io, AS_IO, 32, xtom3d_state)
	AM_IMPORT_FROM(pcat32_io_common)

	AM_RANGE(0x00e8, 0x00ef) AM_NOP

	AM_RANGE(0x03b0, 0x03bf) AM_DEVREADWRITE8("vga", vga_device, port_03b0_r, port_03b0_w, 0xffffffff)
	AM_RANGE(0x03c0, 0x03cf) AM_DEVREADWRITE8("vga", vga_device, port_03c0_r, port_03c0_w, 0xffffffff)
	AM_RANGE(0x03d0, 0x03df) AM_DEVREADWRITE8("vga", vga_device, port_03d0_r, port_03d0_w, 0xffffffff)

	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_legacy_device, read, write)
ADDRESS_MAP_END


void xtom3d_state::machine_start()
{
	m_bios_ram = auto_alloc_array(machine(), UINT32, 0x10000/4);
	m_bios_ext1_ram = auto_alloc_array(machine(), UINT32, 0x4000/4);
	m_bios_ext2_ram = auto_alloc_array(machine(), UINT32, 0x4000/4);
	m_bios_ext3_ram = auto_alloc_array(machine(), UINT32, 0x4000/4);
	m_bios_ext4_ram = auto_alloc_array(machine(), UINT32, 0x4000/4);
	m_isa_ram1 = auto_alloc_array(machine(), UINT32, 0x4000/4);
	m_isa_ram2 = auto_alloc_array(machine(), UINT32, 0x4000/4);

	intel82439tx_init();
}

void xtom3d_state::machine_reset()
{
	membank("bios_bank")->set_base(memregion("bios")->base() + 0x10000);
	membank("bios_ext1")->set_base(memregion("bios")->base() + 0);
	membank("bios_ext2")->set_base(memregion("bios")->base() + 0x4000);
	membank("bios_ext3")->set_base(memregion("bios")->base() + 0x8000);
	membank("bios_ext4")->set_base(memregion("bios")->base() + 0xc000);
	membank("video_bank1")->set_base(memregion("video_bios")->base() + 0);
	membank("video_bank2")->set_base(memregion("video_bios")->base() + 0x4000);
}

static MACHINE_CONFIG_START( xtom3d, xtom3d_state )
	MCFG_CPU_ADD("maincpu", PENTIUM2, 450000000/16)  // actually Pentium II 450
	MCFG_CPU_PROGRAM_MAP(xtom3d_map)
	MCFG_CPU_IO_MAP(xtom3d_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_1", pic8259_device, inta_cb)


	MCFG_FRAGMENT_ADD( pcat_common )

	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE(0, nullptr, intel82439tx_pci_r, intel82439tx_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(7, nullptr, intel82371ab_pci_r, intel82371ab_pci_w)

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_vga )
MACHINE_CONFIG_END


ROM_START( xtom3d )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "bios.u22", 0x000000, 0x020000, CRC(f7c58044) SHA1(fd967d009e0d3c8ed9dd7be852946f2b9dee7671) )

	ROM_REGION( 0x8000, "video_bios", 0 ) // TODO: no VGA card is hooked up, to be removed
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, BAD_DUMP CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )

	ROM_REGION(0xe00000, "user2", 0)
	ROM_LOAD( "u3",  0x000000, 0x200000, CRC(f332e030) SHA1(f04fc7fc97e6ada8122ea7d111455043d7cc42df) )
	ROM_LOAD( "u4",  0x200000, 0x200000, CRC(ac40ea0b) SHA1(6fcb86f493885d62d20df6bddaa1a1b19d478c65) )
	ROM_LOAD( "u5",  0x400000, 0x200000, CRC(0fb98a20) SHA1(d21f33b0ca65dc6f90a411a9682f960e9c60244c) )
	ROM_LOAD( "u6",  0x600000, 0x200000, CRC(5c092c58) SHA1(d347e1ed957cc989dc71f4f347af926589ae926d) )
	ROM_LOAD( "u7",  0x800000, 0x200000, CRC(833c179c) SHA1(586555f5a4066a762fc05a43ef01be9fa202bb7f) )
	ROM_LOAD( "u19", 0xa00000, 0x200000, CRC(a1ae73d0) SHA1(232c73bfee426b5f651a015c505c26b8ed7176b7) )
	ROM_LOAD( "u20", 0xc00000, 0x200000, CRC(452131d9) SHA1(f62a0f1a7da9025ac1f7d5de4df90166871ac1e5) )
ROM_END


GAME(1999, xtom3d, 0, xtom3d, at_keyboard, driver_device, 0, ROT0, "Jamie System Development", "X Tom 3D", MACHINE_IS_SKELETON)
