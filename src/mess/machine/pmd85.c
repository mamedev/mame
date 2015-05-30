// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/***************************************************************************

  machine.c

  Functions to emulate general aspects of PMD-85 (RAM, ROM, interrupts,
  I/O ports)

  Krzysztof Strzecha

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "includes/pmd85.h"
#include "machine/pit8253.h"


enum {PMD85_LED_1, PMD85_LED_2, PMD85_LED_3};
enum {PMD85_1, PMD85_2, PMD85_2A, PMD85_2B, PMD85_3, ALFA, MATO, C2717};



void pmd85_state::pmd851_update_memory()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();

	if (m_startup_mem_map)
	{
		UINT8 *mem = m_region_maincpu->base();

		space.unmap_write(0x0000, 0x0fff);
		space.nop_write(0x1000, 0x1fff);
		space.unmap_write(0x2000, 0x2fff);
		space.nop_write(0x3000, 0x3fff);

		space.nop_read(0x1000, 0x1fff);
		space.nop_read(0x3000, 0x3fff);

		m_bank1->set_base(mem + 0x010000);
		m_bank3->set_base(mem + 0x010000);
		m_bank5->set_base(ram + 0xc000);

		m_bank6->set_base(mem + 0x010000);
		m_bank7->set_base(mem + 0x010000);
		m_bank8->set_base(ram + 0xc000);
	}
	else
	{
		space.install_write_bank(0x0000, 0x0fff, "bank1");
		space.install_write_bank(0x1000, 0x1fff, "bank2");
		space.install_write_bank(0x2000, 0x2fff, "bank3");
		space.install_write_bank(0x3000, 0x3fff, "bank4");
		space.install_write_bank(0x4000, 0x7fff, "bank5");

		space.install_read_bank(0x1000, 0x1fff, "bank2");
		space.install_read_bank(0x3000, 0x3fff, "bank4");

		m_bank1->set_base(ram);
		m_bank2->set_base(ram + 0x1000);
		m_bank3->set_base(ram + 0x2000);
		m_bank4->set_base(ram + 0x3000);
		m_bank5->set_base(ram + 0x4000);
	}
}

void pmd85_state::pmd852a_update_memory()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();

	if (m_startup_mem_map)
	{
		UINT8 *mem = m_region_maincpu->base();

		space.unmap_write(0x0000, 0x0fff);
		space.unmap_write(0x2000, 0x2fff);

		m_bank1->set_base(mem + 0x010000);
		m_bank2->set_base(ram + 0x9000);
		m_bank3->set_base(mem + 0x010000);
		m_bank4->set_base(ram + 0xb000);
		m_bank5->set_base(ram + 0xc000);
		m_bank6->set_base(mem + 0x010000);
		m_bank7->set_base(ram + 0x9000);
		m_bank8->set_base(mem + 0x010000);
		m_bank9->set_base(ram + 0xb000);
		m_bank10->set_base(ram + 0xc000);

	}
	else
	{
		space.install_write_bank(0x0000, 0x0fff, "bank1");
		space.install_write_bank(0x2000, 0x2fff, "bank3");

		m_bank1->set_base(ram);
		m_bank2->set_base(ram + 0x1000);
		m_bank3->set_base(ram + 0x2000);
		m_bank4->set_base(ram + 0x5000);
		m_bank5->set_base(ram + 0x4000);
	}
}

