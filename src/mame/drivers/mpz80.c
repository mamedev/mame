// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Morrow MPZ80 "Decision"

    Skeleton driver

*/

/*

    TODO:

    - trap logic
        - trap stop
        - trap aux
        - trap halt
        - trap int
        - trap void
        - in/out trap
        - read/write trap
        - exec trap
    - front panel LEDs
    - keyboard
    - I/O
        - Mult I/O (8259A PIC, 3x 8250 ACE, uPD1990C RTC, 4K ROM/RAM)
    - floppy
        - DJ/DMA controller (Z80, 1K RAM, 2/4K ROM, TTL floppy control logic) for 5.25" floppy drives
        - DJ2D/B controller for 8" floppy drives
    - hard disk
        - HDC/DMA controller (Seagate ST-506/Shugart SA1000)
        - HDCA controller (Shugart SA4000/Fujitsu M2301B/Winchester M2320B)
    - AM9512 FPU
    - models
        - Decision I Desk Top Model D1 (MPZ80, MM65KS, Wunderbus)
        - Decision I Desk Top Model D2 (MPZ80, MM65KS, Wunderbus, DJDMA, 2x DSDD 5.25")
        - Decision I Desk Top Model D2A (MPZ80, MM65KS, Wunderbus, DJDMA, 2x DSDD 5.25", HDCDMA, 5 or 16 MB hard disk?)
        - Decision I Desk Top Model D3A (MPZ80, MM65KS, Wunderbus, DJDMA, 2x DSDD 5.25", HDCDMA, 5 or 16 MB hard disk?)
        - Decision I Desk Top Model D3C (MPZ80, MM65KS, Wunderbus, DJDMA, 2x DSDD 5.25", HDCDMA, 5 or 16 MB hard disk?)

*/

#include "includes/mpz80.h"
#include "softlist.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

enum
{
	NO_ACCESS = 0,
	READ_ONLY,
	EXECUTE_ONLY,
	FULL_ACCESS
};

#define TASK0   ((m_task & 0x0f) == 0)

#define R10 0x04


// mask register
#define MASK_STOP_ENBL      0x01
#define MASK_AUX_ENBL       0x02
#define MASK_TINT_ENBL      0x04
#define MASK_RUN_ENBL       0x08
#define MASK_HALT_ENBL      0x10
#define MASK_SINT_ENBL      0x20
#define MASK_IOENBL         0x40
#define MASK_ZIO_MODE       0x80



//**************************************************************************
//  INTERRUPTS
//**************************************************************************

//-------------------------------------------------
//  check_traps -
//-------------------------------------------------

inline void mpz80_state::check_traps()
{
	m_pretrap = !(m_trap_int & m_trap_halt & m_trap_stop & m_trap_aux);

	if (m_pretrap)
	{
		// latch trap condition
		m_status = m_trap_void;
		m_status |= m_trap_halt << 2;
		m_status |= m_trap_int << 3;
		m_status |= m_trap_stop << 4;
		m_status |= m_trap_aux << 5;

		// latch trap address
		m_pretrap_addr = ((m_addr >> 8) & 0xf0) | (m_pretrap_addr >> 4);

		// set M1 trap region start address
		m_trap_start = m_addr;
	}
}


//-------------------------------------------------
//  check_interrupt -
//-------------------------------------------------

inline void mpz80_state::check_interrupt()
{
	int tint_enbl = (m_mask & MASK_TINT_ENBL) ? 1 : 0;
	int sint_enbl = (m_mask & MASK_SINT_ENBL) ? 0 : 1;

	m_int_pend = !(m_nmi & m_pint);
	m_trap_int = !(m_int_pend & tint_enbl);

	int z80irq = CLEAR_LINE;
	int z80nmi = CLEAR_LINE;

	if (TASK0)
	{
		if (!m_pint && !sint_enbl) z80irq = ASSERT_LINE;
		if (!m_nmi && !sint_enbl) z80nmi = ASSERT_LINE;
	}
	else
	{
		if (!m_pint && !tint_enbl) z80irq = ASSERT_LINE;
		if (!m_nmi && !tint_enbl) z80nmi = ASSERT_LINE;
	}

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, z80irq);
	m_maincpu->set_input_line(INPUT_LINE_NMI, z80nmi);

	check_traps();
}



//**************************************************************************
//  MEMORY MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  get_address -
//-------------------------------------------------

