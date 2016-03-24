// license:GPL-2.0+
// copyright-holders:Kevin Thacker,Sandro Ronco
/* Core includes */
#include "emu.h"
#include "includes/kc.h"

#define KC_DEBUG 0
#define LOG(x) do { if (KC_DEBUG) logerror x; } while (0)

struct kcc_header
{
	UINT8   name[10];
	UINT8   reserved[6];
	UINT8   number_addresses;
	UINT8   load_address_l;
	UINT8   load_address_h;
	UINT8   end_address_l;
	UINT8   end_address_h;
	UINT8   execution_address_l;
	UINT8   execution_address_h;
	UINT8   pad[128-2-2-2-1-16];
};

/* appears to work a bit.. */
/* load file, then type: MENU and it should now be displayed. */
/* now type name that has appeared! */

/* load snapshot */
QUICKLOAD_LOAD_MEMBER( kc_state,kc)
{
	struct kcc_header *header;
	UINT16 addr;
	UINT16 datasize;
	UINT16 execution_address;
	UINT16 i;

	/* get file size */
	UINT64 size = image.length();

	if (size == 0)
		return IMAGE_INIT_FAIL;

	dynamic_buffer data(size);
	image.fread( &data[0], size);

	header = (struct kcc_header *) &data[0];
	addr = (header->load_address_l & 0x0ff) | ((header->load_address_h & 0x0ff)<<8);
	datasize = ((header->end_address_l & 0x0ff) | ((header->end_address_h & 0x0ff)<<8)) - addr;
	execution_address = (header->execution_address_l & 0x0ff) | ((header->execution_address_h & 0x0ff)<<8);

	if (datasize > size - 128)
	{
		osd_printf_info("Invalid snapshot size: expected 0x%04x, found 0x%04x\n", datasize, (UINT32)size - 128);
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

	return IMAGE_INIT_PASS;
}


//**************************************************************************
//  MODULE SYSTEM EMULATION
//**************************************************************************

// The KC85/4 and KC85/3 are "modular systems". These computers can be expanded with modules.

READ8_MEMBER( kc_state::expansion_read )
{
	UINT8 result = 0xff;

	// assert MEI line of first slot
	m_expansions[0]->mei_w(ASSERT_LINE);

	for (auto & elem : m_expansions)
		elem->read(offset, result);

	return result;
}

WRITE8_MEMBER( kc_state::expansion_write )
{
	// assert MEI line of first slot
	m_expansions[0]->mei_w(ASSERT_LINE);

	for (auto & elem : m_expansions)
		elem->write(offset, data);
}

/*
    port xx80

    - xx is module id.

    Only addressess divisible by 4 are checked.
    If module does not exist, 0x0ff is returned.

    When xx80 is read, if a module exists a id will be returned.
    Id's for known modules are listed above.
*/

READ8_MEMBER( kc_state::expansion_io_read )
{
	UINT8 result = 0xff;

	// assert MEI line of first slot
	m_expansions[0]->mei_w(ASSERT_LINE);

	if ((offset & 0xff) == 0x80)
	{
		UINT8 slot_id = (offset>>8) & 0xff;

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

WRITE8_MEMBER( kc_state::expansion_io_write )
{
	// assert MEI line of first slot
	m_expansions[0]->mei_w(ASSERT_LINE);

	if ((offset & 0xff) == 0x80)
	{
		UINT8 slot_id = (offset>>8) & 0xff;

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
READ8_MEMBER ( kc_state::expansion_4000_r ){ return expansion_read(space, offset + 0x4000); }
WRITE8_MEMBER( kc_state::expansion_4000_w ){ expansion_write(space, offset + 0x4000, data); }
READ8_MEMBER ( kc_state::expansion_8000_r ){ return expansion_read(space, offset + 0x8000); }
WRITE8_MEMBER( kc_state::expansion_8000_w ){ expansion_write(space, offset + 0x8000, data); }
READ8_MEMBER ( kc_state::expansion_c000_r ){ return expansion_read(space, offset + 0xc000); }
WRITE8_MEMBER( kc_state::expansion_c000_w ){ expansion_write(space, offset + 0xc000, data); }
READ8_MEMBER ( kc_state::expansion_e000_r ){ return expansion_read(space, offset + 0xe000); }
WRITE8_MEMBER( kc_state::expansion_e000_w ){ expansion_write(space, offset + 0xe000, data); }


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
		space.install_read_bank(0x0000, 0x3fff, "bank1");
		membank("bank1")->set_base(m_ram_base);

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
			space.install_write_bank(0x0000, 0x3fff, "bank1");
		}
	}
	else
	{
		LOG(("Module at 0x0000\n"));

		space.install_read_handler (0x0000, 0x3fff, 0, 0, read8_delegate(FUNC(kc_state::expansion_read), this), 0);
		space.install_write_handler(0x0000, 0x3fff, 0, 0, write8_delegate(FUNC(kc_state::expansion_write), this), 0);
	}
}

/* update status of memory area 0x4000-0x07fff */
void kc_state::update_0x04000()
{
	address_space &space = m_maincpu->space( AS_PROGRAM );

	LOG(("Module at 0x4000\n"));

	space.install_read_handler (0x4000, 0x7fff, 0, 0, read8_delegate(FUNC(kc_state::expansion_4000_r), this), 0);
	space.install_write_handler(0x4000, 0x7fff, 0, 0, write8_delegate(FUNC(kc_state::expansion_4000_w), this), 0);

}


/* update memory address 0x0c000-0x0e000 */
void kc_state::update_0x0c000()
{
	address_space &space = m_maincpu->space( AS_PROGRAM );

	if ((m_pio_data[0] & (1<<7)) && memregion("basic") != nullptr)
	{
		/* BASIC takes next priority */
			LOG(("BASIC rom 0x0c000\n"));

		membank("bank4")->set_base(memregion("basic")->base());
		space.install_read_bank(0xc000, 0xdfff, "bank4");
		space.unmap_write(0xc000, 0xdfff);
	}
	else
	{
		LOG(("Module at 0x0c000\n"));

		space.install_read_handler (0xc000, 0xdfff, 0, 0, read8_delegate(FUNC(kc_state::expansion_c000_r), this), 0);
		space.install_write_handler(0xc000, 0xdfff, 0, 0, write8_delegate(FUNC(kc_state::expansion_c000_w), this), 0);
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
		membank("bank5")->set_base(memregion("caos")->base() + 0x2000);
		space.install_read_bank(0xe000, 0xffff, "bank5");
		space.unmap_write(0xe000, 0xffff);
	}
	else
	{
		LOG(("Module at 0x0e000\n"));

		space.install_read_handler (0xe000, 0xffff, 0, 0, read8_delegate(FUNC(kc_state::expansion_e000_r), this), 0);
		space.install_write_handler(0xe000, 0xffff, 0, 0, write8_delegate(FUNC(kc_state::expansion_e000_w), this), 0);
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

		membank("bank3")->set_base(m_video_ram);
		space.install_readwrite_bank(0x8000, 0xbfff, "bank3");
	}
	else
	{
		LOG(("Module at 0x8000!\n"));

		space.install_read_handler(0x8000, 0xbfff, 0, 0, read8_delegate(FUNC(kc_state::expansion_8000_r), this), 0);
		space.install_write_handler(0x8000, 0xbfff, 0, 0, write8_delegate(FUNC(kc_state::expansion_8000_w), this), 0);
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
		space.install_read_bank(0x4000, 0x7fff, "bank2");
		/* set address of bank */
		membank("bank2")->set_base(m_ram_base + 0x4000);

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
			space.install_write_bank(0x4000, 0x7fff, "bank2");
		}
	}
	else
	{
		LOG(("Module at 0x4000\n"));

		space.install_read_handler (0x4000, 0x7fff, 0, 0, read8_delegate(FUNC(kc_state::expansion_4000_r), this), 0);
		space.install_write_handler(0x4000, 0x7fff, 0, 0, write8_delegate(FUNC(kc_state::expansion_4000_w), this), 0);
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

		membank("bank4")->set_base(memregion("caos")->base());
		space.install_read_bank(0xc000, 0xdfff, "bank4");
		space.unmap_write(0xc000, 0xdfff);
	}
	else
	{
		if (m_pio_data[0] & (1<<7))
		{
			/* BASIC takes next priority */
			LOG(("BASIC rom 0x0c000\n"));

			int bank = memregion("basic")->bytes() == 0x8000 ? (m_port_86_data>>5) & 0x03 : 0;

			membank("bank4")->set_base(memregion("basic")->base() + (bank << 13));
			space.install_read_bank(0xc000, 0xdfff, "bank4");
			space.unmap_write(0xc000, 0xdfff);
		}
		else
		{
			LOG(("Module at 0x0c000\n"));

			space.install_read_handler (0xc000, 0xdfff, 0, 0, read8_delegate(FUNC(kc_state::expansion_c000_r), this), 0);
			space.install_write_handler(0xc000, 0xdfff, 0, 0, write8_delegate(FUNC(kc_state::expansion_c000_w), this), 0);
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

		UINT8* ram_page = m_video_ram + ((BIT(m_port_84_data, 2)<<15) | (BIT(m_port_84_data, 1)<<14));

		membank("bank3")->set_base(ram_page);
		space.install_readwrite_bank(0x8000, 0xa7ff, "bank3");

		membank("bank6")->set_base(m_video_ram + 0x2800);
		space.install_readwrite_bank(0xa800, 0xbfff, "bank6");
	}
	else if (m_pio_data[1] & (1<<5))
	{
		LOG(("RAM8 enabled\n"));

		int ram8_block;
		UINT8 *mem_ptr;

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

		membank("bank3")->set_base(mem_ptr);
		membank("bank6")->set_base(mem_ptr + 0x2800);
		space.install_read_bank(0x8000, 0xa7ff, "bank3");
		space.install_read_bank(0xa800, 0xbfff, "bank6");

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
			space.install_write_bank(0x8000, 0xa7ff, "bank3");
			space.install_write_bank(0xa800, 0xbfff, "bank6");
		}
	}
	else
	{
		LOG(("Module at 0x8000\n"));

		space.install_read_handler(0x8000, 0xbfff, 0, 0, read8_delegate(FUNC(kc_state::expansion_8000_r), this), 0);
		space.install_write_handler(0x8000, 0xbfff, 0, 0, write8_delegate(FUNC(kc_state::expansion_8000_w), this), 0);
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

READ8_MEMBER( kc_state::pio_porta_r )
{
	return m_pio_data[0];
}

WRITE8_MEMBER( kc_state::pio_porta_w )
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

READ8_MEMBER( kc_state::pio_portb_r )
{
	return m_pio_data[1];
}

WRITE8_MEMBER( kc_state::pio_portb_w )
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

WRITE8_MEMBER( kc85_4_state::kc85_4_84_w )
{
	LOG(("0x84 W: %02x\n", data));

	m_port_84_data = data;

	video_control_w(data);

	update_0x08000();
}

READ8_MEMBER( kc85_4_state::kc85_4_84_r )
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

WRITE8_MEMBER( kc85_4_state::kc85_4_86_w )
{
	LOG(("0x86 W: %02x\n", data));

	m_port_86_data = data;

	update_0x0c000();
	update_0x04000();
}

READ8_MEMBER( kc85_4_state::kc85_4_86_r )
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
	m_cassette_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(kc_state::kc_cassette_timer_callback),this));
	m_cassette_oneshot_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(kc_state::kc_cassette_oneshot_timer),this));

	m_ram_base = m_ram->pointer();

	m_expansions[0] = machine().device<kcexp_slot_device>("m8");
	m_expansions[1] = machine().device<kcexp_slot_device>("mc");
	m_expansions[2] = machine().device<kcexp_slot_device>("exp");
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