void pmd85_state::pmd853_update_memory()
{
	UINT8 *mem = m_region_maincpu->base();
	UINT8 *ram = m_ram->pointer();

	if (m_startup_mem_map)
	{
		m_bank1->set_base(mem + 0x010000);
		m_bank2->set_base(mem + 0x010000);
		m_bank3->set_base(mem + 0x010000);
		m_bank4->set_base(mem + 0x010000);
		m_bank5->set_base(mem + 0x010000);
		m_bank6->set_base(mem + 0x010000);
		m_bank7->set_base(mem + 0x010000);
		m_bank8->set_base(mem + 0x010000);
		m_bank9->set_base(ram);
		m_bank10->set_base(ram + 0x2000);
		m_bank11->set_base(ram + 0x4000);
		m_bank12->set_base(ram + 0x6000);
		m_bank13->set_base(ram + 0x8000);
		m_bank14->set_base(ram + 0xa000);
		m_bank15->set_base(ram + 0xc000);
		m_bank16->set_base(ram + 0xe000);
	}
	else
	{
		m_bank1->set_base(ram);
		m_bank2->set_base(ram + 0x2000);
		m_bank3->set_base(ram + 0x4000);
		m_bank4->set_base(ram + 0x6000);
		m_bank5->set_base(ram + 0x8000);
		m_bank6->set_base(ram + 0xa000);
		m_bank7->set_base(ram + 0xc000);
		m_bank8->set_base(m_pmd853_memory_mapping ? mem + 0x010000 : ram + 0xe000);
	}
}

void pmd85_state::alfa_update_memory()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();

	if (m_startup_mem_map)
	{
		UINT8 *mem = m_region_maincpu->base();

		space.unmap_write(0x0000, 0x0fff);
		space.unmap_write(0x1000, 0x33ff);
		space.nop_write(0x3400, 0x3fff);

		m_bank1->set_base(mem + 0x010000);
		m_bank2->set_base(mem + 0x011000);
		m_bank4->set_base(ram + 0xc000);
		m_bank5->set_base(mem + 0x010000);
		m_bank6->set_base(mem + 0x011000);
		m_bank7->set_base(ram + 0xc000);
	}
	else
	{
		space.install_write_bank(0x0000, 0x0fff, "bank1");
		space.install_write_bank(0x1000, 0x33ff, "bank2");
		space.install_write_bank(0x3400, 0x3fff, "bank3");

		m_bank1->set_base(ram);
		m_bank2->set_base(ram + 0x1000);
		m_bank3->set_base(ram + 0x3400);
		m_bank4->set_base(ram + 0x4000);
	}
}

void pmd85_state::mato_update_memory()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();

	if (m_startup_mem_map)
	{
		UINT8 *mem = m_region_maincpu->base();

		space.unmap_write(0x0000, 0x3fff);

		m_bank1->set_base(mem + 0x010000);
		m_bank2->set_base(ram + 0xc000);
		m_bank3->set_base(mem + 0x010000);
		m_bank4->set_base(ram + 0xc000);
	}
	else
	{
		space.install_write_bank(0x0000, 0x3fff, "bank1");

		m_bank1->set_base(ram);
		m_bank2->set_base(ram + 0x4000);
	}
}

void pmd85_state::c2717_update_memory()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	UINT8 *mem = m_region_maincpu->base();
	UINT8 *ram = m_ram->pointer();

	if (m_startup_mem_map)
	{
		space.unmap_write(0x0000, 0x3fff);

		m_bank1->set_base(mem + 0x010000);
		m_bank2->set_base(ram + 0x4000);
		m_bank3->set_base(mem + 0x010000);
		m_bank4->set_base(ram + 0xc000);
	}
	else
	{
		space.install_write_bank(0x0000, 0x3fff, "bank1");
		m_bank1->set_base(ram);
		m_bank2->set_base(ram + 0x4000);
	}
}

/*******************************************************************************

    Motherboard 8255 (PMD-85.1, PMD-85.2, PMD-85.3, Didaktik Alfa)
    --------------------------------------------------------------
        keyboard, speaker, LEDs

*******************************************************************************/

READ8_MEMBER(pmd85_state::pmd85_ppi_0_porta_r)
{
	return 0xff;
}

READ8_MEMBER(pmd85_state::pmd85_ppi_0_portb_r)
{
	return m_io_port[(m_ppi_port_outputs[0][0] & 0x0f)]->read() & m_io_port[15]->read();
}

READ8_MEMBER(pmd85_state::pmd85_ppi_0_portc_r)
{
	return 0xff;
}

