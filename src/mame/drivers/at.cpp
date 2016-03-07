// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
/***************************************************************************

    IBM AT Compatibles

***************************************************************************/

/* mingw-gcc defines this */
#ifdef i386
#undef i386
#endif /* i386 */


#include "emu.h"
#include "cpu/i86/i286.h"
#include "cpu/i386/i386.h"
#include "machine/at.h"
#include "machine/wd7600.h"
#include "machine/cs8221.h"
#include "machine/nvram.h"
#include "machine/vt82c496.h"
#include "bus/lpci/pci.h"
#include "bus/lpci/vt82c505.h"
#include "machine/ds128x.h"
#include "machine/ram.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"

class at_state : public driver_device
{
public:
	at_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb(*this, "mb"),
		m_ram(*this, RAM_TAG)
		{ }
	required_device<cpu_device> m_maincpu;
	required_device<at_mb_device> m_mb;
	required_device<ram_device> m_ram;
	DECLARE_DRIVER_INIT(at);
	DECLARE_DRIVER_INIT(atpci);
	DECLARE_DRIVER_INIT(megapcpla);
	DECLARE_READ16_MEMBER(ps1_unk_r);
	DECLARE_WRITE16_MEMBER(ps1_unk_w);
	DECLARE_READ8_MEMBER(ps1_portb_r);
	DECLARE_MACHINE_START(vrom_fix);

	void init_at_common(int xmsbase);
	UINT16 m_ps1_reg[2];
};

class megapc_state : public driver_device
{
public:
	megapc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_wd7600(*this, "wd7600"),
		m_isabus(*this, "isabus"),
		m_speaker(*this, "speaker")
		{ }

public:
	required_device<cpu_device> m_maincpu;
	required_device<wd7600_device> m_wd7600;
	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;

	DECLARE_DRIVER_INIT(megapc);
	DECLARE_DRIVER_INIT(megapcpl);

	DECLARE_READ16_MEMBER( wd7600_ior );
	DECLARE_WRITE16_MEMBER( wd7600_iow );
	DECLARE_WRITE_LINE_MEMBER( wd7600_hold );
	DECLARE_WRITE8_MEMBER( wd7600_tc ) { m_isabus->eop_w(offset, data); }
	DECLARE_WRITE_LINE_MEMBER( wd7600_spkr ) { m_speaker->level_w(state); }
};


