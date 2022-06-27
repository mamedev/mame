// license:GPL-2.0+
// copyright-holders:Kevin Thacker,Sandro Ronco
/* Core includes */
#include "emu.h"
#include "kc.h"

#define KC_DEBUG 0
#define LOG(x) do { if (KC_DEBUG) logerror x; } while (0)

struct kcc_header
{
	uint8_t   name[10];
	uint8_t   reserved[6];
	uint8_t   number_addresses;
	uint8_t   load_address_l;
	uint8_t   load_address_h;
	uint8_t   end_address_l;
	uint8_t   end_address_h;
	uint8_t   execution_address_l;
	uint8_t   execution_address_h;
	uint8_t   pad[128-2-2-2-1-16];
};

/* appears to work a bit.. */
/* load file, then type: MENU and it should now be displayed. */
/* now type name that has appeared! */

/* load snapshot */
QUICKLOAD_LOAD_MEMBER(kc_state::quickload_cb)
{
	struct kcc_header *header;
	uint16_t addr;
	uint16_t datasize;
	uint16_t execution_address;
	uint16_t i;

	/* get file size */
	uint64_t size = image.length();

	if (size == 0)
		return image_init_result::FAIL;

	std::vector<uint8_t> data(size);
	image.fread( &data[0], size);

	header = (struct kcc_header *) &data[0];
	addr = (header->load_address_l & 0x0ff) | ((header->load_address_h & 0x0ff)<<8);
	datasize = ((header->end_address_l & 0x0ff) | ((header->end_address_h & 0x0ff)<<8)) - addr;
	execution_address = (header->execution_address_l & 0x0ff) | ((header->execution_address_h & 0x0ff)<<8);

	if (datasize > size - 128)
	{
		osd_printf_info("Invalid snapshot size: expected 0x%04x, found 0x%04x\n", datasize, (uint32_t)size - 128);
		datasize = size - 128;
	}

	address_space &space = m_maincpu->space( AS_PROGRAM );

	for (i=0; i<datasize; i++)
		space.write_byte((addr+i) & 0xffff, data[i+128]);

	if (execution_address != 0 && header->number_addresses >= 3 )
	{
		// if specified, jumps to the quickload start address
		m_maincpu->set_pc(execution_address);
	}

	logerror("Snapshot loaded at: 0x%04x-0x%04x, execution address: 0x%04x\n", addr, addr + datasize - 1, execution_address);

	return image_init_result::PASS;
}


//**************************************************************************
//  MODULE SYSTEM EMULATION
//**************************************************************************

// The KC85/4 and KC85/3 are "modular systems". These computers can be expanded with modules.

uint8_t kc_state::expansion_read(offs_t offset)
{
	uint8_t result = 0xff;

	// assert MEI line of first slot
	m_expansions[0]->mei_w(ASSERT_LINE);

	for (auto & elem : m_expansions)
		elem->read(offset, result);

	return result;
}

void kc_state::expansion_write(offs_t offset, uint8_t data)
{
	// assert MEI line of first slot
	m_expansions[0]->mei_w(ASSERT_LINE);

	for (auto & elem : m_expansions)
		elem->write(offset, data);
}

/*
    port xx80

    - xx is module id.

    Only addresses divisible by 4 are checked.
    If module does not exist, 0x0ff is returned.

    When xx80 is read, if a module exists a id will be returned.
    Id's for known modules are listed above.
*/

uint8_t kc_state::expansion_io_read(offs_t offset)
{
	uint8_t result = 0xff;

	// assert MEI line of first slot
	m_expansions[0]->mei_w(ASSERT_LINE);

	if ((offset & 0xff) == 0x80)
	{
		uint8_t slot_id = (offset>>8) & 0xff;

		if (slot_id == 0x08 || slot_id == 0x0c)
			result = m_expansions[(slot_id - 8) >> 2]->module_id_r();
		else
			m_expansions[2]->io_read(offset, result);
	}
	else
	{
		for (auto & elem : m_expansions)
			elem->io_read(offset, result);
	}

	return result;
}