WRITE8_MEMBER(pmd85_state::pmd85_ppi_0_porta_w)
{
	m_ppi_port_outputs[0][0] = data;
}

WRITE8_MEMBER(pmd85_state::pmd85_ppi_0_portb_w)
{
	m_ppi_port_outputs[0][1] = data;
}

WRITE8_MEMBER(pmd85_state::pmd85_ppi_0_portc_w)
{
	m_ppi_port_outputs[0][2] = data;
	set_led_status(machine(), PMD85_LED_2, (data & 0x08) ? 1 : 0);
	set_led_status(machine(), PMD85_LED_3, (data & 0x04) ? 1 : 0);
}

/*******************************************************************************

    Motherboard 8255 (Mato)
    -----------------------
        keyboard, speaker, LEDs, tape

*******************************************************************************/

READ8_MEMBER(pmd85_state::mato_ppi_0_portb_r)
{
	int i;
	UINT8 data = 0xff;

	for (i = 0; i < 8; i++)
	{
		if (!BIT(m_ppi_port_outputs[0][0], i))
			data &= m_io_port[i]->read();
	}
	return data;
}

READ8_MEMBER(pmd85_state::mato_ppi_0_portc_r)
{
	return m_io_port[8]->read() | 0x8f;
}

WRITE8_MEMBER(pmd85_state::mato_ppi_0_portc_w)
{
	m_ppi_port_outputs[0][2] = data;
	set_led_status(machine(), PMD85_LED_2, BIT(data, 3));
	set_led_status(machine(), PMD85_LED_3, BIT(data, 2));
}

/*******************************************************************************

    I/O board 8255
    --------------
        GPIO/0 (K3 connector), GPIO/1 (K4 connector)

*******************************************************************************/

READ8_MEMBER(pmd85_state::pmd85_ppi_1_porta_r)
{
	return 0xff;
}

READ8_MEMBER(pmd85_state::pmd85_ppi_1_portb_r)
{
	return 0xff;
}

READ8_MEMBER(pmd85_state::pmd85_ppi_1_portc_r)
{
	return 0xff;
}

WRITE8_MEMBER(pmd85_state::pmd85_ppi_1_porta_w)
{
	m_ppi_port_outputs[1][0] = data;
}

WRITE8_MEMBER(pmd85_state::pmd85_ppi_1_portb_w)
{
	m_ppi_port_outputs[1][1] = data;
}

WRITE8_MEMBER(pmd85_state::pmd85_ppi_1_portc_w)
{
	m_ppi_port_outputs[1][2] = data;
}

/*******************************************************************************

    I/O board 8255
    --------------
        IMS-2 (K5 connector)

    - 8251 - cassette recorder and V.24/IFSS (selectable by switch)

    - external interfaces connector (K2)

*******************************************************************************/

READ8_MEMBER(pmd85_state::pmd85_ppi_2_porta_r)
{
	return 0xff;
}

READ8_MEMBER(pmd85_state::pmd85_ppi_2_portb_r)
{
	return 0xff;
}

READ8_MEMBER(pmd85_state::pmd85_ppi_2_portc_r)
{
	return 0xff;
}

WRITE8_MEMBER(pmd85_state::pmd85_ppi_2_porta_w)
{
	m_ppi_port_outputs[2][0] = data;
}

WRITE8_MEMBER(pmd85_state::pmd85_ppi_2_portb_w)
{
	m_ppi_port_outputs[2][1] = data;
}

WRITE8_MEMBER(pmd85_state::pmd85_ppi_2_portc_w)
{
	m_ppi_port_outputs[2][2] = data;
}

/*******************************************************************************

    I/O board 8251
    --------------
        cassette recorder and V.24/IFSS (selectable by switch)

*******************************************************************************/

/*******************************************************************************

    I/O board external interfaces connector (K2)
    --------------------------------------------

*******************************************************************************/