static ADDRESS_MAP_START( at16_map, AS_PROGRAM, 16, at_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x09ffff) AM_RAMBANK("bank10")
	AM_RANGE(0x0e0000, 0x0fffff) AM_ROM AM_REGION("bios", 0)
	AM_RANGE(0xfe0000, 0xffffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( at16l_map, AS_PROGRAM, 16, at_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x09ffff) AM_RAMBANK("bank10")
	AM_RANGE(0x0e0000, 0x0fffff) AM_ROM AM_REGION("bios", 0x20000)
	AM_RANGE(0xfe0000, 0xffffff) AM_ROM AM_REGION("bios", 0x20000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( at32_map, AS_PROGRAM, 32, at_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAMBANK("bank10")
	AM_RANGE(0x000e0000, 0x000fffff) AM_ROM AM_REGION("bios", 0)
	AM_RANGE(0x00800000, 0x00800bff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( at32l_map, AS_PROGRAM, 32, at_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAMBANK("bank10")
	AM_RANGE(0x000e0000, 0x000fffff) AM_ROM AM_REGION("bios", 0x20000)
	AM_RANGE(0x00800000, 0x00800bff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("bios", 0x20000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ficpio_map, AS_PROGRAM, 32, at_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAMBANK("bank10")
	AM_RANGE(0x00800000, 0x00800bff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("isa", 0x20000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( at16_io, AS_IO, 16, at_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x00ff) AM_DEVICE("mb", at_mb_device, map)
ADDRESS_MAP_END

READ16_MEMBER( at_state::ps1_unk_r )
{
	return m_ps1_reg[offset];
}

WRITE16_MEMBER( at_state::ps1_unk_w )
{
	if((offset == 0) && (data == 0x60))
		data = 0x68;

	COMBINE_DATA(&m_ps1_reg[offset]);
}

READ8_MEMBER( at_state::ps1_portb_r )
{
	UINT8 data = m_mb->portb_r(space, offset);
	/* 0x10 is the dram refresh line bit, 15.085us. */
	data = (data & ~0x10) | ((machine().time().as_ticks(66291) & 1) ? 0x10 : 0);

	return data;
}

static ADDRESS_MAP_START(ps1_16_io, AS_IO, 16, at_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0060, 0x0061) AM_READ8(ps1_portb_r, 0xff00)
	AM_RANGE(0x0000, 0x00ff) AM_DEVICE("mb", at_mb_device, map)
	AM_RANGE(0x0102, 0x0105) AM_READWRITE(ps1_unk_r, ps1_unk_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( neat_io, AS_IO, 16, at_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0022, 0x0023) AM_DEVWRITE8("cs8221", cs8221_device, address_w, 0x00ff)
	AM_RANGE(0x0022, 0x0023) AM_DEVREADWRITE8("cs8221", cs8221_device, data_r, data_w, 0xff00)
	AM_RANGE(0x0000, 0x00ff) AM_DEVICE("mb", at_mb_device, map)
ADDRESS_MAP_END

static ADDRESS_MAP_START( at32_io, AS_IO, 32, at_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x00ff) AM_DEVICE16("mb", at_mb_device, map, 0xffffffff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ficpio_io, AS_IO, 32, at_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00a8, 0x00af) AM_DEVREADWRITE8("chipset", vt82c496_device, read, write, 0xffffffff)
	AM_RANGE(0x0000, 0x00ff) AM_DEVICE16("mb", at_mb_device, map, 0xffffffff)
	AM_RANGE(0x0170, 0x0177) AM_DEVREADWRITE("ide2", ide_controller_32_device, read_cs0, write_cs0)
	AM_RANGE(0x01f0, 0x01f7) AM_DEVREADWRITE("ide", ide_controller_32_device, read_cs0, write_cs0)
	AM_RANGE(0x0370, 0x0377) AM_DEVREADWRITE("ide2", ide_controller_32_device, read_cs1, write_cs1)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVREADWRITE("ide", ide_controller_32_device, read_cs1, write_cs1)
	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_device, read, write)
ADDRESS_MAP_END

DRIVER_INIT_MEMBER(megapc_state, megapc)
{
	UINT8* ROM = memregion("bios")->base();
	ROM[0x19145] = 0x45;  // hack to fix keyboard.  To be removed when the keyboard controller from the MegaPC is dumped
	ROM[0x1fea0] = 0x20;  // to correct checksum
}

DRIVER_INIT_MEMBER(megapc_state, megapcpl)
{
	UINT8* ROM = memregion("bios")->base();
	ROM[0x187b1] = 0x55;  // hack to fix keyboard.  To be removed when the keyboard controller from the MegaPC is dumped
	ROM[0x1fea0] = 0x20;  // to correct checksum
}

DRIVER_INIT_MEMBER(at_state, megapcpla)
{
	UINT8* ROM = memregion("bios")->base();

	init_at_common(0xa0000);

	ROM[0x33c2a] = 0x45;  // hack to fix keyboard.  To be removed when the keyboard controller from the MegaPC is dumped
	ROM[0x3af37] = 0x45;
	ROM[0x3cf1b] = 0x54;  // this will allow the keyboard to work during the POST memory test
	ROM[0x3fffe] = 0x1c;
	ROM[0x3ffff] = 0x41;  // to correct checksum
}

READ16_MEMBER( megapc_state::wd7600_ior )
{
	if (offset < 4)
		return m_isabus->dack_r(offset);
	else
		return m_isabus->dack16_r(offset);
}

WRITE16_MEMBER( megapc_state::wd7600_iow )
{
	if (offset < 4)
		m_isabus->dack_w(offset, data);
	else
		m_isabus->dack16_w(offset, data);
}

WRITE_LINE_MEMBER( megapc_state::wd7600_hold )
{
	// halt cpu
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	// and acknowledge hold
	m_wd7600->hlda_w(state);
}

static ADDRESS_MAP_START( megapc_map, AS_PROGRAM, 16, at_state )
ADDRESS_MAP_END

static ADDRESS_MAP_START( megapcpl_map, AS_PROGRAM, 32, at_state )
ADDRESS_MAP_END

static ADDRESS_MAP_START( megapc_io, AS_IO, 16, at_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static ADDRESS_MAP_START( megapcpl_io, AS_IO, 32, at_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

/**********************************************************
 *
 * Init functions
 *
 **********************************************************/

void at_state::init_at_common(int xmsbase)
{
	address_space& space = m_maincpu->space(AS_PROGRAM);

	/* MESS managed RAM */
	membank("bank10")->set_base(m_ram->pointer());

	if (m_ram->size() > xmsbase)
	{
		offs_t ram_limit = 0x100000 + m_ram->size() - xmsbase;
		space.install_read_bank(0x100000,  ram_limit - 1, "bank1");
		space.install_write_bank(0x100000,  ram_limit - 1, "bank1");
		membank("bank1")->set_base(m_ram->pointer() + xmsbase);
	}
}

DRIVER_INIT_MEMBER(at_state,at)
{
	init_at_common(0xa0000);
}

DRIVER_INIT_MEMBER(at_state,atpci)
{
	init_at_common(0x100000);
}

MACHINE_START_MEMBER(at_state,vrom_fix)
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	space.install_read_bank(0xc0000, 0xcffff, "vrom_bank");
	membank("vrom_bank")->set_base(machine().root_device().memregion("bios")->base());
}

static SLOT_INTERFACE_START( pci_devices )
	SLOT_INTERFACE_INTERNAL("vt82c505", VT82C505)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( ibm5170, at_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80286, XTAL_12MHz/2 /*6000000*/)
	MCFG_CPU_PROGRAM_MAP(at16_map)
	MCFG_CPU_IO_MAP(at16_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259_master", pic8259_device, inta_cb)
	MCFG_80286_SHUTDOWN(DEVWRITELINE("mb", at_mb_device, shutdown))

	MCFG_DEVICE_ADD("mb", AT_MB, 0)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))
	MCFG_FRAGMENT_ADD(at_softlists)

	MCFG_ISA16_SLOT_ADD("mb:isabus","isa1", pc_isa16_cards, "ega", false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa2", pc_isa16_cards, "fdc", false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa3", pc_isa16_cards, "comat", false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa4", pc_isa16_cards, "ide", false)
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_at_keyboards, STR_KBD_IBM_PC_AT_84)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1664K")
	MCFG_RAM_EXTRA_OPTIONS("2M,4M,8M,15M")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( ibm5170a, ibm5170 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(XTAL_16MHz/2)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ec1842, ibm5170 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(12000000)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ibm5162, ibm5170 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(6000000)
	MCFG_DEVICE_MODIFY("isa1")
	MCFG_DEVICE_SLOT_INTERFACE(pc_isa16_cards, "cga", false)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ibmps1, ibm5170 )
	MCFG_MACHINE_START_OVERRIDE(at_state, vrom_fix)
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(at16l_map)
	MCFG_CPU_IO_MAP(ps1_16_io)
	MCFG_DEVICE_MODIFY("isa1")
	MCFG_DEVICE_SLOT_INTERFACE(pc_isa16_cards, "vga", true)
	MCFG_DEVICE_MODIFY("kbd")
	MCFG_DEVICE_SLOT_INTERFACE(pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL, false)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( atvga, ibm5170 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(12000000)
	MCFG_DEVICE_MODIFY("isa1")
	MCFG_DEVICE_SLOT_INTERFACE(pc_isa16_cards, "svga_et4k", false)
	MCFG_DEVICE_MODIFY("kbd")
	MCFG_DEVICE_SLOT_INTERFACE(pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa5", pc_isa16_cards, nullptr, false)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( neat, atvga )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(neat_io)
	MCFG_CS8221_ADD("cs8221", "maincpu", "mb:isa", "bios")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( xb42639, atvga )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(12500000)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( k286i, ibm5162 )
	MCFG_DEVICE_MODIFY("kbd")
	MCFG_DEVICE_SLOT_INTERFACE(pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa5", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa6", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa7", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa8", pc_isa16_cards, nullptr, false)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( at386, at_state )
	MCFG_CPU_ADD("maincpu", I386, 12000000)
	MCFG_CPU_PROGRAM_MAP(at32_map)
	MCFG_CPU_IO_MAP(at32_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259_master", pic8259_device, inta_cb)

	MCFG_DEVICE_ADD("mb", AT_MB, 0)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))
	MCFG_FRAGMENT_ADD(at_softlists)
	MCFG_NVRAM_ADD_0FILL("nvram")

	// on board devices
	MCFG_ISA16_SLOT_ADD("mb:isabus","board1", pc_isa16_cards, "fdcsmc", true)
	MCFG_ISA16_SLOT_ADD("mb:isabus","board2", pc_isa16_cards, "comat", true)
	MCFG_ISA16_SLOT_ADD("mb:isabus","board3", pc_isa16_cards, "ide", true)
	MCFG_ISA16_SLOT_ADD("mb:isabus","board4", pc_isa16_cards, "lpt", true)
	// ISA cards
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa1", pc_isa16_cards, "svga_et4k", false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa2", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa3", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa4", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa5", pc_isa16_cards, nullptr, false)
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1664K")
	MCFG_RAM_EXTRA_OPTIONS("2M,4M,8M,15M,16M,32M,64M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( at386l, at386 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(at32l_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( at486, at386 )
	MCFG_CPU_REPLACE("maincpu", I486, 25000000)
	MCFG_CPU_PROGRAM_MAP(at32_map)
	MCFG_CPU_IO_MAP(at32_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259_master", pic8259_device, inta_cb)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( at486l, at486 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(at32l_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( at386sx, atvga )
	MCFG_CPU_REPLACE("maincpu", I386SX, 16000000)     /* 386SX */
	MCFG_CPU_PROGRAM_MAP(at16_map)
	MCFG_CPU_IO_MAP(at16_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259_master", pic8259_device, inta_cb)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ct386sx, at386sx )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(neat_io)
	MCFG_CS8221_ADD("cs8221", "maincpu", "mb:isa", "maincpu")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( megapc, megapc_state )
	MCFG_CPU_ADD("maincpu", I386SX, XTAL_50MHz / 2)
	MCFG_CPU_PROGRAM_MAP(megapc_map)
	MCFG_CPU_IO_MAP(megapc_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("wd7600", wd7600_device, intack_cb)

	MCFG_WD7600_ADD("wd7600",XTAL_50MHz / 2, ":maincpu", ":isa", ":bios", ":keybc")
	MCFG_WD7600_HOLD(WRITELINE(megapc_state, wd7600_hold));
	MCFG_WD7600_NMI(INPUTLINE("maincpu", INPUT_LINE_NMI));
	MCFG_WD7600_INTR(INPUTLINE("maincpu", INPUT_LINE_IRQ0));
	MCFG_WD7600_CPURESET(INPUTLINE("maincpu", INPUT_LINE_RESET));
	MCFG_WD7600_A20M(INPUTLINE("maincpu", INPUT_LINE_A20));
	// isa dma
	MCFG_WD7600_IOR(READ16(megapc_state, wd7600_ior))
	MCFG_WD7600_IOW(WRITE16(megapc_state, wd7600_iow))
	MCFG_WD7600_TC(WRITE8(megapc_state, wd7600_tc))
	// speaker
	MCFG_WD7600_SPKR(WRITELINE(megapc_state, wd7600_spkr))

	// on board devices
	MCFG_DEVICE_ADD("isabus", ISA16, 0)
	MCFG_ISA16_CPU(":maincpu")
	MCFG_ISA_BUS_IOCHCK(DEVWRITELINE("wd7600", wd7600_device, iochck_w))
	MCFG_ISA_OUT_IRQ2_CB(DEVWRITELINE("wd7600", wd7600_device, irq09_w))
	MCFG_ISA_OUT_IRQ3_CB(DEVWRITELINE("wd7600", wd7600_device, irq03_w))
	MCFG_ISA_OUT_IRQ4_CB(DEVWRITELINE("wd7600", wd7600_device, irq04_w))
	MCFG_ISA_OUT_IRQ5_CB(DEVWRITELINE("wd7600", wd7600_device, irq05_w))
	MCFG_ISA_OUT_IRQ6_CB(DEVWRITELINE("wd7600", wd7600_device, irq06_w))
	MCFG_ISA_OUT_IRQ7_CB(DEVWRITELINE("wd7600", wd7600_device, irq07_w))
	MCFG_ISA_OUT_IRQ10_CB(DEVWRITELINE("wd7600", wd7600_device, irq10_w))
	MCFG_ISA_OUT_IRQ11_CB(DEVWRITELINE("wd7600", wd7600_device, irq11_w))
	MCFG_ISA_OUT_IRQ12_CB(DEVWRITELINE("wd7600", wd7600_device, irq12_w))
	MCFG_ISA_OUT_IRQ14_CB(DEVWRITELINE("wd7600", wd7600_device, irq14_w))
	MCFG_ISA_OUT_IRQ15_CB(DEVWRITELINE("wd7600", wd7600_device, irq15_w))
	MCFG_ISA_OUT_DRQ0_CB(DEVWRITELINE("wd7600", wd7600_device, dreq0_w))
	MCFG_ISA_OUT_DRQ1_CB(DEVWRITELINE("wd7600", wd7600_device, dreq1_w))
	MCFG_ISA_OUT_DRQ2_CB(DEVWRITELINE("wd7600", wd7600_device, dreq2_w))
	MCFG_ISA_OUT_DRQ3_CB(DEVWRITELINE("wd7600", wd7600_device, dreq3_w))
	MCFG_ISA_OUT_DRQ5_CB(DEVWRITELINE("wd7600", wd7600_device, dreq5_w))
	MCFG_ISA_OUT_DRQ6_CB(DEVWRITELINE("wd7600", wd7600_device, dreq6_w))
	MCFG_ISA_OUT_DRQ7_CB(DEVWRITELINE("wd7600", wd7600_device, dreq7_w))
	MCFG_ISA16_SLOT_ADD("isabus","board1", pc_isa16_cards, "fdcsmc", true)
	MCFG_ISA16_SLOT_ADD("isabus","board2", pc_isa16_cards, "comat", true)
	MCFG_ISA16_SLOT_ADD("isabus","board3", pc_isa16_cards, "ide", true)
	MCFG_ISA16_SLOT_ADD("isabus","board4", pc_isa16_cards, "lpt", true)
	MCFG_ISA16_SLOT_ADD("isabus","board5", pc_isa16_cards, "vga", true)
	// ISA cards
	MCFG_ISA16_SLOT_ADD("isabus","isa1", pc_isa16_cards, nullptr, false)

	MCFG_DEVICE_ADD("keybc", AT_KEYBOARD_CONTROLLER, XTAL_12MHz)
	MCFG_AT_KEYBOARD_CONTROLLER_SYSTEM_RESET_CB(DEVWRITELINE("wd7600", wd7600_device, kbrst_w))
	MCFG_AT_KEYBOARD_CONTROLLER_GATE_A20_CB(DEVWRITELINE("wd7600", wd7600_device, gatea20_w))
	MCFG_AT_KEYBOARD_CONTROLLER_INPUT_BUFFER_FULL_CB(DEVWRITELINE("wd7600", wd7600_device, irq01_w))
	MCFG_AT_KEYBOARD_CONTROLLER_KEYBOARD_CLOCK_CB(DEVWRITELINE("pc_kbdc", pc_kbdc_device, clock_write_from_mb))
	MCFG_AT_KEYBOARD_CONTROLLER_KEYBOARD_DATA_CB(DEVWRITELINE("pc_kbdc", pc_kbdc_device, data_write_from_mb))
	MCFG_DEVICE_ADD("pc_kbdc", PC_KBDC, 0)
	MCFG_PC_KBDC_OUT_CLOCK_CB(DEVWRITELINE("keybc", at_keyboard_controller_device, keyboard_clock_w))
	MCFG_PC_KBDC_OUT_DATA_CB(DEVWRITELINE("keybc", at_keyboard_controller_device, keyboard_data_w))
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("1M,2M,8M,15M,16M")

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	// video hardware
	MCFG_PALETTE_ADD("palette", 256) // todo: really needed?

	/* software lists */
	MCFG_FRAGMENT_ADD( at_softlists )
	MCFG_SOFTWARE_LIST_ADD("disk_list","megapc")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( megapcpl, megapc )
	MCFG_CPU_REPLACE("maincpu", I486, 66000000 / 2)
	MCFG_CPU_PROGRAM_MAP(megapcpl_map)
	MCFG_CPU_IO_MAP(megapcpl_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("wd7600", wd7600_device, intack_cb)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( megapcpla, at_state )
	MCFG_CPU_ADD("maincpu", I486, 66000000 / 2)  // 486SLC
	MCFG_CPU_PROGRAM_MAP(at32l_map)
	MCFG_CPU_IO_MAP(at32_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259_master", pic8259_device, inta_cb)
	MCFG_MACHINE_START_OVERRIDE(at_state, vrom_fix)

	MCFG_DEVICE_ADD("mb", AT_MB, 0)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))
	MCFG_FRAGMENT_ADD(at_softlists)
	MCFG_NVRAM_ADD_0FILL("nvram")

	// on board devices
	MCFG_ISA16_SLOT_ADD("mb:isabus","board1", pc_isa16_cards, "fdcsmc", true)
	MCFG_ISA16_SLOT_ADD("mb:isabus","board2", pc_isa16_cards, "comat", true)
	MCFG_ISA16_SLOT_ADD("mb:isabus","board3", pc_isa16_cards, "ide", true)
	MCFG_ISA16_SLOT_ADD("mb:isabus","board4", pc_isa16_cards, "lpt", true)
	// ISA cards
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa1", pc_isa16_cards, "svga_dm", false)  // closest to the CL-GD5420
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa2", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa3", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa4", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa5", pc_isa16_cards, nullptr, false)
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("2M,8M,15M,16M,32M,64M,128M,256M")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","megapc")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ficpio2, at_state )
	MCFG_CPU_ADD("maincpu", I486, 25000000)
	MCFG_CPU_PROGRAM_MAP(ficpio_map)
	MCFG_CPU_IO_MAP(ficpio_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259_master", pic8259_device, inta_cb)

	MCFG_DEVICE_ADD("mb", AT_MB, 0)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))
	MCFG_FRAGMENT_ADD(at_softlists)

	MCFG_DEVICE_REMOVE("mb:rtc")
	MCFG_DS12885_ADD("mb:rtc")
	MCFG_MC146818_IRQ_HANDLER(DEVWRITELINE("pic8259_slave", pic8259_device, ir0_w)) // this is in :mb
	MCFG_MC146818_CENTURY_INDEX(0x32)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("1M,2M,8M,16M,32M,64M,128M")

	// on board devices
	MCFG_ISA16_SLOT_ADD("mb:isabus","board1", pc_isa16_cards, "fdcsmc", true)
	MCFG_ISA16_SLOT_ADD("mb:isabus","board2", pc_isa16_cards, "comat", true)
	MCFG_ISA16_SLOT_ADD("mb:isabus","board3", pc_isa16_cards, "lpt", true)

	MCFG_IDE_CONTROLLER_32_ADD("ide", ata_devices, "hdd", nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("mb:pic8259_slave", pic8259_device, ir6_w))
	MCFG_IDE_CONTROLLER_32_ADD("ide2", ata_devices, "cdrom", nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("mb:pic8259_slave", pic8259_device, ir7_w))

	MCFG_PCI_BUS_ADD("pcibus", 0)
	MCFG_PCI_BUS_DEVICE("pcibus:0", pci_devices, "vt82c505", true)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa1", pc_isa16_cards, "svga_et4k", false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa2", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa3", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa4", pc_isa16_cards, nullptr, false)
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL)

	MCFG_VT82C496_ADD("chipset")
	MCFG_VT82C496_CPU("maincpu")
	MCFG_VT82C496_REGION("isa")
MACHINE_CONFIG_END

ROM_START( ibm5170 )
	ROM_REGION(0x20000,"bios", 0)

	ROM_SYSTEM_BIOS( 0, "rev1", "IBM PC/AT 5170 01/10/84")
	ROMX_LOAD("6181028.u27", 0x10000, 0x8000, CRC(f6573f2a) SHA1(3e52cfa6a6a62b4e8576f4fe076c858c220e6c1a), ROM_SKIP(1) | ROM_BIOS(1)) /* T 6181028 8506AAA // TMM23256P-5878 // (C)IBM CORP 1981,-1984 */
	ROMX_LOAD("6181029.u47", 0x10001, 0x8000, CRC(7075fbb2) SHA1(a7b885cfd38710c9bc509da1e3ba9b543a2760be), ROM_SKIP(1) | ROM_BIOS(1)) /* T 6181029 8506AAA // TMM23256P-5879 // (C)IBM CORP 1981,-1984 */

	ROM_SYSTEM_BIOS( 1, "rev2", "IBM PC/AT 5170 06/10/85")  /* Another verifaction of these crcs would be nice */
	ROMX_LOAD("6480090.u27", 0x10000, 0x8000, CRC(99703aa9) SHA1(18022e93a0412c8477e58f8c61a87718a0b9ab0e), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("6480091.u47", 0x10001, 0x8000, CRC(013ef44b) SHA1(bfa15d2180a1902cb6d38c6eed3740f5617afd16), ROM_SKIP(1) | ROM_BIOS(2))

//  ROM_SYSTEM_BIOS( 2, "atdiag", "IBM PC/AT 5170 w/Super Diagnostics")
//  ROMX_LOAD("atdiage.bin", 0xf8000, 0x4000, CRC(e8855d0c) SHA1(c9d53e61c08da0a64f43d691bf6cadae5393843a), ROM_SKIP(1) | ROM_BIOS(3))
//  ROMX_LOAD("atdiago.bin", 0xf8001, 0x4000, CRC(606fa71d) SHA1(165e45bae7ae2da274f1e645c763c5bfcbde027b), ROM_SKIP(1) | ROM_BIOS(3))

	/* Mainboard PALS */
	ROM_REGION( 0x2000, "pals", 0 )
	ROM_LOAD("1501824_717750.mmipal14l4.u87.jed", 0x0000, 0x02E7, CRC(3c819a27) SHA1(d2f4889e628dbbef50b7f48cb1d1a313232bacc8)) /* MMI 1501824 717750 // (C)1983 IBM(M) */
	ROM_LOAD("1503135_705075.mmipal14l4.u130.jed", 0x02E7, 0x02E7, CRC(aac77198) SHA1(b318da3a1fbe5402836c1b548e231e0794d0c032)) /* MMI 1503135 705075 // (C) IBM CORP 83 */
	/* P/N 6320947 Serial/Parallel ISA expansion card PAL */
	ROM_LOAD("1503085.mmipal.u14.jed", 0x1000, 0x0800, NO_DUMP) /* MMI 1503085 8449 // (C) IBM CORP 83 */ /* Not sure of type */

	/* Mainboard PROMS */
	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD("1501814.82s123an.u115", 0x0000, 0x0020, CRC(849c9217) SHA1(2955ae1705c3b59170f1373f99b3ea5c174c4544)) /* N82S123AN 8713 // SK-D 1501814 */
	ROM_LOAD("55x8041.82s147an.u72", 0x0020, 0x0200, CRC(f2cc4fe6) SHA1(e285468516bd05083155a8a272583deef655315a)) /* S N82S147AN 8709 // V-C55X8041 */
ROM_END

ROM_START( ec1842 )
	ROM_REGION16_LE(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "4202004.bin", 0x1c001, 0x2000, CRC(33fb5382) SHA1(35eb62328324d93e7a06f2f9d1ad0002f83fc99b))
	ROM_LOAD16_BYTE( "4202005.bin", 0x1c000, 0x2000, CRC(8e05c119) SHA1(9d81613b4fc305c14ae9fda0b1dd97a290715530))
	ROM_LOAD16_BYTE( "4202006.bin", 0x18001, 0x2000, CRC(6da537ef) SHA1(f79feb433dcf41f5cdef52b845e3550d5f0fb5c0))
	ROM_LOAD16_BYTE( "4202007.bin", 0x18000, 0x2000, CRC(d6ee0e95) SHA1(6fd4c42190e879501198fede70ae43bc420681d0))
	// EGA ROM
	//ROM_LOAD16_BYTE( "4200009.bin", 0xc0000, 0x2000, CRC(9deeb39f) SHA1(255b859d3ea05891aa65a4a742ecaba744dfc923))
	//ROM_LOAD16_BYTE( "4200010.bin", 0xc0001, 0x2000, CRC(f2c38d93) SHA1(dcb3741d06089bf1a80cb766a6b94029ad698d73))
ROM_END

ROM_START( ec1849 )
	ROM_REGION16_LE(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "cpu-card_27c256_015.rom", 0x10000, 0x8000, CRC(68eadf0a) SHA1(903a7f1c3ebc6b27c31b512b2908c483608b5c13))
	ROM_LOAD16_BYTE( "cpu-card_27c256_016.rom", 0x10001, 0x8000, CRC(bc3924d6) SHA1(596be415e6c2bc4ff30a187f146664531565712c))
	//ROM_LOAD16_BYTE( "video-card_573rf6( 2764)_040.rom", 0xc0001, 0x2000, CRC(a3ece315) SHA1(e800e11c3b1b6fcaf41bfb7d4058a9d34fdd2b3f))
	//ROM_LOAD16_BYTE( "video-card_573rf6( 2764)_041.rom", 0xc0000, 0x2000, CRC(b0a2ba7f) SHA1(c8160e8bc97cd391558f1dddd3fd3ec4a19d030c))
ROM_END

ROM_START( ibm5170a )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS( 0, "rev3", "IBM PC/AT 5170 11/15/85")
	ROMX_LOAD("61x9266.u27", 0x10000, 0x8000, CRC(4995be7a) SHA1(8e8e5c863ae3b8c55fd394e345d8cca48b6e575c), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("61x9265.u47", 0x10001, 0x8000, CRC(c32713e4) SHA1(22ed4e2be9f948682891e2fd056a97dbea01203c), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_SYSTEM_BIOS( 1, "3270at", "IBM 3270 PC/AT 5281 11/15/85") /* pretty much just a part string and checksum change from the 5170 rev3 */
	ROMX_LOAD("62x0820.u27", 0x10000, 0x8000, CRC(e9cc3761) SHA1(ff9373c1a1f34a32fb6acdabc189c61b01acf9aa), ROM_SKIP(1) | ROM_BIOS(2)) /* T 62X0820-U27 8714HAK // TMM23256P-6746 // (C)IBM CORP 1981,-1985 */
	ROMX_LOAD("62x0821.u47", 0x10001, 0x8000, CRC(b5978ccb) SHA1(2a1aeb9ae3cd7e60fc4c383ca026208b82156810), ROM_SKIP(1) | ROM_BIOS(2)) /* T 62X0821-U47 8715HAK // TMM23256P-6747 // (C)IBM CORP 1981,-1985 */

	/* Mainboard PALS */
	ROM_REGION( 0x2000, "pals", 0 )
	ROM_LOAD("1501824_717750.mmipal14l4.u87.jed", 0x0000, 0x02E7, CRC(3c819a27) SHA1(d2f4889e628dbbef50b7f48cb1d1a313232bacc8)) /* MMI 1501824 717750 // (C)1983 IBM(M) */
	ROM_LOAD("1503135_705075.mmipal14l4.u130.jed", 0x02E7, 0x02E7, CRC(aac77198) SHA1(b318da3a1fbe5402836c1b548e231e0794d0c032)) /* MMI 1503135 705075 // (C) IBM CORP 83 */    /* P/N 6320947 Serial/Parallel ISA expansion card PAL */
	ROM_LOAD("1503085.mmipal.u14.jed", 0x1000, 0x0800, NO_DUMP) /* MMI 1503085 8449 // (C) IBM CORP 83 */ /* Not sure of type */

	/* Mainboard PROMS */
	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD("1501814.82s123an.u115", 0x0000, 0x0020, CRC(849c9217) SHA1(2955ae1705c3b59170f1373f99b3ea5c174c4544)) /* N82S123AN 8713 // SK-D 1501814 */
	ROM_LOAD("55x8041.82s147an.u72", 0x0020, 0x0200, CRC(f2cc4fe6) SHA1(e285468516bd05083155a8a272583deef655315a)) /* S N82S147AN 8709 // V-C55X8041 */
ROM_END


ROM_START( ibm5162 ) //MB p/n 62x1168
	ROM_REGION16_LE(0x20000,"bios", 0)

	ROM_LOAD16_BYTE("78x7460.u34", 0x10000, 0x8000, CRC(1db4bd8f) SHA1(7be669fbb998d8b4626fefa7cd1208d3b2a88c31)) /* 78X7460 U34 // (C) IBM CORP // 1981-1986 */
	ROM_LOAD16_BYTE("78x7461.u35", 0x10001, 0x8000, CRC(be14b453) SHA1(ec7c10087dbd53f9c6d1174e8f14212e2aec1818)) /* 78X7461 U35 // (C) IBM CORP // 1981-1986 */

	/* Mainboard PALS */
	ROM_REGION( 0x2000, "pals", 0 )
	ROM_LOAD("59x7599.mmipal20l8.u27.jed", 0x0000, 0x02E7, NO_DUMP) /* MMI PAL20L8ACN5 8631 // N59X7599 IBM (C)85 K3 */
	ROM_LOAD("1503135.mmipal14l4.u81.jed", 0x02E7, 0x02E7, CRC(aac77198) SHA1(b318da3a1fbe5402836c1b548e231e0794d0c032)) /* MMI 1503135 8625 // (C) IBM CORP 83 */
	/* P/N 6320947 Serial/Parallel ISA expansion card PAL */
	ROM_LOAD("1503085.mmipal.u14.jed", 0x1000, 0x0800, NO_DUMP) /* MMI 1503085 8449 // (C) IBM CORP 83 */ /* Not sure of type */

	/* Mainboard PROMS */
	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD("1501814.82s123an.u72", 0x0000, 0x0020, CRC(849c9217) SHA1(2955ae1705c3b59170f1373f99b3ea5c174c4544)) /* N82S123AN 8623 // SK-U 1501814 */
	ROM_LOAD("59x7594.82s147an.u90", 0x0020, 0x0200, NO_DUMP) /* S N82S147AN 8629 // VCT 59X7594 */
ROM_END

ROM_START( ibmps1es )
	ROM_REGION(0x40000, "bios", 0)
	ROM_LOAD16_BYTE( "ibm_1057757_24-05-90.bin", 0x00000, 0x20000, CRC(c8f81ea4) SHA1(925ed0e98f9f2997cb86554ef384bcfaf2a4ecbe))
	ROM_LOAD16_BYTE( "ibm_1057757_29-15-90.bin", 0x00001, 0x20000, CRC(c2dd6b5c) SHA1(f6b5785002dd628b6b1fb3bb101e076299eba3b6))
ROM_END

ROM_START( at )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "ami211", "AMI 21.1") /*(Motherboard Manufacturer: Dataexpert Corp. Motherboard) (Neat 286 Bios, 82c21x Chipset ) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "ami211.bin",     0x10000, 0x10000,CRC(a0b5d269) SHA1(44db8227d35a09e39b93ed944f85dcddb0dd0d39), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "at", "PC 286") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 03-11-1987)*/
	ROMX_LOAD("at110387.1", 0x10001, 0x8000, CRC(679296a7) SHA1(ae891314cac614dfece686d8e1d74f4763cf40e3),ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD("at110387.0", 0x10000, 0x8000, CRC(65ae1f97) SHA1(91a29c7deecf7a9afbba330e64e0eee9aafee4d1),ROM_SKIP(1) | ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(2, "ami206", "AMI C 206.1")  /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 15-10-1990)*/
	ROMX_LOAD( "amic206.bin",    0x10000, 0x10000,CRC(25a67c34) SHA1(91e9d8cdc2f1b40a601a23ceaff2189fd1245f3b), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(3, "amic21", "AMI C 21.1") /* bad dump, checksum off by 8 in the lsb*/
	ROMX_LOAD( "amic21-2.bin",  0x10001, 0x8000, CRC(8ffe7752) SHA1(68215f07a170ee7bdcb3e52b370d470af1741f7e),ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "amic21-1.bin",  0x10000, 0x8000, CRC(5644ed38) SHA1(963555ec77845defc3b42b433280908e1797076e),ROM_SKIP(1) | ROM_BIOS(4) )
	ROM_SYSTEM_BIOS(4, "ami101", "AMI HT 101.1") /* Quadtel Enhanced 286 Bios Version 3.04.02 */
	ROMX_LOAD( "amiht-h.bin",   0x10001, 0x8000, CRC(8022545f) SHA1(42541d4392ad00b0e064b3a8ccf2786d875c7c19),ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "amiht-l.bin",   0x10000, 0x8000, CRC(285f6b8f) SHA1(2fce4ec53b68c9a7580858e16c926dc907820872),ROM_SKIP(1) | ROM_BIOS(5) )
	ROM_SYSTEM_BIOS(5, "ami121", "AMI HT 12.1")
	ROMX_LOAD( "ami2od86.bin",  0x10001, 0x8000, CRC(04a2cec4) SHA1(564d37a8b2c0f4d0e23cd1e280a09d47c9945da8),ROM_SKIP(1) | ROM_BIOS(6) )
	ROMX_LOAD( "ami2ev86.bin",  0x10000, 0x8000, CRC(55deb5c2) SHA1(19ce1a7cc985b5895c585e39211475de2e3b0dd1),ROM_SKIP(1) | ROM_BIOS(6) )
	ROM_SYSTEM_BIOS(6, "ami122", "AMI HT 12.2")
	ROMX_LOAD( "ami2od89.bin",  0x10001, 0x8000, CRC(7c81bbe8) SHA1(a2c7eca586f6e2e76b9101191e080a1f1cb8b833),ROM_SKIP(1) | ROM_BIOS(7) )
	ROMX_LOAD( "ami2ev89.bin",  0x10000, 0x8000, CRC(705d36e0) SHA1(0c9cfb71ced4587f109b9b6dfc2a9c92302fdb99),ROM_SKIP(1) | ROM_BIOS(7) )
	ROM_SYSTEM_BIOS(7, "ami123", "AMI HT 12.3") /*(Motherboard Manufacturer: Aquarius Systems USA Inc.) (BIOS release date:: 13-06-1990)*/
	ROMX_LOAD( "ht12h.bin",     0x10001, 0x8000, CRC(db8b471e) SHA1(7b5fa1c131061fa7719247db3e282f6d30226778),ROM_SKIP(1) | ROM_BIOS(8) )
	ROMX_LOAD( "ht12l.bin",     0x10000, 0x8000, CRC(74fd178a) SHA1(97c8283e574abbed962b701f3e8091fb82823b80),ROM_SKIP(1) | ROM_BIOS(8) )
	ROM_SYSTEM_BIOS(8, "ami181", "AMI HT 18.1") /* not a bad dump, sets unknown probably chipset related registers at 0x1e8 before failing post */
	ROMX_LOAD( "ht18.bin",     0x10000, 0x10000, CRC(f65a6f9a) SHA1(7dfdf7d243f9f645165dc009c5097dd515f86fbb), ROM_BIOS(9) )
	ROM_SYSTEM_BIOS(9, "amiht21", "AMI HT 21.1") /* as above */
	ROMX_LOAD( "ht21e.bin",    0x10000, 0x10000, CRC(e80f7fed) SHA1(62d958d98c95e9e4d1b290a6c1054ae98770f276), ROM_BIOS(10) )
	ROM_SYSTEM_BIOS(10, "amip1", "AMI P.1") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "poisk-h.bin",   0x10001, 0x8000, CRC(83fd3f8c) SHA1(ca94850bbd949b97b11710629886b0ee69489a81),ROM_SKIP(1) | ROM_BIOS(11) )
	ROMX_LOAD( "poisk-l.bin",   0x10000, 0x8000, CRC(0b2ed291) SHA1(bb51a3f317cf4d429a6cfb44a46ca0ac39d9aaa7),ROM_SKIP(1) | ROM_BIOS(11) )
	ROM_SYSTEM_BIOS(11, "aw201", "Award 201")
	ROMX_LOAD( "83201-5h.bin",  0x10001, 0x8000, CRC(968d1fc0) SHA1(dc4122a6c696f0b43e7894dc1b669346eed755d5),ROM_SKIP(1) | ROM_BIOS(12) )
	ROMX_LOAD( "83201-5l.bin",  0x10000, 0x8000, CRC(bf50a89a) SHA1(2349a1db6017a7fb0673e99d3680c8753407be8d),ROM_SKIP(1) | ROM_BIOS(12) )
	ROM_SYSTEM_BIOS(12, "aw303", "Award 303 NFS")
	ROMX_LOAD( "aw303-hi.bin",  0x18001, 0x4000, CRC(78f32d7e) SHA1(1c88398fb171b33b7e6191bad63704ae85bfed8b), ROM_SKIP(1) | ROM_BIOS(13) )
	ROMX_LOAD( "aw303-lo.bin",  0x18000, 0x4000, CRC(3d2a70c0) SHA1(1329113bec514ed2a6d803067b1132744ef534dd), ROM_SKIP(1) | ROM_BIOS(13) )
	ROM_SYSTEM_BIOS(13, "aw303gs", "Award 303GS")
	ROMX_LOAD( "aw303gs-hi.bin",  0x18001, 0x4000, CRC(82392e18) SHA1(042453b7b29933a1b72301d21fcf8fa6b293c9c9), ROM_SKIP(1) | ROM_BIOS(14) )
	ROMX_LOAD( "aw303gs-lo.bin",  0x18000, 0x4000, CRC(a4cf8ba1) SHA1(b73e34be3b2754aaed1ac06471f4441fea06c67c), ROM_SKIP(1) | ROM_BIOS(14) )
ROM_END

ROM_START( cmdpc30 )
	ROM_REGION(0x20000,"bios", 0)
	ROMX_LOAD( "commodore pc 30 iii even.bin", 0x18000, 0x4000, CRC(36307aa9) SHA1(50237ffea703b867de426ab9ebc2af46bac1d0e1),ROM_SKIP(1))
	ROMX_LOAD( "commodore pc 30 iii odd.bin",  0x18001, 0x4000, CRC(41bae42d) SHA1(27d6ad9554be86359d44331f25591e3122a31519),ROM_SKIP(1))
ROM_END

ROM_START( atvga )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "vl82c", "VL82C311L-FC4")/*(Motherboard Manufacturer: Biostar Microtech Corp.) (BIOS release date: 05-05-1991)*/
	ROMX_LOAD( "2vlm001.bin",     0x10000, 0x10000, CRC(f34d800a) SHA1(638aca592a0e525f957beb525e95ca666a994ee8), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "ami211", "AMI 21.1") /*(Motherboard Manufacturer: Dataexpert Corp. Motherboard) (Neat 286 Bios, 82c21x Chipset ) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "ami211.bin",     0x10000, 0x10000,CRC(a0b5d269) SHA1(44db8227d35a09e39b93ed944f85dcddb0dd0d39), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "ami206", "AMI C 206.1") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 15-10-1990)*/
	ROMX_LOAD( "amic206.bin",    0x10000, 0x10000,CRC(25a67c34) SHA1(91e9d8cdc2f1b40a601a23ceaff2189fd1245f3b), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(3, "amic21", "AMI C 21.1") /* bad dump, checksum off by 8 in the lsb*/
	ROMX_LOAD( "amic21-2.bin",  0x10001, 0x8000, CRC(8ffe7752) SHA1(68215f07a170ee7bdcb3e52b370d470af1741f7e),ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "amic21-1.bin",  0x10000, 0x8000, CRC(5644ed38) SHA1(963555ec77845defc3b42b433280908e1797076e),ROM_SKIP(1) | ROM_BIOS(4) )
	ROM_SYSTEM_BIOS(4, "ami101", "AMI HT 101.1") /* Quadtel Enhanced 286 Bios Version 3.04.02 */
	ROMX_LOAD( "amiht-h.bin",   0x10001, 0x8000, CRC(8022545f) SHA1(42541d4392ad00b0e064b3a8ccf2786d875c7c19),ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "amiht-l.bin",   0x10000, 0x8000, CRC(285f6b8f) SHA1(2fce4ec53b68c9a7580858e16c926dc907820872),ROM_SKIP(1) | ROM_BIOS(5) )
	ROM_SYSTEM_BIOS(5, "ami121", "AMI HT 12.1")
	ROMX_LOAD( "ami2od86.bin",  0x10001, 0x8000, CRC(04a2cec4) SHA1(564d37a8b2c0f4d0e23cd1e280a09d47c9945da8),ROM_SKIP(1) | ROM_BIOS(6) )
	ROMX_LOAD( "ami2ev86.bin",  0x10000, 0x8000, CRC(55deb5c2) SHA1(19ce1a7cc985b5895c585e39211475de2e3b0dd1),ROM_SKIP(1) | ROM_BIOS(6) )
	ROM_SYSTEM_BIOS(6, "ami122", "AMI HT 12.2")
	ROMX_LOAD( "ami2od89.bin",  0x10001, 0x8000, CRC(7c81bbe8) SHA1(a2c7eca586f6e2e76b9101191e080a1f1cb8b833),ROM_SKIP(1) | ROM_BIOS(7) )
	ROMX_LOAD( "ami2ev89.bin",  0x10000, 0x8000, CRC(705d36e0) SHA1(0c9cfb71ced4587f109b9b6dfc2a9c92302fdb99),ROM_SKIP(1) | ROM_BIOS(7) )
	ROM_SYSTEM_BIOS(7, "ami123", "AMI HT 12.3") /*(Motherboard Manufacturer: Aquarius Systems USA Inc.) (BIOS release date:: 13-06-1990)*/
	ROMX_LOAD( "ht12h.bin",     0x10001, 0x8000, CRC(db8b471e) SHA1(7b5fa1c131061fa7719247db3e282f6d30226778),ROM_SKIP(1) | ROM_BIOS(8) )
	ROMX_LOAD( "ht12l.bin",     0x10000, 0x8000, CRC(74fd178a) SHA1(97c8283e574abbed962b701f3e8091fb82823b80),ROM_SKIP(1) | ROM_BIOS(8) )
	ROM_SYSTEM_BIOS(8, "ami181", "AMI HT 18.1") /* not a bad dump, sets unknown probably chipset related registers at 0x1e8 before failing post */
	ROMX_LOAD( "ht18.bin",     0x10000, 0x10000, CRC(f65a6f9a) SHA1(7dfdf7d243f9f645165dc009c5097dd515f86fbb), ROM_BIOS(9) )
	ROM_SYSTEM_BIOS(9, "amiht21", "AMI HT 21.1") /* as above */
	ROMX_LOAD( "ht21e.bin",    0x10000, 0x10000, CRC(e80f7fed) SHA1(62d958d98c95e9e4d1b290a6c1054ae98770f276), ROM_BIOS(10) )
	ROM_SYSTEM_BIOS(10, "amip1", "AMI P.1") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "poisk-h.bin",   0x10001, 0x8000, CRC(83fd3f8c) SHA1(ca94850bbd949b97b11710629886b0ee69489a81),ROM_SKIP(1) | ROM_BIOS(11) )
	ROMX_LOAD( "poisk-l.bin",   0x10000, 0x8000, CRC(0b2ed291) SHA1(bb51a3f317cf4d429a6cfb44a46ca0ac39d9aaa7),ROM_SKIP(1) | ROM_BIOS(11) )
	ROM_SYSTEM_BIOS(11, "ami1131", "AMI-1131") /*(Motherboard Manufacturer: Elitegroup Computer Co., Ltd.) (BIOS release date:: 09-04-1990)*/
	ROMX_LOAD( "2hlm003h.bin",   0x10001, 0x8000, CRC(2babb42b) SHA1(3da6538f44b434cdec0cbdddd392ccfd34666f06),ROM_SKIP(1) | ROM_BIOS(12) )
	ROMX_LOAD( "2hlm003l.bin",   0x10000, 0x8000, CRC(317cbcbf) SHA1(1adad6280d8b07c2921fc5fc13ecaa10e6bfebdc),ROM_SKIP(1) | ROM_BIOS(12) )
	ROM_SYSTEM_BIOS(12, "at", "PC 286") /*(Motherboard Manufacturer: Unknown.) (BIOS release date:: 03-11-1987)*/
	ROMX_LOAD("at110387.1", 0x10001, 0x8000, CRC(679296a7) SHA1(ae891314cac614dfece686d8e1d74f4763cf40e3),ROM_SKIP(1) | ROM_BIOS(13) )
	ROMX_LOAD("at110387.0", 0x10000, 0x8000, CRC(65ae1f97) SHA1(91a29c7deecf7a9afbba330e64e0eee9aafee4d1),ROM_SKIP(1) | ROM_BIOS(13) )
ROM_END

ROM_START( xb42639 )
	/* actual VGA BIOS not dumped*/
	ROM_REGION(0x20000, "bios", 0)
	// XEN-S (Venus I Motherboard)
	ROM_LOAD16_BYTE("3-10-17i.lo", 0x10000, 0x8000, CRC(3786ca1e) SHA1(c682d7c76f234559d03bcf21010c13c4dbeafb69))
	ROM_LOAD16_BYTE("3-10-17i.hi", 0x10001, 0x8000, CRC(d66710eb) SHA1(e8c1cd5f9ecfbd8825655e416d7ddf2ae362e69b))
ROM_END

ROM_START( xb42639a )
	/* actual VGA BIOS not dumped*/
	ROM_REGION(0x20000, "bios", 0)
	// XEN-S (Venus II Motherboard)
	ROM_LOAD16_BYTE("10217.lo", 0x10000, 0x8000, CRC(ea53406f) SHA1(2958dfdbda14de4e6b9d6a8c3781131ab1e32bef))
	ROM_LOAD16_BYTE("10217.hi", 0x10001, 0x8000, CRC(111725cf) SHA1(f6018a45bda4476d40c5881fb0a506ff75ec1688))
ROM_END

ROM_START( xb42664 )
	/* actual VGA BIOS not dumped */
	ROM_REGION(0x20000, "bios", 0)
	// XEN-S (Venus I Motherboard)
	ROM_LOAD16_BYTE("3-10-17i.lo", 0x10000, 0x8000, CRC(3786ca1e) SHA1(c682d7c76f234559d03bcf21010c13c4dbeafb69))
	ROM_LOAD16_BYTE("3-10-17i.hi", 0x10001, 0x8000, CRC(d66710eb) SHA1(e8c1cd5f9ecfbd8825655e416d7ddf2ae362e69b))
ROM_END

ROM_START( xb42664a )
	/* actual VGA BIOS not dumped*/
	ROM_REGION(0x20000, "bios", 0)
	// XEN-S (Venus II Motherboard)
	ROM_LOAD16_BYTE("10217.lo", 0x10000, 0x8000, CRC(ea53406f) SHA1(2958dfdbda14de4e6b9d6a8c3781131ab1e32bef))
	ROM_LOAD16_BYTE("10217.hi", 0x10001, 0x8000, CRC(111725cf) SHA1(f6018a45bda4476d40c5881fb0a506ff75ec1688))
ROM_END


ROM_START( neat )
	ROM_REGION(0x20000,"bios", 0)
	//ROM_SYSTEM_BIOS(0, "neat286", "NEAT 286")
	ROM_LOAD16_BYTE("at030389.0", 0x10000, 0x8000, CRC(4c36e61d) SHA1(094e8d5e6819889163cb22a2cf559186de782582))
	ROM_LOAD16_BYTE("at030389.1", 0x10001, 0x8000, CRC(4e90f294) SHA1(18c21fd8d7e959e2292a9afbbaf78310f9cad12f))
ROM_END

ROM_START( ct386sx )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "neatsx", "NEATsx 386sx")
	ROMX_LOAD("012l-u25.bin", 0x10000, 0x8000, CRC(4ab1862d) SHA1(d4e8d0ff43731270478ca7671a129080ff350a4f),ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD("012h-u24.bin", 0x10001, 0x8000, CRC(17472521) SHA1(7588c148fe53d9dc4cb2d0ab6e0fd51a39bb5d1a),ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_FILL(0x1e2c9, 1, 0x00) // skip incompatible keyboard controller test
	ROM_FILL(0x1e2cb, 1, 0xbb) // fix checksum
ROM_END

ROM_START( at386 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "ami386", "AMI 386")
	ROMX_LOAD("ami386.bin",  0x10000, 0x10000, CRC(3a807d7f) SHA1(8289ba36a3dfc3324333b1a834bc6b0402b546f0), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "at386", "unknown 386")  // This dump possibly comes from a MITAC INC 386 board, given that the original driver had it as manufacturer
	ROMX_LOAD("at386.bin",  0x10000, 0x10000, CRC(3df9732a) SHA1(def71567dee373dc67063f204ef44ffab9453ead), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "amicg", "AMI CG")
	ROMX_LOAD( "amicg.1",        0x10000, 0x10000,CRC(8408965a) SHA1(9893d3ac851e01b06a68a67d3721df36ca2c96f5), ROM_BIOS(3) )
ROM_END


ROM_START( at486 )
	ROM_REGION(0x20000, "bios", 0)

	ROM_SYSTEM_BIOS(0, "at486", "PC/AT 486")
	ROMX_LOAD("at486.bin",   0x10000, 0x10000, CRC(31214616) SHA1(51b41fa44d92151025fc9ad06e518e906935e689), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "mg48602", "UMC MG-48602")
	ROMX_LOAD("mg48602.bin", 0x10000, 0x10000, CRC(45797823) SHA1(a5fab258aecabde615e1e97af5911d6cf9938c11), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "ft01232", "Free Tech 01-232")
	ROMX_LOAD("ft01232.bin", 0x10000, 0x10000, CRC(30efaf92) SHA1(665c8ef05ca052dcc06bb473c9539546bfef1e86), ROM_BIOS(3))

	/* 486 boards from FIC

	naming convention
	xxxxx101 --> for EPROM
	xxxxx701 --> for EEPROM using a Flash Utility v5.02
	xxxxBxxx --> NS 311/312 IO Core Logic
	xxxxCxxx --> NS 332 IO Core Logic
	xxxxGxxx --> Winbond W83787F IO Core Logic
	xxxxJxxx --> Winbond W83877F IO Core Logic

	*/

	/* this is the year 2000 beta bios from FIC, supports GIO-VT, GAC-V, GAC-2, VIP-IO, VIO-VP and GVT-2 */
	ROM_SYSTEM_BIOS(3, "ficy2k", "FIC 486 3.276GN1") /* 1997-06-16, includes CL-GD5429 VGA BIOS 1.00a */
	ROMX_LOAD("3276gn1.bin",  0x00000, 0x20000, CRC(d4ff0cc4) SHA1(567b6bdbc9bff306c8c955f275e01ae4c45fd5f2), ROM_BIOS(4))

	ROM_SYSTEM_BIOS(4, "ficgac2", "FIC 486-GAC-2") /* 1994-04-29, includes CL-GD542X VGA BIOS 1.50 */
	ROMX_LOAD("att409be.bin", 0x00000, 0x20000, CRC(c58e017b) SHA1(14c19e720ce62eb2afe28a70f4e4ebafab0f9e77), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(5, "ficgacv", "FIC 486-GAC-V 3.27GN1") /* 1996-04-08, includes CL-GD542X VGA BIOS 1.41 */
	ROMX_LOAD("327gn1.awd",   0x00000, 0x20000, CRC(017614d4) SHA1(2228c28f21a7e78033d24319449297936465b164), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(6, "ficgiovp", "FIC 486-GIO-VP 3.15GN") /* 1994-05-06 */
	ROMX_LOAD("giovp315.rom", 0x10000, 0x10000, CRC(e102c3f5) SHA1(f15a7e9311cc17afe86da0b369607768b030ddec), ROM_BIOS(7))
	ROM_SYSTEM_BIOS(7, "ficgiovt", "FIC 486-GIO-VT 3.06G") /* 1994-11-20 */
	ROMX_LOAD("306gcd00.awd", 0x10000, 0x10000, CRC(75f3ded4) SHA1(999d4b58204e0b0f33262d0613c855b528bf9597), ROM_BIOS(8))

	ROM_SYSTEM_BIOS(8, "ficgiovt2_326", "FIC 486-GIO-VT2 3.26G")  /* 1994-07-06 */
	ROMX_LOAD("326g1c00.awd", 0x10000, 0x10000, CRC(2e729ab5) SHA1(b713f97fa0e0b62856dab917f417f5b21020b354), ROM_BIOS(9))
	ROM_SYSTEM_BIOS(9, "ficgiovt2_3276", "FIC 486-GIO-VT2 3.276") /* 1997-07-17 */
	ROMX_LOAD("32760000.bin", 0x10000, 0x10000, CRC(ad179128) SHA1(595f67ba4a1c8eb5e118d75bf657fff3803dcf4f), ROM_BIOS(10))

	ROM_SYSTEM_BIOS(10, "ficgvt2", "FIC 486-GVT-2 3.07G") /* 1994-11-02 */
	ROMX_LOAD("3073.bin",     0x10000, 0x10000, CRC(a6723863) SHA1(ee93a2f1ec84a3d67e267d0a490029f9165f1533), ROM_BIOS(11))
	ROM_SYSTEM_BIOS(11, "ficgpak2", "FIC 486-PAK-2 5.15S") /* 1995-06-27, includes Phoenix S3 TRIO64 Enhanced VGA BIOS 1.4-01 */
	ROMX_LOAD("515sbd8a.awd", 0x00000, 0x20000, CRC(778247e1) SHA1(07d8f0f2464abf507be1e8dfa06cd88737782411), ROM_BIOS(12))

	ROM_SYSTEM_BIOS(12, "ficpio3g7", "FIC 486-PIO-3 1.15G705") /* pnp */
	ROMX_LOAD("115g705.awd",  0x00000, 0x20000, CRC(ddb1544a) SHA1(d165c9ecdc9397789abddfe0fef69fdf954fa41b), ROM_BIOS(13))
	ROM_SYSTEM_BIOS(13, "ficpio3g1", "FIC 486-PIO-3 1.15G105") /* non-pnp */
	ROMX_LOAD("115g105.awd",  0x00000, 0x20000, CRC(b327eb83) SHA1(9e1ff53e07ca035d8d43951bac345fec7131678d), ROM_BIOS(14))

	ROM_SYSTEM_BIOS(14, "ficpos", "FIC 486-POS")
	ROMX_LOAD("116di6b7.bin", 0x00000, 0x20000, CRC(d1d84616) SHA1(2f2b27ce100cf784260d8e155b48db8cfbc63285), ROM_BIOS(15))
	ROM_SYSTEM_BIOS(15, "ficpvt", "FIC 486-PVT 5.15")          /* 1995-06-27 */
	ROMX_LOAD("5150eef3.awd", 0x00000, 0x20000, CRC(eb35785d) SHA1(1e601bc8da73f22f11effe9cdf5a84d52576142b), ROM_BIOS(16))
	ROM_SYSTEM_BIOS(16, "ficpvtio", "FIC 486-PVT-IO 5.162W2")  /* 1995-10-05 */
	ROMX_LOAD("5162cf37.awd", 0x00000, 0x20000, CRC(378d813d) SHA1(aa674eff5b972b31924941534c3c988f6f78dc93), ROM_BIOS(17))
	ROM_SYSTEM_BIOS(17, "ficvipio426", "FIC 486-VIP-IO 4.26GN2") /* 1994-12-07 */
	ROMX_LOAD("426gn2.awd",   0x00000, 0x20000, CRC(5f472aa9) SHA1(9160abefae32b450e973651c052657b4becc72ba), ROM_BIOS(18))
	ROM_SYSTEM_BIOS(18, "ficvipio427", "FIC 486-VIP-IO 4.27GN2A") /* 1996-02-14 */
	ROMX_LOAD("427gn2a.awd",  0x00000, 0x20000, CRC(035ad56d) SHA1(0086db3eff711fc710b30e7f422fc5b4ab8d47aa), ROM_BIOS(19))
	ROM_SYSTEM_BIOS(19, "ficvipio2", "FIC 486-VIP-IO2")
	ROMX_LOAD("1164g701.awd", 0x00000, 0x20000, CRC(7b762683) SHA1(84debce7239c8b1978246688ae538f7c4f519d13), ROM_BIOS(20))

	ROM_SYSTEM_BIOS(20, "qdi", "QDI PX486DX33/50P3")
	ROMX_LOAD("qdi_px486.u23", 0x10000, 0x10000, CRC(c80ecfb6) SHA1(34cc9ef68ff719cd0771297bf184efa83a805f3e), ROM_BIOS(21))
ROM_END


// FIC 486-PIO-2 (4 ISA, 4 PCI)
// VIA VT82C505 (ISA/VL to PCI bridge) + VT82C496G (system chipset) + VT82C406MV (keyboard controller, RTC, CMOS), NS311/312 or NS332 I/O
ROM_START( ficpio2 )
	ROM_REGION(0x40000, "isa", 0)
	ROM_SYSTEM_BIOS(0, "ficpio2c7", "FIC 486-PIO-2 1.15C701") /* pnp, i/o core: NS 332, doesn't boot, requires cache emulation? */
	ROMX_LOAD("115c701.awd",  0x020000, 0x20000, CRC(b0dd7975) SHA1(bfde13b0fbd141bc945d37d92faca9f4f59b716d), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "ficpio2b7", "FIC 486-PIO-2 1.15B701") /* pnp, i/o core: NS 311/312, doesn't boot, requires cache emulation? */
	ROMX_LOAD("115b701.awd",  0x020000, 0x20000, CRC(ac24abad) SHA1(01174d84ed32fb1d95cd632d09f773acb8666c83), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "ficpio2c1", "FIC 486-PIO-2 1.15C101") /* non-pnp, i/o core: NS 332, working  */
	ROMX_LOAD("115c101.awd",  0x020000, 0x20000, CRC(5fadde88) SHA1(eff79692c1ecf34b6ea3f02409d14ce1f5c51bf9), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "ficpio2b1", "FIC 486-PIO-2 1.15B101") /* non-pnp, i/o core: NS 311/312, working  */
	ROMX_LOAD("115b101.awd",  0x020000, 0x20000, CRC(ff69617d) SHA1(ecbfc7315dcf6bd3e5b59e3ae9258759f64fe7a0), ROM_BIOS(4))
ROM_END

ROM_START( c386sx16 )
	ROM_REGION(0x20000,"bios", 0)
	/* actual VGA BIOS not dumped - uses a WD Paradise according to http://www.cbmhardware.de/pc/pc.php */

	/* Commodore 80386SX BIOS Rev. 1.03 */
	/* Copyright (C) 1985-1990 Commodore Electronics Ltd. */
	/* Copyright (C) 1985-1990 Phoenix Technologies Ltd. */
	ROM_LOAD16_BYTE( "390914-01.u39", 0x10000, 0x8000, CRC(8f849198) SHA1(550b04bac0d0807d6e95ec25391a81272779b41b)) /* 390914-01 V1.03 CS-2100 U39 Copyright (C) 1990 CBM */
	ROM_LOAD16_BYTE( "390915-01.u38", 0x10001, 0x8000, CRC(ee4bad92) SHA1(6e02ef97a7ce336485814c06a1693bc099ce5cfb)) /* 390915-01 V1.03 CS-2100 U38 Copyright (C) 1990 CBM */
ROM_END

ROM_START( xb42663 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "qi310223.lo", 0x00000, 0x10000, CRC(53047f49) SHA1(7b38e533f7f27295269549c63e5477d950239167))
	ROM_LOAD16_BYTE( "qi310223.hi", 0x00001, 0x10000, CRC(4852869f) SHA1(98599d4691d40b3fac2936034c70b386ce4caf77))
ROM_END

ROM_START( qi600 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "qi610223.lo", 0x00000, 0x10000, CRC(563114a9) SHA1(62932b3bf0b5502ff708f604c21773f00afda58e))
	ROM_LOAD16_BYTE( "qi610223.hi", 0x00001, 0x10000, CRC(0ae133f6) SHA1(6039c366f7fe0ebf60b34c1a7d6b2d781b664001))
ROM_END

ROM_START( qi900 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "qi910224.lo", 0x00000, 0x10000, CRC(b012ad3c) SHA1(807e788a6bd03f5e983fe503af3d0b202c754b8a))
	ROM_LOAD16_BYTE( "qi910224.hi", 0x00001, 0x10000, CRC(36e66d56) SHA1(0900c5272ec3ced550f18fb08db59ab7f67a621e))
ROM_END

ROM_START( ftsserv )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "fts10226.lo", 0x00000, 0x10000, CRC(efbd738f) SHA1(d5258760bafdaf1bf13c4a49da76d4b5e7b4ccbd))
	ROM_LOAD16_BYTE( "fts10226.hi", 0x00001, 0x10000, CRC(2460853f) SHA1(a6bba8d2f800140afd129c4d5278f7ae8fe7e63a))
	/* FT Server series Front Panel */
	ROM_REGION(0x10000,"front", 0)
	ROM_LOAD( "fp10009.bin",     0x0000, 0x8000, CRC(8aa7f718) SHA1(9ee6c6a5bb92622ea8d3805196d42ff68887d820))
ROM_END

ROM_START( apxenls3 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "31020.lo", 0x10000, 0x8000, CRC(a19678d2) SHA1(d13c12fa7e94333555eabf58b81bad421e21cd91))
	ROM_LOAD16_BYTE( "31020.hi", 0x10001, 0x8000, CRC(4922e020) SHA1(64e6448323dad2209e004cd93fa181582e768ed5))
ROM_END

ROM_START( aplanst )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "31024", "Bios 3-10-24")
	ROMX_LOAD("31024.lo", 0x10000, 0x8000, CRC(e52b59e1) SHA1(cfcaa4d8d658df8df463108ef30695bd4ee7a617), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD("31024.hi", 0x10001, 0x8000, CRC(7286aefa) SHA1(dfc0e3f4936780fa62ae9ec392ce17aa65e717cd), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "31025", "Bios 3-10-25")
	ROMX_LOAD("31025.lo", 0x10000, 0x8000, CRC(1aec09bc) SHA1(51d56c97c7c1674554aa89b68945329ea967a8bc), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD("31025.hi", 0x10001, 0x8000, CRC(0763caa5) SHA1(48510a933dcd6efea3b14d04444f584c3e6fefeb), ROM_SKIP(1) | ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(2, "31026", "Bios 3-10-26i")
	ROMX_LOAD("31026i.lo", 0x10000, 0x8000, CRC(670b6ab4) SHA1(8d61a0edf187f99b67eb58f5e11276deee801d17), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD("31026i.hi", 0x10001, 0x8000, CRC(ef01c54f) SHA1(911f95d65ab96878e5e7ebccfc4b329db47a1351), ROM_SKIP(1) | ROM_BIOS(3) )
ROM_END

ROM_START( aplannb )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "lsl31025.lo", 0x00000, 0x10000, CRC(8bb7229b) SHA1(31449d12884ec4e7752e6c1ce7ce9e0d044eadf2))
	ROM_LOAD16_BYTE( "lsh31025.hi", 0x00001, 0x10000, CRC(09e5c1b9) SHA1(d42be83b4181d3733268c29df04a4d2918370f4e))
ROM_END

ROM_START( apvxft )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "ft10221.lo", 0x00000, 0x10000, CRC(8f339de0) SHA1(a6542406746eaf1ff7f9e3678c5cbe5522fb314a))
	ROM_LOAD16_BYTE( "ft10221.hi", 0x00001, 0x10000, CRC(3b16bc31) SHA1(0592d1d81e7fd4715b0612083482db122d78c7f2))
ROM_END

ROM_START( aplscar )
	ROM_REGION(0x40000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "car306", "Caracal 3.06")
	ROMX_LOAD("car306.bin",   0x00000, 0x40000, CRC(fc271dea) SHA1(6207cfd312c9957243b8157c90a952404e43b237), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "car307", "Caracal 3.07")
	ROMX_LOAD("car307.bin",   0x00000, 0x40000, CRC(66a01852) SHA1(b0a68c9d67921d27ba483a1c50463406c08d3085), ROM_BIOS(2))
ROM_END

ROM_START( apxena1 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD("a1-r26.bin",   0x00000, 0x20000, CRC(d29e983e) SHA1(5977df7f8d7ac2a154aa043bb6f539d96d51fcad))
ROM_END

ROM_START( apxenp2 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "p2r02g2", "p2r02g2")
	ROMX_LOAD("p2r02g2.bin",   0x00000, 0x20000, CRC(311bcc5a) SHA1(be6fa144322077dcf66b065e7f4e61aab8c278b4), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "lep121s", "SCSI-Enabling ROMs")
	ROMX_LOAD("p2r01f0.bin",   0x00000, 0x20000, CRC(bbc68f2e) SHA1(6954a52a7dda5521794151aff7a04225e9c7df77), ROM_BIOS(2))
ROM_END

ROM_START( apxeni )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "lep121", "Rom Bios 1.2.1")
	ROMX_LOAD( "lep121.bin", 0x18000, 0x8000, CRC(948c1927) SHA1(d06bdbd6292db73c815ad1060daf055293dfddf5), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "lep121s", "SCSI-Enabling ROMs")
	ROMX_LOAD( "lep121s.bin", 0x18000, 0x8000, CRC(296118e4) SHA1(d1feaa9704e6ce3bc10c900bdd310d9494b02304), ROM_BIOS(2))
ROM_END

ROM_START( aplsbon )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "bon106", "Boinsai 1-06")
	ROMX_LOAD("bon106.bin",   0x00000, 0x20000, CRC(98a4eb76) SHA1(e0587afa78aeb9a8803f9b9f9e457e9847b0a2b2), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "bon203", "Boinsai 2-03")
	ROMX_LOAD("bon203.bin",   0x00000, 0x20000, CRC(32a0e125) SHA1(a4fcbd76952599993fa8b76aa36a96386648abb2), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "bon10703", "Boinsai 1-07-03")
	ROMX_LOAD("bon10703.bin",   0x00000, 0x20000, CRC(0275b3c2) SHA1(55ef4cbb7f3166f678aaa478234a42049deaba5f), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "bon20402", "Boinsai 2.03")
	ROMX_LOAD("bon20402.bin",   0x00000, 0x20000, CRC(ac5803fb) SHA1(b8fe92711c6a38a5d9e6497e76a0929c1685c631), ROM_BIOS(4))
ROM_END

ROM_START( apxlsam )
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS(0, "sam107", "ROM BIOS Version 1-07")
	ROMX_LOAD("sam1-07.bin",   0x00000, 0x20000, CRC(65e05a8e) SHA1(c3cd198a129122cb05a28798e54331b06cfdd310), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "sam206", "ROM BIOS Version 2-06")
	ROMX_LOAD("sam2-06.bin",   0x00000, 0x20000, CRC(9768bb0f) SHA1(8166b77b133072f72f23debf85984eb19578ffc1), ROM_BIOS(2))
ROM_END

ROM_START( aprpand )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "pf10226.std", 0x00000, 0x10000, CRC(7396fb87) SHA1(a109cbad2179eec55f86c0297a59bb015461da21))
	ROM_CONTINUE( 0x00001, 0x10000 )
ROM_END

ROM_START( aprfte )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD( "1-2r2-4.486", 0x00000, 0x20000, CRC(bccc236d) SHA1(0765299363e68cf65710a688c360a087856ece8f))
ROM_END

ROM_START( megapc )
	ROM_REGION(0x40000, "isa", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "41651-bios lo.u18",  0x00000, 0x10000, CRC(1e9bd3b7) SHA1(14fd39ec12df7fae99ccdb0484ee097d93bf8d95))
	ROM_LOAD16_BYTE( "211253-bios hi.u19", 0x00001, 0x10000, CRC(6acb573f) SHA1(376d483db2bd1c775d46424e1176b24779591525))
ROM_END

ROM_START( megapcpl )
	ROM_REGION(0x40000, "isa", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "41652.u18",  0x00000, 0x10000, CRC(6f5b9a1c) SHA1(cae981a35a01234fcec99a96cb38075d7bf23474))
	ROM_LOAD16_BYTE( "486slc.u19", 0x00001, 0x10000, CRC(6fb7e3e9) SHA1(c439cb5a0d83176ceb2a3555e295dc1f84d85103))
ROM_END

ROM_START( megapcpla )
	ROM_REGION(0x40000, "bios", 0)
	ROM_LOAD( "megapc_bios.bin",  0x00000, 0x10000, CRC(b84938a2) SHA1(cecab72a96993db4f7c648c229b4211a8c53a380))
	ROM_CONTINUE(0x30000, 0x10000)
ROM_END

ROM_START( t2000sx )
	ROM_REGION( 0x20000, "bios", 0 )
	ROM_LOAD( "014d.ic9", 0x00000, 0x20000, CRC(e9010b02) SHA1(75688fc8e222640fa22bcc90343c6966fe0da87f))
ROM_END

ROM_START( pc2386 )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "c000.bin", 0x00000, 0x4000, CRC(33145bbf) SHA1(c49eaec19f656482e12c8bf282cd4ee5986d227d) )
	ROM_LOAD( "f000.bin", 0x30000, 0x10000, CRC(f54a063c) SHA1(ce70ec493053afab662f51199ef9c9304a209b8e) )
	ROM_FILL(0x3fff1, 1, 0x5b) // f000:e05b is the standard at reset vector jump address
	ROM_FILL(0x3fff2, 1, 0xe0) // why does this rom's point to nowhere sane?
	ROM_FILL(0x3fff3, 1, 0x00) // and why does the rest of the rom look okay?
	ROM_FILL(0x3fff4, 1, 0xf0)


	ROM_REGION( 0x1000, "keyboard", 0 ) // PC2286 / PC2386 102-key keyboard
	ROM_LOAD( "40211.ic801", 0x000, 0x1000, CRC(4440d981) SHA1(a76006a929f26c178e09908c66f28abc92e7744c) )
ROM_END

ROM_START( k286i )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "81_1598", 0x18000, 0x4000, CRC(e25a1e43) SHA1(d00b976ac94323f3867b1c256e315839c906dd5a) )
	ROM_LOAD16_BYTE( "81_1599", 0x18001, 0x4000, CRC(08e2a17b) SHA1(a86ef116e82eb9240e60b52f76e5e510cdd393fd) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*     YEAR  NAME      PARENT   COMPAT   MACHINE    INPUT       INIT    COMPANY     FULLNAME */
COMP ( 1984, ibm5170,  0,       ibm5150, ibm5170,   0,    at_state,      at,      "International Business Machines",  "IBM PC/AT 5170", MACHINE_NOT_WORKING )
COMP ( 1985, ibm5170a, ibm5170, 0,       ibm5170a,  0,    at_state,      at,      "International Business Machines",  "IBM PC/AT 5170 8MHz", MACHINE_NOT_WORKING )
COMP ( 1985, ibm5162,  ibm5170, 0,       ibm5162,   0,    at_state,      at,      "International Business Machines",  "IBM PC/XT-286 5162", MACHINE_NOT_WORKING )
COMP ( 1989, ibmps1es, ibm5170, 0,       ibmps1,    0,    at_state,      at,      "International Business Machines",  "IBM PS/1 (Spanish)", MACHINE_NOT_WORKING )
COMP ( 1987, at,       ibm5170, 0,       ibm5162,   0,    at_state,      at,      "<generic>",  "PC/AT (CGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP ( 1987, atvga,    ibm5170, 0,       atvga,     0,    at_state,      at,      "<generic>",  "PC/AT (VGA, MF2 Keyboard)" , MACHINE_NOT_WORKING )
COMP ( 1988, at386,    ibm5170, 0,       at386,     0,    at_state,      at,      "<generic>",  "PC/AT 386 (VGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP ( 1988, ct386sx,  ibm5170, 0,       ct386sx,   0,    at_state,      at,      "<generic>",  "NEAT 386SX (VGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
//COMP ( 1988, at386sx,  ibm5170, 0,       ct386sx,   0,    at_state,      at,      "<generic>",  "PC/AT 386SX (VGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP ( 1990, at486,    ibm5170, 0,       at486,     0,    at_state,      at,      "<generic>",  "PC/AT 486 (VGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP ( 1989, neat,     ibm5170, 0,       neat,      0,    at_state,      at,      "<generic>",  "NEAT (VGA, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP ( 1989, ec1842,   ibm5150, 0,       ec1842,    0,    at_state,      at,      "<unknown>",  "EC-1842", MACHINE_NOT_WORKING )
COMP ( 1993, ec1849,   ibm5170, 0,       ec1842,    0,    at_state,      at,      "<unknown>",  "EC-1849", MACHINE_NOT_WORKING )
COMP ( 1993, megapc,   0,       0,       megapc,    0,    megapc_state,  megapc,   "Amstrad plc", "MegaPC", MACHINE_NOT_WORKING )
COMP ( 199?, megapcpl, megapc,  0,       megapcpl,  0,    megapc_state,  megapcpl, "Amstrad plc", "MegaPC Plus", MACHINE_NOT_WORKING )
COMP ( 199?, megapcpla, megapc, 0,       megapcpla, 0,    at_state,      megapcpla,"Amstrad plc", "MegaPC Plus (WINBUS chipset)", MACHINE_NOT_WORKING )
COMP ( 1989, pc2386,   ibm5170, 0,       at386l,    0,    at_state,      at,      "Amstrad plc", "Amstrad PC2386", MACHINE_NOT_WORKING )
COMP ( 1991, aprfte,   ibm5170, 0,       at486,     0,    at_state,      at,      "Apricot",  "Apricot FT//ex 486 (J3 Motherboard)", MACHINE_NOT_WORKING )
COMP ( 1991, ftsserv,  ibm5170, 0,       at486,     0,    at_state,      at,      "Apricot",  "Apricot FTs (Scorpion)", MACHINE_NOT_WORKING )
COMP ( 1992, aprpand,  ibm5170, 0,       at486,     0,    at_state,      at,      "Apricot",  "Apricot FTs (Panther Rev F 1.02.26)", MACHINE_NOT_WORKING )
COMP ( 1990, aplanst,  ibm5170, 0,       at386,     0,    at_state,      at,      "Apricot",  "Apricot LANstation (Krypton Motherboard)", MACHINE_NOT_WORKING )
COMP ( 1990, aplannb,  ibm5170, 0,       at386,     0,    at_state,      at,      "Apricot",  "Apricot LANstation (Novell Remote Boot)", MACHINE_NOT_WORKING )
COMP ( 1992, aplscar,  ibm5170, 0,       at486l,    0,    at_state,      at,      "Apricot",  "Apricot LS Pro (Caracal Motherboard)", MACHINE_NOT_WORKING )
COMP ( 1992, aplsbon,  ibm5170, 0,       at486,     0,    at_state,      at,      "Apricot",  "Apricot LS Pro (Bonsai Motherboard)", MACHINE_NOT_WORKING )
COMP ( 1988, xb42663,  ibm5170, 0,       at386,     0,    at_state,      at,      "Apricot",  "Apricot Qi 300 (Rev D,E & F Motherboard)", MACHINE_NOT_WORKING )
COMP ( 1988, qi600,    ibm5170, 0,       at386,     0,    at_state,      at,      "Apricot",  "Apricot Qi 600 (Neptune Motherboard)", MACHINE_NOT_WORKING )
COMP ( 1990, qi900,    ibm5170, 0,       at486,     0,    at_state,      at,      "Apricot",  "Apricot Qi 900 (Scorpion Motherboard)", MACHINE_NOT_WORKING )
COMP ( 1989, apvxft,   ibm5170, 0,       at486,     0,    at_state,      at,      "Apricot",  "Apricot VX FT server", MACHINE_NOT_WORKING )
COMP ( 1991, apxenls3, ibm5170, 0,       at486,     0,    at_state,      at,      "Apricot",  "Apricot XEN-LS (Venus IV Motherboard)", MACHINE_NOT_WORKING )
COMP ( 1993, apxlsam,  ibm5170, 0,       at486,     0,    at_state,      at,      "Apricot",  "Apricot XEN-LS II (Samurai Motherboard)", MACHINE_NOT_WORKING )
COMP ( 1987, apxeni,   ibm5170, 0,       at386,     0,    at_state,      at,      "Apricot",  "Apricot XEN-i 386 (Leopard Motherboard)" , MACHINE_NOT_WORKING )
COMP ( 1989, xb42639,  ibm5170, 0,       xb42639,   0,    at_state,      at,      "Apricot",  "Apricot XEN-S (Venus I Motherboard 286)" , MACHINE_NOT_WORKING )
COMP ( 1990, xb42639a, ibm5170, 0,       xb42639,   0,    at_state,      at,      "Apricot",  "Apricot XEN-S (Venus II Motherboard 286)" , MACHINE_NOT_WORKING )
COMP ( 1989, xb42664,  ibm5170, 0,       at386,     0,    at_state,      at,      "Apricot",  "Apricot XEN-S (Venus I Motherboard 386)" , MACHINE_NOT_WORKING )
COMP ( 1990, xb42664a, ibm5170, 0,       at386,     0,    at_state,      at,      "Apricot",  "Apricot XEN-S (Venus II Motherboard 386)" , MACHINE_NOT_WORKING )
COMP ( 1993, apxena1,  ibm5170, 0,       at486,     0,    at_state,      at,      "Apricot",  "Apricot XEN PC (A1 Motherboard)", MACHINE_NOT_WORKING )
COMP ( 1993, apxenp2,  ibm5170, 0,       at486,     0,    at_state,      at,      "Apricot",  "Apricot XEN PC (P2 Motherboard)", MACHINE_NOT_WORKING )
COMP ( 1990, c386sx16, ibm5170, 0,       at386sx,   0,    at_state,      at,      "Commodore Business Machines", "Commodore 386SX-16", MACHINE_NOT_WORKING )
COMP ( 1988, cmdpc30,  ibm5170, 0,       ibm5162,   0,    at_state,      at,      "Commodore Business Machines",  "PC 30 III", MACHINE_NOT_WORKING )
COMP ( 1995, ficpio2,  ibm5170, 0,       ficpio2,   0,    at_state,      atpci,   "FIC", "486-PIO-2", MACHINE_NOT_WORKING )
COMP ( 1985, k286i,    ibm5170, 0,       k286i,     0,    at_state,      at,      "Kaypro",   "286i", MACHINE_NOT_WORKING )
COMP ( 1991, t2000sx,  ibm5170, 0,       at386sx,   0,    at_state,      at,      "Toshiba",  "T2000SX", MACHINE_NOT_WORKING )