void kc_state::expansion_io_write(offs_t offset, uint8_t data)
{
	// assert MEI line of first slot
	m_expansions[0]->mei_w(ASSERT_LINE);

	if ((offset & 0xff) == 0x80)
	{
		uint8_t slot_id = (offset>>8) & 0xff;

		if (slot_id == 0x08 || slot_id == 0x0c)
			m_expansions[(slot_id - 8) >> 2]->control_w(data);
		else
			m_expansions[2]->io_write(offset, data);
	}
	else
	{
		for (auto & elem : m_expansions)
			elem->io_write(offset, data);
	}
}

// module read/write handlers
uint8_t kc_state::expansion_4000_r(offs_t offset){ return expansion_read(offset + 0x4000); }
void kc_state::expansion_4000_w(offs_t offset, uint8_t data){ expansion_write(offset + 0x4000, data); }
uint8_t kc_state::expansion_8000_r(offs_t offset){ return expansion_read(offset + 0x8000); }
void kc_state::expansion_8000_w(offs_t offset, uint8_t data){ expansion_write(offset + 0x8000, data); }
uint8_t kc_state::expansion_c000_r(offs_t offset){ return expansion_read(offset + 0xc000); }
void kc_state::expansion_c000_w(offs_t offset, uint8_t data){ expansion_write(offset + 0xc000, data); }
uint8_t kc_state::expansion_e000_r(offs_t offset){ return expansion_read(offset + 0xe000); }
void kc_state::expansion_e000_w(offs_t offset, uint8_t data){ expansion_write(offset + 0xe000, data); }


//**************************************************************************
//  CASSETTE EMULATION
//**************************************************************************

/*
    The cassette motor is controlled by bit 6 of PIO port A.
    The cassette read data is connected to /ASTB input of the PIO.
    A edge from the cassette therefore will trigger a interrupt
    from the PIO.
    The duration between two edges can be timed and the data-bit
    identified.

    I have used a timer to feed data into /ASTB. The timer is only
    active when the cassette motor is on.
*/

void kc_state::update_cassette(int state)
{
	int astb = (state & m_ardy) ? 0 : 1;

	// if state is changed updates the /ASTB line
	if (m_astb ^ astb)
	{
		m_z80pio->strobe_a(astb);

		m_astb = astb;

		// FIXME: temporary for allow kc85_2-3 to load cassette
		if ((m_cassette->get_state() & 0x03) == CASSETTE_PLAY)
			m_z80pio->data_write(0, m_pio_data[0]);
	}
}

TIMER_CALLBACK_MEMBER(kc_state::kc_cassette_oneshot_timer)
{
	update_cassette(0);

	m_cassette_oneshot_timer->reset();
}

// timer used for polling data from cassette input
// enabled only when cassette motor is on
TIMER_CALLBACK_MEMBER(kc_state::kc_cassette_timer_callback)
{
	// read cassette data
	int bit = (m_cassette->input() > 0.0038) ? 1 : 0;

	// generates a pulse when the cassette input changes state
	if (bit ^ m_cassette_in)
	{
		update_cassette(1);
		m_cassette_in = bit;
		m_cassette_oneshot_timer->adjust(attotime::from_double(TIME_OF_74LS123(RES_K(10), CAP_N(1))));
	}
}

void kc_state::cassette_set_motor(int motor_state)
{
	/* set new motor state in cassette device */
	m_cassette->change_state(motor_state ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);

	if (motor_state)
	{
		/* start timer */
		m_cassette_timer->adjust(attotime::zero, 0, KC_CASSETTE_TIMER_FREQUENCY);
	}
	else
	{
		/* stop timer */
		m_cassette_timer->reset();
	}
}