/*******************************************************************************

    ROM Module 8255
    ---------------
        port A - data read
        ports B, C - address select

*******************************************************************************/

READ8_MEMBER(pmd85_state::pmd85_ppi_3_porta_r)
{
	if (memregion("user1")->base() != NULL)
		return memregion("user1")->base()[m_ppi_port_outputs[3][1] | (m_ppi_port_outputs[3][2] << 8)];
	else
		return 0;
}

READ8_MEMBER(pmd85_state::pmd85_ppi_3_portb_r)
{
	return 0xff;
}

READ8_MEMBER(pmd85_state::pmd85_ppi_3_portc_r)
{
	return 0xff;
}

WRITE8_MEMBER(pmd85_state::pmd85_ppi_3_porta_w)
{
	m_ppi_port_outputs[3][0] = data;
}

WRITE8_MEMBER(pmd85_state::pmd85_ppi_3_portb_w)
{
	m_ppi_port_outputs[3][1] = data;
}

WRITE8_MEMBER(pmd85_state::pmd85_ppi_3_portc_w)
{
	m_ppi_port_outputs[3][2] = data;
}

/*******************************************************************************

    I/O ports (PMD-85.1, PMD-85.2, PMD-85.3, Didaktik Alfa)
    -------------------------------------------------------

    I/O board
    1xxx11aa    external interfaces connector (K2)

    0xxx11aa    I/O board interfaces
        000111aa    8251 (casette recorder, V24)
        010011aa    8255 (GPIO/0, GPIO/1)
        010111aa    8253
        011111aa    8255 (IMS-2)

    Motherboard
    1xxx01aa    8255 (keyboard, speaker, LEDs)
            PMD-85.3 memory banking

    ROM Module
    1xxx10aa    8255 (ROM reading)

*******************************************************************************/

READ8_MEMBER(pmd85_state::pmd85_io_r)
{
	if (m_startup_mem_map)
	{
		return 0xff;
	}

	switch (offset & 0x0c)
	{
		case 0x04:  /* Motherboard */
				switch (offset & 0x80)
				{
					case 0x80:  /* Motherboard 8255 */
							return m_ppi8255_0->read(space, offset & 0x03);
				}
				break;
		case 0x08:  /* ROM module connector */
				switch (m_model)
				{
					case PMD85_1:
					case PMD85_2:
					case PMD85_2A:
					case C2717:
					case PMD85_3:
						if (m_rom_module_present)
						{
							switch (offset & 0x80)
							{
								case 0x80:  /* ROM module 8255 */
									return m_ppi8255_3->read(space, offset & 0x03);
							}
						}
						break;
				}
				break;
		case 0x0c:  /* I/O board */
				switch (offset & 0x80)
				{
					case 0x00:  /* I/O board interfaces */
							switch (offset & 0x70)
							{
								case 0x10:  /* 8251 (casette recorder, V24) */
										switch (offset & 0x01)
										{
											case 0x00: return m_uart->data_r(space, offset & 0x01);
											case 0x01: return m_uart->status_r(space, offset & 0x01);
										}
										break;
								case 0x40:      /* 8255 (GPIO/0, GPIO/1) */
										return m_ppi8255_1->read(space, offset & 0x03);
								case 0x50:  /* 8253 */
										return m_pit8253->read(space, offset & 0x03);
								case 0x70:  /* 8255 (IMS-2) */
										return m_ppi8255_2->read(space, offset & 0x03);
							}
							break;
					case 0x80:  /* external interfaces */
							break;
				}
				break;
	}

	logerror ("Reading from unmapped port: %02x\n", offset);
	return 0xff;
}