inline offs_t mpz80_state::get_address(offs_t offset)
{
	UINT16 map_addr = ((m_task & 0x0f) << 5) | ((offset & 0xf000) >> 11);
	UINT8 map = m_map_ram[map_addr];
	//UINT8 attr = m_map_ram[map_addr + 1];

	//logerror("task %02x map_addr %03x map %02x attr %02x address %06x\n", m_task, map_addr, map, attr, offset);

	// T7 T6 T5 T4 M7 M6 M5 M4 M3 M2 M1 M0 A11 A10 A9 A8 A7 A6 A5 A4 A3 A2 A1 A0
	return ((m_task & 0xf0) << 16) | (map << 12) | (offset & 0xfff);
}


//-------------------------------------------------
//  mmu_r -
//-------------------------------------------------

READ8_MEMBER( mpz80_state::mmu_r )
{
	m_addr = get_address(offset);
	UINT8 data = 0;

	if (m_pretrap)
	{
		m_pretrap = 0;
		m_trap = 1;

		// latch trap address
		m_pretrap_addr = ((m_addr >> 8) & 0xf0) | (m_pretrap_addr >> 4);
		m_trap_addr = m_pretrap_addr;
	}

	if (TASK0 && (offset < 0x1000))
	{
		if (offset < 0x400)
		{
			data = m_ram->pointer()[offset & 0x3ff];
		}
		else if (offset == 0x400)
		{
			data = trap_addr_r(space, 0);
		}
		else if (offset == 0x401)
		{
			data = keyboard_r(space, 0);
		}
		else if (offset == 0x402)
		{
			data = switch_r(space, 0);
		}
		else if (offset == 0x403)
		{
			data = status_r(space, 0);
		}
		else if (offset >= 0x600 && offset < 0x800)
		{
			// TODO this might change the map RAM contents
		}
		else if (offset < 0xc00)
		{
			UINT16 rom_addr = (m_trap_reset << 10) | (offset & 0x3ff);
			data = m_rom->base()[rom_addr];
		}
		else
		{
			logerror("Unmapped LOCAL read at %06x\n", offset);
		}
	}
	else
	{
		data = m_s100->smemr_r(space, m_addr);
	}

	return data;
}


//-------------------------------------------------
//  mmu_w -
//-------------------------------------------------

WRITE8_MEMBER( mpz80_state::mmu_w )
{
	m_addr = get_address(offset);

	if (TASK0 && (offset < 0x1000))
	{
		if (offset < 0x400)
		{
			m_ram->pointer()[offset & 0x3ff] = data;
		}
		else if (offset == 0x400)
		{
			disp_seg_w(space, 0, data);
		}
		else if (offset == 0x401)
		{
			disp_col_w(space, 0, data);
		}
		else if (offset == 0x402)
		{
			task_w(space, 0, data);
		}
		else if (offset == 0x403)
		{
			mask_w(space, 0, data);
		}
		else if (offset >= 0x600 && offset < 0x800)
		{
			m_map_ram[offset - 0x600] = data;
		}
		else
		{
			logerror("Unmapped LOCAL write at %06x\n", offset);
		}
	}
	else
	{
		m_s100->mwrt_w(space, m_addr, data);
	}
}


//-------------------------------------------------
//  get_io_address -
//-------------------------------------------------

inline offs_t mpz80_state::get_io_address(offs_t offset)
{
	if (m_mask & MASK_ZIO_MODE)
	{
		// echo port address onto upper address lines (8080 emulation)
		offset = ((offset << 8) & 0xff00) | (offset & 0xff);
	}

	return offset;
}


//-------------------------------------------------
//  mmu_io_r -
//-------------------------------------------------

READ8_MEMBER( mpz80_state::mmu_io_r )
{
	return m_s100->sinp_r(space, get_io_address(offset));
}


//-------------------------------------------------
//  mmu_io_w -
//-------------------------------------------------

WRITE8_MEMBER( mpz80_state::mmu_io_w )
{
	m_s100->sout_w(space, get_io_address(offset), data);
}


//-------------------------------------------------
//  trap_addr_r - trap address register
//-------------------------------------------------

READ8_MEMBER( mpz80_state::trap_addr_r )
{
	/*

	    bit     description

	    0       DADDR 12
	    1       DADDR 13
	    2       DADDR 14
	    3       DADDR 15
	    4       I-ADDR 12
	    5       I-ADDR 13
	    6       I-ADDR 14
	    7       I-ADDR 15

	*/

	return m_trap_addr;
}


//-------------------------------------------------
//  status_r - trap status register
//-------------------------------------------------

READ8_MEMBER( mpz80_state::status_r )
{
	/*

	    bit     description

	    0       _TRAP VOID
	    1       _IORQ
	    2       _TRAP HALT
	    3       _TRAP INT
	    4       _TRAP STOP
	    5       _TRAP AUX
	    6       R10
	    7       _RD STB

	*/

	return m_status;
}