/*
  pin 2 = gnd
  pin 3 = read
  pin 1 = k1        ?? modulating signal
  pin 4 = k0        ?? signal??
  pin 5 = motor on


    Tape signals:
        K0, K1      ??
        MOTON       motor control
        ASTB        read?

        T1-T4 give 4 bit a/d tone sound?
        K1, K0 are mixed with tone.

    Cassette read goes into ASTB of PIO.
    From this, KC must be able to detect the length
    of the pulses and can read the data.


    Tape write: clock comes from CTC?
    truck signal resets, 5v signal for set.
    output gives k0 and k1.

*/



//**************************************************************************
//  KC85 bankswitch
//**************************************************************************

/* update status of memory area 0x0000-0x03fff */
void kc_state::update_0x00000()
{
	address_space &space = m_maincpu->space( AS_PROGRAM );

	/* access ram? */
	if (m_pio_data[0] & (1<<1))
	{
		LOG(("ram0 enabled\n"));

		/* yes; set address of bank */
		space.install_rom(0x0000, 0x3fff, m_ram_base);

		/* write protect ram? */
		if ((m_pio_data[0] & (1<<3)) == 0)
		{
			/* yes */
			LOG(("ram0 write protected\n"));

			/* ram is enabled and write protected */
			space.unmap_write(0x0000, 0x3fff);
		}
		else
		{
			LOG(("ram0 write enabled\n"));

			/* ram is enabled and write enabled */
			space.install_writeonly(0x0000, 0x3fff, m_ram_base);
		}
	}
	else
	{
		LOG(("Module at 0x0000\n"));

		space.install_read_handler (0x0000, 0x3fff, read8sm_delegate(*this, FUNC(kc_state::expansion_read)), 0);
		space.install_write_handler(0x0000, 0x3fff, write8sm_delegate(*this, FUNC(kc_state::expansion_write)), 0);
	}
}

/* update status of memory area 0x4000-0x07fff */
void kc_state::update_0x04000()
{
	address_space &space = m_maincpu->space( AS_PROGRAM );

	LOG(("Module at 0x4000\n"));

	space.install_read_handler (0x4000, 0x7fff, read8sm_delegate(*this, FUNC(kc_state::expansion_4000_r)), 0);
	space.install_write_handler(0x4000, 0x7fff, write8sm_delegate(*this, FUNC(kc_state::expansion_4000_w)), 0);

}


/* update memory address 0x0c000-0x0e000 */
void kc_state::update_0x0c000()
{
	address_space &space = m_maincpu->space( AS_PROGRAM );

	if ((m_pio_data[0] & (1<<7)) && memregion("basic") != nullptr)
	{
		/* BASIC takes next priority */
			LOG(("BASIC rom 0x0c000\n"));

		space.install_rom(0xc000, 0xdfff, memregion("basic")->base());
		space.unmap_write(0xc000, 0xdfff);
	}
	else
	{
		LOG(("Module at 0x0c000\n"));

		space.install_read_handler (0xc000, 0xdfff, read8sm_delegate(*this, FUNC(kc_state::expansion_c000_r)), 0);
		space.install_write_handler(0xc000, 0xdfff, write8sm_delegate(*this, FUNC(kc_state::expansion_c000_w)), 0);
	}
}

/* update memory address 0x0e000-0x0ffff */
void kc_state::update_0x0e000()
{
	address_space &space = m_maincpu->space( AS_PROGRAM );

	if (m_pio_data[0] & (1<<0))
	{
		/* enable CAOS rom in memory range 0x0e000-0x0ffff */
		LOG(("CAOS rom 0x0e000\n"));
		/* read will access the rom */
		space.install_rom(0xe000, 0xffff, memregion("caos")->base() + 0x2000);
		space.unmap_write(0xe000, 0xffff);
	}
	else
	{
		LOG(("Module at 0x0e000\n"));

		space.install_read_handler (0xe000, 0xffff, read8sm_delegate(*this, FUNC(kc_state::expansion_e000_r)), 0);
		space.install_write_handler(0xe000, 0xffff, write8sm_delegate(*this, FUNC(kc_state::expansion_e000_w)), 0);
	}
}