WRITE8_MEMBER(pmd85_state::pmd85_io_w)
{
	if (m_startup_mem_map)
	{
		m_startup_mem_map = 0;
		(this->*update_memory)();
	}

	switch (offset & 0x0c)
	{
		case 0x04:  /* Motherboard */
				switch (offset & 0x80)
				{
					case 0x80:  /* Motherboard 8255 */
							m_ppi8255_0->write(space, offset & 0x03, data);
							/* PMD-85.3 memory banking */
							if ((offset & 0x03) == 0x03)
							{
								m_pmd853_memory_mapping = data & 0x01;
								(this->*update_memory)();
							}
							break;
				}
				break;
		case 0x08:  /* ROM module connector */
				switch (m_model)
				{
					case PMD85_1:
					case PMD85_2:
					case PMD85_2A:
					case C2717:
					case PMD85_3:
						if (m_rom_module_present)
						{
							switch (offset & 0x80)
							{
								case 0x80:  /* ROM module 8255 */
										m_ppi8255_3->write(space, offset & 0x03, data);
										break;
							}
						}
						break;
				}
				break;
		case 0x0c:  /* I/O board */
				switch (offset & 0x80)
				{
					case 0x00:  /* I/O board interfaces */
							switch (offset & 0x70)
							{
								case 0x10:  /* 8251 (casette recorder, V24) */
										switch (offset & 0x01)
										{
											case 0x00: m_uart->data_w(space, offset & 0x01, data); break;
											case 0x01: m_uart->control_w(space, offset & 0x01, data); break;
										}
										break;
								case 0x40:      /* 8255 (GPIO/0, GPIO/0) */
										m_ppi8255_1->write(space, offset & 0x03, data);
										break;
								case 0x50:  /* 8253 */
										m_pit8253->write(space, offset & 0x03, data);
										logerror ("8253 writing. Address: %02x, Data: %02x\n", offset, data);
										break;
								case 0x70:  /* 8255 (IMS-2) */
										m_ppi8255_2->write(space, offset & 0x03, data);
										break;
							}
							break;
					case 0x80:  /* external interfaces */
							break;
				}
				break;
	}
}

/*******************************************************************************

    I/O ports (Mato)
    ----------------

    Motherboard
    1xxx01aa    8255 (keyboard, speaker, LEDs, tape)

*******************************************************************************/

READ8_MEMBER(pmd85_state::mato_io_r)
{
	if (m_startup_mem_map)
	{
		return 0xff;
	}

	switch (offset & 0x0c)
	{
		case 0x04:  /* Motherboard */
				switch (offset & 0x80)
				{
					case 0x80:  /* Motherboard 8255 */
							return m_ppi8255_0->read(space, offset & 0x03);
				}
				break;
	}

	logerror ("Reading from unmapped port: %02x\n", offset);
	return 0xff;
}

WRITE8_MEMBER(pmd85_state::mato_io_w)
{
	if (m_startup_mem_map)
	{
		m_startup_mem_map = 0;
		(this->*update_memory)();
	}

	switch (offset & 0x0c)
	{
		case 0x04:  /* Motherboard */
				switch (offset & 0x80)
				{
					case 0x80:  /* Motherboard 8255 */
							return m_ppi8255_0->write(space, offset & 0x03, data);
				}
				break;
	}
}

void pmd85_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_CASSETTE:
		pmd85_cassette_timer_callback(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in pmd85_state::device_timer");
	}
}

WRITE_LINE_MEMBER(pmd85_state::write_cas_tx)
{
	m_cas_tx = state;
}

