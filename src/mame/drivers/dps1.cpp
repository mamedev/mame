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

	uint8_t port00_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port00_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portb2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portb4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portb6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portb8_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portba_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portbc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portbe_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t portff_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void portff_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fdc_drq_w(int state);
	void init_dps1();
	void machine_reset_dps1();
	void kbd_put(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

private:
	bool m_dma_dir;
	uint16_t m_dma_adr;
	uint8_t m_term_data;
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
uint8_t dps1_state::port00_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	uint8_t data = 0x4e;
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
void dps1_state::port00_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if (offset == 0)
		m_terminal->write(space, 0, data);
}

// read from disk, to memory
void dps1_state::portb2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_dma_dir = 1;
}

// write to disk, from memory
void dps1_state::portb4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_dma_dir = 0;
}

// enable eprom
void dps1_state::portb6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	membank("bankr0")->set_entry(1); // point at rom
}

// set A16-23
void dps1_state::portb8_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
}

// set A8-15
void dps1_state::portba_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_dma_adr = (data << 8) | (m_dma_adr & 0xff);
}

// set A0-7
void dps1_state::portbc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_dma_adr = (m_dma_adr & 0xff00) | data;
}

// disable eprom
void dps1_state::portbe_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	membank("bankr0")->set_entry(0); // point at ram
}

// read 8 front-panel switches
uint8_t dps1_state::portff_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return 0x0e;
}

// write to 8 leds
void dps1_state::portff_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
}

// do dma
void dps1_state::fdc_drq_w(int state)
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

void dps1_state::machine_reset_dps1()
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

void dps1_state::init_dps1()
{
	uint8_t *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x0400]);
	membank("bankw0")->configure_entry(0, &main[0x0400]);
}

static INPUT_PORTS_START( dps1 )
INPUT_PORTS_END

void dps1_state::kbd_put(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
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