/* update status of memory area 0x08000-0x0ffff */
void kc_state::update_0x08000()
{
	address_space &space = m_maincpu->space( AS_PROGRAM );

	if (m_pio_data[0] & (1<<2))
	{
		/* IRM enabled */
		LOG(("IRM enabled\n"));

		space.install_ram(0x8000, 0xbfff, &m_video_ram[0]);
	}
	else
	{
		LOG(("Module at 0x8000!\n"));

		space.install_read_handler(0x8000, 0xbfff, read8sm_delegate(*this, FUNC(kc_state::expansion_8000_r)), 0);
		space.install_write_handler(0x8000, 0xbfff, write8sm_delegate(*this, FUNC(kc_state::expansion_8000_w)), 0);
	}
}


/* update status of memory area 0x4000-0x07fff */
void kc85_4_state::update_0x04000()
{
	address_space &space = m_maincpu->space( AS_PROGRAM );

	/* access ram? */
	if (m_port_86_data & (1<<0))
	{
		LOG(("RAM4 enabled\n"));

		/* yes */
		space.install_rom(0x4000, 0x7fff, m_ram_base + 0x4000);

		/* write protect ram? */
		if ((m_port_86_data & (1<<1)) == 0)
		{
			/* yes */
			LOG(("ram4 write protected\n"));

			/* ram is enabled and write protected */
			space.nop_write(0x4000, 0x7fff);
		}
		else
		{
			LOG(("ram4 write enabled\n"));

			/* ram is enabled and write enabled */
			space.install_writeonly(0x4000, 0x7fff, m_ram_base + 0x4000);
		}
	}
	else
	{
		LOG(("Module at 0x4000\n"));

		space.install_read_handler (0x4000, 0x7fff, read8sm_delegate(*this, FUNC(kc_state::expansion_4000_r)), 0);
		space.install_write_handler(0x4000, 0x7fff, write8sm_delegate(*this, FUNC(kc_state::expansion_4000_w)), 0);
	}

}

/* update memory address 0x0c000-0x0e000 */
void kc85_4_state::update_0x0c000()
{
	address_space &space = m_maincpu->space( AS_PROGRAM );

	if (m_port_86_data & (1<<7))
	{
		/* CAOS rom takes priority */
		LOG(("CAOS rom 0x0c000\n"));

		space.install_rom(0xc000, 0xdfff, memregion("caos")->base());
		space.unmap_write(0xc000, 0xdfff);
	}
	else
	{
		if (m_pio_data[0] & (1<<7))
		{
			/* BASIC takes next priority */
			LOG(("BASIC rom 0x0c000\n"));

			int bank = memregion("basic")->bytes() == 0x8000 ? (m_port_86_data>>5) & 0x03 : 0;

			space.install_rom(0xc000, 0xdfff, memregion("basic")->base() + (bank << 13));
			space.unmap_write(0xc000, 0xdfff);
		}
		else
		{
			LOG(("Module at 0x0c000\n"));

			space.install_read_handler (0xc000, 0xdfff, read8sm_delegate(*this, FUNC(kc_state::expansion_c000_r)), 0);
			space.install_write_handler(0xc000, 0xdfff, write8sm_delegate(*this, FUNC(kc_state::expansion_c000_w)), 0);
		}
	}
}

