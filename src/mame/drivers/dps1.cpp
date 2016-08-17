// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/****************************************************************************************************************

Ithaca Intersystems DPS-1

The last commercial release of a computer fitted with a front panel.

ToDo:
- Need artwork of the front panel switches and LEDs, and port FF.
- Replace terminal with s2651 UART and RS232.

***************************************************************************************************************/
#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/upd765.h"
#include "machine/terminal.h"
#include "softlist.h"

class dps1_state : public driver_device
{
public:
	dps1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		//, m_floppy1(*this, "fdc:1")
		, m_terminal(*this, "terminal")
	{ }

	DECLARE_READ8_MEMBER(port00_r);
	DECLARE_WRITE8_MEMBER(port00_w);
	DECLARE_WRITE8_MEMBER(portb2_w);
	DECLARE_WRITE8_MEMBER(portb4_w);
	DECLARE_WRITE8_MEMBER(portb6_w);
	DECLARE_WRITE8_MEMBER(portb8_w);
	DECLARE_WRITE8_MEMBER(portba_w);
	DECLARE_WRITE8_MEMBER(portbc_w);
	DECLARE_WRITE8_MEMBER(portbe_w);
	DECLARE_READ8_MEMBER(portff_r);
	DECLARE_WRITE8_MEMBER(portff_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_DRIVER_INIT(dps1);
	DECLARE_MACHINE_RESET(dps1);
	DECLARE_WRITE8_MEMBER(kbd_put);

private:
	bool m_dma_dir;
	UINT16 m_dma_adr;
	UINT8 m_term_data;
	required_device<cpu_device> m_maincpu;
	required_device<upd765_family_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	//required_device<floppy_connector> m_floppy1;
	required_device<generic_terminal_device> m_terminal;
};

static ADDRESS_MAP_START( dps1_mem, AS_PROGRAM, 8, dps1_state )
	AM_RANGE(0x0000, 0x03ff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x0400, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( dps1_io, AS_IO, 8, dps1_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_READWRITE(port00_r,port00_w) // 2651 uart
	AM_RANGE(0xb0, 0xb1) AM_DEVICE("fdc", upd765_family_device, map)
	AM_RANGE(0xb2, 0xb3) AM_WRITE(portb2_w) // set dma fdc->memory
	AM_RANGE(0xb4, 0xb5) AM_WRITE(portb4_w) // set dma memory->fdc
	AM_RANGE(0xb6, 0xb7) AM_WRITE(portb6_w) // enable eprom
	AM_RANGE(0xb8, 0xb9) AM_WRITE(portb8_w) // set A16-23
	AM_RANGE(0xba, 0xbb) AM_WRITE(portba_w) // set A8-15
	AM_RANGE(0xbc, 0xbd) AM_WRITE(portbc_w) // set A0-7
	AM_RANGE(0xbe, 0xbf) AM_WRITE(portbe_w) // disable eprom
	AM_RANGE(0xff, 0xff) AM_READWRITE(portff_r, portff_w)
ADDRESS_MAP_END

// uart in
READ8_MEMBER( dps1_state::port00_r )
{
	UINT8 data = 0x4e;
	switch(offset)
	{
		case 0:
			data = m_term_data;
			m_term_data = 0;
			break;
		case 1:
			data = (m_term_data) ? 3 : 1;
			break;
		case 3:
			data = 0x27;
		default:
			break;
	}
	return data;
}

// uart out
WRITE8_MEMBER( dps1_state::port00_w )
{
	if (offset == 0)
		m_terminal->write(space, 0, data);
}

// read from disk, to memory
WRITE8_MEMBER( dps1_state::portb2_w )
{
	m_dma_dir = 1;
}

// write to disk, from memory
WRITE8_MEMBER( dps1_state::portb4_w )
{
	m_dma_dir = 0;
}

// enable eprom
WRITE8_MEMBER( dps1_state::portb6_w )
{
	membank("bankr0")->set_entry(1); // point at rom
}

// set A16-23
WRITE8_MEMBER( dps1_state::portb8_w )
{
}

// set A8-15
WRITE8_MEMBER( dps1_state::portba_w )
{
	m_dma_adr = (data << 8) | (m_dma_adr & 0xff);
}

// set A0-7
WRITE8_MEMBER( dps1_state::portbc_w )
{
	m_dma_adr = (m_dma_adr & 0xff00) | data;
}

// disable eprom
WRITE8_MEMBER( dps1_state::portbe_w )
{
	membank("bankr0")->set_entry(0); // point at ram
}

// read 8 front-panel switches
READ8_MEMBER( dps1_state::portff_r )
{
	return 0x0e;
}

// write to 8 leds
WRITE8_MEMBER( dps1_state::portff_w )
{
}

// do dma
WRITE_LINE_MEMBER( dps1_state::fdc_drq_w )
{
	if (state)
	{
		// acknowledge drq by taking /dack low (unsupported)
		// then depending on direction, transfer a byte
		address_space& mem = m_maincpu->space(AS_PROGRAM);
		if (m_dma_dir)
		{ // disk to mem
			mem.write_byte(m_dma_adr, m_fdc->mdma_r(mem, 0));
		}
		else
		{ // mem to disk
			m_fdc->mdma_w(mem, 0, mem.read_byte(m_dma_adr));
		}
		m_dma_adr++;
	}
	// else take /dack high (unsupported)
}

MACHINE_RESET_MEMBER( dps1_state, dps1 )
{
	membank("bankr0")->set_entry(1); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
	// set fdc for 8 inch floppies
	m_fdc->set_rate(500000);
	// turn on the motor
	floppy_image_device *floppy = m_floppy0->get_device();
	m_fdc->set_floppy(floppy);
	floppy->mon_w(0);
}

DRIVER_INIT_MEMBER( dps1_state, dps1 )
{
	UINT8 *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x0400]);
	membank("bankw0")->configure_entry(0, &main[0x0400]);
}

static INPUT_PORTS_START( dps1 )
INPUT_PORTS_END

WRITE8_MEMBER( dps1_state::kbd_put )
{
	m_term_data = data;
}

static SLOT_INTERFACE_START( floppies )
	SLOT_INTERFACE( "floppy0", FLOPPY_8_DSDD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( dps1, dps1_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(dps1_mem)
	MCFG_CPU_IO_MAP(dps1_io)
	MCFG_MACHINE_RESET_OVERRIDE(dps1_state, dps1)

	/* video hardware */
	MCFG_DEVICE_ADD("terminal", GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(dps1_state, kbd_put))

	// floppy
	MCFG_UPD765A_ADD("fdc", false, true)
	//MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(dps1_state, fdc_int_w)) // doesn't appear to be used
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(dps1_state, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", floppies, "floppy0", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	//MCFG_FLOPPY_DRIVE_ADD("fdc:1", floppies, "floppy1", floppy_image_device::default_floppy_formats)
	//MCFG_FLOPPY_DRIVE_SOUND(true)

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "dps1")
MACHINE_CONFIG_END

ROM_START( dps1 )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "boot 1280", 0x000, 0x400, CRC(9c2e98fa) SHA1(78e6c9d00aa6e8f6c4d3c65984cfdf4e99434c66) ) // actually on the FDC-2 board
ROM_END

COMP( 1979, dps1, 0, 0, dps1, dps1, dps1_state, dps1, "Ithaca InterSystems", "DPS-1", MACHINE_NO_SOUND_HW )