TIMER_CALLBACK_MEMBER(pmd85_state::pmd85_cassette_timer_callback)
{
	int data;
	int current_level;

	if (!(m_io_dsw0->read() & 0x02))   /* V.24 / Tape Switch */
	{
		/* tape reading */
		if (m_cassette->get_state()&CASSETTE_PLAY)
		{
			switch (m_model)
			{
				case PMD85_1:
					if (m_clk_level_tape)
					{
						m_previous_level = (m_cassette->input() > 0.038) ? 1 : 0;
						m_clk_level_tape = 0;
					}
					else
					{
						current_level = (m_cassette->input() > 0.038) ? 1 : 0;

						if (m_previous_level!=current_level)
						{
							data = (!m_previous_level && current_level) ? 1 : 0;

							m_uart->write_rxd(data);

							m_clk_level_tape = 1;
						}
					}

					m_uart->write_rxc(m_clk_level_tape);
					return;
				case PMD85_2:
				case PMD85_2A:
				case C2717:
				case PMD85_3:
				case ALFA:
					/* not hardware data decoding */
					return;
			}
		}

		/* tape writing */
		if (m_cassette->get_state()&CASSETTE_RECORD)
		{
			data = m_cas_tx;
			data ^= m_clk_level_tape;
			m_cassette->output(data&0x01 ? 1 : -1);

			m_clk_level_tape = m_clk_level_tape ? 0 : 1;
			m_uart->write_txc(m_clk_level_tape);

			return;
		}

		m_clk_level_tape = 1;

		m_clk_level = m_clk_level ? 0 : 1;
		m_uart->write_txc(m_clk_level);
	}
}

INPUT_CHANGED_MEMBER(pmd85_state::pmd85_reset)
{
	machine().schedule_soft_reset();
}

void pmd85_state::pmd85_common_driver_init()
{
	static const char *const keynames[] = {
		"KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7",
		"KEY8", "KEY9", "KEY10", "KEY11", "KEY12", "KEY13", "KEY14", "KEY15"
	};

	for ( int i = 0; i < 16; i++ )
	{
		m_io_port[i] = ioport( keynames[i] );
	}

	m_previous_level = 0;
	m_clk_level = m_clk_level_tape = 1;
	m_cassette_timer = timer_alloc(TIMER_CASSETTE);
	m_cassette_timer->adjust(attotime::zero, 0, attotime::from_hz(2400));
}

DRIVER_INIT_MEMBER(pmd85_state,pmd851)
{
	m_model = PMD85_1;
	update_memory = &pmd85_state::pmd851_update_memory;
	pmd85_common_driver_init();
}

DRIVER_INIT_MEMBER(pmd85_state,pmd852a)
{
	m_model = PMD85_2A;
	update_memory = &pmd85_state::pmd852a_update_memory;
	pmd85_common_driver_init();
}

DRIVER_INIT_MEMBER(pmd85_state,pmd853)
{
	m_model = PMD85_3;
	update_memory = &pmd85_state::pmd853_update_memory;
	pmd85_common_driver_init();
}

DRIVER_INIT_MEMBER(pmd85_state,alfa)
{
	m_model = ALFA;
	update_memory = &pmd85_state::alfa_update_memory;
	pmd85_common_driver_init();
}

DRIVER_INIT_MEMBER(pmd85_state,mato)
{
	m_model = MATO;
	update_memory = &pmd85_state::mato_update_memory;

	static const char *const keynames[] = {
		"KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7", "KEY8"
	};

	for ( int i = 0; i < 9; i++ )
	{
		m_io_port[i] = ioport( keynames[i] );
	}
	for ( int i = 9; i < 16; i++ )
	{
		m_io_port[i] = NULL;
	}
}

DRIVER_INIT_MEMBER(pmd85_state,c2717)
{
	m_model = C2717;
	update_memory = &pmd85_state::c2717_update_memory;
	pmd85_common_driver_init();
}

void pmd85_state::machine_reset()
{
	int i, j;

	/* checking for Rom Module */
	switch (m_model)
	{
		case PMD85_1:
		case PMD85_2A:
		case PMD85_3:
		case C2717:
			m_rom_module_present = (m_io_dsw0->read() & 0x01) ? 1 : 0;
			break;
		case ALFA:
		case MATO:
			break;
	}

	for (i = 0; i < 4; i++)
		for (j = 0; j < 3; j++)
			m_ppi_port_outputs[i][j] = 0;

	/* memory initialization */
	memset(m_ram->pointer(), 0, sizeof(unsigned char)*0x10000);
	m_pmd853_memory_mapping = 1;
	m_startup_mem_map = 1;
	(this->*update_memory)();
}