void kc85_4_state::update_0x08000()
{
	address_space &space = m_maincpu->space( AS_PROGRAM );

	if (m_pio_data[0] & (1<<2))
	{
		/* IRM enabled - has priority over RAM8 enabled */
		LOG(("IRM enabled\n"));

		uint8_t* ram_page = &m_video_ram[(BIT(m_port_84_data, 2)<<15) | (BIT(m_port_84_data, 1)<<14)];

		space.install_ram(0x8000, 0xa7ff, ram_page);
		space.install_ram(0xa800, 0xbfff, &m_video_ram[0x2800]);
	}
	else if (m_pio_data[1] & (1<<5))
	{
		LOG(("RAM8 enabled\n"));

		int ram8_block;
		uint8_t *mem_ptr;

		/* ram8 block chosen */

		if (m_ram->size() == 64 * 1024)
		{
			// kc85_4 64K RAM
			ram8_block = ((m_port_84_data)>>4) & 0x01;
			mem_ptr = m_ram_base + 0x8000 + (ram8_block<<14);
		}
		else
		{
			// kc85_5 256K RAM
			ram8_block = ((m_port_84_data)>>4) & 0x0f;
			mem_ptr = m_ram_base + (ram8_block<<14);
		}

		space.install_rom(0x8000, 0xa7ff, mem_ptr);
		space.install_rom(0xa800, 0xbfff, mem_ptr + 0x2800);

		/* write protect RAM8 ? */
		if ((m_pio_data[1] & (1<<6)) == 0)
		{
			/* ram8 is enabled and write protected */
			LOG(("RAM8 write protected\n"));

			space.nop_write(0x8000, 0xa7ff);
			space.nop_write(0xa800, 0xbfff);
		}
		else
		{
			LOG(("RAM8 write enabled\n"));

			/* ram8 is enabled and write enabled */
			space.install_writeonly(0x8000, 0xa7ff, mem_ptr);
			space.install_writeonly(0xa800, 0xbfff, mem_ptr + 0x2800);
		}
	}
	else
	{
		LOG(("Module at 0x8000\n"));

		space.install_read_handler(0x8000, 0xbfff, read8sm_delegate(*this, FUNC(kc_state::expansion_8000_r)), 0);
		space.install_write_handler(0x8000, 0xbfff, write8sm_delegate(*this, FUNC(kc_state::expansion_8000_w)), 0);
	}
}

//**************************************************************************
//  KC85 Z80PIO Interface
//**************************************************************************


/* PIO PORT A: port 0x088:

bit 7: ROM C (BASIC)
bit 6: Tape Motor on
bit 5: LED
bit 4: K OUT
bit 3: WRITE PROTECT RAM 0
bit 2: IRM
bit 1: ACCESS RAM 0
bit 0: CAOS ROM E
*/

uint8_t kc_state::pio_porta_r()
{
	return m_pio_data[0];
}

void kc_state::pio_porta_w(uint8_t data)
{
	if (m_pio_data[0] != data) // to avoid a severe slowdown during cassette loading
	{
		m_pio_data[0] = data;

		update_0x00000();
		update_0x08000();
		update_0x0c000();
		update_0x0e000();

		cassette_set_motor(BIT(data, 6));
	}
}


/* PIO PORT B: port 0x089:
bit 7: BLINK ENABLE
bit 6: WRITE PROTECT RAM 8
bit 5: ACCESS RAM 8
bit 4: TONE 4
bit 3: TONE 3
bit 2: TONE 2
bit 1: TONE 1
bit 0: TRUCK */

uint8_t kc_state::pio_portb_r()
{
	return m_pio_data[1];
}

void kc_state::pio_portb_w(uint8_t data)
{
	m_pio_data[1] = data;

	update_0x08000();

	/* 16 speaker levels */
	m_speaker_level = (data>>1) & 0x0f;

	speaker_update();
}

/* port 0x84/0x85:

bit 7: RAF3
bit 6: RAF2
bit 5: RAF1
bit 4: RAF0
bit 3: FPIX. high resolution
bit 2: BLA1 .access screen
bit 1: BLA0 .pixel/color
bit 0: BILD .display screen 0 or 1
*/

void kc85_4_state::kc85_4_84_w(uint8_t data)
{
	LOG(("0x84 W: %02x\n", data));

	m_port_84_data = data;

	video_control_w(data);

	update_0x08000();
}