//-------------------------------------------------
//  task_w - task register
//-------------------------------------------------

WRITE8_MEMBER( mpz80_state::task_w )
{
	/*

	    bit     description

	    0       T0, A16
	    1       T1, A17
	    2       T2, A18
	    3       T3, A19
	    4       T4, S-100 A20
	    5       T5, S-100 A21
	    6       T6, S-100 A22
	    7       T7, S-100 A23

	*/

	m_task = data;

	m_trap_reset = 1;
	check_traps();
}


//-------------------------------------------------
//  mask_w - mask register
//-------------------------------------------------

WRITE8_MEMBER( mpz80_state::mask_w )
{
	/*

	    bit     description

	    0       _STOP ENBL
	    1       AUX ENBL
	    2       _TINT ENBL
	    3       RUN ENBL
	    4       _HALT ENBL
	    5       SINT ENBL
	    6       _IOENBL
	    7       _ZIO MODE

	*/

	m_mask = data;
}



//**************************************************************************
//  FRONT PANEL
//**************************************************************************

//-------------------------------------------------
//  keyboard_r - front panel keyboard
//-------------------------------------------------

READ8_MEMBER( mpz80_state::keyboard_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	return 0;
}


//-------------------------------------------------
//  switch_r - switch register
//-------------------------------------------------

READ8_MEMBER( mpz80_state::switch_r )
{
	/*

	    bit     description

	    0       _TRAP RESET
	    1       INT PEND
	    2       16C S6
	    3       16C S5
	    4       16C S4
	    5       16C S3
	    6       16C S2
	    7       16C S1

	*/

	UINT8 data = 0;

	// trap reset
	data |= m_trap_reset;

	// interrupt pending
	data |= m_int_pend << 1;

	// boot address
	data |= m_16c->read() & 0xfc;

	return data;
}


//-------------------------------------------------
//  disp_seg_w - front panel segment
//-------------------------------------------------

WRITE8_MEMBER( mpz80_state::disp_seg_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7

	*/
}


//-------------------------------------------------
//  disp_col_w - front panel column
//-------------------------------------------------