uint8_t kc85_4_state::kc85_4_84_r()
{
	return m_port_84_data;
}


/* port 0x86/0x87:

bit 7: ROCC
bit 6: ROF1
bit 5: ROF0
bit 4-2 are not connected
bit 1: WRITE PROTECT RAM 4
bit 0: ACCESS RAM 4
*/

void kc85_4_state::kc85_4_86_w(uint8_t data)
{
	LOG(("0x86 W: %02x\n", data));

	m_port_86_data = data;

	update_0x0c000();
	update_0x04000();
}

uint8_t kc85_4_state::kc85_4_86_r()
{
	return m_port_86_data;
}

/*****************************************************************/


/* callback for ardy output from PIO */
/* used in KC85/4 & KC85/3 cassette interface */
WRITE_LINE_MEMBER( kc_state::pio_ardy_cb)
{
	m_ardy = state & 0x01;
}

/* callback for brdy output from PIO */
/* used in KC85/4 & KC85/3 keyboard interface */
WRITE_LINE_MEMBER( kc_state::pio_brdy_cb)
{
	m_brdy = state & 0x01;
}

/* used in cassette write -> K0 */
WRITE_LINE_MEMBER( kc_state::ctc_zc0_callback )
{
	if (state)
	{
		m_k0_line^=1;
		speaker_update();
	}
}

/* used in cassette write -> K1 */
WRITE_LINE_MEMBER( kc_state::ctc_zc1_callback)
{
	if (state)
	{
		m_k1_line^=1;
		speaker_update();

		// K1 line is also cassette output
		m_cassette->output((m_k1_line & 1) ? +1 : -1);
	}

}

TIMER_DEVICE_CALLBACK_MEMBER(kc_state::kc_scanline)
{
	int scanline = (int)param;

	/* set clock input for channel 0 and 1 to ctc */
	m_z80ctc->trg0(1);
	m_z80ctc->trg0(0);
	m_z80ctc->trg1(1);
	m_z80ctc->trg1(0);

	if (scanline == 256)
	{
		/* set clock input for channel 2 and 3 to ctc */
		m_z80ctc->trg2(1);
		m_z80ctc->trg2(0);
		m_z80ctc->trg3(1);
		m_z80ctc->trg3(0);
	}
}

void kc_state::speaker_update()
{
	/* this might not be correct, the range might be logarithmic and not linear! */
	m_speaker->level_w(m_k0_line ? (m_speaker_level | (m_k1_line ? 0x01 : 0)) : 0);
}

/* keyboard callback */
WRITE_LINE_MEMBER( kc_state::keyboard_cb )
{
	m_z80pio->strobe_b(state & m_brdy);

	// FIXME: understand why the PIO fail to acknowledge the irq on kc85_2/3
	m_z80pio->data_write(1, m_pio_data[1]);
}


void kc_state::machine_start()
{
	m_cassette_timer = timer_alloc(FUNC(kc_state::kc_cassette_timer_callback), this);
	m_cassette_oneshot_timer = timer_alloc(FUNC(kc_state::kc_cassette_oneshot_timer), this);

	m_ram_base = m_ram->pointer();
}

void kc_state::machine_reset()
{
	m_pio_data[0] = 0x0f;
	m_pio_data[1] = 0xf1;

	update_0x00000();
	update_0x04000();
	update_0x08000();
	update_0x0c000();
	update_0x0e000();

	// set low resolution at reset
	m_high_resolution = 0;

	cassette_set_motor(0);

	/* this is temporary. Normally when a Z80 is reset, it will
	execute address 0. It appears the KC85 series pages the rom
	at address 0x0000-0x01000 which has a single jump in it,
	can't see yet where it disables it later!!!! so for now
	here will be a override */
	m_maincpu->set_pc(0x0f000);
}

void kc85_4_state::machine_reset()
{
	kc_state::machine_reset();

	m_port_84_data = 0x00;
	m_port_86_data = 0x00;
}