WRITE8_MEMBER( mpz80_state::disp_col_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7

	*/
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( mpz80_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( mpz80_mem, AS_PROGRAM, 8, mpz80_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(mmu_r, mmu_w)
/*
    Task 0 Segment 0 map:

    AM_RANGE(0x0000, 0x03ff) AM_RAM
    AM_RANGE(0x0400, 0x0400) AM_READWRITE(trap_addr_r, disp_seg_w)
    AM_RANGE(0x0401, 0x0401) AM_READWRITE(keyboard_r, disp_col_w)
    AM_RANGE(0x0402, 0x0402) AM_READWRITE(switch_r, task_w)
    AM_RANGE(0x0403, 0x0403) AM_READWRITE(status_r, mask_w)
    AM_RANGE(0x0600, 0x07ff) AM_RAM AM_SHARE("map_ram")
    AM_RANGE(0x0800, 0x0bff) AM_ROM AM_REGION(Z80_TAG, 0)
    AM_RANGE(0x0c00, 0x0c00) AM_DEVREADWRITE(AM9512_TAG, am9512_device, read, write)
*/
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( mpz80_io )
//-------------------------------------------------

static ADDRESS_MAP_START( mpz80_io, AS_IO, 8, mpz80_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(mmu_io_r, mmu_io_w)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( mpz80 )
//-------------------------------------------------

static INPUT_PORTS_START( mpz80 )
	PORT_START("16C")
	PORT_DIPNAME( 0xf8, 0xf8, "Power-On-Jump Address" ) PORT_DIPLOCATION("16C:1,2,3,4,5") PORT_CONDITION("12C", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(    0xf8, "F800H" )
	PORT_DIPSETTING(    0xf0, "F000H" )
	PORT_DIPSETTING(    0xe8, "E800H" )
	PORT_DIPSETTING(    0xe0, "E000H" )
	PORT_DIPSETTING(    0xd8, "D800H" )
	PORT_DIPSETTING(    0xd0, "D000H" )
	PORT_DIPSETTING(    0xc8, "C800H" )
	PORT_DIPSETTING(    0xc0, "C000H" )
	PORT_DIPSETTING(    0xb8, "B800H" )
	PORT_DIPSETTING(    0xb0, "B000H" )
	PORT_DIPSETTING(    0xa8, "A800H" )
	PORT_DIPSETTING(    0xa0, "A000H" )
	PORT_DIPSETTING(    0x98, "9800H" )
	PORT_DIPSETTING(    0x90, "9000H" )
	PORT_DIPSETTING(    0x88, "8800H" )
	PORT_DIPSETTING(    0x80, "8000H" )
	PORT_DIPSETTING(    0x78, "7800H" )
	PORT_DIPSETTING(    0x70, "7000H" )
	PORT_DIPSETTING(    0x68, "6800H" )
	PORT_DIPSETTING(    0x60, "6000H" )
	PORT_DIPSETTING(    0x58, "5800H" )
	PORT_DIPSETTING(    0x50, "5000H" )
	PORT_DIPSETTING(    0x48, "4800H" )
	PORT_DIPSETTING(    0x40, "4000H" )
	PORT_DIPSETTING(    0x38, "3800H" )
	PORT_DIPSETTING(    0x30, "3000H" )
	PORT_DIPSETTING(    0x28, "2800H" )
	PORT_DIPSETTING(    0x20, "2000H" )
	PORT_DIPSETTING(    0x18, "1800H" )
	PORT_DIPSETTING(    0x10, "Boot DJ/DMA" )
	PORT_DIPSETTING(    0x08, "Boot HD/DMA" )
	PORT_DIPSETTING(    0x00, "Boot HDCA" )
	PORT_DIPNAME( 0x70, 0x00, "Diagnostics" ) PORT_DIPLOCATION("16C:2,3,4") PORT_CONDITION("12C", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "Read Registers" )
	PORT_DIPSETTING(    0x10, "Write Registers" )
	PORT_DIPSETTING(    0x20, "Write Map RAMs" )
	PORT_DIPSETTING(    0x30, "Write R/W RAMs" )
	PORT_DIPSETTING(    0x40, "R/W FPP" )
	PORT_DIPSETTING(    0x50, "R/W S-100 Bus (High/Low)" )
	PORT_DIPSETTING(    0x60, "R/W S-100 Bus (Alternating)" )
	PORT_DIPSETTING(    0x70, "Read Switches" )
	PORT_DIPNAME( 0x04, 0x00, "Power Up" ) PORT_DIPLOCATION("16C:6")
	PORT_DIPSETTING(    0x04, "Boot Address" )
	PORT_DIPSETTING(    0x00, "Monitor" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) ) PORT_DIPLOCATION("16C:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x00, "S-100 MWRITE" ) PORT_DIPLOCATION("16C:8")
	PORT_DIPSETTING(    0x01, "Disabled" )
	PORT_DIPSETTING(    0x00, "Enabled" )

	PORT_START("12C")
	PORT_DIPNAME( 0x02, 0x02, "Operation Mode" )
	PORT_DIPSETTING(    0x02, "Monitor" )
	PORT_DIPSETTING(    0x00, "Diagnostic" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  S100_INTERFACE( s100_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( mpz80_state::s100_pint_w )
{
	m_pint = (state == ASSERT_LINE) ? 0 : 1;

	check_interrupt();
}

WRITE_LINE_MEMBER( mpz80_state::s100_nmi_w )
{
	if (state == ASSERT_LINE)
	{
		m_nmi = 0;
	}

	check_interrupt();
}

// slot devices
#include "bus/s100/dj2db.h"
#include "bus/s100/djdma.h"
#include "bus/s100/mm65k16s.h"
//#include "bus/s100/nsmdsa.h"
//#include "bus/s100/nsmdsad.h"
#include "bus/s100/wunderbus.h"

static SLOT_INTERFACE_START( mpz80_s100_cards )
	SLOT_INTERFACE("mm65k16s", S100_MM65K16S)
	SLOT_INTERFACE("wunderbus", S100_WUNDERBUS)
	SLOT_INTERFACE("dj2db", S100_DJ2DB)
	SLOT_INTERFACE("djdma", S100_DJDMA)
//  SLOT_INTERFACE("multio", S100_MULTIO)
//  SLOT_INTERFACE("hdcdma", S100_HDCDMA)
//  SLOT_INTERFACE("hdca", S100_HDCA)
SLOT_INTERFACE_END



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( mpz80 )
//-------------------------------------------------

void mpz80_state::machine_start()
{
	m_map_ram.allocate(0x200);
}


void mpz80_state::machine_reset()
{
	m_trap_reset = 0;
	m_trap = 1;
	m_trap_start = 0;

	m_nmi = 1;

	check_interrupt();
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( mpz80 )
//-------------------------------------------------

static MACHINE_CONFIG_START( mpz80, mpz80_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(mpz80_mem)
	MCFG_CPU_IO_MAP(mpz80_io)

	// S-100
	MCFG_S100_BUS_ADD()
	MCFG_S100_IRQ_CALLBACK(WRITELINE(mpz80_state, s100_pint_w))
	MCFG_S100_NMI_CALLBACK(WRITELINE(mpz80_state, s100_nmi_w))
	MCFG_S100_RDY_CALLBACK(INPUTLINE(Z80_TAG, Z80_INPUT_LINE_WAIT))
	MCFG_S100_SLOT_ADD("s100_1", mpz80_s100_cards, "mm65k16s")
	MCFG_S100_SLOT_ADD("s100_2", mpz80_s100_cards, "wunderbus")
	MCFG_S100_SLOT_ADD("s100_3", mpz80_s100_cards, "dj2db")
	MCFG_S100_SLOT_ADD("s100_4", mpz80_s100_cards, NULL)//"hdcdma")
	MCFG_S100_SLOT_ADD("s100_5", mpz80_s100_cards, NULL)
	MCFG_S100_SLOT_ADD("s100_6", mpz80_s100_cards, NULL)
	MCFG_S100_SLOT_ADD("s100_7", mpz80_s100_cards, NULL)
	MCFG_S100_SLOT_ADD("s100_8", mpz80_s100_cards, NULL)
	MCFG_S100_SLOT_ADD("s100_9", mpz80_s100_cards, NULL)
	MCFG_S100_SLOT_ADD("s100_10", mpz80_s100_cards, NULL)
	MCFG_S100_SLOT_ADD("s100_11", mpz80_s100_cards, NULL)
	MCFG_S100_SLOT_ADD("s100_12", mpz80_s100_cards, NULL)
	MCFG_S100_SLOT_ADD("s100_13", mpz80_s100_cards, NULL)
	MCFG_S100_SLOT_ADD("s100_14", mpz80_s100_cards, NULL)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("65K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "mpz80")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( mpz80 )
//-------------------------------------------------

ROM_START( mpz80 )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("447")
	ROM_SYSTEM_BIOS( 0, "373", "3.73" )
	ROMX_LOAD( "mpz80 mon3.73 fb34.17c", 0x0000, 0x1000, CRC(0bbffaec) SHA1(005ba726fc071f06cb1c969d170960438a3fc1a8), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "375", "3.75" )
	ROMX_LOAD( "mpz80 mon3.75 0706.17c", 0x0000, 0x1000, CRC(1118a592) SHA1(d70f94c09602cd0bdc4fbaeb14989e8cc1540960), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "447", "4.47" )
	ROMX_LOAD( "mon 4.47 f4f6.17c", 0x0000, 0x1000, CRC(b99c5d7f) SHA1(11181432ee524c7e5a68ead0671fc945256f5d1b), ROM_BIOS(3) )

	ROM_REGION( 0x20, "s100rev2", 0 )
	ROM_LOAD( "z80-2 15a.15a", 0x00, 0x20, CRC(8a84249d) SHA1(dfbc49c5944f110f48419fd893fa84f4f0e113b8) ) // 82S123 or 6331

	ROM_REGION( 0xeb, "s100rev3", 0 )
	ROM_LOAD( "z80-15a-a.15a", 0x00, 0xeb, CRC(713243cd) SHA1(802b318cc9795d87f03622e878d9a4d5d7dea7d4) ) // 82S153

	ROM_REGION( 0x104, "mm1", 0 )
	ROM_LOAD( "z80-2 5c.5c", 0x000, 0x104, CRC(732be0cd) SHA1(545a37e5a871fcd1bb59b30056b837f03889b4d5) ) // PAL16R4
ROM_END



//**************************************************************************
//  DRIVER INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  DRIVER_INIT( mpz80 )
//-------------------------------------------------

DIRECT_UPDATE_MEMBER(mpz80_state::mpz80_direct_update_handler)
{
	if (m_trap && address >= m_trap_start && address <= m_trap_start + 0xf)
	{
		direct.explicit_configure(m_trap_start, m_trap_start + 0xf, 0xf, m_rom->base() + ((m_trap_reset << 10) | 0x3f0));
		return ~0;
	}

	return address;
}

DRIVER_INIT_MEMBER(mpz80_state,mpz80)
{
	address_space &program = machine().device<cpu_device>(Z80_TAG)->space(AS_PROGRAM);
	program.set_direct_update_handler(direct_update_delegate(FUNC(mpz80_state::mpz80_direct_update_handler), this));
}



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    INIT    COMPANY                          FULLNAME        FLAGS
COMP( 1980, mpz80,  0,      0,      mpz80,  mpz80, mpz80_state,  mpz80,      "Morrow Designs",  "MPZ80",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
